/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/layout/layout_wrapper.h"

#include "base/log/ace_trace.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper_builder.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
RefPtr<LayoutWrapper> LayoutWrapper::GetOrCreateChildByIndex(int32_t index, bool addToRenderTree)
{
    if ((index >= currentChildCount_) || (index < 0)) {
        LOGE("fail to get layout wrapper due to index is illegal, total count: %{public}d, target index: %{public}d",
            currentChildCount_, index);
        return nullptr;
    }
    auto iter = children_.find(index);
    if (iter != children_.end()) {
        if (addToRenderTree) {
            iter->second->isActive_ = true;
            pendingRender_.emplace(index, iter->second);
        }
        return iter->second;
    }
    CHECK_NULL_RETURN(layoutWrapperBuilder_, nullptr);
    auto wrapper = layoutWrapperBuilder_->GetOrCreateWrapperByIndex(index);
    CHECK_NULL_RETURN(wrapper, nullptr);
    if (addToRenderTree) {
        wrapper->isActive_ = true;
        pendingRender_.emplace(index, wrapper);
    }
    return wrapper;
}

std::list<RefPtr<LayoutWrapper>> LayoutWrapper::GetAllChildrenWithBuild(bool addToRenderTree)
{
    std::list<RefPtr<LayoutWrapper>> childLayoutWrappers;
    for (const auto& [index, wrapper] : children_) {
        wrapper->isActive_ = true;
        childLayoutWrappers.emplace_back(wrapper);
    }
    if (layoutWrapperBuilder_) {
        auto buildItems = layoutWrapperBuilder_->ExpandAllChildWrappers();
        auto index = layoutWrapperBuilder_->GetStartIndex();
        auto insertIter = childLayoutWrappers.begin();
        std::advance(insertIter, index);
        childLayoutWrappers.splice(insertIter, buildItems);
    }
    if (addToRenderTree) {
        int32_t index = 0;
        for (const auto& child : childLayoutWrappers) {
            child->isActive_ = true;
            pendingRender_.emplace(index++, child);
        }
    }
    return childLayoutWrappers;
}

void LayoutWrapper::RemoveChildInRenderTree(const RefPtr<LayoutWrapper>& wrapper)
{
    auto iter = std::find_if(
        pendingRender_.begin(), pendingRender_.end(), [wrapper](const auto& value) { return value.second == wrapper; });
    if (iter == pendingRender_.end()) {
        LOGW("can not find current wrapper in pending render map");
        return;
    }
    iter->second->isActive_ = false;
    pendingRender_.erase(iter);
}

void LayoutWrapper::RemoveChildInRenderTree(int32_t index)
{
    auto iter = pendingRender_.find(index);
    if (iter == pendingRender_.end()) {
        LOGW("can not find current wrapper in pending render map");
        return;
    }
    iter->second->isActive_ = false;
    pendingRender_.erase(iter);
}

void LayoutWrapper::ResetHostNode()
{
    hostNode_.Reset();
}

RefPtr<FrameNode> LayoutWrapper::GetHostNode() const
{
    return hostNode_.Upgrade();
}

std::string LayoutWrapper::GetHostTag() const
{
    auto host = GetHostNode();
    if (host) {
        return host->GetTag();
    }
    return "";
}

// This will call child and self measure process.
void LayoutWrapper::Measure(const std::optional<LayoutConstraintF>& parentConstraint)
{
    if (!layoutAlgorithm_ || layoutAlgorithm_->SkipMeasure()) {
        LOGD("the layoutAlgorithm skip measure");
        return;
    }
    auto host = GetHostNode();
    if (!layoutProperty_ || !geometryNode_ || !host) {
        LOGE("Measure failed: the layoutProperty_ or geometryNode_ or host is nullptr");
        return;
    }
    if (!CheckMeasureFlag(layoutProperty_->GetPropertyChangeFlag()) &&
        parentConstraint == geometryNode_->GetParentLayoutConstraint()) {
        skipMeasureContent_ = true;
    }
    geometryNode_->SetParentLayoutConstraint(parentConstraint.value_or(LayoutConstraintF()));
    layoutProperty_->UpdateLayoutConstraint(parentConstraint.value_or(LayoutConstraintF()));
    layoutProperty_->UpdateContentConstraint();
    LOGI("Measure: %{public}s, Constraint: %{public}s", GetHostTag().c_str(),
        layoutProperty_->GetLayoutConstraint()->ToString().c_str());

    if (!skipMeasureContent_) {
        auto size = layoutAlgorithm_->MeasureContent(layoutProperty_->CreateContentConstraint(), this);
        if (size.has_value()) {
            geometryNode_->SetContentSize(size.value());
        }
    }

    layoutAlgorithm_->Measure(this);
    LOGI("on Measure Done: %{public}s, Size: %{public}s", GetHostTag().c_str(),
        geometryNode_->GetFrameSize().ToString().c_str());
}

// Called to perform layout children.
void LayoutWrapper::Layout(const std::optional<OffsetF>& parentGlobalOffset)
{
    if (!layoutAlgorithm_ || layoutAlgorithm_->SkipLayout()) {
        LOGD("the layoutAlgorithm skip layout");
        return;
    }
    auto host = GetHostNode();
    if (!layoutProperty_ || !geometryNode_ || !host) {
        LOGE("Layout failed: the layoutProperty_ or geometryNode_ or host or frameNode is nullptr");
        return;
    }
    geometryNode_->SetParentGlobalOffset(parentGlobalOffset.value_or(OffsetF(0, 0)));
    layoutAlgorithm_->Layout(this);
    LOGD("On Layout Done: %{public}s, Offset: %{public}s", GetHostTag().c_str(),
        geometryNode_->GetFrameOffset().ToString().c_str());
}

std::list<RefPtr<FrameNode>> LayoutWrapper::GetChildrenInRenderArea() const
{
    std::list<RefPtr<FrameNode>> frameNodes;
    for (const auto& [index, wrapper] : pendingRender_) {
        if (!wrapper) {
            continue;
        }
        auto host = wrapper->GetHostNode();
        if (host) {
            frameNodes.emplace_back(host);
        }
    }
    return frameNodes;
}

void LayoutWrapper::MountToHostOnMainThread()
{
    SwapDirtyLayoutWrapperOnMainThread();
}

void LayoutWrapper::SwapDirtyLayoutWrapperOnMainThread()
{
    for (const auto& [index, wrapper] : pendingRender_) {
        if (wrapper) {
            wrapper->SwapDirtyLayoutWrapperOnMainThread();
        }
    }

    if (layoutWrapperBuilder_) {
        layoutWrapperBuilder_->UpdateBuildCacheOnMainThread();
    }

    auto host = hostNode_.Upgrade();
    if (!host) {
        LOGE("the host is nullptr");
        return;
    }
    host->SwapDirtyLayoutWrapperOnMainThread(Claim(this));
}
} // namespace OHOS::Ace::NG

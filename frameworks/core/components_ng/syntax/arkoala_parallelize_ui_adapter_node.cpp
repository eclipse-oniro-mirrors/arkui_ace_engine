/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "core/components_ng/syntax/arkoala_parallelize_ui_adapter_node.h"

#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

int32_t ParallelizeUIAdapterNode::FrameCount() const
{
    auto value = getCount_ ? getCount_() : 0;
    LOGE("ParallelizeUIAdapterNode::FrameCount  %{public}d %{public}d", getCount_ ? 1 : 0, value);
    return value;
}

RefPtr<UINode> ParallelizeUIAdapterNode::GetFrameChildByIndex(
    uint32_t index, bool needBuild, bool isCache, bool addToRenderTree)
{
    if (!registerCallback_) {
        return nullptr;
    }
    auto node = reinterpret_cast<NG::FrameNode*>(registerCallback_(index, needBuild, isCache));
    auto child = Referenced::Claim(node);
    if (!child) {
        return nullptr;
    }
    childMap_.try_emplace(index, child);
    children_.clear();
    auto frameNode = AceType::DynamicCast<FrameNode>(child->GetFrameChildByIndex(0, true));
    if (!frameNode) {
        return nullptr;
    }
    if (addToRenderTree && !frameNode->IsActive()) {
        frameNode->SetActive(true);
        if (IsOnMainTree()) {
            frameNode->AttachToMainTree(false, GetContext());
        }
    }
    MarkNeedSyncRenderTree();
    return frameNode;
}

const std::list<RefPtr<UINode>>& ParallelizeUIAdapterNode::GetChildren(bool notDetach) const
{
    if (children_.empty()) {
        for (const auto& [key, value] : childMap_) {
            children_.emplace_back(value);
        }
    }
    ACE_SCOPED_TRACE("ParallelizeUIAdapterNode::GetChildren siez: %zu", children_.size());
    return children_;
}

void ParallelizeUIAdapterNode::Reset()
{
    LOGE("ParallelizeUIAdapterNode::FrameCount  Reset");
    MarkNeedSyncRenderTree(true);
    MarkNeedFrameFlushDirty(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

} // namespace OHOS::Ace::NG
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

#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {
UINode::~UINode() {}
void UINode::AddChild(const RefPtr<UINode>& child, int32_t slot) {}
void UINode::ReplaceChild(const RefPtr<UINode>& oldNode, const RefPtr<UINode>& newNode) {}
void UINode::Clean() {}
void UINode::MountToParent(const RefPtr<UINode>& parent, int32_t slot) {}
void UINode::OnRemoveFromParent() {}
void UINode::GetFocusChildren(std::list<RefPtr<FrameNode>>& children) const {}
void UINode::AttachToMainTree() {}
void UINode::DetachFromMainTree() {}
void UINode::MovePosition(int32_t slot) {}
void UINode::UpdateLayoutPropertyFlag() {}
void UINode::AdjustParentLayoutFlag(PropertyChangeFlag& flag) {}
void UINode::MarkDirtyNode(PropertyChangeFlag extraFlag) {}
void UINode::MarkNeedFrameFlushDirty(PropertyChangeFlag extraFlag) {}
void UINode::MarkNeedSyncRenderTree() {}
void UINode::RebuildRenderContextTree() {}
void UINode::OnDetachFromMainTree() {}
void UINode::OnAttachToMainTree() {}
void UINode::DumpTree(int32_t depth) {}
void UINode::AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout) {}
void UINode::GenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList) {}
void UINode::Build() {}
void UINode::SetActive(bool active) {}

std::list<RefPtr<UINode>>::iterator UINode::RemoveChild(const RefPtr<UINode>& child)
{
    return children_.end();
}

int32_t UINode::RemoveChildAndReturnIndex(const RefPtr<UINode>& child)
{
    return 1;
}

void UINode::RemoveChildAtIndex(int32_t index) {}

RefPtr<UINode> UINode::GetChildAtIndex(int32_t index) const
{
    return nullptr;
}

RefPtr<FrameNode> UINode::GetFocusParent() const
{
    return nullptr;
}

RefPtr<PipelineContext> UINode::GetContext()
{
    return nullptr;
}

HitTestResult UINode::TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    const TouchRestrict& touchRestrict, TouchTestResult& result)
{
    return HitTestResult::OUT_OF_REGION;
}

HitTestResult UINode::MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    MouseTestResult& onMouseResult, MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode)
{
    return HitTestResult::OUT_OF_REGION;
}

HitTestResult UINode::AxisTest(const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult)
{
    return HitTestResult::OUT_OF_REGION;
}

int32_t UINode::FrameCount() const
{
    return TotalChildCount();
}

int32_t UINode::TotalChildCount() const
{
    return 0;
}

int32_t UINode::GetChildIndexById(int32_t id)
{
    return -1;
}

RefPtr<LayoutWrapper> UINode::CreateLayoutWrapper(bool forceMeasure, bool forceLayout) const
{
    return nullptr;
}
} // namespace OHOS::Ace::NG

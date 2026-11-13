/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/frame_node_adopted_render_adapter.h"

#include <unordered_set>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/mixed_mount_registry.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"

namespace OHOS::Ace::NG {
RectF FrameNodeAdoptedRenderAdapter::GetBounds(const RSNodePtr& rsNode)
{
    CHECK_NULL_RETURN(rsNode, RectF());
    const auto& bounds = rsNode->GetStagingProperties().GetBounds();
    return RectF(bounds.x_, bounds.y_, bounds.z_, bounds.w_);
}

bool FrameNodeAdoptedRenderAdapter::CollectLayers(const FrameNode* frameNode, std::vector<RSNodePtr>& layers)
{
    if (!FrameNodeAdoptedCompensation::HasAdoptedCompensation(frameNode)) {
        return false;
    }
    auto adoptParent = AceType::DynamicCast<FrameNode>(frameNode->GetAdoptParent());
    auto childContext = AceType::DynamicCast<RosenRenderContext>(frameNode->GetRenderContext());
    auto parentContext = adoptParent
        ? AceType::DynamicCast<RosenRenderContext>(adoptParent->GetRenderContext()) : nullptr;
    if (!childContext || !parentContext) {
        return false;
    }
    auto childRsNode = childContext->GetRSNode();
    auto parentRsNode = parentContext->GetRSNode();
    if (!childRsNode || !parentRsNode) {
        return false;
    }

    auto current = childRsNode->GetParent();
    if (!current) {
        return MixedMountRegistry::GetParent(childRsNode->GetId()) == adoptParent;
    }
    std::unordered_set<uint64_t> visited;
    while (current && current != parentRsNode) {
        if (!visited.emplace(current->GetId()).second) {
            return false;
        }
        layers.emplace_back(current);
        auto parent = current->GetParent();
        if (!parent) {
            return MixedMountRegistry::GetParent(current->GetId()) == adoptParent;
        }
        current = parent;
    }
    return current == parentRsNode;
}

Matrix4 FrameNodeAdoptedRenderAdapter::GetTransform(const RSNodePtr& rsNode)
{
    auto bounds = GetBounds(rsNode);
    return RosenRenderContext::GetMatrixWithTransformRotate(rsNode, bounds);
}

VectorF FrameNodeAdoptedRenderAdapter::GetScale(const RSNodePtr& rsNode)
{
    CHECK_NULL_RETURN(rsNode, VectorF(1.0f, 1.0f));
    auto transform = rsNode->GetStagingProperties().GetAllTransformPropertyValues();
    return VectorF(transform.scale[0], transform.scale[1]);
}

RectF FrameNodeAdoptedRenderAdapter::GetPaintRect(const RSNodePtr& rsNode)
{
    return RosenRenderContext::GetPaintRectWithTransform(rsNode, GetBounds(rsNode));
}

OffsetF FrameNodeAdoptedRenderAdapter::GetTranslate(const RSNodePtr& rsNode)
{
    CHECK_NULL_RETURN(rsNode, OffsetF());
    auto translate = rsNode->GetStagingProperties().GetTranslate();
    return OffsetF(translate[0], translate[1]);
}
} // namespace OHOS::Ace::NG

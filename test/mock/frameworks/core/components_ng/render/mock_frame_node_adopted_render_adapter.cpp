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

#include "base/geometry/matrix4.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/vector.h"

namespace OHOS::Ace::NG {
bool FrameNodeAdoptedRenderAdapter::CollectLayers(const FrameNode* frameNode, std::vector<RSNodePtr>& layers)
{
    // MockRenderContext has no RS node chain. Keep the real compensation fallback under test.
    layers.clear();
    return false;
}

RectF FrameNodeAdoptedRenderAdapter::GetBounds(const RSNodePtr& rsNode)
{
    return RectF();
}

Matrix4 FrameNodeAdoptedRenderAdapter::GetTransform(const RSNodePtr& rsNode)
{
    return Matrix4();
}

VectorF FrameNodeAdoptedRenderAdapter::GetScale(const RSNodePtr& rsNode)
{
    return VectorF(1.0f, 1.0f);
}

RectF FrameNodeAdoptedRenderAdapter::GetPaintRect(const RSNodePtr& rsNode)
{
    return RectF();
}

OffsetF FrameNodeAdoptedRenderAdapter::GetTranslate(const RSNodePtr& rsNode)
{
    return OffsetF();
}
} // namespace OHOS::Ace::NG

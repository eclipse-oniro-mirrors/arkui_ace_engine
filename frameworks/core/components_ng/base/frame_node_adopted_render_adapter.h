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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_RENDER_ADAPTER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_RENDER_ADAPTER_H

#include <memory>
#include <vector>

#include "core/components_ng/base/frame_node_adopted_compensation.h"

namespace OHOS::Rosen {
class RSNode;
}

namespace OHOS::Ace::NG {
// Keeps RS node access separate from compensation geometry so unit tests can replace the adapter.
class FrameNodeAdoptedRenderAdapter final {
public:
    using RSNodePtr = std::shared_ptr<Rosen::RSNode>;

    static bool CollectLayers(const FrameNode* frameNode, std::vector<RSNodePtr>& layers);
    static RectF GetBounds(const RSNodePtr& rsNode);
    static Matrix4 GetTransform(const RSNodePtr& rsNode);
    static VectorF GetScale(const RSNodePtr& rsNode);
    static RectF GetPaintRect(const RSNodePtr& rsNode);
    static OffsetF GetTranslate(const RSNodePtr& rsNode);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_RENDER_ADAPTER_H

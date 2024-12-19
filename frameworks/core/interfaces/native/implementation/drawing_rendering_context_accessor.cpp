/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"

#include "drawing_rendering_context_peer_impl.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::Converter {
template<>
void AssignCast(std::optional<CanvasUnit>& dst, const Ark_LengthMetricsUnit& src)
{
    LOGW("AssignCast for Ark_LengthMetricsUnit is not implemented yet");
    dst = CanvasUnit::DEFAULT;
}
}

namespace OHOS::Ace::NG::GeneratedModifier {
namespace DrawingRenderingContextAccessor {
void DestroyPeerImpl(DrawingRenderingContextPeer* peer)
{
    auto peerImpl = reinterpret_cast<DrawingRenderingContextPeerImpl*>(peer);
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_NativePointer CtorImpl(const Opt_LengthMetricsUnit* unit)
{
    auto peerImpl = Referenced::MakeRefPtr<DrawingRenderingContextPeerImpl>();
    peerImpl->IncRefCount();

    if (unit) {
        auto units = Converter::OptConvert<CanvasUnit>(*unit);
        if (units) {
            peerImpl->SetUnit(*units);
        }
    }
    return reinterpret_cast<DrawingRenderingContextPeer *>(Referenced::RawPtr(peerImpl));
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void InvalidateImpl(DrawingRenderingContextPeer* peer)
{
    auto peerImpl = reinterpret_cast<DrawingRenderingContextPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerInvalidate();
}
} // DrawingRenderingContextAccessor
const GENERATED_ArkUIDrawingRenderingContextAccessor* GetDrawingRenderingContextAccessor()
{
    static const GENERATED_ArkUIDrawingRenderingContextAccessor DrawingRenderingContextAccessorImpl {
        DrawingRenderingContextAccessor::DestroyPeerImpl,
        DrawingRenderingContextAccessor::CtorImpl,
        DrawingRenderingContextAccessor::GetFinalizerImpl,
        DrawingRenderingContextAccessor::InvalidateImpl,
    };
    return &DrawingRenderingContextAccessorImpl;
}

struct DrawingRenderingContextPeer {
    virtual ~DrawingRenderingContextPeer() = default;
};
}

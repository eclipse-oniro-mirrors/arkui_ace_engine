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
#include <utility>

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"

#include "canvas_rendering_context2d_peer_impl.h"
#include "drawing_rendering_context_peer_impl.h"
#include "core/interfaces/native/utility/callback_helper.h"

#include "arkoala_api_generated.h"

namespace {
constexpr auto ERROR_VALUE = -1;
} // namespace

namespace OHOS::Ace::NG::Converter {
template<>
inline void AssignCast(std::optional<bool>& dst, const Ark_RenderingContextSettings& src)
{
    dst = Converter::OptConvert<bool>(src.antialias);
}
inline void AssignCast(std::optional<bool>& dst, const Ark_RenderingContextSettings& src)
{
    dst = Converter::OptConvert<bool>(src.antialias);
}
template<>
inline void AssignCast(std::optional<ImageAnalyzerType>& dst, const Ark_ImageAnalyzerType& src)
{
    switch (src) {
        case ARK_IMAGE_ANALYZER_TYPE_TEXT: dst = ImageAnalyzerType::TEXT; break;
        case ARK_IMAGE_ANALYZER_TYPE_SUBJECT: dst = ImageAnalyzerType::SUBJECT; break;
        default: LOGE("Unexpected enum value in Ark_ImageAnalyzerType: %{public}d", src);
    }
}
template<>
std::vector<ImageAnalyzerType> Convert(const Array_ImageAnalyzerType& src)
{
    std::vector<ImageAnalyzerType> dst;
    auto length = Converter::Convert<int>(src.length);
    for (int i = 0; i < length; i++) {
        auto opt = Converter::OptConvert<ImageAnalyzerType>(*(src.array + i));
        if (opt) {
            dst.push_back(*opt);
        }
    }
    return dst;
}

} // namespace OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace CanvasRenderingContext2DAccessor {
void DestroyPeerImpl(CanvasRenderingContext2DPeer* peer)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl *>(peer);
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_NativePointer CtorImpl(const Opt_RenderingContextSettings* settings)
{
    CHECK_NULL_RETURN(settings, nullptr);

    auto peerImpl = Referenced::MakeRefPtr<CanvasRenderingContext2DPeerImpl>();
    peerImpl->IncRefCount();
    auto antialias = Converter::OptConvert<bool>(*settings);
    if (antialias) {
        peerImpl->SetAntiAlias(*antialias);
    }
    return reinterpret_cast<CanvasRenderingContext2DPeer*>(Referenced::RawPtr(peerImpl));
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void ToDataURLImpl(CanvasRenderingContext2DPeer* peer,
                   const Opt_String* type,
                   const Opt_CustomObject* quality)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(type);
    CHECK_NULL_VOID(quality);

    auto dataUrl = Converter::OptConvert<std::string>(*type);
    if (!dataUrl) {
        return;
    }
    LOGE("ARKOALA CanvasRenderingContext2DPeerImpl::TriggerToDataURL Opt_CustomObject not implemented.");
}
void StartImageAnalyzerImpl(CanvasRenderingContext2DPeer* peer,
                            const Ark_ImageAnalyzerConfig* config,
                            const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(config);

    auto vector = Converter::Convert<std::vector<ImageAnalyzerType>>(config->types);
    peerImpl->TriggerStartImageAnalyzer(vector);
}
void StopImageAnalyzerImpl(CanvasRenderingContext2DPeer* peer)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerStopImageAnalyzer();
}
void OnOnAttachImpl(CanvasRenderingContext2DPeer* peer,
                    const Callback_Void* callback)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(callback);
    auto arkCallback = CallbackHelper(*callback);
    peerImpl->On(std::move(arkCallback), CanvasCallbackType::ON_ATTACH);
}
void OffOnAttachImpl(CanvasRenderingContext2DPeer* peer,
                     const Opt_Callback_Void* callback)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    std::function<void()> cb = nullptr;
    auto optCallback = Converter::OptConvert<Callback_Void>(*callback);
    auto arkCallback = CallbackHelper(*optCallback);
    peerImpl->Off(std::move(arkCallback), CanvasCallbackType::ON_ATTACH);
}
void OnOnDetachImpl(CanvasRenderingContext2DPeer* peer,
                    const Callback_Void* callback)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(callback);
    auto arkCallback = CallbackHelper(*callback);
    peerImpl->On(std::move(arkCallback), CanvasCallbackType::ON_DETACH);
}
void OffOnDetachImpl(CanvasRenderingContext2DPeer* peer,
                     const Opt_Callback_Void* callback)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto optCallback = Converter::OptConvert<Callback_Void>(*callback);
    auto arkCallback = CallbackHelper(*optCallback);
    peerImpl->Off(std::move(arkCallback), CanvasCallbackType::ON_DETACH);
}
Ark_Int32 GetHeightImpl(CanvasRenderingContext2DPeer* peer)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, ::ERROR_VALUE);
    return peerImpl->TriggerGetHeight();
}
Ark_Int32 GetWidthImpl(CanvasRenderingContext2DPeer* peer)
{
    auto peerImpl = reinterpret_cast<CanvasRenderingContext2DPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, ::ERROR_VALUE);
    return peerImpl->TriggerGetWidth();
}
} // CanvasRenderingContext2DAccessor
const GENERATED_ArkUICanvasRenderingContext2DAccessor* GetCanvasRenderingContext2DAccessor()
{
    static const GENERATED_ArkUICanvasRenderingContext2DAccessor CanvasRenderingContext2DAccessorImpl {
        CanvasRenderingContext2DAccessor::DestroyPeerImpl,
        CanvasRenderingContext2DAccessor::CtorImpl,
        CanvasRenderingContext2DAccessor::GetFinalizerImpl,
        CanvasRenderingContext2DAccessor::ToDataURLImpl,
        CanvasRenderingContext2DAccessor::StartImageAnalyzerImpl,
        CanvasRenderingContext2DAccessor::StopImageAnalyzerImpl,
        CanvasRenderingContext2DAccessor::OnOnAttachImpl,
        CanvasRenderingContext2DAccessor::OffOnAttachImpl,
        CanvasRenderingContext2DAccessor::OnOnDetachImpl,
        CanvasRenderingContext2DAccessor::OffOnDetachImpl,
        CanvasRenderingContext2DAccessor::GetHeightImpl,
        CanvasRenderingContext2DAccessor::GetWidthImpl,
    };
    return &CanvasRenderingContext2DAccessorImpl;
}

struct CanvasRenderingContext2DPeer {
    virtual ~CanvasRenderingContext2DPeer() = default;
};
}

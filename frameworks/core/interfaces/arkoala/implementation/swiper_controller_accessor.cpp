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

#include "swiper_controller_modifier_peer_impl.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::Converter {
template<>
inline void AssignCast(std::optional<Ark_Function>& dst, const Ark_Function& src)
{
    dst = src;
}
} // namespace OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace SwiperControllerModifier {

static void DestroyPeer(SwiperControllerPeerImpl *peerImpl)
{
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}

Ark_NativePointer CtorImpl()
{
    auto peerImpl = Referenced::MakeRefPtr<SwiperControllerPeerImpl>();
    peerImpl->IncRefCount();
    return Referenced::RawPtr(peerImpl);
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeer);
}
void ShowNextImpl(SwiperControllerPeer* peer)
{
    auto peerImpl = reinterpret_cast<SwiperControllerPeerImpl *>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerShowNext();
}
void ShowPreviousImpl(SwiperControllerPeer* peer)
{
    auto peerImpl = reinterpret_cast<SwiperControllerPeerImpl *>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerShowPrevious();
}
void ChangeIndexImpl(SwiperControllerPeer* peer,
                     const Ark_Number* index,
                     const Opt_Boolean* useAnimation)
{
    auto peerImpl = reinterpret_cast<SwiperControllerPeerImpl *>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(index);
    auto aceIdx = Converter::Convert<Ark_Int32>(*index);
    auto aceUseAnim = useAnimation ? Converter::OptConvert<bool>(*useAnimation) : std::nullopt;
    peerImpl->TriggerChangeIndex(aceIdx, aceUseAnim);
}
void FinishAnimationImpl(SwiperControllerPeer* peer,
                         const Opt_Function* callback)
{
    auto peerImpl = reinterpret_cast<SwiperControllerPeerImpl *>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto aceCallbackOpt = callback ? Converter::OptConvert<Ark_Function>(*callback) : std::nullopt;
    if (aceCallbackOpt) {
        auto onFinish = []() -> void {
            LOGE("Invoking the Arkoala FinishAnimation callbacks not implemented");
        };
        peerImpl->SetFinishCallback(onFinish);
    }
    peerImpl->TriggerFinishAnimation();
}
} // SwiperControllerModifier
const GENERATED_ArkUISwiperControllerAccessor* GetSwiperControllerAccessor()
{
    static const GENERATED_ArkUISwiperControllerAccessor SwiperControllerAccessorImpl {
        SwiperControllerModifier::CtorImpl,
        SwiperControllerModifier::GetFinalizerImpl,
        SwiperControllerModifier::ShowNextImpl,
        SwiperControllerModifier::ShowPreviousImpl,
        SwiperControllerModifier::ChangeIndexImpl,
        SwiperControllerModifier::FinishAnimationImpl,
    };
    return &SwiperControllerAccessorImpl;
}
}
/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/implementation/click_event_peer.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace ClickEventAccessor {
void DestroyPeerImpl(ClickEventPeer* peer)
{
    delete peer;
}
Ark_NativePointer CtorImpl()
{
    return new ClickEventPeer();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_Int32 GetDisplayXImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetScreenLocation();
    const auto x = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
    LOGE("ARKOALA ClickEventAccessor::GetDisplayXImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(x));
}
void SetDisplayXImpl(ClickEventPeer* peer,
                     const Ark_Number* displayX)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(displayX);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetScreenLocation();
    const auto animation = offset.GetXAnimationOption();
    const auto convWindowsX = Converter::Convert<float>(*displayX);
    const auto x = PipelineBase::Vp2PxWithCurrentDensity(convWindowsX);
    offset.SetX(x, animation);
    info->SetScreenLocation(offset);
}
Ark_Int32 GetDisplayYImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetScreenLocation();
    const auto y = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
    LOGE("ARKOALA ClickEventAccessor::GetDisplayYImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(y));
}
void SetDisplayYImpl(ClickEventPeer* peer,
                     const Ark_Number* displayY)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(displayY);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetScreenLocation();
    const auto animation = offset.GetYAnimationOption();
    const auto convWindowsY = Converter::Convert<float>(*displayY);
    const auto y = PipelineBase::Vp2PxWithCurrentDensity(convWindowsY);
    offset.SetY(y, animation);
    info->SetScreenLocation(offset);
}
Ark_Int32 GetWindowXImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetGlobalLocation();
    const auto x = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
    LOGE("ARKOALA ClickEventAccessor::GetWindowXImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(x));
}
void SetWindowXImpl(ClickEventPeer* peer,
                    const Ark_Number* windowX)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(windowX);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetGlobalLocation();
    const auto animation = offset.GetXAnimationOption();
    const auto convWindowsX = Converter::Convert<float>(*windowX);
    const auto x = PipelineBase::Vp2PxWithCurrentDensity(convWindowsX);
    offset.SetX(x, animation);
    info->SetGlobalLocation(offset);
}
Ark_Int32 GetWindowYImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetGlobalLocation();
    const auto y = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
    LOGE("ARKOALA ClickEventAccessor::GetWindowYImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(y));
}
void SetWindowYImpl(ClickEventPeer* peer,
                    const Ark_Number* windowY)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(windowY);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetGlobalLocation();
    const auto animation = offset.GetYAnimationOption();
    const auto convWindowsY = Converter::Convert<float>(*windowY);
    const auto y = PipelineBase::Vp2PxWithCurrentDensity(convWindowsY);
    offset.SetY(y, animation);
    info->SetGlobalLocation(offset);
}
Ark_Int32 GetScreenXImpl(ClickEventPeer* peer)
{
    return GetWindowXImpl(peer);
}
void SetScreenXImpl(ClickEventPeer* peer,
                    const Ark_Number* screenX)
{
    SetWindowXImpl(peer, screenX);
}
Ark_Int32 GetScreenYImpl(ClickEventPeer* peer)
{
    return GetWindowYImpl(peer);
}
void SetScreenYImpl(ClickEventPeer* peer,
                    const Ark_Number* screenY)
{
    SetWindowYImpl(peer, screenY);
}
Ark_Int32 GetXImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetLocalLocation();
    const auto x = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
    LOGE("ARKOALA ClickEventAccessor::GetXImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(x));
}
void SetXImpl(ClickEventPeer* peer,
              const Ark_Number* x)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetLocalLocation();
    const auto animation = offset.GetXAnimationOption();
    const auto convWindowsX = Converter::Convert<float>(*x);
    const auto newX = PipelineBase::Vp2PxWithCurrentDensity(convWindowsX);
    offset.SetX(newX, animation);
    info->SetLocalLocation(offset);
}
Ark_Int32 GetYImpl(ClickEventPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    const auto* info = peer->GetEventInfo();
    CHECK_NULL_RETURN(info, 0);
    const auto& offset = info->GetLocalLocation();
    const auto y = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
    LOGI("ARKOALA ClickEventAccessor::GetYImpl return type Ark_Int32 "
        "should be replaced with a Ark_Number type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(y));
}
void SetYImpl(ClickEventPeer* peer,
              const Ark_Number* y)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(y);
    const auto info = peer->GetEventInfo();
    CHECK_NULL_VOID(info);
    Offset offset = info->GetLocalLocation();
    const auto animation = offset.GetYAnimationOption();
    const auto convWindowsY = Converter::Convert<float>(*y);
    const auto newY = PipelineBase::Vp2PxWithCurrentDensity(convWindowsY);
    offset.SetY(newY, animation);
    info->SetLocalLocation(offset);
}
Callback_Void GetPreventDefaultImpl(ClickEventPeer* peer)
{
    return {};
}
void SetPreventDefaultImpl(ClickEventPeer* peer,
                           const Callback_Void* preventDefault)
{
}
} // ClickEventAccessor
const GENERATED_ArkUIClickEventAccessor* GetClickEventAccessor()
{
    static const GENERATED_ArkUIClickEventAccessor ClickEventAccessorImpl {
        ClickEventAccessor::DestroyPeerImpl,
        ClickEventAccessor::CtorImpl,
        ClickEventAccessor::GetFinalizerImpl,
        ClickEventAccessor::GetDisplayXImpl,
        ClickEventAccessor::SetDisplayXImpl,
        ClickEventAccessor::GetDisplayYImpl,
        ClickEventAccessor::SetDisplayYImpl,
        ClickEventAccessor::GetWindowXImpl,
        ClickEventAccessor::SetWindowXImpl,
        ClickEventAccessor::GetWindowYImpl,
        ClickEventAccessor::SetWindowYImpl,
        ClickEventAccessor::GetScreenXImpl,
        ClickEventAccessor::SetScreenXImpl,
        ClickEventAccessor::GetScreenYImpl,
        ClickEventAccessor::SetScreenYImpl,
        ClickEventAccessor::GetXImpl,
        ClickEventAccessor::SetXImpl,
        ClickEventAccessor::GetYImpl,
        ClickEventAccessor::SetYImpl,
        ClickEventAccessor::GetPreventDefaultImpl,
        ClickEventAccessor::SetPreventDefaultImpl,
    };
    return &ClickEventAccessorImpl;
}
}

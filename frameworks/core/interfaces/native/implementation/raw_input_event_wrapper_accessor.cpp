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

#include "arkoala_api_generated.h"
#include "core/components_ng/base/node_sensitive_input.h"
#include "core/interfaces/native/implementation/raw_input_event_wrapper_peer.h"
#include "core/interfaces/native/implementation/key_event_peer.h"
#include "core/interfaces/native/implementation/mouse_event_peer.h"
#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG::GeneratedModifier {

namespace RawInputEventWrapperAccessor {
namespace {
constexpr uint32_t MOUSE_EVENT_SELECTOR = 0;
constexpr uint32_t TOUCH_EVENT_SELECTOR = 1;
constexpr uint32_t KEY_EVENT_SELECTOR = 2;
} // namespace

void DestroyPeerImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    if (peerImpl) {
        delete peerImpl;
    }
}

Ark_RawInputEventWrapper ConstructImpl()
{
    return nullptr;
}

Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void*>(&DestroyPeerImpl);
}

Ark_Boolean IsMouseEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    return peerImpl != nullptr && peerImpl->event.selector == MOUSE_EVENT_SELECTOR;
}

Ark_Boolean IsTouchEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    return peerImpl != nullptr && peerImpl->event.selector == TOUCH_EVENT_SELECTOR;
}

Ark_Boolean IsKeyEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    return peerImpl != nullptr && peerImpl->event.selector == KEY_EVENT_SELECTOR;
}

Opt_MouseEvent AsMouseEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    if (peerImpl == nullptr || peerImpl->event.selector != MOUSE_EVENT_SELECTOR) {
        return Converter::ArkValue<Opt_MouseEvent>();
    }
    if (peerImpl->mouseInfo) {
        auto peer = PeerUtils::CreatePeer<MouseEventPeer>();
        peer->eventInfo->object = *peerImpl->mouseInfo;
        peer->SetEventInfo(&peer->eventInfo->object.value());
        return Converter::ArkValue<Opt_MouseEvent>(peer);
    }
    return Converter::ArkValue<Opt_MouseEvent>(peerImpl->event.value0);
}

Opt_TouchEventProxy AsTouchEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    if (peerImpl == nullptr || peerImpl->event.selector != TOUCH_EVENT_SELECTOR) {
        return Converter::ArkValue<Opt_TouchEventProxy>();
    }
    if (peerImpl->touchInfo) {
        const auto& info = *peerImpl->touchInfo;
        auto ctx = peerImpl->context.get();
        Ark_TouchEventProxy proxy {
            .target = Converter::ArkValue<Ark_EventTarget>(info.GetTarget(), ctx),
            .timeStamp = Converter::ArkValue<Ark_Int64>(
                static_cast<int64_t>(info.GetTimeStamp().time_since_epoch().count())),
            .source = Converter::ArkValue<Ark_SourceType>(info.GetSourceDevice()),
            .pressure = Converter::ArkValue<Ark_Float64>(info.GetForce()),
            .tiltX = Converter::ArkValue<Ark_Float64>(info.GetTiltX().value_or(0)),
            .tiltY = Converter::ArkValue<Ark_Float64>(info.GetTiltY().value_or(0)),
            .rollAngle = Converter::ArkValue<Opt_Float64>(info.GetRollAngle()),
            .sourceTool = Converter::ArkValue<Ark_SourceTool>(info.GetSourceTool()),
            .deviceId = Converter::ArkValue<Opt_Int32>(info.GetDeviceId()),
            .targetDisplayId = Converter::ArkValue<Opt_Int32>(info.GetTargetDisplayId()),
            .type = Converter::ArkValue<Ark_TouchType>(info.GetChangedTouches().empty() ?
                TouchType::UNKNOWN : info.GetChangedTouches().front().GetTouchType()),
            .touches = Converter::ArkValue<Array_TouchObject>(info.GetTouches(), ctx),
            .changedTouches = Converter::ArkValue<Array_TouchObject>(info.GetChangedTouches(), ctx),
            .ptr = peerImpl->touchInfo.get(),
        };
        return Converter::ArkValue<Opt_TouchEventProxy>(proxy);
    }
    return Converter::ArkValue<Opt_TouchEventProxy>(peerImpl->event.value1);
}

Opt_KeyEvent AsKeyEventImpl(Ark_RawInputEventWrapper peer)
{
    auto peerImpl = reinterpret_cast<RawInputEventWrapperPeer*>(peer);
    if (peerImpl == nullptr || peerImpl->event.selector != KEY_EVENT_SELECTOR) {
        return Converter::ArkValue<Opt_KeyEvent>();
    }
    if (peerImpl->keyInfo) {
        auto peer = PeerUtils::CreatePeer<KeyEventPeer>();
        peer->eventInfo->object = *peerImpl->keyInfo;
        peer->SetEventInfo(&peer->eventInfo->object.value());
        return Converter::ArkValue<Opt_KeyEvent>(peer);
    }
    return Converter::ArkValue<Opt_KeyEvent>(peerImpl->event.value2);
}
} // namespace RawInputEventWrapperAccessor

Ark_RawInputEventWrapper CreateRawInputEventWrapperPeer(const RawInputEventUnion* event)
{
    CHECK_NULL_RETURN(event, nullptr);
    auto* peer = new RawInputEventWrapperPeer();
    peer->event = *event;
    return reinterpret_cast<Ark_RawInputEventWrapper>(peer);
}

const GENERATED_ArkUIRawInputEventWrapperAccessor* GetRawInputEventWrapperAccessor()
{
    static const GENERATED_ArkUIRawInputEventWrapperAccessor RawInputEventWrapperAccessorImpl {
        RawInputEventWrapperAccessor::DestroyPeerImpl,
        RawInputEventWrapperAccessor::ConstructImpl,
        RawInputEventWrapperAccessor::GetFinalizerImpl,
        RawInputEventWrapperAccessor::IsMouseEventImpl,
        RawInputEventWrapperAccessor::IsTouchEventImpl,
        RawInputEventWrapperAccessor::IsKeyEventImpl,
        RawInputEventWrapperAccessor::AsMouseEventImpl,
        RawInputEventWrapperAccessor::AsTouchEventImpl,
        RawInputEventWrapperAccessor::AsKeyEventImpl,
    };
    return &RawInputEventWrapperAccessorImpl;
}
} // namespace OHOS::Ace::NG::GeneratedModifier

namespace OHOS::Ace::NG {
OwnedNodeSensitiveInputPeer CreateNodeSensitiveInputPeer(const NodeSensitiveInputInfo& info, FrameNode* node)
{
    using namespace GeneratedModifier;
    auto peer = std::make_unique<RawInputEventWrapperPeer>();
    if (info.type == NodeSensitiveInputEventType::TOUCH && info.touchEvent) {
        peer->event.selector = RawInputEventWrapperAccessor::TOUCH_EVENT_SELECTOR;
        peer->touchInfo = std::make_shared<TouchEventInfo>(CreateNodeSensitiveTouchInfo(*info.touchEvent, node));
        peer->context = std::make_shared<Converter::ConvContext>();
    } else if (info.type == NodeSensitiveInputEventType::MOUSE && info.mouseEvent) {
        peer->event.selector = RawInputEventWrapperAccessor::MOUSE_EVENT_SELECTOR;
        peer->mouseInfo = std::make_shared<MouseInfo>(CreateNodeSensitiveMouseInfo(*info.mouseEvent, node));
    } else if (info.type == NodeSensitiveInputEventType::KEY && info.keyEvent) {
        peer->event.selector = RawInputEventWrapperAccessor::KEY_EVENT_SELECTOR;
        peer->keyInfo = std::make_shared<KeyEventInfo>(CreateNodeSensitiveKeyInfo(*info.keyEvent, node));
    } else {
        peer.reset();
    }
    return { peer.release(), [](void* value) { delete static_cast<RawInputEventWrapperPeer*>(value); } };
}
} // namespace OHOS::Ace::NG

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

#include "mock_swiper_dynamic_module.h"

#include "arkoala_api_generated.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"
#include "core/components_ng/pattern/swiper/bridge/swiper_content_transition_proxy_peer.h"

namespace OHOS::Ace {

namespace {
void MockDestroyPeerImpl(Ark_SwiperContentTransitionProxy peer)
{
}

Ark_SwiperContentTransitionProxy MockConstructImpl()
{
    return nullptr;
}

Ark_NativePointer MockGetFinalizerImpl()
{
    return reinterpret_cast<void*>(&MockDestroyPeerImpl);
}

void MockFinishTransitionImpl(Ark_SwiperContentTransitionProxy peer)
{
}

Ark_Int32 MockGetSelectedIndexImpl(Ark_SwiperContentTransitionProxy peer)
{
    return 0;
}

void MockSetSelectedIndexImpl(Ark_SwiperContentTransitionProxy peer, Ark_Int32 selectedIndex)
{
}

Ark_Int32 MockGetIndexImpl(Ark_SwiperContentTransitionProxy peer)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return 0;
    }
    return static_cast<Ark_Int32>(peer->handler->GetIndex());
}

void MockSetIndexImpl(Ark_SwiperContentTransitionProxy peer, Ark_Int32 index)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return;
    }
    peer->handler->SetIndex(static_cast<int32_t>(index));
}

Ark_Float64 MockGetPositionImpl(Ark_SwiperContentTransitionProxy peer)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return 0.0;
    }
    return static_cast<Ark_Float64>(peer->handler->GetPosition());
}

void MockSetPositionImpl(Ark_SwiperContentTransitionProxy peer, Ark_Float64 position)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return;
    }
    peer->handler->SetPosition(static_cast<float>(position));
}

Ark_Float64 MockGetMainAxisLengthImpl(Ark_SwiperContentTransitionProxy peer)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return 0.0;
    }
    return static_cast<Ark_Float64>(peer->handler->GetMainAxisLength());
}

void MockSetMainAxisLengthImpl(Ark_SwiperContentTransitionProxy peer, Ark_Float64 mainAxisLength)
{
    if (peer == nullptr || peer->handler == nullptr) {
        return;
    }
    peer->handler->SetMainAxisLength(static_cast<float>(mainAxisLength));
}
const GENERATED_ArkUISwiperContentTransitionProxyAccessor g_swiperContentTransitionProxyAccessor = {
    .destroyPeer = MockDestroyPeerImpl,
    .construct = MockConstructImpl,
    .getFinalizer = MockGetFinalizerImpl,
    .finishTransition = MockFinishTransitionImpl,
    .getSelectedIndex = MockGetSelectedIndexImpl,
    .setSelectedIndex = MockSetSelectedIndexImpl,
    .getIndex = MockGetIndexImpl,
    .setIndex = MockSetIndexImpl,
    .getPosition = MockGetPositionImpl,
    .setPosition = MockSetPositionImpl,
    .getMainAxisLength = MockGetMainAxisLengthImpl,
    .setMainAxisLength = MockSetMainAxisLengthImpl,
};
} // namespace

void MockSwiperDynamicModule::RegisterAttributes(panda::Local<panda::ObjectRef> object, panda::ecmascript::EcmaVM* vm)
{
}

const void* MockSwiperDynamicModule::GetCustomModifier(const std::string& name)
{
    if (name == "swiperContentTransitionProxyAccessor") {
        return &g_swiperContentTransitionProxyAccessor;
    }
    return nullptr;
}

} // namespace OHOS::Ace

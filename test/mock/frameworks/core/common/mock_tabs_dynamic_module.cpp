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

#include "mock_tabs_dynamic_module.h"

#include "arkoala_api_generated.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"

namespace OHOS::Ace {

namespace {
void MockDestroyPeerImpl(Ark_TabContentTransitionProxy peer)
{
}

Ark_TabContentTransitionProxy MockConstructImpl()
{
    return nullptr;
}

Ark_NativePointer MockGetFinalizerImpl()
{
    return reinterpret_cast<void*>(&MockDestroyPeerImpl);
}

void MockFinishTransitionImpl(Ark_TabContentTransitionProxy peer)
{
}

Ark_Int32 MockGetFromImpl(Ark_TabContentTransitionProxy peer)
{
    return 0;
}

void MockSetFromImpl(Ark_TabContentTransitionProxy peer, Ark_Int32 from)
{
}

Ark_Int32 MockGetToImpl(Ark_TabContentTransitionProxy peer)
{
    return 0;
}

void MockSetToImpl(Ark_TabContentTransitionProxy peer, Ark_Int32 to)
{
}
const GENERATED_ArkUITabContentTransitionProxyAccessor g_tabContentTransitionProxyAccessor = {
    .destroyPeer = MockDestroyPeerImpl,
    .construct = MockConstructImpl,
    .getFinalizer = MockGetFinalizerImpl,
    .finishTransition = MockFinishTransitionImpl,
    .getFrom = MockGetFromImpl,
    .setFrom = MockSetFromImpl,
    .getTo = MockGetToImpl,
    .setTo = MockSetToImpl,
};
} // namespace

void MockTabsDynamicModule::RegisterAttributes(panda::Local<panda::ObjectRef> object, panda::ecmascript::EcmaVM* vm)
{
}

const void* MockTabsDynamicModule::GetCustomModifier(const std::string& name)
{
    if (name == "tabContentTransitionProxyAccessor") {
        return &g_tabContentTransitionProxyAccessor;
    }
    return nullptr;
}

} // namespace OHOS::Ace

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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace KeyEventAccessor {
void DestroyPeerImpl(Ark_KeyEvent peer)
{
}
Ark_KeyEvent ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_KeyType GetTypeImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetTypeImpl(Ark_KeyEvent peer,
                 Ark_KeyType type)
{
}
Ark_Number GetKeyCodeImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetKeyCodeImpl(Ark_KeyEvent peer,
                    const Ark_Number* keyCode)
{
}
Ark_String GetKeyTextImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetKeyTextImpl(Ark_KeyEvent peer,
                    const Ark_String* keyText)
{
}
Ark_KeySource GetKeySourceImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetKeySourceImpl(Ark_KeyEvent peer,
                      Ark_KeySource keySource)
{
}
Ark_Number GetDeviceIdImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetDeviceIdImpl(Ark_KeyEvent peer,
                     const Ark_Number* deviceId)
{
}
Ark_Number GetMetaKeyImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetMetaKeyImpl(Ark_KeyEvent peer,
                    const Ark_Number* metaKey)
{
}
Ark_Number GetTimestampImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetTimestampImpl(Ark_KeyEvent peer,
                      const Ark_Number* timestamp)
{
}
Callback_Void GetStopPropagationImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetStopPropagationImpl(Ark_KeyEvent peer,
                            const Callback_Void* stopPropagation)
{
}
Ark_IntentionCode GetIntentionCodeImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetIntentionCodeImpl(Ark_KeyEvent peer,
                          Ark_IntentionCode intentionCode)
{
}
Opt_ModifierKeyStateGetter GetGetModifierKeyStateImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetGetModifierKeyStateImpl(Ark_KeyEvent peer,
                                const Opt_ModifierKeyStateGetter* getModifierKeyState)
{
}
Opt_Number GetUnicodeImpl(Ark_KeyEvent peer)
{
    return {};
}
void SetUnicodeImpl(Ark_KeyEvent peer,
                    const Opt_Number* unicode)
{
}
} // KeyEventAccessor
const GENERATED_ArkUIKeyEventAccessor* GetKeyEventAccessor()
{
    static const GENERATED_ArkUIKeyEventAccessor KeyEventAccessorImpl {
        KeyEventAccessor::DestroyPeerImpl,
        KeyEventAccessor::ConstructImpl,
        KeyEventAccessor::GetFinalizerImpl,
        KeyEventAccessor::GetTypeImpl,
        KeyEventAccessor::SetTypeImpl,
        KeyEventAccessor::GetKeyCodeImpl,
        KeyEventAccessor::SetKeyCodeImpl,
        KeyEventAccessor::GetKeyTextImpl,
        KeyEventAccessor::SetKeyTextImpl,
        KeyEventAccessor::GetKeySourceImpl,
        KeyEventAccessor::SetKeySourceImpl,
        KeyEventAccessor::GetDeviceIdImpl,
        KeyEventAccessor::SetDeviceIdImpl,
        KeyEventAccessor::GetMetaKeyImpl,
        KeyEventAccessor::SetMetaKeyImpl,
        KeyEventAccessor::GetTimestampImpl,
        KeyEventAccessor::SetTimestampImpl,
        KeyEventAccessor::GetStopPropagationImpl,
        KeyEventAccessor::SetStopPropagationImpl,
        KeyEventAccessor::GetIntentionCodeImpl,
        KeyEventAccessor::SetIntentionCodeImpl,
        KeyEventAccessor::GetGetModifierKeyStateImpl,
        KeyEventAccessor::SetGetModifierKeyStateImpl,
        KeyEventAccessor::GetUnicodeImpl,
        KeyEventAccessor::SetUnicodeImpl,
    };
    return &KeyEventAccessorImpl;
}

}

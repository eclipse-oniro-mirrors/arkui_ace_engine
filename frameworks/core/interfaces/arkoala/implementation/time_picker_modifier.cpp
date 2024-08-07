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

#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier
{
    namespace TimePickerInterfaceModifier {
        void _setTimePickerOptionsImpl(Ark_NativePointer node, const Opt_TimePickerOptions* options)
        {
        }
    } // TimePickerInterfaceModifier
    namespace TimePickerAttributeModifier {
        void UseMilitaryTimeImpl(Ark_NativePointer node, Ark_Boolean value)
        {
        }
        void LoopImpl(Ark_NativePointer node, Ark_Boolean value)
        {
        }
        void DisappearTextStyleImpl(Ark_NativePointer node, const Ark_PickerTextStyle* value)
        {
        }
        void TextStyleImpl(Ark_NativePointer node, const Ark_PickerTextStyle* value)
        {
        }
        void SelectedTextStyleImpl(Ark_NativePointer node, const Ark_PickerTextStyle* value)
        {
        }
        void DateTimeOptionsImpl(Ark_NativePointer node, const Ark_CustomObject* value)
        {
        }
        void OnChangeImpl(Ark_NativePointer node, Ark_Function callback)
        {
        }
        void EnableHapticFeedbackImpl(Ark_NativePointer node, Ark_Boolean enable)
        {
        }
    } // TimePickerAttributeModifier
    const GENERATED_ArkUITimePickerModifier* GetTimePickerModifier()
    {
        static const GENERATED_ArkUITimePickerModifier ArkUITimePickerModifierImpl {
            TimePickerInterfaceModifier::_setTimePickerOptionsImpl,
            TimePickerAttributeModifier::UseMilitaryTimeImpl,
            TimePickerAttributeModifier::LoopImpl,
            TimePickerAttributeModifier::DisappearTextStyleImpl,
            TimePickerAttributeModifier::TextStyleImpl,
            TimePickerAttributeModifier::SelectedTextStyleImpl,
            TimePickerAttributeModifier::DateTimeOptionsImpl,
            TimePickerAttributeModifier::OnChangeImpl,
            TimePickerAttributeModifier::EnableHapticFeedbackImpl,
        };
        return &ArkUITimePickerModifierImpl;
    }

}

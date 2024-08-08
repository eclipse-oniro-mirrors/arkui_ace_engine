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

namespace OHOS::Ace::NG::GeneratedModifier {
    namespace SideBarContainerInterfaceModifier {
        void _setSideBarContainerOptionsImpl(Ark_NativePointer node, const Opt_SideBarContainerType* type)
        {
        }
    } // SideBarContainerInterfaceModifier
    namespace SideBarContainerAttributeModifier {
        void ShowSideBarImpl(Ark_NativePointer node, Ark_Boolean value)
        {
        }
        void ControlButtonImpl(Ark_NativePointer node, const Ark_ButtonStyle* value)
        {
        }
        void ShowControlButtonImpl(Ark_NativePointer node, Ark_Boolean value)
        {
        }
        void OnChangeImpl(Ark_NativePointer node, Ark_Function callback)
        {
        }
        void SideBarWidth_numberImpl(Ark_NativePointer node, const Ark_Number* value)
        {
        }
        void MinSideBarWidth_numberImpl(Ark_NativePointer node, const Ark_Number* value)
        {
        }
        void MaxSideBarWidth_numberImpl(Ark_NativePointer node, const Ark_Number* value)
        {
        }
        void SideBarWidth_LengthImpl(Ark_NativePointer node, const Ark_Length* value)
        {
        }
        void MinSideBarWidth_LengthImpl(Ark_NativePointer node, const Ark_Length* value)
        {
        }
        void MaxSideBarWidth_LengthImpl(Ark_NativePointer node, const Ark_Length* value)
        {
        }
        void AutoHideImpl(Ark_NativePointer node, Ark_Boolean value)
        {
        }
        void SideBarPositionImpl(Ark_NativePointer node, Ark_Int32 value)
        {
        }
        void DividerImpl(Ark_NativePointer node, const Type_SideBarContainerAttribute_divider_Arg0* value)
        {
        }
        void MinContentWidthImpl(Ark_NativePointer node, const Ark_Length* value)
        {
        }
    } // SideBarContainerAttributeModifier
    const GENERATED_ArkUISideBarContainerModifier* GetSideBarContainerModifier()
    {
        static const GENERATED_ArkUISideBarContainerModifier ArkUISideBarContainerModifierImpl {
            SideBarContainerInterfaceModifier::_setSideBarContainerOptionsImpl,
            SideBarContainerAttributeModifier::ShowSideBarImpl,
            SideBarContainerAttributeModifier::ControlButtonImpl,
            SideBarContainerAttributeModifier::ShowControlButtonImpl,
            SideBarContainerAttributeModifier::OnChangeImpl,
            SideBarContainerAttributeModifier::SideBarWidth_numberImpl,
            SideBarContainerAttributeModifier::MinSideBarWidth_numberImpl,
            SideBarContainerAttributeModifier::MaxSideBarWidth_numberImpl,
            SideBarContainerAttributeModifier::SideBarWidth_LengthImpl,
            SideBarContainerAttributeModifier::MinSideBarWidth_LengthImpl,
            SideBarContainerAttributeModifier::MaxSideBarWidth_LengthImpl,
            SideBarContainerAttributeModifier::AutoHideImpl,
            SideBarContainerAttributeModifier::SideBarPositionImpl,
            SideBarContainerAttributeModifier::DividerImpl,
            SideBarContainerAttributeModifier::MinContentWidthImpl,
        };
        return &ArkUISideBarContainerModifierImpl;
    }

}

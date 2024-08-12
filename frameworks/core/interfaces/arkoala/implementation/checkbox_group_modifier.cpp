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
namespace CheckboxGroupInterfaceModifier {
void _setCheckboxGroupOptionsImpl(Ark_NativePointer node,
                                  const Opt_CheckboxGroupOptions* options)
{
}
} // CheckboxGroupInterfaceModifier
namespace CheckboxGroupAttributeModifier {
void SelectAllImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
}
void SelectedColorImpl(Ark_NativePointer node,
                       const ResourceColor* value)
{
}
void UnselectedColorImpl(Ark_NativePointer node,
                         const ResourceColor* value)
{
}
void MarkImpl(Ark_NativePointer node,
              const Ark_MarkStyle* value)
{
}
void OnChangeImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
}
void CheckboxShapeImpl(Ark_NativePointer node,
                       Ark_Int32 value)
{
}
} // CheckboxGroupAttributeModifier
const GENERATED_ArkUICheckboxGroupModifier* GetCheckboxGroupModifier()
{
    static const GENERATED_ArkUICheckboxGroupModifier ArkUICheckboxGroupModifierImpl {
        CheckboxGroupInterfaceModifier::_setCheckboxGroupOptionsImpl,
        CheckboxGroupAttributeModifier::SelectAllImpl,
        CheckboxGroupAttributeModifier::SelectedColorImpl,
        CheckboxGroupAttributeModifier::UnselectedColorImpl,
        CheckboxGroupAttributeModifier::MarkImpl,
        CheckboxGroupAttributeModifier::OnChangeImpl,
        CheckboxGroupAttributeModifier::CheckboxShapeImpl,
    };
    return &ArkUICheckboxGroupModifierImpl;
}

}

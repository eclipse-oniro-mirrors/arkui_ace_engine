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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace ToggleInterfaceModifier {
void SetToggleOptionsImpl(Ark_NativePointer node,
                          const Ark_ToggleOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    //auto convValue = Converter::OptConvert<type_name>(*options);
    //ToggleModelNG::SetSetToggleOptions(frameNode, convValue);
}
} // ToggleInterfaceModifier
namespace ToggleAttributeModifier {
void OnChangeImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //ToggleModelNG::SetOnChange(frameNode, convValue);
}
void ContentModifierImpl(Ark_NativePointer node,
                         const Ark_CustomObject* modifier)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(modifier);
    //auto convValue = Converter::OptConvert<type_name>(*modifier);
    //ToggleModelNG::SetContentModifier(frameNode, convValue);
}
void SelectedColorImpl(Ark_NativePointer node,
                       const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //ToggleModelNG::SetSelectedColor(frameNode, convValue);
}
void SwitchPointColorImpl(Ark_NativePointer node,
                          const Ark_ResourceColor* color)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(color);
    //auto convValue = Converter::OptConvert<type_name>(*color);
    //ToggleModelNG::SetSwitchPointColor(frameNode, convValue);
}
void SwitchStyleImpl(Ark_NativePointer node,
                     const Ark_SwitchStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //ToggleModelNG::SetSwitchStyle(frameNode, convValue);
}
} // ToggleAttributeModifier
const GENERATED_ArkUIToggleModifier* GetToggleModifier()
{
    static const GENERATED_ArkUIToggleModifier ArkUIToggleModifierImpl {
        ToggleInterfaceModifier::SetToggleOptionsImpl,
        ToggleAttributeModifier::OnChangeImpl,
        ToggleAttributeModifier::ContentModifierImpl,
        ToggleAttributeModifier::SelectedColorImpl,
        ToggleAttributeModifier::SwitchPointColorImpl,
        ToggleAttributeModifier::SwitchStyleImpl,
    };
    return &ArkUIToggleModifierImpl;
}

}

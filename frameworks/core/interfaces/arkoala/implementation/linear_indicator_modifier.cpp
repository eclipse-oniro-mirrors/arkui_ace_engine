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
namespace LinearIndicatorInterfaceModifier {
void SetLinearIndicatorOptionsImpl(Ark_NativePointer node,
                                   const Opt_Number* count,
                                   const Opt_LinearIndicatorController* controller)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(count);
    //auto convValue = Converter::OptConvert<type>(count); // for enums
    //LinearIndicatorModelNG::SetSetLinearIndicatorOptions(frameNode, convValue);
}
} // LinearIndicatorInterfaceModifier
namespace LinearIndicatorAttributeModifier {
void IndicatorStyleImpl(Ark_NativePointer node,
                        const Ark_Union_LinearIndicatorStyle_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //LinearIndicatorModelNG::SetIndicatorStyle(frameNode, convValue);
}
void IndicatorLoopImpl(Ark_NativePointer node,
                       const Ark_Union_Boolean_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //LinearIndicatorModelNG::SetIndicatorLoop(frameNode, convValue);
}
void OnChangeImpl(Ark_NativePointer node,
                  const Ark_Union_OnLinearIndicatorChangeCallback_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //LinearIndicatorModelNG::SetOnChange(frameNode, convValue);
}
} // LinearIndicatorAttributeModifier
const GENERATED_ArkUILinearIndicatorModifier* GetLinearIndicatorModifier()
{
    static const GENERATED_ArkUILinearIndicatorModifier ArkUILinearIndicatorModifierImpl {
        LinearIndicatorInterfaceModifier::SetLinearIndicatorOptionsImpl,
        LinearIndicatorAttributeModifier::IndicatorStyleImpl,
        LinearIndicatorAttributeModifier::IndicatorLoopImpl,
        LinearIndicatorAttributeModifier::OnChangeImpl,
    };
    return &ArkUILinearIndicatorModifierImpl;
}

}
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
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace StepperModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return nullptr;
}
} // StepperModifier
namespace StepperInterfaceModifier {
void SetStepperOptionsImpl(Ark_NativePointer node,
                           const Opt_Literal_Number_index* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //StepperModelNG::SetSetStepperOptions(frameNode, convValue);
}
} // StepperInterfaceModifier
namespace StepperAttributeModifier {
void OnFinishImpl(Ark_NativePointer node,
                  const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //StepperModelNG::SetOnFinish(frameNode, convValue);
}
void OnSkipImpl(Ark_NativePointer node,
                const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //StepperModelNG::SetOnSkip(frameNode, convValue);
}
void OnChangeImpl(Ark_NativePointer node,
                  const Callback_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //StepperModelNG::SetOnChange(frameNode, convValue);
}
void OnNextImpl(Ark_NativePointer node,
                const Callback_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //StepperModelNG::SetOnNext(frameNode, convValue);
}
void OnPreviousImpl(Ark_NativePointer node,
                    const Callback_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //StepperModelNG::SetOnPrevious(frameNode, convValue);
}
} // StepperAttributeModifier
const GENERATED_ArkUIStepperModifier* GetStepperModifier()
{
    static const GENERATED_ArkUIStepperModifier ArkUIStepperModifierImpl {
        StepperModifier::ConstructImpl,
        StepperInterfaceModifier::SetStepperOptionsImpl,
        StepperAttributeModifier::OnFinishImpl,
        StepperAttributeModifier::OnSkipImpl,
        StepperAttributeModifier::OnChangeImpl,
        StepperAttributeModifier::OnNextImpl,
        StepperAttributeModifier::OnPreviousImpl,
    };
    return &ArkUIStepperModifierImpl;
}

}

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
namespace CounterModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return nullptr;
}
} // CounterModifier
namespace CounterInterfaceModifier {
void SetCounterOptionsImpl(Ark_NativePointer node)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(undefined);
    //auto convValue = Converter::OptConvert<type>(undefined); // for enums
    //CounterModelNG::SetSetCounterOptions(frameNode, convValue);
}
} // CounterInterfaceModifier
namespace CounterAttributeModifier {
void OnIncImpl(Ark_NativePointer node,
               const VoidCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //CounterModelNG::SetOnInc(frameNode, convValue);
}
void OnDecImpl(Ark_NativePointer node,
               const VoidCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //CounterModelNG::SetOnDec(frameNode, convValue);
}
void EnableDecImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //CounterModelNG::SetEnableDec(frameNode, convValue);
}
void EnableIncImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //CounterModelNG::SetEnableInc(frameNode, convValue);
}
} // CounterAttributeModifier
const GENERATED_ArkUICounterModifier* GetCounterModifier()
{
    static const GENERATED_ArkUICounterModifier ArkUICounterModifierImpl {
        CounterModifier::ConstructImpl,
        CounterInterfaceModifier::SetCounterOptionsImpl,
        CounterAttributeModifier::OnIncImpl,
        CounterAttributeModifier::OnDecImpl,
        CounterAttributeModifier::EnableDecImpl,
        CounterAttributeModifier::EnableIncImpl,
    };
    return &ArkUICounterModifierImpl;
}

}

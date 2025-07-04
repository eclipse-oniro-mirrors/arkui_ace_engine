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
namespace GridRowModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // GridRowModifier
namespace GridRowInterfaceModifier {
void SetGridRowOptionsImpl(Ark_NativePointer node,
                           const Opt_GridRowOptions* option)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = option ? Converter::OptConvert<type>(*option) : std::nullopt;
    //GridRowModelNG::SetSetGridRowOptions(frameNode, convValue);
}
} // GridRowInterfaceModifier
namespace GridRowAttributeModifier {
void SetOnBreakpointChangeImpl(Ark_NativePointer node,
                               const Opt_Callback_String_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //GridRowModelNG::SetSetOnBreakpointChange(frameNode, convValue);
}
void SetAlignItemsImpl(Ark_NativePointer node,
                       const Opt_ItemAlign* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //GridRowModelNG::SetSetAlignItems(frameNode, convValue);
}
} // GridRowAttributeModifier
const GENERATED_ArkUIGridRowModifier* GetGridRowModifier()
{
    static const GENERATED_ArkUIGridRowModifier ArkUIGridRowModifierImpl {
        GridRowModifier::ConstructImpl,
        GridRowInterfaceModifier::SetGridRowOptionsImpl,
        GridRowAttributeModifier::SetOnBreakpointChangeImpl,
        GridRowAttributeModifier::SetAlignItemsImpl,
    };
    return &ArkUIGridRowModifierImpl;
}

}

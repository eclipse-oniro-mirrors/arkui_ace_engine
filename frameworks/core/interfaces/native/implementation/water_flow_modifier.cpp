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
namespace WaterFlowModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // WaterFlowModifier
namespace WaterFlowInterfaceModifier {
void SetWaterFlowOptionsImpl(Ark_NativePointer node,
                             const Opt_WaterFlowOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = options ? Converter::OptConvert<type>(*options) : std::nullopt;
    //WaterFlowModelNG::SetSetWaterFlowOptions(frameNode, convValue);
}
} // WaterFlowInterfaceModifier
namespace WaterFlowAttributeModifier {
void ColumnsTemplateImpl(Ark_NativePointer node,
                         const Ark_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::Convert<std::string>(*value);
    //WaterFlowModelNG::SetColumnsTemplate(frameNode, convValue);
}
void ItemConstraintSizeImpl(Ark_NativePointer node,
                            const Ark_ConstraintSizeOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetItemConstraintSize(frameNode, convValue);
}
void RowsTemplateImpl(Ark_NativePointer node,
                      const Ark_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::Convert<std::string>(*value);
    //WaterFlowModelNG::SetRowsTemplate(frameNode, convValue);
}
void ColumnsGapImpl(Ark_NativePointer node,
                    const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetColumnsGap(frameNode, convValue);
}
void RowsGapImpl(Ark_NativePointer node,
                 const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetRowsGap(frameNode, convValue);
}
void LayoutDirectionImpl(Ark_NativePointer node,
                         Ark_FlexDirection value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //WaterFlowModelNG::SetLayoutDirection(frameNode, convValue);
}
void NestedScrollImpl(Ark_NativePointer node,
                      const Ark_NestedScrollOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetNestedScroll(frameNode, convValue);
}
void EnableScrollInteractionImpl(Ark_NativePointer node,
                                 Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //WaterFlowModelNG::SetEnableScrollInteraction(frameNode, convValue);
}
void FrictionImpl(Ark_NativePointer node,
                  const Ark_Union_Number_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetFriction(frameNode, convValue);
}
void CachedCount0Impl(Ark_NativePointer node,
                      const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetCachedCount0(frameNode, convValue);
}
void CachedCount1Impl(Ark_NativePointer node,
                      const Ark_Number* count,
                      Ark_Boolean show)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(count);
    //auto convValue = Converter::OptConvert<type>(count); // for enums
    //WaterFlowModelNG::SetCachedCount1(frameNode, convValue);
}
void OnReachStartImpl(Ark_NativePointer node,
                      const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetOnReachStart(frameNode, convValue);
}
void OnReachEndImpl(Ark_NativePointer node,
                    const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetOnReachEnd(frameNode, convValue);
}
void OnScrollFrameBeginImpl(Ark_NativePointer node,
                            const Callback_Number_ScrollState_Literal_Number_offsetRemain* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetOnScrollFrameBegin(frameNode, convValue);
}
void OnScrollIndexImpl(Ark_NativePointer node,
                       const Callback_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //WaterFlowModelNG::SetOnScrollIndex(frameNode, convValue);
}
} // WaterFlowAttributeModifier
const GENERATED_ArkUIWaterFlowModifier* GetWaterFlowModifier()
{
    static const GENERATED_ArkUIWaterFlowModifier ArkUIWaterFlowModifierImpl {
        WaterFlowModifier::ConstructImpl,
        WaterFlowInterfaceModifier::SetWaterFlowOptionsImpl,
        WaterFlowAttributeModifier::ColumnsTemplateImpl,
        WaterFlowAttributeModifier::ItemConstraintSizeImpl,
        WaterFlowAttributeModifier::RowsTemplateImpl,
        WaterFlowAttributeModifier::ColumnsGapImpl,
        WaterFlowAttributeModifier::RowsGapImpl,
        WaterFlowAttributeModifier::LayoutDirectionImpl,
        WaterFlowAttributeModifier::NestedScrollImpl,
        WaterFlowAttributeModifier::EnableScrollInteractionImpl,
        WaterFlowAttributeModifier::FrictionImpl,
        WaterFlowAttributeModifier::CachedCount0Impl,
        WaterFlowAttributeModifier::CachedCount1Impl,
        WaterFlowAttributeModifier::OnReachStartImpl,
        WaterFlowAttributeModifier::OnReachEndImpl,
        WaterFlowAttributeModifier::OnScrollFrameBeginImpl,
        WaterFlowAttributeModifier::OnScrollIndexImpl,
    };
    return &ArkUIWaterFlowModifierImpl;
}

}

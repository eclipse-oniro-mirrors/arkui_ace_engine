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
#include "core/components_ng/pattern/panel/sliding_panel_model_ng.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "core/interfaces/arkoala/utility/validators.h"
#include "core/interfaces/arkoala/generated/interface/node_api.h"

namespace OHOS::Ace::NG::Converter {
void AssignArkValue(Ark_PanelMode& dst, const PanelMode& src)
{
    switch (src) {
        case PanelMode::MINI: dst = Ark_PanelMode::ARK_PANEL_MODE_MINI; break;
        case PanelMode::HALF: dst = Ark_PanelMode::ARK_PANEL_MODE_HALF; break;
        case PanelMode::FULL: dst = Ark_PanelMode::ARK_PANEL_MODE_FULL; break;
        default: dst = static_cast<Ark_PanelMode>(-1);
            LOGE("Unexpected enum value in PanelMode: %{public}d", src);
    }
}
template<>
void AssignCast(std::optional<PanelType>& dst, const Ark_PanelType& src)
{
    switch (src) {
        case Ark_PanelType::ARK_PANEL_TYPE_MINIBAR: dst = PanelType::MINI_BAR; break;
        case Ark_PanelType::ARK_PANEL_TYPE_FOLDABLE: dst = PanelType::FOLDABLE_BAR; break;
        case Ark_PanelType::ARK_PANEL_TYPE_TEMPORARY: dst = PanelType::TEMP_DISPLAY; break;
        case Ark_PanelType::ARK_PANEL_TYPE_CUSTOM: dst = PanelType::CUSTOM; break;
        default: LOGE("Unexpected enum value in Ark_PanelType: %{public}d", src);
    }
}
template<>
void AssignCast(std::optional<PanelMode>& dst, const Ark_PanelMode& src)
{
    switch (src) {
        case Ark_PanelMode::ARK_PANEL_MODE_MINI: dst = PanelMode::MINI; break;
        case Ark_PanelMode::ARK_PANEL_MODE_HALF: dst = PanelMode::HALF; break;
        case Ark_PanelMode::ARK_PANEL_MODE_FULL: dst = PanelMode::FULL; break;
        default: LOGE("Unexpected enum value in Ark_PanelMode: %{public}d", src);
    }
}
template<>
void AssignCast(std::optional<CalcDimension>& dst, const Ark_PanelHeight& src)
{
    switch (src) {
        case Ark_PanelHeight::ARK_PANEL_HEIGHT_WRAP_CONTENT: dst = CalcDimension("wrapContent"); break;
        default: LOGE("Unexpected enum value in Ark_PanelHeight: %{public}d", src);
    }
}
} // OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace PanelInterfaceModifier {
void SetPanelOptionsImpl(Ark_NativePointer node,
                         Ark_Boolean show)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(show);
    SlidingPanelModelNG::SetIsShow(frameNode, convValue);
}
} // PanelInterfaceModifier
namespace PanelAttributeModifier {
void ModeImpl(Ark_NativePointer node,
              Ark_PanelMode value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    SlidingPanelModelNG::SetPanelMode(frameNode, Converter::OptConvert<PanelMode>(value));
}
void TypeImpl(Ark_NativePointer node,
              Ark_PanelType value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    SlidingPanelModelNG::SetPanelType(frameNode, Converter::OptConvert<PanelType>(value));
}
void DragBarImpl(Ark_NativePointer node,
                 Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto isChecked = Converter::Convert<bool>(value);
    SlidingPanelModelNG::SetHasDragBar(frameNode, isChecked);
}
void CustomHeightImpl(Ark_NativePointer node,
                      const Ark_Union_Dimension_PanelHeight* value)
{
    CHECK_NULL_VOID(value);
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto customHeight = Converter::OptConvert<CalcDimension>(*value);
    Validator::ValidateNonNegative(customHeight);
    Validator::ValidateNonPercent(customHeight);
    Validator::ValidateNonEmpty(customHeight);
    SlidingPanelModelNG::SetPanelCustomHeight(frameNode, customHeight);
}
void FullHeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_String* value)
{
    CHECK_NULL_VOID(value);
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto fullHeight = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(fullHeight);
    Validator::ValidateNonPercent(fullHeight);
    SlidingPanelModelNG::SetPanelFullHeight(frameNode, fullHeight);
}
void HalfHeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_String* value)
{
    CHECK_NULL_VOID(value);
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto halfHeight = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(halfHeight);
    Validator::ValidateNonPercent(halfHeight);
    SlidingPanelModelNG::SetPanelHalfHeight(frameNode, halfHeight);
}
void MiniHeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_String* value)
{
    CHECK_NULL_VOID(value);
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto miniHeight = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(miniHeight);
    Validator::ValidateNonPercent(miniHeight);
    SlidingPanelModelNG::SetPanelMiniHeight(frameNode, miniHeight);
}
void ShowImpl(Ark_NativePointer node,
              Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto isChecked = Converter::Convert<bool>(value);
    SlidingPanelModelNG::SetIsShow(frameNode, isChecked);
}
void BackgroundMaskImpl(Ark_NativePointer node,
                        const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto colorValue = Converter::OptConvert<Color>(*value);
    SlidingPanelModelNG::SetPanelBackgroundMask(frameNode, colorValue);
}
void ShowCloseIconImpl(Ark_NativePointer node,
                       Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    SlidingPanelModelNG::SetShowCloseIcon(frameNode, Converter::Convert<bool>(value));
}
void OnChangeImpl(Ark_NativePointer node,
                  const Callback_Number_Number_PanelMode_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onEvent = [frameNode](const BaseEventInfo* info) {
        const auto* eventInfo = TypeInfoHelper::DynamicCast<SlidingPanelSizeChangeEvent>(info);
        auto width = Converter::ArkValue<Ark_Number>(eventInfo->GetWidth());
        auto height = Converter::ArkValue<Ark_Number>(eventInfo->GetHeight());
        auto mode = Converter::ArkValue<Ark_PanelMode>(eventInfo->GetMode());
        GetFullAPI()->getEventsAPI()->getPanelEventsReceiver()->onChange(frameNode->GetId(), width, height, mode);
    };
    SlidingPanelModelNG::SetOnSizeChange(frameNode, std::move(onEvent));
}
void OnHeightChangeImpl(Ark_NativePointer node,
                        const Callback_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onEvent = [frameNode](const int32_t value) {
        auto arkIndex = Converter::ArkValue<Ark_Number>(value);
        GetFullAPI()->getEventsAPI()->getPanelEventsReceiver()->onHeightChange(frameNode->GetId(), arkIndex);
    };
    SlidingPanelModelNG::SetOnHeightChange(frameNode, std::move(onEvent));
}
} // PanelAttributeModifier
const GENERATED_ArkUIPanelModifier* GetPanelModifier()
{
    static const GENERATED_ArkUIPanelModifier ArkUIPanelModifierImpl {
        PanelInterfaceModifier::SetPanelOptionsImpl,
        PanelAttributeModifier::ModeImpl,
        PanelAttributeModifier::TypeImpl,
        PanelAttributeModifier::DragBarImpl,
        PanelAttributeModifier::CustomHeightImpl,
        PanelAttributeModifier::FullHeightImpl,
        PanelAttributeModifier::HalfHeightImpl,
        PanelAttributeModifier::MiniHeightImpl,
        PanelAttributeModifier::ShowImpl,
        PanelAttributeModifier::BackgroundMaskImpl,
        PanelAttributeModifier::ShowCloseIconImpl,
        PanelAttributeModifier::OnChangeImpl,
        PanelAttributeModifier::OnHeightChangeImpl,
    };
    return &ArkUIPanelModifierImpl;
}

}
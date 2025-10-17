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
#include "core/components_ng/pattern/security_component/security_component_model_ng.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/validators.h"
#include "core/interfaces/native/generated/interface/ui_node_api.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::Converter {
template<>
void AssignCast(std::optional<SecurityComponentLayoutDirection>& dst, const Ark_SecurityComponentLayoutDirection& src)
{
    switch (src) {
        case ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_HORIZONTAL:
            dst = SecurityComponentLayoutDirection::HORIZONTAL;
            break;
        case ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_VERTICAL:
            dst = SecurityComponentLayoutDirection::VERTICAL;
            break;
        default:
            LOGE("Unexpected enum value in Ark_SecurityComponentLayoutDirection: %{public}d", src);
    }
}
template<>
OffsetT<Dimension> Convert(const Ark_Position& src)
{
    OffsetT<Dimension> offset;
    auto x = Converter::OptConvert<Dimension>(src.x);
    auto y = Converter::OptConvert<Dimension>(src.y);
    if (x.has_value()) {
        offset.SetX(x.value());
    }
    if (y.has_value()) {
        offset.SetY(y.value());
    }
    return offset;
}
template<>
PaddingPropertyT<Dimension> Convert(const Ark_Padding& src)
{
    PaddingPropertyT<Dimension> padding;
    // padding.left = Converter::OptConvert<Dimension>(src.left);
    // padding.top = Converter::OptConvert<Dimension>(src.top);
    // padding.right = Converter::OptConvert<Dimension>(src.right);
    // padding.bottom = Converter::OptConvert<Dimension>(src.bottom);
    return padding;
}
template<>
PaddingPropertyT<Dimension> Convert(const Ark_Dimension& src)
{
    PaddingPropertyT<Dimension> padding;
    // auto value = Converter::Convert<Dimension>(src);
    // padding.left = value;
    // padding.top = value;
    // padding.right = value;
    // padding.bottom = value;
    return padding;
}
} //namespace OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace CommonMethodModifier {
void SetSizeImpl(Ark_NativePointer node,
    const Opt_SizeOptions* value);
void SetConstraintSizeImpl(Ark_NativePointer node,
    const Opt_ConstraintSizeOptions* value);
void SetOffsetImpl(Ark_NativePointer node,
    const Opt_Union_Position_Edges_LocalizedEdges* value);
void SetIdImpl(Ark_NativePointer node,
    const Opt_String* value);
void SetEnabledImpl(Ark_NativePointer node,
    const Opt_Boolean *value);
void SetChainModeImpl(Ark_NativePointer node,
    const Opt_Axis *direction,
    const Opt_ChainStyle *style);
} // namespace CommonMethodModifier
namespace SecurityComponentMethodModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
void SetIconSizeImpl(Ark_NativePointer node,
                     const Opt_Dimension* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto valueOpt = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(valueOpt);
    Validator::ValidateNonPercent(valueOpt);
    SecurityComponentModelNG::SetIconSize(frameNode, valueOpt);
}
void SetLayoutDirectionImpl(Ark_NativePointer node,
                         const Opt_SecurityComponentLayoutDirection* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto layoutDirection = Converter::OptConvert<SecurityComponentLayoutDirection>(*value);
    SecurityComponentModelNG::SetTextIconLayoutDirection(frameNode, layoutDirection);
}
void SetPositionImpl(Ark_NativePointer node,
                  const Opt_Position* value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<OffsetT<Dimension>>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    // ViewAbstract::SetPosition(frameNode, *convValue);
}
void SetMarkAnchorImpl(Ark_NativePointer node,
                    const Opt_Position* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    // ViewAbstract::MarkAnchor(frameNode, Converter::OptConvert<OffsetT<Dimension>>(*value));
}
void SetOffsetImpl(Ark_NativePointer node,
                const Opt_Union_Position_Edges_LocalizedEdges* value)
{
    CommonMethodModifier::SetOffsetImpl(node, value);
}
void SetFontSizeImpl(Ark_NativePointer node,
                     const Opt_Dimension* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto fontSize = Converter::OptConvert<Dimension>(*value);
    Validator::ValidatePositive(fontSize);
    Validator::ValidateNonPercent(fontSize);
    SecurityComponentModelNG::SetFontSize(frameNode, fontSize);
}
void SetFontStyleImpl(Ark_NativePointer node,
                   const Opt_FontStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    SecurityComponentModelNG::SetFontStyle(frameNode, Converter::OptConvert<Ace::FontStyle>(*value));
}
void SetFontWeightImpl(Ark_NativePointer node,
                       const Opt_Union_I32_FontWeight_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    SecurityComponentModelNG::SetFontWeight(frameNode, Converter::OptConvert<FontWeight>(*value));
}
void SetFontFamilyImpl(Ark_NativePointer node,
                    const Opt_Union_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::optional<StringArray> families;
    if (auto fontfamiliesOpt = Converter::OptConvert<Converter::FontFamilies>(*value); fontfamiliesOpt) {
        families = fontfamiliesOpt->families;
    }
    Validator::ValidateNonEmpty(families);
    SecurityComponentModelNG::SetFontFamily(frameNode, families);
}
void SetFontColorImpl(Ark_NativePointer node,
                   const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto fontColor = Converter::OptConvert<Color>(*value);
    SecurityComponentModelNG::SetFontColor(frameNode, fontColor);
}
void SetIconColorImpl(Ark_NativePointer node,
                   const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto iconColor = Converter::OptConvert<Color>(*value);
    SecurityComponentModelNG::SetIconColor(frameNode, iconColor);
}
void SetBackgroundColorImpl(Ark_NativePointer node,
                         const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto сolor = Converter::OptConvert<Color>(*value);
    SecurityComponentModelNG::SetBackgroundColor(frameNode, сolor);
}
void SetBorderStyleImpl(Ark_NativePointer node,
                     const Opt_BorderStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::OptConvert<BorderStyle>(*value);
    SecurityComponentModelNG::SetBackgroundBorderStyle(frameNode, optValue);
}
void SetBorderWidthImpl(Ark_NativePointer node,
                        const Opt_Dimension* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(convValue);
    SecurityComponentModelNG::SetBackgroundBorderWidth(frameNode, convValue);
}
void SetBorderColorImpl(Ark_NativePointer node,
                     const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto color = Converter::OptConvert<Color>(*value);
    SecurityComponentModelNG::SetBackgroundBorderColor(frameNode, color);
}
void SetBorderRadiusImpl(Ark_NativePointer node,
                         const Opt_Union_Dimension_BorderRadiuses* radius)
{
    // auto frameNode = reinterpret_cast<FrameNode *>(node);
    // CHECK_NULL_VOID(frameNode);
    // auto convValue = Converter::OptConvert<Dimension>(*radius);
    // Validator::ValidateNonNegative(convValue);
    // SecurityComponentModelNG::SetBackgroundBorderRadius(frameNode, convValue);
}
void SetPaddingImpl(Ark_NativePointer node,
                    const Opt_Union_Padding_Dimension* value)
{
    // auto frameNode = reinterpret_cast<FrameNode *>(node);
    // CHECK_NULL_VOID(frameNode);
    // auto convValue = Converter::OptConvert<BorderRadiusProperty>(*value);
    // SecurityComponentModelNG::SetBackgroundBorderRadius(frameNode, convValue);
}
void SetTextIconSpaceImpl(Ark_NativePointer node,
                          const Opt_Dimension* value)
{
    // auto frameNode = reinterpret_cast<FrameNode *>(node);
    // CHECK_NULL_VOID(frameNode);
    // auto padding = Converter::OptConvert<PaddingPropertyT<Dimension>>(*value);
    // if (padding) {
    //     SecurityComponentModelNG::SetBackgroundPadding(frameNode, padding->left, padding->right, padding->top,
    //         padding->bottom);
    // } else {
    //     SecurityComponentModelNG::SetBackgroundPadding(frameNode, std::nullopt, std::nullopt, std::nullopt,
    //         std::nullopt);
    // }
}
void SetTextIconSpaceImpl(Ark_NativePointer node,
                       const Opt_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto valueOpt = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(valueOpt);
    SecurityComponentModelNG::SetTextIconSpace(frameNode, valueOpt);
}
void SetKeyImpl(Ark_NativePointer node,
             const Opt_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<std::string>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    // ViewAbstract::SetInspectorId(frameNode, *convValue);
}
void SetWidthImpl(Ark_NativePointer node,
               const Opt_Length* value)
{
    // CommonMethodModifier::Width0Impl(node, value);
}
void SetHeightImpl(Ark_NativePointer node,
                const Opt_Length* value)
{
    // CommonMethodModifier::Height0Impl(node, value);
}
void SetSizeImpl(Ark_NativePointer node,
              const Opt_SizeOptions* value)
{
    CommonMethodModifier::SetSizeImpl(node, value);
}
void SetConstraintSizeImpl(Ark_NativePointer node,
                        const Opt_ConstraintSizeOptions* value)
{
    CommonMethodModifier::SetConstraintSizeImpl(node, value);
}
void SetAlignImpl(Ark_NativePointer node,
               const Opt_Alignment* alignType)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<Alignment>(*alignType);
    SecurityComponentModelNG::SetAlign(frameNode, convValue);
}
void SetAlignRulesImpl(Ark_NativePointer node,
                       const Opt_Union_AlignRuleOption_LocalizedAlignRuleOptions* alignRule)
{
}
void SetIdImpl(Ark_NativePointer node,
               const Opt_String* description)
{
    CommonMethodModifier::SetIdImpl(node, description);
}
void SetChainModeImpl(Ark_NativePointer node,
                      const Opt_Axis* direction,
                      const Opt_ChainStyle* style)
{
    CommonMethodModifier::SetChainModeImpl(node, direction, style);
}
void SetMinFontScaleImpl(Ark_NativePointer node,
                         const Opt_Union_F64_Resource* scale)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto minFontScale = Converter::OptConvert<float>(*scale);
    Validator::ValidatePositive(minFontScale);
    const auto maxValue = 1.f;
    Validator::ValidateLessOrEqual(minFontScale, maxValue);
    SecurityComponentModelNG::SetMinFontScale(frameNode, minFontScale);
}
void SetMaxFontScaleImpl(Ark_NativePointer node,
                         const Opt_Union_F64_Resource* scale)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto maxFontScale = Converter::OptConvert<float>(*scale);
    const auto minValue = 1.f;
    Validator::ValidateGreatOrEqual(maxFontScale, minValue);
    SecurityComponentModelNG::SetMaxFontScale(frameNode, maxFontScale);
}
void SetMaxLinesImpl(Ark_NativePointer node,
                     const Opt_Union_I32_Resource* line)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto maxLines = Converter::OptConvert<int32_t>(*line);
    Validator::ValidateNonNegative(maxLines);
    SecurityComponentModelNG::SetMaxLines(frameNode, maxLines);
}
void SetMinFontSizeImpl(Ark_NativePointer node,
                        const Opt_Union_F64_String_Resource* minSize)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto fontSize = Converter::OptConvert<Dimension>(*minSize);
    Validator::ValidateNonNegative(fontSize);
    Validator::ValidateNonPercent(fontSize);
    SecurityComponentModelNG::SetAdaptMinFontSize(frameNode, fontSize);
}
void SetMaxFontSizeImpl(Ark_NativePointer node,
                        const Opt_Union_F64_String_Resource* maxSize)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto fontSize = Converter::OptConvert<Dimension>(*maxSize);
    Validator::ValidateNonNegative(fontSize);
    Validator::ValidateNonPercent(fontSize);
    SecurityComponentModelNG::SetAdaptMaxFontSize(frameNode, fontSize);
}
void SetHeightAdaptivePolicyImpl(Ark_NativePointer node,
                                 const Opt_TextHeightAdaptivePolicy* policy)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<TextHeightAdaptivePolicy>(*policy);
    SecurityComponentModelNG::SetHeightAdaptivePolicy(frameNode, convValue);
}
void SetEnabledImpl(Ark_NativePointer node,
                    const Opt_Boolean* respond)
{
    CommonMethodModifier::SetEnabledImpl(node, respond);
}
} // SecurityComponentMethodModifier
const GENERATED_ArkUISecurityComponentMethodModifier* GetSecurityComponentMethodModifier()
{
    static const GENERATED_ArkUISecurityComponentMethodModifier ArkUISecurityComponentMethodModifierImpl {
        SecurityComponentMethodModifier::ConstructImpl,
        SecurityComponentMethodModifier::SetIconSizeImpl,
        SecurityComponentMethodModifier::SetLayoutDirectionImpl,
        SecurityComponentMethodModifier::SetPositionImpl,
        SecurityComponentMethodModifier::SetMarkAnchorImpl,
        SecurityComponentMethodModifier::SetOffsetImpl,
        SecurityComponentMethodModifier::SetFontSizeImpl,
        SecurityComponentMethodModifier::SetFontStyleImpl,
        SecurityComponentMethodModifier::SetFontWeightImpl,
        SecurityComponentMethodModifier::SetFontFamilyImpl,
        SecurityComponentMethodModifier::SetFontColorImpl,
        SecurityComponentMethodModifier::SetIconColorImpl,
        SecurityComponentMethodModifier::SetBackgroundColorImpl,
        SecurityComponentMethodModifier::SetBorderStyleImpl,
        SecurityComponentMethodModifier::SetBorderWidthImpl,
        SecurityComponentMethodModifier::SetBorderColorImpl,
        SecurityComponentMethodModifier::SetBorderRadiusImpl,
        SecurityComponentMethodModifier::SetPaddingImpl,
        SecurityComponentMethodModifier::SetTextIconSpaceImpl,
        SecurityComponentMethodModifier::SetKeyImpl,
        SecurityComponentMethodModifier::SetWidthImpl,
        SecurityComponentMethodModifier::SetHeightImpl,
        SecurityComponentMethodModifier::SetSizeImpl,
        SecurityComponentMethodModifier::SetConstraintSizeImpl,
        SecurityComponentMethodModifier::SetAlignImpl,
        SecurityComponentMethodModifier::SetAlignRulesImpl,
        SecurityComponentMethodModifier::SetIdImpl,
        SecurityComponentMethodModifier::SetChainModeImpl,
        SecurityComponentMethodModifier::SetMinFontScaleImpl,
        SecurityComponentMethodModifier::SetMaxFontScaleImpl,
        SecurityComponentMethodModifier::SetMaxLinesImpl,
        SecurityComponentMethodModifier::SetMinFontSizeImpl,
        SecurityComponentMethodModifier::SetMaxFontSizeImpl,
        SecurityComponentMethodModifier::SetHeightAdaptivePolicyImpl,
        SecurityComponentMethodModifier::SetEnabledImpl,
    };
    return &ArkUISecurityComponentMethodModifierImpl;
}

}

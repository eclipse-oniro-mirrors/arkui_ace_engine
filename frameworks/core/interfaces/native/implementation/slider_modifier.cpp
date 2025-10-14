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
#include "core/components_ng/pattern/slider/slider_model_ng.h"
#include "core/components_ng/pattern/slider/slider_model_static.h"
#include "core/components/slider/slider_theme.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/validators.h"

namespace OHOS::Ace::NG {
namespace {
std::optional<float> ProcessBindableValue(FrameNode* frameNode, const Opt_Union_F64_Bindable& value)
{
    std::optional<float> result;
    Converter::VisitUnion(value,
        [&result](const Ark_Float64& src) {
            result = Converter::OptConvert<float>(src);
        },
        [&result, frameNode](const Ark_Bindable_F64& src) {
            result = Converter::OptConvert<float>(src.value);
            WeakPtr<FrameNode> weakNode = AceType::WeakClaim(frameNode);
            auto onEvent = [arkCallback = CallbackHelper(src.onChange), weakNode](float value) {
                PipelineContext::SetCallBackNode(weakNode);
                arkCallback.Invoke(Converter::ArkValue<Ark_Float64>(value));
            };
            SliderModelStatic::SetOnChangeEvent(frameNode, std::move(onEvent));
        },
        [] {});
    return result;
}
} // namespace
namespace Converter {
struct SliderRange {
    std::optional<float> from;
    std::optional<float> to;
};

template<>
SliderRange Convert(const Ark_SlideRange& src)
{
    return {
        .from = Converter::OptConvert<float>(src.from),
        .to = Converter::OptConvert<float>(src.to)
    };
}

struct SliderBlockSizeOptions {
    std::optional<Dimension> width;
    std::optional<Dimension> height;
};

template<>
SliderBlockSizeOptions Convert(const Ark_SizeOptions& src)
{
    return {
        .width = Converter::OptConvert<Dimension>(src.width),
        .height = Converter::OptConvert<Dimension>(src.height)
    };
}

struct SliderBlockImageInfo {
    std::optional<std::string> value;
    std::optional<std::string> bundleName;
    std::optional<std::string> moduleName;
};

struct SliderBlockStyle {
    std::optional<SliderModel::BlockStyleType> type;
    std::optional<ImageSourceInfo> image;
    std::optional<std::string> shape;
};

template<>
SliderBlockStyle Convert(const Ark_SliderBlockStyle& src)
{
    return {
        .type = Converter::OptConvert<SliderModel::BlockStyleType>(src.type),
        .image = Converter::OptConvert<ImageSourceInfo>(src.image),
#ifdef WRONG_GEN
        .shape = Converter::OptConvert<std::string>(src.shape)
#endif
    };
}
} // namespace Converter
} // namespace OHOS::Ace::NG
namespace OHOS::Ace::NG::GeneratedModifier {
namespace SliderModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = SliderModelNG::CreateFrameNode(id);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // SliderModifier
namespace SliderInterfaceModifier {
void SetSliderOptionsImpl(Ark_NativePointer node,
                          const Opt_SliderOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optOptions = Converter::GetOptPtr(options);
    CHECK_NULL_VOID(optOptions);
    auto value = ProcessBindableValue(frameNode, optOptions->value);
    auto minVal = Converter::OptConvert<float>(optOptions->min);
    auto maxVal = Converter::OptConvert<float>(optOptions->max);
    auto step = Converter::OptConvert<float>(optOptions->step);
    auto style = Converter::OptConvert<SliderModel::SliderMode>(optOptions->style);
    auto direction = Converter::OptConvert<Axis>(optOptions->direction);
    auto reverse = Converter::OptConvert<bool>(optOptions->reverse);

    SliderModelStatic::SetMinLabel(frameNode, minVal);
    SliderModelStatic::SetMaxLabel(frameNode, maxVal);
    Validator::ValidatePositive(step);
    SliderModelStatic::SetStep(frameNode, step);
    SliderModelStatic::SetSliderValue(frameNode, value);
    SliderModelStatic::SetDirection(frameNode, direction);
    SliderModelStatic::SetReverse(frameNode, reverse);
    SliderModelStatic::SetSliderMode(frameNode, style);
}
} // SliderInterfaceModifier
namespace SliderAttributeModifier {
void SetBlockColorImpl(Ark_NativePointer node,
                       const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Color>(value);
    SliderModelStatic::SetBlockColor(frameNode, convValue);
}

void SetTrackColorImpl(Ark_NativePointer node,
                       const Opt_Union_ResourceColor_LinearGradient* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Converter::VisitUnion(*value,
        [frameNode](const Ark_ResourceColor& value) {
            auto colorOpt = Converter::OptConvert<Color>(value);
            auto gradientOpt = colorOpt.has_value() ?
                std::optional<Gradient>{SliderModelNG::CreateSolidGradient(colorOpt.value())} : std::nullopt;
            SliderModelStatic::SetTrackBackgroundColor(frameNode, gradientOpt, true);
        },
        [frameNode](const Ark_LinearGradient& value) {
            auto gradientOpt = Converter::OptConvert<Gradient>(value);
            SliderModelStatic::SetTrackBackgroundColor(frameNode, gradientOpt);
        },
        [frameNode]() {
            SliderModelStatic::ResetTrackBackgroundColor(frameNode);
        }
    );
}
void SetSelectedColorImpl(Ark_NativePointer node,
                          const Opt_Union_ResourceColor_LinearGradient* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Converter::VisitUnion(*value,
        [frameNode](const Ark_ResourceColor& value) {
            auto colorOpt = Converter::OptConvert<Color>(value);
            auto gradientOpt = colorOpt.has_value() ?
                std::optional<Gradient>{SliderModelNG::CreateSolidGradient(colorOpt.value())} : std::nullopt;
            SliderModelStatic::SetSelectColor(frameNode, gradientOpt, true);
        },
        [frameNode](const Ark_LinearGradient& value) {
            auto gradientOpt = Converter::OptConvert<Gradient>(value);
            SliderModelStatic::SetSelectColor(frameNode, gradientOpt);
        },
        [frameNode]() {
            SliderModelStatic::SetSelectColor(frameNode, std::nullopt, false);
        });
}
void SetShowStepsImpl(Ark_NativePointer node,
                      const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<bool>(value);
    if (!convValue) {
        SliderModelStatic::ResetShowSteps(frameNode);
        return;
    }
    SliderModelNG::SetShowSteps(frameNode, *convValue);
}
void SetTrackThicknessImpl(Ark_NativePointer node,
                           const Opt_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Dimension>(value);
    Validator::ValidatePositive(convValue);
    SliderModelStatic::SetThickness(frameNode, convValue);
}
void SetOnChangeImpl(Ark_NativePointer node,
                     const Opt_Callback_F64_SliderChangeMode_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        SliderModelNG::SetOnChange(frameNode, nullptr);
        return;
    }
    auto onChange = [arkCallback = CallbackHelper(*optValue)](float newValue, int32_t mode) {
        Ark_Float64 arkValue = Converter::ArkValue<Ark_Float64>(newValue);
        Ark_SliderChangeMode arkMode = Converter::ArkValue<Ark_SliderChangeMode>(
            static_cast<SliderModel::SliderChangeMode>(mode));
        arkCallback.Invoke(arkValue, arkMode);
    };
    SliderModelNG::SetOnChange(frameNode, std::move(onChange));
}
void SetBlockBorderColorImpl(Ark_NativePointer node,
                             const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Color>(value);
    SliderModelStatic::SetBlockBorderColor(frameNode, convValue);
}
void SetBlockBorderWidthImpl(Ark_NativePointer node,
                             const Opt_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Dimension>(value);
    Validator::ValidateNonNegative(convValue);
    SliderModelStatic::SetBlockBorderWidth(frameNode, convValue);
}
void SetStepColorImpl(Ark_NativePointer node,
                      const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Color>(value);
    SliderModelStatic::SetStepColor(frameNode, convValue);
}
void SetTrackBorderRadiusImpl(Ark_NativePointer node,
                              const Opt_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Dimension>(value);
    Validator::ValidateNonNegative(convValue);
    SliderModelStatic::SetTrackBorderRadius(frameNode, convValue);
}
void SetSelectedBorderRadiusImpl(Ark_NativePointer node,
                                 const Opt_Dimension* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Dimension>(value);
    Validator::ValidateNonNegative(convValue);
    Validator::ValidateNonPercent(convValue);
    SliderModelStatic::SetSelectedBorderRadius(frameNode, convValue);
}
void SetBlockSizeImpl(Ark_NativePointer node,
                      const Opt_SizeOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Converter::SliderBlockSizeOptions>(value)
        .value_or(Converter::SliderBlockSizeOptions{});
    SliderModelStatic::SetBlockSize(frameNode, convValue.width, convValue.height);
}
void SetBlockStyleImpl(Ark_NativePointer node,
                       const Opt_SliderBlockStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Converter::SliderBlockStyle>(value);
    if (convValue.has_value()) {
        SliderModelStatic::SetBlockType(frameNode, convValue.value().type);
        if (convValue.value().image.has_value()) {
            SliderModelNG::SetBlockImage(frameNode, convValue.value().image->GetSrc(),
                convValue.value().image->GetBundleName(), convValue.value().image->GetModuleName());
        } else {
            SliderModelNG::ResetBlockImage(frameNode);
        }
        LOGE("SliderModifier::BlockStyleImpl is not implemented, raw pointer is not supported!");
    } else {
        SliderModelStatic::SetBlockType(frameNode, std::nullopt);
        SliderModelNG::ResetBlockImage(frameNode);
    }
}
void SetStepSizeImpl(Ark_NativePointer node,
                     const Opt_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Dimension>(value);
    Validator::ValidateNonNegative(convValue);
    SliderModelStatic::SetStepSize(frameNode, convValue);
}
void SetSliderInteractionModeImpl(Ark_NativePointer node,
                                  const Opt_SliderInteraction* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<SliderModel::SliderInteraction>(value);
    SliderModelStatic::SetSliderInteractionMode(frameNode, convValue);
}
void SetMinResponsiveDistanceImpl(Ark_NativePointer node,
                                  const Opt_Float64* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<float>(value);
    Validator::ValidateNonNegative(convValue);
    SliderModelStatic::SetMinResponsiveDistance(frameNode, convValue);
}
void SetSlideRangeImpl(Ark_NativePointer node,
                       const Opt_SlideRange* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Converter::SliderRange>(value).value_or(Converter::SliderRange{});
    SliderModelStatic::SetValidSlideRange(frameNode, convValue.from, convValue.to);
}
void SetDigitalCrownSensitivityImpl(Ark_NativePointer node,
                                    const Opt_CrownSensitivity* value)
{
#ifdef SUPPORT_DIGITAL_CROWN
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<CrownSensitivity>(value);
    SliderModelStatic::SetDigitalCrownSensitivity(frameNode, convValue);
#endif
}
void SetEnableHapticFeedbackImpl(Ark_NativePointer node,
                                 const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<bool>(value);
    if (!convValue.has_value()) {
        SliderModelNG::SetEnableHapticFeedback(frameNode, true);
        return;
    }
    SliderModelNG::SetEnableHapticFeedback(frameNode, *convValue);
}
void SetShowTipsImpl(Ark_NativePointer node,
                     const Opt_Boolean* value,
                     const Opt_ResourceStr* content)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<bool>(value);
    if (!convValue) {
        SliderModelStatic::ResetShowTips(frameNode);
        return;
    }
    auto convContent = Converter::OptConvertPtr<std::string>(content);
    SliderModelNG::SetShowTips(frameNode, *convValue, convContent);
}
} // SliderAttributeModifier
const GENERATED_ArkUISliderModifier* GetSliderModifier()
{
    static const GENERATED_ArkUISliderModifier ArkUISliderModifierImpl {
        SliderModifier::ConstructImpl,
        SliderInterfaceModifier::SetSliderOptionsImpl,
        SliderAttributeModifier::SetBlockColorImpl,
        SliderAttributeModifier::SetTrackColorImpl,
        SliderAttributeModifier::SetSelectedColorImpl,
        SliderAttributeModifier::SetShowStepsImpl,
        SliderAttributeModifier::SetTrackThicknessImpl,
        SliderAttributeModifier::SetOnChangeImpl,
        SliderAttributeModifier::SetBlockBorderColorImpl,
        SliderAttributeModifier::SetBlockBorderWidthImpl,
        SliderAttributeModifier::SetStepColorImpl,
        SliderAttributeModifier::SetTrackBorderRadiusImpl,
        SliderAttributeModifier::SetSelectedBorderRadiusImpl,
        SliderAttributeModifier::SetBlockSizeImpl,
        SliderAttributeModifier::SetBlockStyleImpl,
        SliderAttributeModifier::SetStepSizeImpl,
        SliderAttributeModifier::SetSliderInteractionModeImpl,
        SliderAttributeModifier::SetMinResponsiveDistanceImpl,
        SliderAttributeModifier::SetSlideRangeImpl,
        SliderAttributeModifier::SetDigitalCrownSensitivityImpl,
        SliderAttributeModifier::SetEnableHapticFeedbackImpl,
        SliderAttributeModifier::SetShowTipsImpl,
    };
    return &ArkUISliderModifierImpl;
}

}

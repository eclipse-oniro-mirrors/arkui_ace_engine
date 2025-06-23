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

#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/native/generated/interface/node_api.h"
#include "core/components_ng/pattern/rating/rating_model_ng.h"
#include "core/components_ng/pattern/rating/rating_model_static.h"
#include "core/interfaces/native/utility/validators.h"

namespace OHOS::Ace::NG {
namespace {
struct StarStyleOptions {
    std::string backgroundUri = {};
    std::string foregroundUri = {};
    std::string secondaryUri = {};
};
}

namespace Converter {
template<>
StarStyleOptions Convert(const Ark_StarStyleOptions& value)
{
    StarStyleOptions options;
    options.backgroundUri = Converter::OptConvert<std::string>(value.backgroundUri).value_or(std::string());
    options.foregroundUri = Converter::OptConvert<std::string>(value.foregroundUri).value_or(std::string());
    options.secondaryUri = Converter::OptConvert<std::string>(value.secondaryUri).value_or(std::string());
    return options;
}
} // namespace Converter
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RatingAttributeModifier {
    void Stars1Impl(Ark_NativePointer node, const Opt_Number* value);
    void StepSize1Impl(Ark_NativePointer node, const Opt_Number* value);
    void StarStyle1Impl(Ark_NativePointer node, const Opt_StarStyleOptions* value);
}
namespace RatingModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = RatingModelNG::CreateFrameNode(id);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // RatingModifier
namespace RatingInterfaceModifier {
void SetRatingOptionsImpl(Ark_NativePointer node,
                          const Opt_RatingOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optOptions = Converter::GetOptPtr(options);
    auto rating = optOptions ? Converter::OptConvert<float>(optOptions->rating) : std::nullopt;
    auto indicator = optOptions ? Converter::OptConvert<bool>(optOptions->indicator) : std::nullopt;
    Validator::ValidateNonNegative(rating);
    auto dRating = FloatToDouble(rating);
    RatingModelStatic::SetRatingOptions(frameNode, dRating, indicator);
}
} // RatingInterfaceModifier
namespace RatingAttributeModifier {
void StarsImpl(Ark_NativePointer node,
               const Opt_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convVal = Converter::OptConvertPtr<float>(value);
    Validator::ValidateNonNegative(convVal);
    auto optdVal = FloatToDouble(convVal);
    RatingModelStatic::SetStars(frameNode,  optdVal);
}
void StepSizeImpl(Ark_NativePointer node,
                  const Opt_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convVal = Converter::OptConvertPtr<float>(value);
    static const float stepSizeMin = 0.1;
    Validator::ValidateGreatOrEqual(convVal, stepSizeMin);
    auto optdVal = FloatToDouble(convVal);
    RatingModelStatic::SetStepSize(frameNode,  optdVal);
}
void StarStyleImpl(Ark_NativePointer node,
                   const Opt_StarStyleOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto options = Converter::OptConvertPtr<StarStyleOptions>(value).value_or(StarStyleOptions());
    RatingModelNG::SetBackgroundSrc(frameNode, options.backgroundUri,  options.backgroundUri.empty());
    RatingModelNG::SetForegroundSrc(frameNode, options.foregroundUri, options.foregroundUri.empty());
    RatingModelNG::SetSecondarySrc(frameNode, options.secondaryUri, options.secondaryUri.empty());
}
void OnChangeImpl(Ark_NativePointer node,
                  const Opt_OnRatingChangeCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    RatingChangeEvent onChange = {};
    if (optValue) {
        onChange = [arkCallback = CallbackHelper(*optValue)](const std::string& value) {
            Ark_Number convValue = Converter::ArkValue<Ark_Number>(std::stof(value));
            arkCallback.Invoke(convValue);
        };
    }
    RatingModelStatic::SetOnChange(frameNode, std::move(onChange));
}
void ContentModifierImpl(Ark_NativePointer node,
                         const Opt_ContentModifier* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::OptConvertPtr<type>(value);
    //RatingModelNG::SetContentModifier(frameNode, convValue);
    LOGE("ARKOALA RatingInterfaceModifier::ContentModifier is not implemented.");
}
#ifdef WRONG_GEN
void _onChangeEvent_ratingImpl(Ark_NativePointer node,
                               const Callback_Number_Void* callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(callback);
    WeakPtr<FrameNode> weakNode = AceType::WeakClaim(frameNode);
    auto onEvent = [arkCallback = CallbackHelper(*callback), weakNode](const std::string& value) {
        Ark_Number nValue = Converter::ArkValue<Ark_Number>(std::stof(value));
        PipelineContext::SetCallBackNode(weakNode);
        arkCallback.Invoke(nValue);
    };
    RatingModelStatic::SetOnChangeEvent(frameNode, std::move(onEvent));
}
#endif
} // RatingAttributeModifier
const GENERATED_ArkUIRatingModifier* GetRatingModifier()
{
    static const GENERATED_ArkUIRatingModifier ArkUIRatingModifierImpl {
        RatingModifier::ConstructImpl,
        RatingInterfaceModifier::SetRatingOptionsImpl,
        RatingAttributeModifier::StarsImpl,
        RatingAttributeModifier::StepSizeImpl,
        RatingAttributeModifier::StarStyleImpl,
        RatingAttributeModifier::OnChangeImpl,
        RatingAttributeModifier::ContentModifierImpl,
    };
    return &ArkUIRatingModifierImpl;
}

}

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
#include "core/interfaces/native/utility/converter.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_model_ng.h"
#include "core/interfaces/native/generated/interface/node_api.h"
#include "frameworks/core/components/common/layout/constants.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/validators.h"
#include "core/interfaces/native/utility/ace_engine_types.h"
#include "core/components_ng/base/view_abstract_model_ng.h"

namespace OHOS::Ace::NG {
namespace {
// similar as in the js_image.cpp
constexpr float CEIL_SMOOTHEDGE_VALUE = 1.333f;
constexpr float FLOOR_SMOOTHEDGE_VALUE = 0.334f;
} // namespace

namespace Converter {
template<>
Ark_ImageError ArkValue(const LoadImageFailEvent& event)
{
    Ark_ImageError arkEvent;
    arkEvent.componentWidth = Converter::ArkValue<Ark_Number>(event.GetComponentWidth());
    arkEvent.componentHeight = Converter::ArkValue<Ark_Number>(event.GetComponentHeight());
    arkEvent.message = Converter::ArkValue<Ark_String>(event.GetErrorMessage());
    return arkEvent;
}

template<>
void AssignCast(std::optional<std::pair<CalcDimension, CalcDimension>>& dst,
    const Ark_ImageSourceSize& src)
{
    auto width = Converter::OptConvert<float>(src.width);
    auto height = Converter::OptConvert<float>(src.height);
    Validator::ValidateNonNegative(width);
    Validator::ValidateNonNegative(height);
    if (width && height) {
        CalcDimension calcWidth(*width, DimensionUnit::VP);
        CalcDimension calcHeight(*height, DimensionUnit::VP);
        dst = std::make_pair(calcWidth, calcHeight);
    }
}
} // Converter
} // OHOS::Ace::NG

namespace OHOS::Ace::NG::GeneratedModifier {
namespace ImageInterfaceModifier {
void SetImageOptions0Impl(Ark_NativePointer node,
                          const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor* src)
{
    CHECK_NULL_VOID(src);
    auto info = Converter::OptConvert<ImageSourceInfo>(*src);
    if (info) {
        auto frameNode = reinterpret_cast<FrameNode*>(node);
        CHECK_NULL_VOID(frameNode);
        ImageModelNG::InitImage(frameNode, info->GetSrc());
    }
}
void SetImageOptions1Impl(Ark_NativePointer node,
                          const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor_ImageContent* src)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(src);
    //auto convValue = Converter::OptConvert<type_name>(*src);
    //ImageModelNG::SetSetImageOptions1(frameNode, convValue);
    LOGE("Arkoala: Image.SetImageOptions1Impl - method not implemented");
}
void SetImageOptions2Impl(Ark_NativePointer node,
                          const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor* src,
                          const Ark_ImageAIOptions* imageAIOptions)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(src);
    //auto convValue = Converter::OptConvert<type>(src); // for enums
    //ImageModelNG::SetSetImageOptions2(frameNode, convValue);
    LOGE("Arkoala: Image.SetImageOptions2Impl - method not implemented");
}
} // ImageInterfaceModifier
namespace ImageAttributeModifier {
void AltImpl(Ark_NativePointer node,
             const Ark_Union_String_Resource_PixelMap* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto info = Converter::OptConvert<ImageSourceInfo>(*value);
    LOGE("Arkoala: GENERATED_ArkUIImageModifier.AltImpl - method doesn't support PixelMap");
    ImageModelNG::SetAlt(frameNode, info);
}
void MatchTextDirectionImpl(Ark_NativePointer node,
                            Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetMatchTextDirection(frameNode, Converter::Convert<bool>(value));
}
void FitOriginalSizeImpl(Ark_NativePointer node,
                         Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetFitOriginSize(frameNode, Converter::Convert<bool>(value));
}
void FillColorImpl(Ark_NativePointer node,
                   const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    ImageModelNG::SetImageFill(frameNode, Converter::OptConvert<Color>(*value));
}
void ObjectFitImpl(Ark_NativePointer node,
                   Ark_ImageFit value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto fit = Converter::OptConvert<ImageFit>(value);
    ImageModelNG::SetImageFit(frameNode, fit);
}
void ObjectRepeatImpl(Ark_NativePointer node,
                      Ark_ImageRepeat value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageRepeat(frameNode, Converter::OptConvert<ImageRepeat>(value));
}
void AutoResizeImpl(Ark_NativePointer node,
                    Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetAutoResize(frameNode, Converter::Convert<bool>(value));
}
void RenderModeImpl(Ark_NativePointer node,
                    Ark_ImageRenderMode value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageRenderMode(frameNode, Converter::OptConvert<ImageRenderMode>(value));
}
void DynamicRangeModeImpl(Ark_NativePointer node,
                          Ark_DynamicRangeMode value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetDynamicRangeMode(frameNode, Converter::OptConvert<DynamicRangeMode>(value));
}
void InterpolationImpl(Ark_NativePointer node,
                       Ark_ImageInterpolation value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageInterpolation(frameNode, Converter::OptConvert<ImageInterpolation>(value));
}
void SourceSizeImpl(Ark_NativePointer node,
                    const Ark_ImageSourceSize* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto sourceSize = Converter::OptConvert<std::pair<CalcDimension, CalcDimension>>(*value);
    ImageModelNG::SetImageSourceSize(frameNode, sourceSize);
}
void SyncLoadImpl(Ark_NativePointer node,
                  Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSyncMode(frameNode, Converter::Convert<bool>(value));
}
void ColorFilterImpl(Ark_NativePointer node,
                     const Ark_Union_ColorFilter_DrawingColorFilter* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    LOGE("Arkoala: Image.ColorFilterImpl - method not implemented");
}
void CopyOptionImpl(Ark_NativePointer node,
                    Ark_CopyOptions value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetCopyOption(frameNode, Converter::OptConvert<CopyOptions>(value));
}
void DraggableImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetDraggable(frameNode, Converter::Convert<bool>(value));
}
void PointLightImpl(Ark_NativePointer node,
                    const Ark_PointLightStyle* value)
{
#ifdef POINT_LIGHT_ENABLE
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto pointLightStyle = Converter::OptConvert<Converter::PointLightStyle>(*value);
    auto uiNode = reinterpret_cast<ArkUINodeHandle>(node);
    auto themeConstants = NodeModifier::GetThemeConstants(uiNode, "", "");
    CHECK_NULL_VOID(themeConstants);
    if (pointLightStyle) {
        if (pointLightStyle->lightSource) {
            ViewAbstractModelNG::SetLightPosition(frameNode, pointLightStyle->lightSource->x,
                pointLightStyle->lightSource->y,
                pointLightStyle->lightSource->z);
            ViewAbstractModelNG::SetLightIntensity(frameNode,
                pointLightStyle->lightSource->intensity);
            ViewAbstractModelNG::SetLightColor(frameNode, pointLightStyle->lightSource->lightColor);
        } else {
            ViewAbstractModelNG::SetLightPosition(frameNode, std::nullopt, std::nullopt, std::nullopt);
            ViewAbstractModelNG::SetLightIntensity(frameNode, std::nullopt);
            ViewAbstractModelNG::SetLightColor(frameNode, std::nullopt);
        }
        // illuminated
        ViewAbstractModelNG::SetLightIlluminated(frameNode, pointLightStyle->illuminationType, themeConstants);
        // bloom
        ViewAbstractModelNG::SetBloom(frameNode, pointLightStyle->bloom, themeConstants);
    } else {
        ViewAbstractModelNG::SetLightPosition(frameNode, std::nullopt, std::nullopt, std::nullopt);
        ViewAbstractModelNG::SetLightIntensity(frameNode, std::nullopt);
        ViewAbstractModelNG::SetLightColor(frameNode, std::nullopt);
        ViewAbstractModelNG::SetLightIlluminated(frameNode, std::nullopt, themeConstants);
        ViewAbstractModelNG::SetBloom(frameNode, std::nullopt, themeConstants);
    }
#endif
}
void EdgeAntialiasingImpl(Ark_NativePointer node,
                          const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<float>(*value);
    Validator::ValidateByRange(convValue, FLOOR_SMOOTHEDGE_VALUE, CEIL_SMOOTHEDGE_VALUE);
    ImageModelNG::SetSmoothEdge(frameNode, convValue);
}
void OnCompleteImpl(Ark_NativePointer node,
                    const Callback_Type_ImageAttribute_onComplete_callback_event_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onEvent = [frameNode](const LoadImageSuccessEvent& info) {
        Ark_Type_ImageAttribute_onComplete_callback_event event;
        event.width = Converter::ArkValue<Ark_Number>(info.GetWidth());
        event.height = Converter::ArkValue<Ark_Number>(info.GetHeight());
        event.componentWidth = Converter::ArkValue<Ark_Number>(info.GetComponentWidth());
        event.componentHeight = Converter::ArkValue<Ark_Number>(info.GetComponentHeight());
        event.loadingStatus = Converter::ArkValue<Ark_Number>(info.GetLoadingStatus());
        event.contentOffsetX = Converter::ArkValue<Ark_Number>(info.GetContentOffsetX());
        event.contentOffsetY = Converter::ArkValue<Ark_Number>(info.GetContentOffsetY());
        event.contentWidth = Converter::ArkValue<Ark_Number>(info.GetContentWidth());
        event.contentHeight = Converter::ArkValue<Ark_Number>(info.GetContentHeight());
        auto optEvent = Converter::ArkValue<Opt_Type_ImageAttribute_onComplete_callback_event>(event);
        GetFullAPI()->getEventsAPI()->getImageEventsReceiver()->onComplete(frameNode->GetId(), optEvent);
    };
    ImageModelNG::SetOnComplete(frameNode, std::move(onEvent));
}
void OnErrorImpl(Ark_NativePointer node,
                 const ImageErrorCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onEvent = [frameNode](const LoadImageFailEvent& info) {
        auto arkInfo = Converter::ArkValue<Ark_ImageError>(info);
        GetFullAPI()->getEventsAPI()->getImageEventsReceiver()->onError(frameNode->GetId(), arkInfo);
    };
    ImageModelNG::SetOnError(frameNode, std::move(onEvent));
}
void OnFinishImpl(Ark_NativePointer node,
                  const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getImageEventsReceiver()->onFinish(frameNode->GetId());
    };
    ImageModelNG::SetOnSvgPlayFinish(frameNode, std::move(onEvent));
}
void EnableAnalyzerImpl(Ark_NativePointer node,
                        Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::EnableAnalyzer(frameNode, Converter::Convert<bool>(value));
}
void AnalyzerConfigImpl(Ark_NativePointer node,
                        const Ark_ImageAnalyzerConfig* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    ImageModelNG::SetImageAnalyzerConfig(frameNode, reinterpret_cast<void*>(value->types.array));
}
void ResizableImpl(Ark_NativePointer node,
                   const Ark_ResizableOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    LOGE("Arkoala: Image.ResizableImpl - method not implemented");
}
void PrivacySensitiveImpl(Ark_NativePointer node,
                          Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetPrivacySensitive(frameNode, Converter::Convert<bool>(value));
}
void EnhancedImageQualityImpl(Ark_NativePointer node,
                              const Ark_CustomObject* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    LOGE("Arkoala: Image.EnhancedImageQualityImpl - method not implemented");
}
} // ImageAttributeModifier
const GENERATED_ArkUIImageModifier* GetImageModifier()
{
    static const GENERATED_ArkUIImageModifier ArkUIImageModifierImpl {
        ImageInterfaceModifier::SetImageOptions0Impl,
        ImageInterfaceModifier::SetImageOptions1Impl,
        ImageInterfaceModifier::SetImageOptions2Impl,
        ImageAttributeModifier::AltImpl,
        ImageAttributeModifier::MatchTextDirectionImpl,
        ImageAttributeModifier::FitOriginalSizeImpl,
        ImageAttributeModifier::FillColorImpl,
        ImageAttributeModifier::ObjectFitImpl,
        ImageAttributeModifier::ObjectRepeatImpl,
        ImageAttributeModifier::AutoResizeImpl,
        ImageAttributeModifier::RenderModeImpl,
        ImageAttributeModifier::DynamicRangeModeImpl,
        ImageAttributeModifier::InterpolationImpl,
        ImageAttributeModifier::SourceSizeImpl,
        ImageAttributeModifier::SyncLoadImpl,
        ImageAttributeModifier::ColorFilterImpl,
        ImageAttributeModifier::CopyOptionImpl,
        ImageAttributeModifier::DraggableImpl,
        ImageAttributeModifier::PointLightImpl,
        ImageAttributeModifier::EdgeAntialiasingImpl,
        ImageAttributeModifier::OnCompleteImpl,
        ImageAttributeModifier::OnErrorImpl,
        ImageAttributeModifier::OnFinishImpl,
        ImageAttributeModifier::EnableAnalyzerImpl,
        ImageAttributeModifier::AnalyzerConfigImpl,
        ImageAttributeModifier::ResizableImpl,
        ImageAttributeModifier::PrivacySensitiveImpl,
        ImageAttributeModifier::EnhancedImageQualityImpl,
    };
    return &ArkUIImageModifierImpl;
}

}
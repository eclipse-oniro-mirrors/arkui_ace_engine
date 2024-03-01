/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "core/interfaces/native/node/node_image_modifier.h"

#include <cstdint>

#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/image/image_component.h"
#include "core/components/image/image_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/image/image_model_ng.h"
#include "core/pipeline/base/element_register.h"
#include "frameworks/core/components/common/layout/constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr CopyOptions DEFAULT_IMAGE_COPYOPTION = CopyOptions::None;
constexpr bool DEFAULT_IMAGE_AUTORESIZE = true;
constexpr bool DEFAULT_SYNC_LOAD_VALUE = false;
constexpr ImageFit DEFAULT_OBJECT_FIT_VALUE = ImageFit::COVER;
constexpr bool DEFAULT_FIT_ORIGINAL_SIZE = false;
constexpr ImageInterpolation DEFAULT_IMAGE_INTERPOLATION = ImageInterpolation::NONE;
constexpr bool DEFAULT_DRAGGABLE = false;
constexpr ArkUI_Float32 DEFAULT_IMAGE_EDGE_ANTIALIASING = 0;
const std::vector<float> DEFAULT_COLOR_FILTER = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 };
constexpr int32_t LOAD_ERROR_CODE = 401;
constexpr int32_t IMAGE_LOAD_STATUS_INDEX = 0;
constexpr int32_t IMAGE_WIDTH_INDEX = 1;
constexpr int32_t IMAGE_HEIGHT_INDEX = 2;
constexpr int32_t IMAGE_COMPONENT_WIDTH_INDEX = 3;
constexpr int32_t IMAGE_COMPONENT_HEIGHT_INDEX = 4;
constexpr int32_t IMAGE_CONTENT_OFFSET_X_INDEX = 5;
constexpr int32_t IMAGE_CONTENT_OFFSET_Y_INDEX = 6;
constexpr int32_t IMAGE_CONTENT_WIDTH_INDEX = 7;
constexpr int32_t IMAGE_CONTENT_HEIGHT_INDEX = 8;
constexpr int32_t IMAGE_OBJECT_FIT_CONTAIN_INDEX = 0;
constexpr int32_t IMAGE_OBJECT_FIT_COVER_INDEX = 1;
constexpr int32_t IMAGE_OBJECT_FIT_AUTO_INDEX = 2;
constexpr int32_t IMAGE_OBJECT_FIT_FILL_INDEX = 3;
constexpr int32_t IMAGE_OBJECT_FIT_SCALE_DOWN_INDEX = 4;
constexpr int32_t IMAGE_OBJECT_FIT_NONE_INDEX = 5;
std::string g_strValue;

void SetImageSrc(ArkUINodeHandle node, const char* value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    std::string src(value);
    ImageModelNG::InitImage(frameNode, src);
}

const char* GetImageSrc(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    g_strValue = ImageModelNG::GetSrc(frameNode).GetSrc();
    return g_strValue.c_str();
}

void SetCopyOption(ArkUINodeHandle node, ArkUI_Int32 copyOption)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto copyOptions = static_cast<CopyOptions>(copyOption);
    if (copyOptions < CopyOptions::None || copyOptions > CopyOptions::Distributed) {
        copyOptions = DEFAULT_IMAGE_COPYOPTION;
    }
    ImageModelNG::SetCopyOption(frameNode, copyOptions);
}

void ResetCopyOption(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetCopyOption(frameNode, DEFAULT_IMAGE_COPYOPTION);
}

void SetAutoResize(ArkUINodeHandle node, ArkUI_Bool autoResize)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetAutoResize(frameNode, autoResize);
}

int32_t GetAutoResize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, DEFAULT_IMAGE_AUTORESIZE);
    return ImageModelNG::GetAutoResize(frameNode);
}

void ResetAutoResize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetAutoResize(frameNode, DEFAULT_IMAGE_AUTORESIZE);
}

void SetObjectRepeat(ArkUINodeHandle node, ArkUI_Int32 imageRepeat)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto repeat = static_cast<ImageRepeat>(imageRepeat);
    if (repeat < ImageRepeat::NO_REPEAT || repeat > ImageRepeat::REPEAT) {
        repeat = ImageRepeat::NO_REPEAT;
    }
    ImageModelNG::SetImageRepeat(frameNode, repeat);
}

int32_t GetObjectRepeat(ArkUINodeHandle node)
{
    int32_t defaultObjectRepeat = static_cast<int32_t>(ImageRepeat::NO_REPEAT);
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, defaultObjectRepeat);
    return static_cast<int32_t>(ImageModelNG::GetObjectRepeat(frameNode));
}

void ResetObjectRepeat(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageRepeat(frameNode, ImageRepeat::NO_REPEAT);
}

void SetRenderMode(ArkUINodeHandle node, ArkUI_Int32 imageRenderMode)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto renderMode = static_cast<ImageRenderMode>(imageRenderMode);
    if (renderMode < ImageRenderMode::ORIGINAL || renderMode > ImageRenderMode::TEMPLATE) {
        renderMode = ImageRenderMode::ORIGINAL;
    }
    ImageModelNG::SetImageRenderMode(frameNode, renderMode);
}

void ResetRenderMode(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageRenderMode(frameNode, ImageRenderMode::ORIGINAL);
}

void SetSyncLoad(ArkUINodeHandle node, ArkUI_Bool syncLoadValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSyncMode(frameNode, syncLoadValue);
}

void ResetSyncLoad(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSyncMode(frameNode, DEFAULT_SYNC_LOAD_VALUE);
}

int32_t GetObjectFit(int32_t originObjectFilt)
{
    switch (originObjectFilt) {
        case IMAGE_OBJECT_FIT_CONTAIN_INDEX:
            return static_cast<int32_t>(ImageFit::CONTAIN);
        case IMAGE_OBJECT_FIT_COVER_INDEX:
            return static_cast<int32_t>(ImageFit::COVER);
        case IMAGE_OBJECT_FIT_AUTO_INDEX:
            return static_cast<int32_t>(ImageFit::FITWIDTH);
        case IMAGE_OBJECT_FIT_FILL_INDEX:
            return static_cast<int32_t>(ImageFit::FILL);
        case IMAGE_OBJECT_FIT_SCALE_DOWN_INDEX:
            return static_cast<int32_t>(ImageFit::SCALE_DOWN);
        case IMAGE_OBJECT_FIT_NONE_INDEX:
            return static_cast<int32_t>(ImageFit::NONE);
        default:
            break;
    }
    return originObjectFilt;
}

void SetObjectFit(ArkUINodeHandle node, ArkUI_Int32 objectFitNumber)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageFit objectFitValue = static_cast<ImageFit>(GetObjectFit(objectFitNumber));
    if (objectFitValue < ImageFit::FILL || objectFitValue > ImageFit::SCALE_DOWN) {
        objectFitValue = ImageFit::COVER;
    }
    ImageModelNG::SetImageFit(frameNode, objectFitValue);
}

int32_t GetObjectFit(ArkUINodeHandle node)
{
    int32_t defaultObjectFit = static_cast<int32_t>(ImageFit::COVER);
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, defaultObjectFit);
    return static_cast<int32_t>(ImageModelNG::GetObjectFit(frameNode));
}

void ResetObjectFit(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageFit(frameNode, DEFAULT_OBJECT_FIT_VALUE);
}

void SetFitOriginalSize(ArkUINodeHandle node, ArkUI_Bool fitOriginalSizeValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetFitOriginSize(frameNode, fitOriginalSizeValue);
}

void ResetFitOriginalSize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetFitOriginSize(frameNode, DEFAULT_FIT_ORIGINAL_SIZE);
}

void SetSourceSize(ArkUINodeHandle node, ArkUI_Float32 width, ArkUI_Float32 height)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension widthObj(width, DimensionUnit::VP);
    CalcDimension heightObj(height, DimensionUnit::VP);
    ImageModelNG::SetImageSourceSize(frameNode, std::pair<CalcDimension, CalcDimension>(widthObj, heightObj));
}

void ResetSourceSize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageSourceSize(frameNode, std::pair<CalcDimension, CalcDimension>());
}

void SetMatchTextDirection(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetMatchTextDirection(frameNode, static_cast<bool>(value));
}

void ResetMatchTextDirection(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetMatchTextDirection(frameNode, false);
}

void SetFillColor(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageFill(frameNode, Color(value));
}

void ResetFillColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto theme = pipelineContext->GetTheme<ImageTheme>();
    CHECK_NULL_VOID(theme);
    ImageModelNG::SetImageFill(frameNode, theme->GetFillColor());
}

void SetAlt(ArkUINodeHandle node, const char* src, const char* bundleName, const char* moduleName)
{
    if (ImageSourceInfo::ResolveURIType(src) == SrcType::NETWORK) {
        return;
    }

    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetAlt(frameNode, ImageSourceInfo { src, bundleName, moduleName });
}

const char* GetAlt(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    g_strValue = ImageModelNG::GetAlt(frameNode).GetSrc();
    return g_strValue.c_str();
}

void ResetAlt(ArkUINodeHandle node)
{
    return;
}

void SetImageInterpolation(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto interpolation = static_cast<Ace::ImageInterpolation>(value);
    if (interpolation < Ace::ImageInterpolation::NONE || interpolation > Ace::ImageInterpolation::HIGH) {
        interpolation = Ace::ImageInterpolation::NONE;
    }
    ImageModelNG::SetImageInterpolation(frameNode, interpolation);
}

int32_t GetImageInterpolation(ArkUINodeHandle node)
{
    int32_t defaultInterpolation = static_cast<int32_t>(ImageInterpolation::NONE);
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, defaultInterpolation);
    return static_cast<int32_t>(ImageModelNG::GetInterpolation(frameNode));
}

void ResetImageInterpolation(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageInterpolation(frameNode, DEFAULT_IMAGE_INTERPOLATION);
}

void SetColorFilter(ArkUINodeHandle node, const ArkUI_Float32* array, int length)
{
    CHECK_NULL_VOID(array);
    if (length != COLOR_FILTER_MATRIX_SIZE) {
        return;
    }
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetColorFilterMatrix(frameNode, std::vector<float>(array, array + length));
}

ArkUIFilterColorType GetColorFilter(ArkUINodeHandle node)
{
    ArkUIFilterColorType colorFilter = { nullptr, 0 };
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, colorFilter);
    auto filterFloatArray = ImageModelNG::GetColorFilter(frameNode);
    std::vector<int32_t> filterArray;
    for (size_t i = 0; i < filterFloatArray.size(); i++) {
        filterArray.emplace_back(static_cast<int32_t>(filterFloatArray[i]));
    }
    colorFilter.filterArray = filterArray.size() > 0 ? &filterArray[0] : nullptr;
    colorFilter.filterSize = filterArray.size();
    return colorFilter;
}

void ResetColorFilter(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetColorFilterMatrix(frameNode, DEFAULT_COLOR_FILTER);
}

void SetImageSyncLoad(ArkUINodeHandle node, ArkUI_Bool syncLoadValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSyncMode(frameNode, syncLoadValue);
}

void ResetImageSyncLoad(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSyncMode(frameNode, DEFAULT_SYNC_LOAD_VALUE);
}

void SetImageObjectFit(ArkUINodeHandle node, ArkUI_Int32 objectFitNumber)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageFit objectFitValue = static_cast<ImageFit>(objectFitNumber);
    ImageModelNG::SetImageFit(frameNode, objectFitValue);
}

void ResetImageObjectFit(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetImageFit(frameNode, DEFAULT_OBJECT_FIT_VALUE);
}

void SetImageFitOriginalSize(ArkUINodeHandle node, ArkUI_Bool fitOriginalSizeValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetFitOriginSize(frameNode, fitOriginalSizeValue);
}

void ResetImageFitOriginalSize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetFitOriginSize(frameNode, DEFAULT_FIT_ORIGINAL_SIZE);
}

void SetImageDraggable(ArkUINodeHandle node, ArkUI_Bool value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetDraggable(frameNode, value);
}

void ResetImageDraggable(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetDraggable(frameNode, DEFAULT_DRAGGABLE);
}

/**
 * @param values radius values
 * value[0] : radius value for TopLeft，value[1] : radius value for TopRight
 * value[2] : radius value for BottomLeft，value[3] : radius value for BottomRight
 * @param units radius units
 * units[0]: radius unit for TopLeft ,units[1] : radius unit for TopRight
 * units[2]: radius unit for BottomLeft, units[3] : radius unit for TopRight
 */
void SetImageBorderRadius(ArkUINodeHandle node, const ArkUI_Float32* values, const int* units, ArkUI_Int32 length) {}

void ResetImageBorderRadius(ArkUINodeHandle node) {}

void SetImageBorder(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetBackBorder(frameNode);
}

void ResetImageBorder(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetBackBorder(frameNode);
}

void SetImageOpacity(ArkUINodeHandle node, ArkUI_Float32 opacity)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if ((LessNotEqual(opacity, 0.0)) || opacity > 1) {
        opacity = 1.0f;
    }
    ViewAbstract::SetOpacity(frameNode, opacity);
}

void ResetImageOpacity(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetOpacity(frameNode, 1.0f);
}

void SetEdgeAntialiasing(ArkUINodeHandle node, ArkUI_Float32 edgeAntialiasing)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSmoothEdge(frameNode, edgeAntialiasing);
}

void ResetEdgeAntialiasing(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageModelNG::SetSmoothEdge(frameNode, DEFAULT_IMAGE_EDGE_ANTIALIASING);
}
} // namespace

namespace NodeModifier {
const ArkUIImageModifier* GetImageModifier()
{
    static const ArkUIImageModifier modifier = { SetImageSrc, SetCopyOption, ResetCopyOption, SetAutoResize,
        ResetAutoResize, SetObjectRepeat, ResetObjectRepeat, SetRenderMode, ResetRenderMode, SetSyncLoad, ResetSyncLoad,
        SetObjectFit, ResetObjectFit, SetFitOriginalSize, ResetFitOriginalSize, SetSourceSize, ResetSourceSize,
        SetMatchTextDirection, ResetMatchTextDirection, SetFillColor, ResetFillColor, SetAlt, ResetAlt,
        SetImageInterpolation, ResetImageInterpolation, SetColorFilter, ResetColorFilter, SetImageSyncLoad,
        ResetImageSyncLoad, SetImageObjectFit, ResetImageObjectFit, SetImageFitOriginalSize, ResetImageFitOriginalSize,
        SetImageDraggable, ResetImageDraggable, SetImageBorderRadius, ResetImageBorderRadius, SetImageBorder,
        ResetImageBorder, SetImageOpacity, ResetImageOpacity, SetEdgeAntialiasing, ResetEdgeAntialiasing, GetImageSrc,
        GetAutoResize, GetObjectRepeat, GetObjectFit, GetImageInterpolation, GetColorFilter, GetAlt };
    return &modifier;
}

void SetImageOnComplete(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onEvent = [node, eventId, extraParam](const LoadImageSuccessEvent& info) {
        ArkUINodeEvent event;
        event.kind = ON_IMAGE_COMPLETE;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.data[IMAGE_LOAD_STATUS_INDEX].i32 = info.GetLoadingStatus();
        event.componentAsyncEvent.data[IMAGE_WIDTH_INDEX].f32 = info.GetWidth();
        event.componentAsyncEvent.data[IMAGE_HEIGHT_INDEX].f32 = info.GetHeight();
        event.componentAsyncEvent.data[IMAGE_COMPONENT_WIDTH_INDEX].f32 = info.GetComponentWidth();
        event.componentAsyncEvent.data[IMAGE_COMPONENT_HEIGHT_INDEX].f32 = info.GetComponentHeight();
        event.componentAsyncEvent.data[IMAGE_CONTENT_OFFSET_X_INDEX].f32 = info.GetContentOffsetX();
        event.componentAsyncEvent.data[IMAGE_CONTENT_OFFSET_Y_INDEX].f32 = info.GetContentOffsetY();
        event.componentAsyncEvent.data[IMAGE_CONTENT_WIDTH_INDEX].f32 = info.GetContentWidth();
        event.componentAsyncEvent.data[IMAGE_CONTENT_HEIGHT_INDEX].f32 = info.GetContentHeight();
        SendArkUIAsyncEvent(&event);
    };
    ImageModelNG::SetOnComplete(frameNode, std::move(onEvent));
}

void SetImageOnError(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onEvent = [node, eventId, extraParam](const LoadImageFailEvent& info) {
        ArkUINodeEvent event;
        event.kind = ON_IMAGE_ERROR;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.data[0].i32 = LOAD_ERROR_CODE;
        SendArkUIAsyncEvent(&event);
    };
    ImageModelNG::SetOnError(frameNode, std::move(onEvent));
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG
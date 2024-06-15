/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "core/common/ai/image_analyzer_manager.h"

#include "interfaces/inner_api/ace/ai/image_analyzer.h"
#include "js_native_api_types.h"

#include "base/geometry/offset.h"
#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/common/ai/image_analyzer_adapter.h"
#include "core/common/ai/image_analyzer_mgr.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/image/image_render_property.h"
#include "core/components_ng/property/measure_property.h"

namespace OHOS::Ace {

ImageAnalyzerManager::ImageAnalyzerManager(const RefPtr<NG::FrameNode>& frameNode, ImageAnalyzerHolder holder)
    : frameNode_(frameNode), holder_(holder)
{
    imageAnalyzerAdapter_ = std::shared_ptr<ImageAnalyzerAdapter>(CreateImageAnalyzerAdapter());
}

void ImageAnalyzerManager::CreateAnalyzerOverlay(const RefPtr<OHOS::Ace::PixelMap>& pixelMap,
    const NG::OffsetF& offset)
{
    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    void* pixelmapNapiVal = nullptr;
    if (pixelMap) {
        pixelmapNapiVal = imageAnalyzerAdapter_->ConvertPixmapNapi(pixelMap);
    }

    analyzerUIConfig_.holder = holder_;
    if (holder_ != ImageAnalyzerHolder::IMAGE && holder_ != ImageAnalyzerHolder::WEB) {
        analyzerUIConfig_.contentWidth = pixelMap->GetWidth();
        analyzerUIConfig_.contentHeight = pixelMap->GetHeight();
    }

    if (holder_ == ImageAnalyzerHolder::VIDEO_CUSTOM) {
        analyzerUIConfig_.pixelMapWidth = pixelMap->GetWidth();
        analyzerUIConfig_.pixelMapHeight = pixelMap->GetHeight();
        analyzerUIConfig_.overlayOffset = offset;
    }

    RefPtr<NG::UINode> customNode;
    {
        NG::ScopedViewStackProcessor builderViewStackProcessor;
        auto analyzerConfig = imageAnalyzerAdapter_->GetImageAnalyzerConfig();
        ImageAnalyzerMgr::GetInstance().BuildNodeFunc(
            pixelmapNapiVal, analyzerConfig, &analyzerUIConfig_, &overlayData_);
        customNode = NG::ViewStackProcessor::GetInstance()->Finish();
    }
    auto overlayNode = AceType::DynamicCast<NG::FrameNode>(customNode);
    CHECK_NULL_VOID(overlayNode);
    auto node = frameNode_.Upgrade();
    CHECK_NULL_VOID(node);
    node->SetOverlayNode(overlayNode);
    overlayNode->SetParent(AceType::WeakClaim(AceType::RawPtr(node)));
    overlayNode->SetActive(true);
    UpdateAnalyzerOverlayLayout();

    auto renderContext = overlayNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateZIndex(INT32_MAX);
    auto focusHub = overlayNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetFocusable(false);
    overlayNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);

    isAnalyzerOverlayBuild_ = true;
    CHECK_NULL_VOID(analyzerUIConfig_.onAnalyzed);
    (analyzerUIConfig_.onAnalyzed.value())(ImageAnalyzerState::FINISHED);
    analyzerUIConfig_.onAnalyzed = std::nullopt;
}

void ImageAnalyzerManager::UpdateAnalyzerOverlay(const RefPtr<OHOS::Ace::PixelMap>& pixelMap)
{
    if (!isAnalyzerOverlayBuild_) {
        return;
    }

    auto node = frameNode_.Upgrade();
    CHECK_NULL_VOID(node);
    if (holder_ == ImageAnalyzerHolder::IMAGE) {
        auto imagePattern = AceType::DynamicCast<NG::ImagePattern>(node->GetPattern());
        CHECK_NULL_VOID(imagePattern);
        if (!imagePattern->hasSceneChanged()) {
            return;
        }
    }

    CHECK_NULL_VOID(pixelMap);
    if (holder_ != ImageAnalyzerHolder::IMAGE) {
        analyzerUIConfig_.contentWidth = pixelMap->GetWidth();
        analyzerUIConfig_.contentHeight = pixelMap->GetHeight();
    }

    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    auto pixelmapNapiVal = imageAnalyzerAdapter_->ConvertPixmapNapi(pixelMap);
    auto overlayNode = node->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto analyzerConfig = imageAnalyzerAdapter_->GetImageAnalyzerConfig();
    ImageAnalyzerMgr::GetInstance().UpdateImage(&overlayData_, pixelmapNapiVal, analyzerConfig, &analyzerUIConfig_);
    overlayNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
}

void ImageAnalyzerManager::DestroyAnalyzerOverlay()
{
    if (!isAnalyzerOverlayBuild_) {
        return;
    }
    auto node = frameNode_.Upgrade();
    CHECK_NULL_VOID(node);
    auto overlayNode = node->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    node->SetOverlayNode(RefPtr<NG::FrameNode>());

    isAnalyzerOverlayBuild_ = false;
    CHECK_NULL_VOID(analyzerUIConfig_.onAnalyzed);
    (analyzerUIConfig_.onAnalyzed.value())(ImageAnalyzerState::STOPPED);
    analyzerUIConfig_.onAnalyzed = std::nullopt;

    napi_value nullValue = nullptr;
    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    imageAnalyzerAdapter_->SetImageAnalyzerConfig(nullValue);
}

bool ImageAnalyzerManager::IsSupportImageAnalyzerFeature()
{
    auto node = frameNode_.Upgrade();
    CHECK_NULL_RETURN(node, false);
    auto eventHub = node->GetEventHub<NG::EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (!eventHub->IsEnabled()) {
        return false;
    }

    bool hasObscured = false;
    if (node->GetRenderContext()->GetObscured().has_value()) {
        auto obscuredReasons = node->GetRenderContext()->GetObscured().value();
        hasObscured = std::any_of(obscuredReasons.begin(), obscuredReasons.end(),
            [](const auto& reason) { return reason == ObscuredReasons::PLACEHOLDER; });
        if (hasObscured) {
            return false;
        }
    }

    if (holder_ == ImageAnalyzerHolder::IMAGE) {
        auto imageRenderProperty = node->GetPaintProperty<NG::ImageRenderProperty>();
        CHECK_NULL_RETURN(imageRenderProperty, false);
        ImageRepeat repeat = imageRenderProperty->GetImageRepeat().value_or(ImageRepeat::NO_REPEAT);
        if (repeat != ImageRepeat::NO_REPEAT) {
            return false;
        }
    }

    return ImageAnalyzerMgr::GetInstance().IsImageAnalyzerSupported();
}

bool ImageAnalyzerManager::IsOverlayCreated()
{
    return isAnalyzerOverlayBuild_;
}

void ImageAnalyzerManager::UpdateAnalyzerOverlayLayout()
{
    auto node = frameNode_.Upgrade();
    CHECK_NULL_VOID(node);
    auto layoutProperty = node->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto overlayNode = node->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto overlayLayoutProperty = overlayNode->GetLayoutProperty();
    CHECK_NULL_VOID(overlayLayoutProperty);
    overlayLayoutProperty->UpdateMeasureType(NG::MeasureType::MATCH_PARENT);
    overlayLayoutProperty->UpdateAlignment(Alignment::TOP_LEFT);
    if (holder_ == ImageAnalyzerHolder::IMAGE || holder_ == ImageAnalyzerHolder::VIDEO_CUSTOM) {
        overlayLayoutProperty->SetOverlayOffset(Dimension(padding.Offset().GetX()),
                                                Dimension(padding.Offset().GetY()));
        if (holder_ == ImageAnalyzerHolder::IMAGE) {
            auto renderContext = overlayNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            renderContext->SetRenderFrameOffset({ -padding.Offset().GetX(), -padding.Offset().GetY() });
        }
    }
    overlayNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
}

void ImageAnalyzerManager::UpdateAnalyzerUIConfig(const RefPtr<NG::GeometryNode>& geometryNode)
{
    CHECK_NULL_VOID(geometryNode);
    auto node = frameNode_.Upgrade();
    CHECK_NULL_VOID(node);
    bool isUIConfigUpdate = false;

    auto layoutProps = node->GetLayoutProperty<NG::ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    if (holder_ == ImageAnalyzerHolder::IMAGE || holder_ == ImageAnalyzerHolder::VIDEO_CUSTOM) {
        if (analyzerUIConfig_.imageFit != layoutProps->GetImageFit().value_or(ImageFit::COVER)) {
            analyzerUIConfig_.imageFit = layoutProps->GetImageFit().value_or(ImageFit::COVER);
            isUIConfigUpdate = true;
        }
    }

    if (holder_ != ImageAnalyzerHolder::VIDEO_CUSTOM) {
        auto padding = layoutProps->CreatePaddingAndBorder();
        float paddingWidth = holder_ == ImageAnalyzerHolder::IMAGE ? padding.left.value_or(0) +
                                                                     padding.right.value_or(0) : 0.0f;
        float paddingHeight = holder_ == ImageAnalyzerHolder::IMAGE ? padding.top.value_or(0) +
                                                                      padding.bottom.value_or(0) : 0.0f;
        NG::SizeF frameSize = geometryNode->GetFrameSize();
        if (analyzerUIConfig_.contentWidth != frameSize.Width() - paddingWidth ||
            analyzerUIConfig_.contentHeight != frameSize.Height() - paddingHeight) {
            analyzerUIConfig_.contentWidth = frameSize.Width() - paddingWidth;
            analyzerUIConfig_.contentHeight = frameSize.Height()- paddingHeight;
            isUIConfigUpdate = true;
        }
    }

    auto renderContext = node->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto transformMat = renderContext->GetTransformMatrixValue(Matrix4::CreateIdentity());
    if (!(analyzerUIConfig_.transformMat == transformMat)) {
        analyzerUIConfig_.transformMat = transformMat;
        isUIConfigUpdate = true;
    }

    if (isUIConfigUpdate) {
        ImageAnalyzerMgr::GetInstance().UpdateInnerConfig(&overlayData_, &analyzerUIConfig_);
    }
}

void ImageAnalyzerManager::SetImageAnalyzerConfig(void* config)
{
    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    bool hasConfig = imageAnalyzerAdapter_->HasImageAnalyzerConfig();
    if (hasConfig) {
        return;
    }
    imageAnalyzerAdapter_->SetImageAnalyzerConfig(config);
    auto analyzerConfig = imageAnalyzerAdapter_->GetImageAnalyzerConfig();
    if (isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().UpdateConfig(&overlayData_, analyzerConfig);
    }
}

void ImageAnalyzerManager::SetImageAIOptions(void* options)
{
    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    imageAnalyzerAdapter_->SetImageAnalyzerConfig(options, true);
    auto analyzerConfig = imageAnalyzerAdapter_->GetImageAnalyzerConfig();
    if (isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().UpdateConfig(&overlayData_, analyzerConfig);
    }
}

void ImageAnalyzerManager::SetImageAnalyzerCallback(OnAnalyzedCallback& callback)
{
    analyzerUIConfig_.onAnalyzed = callback;
}

void ImageAnalyzerManager::ReleaseImageAnalyzer()
{
    if (isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().Release(&overlayData_);
    }
}

void ImageAnalyzerManager::UpdatePressOverlay(
    const RefPtr<OHOS::Ace::PixelMap>& pixelMap,
    int offsetX,
    int offsetY,
    int rectWidth,
    int rectHeight,
    int pointX,
    int pointY,
    OnTextSelectedCallback callback)
{
    analyzerUIConfig_.overlayOffset.SetX(offsetX);
    analyzerUIConfig_.overlayOffset.SetY(offsetY);
    analyzerUIConfig_.touchInfo.touchPoint.x = pointX;
    analyzerUIConfig_.touchInfo.touchPoint.y = pointY;
    analyzerUIConfig_.touchInfo.touchType = TouchType::DOWN;
    analyzerUIConfig_.selectedStatus = Status::SELECTED;
    analyzerUIConfig_.menuStatus = Status::MENU_SHOW;
    if (!analyzerUIConfig_.onTextSelected) {
        analyzerUIConfig_.onTextSelected = std::move(callback);
    }
    if (pixelMap && imageAnalyzerAdapter_) {
        analyzerUIConfig_.contentWidth = rectWidth;
        analyzerUIConfig_.contentHeight = rectHeight;
        analyzerUIConfig_.pixelMapWidth = pixelMap->GetWidth();
        analyzerUIConfig_.pixelMapHeight = pixelMap->GetHeight();
        analyzerUIConfig_.pixelmapNapiVal =
            imageAnalyzerAdapter_->ConvertPixmapNapi(pixelMap);
    }
    ImageAnalyzerMgr::GetInstance().UpdatePressOverlay(&overlayData_, &analyzerUIConfig_);
    analyzerUIConfig_.pixelmapNapiVal = nullptr;
}

void ImageAnalyzerManager::UpdateOverlayTouchInfo(
    int touchPointX,
    int touchPointY,
    TouchType touchType)
{
    analyzerUIConfig_.touchInfo.touchPoint.x = touchPointX - analyzerUIConfig_.overlayOffset.GetX();
    analyzerUIConfig_.touchInfo.touchPoint.y = touchPointY - analyzerUIConfig_.overlayOffset.GetY();
    analyzerUIConfig_.touchInfo.touchType = touchType;
    ImageAnalyzerMgr::GetInstance().UpdatePressOverlay(&overlayData_, &analyzerUIConfig_);
}

void ImageAnalyzerManager::UpdateOverlayStatus(
    bool status,
    int offsetX,
    int offsetY,
    int rectWidth,
    int rectHeight)
{
    if (status) {
        analyzerUIConfig_.overlayOffset.SetX(offsetX);
        analyzerUIConfig_.overlayOffset.SetY(offsetY);
        analyzerUIConfig_.contentWidth = rectWidth;
        analyzerUIConfig_.contentHeight = rectHeight;
        analyzerUIConfig_.selectedStatus = Status::SELECTED;
        analyzerUIConfig_.menuStatus = Status::MENU_SHOW;
    } else {
        analyzerUIConfig_.selectedStatus = Status::UNSELECTED;
        analyzerUIConfig_.menuStatus = Status::MENU_HIDE;
    }
    ImageAnalyzerMgr::GetInstance().UpdateOverlayStatus(&overlayData_, &analyzerUIConfig_);
}
} // namespace OHOS::Ace
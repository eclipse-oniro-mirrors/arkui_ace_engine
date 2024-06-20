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
    CHECK_NULL_VOID(pixelMap);
    CHECK_NULL_VOID(imageAnalyzerAdapter_);
    auto pixelmapNapiVal = imageAnalyzerAdapter_->ConvertPixmapNapi(pixelMap);

    analyzerUIConfig_.holder = holder_;
    if (holder_ != ImageAnalyzerHolder::IMAGE) {
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
    CHECK_NULL_VOID(frameNode_);
    frameNode_->SetOverlayNode(overlayNode);
    overlayNode->SetParent(AceType::WeakClaim(AceType::RawPtr(frameNode_)));
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

    if (holder_ == ImageAnalyzerHolder::IMAGE) {
        auto imagePattern = AceType::DynamicCast<NG::ImagePattern>(frameNode_->GetPattern());
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
    auto overlayNode = frameNode_->GetOverlayNode();
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
    CHECK_NULL_VOID(frameNode_);
    auto overlayNode = frameNode_->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    frameNode_->SetOverlayNode(RefPtr<NG::FrameNode>());

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
    CHECK_NULL_RETURN(frameNode_, false);
    auto eventHub = frameNode_->GetEventHub<NG::EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (!eventHub->IsEnabled()) {
        return false;
    }

    bool hasObscured = false;
    if (frameNode_->GetRenderContext()->GetObscured().has_value()) {
        auto obscuredReasons = frameNode_->GetRenderContext()->GetObscured().value();
        hasObscured = std::any_of(obscuredReasons.begin(), obscuredReasons.end(),
            [](const auto& reason) { return reason == ObscuredReasons::PLACEHOLDER; });
        if (hasObscured) {
            return false;
        }
    }

    if (holder_ == ImageAnalyzerHolder::IMAGE) {
        auto imageRenderProperty = frameNode_->GetPaintProperty<NG::ImageRenderProperty>();
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
    CHECK_NULL_VOID(frameNode_);
    auto layoutProperty = frameNode_->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto overlayNode = frameNode_->GetOverlayNode();
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
    CHECK_NULL_VOID(frameNode_);
    bool isUIConfigUpdate = false;

    auto layoutProps = frameNode_->GetLayoutProperty<NG::ImageLayoutProperty>();
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

    auto renderContext = frameNode_->GetRenderContext();
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
    if ((holder_ != ImageAnalyzerHolder::XCOMPONENT && holder_ != ImageAnalyzerHolder::VIDEO_CUSTOM) || !hasConfig) {
        imageAnalyzerAdapter_->SetImageAnalyzerConfig(config);
    }
    auto analyzerConfig = imageAnalyzerAdapter_->GetImageAnalyzerConfig();
    if (isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().UpdateConfig(&overlayData_, analyzerConfig);
    }
}

void ImageAnalyzerManager::SetImageAIOptions(void* options)
{
    SetImageAnalyzerConfig(options);
}

void ImageAnalyzerManager::SetImageAnalyzerCallback(onAnalyzedCallback& callback)
{
    analyzerUIConfig_.onAnalyzed = callback;
}

void ImageAnalyzerManager::ReleaseImageAnalyzer()
{
    if (isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().Release(&overlayData_);
    }
}
} // namespace OHOS::Ace
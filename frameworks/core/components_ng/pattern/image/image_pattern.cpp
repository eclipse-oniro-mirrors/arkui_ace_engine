/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/image/image_source_info.h"
#define NAPI_VERSION 8

#include <array>
#include <cstdint>

#include "base/geometry/dimension_offset.h"
#include "base/geometry/matrix4.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/vector.h"
#include "base/log/dump_log.h"
#include "base/utils/utils.h"
#include "core/common/ai/image_analyzer_mgr.h"
#include "core/common/frontend.h"
#include "core/components/common/layout/constants.h"
#include "core/components/image/image_theme.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_paint_method.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#if defined(PIXEL_MAP_SUPPORTED)
#include "foundation/multimedia/image_framework/interfaces/kits/js/common/include/pixel_map_napi.h"
#endif
#include "core/common/ace_engine_ext.h"
#include "core/common/udmf/udmf_client.h"

namespace OHOS::Ace::NG {
napi_value ConvertPixmapNapi(const RefPtr<PixelMap>& pixelMap)
{
#if defined(PIXEL_MAP_SUPPORTED)
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, {});
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    auto* env = reinterpret_cast<napi_env>(nativeEngine);
    napi_value napiValue = OHOS::Media::PixelMapNapi::CreatePixelMap(env, pixelMap->GetPixelMapSharedPtr());
    return napiValue;
#else
    return nullptr;
#endif
}

DataReadyNotifyTask ImagePattern::CreateDataReadyCallback()
{
    return [weak = WeakClaim(this)](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto imageLayoutProperty = pattern->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        auto currentSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
        if (currentSourceInfo != sourceInfo) {
            TAG_LOGW(AceLogTag::ACE_IMAGE,
                "sourceInfo does not match, ignore current callback. "
                "current: %{public}s vs callback's: %{public}s",
                currentSourceInfo.ToString().c_str(), sourceInfo.ToString().c_str());
            return;
        }
        pattern->OnImageDataReady();
    };
}

LoadSuccessNotifyTask ImagePattern::CreateLoadSuccessCallback()
{
    return [weak = WeakClaim(this)](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto imageLayoutProperty = pattern->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        auto currentSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
        if (currentSourceInfo != sourceInfo) {
            TAG_LOGW(AceLogTag::ACE_IMAGE,
                "sourceInfo does not match, ignore current callback. "
                "current: %{public}s vs callback's: %{public}s",
                currentSourceInfo.ToString().c_str(), sourceInfo.ToString().c_str());
            return;
        }
        pattern->OnImageLoadSuccess();
    };
}

LoadFailNotifyTask ImagePattern::CreateLoadFailCallback()
{
    return [weak = WeakClaim(this)](const ImageSourceInfo& sourceInfo, const std::string& errorMsg) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto imageLayoutProperty = pattern->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        auto currentSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
        if (currentSourceInfo != sourceInfo) {
            TAG_LOGW(AceLogTag::ACE_IMAGE,
                "sourceInfo does not match, ignore current callback. "
                "current: %{public}s vs callback's: %{public}s",
                currentSourceInfo.ToString().c_str(), sourceInfo.ToString().c_str());
            return;
        }
        pattern->OnImageLoadFail(errorMsg);
    };
}

void ImagePattern::PrepareAnimation(const RefPtr<CanvasImage>& image)
{
    if (image->IsStatic()) {
        return;
    }
    SetRedrawCallback(image);
    RegisterVisibleAreaChange();
    auto layoutProps = GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    // pause animation if prop is initially set to invisible
    if (layoutProps->GetVisibility().value_or(VisibleType::VISIBLE) != VisibleType::VISIBLE) {
        image->ControlAnimation(false);
    }
}

void ImagePattern::SetRedrawCallback(const RefPtr<CanvasImage>& image)
{
    CHECK_NULL_VOID(image);
    // set animation flush function for svg / gif
    image->SetRedrawCallback([weak = WeakPtr(GetHost())] {
        auto imageNode = weak.Upgrade();
        CHECK_NULL_VOID(imageNode);
        imageNode->MarkNeedRenderOnly();
    });
}

void ImagePattern::RegisterVisibleAreaChange()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    // register to onVisibleAreaChange
    CHECK_NULL_VOID(pipeline);
    auto callback = [weak = WeakClaim(this)](bool visible, double ratio) {
        auto self = weak.Upgrade();
        CHECK_NULL_VOID(self);
        self->OnVisibleChange(visible);
    };
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    // add visibleAreaChangeNode(inner callback)
    pipeline->AddVisibleAreaChangeNode(host, 0.0f, callback, false);
}

RectF ImagePattern::CalcImageContentPaintSize(const RefPtr<GeometryNode>& geometryNode)
{
    RectF paintSize;
    auto imageRenderProperty = GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(imageRenderProperty, paintSize);
    ImageRepeat repeat = imageRenderProperty->GetImageRepeat().value_or(ImageRepeat::NO_REPEAT);
    bool imageRepeatX = repeat == ImageRepeat::REPEAT || repeat == ImageRepeat::REPEAT_X;
    bool imageRepeatY = repeat == ImageRepeat::REPEAT || repeat == ImageRepeat::REPEAT_Y;

    if (loadingCtx_->GetSourceInfo().IsSvg()) {
        const float invalidValue = -1;
        paintSize.SetWidth(dstRect_.IsValid() ? dstRect_.Width() : invalidValue);
        paintSize.SetHeight(dstRect_.IsValid() ? dstRect_.Height() : invalidValue);
        paintSize.SetLeft(
            dstRect_.IsValid() ? dstRect_.GetX() + geometryNode->GetContentOffset().GetX() : invalidValue);
        paintSize.SetTop(dstRect_.IsValid() ? dstRect_.GetY() + geometryNode->GetContentOffset().GetY() : invalidValue);
    } else {
        paintSize.SetWidth(imageRepeatX ? geometryNode->GetContentSize().Width() : dstRect_.Width());
        paintSize.SetHeight(imageRepeatY ? geometryNode->GetContentSize().Height() : dstRect_.Height());
        paintSize.SetLeft((imageRepeatX ? 0 : dstRect_.GetX()) + geometryNode->GetContentOffset().GetX());
        paintSize.SetTop((imageRepeatY ? 0 : dstRect_.GetY()) + geometryNode->GetContentOffset().GetY());
    }
    return paintSize;
}

void ImagePattern::OnImageLoadSuccess()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);

    image_ = loadingCtx_->MoveCanvasImage();
    srcRect_ = loadingCtx_->GetSrcRect();
    dstRect_ = loadingCtx_->GetDstRect();

    RectF paintRect = CalcImageContentPaintSize(geometryNode);
    LoadImageSuccessEvent event(loadingCtx_->GetImageSize().Width(), loadingCtx_->GetImageSize().Height(),
        geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height(), 1, paintRect.Width(),
        paintRect.Height(), paintRect.GetX(), paintRect.GetY());
    auto eventHub = GetEventHub<ImageEventHub>();
    if (eventHub) {
        eventHub->FireCompleteEvent(event);
    }

    SetImagePaintConfig(image_, srcRect_, dstRect_, loadingCtx_->GetSourceInfo(), loadingCtx_->GetFrameCount());
    PrepareAnimation(image_);
    if (host->IsDraggable()) {
        EnableDrag();
    }
    // clear alt data
    altLoadingCtx_ = nullptr;
    altImage_ = nullptr;
    altDstRect_.reset();
    altSrcRect_.reset();

    if (!IsSupportImageAnalyzerFeature() && isAnalyzerOverlayBuild_) {
        DeleteAnalyzerOverlay();
    }
    UpdateAnalyzerOverlay();
    CreateAnalyzerOverlay();
    host->MarkNeedRenderOnly();
}

void ImagePattern::OnImageDataReady()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto imageEventHub = GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(imageEventHub);
    LoadImageSuccessEvent event(loadingCtx_->GetImageSize().Width(), loadingCtx_->GetImageSize().Height(),
        geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height(), 0,
        geometryNode->GetContentSize().Width(), geometryNode->GetContentSize().Height(),
        geometryNode->GetContentOffset().GetX(), geometryNode->GetContentOffset().GetY());
    imageEventHub->FireCompleteEvent(event);

    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void ImagePattern::OnImageLoadFail(const std::string& errorMsg)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& geometryNode = host->GetGeometryNode();
    auto imageEventHub = GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(imageEventHub);
    LoadImageFailEvent event(geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height(), errorMsg);
    imageEventHub->FireErrorEvent(event);
}

void ImagePattern::SetImagePaintConfig(const RefPtr<CanvasImage>& canvasImage, const RectF& srcRect,
    const RectF& dstRect, const ImageSourceInfo& sourceInfo, int32_t frameCount)
{
    auto layoutProps = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);

    ImagePaintConfig config {
        .srcRect_ = srcRect,
        .dstRect_ = dstRect,
    };
    config.imageFit_ = layoutProps->GetImageFit().value_or(ImageFit::COVER);
    config.isSvg_ = sourceInfo.IsSvg();
    config.frameCount_ = frameCount;
    config.sourceInfo_ = sourceInfo;
    auto host = GetHost();
    if (!host) {
        canvasImage->SetPaintConfig(config);
        return;
    }
    auto renderContext = host->GetRenderContext();
    if (!renderContext || !renderContext->HasBorderRadius()) {
        canvasImage->SetPaintConfig(config);
        return;
    }

    auto renderProps = host->GetPaintProperty<ImageRenderProperty>();
    if (renderProps) {
        renderProps->UpdateNeedBorderRadius(true);
    }
    canvasImage->SetPaintConfig(config);
}

RefPtr<NodePaintMethod> ImagePattern::CreateNodePaintMethod()
{
    if (image_) {
        return MakeRefPtr<ImagePaintMethod>(image_, selectOverlay_);
    }
    if (altImage_ && altDstRect_ && altSrcRect_) {
        return MakeRefPtr<ImagePaintMethod>(altImage_, selectOverlay_);
    }
    CreateObscuredImage();
    if (obscuredImage_) {
        return MakeRefPtr<ImagePaintMethod>(obscuredImage_, selectOverlay_);
    }
    return nullptr;
}

bool ImagePattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }

    if (loadingCtx_) {
        auto renderProp = GetPaintProperty<ImageRenderProperty>();
        if (renderProp && renderProp->HasImageResizableSlice() && image_) {
            loadingCtx_->ResizableCalcDstSize();
            SetImagePaintConfig(image_, loadingCtx_->GetSrcRect(), loadingCtx_->GetDstRect(), loadingCtx_->GetSrc(),
                loadingCtx_->GetFrameCount());
        }
    }

    if (altLoadingCtx_) {
        auto renderProp = GetPaintProperty<ImageRenderProperty>();
        if (renderProp && renderProp->HasImageResizableSlice() && altImage_) {
            altLoadingCtx_->ResizableCalcDstSize();
            SetImagePaintConfig(altImage_, altLoadingCtx_->GetSrcRect(), altLoadingCtx_->GetDstRect(),
                altLoadingCtx_->GetSrc(), altLoadingCtx_->GetFrameCount());
        }
    }

    if (IsSupportImageAnalyzerFeature()) {
        UpdateAnalyzerUIConfig(dirty->GetGeometryNode());
    }

    return image_ || altImage_;
}

void ImagePattern::CreateObscuredImage()
{
    auto props = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(props);
    auto layoutConstraint = props->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto sourceInfo = props->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    auto reasons = host->GetRenderContext()->GetObscured().value_or(std::vector<ObscuredReasons>());
    if (reasons.size() && layoutConstraint->selfIdealSize.IsValid()) {
        if (!obscuredImage_) {
            obscuredImage_ = MakeRefPtr<ObscuredImage>();
            SetImagePaintConfig(obscuredImage_, srcRect_, dstRect_, sourceInfo);
        }
    }
}

void ImagePattern::LoadImage(const ImageSourceInfo& src)
{
    LoadNotifier loadNotifier(CreateDataReadyCallback(), CreateLoadSuccessCallback(), CreateLoadFailCallback());

    loadingCtx_ = AceType::MakeRefPtr<ImageLoadingContext>(src, std::move(loadNotifier), syncLoad_);
    if (SystemProperties::GetDebugEnabled()) {
        TAG_LOGI(AceLogTag::ACE_IMAGE, "start loading image %{public}s", src.ToString().c_str());
    }
    loadingCtx_->LoadImageData();
}

void ImagePattern::LoadAltImage(const ImageSourceInfo& altImageSourceInfo)
{
    LoadNotifier altLoadNotifier(CreateDataReadyCallbackForAlt(), CreateLoadSuccessCallbackForAlt(), nullptr);
    if (!altLoadingCtx_ || altLoadingCtx_->GetSourceInfo() != altImageSourceInfo ||
        (altLoadingCtx_ && altImageSourceInfo.IsSvg())) {
        altLoadingCtx_ = AceType::MakeRefPtr<ImageLoadingContext>(altImageSourceInfo, std::move(altLoadNotifier));
        altLoadingCtx_->LoadImageData();
    }
}

void ImagePattern::LoadImageDataIfNeed()
{
    auto imageLayoutProperty = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto imageRenderProperty = GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(imageRenderProperty);
    auto src = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    UpdateInternalResource(src);

    if (!loadingCtx_ || loadingCtx_->GetSourceInfo() != src) {
        LoadImage(src);
    } else {
        auto currentContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(currentContext);
        int32_t instanceID = currentContext->GetInstanceId();
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto context = host->GetContext();
        CHECK_NULL_VOID(context);
        auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([weak = WeakClaim(this), instanceID] {
            ContainerScope scope(instanceID);
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->CreateAnalyzerOverlay();
            if (pattern->IsSupportImageAnalyzerFeature()) {
                auto host = pattern->GetHost();
                pattern->UpdateAnalyzerUIConfig(host->GetGeometryNode());
            }
        });
    }
    if (loadingCtx_->NeedAlt() && imageLayoutProperty->GetAlt()) {
        auto altImageSourceInfo = imageLayoutProperty->GetAlt().value_or(ImageSourceInfo(""));
        LoadAltImage(altImageSourceInfo);
    }
}

void ImagePattern::UpdateGestureAndDragWhenModify()
{
    // remove long press and mouse events
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto gestureHub = host->GetOrCreateGestureEventHub();
    if (longPressEvent_) {
        gestureHub->SetLongPressEvent(nullptr);
        longPressEvent_ = nullptr;
    }

    if (clickEvent_) {
        gestureHub->RemoveClickEvent(clickEvent_);
        clickEvent_ = nullptr;
    }

    if (mouseEvent_) {
        auto inputHub = host->GetOrCreateInputEventHub();
        inputHub->RemoveOnMouseEvent(mouseEvent_);
        mouseEvent_ = nullptr;
    }

    if (host->IsDraggable()) {
        EnableDrag();
    }
}

void ImagePattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    LoadImageDataIfNeed();

    if (copyOption_ != CopyOptions::None) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        bool hasObscured = false;
        if (host->GetRenderContext()->GetObscured().has_value()) {
            auto obscuredReasons = host->GetRenderContext()->GetObscured().value();
            hasObscured = std::any_of(obscuredReasons.begin(), obscuredReasons.end(),
                [](const auto& reason) { return reason == ObscuredReasons::PLACEHOLDER; });
        }
        if (!hasObscured) {
            InitCopy();
            return;
        }
    }

    CloseSelectOverlay();

    auto host = GetHost();
    CHECK_NULL_VOID(host);

    UpdateGestureAndDragWhenModify();

    if (isAnalyzerOverlayBuild_) {
        if (!IsSupportImageAnalyzerFeature()) {
            DeleteAnalyzerOverlay();
        } else {
            UpdateAnalyzerOverlayLayout();
        }
    }

    // SetUsingContentRectForRenderFrame is set for image paint
    auto overlayNode = host->GetOverlayNode();
    if (overlayNode) {
        auto layoutProperty = host->GetLayoutProperty();
        CHECK_NULL_VOID(layoutProperty);
        auto padding = layoutProperty->CreatePaddingAndBorder();
        auto renderContext = overlayNode->GetRenderContext();
        if (renderContext) {
            renderContext->SetRenderFrameOffset({-padding.Offset().GetX(), -padding.Offset().GetY()});
        }
    }
}

DataReadyNotifyTask ImagePattern::CreateDataReadyCallbackForAlt()
{
    return [weak = WeakClaim(this)](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto imageLayoutProperty = pattern->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        auto currentAltSourceInfo = imageLayoutProperty->GetAlt().value_or(ImageSourceInfo(""));
        if (currentAltSourceInfo != sourceInfo) {
            TAG_LOGW(AceLogTag::ACE_IMAGE,
                "alt image sourceInfo does not match, ignore current callback. "
                "current: %{public}s vs callback's: %{public}s",
                currentAltSourceInfo.ToString().c_str(), sourceInfo.ToString().c_str());
            return;
        }
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        if (!host->IsActive()) {
            return;
        }
        const auto& geometryNode = host->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        if (!geometryNode->GetContent()) {
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            return;
        }

        // calculate params for [altLoadingCtx] to do [MakeCanvasImage] if component size is already settled
        pattern->altLoadingCtx_->MakeCanvasImageIfNeed(
            geometryNode->GetContentSize(), true, imageLayoutProperty->GetImageFit().value_or(ImageFit::COVER));
    };
}

LoadSuccessNotifyTask ImagePattern::CreateLoadSuccessCallbackForAlt()
{
    return [weak = WeakClaim(this)](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        CHECK_NULL_VOID(pattern->altLoadingCtx_);
        auto layoutProps = pattern->GetLayoutProperty<ImageLayoutProperty>();
        auto currentAltSrc = layoutProps->GetAlt().value_or(ImageSourceInfo(""));
        if (currentAltSrc != sourceInfo) {
            TAG_LOGW(AceLogTag::ACE_IMAGE,
                "alt image sourceInfo does not match, ignore current callback. "
                "current: %{public}s vs callback's: %{public}s",
                currentAltSrc.ToString().c_str(), sourceInfo.ToString().c_str());
            return;
        }
        pattern->altImage_ = pattern->altLoadingCtx_->MoveCanvasImage();
        pattern->altSrcRect_ = std::make_unique<RectF>(pattern->altLoadingCtx_->GetSrcRect());
        pattern->altDstRect_ = std::make_unique<RectF>(pattern->altLoadingCtx_->GetDstRect());
        pattern->SetImagePaintConfig(pattern->altImage_, *pattern->altSrcRect_, *pattern->altDstRect_,
            pattern->altLoadingCtx_->GetSourceInfo(), pattern->altLoadingCtx_->GetFrameCount());

        pattern->PrepareAnimation(pattern->altImage_);

        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    };
}

void ImagePattern::UpdateInternalResource(ImageSourceInfo& sourceInfo)
{
    if (!sourceInfo.IsInternalResource()) {
        return;
    }

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto iconTheme = pipeline->GetTheme<IconTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto iconPath = iconTheme->GetIconPath(sourceInfo.GetResourceId());
    if (!iconPath.empty()) {
        sourceInfo.SetSrc(iconPath, sourceInfo.GetFillColor());
        auto imageLayoutProperty = GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        imageLayoutProperty->UpdateImageSourceInfo(sourceInfo);
    }
}

void ImagePattern::OnNotifyMemoryLevel(int32_t level)
{
    // TODO: do different data cleaning operation according to level
    // when image component is [onShow], do not clean image data
    // TODO: use [isActive_] to determine image data management
    if (isShow_) {
        return;
    }

    // clean image data
    loadingCtx_ = nullptr;
    image_ = nullptr;
    altLoadingCtx_ = nullptr;
    altImage_ = nullptr;

    // clean rs node to release the sk_sp<SkImage> held by it
    // TODO: release PixelMap resource when use PixelMap resource to draw image
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto rsRenderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(rsRenderContext);
    rsRenderContext->ClearDrawCommands();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->FlushMessages();
}

// when recycle image component, release the pixelmap resource
void ImagePattern::OnRecycle()
{
    loadingCtx_ = nullptr;
    image_ = nullptr;
    altLoadingCtx_ = nullptr;
    altImage_ = nullptr;

    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto rsRenderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(rsRenderContext);
    rsRenderContext->ClearDrawCommands();
}

void ImagePattern::OnReuse()
{
    LoadImageDataIfNeed();
}

void ImagePattern::OnWindowHide()
{
    isShow_ = false;
}

void ImagePattern::OnWindowShow()
{
    isShow_ = true;
    LoadImageDataIfNeed();
}

void ImagePattern::OnVisibleChange(bool visible)
{
    if (!visible) {
        CloseSelectOverlay();
    }
    // control svg / gif animation
    if (image_) {
        image_->ControlAnimation(visible);
    } else if (altImage_) {
        altImage_->ControlAnimation(visible);
    }
}

void ImagePattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderCtx = host->GetRenderContext();
    renderCtx->SetClipToBounds(false);
    renderCtx->SetUsingContentRectForRenderFrame(true);

    // register image frame node to pipeline context to receive memory level notification and window state change
    // notification
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddNodesToNotifyMemoryLevel(host->GetId());
    pipeline->AddWindowStateChangedCallback(host->GetId());
}

void ImagePattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CloseSelectOverlay();

    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
    pipeline->RemoveNodesToNotifyMemoryLevel(id);
}

void ImagePattern::EnableDrag()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto dragStart = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string&
                         /* extraParams */) -> DragDropInfo {
        DragDropInfo info;
        auto imagePattern = weak.Upgrade();
        CHECK_NULL_RETURN(imagePattern && imagePattern->loadingCtx_, info);
        AceEngineExt::GetInstance().DragStartExt();
        imagePattern->UpdateDragEvent(event);
        info.extraInfo = imagePattern->loadingCtx_->GetSourceInfo().GetSrc();
        return info;
    };
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    if (!eventHub->HasOnDragStart()) {
        eventHub->SetOnDragStart(std::move(dragStart));
    }
}

bool ImagePattern::BetweenSelectedPosition(const Offset& globalOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto globalRect = host->GetTransformRectRelativeToWindow();
    return globalRect.IsInRegion(PointF { globalOffset.GetX(), globalOffset.GetY() });
}

void ImagePattern::BeforeCreatePaintWrapper()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->MarkContentChanged(true);
}

void ImagePattern::InitCopy()
{
    if (longPressEvent_ && mouseEvent_ && clickEvent_) {
        return;
    }
    auto longPressTask = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OpenSelectOverlay();
    };
    longPressEvent_ = MakeRefPtr<LongPressEvent>(std::move(longPressTask));

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    gestureHub->SetLongPressEvent(longPressEvent_);

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        if (info.GetButton() == MouseButton::RIGHT_BUTTON && info.GetAction() == MouseAction::PRESS) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OpenSelectOverlay();
        }
    };
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    auto inputHub = host->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    inputHub->AddOnMouseEvent(mouseEvent_);

    // close overlay on click
    clickEvent_ = MakeRefPtr<ClickEvent>([weak = WeakClaim(this)](GestureEvent& callback) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->CloseSelectOverlay();
    });
    gestureHub->AddClickEvent(clickEvent_);
}

void ImagePattern::OpenSelectOverlay()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto rect = host->GetTransformRectRelativeToWindow();
    SelectOverlayInfo info;
    SizeF handleSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), info.singleLineHeight };
    info.firstHandle.paintRect = RectF(rect.GetOffset(), handleSize);
    OffsetF offset(rect.Width() - handleSize.Width(), rect.Height() - handleSize.Height());
    info.secondHandle.paintRect = RectF(rect.GetOffset() + offset, handleSize);
    info.menuInfo.menuIsShow = true;
    info.menuInfo.showCut = false;
    info.menuInfo.showPaste = false;
    info.menuCallback.onCopy = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleCopy();
        pattern->CloseSelectOverlay();
    };
    info.onHandleMoveDone = [weak = WeakClaim(this), firstRect = info.firstHandle.paintRect,
                                secondRect = info.secondHandle.paintRect](const RectF&, bool isFirst) {
        // reset handle position
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern && pattern->selectOverlay_);
        SelectHandleInfo info;
        if (isFirst) {
            info.paintRect = firstRect;
            pattern->selectOverlay_->UpdateFirstSelectHandleInfo(info);
        } else {
            info.paintRect = secondRect;
            pattern->selectOverlay_->UpdateSecondSelectHandleInfo(info);
        }
    };
    info.onClose = [weak = WeakClaim(this)](bool closedByGlobalEvent) {
        if (closedByGlobalEvent) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->CloseSelectOverlay();
        }
    };

    CloseSelectOverlay();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    selectOverlay_ = pipeline->GetSelectOverlayManager()->CreateAndShowSelectOverlay(info, WeakClaim(this));

    // paint selected mask effect
    host->MarkNeedRenderOnly();
}

void ImagePattern::CloseSelectOverlay()
{
    if (!selectOverlay_) {
        return;
    }
    if (!selectOverlay_->IsClosed()) {
        selectOverlay_->Close();
    }
    selectOverlay_ = nullptr;
    // remove selected mask effect
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkNeedRenderOnly();
}

void ImagePattern::HandleCopy()
{
    CHECK_NULL_VOID(image_);
    if (!clipboard_) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    }
    auto pixmap = image_->GetPixelMap();
    if (pixmap) {
        clipboard_->SetPixelMapData(pixmap, copyOption_);
    } else {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        clipboard_->SetData(loadingCtx_->GetSourceInfo().GetSrc());
    }
}

void ImagePattern::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    static const char* COPY_OPTIONS[] = { "CopyOptions.None", "CopyOptions.InApp", "CopyOptions.Local",
        "CopyOptions.Distributed" };
    json->Put("copyOption", COPY_OPTIONS[static_cast<int32_t>(copyOption_)]);

    json->Put("syncLoad", syncLoad_ ? "true" : "false");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    json->Put("draggable", host->IsDraggable() ? "true" : "false");
    json->Put("enableAnalyzer", isEnableAnalyzer_ ? "true" : "false");
}

void ImagePattern::UpdateFillColorIfForegroundColor()
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (renderContext->HasForegroundColor() || renderContext->HasForegroundColorStrategy()) {
        auto imageLayoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
        auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value();
        if (imageSourceInfo.IsSvg()) {
            imageSourceInfo.SetFillColor(Color::FOREGROUND);
            imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
        }
        auto imageRenderProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
        CHECK_NULL_VOID(imageRenderProperty);
        imageRenderProperty->UpdateSvgFillColor(Color::FOREGROUND);
    }
}

void ImagePattern::DumpInfo()
{
    auto layoutProp = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProp);
    auto src = layoutProp->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    DumpLog::GetInstance().AddDesc(std::string("url: ").append(src.ToString()));
    syncLoad_ ? DumpLog::GetInstance().AddDesc("syncLoad:true") : DumpLog::GetInstance().AddDesc("syncLoad:false");
    DumpLog::GetInstance().AddDesc("imageInterpolation:" + GetImageInterpolation());
    if (loadingCtx_) {
        auto currentLoadImageState = loadingCtx_->GetCurrentLoadingState();
        DumpLog::GetInstance().AddDesc(std::string("currentLoadImageState : ").append(currentLoadImageState));
        DumpLog::GetInstance().AddDesc(std::string("rawImageSize: ").append(loadingCtx_->GetImageSize().ToString()));
    }
    auto imageRenderProperty = GetPaintProperty<ImageRenderProperty>();
    if (imageRenderProperty && imageRenderProperty->HasImageResizableSlice() &&
        imageRenderProperty->GetImageResizableSliceValue({}).Valid()) {
        DumpLog::GetInstance().AddDesc(
            std::string("reslzable slice: ").append(imageRenderProperty->GetImageResizableSliceValue({}).ToString()));
    }
    DumpLog::GetInstance().AddDesc(std::string("enableAnalyzer: ").append(isEnableAnalyzer_ ? "true" : "false"));
}

void ImagePattern::DumpAdvanceInfo()
{
    auto layoutProp = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProp);
    auto src = layoutProp->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    DumpLog::GetInstance().AddDesc(std::string("url: ").append(src.ToString()));
    syncLoad_ ? DumpLog::GetInstance().AddDesc("syncLoad:true") : DumpLog::GetInstance().AddDesc("syncLoad:false");
    if (loadingCtx_) {
        auto currentLoadImageState = loadingCtx_->GetCurrentLoadingState();
        DumpLog::GetInstance().AddDesc(std::string("currentLoadImageState : ").append(currentLoadImageState));
    }
}

void ImagePattern::UpdateDragEvent(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    RefPtr<UnifiedData> unifiedData = UdmfClient::GetInstance()->CreateUnifiedData();
    CHECK_NULL_VOID(loadingCtx_ && image_);
    if (loadingCtx_->GetSourceInfo().IsPixmap()) {
        auto pixelMap = image_->GetPixelMap();
        CHECK_NULL_VOID(pixelMap);
        const uint8_t* pixels = pixelMap->GetPixels();
        CHECK_NULL_VOID(pixels);
        int32_t length = pixelMap->GetByteCount();
        std::vector<uint8_t> data(pixels, pixels + length);
        PixelMapRecordDetails details = { pixelMap->GetWidth(), pixelMap->GetHeight(), pixelMap->GetPixelFormat(),
            pixelMap->GetAlphaType() };
        UdmfClient::GetInstance()->AddPixelMapRecord(unifiedData, data, details);
    } else {
        UdmfClient::GetInstance()->AddImageRecord(unifiedData, loadingCtx_->GetSourceInfo().GetSrc());
    }
    event->SetData(unifiedData);
}

void ImagePattern::OnLanguageConfigurationUpdate()
{
    CHECK_NULL_VOID(loadingCtx_);
    auto&& src = loadingCtx_->GetSourceInfo();
    // Resource image needs to reload when Language changes
    if (src.GetSrcType() == SrcType::RESOURCE) {
        loadingCtx_.Reset();
    }
}

void ImagePattern::OnColorConfigurationUpdate()
{
    OnConfigurationUpdate();
}

void ImagePattern::OnIconConfigurationUpdate()
{
    OnConfigurationUpdate();
}

void ImagePattern::OnConfigurationUpdate()
{
    CHECK_NULL_VOID(loadingCtx_);

    auto imageLayoutProperty = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto src = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    UpdateInternalResource(src);
    src.SetIsConfigurationChange(true);

    LoadImage(src);
    if (loadingCtx_->NeedAlt() && imageLayoutProperty->GetAlt()) {
        auto altImageSourceInfo = imageLayoutProperty->GetAlt().value_or(ImageSourceInfo(""));
        if (altLoadingCtx_ && altLoadingCtx_->GetSourceInfo() == altImageSourceInfo) {
            altLoadingCtx_.Reset();
        }
        altImageSourceInfo.SetIsConfigurationChange(true);
        LoadAltImage(altImageSourceInfo);
    }
}

void ImagePattern::SetImageAnalyzerConfig(const ImageAnalyzerConfig& config)
{
    if (!isEnableAnalyzer_) {
        return;
    }
    analyzerConfig_ = config;
    if (IsSupportImageAnalyzerFeature() && isAnalyzerOverlayBuild_) {
        ImageAnalyzerMgr::GetInstance().UpdateConfig(&overlayData_, &analyzerConfig_);
    }
}

void ImagePattern::CreateAnalyzerOverlay()
{
    if (!IsSupportImageAnalyzerFeature() || isAnalyzerOverlayBuild_) {
        return;
    }

    auto pixelMap = image_->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    napi_value pixelmapNapiVal = ConvertPixmapNapi(pixelMap);
    auto frameNode = GetHost();
    auto overlayNode = frameNode->GetOverlayNode();

    auto layoutProps = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    analyzerUIConfig_.imageFit = layoutProps->GetImageFit().value_or(ImageFit::COVER);
    auto buildNodeFunction = [this, &pixelmapNapiVal]() -> RefPtr<UINode> {
        ScopedViewStackProcessor builderViewStackProcessor;
        ImageAnalyzerMgr::GetInstance().BuildNodeFunc(
            pixelmapNapiVal, &analyzerConfig_, &analyzerUIConfig_, &overlayData_);
        auto customNode = ViewStackProcessor::GetInstance()->Finish();
        return customNode;
    };
    overlayNode = AceType::DynamicCast<FrameNode>(buildNodeFunction());
    CHECK_NULL_VOID(overlayNode);
    frameNode->SetOverlayNode(overlayNode);
    overlayNode->SetParent(AceType::WeakClaim(AceType::RawPtr(frameNode)));
    overlayNode->SetActive(true);
    isAnalyzerOverlayBuild_ = true;

    UpdateAnalyzerOverlayLayout();
    auto renderContext = overlayNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateZIndex(INT32_MAX);
    auto focusHub = overlayNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetFocusable(false);
    overlayNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void ImagePattern::UpdateAnalyzerOverlay()
{
    if (!IsSupportImageAnalyzerFeature() || !isAnalyzerOverlayBuild_) {
        return;
    }

    auto imageLayoutProperty = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto src = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo(""));
    UpdateInternalResource(src);
    if (loadingCtx_ && loadingCtx_->GetSourceInfo() == src && srcRect_ == dstRect_) {
        return;
    }

    auto pixelMap = image_->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    napi_value pixelmapNapiVal = ConvertPixmapNapi(pixelMap);
    auto frameNode = GetHost();
    auto overlayNode = frameNode->GetOverlayNode();
    ImageAnalyzerMgr::GetInstance().UpdateImage(&overlayData_, pixelmapNapiVal, &analyzerConfig_, &analyzerUIConfig_);
    overlayNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void ImagePattern::DeleteAnalyzerOverlay()
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto overlayNode = frameNode->GetOverlayNode();
    isAnalyzerOverlayBuild_ = false;
    if (!overlayNode) {
        return;
    }
    RefPtr<FrameNode> node;
    frameNode->SetOverlayNode(node);
}

bool ImagePattern::IsSupportImageAnalyzerFeature()
{
    auto eventHub = GetEventHub<EventHub>();
    bool isEnabled = true;
    if (eventHub) {
        isEnabled = eventHub->IsEnabled();
    }

    bool hasObscured = false;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    if (host->GetRenderContext()->GetObscured().has_value()) {
        auto obscuredReasons = host->GetRenderContext()->GetObscured().value();
        hasObscured = std::any_of(obscuredReasons.begin(), obscuredReasons.end(),
            [](const auto& reason) { return reason == ObscuredReasons::PLACEHOLDER; });
    }

    auto imageRenderProperty = GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(imageRenderProperty, false);
    ImageRepeat repeat = imageRenderProperty->GetImageRepeat().value_or(ImageRepeat::NO_REPEAT);

    return isEnabled && !hasObscured && isEnableAnalyzer_ &&
           ImageAnalyzerMgr::GetInstance().IsImageAnalyzerSupported() && image_ &&
           !loadingCtx_->GetSourceInfo().IsSvg() && repeat == ImageRepeat::NO_REPEAT &&
           loadingCtx_->GetFrameCount() == 1;
}

void ImagePattern::UpdateAnalyzerUIConfig(const RefPtr<GeometryNode>& geometryNode)
{
    bool isUIConfigUpdate = false;
    CHECK_NULL_VOID(geometryNode);
    if (analyzerUIConfig_.contentWidth != geometryNode->GetContentSize().Width() ||
        analyzerUIConfig_.contentHeight != geometryNode->GetContentSize().Height()) {
        analyzerUIConfig_.contentWidth = geometryNode->GetContentSize().Width();
        analyzerUIConfig_.contentHeight = geometryNode->GetContentSize().Height();
        isUIConfigUpdate = true;
    }

    auto layoutProps = GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    if (analyzerUIConfig_.imageFit != layoutProps->GetImageFit().value_or(ImageFit::COVER)) {
        analyzerUIConfig_.imageFit = layoutProps->GetImageFit().value_or(ImageFit::COVER);
        isUIConfigUpdate = true;
    }

    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

    auto localCenter = renderContext->GetTransformCenterValue(DimensionOffset(0.5_pct, 0.5_pct));
    auto localScale = renderContext->GetTransformScaleValue(VectorF(1.0f, 1.0f));
    Matrix4 localScaleMat = Matrix4::CreateTranslate(localCenter.GetX().Value(), localCenter.GetY().Value(), 0) *
                            Matrix4::CreateScale(localScale.x, localScale.y, 1.0f) *
                            Matrix4::CreateTranslate(-localCenter.GetX().Value(), -localCenter.GetY().Value(), 0);

    auto transformMat = renderContext->GetTransformMatrixValue(Matrix4::CreateIdentity());
    VectorF transCenter(transformMat.Get(0, 3), transformMat.Get(1, 3));
    Matrix4 transScaleMat = Matrix4::CreateTranslate(transCenter.x, transCenter.y, 0) *
                            Matrix4::CreateScale(transformMat.GetScaleX(), transformMat.GetScaleY(), 1.0f) *
                            Matrix4::CreateTranslate(-transCenter.x, -transCenter.y, 0);
    Matrix4 scaleMat = localScaleMat * transScaleMat;
    if (!(analyzerUIConfig_.transformMat == scaleMat)) {
        analyzerUIConfig_.transformMat = scaleMat;
        isUIConfigUpdate = true;
    }

    if (isUIConfigUpdate) {
        ImageAnalyzerMgr::GetInstance().UpdateInnerConfig(&overlayData_, &analyzerUIConfig_);
    }
}

void ImagePattern::UpdateAnalyzerOverlayLayout()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto overlayNode = host->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto overlayLayoutProperty = overlayNode->GetLayoutProperty();
    CHECK_NULL_VOID(overlayLayoutProperty);
    overlayLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    overlayLayoutProperty->UpdateAlignment(Alignment::TOP_LEFT);
    overlayLayoutProperty->SetOverlayOffset(Dimension(padding.Offset().GetX()), Dimension(padding.Offset().GetY()));
    overlayNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    auto renderContext = overlayNode->GetRenderContext();
    if (renderContext) {
        renderContext->SetRenderFrameOffset({-padding.Offset().GetX(), -padding.Offset().GetY()});
    }
}
} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/image_provider/image_loading_context.h"

#include "core/common/container.h"
#include "core/components_ng/render/image_painter.h"

namespace OHOS::Ace::NG {

ImageLoadingContext::ImageLoadingContext(const ImageSourceInfo& sourceInfo, const LoadNotifier& loadNotifier)
    : sourceInfo_(sourceInfo), loadNotifier_(loadNotifier),
      loadCallbacks_(GenerateDataReadyCallback(), GenerateLoadSuccessCallback(), GenerateLoadFailCallback())
{
    stateManager_ = MakeRefPtr<ImageStateManager>();
    RegisterStateChangeCallbacks();
}

SizeF ImageLoadingContext::CalculateResizeTarget(const SizeF& srcSize, const SizeF& dstSize, const SizeF& rawImageSize)
{
    SizeF resizeTarget = rawImageSize;
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, rawImageSize);
    auto viewScale = context->GetViewScale();
    do {
        if (!srcSize.IsPositive()) {
            break;
        }
        double widthScale = dstSize.Width() / srcSize.Width() * viewScale;
        double heightScale = dstSize.Height() / srcSize.Height() * viewScale;
        if (widthScale < 1.0 && heightScale < 1.0) {
            resizeTarget = SizeF(resizeTarget.Width() * widthScale, resizeTarget.Height() * heightScale);
        }
    } while (0);
    return resizeTarget;
}

void ImageLoadingContext::RegisterStateChangeCallbacks()
{
    // these tasks are called when loading state changes
    stateManager_->SetOnUnloadedCallback(CreateOnUnloadedTask());
    stateManager_->SetOnDataLoadingCallback(CreateOnDataLoadingTask());
    stateManager_->SetOnDataReadyCallback(CreateOnDataReadyTask());
    stateManager_->SetOnCanvasImageMakingCallback(CreateOnMakeCanvasImageTask());
    stateManager_->SetOnLoadSuccessCallback(CreateOnLoadSuccessTask());
    stateManager_->SetOnLoadFailCallback(CreateOnLoadFailTask());
}

EnterStateTask ImageLoadingContext::CreateOnUnloadedTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        LOGD("Enter State: Unloaded");
    };
    return task;
}

EnterStateTask ImageLoadingContext::CreateOnDataLoadingTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        ImageProvider::CreateImageObject(imageLoadingContext->sourceInfo_, imageLoadingContext->loadCallbacks_);
    };
    return task;
}

EnterStateTask ImageLoadingContext::CreateOnDataReadyTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        imageLoadingContext->loadNotifier_.dataReadyNotifyTask_(imageLoadingContext->GetSourceInfo());
    };
    return task;
}

EnterStateTask ImageLoadingContext::CreateOnMakeCanvasImageTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        if (!imageLoadingContext->imageObj_) {
            // there must be something wrong with state manager if [imageObj_] is null here.
            LOGE("image object is null during canvas image making task, sourceInfo: %{public}s",
                imageLoadingContext->GetSourceInfo().ToString().c_str());
            return;
        }
        // step1: do first [ApplyImageFit] to calculate the srcRect based on original image size
        ImagePainter::ApplyImageFit(imageLoadingContext->imageFit_, imageLoadingContext->GetImageSize(),
            imageLoadingContext->dstSize_, imageLoadingContext->srcRect_, imageLoadingContext->dstRect_);

        // step2: calculate resize target
        auto resizeTarget = imageLoadingContext->GetImageSize();
        if (imageLoadingContext->needResize_) {
            resizeTarget = ImageLoadingContext::CalculateResizeTarget(imageLoadingContext->srcRect_.GetSize(),
                imageLoadingContext->dstRect_.GetSize(), imageLoadingContext->GetImageSize());
        }

        // step3: do second [ApplyImageFit] to calculate real srcRect used for paint based on resized image size
        ImagePainter::ApplyImageFit(imageLoadingContext->imageFit_, resizeTarget, imageLoadingContext->dstSize_,
            imageLoadingContext->srcRect_, imageLoadingContext->dstRect_);

        // step4: [MakeCanvasImage] according to [resizeTarget]
        imageLoadingContext->imageObj_->MakeCanvasImage(imageLoadingContext->loadCallbacks_, resizeTarget);
    };
    return task;
}

EnterStateTask ImageLoadingContext::CreateOnLoadSuccessTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        imageLoadingContext->loadNotifier_.loadSuccessNotifyTask_(imageLoadingContext->GetSourceInfo());
    };
    return task;
}

EnterStateTask ImageLoadingContext::CreateOnLoadFailTask()
{
    auto task = [weakCtx = WeakClaim(this)]() {
        auto imageLoadingContext = weakCtx.Upgrade();
        CHECK_NULL_VOID(imageLoadingContext);
        imageLoadingContext->loadNotifier_.loadFailNotifyTask_(imageLoadingContext->GetSourceInfo());
    };
    return task;
}

DataReadyCallback ImageLoadingContext::GenerateDataReadyCallback()
{
    auto task = [weakCtx = WeakClaim(this)](const ImageSourceInfo& sourceInfo, const RefPtr<ImageObject>& imageObj) {
        auto loadingCtx = weakCtx.Upgrade();
        CHECK_NULL_VOID(loadingCtx);
        loadingCtx->OnDataReady(sourceInfo, imageObj);
    };
    return task;
}

void ImageLoadingContext::OnDataReady(const ImageSourceInfo& sourceInfo, const RefPtr<ImageObject> imageObj)
{
    if (sourceInfo_ != sourceInfo) {
        LOGI("DataReady callback with sourceInfo: %{private}s does not match current: %{private}s",
            sourceInfo.ToString().c_str(), sourceInfo_.ToString().c_str());
        return;
    }
    imageObj_ = imageObj;
    stateManager_->HandleCommand(ImageLoadingCommand::LOAD_DATA_SUCCESS);
}

LoadSuccessCallback ImageLoadingContext::GenerateLoadSuccessCallback()
{
    auto task = [weakCtx = WeakClaim(this)](const ImageSourceInfo& sourceInfo) {
        auto loadingCtx = weakCtx.Upgrade();
        CHECK_NULL_VOID(loadingCtx);
        loadingCtx->OnLoadSuccess(sourceInfo);
    };
    return task;
}

void ImageLoadingContext::OnLoadSuccess(const ImageSourceInfo& sourceInfo)
{
    if (sourceInfo_ != sourceInfo) {
        LOGI("LoadSuccess callback with sourceInfo: %{private}s does not match current: %{private}s",
            sourceInfo.ToString().c_str(), sourceInfo_.ToString().c_str());
        return;
    }
    stateManager_->HandleCommand(ImageLoadingCommand::MAKE_CANVAS_IMAGE_SUCCESS);
}

LoadFailCallback ImageLoadingContext::GenerateLoadFailCallback()
{
    auto task = [weakCtx = WeakClaim(this)](const ImageSourceInfo& sourceInfo, const std::string& errorMsg) {
        auto loadingCtx = weakCtx.Upgrade();
        CHECK_NULL_VOID(loadingCtx);
        loadingCtx->OnLoadFail(sourceInfo, errorMsg);
    };
    return task;
}

void ImageLoadingContext::OnLoadFail(const ImageSourceInfo& sourceInfo, const std::string& errorMsg)
{
    if (sourceInfo_ != sourceInfo) {
        LOGI("LoadFail callback with sourceInfo: %{private}s does not match current: %{private}s",
            sourceInfo.ToString().c_str(), sourceInfo_.ToString().c_str());
        return;
    }
    stateManager_->HandleCommand(ImageLoadingCommand::MAKE_CANVAS_IMAGE_FAIL);
}

const RectF& ImageLoadingContext::GetDstRect() const
{
    return dstRect_;
}

const RectF& ImageLoadingContext::GetSrcRect() const
{
    return srcRect_;
}

RefPtr<CanvasImage> ImageLoadingContext::GetCanvasImage() const
{
    return imageObj_ ? imageObj_->GetCanvasImage() : nullptr;
}

void ImageLoadingContext::LoadImageData()
{
    stateManager_->HandleCommand(ImageLoadingCommand::LOAD_DATA);
}

void ImageLoadingContext::MakeCanvasImage(const SizeF& dstSize, ImageFit imageFit)
{
    dstSize_ = dstSize;
    imageFit_ = imageFit;
    // send command to [StateManager] and waiting the callback from it to determine next step
    stateManager_->HandleCommand(ImageLoadingCommand::MAKE_CANVAS_IMAGE);
}

SizeF ImageLoadingContext::GetImageSize() const
{
    return imageObj_ ? imageObj_->GetImageSize() : SizeF(-1.0, -1.0);
}

ImageFit ImageLoadingContext::GetImageFit() const
{
    return imageFit_;
}

void ImageLoadingContext::SetImageFit(ImageFit imageFit)
{
    imageFit_ = imageFit;
}

const ImageSourceInfo& ImageLoadingContext::GetSourceInfo() const
{
    return sourceInfo_;
}

} // namespace OHOS::Ace::NG
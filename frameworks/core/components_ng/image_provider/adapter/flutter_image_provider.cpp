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

#include "core/components_ng/image_provider/adapter/flutter_image_provider.h"

#include <utility>

#include "flutter/fml/memory/ref_counted.h"
#ifdef NG_BUILD
#include "ace_shell/shell/common/window_manager.h"
#include "flutter/lib/ui/io_manager.h"
#else
#include "flutter/lib/ui/painting/image.h"
#endif
#include "third_party/skia/include/codec/SkCodec.h"
#include "third_party/skia/include/core/SkGraphics.h"

#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/common/thread_checker.h"
#include "core/components_ng/image_provider/adapter/skia_image_data.h"
#include "core/components_ng/image_provider/image_object.h"
#include "core/components_ng/image_provider/svg_image_object.h"
#include "core/components_ng/render/adapter/skia_canvas_image.h"
#include "core/image/flutter_image_cache.h"
#include "core/image/image_compressor.h"
#include "core/image/image_loader.h"
#include "core/pipeline_ng/pipeline_context.h"

#ifdef NG_BUILD
#include "core/components_ng/render/adapter/flutter_canvas_image.h"
#endif

namespace OHOS::Ace::NG {
namespace {

static sk_sp<SkImage> ApplySizeToSkImage(
    const sk_sp<SkImage>& rawImage, int32_t dstWidth, int32_t dstHeight, const std::string& srcKey)
{
    ACE_SCOPED_TRACE("ApplySizeToSkImage");
    auto scaledImageInfo =
        SkImageInfo::Make(dstWidth, dstHeight, rawImage->colorType(), rawImage->alphaType(), rawImage->refColorSpace());
    SkBitmap scaledBitmap;
    if (!scaledBitmap.tryAllocPixels(scaledImageInfo)) {
        LOGE("Could not allocate bitmap when attempting to scale. srcKey: %{private}s, destination size: [%{public}d x"
             " %{public}d], raw image size: [%{public}d x %{public}d]",
            srcKey.c_str(), dstWidth, dstHeight, rawImage->width(), rawImage->height());
        return rawImage;
    }
#ifdef NG_BUILD
    if (!rawImage->scalePixels(
            scaledBitmap.pixmap(), SkSamplingOptions(SkFilterMode::kLinear), SkImage::kDisallow_CachingHint)) {
#else
    if (!rawImage->scalePixels(scaledBitmap.pixmap(), kLow_SkFilterQuality, SkImage::kDisallow_CachingHint)) {
#endif
        LOGE("Could not scale pixels srcKey: %{private}s, destination size: [%{public}d x"
             " %{public}d], raw image size: [%{public}d x %{public}d]",
            srcKey.c_str(), dstWidth, dstHeight, rawImage->width(), rawImage->height());
        return rawImage;
    }
    // Marking this as immutable makes the MakeFromBitmap call share the pixels instead of copying.
    scaledBitmap.setImmutable();
    auto scaledImage = SkImage::MakeFromBitmap(scaledBitmap);
    if (scaledImage) {
        // TODO: cache
        return scaledImage;
    }
    LOGE("Could not create a scaled image from a scaled bitmap. srcKey: %{private}s, destination size: [%{public}d x"
         " %{public}d], raw image size: [%{public}d x %{public}d]",
        srcKey.c_str(), dstWidth, dstHeight, rawImage->width(), rawImage->height());
    return rawImage;
}

static sk_sp<SkImage> ResizeSkImage(
    const sk_sp<SkImage>& rawImage, const std::string& src, const SizeF& resizeTarget, bool forceResize)
{
    if (!resizeTarget.IsPositive()) {
        LOGE("not valid size! resizeTarget: %{public}s, src: %{public}s", resizeTarget.ToString().c_str(), src.c_str());
        return rawImage;
    }
    int32_t dstWidth = static_cast<int32_t>(resizeTarget.Width() + 0.5);
    int32_t dstHeight = static_cast<int32_t>(resizeTarget.Height() + 0.5);

    bool needResize = false;

    if (!forceResize) {
        if (rawImage->width() > dstWidth) {
            needResize = true;
        } else {
            dstWidth = rawImage->width();
        }
        if (rawImage->height() > dstHeight) {
            needResize = true;
        } else {
            dstHeight = rawImage->height();
        }
    }

    if (!needResize && !forceResize) {
        return rawImage;
    }
    return ApplySizeToSkImage(
        rawImage, dstWidth, dstHeight, ImageObject::GenerateCacheKey(ImageSourceInfo(src), resizeTarget));
}

} // namespace

RefPtr<ImageEncodedInfo> ImageEncodedInfo::CreateImageEncodedInfoForStaticImage(const RefPtr<NG::ImageData>& data)
{
    auto skiaImageData = DynamicCast<SkiaImageData>(data);
    CHECK_NULL_RETURN(skiaImageData, nullptr);
    auto codec = SkCodec::MakeFromData(skiaImageData->GetSkData());
    CHECK_NULL_RETURN(codec, nullptr);
    int32_t totalFrames = 1;
    SizeF imageSize;
    totalFrames = codec->getFrameCount();
    switch (codec->getOrigin()) {
        case SkEncodedOrigin::kLeftTop_SkEncodedOrigin:
        case SkEncodedOrigin::kRightTop_SkEncodedOrigin:
        case SkEncodedOrigin::kRightBottom_SkEncodedOrigin:
        case SkEncodedOrigin::kLeftBottom_SkEncodedOrigin:
            imageSize.SetSizeT(SizeF(codec->dimensions().fHeight, codec->dimensions().fWidth));
            break;
        default:
            imageSize.SetSizeT(SizeF(codec->dimensions().fWidth, codec->dimensions().fHeight));
    }
    return MakeRefPtr<ImageEncodedInfo>(imageSize, totalFrames);
}

RefPtr<ImageEncodedInfo> ImageEncodedInfo::CreateImageEncodedInfoForSvg(const RefPtr<NG::ImageData>& data)
{
    auto skiaImageData = DynamicCast<SkiaImageData>(data);
    CHECK_NULL_RETURN(skiaImageData, nullptr);
    // TODO: Encode svg
    int32_t totalFrames = 1;
    return MakeRefPtr<ImageEncodedInfo>(SizeF(), totalFrames);
}

RefPtr<CanvasImage> ImageProvider::QueryCanvasImageFromCache(
    const RefPtr<ImageObject> obj, const LoadCallbacks& loadCallbacks, const SizeF& resizeTarget)
{
    RefPtr<NG::CanvasImage> canvasImage;
    auto pipelineCtx = PipelineContext::GetCurrentContext();
    if (!pipelineCtx || !pipelineCtx->GetImageCache()) {
        return nullptr;
    }
    auto key = GenerateCacheKey(obj->GetSourceInfo(), resizeTarget);
    auto cacheImage =
        pipelineCtx->GetImageCache()->GetCacheImage(key);
    CHECK_NULL_RETURN(cacheImage, nullptr);
#ifdef NG_BUILD
    canvasImage = cacheImage->imagePtr;
#else
    auto flutterCanvasImage = cacheImage->imagePtr;
    canvasImage = CanvasImage::Create(&flutterCanvasImage);
#endif
    LOGD("[ImageCache][CanvasImage] succeed find canvas image from cache: %{public}s", key.c_str());
    return canvasImage;
}

void ImageProvider::MakeCanvasImage(const WeakPtr<ImageObject>& imageObjWp, const LoadCallbacks& loadCallbacks,
    const SizeF& resizeTarget, const RefPtr<RenderTaskHolder>& renderTaskHolder, bool forceResize)
{
    // Query [CanvasImage] from cache, if hit, notify load success immediately
    auto obj = imageObjWp.Upgrade();
    CHECK_NULL_VOID(obj);
    auto canvasImage = ImageProvider::QueryCanvasImageFromCache(obj, loadCallbacks, resizeTarget);
    if (canvasImage) {
        obj->SetCanvasImage(canvasImage);
        loadCallbacks.loadSuccessCallback_(obj->GetSourceInfo());
        return;
    }
    // If [CanvasImage] not in cache, post task to make canvas image
    auto canvasImageMakingTask = [objWp = imageObjWp, loadCallbacks, resizeTarget, renderTaskHolder, forceResize] {
        auto obj = objWp.Upgrade();
        CHECK_NULL_VOID(obj);
        CHECK_NULL_VOID(renderTaskHolder);
        auto flutterRenderTaskHolder = DynamicCast<FlutterRenderTaskHolder>(renderTaskHolder);
        CHECK_NULL_VOID(flutterRenderTaskHolder);
        ImageProvider::PrepareImageData(obj, loadCallbacks);
        // resize image
        auto skiaImageData = DynamicCast<SkiaImageData>(obj->GetData());
        ACE_DCHECK(skiaImageData);
        auto rawImage = SkImage::MakeFromEncoded(skiaImageData->GetSkData());
        if (!rawImage) {
            LOGE(
                "static image MakeFromEncoded fail! source info: %{private}s", obj->GetSourceInfo().ToString().c_str());
            std::string errorMessage("The image format is not supported, please check image format.");
            auto notifyLoadFailTask = [errorMsg = std::move(errorMessage), loadCallbacks,
                                          sourceInfo = obj->GetSourceInfo()] {
                loadCallbacks.loadFailCallback_(sourceInfo, errorMsg, ImageLoadingCommand::MAKE_CANVAS_IMAGE_FAIL);
            };
            ImageProvider::WrapTaskAndPostToUI(std::move(notifyLoadFailTask));
            return;
        }
        // upload to gpu for render
        auto key = ImageObject::GenerateCacheKey(obj->GetSourceInfo(), resizeTarget);
        sk_sp<SkImage> image = rawImage;
        auto compressFileData = ImageLoader::LoadImageDataFromFileCache(key, ".astc");
        if (!compressFileData) {
            image = ResizeSkImage(rawImage, obj->GetSourceInfo().GetSrc(), resizeTarget, forceResize);
        }
        flutter::SkiaGPUObject<SkImage> skiaGpuObjSkImage({ image, flutterRenderTaskHolder->unrefQueue });
#ifdef NG_BUILD
        auto canvasImage = CanvasImage::Create();
        auto flutterImage = AceType::DynamicCast<NG::FlutterCanvasImage>(canvasImage);
        if (flutterImage) {
            flutterImage->SetImage(std::move(skiaGpuObjSkImage));
        }
#else
        auto flutterCanvasImage = flutter::CanvasImage::Create();
        flutterCanvasImage->set_image(std::move(skiaGpuObjSkImage));
        auto canvasImage = CanvasImage::Create(&flutterCanvasImage);
        ImageProvider::CacheCanvasImageToImageCache(canvasImage, GenerateCacheKey(obj->GetSourceInfo(), resizeTarget));
#endif
        auto uploadTask = [objWp, loadCallbacks](const RefPtr<CanvasImage>& canvasImage) {
            // when upload success, update canvas image to ImageObject and trigger loadSuccessCallback_
            auto obj = objWp.Upgrade();
            CHECK_NULL_VOID(obj);
            auto notifyLoadSuccessTask = [objWp, loadCallbacks, canvasImage] {
                auto obj = objWp.Upgrade();
                CHECK_NULL_VOID(obj);
                obj->SetCanvasImage(canvasImage);
                obj->SetData(nullptr); // clear raw image data
                loadCallbacks.loadSuccessCallback_(obj->GetSourceInfo());
            };
            ImageProvider::WrapTaskAndPostToUI(std::move(notifyLoadSuccessTask));
        };
        ImageProvider::UploadImageToGPUForRender(canvasImage, std::move(uploadTask), renderTaskHolder,
            key, resizeTarget, compressFileData);
    };
    // TODO: add sync load
    ImageProvider::WrapTaskAndPostToBackground(std::move(canvasImageMakingTask));
}

RefPtr<RenderTaskHolder> ImageProvider::CreateRenderTaskHolder()
{
    if (!CheckThread(TaskExecutor::TaskType::UI)) {
        LOGE("RenderTaskHolder must be initialized on UI thread, please check.");
        return nullptr;
    }
#ifdef NG_BUILD
    int32_t id = Container::CurrentId();
    auto currentState = flutter::ace::WindowManager::GetWindow(id);
#else
    auto currentState = flutter::UIDartState::Current();
#endif
    CHECK_NULL_RETURN(currentState, nullptr);
    return MakeRefPtr<FlutterRenderTaskHolder>(currentState->GetSkiaUnrefQueue(), currentState->GetIOManager(),
        currentState->GetTaskRunners().GetIOTaskRunner());
}

void ImageProvider::UploadImageToGPUForRender(const RefPtr<CanvasImage>& canvasImage,
    std::function<void(RefPtr<CanvasImage>)>&& callback, const RefPtr<RenderTaskHolder>& renderTaskHolder,
    const std::string key, const SizeF& resizeTarget, const RefPtr<ImageData>& data)
{
    callback(canvasImage);
    return;
    CHECK_NULL_VOID(renderTaskHolder);
    auto flutterRenderTaskHolder = DynamicCast<FlutterRenderTaskHolder>(renderTaskHolder);
    CHECK_NULL_VOID(flutterRenderTaskHolder);
#ifdef UPLOAD_GPU_DISABLED
    // If want to dump draw command or gpu disabled, should use CPU image.
    callback(canvasImage);
#else
    auto skiaCanvasImage = DynamicCast<SkiaCanvasImage>(canvasImage);
    CHECK_NULL_VOID(skiaCanvasImage);
    // load compress cache
    if (data) {
        int32_t dstWidth = static_cast<int32_t>(resizeTarget.Width() + 0.5);
        int32_t dstHeight = static_cast<int32_t>(resizeTarget.Height() + 0.5);

        auto skiaImageData = DynamicCast<SkiaImageData>(data);
        CHECK_NULL_VOID(skiaImageData);
        auto skdata = skiaImageData->GetSkData();
        auto stripped = ImageCompressor::StripFileHeader(skdata);
        LOGI("use astc cache %{public}s %{public}d×%{public}d", key.c_str(),
            dstWidth, dstHeight);
        skiaCanvasImage->SetCompressData(stripped, dstWidth, dstHeight);
        skiaCanvasImage->ReplaceSkImage({ nullptr, flutterRenderTaskHolder->unrefQueue });
        callback(skiaCanvasImage);
        return;
    }
    if (!ImageCompressor::GetInstance()->CanCompress()) {
        callback(skiaCanvasImage);
        return;
    }

    auto task = [callback, flutterRenderTaskHolder, skiaCanvasImage,
        id = Container::CurrentId(), src = key] {
        ContainerScope scope(id);
        if (!flutterRenderTaskHolder) {
            LOGW("flutterRenderTaskHolder has been released.");
            return;
        }
        auto skImage = skiaCanvasImage->GetCanvasImage();
        CHECK_NULL_VOID(skImage);
        auto rasterizedImage = skImage->makeRasterImage();
        if (!rasterizedImage) {
            LOGW("Rasterize image failed. callback.");
            callback(skiaCanvasImage);
            return;
        }
        ACE_DCHECK(!rasterizedImage->isTextureBacked());
        SkPixmap pixmap;
        if (!rasterizedImage->peekPixels(&pixmap)) {
            LOGW("Could not peek pixels of image for texture upload.");
            callback(skiaCanvasImage);
            return;
        }
        int32_t width = static_cast<int32_t>(pixmap.width());
        int32_t height = static_cast<int32_t>(pixmap.height());
        if (ImageCompressor::GetInstance()->CanCompress()) {
            auto compressData = ImageCompressor::GetInstance()->GpuCompress(src, pixmap, width, height);
            ImageCompressor::GetInstance()->WriteToFile(src, compressData, { width, height });
            if (compressData) {
                // replace skImage of [CanvasImage] with [rasterizedImage]
                skiaCanvasImage->SetCompressData(compressData, width, height);
                skiaCanvasImage->ReplaceSkImage({ nullptr, flutterRenderTaskHolder->unrefQueue });
            } else {
                skiaCanvasImage->ReplaceSkImage({ rasterizedImage, flutterRenderTaskHolder->unrefQueue });
            }
            auto releaseTask = ImageCompressor::GetInstance()->ScheduleReleaseTask();
            if (flutterRenderTaskHolder->ioTaskRunner) {
                flutterRenderTaskHolder->ioTaskRunner->PostDelayedTask(releaseTask,
                    fml::TimeDelta::FromMilliseconds(ImageCompressor::releaseTimeMs));
            } else {
                ImageProvider::WrapTaskAndPostToBackground(std::move(releaseTask));
            }
        }
        callback(skiaCanvasImage);
        // Trigger purge cpu bitmap resource, after image upload to gpu.
        SkGraphics::PurgeResourceCache();
    };
    ImageProvider::WrapTaskAndPostToBackground(std::move(task));
#endif
}

void ImageProvider::CacheCanvasImageToImageCache(const RefPtr<CanvasImage>& canvasImage, const std::string& key)
{
    auto pipelineCtx = PipelineContext::GetCurrentContext();
    if (!pipelineCtx || !pipelineCtx->GetImageCache()) {
        return;
    }
#ifdef NG_BUILD
    pipelineCtx->GetImageCache()->CacheImage(key, std::make_shared<CachedImage>(canvasImage));
#else
    auto skiaCanvasImage = AceType::DynamicCast<SkiaCanvasImage>(canvasImage);
    CHECK_NULL_VOID(skiaCanvasImage);
    LOGD("[ImageCache][CanvasImage] succeed caching image: %{public}s", key.c_str());
    pipelineCtx->GetImageCache()->CacheImage(
        key, std::make_shared<CachedImage>(skiaCanvasImage->GetFlutterCanvasImage()));
#endif
}

} // namespace OHOS::Ace::NG

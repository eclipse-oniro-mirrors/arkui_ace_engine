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

#include "core/components_ng/render/adapter/skia_canvas_image.h"

#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/components_ng/image_provider/adapter/flutter_image_provider.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/ui/rs_node.h"
#include "render_service_client/core/ui/rs_surface_node.h"
#include "render_service_client/core/ui/rs_ui_director.h"
#endif

namespace OHOS::Ace::NG {

RefPtr<CanvasImage> CanvasImage::Create(void* rawImage)
{
#ifdef NG_BUILD
    return AceType::MakeRefPtr<SkiaCanvasImage>();
#else
    auto* imageRef = reinterpret_cast<fml::RefPtr<flutter::CanvasImage>*>(rawImage);
    CHECK_NULL_RETURN(imageRef, nullptr);

    return AceType::MakeRefPtr<SkiaCanvasImage>(*imageRef);
#endif
}

RefPtr<CanvasImage> CanvasImage::Create(
    const RefPtr<PixelMap>& pixelMap, const RefPtr<RenderTaskHolder>& renderTaskHolder)
{
#ifndef NG_BUILD
    // TODO: use [PixelMap] as data source when rs provides interface like
    // DrawBitmapRect(Media::PixelMap* pixelMap, const Rect& dstRect, const Rect& srcRect, ...)
    // now we make [SkImage] from [PixelMap] and use [drawImageRect] to draw image

    auto flutterRenderTaskHolder = DynamicCast<FlutterRenderTaskHolder>(renderTaskHolder);
    CHECK_NULL_RETURN(flutterRenderTaskHolder, nullptr);

    // Step1: Create SkPixmap
    auto imageInfo = SkiaCanvasImage::MakeSkImageInfoFromPixelMap(pixelMap);
    SkPixmap imagePixmap(imageInfo, reinterpret_cast<const void*>(pixelMap->GetPixels()), pixelMap->GetRowBytes());

    // Step2: Create SkImage and draw it, using gpu or cpu
    sk_sp<SkImage> skImage;
    if (!flutterRenderTaskHolder->ioManager) {
        skImage =
            SkImage::MakeFromRaster(imagePixmap, &PixelMap::ReleaseProc, PixelMap::GetReleaseContext(pixelMap));
    } else {
#ifndef GPU_DISABLED
        skImage = SkImage::MakeCrossContextFromPixmap(flutterRenderTaskHolder->ioManager->GetResourceContext().get(),
            imagePixmap, true, imagePixmap.colorSpace(), true);
#else
        // SkImage needs to hold PixelMap shared_ptr
        skImage =
            SkImage::MakeFromRaster(imagePixmap, &PixelMap::ReleaseProc, PixelMap::GetReleaseContext(pixelMap));
#endif
    }
    auto canvasImage = flutter::CanvasImage::Create();
    canvasImage->set_image(flutter::SkiaGPUObject<SkImage>(skImage, flutterRenderTaskHolder->unrefQueue));
    return CanvasImage::Create(&canvasImage);
#else
    return AceType::MakeRefPtr<SkiaCanvasImage>();
#endif
}

SkImageInfo SkiaCanvasImage::MakeSkImageInfoFromPixelMap(const RefPtr<PixelMap>& pixmap)
{
    SkColorType colorType = PixelFormatToSkColorType(pixmap);
    SkAlphaType alphaType = AlphaTypeToSkAlphaType(pixmap);
    sk_sp<SkColorSpace> colorSpace = ColorSpaceToSkColorSpace(pixmap);
    return SkImageInfo::Make(pixmap->GetWidth(), pixmap->GetHeight(), colorType, alphaType, colorSpace);
}

sk_sp<SkColorSpace> SkiaCanvasImage::ColorSpaceToSkColorSpace(const RefPtr<PixelMap>& pixmap)
{
    return SkColorSpace::MakeSRGB(); // Media::PixelMap has not support wide gamut yet.
}

SkAlphaType SkiaCanvasImage::AlphaTypeToSkAlphaType(const RefPtr<PixelMap>& pixmap)
{
    switch (pixmap->GetAlphaType()) {
        case AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN:
            return SkAlphaType::kUnknown_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_OPAQUE:
            return SkAlphaType::kOpaque_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_PREMUL:
            return SkAlphaType::kPremul_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL:
            return SkAlphaType::kUnpremul_SkAlphaType;
        default:
            return SkAlphaType::kUnknown_SkAlphaType;
    }
}

SkColorType SkiaCanvasImage::PixelFormatToSkColorType(const RefPtr<PixelMap>& pixmap)
{
    switch (pixmap->GetPixelFormat()) {
        case PixelFormat::RGB_565:
            return SkColorType::kRGB_565_SkColorType;
        case PixelFormat::RGBA_8888:
            return SkColorType::kRGBA_8888_SkColorType;
        case PixelFormat::BGRA_8888:
            return SkColorType::kBGRA_8888_SkColorType;
        case PixelFormat::ALPHA_8:
            return SkColorType::kAlpha_8_SkColorType;
        case PixelFormat::RGBA_F16:
            return SkColorType::kRGBA_F16_SkColorType;
        case PixelFormat::UNKNOWN:
        case PixelFormat::ARGB_8888:
        case PixelFormat::RGB_888:
        case PixelFormat::NV21:
        case PixelFormat::NV12:
        case PixelFormat::CMYK:
        default:
            return SkColorType::kUnknown_SkColorType;
    }
}

void SkiaCanvasImage::ReplaceSkImage(flutter::SkiaGPUObject<SkImage> newSkGpuObjSkImage)
{
#ifndef NG_BUILD
    image_->set_image(std::move(newSkGpuObjSkImage));
#endif
}

int32_t SkiaCanvasImage::GetWidth() const
{
#ifdef NG_BUILD
    return 0;
#else
    return image_->image() ? image_->width() : image_->compressWidth();
#endif
}

int32_t SkiaCanvasImage::GetHeight() const
{
#ifdef NG_BUILD
    return 0;
#else
    return image_->image() ? image_->height() : image_->compressHeight();
#endif
}

void SkiaCanvasImage::DrawToRSCanvas(RSCanvas& canvas, const RSRect& srcRect, const RSRect& dstRect)
{
    auto image = GetCanvasImage();
    RSImage rsImage(&image);
    RSSamplingOptions options;

#ifdef ENABLE_ROSEN_BACKEND
    auto rsCanvas = canvas.GetImpl<RSSkCanvas>();
    if (rsCanvas == nullptr) {
        canvas.DrawImageRect(rsImage, srcRect, dstRect, options);
        return;
    }
    auto skCanvas = rsCanvas->ExportSkCanvas();
    if (skCanvas == nullptr) {
        canvas.DrawImageRect(rsImage, srcRect, dstRect, options);
        return;
    }
    auto recordingCanvas = static_cast<OHOS::Rosen::RSRecordingCanvas*>(skCanvas);
    if (recordingCanvas == nullptr) {
        canvas.DrawImageRect(rsImage, srcRect, dstRect, options);
        return;
    }
    SkPaint paint;
    SkVector radii[4] = { { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
    Rosen::RsImageInfo rsImageInfo(
        (int)(imagePaintConfig_->imageFit_),
        (int)(imagePaintConfig_->imageRepeat_),
        radii,
        1.0,
        GetUniqueID(),
        GetCompressWidth(),
        GetCompressHeight()
    );
    auto data = GetCompressData();
    recordingCanvas->DrawImageWithParm(image, std::move(data), rsImageInfo, paint);
#else
    canvas.DrawImageRect(rsImage, srcRect, dstRect, options);
#endif
}

} // namespace OHOS::Ace::NG

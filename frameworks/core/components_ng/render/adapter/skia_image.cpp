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

#include "core/components_ng/render/adapter/skia_image.h"

#include <utility>

#include "image_painter_utils.h"
#include "include/core/SkColorFilter.h"

#include "base/image/pixel_map.h"
#include "core/components_ng/render/drawing.h"
#ifdef ENABLE_ROSEN_BACKEND
#include "pipeline/rs_recording_canvas.h"
#endif

namespace OHOS::Ace::NG {
namespace {
SkPixmap CloneSkPixmap(SkPixmap& srcPixmap, const std::unique_ptr<uint8_t[]>& dstPixels)
{
    // Media::PixelMap::Create only accepts BGRA ColorType pixmap, have to clone and change ColorType.
    SkImageInfo dstImageInfo = SkImageInfo::Make(srcPixmap.info().width(), srcPixmap.info().height(),
        SkColorType::kBGRA_8888_SkColorType, srcPixmap.alphaType());
    SkPixmap dstPixmap(dstImageInfo, dstPixels.get(), srcPixmap.rowBytes());

    SkBitmap dstBitmap;
    if (!dstBitmap.installPixels(dstPixmap)) {
        return dstPixmap;
    }
    if (!dstBitmap.writePixels(srcPixmap, 0, 0)) {
        return dstPixmap;
    }
    return dstPixmap;
}
} // namespace

RefPtr<CanvasImage> CanvasImage::Create(void* rawImage)
{
    auto* skImagePtr = reinterpret_cast<sk_sp<SkImage>*>(rawImage);
    return AceType::MakeRefPtr<SkiaImage>(*skImagePtr);
}

RefPtr<CanvasImage> CanvasImage::Create()
{
    return AceType::MakeRefPtr<SkiaImage>();
}

SkImageInfo SkiaImage::MakeSkImageInfoFromPixelMap(const RefPtr<PixelMap>& pixmap)
{
    SkColorType colorType = PixelFormatToSkColorType(pixmap);
    SkAlphaType alphaType = AlphaTypeToSkAlphaType(pixmap);
    sk_sp<SkColorSpace> colorSpace = ColorSpaceToSkColorSpace(pixmap);
    return SkImageInfo::Make(pixmap->GetWidth(), pixmap->GetHeight(), colorType, alphaType, colorSpace);
}

sk_sp<SkColorSpace> SkiaImage::ColorSpaceToSkColorSpace(const RefPtr<PixelMap>& pixmap)
{
    return SkColorSpace::MakeSRGB(); // Media::PixelMap has not support wide gamut yet.
}

SkAlphaType SkiaImage::AlphaTypeToSkAlphaType(const RefPtr<PixelMap>& pixmap)
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

SkColorType SkiaImage::PixelFormatToSkColorType(const RefPtr<PixelMap>& pixmap)
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

void SkiaImage::ReplaceSkImage(sk_sp<SkImage> newSkImage)
{
    image_ = std::move(newSkImage);
}

int32_t SkiaImage::GetWidth() const
{
    return image_ ? image_->width() : compressWidth_;
}

int32_t SkiaImage::GetHeight() const
{
    return image_ ? image_->height() : compressHeight_;
}

RefPtr<CanvasImage> SkiaImage::Clone()
{
    auto clone = MakeRefPtr<SkiaImage>(image_);
    clone->uniqueId_ = uniqueId_;
    return clone;
}

RefPtr<PixelMap> SkiaImage::GetPixelMap()
{
    CHECK_NULL_RETURN(GetImage(), nullptr);
    auto rasterImage = GetImage()->makeRasterImage();
    SkPixmap srcPixmap;
    if (!rasterImage->peekPixels(&srcPixmap)) {
        return nullptr;
    }
    auto dstPixels = std::make_unique<uint8_t[]>(srcPixmap.computeByteSize());
    SkPixmap newSrcPixmap = CloneSkPixmap(srcPixmap, dstPixels);
    const auto* addr = newSrcPixmap.addr32();
    auto width = static_cast<int32_t>(newSrcPixmap.width());
    auto height = static_cast<int32_t>(newSrcPixmap.height());
    int32_t length = width * height;
    return PixelMap::ConvertSkImageToPixmap(addr, length, width, height);
}

void SkiaImage::DrawToRSCanvas(
    RSCanvas& canvas, const RSRect& srcRect, const RSRect& dstRect, const BorderRadiusArray& radiusXY)
{
    auto image = GetImage();
    CHECK_NULL_VOID(image || GetCompressData());
    if (!DrawWithRecordingCanvas(canvas, srcRect, dstRect, radiusXY)) {
        RSImage rsImage(&image);
        RSSamplingOptions options;
        ImagePainterUtils::ClipRRect(canvas, dstRect, radiusXY);
        canvas.DrawImageRect(rsImage, srcRect, dstRect, options);
    }
}

bool SkiaImage::DrawWithRecordingCanvas(
    RSCanvas& canvas, const RSRect& srcRect, const RSRect& dstRect, const BorderRadiusArray& radiusXY)
{
#ifdef ENABLE_ROSEN_BACKEND
    auto rsCanvas = canvas.GetImpl<RSSkCanvas>();
    CHECK_NULL_RETURN(rsCanvas, false);
    auto skCanvas = rsCanvas->ExportSkCanvas();
    CHECK_NULL_RETURN(skCanvas, false);
    auto recordingCanvas = static_cast<Rosen::RSRecordingCanvas*>(skCanvas);
    CHECK_NULL_RETURN(recordingCanvas, false);

    SkPaint paint;
    auto config = GetPaintConfig();
#ifndef NEW_SKIA
    ImagePainterUtils::AddFilter(paint, config);
#else
    SkSamplingOptions options;
    ImagePainterUtils::AddFilter(paint, options, config);
#endif
    auto radii = ImagePainterUtils::ToSkRadius(radiusXY);
    recordingCanvas->ClipAdaptiveRRect(radii.get());
    if (config.imageFit_ == ImageFit::TOP_LEFT) {
        SkAutoCanvasRestore acr(recordingCanvas, true);
        auto skSrcRect = SkRect::MakeXYWH(srcRect.GetLeft(), srcRect.GetTop(), srcRect.GetWidth(), srcRect.GetHeight());
        auto skDstRect = SkRect::MakeXYWH(dstRect.GetLeft(), dstRect.GetTop(), dstRect.GetWidth(), dstRect.GetHeight());
        recordingCanvas->concat(SkMatrix::MakeRectToRect(skSrcRect, skDstRect, SkMatrix::kFill_ScaleToFit));
    }
    recordingCanvas->scale(config.scaleX_, config.scaleY_);

    Rosen::RsImageInfo rsImageInfo((int)(config.imageFit_), (int)(config.imageRepeat_), radii.get(), 1.0, GetUniqueID(),
        GetCompressWidth(), GetCompressHeight());
    auto data = GetCompressData();
#ifndef NEW_SKIA
    recordingCanvas->DrawImageWithParm(GetImage(), std::move(data), rsImageInfo, paint);
#else
    // TODO:Haw to set SamplingOptions?
    recordingCanvas->DrawImageWithParm(GetImage(), std::move(data), rsImageInfo, options, paint);
#endif
    return true;
#else // !ENABLE_ROSEN_BACKEND
    return false;
#endif
}
} // namespace OHOS::Ace::NG

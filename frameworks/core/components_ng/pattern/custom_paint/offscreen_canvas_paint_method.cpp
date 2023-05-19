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

#include "core/components_ng/pattern/custom_paint/offscreen_canvas_paint_method.h"

#include "txt/paragraph_builder.h"
#include "txt/paragraph_style.h"
#include "include/core/SkMaskFilter.h"
#include "core/components/common/properties/paint_state.h"
#ifndef NEW_SKIA
#include "include/effects/SkBlurImageFilter.h"
#else
#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"
#endif
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/utils/SkBase64.h"

#include "base/geometry/ng/offset_t.h"
#include "base/i18n/localization.h"
#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#ifndef NEW_SKIA
#include "core/components/common/painter/flutter_decoration_painter.h"
#endif
#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/font/constants_converter.h"
#ifndef NEW_SKIA
#include "core/components/font/flutter_font_collection.h"
#endif
#include "core/components/font/rosen_font_collection.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double HANGING_PERCENT = 0.8;
constexpr double DEFAULT_QUALITY = 0.92;
constexpr int32_t MAX_LENGTH = 2048 * 2048;
const std::string UNSUPPORTED = "data:image/png";
const std::string URL_PREFIX = "data:";
const std::string URL_SYMBOL = ";base64,";
const std::string IMAGE_PNG = "image/png";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_WEBP = "image/webp";
// If args is empty or invalid format, use default: image/png
std::string GetMimeType(const std::string& args)
{
    std::string type = args;
    for (size_t i = 0; i < type.size(); ++i) {
        type[i] = static_cast<uint8_t>(tolower(type[i]));
    }
    return type;
}

// Quality need between 0.0 and 1.0 for MimeType jpeg and webp
double GetQuality(const std::string& args, const double quality)
{
    std::string type = args;
    auto mimeType = GetMimeType(type);
    if (mimeType != IMAGE_JPEG && mimeType != IMAGE_WEBP) {
        return DEFAULT_QUALITY;
    }
    if (quality < 0.0 || quality > 1.0) {
        return DEFAULT_QUALITY;
    }
    return quality;
}
} // namespace

OffscreenCanvasPaintMethod::OffscreenCanvasPaintMethod(
    const WeakPtr<PipelineBase> context, int32_t width, int32_t height)
{
    antiAlias_ = true;
    context_ = context;
    width_ = width;
    height_ = height;

    auto imageInfo =
        SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kUnpremul_SkAlphaType);
    canvasCache_.allocPixels(imageInfo);
    cacheBitmap_.allocPixels(imageInfo);
    canvasCache_.eraseColor(SK_ColorTRANSPARENT);
    cacheBitmap_.eraseColor(SK_ColorTRANSPARENT);
    skCanvas_ = std::make_unique<SkCanvas>(canvasCache_);
    cacheCanvas_ = std::make_unique<SkCanvas>(cacheBitmap_);

    imageShadow_ = std::make_unique<Shadow>();
    InitImageCallbacks();
}

void OffscreenCanvasPaintMethod::ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj)
{
    if (imageObj->IsSvg()) {
        skiaDom_ = AceType::DynamicCast<SvgSkiaImageObject>(imageObj)->GetSkiaDom();
        currentSource_ = loadingSource_;
    } else {
        LOGE("image is not svg");
    }
}

void OffscreenCanvasPaintMethod::ImageObjFailed()
{
    loadingSource_.SetSrc("");
    currentSource_.SetSrc("");
    skiaDom_ = nullptr;
}

void OffscreenCanvasPaintMethod::DrawImage(
    PaintWrapper* paintWrapper, const Ace::CanvasImage& canvasImage, double width, double height)
{
    std::string::size_type tmp = canvasImage.src.find(".svg");
    if (tmp != std::string::npos) {
        DrawSvgImage(paintWrapper, canvasImage);
        return;
    }

    auto image = GreatOrEqual(width, 0) && GreatOrEqual(height, 0)
                     ? Ace::ImageProvider::GetSkImage(canvasImage.src, context_, Size(width, height))
                     : Ace::ImageProvider::GetSkImage(canvasImage.src, context_);
    CHECK_NULL_VOID(image);
    InitPaintBlend(cachePaint_);
    const auto skCanvas =
        globalState_.GetType() == CompositeOperation::SOURCE_OVER ? skCanvas_.get() : cacheCanvas_.get();
#ifndef NEW_SKIA
    InitImagePaint(imagePaint_);
#else
    InitImagePaint(imagePaint_, sampleOptions_);
#endif
    if (HasImageShadow()) {
        SkRect skRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
        SkPath path;
        path.addRect(skRect);
        RosenDecorationPainter::PaintShadow(path, *imageShadow_, skCanvas);
    }
    switch (canvasImage.flag) {
        case 0:
            skCanvas->drawImage(image, canvasImage.dx, canvasImage.dy);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
#ifndef NEW_SKIA
            skCanvas->drawImageRect(image, rect, &imagePaint_);
#else
            skCanvas->drawImageRect(image, rect, sampleOptions_, &imagePaint_);
#endif
            break;
        }
        case 2: {
            SkRect dstRect =
                SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect =
                SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
#ifndef NEW_SKIA
            skCanvas->drawImageRect(image, srcRect, dstRect, &imagePaint_);
#else
            skCanvas->drawImageRect(
                image, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kFast_SrcRectConstraint);
#endif
            break;
        }
        default:
            break;
    }
    if (globalState_.GetType() != CompositeOperation::SOURCE_OVER) {
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, sampleOptions_, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void OffscreenCanvasPaintMethod::DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& canvasImage)
{
    // get skImage form pixelMap
    auto imageInfo = Ace::ImageProvider::MakeSkImageInfoFromPixelMap(pixelMap);
    SkPixmap imagePixmap(imageInfo, reinterpret_cast<const void*>(pixelMap->GetPixels()), pixelMap->GetRowBytes());

    // Step2: Create SkImage and draw it, using gpu or cpu
    sk_sp<SkImage> image;

    image = SkImage::MakeFromRaster(imagePixmap, &PixelMap::ReleaseProc, PixelMap::GetReleaseContext(pixelMap));
    CHECK_NULL_VOID(image);

    InitPaintBlend(cachePaint_);
    const auto skCanvas =
        globalState_.GetType() == CompositeOperation::SOURCE_OVER ? skCanvas_.get() : cacheCanvas_.get();
#ifndef NEW_SKIA
    InitImagePaint(imagePaint_);
#else
    InitImagePaint(imagePaint_, sampleOptions_);
#endif
    switch (canvasImage.flag) {
        case 0:
            skCanvas->drawImage(image, canvasImage.dx, canvasImage.dy);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
#ifndef NEW_SKIA
            skCanvas->drawImageRect(image, rect, &imagePaint_);
#else
            skCanvas->drawImageRect(image, rect, sampleOptions_, &imagePaint_);
#endif
            break;
        }
        case 2: {
            SkRect dstRect =
                SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect =
                SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
#ifndef NEW_SKIA
            skCanvas->drawImageRect(image, srcRect, dstRect, &imagePaint_);
#else
            skCanvas->drawImageRect(
                image, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kFast_SrcRectConstraint);
#endif
            break;
        }
        default:
            break;
    }
    if (globalState_.GetType() != CompositeOperation::SOURCE_OVER) {
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, sampleOptions_, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

std::unique_ptr<Ace::ImageData> OffscreenCanvasPaintMethod::GetImageData(
    double left, double top, double width, double height)
{
    double viewScale = 1.0;
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, std::unique_ptr<Ace::ImageData>());
    viewScale = context->GetViewScale();

    // copy the bitmap to tempCanvas
    auto imageInfo =
        SkImageInfo::Make(width, height, SkColorType::kBGRA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
    double scaledLeft = left * viewScale;
    double scaledTop = top * viewScale;
    double dirtyWidth = width >= 0 ? width : 0;
    double dirtyHeight = height >= 0 ? height : 0;
    int32_t size = dirtyWidth * dirtyHeight;
    auto srcRect = SkRect::MakeXYWH(scaledLeft, scaledTop, width * viewScale, height * viewScale);
    auto dstRect = SkRect::MakeXYWH(0.0, 0.0, dirtyWidth, dirtyHeight);
    SkBitmap tempCache;
    tempCache.allocPixels(imageInfo);
    SkCanvas tempCanvas(tempCache);
#if defined(USE_SYSTEM_SKIA_S) || defined (NEW_SKIA)
    tempCanvas.drawImageRect(
        canvasCache_.asImage(), srcRect, dstRect, SkSamplingOptions(), nullptr, SkCanvas::kFast_SrcRectConstraint);
#else
    tempCanvas.drawBitmapRect(canvasCache_, srcRect, dstRect, nullptr);
#endif
    // write color
    std::unique_ptr<uint8_t[]> pixels = std::make_unique<uint8_t[]>(size * 4);
    tempCanvas.readPixels(imageInfo, pixels.get(), dirtyWidth * imageInfo.bytesPerPixel(), 0, 0);
    std::unique_ptr<Ace::ImageData> imageData = std::make_unique<Ace::ImageData>();
    imageData->dirtyWidth = dirtyWidth;
    imageData->dirtyHeight = dirtyHeight;
    // a pixel include 4 data blue, green, red, alpha
    for (int i = 0; i < size * 4; i += 4) {
        auto blue = pixels[i];
        auto green = pixels[i + 1];
        auto red = pixels[i + 2];
        auto alpha = pixels[i + 3];
        imageData->data.emplace_back(Color::FromARGB(alpha, red, green, blue));
    }
    return imageData;
}

void OffscreenCanvasPaintMethod::FillText(const std::string& text, double x, double y, const PaintState& state)
{
    if (!UpdateOffParagraph(text, false, state, HasShadow())) {
        return;
    }
    PaintText(text, x, y, false, HasShadow());
}

void OffscreenCanvasPaintMethod::StrokeText(const std::string& text, double x, double y, const PaintState& state)
{
    if (HasShadow()) {
        if (!UpdateOffParagraph(text, true, state, true)) {
            return;
        }
        PaintText(text, x, y, true, true);
    }

    if (!UpdateOffParagraph(text, true, state)) {
        return;
    }
    PaintText(text, x, y, true);
}

double OffscreenCanvasPaintMethod::MeasureText(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    style.text_direction = ConvertTxtTextDirection(state.GetOffTextDirection());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetMaxIntrinsicWidth();
}

double OffscreenCanvasPaintMethod::MeasureTextHeight(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    style.text_direction = ConvertTxtTextDirection(state.GetOffTextDirection());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetHeight();
}

TextMetrics OffscreenCanvasPaintMethod::MeasureTextMetrics(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    TextMetrics textMetrics = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    style.text_direction = ConvertTxtTextDirection(state.GetOffTextDirection());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, textMetrics);
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);

    auto textAlign = state.GetTextAlign();
    auto textBaseLine = state.GetTextStyle().GetTextBaseline();
    textMetrics.width = paragraph->GetMaxIntrinsicWidth();
    textMetrics.height = paragraph->GetHeight();
    textMetrics.actualBoundingBoxLeft = -GetAlignOffset(textAlign, paragraph);
    textMetrics.actualBoundingBoxRight = textMetrics.width - textMetrics.actualBoundingBoxLeft;
    textMetrics.actualBoundingBoxAscent = -GetBaselineOffset(textBaseLine, paragraph);
    textMetrics.actualBoundingBoxDescent = textMetrics.height - textMetrics.actualBoundingBoxAscent;
    return textMetrics;
}

void OffscreenCanvasPaintMethod::PaintText(
    const std::string& text, double x, double y, bool isStroke, bool hasShadow)
{
    paragraph_->Layout(width_);
    if (width_ > paragraph_->GetMaxIntrinsicWidth()) {
        paragraph_->Layout(std::ceil(paragraph_->GetMaxIntrinsicWidth()));
    }
    auto align = isStroke ? strokeState_.GetTextAlign() : fillState_.GetTextAlign();
    double dx = x + GetAlignOffset(align, paragraph_);
    auto baseline =
        isStroke ? strokeState_.GetTextStyle().GetTextBaseline() : fillState_.GetTextStyle().GetTextBaseline();
    double dy = y + GetBaselineOffset(baseline, paragraph_);

    if (hasShadow) {
        skCanvas_->save();
        auto shadowOffsetX = shadow_.GetOffset().GetX();
        auto shadowOffsetY = shadow_.GetOffset().GetY();
        paragraph_->Paint(skCanvas_.get(), dx + shadowOffsetX, dy + shadowOffsetY);
        skCanvas_->restore();
        return;
    }
    paragraph_->Paint(skCanvas_.get(), dx, dy);
}

double OffscreenCanvasPaintMethod::GetBaselineOffset(TextBaseline baseline, std::unique_ptr<txt::Paragraph>& paragraph)
{
    double y = 0.0;
    switch (baseline) {
        case TextBaseline::ALPHABETIC:
            y = -paragraph->GetAlphabeticBaseline();
            break;
        case TextBaseline::IDEOGRAPHIC:
            y = -paragraph->GetIdeographicBaseline();
            break;
        case TextBaseline::BOTTOM:
            y = -paragraph->GetHeight();
            break;
        case TextBaseline::TOP:
            y = 0.0;
            break;
        case TextBaseline::MIDDLE:
            y = -paragraph->GetHeight() / 2;
            break;
        case TextBaseline::HANGING:
            y = -HANGING_PERCENT * (paragraph->GetHeight() - paragraph->GetAlphabeticBaseline());
            break;
        default:
            y = -paragraph->GetAlphabeticBaseline();
            break;
    }
    return y;
}

bool OffscreenCanvasPaintMethod::UpdateOffParagraph(const std::string& text, bool isStroke, const PaintState& state, bool hasShadow)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    if (isStroke) {
        style.text_align = ConvertTxtTextAlign(strokeState_.GetTextAlign());
    } else {
        style.text_align = ConvertTxtTextAlign(fillState_.GetTextAlign());
    }
    style.text_direction = ConvertTxtTextDirection(fillState_.GetOffTextDirection());
    style.text_align = GetEffectiveAlign(style.text_align, style.text_direction);
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, false);
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    if (!isStroke && hasShadow) {
        txt::TextShadow txtShadow;
        txtShadow.color = shadow_.GetColor().GetValue();
        txtShadow.offset.fX = shadow_.GetOffset().GetX();
        txtShadow.offset.fY = shadow_.GetOffset().GetY();
#ifndef NEW_SKIA
        txtShadow.blur_radius = shadow_.GetBlurRadius();
#else
        txtShadow.blur_sigma = shadow_.GetBlurRadius();
#endif
        txtStyle.text_shadows.emplace_back(txtShadow);
    }
    txtStyle.locale = Localization::GetInstance()->GetFontLocale();
    UpdateTextStyleForeground(isStroke, txtStyle, hasShadow);
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->Build();
    return true;
}

void OffscreenCanvasPaintMethod::UpdateTextStyleForeground(bool isStroke, txt::TextStyle& txtStyle, bool hasShadow)
{
    using namespace Constants;
    if (!isStroke) {
        txtStyle.color = ConvertSkColor(fillState_.GetColor());
        txtStyle.font_size = fillState_.GetTextStyle().GetFontSize().Value();
        ConvertTxtStyle(fillState_.GetTextStyle(), context_, txtStyle);
        if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
            SkPaint paint;
#ifndef NEW_SKIA
            InitImagePaint(paint);
#else
            SkSamplingOptions options;
            InitImagePaint(paint, options);
#endif
            paint.setStyle(SkPaint::Style::kFill_Style);
            UpdatePaintShader(OffsetF(0, 0), paint, fillState_.GetGradient());
            txtStyle.foreground = paint;
            txtStyle.has_foreground = true;
        }
        if (globalState_.HasGlobalAlpha()) {
            if (txtStyle.has_foreground) {
                txtStyle.foreground.setColor(fillState_.GetColor().GetValue());
                txtStyle.foreground.setAlphaf(globalState_.GetAlpha()); // set alpha after color
            } else {
                SkPaint paint;
#ifndef NEW_SKIA
                InitImagePaint(paint);
#else
                SkSamplingOptions options;
                InitImagePaint(paint, options);
#endif
                paint.setColor(fillState_.GetColor().GetValue());
                paint.setAlphaf(globalState_.GetAlpha()); // set alpha after color
                txtStyle.foreground = paint;
                txtStyle.has_foreground = true;
            }
        }
    } else {
        // use foreground to draw stroke
        SkPaint paint;
#ifndef NEW_SKIA
        GetStrokePaint(paint);
#else
        SkSamplingOptions options;
        GetStrokePaint(paint, options);
#endif
        ConvertTxtStyle(strokeState_.GetTextStyle(), context_, txtStyle);
        txtStyle.font_size = strokeState_.GetTextStyle().GetFontSize().Value();
        if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
            UpdatePaintShader(OffsetF(0, 0), paint, strokeState_.GetGradient());
        }
        if (hasShadow) {
            paint.setColor(shadow_.GetColor().GetValue());
            paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle,
                NG::SkiaDecorationPainter::ConvertRadiusToSigma(shadow_.GetBlurRadius())));
        }
        txtStyle.foreground = paint;
        txtStyle.has_foreground = true;
    }
}

void OffscreenCanvasPaintMethod::PaintShadow(const SkPath& path, const Shadow& shadow, SkCanvas* canvas)
{
    RosenDecorationPainter::PaintShadow(path, shadow, canvas);
}

void OffscreenCanvasPaintMethod::Path2DRect(const OffsetF& offset, const PathArgs& args)
{
    double left = args.para1 + offset.GetX();
    double top = args.para2 + offset.GetY();
    double right = args.para3 + args.para1;
    double bottom = args.para4 + args.para2;
    skPath2d_.addRect(SkRect::MakeLTRB(left, top, right, bottom));
}

void OffscreenCanvasPaintMethod::SetTransform(const TransformParam& param)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    double viewScale = context->GetViewScale();
    SkMatrix skMatrix;
    skMatrix.setAll(param.scaleX * viewScale, param.skewX * viewScale, param.translateX, param.skewY * viewScale,
        param.scaleY * viewScale, param.translateY, 0, 0, 1);
    skCanvas_->setMatrix(skMatrix);
}

std::string OffscreenCanvasPaintMethod::ToDataURL(const std::string& type, const double quality)
{
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, UNSUPPORTED);
    std::string mimeType = GetMimeType(type);
    double qua = GetQuality(type, quality);
    SkBitmap tempCache;
    tempCache.allocPixels(SkImageInfo::Make(width_, height_, SkColorType::kBGRA_8888_SkColorType,
        (mimeType == IMAGE_JPEG) ? SkAlphaType::kOpaque_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType));
    SkCanvas tempCanvas(tempCache);
    double viewScale = context->GetViewScale();
    tempCanvas.clear(SK_ColorTRANSPARENT);
    tempCanvas.scale(1.0 / viewScale, 1.0 / viewScale);
#if defined(USE_SYSTEM_SKIA_S) || defined (NEW_SKIA)
    //The return value of the dual framework interface has no alpha
    tempCanvas.drawImage(canvasCache_.asImage(), 0.0f, 0.0f);
#else
    tempCanvas.drawBitmap(canvasCache_, 0.0f, 0.0f);
#endif
    SkPixmap src;
    bool success = tempCache.peekPixels(&src);
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    SkDynamicMemoryWStream dst;
    if (mimeType == IMAGE_JPEG) {
        SkJpegEncoder::Options options;
        options.fQuality = qua;
        success = SkJpegEncoder::Encode(&dst, src, options);
    } else if (mimeType == IMAGE_WEBP) {
        SkWebpEncoder::Options options;
        options.fQuality = qua * 100.0;
        success = SkWebpEncoder::Encode(&dst, src, options);
    } else {
        mimeType = IMAGE_PNG;
        SkPngEncoder::Options options;
        success = SkPngEncoder::Encode(&dst, src, options);
    }
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    auto result = dst.detachAsData();
    CHECK_NULL_RETURN(result, UNSUPPORTED);
    size_t len = SkBase64::Encode(result->data(), result->size(), nullptr);
    if (len > MAX_LENGTH) {
        return UNSUPPORTED;
    }
    SkString info(len);
    SkBase64::Encode(result->data(), result->size(), info.writable_str());
    return std::string(URL_PREFIX).append(mimeType).append(URL_SYMBOL).append(info.c_str());
}
} // namespace OHOS::Ace::NG

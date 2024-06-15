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

#ifndef ACE_UNITTEST
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/utils/SkBase64.h"

#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/rosen_font_collection.h"
#endif

#include "base/geometry/ng/offset_t.h"
#include "base/i18n/localization.h"
#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/properties/paint_state.h"

namespace OHOS::Ace::NG {
namespace {
#ifndef ACE_UNITTEST
constexpr double DEFAULT_QUALITY = 0.92;
constexpr int32_t MAX_LENGTH = 2048 * 2048;
constexpr double HANGING_PERCENT = 0.8;
#endif
const std::string UNSUPPORTED = "data:image/png";
const std::string URL_PREFIX = "data:";
const std::string URL_SYMBOL = ";base64,";
const std::string IMAGE_PNG = "image/png";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_WEBP = "image/webp";

#ifndef ACE_UNITTEST
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
#endif
} // namespace

OffscreenCanvasPaintMethod::OffscreenCanvasPaintMethod(int32_t width, int32_t height)
{
    antiAlias_ = false;
    matrix_.Reset();
    width_ = width;
    height_ = height;
    lastLayoutSize_.SetWidth(static_cast<float>(width));
    lastLayoutSize_.SetHeight(static_cast<float>(height));
    InitBitmap();
    InitImageCallbacks();
}

void OffscreenCanvasPaintMethod::InitBitmap()
{
    RSBitmapFormat bitmapFormat = { RSColorType::COLORTYPE_RGBA_8888, RSAlphaType::ALPHATYPE_UNPREMUL };
    bitmap_.Build(width_, height_, bitmapFormat);
    bitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    bitmapSize_ = bitmap_.ComputeByteSize();
    rsCanvas_ = std::make_unique<RSCanvas>();
    rsCanvas_->Bind(bitmap_);
}

RefPtr<PixelMap> OffscreenCanvasPaintMethod::TransferToImageBitmap()
{
#ifdef PIXEL_MAP_SUPPORTED
    OHOS::Media::InitializationOptions options;
    options.alphaType = OHOS::Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    options.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    options.scaleMode = OHOS::Media::ScaleMode::CENTER_CROP;
    options.size.width = width_;
    options.size.height = height_;
    options.editable = true;
    auto pixelMap = Ace::PixelMap::Create(OHOS::Media::PixelMap::Create(options));
    if (pixelMap) {
        std::shared_ptr<Ace::ImageData> imageData = std::make_shared<Ace::ImageData>();
        imageData->pixelMap = pixelMap;
        imageData->dirtyX = 0.0f;
        imageData->dirtyY = 0.0f;
        imageData->dirtyWidth = width_;
        imageData->dirtyHeight = height_;
        GetImageData(imageData);
        return pixelMap;
    }
#endif
    return nullptr;
}

void OffscreenCanvasPaintMethod::Reset()
{
    matrix_.Reset();
    ResetStates();
    InitBitmap();
}

void OffscreenCanvasPaintMethod::UpdateSize(int32_t width, int32_t height)
{
    width_ = width;
    height_ = height;
    lastLayoutSize_.SetWidth(static_cast<float>(width));
    lastLayoutSize_.SetHeight(static_cast<float>(height));
    Reset();
}

void OffscreenCanvasPaintMethod::ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj)
{
#ifndef ACE_UNITTEST
    if (imageObj->IsSvg()) {
        skiaDom_ = AceType::DynamicCast<SvgSkiaImageObject>(imageObj)->GetSkiaDom();
        currentSource_ = loadingSource_;
    }
#endif
}

void OffscreenCanvasPaintMethod::ImageObjFailed()
{
#ifndef ACE_UNITTEST
    loadingSource_.SetSrc("");
    currentSource_.SetSrc("");
    skiaDom_ = nullptr;
#endif
}

void OffscreenCanvasPaintMethod::DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& canvasImage)
{
#ifndef ACE_UNITTEST
    // get Image form pixelMap
    CHECK_NULL_VOID(pixelMap);
    auto rsBitmapFormat = Ace::ImageProvider::MakeRSBitmapFormatFromPixelMap(pixelMap);
    auto rsBitmap = std::make_shared<RSBitmap>();
    rsBitmap->Build(pixelMap->GetWidth(), pixelMap->GetHeight(), rsBitmapFormat, pixelMap->GetRowStride());
    rsBitmap->SetPixels(const_cast<void*>(reinterpret_cast<const void*>(pixelMap->GetPixels())));

    // Step2: Create Image and draw it, using gpu or cpu
    auto image = std::make_shared<RSImage>();
    CHECK_NULL_VOID(image->BuildFromBitmap(*rsBitmap));
    DrawImageInternal(canvasImage, image);
#endif
}

std::unique_ptr<Ace::ImageData> OffscreenCanvasPaintMethod::GetImageData(
    double left, double top, double width, double height)
{
    double dirtyWidth = std::abs(width);
    double dirtyHeight = std::abs(height);
    double scaledLeft = left;
    double scaledTop = top;
    if (Negative(width)) {
        scaledLeft += width;
    }
    if (Negative(height)) {
        scaledTop += height;
    }
    // copy the bitmap to tempCanvas
    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888, RSAlphaType::ALPHATYPE_OPAQUE };
    int32_t size = dirtyWidth * dirtyHeight;
    auto srcRect =
        RSRect(scaledLeft, scaledTop, dirtyWidth + scaledLeft, dirtyHeight + scaledTop);
    auto dstRect = RSRect(0.0, 0.0, dirtyWidth, dirtyHeight);
    RSBitmap tempCache;
    tempCache.Build(dirtyWidth, dirtyHeight, format);
    RSCanvas tempCanvas;
    tempCanvas.Bind(tempCache);
    RSImage rsImage;
    rsImage.BuildFromBitmap(bitmap_);
    tempCanvas.DrawImageRect(
        rsImage, srcRect, dstRect, RSSamplingOptions(), RSSrcRectConstraint::FAST_SRC_RECT_CONSTRAINT);
    // write color
    uint8_t* pixels = static_cast<uint8_t*>(tempCache.GetPixels());
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

void OffscreenCanvasPaintMethod::GetImageData(const std::shared_ptr<Ace::ImageData>& imageData)
{
    int32_t dirtyWidth = std::abs(imageData->dirtyWidth);
    int32_t dirtyHeight = std::abs(imageData->dirtyHeight);
    double scaledLeft = imageData->dirtyX;
    double scaledTop = imageData->dirtyY;
    double dx = 0;
    double dy = 0;
    if (Negative(imageData->dirtyWidth)) {
        scaledLeft += imageData->dirtyWidth;
    }
    if (Negative(imageData->dirtyHeight)) {
        scaledTop += imageData->dirtyHeight;
    }
    if (Negative(scaledLeft)) {
        dx = scaledLeft;
    }
    if (Negative(scaledTop)) {
        dy = scaledTop;
    }
    // copy the bitmap to tempCanvas
    RSBitmap subBitmap;
    auto rect = RSRect(scaledLeft, scaledTop, dirtyWidth + scaledLeft, dirtyHeight + scaledTop);
    bool ret = bitmap_.ExtractSubset(subBitmap, rect);
    if (!ret) {
        return;
    }
    auto pixelMap = imageData->pixelMap;
    CHECK_NULL_VOID(pixelMap);
    auto* rawData = pixelMap->GetWritablePixels();
    CHECK_NULL_VOID(rawData);
    RSImageInfo imageInfo =
        RSImageInfo(dirtyWidth, dirtyHeight, RSColorType::COLORTYPE_RGBA_8888, RSAlphaType::ALPHATYPE_PREMUL);
    subBitmap.ReadPixels(imageInfo, rawData, dirtyWidth * imageInfo.GetBytesPerPixel(), dx, dy);
}

void OffscreenCanvasPaintMethod::FillText(
    const std::string& text, double x, double y, std::optional<double> maxWidth, const PaintState& state)
{
    if (!UpdateOffParagraph(text, false, state, HasShadow())) {
        return;
    }
    PaintText(static_cast<float>(width_), x, y, maxWidth, false, HasShadow());
}

void OffscreenCanvasPaintMethod::StrokeText(
    const std::string& text, double x, double y, std::optional<double> maxWidth, const PaintState& state)
{
    if (HasShadow()) {
        if (!UpdateOffParagraph(text, true, state, true)) {
            return;
        }
        PaintText(static_cast<float>(width_), x, y, maxWidth, true, true);
    }

    if (!UpdateOffParagraph(text, true, state)) {
        return;
    }
    PaintText(static_cast<float>(width_), x, y, maxWidth, true);
}

double OffscreenCanvasPaintMethod::MeasureText(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    using namespace Constants;
    RSParagraphStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
    style.textDirection = ConvertTxtTextDirection(state.GetOffTextDirection());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), txtStyle);
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AppendText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->CreateTypography();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetMaxIntrinsicWidth();
#else
    return 0.0;
#endif
}

double OffscreenCanvasPaintMethod::MeasureTextHeight(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    using namespace Constants;
    RSParagraphStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
    style.textDirection = ConvertTxtTextDirection(state.GetOffTextDirection());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), txtStyle);
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AppendText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->CreateTypography();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetHeight();
#else
    return 0.0;
#endif
}

TextMetrics OffscreenCanvasPaintMethod::MeasureTextMetrics(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    using namespace Constants;
    TextMetrics textMetrics;
    RSParagraphStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, textMetrics);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), txtStyle);
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AppendText(StringUtils::Str8ToStr16(text));

    auto paragraph = builder->CreateTypography();
    paragraph->Layout(Size::INFINITE_SIZE);
    /**
     * @brief reference: https://html.spec.whatwg.org/multipage/canvas.html#dom-textmetrics-alphabeticbaseline
     *
     */
    auto fontMetrics = paragraph->MeasureText();
    auto glyphsBoundsTop = paragraph->GetGlyphsBoundsTop();
    auto glyphsBoundsBottom = paragraph->GetGlyphsBoundsBottom();
    auto glyphsBoundsLeft = paragraph->GetGlyphsBoundsLeft();
    auto glyphsBoundsRight = paragraph->GetGlyphsBoundsRight();
    auto textAlign = state.GetTextAlign();
    auto textBaseLine = state.GetTextStyle().GetTextBaseline();
    const double baseLineY = GetFontBaseline(fontMetrics, textBaseLine);
    const double baseLineX = GetFontAlign(textAlign, paragraph);

    textMetrics.width = paragraph->GetMaxIntrinsicWidth();
    textMetrics.height = paragraph->GetHeight();
    textMetrics.actualBoundingBoxAscent = baseLineY - glyphsBoundsTop;
    textMetrics.actualBoundingBoxDescent = glyphsBoundsBottom - baseLineY;
    textMetrics.actualBoundingBoxLeft = baseLineX - glyphsBoundsLeft;
    textMetrics.actualBoundingBoxRight = glyphsBoundsRight - baseLineX;
    textMetrics.alphabeticBaseline = baseLineY;
    textMetrics.ideographicBaseline = baseLineY - fontMetrics.fDescent;
    textMetrics.fontBoundingBoxAscent = baseLineY - fontMetrics.fTop;
    textMetrics.fontBoundingBoxDescent = fontMetrics.fBottom - baseLineY;
    textMetrics.hangingBaseline = baseLineY - (HANGING_PERCENT * fontMetrics.fAscent);
    textMetrics.emHeightAscent = baseLineY - fontMetrics.fAscent;
    textMetrics.emHeightDescent = fontMetrics.fDescent - baseLineY;
    return textMetrics;
#else
    return TextMetrics {};
#endif
}

bool OffscreenCanvasPaintMethod::UpdateOffParagraph(
    const std::string& text, bool isStroke, const PaintState& state, bool hasShadow)
{
#ifndef ACE_UNITTEST
    using namespace Constants;
    RSParagraphStyle style;
    if (isStroke) {
        style.textAlign = ConvertTxtTextAlign(state_.strokeState.GetTextAlign());
    } else {
        style.textAlign = ConvertTxtTextAlign(state_.fillState.GetTextAlign());
    }
    style.textDirection = ConvertTxtTextDirection(state_.fillState.GetOffTextDirection());
    style.textAlign = GetEffectiveAlign(style.textAlign, style.textDirection);
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, false);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    if (!isStroke && hasShadow) {
        Rosen::TextShadow txtShadow;
        txtShadow.color = state_.shadow.GetColor().GetValue();
        txtShadow.offset.SetX(state_.shadow.GetOffset().GetX());
        txtShadow.offset.SetY(state_.shadow.GetOffset().GetY());
        txtShadow.blurRadius = state_.shadow.GetBlurRadius();
        txtStyle.shadows.emplace_back(txtShadow);
    }
    txtStyle.locale = Localization::GetInstance()->GetFontLocale();
    UpdateFontFamilies();
    UpdateTextStyleForeground(isStroke, txtStyle, hasShadow);
    builder->PushStyle(txtStyle);
    builder->AppendText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->CreateTypography();
    return true;
#else
    return false;
#endif
}

void OffscreenCanvasPaintMethod::UpdateTextStyleForeground(bool isStroke, RSTextStyle& txtStyle, bool hasShadow)
{
#ifndef ACE_UNITTEST
    using namespace Constants;
    if (!isStroke) {
        txtStyle.foregroundPen = std::nullopt;
        txtStyle.color = ConvertSkColor(state_.fillState.GetColor());
        txtStyle.fontSize = state_.fillState.GetTextStyle().GetFontSize().Value();
        ConvertTxtStyle(state_.fillState.GetTextStyle(), txtStyle);
        if (state_.fillState.GetGradient().IsValid() && state_.fillState.GetPaintStyle() == PaintStyle::Gradient) {
            RSBrush brush;
            RSSamplingOptions options;
            InitImagePaint(nullptr, &brush, options);
            UpdatePaintShader(nullptr, &brush, state_.fillState.GetGradient());
            txtStyle.foregroundBrush = brush;
        }
        if (state_.globalState.HasGlobalAlpha()) {
            if (txtStyle.foregroundBrush.has_value()) {
                txtStyle.foregroundBrush->SetColor(state_.fillState.GetColor().GetValue());
                txtStyle.foregroundBrush->SetAlphaF(state_.globalState.GetAlpha()); // set alpha after color
            } else {
                RSBrush brush;
                RSSamplingOptions options;
                InitImagePaint(nullptr, &brush, options);
                brush.SetColor(state_.fillState.GetColor().GetValue());
                brush.SetAlphaF(state_.globalState.GetAlpha()); // set alpha after color
                txtStyle.foregroundBrush = brush;
            }
        }
    } else {
        // use foreground to draw stroke
        txtStyle.foregroundPen = std::nullopt;
        RSPen pen;
        RSSamplingOptions options;
        GetStrokePaint(pen, options);
        ConvertTxtStyle(state_.strokeState.GetTextStyle(), txtStyle);
        txtStyle.fontSize = state_.strokeState.GetTextStyle().GetFontSize().Value();
        if (state_.strokeState.GetGradient().IsValid()) {
            UpdatePaintShader(&pen, nullptr, state_.strokeState.GetGradient());
        }
        if (hasShadow) {
            pen.SetColor(state_.shadow.GetColor().GetValue());
            RSFilter filter;
            filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL,
                RosenDecorationPainter::ConvertRadiusToSigma(state_.shadow.GetBlurRadius())));
            pen.SetFilter(filter);
        }
        txtStyle.foregroundPen = pen;
    }
#endif
}

std::string OffscreenCanvasPaintMethod::ToDataURL(const std::string& type, const double quality)
{
#ifndef ACE_UNITTEST
    std::string mimeType = GetMimeType(type);
    double qua = GetQuality(type, quality);
    auto imageInfo = SkImageInfo::Make(width_, height_, SkColorType::kBGRA_8888_SkColorType,
        (mimeType == IMAGE_JPEG) ? SkAlphaType::kOpaque_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType);
    RSBitmap tempCache;
    tempCache.Build(width_, height_,
        { RSColorType::COLORTYPE_BGRA_8888,
            (mimeType == IMAGE_JPEG) ? RSAlphaType::ALPHATYPE_OPAQUE : RSAlphaType::ALPHATYPE_UNPREMUL });
    RSCanvas tempCanvas;
    tempCanvas.Bind(tempCache);
    tempCanvas.Clear(RSColor::COLOR_TRANSPARENT);
    tempCanvas.Scale(1.0, 1.0);
    tempCanvas.DrawBitmap(bitmap_, 0.0f, 0.0f);
    RSPixmap rsSrc;
    bool success = tempCache.PeekPixels(rsSrc);
    SkPixmap src { imageInfo, rsSrc.GetAddr(), rsSrc.GetRowBytes() };
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
#else
    return UNSUPPORTED;
#endif
}

TransformParam OffscreenCanvasPaintMethod::GetTransform() const
{
    TransformParam param;
    if (rsCanvas_ != nullptr) {
        RSMatrix matrix = rsCanvas_->GetTotalMatrix();
        param.scaleX = matrix.Get(RSMatrix::SCALE_X);
        param.scaleY = matrix.Get(RSMatrix::SCALE_Y);
        param.skewX = matrix.Get(RSMatrix::SKEW_X);
        param.skewY = matrix.Get(RSMatrix::SKEW_Y);
        param.translateX = matrix.Get(RSMatrix::TRANS_X);
        param.translateY = matrix.Get(RSMatrix::TRANS_Y);
    }
    return param;
}

LineDashParam OffscreenCanvasPaintMethod::GetLineDash() const
{
    return state_.strokeState.GetLineDash();
}
} // namespace OHOS::Ace::NG

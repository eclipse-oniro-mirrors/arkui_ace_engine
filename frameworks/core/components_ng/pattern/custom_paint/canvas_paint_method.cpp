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

#include "core/components_ng/pattern/custom_paint/canvas_paint_method.h"

#ifndef ACE_UNITTEST
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/utils/SkBase64.h"

#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/rosen_font_collection.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/image/sk_image_cache.h"
#endif

#include "base/i18n/localization.h"
#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components_ng/image_provider/image_object.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_op.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {
namespace {
#ifndef ACE_UNITTEST
constexpr double DEFAULT_QUALITY = 0.92;
constexpr int32_t MAX_LENGTH = 2048 * 2048;
constexpr double HANGING_PERCENT = 0.8;
#endif
constexpr int32_t DEFAULT_SAVE_COUNT = 1;
const std::string UNSUPPORTED = "data:image/png";
const std::string URL_PREFIX = "data:";
const std::string URL_SYMBOL = ";base64,";
const std::string IMAGE_PNG = "image/png";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_WEBP = "image/webp";

#ifndef ACE_UNITTEST
std::string GetMimeType(const std::string& args)
{
    // Args example: ["image/png"]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, ',', values);
    if (values.size() > 2) {
        return IMAGE_PNG;
    }
    // Convert to lowercase string.
    for (size_t i = 0; i < values[0].size(); ++i) {
        values[0][i] = static_cast<uint8_t>(tolower(values[0][i]));
    }
    return values[0];
}

// Quality need between 0.0 and 1.0 for MimeType jpeg and webp
double GetQuality(const std::string& args)
{
    // Args example: ["image/jpeg", 0.8]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, ',', values);
    if (values.size() < 2) {
        return DEFAULT_QUALITY;
    }
    auto mimeType = GetMimeType(args);
    if (mimeType != IMAGE_JPEG && mimeType != IMAGE_WEBP) {
        return DEFAULT_QUALITY;
    }
    double quality = StringUtils::StringToDouble(values[1]);
    if (quality < 0.0 || quality > 1.0) {
        return DEFAULT_QUALITY;
    }
    return quality;
}
#endif
} // namespace

#ifndef USE_FAST_TASKPOOL
void CanvasPaintMethod::PushTask(const TaskFunc& task)
{
    tasks_.emplace_back(task);
    if (needMarkDirty_) {
        needMarkDirty_ = false;
        auto host = frameNode_.Upgrade();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}
#endif

bool CanvasPaintMethod::HasTask() const
{
#ifndef USE_FAST_TASKPOOL
    return !tasks_.empty();
#else
    return fastTaskPool_ && !fastTaskPool_->Empty();
#endif
}

void CanvasPaintMethod::FlushTask()
{
#ifndef USE_FAST_TASKPOOL
    TAG_LOGD(AceLogTag::ACE_CANVAS, "There are %{public}zu tasks will be run.", tasks_.size());
    for (auto& task : tasks_) {
        task(*this);
    }
    tasks_.clear();
#else
    CHECK_NULL_VOID(fastTaskPool_);
    fastTaskPool_->Draw(this);
    fastTaskPool_->Reset();
#endif
    needMarkDirty_ = true;
}

void CanvasPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    ACE_SCOPED_TRACE("CanvasPaintMethod::UpdateContentModifier");
    CHECK_NULL_VOID(paintWrapper);
    auto recordingCanvas = std::static_pointer_cast<RSRecordingCanvas>(rsCanvas_);
    CHECK_NULL_VOID(recordingCanvas);
    lastLayoutSize_ = paintWrapper->GetGeometryNode()->GetFrameSize();
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto fontManager = context->GetFontManager();
    if (fontManager) {
        recordingCanvas->SetIsCustomTextType(fontManager->IsDefaultFontChanged());
    }

    if (!HasTask()) {
        return;
    }

    if (onModifierUpdate_) {
        onModifierUpdate_();
    }
    recordingCanvas->Scale(1.0, 1.0);
    FlushTask();
    CHECK_NULL_VOID(contentModifier_);
    contentModifier_->MarkModifierDirty();
}

void CanvasPaintMethod::ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj)
{
#ifndef ACE_UNITTEST
    imageObj_ = imageObj;
    if (!imageObj_->IsSvg()) {
        return;
    }
    skiaDom_ = AceType::DynamicCast<SvgSkiaImageObject>(imageObj_)->GetSkiaDom();
    currentSource_ = loadingSource_;
    Ace::CanvasImage canvasImage = canvasImage_;
#ifndef USE_FAST_TASKPOOL
    TaskFunc func = [canvasImage](CanvasPaintMethod& paintMethod) {
        paintMethod.DrawImage(canvasImage, 0, 0);
    };
    PushTask(func);
#else
    PushTask<DrawImageOp>(canvasImage, 0, 0);
#endif
#endif
}

void CanvasPaintMethod::ImageObjFailed()
{
#ifndef ACE_UNITTEST
    imageObj_ = nullptr;
    skiaDom_ = nullptr;
#endif
}

void CanvasPaintMethod::DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& canvasImage)
{
#ifndef ACE_UNITTEST
    InitImagePaint(nullptr, &imageBrush_, sampleOptions_);
    RSBrush compositeOperationpBrush;
    InitPaintBlend(compositeOperationpBrush);
    RSRect rec = RSRect(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height());
    RSSaveLayerOps layerOps(&rec, &compositeOperationpBrush);
    if (state_.globalState.GetType() != CompositeOperation::SOURCE_OVER) {
        rsCanvas_->SaveLayer(layerOps);
    }
    if (state_.globalState.HasGlobalAlpha()) {
        imageBrush_.SetAlphaF(state_.globalState.GetAlpha());
    }

    if (HasShadow()) {
        RSRect rec = RSRect(canvasImage.dx, canvasImage.dy,
            canvasImage.dx + canvasImage.dWidth, canvasImage.dy + canvasImage.dHeight);
        RSPath path;
        path.AddRect(rec);
        PaintImageShadow(path, state_.shadow, &imageBrush_, nullptr,
            (state_.globalState.GetType() != CompositeOperation::SOURCE_OVER) ? &layerOps : nullptr);
    }
    auto recordingCanvas = std::static_pointer_cast<RSRecordingCanvas>(rsCanvas_);
    CHECK_NULL_VOID(recordingCanvas);
    const std::shared_ptr<Media::PixelMap> tempPixelMap = pixelMap->GetPixelMapSharedPtr();
    CHECK_NULL_VOID(tempPixelMap);
    switch (canvasImage.flag) {
        case 0: {
            RSRect srcRect = RSRect(0, 0, tempPixelMap->GetWidth(), tempPixelMap->GetHeight());
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + tempPixelMap->GetWidth(), canvasImage.dy + tempPixelMap->GetHeight());
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect, sampleOptions_);
            recordingCanvas->DetachBrush();
            break;
        }
        case 1: {
            RSRect srcRect = RSRect(0, 0, tempPixelMap->GetWidth(), tempPixelMap->GetHeight());
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + canvasImage.dWidth, canvasImage.dy + canvasImage.dHeight);
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect, sampleOptions_);
            recordingCanvas->DetachBrush();
            break;
        }
        case 2: {
            RSRect srcRect = RSRect(canvasImage.sx, canvasImage.sy,
                canvasImage.sx + canvasImage.sWidth, canvasImage.sy + canvasImage.sHeight);
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + canvasImage.dWidth, canvasImage.dy + canvasImage.dHeight);
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect,
                sampleOptions_, RSSrcRectConstraint::STRICT_SRC_RECT_CONSTRAINT);
            recordingCanvas->DetachBrush();
            break;
        }
        default:
            break;
    }
    if (state_.globalState.GetType() != CompositeOperation::SOURCE_OVER) {
        rsCanvas_->Restore();
    }
#endif
}

void CanvasPaintMethod::DrawPixelMapWithoutGlobalState(
    const RefPtr<PixelMap>& pixelMap, const Ace::CanvasImage& canvasImage)
{
#ifndef ACE_UNITTEST
    InitImagePaint(nullptr, &imageBrush_, sampleOptions_);
    auto recordingCanvas = std::static_pointer_cast<RSRecordingCanvas>(rsCanvas_);
    CHECK_NULL_VOID(recordingCanvas);
    const std::shared_ptr<Media::PixelMap> tempPixelMap = pixelMap->GetPixelMapSharedPtr();
    CHECK_NULL_VOID(tempPixelMap);
    switch (canvasImage.flag) {
        case 0: {
            RSRect srcRect = RSRect(0, 0, tempPixelMap->GetWidth(), tempPixelMap->GetHeight());
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + tempPixelMap->GetWidth(), canvasImage.dy + tempPixelMap->GetHeight());
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect, sampleOptions_);
            recordingCanvas->DetachBrush();
            break;
        }
        case 1: {
            RSRect srcRect = RSRect(0, 0, tempPixelMap->GetWidth(), tempPixelMap->GetHeight());
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + canvasImage.dWidth, canvasImage.dy + canvasImage.dHeight);
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect, sampleOptions_);
            recordingCanvas->DetachBrush();
            break;
        }
        case 2: {
            RSRect srcRect = RSRect(canvasImage.sx, canvasImage.sy,
                canvasImage.sx + canvasImage.sWidth, canvasImage.sy + canvasImage.sHeight);
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dx + canvasImage.dWidth, canvasImage.dy + canvasImage.dHeight);
            recordingCanvas->AttachBrush(imageBrush_);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, srcRect, dstRect,
                sampleOptions_, RSSrcRectConstraint::STRICT_SRC_RECT_CONSTRAINT);
            recordingCanvas->DetachBrush();
            break;
        }
        default:
            break;
    }
#endif
}

void CanvasPaintMethod::CloseImageBitmap(const std::string& src)
{
#ifndef ACE_UNITTEST
    CHECK_NULL_VOID(imageCache_);
    auto cacheImage = imageCache_->GetCacheImage(src);
    if (cacheImage && cacheImage->imagePtr) {
        imageCache_->ClearCacheImage(src);
    }
#endif
}

std::unique_ptr<Ace::ImageData> CanvasPaintMethod::GetImageData(
    RefPtr<RenderContext> renderContext, double left, double top, double width, double height)
{
    CHECK_NULL_RETURN(renderContext, nullptr);
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
    RSBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return nullptr;
    }

    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888, RSAlphaType::ALPHATYPE_OPAQUE };
    RSBitmap tempCache;
    tempCache.Build(dirtyWidth, dirtyHeight, format);
    int32_t size = dirtyWidth * dirtyHeight;

    RSCanvas tempCanvas;
    tempCanvas.Bind(tempCache);
    auto srcRect = RSRect(scaledLeft, scaledTop, dirtyWidth + scaledLeft, dirtyHeight + scaledTop);
    auto dstRect = RSRect(0.0, 0.0, dirtyWidth, dirtyHeight);
    RSImage rsImage;
    rsImage.BuildFromBitmap(currentBitmap);
    tempCanvas.DrawImageRect(rsImage, srcRect, dstRect, RSSamplingOptions());
    const uint8_t* pixels = static_cast<const uint8_t*>(tempCache.GetPixels());
    CHECK_NULL_RETURN(pixels, nullptr);
    std::unique_ptr<Ace::ImageData> imageData = std::make_unique<Ace::ImageData>();
    imageData->dirtyWidth = dirtyWidth;
    imageData->dirtyHeight = dirtyHeight;
    // a pixel include 4 data(blue, green, red, alpha)
    for (int i = 0; i < size * 4; i += 4) {
        auto blue = pixels[i];
        auto green = pixels[i + 1];
        auto red = pixels[i + 2];
        auto alpha = pixels[i + 3];
        imageData->data.emplace_back(Color::FromARGB(alpha, red, green, blue));
    }
    return imageData;
}

void CanvasPaintMethod::GetImageData(
    const RefPtr<RenderContext>& renderContext, const std::shared_ptr<Ace::ImageData>& imageData)
{
#ifndef ACE_UNITTEST
    auto rosenRenderContext = AceType::DynamicCast<RosenRenderContext>(renderContext);
    CHECK_NULL_VOID(rosenRenderContext);
    CHECK_NULL_VOID(imageData);
    auto dirtyWidth = std::abs(imageData->dirtyWidth);
    auto dirtyHeight = std::abs(imageData->dirtyHeight);
    auto scaledLeft = imageData->dirtyX;
    auto scaledTop = imageData->dirtyY;
    auto dx = 0.0f;
    auto dy = 0.0f;
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

    auto recordingCanvas = std::static_pointer_cast<RSRecordingCanvas>(rsCanvas_);
    CHECK_NULL_VOID(recordingCanvas);
    auto drawCmdList = recordingCanvas->GetDrawCmdList();
    auto rect = RSRect(scaledLeft, scaledTop, dirtyWidth + scaledLeft, dirtyHeight + scaledTop);
    auto pixelMap = imageData->pixelMap;
    CHECK_NULL_VOID(pixelMap);
    auto sharedPixelMap = pixelMap->GetPixelMapSharedPtr();
    auto ret = rosenRenderContext->GetPixelMap(sharedPixelMap, drawCmdList, &rect);
    if (!ret) {
        if (!drawCmdList || drawCmdList->IsEmpty()) {
            return;
        }
        RSBitmap bitmap;
        RSImageInfo info = RSImageInfo(rect.GetWidth(), rect.GetHeight(),
            RSColorType::COLORTYPE_RGBA_8888, RSAlphaType::ALPHATYPE_PREMUL);
        bitmap.InstallPixels(info, pixelMap->GetWritablePixels(), pixelMap->GetRowBytes());
        RSCanvas canvas;
        canvas.Bind(bitmap);
        canvas.Translate(-rect.GetLeft(), -rect.GetTop());
        drawCmdList->Playback(canvas, &rect);
    }
#endif
}

#ifdef PIXEL_MAP_SUPPORTED
void CanvasPaintMethod::TransferFromImageBitmap(const RefPtr<PixelMap>& pixelMap)
{
    CHECK_NULL_VOID(pixelMap);
    Ace::CanvasImage canvasImage;
    canvasImage.flag = 0;
    DrawPixelMapWithoutGlobalState(pixelMap, canvasImage);
}
#endif

void CanvasPaintMethod::FillText(const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    auto success = UpdateParagraph(text, false, HasShadow());
    CHECK_NULL_VOID(success);
    PaintText(lastLayoutSize_.Width(), x, y, maxWidth, false, HasShadow());
}

void CanvasPaintMethod::StrokeText(const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    if (HasShadow()) {
        auto success = UpdateParagraph(text, true, true);
        CHECK_NULL_VOID(success);
        PaintText(lastLayoutSize_.Width(), x, y, maxWidth, true, true);
    }

    auto success = UpdateParagraph(text, true);
    CHECK_NULL_VOID(success);
    PaintText(lastLayoutSize_.Width(), x, y, maxWidth, true);
}

double CanvasPaintMethod::MeasureText(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    RSParagraphStyle style;
    style.textAlign = Constants::ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    Constants::ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
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

double CanvasPaintMethod::MeasureTextHeight(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    RSParagraphStyle style;
    style.textAlign = Constants::ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    Constants::ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
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

TextMetrics CanvasPaintMethod::MeasureTextMetrics(const std::string& text, const PaintState& state)
{
#ifndef ACE_UNITTEST
    TextMetrics textMetrics;
    RSParagraphStyle style;
    style.textAlign = Constants::ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, textMetrics);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::Create(style, fontCollection);
    RSTextStyle txtStyle;
    Constants::ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
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

bool CanvasPaintMethod::UpdateParagraph(const std::string& text, bool isStroke, bool hasShadow)
{
#ifndef ACE_UNITTEST
    RSParagraphStyle style;
    if (isStroke) {
        style.textAlign = Constants::ConvertTxtTextAlign(state_.strokeState.GetTextAlign());
    } else {
        style.textAlign = Constants::ConvertTxtTextAlign(state_.fillState.GetTextAlign());
    }
    style.textDirection = Constants::ConvertTxtTextDirection(state_.fillState.GetOffTextDirection());
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

void CanvasPaintMethod::UpdateTextStyleForeground(bool isStroke, RSTextStyle& txtStyle, bool hasShadow)
{
#ifndef ACE_UNITTEST
    if (!isStroke) {
        txtStyle.foregroundPen = std::nullopt;
        txtStyle.color = Constants::ConvertSkColor(state_.fillState.GetColor());
        txtStyle.fontSize = state_.fillState.GetTextStyle().GetFontSize().Value();
        Constants::ConvertTxtStyle(state_.fillState.GetTextStyle(), context_, txtStyle);
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
                InitPaintBlend(brush);
                txtStyle.foregroundBrush = brush;
            }
        }
    } else {
        // use foreground to draw stroke
        txtStyle.foregroundPen = std::nullopt;
        RSPen pen;
        RSSamplingOptions options;
        GetStrokePaint(pen, options);
        InitPaintBlend(pen);
        Constants::ConvertTxtStyle(state_.strokeState.GetTextStyle(), context_, txtStyle);
        txtStyle.fontSize = state_.strokeState.GetTextStyle().GetFontSize().Value();
        if (state_.strokeState.GetGradient().IsValid() && state_.strokeState.GetPaintStyle() == PaintStyle::Gradient) {
            UpdatePaintShader(&pen, nullptr, state_.strokeState.GetGradient());
        }
        if (hasShadow) {
            pen.SetColor(state_.shadow.GetColor().GetValue());
            RSFilter filter;
            filter.SetMaskFilter(RSRecordingMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL,
                RosenDecorationPainter::ConvertRadiusToSigma(state_.shadow.GetBlurRadius())));
            pen.SetFilter(filter);
        }
        txtStyle.foregroundPen = pen;
    }
#endif
}

std::string CanvasPaintMethod::ToDataURL(RefPtr<RenderContext> renderContext, const std::string& args)
{
#ifndef ACE_UNITTEST
    CHECK_NULL_RETURN(renderContext, UNSUPPORTED);
    std::string mimeType = GetMimeType(args);
    double quality = GetQuality(args);
    double width = lastLayoutSize_.Width();
    double height = lastLayoutSize_.Height();
    auto imageInfo = SkImageInfo::Make(width, height, SkColorType::kBGRA_8888_SkColorType,
        (mimeType == IMAGE_JPEG) ? SkAlphaType::kOpaque_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType);
    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888,
        (mimeType == IMAGE_JPEG) ? RSAlphaType::ALPHATYPE_OPAQUE : RSAlphaType::ALPHATYPE_UNPREMUL };
    RSBitmap tempCache;
    tempCache.Build(width, height, format);
    RSBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return UNSUPPORTED;
    }
    bool success = currentBitmap.GetPixmap().ScalePixels(
        tempCache.GetPixmap(), RSSamplingOptions(RSCubicResampler { 1 / 3.0f, 1 / 3.0f }));
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    RSPixmap rsSrc = tempCache.GetPixmap();
    SkPixmap src { imageInfo, rsSrc.GetAddr(), rsSrc.GetRowBytes() };
    SkDynamicMemoryWStream dst;
    if (mimeType == IMAGE_JPEG) {
        SkJpegEncoder::Options options;
        options.fQuality = quality * 100;
        success = SkJpegEncoder::Encode(&dst, src, options);
    } else if (mimeType == IMAGE_WEBP) {
        SkWebpEncoder::Options options;
        options.fQuality = quality * 100.0;
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

bool CanvasPaintMethod::DrawBitmap(RefPtr<RenderContext> renderContext, RSBitmap& currentBitmap)
{
#ifndef ACE_UNITTEST
    auto recordingCanvas = std::static_pointer_cast<RSRecordingCanvas>(rsCanvas_);
    CHECK_NULL_RETURN(recordingCanvas, false);
    auto rosenRenderContext = AceType::DynamicCast<RosenRenderContext>(renderContext);
    CHECK_NULL_RETURN(rosenRenderContext, false);
    auto drawCmdList = recordingCanvas->GetDrawCmdList();
    CHECK_NULL_RETURN(drawCmdList, false);
    bool res = rosenRenderContext->GetBitmap(currentBitmap, drawCmdList);
    if (res) {
        return true;
    }
    if (drawCmdList->IsEmpty()) {
        return false;
    }
    currentBitmap.Free();
    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888, RSAlphaType::ALPHATYPE_OPAQUE };
    currentBitmap.Build(lastLayoutSize_.Width(), lastLayoutSize_.Height(), format);

    RSCanvas currentCanvas;
    currentCanvas.Bind(currentBitmap);
    drawCmdList->Playback(currentCanvas);
    return true;
#else
    return false;
#endif
}

std::string CanvasPaintMethod::GetJsonData(const std::string& path)
{
#ifndef ACE_UNITTEST
    AssetImageLoader imageLoader;
    return imageLoader.LoadJsonData(path, context_);
#else
    return "";
#endif
}

void CanvasPaintMethod::Reset()
{
    ResetStates();
    CHECK_NULL_VOID(rsCanvas_);
    if (rsCanvas_->GetSaveCount() >= DEFAULT_SAVE_COUNT) {
        rsCanvas_->RestoreToCount(0);
    }
    rsCanvas_->ResetMatrix();
    rsCanvas_->Clear(RSColor::COLOR_TRANSPARENT);
    rsCanvas_->Save();
}
} // namespace OHOS::Ace::NG

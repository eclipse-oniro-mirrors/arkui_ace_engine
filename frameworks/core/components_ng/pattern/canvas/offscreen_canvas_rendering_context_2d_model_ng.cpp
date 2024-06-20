/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/canvas/offscreen_canvas_rendering_context_2d_model_ng.h"

#include <cstring>

#include "securec.h"

#include "core/components/common/properties/paint_state.h"

#ifdef PIXEL_MAP_SUPPORTED
#include "pixel_map.h"
#else
constexpr int BUFFER_SIZE = 4;
constexpr int STEP_1 = 1;
constexpr int STEP_2 = 2;
constexpr int STEP_3 = 3;
#endif

namespace OHOS::Ace::NG {

void OffscreenCanvasRenderingContext2DModelNG::SetPattern(RefPtr<AceType> pattern)
{
    pattern_ = AceType::DynamicCast<OffscreenCanvasPattern>(pattern);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillText(const PaintState& state, const FillTextInfo& fillTextInfo)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->FillText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y, fillTextInfo.maxWidth, state);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokeText(const PaintState& state, const FillTextInfo& fillTextInfo)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->StrokeText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y, fillTextInfo.maxWidth, state);
}

void OffscreenCanvasRenderingContext2DModelNG::SetAntiAlias(bool anti)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetAntiAlias(anti);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFontWeight(const FontWeight& weight)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFontWeight(weight);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFontStyle(const FontStyle& fontStyle)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFontStyle(fontStyle);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFontFamilies(const std::vector<std::string>& families)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFontFamilies(families);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFontSize(const Dimension& size)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFontSize(size);
}

std::vector<double> OffscreenCanvasRenderingContext2DModelNG::GetLineDash()
{
    return pattern_ ? pattern_->GetLineDash().lineDash : std::vector<double> {};
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillGradient(const Ace::Gradient& gradient)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillGradient(gradient);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillPattern(const std::shared_ptr<Ace::Pattern>& pattern)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillPattern(pattern);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillColor(const Color& color, bool colorFlag)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillColor(color);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokeGradient(const Ace::Gradient& gradient)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetStrokeGradient(gradient);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokePattern(const std::shared_ptr<Ace::Pattern>& pattern)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetStrokePattern(pattern);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokeColor(const Color& color, bool colorFlag)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetStrokeColor(color);
}

void OffscreenCanvasRenderingContext2DModelNG::DrawImage(const ImageInfo& imageInfo)
{
    CHECK_NULL_VOID(pattern_);
    if (imageInfo.isImage) {
        pattern_->DrawImage(imageInfo.image, imageInfo.imgWidth, imageInfo.imgHeight);
        return;
    }
    if (imageInfo.isSvg) {
        pattern_->DrawSvgImage(imageInfo.svgDom, imageInfo.image, imageInfo.imageFit);
        return;
    }
    pattern_->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
}

void OffscreenCanvasRenderingContext2DModelNG::PutImageData(const Ace::ImageData& imageData)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->PutImageData(imageData);
}

void OffscreenCanvasRenderingContext2DModelNG::CloseImageBitmap(const std::string& src) {}

std::unique_ptr<Ace::ImageData> OffscreenCanvasRenderingContext2DModelNG::GetImageData(const ImageSize& imageSize)
{
    return pattern_ ? pattern_->GetImageData(imageSize.left, imageSize.top, imageSize.width, imageSize.height)
                    : nullptr;
}

void OffscreenCanvasRenderingContext2DModelNG::DrawPixelMap(const ImageInfo& imageInfo)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFilterParam(const std::string& src)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFilterParam(src);
}

void OffscreenCanvasRenderingContext2DModelNG::SetTextDirection(const TextDirection& direction)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetTextDirection(direction);
}

std::string OffscreenCanvasRenderingContext2DModelNG::GetJsonData(const std::string& path)
{
    return "";
}

std::string OffscreenCanvasRenderingContext2DModelNG::ToDataURL(const std::string& dataUrl, double quality)
{
    return pattern_ ? pattern_->ToDataURL(dataUrl, quality) : "";
}

void OffscreenCanvasRenderingContext2DModelNG::SetLineCap(const LineCapStyle& lineCap)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetLineCap(lineCap);
}

void OffscreenCanvasRenderingContext2DModelNG::SetLineJoin(const LineJoinStyle& lineJoin)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetLineJoin(lineJoin);
}

void OffscreenCanvasRenderingContext2DModelNG::SetMiterLimit(double limit)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetMiterLimit(limit);
}

void OffscreenCanvasRenderingContext2DModelNG::SetLineWidth(double lineWidth)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetLineWidth(lineWidth);
}

void OffscreenCanvasRenderingContext2DModelNG::SetGlobalAlpha(double alpha)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetAlpha(alpha);
}

void OffscreenCanvasRenderingContext2DModelNG::SetCompositeType(const CompositeOperation& type)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetCompositeType(type);
}

void OffscreenCanvasRenderingContext2DModelNG::SetLineDashOffset(double lineDashOffset)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetLineDashOffset(lineDashOffset);
}

void OffscreenCanvasRenderingContext2DModelNG::SetShadowBlur(double blur)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetShadowBlur(blur);
}

void OffscreenCanvasRenderingContext2DModelNG::SetShadowColor(const Color& color)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetShadowColor(color);
}

void OffscreenCanvasRenderingContext2DModelNG::SetShadowOffsetX(double offsetX)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetShadowOffsetX(offsetX);
}

void OffscreenCanvasRenderingContext2DModelNG::SetShadowOffsetY(double offsetY)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetShadowOffsetY(offsetY);
}

void OffscreenCanvasRenderingContext2DModelNG::SetSmoothingEnabled(bool enabled)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetSmoothingEnabled(enabled);
}

void OffscreenCanvasRenderingContext2DModelNG::SetSmoothingQuality(const std::string& quality)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetSmoothingQuality(quality);
}

void OffscreenCanvasRenderingContext2DModelNG::MoveTo(double x, double y)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->MoveTo(x, y);
}

void OffscreenCanvasRenderingContext2DModelNG::LineTo(double x, double y)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->LineTo(x, y);
}

void OffscreenCanvasRenderingContext2DModelNG::BezierCurveTo(const BezierCurveParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->BezierCurveTo(param);
}

void OffscreenCanvasRenderingContext2DModelNG::QuadraticCurveTo(const QuadraticCurveParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->QuadraticCurveTo(param);
}

void OffscreenCanvasRenderingContext2DModelNG::ArcTo(const ArcToParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->ArcTo(param);
}

void OffscreenCanvasRenderingContext2DModelNG::Arc(const ArcParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Arc(param);
}

void OffscreenCanvasRenderingContext2DModelNG::Ellipse(const EllipseParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Ellipse(param);
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillRuleForPath(const CanvasFillRule& fillRule)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillRuleForPath(fillRule);
    pattern_->Fill();
}

void OffscreenCanvasRenderingContext2DModelNG::SetFillRuleForPath2D(
    const CanvasFillRule& fillRule, const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillRuleForPath2D(fillRule);
    pattern_->Fill(path);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokeRuleForPath2D(
    const CanvasFillRule& fillRule, const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(pattern_);

    pattern_->SetFillRuleForPath(fillRule);
    pattern_->Stroke(path);
}

void OffscreenCanvasRenderingContext2DModelNG::SetStrokeRuleForPath(const CanvasFillRule& fillRule)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillRuleForPath(fillRule);
    pattern_->Stroke();
}

void OffscreenCanvasRenderingContext2DModelNG::SetClipRuleForPath(const CanvasFillRule& fillRule)
{
    CHECK_NULL_VOID(pattern_);

    pattern_->SetFillRuleForPath(fillRule);
    pattern_->Clip();
}

void OffscreenCanvasRenderingContext2DModelNG::SetClipRuleForPath2D(
    const CanvasFillRule& fillRule, const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetFillRuleForPath2D(fillRule);
    pattern_->Clip(path);
}

void OffscreenCanvasRenderingContext2DModelNG::AddRect(const Rect& rect)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->AddRect(rect);
}

void OffscreenCanvasRenderingContext2DModelNG::BeginPath()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->BeginPath();
}

void OffscreenCanvasRenderingContext2DModelNG::ClosePath()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->ClosePath();
}

void OffscreenCanvasRenderingContext2DModelNG::Restore()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Restore();
}

void OffscreenCanvasRenderingContext2DModelNG::CanvasRendererSave()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Save();
}

void OffscreenCanvasRenderingContext2DModelNG::CanvasRendererRotate(double angle)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Rotate(angle);
}

void OffscreenCanvasRenderingContext2DModelNG::CanvasRendererScale(double x, double y)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Scale(x, y);
}

void OffscreenCanvasRenderingContext2DModelNG::SetTransform(TransformParam& param, bool lengthFlag)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetTransform(param);
}

void OffscreenCanvasRenderingContext2DModelNG::ResetTransform()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->ResetTransform();
}

void OffscreenCanvasRenderingContext2DModelNG::Transform(const TransformParam& param)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Transform(param);
}

void OffscreenCanvasRenderingContext2DModelNG::Translate(double x, double y)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Translate(x, y);
}

void OffscreenCanvasRenderingContext2DModelNG::SetLineDash(const std::vector<double>& lineDash)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetLineDash(lineDash);
}

void OffscreenCanvasRenderingContext2DModelNG::SetTextAlign(const TextAlign& align)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetTextAlign(align);
}

void OffscreenCanvasRenderingContext2DModelNG::SetTextBaseline(const TextBaseline& baseline)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SetTextBaseline(baseline);
}

double OffscreenCanvasRenderingContext2DModelNG::GetMeasureTextWidth(const PaintState& state, const std::string& text)
{
    return pattern_ ? pattern_->MeasureText(text, state) : 0.0;
}

double OffscreenCanvasRenderingContext2DModelNG::GetMeasureTextHeight(const PaintState& state, const std::string& text)
{
    return pattern_ ? pattern_->MeasureTextHeight(text, state) : 0.0;
}

void OffscreenCanvasRenderingContext2DModelNG::FillRect(const Rect& rect)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->FillRect(rect);
}

void OffscreenCanvasRenderingContext2DModelNG::StrokeRect(const Rect& rect)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->StrokeRect(rect);
}

void OffscreenCanvasRenderingContext2DModelNG::ClearRect(const Rect& rect)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->ClearRect(rect);
}

TransformParam OffscreenCanvasRenderingContext2DModelNG::GetTransform()
{
    return pattern_ ? pattern_->GetTransform() : TransformParam {};
}

RefPtr<Ace::PixelMap> OffscreenCanvasRenderingContext2DModelNG::GetPixelMap(const ImageSize& imageSize)
{
#ifdef PIXEL_MAP_SUPPORTED
    // 1 Get data from canvas
    auto finalHeight = static_cast<uint32_t>(std::abs(imageSize.height));
    auto finalWidth = static_cast<uint32_t>(std::abs(imageSize.width));
    if (finalHeight > 0 && finalWidth > (UINT32_MAX / finalHeight)) {
        return nullptr;
    }

    // 2 Create pixelmap
    OHOS::Media::InitializationOptions options;
    options.alphaType = OHOS::Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    options.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    options.scaleMode = OHOS::Media::ScaleMode::CENTER_CROP;
    options.size.width = static_cast<int32_t>(finalWidth);
    options.size.height = static_cast<int32_t>(finalHeight);
    options.editable = true;
    auto pixelMap = Ace::PixelMap::Create(OHOS::Media::PixelMap::Create(options));
    if (pixelMap) {
        std::shared_ptr<Ace::ImageData> imageData = std::make_shared<Ace::ImageData>();
        imageData->pixelMap = pixelMap;
        imageData->dirtyX = static_cast<int32_t>(imageSize.left);
        imageData->dirtyY = static_cast<int32_t>(imageSize.top);
        imageData->dirtyWidth = static_cast<int32_t>(imageSize.width);
        imageData->dirtyHeight = static_cast<int32_t>(imageSize.height);
        GetImageData(imageData);
    }
    return pixelMap;
#else
    return nullptr;
#endif
}

void OffscreenCanvasRenderingContext2DModelNG::GetImageDataModel(const ImageSize& imageSize, uint8_t* buffer)
{
#ifdef PIXEL_MAP_SUPPORTED
    auto finalHeight = static_cast<uint32_t>(std::abs(imageSize.height));
    auto finalWidth = static_cast<uint32_t>(std::abs(imageSize.width));
    if (finalHeight > 0 && finalWidth > (UINT32_MAX / finalHeight)) {
        return;
    }
    OHOS::Media::InitializationOptions options;
    options.alphaType = OHOS::Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    options.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    options.scaleMode = OHOS::Media::ScaleMode::CENTER_CROP;
    options.size.width = static_cast<int32_t>(finalWidth);
    options.size.height = static_cast<int32_t>(finalHeight);
    options.editable = true;
    auto pixelMap = Ace::PixelMap::Create(OHOS::Media::PixelMap::Create(options));
    if (pixelMap) {
        std::shared_ptr<Ace::ImageData> imageData = std::make_shared<Ace::ImageData>();
        imageData->pixelMap = pixelMap;
        imageData->dirtyX = static_cast<int32_t>(imageSize.left);
        imageData->dirtyY = static_cast<int32_t>(imageSize.top);
        imageData->dirtyWidth = static_cast<int32_t>(imageSize.width);
        imageData->dirtyHeight = static_cast<int32_t>(imageSize.height);
        GetImageData(imageData);
        auto pixelsSize = pixelMap->GetRowBytes() * pixelMap->GetHeight();
        memcpy_s(buffer, pixelsSize, pixelMap->GetWritablePixels(), pixelsSize);
    }
#else
    auto finalHeight = static_cast<uint32_t>(std::abs(imageSize.height));
    auto finalWidth = static_cast<uint32_t>(std::abs(imageSize.width));
    std::unique_ptr<Ace::ImageData> data = GetImageData(imageSize);

    if (data != nullptr) {
        for (uint32_t idx = 0; idx < finalHeight * finalWidth; ++idx) {
            buffer[BUFFER_SIZE * idx] = data->data[idx].GetRed();
            buffer[BUFFER_SIZE * idx + STEP_1] = data->data[idx].GetGreen();
            buffer[BUFFER_SIZE * idx + STEP_2] = data->data[idx].GetBlue();
            buffer[BUFFER_SIZE * idx + STEP_3] = data->data[idx].GetAlpha();
        }
    }
#endif
}

TextMetrics OffscreenCanvasRenderingContext2DModelNG::GetMeasureTextMetrics(
    const PaintState& state, const std::string& text)
{
    return pattern_ ? pattern_->MeasureTextMetrics(text, state) : TextMetrics {};
}

void OffscreenCanvasRenderingContext2DModelNG::GetImageData(const std::shared_ptr<Ace::ImageData>& imageData)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->GetImageData(imageData);
}

void OffscreenCanvasRenderingContext2DModelNG::SaveLayer()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->SaveLayer();
}

void OffscreenCanvasRenderingContext2DModelNG::RestoreLayer()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->RestoreLayer();
}

void OffscreenCanvasRenderingContext2DModelNG::Reset()
{
    CHECK_NULL_VOID(pattern_);
    pattern_->Reset();
}

// All interfaces that only the 'OffscreenCanvasRenderingContext2D' has.
RefPtr<AceType> OffscreenCanvasRenderingContext2DModelNG::CreateOffscreenPattern(int width, int height)
{
    return AceType::MakeRefPtr<OffscreenCanvasPattern>(width, height);
}

size_t OffscreenCanvasRenderingContext2DModelNG::GetBitmapSize(RefPtr<AceType> offscreenPattern)
{
    auto tempPattern = AceType::DynamicCast<OffscreenCanvasPattern>(offscreenPattern);
    return tempPattern ? tempPattern->GetBitmapSize() : 0;
}
} // namespace OHOS::Ace::NG

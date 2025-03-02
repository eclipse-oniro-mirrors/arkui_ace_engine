/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "canvas_renderer_peer_impl.h"

namespace OHOS::Ace::NG {
const std::set<std::string> FONT_WEIGHTS = {
    "100", "200", "300", "400", "500", "600", "700", "800", "900",
    "bold", "bolder", "lighter", "medium", "normal", "regular",
};
const std::set<std::string> FONT_STYLES = { "italic", "oblique", "normal" };
const std::set<std::string> FONT_FAMILIES = { "sans-serif", "serif", "monospace" };
constexpr double MATH_2_PI = 2 * M_PI;
constexpr double DIFF = 1e-10;
constexpr Dimension DEFAULT_FONT_SIZE = 14.0_px;
const std::set<std::string> QUALITY_TYPE = { "low", "medium", "high" };
const std::unordered_map<std::string, LineCapStyle> LINE_CAP_MAP = {
    { "butt", LineCapStyle::BUTT },
    { "round", LineCapStyle::ROUND },
    { "square", LineCapStyle::SQUARE },
};
const std::unordered_map<std::string, LineJoinStyle> LINE_JOIN_MAP = {
    { "bevel", LineJoinStyle::BEVEL },
    { "miter", LineJoinStyle::MITER },
    { "round", LineJoinStyle::ROUND },
};
} // namespace OHOS::Ace::NG
namespace OHOS::Ace::NG::GeneratedModifier {
CanvasRendererPeerImpl::CanvasRendererPeerImpl()
{
    density_ = PipelineBase::GetCurrentDensity();
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_THIRTEEN)) {
        paintState_ = PaintState(TextAlign::START, TextDirection::INHERIT, DEFAULT_FONT_SIZE);
    }
    auto pipeline = PipelineBase::GetCurrentContextSafely();
    if (pipeline) {
        densityCallbackId_ = pipeline->RegisterDensityChangedCallback([self = WeakClaim(this)](double density) {
            auto canvasRender = self.Upgrade();
            CHECK_NULL_VOID(canvasRender);
            canvasRender->density_ = density;
            canvasRender->SetDensity();
        });
    }
}
void CanvasRendererPeerImpl::TriggerBeginPathImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerBeginPathImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->BeginPath();
}
void CanvasRendererPeerImpl::TriggerStroke0Impl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerStroke0Impl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Stroke();
}
void CanvasRendererPeerImpl::TriggerRestoreImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerRestoreImpl pattern "
             "not bound to component.");
        return;
    }
    if (!savePaintState_.empty()) {
        paintState_ = savePaintState_.back();
        savePaintState_.pop_back();
    }
    pattern_->Restore();
}
void CanvasRendererPeerImpl::TriggerSaveImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSaveImpl pattern "
             "not bound to component.");
        return;
    }
    savePaintState_.push_back(paintState_);
    pattern_->Save();
}
void CanvasRendererPeerImpl::TriggerResetTransformImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerResetTransformImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->ResetTransform();
}
void CanvasRendererPeerImpl::TriggerSaveLayerImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSaveLayerImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->SaveLayer();
}
void CanvasRendererPeerImpl::TriggerRestoreLayerImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerRestoreLayerImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->RestoreLayer();
}
void CanvasRendererPeerImpl::TriggerResetImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerResetImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Reset();
}
std::optional<LineDashParam> CanvasRendererPeerImpl::TriggerGetLineDashImpl()
{
    auto opt = std::make_optional<LineDashParam>();
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerGetLineDashImpl pattern "
             "not bound to component.");
        return opt;
    }
    opt = pattern_->GetLineDash();
    return opt;
}
void CanvasRendererPeerImpl::TriggerSetLineDashImpl(const std::vector<double>& segments)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetLineDashImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateLineDash(segments);
}
void CanvasRendererPeerImpl::TriggerClearRectImpl(const Rect& rect)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerClearRect pattern "
             "not bound to component.");
        return;
    }
    pattern_->ClearRect(rect);
}
void CanvasRendererPeerImpl::TriggerFillRectImpl(const Rect& rect)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerFillRect pattern "
             "not bound to component.");
        return;
    }
    pattern_->FillRect(rect);
}
void CanvasRendererPeerImpl::TriggerStrokeRectImpl(const Rect& rect)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerFillRect pattern "
             "not bound to component.");
        return;
    }
    pattern_->StrokeRect(rect);
}
void CanvasRendererPeerImpl::TriggerRotateImpl(double angle)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerRotate pattern "
             "not bound to component.");
        return;
    }
    pattern_->Rotate(angle);
}
void CanvasRendererPeerImpl::TriggerScaleImpl(double x, double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerScale pattern "
             "not bound to component.");
        return;
    }
    pattern_->Scale(x, y);
}
void CanvasRendererPeerImpl::TriggerTranslateImpl(double x, double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerTranslate pattern "
             "not bound to component.");
        return;
    }
    pattern_->Translate(x, y);
}
void CanvasRendererPeerImpl::TriggerSetGlobalAlphaImpl(double alpha)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerUpdateGlobalAlpha pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateGlobalAlpha(alpha);
}
void CanvasRendererPeerImpl::TriggerFillTextImpl(
    const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerFillTextImpl pattern "
             "not bound to component.");
        return;
    }
    double density = GetDensity();
    if (maxWidth.has_value()) {
        maxWidth = maxWidth.value() * density;
    }
    pattern_->FillText(text, x * density, y * density, maxWidth);
}
void CanvasRendererPeerImpl::TriggerStrokeTextImpl(
    const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerStrokeTextImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->StrokeText(text, x, y, maxWidth);
}
void CanvasRendererPeerImpl::TriggerSetTransformImpl(const TransformParam& param)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetTransform0Impl pattern "
             "not bound to component.");
        return;
    }
    pattern_->SetTransform(param);
}
void CanvasRendererPeerImpl::TriggerTransformImpl(const TransformParam& param)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerTransformImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Transform(param);
}
void CanvasRendererPeerImpl::TriggerSetGlobalCompositeOperationImpl(CompositeOperation& type)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetGlobalCompositeOperationImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateCompositeOperation(type);
}
void CanvasRendererPeerImpl::TriggerSetFilterImpl(const std::string& filterStr)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetFilterImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->SetFilterParam(filterStr);
}
void CanvasRendererPeerImpl::TriggerSetImageSmoothingEnabledImpl(bool enabled)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetImageSmoothingEnabledImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateSmoothingEnabled(enabled);
}
double CanvasRendererPeerImpl::TriggerGetLineDashOffsetImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerGetLineDashOffsetImpl pattern "
             "not bound to component.");
        return 0;
    }
    return pattern_->GetLineDash().dashOffset;
}
void CanvasRendererPeerImpl::TriggerSetLineDashOffsetImpl(double dash)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetLineDashOffsetImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateLineDashOffset(dash);
}
void CanvasRendererPeerImpl::TriggerSetLineWidthImpl(double width)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetLineWidthImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateLineWidth(width);
}
void CanvasRendererPeerImpl::TriggerSetMiterLimitImpl(double limit)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetMiterLimitImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateMiterLimit(limit);
}
void CanvasRendererPeerImpl::TriggerSetShadowBlurImpl(double blur)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetShadowBlurImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateShadowBlur(blur);
}
void CanvasRendererPeerImpl::TriggerSetShadowColorImpl(Color& color)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetShadowColorImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateShadowColor(color);
}
void CanvasRendererPeerImpl::TriggerSetShadowOffsetXImpl(double offsetX)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetShadowOffsetXImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateShadowOffsetX(offsetX);
}
void CanvasRendererPeerImpl::TriggerSetShadowOffsetYImpl(double offsetY)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetShadowOffsetYImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateShadowOffsetY(offsetY);
}
void CanvasRendererPeerImpl::TriggerStroke1Impl(const RefPtr<CanvasPath2D>& path)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerStroke1Impl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Stroke(path);
}
#ifdef PIXEL_MAP_SUPPORTED
void CanvasRendererPeerImpl::TriggerTransferFromImageBitmapImpl(const RefPtr<PixelMap>& pixelMap)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerTransferFromImageBitmapImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->TransferFromImageBitmap(pixelMap);
}
#else
void CanvasRendererPeerImpl::TriggerTransferFromImageBitmapImpl(const double width, const double height)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerTransferFromImageBitmapImpl pattern "
             "not bound to component.");
        return;
    }
    auto imageData = pattern_->GetImageData(0, 0, width, height);
    CHECK_NULL_VOID(imageData);
    pattern_->TransferFromImageBitmap(*imageData);
}
#endif
void CanvasRendererPeerImpl::TriggerSetFillStyleImpl(const Color& color)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetFillStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateFillColor(color);
}
void CanvasRendererPeerImpl::TriggerSetFillStyleImpl(const std::shared_ptr<Ace::Gradient>& gradient)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetFillStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->SetFillGradient(gradient);
}
void CanvasRendererPeerImpl::TriggerSetFillStyleImpl(const std::weak_ptr<Ace::Pattern>& pattern)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetFillStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateFillPattern(pattern);
}
void CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl(const Color& color)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateStrokeColor(color);
}
void CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl(const std::shared_ptr<Ace::Gradient>& gradient)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->SetStrokeGradient(gradient);
}
void CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl(const std::weak_ptr<Ace::Pattern>& pattern)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerSetStrokeStyleImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->UpdateStrokePattern(pattern);
}
void CanvasRendererPeerImpl::TriggerArcImpl(double x,
                                            double y,
                                            double radius,
                                            double startAngle,
                                            double endAngle,
                                            bool counterclockwise)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerArcImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Arc({x, y, radius, startAngle, endAngle, counterclockwise});
}
void CanvasRendererPeerImpl::TriggerArcToImpl(double x1, double y1, double x2, double y2, double radius)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerArcToImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->ArcTo({x1, y1, x2, y2, radius});
}
void CanvasRendererPeerImpl::TriggerBezierCurveToImpl(double cp1x,
                                                      double cp1y,
                                                      double cp2x,
                                                      double cp2y,
                                                      double x,
                                                      double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerBezierCurveToImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->BezierCurveTo({cp1x, cp1y, cp2x, cp2y, x, y});
}
void CanvasRendererPeerImpl::TriggerClosePathImpl()
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerClosePathImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->ClosePath();
}
void CanvasRendererPeerImpl::TriggerEllipseImpl(double x,
                                                double y,
                                                double radiusX,
                                                double radiusY,
                                                double rotation,
                                                double startAngle,
                                                double endAngle,
                                                bool counterclockwise)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerEllipseImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->Ellipse({x, y, radiusX, radiusY, rotation, startAngle, endAngle, counterclockwise});
}
void CanvasRendererPeerImpl::TriggerLineToImpl(double x, double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerLineToImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->LineTo(x, y);
}
void CanvasRendererPeerImpl::TriggerMoveToImpl(double x, double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerMoveToImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->MoveTo(x, y);
}
void CanvasRendererPeerImpl::TriggerQuadraticCurveToImpl(double cpx, double cpy, double x, double y)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerQuadraticCurveToImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->QuadraticCurveTo({cpx, cpy, x, y});
}
void CanvasRendererPeerImpl::TriggerRectImpl(double x, double y, double w, double h)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::TriggerRectImpl pattern "
             "not bound to component.");
        return;
    }
    pattern_->AddRect({x, y, w, h});
}
Dimension CanvasRendererPeerImpl::GetDimensionValue(const std::string& str)
{
    Dimension dimension = StringUtils::StringToDimension(str);
    if ((dimension.Unit() == DimensionUnit::NONE) || (dimension.Unit() == DimensionUnit::PX)) {
        return Dimension(dimension.Value());
    }
    if (dimension.Unit() == DimensionUnit::VP) {
        return Dimension(dimension.Value() * GetDensity(true));
    }
    return Dimension(0.0);
}
void CanvasRendererPeerImpl::SetFont(std::string fontStr)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetFont pattern not bound to component.");
        return;
    }
    bool updateFontweight = false;
    bool updateFontStyle = false;
    std::vector<std::string> fontProps;
    StringUtils::StringSplitter(fontStr.c_str(), ' ', fontProps);
    for (const auto& fontProp : fontProps) {
        if (FONT_WEIGHTS.find(fontProp) != FONT_WEIGHTS.end()) {
            updateFontweight = true;
            auto weight = StringUtils::StringToFontWeight(fontProp, Ace::FontWeight::NORMAL);
            pattern_->UpdateFontWeight(weight);
        } else if (FONT_STYLES.find(fontProp) != FONT_STYLES.end()) {
            updateFontStyle = true;
            auto fontStyle =
                fontProp == DOM_TEXT_FONT_STYLE_ITALIC ? OHOS::Ace::FontStyle::ITALIC : Ace::FontStyle::NORMAL;
            pattern_->UpdateFontStyle(fontStyle);
        } else if (FONT_FAMILIES.find(fontProp) != FONT_FAMILIES.end()) {
            std::vector<std::string> fontFamilies;
            std::stringstream stream(fontProp);
            std::string fontFamily;
            while (getline(stream, fontFamily, ',')) {
                fontFamilies.emplace_back(fontFamily);
            }
            pattern_->UpdateFontFamilies(fontFamilies);
        } else if (fontProp.find("px") != std::string::npos || fontProp.find("vp") != std::string::npos) {
            Dimension size;
            if (fontProp.find("vp") != std::string::npos) {
                size = GetDimensionValue(fontProp);
            } else {
                std::string fontSize = fontProp.substr(0, fontProp.size() - 2);
                size = Dimension(StringUtils::StringToDouble(fontProp));
            }
            if (size.IsNonNegative()) {
                pattern_->UpdateFontSize(size);
            }
        }
    }
    if (!updateFontStyle) {
        pattern_->UpdateFontStyle(Ace::FontStyle::NORMAL);
    }
    if (!updateFontweight) {
        pattern_->UpdateFontWeight(Ace::FontWeight::NORMAL);
    }
}
std::shared_ptr<OHOS::Ace::Gradient> CanvasRendererPeerImpl::CreateLinearGradient(
    const double x0, const double y0, const double x1, const double y1)
{
    double density = GetDensity();
    auto gradient = std::make_shared<OHOS::Ace::Gradient>();
    gradient->SetType(OHOS::Ace::GradientType::LINEAR);
    gradient->SetBeginOffset(Offset(x0 * density, y0 * density));
    gradient->SetEndOffset(Offset(x1 * density, y1 * density));
    return gradient;
}
std::shared_ptr<OHOS::Ace::Gradient> CanvasRendererPeerImpl::CreateRadialGradient(const std::vector<double> params)
{
    double density = GetDensity();
    auto gradient = std::make_shared<OHOS::Ace::Gradient>();
    auto x0 = params[0];
    auto y0 = params[1];
    auto r0 = params[2];
    auto x1 = params[3];
    auto y1 = params[4];
    auto r1 = params[5];
    gradient->SetType(OHOS::Ace::GradientType::RADIAL);
    gradient->SetBeginOffset(Offset(x0 * density, y0 * density));
    gradient->SetEndOffset(Offset(x1 * density, y1 * density));
    gradient->SetInnerRadius(r0 * density);
    gradient->SetOuterRadius(r1 * density);
    return gradient;
}
std::shared_ptr<OHOS::Ace::Gradient> CanvasRendererPeerImpl::CreateConicGradient(
    const double startAngle, const double x, const double y)
{
    double density = GetDensity();
    auto gradient = std::make_shared<OHOS::Ace::Gradient>();
    gradient->SetType(OHOS::Ace::GradientType::CONIC);
    gradient->GetConicGradient().startAngle = AnimatableDimension(Dimension(fmod(startAngle, (MATH_2_PI))));
    gradient->GetConicGradient().centerX = AnimatableDimension(Dimension(x * density));
    gradient->GetConicGradient().centerY = AnimatableDimension(Dimension(y * density));
    return gradient;
}
void CanvasRendererPeerImpl::ClearImageData()
{
    imageData.x = 0;
    imageData.y = 0;
    imageData.dirtyX = 0;
    imageData.dirtyY = 0;
    imageData.dirtyWidth = 0;
    imageData.dirtyHeight = 0;
    imageData.data.clear();
    imageData.pixelMap = nullptr;
}
Ace::ImageSize CanvasRendererPeerImpl::GetImageSize(
    const double& x, const double& y, const double& width, const double& height)
{
    Ace::ImageSize imageSize;
    double density = GetDensity();
    imageSize.left = x * density + DIFF;
    imageSize.top = y * density + DIFF;
    imageSize.width = width * density + DIFF;
    imageSize.height = height * density + DIFF;
    return imageSize;
}
std::unique_ptr<Ace::ImageData> CanvasRendererPeerImpl::GetImageData(const ImageSize& imageSize)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::GetImageData pattern not bound to component.");
        return nullptr;
    }
    return pattern_->GetImageData(imageSize.left, imageSize.top, imageSize.width, imageSize.height);
}
void CanvasRendererPeerImpl::GetPixelMap(const ImageSize& imageSize)
{
#ifdef PIXEL_MAP_SUPPORTED
    LOGE("ARKOALA CanvasRendererPeerImpl::GetPixelMap not implemented because PixelMap not implemented.");
#endif
}
double CanvasRendererPeerImpl::GetDimension(const Dimension& dimension, const bool force)
{
    auto value = dimension.Value();
    if (force) {
        return value;
    }
    double density = GetDensity();
    return value * density + DIFF;
}
void CanvasRendererPeerImpl::ParseImageData(const ImageSizeExt& ext)
{
    if (ext.x) {
        imageData.x = static_cast<int32_t>(*ext.x);
    }
    if (ext.y) {
        imageData.y = static_cast<int32_t>(*ext.y);
    }
    if (ext.dirtyX) {
        imageData.dirtyX = static_cast<int32_t>(*ext.dirtyX);
    }
    if (ext.dirtyY) {
        imageData.dirtyY = static_cast<int32_t>(*ext.dirtyY);
    }
    if (ext.dirtyWidth) {
        imageData.dirtyWidth = static_cast<int32_t>(*ext.dirtyWidth);
    }
    if (ext.dirtyHeight) {
        imageData.dirtyHeight = static_cast<int32_t>(*ext.dirtyHeight);
    }
}
void CanvasRendererPeerImpl::PutImageData(const Ace::ImageData& src, const ImageSizeExt& ext)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::PutImageData pattern not bound to component.");
        return;
    }
    auto finalWidth = static_cast<int32_t>(std::abs(src.dirtyWidth));
    auto finalHeight = static_cast<int32_t>(std::abs(src.dirtyHeight));
    ClearImageData();
    imageData.dirtyWidth = finalWidth;
    imageData.dirtyHeight = finalHeight;
    ParseImageData(ext);
    imageData.dirtyWidth = imageData.dirtyX < 0 ? std::min(imageData.dirtyX + imageData.dirtyWidth, finalWidth)
                                                : std::min(finalWidth - imageData.dirtyX, imageData.dirtyWidth);
    imageData.dirtyHeight = imageData.dirtyY < 0 ? std::min(imageData.dirtyY + imageData.dirtyHeight, finalHeight)
                                                 : std::min(finalHeight - imageData.dirtyY, imageData.dirtyHeight);
    auto size = static_cast<uint32_t>(src.data.size());
    for (int32_t i = std::max(imageData.dirtyY, 0); i < imageData.dirtyY + imageData.dirtyHeight; ++i) {
        for (int32_t j = std::max(imageData.dirtyX, 0); j < imageData.dirtyX + imageData.dirtyWidth; ++j) {
            uint32_t idx = static_cast<uint32_t>(j + finalWidth * i);
            if (size > idx) {
                imageData.data.emplace_back(src.data[idx]);
            }
        }
    }
    pattern_->PutImageData(imageData);
}
std::optional<OHOS::Ace::TextMetrics> CanvasRendererPeerImpl::GetTextMetrics(const std::string& text)
{
    std::optional<OHOS::Ace::TextMetrics> textMetrics = std::nullopt;
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::GetTextMetrics pattern not bound to component.");
        return textMetrics;
    }
    auto density = GetDensity();
    if (NonPositive(density) || density == 0) {
        return textMetrics;
    }
    textMetrics = pattern_->MeasureTextMetrics(text, paintState_);
    textMetrics->height /= density;
    textMetrics->actualBoundingBoxLeft /= density;
    textMetrics->actualBoundingBoxRight /= density;
    textMetrics->actualBoundingBoxAscent /= density;
    textMetrics->actualBoundingBoxDescent /= density;
    textMetrics->hangingBaseline /= density;
    textMetrics->alphabeticBaseline /= density;
    textMetrics->ideographicBaseline /= density;
    textMetrics->emHeightAscent /= density;
    textMetrics->emHeightDescent /= density;
    textMetrics->fontBoundingBoxAscent /= density;
    textMetrics->fontBoundingBoxDescent /= density;
    return textMetrics;
}
std::optional<TransformParam> CanvasRendererPeerImpl::GetTransform()
{
    std::optional<TransformParam> param = std::nullopt;
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::GetTransform pattern not bound to component.");
        return param;
    }
    param = pattern_->GetTransform();
    return param;
}
void CanvasRendererPeerImpl::SetPixelMap(RefPtr<PixelMap> pixelMap)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetPixelMap pattern not bound to component.");
        return;
    }
    OHOS::Ace::CanvasImage canvasImage;
    pattern_->DrawPixelMap(pixelMap, canvasImage);
}
void CanvasRendererPeerImpl::SetImageSmoothingQuality(const std::string& quality)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetImageSmoothingQuality pattern not bound to component.");
        return;
    }
    if (QUALITY_TYPE.find(quality) == QUALITY_TYPE.end()) {
        return;
    }
    pattern_->UpdateSmoothingQuality(quality);
}
void CanvasRendererPeerImpl::SetLineCap(const std::string& capStr)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetLineCap pattern not bound to component.");
        return;
    }
    auto cap = LineCapStyle::BUTT;
    auto iter = LINE_CAP_MAP.find(capStr);
    if (iter != LINE_CAP_MAP.end()) {
        cap = iter->second;
    }
    pattern_->UpdateLineCap(cap);
}
void CanvasRendererPeerImpl::SetLineJoin(const std::string& joinStr)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetLineJoin pattern not bound to component.");
        return;
    }
    auto join = LineJoinStyle::MITER;
    auto iter = LINE_JOIN_MAP.find(joinStr);
    if (iter != LINE_JOIN_MAP.end()) {
        join = iter->second;
    }
    pattern_->UpdateLineJoin(join);
}
void CanvasRendererPeerImpl::Clip(const Ace::CanvasFillRule& fillRule)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::Clip pattern not bound to component.");
        return;
    }
    pattern_->UpdateFillRuleForPath(fillRule);
    pattern_->Clip();
}
void CanvasRendererPeerImpl::Clip(const Ace::CanvasFillRule& fillRule, const RefPtr<CanvasPath2D>& path)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::Clip pattern not bound to component.");
        return;
    }
    pattern_->UpdateFillRuleForPath(fillRule);
    pattern_->Clip(path);
}
void CanvasRendererPeerImpl::Fill(const Ace::CanvasFillRule& fillRule)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::Fill pattern not bound to component.");
        return;
    }
    pattern_->UpdateFillRuleForPath(fillRule);
    pattern_->Fill();
}
void CanvasRendererPeerImpl::Fill(const Ace::CanvasFillRule& fillRule, const RefPtr<CanvasPath2D>& path)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::Fill pattern not bound to component.");
        return;
    }
    pattern_->UpdateFillRuleForPath(fillRule);
    pattern_->Fill(path);
}
void CanvasRendererPeerImpl::SetTextDirection(const Ace::TextDirection& direction)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetTextDirection pattern not bound to component.");
        return;
    }
    pattern_->SetTextDirection(direction);
}

void CanvasRendererPeerImpl::SetTextAlign(const TextAlign& align)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetTextAlign pattern not bound to component.");
        return;
    }
    pattern_->UpdateTextAlign(align);
}
void CanvasRendererPeerImpl::SetTextBaseline(const TextBaseline& baseline)
{
    if (!pattern_) {
        LOGE("ARKOALA CanvasRendererPeerImpl::SetTextBaseline pattern not bound to component.");
        return;
    }
    pattern_->UpdateTextBaseline(baseline);
}
} // namespace OHOS::Ace::NG::GeneratedModifier

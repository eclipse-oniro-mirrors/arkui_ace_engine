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

#include "core/components_ng/pattern/custom_paint/custom_paint_paint_method.h"

#include <cmath>
#include <unistd.h>

#include "securec.h"

#include "drawing/engine_adapter/skia_adapter/skia_canvas.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkShader.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurface.h"
#ifdef NEW_SKIA
#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"
#else
#include "include/effects/SkBlurImageFilter.h"
#endif
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkParsePath.h"

#include "base/geometry/ng/offset_t.h"
#include "base/json/json_util.h"
#include "base/log/ace_trace.h"
#include "base/utils/linear_map.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/calendar/rosen_render_calendar.h"
#ifndef NEW_SKIA
#include "core/components/common/painter/flutter_decoration_painter.h"
#endif
#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/common/properties/decoration.h"
#include "core/components_ng/render/drawing.h"
#include "core/image/flutter_image_cache.h"
#include "core/image/image_cache.h"
#include "core/image/image_provider.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double HALF_CIRCLE_ANGLE = 180.0;
constexpr double FULL_CIRCLE_ANGLE = 360.0;
constexpr double CONIC_START_ANGLE = 0.0;
constexpr double CONIC_END_ANGLE = 359.9;

const LinearEnumMapNode<CompositeOperation, SkBlendMode> SK_BLEND_MODE_TABLE[] = {
    { CompositeOperation::SOURCE_OVER, SkBlendMode::kSrcOver },
    { CompositeOperation::SOURCE_ATOP, SkBlendMode::kSrcATop },
    { CompositeOperation::SOURCE_IN, SkBlendMode::kSrcIn },
    { CompositeOperation::SOURCE_OUT, SkBlendMode::kSrcOut },
    { CompositeOperation::DESTINATION_OVER, SkBlendMode::kDstOver },
    { CompositeOperation::DESTINATION_ATOP, SkBlendMode::kDstATop },
    { CompositeOperation::DESTINATION_IN, SkBlendMode::kDstIn },
    { CompositeOperation::DESTINATION_OUT, SkBlendMode::kDstOut },
    { CompositeOperation::LIGHTER, SkBlendMode::kLighten },
    { CompositeOperation::COPY, SkBlendMode::kSrc },
    { CompositeOperation::XOR, SkBlendMode::kXor },
};
constexpr size_t BLEND_MODE_SIZE = ArraySize(SK_BLEND_MODE_TABLE);

template<typename T, typename N>
N ConvertEnumToSkEnum(T key, const LinearEnumMapNode<T, N>* map, size_t length, N defaultValue)
{
    int64_t index = BinarySearchFindIndex(map, length, key);
    return index != -1 ? map[index].value : defaultValue;
}

template<typename T>
inline T ConvertStrToEnum(const char* key, const LinearMapNode<T>* map, size_t length, T defaultValue)
{
    int64_t index = BinarySearchFindIndex(map, length, key);
    return index != -1 ? map[index].value : defaultValue;
}
} // namespace

bool CustomPaintPaintMethod::HasShadow() const
{
    return !(NearZero(shadow_.GetOffset().GetX()) && NearZero(shadow_.GetOffset().GetY()) &&
             NearZero(shadow_.GetBlurRadius()));
}

void CustomPaintPaintMethod::UpdateLineDash(SkPaint& paint)
{
    if (!strokeState_.GetLineDash().lineDash.empty()) {
        auto lineDashState = strokeState_.GetLineDash().lineDash;
        SkScalar intervals[lineDashState.size()];
        for (size_t i = 0; i < lineDashState.size(); ++i) {
            intervals[i] = SkDoubleToScalar(lineDashState[i]);
        }
        SkScalar phase = SkDoubleToScalar(strokeState_.GetLineDash().dashOffset);
        paint.setPathEffect(SkDashPathEffect::Make(intervals, lineDashState.size(), phase));
    }
}

sk_sp<SkShader> CustomPaintPaintMethod::MakeConicGradient(SkPaint& paint, const Ace::Gradient& gradient)
{
    sk_sp<SkShader> skShader = nullptr;
    if (gradient.GetType() == Ace::GradientType::CONIC) {
        if (!gradient.GetConicGradient().centerX.has_value() ||
            !gradient.GetConicGradient().centerY.has_value() ||
            !gradient.GetConicGradient().startAngle.has_value()) {
            return skShader;
        }
        SkMatrix matrix = SkMatrix::I();
        SkScalar centerX = SkDoubleToScalar(gradient.GetConicGradient().centerX->Value());
        SkScalar centerY = SkDoubleToScalar(gradient.GetConicGradient().centerY->Value());
        auto gradientColors = gradient.GetColors();
        std::stable_sort(gradientColors.begin(), gradientColors.end(),
            [](auto& colorA, auto& colorB) { return colorA.GetDimension() < colorB.GetDimension(); });
        uint32_t colorsSize = gradientColors.size();
        SkColor colors[gradientColors.size()];
        float pos[gradientColors.size()];
        double angle = gradient.GetConicGradient().startAngle->Value() / M_PI * 180.0;
        SkScalar startAngle = SkDoubleToScalar(angle);
        matrix.preRotate(startAngle, centerX, centerY);
        for (uint32_t i = 0; i < colorsSize; ++i) {
            const auto& gradientColor = gradientColors[i];
            colors[i] = gradientColor.GetColor().GetValue();
            pos[i] = gradientColor.GetDimension().Value();
        }
#ifdef USE_SYSTEM_SKIA
    auto mode = SkShader::kClamp_TileMode;
#else
    auto mode = SkTileMode::kClamp;
#endif
        skShader = SkGradientShader::MakeSweep(centerX, centerY,
                                               colors, pos, colorsSize, mode,
                                               SkDoubleToScalar(CONIC_START_ANGLE),
                                               SkDoubleToScalar(CONIC_END_ANGLE),
                                               0, &matrix);
    }
    return skShader;
}

void CustomPaintPaintMethod::UpdatePaintShader(const OffsetF& offset, SkPaint& paint, const Ace::Gradient& gradient)
{
    SkPoint beginPoint = SkPoint::Make(SkDoubleToScalar(gradient.GetBeginOffset().GetX() + offset.GetX()),
        SkDoubleToScalar(gradient.GetBeginOffset().GetY() + offset.GetY()));
    SkPoint endPoint = SkPoint::Make(SkDoubleToScalar(gradient.GetEndOffset().GetX() + offset.GetX()),
        SkDoubleToScalar(gradient.GetEndOffset().GetY() + offset.GetY()));
    SkPoint pts[2] = { beginPoint, endPoint };
    auto gradientColors = gradient.GetColors();
    std::stable_sort(gradientColors.begin(), gradientColors.end(),
        [](auto& colorA, auto& colorB) { return colorA.GetDimension() < colorB.GetDimension(); });
    uint32_t colorsSize = gradientColors.size();
    SkColor colors[gradientColors.size()];
    float pos[gradientColors.size()];
    for (uint32_t i = 0; i < colorsSize; ++i) {
        const auto& gradientColor = gradientColors[i];
        colors[i] = gradientColor.GetColor().GetValue();
        pos[i] = gradientColor.GetDimension().Value();
    }
#ifdef USE_SYSTEM_SKIA
    auto mode = SkShader::kClamp_TileMode;
#else
    auto mode = SkTileMode::kClamp;
#endif
    sk_sp<SkShader> skShader = nullptr;
    if (gradient.GetType() == Ace::GradientType::LINEAR) {
        skShader = SkGradientShader::MakeLinear(pts, colors, pos, gradientColors.size(), mode);
    } else if (gradient.GetType() == Ace::GradientType::CONIC) {
        skShader = MakeConicGradient(paint, gradient);
    } else {
        if (gradient.GetInnerRadius() <= 0.0 && beginPoint == endPoint) {
            skShader = SkGradientShader::MakeRadial(
                endPoint, gradient.GetOuterRadius(), colors, pos, gradientColors.size(), mode);
        } else {
            skShader = SkGradientShader::MakeTwoPointConical(beginPoint, gradient.GetInnerRadius(), endPoint,
                gradient.GetOuterRadius(), colors, pos, gradientColors.size(), mode);
        }
    }
    paint.setShader(skShader);
}

SkMatrix CustomPaintPaintMethod::GetMatrixFromPattern(const Ace::Pattern& pattern)
{
    SkMatrix matrix;
    double viewScale = 1.0;
    auto context = context_.Upgrade();
    if (context) {
        viewScale = context->GetViewScale();
    }
    matrix.setAll(pattern.GetScaleX() * viewScale, pattern.GetSkewX() * viewScale, pattern.GetTranslateX() * viewScale,
        pattern.GetSkewY() * viewScale, pattern.GetScaleY() * viewScale, pattern.GetTranslateY() * viewScale, 0.0f,
        0.0f, 1.0f);
    return matrix;
}

void CustomPaintPaintMethod::UpdatePaintShader(const Ace::Pattern& pattern, SkPaint& paint)
{
    auto width = pattern.GetImageWidth();
    auto height = pattern.GetImageHeight();
    auto image = GreatOrEqual(width, 0) && GreatOrEqual(height, 0)
                     ? ImageProvider::GetSkImage(pattern.GetImgSrc(), context_, Size(width, height))
                     : ImageProvider::GetSkImage(pattern.GetImgSrc(), context_);
    CHECK_NULL_VOID(image);
    SkMatrix* matrix = nullptr;
    SkMatrix tempMatrix;
    if (pattern.IsTransformable()) {
        tempMatrix = GetMatrixFromPattern(pattern);
        matrix = &tempMatrix;
    }
    static const LinearMapNode<void (*)(sk_sp<SkImage>, SkPaint&, SkMatrix*)> staticPattern[] = {
        { "clamp",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, matrix));
#endif
#endif
            } },
        { "mirror",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kMirror, SkTileMode::kMirror, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kMirror, SkTileMode::kMirror, matrix));
#endif
#endif
            } },
        { "no-repeat",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kDecal_TileMode, SkShader::kDecal_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, matrix));
#endif
#endif
            } },
        { "repeat",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, matrix));
#endif
#endif
            } },
        { "repeat-x",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kRepeat_TileMode, SkShader::kDecal_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kRepeat, SkTileMode::kDecal, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kDecal, matrix));
#endif
#endif
            } },
        { "repeat-y",
            [](sk_sp<SkImage> image, SkPaint& paint, SkMatrix* matrix) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kDecal_TileMode, SkShader::kRepeat_TileMode, matrix));
#else
#ifdef NEW_SKIA
                paint.setShader(
                    image->makeShader(SkTileMode::kDecal, SkTileMode::kRepeat, SkSamplingOptions(), matrix));
#else
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kRepeat, matrix));
#endif
#endif
            } },
    };
    auto operatorIter = BinarySearchFindIndex(staticPattern, ArraySize(staticPattern),
        pattern.GetRepetition().c_str());
    if (operatorIter != -1) {
        staticPattern[operatorIter].value(image, paint, matrix);
    }
}

void CustomPaintPaintMethod::InitPaintBlend(SkPaint& paint)
{
    paint.setBlendMode(
        ConvertEnumToSkEnum(globalState_.GetType(), SK_BLEND_MODE_TABLE, BLEND_MODE_SIZE, SkBlendMode::kSrcOver));
}

#ifndef NEW_SKIA
void CustomPaintPaintMethod::GetStrokePaint(SkPaint& paint)
{
    static const LinearEnumMapNode<LineJoinStyle, SkPaint::Join> skLineJoinTable[] = {
        { LineJoinStyle::MITER, SkPaint::Join::kMiter_Join },
        { LineJoinStyle::ROUND, SkPaint::Join::kRound_Join },
        { LineJoinStyle::BEVEL, SkPaint::Join::kBevel_Join },
    };
    static const LinearEnumMapNode<LineCapStyle, SkPaint::Cap> skLineCapTable[] = {
        { LineCapStyle::BUTT, SkPaint::Cap::kButt_Cap },
        { LineCapStyle::ROUND, SkPaint::Cap::kRound_Cap },
        { LineCapStyle::SQUARE, SkPaint::Cap::kSquare_Cap },
    };
    InitImagePaint(paint);
    paint.setColor(strokeState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeJoin(ConvertEnumToSkEnum(
        strokeState_.GetLineJoin(), skLineJoinTable, ArraySize(skLineJoinTable), SkPaint::Join::kMiter_Join));
    paint.setStrokeCap(ConvertEnumToSkEnum(
        strokeState_.GetLineCap(), skLineCapTable, ArraySize(skLineCapTable), SkPaint::Cap::kButt_Cap));
    paint.setStrokeWidth(static_cast<SkScalar>(strokeState_.GetLineWidth()));
    paint.setStrokeMiter(static_cast<SkScalar>(strokeState_.GetMiterLimit()));

    // set line Dash
    UpdateLineDash(paint);

    // set global alpha
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
}

void CustomPaintPaintMethod::InitImagePaint(SkPaint& paint)
{
    if (smoothingEnabled_) {
        if (smoothingQuality_ == "low") {
            paint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
        } else if (smoothingQuality_ == "medium") {
            paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        } else if (smoothingQuality_ == "high") {
            paint.setFilterQuality(SkFilterQuality::kHigh_SkFilterQuality);
        } else {
            LOGE("Unsupported Quality type:%{public}s", smoothingQuality_.c_str());
        }
    } else {
        paint.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
    }
    ClearPaintImage(paint);
    SetPaintImage(paint);
}
#else
void CustomPaintPaintMethod::GetStrokePaint(SkPaint& paint, SkSamplingOptions& options)
{
    static const LinearEnumMapNode<LineJoinStyle, SkPaint::Join> skLineJoinTable[] = {
        { LineJoinStyle::MITER, SkPaint::Join::kMiter_Join },
        { LineJoinStyle::ROUND, SkPaint::Join::kRound_Join },
        { LineJoinStyle::BEVEL, SkPaint::Join::kBevel_Join },
    };
    static const LinearEnumMapNode<LineCapStyle, SkPaint::Cap> skLineCapTable[] = {
        { LineCapStyle::BUTT, SkPaint::Cap::kButt_Cap },
        { LineCapStyle::ROUND, SkPaint::Cap::kRound_Cap },
        { LineCapStyle::SQUARE, SkPaint::Cap::kSquare_Cap },
    };
    InitImagePaint(paint, options);
    paint.setColor(strokeState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeJoin(ConvertEnumToSkEnum(
        strokeState_.GetLineJoin(), skLineJoinTable, ArraySize(skLineJoinTable), SkPaint::Join::kMiter_Join));
    paint.setStrokeCap(ConvertEnumToSkEnum(
        strokeState_.GetLineCap(), skLineCapTable, ArraySize(skLineCapTable), SkPaint::Cap::kButt_Cap));
    paint.setStrokeWidth(static_cast<SkScalar>(strokeState_.GetLineWidth()));
    paint.setStrokeMiter(static_cast<SkScalar>(strokeState_.GetMiterLimit()));

    // set line Dash
    UpdateLineDash(paint);

    // set global alpha
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
}

void CustomPaintPaintMethod::InitImagePaint(SkPaint& paint, SkSamplingOptions& options)
{
    if (smoothingEnabled_) {
        if (smoothingQuality_ == "low") {
            options = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        } else if (smoothingQuality_ == "medium") {
            options = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
        } else if (smoothingQuality_ == "high") {
            options = SkSamplingOptions(SkCubicResampler::Mitchell());
        } else {
            LOGE("Unsupported Quality type:%{public}s", smoothingQuality_.c_str());
        }
    } else {
        options = SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
    }
    ClearPaintImage(paint);
    SetPaintImage(paint);
}
#endif

void CustomPaintPaintMethod::InitImageCallbacks()
{
    imageObjSuccessCallback_ = [weak = AceType::WeakClaim(this)](
                                   ImageSourceInfo info, const RefPtr<Ace::ImageObject>& imageObj) {
        auto paintMethod = weak.Upgrade();
        if (paintMethod->loadingSource_ == info) {
            paintMethod->ImageObjReady(imageObj);
            return;
        } else {
            LOGE("image sourceInfo_ check error, : %{public}s vs %{public}s",
                paintMethod->loadingSource_.ToString().c_str(), info.ToString().c_str());
        }
    };

    failedCallback_ = [weak = AceType::WeakClaim(this)](ImageSourceInfo info, const std::string& errorMsg = "") {
        auto paintMethod = weak.Upgrade();
        paintMethod->ImageObjFailed();
    };

    uploadSuccessCallback_ = [weak = AceType::WeakClaim(this)](
                                 ImageSourceInfo sourceInfo, const RefPtr<NG::CanvasImage>& image) {};

    onPostBackgroundTask_ = [weak = AceType::WeakClaim(this)](CancelableTask task) {};
}

void CustomPaintPaintMethod::DrawSvgImage(PaintWrapper* paintWrapper, const Ace::CanvasImage& canvasImage)
{
    // Make the ImageSourceInfo
    canvasImage_ = canvasImage;
    loadingSource_ = ImageSourceInfo(canvasImage.src);
    // get the ImageObject
    if (currentSource_ != loadingSource_) {
        ImageProvider::FetchImageObject(loadingSource_, imageObjSuccessCallback_, uploadSuccessCallback_,
            failedCallback_, context_, true, true, true, onPostBackgroundTask_);
    }

    CHECK_NULL_VOID(skiaDom_);
    // draw the svg
    SkRect srcRect;
    SkRect dstRect;
    switch (canvasImage.flag) {
        case 0:
            srcRect = SkRect::MakeXYWH(0, 0, skiaDom_->containerSize().width(), skiaDom_->containerSize().height());
            dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, skiaDom_->containerSize().width(),
                skiaDom_->containerSize().height());
            break;
        case 1: {
            srcRect = SkRect::MakeXYWH(0, 0, skiaDom_->containerSize().width(), skiaDom_->containerSize().height());
            dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            break;
        }
        case 2: {
            srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
            dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            break;
        }
        default:
            break;
    }
    float scaleX = dstRect.width() / srcRect.width();
    float scaleY = dstRect.height() / srcRect.height();
    OffsetF offset = GetContentOffset(paintWrapper);
    OffsetF startPoint = offset + OffsetF(dstRect.left(), dstRect.top()) -
        OffsetF(srcRect.left() * scaleX, srcRect.top() * scaleY);

    SkCanvas* skCanvas = GetRawPtrOfSkCanvas();
    skCanvas->save();
    skCanvas->clipRect(dstRect);
    skCanvas->translate(startPoint.GetX(), startPoint.GetY());
    skCanvas->scale(scaleX, scaleY);
    skiaDom_->render(skCanvas);
    skCanvas->restore();
}

void CustomPaintPaintMethod::PutImageData(PaintWrapper* paintWrapper, const Ace::ImageData& imageData)
{
    if (imageData.data.empty()) {
        LOGE("PutImageData failed, image data is empty.");
        return;
    }
    uint32_t* data = new (std::nothrow) uint32_t[imageData.data.size()];
    CHECK_NULL_VOID(data);

    for (uint32_t i = 0; i < imageData.data.size(); ++i) {
        data[i] = imageData.data[i].GetValue();
    }
    SkBitmap skBitmap;
    auto imageInfo = SkImageInfo::Make(imageData.dirtyWidth, imageData.dirtyHeight, SkColorType::kBGRA_8888_SkColorType,
        SkAlphaType::kOpaque_SkAlphaType);
    skBitmap.allocPixels(imageInfo);
    skBitmap.setPixels(data);
    auto contentOffset = GetContentOffset(paintWrapper);
#ifndef NEW_SKIA
    skCanvas_->drawBitmap(skBitmap, imageData.x + contentOffset.GetX(), imageData.y + contentOffset.GetY());
#else
    skCanvas_->drawImage(skBitmap.asImage(), imageData.x + contentOffset.GetX(), imageData.y + contentOffset.GetY());
#endif
    delete[] data;
}

void CustomPaintPaintMethod::FillRect(PaintWrapper* paintWrapper, const Rect& rect)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkPaint paint;
#ifndef NEW_SKIA
    InitImagePaint(paint);
#else
    SkSamplingOptions options;
    InitImagePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    if (fillState_.GetPaintStyle() == OHOS::Ace::PaintStyle::Color) {
        paint.setColor(fillState_.GetColor().GetValue());
    }
    paint.setStyle(SkPaint::Style::kFill_Style);
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        SkPath path;
        path.addRect(skRect);
        PaintShadow(path, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPatternValue().IsValid() && fillState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(fillState_.GetPatternValue(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha()); // update the global alpha after setting the color
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawRect(skRect, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawRect(skRect, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::StrokeRect(PaintWrapper* paintWrapper, const Rect& rect)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkPaint paint;
#ifndef NEW_SKIA
    GetStrokePaint(paint);
#else
    SkSamplingOptions options;
    GetStrokePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        SkPath path;
        path.addRect(skRect);
        PaintShadow(path, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPatternValue().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(strokeState_.GetPatternValue(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawRect(skRect, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawRect(skRect, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::ClearRect(PaintWrapper* paintWrapper, const Rect& rect)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkPaint paint;
#ifndef NEW_SKIA
    InitImagePaint(paint);
#else
    SkSamplingOptions options;
    InitImagePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    paint.setBlendMode(SkBlendMode::kClear);
    auto skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), rect.Bottom() + offset.GetY());
    skCanvas_->drawRect(skRect, paint);
}

void CustomPaintPaintMethod::SetFillRuleForPath(const CanvasFillRule& rule)
{
    if (rule == CanvasFillRule::NONZERO) {
#ifndef NEW_SKIA
        skPath_.setFillType(SkPath::FillType::kWinding_FillType);
#else
        skPath_.setFillType(SkPathFillType::kWinding);
#endif
    } else if (rule == CanvasFillRule::EVENODD) {
#ifndef NEW_SKIA
        skPath_.setFillType(SkPath::FillType::kEvenOdd_FillType);
#else
        skPath_.setFillType(SkPathFillType::kEvenOdd);
#endif
    }
}

void CustomPaintPaintMethod::SetFillRuleForPath2D(const CanvasFillRule& rule)
{
    if (rule == CanvasFillRule::NONZERO) {
#ifndef NEW_SKIA
        skPath2d_.setFillType(SkPath::FillType::kWinding_FillType);
#else
        skPath2d_.setFillType(SkPathFillType::kWinding);
#endif
    } else if (rule == CanvasFillRule::EVENODD) {
#ifndef NEW_SKIA
        skPath2d_.setFillType(SkPath::FillType::kEvenOdd_FillType);
#else
        skPath2d_.setFillType(SkPathFillType::kEvenOdd);
#endif
    }
}

void CustomPaintPaintMethod::Fill(PaintWrapper* paintWrapper)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkPaint paint;
#ifndef NEW_SKIA
    InitImagePaint(paint);
#else
    SkSamplingOptions options;
    InitImagePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    if (fillState_.GetPaintStyle() == OHOS::Ace::PaintStyle::Color) {
        paint.setColor(fillState_.GetColor().GetValue());
    }
    paint.setStyle(SkPaint::Style::kFill_Style);
    if (HasShadow()) {
        PaintShadow(skPath_, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPatternValue().IsValid() && fillState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(fillState_.GetPatternValue(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::Fill(PaintWrapper* paintWrapper, const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(path);
    OffsetF offset = GetContentOffset(paintWrapper);
    ParsePath2D(offset, path);
    Path2DFill(offset);
    skPath2d_.reset();
}

void CustomPaintPaintMethod::Path2DFill(const OffsetF& offset)
{
    SkPaint paint;
#ifndef NEW_SKIA
    InitImagePaint(paint);
#else
    SkSamplingOptions options;
    InitImagePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    if (fillState_.GetPaintStyle() == OHOS::Ace::PaintStyle::Color) {
        paint.setColor(fillState_.GetColor().GetValue());
    }
    paint.setStyle(SkPaint::Style::kFill_Style);
    if (HasShadow()) {
        PaintShadow(skPath2d_, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPatternValue().IsValid() && fillState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(fillState_.GetPatternValue(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath2d_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath2d_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::Stroke(PaintWrapper* paintWrapper)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkPaint paint;
#ifndef NEW_SKIA
    GetStrokePaint(paint);
#else
    SkSamplingOptions options;
    GetStrokePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    if (HasShadow()) {
        PaintShadow(skPath_, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPatternValue().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(strokeState_.GetPatternValue(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::Stroke(PaintWrapper* paintWrapper, const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(path);
    OffsetF offset = GetContentOffset(paintWrapper);
    ParsePath2D(offset, path);
    Path2DStroke(offset);
    skPath2d_.reset();
}

void CustomPaintPaintMethod::Path2DStroke(const OffsetF& offset)
{
    SkPaint paint;
#ifndef NEW_SKIA
    GetStrokePaint(paint);
#else
    SkSamplingOptions options;
    GetStrokePaint(paint, options);
#endif
    paint.setAntiAlias(antiAlias_);
    if (HasShadow()) {
        PaintShadow(skPath2d_, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPatternValue().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::ImagePattern) {
        UpdatePaintShader(strokeState_.GetPatternValue(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath2d_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath2d_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, options, &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
}

void CustomPaintPaintMethod::Clip()
{
    skCanvas_->clipPath(skPath_);
}

void CustomPaintPaintMethod::Clip(const RefPtr<CanvasPath2D>& path)
{
    CHECK_NULL_VOID(path);
    auto offset = OffsetF(0, 0);
    ParsePath2D(offset, path);
    Path2DClip();
    skPath2d_.reset();
}

void CustomPaintPaintMethod::Path2DClip()
{
    skCanvas_->clipPath(skPath2d_);
}

void CustomPaintPaintMethod::BeginPath()
{
    skPath_.reset();
}

void CustomPaintPaintMethod::ClosePath()
{
    skPath_.close();
}

void CustomPaintPaintMethod::MoveTo(PaintWrapper* paintWrapper, double x, double y)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    skPath_.moveTo(SkDoubleToScalar(x + offset.GetX()), SkDoubleToScalar(y + offset.GetY()));
}

void CustomPaintPaintMethod::LineTo(PaintWrapper* paintWrapper, double x, double y)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    skPath_.lineTo(SkDoubleToScalar(x + offset.GetX()), SkDoubleToScalar(y + offset.GetY()));
}

void CustomPaintPaintMethod::Arc(PaintWrapper* paintWrapper, const ArcParam& param)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    double left = param.x - param.radius + offset.GetX();
    double top = param.y - param.radius + offset.GetY();
    double right = param.x + param.radius + offset.GetX();
    double bottom = param.y + param.radius + offset.GetY();
    double startAngle = param.startAngle * HALF_CIRCLE_ANGLE / M_PI;
    double endAngle = param.endAngle * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (param.anticlockwise) {
        sweepAngle =
            endAngle > startAngle ? (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) - FULL_CIRCLE_ANGLE) : sweepAngle;
    } else {
        sweepAngle =
            endAngle > startAngle ? sweepAngle : (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) + FULL_CIRCLE_ANGLE);
    }
    auto rect = SkRect::MakeLTRB(left, top, right, bottom);
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        // draw circle
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + startAngle), SkDoubleToScalar(half), false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + half + startAngle), SkDoubleToScalar(sweepAngle), false);
    } else {
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(sweepAngle), false);
    }
}

void CustomPaintPaintMethod::ArcTo(PaintWrapper* paintWrapper, const ArcToParam& param)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    double x1 = param.x1 + offset.GetX();
    double y1 = param.y1 + offset.GetY();
    double x2 = param.x2 + offset.GetX();
    double y2 = param.y2 + offset.GetY();
    double radius = param.radius;
    skPath_.arcTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1), SkDoubleToScalar(x2), SkDoubleToScalar(y2),
        SkDoubleToScalar(radius));
}

void CustomPaintPaintMethod::AddRect(PaintWrapper* paintWrapper, const Rect& rect)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    skPath_.addRect(skRect);
}

void CustomPaintPaintMethod::Ellipse(PaintWrapper* paintWrapper, const EllipseParam& param)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    // Init the start and end angle, then calculated the sweepAngle.
    double startAngle = std::fmod(param.startAngle, M_PI * 2.0);
    double endAngle = std::fmod(param.endAngle, M_PI * 2.0);
    startAngle = (startAngle < 0.0 ? startAngle + M_PI * 2.0 : startAngle) * HALF_CIRCLE_ANGLE / M_PI;
    endAngle = (endAngle < 0.0 ? endAngle + M_PI * 2.0 : endAngle) * HALF_CIRCLE_ANGLE / M_PI;
    if (NearEqual(param.startAngle, param.endAngle)) {
        return; // Just return when startAngle is same as endAngle.
    }
    double rotation = param.rotation * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (param.anticlockwise) {
        if (sweepAngle > 0.0) { // Make sure the sweepAngle is negative when anticlockwise.
            sweepAngle -= FULL_CIRCLE_ANGLE;
        }
    } else {
        if (sweepAngle < 0.0) { // Make sure the sweepAngle is positive when clockwise.
            sweepAngle += FULL_CIRCLE_ANGLE;
        }
    }

    // Init the oval Rect(left, top, right, bottom).
    double left = param.x - param.radiusX + offset.GetX();
    double top = param.y - param.radiusY + offset.GetY();
    double right = param.x + param.radiusX + offset.GetX();
    double bottom = param.y + param.radiusY + offset.GetY();
    auto rect = SkRect::MakeLTRB(left, top, right, bottom);
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(-rotation, param.x + offset.GetX(), param.y + offset.GetY());
        skPath_.transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(param.endAngle - param.startAngle)) {
        // The entire ellipse needs to be drawn with two arcTo.
        skPath_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        skPath_.arcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(rotation, param.x + offset.GetX(), param.y + offset.GetY());
        skPath_.transform(matrix);
    }
}

void CustomPaintPaintMethod::BezierCurveTo(PaintWrapper* paintWrapper, const BezierCurveParam& param)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    skPath_.cubicTo(SkDoubleToScalar(param.cp1x + offset.GetX()), SkDoubleToScalar(param.cp1y + offset.GetY()),
        SkDoubleToScalar(param.cp2x + offset.GetX()), SkDoubleToScalar(param.cp2y + offset.GetY()),
        SkDoubleToScalar(param.x + offset.GetX()), SkDoubleToScalar(param.y + offset.GetY()));
}

void CustomPaintPaintMethod::QuadraticCurveTo(PaintWrapper* paintWrapper, const QuadraticCurveParam& param)
{
    OffsetF offset = GetContentOffset(paintWrapper);
    skPath_.quadTo(SkDoubleToScalar(param.cpx + offset.GetX()), SkDoubleToScalar(param.cpy + offset.GetY()),
        SkDoubleToScalar(param.x + offset.GetX()), SkDoubleToScalar(param.y + offset.GetY()));
}

void CustomPaintPaintMethod::ParsePath2D(const OffsetF& offset, const RefPtr<CanvasPath2D>& path)
{
    for (const auto& [cmd, args] : path->GetCaches()) {
        switch (cmd) {
            case PathCmd::CMDS: {
                Path2DAddPath(offset, args);
                break;
            }
            case PathCmd::TRANSFORM: {
                Path2DSetTransform(offset, args);
                break;
            }
            case PathCmd::MOVE_TO: {
                Path2DMoveTo(offset, args);
                break;
            }
            case PathCmd::LINE_TO: {
                Path2DLineTo(offset, args);
                break;
            }
            case PathCmd::ARC: {
                Path2DArc(offset, args);
                break;
            }
            case PathCmd::ARC_TO: {
                Path2DArcTo(offset, args);
                break;
            }
            case PathCmd::QUADRATIC_CURVE_TO: {
                Path2DQuadraticCurveTo(offset, args);
                break;
            }
            case PathCmd::BEZIER_CURVE_TO: {
                Path2DBezierCurveTo(offset, args);
                break;
            }
            case PathCmd::ELLIPSE: {
                Path2DEllipse(offset, args);
                break;
            }
            case PathCmd::RECT: {
                Path2DRect(offset, args);
                break;
            }
            case PathCmd::CLOSE_PATH: {
                Path2DClosePath(offset, args);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void CustomPaintPaintMethod::Path2DAddPath(const OffsetF& offset, const PathArgs& args)
{
    SkPath out;
    SkParsePath::FromSVGString(args.cmds.c_str(), &out);
    skPath2d_.addPath(out);
}

void CustomPaintPaintMethod::Path2DClosePath(const OffsetF& offset, const PathArgs& args)
{
    skPath2d_.close();
}

void CustomPaintPaintMethod::Path2DMoveTo(const OffsetF& offset, const PathArgs& args)
{
    double x = args.para1 + offset.GetX();
    double y = args.para2 + offset.GetY();
    skPath2d_.moveTo(x, y);
}

void CustomPaintPaintMethod::Path2DLineTo(const OffsetF& offset, const PathArgs& args)
{
    double x = args.para1 + offset.GetX();
    double y = args.para2 + offset.GetY();
    skPath2d_.lineTo(x, y);
}

void CustomPaintPaintMethod::Path2DArc(const OffsetF& offset, const PathArgs& args)
{
    double x = args.para1;
    double y = args.para2;
    double r = args.para3;
    auto rect =
        SkRect::MakeLTRB(x - r + offset.GetX(), y - r + offset.GetY(), x + r + offset.GetX(), y + r + offset.GetY());
    double startAngle = args.para4 * HALF_CIRCLE_ANGLE / M_PI;
    double endAngle = args.para5 * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (!NearZero(args.para6)) {
        sweepAngle =
            endAngle > startAngle ? (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) - FULL_CIRCLE_ANGLE) : sweepAngle;
    } else {
        sweepAngle =
            endAngle > startAngle ? sweepAngle : (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) + FULL_CIRCLE_ANGLE);
    }
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE + HALF_CIRCLE_ANGLE, sweepAngle, false);
    } else {
        skPath2d_.arcTo(rect, startAngle, sweepAngle, false);
    }
}

void CustomPaintPaintMethod::Path2DArcTo(const OffsetF& offset, const PathArgs& args)
{
    double x1 = args.para1 + offset.GetX();
    double y1 = args.para2 + offset.GetY();
    double x2 = args.para3 + offset.GetX();
    double y2 = args.para4 + offset.GetY();
    double r = args.para5;
    skPath2d_.arcTo(x1, y1, x2, y2, r);
}

void CustomPaintPaintMethod::Path2DEllipse(const OffsetF& offset, const PathArgs& args)
{
    if (NearEqual(args.para6, args.para7)) {
        return; // Just return when startAngle is same as endAngle.
    }

    double x = args.para1;
    double y = args.para2;
    double rx = args.para3;
    double ry = args.para4;
    double rotation = args.para5 * HALF_CIRCLE_ANGLE / M_PI;
    double startAngle = std::fmod(args.para6, M_PI * 2.0);
    double endAngle = std::fmod(args.para7, M_PI * 2.0);
    bool anticlockwise = NearZero(args.para8) ? false : true;
    startAngle = (startAngle < 0.0 ? startAngle + M_PI * 2.0 : startAngle) * HALF_CIRCLE_ANGLE / M_PI;
    endAngle = (endAngle < 0.0 ? endAngle + M_PI * 2.0 : endAngle) * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (anticlockwise) {
        if (sweepAngle > 0.0) { // Make sure the sweepAngle is negative when anticlockwise.
            sweepAngle -= FULL_CIRCLE_ANGLE;
        }
    } else {
        if (sweepAngle < 0.0) { // Make sure the sweepAngle is positive when clockwise.
            sweepAngle += FULL_CIRCLE_ANGLE;
        }
    }
    auto rect = SkRect::MakeLTRB(
        x - rx + offset.GetX(), y - ry + offset.GetY(), x + rx + offset.GetX(), y + ry + offset.GetY());

    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(-rotation, x + offset.GetX(), y + offset.GetY());
        skPath2d_.transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(args.para6 - args.para7)) {
        // The entire ellipse needs to be drawn with two arcTo.
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        skPath2d_.arcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(rotation, x + offset.GetX(), y + offset.GetY());
        skPath2d_.transform(matrix);
    }
}

void CustomPaintPaintMethod::Path2DBezierCurveTo(const OffsetF& offset, const PathArgs& args)
{
    double cp1x = args.para1 + offset.GetX();
    double cp1y = args.para2 + offset.GetY();
    double cp2x = args.para3 + offset.GetX();
    double cp2y = args.para4 + offset.GetY();
    double x = args.para5 + offset.GetX();
    double y = args.para6 + offset.GetY();
    skPath2d_.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
}

void CustomPaintPaintMethod::Path2DQuadraticCurveTo(const OffsetF& offset, const PathArgs& args)
{
    double cpx = args.para1 + offset.GetX();
    double cpy = args.para2 + offset.GetY();
    double x = args.para3 + offset.GetX();
    double y = args.para4 + offset.GetY();
    skPath2d_.quadTo(cpx, cpy, x, y);
}

void CustomPaintPaintMethod::Path2DSetTransform(const OffsetF& offset, const PathArgs& args)
{
    SkMatrix skMatrix;
    double scaleX = args.para1;
    double skewX = args.para2;
    double skewY = args.para3;
    double scaleY = args.para4;
    double translateX = args.para5;
    double translateY = args.para6;
    skMatrix.setAll(scaleX, skewY, translateX, skewX, scaleY, translateY, 0.0f, 0.0f, 1.0f);
    skPath2d_.transform(skMatrix);
}

void CustomPaintPaintMethod::Save()
{
    SaveStates();
    skCanvas_->save();
}

void CustomPaintPaintMethod::Restore()
{
    RestoreStates();
    skCanvas_->restore();
}

void CustomPaintPaintMethod::Scale(double x, double y)
{
    skCanvas_->scale(x, y);
}

void CustomPaintPaintMethod::Rotate(double angle)
{
    skCanvas_->rotate(angle * 180 / M_PI);
}

void CustomPaintPaintMethod::ResetTransform()
{
    skCanvas_->resetMatrix();
}

void CustomPaintPaintMethod::Transform(const TransformParam& param)
{
    SkMatrix skMatrix;
    skMatrix.setAll(param.scaleX, param.skewY, param.translateX, param.skewX, param.scaleY, param.translateY, 0, 0, 1);
    skCanvas_->concat(skMatrix);
}

void CustomPaintPaintMethod::Translate(double x, double y)
{
    skCanvas_->translate(x, y);
}

double CustomPaintPaintMethod::GetAlignOffset(TextAlign align, std::unique_ptr<txt::Paragraph>& paragraph)
{
    double x = 0.0;
    TextDirection textDirection = fillState_.GetOffTextDirection();
    switch (align) {
        case TextAlign::LEFT:
            x = 0.0;
            break;
        case TextAlign::START:
            x = (textDirection == TextDirection::LTR) ? 0.0 : -paragraph->GetMaxIntrinsicWidth();
            break;
        case TextAlign::RIGHT:
            x = -paragraph->GetMaxIntrinsicWidth();
            break;
        case TextAlign::END:
            x = (textDirection == TextDirection::LTR) ? -paragraph->GetMaxIntrinsicWidth() : 0.0;
            break;
        case TextAlign::CENTER:
            x = -paragraph->GetMaxIntrinsicWidth() / 2.0;
            break;
        default:
            x = 0.0;
            break;
    }
    return x;
}

txt::TextAlign CustomPaintPaintMethod::GetEffectiveAlign(txt::TextAlign align, txt::TextDirection direction) const
{
    if (align == txt::TextAlign::start) {
        return (direction == txt::TextDirection::ltr) ? txt::TextAlign::left
                                                      : txt::TextAlign::right;
    } else if (align == txt::TextAlign::end) {
        return (direction == txt::TextDirection::ltr) ? txt::TextAlign::right
                                                      : txt::TextAlign::left;
    } else {
        return align;
    }
}

void CustomPaintPaintMethod::ClearPaintImage(SkPaint& paint)
{
    float matrix[20] = { 0.0f };
    matrix[0] = matrix[6] = matrix[12] = matrix[18] = 1.0f;
#ifdef USE_SYSTEM_SKIA
    paint.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
#else
    paint.setColorFilter(SkColorFilters::Matrix(matrix));
#endif
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 0));
#ifdef NEW_SKIA
    paint.setImageFilter(SkImageFilters::Blur(0, 0, nullptr));
#else
    paint.setImageFilter(SkBlurImageFilter::Make(0, 0, nullptr));
#endif
}

void CustomPaintPaintMethod::SetPaintImage(SkPaint& paint)
{
    FilterType filterType;
    std::string filterParam;
    if (!GetFilterType(filterType, filterParam)) {
        return;
    }
    switch (filterType) {
        case FilterType::NONE:
            break;
        case FilterType::GRAYSCALE:
            SetGrayFilter(filterParam, paint);
            break;
        case FilterType::SEPIA:
            SetSepiaFilter(filterParam, paint);
            break;
        case FilterType::SATURATE:
            SetSaturateFilter(filterParam, paint);
            break;
        case FilterType::HUE_ROTATE:
            SetHueRotateFilter(filterParam, paint);
            break;
        case FilterType::INVERT:
            SetInvertFilter(filterParam, paint);
            break;
        case FilterType::OPACITY:
            SetOpacityFilter(filterParam, paint);
            break;
        case FilterType::BRIGHTNESS:
            SetBrightnessFilter(filterParam, paint);
            break;
        case FilterType::CONTRAST:
            SetContrastFilter(filterParam, paint);
            break;
        case FilterType::BLUR:
            SetBlurFilter(filterParam, paint);
            break;
        case FilterType::DROP_SHADOW:
            LOGW("Dropshadow is not supported yet.");
            break;
        default:
            LOGE("invalid type of filter");
    }
}

// https://drafts.fxtf.org/filter-effects/#grayscaleEquivalent
void CustomPaintPaintMethod::SetGrayFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    if (percentNum > 1) {
        percentNum = 1;
    }

    float matrix[20] = { 0.0f };
    float value = 1 - percentNum;

    matrix[0] = LUMR + (1 - LUMR) * value;
    matrix[5] = LUMR - LUMR * value;
    matrix[10] = LUMR - LUMR * value;

    matrix[1] = LUMG - LUMG * value;
    matrix[6] = LUMG + (1 - LUMG) * value;
    matrix[11] = LUMG - LUMG * value;

    matrix[2] = LUMB - LUMB * value;
    matrix[7] = LUMB - LUMB * value;
    matrix[12] = LUMB + (1 - LUMB) * value;

    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

// https://drafts.fxtf.org/filter-effects/#sepiaEquivalent
void CustomPaintPaintMethod::SetSepiaFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    if (percentNum > 1) {
        percentNum = 1;
    }
    float matrix[20] = { 0.0f };
    matrix[0] = 1.0f - percentNum * 0.607f;
    matrix[1] = percentNum * 0.769f;
    matrix[2] = percentNum * 0.189f;

    matrix[5] = percentNum * 0.349f;
    matrix[6] = 1.0f - percentNum * 0.314f;
    matrix[7] = percentNum * 0.168f;

    matrix[10] = percentNum * 0.272f;
    matrix[11] = percentNum * 0.534f;
    matrix[12] = 1.0f - percentNum * 0.869f;

    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

// https://drafts.fxtf.org/filter-effects/#saturateEquivalent
void CustomPaintPaintMethod::SetSaturateFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    float matrix[20] = { 0.0f };

    matrix[0] = LUMR + (1 - LUMR) * percentNum;
    matrix[5] = LUMR - LUMR * percentNum;
    matrix[10] = LUMR - LUMR * percentNum;

    matrix[1] = LUMG - LUMG * percentNum;
    matrix[6] = LUMG + (1 - LUMG) * percentNum;
    matrix[11] = LUMG - LUMG * percentNum;

    matrix[2] = LUMB - LUMB * percentNum;
    matrix[7] = LUMB - LUMB * percentNum;
    matrix[12] = LUMB + (1 - LUMB) * percentNum;

    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

// https://drafts.fxtf.org/filter-effects/#huerotateEquivalent
void CustomPaintPaintMethod::SetHueRotateFilter(const std::string& filterParam, SkPaint& paint)
{
    std::string percent = filterParam;
    float rad = 0.0f;
    if (percent.find("deg") != std::string::npos) {
        size_t index = percent.find("deg");
        percent.resize(index);
        std::istringstream iss(percent);
        iss >> rad;
        rad = rad / HALF_CIRCLE_ANGLE * M_PI;
    }
    if (percent.find("turn") != std::string::npos) {
        size_t index = percent.find("turn");
        percent.resize(index);
        std::istringstream iss(percent);
        iss >> rad;
        rad = rad * 2 * M_PI;
    }
    if (percent.find("rad") != std::string::npos) {
        size_t index = percent.find("rad");
        percent.resize(index);
        std::istringstream iss(percent);
        iss >> rad;
    }

    float cosValue = std::cos(rad);
    float sinValue = std::sin(rad);
    float matrix[20] = { 0.0f };

    matrix[0] = LUMR + cosValue * (1 - LUMR) + sinValue * (-LUMR);
    matrix[5] = LUMR + cosValue * (-LUMR) + sinValue * 0.143f;
    matrix[10] = LUMR + cosValue * (-LUMR) + sinValue * (LUMR - 1);

    matrix[1] = LUMG + cosValue * (-LUMG) + sinValue * (-LUMG);
    matrix[6] = LUMG + cosValue * (1 - LUMG) + sinValue * 0.140f;
    matrix[11] = LUMG + cosValue * (-LUMG) + sinValue * LUMG;

    matrix[2] = LUMB + cosValue * (-LUMB) + sinValue * (1 - LUMB);
    matrix[7] = LUMB + cosValue * (-LUMB) + sinValue * (-0.283f);
    matrix[12] = LUMB + cosValue * (1 - LUMB) + sinValue * LUMB;

    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

/*
 * https://drafts.fxtf.org/filter-effects/#invertEquivalent
 * Example for R in RGB:
 * v0 = percentNum, v1 = 1 - percentNum, n = 1
 * If 0 <= R < 1,
 * k / n <= R < (k + 1) / n => R * n - 1 < k <= R * n => k = 0
 * R' = funcR(R) = v0 + (R - k / n) * n * (v1 - v0) = percentNum + (1 - 2 * percentNum) * R
 * If R==1, R' = v1 = 1 - percentNum = percentNum + (1 - 2 * percentNum) * R
 * so R' = funcR(R) = percentNum + (1 - 2 * percentNum) * R, where 0 <= R <= 1.
 */
void CustomPaintPaintMethod::SetInvertFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    if (percentNum > 1) {
        percentNum = 1;
    }
    float matrix[20] = { 0.0f };
    matrix[0] = matrix[6] = matrix[12] = 1.0 - 2.0 * percentNum;
    matrix[4] = matrix[9] = matrix[14] = percentNum;
    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

/*
 * https://drafts.fxtf.org/filter-effects/#opacityEquivalent
 * A is short for Alpha:
 * v0 = 0, v1 = percentNum, n = 1
 * If 0 <= A < 1, k = 0. reference：SetInvertFilter.
 * A' = funcR(A) = v0 + (A - k / n) * n * (v1 - v0) = percentNum * A
 * If A==1, A' = v1 = percentNum = percentNum * A
 * so A' = funcR(A) = percentNum * A, where 0 <= A <= 1.
 */
void CustomPaintPaintMethod::SetOpacityFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    if (percentNum > 1) {
        percentNum = 1;
    }
    float matrix[20] = { 0.0f };
    matrix[0] = matrix[6] = matrix[12] = 1.0f;
    matrix[18] = percentNum;
    SetColorFilter(matrix, paint);
}

/*
 * https://drafts.fxtf.org/filter-effects/#brightnessEquivalent
 * Example for R in RGB:
 * R' = funcR(R) = slope * R + intercept
 * where: slope = percentNum, intercept = 0
 */
void CustomPaintPaintMethod::SetBrightnessFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    if (percentNum < 0) {
        return;
    }
    float matrix[20] = { 0.0f };
    matrix[0] = matrix[6] = matrix[12] = percentNum;
    matrix[18] = 1.0f;
    SetColorFilter(matrix, paint);
}

/*
 * https://drafts.fxtf.org/filter-effects/#contrastEquivalent
 * Example for R in RGB:
 * R' = funcR(R) = slope * R + intercept
 * where: slope = percentNum, intercept = 0.5 * (1 - percentNum)
 */
void CustomPaintPaintMethod::SetContrastFilter(const std::string& percent, SkPaint& paint)
{
    float percentNum = PercentStrToFloat(percent);
    float matrix[20] = { 0.0f };
    matrix[0] = matrix[6] = matrix[12] = percentNum;
    matrix[4] = matrix[9] = matrix[14] = 0.5f * (1 - percentNum);
    matrix[18] = 1;
    SetColorFilter(matrix, paint);
}

// https://drafts.fxtf.org/filter-effects/#blurEquivalent
void CustomPaintPaintMethod::SetBlurFilter(const std::string& percent, SkPaint& paint)
{
#ifdef NEW_SKIA
    paint.setImageFilter(SkImageFilters::Blur(BlurStrToDouble(percent), BlurStrToDouble(percent), nullptr));
#else
    paint.setImageFilter(SkBlurImageFilter::Make(BlurStrToDouble(percent), BlurStrToDouble(percent), nullptr));
#endif
}

void CustomPaintPaintMethod::SetColorFilter(float matrix[20], SkPaint& paint)
{
#ifdef USE_SYSTEM_SKIA
    matrix[4] *= 255;
    matrix[9] *= 255;
    matrix[14] *= 255;
    matrix[19] *= 255;
    paint.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
#else
    paint.setColorFilter(SkColorFilters::Matrix(matrix));
#endif
}

bool CustomPaintPaintMethod::GetFilterType(FilterType& filterType, std::string& filterParam)
{
    std::string paramData = filterParam_;
    size_t index = paramData.find("(");
    if (index == std::string::npos) {
        return false;
    }
    filterType = FilterStrToFilterType(paramData.substr(0, index));
    filterParam = paramData.substr(index + 1);
    size_t endIndex = filterParam.find(")");
    if (endIndex  == std::string::npos) {
        return false;
    }
    filterParam.erase(endIndex, 1);
    return true;
}

bool CustomPaintPaintMethod::IsPercentStr(std::string& percent)
{
    if (percent.find("%") != std::string::npos) {
        size_t index = percent.find("%");
        percent = percent.substr(0, index);
        return true;
    }
    return false;
}

double CustomPaintPaintMethod::PxStrToDouble(const std::string& str)
{
    double ret = 0;
    size_t index = str.find("px");
    if (index != std::string::npos) {
        std::string result = str.substr(0, index);
        std::istringstream iss(result);
        iss >> ret;
    }
    return ret;
}

double CustomPaintPaintMethod::BlurStrToDouble(const std::string& str)
{
    double ret = 0;
    size_t index = str.find("px");
    size_t index1 = str.find("rem");
    size_t demIndex = std::string::npos;
    if (index != std::string::npos) {
        demIndex = index;
    }
    if (index1 != std::string::npos) {
        demIndex = index1;
    }
    if (demIndex != std::string::npos) {
        std::string result = str.substr(0, demIndex);
        std::istringstream iss(result);
        iss >> ret;
    }
    if (str.find("rem") != std::string::npos) {
        return ret * 15;
    }
    return ret;
}

float CustomPaintPaintMethod::PercentStrToFloat(const std::string& percentStr)
{
    std::string percentage = percentStr;
    bool hasPercent = IsPercentStr(percentage);
    float percentNum = 0.0f;
    std::istringstream iss(percentage);
    iss >> percentNum;
    if (hasPercent) {
        percentNum = percentNum / 100;
    }
    return percentNum;
}

FilterType CustomPaintPaintMethod::FilterStrToFilterType(const std::string& filterStr)
{
    const LinearMapNode<FilterType> filterTypeTable[] = {
        { "blur", FilterType::BLUR },
        { "brightness", FilterType::BRIGHTNESS },
        { "contrast", FilterType::CONTRAST },
        { "drop-shadow", FilterType::DROP_SHADOW },
        { "grayscale", FilterType::GRAYSCALE },
        { "hue-rotate", FilterType::HUE_ROTATE },
        { "invert", FilterType::INVERT },
        { "none", FilterType::NONE },
        { "opacity", FilterType::OPACITY },
        { "saturate", FilterType::SATURATE },
        { "sepia", FilterType::SEPIA },
    };
    return ConvertStrToEnum(filterStr.c_str(), filterTypeTable, ArraySize(filterTypeTable), FilterType::NONE);
}

bool CustomPaintPaintMethod::HasImageShadow() const
{
    return !(NearZero(imageShadow_->GetOffset().GetX()) && NearZero(imageShadow_->GetOffset().GetY()) &&
         NearZero(imageShadow_->GetBlurRadius()));
}
} // namespace OHOS::Ace::NG

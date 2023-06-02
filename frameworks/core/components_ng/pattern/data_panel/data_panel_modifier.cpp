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

#include "core/components_ng/pattern/data_panel/data_panel_modifier.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/color.h"
#include "core/components/data_panel/data_panel_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/data_panel/data_panel_paint_property.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float FIXED_WIDTH = 1.0f;
constexpr float HALF_CIRCLE = 180.0f;
constexpr float WHOLE_CIRCLE = 360.0f;
constexpr float QUARTER_CIRCLE = 90.0f;
constexpr float PERCENT_HALF = 0.5f;
constexpr float DIAMETER_TO_THICKNESS_RATIO = 0.12f;
} // namespace

DataPanelModifier::DataPanelModifier()
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto theme = pipelineContext->GetTheme<DataPanelTheme>();
    auto colors = theme->GetColorsArray();

    date_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0);
    for (size_t i = 0; i < MAX_COUNT; i++) {
        auto value = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0);
        AttachProperty(value);
        values_.emplace_back(value);
    }
    max_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_MAX_VALUE);
    trackBackgroundColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(theme->GetBackgroundColor()));
    strokeWidth_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetThickness().ConvertToPx());
    isEffect_ = AceType::MakeRefPtr<PropertyBool>(true);
    AttachProperty(date_);
    AttachProperty(max_);
    AttachProperty(trackBackgroundColor_);
    AttachProperty(strokeWidth_);
    AttachProperty(isEffect_);

    shadowRadiusFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowRadius().ConvertToPx());
    shadowOffsetXFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowOffsetX().ConvertToPx());
    shadowOffsetYFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowOffsetY().ConvertToPx());
    AttachProperty(shadowRadiusFloat_);
    AttachProperty(shadowOffsetXFloat_);
    AttachProperty(shadowOffsetYFloat_);

    for (const auto& item : colors) {
        Gradient gradient;
        GradientColor gradientColorStart;
        gradientColorStart.SetLinearColor(LinearColor(item.first));
        gradientColorStart.SetDimension(Dimension(0.0));
        gradient.AddColor(gradientColorStart);
        GradientColor gradientColorEnd;
        gradientColorEnd.SetLinearColor(LinearColor(item.second));
        gradientColorEnd.SetDimension(Dimension(1.0));
        gradient.AddColor(gradientColorEnd);

        auto gradientColor = AceType::MakeRefPtr<AnimatablePropertyVectorColor>(GradientArithmetic(gradient));
        AttachProperty(gradientColor);
        valueColors_.emplace_back(gradientColor);

        auto gradientShadowColor = AceType::MakeRefPtr<AnimatablePropertyVectorColor>(GradientArithmetic(gradient));
        AttachProperty(gradientShadowColor);
        shadowColors_.emplace_back(gradientShadowColor);
    }
}

void DataPanelModifier::onDraw(DrawingContext& context)
{
    float date = date_->Get();
    if (dataPanelType_ == 0) {
        PaintCircle(context, offset_, date);
    } else {
        PaintLinearProgress(context, offset_);
    }
}

void DataPanelModifier::UpdateDate()
{
    if (date_ && isFirstAnimate_) {
        // When the date update, the animation will repeat once.
        date_->Set(ANIMATION_START);
        AnimationOption option = AnimationOption();
        RefPtr<Curve> curve = AceType::MakeRefPtr<SpringCurve>(
            ANIMATION_CURVE_VELOCITY, ANIMATION_CURVE_MASS, ANIMATION_CURVE_STIFFNESS, ANIMATION_CURVE_DAMPING);
        option.SetDuration(ANIMATION_DURATION);
        option.SetDelay(ANIMATION_DELAY);
        option.SetCurve(curve);
        option.SetIteration(ANIMATION_TIMES);
        AnimationUtils::Animate(option, [&]() { date_->Set(ANIMATION_END); });
        isFirstAnimate_ = false;
    }
}

void DataPanelModifier::PaintRainbowFilterMask(RSCanvas& canvas, double factor, ArcData arcData) const
{
    float thickness = arcData.thickness;
    float radius = arcData.radius;
    float progress = arcData.progress;
    if (GreatNotEqual(progress, DEFAULT_MAX_VALUE)) {
        progress = DEFAULT_MAX_VALUE;
    }
    if (LessNotEqual(progress, 0.0f)) {
        progress = 0.0f;
    }
    if (NearEqual(progress, 0.0f)) {
        return;
    }
    Offset center = arcData.center + Offset(shadowOffsetXFloat_->Get(), shadowOffsetYFloat_->Get());
    PointF centerPt = PointF(center.GetX(), center.GetY() - radius + thickness * PERCENT_HALF);

    // for example whole circle is 100 which is divided into 100 piece 360 / 100 = 3.6
    float drawAngle = arcData.wholeAngle * 0.01 * progress;
    float startAngle = arcData.startAngle;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = arcData.shadowColor.GetColors().size();
    for (size_t i = 0; i < length; i++) {
        colors.emplace_back(arcData.shadowColor.GetColors().at(i).GetLinearColor().GetValue());
        if (arcData.gradientPointBase == 0.0) {
            pos.emplace_back(arcData.shadowColor.GetColors().at(i).GetDimension().Value());
        } else {
            auto itemPos = (1.0f - arcData.gradientPointBase) *
                arcData.shadowColor.GetColors().at(i).GetDimension().Value() + arcData.gradientPointBase;
            pos.emplace_back(itemPos);
        }
    }
    pos.at(pos.size() - 1) = 1.0f;
    RSPen gradientPaint;
    gradientPaint.SetWidth(thickness);
    gradientPaint.SetAntiAlias(true);
    RSFilter filter;
    filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL, shadowRadiusFloat_->Get()));
    gradientPaint.SetFilter(filter);
    RSPath path;
    RSRect rRect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);
    path.AddArc(rRect, startAngle, drawAngle);

    RSBrush startCirclePaint;
    startCirclePaint.SetAntiAlias(true);
    startCirclePaint.SetColor(arcData.shadowColor.GetColors().begin()->GetLinearColor().GetValue());
    startCirclePaint.SetFilter(filter);

    RSBrush endCirclePaint;
    endCirclePaint.SetAntiAlias(true);
    endCirclePaint.SetColor(arcData.shadowColor.GetColors().rbegin()->GetLinearColor().GetValue());
    endCirclePaint.SetFilter(filter);

    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::DECAL, 0, drawAngle));

    canvas.Save();
    canvas.AttachBrush(startCirclePaint);
    RSRect edgeRect(center.GetX() - thickness * PERCENT_HALF, center.GetY() - radius,
        center.GetX() + thickness * PERCENT_HALF, center.GetY() - radius + thickness);
    canvas.DrawArc(edgeRect, QUARTER_CIRCLE, HALF_CIRCLE);
    canvas.DetachBrush();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(-QUARTER_CIRCLE, center.GetX(), center.GetY());
    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::DECAL, 0, drawAngle));
    canvas.AttachPen(gradientPaint);
    canvas.DrawPath(path);
    canvas.DetachPen();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(drawAngle, center.GetX(), center.GetY());
    canvas.AttachBrush(endCirclePaint);
    canvas.DrawArc(edgeRect, -QUARTER_CIRCLE, HALF_CIRCLE);
    canvas.DetachBrush();
    canvas.Restore();
}

void DataPanelModifier::PaintCircle(DrawingContext& context, OffsetF offset, float date) const
{
    RSCanvas canvas = context.canvas;
    canvas.Save();
    canvas.Translate(offset.GetX(), offset.GetY());

    auto defaultThickness = strokeWidth_->Get();
    ArcData arcData;
    arcData.center = Offset(context.width * PERCENT_HALF, context.height * PERCENT_HALF);

    // Here radius will minus defaultThickness,when there will be new api to set padding, use the new padding.
    arcData.radius = std::min(context.width, context.height) * PERCENT_HALF - defaultThickness;
    if (defaultThickness >= arcData.radius) {
        arcData.thickness = arcData.radius * DIAMETER_TO_THICKNESS_RATIO;
    } else {
        arcData.thickness = defaultThickness;
    }
    arcData.wholeAngle = WHOLE_CIRCLE;
    arcData.startAngle = 0.0f;
    PaintTrackBackground(canvas, arcData, trackBackgroundColor_->Get().ToColor());
    double proportions = 1.0;
    double maxValue = max_->Get();
    if (LessOrEqual(maxValue, 0.0)) {
        maxValue = DEFAULT_MAX_VALUE;
    }
    double totalValue = 0.0;
    float factor = 1.0;
    for (size_t i = 0; i < valuesLastLength_; i++) {
        totalValue += values_[i]->Get();
    }
    if (GreatNotEqual(totalValue, maxValue)) {
        factor = maxValue / totalValue;
        proportions = DEFAULT_MAX_VALUE / totalValue;
    } else {
        proportions = maxValue == 0 ? 1.0 : DEFAULT_MAX_VALUE / maxValue;
    }
    totalValue = totalValue * proportions;
    for (int32_t i = static_cast<int32_t>(valuesLastLength_) - 1; i >= 0; i--) {
        arcData.progressColors = valueColors_[i]->Get().GetGradient();
        float totalValuePre = totalValue * 1.0f;
        if (isEffect_->Get() && GreatNotEqual(totalValue, 0.0)) {
            arcData.progress = totalValue * date;
        } else {
            arcData.progress = totalValue;
        }
        totalValue -= values_[i]->Get() * proportions;
        arcData.gradientPointBase = (totalValue * 1.0f) / totalValuePre;
        if ((isShadowVisible_ && (isHasShadowValue_ || isEffect_->Get())) && (i < shadowColorsLastLength_)) {
            arcData.shadowColor = shadowColors_[i]->Get().GetGradient();
            PaintRainbowFilterMask(canvas, factor * date, arcData);
        }
        PaintProgress(canvas, arcData, isEffect_->Get(), false, 0.0);
    }

    canvas.Restore();
}

void DataPanelModifier::PaintLinearProgress(DrawingContext& context, OffsetF offset) const
{
    auto canvas = context.canvas;
    auto totalWidth = context.width;
    auto spaceWidth = SystemProperties::Vp2Px(FIXED_WIDTH);
    auto segmentWidthSum = 0.0f;
    auto segmentSize = 0.0;
    for (size_t i = 0; i < valuesLastLength_; i++) {
        if (NearZero(values_[i]->Get())) {
            continue;
        }
        segmentWidthSum += values_[i]->Get();
        if (LessNotEqual(segmentWidthSum, max_->Get())) {
            segmentSize++;
        } else {
            break;
        }
    }

    float scaleMaxValue = 0.0f;
    if (Positive(max_->Get())) {
        scaleMaxValue = (totalWidth - segmentSize * spaceWidth) / max_->Get();
    }

    auto widthSegment = offset.GetX();
    PaintBackground(canvas, offset, totalWidth, context.height);
    bool isStopPaint = false;
    float totalPaintWidth = 0.0f;
    float preWidthSegment = 0.0f;
    for (size_t i = 0; i < valuesLastLength_; i++) {
        auto segmentWidth = values_[i]->Get();
        if (NonPositive(segmentWidth)) {
            continue;
        }
        LinearData segmentLinearData;
        segmentLinearData.offset = offset;
        segmentLinearData.height = context.height;
        totalPaintWidth += segmentWidth;

        if (i == 0) {
            segmentLinearData.isFirstData = true;
        }
        if (GreatOrEqual(totalPaintWidth, max_->Get())) {
            segmentLinearData.isEndData = true;
            isStopPaint = true;
        }
        segmentLinearData.segmentColor = valueColors_[i]->Get().GetGradient();
        segmentLinearData.segmentWidth = segmentWidth * scaleMaxValue;
        segmentLinearData.xSegment = widthSegment;
        preWidthSegment = widthSegment;
        if (GreatOrEqual(segmentLinearData.segmentWidth + segmentLinearData.xSegment, totalWidth)) {
            segmentLinearData.segmentWidth = totalWidth - preWidthSegment;
        }
        if ((isShadowVisible_ && (isHasShadowValue_ || isEffect_->Get())) && (i < shadowColorsLastLength_)) {
            segmentLinearData.segmentShadowColor = shadowColors_[i]->Get().GetGradient();
            PaintColorSegmentFilterMask(canvas, segmentLinearData);
        }
        PaintColorSegment(canvas, segmentLinearData);
        if (isStopPaint) {
            break;
        }
        widthSegment += values_[i]->Get() * scaleMaxValue;
        if (!segmentLinearData.isEndData) {
            PaintSpace(canvas, offset, spaceWidth, widthSegment, segmentLinearData.height);
            widthSegment += spaceWidth;
        }
    }
}

void DataPanelModifier::PaintBackground(RSCanvas& canvas, OffsetF offset, float totalWidth, float height) const
{
    RSBrush brush;
    brush.SetColor(ToRSColor(trackBackgroundColor_->Get()));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    RSRect rRect(offset.GetX() - (height * PERCENT_HALF), offset.GetY(),
        totalWidth + offset.GetX() + (height * PERCENT_HALF), height + offset.GetY());
    RSRoundRect rrRect(rRect, height, height);
    canvas.DrawRoundRect(rrRect);
    canvas.DetachBrush();
}

void DataPanelModifier::PaintColorSegment(RSCanvas& canvas, const LinearData& segmentLinearData) const
{
    auto offset = segmentLinearData.offset;
    auto xSegment = segmentLinearData.xSegment;
    auto segmentWidth = segmentLinearData.segmentWidth;
    auto height = segmentLinearData.height;
    RSBrush brush;
    RSRect rect(xSegment, offset.GetY(), xSegment + segmentWidth, offset.GetY() + height);
    RSPoint segmentStartPoint;
    segmentStartPoint.SetX(rect.GetLeft());
    segmentStartPoint.SetY(rect.GetTop());
    RSPoint segmentEndPoint;
    segmentEndPoint.SetX(rect.GetRight());
    segmentEndPoint.SetY(rect.GetBottom());
    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = segmentLinearData.segmentColor.GetColors().size();
    for (size_t i = 0; i < length; i++) {
        colors.emplace_back(segmentLinearData.segmentColor.GetColors().at(i).GetLinearColor().GetValue());
        pos.emplace_back(segmentLinearData.segmentColor.GetColors().at(i).GetDimension().Value());
    }

    if (segmentLinearData.isFirstData) {
        RSBrush startCirclePaint;
        startCirclePaint.SetAntiAlias(true);
        startCirclePaint.SetColor(segmentLinearData.segmentColor.GetColors().begin()->GetLinearColor().GetValue());
        canvas.Save();
        canvas.AttachBrush(startCirclePaint);
        RSRect edgeRect(xSegment - (height * PERCENT_HALF), offset.GetY(),
            xSegment + height * PERCENT_HALF, offset.GetY() + height);
        canvas.DrawArc(edgeRect, QUARTER_CIRCLE, HALF_CIRCLE);
        canvas.DetachBrush();
        canvas.Restore();
    }

    canvas.Save();
    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
    canvas.AttachBrush(brush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
    canvas.Restore();

    if (segmentLinearData.isEndData) {
        RSBrush endCirclePaint;
        endCirclePaint.SetAntiAlias(true);
        endCirclePaint.SetColor(segmentLinearData.segmentColor.GetColors().rbegin()->GetLinearColor().GetValue());

        canvas.Save();
        canvas.AttachBrush(endCirclePaint);
        RSRect edgeRectEnd(xSegment + segmentWidth - (height * PERCENT_HALF), offset.GetY(),
            xSegment + segmentWidth + height * PERCENT_HALF, offset.GetY() + height);
        canvas.DrawArc(edgeRectEnd, -QUARTER_CIRCLE, HALF_CIRCLE);
        canvas.DetachBrush();
        canvas.Restore();
    }
}

void DataPanelModifier::PaintColorSegmentFilterMask(RSCanvas& canvas, const LinearData& segmentLinearData) const
{
    auto offset = segmentLinearData.offset;
    auto xSegment = segmentLinearData.xSegment;
    auto segmentWidth = segmentLinearData.segmentWidth;
    auto height = segmentLinearData.height;
    RSBrush brush;
    RSFilter filter;
    filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL, shadowRadiusFloat_->Get()));
    brush.SetFilter(filter);

    RSRect rect(xSegment + shadowOffsetXFloat_->Get(), offset.GetY() + shadowOffsetYFloat_->Get(),
        xSegment + segmentWidth + shadowOffsetXFloat_->Get(), offset.GetY() + height + shadowOffsetYFloat_->Get());
    RSPoint segmentStartPoint;
    segmentStartPoint.SetX(rect.GetLeft());
    segmentStartPoint.SetY(rect.GetTop());
    RSPoint segmentEndPoint;
    segmentEndPoint.SetX(rect.GetRight());
    segmentEndPoint.SetY(rect.GetBottom());

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = segmentLinearData.segmentShadowColor.GetColors().size();
    for (size_t i = 0; i < length; i++) {
        colors.emplace_back(segmentLinearData.segmentShadowColor.GetColors().at(i).GetLinearColor().GetValue());
        pos.emplace_back(segmentLinearData.segmentShadowColor.GetColors().at(i).GetDimension().Value());
    }

    if (segmentLinearData.isFirstData) {
        RSBrush startCirclePaint;
        startCirclePaint.SetAntiAlias(true);
        startCirclePaint.SetColor(
            segmentLinearData.segmentShadowColor.GetColors().begin()->GetLinearColor().GetValue());
        startCirclePaint.SetFilter(filter);
        canvas.Save();
        canvas.AttachBrush(startCirclePaint);
        RSRect edgeRect(xSegment + shadowOffsetXFloat_->Get() - (height * PERCENT_HALF) + 1,
            offset.GetY() + shadowOffsetYFloat_->Get(),
            xSegment + shadowOffsetXFloat_->Get() + height * PERCENT_HALF + 1,
            offset.GetY() + shadowOffsetYFloat_->Get() + height);
        canvas.DrawArc(edgeRect, QUARTER_CIRCLE, HALF_CIRCLE);
        canvas.DetachBrush();
        canvas.Restore();
    }

    canvas.Save();
    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
    canvas.AttachBrush(brush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
    canvas.Restore();

    if (segmentLinearData.isEndData) {
        RSBrush endCirclePaint;
        endCirclePaint.SetAntiAlias(true);
        endCirclePaint.SetColor(segmentLinearData.segmentShadowColor.GetColors().rbegin()->GetLinearColor().GetValue());
        endCirclePaint.SetFilter(filter);
        canvas.Save();
        canvas.AttachBrush(endCirclePaint);
        RSRect edgeRectEnd(xSegment + shadowOffsetXFloat_->Get() + segmentWidth - (height * PERCENT_HALF),
            offset.GetY() + shadowOffsetYFloat_->Get(),
            xSegment + shadowOffsetXFloat_->Get() + segmentWidth + height * PERCENT_HALF,
            offset.GetY() + shadowOffsetYFloat_->Get() + height);
        canvas.DrawArc(edgeRectEnd, -QUARTER_CIRCLE, HALF_CIRCLE);
        canvas.DetachBrush();
        canvas.Restore();
    }
}

void DataPanelModifier::PaintSpace(RSCanvas& canvas, OffsetF offset, float spaceWidth, float xSpace, float height) const
{
    RSBrush brush;
    RSRect rect(xSpace, offset.GetY(), xSpace + spaceWidth, offset.GetY() + height);
    brush.SetColor(ToRSColor(Color::WHITE));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
}

void DataPanelModifier::PaintTrackBackground(RSCanvas& canvas, ArcData arcData, const Color color) const
{
    RSPen backgroundTrackData;
    RSPath backgroundTrackPath;
    auto center = arcData.center;
    float thickness = arcData.thickness;
    float radius = arcData.radius;

    RSRect rect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);

    backgroundTrackPath.AddArc(rect, 0.0, WHOLE_CIRCLE);
    backgroundTrackData.SetColor(ToRSColor(color));
    backgroundTrackData.SetAntiAlias(true);
    backgroundTrackData.SetWidth(thickness);

    canvas.AttachPen(backgroundTrackData);
    canvas.DrawPath(backgroundTrackPath);
    canvas.DetachPen();
}

void DataPanelModifier::PaintProgress(
    RSCanvas& canvas, ArcData arcData, bool useEffect, bool useAnimator, float percent) const
{
    float thickness = arcData.thickness;
    float radius = arcData.radius;
    float progress = arcData.progress;

    if (GreatNotEqual(progress, DEFAULT_MAX_VALUE)) {
        progress = DEFAULT_MAX_VALUE;
    }
    if (LessNotEqual(progress, 0.0f)) {
        progress = 0.0f;
    }
    if (NearEqual(progress, 0.0f)) {
        return;
    }
    Offset center = arcData.center;
    PointF centerPt = PointF(center.GetX(), center.GetY() - radius + thickness * PERCENT_HALF);

    // for example whole circle is 100 which is divided into 100 piece 360 / 100 = 3.6
    float drawAngle = arcData.wholeAngle * 0.01 * progress;
    float startAngle = arcData.startAngle;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = arcData.progressColors.GetColors().size();
    for (size_t i = 0; i < length; i++) {
        colors.emplace_back(arcData.progressColors.GetColors().at(i).GetLinearColor().GetValue());
        if (arcData.gradientPointBase == 0.0) {
            pos.emplace_back(arcData.progressColors.GetColors().at(i).GetDimension().Value());
        } else {
            auto itemPos = (1.0f - arcData.gradientPointBase) *
                arcData.progressColors.GetColors().at(i).GetDimension().Value() + arcData.gradientPointBase;
            pos.emplace_back(itemPos);
        }
    }
    pos.at(pos.size() - 1) = 1.0f;

    RSPen gradientPaint;
    gradientPaint.SetWidth(thickness);
    gradientPaint.SetAntiAlias(true);
    RSPath path;
    RSRect rRect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);
    path.AddArc(rRect, startAngle, drawAngle);

    RSBrush startCirclePaint;
    startCirclePaint.SetAntiAlias(true);
    startCirclePaint.SetColor(arcData.progressColors.GetColors().begin()->GetLinearColor().GetValue());

    RSBrush endCirclePaint;
    endCirclePaint.SetAntiAlias(true);
    endCirclePaint.SetColor(arcData.progressColors.GetColors().rbegin()->GetLinearColor().GetValue());

    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::CLAMP, 0, drawAngle));

    canvas.Save();
    canvas.AttachBrush(startCirclePaint);
    RSRect edgeRect(center.GetX() - thickness * PERCENT_HALF, center.GetY() - radius,
        center.GetX() + thickness * PERCENT_HALF, center.GetY() - radius + thickness);
    canvas.DrawArc(edgeRect, QUARTER_CIRCLE, HALF_CIRCLE);
    canvas.DetachBrush();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(-QUARTER_CIRCLE, center.GetX(), center.GetY());
    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::CLAMP, 0, drawAngle));
    canvas.AttachPen(gradientPaint);
    canvas.DrawPath(path);
    canvas.DetachPen();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(drawAngle, center.GetX(), center.GetY());
    canvas.AttachBrush(endCirclePaint);
    canvas.DrawArc(edgeRect, -QUARTER_CIRCLE, HALF_CIRCLE);
    canvas.DetachBrush();
    canvas.Restore();
}
} // namespace OHOS::Ace::NG

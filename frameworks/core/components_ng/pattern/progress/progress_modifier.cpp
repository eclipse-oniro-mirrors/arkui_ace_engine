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

#include "core/components_ng/pattern/progress/progress_modifier.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/progress/progress_theme.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/progress/progress_paint_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t DEFAULT_BORDER_COLOR = 0x33007dff;
constexpr int32_t INT32_TWO = 2;
constexpr int32_t ANGLE_45 = 45;
constexpr int32_t ANGLE_90 = 90;
constexpr int32_t ANGLE_180 = 180;
constexpr int32_t ANGLE_270 = 270;
constexpr int32_t ANGLE_360 = 360;
constexpr int32_t LOADING_ANIMATION_DURATION = 2000;
constexpr float DEFAULT_MAX_VALUE = 100.0f;
constexpr float DEFAULT_SCALE_WIDTH = 10.0f;
constexpr int32_t DEFAULT_SCALE_COUNT = 100;
constexpr double DEFAULT_CAPSULE_BORDER_WIDTH = 0.0;
constexpr float FLOAT_ZERO_FIVE = 0.5f;
constexpr float FLOAT_TWO_ZERO = 2.0f;
constexpr float SPRING_MOTION_RESPONSE = 0.314f;
constexpr float SPRING_MOTION_DAMPING_FRACTION = 0.95f;
constexpr float SWEEPING_MOTION_RESPONSE = 1.62f;
constexpr float SWEEPING_MOTION_DAMPING_RATIO = 0.98f;
constexpr Dimension SWEEP_WIDTH = 80.0_vp;
constexpr float RING_SHADOW_OFFSET_X = 5.0f;
constexpr float RING_SHADOW_OFFSET_Y = 5.0f;
constexpr float RING_SHADOW_BLUR_RADIUS_MIN = 5.0f;
constexpr float RING_SHADOW_VALID_RADIUS_MIN = 10.0f;
constexpr Dimension LINEAR_SWEEPING_LEN = 80.0_vp;
constexpr float OPACITY_MAX = 1.0f;
constexpr float OPACITY_MIN = 0.001f;
constexpr float POINT_INTERVAL = 2.0f;
} // namespace
ProgressModifier::ProgressModifier()
    : strokeWidth_(AceType::MakeRefPtr<AnimatablePropertyFloat>(FLOAT_TWO_ZERO)),
      color_(AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor::BLUE)),
      bgColor_(AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor::GRAY)),
      borderColor_(AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(DEFAULT_BORDER_COLOR))),
      value_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      ringProgressColors_(AceType::MakeRefPtr<AnimatablePropertyVectorColor>(GradientArithmetic())),
      sweepingDate_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      trailingHeadDate_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      trailingTailDate_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      offset_(AceType::MakeRefPtr<PropertyOffsetF>(OffsetF())),
      contentSize_(AceType::MakeRefPtr<PropertySizeF>(SizeF())),
      maxValue_(AceType::MakeRefPtr<PropertyFloat>(DEFAULT_MAX_VALUE)),
      scaleWidth_(AceType::MakeRefPtr<PropertyFloat>(DEFAULT_SCALE_WIDTH)),
      scaleCount_(AceType::MakeRefPtr<PropertyInt>(DEFAULT_SCALE_COUNT)),
      progressType_(AceType::MakeRefPtr<PropertyInt>(static_cast<int32_t>(ProgressType::LINEAR))),
      capsuleBorderWidth_(AceType::MakeRefPtr<PropertyFloat>(DEFAULT_CAPSULE_BORDER_WIDTH)),
      sweepEffect_(AceType::MakeRefPtr<PropertyBool>(false)),
      paintShadow_(AceType::MakeRefPtr<PropertyBool>(false)),
      progressStatus_(AceType::MakeRefPtr<PropertyInt>(static_cast<int32_t>(ProgressStatus::PROGRESSING)))
{
    AttachProperty(strokeWidth_);
    AttachProperty(color_);
    AttachProperty(bgColor_);
    AttachProperty(borderColor_);
    AttachProperty(maxValue_);
    AttachProperty(value_);
    AttachProperty(scaleWidth_);
    AttachProperty(scaleCount_);
    AttachProperty(progressType_);
    AttachProperty(capsuleBorderWidth_);
    AttachProperty(sweepEffect_);
    AttachProperty(trailingHeadDate_);
    AttachProperty(trailingTailDate_);

    AttachProperty(ringProgressColors_);
    AttachProperty(sweepingDate_);
    AttachProperty(paintShadow_);
    AttachProperty(progressStatus_);
}

void ProgressModifier::onDraw(DrawingContext& context)
{
    ContentDrawWithFunction(context);
}

void ProgressModifier::SetStrokeWidth(float width)
{
    CHECK_NULL_VOID(strokeWidth_);
    strokeWidth_->Set(width);
}

void ProgressModifier::SetColor(LinearColor color)
{
    CHECK_NULL_VOID(color_);
    color_->Set(color);
}

void ProgressModifier::SetBackgroundColor(LinearColor color)
{
    CHECK_NULL_VOID(bgColor_);
    bgColor_->Set(color);
}

void ProgressModifier::SetBorderColor(LinearColor color)
{
    CHECK_NULL_VOID(borderColor_);
    borderColor_->Set(color);
}

void ProgressModifier::SetProgressType(ProgressType type)
{
    CHECK_NULL_VOID(progressType_);
    progressType_->Set(static_cast<int32_t>(type));
}

void ProgressModifier::ProcessSweepingAnimation(ProgressType type, float value)
{
    if (!isVisible_) {
        return;
    }

    if (type == ProgressType::CAPSULE) {
        StartCapsuleSweepingAnimation(value);
    }
}

void ProgressModifier::StartCapsuleSweepingAnimation(float value)
{
    float curLength = 0.0f;
    auto contentSize = contentSize_->Get();
    if (contentSize.Width() >= contentSize.Height()) {
        curLength = (value / maxValue_->Get()) * contentSize_->Get().Width() + SWEEP_WIDTH.ConvertToPx();
    } else {
        curLength = (value / maxValue_->Get()) * contentSize_->Get().Height() + SWEEP_WIDTH.ConvertToPx();
    }
    
    if (!isSweeping_ && sweepEffect_->Get() && isVisible_) {
        StartCapsuleSweepingAnimationImpl(curLength);
    } else if (!sweepEffect_->Get() || !isVisible_) {
        isSweeping_ = false;
        AnimationOption option = AnimationOption();
        auto curve = AceType::MakeRefPtr<LinearCurve>();
        option.SetCurve(curve);
        option.SetDuration(0);
        option.SetIteration(-1);
        AnimationUtils::Animate(option, [&]() { sweepingDate_->Set(0.0f); });
    } else {
        sweepingDate_->Set(curLength);
        if (!NearEqual(sweepingDateBackup_, curLength)) {
            sweepingDateUpdated_ = true;
        }
    }
    sweepingDateBackup_ = curLength;
}

void ProgressModifier::StartCapsuleSweepingAnimationImpl(float value)
{
    isSweeping_ = true;
    sweepingDate_->Set(0.0f);
    AnimationOption option = AnimationOption();
    auto motion =
        AceType::MakeRefPtr<ResponsiveSpringMotion>(SWEEPING_MOTION_RESPONSE, SWEEPING_MOTION_DAMPING_RATIO);
    option.SetCurve(motion);
    option.SetIteration(-1);

    AnimationUtils::Animate(option,
        [value, id = Container::CurrentId(), weak = WeakClaim(this)]() {
            ContainerScope scope(id);
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->sweepingDate_->Set(value);
        },
        [id = Container::CurrentId(), weak = WeakClaim(this)]() {
            ContainerScope scope(id);
            auto context = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(context);
            auto taskExecutor = context->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID_NOLOG(modifier);
                    if (modifier->sweepEffect_->Get() && modifier->isVisible_) {
                        if (modifier->sweepingDateUpdated_) {
                            modifier->StartCapsuleSweepingAnimationImpl(modifier->sweepingDateBackup_);
                        }
                        modifier->sweepingDateUpdated_ = false;
                    }
                },
                TaskExecutor::TaskType::UI);
        },
        [id = Container::CurrentId(), weak = WeakClaim(this)]() {
            ContainerScope scope(id);
            auto context = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(context);
            auto taskExecutor = context->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID_NOLOG(modifier);
                    if (modifier->sweepingDateUpdated_ || !modifier->isVisible_) {
                        if (modifier->isSweeping_) {
                            modifier->isSweeping_ = false;
                            AnimationOption option = AnimationOption();
                            auto curve = AceType::MakeRefPtr<LinearCurve>();
                            option.SetCurve(curve);
                            option.SetDuration(0);
                            option.SetIteration(1);
                            AnimationUtils::Animate(option, [modifier]() { modifier->sweepingDate_->Set(0.0f); });
                        }
                    }
                },
                TaskExecutor::TaskType::UI);
        });
}

void ProgressModifier::SetRingProgressColor(const Gradient& color)
{
    CHECK_NULL_VOID(ringProgressColors_);
    ringProgressColors_->Set(GradientArithmetic(color));
}

void ProgressModifier::SetPaintShadow(bool paintShadow)
{
    CHECK_NULL_VOID(paintShadow_);
    paintShadow_->Set(paintShadow);
}

void ProgressModifier::SetProgressStatus(ProgressStatus status)
{
    CHECK_NULL_VOID(progressStatus_);
    if (status == ProgressStatus::LOADING) {
        StartRingLoadingAnimation();
    }
    progressStatus_->Set(static_cast<int32_t>(status));
}

void ProgressModifier::SetVisible(bool isVisible)
{
    CHECK_NULL_VOID(isVisible_ != isVisible);
    isVisible_ = isVisible;

    if (!isVisible) {
        if (isLoading_) {
            StopRingLoadingHeadAnimation();
            StopRingLoadingTailAnimation();
        }

        if (isSweeping_) {
            StopSweepingAnimation();
        }
    }
}

void ProgressModifier::StartRingLoadingAnimation()
{
    if (!isLoading_ && isVisible_) {
        isLoading_ = true;
        StartRingLoadingHeadAnimation();
        StartRingLoadingTailAnimation();
    }
}

void ProgressModifier::StartRingLoadingHeadAnimation()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    bool isFormRender = context->IsFormRender();
    AnimationOption optionHead = AnimationOption();
    auto curveHead = AceType::MakeRefPtr<TailingHeadCurve>();
    optionHead.SetDuration(LOADING_ANIMATION_DURATION);
    optionHead.SetCurve(curveHead);
    optionHead.SetIteration(isFormRender ? 1 : -1);
    AnimationUtils::Animate(optionHead,
        [&]() { trailingHeadDate_->Set(ANGLE_360); },
        nullptr,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    if (static_cast<ProgressStatus>(
                        modifier->progressStatus_->Get()) == ProgressStatus::PROGRESSING) {
                        modifier->StopRingLoadingHeadAnimation();
                    }
                },
                TaskExecutor::TaskType::UI);
        });
}

void ProgressModifier::StartRingLoadingTailAnimation()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    bool isFormRender = context->IsFormRender();
    AnimationOption optionTail = AnimationOption();
    auto curveTail = AceType::MakeRefPtr<CubicCurve>(0.5f, 0.15f, 0.75f, 0.45f);
    optionTail.SetDuration(LOADING_ANIMATION_DURATION);
    optionTail.SetCurve(curveTail);
    optionTail.SetIteration(isFormRender ? 1 : -1);
    AnimationUtils::Animate(optionTail,
        [&]() { trailingTailDate_->Set(ANGLE_360); },
        nullptr,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    if (static_cast<ProgressStatus>(modifier->progressStatus_->Get()) ==
                        ProgressStatus::PROGRESSING) {
                        modifier->StopRingLoadingTailAnimation();
                        if (GreatOrEqual(modifier->valueBackup_, 0.0f)) {
                            modifier->SetValue(modifier->valueBackup_);
                        }
                    }
                },
                TaskExecutor::TaskType::UI);
        });
}

void ProgressModifier::StopRingLoadingHeadAnimation()
{
    AnimationOption option = AnimationOption();
    option.SetDuration(0);
    AnimationUtils::Animate(option, [&]() { trailingHeadDate_->Set(0.0f); });
}

void ProgressModifier::StopRingLoadingTailAnimation()
{
    AnimationOption option = AnimationOption();
    option.SetDuration(0);
    AnimationUtils::Animate(option, [&]() { trailingTailDate_->Set(0.0f); });
    isLoading_ = false;
}

void ProgressModifier::ProcessRingSweepingAnimation(float value)
{
    if (NearZero(value) || NearEqual(value, maxValue_->Get())) {
        StopSweepingAnimation();
    } else {
        StartRingSweepingAnimation(value);
    }
}

void ProgressModifier::ProcessLinearSweepingAnimation(float value)
{
    if (NearZero(value) || NearEqual(value, maxValue_->Get())) {
        StopSweepingAnimation();
    } else {
        StartLinearSweepingAnimation(value);
    }
}

void ProgressModifier::StartRingSweepingAnimation(float value)
{
    float maxValue = NearZero(maxValue_->Get()) ? 1.0f : maxValue_->Get();
    float date = value / maxValue * ANGLE_360 + ANGLE_45;
    float additionalAngle = CalcRingProgressAdditionalAngle();
    date += additionalAngle * 2;

    if (!isSweeping_) {
        StartRingSweepingAnimationImpl(date);
    } else {
        sweepingDate_->Set(date);
        if (!NearEqual(sweepingDateBackup_, date)) {
            sweepingDateUpdated_ = true;
        }
    }

    sweepingDateBackup_ = date;
}

void ProgressModifier::StartRingSweepingAnimationImpl(float date)
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    bool isFormRender = context->IsFormRender();
    isSweeping_ = true;
    AnimationOption option = AnimationOption();
    auto motion =
            AceType::MakeRefPtr<ResponsiveSpringMotion>(SWEEPING_MOTION_RESPONSE, SWEEPING_MOTION_DAMPING_RATIO);
    option.SetCurve(motion);
    option.SetIteration(isFormRender ? 1 : -1);
    AnimationUtils::Animate(option,
        [&]() { sweepingDate_->Set(date); },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    CHECK_NULL_VOID(modifier->isVisible_);
                    if (modifier->sweepingDateUpdated_) {
                        modifier->sweepingDateUpdated_ = false;
                        modifier->StartRingSweepingAnimationImpl(modifier->sweepingDateBackup_);
                    }
                },
                TaskExecutor::TaskType::UI);
        },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    if (modifier->sweepingDateUpdated_) {
                        modifier->StopSweepingAnimation();
                    }
                },
                TaskExecutor::TaskType::UI);
        });
}

void ProgressModifier::StopSweepingAnimation()
{
    if (isSweeping_) {
        isSweeping_ = false;
        AnimationOption option = AnimationOption();
        option.SetDuration(0);
        AnimationUtils::Animate(option, [&]() { sweepingDate_->Set(0.0f); });
    }
}

float ProgressModifier::CalcRingProgressAdditionalAngle() const
{
    auto contentSize = contentSize_->Get();
    auto strokeWidth = strokeWidth_->Get();
    PointF centerPt = PointF(contentSize.Width() / 2, contentSize.Height() / 2);
    float radius = std::min(contentSize.Width() / 2, contentSize.Height() / 2);
    return atan2f(strokeWidth / 2, radius - strokeWidth / 2) * ANGLE_180 / PI_NUM;
}

void ProgressModifier::StartLinearSweepingAnimation(float value)
{
    float date = 0.0f;
    auto contentSize = contentSize_->Get();
    float radius = strokeWidth_->Get() / 2;
    float barLength = 0.0f;
    float dateLength = 0.0f;

    if (GreatOrEqual(contentSize.Width(), contentSize.Height())) {
        barLength = contentSize.Width() - radius * 2;
    } else {
        barLength = contentSize.Height() - radius * 2;
    }

    if (NearEqual(barLength, 0.0f)) {
        return;
    }

    dateLength = barLength * value / maxValue_->Get();
    if (NearEqual(dateLength, 0.0f)) {
        return;
    }

    date = dateLength + LINEAR_SWEEPING_LEN.ConvertToPx();

    if (!isSweeping_) {
        StartLinearSweepingAnimationImpl(date);
    } else {
        sweepingDate_->Set(date);
        if (!NearEqual(sweepingDateBackup_, date)) {
            sweepingDateUpdated_ = true;
        }
    }

    sweepingDateBackup_ = date;
}

void ProgressModifier::StartLinearSweepingAnimationImpl(float date)
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    bool isFormRender = context->IsFormRender();
    isSweeping_ = true;
    sweepingDate_->Set(0.0f);
    AnimationOption option = AnimationOption();
    auto motion =
        AceType::MakeRefPtr<ResponsiveSpringMotion>(SWEEPING_MOTION_RESPONSE, SWEEPING_MOTION_DAMPING_RATIO);
    option.SetCurve(motion);
    option.SetIteration(isFormRender ? 1 : -1);

    AnimationUtils::Animate(option,
        [&]() { sweepingDate_->Set(date); },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    CHECK_NULL_VOID(modifier->isVisible_);
                    if (modifier->sweepingDateUpdated_) {
                        modifier->sweepingDateUpdated_ = false;
                        modifier->StartLinearSweepingAnimationImpl(modifier->sweepingDateBackup_);
                    }
                },
                TaskExecutor::TaskType::UI);
        },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, id]() {
                    ContainerScope scope(id);
                    auto modifier = weak.Upgrade();
                    CHECK_NULL_VOID(modifier);
                    if (modifier->sweepingDateUpdated_) {
                        modifier->StopSweepingAnimation();
                    }
                },
                TaskExecutor::TaskType::UI);
        });
}

void ProgressModifier::SetMaxValue(float value)
{
    CHECK_NULL_VOID(maxValue_);
    maxValue_->Set(value);
}

void ProgressModifier::SetValue(float value)
{
    valueBackup_ = value;

    if (isLoading_) {
        return;
    }

    CHECK_NULL_VOID(value_);
    AnimationOption option = AnimationOption();
    auto motion =
        AceType::MakeRefPtr<ResponsiveSpringMotion>(SPRING_MOTION_RESPONSE, SPRING_MOTION_DAMPING_FRACTION);
    option.SetCurve(motion);
    AnimationUtils::Animate(option, [&]() { value_->Set(value); });

    ProcessSweepingAnimation(ProgressType(progressType_->Get()), value);
}

void ProgressModifier::SetScaleWidth(float value)
{
    CHECK_NULL_VOID(scaleWidth_);
    scaleWidth_->Set(value);
}

void ProgressModifier::SetScaleCount(int32_t value)
{
    CHECK_NULL_VOID(scaleCount_);
    scaleCount_->Set(value);
}

void ProgressModifier::SetContentOffset(const OffsetF& offset)
{
    CHECK_NULL_VOID(offset_);
    offset_->Set(offset);
}

void ProgressModifier::SetContentSize(const SizeF& contentSize)
{
    CHECK_NULL_VOID(contentSize_);
    contentSize_->Set(contentSize);
}

void ProgressModifier::SetBorderWidth(float width)
{
    capsuleBorderWidth_->Set(width);
}

void ProgressModifier::SetSweepEffect(bool value)
{
    sweepEffect_->Set(value);
}

void ProgressModifier::ContentDrawWithFunction(DrawingContext& context)
{
    auto contentSize = contentSize_->Get();
    auto& canvas = context.canvas;
    if (progressType_->Get() == static_cast<int32_t>(ProgressType::LINEAR)) {
        PaintLinear(canvas, offset_->Get(), contentSize);
    } else if (progressType_->Get() == static_cast<int32_t>(ProgressType::RING)) {
        PaintRing(canvas, offset_->Get(), contentSize);
    } else if (progressType_->Get() == static_cast<int32_t>(ProgressType::SCALE)) {
        PaintScaleRing(canvas, offset_->Get(), contentSize);
    } else if (progressType_->Get() == static_cast<int32_t>(ProgressType::MOON)) {
        PaintMoon(canvas, offset_->Get(), contentSize);
    } else if (progressType_->Get() == static_cast<int32_t>(ProgressType::CAPSULE)) {
        if (contentSize.Width() >= contentSize.Height()) {
            PaintCapsule(canvas, offset_->Get(), contentSize);
        } else {
            PaintVerticalCapsule(canvas, offset_->Get(), contentSize);
        }
    } else {
        PaintLinear(canvas, offset_->Get(), contentSize);
    }
}

void ProgressModifier::PaintLinear(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(bgColor_->Get()));
    double radius = strokeWidth_->Get() / INT32_TWO;
    if (contentSize.Width() >= contentSize.Height()) {
        double barLength = contentSize.Width() - radius * INT32_TWO;
        CHECK_NULL_VOID(!NearEqual(barLength, 0.0));
        double dateLength = barLength * value_->Get() / maxValue_->Get();
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect(
            { { offset.GetX(), offset.GetY(), contentSize.Width() + offset.GetX(),
                                   strokeWidth_->Get() + offset.GetY() },
            radius, radius });
        // progress selected part
        CHECK_NULL_VOID(!NearEqual(dateLength, 0.0));
        brush.SetColor(ToRSColor((color_->Get())));
        canvas.AttachBrush(brush);
        RSPath path;
        path.AddRoundRect(
            { offset.GetX(), offset.GetY(), dateLength + offset.GetX() + strokeWidth_->Get(),
              strokeWidth_->Get() + offset.GetY() },
            radius, radius);
        canvas.DrawPath(path);
        canvas.DetachBrush();
        canvas.Restore();
        PaintLinearSweeping(canvas, offset, path, true);
    } else {
        double barLength = contentSize.Height() - radius * INT32_TWO;
        CHECK_NULL_VOID(!NearEqual(barLength, 0.0));
        double dateLength = barLength * value_->Get() / maxValue_->Get();
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect(
            { { offset.GetX(), offset.GetY(), strokeWidth_->Get() + offset.GetX(),
                                   contentSize.Height() + offset.GetY() },
            radius, radius });
        // progress selected part
        CHECK_NULL_VOID(!NearEqual(dateLength, 0.0));
        brush.SetColor(ToRSColor((color_->Get())));
        canvas.AttachBrush(brush);
        RSPath path;
        path.AddRoundRect(
            { offset.GetX(), offset.GetY(), strokeWidth_->Get() + offset.GetX(),
              dateLength + offset.GetY() + strokeWidth_->Get() },
            radius, radius);
        canvas.DrawPath(path);
        canvas.DetachBrush();
        canvas.Restore();
        PaintLinearSweeping(canvas, offset, path, false);
    }
}

void ProgressModifier::PaintLinearSweeping(
    RSCanvas& canvas, const OffsetF& offset, const RSPath& path, bool isHorizontal) const
{
    if (NearZero(value_->Get()) || NearZero(maxValue_->Get())) {
        LOGI("value or max value is 0.");
        return;
    }

    // If the progress reaches 100%, stop sweeping.
    if (NearEqual(value_->Get(), maxValue_->Get())) {
        LOGI("The progress reaches 100 percent, stop sweeping.");
        return;
    }

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    GenerateLinearSweepingGradientInfo(colors, pos);

    RSBrush brush;
    float leftTopX = 0.0f;
    float leftTopY = 0.0f;
    float rightBottomX = 0.0f;
    float rightBottomY = 0.0f;
    if (isHorizontal) {
        leftTopX = offset.GetX() + sweepingDate_->Get() - LINEAR_SWEEPING_LEN.ConvertToPx();
        leftTopY = offset.GetY();
        rightBottomX = offset.GetX() + sweepingDate_->Get();
        rightBottomY = offset.GetY() + strokeWidth_->Get();
    } else {
        leftTopX = offset.GetX();
        leftTopY = offset.GetY() + sweepingDate_->Get() - LINEAR_SWEEPING_LEN.ConvertToPx();
        rightBottomX = offset.GetX() + strokeWidth_->Get();
        rightBottomY = offset.GetY() + sweepingDate_->Get();
    }

    RSRect rect(leftTopX, leftTopY, rightBottomX, rightBottomY);
    brush.SetShaderEffect(RSShaderEffect::CreateLinearGradient(
        ToRSPoint(PointF(leftTopX, leftTopY)),
        ToRSPoint(PointF(rightBottomX, rightBottomY)),
        colors, pos, RSTileMode::CLAMP));
    canvas.Save();
    canvas.ClipPath(path, RSClipOp::INTERSECT, true);
    canvas.AttachBrush(brush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
    canvas.Restore();
}

void ProgressModifier::GenerateLinearSweepingGradientInfo(
    std::vector<RSColorQuad>& colors, std::vector<float>& pos) const
{
    Gradient gradient;
    Color sweepingColorBase = Color::WHITE;
    Color sweepingColorEnd = sweepingColorBase.ChangeOpacity(0.0);
    GradientColor gradientColorEnd;
    gradientColorEnd.SetColor(sweepingColorEnd);
    gradientColorEnd.SetDimension(Dimension(0.0, DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorEnd);

    // The sweep layer is a white gradient layer of length 80 vp with a opacity of 0.2 at 75 vp and 0.0 at both ends.
    Dimension stageLen = 75.0_vp;
    Color sweepingColorMiddle = sweepingColorBase.ChangeOpacity(0.2);
    GradientColor gradientColorMiddle;
    gradientColorMiddle.SetColor(sweepingColorMiddle);
    gradientColorMiddle.SetDimension(
        Dimension(stageLen.ConvertToPx() / LINEAR_SWEEPING_LEN.ConvertToPx(), DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorMiddle);

    Color sweepingColorStart = sweepingColorBase.ChangeOpacity(0.0);
    GradientColor gradientColorStart;
    gradientColorStart.SetColor(sweepingColorStart);
    gradientColorStart.SetDimension(Dimension(1.0, DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorStart);

    auto gradientColors = gradient.GetColors();
    for (size_t i = 0; i < gradientColors.size(); i++) {
        colors.emplace_back(gradientColors[i].GetColor().GetValue());
        pos.emplace_back(gradientColors[i].GetDimension().Value());
    }
}

void ProgressModifier::PaintRing(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    auto centerPt = PointF(contentSize.Width() / 2, contentSize.Height() / 2) + offset;
    auto radius = std::min(contentSize.Width() / 2, contentSize.Height() / 2);
    auto angle = (value_->Get() / maxValue_->Get()) * ANGLE_360;
    auto thickness = strokeWidth_->Get();
    // No shadow is drawn if radius is less or equal to 10, because it is no enough space to draw both ring and shadow.
    auto paintShadow = paintShadow_->Get() && GreatNotEqual(radius, RING_SHADOW_VALID_RADIUS_MIN);
    auto shadowBlurOffset = paintShadow ? thickness / 2 + std::max(RING_SHADOW_OFFSET_X, RING_SHADOW_OFFSET_Y) : 0.0f;
    if (GreatOrEqual(thickness + shadowBlurOffset, radius)) {
        LOGI("strokeWidth is lager than radius,  auto set strokeWidth as half of radius");
        thickness = radius / 2;
        shadowBlurOffset = paintShadow ? thickness / 2 + std::max(RING_SHADOW_OFFSET_X, RING_SHADOW_OFFSET_Y) : 0.0f;
    }
    // The shadowBlurSigma is an empirical value. If it is greater than thickness / 5, the shadow will be cut by
    // the canvas boundary.
    auto shadowBlurSigma = std::max(thickness / 5, RING_SHADOW_BLUR_RADIUS_MIN);
    radius = radius - thickness / 2 - shadowBlurOffset;

    RingProgressData ringData;
    ringData.centerPt = centerPt;
    ringData.radius = radius;
    ringData.angle = angle;
    ringData.thickness = thickness;
    ringData.shadowBlurSigma = shadowBlurSigma;

    PaintRingBackground(canvas, ringData);

    if (isLoading_) {
        PaintTrailing(canvas, ringData);
        return;
    }

    if (NearZero(angle)) {
        return;
    }

    if (paintShadow) {
        PaintRingProgressOrShadow(canvas, ringData, true);
    }

    PaintRingProgressOrShadow(canvas, ringData, false);
    PaintRingSweeping(canvas, ringData);
}

void ProgressModifier::PaintRingBackground(RSCanvas& canvas, const RingProgressData& ringProgressData) const
{
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(ringProgressData.thickness);
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));
    pen.SetColor(ToRSColor(bgColor_->Get()));

    canvas.Save();
    canvas.AttachPen(pen);
    canvas.DrawCircle(ToRSPoint(ringProgressData.centerPt), ringProgressData.radius);
    canvas.DetachPen();
    canvas.Restore();
}

void ProgressModifier::PaintRingProgressOrShadow(
    RSCanvas& canvas, const RingProgressData& ringProgressData, bool isShadow) const
{
    PointF centerPt = ringProgressData.centerPt;
    auto radius = ringProgressData.radius;
    auto angle = ringProgressData.angle;
    auto thickness = ringProgressData.thickness;
    double halfThickness = thickness / 2;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    auto gradientColors = ringProgressColors_->Get().GetGradient().GetColors();
    for (size_t i = 0; i < gradientColors.size(); i++) {
        colors.emplace_back(gradientColors[i].GetLinearColor().GetValue());
        pos.emplace_back(gradientColors[i].GetDimension().Value());
    }

    RSPen pen;
    pen.SetWidth(thickness);
    pen.SetAntiAlias(true);
    RSBrush startCirclePaint;
    startCirclePaint.SetAntiAlias(true);
    startCirclePaint.SetColor(gradientColors.back().GetLinearColor().GetValue());
    RSBrush endCirclePaint;
    endCirclePaint.SetAntiAlias(true);
    endCirclePaint.SetColor(gradientColors.front().GetLinearColor().GetValue());

    if (isShadow) {
        centerPt = centerPt + OffsetF(RING_SHADOW_OFFSET_X, RING_SHADOW_OFFSET_Y);
        RSFilter filter;
        filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL, ringProgressData.shadowBlurSigma));
        pen.SetFilter(filter);
        startCirclePaint.SetFilter(filter);
        endCirclePaint.SetFilter(filter);
    }

    RSRect edgeRect(centerPt.GetX() - halfThickness, centerPt.GetY() - radius - halfThickness,
        centerPt.GetX() + halfThickness, centerPt.GetY() - radius + halfThickness);
    // Paint begin-side semicircle
    canvas.Save();
    canvas.Rotate(angle, centerPt.GetX(), centerPt.GetY());
    canvas.AttachBrush(startCirclePaint);
    canvas.DrawArc(edgeRect, -ANGLE_90, ANGLE_180);
    canvas.DetachBrush();
    canvas.Restore();

    // Paint progress arc
    canvas.Save();
    canvas.Rotate(-ANGLE_90, centerPt.GetX(), centerPt.GetY());
    pen.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(centerPt.GetX(), centerPt.GetY())), colors, pos, RSTileMode::CLAMP, 0, angle));
    canvas.AttachPen(pen);
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius,
          centerPt.GetX() + radius, centerPt.GetY() + radius },
        0, angle);
    canvas.DetachPen();
    canvas.Restore();

    // Paint end-side semicircle
    canvas.Save();
    canvas.AttachBrush(endCirclePaint);
    canvas.DrawArc(edgeRect, ANGLE_90, ANGLE_180);
    canvas.DetachBrush();
    canvas.Restore();
}

void ProgressModifier::PaintRingSweeping(RSCanvas& canvas, const RingProgressData& ringProgressData) const
{
    if (NearZero(value_->Get()) || NearZero(maxValue_->Get())) {
        LOGI("value or max value is 0.");
        return;
    }

    // If the progress reaches 100%, stop sweeping.
    if (NearEqual(value_->Get(), maxValue_->Get())) {
        LOGI("The progress reaches 100 percent, stop sweeping.");
        return;
    }

    auto centerPt = ringProgressData.centerPt;
    auto radius = ringProgressData.radius;
    auto thickness = ringProgressData.thickness;
    double halfThickness = thickness / 2;

    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(thickness);
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    GenerateRingSweepingGradientInfo(colors, pos);

    float date = sweepingDate_->Get();
    float additionalAngle = CalcRingProgressAdditionalAngle();
    float startSweepAngle = date - ANGLE_45 - additionalAngle;
    pen.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(centerPt.GetX(), centerPt.GetY())), colors, pos,
        RSTileMode::CLAMP, 0, ANGLE_45));

    RSPath path;
    double totalDegree = ANGLE_360;
    double angle_arc = (value_->Get() / maxValue_->Get()) * totalDegree;
    double rad_arc = angle_arc / totalDegree * 2 * PI_NUM;
    double sin_point_inside = std::sin(rad_arc) * (radius - halfThickness);
    double cos_point_inside = std::cos(rad_arc) * (radius - halfThickness);
    if (LessOrEqual(startSweepAngle, additionalAngle)) {
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius + halfThickness);
        path.ArcTo(centerPt.GetX() - halfThickness, centerPt.GetY() - radius - halfThickness,
            centerPt.GetX() + halfThickness, centerPt.GetY() - radius + halfThickness,
            ANGLE_90, ANGLE_180);
        if (GreatNotEqual(angle_arc, ANGLE_180)) {
            path.ArcTo(centerPt.GetX() - radius - halfThickness, centerPt.GetY() - radius - halfThickness,
                centerPt.GetX() + radius + halfThickness, centerPt.GetY() + radius + halfThickness,
                -ANGLE_90, ANGLE_180);
            path.LineTo(centerPt.GetX(), centerPt.GetY() + radius - halfThickness);
            path.ArcTo(centerPt.GetX() - radius + halfThickness, centerPt.GetY() - radius + halfThickness,
                centerPt.GetX() + radius - halfThickness, centerPt.GetY() + radius - halfThickness,
                ANGLE_90, -ANGLE_180);
        } else {
            path.ArcTo(centerPt.GetX() - radius - halfThickness, centerPt.GetY() - radius - halfThickness,
                centerPt.GetX() + radius + halfThickness, centerPt.GetY() + radius + halfThickness,
                -ANGLE_90, angle_arc);
            path.ArcTo(halfThickness, halfThickness, 0.0f, RSPathDirection::CW_DIRECTION,
                centerPt.GetX() + sin_point_inside, centerPt.GetY() - cos_point_inside);
            path.ArcTo(centerPt.GetX() - radius + halfThickness, centerPt.GetY() - radius + halfThickness,
                centerPt.GetX() + radius - halfThickness, centerPt.GetY() + radius - halfThickness,
                -ANGLE_90 + angle_arc, -angle_arc);
        }
    } else if (LessOrEqual(startSweepAngle, totalDegree - ANGLE_45 - additionalAngle)) {
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius + halfThickness);
        path.ArcTo(centerPt.GetX() - halfThickness, centerPt.GetY() - radius - halfThickness,
            centerPt.GetX() + halfThickness, centerPt.GetY() - radius + halfThickness,
            ANGLE_90, ANGLE_180);
        path.ArcTo(centerPt.GetX() - radius - halfThickness, centerPt.GetY() - radius - halfThickness,
            centerPt.GetX() + radius + halfThickness, centerPt.GetY() + radius + halfThickness,
            -ANGLE_90, angle_arc);
        path.ArcTo(halfThickness, halfThickness, 0.0f, RSPathDirection::CW_DIRECTION,
            centerPt.GetX() + sin_point_inside, centerPt.GetY() - cos_point_inside);
        path.ArcTo(centerPt.GetX() - radius + halfThickness, centerPt.GetY() - radius + halfThickness,
            centerPt.GetX() + radius - halfThickness, centerPt.GetY() + radius - halfThickness,
            -ANGLE_90 + angle_arc, -angle_arc);
    } else {
        RSPath path1;
        path1.MoveTo(centerPt.GetX(), centerPt.GetY() + radius - halfThickness);
        path1.LineTo(centerPt.GetX(), centerPt.GetY() + radius + halfThickness);
        path1.ArcTo(centerPt.GetX() - radius - halfThickness, centerPt.GetY() - radius - halfThickness,
            centerPt.GetX() + radius + halfThickness, centerPt.GetY() + radius + halfThickness,
            ANGLE_90, angle_arc - ANGLE_180);
        path1.ArcTo(halfThickness, halfThickness, 0.0f, RSPathDirection::CW_DIRECTION,
            centerPt.GetX() + sin_point_inside, centerPt.GetY() - cos_point_inside);
        path1.ArcTo(centerPt.GetX() - radius + halfThickness, centerPt.GetY() - radius + halfThickness,
            centerPt.GetX() + radius - halfThickness, centerPt.GetY() + radius - halfThickness,
            -ANGLE_90 + angle_arc, -angle_arc + ANGLE_180);

        RSPath path2;
        RSPath path3;
        RSPath path4;
        path2.AddCircle(centerPt.GetX(), centerPt.GetY() - radius, halfThickness);
        float x = centerPt.GetX() + sin(rad_arc) * radius;
        float y = centerPt.GetY() - cos(rad_arc) * radius;
        path3.AddCircle(x, y, halfThickness);
        path4.Op(path2, path3, RSPathOp::INTERSECT);
        path.Op(path1, path4, RSPathOp::DIFFERENCE);
    }

    canvas.Save();
    canvas.ClipPath(path, RSClipOp::INTERSECT, true);

    float rotateAngle = -ANGLE_90 + startSweepAngle;
    canvas.Rotate(rotateAngle, centerPt.GetX(), centerPt.GetY());
    canvas.AttachPen(pen);
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius },
        0, ANGLE_45);
    canvas.DetachPen();
    canvas.Restore();
}

void ProgressModifier::GenerateRingSweepingGradientInfo(
    std::vector<RSColorQuad>& colors, std::vector<float>& pos) const
{
    Gradient gradient;
    Color sweepingColorBase = Color::WHITE;
    Color sweepingColorEnd = sweepingColorBase.ChangeOpacity(0.0f);
    GradientColor gradientColorEnd;
    gradientColorEnd.SetColor(sweepingColorEnd);
    gradientColorEnd.SetDimension(Dimension(0.0, DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorEnd);

    // The sweep layer is a 45-degree white gradient with an opacity of 0.7 at 40 degrees and 0 at both ends.
    Color sweepingColorMiddle = sweepingColorBase.ChangeOpacity(0.7f);
    GradientColor gradientColorMiddle;
    gradientColorMiddle.SetColor(sweepingColorMiddle);
    gradientColorMiddle.SetDimension(Dimension(40.0f / ANGLE_45, DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorMiddle);

    Color sweepingColorStart = sweepingColorBase.ChangeOpacity(0.0f);
    GradientColor gradientColorStart;
    gradientColorStart.SetColor(sweepingColorStart);
    gradientColorStart.SetDimension(Dimension(1.0, DimensionUnit::PERCENT));
    gradient.AddColor(gradientColorStart);

    auto gradientColors = gradient.GetColors();
    for (size_t i = 0; i < gradientColors.size(); i++) {
        colors.emplace_back(gradientColors[i].GetColor().GetValue());
        pos.emplace_back(gradientColors[i].GetDimension().Value());
    }
}

Color ProgressModifier::GenerateTrailingPointColor(
    const Color& headColor, const Color& tailColor, uint32_t index, uint32_t totalPoints) const
{
    float headRed = headColor.GetRed();
    float headGreen = headColor.GetGreen();
    float headBlue = headColor.GetBlue();
    float tailRed = tailColor.GetRed();
    float tailGreen = tailColor.GetGreen();
    float tailBlue = tailColor.GetBlue();
    float opacity = TrailingCoefficient(OPACITY_MAX, index, totalPoints);
    uint32_t red = tailRed - TrailingCoefficient(tailRed - headRed, index, totalPoints);
    uint32_t green = tailGreen - TrailingCoefficient(tailGreen - headGreen, index, totalPoints);
    uint32_t blue = tailBlue - TrailingCoefficient(tailBlue - headBlue, index, totalPoints);
    return Color::FromRGBO(red, green, blue, opacity);
}

float ProgressModifier::TrailingCoefficient(float base, uint32_t index, uint32_t totalPoints) const
{
    totalPoints = totalPoints ? totalPoints : 1;
    float trailingBaseNumber = std::pow(OPACITY_MIN, 1.0f / totalPoints);
    return base * std::pow(trailingBaseNumber, std::clamp(index, 1u, totalPoints) - 1);
}

void ProgressModifier::PaintTrailing(RSCanvas& canvas, const RingProgressData& ringProgressData) const
{
    PointF centerPt = ringProgressData.centerPt;
    double radius = ringProgressData.radius;
    double thickness = ringProgressData.thickness;
    double point_num = PI_NUM / ((std::asin(POINT_INTERVAL / (2.0f * radius)) * 2.0f) * 2.0f);
    float head = trailingHeadDate_->Get();
    float tail = trailingTailDate_->Get();

    RSBrush brush;
    brush.SetAntiAlias(true);
    canvas.Save();
    canvas.Translate(centerPt.GetX(), centerPt.GetY());
    std::vector<RSPoint> points;
    for (uint32_t i = 0; i < point_num; i++) {
        RSPoint point;
        float trailingAngle = tail + (head - tail) / point_num * i ;
        float trailingRad = trailingAngle * PI_NUM / ANGLE_180;
        point.SetX(std::sin(trailingRad) * radius);
        point.SetY(-std::cos(trailingRad) * radius);
        points.push_back(point);
    }

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<ProgressTheme>();
    CHECK_NULL_VOID(theme);

    Color headColor = theme->GetRingProgressBeginSideColor();
    Color tailColor = theme->GetRingProgressEndSideColor();
    auto pointSize = points.size();
    for (uint32_t i = 0; i < pointSize; i++) {
        RSPoint pointCenter = points[i];
        Color color = GenerateTrailingPointColor(headColor, tailColor, pointSize - i, pointSize);
        brush.SetColor(ToRSColor(color));
        canvas.AttachBrush(brush);
        canvas.DrawCircle(pointCenter, thickness / 2);
    }
    canvas.DetachBrush();
    canvas.Restore();
}

void ProgressModifier::PaintScaleRing(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    PointF centerPt = PointF(contentSize.Width() / INT32_TWO, contentSize.Height() / INT32_TWO) + offset;
    double radius = std::min(contentSize.Width() / INT32_TWO, contentSize.Height() / INT32_TWO);
    double lengthOfScale = strokeWidth_->Get();
    double pathDistance = FLOAT_TWO_ZERO * M_PI * radius / scaleCount_->Get();
    if (scaleWidth_->Get() > pathDistance) {
        LOGI("scaleWidth is lager than pathDistance,  auto changeto paint ring");
        PaintRing(canvas, offset, contentSize);
        return;
    }
    double widthOfLine = scaleWidth_->Get();
    RSPen pen;
    RSPath path;
    pen.SetWidth(widthOfLine);
    path.AddRoundRect(
        { 0, 0, widthOfLine, lengthOfScale }, widthOfLine / INT32_TWO, widthOfLine / INT32_TWO,
        RSPathDirection::CW_DIRECTION);
    pen.SetAntiAlias(true);
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));
    pen.SetPathEffect(RSPathEffect::CreatePathDashEffect(path, pathDistance, 0.0f, RSPathDashStyle::ROTATE));
    pen.SetColor(ToRSColor(bgColor_->Get()));
    canvas.AttachPen(pen);
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius },
        ANGLE_270, ANGLE_360);
    // start to draw cur value progress
    pen.SetColor(ToRSColor((color_->Get())));
    canvas.AttachPen(pen);
    double angle = (value_->Get() / maxValue_->Get()) * ANGLE_360;
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius },
        ANGLE_270, angle);
}

void ProgressModifier::PaintMoon(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    static int32_t totalDegree = 1;
    PointF centerPt = PointF(contentSize.Width() / INT32_TWO, contentSize.Height() / INT32_TWO) + offset;
    double radius = std::min(contentSize.Width() / INT32_TWO, contentSize.Height() / INT32_TWO);
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_->Get()));
    double angle = (value_->Get() / maxValue_->Get()) * totalDegree;
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawCircle(ToRSPoint(centerPt), radius);
    brush.SetColor(ToRSColor((color_->Get())));
    canvas.AttachBrush(brush);
    path.AddArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius },
        ANGLE_90, ANGLE_180);
    if (angle <= FLOAT_ZERO_FIVE) {
        double progressOffset = radius - radius * angle / FLOAT_ZERO_FIVE;
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius);
        // startAngle:270  sweepAngle:-180
        path.AddArc(
            { centerPt.GetX() - progressOffset, centerPt.GetY() - radius, centerPt.GetX() + progressOffset,
                        centerPt.GetY() + radius },
            ANGLE_270, -ANGLE_180);
        canvas.DrawPath(path);
    } else {
        double progressOffset = radius * (angle - FLOAT_ZERO_FIVE) / FLOAT_ZERO_FIVE;
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius);
        // startAngle:270  sweepAngle:180
        path.AddArc(
            { centerPt.GetX() - progressOffset, centerPt.GetY() - radius, centerPt.GetX() + progressOffset,
                        centerPt.GetY() + radius },
            ANGLE_270, ANGLE_180);
        canvas.DrawPath(path);
    }
}

void ProgressModifier::PaintCapsule(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    auto borderWidth = capsuleBorderWidth_->Get();
    if (GreatNotEqual(2 * borderWidth, contentSize.Height())) {
        borderWidth = contentSize.Height() / 2;
    }
    static int32_t totalDegree = 1;
    auto capsuleSize = contentSize;
    double radiusBig = std::min((capsuleSize.Width() - borderWidth) / INT32_TWO,
        (capsuleSize.Height() - borderWidth) / INT32_TWO);
    double offsetXBig = offset.GetX() + borderWidth / INT32_TWO;
    double offsetYBig = offset.GetY() + borderWidth / INT32_TWO;
    capsuleSize.SetWidth(contentSize.Width() - borderWidth * INT32_TWO);
    capsuleSize.SetHeight(contentSize.Height() - borderWidth * INT32_TWO);
    double radius = std::min(capsuleSize.Width() / INT32_TWO, capsuleSize.Height() / INT32_TWO);
    double offsetX = offset.GetX() + borderWidth;
    double offsetY = offset.GetY() + borderWidth;
    double progressWidth = (value_->Get() / maxValue_->Get()) * totalDegree * capsuleSize.Width();
    RSBrush brush;
    brush.SetAntiAlias(true);
    RSPen pen;
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_->Get()));
    pen.SetWidth(borderWidth);
    pen.SetAntiAlias(true);
    pen.SetColor(ToRSColor(borderColor_->Get()));
    canvas.AttachPen(pen);
    if (!NearZero(borderWidth)) {
        canvas.DrawRoundRect(
            { { offsetXBig, offsetYBig, contentSize.Width() - borderWidth + offsetXBig,
                contentSize.Height() - borderWidth + offsetYBig },
                radiusBig, radiusBig });
    }
    canvas.DetachPen();
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(
        { { offsetX, offsetY, capsuleSize.Width() + offsetX, capsuleSize.Height() + offsetY }, radius, radius });
    brush.SetColor(ToRSColor((color_->Get())));
    canvas.AttachBrush(brush);
    path.AddArc(
        { offsetX, offsetY, INT32_TWO * radius + offsetX, capsuleSize.Height() + offsetY }, ANGLE_90, ANGLE_180);
    if (LessNotEqual(progressWidth, radius)) {
        // startAngle:270  sweepAngle:-180
        path.AddArc(
            { offsetX + progressWidth, offsetY, INT32_TWO * radius - progressWidth + offsetX,
                        capsuleSize.Height() + offsetY },
            ANGLE_270, -ANGLE_180);
    } else if (GreatNotEqual(progressWidth, capsuleSize.Width() - radius)) {
        path.AddRect(
            { offsetX + radius, offsetY, capsuleSize.Width() + offsetX - radius, capsuleSize.Height() + offsetY });
        // startAngle:270  sweepAngle:180
        path.AddArc(
            { offsetX + (capsuleSize.Width() - radius) * FLOAT_TWO_ZERO - progressWidth, offsetY,
                        offsetX + progressWidth, capsuleSize.Height() + offsetY },
            ANGLE_270, ANGLE_180);
    } else {
        path.AddRect(
            { radius + offsetX, offsetY, progressWidth + offsetX, capsuleSize.Height() + offsetY });
    }
    canvas.DrawPath(path);
    canvas.Restore();

    PaintCapsuleLightSweep(canvas, capsuleSize, offset, path, false);
}

void ProgressModifier::PaintVerticalCapsule(RSCanvas& canvas, const OffsetF& offset, const SizeF& contentSize) const
{
    auto borderWidth = capsuleBorderWidth_->Get();
    if (GreatNotEqual(2 * borderWidth, contentSize.Width())) {
        borderWidth = contentSize.Width() / 2;
    }
    static int32_t totalDegree = 1;
    SizeF capsuleSize = contentSize;
    double radiusBig = std::min((capsuleSize.Width() - borderWidth) / INT32_TWO,
        (capsuleSize.Height() - borderWidth) / INT32_TWO);
    double offsetXBig = offset.GetX() + borderWidth / INT32_TWO;
    double offsetYBig = offset.GetY() + borderWidth / INT32_TWO;
    capsuleSize.SetWidth(contentSize.Width() - borderWidth * INT32_TWO);
    capsuleSize.SetHeight(contentSize.Height() - borderWidth * INT32_TWO);
    double radius = std::min(capsuleSize.Width() / INT32_TWO, capsuleSize.Height() / INT32_TWO);
    double offsetX = offset.GetX() + borderWidth;
    double offsetY = offset.GetY() + borderWidth;
    double progressWidth = (value_->Get() / maxValue_->Get()) * totalDegree * capsuleSize.Height();
    RSBrush brush;
    brush.SetAntiAlias(true);
    RSPen pen;
    pen.SetWidth(borderWidth);
    pen.SetAntiAlias(true);
    pen.SetColor(ToRSColor(borderColor_->Get()));
    canvas.AttachPen(pen);
    if (!NearZero(borderWidth)) {
        canvas.DrawRoundRect(
            { { offsetXBig, offsetYBig, contentSize.Width() - borderWidth + offsetXBig,
                contentSize.Height() - borderWidth + offsetYBig },
                radiusBig, radiusBig });
    }
    canvas.DetachPen();
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_->Get()));
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(
        { { offsetX, offsetY, capsuleSize.Width() + offsetX, capsuleSize.Height() + offsetY }, radius, radius });
    brush.SetColor(ToRSColor((color_->Get())));
    canvas.AttachBrush(brush);
    path.AddArc(
        { offsetX, offsetY, capsuleSize.Width() + offsetX, capsuleSize.Width() + offsetY }, 0, -ANGLE_180);
    if (LessNotEqual(progressWidth, radius)) {
        // startAngle:180  sweepAngle:180
        path.AddArc(
            { offsetX, offsetY + progressWidth, capsuleSize.Width() + offsetX,
                        capsuleSize.Width() - progressWidth + offsetY },
            ANGLE_180, ANGLE_180);
    } else if (GreatNotEqual(progressWidth, capsuleSize.Height() - radius)) {
        path.AddRect(
            { offsetX, offsetY + radius, capsuleSize.Width() + offsetX, capsuleSize.Height() - radius + offsetY });
        // startAngle:180  sweepAngle:-180
        path.AddArc(
            { offsetX, offsetY + (capsuleSize.Height() - radius) * FLOAT_TWO_ZERO - progressWidth,
                        capsuleSize.Width() + offsetX, progressWidth + offsetY },
            ANGLE_180, -ANGLE_180);
    } else {
        path.AddRect(
            { offsetX, radius + offsetY, offsetX + capsuleSize.Width(), progressWidth + offsetY });
    }
    canvas.DrawPath(path);
    canvas.Restore();

    PaintCapsuleLightSweep(canvas, capsuleSize, offset, path, true);
}

void ProgressModifier::PaintCapsuleLightSweep(
    RSCanvas& canvas, const SizeF& contentSize, const OffsetF& offset, const RSPath& path, bool isVertical) const
{
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(1);
    auto gradient = CreateCapsuleGradient();
    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    RSBrush brush;
    auto gradientColors = gradient.GetColors();
    for (size_t i = 0; i < gradientColors.size(); i++) {
        colors.emplace_back(gradientColors[i].GetColor().GetValue());
        pos.emplace_back(gradientColors[i].GetDimension().Value());
    }

    float endPos = sweepingDate_->Get();
    if (isVertical) {
        brush.SetShaderEffect(RSShaderEffect::CreateLinearGradient(
            ToRSPoint(PointF(offset.GetX() + contentSize.Width() / 2,
            offset.GetY() + endPos - SWEEP_WIDTH.ConvertToPx() + capsuleBorderWidth_->Get())),
            ToRSPoint(PointF(offset.GetX() + contentSize.Width() / 2,
            offset.GetY() + endPos - capsuleBorderWidth_->Get())),
            colors, pos, RSTileMode::CLAMP));
    } else {
        brush.SetShaderEffect(RSShaderEffect::CreateLinearGradient(
            ToRSPoint(PointF(offset.GetX() + endPos - SWEEP_WIDTH.ConvertToPx() + capsuleBorderWidth_->Get(),
            offset.GetY() + contentSize.Height() / 2)),
            ToRSPoint(PointF(offset.GetX() + endPos - capsuleBorderWidth_->Get(),
            offset.GetY() + contentSize.Height() / 2)),
            colors, pos, RSTileMode::CLAMP));
    }
    
    auto offsetX = offset.GetX();
    auto offsetY = offset.GetY();
    canvas.Save();
    canvas.ClipPath(path, RSClipOp::INTERSECT, true);
    canvas.AttachBrush(brush);
    if (isVertical) {
        canvas.DrawRect(
            { offsetX + capsuleBorderWidth_->Get(),
            offsetY + endPos - SWEEP_WIDTH.ConvertToPx() + capsuleBorderWidth_->Get(),
            offsetX + contentSize.Width() + capsuleBorderWidth_->Get(),
            offsetY + endPos - capsuleBorderWidth_->Get()});
    } else {
        canvas.DrawRect(
            { offsetX + endPos - SWEEP_WIDTH.ConvertToPx() + capsuleBorderWidth_->Get(),
            offsetY + capsuleBorderWidth_->Get(), offsetX + endPos + capsuleBorderWidth_->Get(),
            offsetY + contentSize.Height() + capsuleBorderWidth_->Get()});
    }
    canvas.DetachPen();
    canvas.Restore();
}

Gradient ProgressModifier::CreateCapsuleGradient() const
{
    Gradient gradient;
    Color lightSweepColorBase = Color::WHITE;
    Color lightSweepColorEnd = lightSweepColorBase.ChangeOpacity(0.0);
    GradientColor gradientColorEnd;
    gradientColorEnd.SetColor(lightSweepColorEnd);
    gradientColorEnd.SetDimension(Dimension(0.0, DimensionUnit::VP));
    gradient.AddColor(gradientColorEnd);

    Color lightSweepColorMiddle = lightSweepColorBase.ChangeOpacity(0.2);
    GradientColor gradientColorMiddle;
    gradientColorMiddle.SetColor(lightSweepColorMiddle);
    gradientColorMiddle.SetDimension(SWEEP_WIDTH);
    gradient.AddColor(gradientColorMiddle);
    return gradient;
}
} // namespace OHOS::Ace::NG

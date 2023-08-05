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

#include "core/components_ng/pattern/scroll/inner/scroll_bar_overlay_modifier.h"

#include "base/geometry/ng/offset_t.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double FULL_ALPHA = 255.0;
constexpr float HALF = 0.5f;
constexpr int32_t BAR_END_DURATION = 400;        // 400ms
constexpr int32_t BAR_APPEAR_DURATION = 100;     // 100ms
constexpr int32_t BAR_GROW_DURATION = 150;       // 150ms, scroll bar width expands from 4dp to 8dp
constexpr int32_t BAR_SHRINK_DURATION = 250;     // 250ms, scroll bar width shrinks from 8dp to 4dp
} // namespace

ScrollBarOverlayModifier::ScrollBarOverlayModifier()
{
    opacity_ = AceType::MakeRefPtr<AnimatablePropertyUint8>(UINT8_MAX);
    AttachProperty(opacity_);
    fgOffset_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    AttachProperty(fgOffset_);
    fgSize_ = AceType::MakeRefPtr<AnimatablePropertySizeF>(SizeF());
    AttachProperty(fgSize_);
    bgOffset_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    AttachProperty(bgOffset_);
    bgSize_ = AceType::MakeRefPtr<AnimatablePropertySizeF>(SizeF());
    AttachProperty(bgSize_);

    fgColor_ = AceType::MakeRefPtr<PropertyColor>(Color());
    AttachProperty(fgColor_);
    bgColor_ = AceType::MakeRefPtr<PropertyColor>(Color());
    AttachProperty(bgColor_);
}

void ScrollBarOverlayModifier::onDraw(DrawingContext& drawingContext)
{
    CHECK_NULL_VOID_NOLOG(opacity_);
    CHECK_NULL_VOID_NOLOG(fgOffset_);
    CHECK_NULL_VOID_NOLOG(fgSize_);
    CHECK_NULL_VOID_NOLOG(bgOffset_);
    CHECK_NULL_VOID_NOLOG(bgSize_);
    CHECK_NULL_VOID_NOLOG(fgColor_);
    CHECK_NULL_VOID_NOLOG(bgColor_);

    auto fgSize = fgSize_->Get();
    auto bgSize = bgSize_->Get();
    auto fgOffset = fgOffset_->Get();
    auto bgOffset = bgOffset_->Get();
    if (!NearZero(fgSize.Height()) && !NearZero(bgSize.Height())) {
        auto& canvas = drawingContext.canvas;
        RSBrush brush;
        brush.SetBlendMode(RSBlendMode::SRC_OVER);
        brush.SetAntiAlias(true);
        RSRect bgRect(
            bgOffset.GetX(), bgOffset.GetY(), bgOffset.GetX() + bgSize.Width(), bgOffset.GetY() + bgSize.Height());
        RSColor bgColor = ToRSColor(bgColor_->Get());
        brush.SetColor(bgColor);
        double filletRadius = bgRect.GetWidth() * HALF;
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect({ bgRect, filletRadius, filletRadius });
        canvas.DetachBrush();

        RSRect fgRect(
            fgOffset.GetX(), fgOffset.GetY(), fgOffset.GetX() + fgSize.Width(), fgOffset.GetY() + fgSize.Height());
        RSColor fgColor = ToRSColor(fgColor_->Get().BlendOpacity(opacity_->Get() / FULL_ALPHA));
        brush.SetColor(fgColor);
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect({ fgRect, filletRadius, filletRadius });
    }
}

void ScrollBarOverlayModifier::StartOpacityAnimation(OpacityAnimationType opacityAnimationType)
{
    CHECK_NULL_VOID_NOLOG(opacity_);
    if (opacityAnimationType == OpacityAnimationType::NONE) {
        return;
    }
    if (opacityAnimationType != opacityAnimatingType_) {
        StopBarOpacityAnimation();
    } else {
        return;
    }
    AnimationOption option;
    option.SetCurve(Curves::SHARP);
    if (opacityAnimationType == OpacityAnimationType::DISAPPEAR) {
        option.SetDuration(BAR_END_DURATION);
    } else if (opacityAnimationType == OpacityAnimationType::APPEAR) {
        option.SetDuration(BAR_APPEAR_DURATION);
    }
    opacityAnimatingType_ = opacityAnimationType;
    opacityAnimation_ = AnimationUtils::StartAnimation(
        option,
        [&]() {
            if (opacityAnimatingType_ == OpacityAnimationType::DISAPPEAR) {
                opacity_->Set(0);
            } else if (opacityAnimatingType_ == OpacityAnimationType::APPEAR) {
                opacity_->Set(UINT8_MAX);
            }
        },
        [weak = WeakClaim(this)]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID_NOLOG(modifier);
            modifier->SetOpacityAnimatingType(OpacityAnimationType::NONE);
        });
}

void ScrollBarOverlayModifier::StopBarOpacityAnimation()
{
    if (opacityAnimation_) {
        AnimationUtils::StopAnimation(opacityAnimation_);
    }
}

void ScrollBarOverlayModifier::SetOffset(OffsetF fgOffset, OffsetF bgOffset)
{
    CHECK_NULL_VOID_NOLOG(fgOffset_);
    CHECK_NULL_VOID_NOLOG(bgOffset_);
    fgOffset_->Set(fgOffset);
    bgOffset_->Set(bgOffset);
}

void ScrollBarOverlayModifier::SetRect(const SizeF& fgSize, const SizeF& bgSize, const OffsetF& fgOffset,
    const OffsetF& bgOffset, HoverAnimationType hoverAnimationType)
{
    CHECK_NULL_VOID_NOLOG(fgSize_);
    CHECK_NULL_VOID_NOLOG(bgSize_);
    CHECK_NULL_VOID_NOLOG(fgOffset_);
    CHECK_NULL_VOID_NOLOG(bgOffset_);
    if (hoverAnimationType == HoverAnimationType::NONE) {
        if (hoverAnimatingType_ == HoverAnimationType::NONE) {
            fgSize_->Set(fgSize);
            bgSize_->Set(bgSize);
            fgOffset_->Set(fgOffset);
            bgOffset_->Set(bgOffset);
        }
        return;
    }
    if (hoverAnimationType != hoverAnimatingType_) {
        StopBarHoverAnimation();
    }
    AnimationOption option;
    option.SetCurve(Curves::SHARP);
    hoverAnimatingType_ = hoverAnimationType;
    if (hoverAnimatingType_ == HoverAnimationType::GROW) {
        option.SetDuration(BAR_GROW_DURATION);
    } else if (hoverAnimatingType_ == HoverAnimationType::SHRINK) {
        option.SetDuration(BAR_SHRINK_DURATION);
    }

    hoverAnimation_ = AnimationUtils::StartAnimation(
        option,
        [&]() {
            fgSize_->Set(fgSize);
            bgSize_->Set(bgSize);
            fgOffset_->Set(fgOffset);
            bgOffset_->Set(bgOffset);
        },
        [weak = WeakClaim(this)]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID_NOLOG(modifier);
            modifier->SetHoverAnimatingType(HoverAnimationType::NONE);
        });
}

void ScrollBarOverlayModifier::StopBarHoverAnimation()
{
    if (hoverAnimation_) {
        AnimationUtils::StopAnimation(hoverAnimation_);
    }
}

void ScrollBarOverlayModifier::SetFgColor(Color fgColor)
{
    CHECK_NULL_VOID_NOLOG(fgColor_);
    fgColor_->Set(fgColor);
}

void ScrollBarOverlayModifier::SetBgColor(Color bgColor)
{
    CHECK_NULL_VOID_NOLOG(bgColor_);
    bgColor_->Set(bgColor);
}
} // namespace OHOS::Ace::NG
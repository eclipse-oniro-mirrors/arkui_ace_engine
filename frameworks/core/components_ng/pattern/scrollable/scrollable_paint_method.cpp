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

#include "core/components_ng/pattern/scrollable/scrollable_paint_method.h"

namespace OHOS::Ace::NG {

constexpr double PERCENT_100 = 100.0;
constexpr float LINEAR_GRADIENT_ANGLE = 90.0f;
namespace {
GradientColor CreatePercentGradientColor(float percent, Color color)
{
    NG::GradientColor gredient = GradientColor(color);
    gredient.SetDimension(CalcDimension(percent * PERCENT_100, DimensionUnit::PERCENT));
    return gredient;
}
} // namespace

void ScrollablePaintMethod::UpdateFadingGradient(const RefPtr<RenderContext>& renderContext)
{
    CHECK_NULL_VOID(renderContext);
    CHECK_NULL_VOID(overlayRenderContext_);
    NG::Gradient gradient;
    gradient.CreateGradientWithType(NG::GradientType::LINEAR);
    if (isFadingTop_) {
        gradient.AddColor(CreatePercentGradientColor(0, Color::TRANSPARENT));
        gradient.AddColor(CreatePercentGradientColor(percentFading_, Color::WHITE));
    }
    if (isFadingBottom_) {
        gradient.AddColor(CreatePercentGradientColor(1 - percentFading_, Color::WHITE));
        gradient.AddColor(CreatePercentGradientColor(1, Color::TRANSPARENT));
    }
    if (!isFadingTop_ && !isFadingBottom_) {
        gradient.AddColor(CreatePercentGradientColor(0, Color::WHITE));
        gradient.AddColor(CreatePercentGradientColor(1, Color::WHITE));
    }
    if (vertical_) {
        gradient.GetLinearGradient()->angle = CalcDimension(LINEAR_GRADIENT_ANGLE, DimensionUnit::PX);
    }
    renderContext->UpdateBackBlendMode(BlendMode::SRC_OVER);
    renderContext->UpdateBackBlendApplyType(BlendApplyType::OFFSCREEN);

    overlayRenderContext_->UpdateZIndex(INT32_MAX);
    overlayRenderContext_->UpdateLinearGradient(gradient);
    overlayRenderContext_->UpdateBackBlendMode(BlendMode::DST_IN);
    overlayRenderContext_->UpdateBackBlendApplyType(BlendApplyType::OFFSCREEN);
}
} // namespace OHOS::Ace::NG

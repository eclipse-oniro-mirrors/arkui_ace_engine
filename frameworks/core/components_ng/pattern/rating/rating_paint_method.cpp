/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/rating/rating_paint_method.h"

#include "base/geometry/ng/offset_t.h"
#include "core/components/rating/rating_theme.h"
#include "core/components_ng/pattern/rating/rating_render_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
constexpr double DEFAULT_RATING_TOUCH_STAR_NUMBER = -1;

CanvasDrawFunction RatingPaintMethod::GetContentDrawFunction(PaintWrapper* paintWrapper)
{
    CHECK_NULL_RETURN(foregroundImageCanvas_, nullptr);
    CHECK_NULL_RETURN(secondaryImageCanvas_, nullptr);
    CHECK_NULL_RETURN(backgroundImageCanvas_, nullptr);
    foregroundImageCanvas_->SetPaintConfig(singleStarImagePaintConfig_);
    secondaryImageCanvas_->SetPaintConfig(singleStarImagePaintConfig_);
    backgroundImageCanvas_->SetPaintConfig(singleStarImagePaintConfig_);
    auto offset = paintWrapper->GetContentOffset();

    const ImagePainter foregroundImagePainter(foregroundImageCanvas_);
    const ImagePainter secondaryImagePainter(secondaryImageCanvas_);
    const ImagePainter backgroundPainter(backgroundImageCanvas_);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto ratingTheme = pipeline->GetTheme<RatingTheme>();
    CHECK_NULL_RETURN(ratingTheme, nullptr);
    Color bgColor = isHover_ ? ratingTheme->GetHoverColor() : ratingTheme->GetPressColor();
    auto ratingRenderProperty = DynamicCast<RatingRenderProperty>(paintWrapper->GetPaintProperty());
    const double drawScore = ratingRenderProperty->GetRatingScoreValue();
    const double stepSize = ratingRenderProperty->GetStepSize().value_or(ratingTheme->GetStepSize());
    const int32_t touchStar = ratingRenderProperty->GetTouchStar().value_or(DEFAULT_RATING_TOUCH_STAR_NUMBER);
    return [foregroundImagePainter, secondaryImagePainter, backgroundPainter, drawScore, stepSize, touchStar,
               starNum = starNum_, offset, config = singleStarImagePaintConfig_, bgColor,
               pressBorderRadius = ratingTheme->GetFocusBorderRadius()](RSCanvas& canvas) {
        // step1: check if touch down any stars.
        const float singleStarWidth = config.dstRect_.Width();
        const float singleStarHeight = config.dstRect_.Height();
        if (touchStar >= 0 && touchStar <= starNum) {
            RSBrush rsBrush(ToRSColor(bgColor));
            rsBrush.SetAntiAlias(true);
            const RSRect rsRect(offset.GetX() + singleStarWidth * static_cast<float>(touchStar), offset.GetY(),
                offset.GetX() + singleStarWidth * static_cast<float>((touchStar + 1)),
                offset.GetY() + singleStarHeight);
            const RSRoundRect rsRoundRect(rsRect, static_cast<float>(pressBorderRadius.ConvertToPx()),
                static_cast<float>(pressBorderRadius.ConvertToPx()));
            canvas.Save();
            canvas.ClipRoundRect(rsRoundRect, RSClipOp::INTERSECT);
            canvas.DrawBackground(rsBrush);
            canvas.Restore();
        }

        // step2: calculate 3 images repeat times.
        const int32_t foregroundImageRepeatNum = ceil(drawScore);
        const double secondaryImageRepeatNum = foregroundImageRepeatNum - drawScore;
        const int32_t backgroundImageRepeatNum = starNum - foregroundImageRepeatNum;
        // step3: draw the foreground images.
        canvas.Save();
        auto offsetTemp = offset;
        auto contentSize = SizeF(singleStarWidth, singleStarHeight);
        // step2.1: calculate the clip area in order to display the secondary image.
        auto clipRect1 = RSRect(offset.GetX(), offsetTemp.GetY(),
            static_cast<float>(offset.GetX() + singleStarWidth * drawScore), offset.GetY() + singleStarHeight);
        canvas.ClipRect(clipRect1, RSClipOp::INTERSECT);
        for (int32_t i = 0; i < foregroundImageRepeatNum; i++) {
            foregroundImagePainter.DrawImage(canvas, offsetTemp, contentSize);
            offsetTemp.SetX(static_cast<float>(offsetTemp.GetX() + singleStarWidth));
        }
        canvas.Restore();

        // step3: if drawScore is a decimal, it needs to draw the secondary image.
        if (secondaryImageRepeatNum != 0) {
            canvas.Save();
            auto clipRect2 = RSRect(static_cast<float>(offset.GetX() + singleStarWidth * drawScore), offsetTemp.GetY(),
                static_cast<float>(offset.GetX() + singleStarWidth * static_cast<float>(foregroundImageRepeatNum)),
                offset.GetY() + singleStarHeight);
            // step3.1: calculate the clip area which already occupied by the foreground image.
            canvas.ClipRect(clipRect2, RSClipOp::INTERSECT);
            offsetTemp.SetX(static_cast<float>(offsetTemp.GetX() - singleStarWidth));
            secondaryImagePainter.DrawImage(canvas, offsetTemp, contentSize);
            offsetTemp.SetX(offsetTemp.GetX() + config.dstRect_.Width());
            canvas.Restore();
        }

        // step4: draw background image.
        for (int32_t i = 0; i < backgroundImageRepeatNum; i++) {
            backgroundPainter.DrawImage(canvas, offsetTemp, contentSize);
            if (i < backgroundImageRepeatNum - 1) {
                offsetTemp.SetX(offsetTemp.GetX() + config.dstRect_.Width());
            }
        }
    };
}

} // namespace OHOS::Ace::NG
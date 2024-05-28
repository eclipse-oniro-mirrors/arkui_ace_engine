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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_CONTENT_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_CONTENT_MODIFIER_H

#include <optional>

#include "base/memory/ace_type.h"
#include "core/components/common/properties/marquee_option.h"
#include "core/components/common/properties/text_style.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/rich_editor/paragraph_manager.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/paragraph.h"

namespace OHOS::Ace::NG {
class TextContentModifier : public ContentModifier {
    DECLARE_ACE_TYPE(TextContentModifier, ContentModifier)

public:
    explicit TextContentModifier(const std::optional<TextStyle>& textStyle, const WeakPtr<Pattern>& pattern = nullptr);
    ~TextContentModifier() override = default;

    void onDraw(DrawingContext& drawingContext) override;

    void SetFontFamilies(const std::vector<std::string>& value);
    void SetFontSize(const Dimension& value);
    void SetAdaptMinFontSize(const Dimension& value);
    void SetAdaptMaxFontSize(const Dimension& value);
    void SetFontWeight(const FontWeight& value);
    void SetTextColor(const Color& value);
    void SetTextShadow(const std::vector<Shadow>& value);
    void SetTextDecoration(const TextDecoration& value);
    void SetTextDecorationStyle(const TextDecorationStyle value);
    void SetTextDecorationColor(const Color& value);
    void SetBaselineOffset(const Dimension& value);
    void SetContentOffset(OffsetF& value);
    void SetContentSize(SizeF& value);

    void ContentChange();

    void ModifyTextStyle(TextStyle& textStyle);

    void StartTextRace(const MarqueeOption& option);
    void StopTextRace();
    void SetIsFocused(const bool& isFocused);
    void SetIsHovered(const bool& isHovered);

    void SetPrintOffset(const OffsetF& paintOffset)
    {
        paintOffset_ = paintOffset;
    }

    void SetObscured(const std::vector<ObscuredReasons>& reasons)
    {
        obscuredReasons_ = reasons;
    }

    void SetIfHaveSpanItemChildren(bool value)
    {
        ifHaveSpanItemChildren_ = value;
    }

    void SetDrawObscuredRects(const std::vector<RectF>& drawObscuredRects)
    {
        drawObscuredRects_ = drawObscuredRects;
    }

    bool NeedMeasureUpdate(PropertyChangeFlag& flag);

    void SetClip(bool clip);

    void SetFontReady(bool value);
    void ChangeDragStatus();

    void SetImageSpanNodeList(std::vector<WeakPtr<FrameNode>> imageNodeList)
    {
        imageNodeList_ = imageNodeList;
    }

protected:
    OffsetF GetPaintOffset() const
    {
        return paintOffset_;
    }

private:
    double NormalizeToPx(const Dimension& dimension);
    void SetDefaultAnimatablePropertyValue(const TextStyle& textStyle);
    void SetDefaultFontSize(const TextStyle& textStyle);
    void SetDefaultAdaptMinFontSize(const TextStyle& textStyle);
    void SetDefaultAdaptMaxFontSize(const TextStyle& textStyle);
    void SetDefaultFontWeight(const TextStyle& textStyle);
    void SetDefaultTextColor(const TextStyle& textStyle);
    void SetDefaultTextShadow(const TextStyle& textStyle);
    void AddShadow(const Shadow& shadow);
    void AddDefaultShadow();
    void SetDefaultTextDecoration(const TextStyle& textStyle);
    void SetDefaultBaselineOffset(const TextStyle& textStyle);
    bool SetTextRace(const MarqueeOption& option);
    void ResumeTextRace(bool bounce);
    void SetTextRaceAnimation(const AnimationOption& option);
    void PauseTextRace();
    bool AllowTextRace();
    void DetermineTextRace();
    float GetTextRacePercent();

    void ModifyFontSizeInTextStyle(TextStyle& textStyle);
    void ModifyAdaptMinFontSizeInTextStyle(TextStyle& textStyle);
    void ModifyAdaptMaxFontSizeInTextStyle(TextStyle& textStyle);
    void ModifyFontWeightInTextStyle(TextStyle& textStyle);
    void ModifyTextColorInTextStyle(TextStyle& textStyle);
    void ModifyTextShadowsInTextStyle(TextStyle& textStyle);
    void ModifyDecorationInTextStyle(TextStyle& textStyle);
    void ModifyBaselineOffsetInTextStyle(TextStyle& textStyle);

    void UpdateFontSizeMeasureFlag(PropertyChangeFlag& flag);
    void UpdateAdaptMinFontSizeMeasureFlag(PropertyChangeFlag& flag);
    void UpdateAdaptMaxFontSizeMeasureFlag(PropertyChangeFlag& flag);
    void UpdateFontWeightMeasureFlag(PropertyChangeFlag& flag);
    void UpdateTextColorMeasureFlag(PropertyChangeFlag& flag);
    void UpdateTextShadowMeasureFlag(PropertyChangeFlag& flag);
    void UpdateTextDecorationMeasureFlag(PropertyChangeFlag& flag);
    void UpdateBaselineOffsetMeasureFlag(PropertyChangeFlag& flag);

    void DrawObscuration(DrawingContext& drawingContext);
    void UpdateFadeout(const DrawingContext& drawingContext);

    void ResetImageNodeList();
    void DrawImageNodeList(const float drawingContextWidth, const float paragraph1Offset, const float paragraph2Offset);
    void UpdateImageNodeVisible(const VisibleType visible);
    void PaintImage(RSCanvas& canvas, float x, float y);
    void PaintCustomSpan(DrawingContext& drawingContext);

    std::optional<Dimension> fontSize_;
    RefPtr<AnimatablePropertyFloat> fontSizeFloat_;

    std::optional<Dimension> adaptMinFontSize_;
    RefPtr<AnimatablePropertyFloat> adaptMinFontSizeFloat_;

    std::optional<Dimension> adaptMaxFontSize_;
    RefPtr<AnimatablePropertyFloat> adaptMaxFontSizeFloat_;

    std::optional<FontWeight> fontWeight_;
    RefPtr<AnimatablePropertyFloat> fontWeightFloat_;

    std::optional<Color> textColor_;
    RefPtr<AnimatablePropertyColor> animatableTextColor_;

    struct ShadowProp {
        Shadow shadow; // final shadow configuration of the animation
        RefPtr<AnimatablePropertyFloat> blurRadius;
        RefPtr<AnimatablePropertyFloat> offsetX;
        RefPtr<AnimatablePropertyFloat> offsetY;
        RefPtr<AnimatablePropertyColor> color;
    };
    std::vector<ShadowProp> shadows_;

    float oldColorAlpha_ { 0.0f };
    std::optional<TextDecoration> textDecoration_;
    std::optional<TextDecorationStyle> textDecorationStyle_;
    std::optional<Color> textDecorationColor_;
    RefPtr<AnimatablePropertyFloat> textDecorationColorAlpha_;
    bool textDecorationAnimatable_ { false };

    std::optional<Dimension> baselineOffset_;
    RefPtr<AnimatablePropertyFloat> baselineOffsetFloat_;

    bool textRacing_ = false;
    bool marqueeSet_ = false;
    MarqueeOption marqueeOption_;
    int32_t marqueeCount_ = 0;
    int32_t marqueeAnimationId_ = 0;
    bool marqueeFocused_ = false;
    bool marqueeHovered_ = false;
    int32_t marqueeDuration_ = 0;
    float marqueeGradientPercent_ = 0.0;
    WeakPtr<Pattern> pattern_;

    RefPtr<AnimatablePropertyFloat> racePercentFloat_;
    std::shared_ptr<AnimationUtils::Animation> raceAnimation_;

    RefPtr<PropertyOffsetF> contentOffset_;
    RefPtr<PropertySizeF> contentSize_;
    RefPtr<PropertyInt> contentChange_;
    RefPtr<PropertyBool> clip_;
    RefPtr<PropertyString> fontFamilyString_;
    RefPtr<PropertyBool> fontReady_;
    RefPtr<PropertyBool> dragStatus_;
    OffsetF paintOffset_;
    float textRaceSpaceWidth_ = 0;

    std::vector<ObscuredReasons> obscuredReasons_;
    bool ifHaveSpanItemChildren_ = false;
    std::vector<RectF> drawObscuredRects_;
    std::vector<WeakPtr<FrameNode>> imageNodeList_;
    ACE_DISALLOW_COPY_AND_MOVE(TextContentModifier);
};
} // namespace OHOS::Ace::NG

#endif
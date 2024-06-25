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

#include "core/components_ng/pattern/text/text_paint_method.h"

#include "base/utils/utils.h"
#include "core/components/common/properties/marquee_option.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {

namespace {
constexpr Dimension DEFAULT_MARQUEE_STEP_VP = 4.0_vp;
} // namespace

TextPaintMethod::TextPaintMethod(const WeakPtr<Pattern>& pattern, float baselineOffset,
    RefPtr<TextContentModifier> textContentModifier, RefPtr<TextOverlayModifier> textOverlayModifier)
    : pattern_(pattern), baselineOffset_(baselineOffset), textContentModifier_(std::move(textContentModifier)),
      textOverlayModifier_(std::move(textOverlayModifier))
{}

RefPtr<Modifier> TextPaintMethod::GetContentModifier(PaintWrapper* paintWrapper)
{
    return textContentModifier_;
}

void TextPaintMethod::UpdateParagraphAndImageSpanNodeList()
{
    CHECK_NULL_VOID(textContentModifier_);
    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    textContentModifier_->SetImageSpanNodeList(textPattern->GetImageSpanNodeList());
}

void TextPaintMethod::DoStartTextRace()
{
    CHECK_NULL_VOID(textContentModifier_);

    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    auto frameNode = textPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);
    auto layoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    MarqueeOption option;
    option.start = layoutProperty->GetTextMarqueeStart().value_or(true);
    option.step = layoutProperty->GetTextMarqueeStep().value_or(DEFAULT_MARQUEE_STEP_VP.ConvertToPx());
    if (GreatNotEqual(option.step, pManager->GetTextWidth())) {
        option.step = DEFAULT_MARQUEE_STEP_VP.ConvertToPx();
    }
    option.loop = layoutProperty->GetTextMarqueeLoop().value_or(-1);
    option.direction = layoutProperty->GetTextMarqueeDirection().value_or(MarqueeDirection::LEFT);
    option.delay = layoutProperty->GetTextMarqueeDelay().value_or(0);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextTheme>();
    CHECK_NULL_VOID(theme);
    option.fadeout = layoutProperty->GetTextMarqueeFadeout().value_or(theme->GetIsTextFadeout());
    option.startPolicy = layoutProperty->GetTextMarqueeStartPolicy().value_or(MarqueeStartPolicy::DEFAULT);

    textContentModifier_->StartTextRace(option);
}

void TextPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    CHECK_NULL_VOID(textContentModifier_);

    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);

    UpdateParagraphAndImageSpanNodeList();

    SizeF contentSize = paintWrapper->GetContentSize();
    textContentModifier_->SetContentSize(contentSize);
    auto offset = OffsetF(0.0, 0.0);
    textContentModifier_->SetContentOffset(offset);
    auto paintOffset = OffsetF(0.0, -std::min(baselineOffset_, 0.0f));
    textContentModifier_->SetPrintOffset(paintOffset);

    auto frameNode = textPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(pattern);

    auto textOverflow = layoutProperty->GetTextOverflow();
    if (textOverflow.has_value() && textOverflow.value() == TextOverflow::MARQUEE &&
        pManager->GetLongestLine() > contentSize.Width()) {
        DoStartTextRace();
    } else {
        textContentModifier_->StopTextRace();
    }

    auto reasons = renderContext->GetObscured().value_or(std::vector<ObscuredReasons>());
    textContentModifier_->SetObscured(reasons);
    auto spanItemChildren = pattern->GetSpanItemChildren();
    textContentModifier_->SetIfHaveSpanItemChildren(!spanItemChildren.empty());
    auto wideTextLength = pattern->GetDisplayWideTextLength();
    std::vector<RectF> drawObscuredRects;
    if (wideTextLength != 0) {
        drawObscuredRects = pManager->GetRects(0, wideTextLength);
    }
    textContentModifier_->SetDrawObscuredRects(drawObscuredRects);
    if (renderContext->GetClipEdge().has_value()) {
        textContentModifier_->SetClip(renderContext->GetClipEdge().value());
    }

    PropertyChangeFlag flag = 0;
    if (textContentModifier_->NeedMeasureUpdate(flag)) {
        frameNode->MarkDirtyNode(flag);
    }
}

RefPtr<Modifier> TextPaintMethod::GetOverlayModifier(PaintWrapper* paintWrapper)
{
    return textOverlayModifier_;
}

void TextPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    CHECK_NULL_VOID(textOverlayModifier_);

    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);

    auto offset = paintWrapper->GetContentOffset();
    auto paintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    textOverlayModifier_->SetPrintOffset(paintOffset);
    auto host = textPattern->GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetRenderContext();
    CHECK_NULL_VOID(context);
    const auto& selection = textPattern->GetTextSelector();
    auto contentRect = textPattern->GetTextContentRect();
    std::vector<RectF> selectedRects;
    if (selection.GetTextStart() != selection.GetTextEnd()) {
        selectedRects = pManager->GetRects(selection.GetTextStart(), selection.GetTextEnd());
        TextBase::CalculateSelectedRect(selectedRects, contentRect.Width());
    }
    textOverlayModifier_->SetContentRect(contentRect);
    textOverlayModifier_->SetShowSelect(textPattern->GetShowSelect());
    textOverlayModifier_->SetSelectedRects(selectedRects);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto theme = themeManager->GetTheme<TextTheme>();
    CHECK_NULL_VOID(theme);
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto selectedColor = layoutProperty->GetSelectedBackgroundColorValue(theme->GetSelectedColor());
    textOverlayModifier_->SetSelectedColor(selectedColor.GetValue());
    if (context->GetClipEdge().has_value()) {
        textOverlayModifier_->SetIsClip(context->GetClipEdge().value());
    }
}
} // namespace OHOS::Ace::NG
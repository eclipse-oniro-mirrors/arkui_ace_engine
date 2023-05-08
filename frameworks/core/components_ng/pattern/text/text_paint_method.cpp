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

#include "core/components_ng/pattern/text/text_pattern.h"
#include "base/log/ace_trace.h"

namespace OHOS::Ace::NG {
TextPaintMethod::TextPaintMethod(const WeakPtr<Pattern>& pattern, RefPtr<Paragraph> paragraph, float baselineOffset,
    RefPtr<TextContentModifier> textContentModifier, RefPtr<TextOverlayModifier> textOverlayModifier)
    : pattern_(pattern), paragraph_(std::move(paragraph)), baselineOffset_(baselineOffset),
      textContentModifier_(textContentModifier), textOverlayModifier_(textOverlayModifier)
{}

RefPtr<Modifier> TextPaintMethod::GetContentModifier(PaintWrapper* paintWrapper)
{
    return textContentModifier_;
}

void TextPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    CHECK_NULL_VOID(paragraph_);
    CHECK_NULL_VOID(textContentModifier_);

    textContentModifier_->SetParagraph(paragraph_);

    auto offset = paintWrapper->GetContentOffset();
    auto paintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    ACE_SCOPED_TRACE("[%s]- [%s]", offset.ToString().c_str(), paintWrapper->GetContentSize().ToString().c_str());
    textContentModifier_->SetPrintOffset(paintOffset);

    auto textPattern = pattern_.Upgrade();
    CHECK_NULL_VOID(textPattern);
    auto frameNode = textPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto textOverflow = layoutProperty->GetTextOverflow();
    if (textOverflow.has_value() && textOverflow.value() == TextOverflow::MARQUEE) {
        if (paragraph_->GetTextWidth() > paintWrapper->GetContentSize().Width()) {
            textContentModifier_->StartTextRace();
        } else {
            textContentModifier_->StopTextRace();
        }
    } else {
        textContentModifier_->StopTextRace();
    }

    textContentModifier_->ContentChange();

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
    CHECK_NULL_VOID(paragraph_);
    CHECK_NULL_VOID(textOverlayModifier_);

    auto offset = paintWrapper->GetContentOffset();
    auto paintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    textOverlayModifier_->SetPrintOffset(paintOffset);

    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    const auto& selection = textPattern->GetTextSelector();
    auto textValue = textPattern->GetTextForDisplay();
    std::vector<Rect> selectedRects;
    if (!textValue.empty() && selection.GetTextStart() != selection.GetTextEnd()) {
        paragraph_->GetRectsForRange(selection.GetTextStart(), selection.GetTextEnd(), selectedRects);
    }
    auto contentRect = textPattern->GetTextContentRect();
    textOverlayModifier_->SetContentRect(contentRect);
    textOverlayModifier_->SetSelectedRects(selectedRects);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto theme = themeManager->GetTheme<TextTheme>();
    auto selectedColor = theme->GetSelectedColor().GetValue();
    textOverlayModifier_->SetSelectedColor(selectedColor);
}
} // namespace OHOS::Ace::NG
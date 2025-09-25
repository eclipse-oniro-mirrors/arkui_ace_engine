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

#include "core/components_ng/pattern/rich_editor/rich_editor_content_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_content_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/render/drawing.h"
#include "base/log/ace_trace.h"

namespace OHOS::Ace::NG {
RichEditorContentModifier::RichEditorContentModifier(const std::optional<TextStyle>& textStyle,
    RichEditorParagraphManager* const pManager, const WeakPtr<OHOS::Ace::NG::Pattern>& pattern)
    : TextContentModifier(textStyle, pattern), pManager_(pManager), pattern_(pattern)
{
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
    richTextRectX_ = AceType::MakeRefPtr<PropertyFloat>(contentPattern->GetTextRect().GetX());
    AttachProperty(richTextRectX_);
    richTextRectY_ = AceType::MakeRefPtr<PropertyFloat>(contentPattern->GetTextRect().GetY());
    AttachProperty(richTextRectY_);
    clipOffset_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    AttachProperty(clipOffset_);
    clipSize_ = AceType::MakeRefPtr<AnimatablePropertySizeF>(SizeF());
    AttachProperty(clipSize_);
}

void RichEditorContentModifier::onDraw(DrawingContext& drawingContext)
{
    ACE_SCOPED_TRACE("RichEditorContentOnDraw");
    CHECK_NULL_VOID(pManager_);
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
    auto& canvas = drawingContext.canvas;
    canvas.Save();
    auto contentRect = contentPattern->GetTextContentRect();
    RSRect clipInnerRect = RSRect(contentRect.GetX(), contentRect.GetY(), contentRect.GetX() + contentRect.Width(),
        contentRect.GetY() + contentRect.Height());
    canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
    auto&& paragraphs = pManager_->GetParagraphs();
    pManager_->CalPosyRange();
    auto offset = contentPattern->GetTextRect().GetOffset(); // relative to component

    auto clipOffset = clipOffset_->Get();
    auto size = clipSize_->Get();

    auto clipTop = clipOffset.GetY();
    auto clipBottom = clipTop + size.Height();

    // Find the first paragraph whose bottom is within the content rect.
    auto findFirstPara = [](const ParagraphManager::ParagraphInfo& info, float pos) { return info.bottomPos < pos; };
    auto lb = std::lower_bound(paragraphs.begin(), paragraphs.end(), clipTop - offset.GetY(), findFirstPara);

    // Find the last paragraph whose top is within the content rect.
    auto findLastPara = [](float pos, const ParagraphManager::ParagraphInfo& info) { return pos < info.topPos; };
    auto ub = std::upper_bound(paragraphs.begin(), paragraphs.end(), clipBottom - offset.GetY(), findLastPara);
    if (ub != paragraphs.begin()) {
        --ub;
    }

    size_t lineCount = 0;
    for (auto iter = lb; iter <= ub && iter != paragraphs.end(); ++iter) {
        const auto& info = *iter;
        float x = AdjustParagraphX(info, contentRect);
        float y = info.topPos + offset.GetY();
        info.paragraph->Paint(drawingContext.canvas, x, y);
        PaintLeadingMarginSpan(info, offset, lineCount, drawingContext);
    }

    PaintCustomSpan(drawingContext);
}

float RichEditorContentModifier::AdjustParagraphX(const ParagraphManager::ParagraphInfo& info, const RectF& contentRect)
{
    auto x = contentRect.GetOffset().GetX();
    CHECK_NULL_RETURN(info.paragraph && info.paragraph->empty(), x);
    const auto& paraStyle = info.paragraphStyle;
    CHECK_NULL_RETURN(paraStyle.leadingMargin && paraStyle.leadingMargin->pixmap, x);
    CHECK_NULL_RETURN(paraStyle.direction == TextDirection::RTL, x);
    float leadingMarginWidth = static_cast<float>(paraStyle.leadingMargin->size.Width().ConvertToPx());
    return contentRect.GetX() + contentRect.Width() - leadingMarginWidth;
}

void RichEditorContentModifier::PaintLeadingMarginSpan(const ParagraphManager::ParagraphInfo& paragraphInfo,
    const OffsetT<float>& offset, size_t& lineCount, DrawingContext& drawingContext)
{
    auto drawableLeadingMargin = paragraphInfo.paragraphStyle.drawableLeadingMargin;
    auto currentParagraphLineCount = paragraphInfo.paragraph->GetLineCount();
    if (!drawableLeadingMargin.has_value() || currentParagraphLineCount <= 0) {
        lineCount += currentParagraphLineCount;
        return;
    }
    auto leadingMarginOnDraw = drawableLeadingMargin.value().onDraw_;
    CHECK_NULL_VOID(leadingMarginOnDraw);
    for (size_t i = 0; i < currentParagraphLineCount; i++) {
        auto lineMetrics = pManager_->GetLineMetrics(lineCount + i);
        LeadingMarginSpanOptions options;
        options.x = paragraphInfo.paragraphStyle.direction != TextDirection::RTL ?
            lineMetrics.x + offset.GetX() : lineMetrics.x + offset.GetX() + lineMetrics.width;
        options.direction = paragraphInfo.paragraphStyle.direction;
        options.top = lineMetrics.y + offset.GetY();
        options.baseline = lineMetrics.baseline + offset.GetY();
        options.bottom = options.top + lineMetrics.height;
        options.start = lineMetrics.startIndex;
        options.end = lineMetrics.endIndex;
        if (i == 0) {
            options.first = true;
        }
        leadingMarginOnDraw(drawingContext, options);
    }
    lineCount += currentParagraphLineCount;
}

void RichEditorContentModifier::PaintCustomSpan(DrawingContext& drawingContext)
{
    CHECK_NULL_VOID(pManager_);
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
    auto offset = contentPattern->GetTextRect().GetOffset();
    const auto& rectsForPlaceholders = contentPattern->GetRectsForPlaceholders();
    auto customSpanPlaceholderInfo = contentPattern->GetCustomSpanPlaceholderInfo();
    auto rectsForPlaceholderSize = rectsForPlaceholders.size();
    for (auto& customSpanPlaceholder : customSpanPlaceholderInfo) {
        if (!customSpanPlaceholder.onDraw || pManager_->GetParagraphs().empty()) {
            continue;
        }
        auto index = customSpanPlaceholder.customSpanIndex;
        if (index >= static_cast<int32_t>(rectsForPlaceholderSize) || index < 0) {
            return;
        }
        const auto& rect = rectsForPlaceholders.at(index);
        auto lineMetrics = pManager_->GetLineMetricsByRectF(rect, customSpanPlaceholder.paragraphIndex);
        CustomSpanOptions customSpanOptions;
        customSpanOptions.x = static_cast<double>(rect.Left()) + offset.GetX();
        customSpanOptions.lineTop = lineMetrics.y + offset.GetY();
        customSpanOptions.lineBottom = customSpanOptions.lineTop + lineMetrics.height;
        customSpanOptions.baseline = customSpanOptions.lineTop + lineMetrics.ascender;
        customSpanPlaceholder.onDraw(drawingContext, customSpanOptions);
    }
}
} // namespace OHOS::Ace::NG

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
#include "core/components/text_field/textfield_theme.h"

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
    SetDefaultAutoFillProperties();
}

void RichEditorContentModifier::onDraw(DrawingContext& drawingContext)
{
    ACE_SCOPED_TRACE("RichEditorContentOnDraw");
    CHECK_NULL_VOID(pManager_);
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
#ifdef ENABLE_AUTO_FILL_CONTROLLER
    auto richEditorPattern = contentPattern->GetParentPattern();
    if (richEditorPattern) {
        auto autoFillController = richEditorPattern->GetOrCreateAutoFillController();
        auto status = autoFillController ?
            autoFillController->GetAutoFillAnimationStatus(): AutoFillAnimationStatus::INIT;
        if (status != AutoFillAnimationStatus::INIT) {
            DoAutoFillDraw(drawingContext);
            return;
        }
    }
#endif
    auto& canvas = drawingContext.canvas;
    canvas.Save();
    auto contentRect = contentPattern->GetTextContentRect();
    RSRect clipInnerRect = RSRect(contentRect.GetX(), contentRect.GetY(), contentRect.GetX() + contentRect.Width(),
        contentRect.GetY() + contentRect.Height());
    canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
    auto&& paragraphs = pManager_->GetParagraphs();
    pManager_->CalPosyRange();
    pManager_->CalLineIndex();
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

    for (auto iter = lb; iter <= ub && iter != paragraphs.end(); ++iter) {
        const auto& info = *iter;
        CHECK_NULL_CONTINUE(info.paragraph);
        float x = AdjustParagraphX(info, contentRect);
        float y = info.topPos + offset.GetY();
        x += offset.GetX() - contentRect.GetX();
        info.paragraph->Paint(drawingContext.canvas, x, y);
        pManager_->PaintLeadingMarginSpan(info, offset, drawingContext);
    }

    PaintCustomSpan(drawingContext);
    contentPattern->ReportPageTranslatePlaceholderDrawn();
}

void RichEditorContentModifier::DoAutoFillDraw(DrawingContext& drawingContext)
{
#ifdef ENABLE_AUTO_FILL_CONTROLLER
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
    auto richEditorPattern = contentPattern->GetParentPattern();
    CHECK_NULL_VOID(richEditorPattern);
    auto autoFillController = richEditorPattern->GetOrCreateAutoFillController();
    CHECK_NULL_VOID(autoFillController);
    // RichEditorContentPattern inherits from Pattern (not TextPattern), so the base
    // TextContentModifier constructor's DynamicCast<TextPattern> fails and contentOffset_
    // is never initialized. Derive the offset from the content rect instead.
    // Use GetAutoFillTextContentRect() which returns the rect in the content-host's
    // local coordinate system (same as the canvas), not GetTextContentRect() which
    // returns it in the parent RichEditor's coordinate system.
    auto contentRect = richEditorPattern->GetAutoFillTextContentRect();
    OffsetF contentOffset(contentRect.GetX(), contentRect.GetY());
    autoFillController->DrawAutoFill(drawingContext.canvas, contentOffset,
        autoFillTranslationOffset_->Get(), autoFillTextScrollOffset_->Get(),
        autoFillDefaultCharIndex_->Get(), autoFillEmphasizeCharTextColor_,
        autoFillDefaultCharInitTextColor_, autoFillOriginTextColor_);
#endif
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

void RichEditorContentModifier::SetRichTextRectX(float value)
{
    richTextRectX_->Set(value);
}

void RichEditorContentModifier::SetRichTextRectY(float value)
{
    richTextRectY_->Set(value);
}

float RichEditorContentModifier::GetRichTextRectX() const
{
    return richTextRectX_ ? richTextRectX_->Get() : 0.0f;
}

float RichEditorContentModifier::GetRichTextRectY() const
{
    return richTextRectY_ ? richTextRectY_->Get() : 0.0f;
}

void RichEditorContentModifier::SetClipOffset(OffsetF offset)
{
    clipOffset_->Set(offset);
}

void RichEditorContentModifier::SetClipSize(SizeF size)
{
    clipSize_->Set(size);
}

void RichEditorContentModifier::SetAutoFillTranslationOffset(float value)
{
    CHECK_NULL_VOID(autoFillTranslationOffset_);
    autoFillTranslationOffset_->Set(value);
}
 
void RichEditorContentModifier::SetAutoFillTextScrollOffset(float value)
{
    CHECK_NULL_VOID(autoFillTextScrollOffset_);
    autoFillTextScrollOffset_->Set(value);
}
 
void RichEditorContentModifier::SetAutoFillEmphasizeCharIndex(float value)
{
    CHECK_NULL_VOID(autoFillEmphasizeCharIndex_);
    autoFillEmphasizeCharIndex_->Set(value);
}
 
void RichEditorContentModifier::SetAutoFillDefaultCharIndex(float value)
{
    CHECK_NULL_VOID(autoFillDefaultCharIndex_);
    autoFillDefaultCharIndex_->Set(value);
}
 
void RichEditorContentModifier::SetAutoFillOriginTextColor(const Color& value)
{
    autoFillOriginTextColor_ = value;
}
 
void RichEditorContentModifier::ResetAutoFillProperties()
{
    SetAutoFillTranslationOffset(0.0f);
    SetAutoFillTextScrollOffset(0.0f);
    SetAutoFillEmphasizeCharIndex(0.0f);
    SetAutoFillDefaultCharIndex(0.0f);
}
 
void RichEditorContentModifier::SetDefaultAutoFillProperties()
{
#ifdef ENABLE_AUTO_FILL_CONTROLLER
    auto contentPattern = AceType::DynamicCast<RichEditorContentPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(contentPattern);
    auto richEditorPattern = contentPattern->GetParentPattern();
    CHECK_NULL_VOID(richEditorPattern);
    auto theme = richEditorPattern->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    autoFillEmphasizeCharTextColor_ = theme->GetAutoFillIconEmphasizeColor();
    autoFillDefaultCharInitTextColor_ = theme->GetTextColor();
#endif
    autoFillTranslationOffset_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    AttachProperty(autoFillTranslationOffset_);
    autoFillTextScrollOffset_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    AttachProperty(autoFillTextScrollOffset_);
    autoFillEmphasizeCharIndex_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    AttachProperty(autoFillEmphasizeCharIndex_);
    autoFillDefaultCharIndex_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    AttachProperty(autoFillDefaultCharIndex_);
}

} // namespace OHOS::Ace::NG

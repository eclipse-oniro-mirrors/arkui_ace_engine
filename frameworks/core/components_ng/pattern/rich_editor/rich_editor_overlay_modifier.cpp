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

#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"

#include "base/utils/utils.h"
#include "core/components_ng/pattern/progress/progress_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t LAND_DURATION = 100;
const RefPtr<CubicCurve> LAND_CURVE = AceType::MakeRefPtr<CubicCurve>(0.2, 0, 0.2, 1.0f);
} // namespace

RichEditorOverlayModifier::RichEditorOverlayModifier(const WeakPtr<OHOS::Ace::NG::Pattern>& pattern,
    const WeakPtr<ScrollBarOverlayModifier>& scrollbarOverlayModifier, WeakPtr<ScrollEdgeEffect>&& edgeEffect)
    : TextOverlayModifier(), pattern_(pattern), edgeEffect_(edgeEffect),
      scrollBarOverlayModifier_(scrollbarOverlayModifier)
{
    caretVisible_ = AceType::MakeRefPtr<PropertyBool>(false);
    AttachProperty(caretVisible_);
    floatingCaretVisible_ = AceType::MakeRefPtr<PropertyBool>(false);
    AttachProperty(floatingCaretVisible_);
    originCaretVisible_ = AceType::MakeRefPtr<PropertyBool>(false);
    AttachProperty(originCaretVisible_);
    caretOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    AttachProperty(caretOffset_);
    floatingCaretOffset_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    AttachProperty(floatingCaretOffset_);
    caretHeight_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    AttachProperty(caretHeight_);
    caretWidth_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    AttachProperty(caretWidth_);
    caretColor_ = AceType::MakeRefPtr<PropertyInt>(0);
    AttachProperty(caretColor_);
    originCaretColor_ = AceType::MakeRefPtr<PropertyInt>(0);
    AttachProperty(originCaretColor_);
    scrollOffset_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    AttachProperty(scrollOffset_);
    frameSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    AttachProperty(frameSize_);
    scrollBarOpacityType_ = AceType::MakeRefPtr<PropertyInt>(-1);
    AttachProperty(scrollBarOpacityType_);
    textHeight_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    AttachProperty(textHeight_);
    previewTextDecorationColor_ = AceType::MakeRefPtr<PropertyColor>(Color());
    AttachProperty(previewTextDecorationColor_);
    previewTextUnderlineWidth_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    AttachProperty(previewTextUnderlineWidth_);
    showPreviewTextDecoration_ = AceType::MakeRefPtr<PropertyBool>(false);
    AttachProperty(showPreviewTextDecoration_);
}

void RichEditorOverlayModifier::SetPreviewTextDecorationColor(const Color& value)
{
    previewTextDecorationColor_->Set(value);
}

void RichEditorOverlayModifier::SetPreviewTextUnderlineWidth(float value)
{
    previewTextUnderlineWidth_->Set(value);
}

void RichEditorOverlayModifier::SetShowPreviewTextDecoration(bool value)
{
    showPreviewTextDecoration_->Set(value);
}

void RichEditorOverlayModifier::SetPreviewTextStyle(const PreviewTextStyle& value)
{
    previewTextStyle_ = value;
}

void RichEditorOverlayModifier::SetCaretOffsetAndHeight(const OffsetF& cursorOffset, float height)
{
    caretOffset_->Set(cursorOffset);
    if (height > 0.0f) {
        caretHeight_->Set(height);
    }
}

void RichEditorOverlayModifier::SetFloatingCaretOffset(const OffsetF& cursorOffset)
{
    floatingCaretOffset_->Set(cursorOffset);
}

void RichEditorOverlayModifier::SetCaretColor(uint32_t caretColor)
{
    CHECK_NULL_VOID(caretColor_);
    caretColor_->Set(static_cast<int32_t>(caretColor));
}

void RichEditorOverlayModifier::SetOriginCaretColor(uint32_t caretColor)
{
    originCaretColor_->Set(static_cast<int32_t>(caretColor));
}

void RichEditorOverlayModifier::SetCaretWidth(float width)
{
    if (width <= 0.0f) {
        return;
    }
    caretWidth_->Set(width);
}

float RichEditorOverlayModifier::GetCaretWidth() const
{
    return caretWidth_->Get();
}

void RichEditorOverlayModifier::SetCaretVisible(bool value)
{
    caretVisible_->Set(value);
}

void RichEditorOverlayModifier::SetOriginCaretVisible(bool value)
{
    originCaretVisible_->Set(value);
}

void RichEditorOverlayModifier::SetFloatingCaretVisible(bool value)
{
    floatingCaretVisible_->Set(value);
}

void RichEditorOverlayModifier::SetFloatCaretLanding(bool caretLanding)
{
    caretLanding_ = caretLanding;
}

void RichEditorOverlayModifier::SetScrollOffset(float value)
{
    scrollOffset_->Set(value);
}

void RichEditorOverlayModifier::SetScrollBarOpacityType(int32_t value)
{
    scrollBarOpacityType_->Set(value);
}

void RichEditorOverlayModifier::SetTextHeight(float value)
{
    textHeight_->Set(value);
}

void RichEditorOverlayModifier::SetFrameSize(const SizeF& value)
{
    frameSize_->Set(value);
}

float RichEditorOverlayModifier::GetCaretHeight() const
{
    return caretHeight_->Get();
}

OffsetF RichEditorOverlayModifier::GetCaretOffset() const
{
    return caretOffset_->Get();
}

bool RichEditorOverlayModifier::GetFloatCaretLanding() const
{
    return caretLanding_;
}

void RichEditorOverlayModifier::PaintPreviewTextDecoration(DrawingContext& drawingContext) const
{
    CHECK_NULL_VOID(showPreviewTextDecoration_->Get());
    if (previewTextStyle_ != PreviewTextStyle::UNDERLINE) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "is not UNDERLINE style");
        return;
    }
    auto pattern = AceType::DynamicCast<RichEditorPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);

    auto previewTextDecorationColor = caretColor_->Get();
    auto previewTextUnderlineWidth = previewTextUnderlineWidth_->Get();
    auto roundRectRadius = previewTextUnderlineWidth / 2;
    auto previewTextRects = pattern->GetPreviewTextRects();
    drawingContext.canvas.Save();
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(previewTextDecorationColor);
    drawingContext.canvas.AttachBrush(brush);
    for (const auto& previewTextRect : previewTextRects) {
        auto padding = &previewTextRect == &previewTextRects.back() ? 0 : roundRectRadius;
        RSRect rect(previewTextRect.Left(), previewTextRect.Bottom() - previewTextUnderlineWidth,
            previewTextRect.Right() + padding, previewTextRect.Bottom());
        drawingContext.canvas.DrawRoundRect(RSRoundRect(rect, roundRectRadius, roundRectRadius));
    }
    drawingContext.canvas.DetachBrush();
    drawingContext.canvas.Restore();
}

void RichEditorOverlayModifier::PaintCaret(DrawingContext& drawingContext) const
{
    if (!caretVisible_->Get()) {
        return;
    }
    bool isShowOriginCaret = floatingCaretVisible_->Get() && originCaretVisible_->Get();
    CHECK_NULL_VOID(!floatingCaretVisible_->Get() || isShowOriginCaret);
    auto& canvas = drawingContext.canvas;
    auto offset = caretOffset_->Get();
    canvas.Save();
    RSPen pen;
    pen.SetAntiAlias(true);
    float caretWidth = caretWidth_->Get();
    float caretHeight = caretHeight_->Get();
    pen.SetWidth(caretWidth);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    pen.SetColor(isShowOriginCaret ? originCaretColor_->Get() : caretColor_->Get());
    canvas.AttachPen(pen);
    float midPosX = offset.GetX() + caretWidth / 2;
    float startPosY = offset.GetY();
    float endPosY = startPosY + caretHeight;
    float roundCapRadius = caretWidth / 2;
    canvas.DrawLine(RSPoint(midPosX, startPosY + roundCapRadius), RSPoint(midPosX, endPosY - roundCapRadius));
    canvas.DetachPen();
    canvas.Restore();
}

void RichEditorOverlayModifier::PaintFloatingCaret(DrawingContext& drawingContext) const
{
    CHECK_NULL_VOID(floatingCaretVisible_->Get());
    auto& canvas = drawingContext.canvas;
    auto floatingCaretOffset = floatingCaretOffset_->Get();
    canvas.Save();
    RSPen pen;
    pen.SetAntiAlias(true);
    float caretWidth = caretWidth_->Get();
    float caretHeight = caretHeight_->Get();
    pen.SetWidth(caretWidth);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    pen.SetColor(caretColor_->Get());
    canvas.AttachPen(pen);
    float midPosX = floatingCaretOffset.GetX() + caretWidth / 2;
    float startPosY = floatingCaretOffset.GetY();
    float endPosY = startPosY + caretHeight;
    float roundCapRadius = caretWidth / 2;
    canvas.DrawLine(RSPoint(midPosX, startPosY + roundCapRadius), RSPoint(midPosX, endPosY - roundCapRadius));
    canvas.DetachPen();
    canvas.Restore();
}

void RichEditorOverlayModifier::PaintScrollBar(DrawingContext& context)
{
    auto scrollBarOverlayModifier = scrollBarOverlayModifier_.Upgrade();
    CHECK_NULL_VOID(scrollBarOverlayModifier);
    auto pattern = AceType::DynamicCast<RichEditorPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(!pattern || pattern->GetBarDisplayMode() != DisplayMode::OFF);
    scrollBarOverlayModifier->onDraw(context);
}

void RichEditorOverlayModifier::PaintEdgeEffect(const SizeF& frameSize, RSCanvas& canvas)
{
    auto edgeEffect = edgeEffect_.Upgrade();
    CHECK_NULL_VOID(edgeEffect);
    edgeEffect->Paint(canvas, frameSize, { 0.0f, 0.0f });
}

void RichEditorOverlayModifier::onDraw(DrawingContext& drawingContext)
{
    ACE_SCOPED_TRACE("RichEditorOverlayOnDraw");
    drawingContext.canvas.Save();
    auto richEditorPattern = AceType::DynamicCast<RichEditorPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    auto contentRect = richEditorPattern->GetTextContentRect();

    drawingContext.canvas.ClipRect(ToRSRect(contentRect), RSClipOp::INTERSECT);
    PaintCaret(drawingContext);
    PaintFloatingCaret(drawingContext);
    PaintPreviewTextDecoration(drawingContext);
    SetSelectedColor(richEditorPattern->GetSelectedBackgroundColor().GetValue());
    TextOverlayModifier::onDraw(drawingContext);
    drawingContext.canvas.Restore();

    PaintScrollBar(drawingContext);
    PaintEdgeEffect(frameSize_->Get(), drawingContext.canvas);
}

void RichEditorOverlayModifier::UpdateScrollBar(PaintWrapper* paintWrapper)
{
    auto richEditorPattern = AceType::DynamicCast<RichEditorPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    auto scrollBar = richEditorPattern->GetScrollControllerBar();
    if (!scrollBar || !scrollBar->NeedPaint()) {
        return;
    }
    auto scrollBarOverlayModifier = scrollBarOverlayModifier_.Upgrade();
    CHECK_NULL_VOID(scrollBarOverlayModifier);
    if (scrollBar->GetPositionModeUpdate()) {
        scrollBarOverlayModifier->SetPositionMode(scrollBar->GetPositionMode());
    }
    SetScrollBarOpacityType(static_cast<int32_t>(scrollBar->GetOpacityAnimationType()));
    scrollBarOverlayModifier->StartBarAnimation(scrollBar->GetHoverAnimationType(),
        scrollBar->GetOpacityAnimationType(), scrollBar->GetNeedAdaptAnimation(), scrollBar->GetActiveRect());
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    scrollBarOverlayModifier->SetBarColor(scrollBar->GetForegroundColor());
    scrollBar->SetOpacityAnimationType(OpacityAnimationType::NONE);
}

void RichEditorOverlayModifier::StartFloatingCaretLand(const OffsetF& originCaretOffset)
{
    AnimationOption option = AnimationOption();
    option.SetDuration(LAND_DURATION);
    option.SetCurve(LAND_CURVE);
    caretLanding_ = true;
    AnimationUtils::Animate(
        option,
        [weak = WeakClaim(this), originCaretOffset]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->SetFloatingCaretOffset(originCaretOffset);
        },
        [weak = WeakClaim(this), pattern = pattern_]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->SetFloatCaretLanding(false);
            auto richEditorPattern = AceType::DynamicCast<RichEditorPattern>(pattern.Upgrade());
            CHECK_NULL_VOID(richEditorPattern);
            richEditorPattern->ResetFloatingCaretState();
        });
}
} // namespace OHOS::Ace::NG

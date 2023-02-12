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
#include "core/components_ng/pattern/text_field/text_field_paint_method.h"

#include "foundation/graphic/graphic_2d/rosen/modules/2d_graphics/include/draw/path.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/border.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/placement.h"
#include "core/components/popup/popup_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/image_painter.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

CanvasDrawFunction TextFieldPaintMethod::GetContentDrawFunction(PaintWrapper* paintWrapper)
{
    auto textFieldPattern = DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_RETURN(textFieldPattern, nullptr);
    CHECK_NULL_RETURN(textFieldPattern->GetParagraph(), nullptr);
    auto offset = paintWrapper->GetContentOffset();
    auto passwordIconCanvasImage = textFieldPattern->GetTextObscured()
                                       ? textFieldPattern->GetHidePasswordIconCanvasImage()
                                       : textFieldPattern->GetShowPasswordIconCanvasImage();
    auto drawFunction = [paragraph = textFieldPattern->GetParagraph(), offset, textFieldPattern,
                            contentSize = paintWrapper->GetContentSize(),
                            contentOffset = paintWrapper->GetContentOffset(),
                            passwordIconCanvasImage](RSCanvas& canvas) {
        CHECK_NULL_VOID_NOLOG(textFieldPattern);
        auto frameRect = textFieldPattern->GetFrameRect();
        auto iconRect = textFieldPattern->GetImageRect();
        RSRect clipInnerRect;
        if (textFieldPattern->IsTextArea()) {
            clipInnerRect = RSRect(offset.GetX(), contentOffset.GetY(), contentSize.Width() + contentOffset.GetX(),
                contentOffset.GetY() + contentSize.Height());
        } else {
            clipInnerRect = RSRect(offset.GetX(), 0.0f,
                textFieldPattern->NeedShowPasswordIcon() ? frameRect.Width()
                                                         : contentSize.Width() + contentOffset.GetX(),
                frameRect.Height());
        }
        canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
        if (paragraph) {
            if (textFieldPattern->IsTextArea()) {
                paragraph->Paint(
                    &canvas, textFieldPattern->GetTextRect().GetX(), textFieldPattern->GetTextRect().GetY());
            } else {
                paragraph->Paint(&canvas, textFieldPattern->GetTextRect().GetX(), contentOffset.GetY());
            }
        }
        canvas.Restore();
        if (!textFieldPattern->NeedShowPasswordIcon()) {
            return;
        }
        CHECK_NULL_VOID_NOLOG(passwordIconCanvasImage);
        const ImagePainter passwordIconImagePainter(passwordIconCanvasImage);
        passwordIconImagePainter.DrawImage(canvas, iconRect.GetOffset(), iconRect.GetSize());
    };
    return drawFunction;
}

CanvasDrawFunction TextFieldPaintMethod::GetOverlayDrawFunction(PaintWrapper* paintWrapper)
{
    return [weak = WeakClaim(this), paintWrapper](RSCanvas& canvas) {
        auto textField = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(textField);
        textField->PaintCursor(canvas, paintWrapper);
        textField->PaintSelection(canvas, paintWrapper);
    };
}

void TextFieldPaintMethod::PaintSelection(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto textFieldPattern = DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    if (!textFieldPattern->InSelectMode()) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto theme = themeManager->GetTheme<TextFieldTheme>();
    auto paintProperty = DynamicCast<TextFieldPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(theme->GetSelectedColor().GetValue());
    canvas.AttachBrush(brush);
    auto startOffset =
        std::min(textFieldPattern->GetSelectionBaseOffsetX(), textFieldPattern->GetSelectionDestinationOffsetX());
    auto endOffset =
        std::max(textFieldPattern->GetSelectionBaseOffsetX(), textFieldPattern->GetSelectionDestinationOffsetX());
    auto paintOffset = paintWrapper->GetContentOffset() - OffsetF(0.0f, textFieldPattern->GetBaseLineOffset());
    RSRect clipInnerRect(paintOffset.GetX(), paintOffset.GetY(),
        paintOffset.GetX() + paintWrapper->GetContentSize().Width(),
        paintOffset.GetY() + paintWrapper->GetContentSize().Height());
    canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
    auto textBoxes = textFieldPattern->GetTextBoxes();
    auto textRect = textFieldPattern->GetTextRect();
    bool isTextArea = textFieldPattern->IsTextArea();
    switch (paintProperty->GetInputStyleValue(InputStyle::DEFAULT)) {
        case InputStyle::DEFAULT:
            // for default style, selection height is equal to the content height
            for (const auto& textBox : textBoxes) {
                canvas.DrawRect(RSRect(
                    textBox.rect_.GetLeft() + (isTextArea ? paintWrapper->GetContentOffset().GetX() : textRect.GetX()),
                    textBox.rect_.GetTop() + (isTextArea ? textRect.GetY() : paintWrapper->GetContentOffset().GetY()),
                    textBox.rect_.GetRight() + (isTextArea ? paintWrapper->GetContentOffset().GetX() : textRect.GetX()),
                    textBox.rect_.GetBottom() +
                        (isTextArea ? textRect.GetY() : paintWrapper->GetContentOffset().GetY())));
            }
            break;
        case InputStyle::INLINE:
            // for inline style, selection height is equal to the frame height
            canvas.DrawRect(RSRect(startOffset, textFieldPattern->GetFrameRect().GetY(), endOffset,
                textFieldPattern->GetFrameRect().GetY() + textFieldPattern->GetFrameRect().Height()));
            break;
        default:
            LOGE("Unsupported style");
    }

    canvas.Restore();
}

void TextFieldPaintMethod::PaintCursor(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto textFieldPattern = DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    if (!textFieldPattern->GetCursorVisible() || textFieldPattern->GetSelectMode() == SelectionMode::SELECT_ALL) {
        return;
    }
    auto paragraph = textFieldPattern->GetParagraph();
    CHECK_NULL_VOID(paragraph);
    auto paintProperty = DynamicCast<TextFieldPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);

    auto cursorColor = paintProperty->GetCursorColorValue();
    canvas.Save();
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(cursorColor.GetValue());
    canvas.AttachBrush(brush);
    auto paintOffset = paintWrapper->GetContentOffset() - OffsetF(0.0f, textFieldPattern->GetBaseLineOffset());
    RSRect clipInnerRect(paintOffset.GetX(), paintOffset.GetY(),
        paintOffset.GetX() + paintWrapper->GetContentSize().Width(),
        paintOffset.GetY() + paintWrapper->GetContentSize().Height());
    canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
    float caretHeight = textFieldPattern->GetCaretRect().Height();
    if (!textFieldPattern->IsTextArea()) {
        auto top = static_cast<float>(
            paintWrapper->GetContentOffset().GetY() + (paintWrapper->GetContentSize().Height() - caretHeight) / 2);
        auto cursorOffsetX = textFieldPattern->GetCaretOffsetX();
        canvas.DrawRect(RSRect(
            cursorOffsetX, top, cursorOffsetX + static_cast<float>(CURSOR_WIDTH.ConvertToPx()), top + caretHeight));
        canvas.Restore();
        return;
    }
    auto cursorOffset = textFieldPattern->GetCaretRect().GetOffset();
    canvas.DrawRect(RSRect(cursorOffset.GetX(), cursorOffset.GetY(),
        cursorOffset.GetX() + static_cast<float>(CURSOR_WIDTH.ConvertToPx()), cursorOffset.GetY() + caretHeight));
    canvas.Restore();
}

} // namespace OHOS::Ace::NG

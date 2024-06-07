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

#include "core/components_ng/pattern/text_input/text_input_layout_algorithm.h"

#include "base/utils/utils.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
std::optional<SizeF> TextInputLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);

    // Construct text style.
    TextStyle textStyle;
    ConstructTextStyles(frameNode, textStyle, textContent_, showPlaceHolder_);
    std::replace(textContent_.begin(), textContent_.end(), '\n', ' ');

    auto isInlineStyle = pattern->IsNormalInlineState();

    direction_ = textFieldLayoutProperty->GetNonAutoLayoutDirection();

    // Create paragraph.
    auto disableTextAlign = !pattern->IsTextArea() && !showPlaceHolder_ && !isInlineStyle;
    auto textFieldContentConstraint = CalculateContentMaxSizeWithCalculateConstraint(contentConstraint, layoutWrapper);
    if (IsNeedAdaptFontSize(textStyle, textFieldLayoutProperty, textFieldContentConstraint)) {
        if (!AddAdaptFontSizeAndAnimations(textStyle, textFieldLayoutProperty,
            BuildLayoutConstraintWithoutResponseArea(textFieldContentConstraint, layoutWrapper), layoutWrapper)) {
            return std::nullopt;
        }
    } else {
        CreateParagraphEx(textStyle, textContent_, contentConstraint, layoutWrapper);
    }

    autoWidth_ = textFieldLayoutProperty->GetWidthAutoValue(false);

    if (textContent_.empty()) {
        // Used for empty text.
        preferredHeight_ = pattern->PreferredLineHeight(true);
    }

    // Paragraph layout.
    if (isInlineStyle) {
        CreateInlineParagraph(textStyle, textContent_, false, pattern->GetNakedCharPosition(), disableTextAlign);
        return InlineMeasureContent(textFieldContentConstraint, layoutWrapper);
    }
    if (showPlaceHolder_) {
        return PlaceHolderMeasureContent(textFieldContentConstraint, layoutWrapper, 0);
    }
    return TextInputMeasureContent(textFieldContentConstraint, layoutWrapper, 0);
}

void TextInputLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    OptionalSizeF frameSize;
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    float contentWidth = 0.0f;
    float contentHeight = 0.0f;
    if (content) {
        auto contentSize = content->GetRect().GetSize();
        contentWidth = contentSize.Width();
        contentHeight = contentSize.Height();
    }
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(textFieldTheme);
    auto defaultHeight = GetDefaultHeightByType(layoutWrapper);

    if (LessOrEqual(contentWidth, 0)) {
        frameSize.SetWidth(contentWidth);
    } else {
        frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingAndBorderSum());
    }

    auto contentConstraint = layoutWrapper->GetLayoutProperty()->CreateContentConstraint();
    auto textFieldContentConstraint = CalculateContentMaxSizeWithCalculateConstraint(contentConstraint, layoutWrapper);
    if (textFieldContentConstraint.selfIdealSize.Height().has_value()) {
        if (LessOrEqual(contentWidth, 0)) {
            frameSize.SetHeight(textFieldContentConstraint.maxSize.Height());
        } else {
            frameSize.SetHeight(
                textFieldContentConstraint.maxSize.Height() + pattern->GetVerticalPaddingAndBorderSum());
        }
    } else {
        auto height = LessNotEqual(contentHeight, defaultHeight)
                          ? defaultHeight + pattern->GetVerticalPaddingAndBorderSum()
                          : contentHeight + pattern->GetVerticalPaddingAndBorderSum();
        frameSize.SetHeight(height);
    }
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        frameSize.Constrain(layoutConstraint->minSize, layoutConstraint->maxSize);
    } else if (!layoutWrapper->GetLayoutProperty()->GetLayoutRect()) {
        auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(frameSize,
            layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
            layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
        frameSize.SetHeight(finalSize.Height());
    }
    ResponseAreaMeasure(layoutWrapper, frameSize, textFieldContentConstraint, contentWidth, contentHeight);
}

void TextInputLayoutAlgorithm::ResponseAreaMeasure(LayoutWrapper* layoutWrapper, OptionalSizeF& frameSize,
    LayoutConstraintF& textFieldContentConstraint, float contentWidth, float contentHeight)
{
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);

    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());
    auto responseArea = pattern->GetResponseArea();
    auto cleanNodeResponseArea = pattern->GetCleanNodeResponseArea();
    float childWidth = 0.0f;
    bool updateFrameSize = false;
    if (responseArea) {
        updateFrameSize = true;
        auto childIndex = frameNode->GetChildIndex(responseArea->GetFrameNode());
        childWidth += responseArea->Measure(layoutWrapper, childIndex).Width();
    }
    if (cleanNodeResponseArea) {
        updateFrameSize = true;
        auto childIndex = frameNode->GetChildIndex(cleanNodeResponseArea->GetFrameNode());
        childWidth += cleanNodeResponseArea->Measure(layoutWrapper, childIndex).Width();
    }
    if (updateFrameSize) {
        if (LessOrEqual(contentWidth + childWidth, textFieldContentConstraint.maxSize.Width())) {
            frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingAndBorderSum() + childWidth);
        } else {
            if (showPlaceHolder_) {
                PlaceHolderMeasureContent(textFieldContentConstraint, layoutWrapper, childWidth);
            }
            content->SetSize(SizeF(textFieldContentConstraint.maxSize.Width() - childWidth, contentHeight));
            frameSize.SetWidth(
                textFieldContentConstraint.maxSize.Width() + pattern->GetHorizontalPaddingAndBorderSum());
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());
    }
}

void TextInputLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize() -
                SizeF(pattern->GetHorizontalPaddingAndBorderSum(), pattern->GetVerticalPaddingAndBorderSum());
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    CHECK_NULL_VOID(content);
    SizeT<float> contentSize = content->GetRect().GetSize();
    auto layoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    PipelineContext* context = layoutWrapper->GetHostNode()->GetContext();
    CHECK_NULL_VOID(context);
    parentGlobalOffset_ = layoutWrapper->GetHostNode()->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    Alignment align = Alignment::CENTER;
    auto isRTL = layoutProperty->GetNonAutoLayoutDirection() == TextDirection::RTL;
    auto hasAlign = false;
    if (layoutProperty->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
        hasAlign = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().has_value();
    }

    OffsetF offsetBase = OffsetF(pattern->GetPaddingLeft() + pattern->GetBorderLeft(),
        pattern->GetPaddingTop() + pattern->GetBorderTop());

    auto responseArea = pattern->GetResponseArea();
    auto cleanNodeResponseArea = pattern->GetCleanNodeResponseArea();
    auto unitNodeWidth = 0.0f;
    if (responseArea) {
        int32_t childIndex = frameNode->GetChildIndex(responseArea->GetFrameNode());
        responseArea->Layout(layoutWrapper, childIndex, unitNodeWidth);
    }
    if (cleanNodeResponseArea) {
        int32_t childIndex = frameNode->GetChildIndex(cleanNodeResponseArea->GetFrameNode());
        cleanNodeResponseArea->Layout(layoutWrapper, childIndex, unitNodeWidth);
    }

    UpdateContentPositionParams params = {
        .isRTL = isRTL,
        .offsetBase = offsetBase,
        .size = size,
        .contentSize = contentSize,
        .align = align,
        .responseArea = responseArea
    };
    UpdateContentPosition(params, content);

    auto paintProperty = pattern->GetPaintProperty<TextFieldPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    UpdateTextRectParams updateTextRectParams = {
        .layoutProperty = layoutProperty,
        .pattern = pattern,
        .contentSize = contentSize,
        .isRTL = isRTL,
        .responseArea = responseArea,
        .offsetBase = offsetBase
    };
    UpdateTextRect(updateTextRectParams);

    bool isInlineStyle = pattern->IsNormalInlineState();
    if (layoutProperty->GetShowCounterValue(false) && layoutProperty->HasMaxLength() && !isInlineStyle) {
        TextFieldLayoutAlgorithm::CounterLayout(layoutWrapper);
    }
}

void TextInputLayoutAlgorithm::UpdateContentPosition(const UpdateContentPositionParams &params,
    const std::unique_ptr<GeometryProperty> &content)
{
    OffsetF contentOffset =
        params.offsetBase + Alignment::GetAlignPosition(params.size, params.contentSize, params.align);
    if (params.isRTL) {
        if (params.responseArea) {
            RectF responseAreaRect = params.responseArea->GetAreaRect();
            content->SetOffset(OffsetF(params.offsetBase.GetX() + responseAreaRect.Width(), contentOffset.GetY()));
        } else {
            content->SetOffset(OffsetF(params.offsetBase.GetX(), contentOffset.GetY()));
        }
    } else {
        content->SetOffset(OffsetF(params.offsetBase.GetX(), contentOffset.GetY()));
    }
}

void TextInputLayoutAlgorithm::UpdateTextRect(const UpdateTextRectParams& params)
{
    if (LessOrEqual(textRect_.Width(), params.contentSize.Width())) {
        float textRectOffsetX = 0.0f;
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            textRectOffsetX = params.pattern->GetPaddingLeft();
        } else {
            textRectOffsetX = params.pattern->GetPaddingLeft() + params.pattern->GetBorderLeft();
        }
        bool isEmptyTextEditValue = params.pattern->GetTextValue().empty();
        bool isInlineStyle = params.pattern->IsNormalInlineState();
        if (!isEmptyTextEditValue && !isInlineStyle) {
            TextAlign textAlign = params.layoutProperty->GetTextAlignValue(TextAlign::START);
            params.pattern->CheckTextAlignByDirection(textAlign, direction_);
            switch (textAlign) {
                case TextAlign::START:
                    break;
                case TextAlign::CENTER:
                    textRectOffsetX += (params.contentSize.Width() - textRect_.Width()) * 0.5f;
                    break;
                case TextAlign::END:
                    textRectOffsetX += params.contentSize.Width() - textRect_.Width();
                    break;
                default:
                    break;
            }
        }
        if (params.isRTL) {
            if (params.responseArea) {
                RectF responseAreaRect = params.responseArea->GetAreaRect();
                textRect_.SetOffset(OffsetF(textRectOffsetX + responseAreaRect.Width(), params.offsetBase.GetY()));
            } else {
                textRect_.SetOffset(OffsetF(textRectOffsetX, params.offsetBase.GetY()));
            }
        } else {
            textRect_.SetOffset(OffsetF(textRectOffsetX, params.offsetBase.GetY()));
        }
    } else {
        textRect_.SetOffset({ params.pattern->GetTextRect().GetOffset().GetX(), params.offsetBase.GetY() });
    }
}

float TextInputLayoutAlgorithm::GetDefaultHeightByType(LayoutWrapper* layoutWrapper)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_RETURN(textFieldTheme, 0.0f);
    return static_cast<float>(textFieldTheme->GetContentHeight().ConvertToPx());
}

bool TextInputLayoutAlgorithm::CreateParagraphEx(const TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    // update child position.
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, false);
    auto isInlineStyle = pattern->IsNormalInlineState();
    auto isPasswordType = pattern->IsInPasswordMode();
    auto disableTextAlign = !pattern->IsTextArea() && !showPlaceHolder_ && !isInlineStyle;

    if (pattern->IsDragging() && !showPlaceHolder_ && !isInlineStyle) {
        CreateParagraph(textStyle, pattern->GetDragContents(), content,
            isPasswordType && pattern->GetTextObscured() && !showPlaceHolder_, disableTextAlign);
    } else {
        CreateParagraph(textStyle, content, isPasswordType && pattern->GetTextObscured() && !showPlaceHolder_,
            pattern->GetNakedCharPosition(), disableTextAlign);
    }
    return true;
}

LayoutConstraintF TextInputLayoutAlgorithm::BuildLayoutConstraintWithoutResponseArea(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, contentConstraint);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, contentConstraint);

    auto responseArea = pattern->GetResponseArea();
    auto cleanNodeResponseArea = pattern->GetCleanNodeResponseArea();
    float childWidth = 0.0f;
    if (responseArea) {
        auto childIndex = frameNode->GetChildIndex(responseArea->GetFrameNode());
        childWidth += responseArea->Measure(layoutWrapper, childIndex).Width();
    }
    if (cleanNodeResponseArea) {
        auto childIndex = frameNode->GetChildIndex(cleanNodeResponseArea->GetFrameNode());
        childWidth += cleanNodeResponseArea->Measure(layoutWrapper, childIndex).Width();
    }

    auto newLayoutConstraint = contentConstraint;
    newLayoutConstraint.maxSize.MinusWidth(childWidth);
    if (newLayoutConstraint.selfIdealSize.Width()) {
        newLayoutConstraint.selfIdealSize.SetWidth(newLayoutConstraint.selfIdealSize.Width().value() - childWidth);
    }
    return newLayoutConstraint;
}
} // namespace OHOS::Ace::NG

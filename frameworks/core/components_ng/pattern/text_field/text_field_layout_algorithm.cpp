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

#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/i18n/localization.h"
#include "base/log/log_wrapper.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "core/common/font_manager.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text/text_layout_adapter.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/pattern/text_field/text_field_content_modifier.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float PARAGRAPH_SAVE_BOUNDARY = 1.0f;
constexpr uint32_t INLINE_DEFAULT_VIEW_MAXLINE = 3;
constexpr uint32_t COUNTER_TEXT_MAXLINE = 1;
constexpr int32_t DEFAULT_MODE = -1;
constexpr int32_t SHOW_COUNTER_PERCENT = 100;
constexpr double TEXT_DECORATION_DISABLED_COLOR_ALPHA = 0.2;
constexpr Dimension INLINE_MIN_WITH = 16.0_vp;
} // namespace
void TextFieldLayoutAlgorithm::ConstructTextStyles(
    const RefPtr<FrameNode>& frameNode, TextStyle& textStyle, std::string& textContent, bool& showPlaceHolder)
{
    CHECK_NULL_VOID(frameNode);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(textFieldTheme);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto textFieldLayoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(textFieldLayoutProperty);
    auto isInlineStyle = pattern->IsNormalInlineState();
    auto isTextArea = pattern->IsTextArea();

    if (!pattern->GetTextValue().empty()) {
        UpdateTextStyle(frameNode, textFieldLayoutProperty, textFieldTheme, textStyle, pattern->IsDisabled());
        textContent = pattern->GetTextValue();
        UpdateTextStyleTextOverflowAndWordBreak(textStyle, isTextArea, isInlineStyle, textFieldLayoutProperty);
    } else {
        UpdatePlaceholderTextStyle(
            frameNode, textFieldLayoutProperty, textFieldTheme, textStyle, pattern->IsDisabled());
        textContent = textFieldLayoutProperty->GetPlaceholderValue("");
        showPlaceHolder = true;
    }

    UpdateTextFadeoutTextStyle(frameNode, textFieldTheme, textStyle);

    textIndent_ = textStyle.GetTextIndent();
    auto fontManager = pipeline->GetFontManager();
    if (fontManager && !(fontManager->GetAppCustomFont().empty()) &&
        !(textFieldLayoutProperty->GetFontFamily().has_value())) {
        textStyle.SetFontFamilies(Framework::ConvertStrToFontFamilies(fontManager->GetAppCustomFont()));
    }
    // use for modifier.
    auto contentModifier = pattern->GetContentModifier();
    if (contentModifier) {
        SetPropertyToModifier(textStyle, contentModifier);
        contentModifier->ModifyTextStyle(textStyle);
        contentModifier->SetFontReady(false);
    }
}

void TextFieldLayoutAlgorithm::UpdateTextStyleTextOverflowAndWordBreak(TextStyle& textStyle, bool isTextArea,
    bool isInlineStyle, const RefPtr<TextFieldLayoutProperty>& textFieldLayoutProperty)
{
    if (!isTextArea && isInlineStyle) {
        //textinput inline
        if (textFieldLayoutProperty->HasTextOverflow()) {
            if (textFieldLayoutProperty->GetTextOverflow() == TextOverflow::DEFAULT) {
                textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
            } else {
                textStyle.SetTextOverflow(textFieldLayoutProperty->GetTextOverflow().value());
            }
        } else {
            textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
        }
    } else {
        if (textFieldLayoutProperty->HasTextOverflow()) {
            if (textFieldLayoutProperty->GetTextOverflow() == TextOverflow::DEFAULT) {
                textStyle.SetTextOverflow(TextOverflow::CLIP);
            } else {
                textStyle.SetTextOverflow(textFieldLayoutProperty->GetTextOverflow().value());
            }
        } else {
            textStyle.SetTextOverflow(TextOverflow::CLIP);
        }
    }

    if (isTextArea || isInlineStyle) {
        textStyle.SetWordBreak(textFieldLayoutProperty->GetWordBreak().value_or(WordBreak::BREAK_WORD));
    }
    // Not Support TextOverflow::MARQUEE Now
    if (textStyle.GetTextOverflow() == TextOverflow::MARQUEE) {
        if (!isTextArea && isInlineStyle) {
            textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
        } else {
            textStyle.SetTextOverflow(TextOverflow::CLIP);
        }
    }
}

std::optional<SizeF> TextFieldLayoutAlgorithm::InlineMeasureContent(const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(textFieldLayoutProperty, std::nullopt);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);
    auto textFieldTheme = pattern->GetTheme();
    CHECK_NULL_RETURN(textFieldTheme, std::nullopt);

    float contentWidth = 0.0f;
    auto safeBoundary = textFieldTheme->GetInlineBorderWidth().ConvertToPx() * 2;
    if (pattern->HasFocus()) {
        ApplyIndent(contentConstraint.maxSize.Width()
            - static_cast<float>(safeBoundary) - PARAGRAPH_SAVE_BOUNDARY);
        paragraph_->Layout(
            contentConstraint.maxSize.Width() - static_cast<float>(safeBoundary) - PARAGRAPH_SAVE_BOUNDARY);
        auto longestLine = std::ceil(paragraph_->GetLongestLine());
        paragraph_->Layout(std::min(static_cast<float>(longestLine), paragraph_->GetMaxWidth()));
        contentWidth = ConstraintWithMinWidth(
            contentConstraint, layoutWrapper, paragraph_, static_cast<float>(safeBoundary) + PARAGRAPH_SAVE_BOUNDARY);
    } else {
        ApplyIndent(contentConstraint.maxSize.Width());
        paragraph_->Layout(contentConstraint.maxSize.Width());
        contentWidth = ConstraintWithMinWidth(contentConstraint, layoutWrapper, paragraph_);
        // calc inline status in advance
        auto widthOffSet = contentConstraint.selfIdealSize.Width().has_value()?
            pattern->GetPaddingLeft() + pattern->GetPaddingRight() - safeBoundary : 0.0f - safeBoundary;
        inlineParagraph_->Layout(contentConstraint.maxSize.Width() + widthOffSet
            - safeBoundary - PARAGRAPH_SAVE_BOUNDARY);
        auto longestLine = std::ceil(inlineParagraph_->GetLongestLine());
        inlineParagraph_->Layout(std::min(static_cast<float>(longestLine), inlineParagraph_->GetMaxWidth()));
        auto inlineContentWidth = ConstraintWithMinWidth(contentConstraint, layoutWrapper, inlineParagraph_,
            static_cast<float>(safeBoundary) + PARAGRAPH_SAVE_BOUNDARY);
        inlineMeasureItem_.inlineScrollRectOffsetX = contentWidth
            + pattern->GetHorizontalPaddingAndBorderSum() - inlineContentWidth - safeBoundary - PARAGRAPH_SAVE_BOUNDARY;
        UpdateParagraphTextFadeoutWidth(contentConstraint, frameNode);
    }

    textRect_.SetSize(SizeF(GetVisualTextWidth(), paragraph_->GetHeight()));

    auto inlineIdealHeight = contentConstraint.maxSize.Height();
    GetInlineMeasureItem(contentConstraint, layoutWrapper, inlineIdealHeight);
    auto contentHeight = GreatNotEqual(paragraph_->GetLongestLine(), 0.0)
        ? paragraph_->GetHeight() : std::max(preferredHeight_, paragraph_->GetHeight());
    auto minWidth = INLINE_MIN_WITH.ConvertToPx();
    contentWidth = GreatNotEqual(contentWidth, minWidth) ? contentWidth : minWidth;
    return SizeF(contentWidth, std::min(inlineIdealHeight, contentHeight));
}

void TextFieldLayoutAlgorithm::GetInlineMeasureItem(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper, float& inlineIdealHeight)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textFieldLayoutProperty);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);

    if (pattern->HasFocus() && paragraph_->GetLineCount() != 0) {
        pattern->SetSingleLineHeight(paragraph_->GetHeight() / paragraph_->GetLineCount());
        // The maximum height of the inline mode defaults to a maximum of three rows.
        inlineIdealHeight =
            pattern->GetSingleLineHeight() * textFieldLayoutProperty->GetMaxViewLinesValue(INLINE_DEFAULT_VIEW_MAXLINE);
        inlineMeasureItem_.inlineSizeHeight = pattern->GetSingleLineHeight() * std::min(
            static_cast<uint32_t>(paragraph_->GetLineCount()),
            textFieldLayoutProperty->GetMaxViewLinesValue(INLINE_DEFAULT_VIEW_MAXLINE));
    } else {
        // calc inline status in advance
        CalcInlineMeasureItem(layoutWrapper);
    }
}

void TextFieldLayoutAlgorithm::ApplyIndent(double width)
{
    if (LessOrEqual(textIndent_.Value(), 0.0)) {
        return;
    }
    // first line indent
    CHECK_NULL_VOID(paragraph_);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    double indentValue = 0.0;
    if (textIndent_.Unit() != DimensionUnit::PERCENT) {
        if (!textIndent_.NormalizeToPx(pipeline->GetDipScale(),
            pipeline->GetFontScale(), pipeline->GetLogicScale(), width, indentValue)) {
            return;
        }
    } else {
        indentValue = width * textIndent_.Value();
    }
    indent_ = static_cast<float>(indentValue);
    std::vector<float> indents;
    // only indent first line
    indents.emplace_back(indent_);
    indents.emplace_back(0.0);
    paragraph_->SetIndents(indents);
}

void TextFieldLayoutAlgorithm::CalcInlineMeasureItem(LayoutWrapper* layoutWrapper)
{
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textFieldLayoutProperty);
    auto lineCount = inlineParagraph_->GetLineCount() != 0 ? inlineParagraph_->GetLineCount() : 1;
    inlineMeasureItem_.inlineSizeHeight = inlineParagraph_->GetHeight() / lineCount
        * std::min(static_cast<uint32_t>(lineCount),
            textFieldLayoutProperty->GetMaxViewLinesValue(INLINE_DEFAULT_VIEW_MAXLINE));
    inlineMeasureItem_.inlineContentRectHeight = GreatNotEqual(inlineParagraph_->GetLongestLine(), 0.0)
        ? inlineParagraph_->GetHeight() : std::max(preferredHeight_, inlineParagraph_->GetHeight());
    inlineMeasureItem_.inlineLastOffsetY =
        std::max(inlineMeasureItem_.inlineSizeHeight, inlineMeasureItem_.inlineContentRectHeight)
        - std::min(inlineMeasureItem_.inlineSizeHeight, inlineMeasureItem_.inlineContentRectHeight);
}

float TextFieldLayoutAlgorithm::ConstraintWithMinWidth(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper,
    RefPtr<Paragraph>& paragraph, float removeValue)
{
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) &&
        !layoutWrapper->GetLayoutProperty()->GetLayoutRect()) {
        const auto& calcLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
        if (calcLayoutConstraint && calcLayoutConstraint->minSize.has_value() &&
            calcLayoutConstraint->minSize->Width().has_value() &&
            !contentConstraint.selfIdealSize.Width().has_value()) {
            auto width = std::max(contentConstraint.minSize.Width() - removeValue, paragraph->GetLongestLine());
            if (width != paragraph->GetLongestLine()) {
                paragraph->Layout(width);
            } else {
                if (LessNotEqual(paragraph->GetLongestLine(), paragraph->GetMaxWidth())) {
                    paragraph->Layout(std::ceil(paragraph->GetLongestLine()));
                }
                return contentConstraint.selfIdealSize.Width().has_value() ? paragraph->GetMaxWidth()
                                                                           : GetVisualTextWidth();
            }
        }
    }
    return std::max(paragraph->GetMaxWidth(), 0.0f);
}

SizeF TextFieldLayoutAlgorithm::PlaceHolderMeasureContent(const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper, float imageWidth)
{
    paragraph_->Layout(contentConstraint.maxSize.Width() - imageWidth);

    // Adapts to auto width.
    if (autoWidth_) {
        auto minWidth = static_cast<float>(INLINE_MIN_WITH.ConvertToPx());
        paragraph_->Layout(std::max(minWidth, std::ceil(paragraph_->GetLongestLine())));
    }

    auto contentWidth = ConstraintWithMinWidth(contentConstraint, layoutWrapper, paragraph_, imageWidth);
    CounterNodeMeasure(contentWidth, layoutWrapper);

    UpdateParagraphTextFadeoutWidth(contentConstraint, layoutWrapper->GetHostNode());

    auto height = GreatNotEqual(paragraph_->GetLongestLine(), 0.0)
                      ? paragraph_->GetHeight()
                      : std::max(preferredHeight_, paragraph_->GetHeight());

    auto contentHeight = std::min(contentConstraint.maxSize.Height(), height);

    textRect_.SetSize(SizeF(GetVisualTextWidth(), paragraph_->GetHeight()));

    return SizeF(contentWidth, contentHeight);
}

SizeF TextFieldLayoutAlgorithm::TextAreaMeasureContent(const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper)
{
    ApplyIndent(contentConstraint.maxSize.Width());
    paragraph_->Layout(contentConstraint.maxSize.Width());

    auto contentWidth = ConstraintWithMinWidth(contentConstraint, layoutWrapper, paragraph_);

    if (autoWidth_) {
        contentWidth = std::min(contentWidth, paragraph_->GetLongestLine());
        auto minWidth = INLINE_MIN_WITH.ConvertToPx();
        contentWidth = GreatNotEqual(contentWidth, minWidth) ? contentWidth : minWidth;
        paragraph_->Layout(std::ceil(contentWidth));
    }

    auto counterNodeHeight = CounterNodeMeasure(contentWidth, layoutWrapper);

    auto height = GreatNotEqual(paragraph_->GetLongestLine(), 0.0)
                      ? paragraph_->GetHeight()
                      : std::max(preferredHeight_, paragraph_->GetHeight());

    auto contentHeight = std::min(contentConstraint.maxSize.Height() - counterNodeHeight, height);

    textRect_.SetSize(SizeF(GetVisualTextWidth(), paragraph_->GetHeight()));
    return SizeF(contentWidth, contentHeight);
}

SizeF TextFieldLayoutAlgorithm::TextInputMeasureContent(const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper, float imageWidth)
{
    ApplyIndent(contentConstraint.maxSize.Width());
    paragraph_->Layout(std::numeric_limits<double>::infinity());
    auto longestLine = paragraph_->GetLongestLine();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, SizeF());
    auto layoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, SizeF());
    if (layoutProperty->HasLetterSpacing()) {
        auto letterSpacing = layoutProperty->GetLetterSpacing().value().ConvertToPx();
        if (GreatNotEqual(letterSpacing, 0.0)) {
            longestLine = longestLine + letterSpacing;
        }
    }
    paragraph_->Layout(std::ceil(longestLine) + indent_); // paragraph_->GetLongestLine()));

    auto contentWidth = contentConstraint.maxSize.Width() - imageWidth;
    CounterNodeMeasure(contentWidth, layoutWrapper);
    if (autoWidth_) {
        auto minWidth = INLINE_MIN_WITH.ConvertToPx();
        contentWidth = GreatNotEqual(contentWidth, minWidth) ? contentWidth : minWidth;
        contentWidth = std::min(contentWidth, longestLine);
    }

    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) &&
        !layoutWrapper->GetLayoutProperty()->GetLayoutRect()) {
        const auto& calcLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
        if (calcLayoutConstraint && calcLayoutConstraint->minSize.has_value() &&
            calcLayoutConstraint->minSize->Width().has_value() &&
            !contentConstraint.selfIdealSize.Width().has_value()) {
            contentWidth = std::min(contentConstraint.maxSize.Width() - imageWidth,
                std::max(longestLine, contentConstraint.minSize.Width() - imageWidth));
        }
    }

    auto height = GreatNotEqual(longestLine, 0.0)
                      ? paragraph_->GetHeight()
                      : std::max(preferredHeight_, paragraph_->GetHeight());

    auto contentHeight = std::min(contentConstraint.maxSize.Height(), height);

    textRect_.SetSize(SizeF(std::max(0.0f, longestLine) + indent_, paragraph_->GetHeight()));
    return SizeF(contentWidth, contentHeight);
}

void TextFieldLayoutAlgorithm::UpdateCounterNode(
    uint32_t textLength, uint32_t maxLength, const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto counterNode = pattern->GetCounterNode().Upgrade();
    CHECK_NULL_VOID(counterNode);
    auto textLayoutProperty = DynamicCast<TextLayoutProperty>(counterNode->GetLayoutProperty());
    CHECK_NULL_VOID(textLayoutProperty);
    auto textFieldLayoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(textFieldLayoutProperty);

    std::string counterText;
    TextStyle countTextStyle =
        pattern->GetShowCounterStyleValue() ? theme->GetOverCountTextStyle() : theme->GetCountTextStyle();
    auto counterType = textFieldLayoutProperty->GetSetCounterValue(DEFAULT_MODE);
    auto limitSize = static_cast<uint32_t>(static_cast<int32_t>(maxLength) * counterType / SHOW_COUNTER_PERCENT);
    if (counterType == DEFAULT_MODE || (textLength >= limitSize && counterType != DEFAULT_MODE)) {
        counterText = std::to_string(textLength) + "/" + std::to_string(maxLength);
    }
    textLayoutProperty->UpdateContent(counterText);
    textLayoutProperty->UpdateFontSize(countTextStyle.GetFontSize());
    textLayoutProperty->UpdateTextColor(countTextStyle.GetTextColor());
    textLayoutProperty->UpdateFontWeight(countTextStyle.GetFontWeight());
    textLayoutProperty->UpdateTextAlign(TextAlign::END);
    textLayoutProperty->UpdateMaxLines(COUNTER_TEXT_MAXLINE);
    auto host = counterNode->GetHostNode();
    CHECK_NULL_VOID(host);
    auto context = host->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateForegroundColor(countTextStyle.GetTextColor());
    host->Measure(contentConstraint);
}

void TextFieldLayoutAlgorithm::CounterLayout(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    auto counterNode = pattern->GetCounterNode().Upgrade();
    auto isInlineStyle = pattern->IsNormalInlineState();
    auto isShowPassword = pattern->IsShowPasswordIcon();
    if (counterNode && !isShowPassword && !isInlineStyle) {
        auto frameNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TextFieldPattern>();
        CHECK_NULL_VOID(pattern);
        auto frameRect = layoutWrapper->GetGeometryNode()->GetFrameRect();
        auto textGeometryNode = counterNode->GetGeometryNode();
        CHECK_NULL_VOID(textGeometryNode);
        const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
        CHECK_NULL_VOID(content);
        if (!pattern->IsTextArea()) {
            auto property = frameNode->GetLayoutProperty();
            CHECK_NULL_VOID(property);
            auto isRTL = property->GetNonAutoLayoutDirection() == TextDirection::RTL;
            if (isRTL) {
                auto contentRect = layoutWrapper->GetGeometryNode()->GetContentRect();
                textGeometryNode->SetFrameOffset(
                    OffsetF(contentRect.GetX(), frameRect.Height() + textGeometryNode->GetFrameRect().Height()));
                counterNode->Layout();
            } else {
                auto contentRect = layoutWrapper->GetGeometryNode()->GetContentRect();
                auto counterWidth = counterNode->GetGeometryNode()->GetFrameSize().Width();
                auto textGeometryNode = counterNode->GetGeometryNode();
                CHECK_NULL_VOID(textGeometryNode);
                textGeometryNode->SetFrameOffset(OffsetF(contentRect.Width() - counterWidth,
                    frameRect.Height() + textGeometryNode->GetFrameRect().Height()));
                counterNode->Layout();
            }
        } else {
            textGeometryNode->SetFrameOffset(OffsetF(content->GetRect().GetX(),
                frameRect.Height() - pattern->GetPaddingBottom() - textGeometryNode->GetFrameRect().Height()));
            counterNode->Layout();
        }
    }
}

float TextFieldLayoutAlgorithm::CounterNodeMeasure(float contentWidth, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, 0.0f);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, 0.0f);
    auto textFieldLayoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_RETURN(textFieldLayoutProperty, 0.0f);
    auto isInlineStyle = pattern->IsNormalInlineState();
    auto isShowPassword = pattern->IsShowPasswordIcon();
    if (textFieldLayoutProperty->GetShowCounterValue(false) && textFieldLayoutProperty->HasMaxLength() &&
        !isInlineStyle && !isShowPassword) {
        auto counterNode = DynamicCast<UINode>(pattern->GetCounterNode().Upgrade());
        CHECK_NULL_RETURN(counterNode, 0.0f);
        auto counterNodeLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(frameNode->GetChildIndex(counterNode));
        if (counterNodeLayoutWrapper) {
            auto textLength =
                static_cast<uint32_t>(showPlaceHolder_ ? 0 : StringUtils::ToWstring(textContent_).length());
            auto maxLength = static_cast<uint32_t>(textFieldLayoutProperty->GetMaxLength().value());
            LayoutConstraintF textContentConstraint;
            textContentConstraint.UpdateIllegalSelfIdealSizeWithCheck(OptionalSizeF(contentWidth, std::nullopt));
            UpdateCounterNode(textLength, maxLength, textContentConstraint, layoutWrapper);
            return counterNodeLayoutWrapper->GetGeometryNode()->GetFrameSize().Height();
        }
    }
    return 0.0f;
}

float TextFieldLayoutAlgorithm::GetVisualTextWidth() const
{
    return std::min(paragraph_->GetMaxWidth(), std::max(0.0f, paragraph_->GetLongestLine()));
}

void TextFieldLayoutAlgorithm::UpdateTextStyle(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme,
    TextStyle& textStyle, bool isDisabled)
{
    const std::vector<std::string> defaultFontFamily = { "sans-serif" };
    textStyle.SetFontFamilies(layoutProperty->GetFontFamilyValue(defaultFontFamily));
    FontRegisterCallback(frameNode, textStyle.GetFontFamilies());

    Dimension fontSize;
    if (layoutProperty->HasFontSize() && layoutProperty->GetFontSize().value_or(Dimension()).IsNonNegative()) {
        fontSize = layoutProperty->GetFontSizeValue(Dimension());
    } else {
        fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
    }
    textStyle.SetFontSize(fontSize);
    textStyle.SetTextAlign(layoutProperty->GetTextAlignValue(TextAlign::START));
    textStyle.SetLineBreakStrategy(layoutProperty->GetLineBreakStrategyValue(LineBreakStrategy::GREEDY));
    textStyle.SetFontWeight(
        layoutProperty->GetFontWeightValue(theme ? theme->GetFontWeight() : textStyle.GetFontWeight()));
    if (isDisabled) {
        textStyle.SetTextColor(theme ? theme->GetDisableTextColor() : textStyle.GetTextColor());
        if (layoutProperty->GetShowUnderlineValue(false) &&
            layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
            textStyle.SetTextColor(theme ? theme->GetTextColorDisable() : textStyle.GetTextColor());
        }
    } else {
        auto renderContext = frameNode->GetRenderContext();
        if (renderContext->HasForegroundColor()) {
            textStyle.SetTextColor(renderContext->GetForegroundColor().value());
        } else if (renderContext->HasForegroundColorStrategy()) {
            textStyle.SetTextColor(Color::BLACK);
        } else {
            textStyle.SetTextColor(
                layoutProperty->GetTextColorValue(theme ? theme->GetTextColor() : textStyle.GetTextColor()));
        }
    }
    if (layoutProperty->GetMaxLines()) {
        textStyle.SetMaxLines(layoutProperty->GetMaxLines().value());
    }
    if (layoutProperty->HasItalicFontStyle()) {
        textStyle.SetFontStyle(layoutProperty->GetItalicFontStyle().value());
    }
    if (layoutProperty->HasTextAlign()) {
        textStyle.SetTextAlign(layoutProperty->GetTextAlign().value());
    }
    if (layoutProperty->HasTextIndent()) {
        textStyle.SetTextIndent(layoutProperty->GetTextIndent().value());
    }
    UpdateTextStyleMore(frameNode, layoutProperty, theme, textStyle, isDisabled);
}

void TextFieldLayoutAlgorithm::UpdatePlaceholderTextStyle(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme, TextStyle& textStyle,
    bool isDisabled)
{
    const std::vector<std::string> defaultFontFamily = { "sans-serif" };
    textStyle.SetFontFamilies(layoutProperty->GetPlaceholderFontFamilyValue(defaultFontFamily));
    FontRegisterCallback(frameNode, textStyle.GetFontFamilies());

    Dimension fontSize;
    if (layoutProperty->GetPlaceholderValue("").empty()) {
        if (layoutProperty->HasFontSize() && layoutProperty->GetFontSize().value_or(Dimension()).IsNonNegative()) {
            fontSize = layoutProperty->GetFontSizeValue(Dimension());
        } else {
            fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
        }
    } else {
        if (layoutProperty->HasPlaceholderFontSize() &&
            layoutProperty->GetPlaceholderFontSize().value_or(Dimension()).IsNonNegative()) {
            fontSize = layoutProperty->GetPlaceholderFontSizeValue(Dimension());
        } else {
            fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
        }
    }

    textStyle.SetFontSize(fontSize);
    textStyle.SetFontWeight(
        layoutProperty->GetPlaceholderFontWeightValue(theme ? theme->GetFontWeight() : textStyle.GetFontWeight()));
    if (isDisabled) {
        textStyle.SetTextColor(theme ? theme->GetDisableTextColor() : textStyle.GetTextColor());
        if (layoutProperty->GetShowUnderlineValue(false) &&
            layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
            textStyle.SetTextColor(theme ? theme->GetTextColorDisable() : textStyle.GetTextColor());
        }
    } else {
        textStyle.SetTextColor(layoutProperty->GetPlaceholderTextColorValue(
            theme ? theme->GetPlaceholderColor() : textStyle.GetTextColor()));
    }
    if (layoutProperty->HasPlaceholderMaxLines()) {
        textStyle.SetMaxLines(layoutProperty->GetPlaceholderMaxLines().value());
    }
    if (layoutProperty->HasPlaceholderItalicFontStyle()) {
        textStyle.SetFontStyle(layoutProperty->GetPlaceholderItalicFontStyle().value());
    }
    if (layoutProperty->HasPlaceholderTextAlign()) {
        textStyle.SetTextAlign(layoutProperty->GetPlaceholderTextAlign().value());
    }
    textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
    textStyle.SetTextAlign(layoutProperty->GetTextAlignValue(TextAlign::START));
    UpdatePlaceholderTextStyleMore(frameNode, layoutProperty, theme, textStyle, isDisabled);
}

LayoutConstraintF TextFieldLayoutAlgorithm::CalculateContentMaxSizeWithCalculateConstraint(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto textFieldContentConstraint = contentConstraint;
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, textFieldContentConstraint);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, textFieldContentConstraint);
    auto idealWidth = contentConstraint.selfIdealSize.Width().value_or(contentConstraint.maxSize.Width());
    auto idealHeight = contentConstraint.selfIdealSize.Height().value_or(contentConstraint.maxSize.Height());
    auto maxIdealSize = SizeF { idealWidth, idealHeight };
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN) &&
        !layoutWrapper->GetLayoutProperty()->GetLayoutRect()) {
        auto frameIdealSize = maxIdealSize + SizeF(pattern->GetHorizontalPaddingAndBorderSum(),
                                                 pattern->GetVerticalPaddingAndBorderSum());
        auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(static_cast<OptionalSize<float>>(frameIdealSize),
            layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
            layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
        finalSize.SetWidth(
            finalSize.Width().value_or(frameIdealSize.Width()) - pattern->GetHorizontalPaddingAndBorderSum());
        finalSize.SetHeight(
            finalSize.Height().value_or(frameIdealSize.Height()) - pattern->GetVerticalPaddingAndBorderSum());
        maxIdealSize.UpdateSizeWhenSmaller(finalSize.ConvertToSizeT());
    }
    textFieldContentConstraint.maxSize = maxIdealSize;
    return textFieldContentConstraint;
}

void TextFieldLayoutAlgorithm::FontRegisterCallback(
    const RefPtr<FrameNode>& frameNode, const std::vector<std::string>& fontFamilies)
{
    auto callback = [weakNode = WeakPtr<FrameNode>(frameNode)] {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pattern = frameNode->GetPattern<TextFieldPattern>();
        CHECK_NULL_VOID(pattern);
        auto modifier = DynamicCast<TextFieldContentModifier>(pattern->GetContentModifier());
        CHECK_NULL_VOID(modifier);
        modifier->SetFontReady(true);
    };
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto fontManager = pipeline->GetFontManager();
    if (fontManager) {
        bool isCustomFont = false;
        for (const auto& familyName : fontFamilies) {
            bool customFont = fontManager->RegisterCallbackNG(frameNode, familyName, callback);
            if (customFont) {
                isCustomFont = true;
            }
        }
        fontManager->AddVariationNodeNG(frameNode);
        if (isCustomFont || fontManager->IsDefaultFontChanged()) {
            auto pattern = frameNode->GetPattern<TextFieldPattern>();
            CHECK_NULL_VOID(pattern);
            pattern->SetIsCustomFont(true);
            auto modifier = DynamicCast<TextFieldContentModifier>(pattern->GetContentModifier());
            CHECK_NULL_VOID(modifier);
            modifier->SetIsCustomFont(true);
        }
    }
}

ParagraphStyle TextFieldLayoutAlgorithm::GetParagraphStyle(const TextStyle& textStyle, const std::string& content) const
{
    return { .direction = GetTextDirection(content, direction_),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = Localization::GetInstance()->GetFontLocale(),
        .wordBreak = textStyle.GetWordBreak(),
        .lineBreakStrategy = textStyle.GetLineBreakStrategy(),
        .textOverflow = textStyle.GetTextOverflow(),
        .fontSize = textStyle.GetFontSize().ConvertToPx() };
}

void TextFieldLayoutAlgorithm::CreateParagraph(const TextStyle& textStyle, std::string content, bool needObscureText,
    int32_t nakedCharPosition, bool disableTextAlign)
{
    auto paraStyle = GetParagraphStyle(textStyle, content);
    if (!disableTextAlign) {
        paraStyle.align = textStyle.GetTextAlign();
    }
    paragraph_ = Paragraph::Create(paraStyle, FontCollection::Current());
    CHECK_NULL_VOID(paragraph_);
    paragraph_->PushStyle(textStyle);
    StringUtils::TransformStrCase(content, static_cast<int32_t>(textStyle.GetTextCase()));
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    auto displayText = TextFieldPattern::CreateDisplayText(content, nakedCharPosition,
        needObscureText, theme->IsShowPasswordDirectly());
    paragraph_->AddText(displayText);
    paragraph_->Build();
}

void TextFieldLayoutAlgorithm::CreateParagraph(const TextStyle& textStyle, const std::vector<std::string>& contents,
    const std::string& content, bool needObscureText, bool disableTextAlign)
{
    TextStyle dragTextStyle = textStyle;
    Color color = textStyle.GetTextColor().ChangeAlpha(DRAGGED_TEXT_TRANSPARENCY);
    dragTextStyle.SetTextColor(color);
    std::vector<TextStyle> textStyles { textStyle, dragTextStyle, textStyle };

    auto style = textStyles.begin();
    ParagraphStyle paraStyle { .direction = GetTextDirection(content, direction_),
        .maxLines = style->GetMaxLines(),
        .fontLocale = Localization::GetInstance()->GetFontLocale(),
        .wordBreak = style->GetWordBreak(),
        .lineBreakStrategy = textStyle.GetLineBreakStrategy(),
        .textOverflow = style->GetTextOverflow(),
        .fontSize = style->GetFontSize().ConvertToPx() };
    if (!disableTextAlign) {
        paraStyle.align = style->GetTextAlign();
    }
    paragraph_ = Paragraph::Create(paraStyle, FontCollection::Current());
    CHECK_NULL_VOID(paragraph_);
    for (size_t i = 0; i < contents.size(); i++) {
        std::string splitStr = contents[i];
        if (splitStr.empty()) {
            continue;
        }
        if (style->GetMaxLines() == 1) {
            std::replace(splitStr.begin(), splitStr.end(), '\n', ' ');
        }
        auto& style = textStyles[i];
        paragraph_->PushStyle(style);
        StringUtils::TransformStrCase(splitStr, static_cast<int32_t>(style.GetTextCase()));
        if (needObscureText) {
            paragraph_->AddText(
                TextFieldPattern::CreateObscuredText(static_cast<int32_t>(StringUtils::ToWstring(splitStr).length())));
        } else {
            paragraph_->AddText(StringUtils::Str8ToStr16(splitStr));
        }
        paragraph_->PopStyle();
    }
    paragraph_->Build();
}

void TextFieldLayoutAlgorithm::CreateInlineParagraph(const TextStyle& textStyle, std::string content,
    bool needObscureText, int32_t nakedCharPosition, bool disableTextAlign)
{
    auto paraStyle = GetParagraphStyle(textStyle, content);
    if (!disableTextAlign) {
        paraStyle.align = textStyle.GetTextAlign();
    }
    paraStyle.maxLines = -1;
    inlineParagraph_ = Paragraph::Create(paraStyle, FontCollection::Current());
    CHECK_NULL_VOID(paragraph_);
    inlineParagraph_->PushStyle(textStyle);
    StringUtils::TransformStrCase(content, static_cast<int32_t>(textStyle.GetTextCase()));
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    auto displayText = TextFieldPattern::CreateDisplayText(content, nakedCharPosition,
        needObscureText, theme->IsShowPasswordDirectly());
    inlineParagraph_->AddText(displayText);
    inlineParagraph_->Build();
}

TextDirection TextFieldLayoutAlgorithm::GetTextDirection(const std::string& content, TextDirection direction)
{
    if (direction == TextDirection::LTR || direction == TextDirection::RTL) {
        return direction;
    }

    TextDirection textDirection = TextDirection::LTR;
    auto showingTextForWString = StringUtils::ToWstring(content);
    for (const auto& charOfShowingText : showingTextForWString) {
        if (TextLayoutadapter::IsLeftToRight(charOfShowingText)) {
            return TextDirection::LTR;
        }
        if (TextLayoutadapter::IsRightToLeft(charOfShowingText) ||
            TextLayoutadapter::IsRightTOLeftArabic(charOfShowingText)) {
            return TextDirection::RTL;
        }
    }
    return textDirection;
}

RefPtr<Paragraph> TextFieldLayoutAlgorithm::GetParagraph() const
{
    return paragraph_;
}

void TextFieldLayoutAlgorithm::GetSuitableSize(SizeF& maxSize, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto textFieldTheme = pattern->GetTheme();
    CHECK_NULL_VOID(textFieldTheme);
    auto safeBoundary = textFieldTheme->GetInlineBorderWidth().ConvertToPx() * 2;
    if (pattern->HasFocus() && pattern->IsInlineMode()) {
        maxSize.SetWidth(maxSize.Width() - static_cast<float>(safeBoundary) - PARAGRAPH_SAVE_BOUNDARY);
    }
}

float TextFieldLayoutAlgorithm::GetTextFieldDefaultHeight()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_RETURN(textFieldTheme, 0.0f);
    auto height = textFieldTheme->GetHeight();
    return static_cast<float>(height.ConvertToPx());
}

void TextFieldLayoutAlgorithm::SetPropertyToModifier(
    const TextStyle& textStyle, RefPtr<TextFieldContentModifier> modifier)
{
    CHECK_NULL_VOID(modifier);
    modifier->SetFontFamilies(textStyle.GetFontFamilies());
    modifier->SetFontSize(textStyle.GetFontSize());
    if (textStyle.GetAdaptTextSize()) {
        modifier->SetAdaptMinFontSize(textStyle.GetAdaptMinFontSize());
        modifier->SetAdaptMaxFontSize(textStyle.GetAdaptMaxFontSize());
    }
    modifier->SetFontWeight(textStyle.GetFontWeight());
    modifier->SetTextColor(textStyle.GetTextColor());
    modifier->SetFontStyle(textStyle.GetFontStyle());
    modifier->SetTextOverflow(textStyle.GetTextOverflow());
    modifier->SetTextDecoration(textStyle.GetTextDecoration(), textStyle.GetTextDecorationColor(),
        textStyle.GetTextDecorationStyle());
}

void TextFieldLayoutAlgorithm::UpdateUnitLayout(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto children = frameNode->GetChildren();
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    CHECK_NULL_VOID(content);
    auto contentSize = content->GetRect().GetSize();
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto layoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    if (!children.empty() && layoutProperty->GetShowUnderlineValue(false) &&
        layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
        CHECK_NULL_VOID(childWrapper);
        auto textGeometryNode = childWrapper->GetGeometryNode();
        CHECK_NULL_VOID(textGeometryNode);
        auto childFrameSize = textGeometryNode->GetFrameSize();
        unitWidth_ = childFrameSize.Width();
        textGeometryNode->SetFrameOffset(
            OffsetF({ content->GetRect().GetX() + contentSize.Width() - childFrameSize.Width(), 0.0 }));
        if (childFrameSize.Height() < size.Height()) {
            childWrapper->GetGeometryNode()->SetFrameSize(SizeF({ unitWidth_, size.Height() }));
        }
        childWrapper->Layout();
    }
}

bool TextFieldLayoutAlgorithm::AddAdaptFontSizeAndAnimations(TextStyle& textStyle,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper)
{
    if (textStyle.HasHeightOverride()) {
        return AdaptFontSizeForLineHeight(textStyle, layoutProperty, contentConstraint, layoutWrapper);
    }
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, false);
    bool result = false;
    switch (layoutProperty->GetHeightAdaptivePolicyValue(TextHeightAdaptivePolicy::MAX_LINES_FIRST)) {
        case TextHeightAdaptivePolicy::MAX_LINES_FIRST:
            if (pattern->IsInlineMode()) {
                result = AdaptInlineFocusMinFontSize(textStyle, textContent_, 1.0_fp, contentConstraint,
                    layoutWrapper);
            } else {
                result = AdaptMinFontSize(textStyle, textContent_, 1.0_fp, contentConstraint, layoutWrapper);
            }
            break;
        case TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST:
            if (pattern->IsInlineMode()) {
                result = AdaptInlineFocusFontSize(textStyle, textContent_, 1.0_fp, contentConstraint, layoutWrapper);
            } else {
                result = AdaptMinFontSize(textStyle, textContent_, 1.0_fp, contentConstraint, layoutWrapper);
            }
            break;
        case TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST:
            if (pattern->IsInlineMode()) {
                result = AdaptInlineFocusFontSize(textStyle, textContent_, 1.0_fp, contentConstraint, layoutWrapper);
            } else {
                result = AdaptMaxFontSize(textStyle, textContent_, 1.0_fp, contentConstraint, layoutWrapper);
            }
            break;
        default:
            break;
    }
    return result;
}

bool TextFieldLayoutAlgorithm::IsNeedAdaptFontSize(const TextStyle& textStyle,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint)
{
    if (!textStyle.GetAdaptTextSize()) {
        return false;
    }
    auto adaptivePolicy = layoutProperty->GetHeightAdaptivePolicyValue(TextHeightAdaptivePolicy::MAX_LINES_FIRST);
    if ((adaptivePolicy != TextHeightAdaptivePolicy::MAX_LINES_FIRST) &&
        (adaptivePolicy != TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST) &&
        (adaptivePolicy != TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST)) {
        return false;
    }
    return TextAdaptFontSizer::IsNeedAdaptFontSize(textStyle, contentConstraint);
}

bool TextFieldLayoutAlgorithm::AdaptFontSizeForLineHeight(TextStyle& textStyle,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint,
    LayoutWrapper* layoutWrapper)
{
    auto keepLineHeight = textStyle.GetLineHeight();
    textStyle.SetLineHeight(Dimension(), false);
    bool result = AddAdaptFontSizeAndAnimations(textStyle, layoutProperty, contentConstraint, layoutWrapper);
    textStyle.SetLineHeight(keepLineHeight);
    if (!result) {
        return result;
    }
    return CreateParagraphAndLayout(textStyle, textContent_, contentConstraint, layoutWrapper, false);
}

bool TextFieldLayoutAlgorithm::AdaptInlineFocusFontSize(TextStyle& textStyle, const std::string& content,
    const Dimension& stepUnit, const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    double maxFontSize = 0.0;
    double minFontSize = 0.0;
    if (!GetAdaptMaxMinFontSize(textStyle, maxFontSize, minFontSize, contentConstraint)) {
        return false;
    }
    if (LessNotEqual(maxFontSize, minFontSize) || LessOrEqual(minFontSize, 0.0)) {
        return CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper, false);
    }
    double stepSize = 0.0;
    if (!GetAdaptFontSizeStep(textStyle, stepSize, stepUnit, contentConstraint)) {
        return false;
    }
    auto tag = static_cast<int32_t>((maxFontSize - minFontSize) / stepSize);
    auto length = tag + 1 + (GreatNotEqual(maxFontSize, minFontSize + stepSize * tag) ? 1 : 0);
    int32_t left = 0;
    int32_t right = length - 1;
    float fontSize = 0.0f;
    auto newContentConstraint = BuildInfinityLayoutConstraint(contentConstraint);
    auto maxSize = GetMaxMeasureSize(contentConstraint);
    GetSuitableSize(maxSize, layoutWrapper);
    while (left <= right) {
        int32_t mid = left + (right - left) / 2;
        fontSize = static_cast<float>((mid == length - 1) ? (maxFontSize) : (minFontSize + stepSize * mid));
        textStyle.SetFontSize(Dimension(fontSize));
        if (!CreateParagraphAndLayout(textStyle, content, newContentConstraint, layoutWrapper)) {
            return false;
        }
        if (!IsInlineFocusAdaptExceedLimit(maxSize)) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    fontSize = static_cast<float>((left - 1 == length - 1) ? (maxFontSize) : (minFontSize + stepSize * (left - 1)));
    fontSize = LessNotEqual(fontSize, minFontSize) ? minFontSize : fontSize;
    fontSize = GreatNotEqual(fontSize, maxFontSize) ? maxFontSize : fontSize;
    textStyle.SetFontSize(Dimension(fontSize));
    return CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper);
}

LayoutConstraintF TextFieldLayoutAlgorithm::BuildInfinityLayoutConstraint(const LayoutConstraintF& contentConstraint)
{
    auto newContentConstraint = contentConstraint;
    newContentConstraint.maxSize = { std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity() };
    if (newContentConstraint.selfIdealSize.Width()) {
        newContentConstraint.selfIdealSize.SetWidth(std::numeric_limits<double>::infinity());
    }
    if (newContentConstraint.selfIdealSize.Height()) {
        newContentConstraint.selfIdealSize.SetHeight(std::numeric_limits<double>::infinity());
    }
    return newContentConstraint;
}

bool TextFieldLayoutAlgorithm::IsInlineFocusAdaptExceedLimit(const SizeF& maxSize)
{
    auto paragraph = GetParagraph();
    CHECK_NULL_RETURN(paragraph, false);
    bool didExceedMaxLines = false;
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph->GetHeight() / paragraph->GetLineCount(),
        maxSize.Height());
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph->GetLongestLine(), maxSize.Width());
    return didExceedMaxLines;
}

bool TextFieldLayoutAlgorithm::AdaptInlineFocusMinFontSize(TextStyle& textStyle, const std::string& content,
    const Dimension& stepUnit, const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    double maxFontSize = 0.0;
    double minFontSize = 0.0;
    if (!GetAdaptMaxMinFontSize(textStyle, maxFontSize, minFontSize, contentConstraint)) {
        return false;
    }
    if (LessNotEqual(maxFontSize, minFontSize) || LessOrEqual(minFontSize, 0.0)) {
        return CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper, false);
    }
    double stepSize = 0.0;
    if (!GetAdaptFontSizeStep(textStyle, stepSize, stepUnit, contentConstraint)) {
        return false;
    }
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(textFieldLayoutProperty, false);
    auto maxViewLines = textFieldLayoutProperty->GetMaxViewLinesValue(INLINE_DEFAULT_VIEW_MAXLINE);
    auto newContentConstraint = BuildInlineFocusLayoutConstraint(contentConstraint, layoutWrapper);
    auto maxSize = GetMaxMeasureSize(contentConstraint);
    GetSuitableSize(maxSize, layoutWrapper);
    while (GreatOrEqual(maxFontSize, minFontSize)) {
        textStyle.SetFontSize(Dimension(maxFontSize));
        if (!CreateParagraphAndLayout(textStyle, content, newContentConstraint, layoutWrapper)) {
            return false;
        }
        if (!IsInlineFocusAdaptMinExceedLimit(maxSize, maxViewLines)) {
            break;
        }
        maxFontSize -= stepSize;
    }
    return true;
}

LayoutConstraintF TextFieldLayoutAlgorithm::BuildInlineFocusLayoutConstraint(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, contentConstraint);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, contentConstraint);
    auto textFieldTheme = pattern->GetTheme();
    CHECK_NULL_RETURN(textFieldTheme, contentConstraint);
    auto safeBoundary = textFieldTheme->GetInlineBorderWidth().ConvertToPx() * 2;
    auto inlineBoundary = static_cast<float>(safeBoundary) + PARAGRAPH_SAVE_BOUNDARY;
    auto newContentConstraint = contentConstraint;
    newContentConstraint.maxSize.SetWidth(newContentConstraint.maxSize.Width() - inlineBoundary);
    if (newContentConstraint.selfIdealSize.Width()) {
        newContentConstraint.selfIdealSize.SetWidth(newContentConstraint.selfIdealSize.Width().value() -
            inlineBoundary);
    }
    return newContentConstraint;
}

bool TextFieldLayoutAlgorithm::IsInlineFocusAdaptMinExceedLimit(const SizeF& maxSize, uint32_t maxViewLines)
{
    auto paragraph = GetParagraph();
    CHECK_NULL_RETURN(paragraph, false);
    bool didExceedMaxLines = paragraph->DidExceedMaxLines();
    didExceedMaxLines = didExceedMaxLines || ((maxViewLines > 0) && (paragraph->GetLineCount() > maxViewLines));
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph->GetHeight() / paragraph->GetLineCount(),
        maxSize.Height());
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph->GetLongestLine(), maxSize.Width());
    return didExceedMaxLines;
}

bool TextFieldLayoutAlgorithm::CreateParagraphAndLayout(const TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper, bool needLayout)
{
    if (!CreateParagraphEx(textStyle, content, contentConstraint, layoutWrapper)) {
        return false;
    }
    if (needLayout) {
        CHECK_NULL_RETURN(paragraph_, false);
        auto maxSize = GetMaxMeasureSize(contentConstraint);
        ApplyIndent(maxSize.Width());
        paragraph_->Layout(maxSize.Width());
    }
    return true;
}

void TextFieldLayoutAlgorithm::UpdateTextStyleMore(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme,
    TextStyle& textStyle, bool isDisabled)
{
    if (layoutProperty->HasAdaptMinFontSize()) {
        textStyle.SetAdaptMinFontSize(layoutProperty->GetAdaptMinFontSize().value());
    }
    if (layoutProperty->HasAdaptMaxFontSize()) {
        textStyle.SetAdaptMaxFontSize(layoutProperty->GetAdaptMaxFontSize().value());
    }
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    if (pattern->IsInPasswordMode()) {
        return;
    }
    if (layoutProperty->HasTextDecoration()) {
        textStyle.SetTextDecoration(layoutProperty->GetTextDecoration().value());
    }
    if (layoutProperty->HasTextDecorationColor()) {
        if (isDisabled) {
            textStyle.SetTextDecorationColor(layoutProperty->GetTextDecorationColor().value()
                .BlendOpacity(TEXT_DECORATION_DISABLED_COLOR_ALPHA));
        } else {
            textStyle.SetTextDecorationColor(layoutProperty->GetTextDecorationColor().value());
        }
    }
    if (layoutProperty->HasTextDecorationStyle()) {
        textStyle.SetTextDecorationStyle(layoutProperty->GetTextDecorationStyle().value());
    }
    if (layoutProperty->HasLetterSpacing()) {
        textStyle.SetLetterSpacing(layoutProperty->GetLetterSpacing().value());
    }
    if (layoutProperty->HasLineHeight()) {
        textStyle.SetLineHeight(layoutProperty->GetLineHeight().value());
        textStyle.SetHalfLeading(pipeline->GetHalfLeading());
    }
    if (layoutProperty->HasFontFeature()) {
        textStyle.SetFontFeatures(layoutProperty->GetFontFeature().value());
    }
    if (layoutProperty->HasLineSpacing()) {
        textStyle.SetLineSpacing(layoutProperty->GetLineSpacing().value());
    }
}

void TextFieldLayoutAlgorithm::UpdatePlaceholderTextStyleMore(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme,
    TextStyle& placeholderTextStyle, bool isDisabled)
{
    if (layoutProperty->GetPlaceholderValue("").empty()) {
        if (layoutProperty->HasAdaptMinFontSize()) {
            placeholderTextStyle.SetAdaptMinFontSize(layoutProperty->GetAdaptMinFontSize().value());
        }
        if (layoutProperty->HasAdaptMaxFontSize()) {
            placeholderTextStyle.SetAdaptMaxFontSize(layoutProperty->GetAdaptMaxFontSize().value());
        }
    }
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    if (pattern->IsInPasswordMode()) {
        return;
    }
    if (layoutProperty->HasLineHeight()) {
        placeholderTextStyle.SetLineHeight(layoutProperty->GetLineHeight().value());
        placeholderTextStyle.SetHalfLeading(pipeline->GetHalfLeading());
    }
    if (layoutProperty->HasLineSpacing()) {
        placeholderTextStyle.SetLineSpacing(layoutProperty->GetLineSpacing().value());
    }
}

void TextFieldLayoutAlgorithm::UpdateTextFadeoutTextStyle(
    const RefPtr<FrameNode>& frameNode, const RefPtr<TextFieldTheme>& theme, TextStyle& textStyle)
{
    CHECK_NULL_VOID(frameNode);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(textFieldPattern);

    if (theme->TextFadeoutEnabled() && textFieldPattern->GetTextFadeoutCapacity()) {
        textStyle.SetTextOverflow(TextOverflow::CLIP);
    }
}

void TextFieldLayoutAlgorithm::UpdateParagraphTextFadeoutWidth(
    const LayoutConstraintF& contentConstraint, const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto textFieldTheme = pattern->GetTheme();
    CHECK_NULL_VOID(textFieldTheme);
    if (textFieldTheme->TextFadeoutEnabled() && pattern->GetTextFadeoutCapacity()) {
        paragraph_->Layout(std::numeric_limits<double>::infinity());
        paragraph_->Layout(std::ceil(paragraph_->GetLongestLine()));
    }
}

bool TextFieldLayoutAlgorithm::IsAdaptExceedLimit(const SizeF& maxSize)
{
    auto paragraph = GetParagraph();
    CHECK_NULL_RETURN(paragraph, false);
    return (paragraph->GetLineCount() > 1) || paragraph->DidExceedMaxLines() ||
        GreatNotEqual(paragraph->GetLongestLine(), maxSize.Width()) ||
        GreatNotEqual(paragraph->GetHeight(), maxSize.Height());
}
} // namespace OHOS::Ace::NG

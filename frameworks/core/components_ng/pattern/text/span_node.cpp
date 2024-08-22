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

#include "core/components_ng/pattern/text/span_node.h"

#include <cstdint>
#include <optional>
#include <string>

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/common/font_manager.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/hyperlink/hyperlink_theme.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/paragraph.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
std::string GetDeclaration(const std::optional<Color>& color, const std::optional<TextDecoration>& textDecoration,
    const std::optional<TextDecorationStyle>& textDecorationStyle)
{
    auto jsonSpanDeclaration = JsonUtil::Create(true);
    jsonSpanDeclaration->Put(
        "type", V2::ConvertWrapTextDecorationToStirng(textDecoration.value_or(TextDecoration::NONE)).c_str());
    jsonSpanDeclaration->Put("color", (color.value_or(Color::BLACK).ColorToString()).c_str());
    jsonSpanDeclaration->Put("style",
        V2::ConvertWrapTextDecorationStyleToString(textDecorationStyle.value_or(TextDecorationStyle::SOLID))
            .c_str());
    return jsonSpanDeclaration->ToString();
}
inline std::unique_ptr<JsonValue> ConvertShadowToJson(const Shadow& shadow)
{
    auto jsonShadow = JsonUtil::Create(true);
    jsonShadow->Put("radius", std::to_string(shadow.GetBlurRadius()).c_str());
    jsonShadow->Put("color", shadow.GetColor().ColorToString().c_str());
    jsonShadow->Put("offsetX", std::to_string(shadow.GetOffset().GetX()).c_str());
    jsonShadow->Put("offsetY", std::to_string(shadow.GetOffset().GetY()).c_str());
    jsonShadow->Put("type", std::to_string(static_cast<int32_t>(shadow.GetShadowType())).c_str());
    return jsonShadow;
}
std::unique_ptr<JsonValue> ConvertShadowsToJson(const std::vector<Shadow>& shadows)
{
    auto jsonShadows = JsonUtil::CreateArray(true);
    for (const auto& shadow : shadows) {
        jsonShadows->Put(ConvertShadowToJson(shadow));
    }
    return jsonShadows;
}
} // namespace

std::string SpanItem::GetFont() const
{
    auto jsonValue = JsonUtil::Create(true);
    jsonValue->Put("style", GetFontStyleInJson(fontStyle->GetItalicFontStyle()).c_str());
    jsonValue->Put("size", GetFontSizeInJson(fontStyle->GetFontSize()).c_str());
    jsonValue->Put("weight", GetFontWeightInJson(fontStyle->GetFontWeight()).c_str());
    jsonValue->Put("family", GetFontFamilyInJson(fontStyle->GetFontFamily()).c_str());
    return jsonValue->ToString();
}

void SpanItem::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    json->PutFixedAttr("content", content.c_str(), filter, FIXED_ATTR_CONTENT);
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        TextBackgroundStyle::ToJsonValue(json, backgroundStyle, filter);
        return;
    }
    if (fontStyle) {
        json->PutExtAttr("font", GetFont().c_str(), filter);
        json->PutExtAttr("fontSize", GetFontSizeInJson(fontStyle->GetFontSize()).c_str(), filter);
        json->PutExtAttr("decoration", GetDeclaration(fontStyle->GetTextDecorationColor(),
            fontStyle->GetTextDecoration(), fontStyle->GetTextDecorationStyle()).c_str(), filter);
        json->PutExtAttr("letterSpacing",
            fontStyle->GetLetterSpacing().value_or(Dimension()).ToString().c_str(), filter);
        json->PutExtAttr("textCase",
            V2::ConvertWrapTextCaseToStirng(fontStyle->GetTextCase().value_or(TextCase::NORMAL)).c_str(), filter);
        json->PutExtAttr("fontColor", fontStyle->GetForegroundColor().value_or(fontStyle->GetTextColor()
            .value_or(Color::BLACK)).ColorToString().c_str(), filter);
        json->PutExtAttr("fontStyle", GetFontStyleInJson(fontStyle->GetItalicFontStyle()).c_str(), filter);
        json->PutExtAttr("fontWeight", GetFontWeightInJson(fontStyle->GetFontWeight()).c_str(), filter);
        json->PutExtAttr("fontFamily", GetFontFamilyInJson(fontStyle->GetFontFamily()).c_str(), filter);
        json->PutExtAttr("renderingStrategy",
            GetSymbolRenderingStrategyInJson(fontStyle->GetSymbolRenderingStrategy()).c_str(), filter);
        json->PutExtAttr(
            "effectStrategy", GetSymbolEffectStrategyInJson(fontStyle->GetSymbolEffectStrategy()).c_str(), filter);
        json->Put("symbolEffect",
            GetSymbolEffectOptionsInJson(fontStyle->GetSymbolEffectOptions().value_or(SymbolEffectOptions())).c_str());

        auto shadow = fontStyle->GetTextShadow().value_or(std::vector<Shadow> { Shadow() });
        // Determines if there are multiple textShadows
        auto jsonShadow = (shadow.size() == 1) ? ConvertShadowToJson(shadow.front()) : ConvertShadowsToJson(shadow);
        json->PutExtAttr("textShadow", jsonShadow, filter);
    }
    if (textLineStyle) {
        json->PutExtAttr("lineHeight",
            textLineStyle->GetLineHeight().value_or(Dimension()).ToString().c_str(), filter);
        json->PutExtAttr("lineSpacing",
            textLineStyle->GetLineSpacing().value_or(Dimension()).ToString().c_str(), filter);
        json->PutExtAttr("baselineOffset",
            textLineStyle->GetBaselineOffset().value_or(Dimension()).ToString().c_str(), filter);
    }
    TextBackgroundStyle::ToJsonValue(json, backgroundStyle, filter);
}

RefPtr<SpanNode> SpanNode::GetOrCreateSpanNode(int32_t nodeId)
{
    auto spanNode = ElementRegister::GetInstance()->GetSpecificItemById<SpanNode>(nodeId);
    if (spanNode) {
        return spanNode;
    }
    spanNode = MakeRefPtr<SpanNode>(nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

RefPtr<SpanNode> SpanNode::CreateSpanNode(int32_t nodeId)
{
    auto spanNode = MakeRefPtr<SpanNode>(nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

RefPtr<SpanNode> SpanNode::GetOrCreateSpanNode(const std::string& tag, int32_t nodeId)
{
    auto spanNode = ElementRegister::GetInstance()->GetSpecificItemById<SpanNode>(nodeId);
    if (spanNode) {
        return spanNode;
    }
    spanNode = MakeRefPtr<SpanNode>(tag, nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

void SpanNode::MountToParagraph()
{
    auto parent = GetParent();
    while (parent) {
        auto spanNode = DynamicCast<SpanNode>(parent);
        if (spanNode) {
            spanNode->AddChildSpanItem(Claim(this));
            return;
        }
        auto textNode = DynamicCast<FrameNode>(parent);
        if (textNode) {
            auto textPattern = textNode->GetPattern<TextPattern>();
            if (textPattern) {
                textPattern->AddChildSpanItem(Claim(this));
                return;
            }
        }
        parent = parent->GetParent();
    }
}

void SpanNode::RequestTextFlushDirty()
{
    RequestTextFlushDirty(Claim<UINode>(this));
}

void SpanNode::RequestTextFlushDirty(const RefPtr<UINode>& node)
{
    CHECK_NULL_VOID(node);
    auto parent = node->GetParent();
    while (parent) {
        auto textNode = DynamicCast<FrameNode>(parent);
        if (textNode) {
            textNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            auto textPattern = textNode->GetPattern<TextPattern>();
            if (textPattern) {
                textPattern->OnModifyDone();
                return;
            }
        }
        parent = parent->GetParent();
    }
}

void SpanNode::SetTextBackgroundStyle(const TextBackgroundStyle& style)
{
    BaseSpan::SetTextBackgroundStyle(style);
    spanItem_->backgroundStyle = GetTextBackgroundStyle();
}

void SpanNode::UpdateTextBackgroundFromParent(const std::optional<TextBackgroundStyle>& style)
{
    BaseSpan::UpdateTextBackgroundFromParent(style);
    spanItem_->backgroundStyle = GetTextBackgroundStyle();
}

void SpanNode::DumpInfo()
{
    auto& dumpLog = DumpLog::GetInstance();
    dumpLog.AddDesc(std::string("Content: ").append("\"").append(spanItem_->content).append("\""));
    auto textStyle = spanItem_->GetTextStyle();
    if (!textStyle) {
        return;
    }
    dumpLog.AddDesc(std::string("FontSize: ").append(textStyle->GetFontSize().ToString()));
    dumpLog.AddDesc(std::string("LineHeight: ").append(textStyle->GetLineHeight().ToString()));
    dumpLog.AddDesc(std::string("LineSpacing: ").append(textStyle->GetLineSpacing().ToString()));
    dumpLog.AddDesc(std::string("BaselineOffset: ").append(textStyle->GetBaselineOffset().ToString()));
    dumpLog.AddDesc(std::string("WordSpacing: ").append(textStyle->GetWordSpacing().ToString()));
    dumpLog.AddDesc(std::string("TextIndent: ").append(textStyle->GetTextIndent().ToString()));
    dumpLog.AddDesc(std::string("LetterSpacing: ").append(textStyle->GetLetterSpacing().ToString()));
    dumpLog.AddDesc(std::string("TextColor: ").append(textStyle->GetTextColor().ColorToString()));
    dumpLog.AddDesc(std::string("FontWeight: ").append(StringUtils::ToString(textStyle->GetFontWeight())));
    dumpLog.AddDesc(std::string("FontStyle: ").append(StringUtils::ToString(textStyle->GetFontStyle())));
    dumpLog.AddDesc(std::string("TextBaseline: ").append(StringUtils::ToString(textStyle->GetTextBaseline())));
    dumpLog.AddDesc(std::string("TextOverflow: ").append(StringUtils::ToString(textStyle->GetTextOverflow())));
    dumpLog.AddDesc(std::string("VerticalAlign: ").append(StringUtils::ToString(textStyle->GetTextVerticalAlign())));
    dumpLog.AddDesc(std::string("TextAlign: ").append(StringUtils::ToString(textStyle->GetTextAlign())));
    dumpLog.AddDesc(std::string("WordBreak: ").append(StringUtils::ToString(textStyle->GetWordBreak())));
    dumpLog.AddDesc(std::string("TextCase: ").append(StringUtils::ToString(textStyle->GetTextCase())));
    dumpLog.AddDesc(std::string("EllipsisMode: ").append(StringUtils::ToString(textStyle->GetEllipsisMode())));
    dumpLog.AddDesc(std::string("HalfLeading: ").append(std::to_string(textStyle->GetHalfLeading())));
    if (GetTag() == V2::SYMBOL_SPAN_ETS_TAG) {
        dumpLog.AddDesc(std::string("SymbolColor:").append(spanItem_->SymbolColorToString()));
        dumpLog.AddDesc(std::string("RenderStrategy: ").append(std::to_string(textStyle->GetRenderStrategy())));
        dumpLog.AddDesc(std::string("EffectStrategy: ").append(std::to_string(textStyle->GetEffectStrategy())));
        dumpLog.AddDesc(std::string("SymbolEffect:").append(
            spanItem_->fontStyle->GetSymbolEffectOptions().value_or(NG::SymbolEffectOptions()).ToString()));
    }
}

int32_t SpanItem::UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder,
    bool isSpanStringMode, PlaceholderStyle /*placeholderStyle*/, bool isMarquee)
{
    CHECK_NULL_RETURN(builder, -1);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, -1);
    auto textStyle = InheritParentProperties(frameNode, isSpanStringMode);
    UseSelfStyle(fontStyle, textLineStyle, textStyle);
    auto fontManager = pipelineContext->GetFontManager();
    if (fontManager && !(fontManager->GetAppCustomFont().empty()) && (textStyle.GetFontFamilies().empty())) {
        textStyle.SetFontFamilies(Framework::ConvertStrToFontFamilies(fontManager->GetAppCustomFont()));
    }
    if (frameNode) {
        FontRegisterCallback(frameNode, textStyle);
    }
    if (NearZero(textStyle.GetFontSize().Value())) {
        return -1;
    }
    auto textLayoutProp = frameNode->GetLayoutProperty<TextLayoutProperty>();
    if (textLayoutProp && textLayoutProp->HasHalfLeading()) {
        textStyle.SetHalfLeading(textLayoutProp->GetHalfLeadingValue(false));
    } else {
        textStyle.SetHalfLeading(pipelineContext->GetHalfLeading());
    }

    auto spanContent = GetSpanContent(content, isMarquee);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(pattern, -1);
    textStyle.SetTextBackgroundStyle(backgroundStyle);
    if (pattern->NeedShowAIDetect() && !aiSpanMap.empty()) {
        TextStyle aiSpanStyle = textStyle;
        pattern->ModifyAISpanStyle(aiSpanStyle);
        UpdateTextStyleForAISpan(spanContent, builder, textStyle, aiSpanStyle);
    } else {
        UpdateTextStyle(spanContent, builder, textStyle, selectedStart, selectedEnd);
    }
    textStyle_ = textStyle;
    return -1;
}

void SpanItem::UpdateSymbolSpanParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder)
{
    CHECK_NULL_VOID(builder);
    std::optional<TextStyle> textStyle;
    auto symbolUnicode = GetSymbolUnicode();
    if (fontStyle || textLineStyle) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        TextStyle themeTextStyle =
            CreateTextStyleUsingTheme(fontStyle, textLineStyle, pipelineContext->GetTheme<TextTheme>());
        if (frameNode) {
            FontRegisterCallback(frameNode, themeTextStyle);
        }
        if (NearZero(themeTextStyle.GetFontSize().Value())) {
            return;
        }
        auto textLayoutProp = frameNode->GetLayoutProperty<TextLayoutProperty>();
        if (textLayoutProp && textLayoutProp->HasHalfLeading()) {
            themeTextStyle.SetHalfLeading(textLayoutProp->GetHalfLeadingValue(false));
        } else {
            themeTextStyle.SetHalfLeading(pipelineContext->GetHalfLeading());
        }
        textStyle = themeTextStyle;
        if (symbolUnicode != 0) {
            UpdateSymbolSpanColor(frameNode, themeTextStyle);
        }
        themeTextStyle.SetFontFamilies({"HM Symbol"});
        builder->PushStyle(themeTextStyle);
    }
    textStyle_ = textStyle;

    if (symbolUnicode != 0) {
        textStyle_->isSymbolGlyph_ = true;
        builder->AddSymbol(symbolUnicode);
    }

    if (fontStyle || textLineStyle) {
        builder->PopStyle();
    }
}

void SpanItem::UpdateSymbolSpanColor(const RefPtr<FrameNode>& frameNode, TextStyle& symbolSpanStyle)
{
    symbolSpanStyle.isSymbolGlyph_ = true;
    CHECK_NULL_VOID(frameNode);
    if (GetIsParentText() && symbolSpanStyle.GetSymbolColorList().empty()) {
        RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
        CHECK_NULL_VOID(layoutProperty);
        RefPtr<TextLayoutProperty> textLayoutProperty = DynamicCast<TextLayoutProperty>(layoutProperty);
        CHECK_NULL_VOID(textLayoutProperty);
        if (textLayoutProperty->GetTextColor().has_value()) {
            std::vector<Color> symbolColor;
            symbolColor.emplace_back(textLayoutProperty->GetTextColor().value());
            symbolSpanStyle.SetSymbolColorList(symbolColor);
        }
    }
}

void SpanItem::UpdateTextStyleForAISpan(const std::string& spanContent, const RefPtr<Paragraph>& builder,
    const TextStyle& textStyle, const TextStyle& aiSpanStyle)
{
    auto wSpanContent = StringUtils::ToWstring(spanContent);
    int32_t wSpanContentLength = static_cast<int32_t>(wSpanContent.length());
    int32_t spanStart = position - wSpanContentLength;
    if (needRemoveNewLine) {
        spanStart -= 1;
    }
    int32_t preEnd = spanStart;
    while (!aiSpanMap.empty()) {
        auto aiSpan = aiSpanMap.begin()->second;
        if (aiSpan.start >= position || preEnd >= position) {
            break;
        }
        int32_t aiSpanStartInSpan = std::max(spanStart, aiSpan.start);
        int32_t aiSpanEndInSpan = std::min(position, aiSpan.end);
        if (aiSpan.end <= spanStart || aiSpanStartInSpan < preEnd) {
            TAG_LOGI(AceLogTag::ACE_TEXT, "Error prediction");
            aiSpanMap.erase(aiSpanMap.begin());
            continue;
        }
        int32_t contentStart = preEnd - spanStart;
        if (preEnd < aiSpanStartInSpan) {
            auto beforeContent =
                StringUtils::ToString(wSpanContent.substr(preEnd - spanStart, aiSpanStartInSpan - preEnd));
            UpdateTextStyle(
                beforeContent, builder, textStyle, selectedStart - contentStart, selectedEnd - contentStart);
            contentStart = contentStart + aiSpanStartInSpan - preEnd;
        }
        auto displayContent = StringUtils::ToWstring(aiSpan.content)
            .substr(aiSpanStartInSpan - aiSpan.start, aiSpanEndInSpan - aiSpanStartInSpan);
        UpdateTextStyle(StringUtils::ToString(displayContent), builder, aiSpanStyle,
            selectedStart - contentStart, selectedEnd - contentStart);
        preEnd = aiSpanEndInSpan;
        if (aiSpan.end > position) {
            return;
        } else {
            aiSpanMap.erase(aiSpanMap.begin());
        }
    }
    if (preEnd < position) {
        int32_t contentStart = preEnd - spanStart;
        auto afterContent = StringUtils::ToString(wSpanContent.substr(preEnd - spanStart, position - preEnd));
        UpdateTextStyle(afterContent, builder, textStyle, selectedStart - contentStart, selectedEnd - contentStart);
    }
}

void SpanItem::FontRegisterCallback(const RefPtr<FrameNode>& frameNode, const TextStyle& textStyle)
{
    auto callback = [weakNode = WeakPtr<FrameNode>(frameNode)] {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pattern = frameNode->GetPattern<TextPattern>();
        CHECK_NULL_VOID(pattern);
        auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
        CHECK_NULL_VOID(modifier);
        modifier->SetFontReady(true);
    };
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto fontManager = pipeline->GetFontManager();
    if (fontManager) {
        bool isCustomFont = false;
        for (const auto& familyName : textStyle.GetFontFamilies()) {
            bool customFont = fontManager->RegisterCallbackNG(frameNode, familyName, callback);
            if (customFont) {
                isCustomFont = true;
            }
        }
        if (isCustomFont) {
            auto pattern = frameNode->GetPattern<TextPattern>();
            CHECK_NULL_VOID(pattern);
            pattern->SetIsCustomFont(true);
            auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
            CHECK_NULL_VOID(modifier);
            modifier->SetIsCustomFont(true);
        }
    }
}

void SpanItem::UpdateTextStyle(const std::string& content, const RefPtr<Paragraph>& builder, const TextStyle& textStyle,
    const int32_t selStart, const int32_t selEnd)
{
    if (!IsDragging()) {
        UpdateContentTextStyle(content, builder, textStyle);
    } else {
        if (content.empty()) {
            builder->PushStyle(textStyle);
            builder->PopStyle();
            return;
        }
        auto displayContent = StringUtils::Str8ToStr16(content);
        auto contentLength = static_cast<int32_t>(displayContent.length());
        if (selStart > 0) {
            auto beforeSelectedText = displayContent.substr(0, selectedStart);
            UpdateContentTextStyle(StringUtils::Str16ToStr8(beforeSelectedText), builder, textStyle);
        }
        auto finalSelStart = selStart;
        if (finalSelStart < 0) {
            finalSelStart = 0;
        }
        auto finalSelEnd = selEnd;
        if (finalSelEnd < 0) {
            finalSelEnd = 0;
        }
        if (finalSelEnd > 0 && finalSelEnd > contentLength) {
            finalSelEnd = contentLength;
        }
        if (finalSelStart < contentLength) {
            auto pipelineContext = PipelineContext::GetCurrentContext();
            TextStyle selectedTextStyle = textStyle;
            Color color = selectedTextStyle.GetTextColor().ChangeAlpha(DRAGGED_TEXT_OPACITY);
            selectedTextStyle.SetTextColor(color);
            Color textDecorationColor = selectedTextStyle.GetTextDecorationColor().ChangeAlpha(DRAGGED_TEXT_OPACITY);
            selectedTextStyle.SetTextDecorationColor(textDecorationColor);
            auto selectedText = displayContent.substr(finalSelStart, finalSelEnd - finalSelStart);
            UpdateContentTextStyle(StringUtils::Str16ToStr8(selectedText), builder, selectedTextStyle);
        }

        if (finalSelEnd < contentLength) {
            auto afterSelectedText = displayContent.substr(finalSelEnd);
            UpdateContentTextStyle(StringUtils::Str16ToStr8(afterSelectedText), builder, textStyle);
        }
    }
}

void SpanItem::UpdateContentTextStyle(
    const std::string& content, const RefPtr<Paragraph>& builder, const TextStyle& textStyle)
{
    builder->PushStyle(textStyle);
    if (!content.empty()) {
        auto displayText = content;
        auto textCase = textStyle.GetTextCase();
        StringUtils::TransformStrCase(displayText, static_cast<int32_t>(textCase));
        builder->AddText(StringUtils::Str8ToStr16(displayText));
    }
    builder->PopStyle();
}

std::string SpanItem::GetSpanContent(const std::string& rawContent, bool isMarquee)
{
    std::string data;
    if (needRemoveNewLine && !rawContent.empty()) {
        data = rawContent.substr(0, static_cast<int32_t>(rawContent.length()) - 1);
    } else {
        data = rawContent;
    }
    if (isMarquee) {
        std::replace(data.begin(), data.end(), '\n', ' ');
    }
    return data;
}

std::string SpanItem::GetSpanContent()
{
    return content;
}

uint32_t SpanItem::GetSymbolUnicode()
{
    return unicode;
}

void SpanItem::StartDrag(int32_t start, int32_t end)
{
    selectedStart = std::max(0, start);
    auto contentLen = static_cast<int32_t>(content.size());
    selectedEnd = std::min(contentLen, end);
}

void SpanItem::EndDrag()
{
    selectedStart = -1;
    selectedEnd = -1;
}

bool SpanItem::IsDragging()
{
    return selectedStart >= 0 && selectedEnd >= 0;
}

ResultObject SpanItem::GetSpanResultObject(int32_t start, int32_t end)
{
    bool selectFlag = true;
    ResultObject resultObject;
    int32_t endPosition = interval.second;
    int32_t startPosition = interval.first;
    int32_t itemLength = endPosition - startPosition;

    if (startPosition >= start && endPosition <= end) {
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition < start && endPosition <= end && endPosition > start) {
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition >= start && startPosition < end && endPosition >= end) {
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    } else if (startPosition <= start && endPosition >= end) {
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    } else {
        selectFlag = false;
    }
    if (selectFlag) {
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.type = SelectSpanType::TYPESPAN;
        resultObject.valueString = content;
        resultObject.span = WeakClaim(this);
        resultObject.isInit = true;
    }
    return resultObject;
}

#define INHERIT_TEXT_STYLE(group, name, func)                    \
    do {                                                         \
        if ((textLayoutProp)->Has##name()) {                     \
            textStyle.func(textLayoutProp->Get##name().value()); \
        }                                                        \
    } while (false)

TextStyle SpanItem::InheritParentProperties(const RefPtr<FrameNode>& frameNode, bool isSpanStringMode)
{
    TextStyle textStyle;
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, textStyle);
    auto theme = context->GetTheme<TextTheme>();
    CHECK_NULL_RETURN(theme, textStyle);
    textStyle = theme->GetTextStyle();
    CHECK_NULL_RETURN(isSpanStringMode, textStyle);
    auto textLayoutProp = frameNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProp, textStyle);
    INHERIT_TEXT_STYLE(fontStyle, FontSize, SetFontSize);
    INHERIT_TEXT_STYLE(fontStyle, TextColor, SetTextColor);
    INHERIT_TEXT_STYLE(fontStyle, ItalicFontStyle, SetFontStyle);
    INHERIT_TEXT_STYLE(fontStyle, FontWeight, SetFontWeight);
    INHERIT_TEXT_STYLE(fontStyle, FontFamily, SetFontFamilies);
    INHERIT_TEXT_STYLE(fontStyle, TextShadow, SetTextShadows);
    INHERIT_TEXT_STYLE(fontStyle, TextCase, SetTextCase);
    INHERIT_TEXT_STYLE(fontStyle, TextDecoration, SetTextDecoration);
    INHERIT_TEXT_STYLE(fontStyle, TextDecorationColor, SetTextDecorationColor);
    INHERIT_TEXT_STYLE(fontStyle, TextDecorationStyle, SetTextDecorationStyle);
    INHERIT_TEXT_STYLE(fontStyle, LetterSpacing, SetLetterSpacing);
    INHERIT_TEXT_STYLE(fontStyle, FontFeature, SetFontFeatures);
    INHERIT_TEXT_STYLE(fontStyle, MinFontScale, SetMinFontScale);
    INHERIT_TEXT_STYLE(fontStyle, MaxFontScale, SetMaxFontScale);
    INHERIT_TEXT_STYLE(textLineStyle, LineHeight, SetLineHeight);
    INHERIT_TEXT_STYLE(textLineStyle, LineSpacing, SetLineSpacing);
    INHERIT_TEXT_STYLE(textLineStyle, HalfLeading, SetHalfLeading);
    return textStyle;
}

#define COPY_TEXT_STYLE(group, name, func)                      \
    do {                                                        \
        if ((group)->Has##name()) {                             \
            sameSpan->group->func((group)->prop##name.value()); \
        }                                                       \
    } while (false)

RefPtr<SpanItem> SpanItem::GetSameStyleSpanItem() const
{
    auto sameSpan = MakeRefPtr<SpanItem>();
    COPY_TEXT_STYLE(fontStyle, FontSize, UpdateFontSize);
    COPY_TEXT_STYLE(fontStyle, TextColor, UpdateTextColor);
    COPY_TEXT_STYLE(fontStyle, TextShadow, UpdateTextShadow);
    COPY_TEXT_STYLE(fontStyle, ItalicFontStyle, UpdateItalicFontStyle);
    COPY_TEXT_STYLE(fontStyle, FontWeight, UpdateFontWeight);
    COPY_TEXT_STYLE(fontStyle, FontFamily, UpdateFontFamily);
    COPY_TEXT_STYLE(fontStyle, FontFeature, UpdateFontFeature);
    COPY_TEXT_STYLE(fontStyle, TextDecoration, UpdateTextDecoration);
    COPY_TEXT_STYLE(fontStyle, TextDecorationColor, UpdateTextDecorationColor);
    COPY_TEXT_STYLE(fontStyle, TextDecorationStyle, UpdateTextDecorationStyle);
    COPY_TEXT_STYLE(fontStyle, TextCase, UpdateTextCase);
    COPY_TEXT_STYLE(fontStyle, AdaptMinFontSize, UpdateAdaptMinFontSize);
    COPY_TEXT_STYLE(fontStyle, AdaptMaxFontSize, UpdateAdaptMaxFontSize);
    COPY_TEXT_STYLE(fontStyle, LetterSpacing, UpdateLetterSpacing);
    COPY_TEXT_STYLE(fontStyle, MinFontScale, UpdateMinFontScale);
    COPY_TEXT_STYLE(fontStyle, MaxFontScale, UpdateMaxFontScale);

    COPY_TEXT_STYLE(textLineStyle, LineHeight, UpdateLineHeight);
    COPY_TEXT_STYLE(textLineStyle, LineSpacing, UpdateLineSpacing);
    COPY_TEXT_STYLE(textLineStyle, TextBaseline, UpdateTextBaseline);
    COPY_TEXT_STYLE(textLineStyle, BaselineOffset, UpdateBaselineOffset);
    COPY_TEXT_STYLE(textLineStyle, TextOverflow, UpdateTextOverflow);
    COPY_TEXT_STYLE(textLineStyle, TextAlign, UpdateTextAlign);
    COPY_TEXT_STYLE(textLineStyle, MaxLength, UpdateMaxLength);
    COPY_TEXT_STYLE(textLineStyle, MaxLines, UpdateMaxLines);
    COPY_TEXT_STYLE(textLineStyle, HeightAdaptivePolicy, UpdateHeightAdaptivePolicy);
    COPY_TEXT_STYLE(textLineStyle, TextIndent, UpdateTextIndent);
    COPY_TEXT_STYLE(textLineStyle, LeadingMargin, UpdateLeadingMargin);
    COPY_TEXT_STYLE(textLineStyle, WordBreak, UpdateWordBreak);
    COPY_TEXT_STYLE(textLineStyle, LineBreakStrategy, UpdateLineBreakStrategy);
    COPY_TEXT_STYLE(textLineStyle, EllipsisMode, UpdateEllipsisMode);
    COPY_TEXT_STYLE(textLineStyle, HalfLeading, UpdateHalfLeading);

    if (backgroundStyle.has_value()) {
        sameSpan->backgroundStyle->backgroundColor = backgroundStyle->backgroundColor;
        sameSpan->backgroundStyle->backgroundRadius = backgroundStyle->backgroundRadius;
        sameSpan->backgroundStyle->groupId = backgroundStyle->groupId;
    }

    sameSpan->onClick = onClick;
    sameSpan->onLongPress = onLongPress;
    return sameSpan;
}


#define WRITE_TEXT_STYLE_TLV(group, name, tag, type)                   \
    do {                                                               \
        if ((group)->Has##name()) {                                    \
            TLVUtil::WriteUint8(buff, (tag));                          \
            TLVUtil::Write##type(buff, (group)->prop##name.value());   \
        }                                                              \
    } while (false)

#define READ_TEXT_STYLE_TLV(group, func, tag, type)                         \
    case tag: {                                                             \
        sameSpan->group->func(TLVUtil::Read##type(buff, cursor));           \
        break;                                                              \
    }

bool SpanItem::EncodeTlv(std::vector<uint8_t>& buff)
{
    TLVUtil::WriteUint8(buff, TLV_SPANITEM_TAG);
    TLVUtil::WriteInt32(buff, interval.first);
    TLVUtil::WriteInt32(buff, interval.second);
    TLVUtil::WriteString(buff, content);
    // encode fontStyle
    WRITE_TEXT_STYLE_TLV(fontStyle, FontSize, TLV_SPAN_FONT_STYLE_FONTSIZE, Dimension);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextColor, TLV_SPAN_FONT_STYLE_TEXTCOLOR, Color);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextShadow, TLV_SPAN_FONT_STYLE_TEXTSHADOW, TextShadows);
    WRITE_TEXT_STYLE_TLV(fontStyle, ItalicFontStyle, TLV_SPAN_FONT_STYLE_ITALICFONTSTYLE, FontStyle);
    WRITE_TEXT_STYLE_TLV(fontStyle, FontWeight, TLV_SPAN_FONT_STYLE_FONTWEIGHT, FontWeight);
    WRITE_TEXT_STYLE_TLV(fontStyle, FontFamily, TLV_SPAN_FONT_STYLE_FONTFAMILY, FontFamily);
    WRITE_TEXT_STYLE_TLV(fontStyle, FontFeature, TLV_SPAN_FONT_STYLE_FONTFEATURE, FontFeature);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextDecoration, TLV_SPAN_FONT_STYLE_TEXTDECORATION, TextDecoration);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextDecorationColor, TLV_SPAN_FONT_STYLE_TEXTDECORATIONCOLOR, Color);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextDecorationStyle, TLV_SPAN_FONT_STYLE_TEXTDECORATIONSTYLE, TextDecorationStyle);
    WRITE_TEXT_STYLE_TLV(fontStyle, TextCase, TLV_SPAN_FONT_STYLE_TEXTCASE, TextCase);
    WRITE_TEXT_STYLE_TLV(fontStyle, AdaptMinFontSize, TLV_SPAN_FONT_STYLE_ADPATMINFONTSIZE, Dimension);
    WRITE_TEXT_STYLE_TLV(fontStyle, AdaptMaxFontSize, TLV_SPAN_FONT_STYLE_ADPATMAXFONTSIZE, Dimension);
    WRITE_TEXT_STYLE_TLV(fontStyle, LetterSpacing, TLV_SPAN_FONT_STYLE_LETTERSPACING, Dimension);

    WRITE_TEXT_STYLE_TLV(textLineStyle, LineHeight, TLV_SPAN_TEXT_LINE_STYLE_LINEHEIGHT, Dimension);
    WRITE_TEXT_STYLE_TLV(textLineStyle, LineSpacing, TLV_SPAN_TEXT_LINE_STYLE_LINESPACING, Dimension);
    WRITE_TEXT_STYLE_TLV(textLineStyle, TextBaseline, TLV_SPAN_TEXT_LINE_STYLE_TEXTBASELINE, TextBaseline);
    WRITE_TEXT_STYLE_TLV(textLineStyle, BaselineOffset, TLV_SPAN_TEXT_LINE_STYLE_BASELINEOFFSET, Dimension);
    WRITE_TEXT_STYLE_TLV(textLineStyle, TextOverflow, TLV_SPAN_TEXT_LINE_STYLE_TEXTOVERFLOW, TextOverflow);
    WRITE_TEXT_STYLE_TLV(textLineStyle, TextAlign, TLV_SPAN_TEXT_LINE_STYLE_TEXTALIGN, TextAlign);
    WRITE_TEXT_STYLE_TLV(textLineStyle, MaxLength, TLV_SPAN_TEXT_LINE_STYLE_MAXLENGTH, Int32);
    WRITE_TEXT_STYLE_TLV(textLineStyle, MaxLines, TLV_SPAN_TEXT_LINE_STYLE_MAXLINES, Int32);
    WRITE_TEXT_STYLE_TLV(textLineStyle, HeightAdaptivePolicy,
        TLV_SPAN_TEXT_LINE_STYLE_HEIGHTADAPTIVEPOLICY, TextHeightAdaptivePolicy);
    WRITE_TEXT_STYLE_TLV(textLineStyle, TextIndent, TLV_SPAN_TEXT_LINE_STYLE_TEXTINDENT, Dimension);
    WRITE_TEXT_STYLE_TLV(textLineStyle, LeadingMargin, TLV_SPAN_TEXT_LINE_STYLE_LEADINGMARGIN, LeadingMargin);
    WRITE_TEXT_STYLE_TLV(textLineStyle, WordBreak, TLV_SPAN_TEXT_LINE_STYLE_WORDBREAK, WordBreak);
    WRITE_TEXT_STYLE_TLV(textLineStyle, LineBreakStrategy,
        TLV_SPAN_TEXT_LINE_STYLE_LINEBREAKSTRATEGY, LineBreakStrategy);
    WRITE_TEXT_STYLE_TLV(textLineStyle, EllipsisMode, TLV_SPAN_TEXT_LINE_STYLE_ELLIPSISMODE, EllipsisMode);
    if (backgroundStyle.has_value()) {
        if (backgroundStyle->backgroundColor.has_value()) {
            TLVUtil::WriteUint8(buff, TLV_SPAN_BACKGROUND_BACKGROUNDCOLOR);
            TLVUtil::WriteColor(buff, backgroundStyle->backgroundColor.value());
        }
        if (backgroundStyle->backgroundRadius.has_value()) {
            TLVUtil::WriteUint8(buff, TLV_SPAN_BACKGROUND_BACKGROUNDRADIUS);
            TLVUtil::WriteBorderRadiusProperty(buff, backgroundStyle->backgroundRadius.value());
        }
        TLVUtil::WriteUint8(buff, TLV_SPAN_BACKGROUND_GROUPID);
        TLVUtil::WriteInt32(buff, backgroundStyle->groupId);
    }
    TLVUtil::WriteUint8(buff, TLV_SPANITEM_END_TAG);
    return true;
};

RefPtr<SpanItem> SpanItem::DecodeTlv(std::vector<uint8_t>& buff, int32_t& cursor)
{
    auto sameSpan = MakeRefPtr<SpanItem>();
    if (TLVUtil::ReadUint8(buff, cursor) != TLV_SPANITEM_TAG) {
        return sameSpan;
    }

    int32_t start = TLVUtil::ReadInt32(buff, cursor);
    int32_t end = TLVUtil::ReadInt32(buff, cursor);
    sameSpan->interval = {start, end};
    sameSpan->content = TLVUtil::ReadString(buff, cursor);

    for (uint8_t tag = TLVUtil::ReadUint8(buff, cursor);
        tag != TLV_SPANITEM_END_TAG; tag = TLVUtil::ReadUint8(buff, cursor)) {
        switch (tag) {
            READ_TEXT_STYLE_TLV(fontStyle, UpdateFontSize, TLV_SPAN_FONT_STYLE_FONTSIZE, Dimension);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextColor, TLV_SPAN_FONT_STYLE_TEXTCOLOR, Color);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextShadow, TLV_SPAN_FONT_STYLE_TEXTSHADOW, TextShadows);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateItalicFontStyle, TLV_SPAN_FONT_STYLE_ITALICFONTSTYLE, FontStyle);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateFontWeight, TLV_SPAN_FONT_STYLE_FONTWEIGHT, FontWeight);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateFontFamily, TLV_SPAN_FONT_STYLE_FONTFAMILY, FontFamily);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateFontFeature, TLV_SPAN_FONT_STYLE_FONTFEATURE, FontFeature);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextDecoration, TLV_SPAN_FONT_STYLE_TEXTDECORATION, TextDecoration);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextDecorationColor, TLV_SPAN_FONT_STYLE_TEXTDECORATIONCOLOR, Color);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextDecorationStyle,
                TLV_SPAN_FONT_STYLE_TEXTDECORATIONSTYLE, TextDecorationStyle);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateTextCase, TLV_SPAN_FONT_STYLE_TEXTCASE, TextCase);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateAdaptMinFontSize, TLV_SPAN_FONT_STYLE_ADPATMINFONTSIZE, Dimension);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateAdaptMaxFontSize, TLV_SPAN_FONT_STYLE_ADPATMAXFONTSIZE, Dimension);
            READ_TEXT_STYLE_TLV(fontStyle, UpdateLetterSpacing, TLV_SPAN_FONT_STYLE_LETTERSPACING, Dimension);

            READ_TEXT_STYLE_TLV(textLineStyle, UpdateLineHeight, TLV_SPAN_TEXT_LINE_STYLE_LINEHEIGHT, Dimension);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateLineSpacing, TLV_SPAN_TEXT_LINE_STYLE_LINESPACING, Dimension);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateTextBaseline, TLV_SPAN_TEXT_LINE_STYLE_TEXTBASELINE, TextBaseline);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateBaselineOffset,
                TLV_SPAN_TEXT_LINE_STYLE_BASELINEOFFSET, Dimension);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateTextOverflow, TLV_SPAN_TEXT_LINE_STYLE_TEXTOVERFLOW, TextOverflow);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateTextAlign, TLV_SPAN_TEXT_LINE_STYLE_TEXTALIGN, TextAlign);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateMaxLength, TLV_SPAN_TEXT_LINE_STYLE_MAXLENGTH, Int32);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateMaxLines, TLV_SPAN_TEXT_LINE_STYLE_MAXLINES, Int32);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateHeightAdaptivePolicy,
                TLV_SPAN_TEXT_LINE_STYLE_HEIGHTADAPTIVEPOLICY, TextHeightAdaptivePolicy);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateTextIndent, TLV_SPAN_TEXT_LINE_STYLE_TEXTINDENT, Dimension);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateLeadingMargin,
                TLV_SPAN_TEXT_LINE_STYLE_LEADINGMARGIN, LeadingMargin);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateWordBreak, TLV_SPAN_TEXT_LINE_STYLE_WORDBREAK, WordBreak);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateLineBreakStrategy,
                TLV_SPAN_TEXT_LINE_STYLE_LINEBREAKSTRATEGY, LineBreakStrategy);
            READ_TEXT_STYLE_TLV(textLineStyle, UpdateEllipsisMode, TLV_SPAN_TEXT_LINE_STYLE_ELLIPSISMODE, EllipsisMode);

            case TLV_SPAN_BACKGROUND_BACKGROUNDCOLOR: {
                sameSpan->backgroundStyle->backgroundColor = TLVUtil::ReadColor(buff, cursor);
                break;
            }
            case TLV_SPAN_BACKGROUND_BACKGROUNDRADIUS: {
                sameSpan->backgroundStyle->backgroundRadius = TLVUtil::ReadBorderRadiusProperty(buff, cursor);
                break;
            }
            case TLV_SPAN_BACKGROUND_GROUPID: {
                sameSpan->backgroundStyle->groupId = TLVUtil::ReadInt32(buff, cursor);
                break;
            }
            default:
                break;
        }
    }

    return sameSpan;
}

std::string SpanItem::SymbolColorToString()
{
    auto colors = fontStyle->GetSymbolColorList();
    auto colorStr = std::string("[");
    if (colors.has_value()) {
        for (const auto& color : colors.value()) {
            colorStr.append(color.ColorToString());
            colorStr.append(",");
        }
    }
    colorStr.append("]");
    return colorStr;
}

std::optional<std::pair<int32_t, int32_t>> SpanItem::GetIntersectionInterval(std::pair<int32_t, int32_t> interval) const
{
    // Check the intersection
    if (this->interval.second <= interval.first || interval.second <= this->interval.first) {
        return std::nullopt;
    }

    // Calculate the intersection interval
    int32_t start = std::max(this->interval.first, interval.first);
    int32_t end = std::min(this->interval.second, interval.second);
    return std::make_optional<std::pair<int32_t, int32_t>>(std::make_pair(start, end));
}

bool ImageSpanItem::EncodeTlv(std::vector<uint8_t>& buff)
{
    TLVUtil::WriteUint8(buff, TLV_IMAGESPANITEM_TAG);
    TLVUtil::WriteInt32(buff, interval.first);
    TLVUtil::WriteInt32(buff, interval.second);
    TLVUtil::WriteString(buff, content);
    if (options.offset.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_OFFSET_TAG);
        TLVUtil::WriteInt32(buff, options.offset.value());
    }
    if (options.image.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_IMAGE_TAG);
        TLVUtil::WriteString(buff, options.image.value());
    }
    if (options.bundleName.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_BUNDLENAME_TAG);
        TLVUtil::WriteString(buff, options.bundleName.value());
    }
    if (options.bundleName.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_MODULENAME_TAG);
        TLVUtil::WriteString(buff, options.moduleName.value());
    }
    if (options.imagePixelMap.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_IMAGEPIXELMAP_TAG);
        TLVUtil::WritePixelMap(buff, options.imagePixelMap.value());
    }
    if (options.imageAttribute.has_value()) {
        TLVUtil::WriteUint8(buff, TLV_IMAGESPANOPTION_IMAGEATTRIBUTE_TAG);
        TLVUtil::WriteImageSpanAttribute(buff, options.imageAttribute.value());
    }
    TLVUtil::WriteUint8(buff, TLV_SPANITEM_END_TAG);
    return true;
}

RefPtr<ImageSpanItem> ImageSpanItem::DecodeTlv(std::vector<uint8_t>& buff, int32_t& cursor)
{
    auto sameSpan = MakeRefPtr<ImageSpanItem>();

    if (TLVUtil::ReadUint8(buff, cursor) != TLV_IMAGESPANITEM_TAG) {
        return sameSpan;
    }

    int32_t start = TLVUtil::ReadInt32(buff, cursor);
    int32_t end = TLVUtil::ReadInt32(buff, cursor);
    sameSpan->interval = {start, end};
    sameSpan->content = TLVUtil::ReadString(buff, cursor);

    for (uint8_t tag = TLVUtil::ReadUint8(buff, cursor);
        tag != TLV_SPANITEM_END_TAG; tag = TLVUtil::ReadUint8(buff, cursor)) {
        switch (tag) {
            case TLV_IMAGESPANOPTION_OFFSET_TAG: {
                sameSpan->options.offset = TLVUtil::ReadInt32(buff, cursor);
                break;
            }
            case TLV_IMAGESPANOPTION_IMAGE_TAG: {
                sameSpan->options.image = TLVUtil::ReadString(buff, cursor);
                break;
            }
            case TLV_IMAGESPANOPTION_BUNDLENAME_TAG: {
                sameSpan->options.bundleName = TLVUtil::ReadString(buff, cursor);
                break;
            }
            case TLV_IMAGESPANOPTION_MODULENAME_TAG: {
                sameSpan->options.moduleName = TLVUtil::ReadString(buff, cursor);
                break;
            }
            case TLV_IMAGESPANOPTION_IMAGEPIXELMAP_TAG: {
                sameSpan->options.imagePixelMap = TLVUtil::ReadPixelMap(buff, cursor);
                break;
            }
            case TLV_IMAGESPANOPTION_IMAGEATTRIBUTE_TAG: {
                sameSpan->options.imageAttribute = TLVUtil::ReadImageSpanAttribute(buff, cursor);
                break;
            }
            default:
                break;
        }
    }
    return sameSpan;
}

int32_t ImageSpanItem::UpdateParagraph(const RefPtr<FrameNode>& /* frameNode */, const RefPtr<Paragraph>& builder,
    bool /* isSpanStringMode */, PlaceholderStyle placeholderStyle, bool /* isMarquee*/)
{
    CHECK_NULL_RETURN(builder, -1);
    PlaceholderRun run;
    textStyle = TextStyle();
    run.width = placeholderStyle.width;
    run.height = placeholderStyle.height;
    if (!NearZero(placeholderStyle.baselineOffset)) {
        run.baseline_offset = placeholderStyle.baselineOffset;
        run.alignment = PlaceholderAlignment::BASELINE;
    } else {
        switch (placeholderStyle.verticalAlign) {
            case VerticalAlign::TOP:
                run.alignment = PlaceholderAlignment::TOP;
                break;
            case VerticalAlign::CENTER:
                run.alignment = PlaceholderAlignment::MIDDLE;
                break;
            case VerticalAlign::BOTTOM:
            case VerticalAlign::NONE:
                run.alignment = PlaceholderAlignment::BOTTOM;
                break;
            case VerticalAlign::BASELINE:
                run.alignment = PlaceholderAlignment::ABOVEBASELINE;
                break;
            default:
                run.alignment = PlaceholderAlignment::BOTTOM;
        }
    }

    // ImageSpan should ignore decoration styles
    textStyle.SetTextDecoration(TextDecoration::NONE);
    textStyle.SetTextBackgroundStyle(backgroundStyle);
    textStyle.SetFontSize(placeholderStyle.paragraphFontSize);
    builder->PushStyle(textStyle);
    int32_t index = builder->AddPlaceholder(run);
    run_ = run;
    builder->PopStyle();
    return index;
}

void ImageSpanItem::UpdatePlaceholderBackgroundStyle(const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(imageNode);
    auto property = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(property);
    backgroundStyle = property->GetPlaceHolderStyle();
}

void ImageSpanItem::SetImageSpanOptions(const ImageSpanOptions& options)
{
    this->options = options;
}

void ImageSpanItem::ResetImageSpanOptions()
{
    options.imageAttribute.reset();
}

RefPtr<SpanItem> ImageSpanItem::GetSameStyleSpanItem() const
{
    auto sameSpan = MakeRefPtr<ImageSpanItem>();
    sameSpan->SetImageSpanOptions(options);
    sameSpan->onClick = onClick;
    sameSpan->onLongPress = onLongPress;
    return sameSpan;
}

ResultObject ImageSpanItem::GetSpanResultObject(int32_t start, int32_t end)
{
    int32_t itemLength = 1;
    ResultObject resultObject;

    int32_t endPosition = interval.second;
    int32_t startPosition = interval.first;
    resultObject.type = SelectSpanType::TYPEIMAGE;
    if ((start <= startPosition) && (end >= endPosition)) {
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
        if (options.image.has_value()) {
            resultObject.valueString = options.image.value();
        }
        if (options.imagePixelMap.has_value()) {
            resultObject.valuePixelMap = options.imagePixelMap.value();
        }
        resultObject.isInit = true;
    }
    return resultObject;
}

ResultObject CustomSpanItem::GetSpanResultObject(int32_t start, int32_t end)
{
    int32_t itemLength = 1;
    ResultObject resultObject;

    int32_t endPosition = interval.second;
    int32_t startPosition = interval.first;
    resultObject.type = SelectSpanType::TYPEBUILDERSPAN;
    if ((start <= startPosition) && (end >= endPosition)) {
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
        resultObject.valueString = " ";
        resultObject.isInit = true;
    }
    return resultObject;
}

bool SpanItem::UpdateSpanTextColor(Color color)
{
    auto pattern = pattern_.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    auto textPattern = DynamicCast<TextPattern>(pattern);
    CHECK_NULL_RETURN(textPattern, false);
    auto paragraphManager = textPattern->GetParagraphManager();
    CHECK_NULL_RETURN(paragraphManager, false);
    auto paragraphInfos = paragraphManager->GetParagraphs();
    if (paragraphIndex != 0 || paragraphInfos.size() != 1) {
        return false;
    }
    auto iter = paragraphInfos.begin();
    auto paragraphInfo = *iter;
    auto paragraph = paragraphInfo.paragraph;
    CHECK_NULL_RETURN(paragraph, false);
    paragraph->UpdateColor(position - length, position, color);
    textPattern->MarkDirtyNodeRender();
    return true;
}

void SpanItem::GetIndex(int32_t& start, int32_t& end) const
{
    auto contentLen = StringUtils::ToWstring(content).length();
    start = position - contentLen;
    end = position;
}

int32_t PlaceholderSpanItem::UpdateParagraph(const RefPtr<FrameNode>& /* frameNode */, const RefPtr<Paragraph>& builder,
    bool /* isSpanStringMode */, PlaceholderStyle placeholderStyle, bool /* isMarquee*/)
{
    CHECK_NULL_RETURN(builder, -1);
    textStyle = TextStyle();
    PlaceholderRun run;
    run.width = placeholderStyle.width;
    run.height = placeholderStyle.height;
    textStyle.SetTextDecoration(TextDecoration::NONE);
    builder->PushStyle(textStyle);
    int32_t index = builder->AddPlaceholder(run);
    run_ = run;
    builder->PopStyle();
    return index;
}

RefPtr<SpanItem> CustomSpanItem::GetSameStyleSpanItem() const
{
    auto sameSpan = MakeRefPtr<CustomSpanItem>();
    sameSpan->onMeasure = onMeasure;
    sameSpan->onDraw = onDraw;
    sameSpan->onClick = onClick;
    sameSpan->onLongPress = onLongPress;
    return sameSpan;
}

void BaseSpan::SetTextBackgroundStyle(const TextBackgroundStyle& style)
{
    textBackgroundStyle_ = style;
    textBackgroundStyle_->groupId = groupId_;
    SetHasTextBackgroundStyle(style.backgroundColor.has_value() || style.backgroundRadius.has_value());
    MarkTextDirty();
}

void ContainerSpanNode::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    TextBackgroundStyle::ToJsonValue(json, GetTextBackgroundStyle(), filter);
}

std::set<PropertyInfo> SpanNode::CalculateInheritPropertyInfo()
{
    std::set<PropertyInfo> inheritPropertyInfo;
    static const std::set<PropertyInfo> propertyInfoContainer = { PropertyInfo::FONTSIZE, PropertyInfo::FONTCOLOR,
        PropertyInfo::FONTSTYLE, PropertyInfo::FONTWEIGHT, PropertyInfo::FONTFAMILY, PropertyInfo::TEXTDECORATION,
        PropertyInfo::TEXTCASE, PropertyInfo::LETTERSPACE, PropertyInfo::BASELINE_OFFSET, PropertyInfo::LINEHEIGHT,
        PropertyInfo::TEXT_ALIGN, PropertyInfo::LEADING_MARGIN, PropertyInfo::TEXTSHADOW, PropertyInfo::SYMBOL_COLOR,
        PropertyInfo::SYMBOL_RENDERING_STRATEGY, PropertyInfo::SYMBOL_EFFECT_STRATEGY, PropertyInfo::WORD_BREAK,
        PropertyInfo::LINE_BREAK_STRATEGY, PropertyInfo::FONTFEATURE, PropertyInfo::LINESPACING,
        PropertyInfo::SYMBOL_EFFECT_OPTIONS, PropertyInfo::HALFLEADING, PropertyInfo::MIN_FONT_SCALE,
        PropertyInfo::MAX_FONT_SCALE };
    set_difference(propertyInfoContainer.begin(), propertyInfoContainer.end(), propertyInfo_.begin(),
        propertyInfo_.end(), inserter(inheritPropertyInfo, inheritPropertyInfo.begin()));
    return inheritPropertyInfo;
}
} // namespace OHOS::Ace::NG

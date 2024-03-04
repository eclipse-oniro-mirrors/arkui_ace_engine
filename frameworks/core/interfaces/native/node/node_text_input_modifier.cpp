/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "core/interfaces/native/node/node_text_input_modifier.h"

#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/interfaces/arkoala/arkoala_api.h"
#include "core/pipeline/base/element_register.h"
#include "bridge/common/utils/utils.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/interfaces/native/node/node_api.h"
#include "core/components_ng/pattern/text_field/text_field_event_hub.h"

namespace OHOS::Ace::NG {
namespace {
const uint32_t MAX_LINES = 3;
constexpr uint32_t DEFAULT_CARET_COLOR = 0xFF007DFF;
constexpr uint32_t DEFAULT_CARE_POSITION = 0;
constexpr CopyOptions DEFAULT_TEXT_INPUT_COPY_OPTION = CopyOptions::Local;
constexpr bool DEFAULT_SHOW_PASSWORD_ICON_VALUE = true;
constexpr TextAlign DEFAULT_TEXT_ALIGN_VALUE = TextAlign::START;
constexpr InputStyle DEFAULT_INPUT_STYLE = InputStyle::DEFAULT;
constexpr bool DEFAULT_SELECTION_MENU_HIDDEN = false;
constexpr bool DEFAULT_SHOW_UNDER_LINE = false;
constexpr bool DEFAULT_REQUEST_KEYBOARD_ON_FOCUS = true;
constexpr DisplayMode DEFAULT_BAR_STATE = DisplayMode::AUTO;
constexpr FontWeight DEFAULT_FONT_WEIGHT = FontWeight::NORMAL;
constexpr Ace::FontStyle DEFAULT_FONT_STYLE = Ace::FontStyle::NORMAL;
constexpr int16_t DEFAULT_ALPHA = 255;
constexpr double DEFAULT_OPACITY = 0.2;
const std::vector<std::string> DEFAULT_FONT_FAMILY = { "HarmonyOS Sans" };
const std::vector<TextAlign> TEXT_ALIGNS = { TextAlign::START, TextAlign::CENTER, TextAlign::END, TextAlign::JUSTIFY };
const uint32_t ERROR_UINT_CODE = -1;
const float ERROR_FLOAT_CODE = -1.0f;
const int32_t ERROR_INT_CODE = -1;
std::string g_strValue;

void SetTextInputCaretColor(ArkUINodeHandle node, ArkUI_Uint32 color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretColor(frameNode, Color(color));
}

void ResetTextInputCaretColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretColor(frameNode, Color(DEFAULT_CARET_COLOR));
}

void SetTextInputType(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetType(frameNode, static_cast<TextInputType>(value));
}

void ResetTextInputType(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetType(frameNode, TextInputType::UNSPECIFIED);
    return;
}

void SetTextInputMaxLines(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    if (value <= 0) {
        TextFieldModelNG::SetMaxViewLines(frameNode, MAX_LINES);
        return;
    }

    TextFieldModelNG::SetMaxViewLines(frameNode, value);
}

void ResetTextInputMaxLines(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, MAX_LINES);
}

void SetTextInputPlaceholderColor(ArkUINodeHandle node, ArkUI_Uint32 color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetPlaceholderColor(frameNode, Color(color));
}

void ResetTextInputPlaceholderColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetPlaceholderColor(frameNode, theme->GetPlaceholderColor());
}

void SetTextInputCaretPosition(ArkUINodeHandle node, ArkUI_Int32 caretPosition)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretPosition(frameNode, caretPosition);
}

void ResetTextInputCaretPosition(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretPosition(frameNode, DEFAULT_CARE_POSITION);
}

void SetTextInputCopyOption(ArkUINodeHandle node, ArkUI_Int32 copyOption)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, static_cast<CopyOptions>(copyOption));
}

void ResetTextInputCopyOption(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, DEFAULT_TEXT_INPUT_COPY_OPTION);
}

void SetTextInputShowPasswordIcon(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowPasswordIcon(frameNode, static_cast<bool>(value));
}

void ResetTextInputShowPasswordIcon(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowPasswordIcon(frameNode, DEFAULT_SHOW_PASSWORD_ICON_VALUE);
}

void SetTextInputPasswordIcon(ArkUINodeHandle node, const struct ArkUIPasswordIconType* value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    PasswordIcon passwordIcon;
    if (value->showResult != nullptr && std::string(value->showResult) != "") {
        passwordIcon.showResult = value->showResult;
    } else {
        if (value->showBundleName != nullptr && std::string(value->showBundleName) != "") {
            passwordIcon.showBundleName = value->showBundleName;
        }
        if (value->showModuleName != nullptr && std::string(value->showModuleName) != "") {
            passwordIcon.showModuleName = value->showModuleName;
        }
    }
    if (value->hideResult != nullptr && std::string(value->hideResult) != "") {
        passwordIcon.hideResult = value->hideResult;
    } else {
        if (value->hideBundleName != nullptr && std::string(value->hideBundleName) != "") {
            passwordIcon.hideBundleName = value->hideBundleName;
        }
        if (value->hideModuleName != nullptr && std::string(value->hideModuleName) != "") {
            passwordIcon.hideModuleName = value->hideModuleName;
        }
    }
    TextFieldModelNG::SetPasswordIcon(frameNode, passwordIcon);
}

void ResetTextInputPasswordIcon(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    PasswordIcon passwordIcon;
    passwordIcon.showResult = "";
    passwordIcon.hideResult = "";
    TextFieldModelNG::SetPasswordIcon(frameNode, passwordIcon);
}

void SetTextInputTextAlign(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    if (value >= 0 && value < static_cast<int32_t>(TEXT_ALIGNS.size())) {
        TextFieldModelNG::SetTextAlign(frameNode, TEXT_ALIGNS[value]);
    } else {
        TextFieldModelNG::SetTextAlign(frameNode, DEFAULT_TEXT_ALIGN_VALUE);
    }
}

void ResetTextInputTextAlign(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetTextAlign(frameNode, DEFAULT_TEXT_ALIGN_VALUE);
}

void SetTextInputStyle(ArkUINodeHandle node, ArkUI_Int32 style)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, static_cast<InputStyle>(style));
}

void ResetTextInputStyle(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, DEFAULT_INPUT_STYLE);
}

void SetTextInputSelectionMenuHidden(ArkUINodeHandle node, ArkUI_Uint32 menuHiddenValue)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, static_cast<bool>(menuHiddenValue));
}

void ResetTextInputSelectionMenuHidden(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, DEFAULT_SELECTION_MENU_HIDDEN);
}

void SetTextInputShowUnderline(ArkUINodeHandle node, ArkUI_Uint32 showUnderLine)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowUnderline(frameNode, static_cast<bool>(showUnderLine));
}

void ResetTextInputShowUnderline(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowUnderline(frameNode, DEFAULT_SHOW_UNDER_LINE);
}

void SetTextInputCaretStyle(ArkUINodeHandle node, const ArkUILengthType* value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width;
    if (value->string != nullptr) {
        width.SetCalcValue(value->string);
    } else {
        width.SetValue(value->number);
    }
    width.SetUnit(static_cast<DimensionUnit>(value->unit));

    CaretStyle caretStyle;
    caretStyle.caretWidth = width;
    TextFieldModelNG::SetCaretStyle(frameNode, caretStyle);
}

void ResetTextInputCaretStyle(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);

    CaretStyle caretStyle;
    caretStyle.caretWidth = theme->GetCursorWidth();
    TextFieldModelNG::SetCaretStyle(frameNode, caretStyle);
}

void SetTextInputEnableKeyboardOnFocus(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, static_cast<bool>(value));
}

void ResetTextInputEnableKeyboardOnFocus(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, DEFAULT_REQUEST_KEYBOARD_ON_FOCUS);
}

void SetTextInputBarState(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetBarState(frameNode, static_cast<DisplayMode>(value));
}

void ResetTextInputBarState(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetBarState(frameNode, DEFAULT_BAR_STATE);
}

void SetTextInputEnterKeyType(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetEnterKeyType(frameNode, static_cast<TextInputAction>(value));
}

void ResetTextInputEnterKeyType(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetEnterKeyType(frameNode, TextInputAction::DONE);
}

void SetTextInputFontWeightStr(ArkUINodeHandle node, ArkUI_CharPtr fontWeight)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, Framework::ConvertStrToFontWeight(fontWeight));
}

void SetTextInputFontWeight(ArkUINodeHandle node, ArkUI_Int32 fontWeight)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, static_cast<FontWeight>(fontWeight));
}

void ResetTextInputFontWeight(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, FontWeight::NORMAL);
}

void SetTextInputFontSize(ArkUINodeHandle node, const struct ArkUILengthType *value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension fontSize;
    if (value->string != nullptr) {
        fontSize.SetCalcValue(value->string);
    } else {
        fontSize.SetValue(value->number);
    }
    fontSize.SetUnit(static_cast<DimensionUnit>(value->unit));
    TextFieldModelNG::SetFontSize(frameNode, fontSize);
}

void ResetTextInputFontSize(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetFontSize(frameNode, theme->GetFontSize());
}

void SetTextInputMaxLength(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxLength(frameNode, value);
}

void ResetTextInputMaxLength(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::ResetMaxLength(frameNode);
}

void SetTextInputSelectedBackgroundColor(ArkUINodeHandle node, ArkUI_Uint32 color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectedBackgroundColor(frameNode, Color(color));
}

void ResetTextInputSelectedBackgroundColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Color selectedColor;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    selectedColor = theme->GetSelectedColor();
    if (selectedColor.GetAlpha() == DEFAULT_ALPHA) {
        // Default setting of 20% opacity
        selectedColor = selectedColor.ChangeOpacity(DEFAULT_OPACITY);
    }
    TextFieldModelNG::SetSelectedBackgroundColor(frameNode, selectedColor);
}

void SetTextInputShowError(ArkUINodeHandle node, ArkUI_CharPtr error, ArkUI_Uint32 visible)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowError(frameNode, std::string(error), static_cast<bool>(visible));
}

void ResetTextInputShowError(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowError(frameNode, std::string(""), false);
}

void SetTextInputPlaceholderFont(ArkUINodeHandle node, const struct ArkUIPlaceholderFontType* placeholderFont)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;
    CalcDimension fontSize;
    if (placeholderFont->size != nullptr) {
        if (placeholderFont->size->string != nullptr) {
            fontSize.SetCalcValue(placeholderFont->size->string);
        } else {
            fontSize.SetValue(placeholderFont->size->number);
        }
    }
    fontSize.SetUnit(static_cast<DimensionUnit>(placeholderFont->size->unit));
    font.fontSize = fontSize;
    if (placeholderFont->weight != nullptr && std::string(placeholderFont->weight) != "") {
        font.fontWeight = Framework::ConvertStrToFontWeight(placeholderFont->weight);
    } else if (placeholderFont->weightEnum > -1) {
        font.fontWeight = static_cast<FontWeight>(placeholderFont->weightEnum);
    }
    if (placeholderFont->fontFamilies != nullptr && placeholderFont->length > 0) {
        for (uint32_t i = 0; i < placeholderFont->length; i++) {
            const char* family = *(placeholderFont->fontFamilies + i);
            if (family != nullptr) {
                font.fontFamilies.emplace_back(std::string(family));
            }
        }
    }
    if (placeholderFont->style >= 0) {
        font.fontStyle = static_cast<Ace::FontStyle>(placeholderFont->style);
    }
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}

void ResetTextInputPlaceholderFont(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    font.fontSize = theme->GetFontSize();
    font.fontWeight = DEFAULT_FONT_WEIGHT;
    font.fontStyle = DEFAULT_FONT_STYLE;
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}

void SetTextInputFontColor(ArkUINodeHandle node, ArkUI_Uint32 color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetTextColor(frameNode, Color(color));
}

void ResetTextInputFontColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetTextColor(frameNode, theme->GetTextColor());
}

void SetTextInputFontStyle(ArkUINodeHandle node, ArkUI_Uint32 value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, static_cast<Ace::FontStyle>(value));
}

void ResetTextInputFontStyle(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, DEFAULT_FONT_STYLE);
}

void SetTextInputFontFamily(ArkUINodeHandle node, ArkUI_CharPtr* fontFamilies, ArkUI_Uint32 length)
{
    CHECK_NULL_VOID(fontFamilies);
    if (length <= 0) {
        return;
    }
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::vector<std::string> families;
    for (uint32_t i = 0; i < length; i++) {
        const char* family = *(fontFamilies + i);
        if (family != nullptr) {
            families.emplace_back(std::string(family));
        }
    }
    TextFieldModelNG::SetFontFamily(frameNode, families);
}

void ResetTextInputFontFamily(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontFamily(frameNode, DEFAULT_FONT_FAMILY);
}

void SetTextInputPlaceholderString(ArkUINodeHandle node, ArkUI_CharPtr value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::string placeholderStr(value);
    TextFieldModelNG::SetTextFieldPlaceHolder(frameNode, placeholderStr);
}

void SetTextInputTextString(ArkUINodeHandle node, ArkUI_CharPtr value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::string textStr(value);
    TextFieldModelNG::SetTextFieldText(frameNode, textStr);
}

void StopTextInputTextEditing(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::StopTextFieldEditing(frameNode);
}

void SetTextInputCancelButton(ArkUINodeHandle node, ArkUI_Int32 style, const struct ArkUISizeType* size,
    ArkUI_Uint32 color, ArkUI_CharPtr src)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCleanNodeStyle(frameNode, static_cast<CleanNodeStyle>(style));
    TextFieldModelNG::SetIsShowCancelButton(frameNode, true);
    // set icon size
    CalcDimension iconSize = CalcDimension(size->value, static_cast<DimensionUnit>(size->unit));
    if (LessNotEqual(iconSize.Value(), 0.0)) {
        auto pipeline = PipelineBase::GetCurrentContextSafely();
        CHECK_NULL_VOID(pipeline);
        auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
        iconSize = theme->GetIconSize();
    }
    TextFieldModelNG::SetCancelIconSize(frameNode, iconSize);
    // set icon src
    std::string iconSrc(src);
    TextFieldModelNG::SetCanacelIconSrc(frameNode, iconSrc);
    // set icon color
    Color iconColor(color);
    TextFieldModelNG::SetCancelIconColor(frameNode, iconColor);
}

void ResetTextInputCancelButton(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCleanNodeStyle(frameNode, CleanNodeStyle::INPUT);
    TextFieldModelNG::SetIsShowCancelButton(frameNode, false);
}

ArkUI_CharPtr GetTextInputPlaceholder(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, "");
    g_strValue = TextFieldModelNG::GetPlaceholderText(frameNode);
    return g_strValue.c_str();
}

ArkUI_CharPtr GetTextInputText(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, "");
    g_strValue = TextFieldModelNG::GetTextFieldText(frameNode);
    return g_strValue.c_str();
}

ArkUI_Uint32 GetTextInputCaretColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return TextFieldModelNG::GetCaretColor(frameNode).GetValue();
}

ArkUI_Float32 GetTextInputCaretStyle(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_FLOAT_CODE);
    return TextFieldModelNG::GetCaretStyle(frameNode).Value();
}

ArkUI_Bool GetTextInputShowUnderline(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Bool>(TextFieldModelNG::GetShowUnderline(frameNode));
}

ArkUI_Uint32 GetTextInputMaxLength(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return TextFieldModelNG::GetMaxLength(frameNode);
}

ArkUI_Int32 GetTextInputEnterKeyType(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Int32>(TextFieldModelNG::GetEnterKeyType(frameNode));
}

ArkUI_Uint32 GetTextInputPlaceholderColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return TextFieldModelNG::GetPlaceholderColor(frameNode).GetValue();
}

void GetTextInputPlaceholderFont(ArkUINodeHandle node, ArkUITextFont* font)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Font value = TextFieldModelNG::GetPlaceholderFont(frameNode);
    if (value.fontSize.has_value()) {
        font->fontSize = value.fontSize.value().Value();
    }
    if (value.fontWeight.has_value()) {
        font->fontWeight = static_cast<ArkUI_Int32>(value.fontWeight.value());
    }
    if (!value.fontFamilies.empty()) {
        std::string families;
        int index = 0;
        for (auto& family : value.fontFamilies) {
            families += family;
            if (index != value.fontFamilies.size() - 1) {
                families += ",";
            }
            index ++;
        }
        g_strValue = families;
        font->fontFamilies = g_strValue.c_str();
    }
    if (value.fontStyle.has_value()) {
        font->fontStyle = static_cast<ArkUI_Int32>(value.fontStyle.value());
    }
}

ArkUI_Bool GetTextInputRequestKeyboardOnFocus(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Bool>(TextFieldModelNG::GetRequestKeyboardOnFocus(frameNode));
}

ArkUI_Int32 GetTextInputType(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Int32>(TextFieldModelNG::GetType(frameNode));
}

ArkUI_Uint32 GetTextInputSelectedBackgroundColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return TextFieldModelNG::GetSelectedBackgroundColor(frameNode).GetValue();
}

ArkUI_Bool GetTextInputShowPasswordIcon(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return TextFieldModelNG::GetShowPasswordIcon(frameNode);
}

ArkUI_Bool GetTextInputEditing(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return TextFieldModelNG::GetTextFieldEditing(frameNode);
}

ArkUI_Bool GetTextInputShowCancelButton(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return TextFieldModelNG::GetShowCancelButton(frameNode);
}

ArkUI_Float32 GetTextInputCancelIconSize(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_FLOAT_CODE);
    return TextFieldModelNG::GetCancelIconSize(frameNode).Value();
}

ArkUI_CharPtr GetTextInputTextCanacelIconSrc(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, "");
    g_strValue = TextFieldModelNG::GetCanacelIconSrc(frameNode);
    return g_strValue.c_str();
}

ArkUI_Uint32 GetTextInputTextCanacelIconColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return TextFieldModelNG::GetCancelIconColor(frameNode).GetValue();
}

} // namespace

namespace NodeModifier {
const ArkUITextInputModifier* GetTextInputModifier()
{
    static const ArkUITextInputModifier modifier = {
        SetTextInputCaretColor,
        ResetTextInputCaretColor,
        SetTextInputType,
        ResetTextInputType,
        SetTextInputMaxLines,
        ResetTextInputMaxLines,
        SetTextInputPlaceholderColor,
        ResetTextInputPlaceholderColor,
        SetTextInputCaretPosition,
        ResetTextInputCaretPosition,
        SetTextInputCopyOption,
        ResetTextInputCopyOption,
        SetTextInputShowPasswordIcon,
        ResetTextInputShowPasswordIcon,
        SetTextInputPasswordIcon,
        ResetTextInputPasswordIcon,
        SetTextInputTextAlign,
        ResetTextInputTextAlign,
        SetTextInputStyle,
        ResetTextInputStyle,
        SetTextInputSelectionMenuHidden,
        ResetTextInputSelectionMenuHidden,
        SetTextInputShowUnderline,
        ResetTextInputShowUnderline,
        SetTextInputCaretStyle,
        ResetTextInputCaretStyle,
        SetTextInputEnableKeyboardOnFocus,
        ResetTextInputEnableKeyboardOnFocus,
        SetTextInputBarState,
        ResetTextInputBarState,
        SetTextInputEnterKeyType,
        ResetTextInputEnterKeyType,
        SetTextInputFontWeight,
        ResetTextInputFontWeight,
        SetTextInputFontSize,
        ResetTextInputFontSize,
        SetTextInputMaxLength,
        ResetTextInputMaxLength,
        SetTextInputSelectedBackgroundColor,
        ResetTextInputSelectedBackgroundColor,
        SetTextInputShowError,
        ResetTextInputShowError,
        SetTextInputPlaceholderFont,
        ResetTextInputPlaceholderFont,
        SetTextInputFontColor,
        ResetTextInputFontColor,
        SetTextInputFontStyle,
        ResetTextInputFontStyle,
        SetTextInputFontFamily,
        ResetTextInputFontFamily,
        SetTextInputPlaceholderString,
        SetTextInputTextString,
        SetTextInputFontWeightStr,
        StopTextInputTextEditing,
        SetTextInputCancelButton,
        ResetTextInputCancelButton,
        GetTextInputPlaceholder,
        GetTextInputText,
        GetTextInputCaretColor,
        GetTextInputCaretStyle,
        GetTextInputShowUnderline,
        GetTextInputMaxLength,
        GetTextInputEnterKeyType,
        GetTextInputPlaceholderColor,
        GetTextInputPlaceholderFont,
        GetTextInputRequestKeyboardOnFocus,
        GetTextInputType,
        GetTextInputSelectedBackgroundColor,
        GetTextInputShowPasswordIcon,
        GetTextInputEditing,
        GetTextInputShowCancelButton,
        GetTextInputCancelIconSize,
        GetTextInputTextCanacelIconSrc,
        GetTextInputTextCanacelIconColor,
    };
    return &modifier;
}

void SetOnTextInputChange(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onChange = [node, eventId, extraParam](const std::string& str) {
        ArkUINodeEvent event;
        event.kind = ON_TEXT_INPUT_CHANGE;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.stringAsyncEvent.pStr = str.c_str();
        SendArkUIAsyncEvent(&event);
    };
    TextFieldModelNG::SetOnChange(frameNode, std::move(onChange));
}

void SetTextInputOnSubmit(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onEvent = [node, eventId, extraParam](int32_t value, NG::TextFieldCommonEvent& commonEvent) {
        ArkUINodeEvent event;
        event.kind = ON_TEXT_INPUT_SUBMIT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.data[0].i32 = value;
        SendArkUIAsyncEvent(&event);
    };
    TextFieldModelNG::SetOnSubmit(frameNode, std::move(onEvent));
}

void SetOnTextInputCut(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onCut = [node, eventId, extraParam](const std::string& str) {
        ArkUINodeEvent event;
        event.kind = ON_TEXT_INPUT_CUT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.stringAsyncEvent.pStr = str.c_str();
        SendArkUIAsyncEvent(&event);
    };
    TextFieldModelNG::SetOnCut(frameNode, std::move(onCut));
}

void SetOnTextInputPaste(ArkUINodeHandle node, ArkUI_Int32 eventId, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onPaste = [node, eventId, extraParam](const std::string& str, NG::TextCommonEvent& commonEvent) {
        ArkUINodeEvent event;
        event.kind = ON_TEXT_INPUT_PASTE;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.stringAsyncEvent.pStr = str.c_str();
        SendArkUIAsyncEvent(&event);
    };
    TextFieldModelNG::SetOnPasteWithEvent(frameNode, std::move(onPaste));
}

} // namespace NodeModifier
} // namespace OHOS::Ace::NG
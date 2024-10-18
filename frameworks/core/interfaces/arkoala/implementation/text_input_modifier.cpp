/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace TextInputInterfaceModifier {
void SetTextInputOptionsImpl(Ark_NativePointer node,
                             const Opt_TextInputOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TextInputModelNG::SetSetTextInputOptions(frameNode, convValue);
}
} // TextInputInterfaceModifier
namespace TextInputAttributeModifier {
void TypeImpl(Ark_NativePointer node,
              Ark_InputType value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetType(frameNode, convValue);
}
void ContentTypeImpl(Ark_NativePointer node,
                     Ark_ContentType value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetContentType(frameNode, convValue);
}
void PlaceholderColorImpl(Ark_NativePointer node,
                          const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetPlaceholderColor(frameNode, convValue);
}
void TextOverflowImpl(Ark_NativePointer node,
                      Ark_TextOverflow value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetTextOverflow(frameNode, convValue);
}
void TextIndentImpl(Ark_NativePointer node,
                    const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetTextIndent(frameNode, convValue);
}
void PlaceholderFontImpl(Ark_NativePointer node,
                         const Opt_Font* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TextInputModelNG::SetPlaceholderFont(frameNode, convValue);
}
void EnterKeyTypeImpl(Ark_NativePointer node,
                      Ark_EnterKeyType value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetEnterKeyType(frameNode, convValue);
}
void CaretColorImpl(Ark_NativePointer node,
                    const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetCaretColor(frameNode, convValue);
}
void OnEditChangedImpl(Ark_NativePointer node,
                       Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnEditChanged(frameNode, convValue);
}
void OnEditChangeImpl(Ark_NativePointer node,
                      Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnEditChange(frameNode, convValue);
}
void OnSubmitImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnSubmit(frameNode, convValue);
}
void OnChangeImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnChange(frameNode, convValue);
}
void OnTextSelectionChangeImpl(Ark_NativePointer node,
                               Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnTextSelectionChange(frameNode, convValue);
}
void OnContentScrollImpl(Ark_NativePointer node,
                         Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnContentScroll(frameNode, convValue);
}
void MaxLengthImpl(Ark_NativePointer node,
                   const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetMaxLength(frameNode, convValue);
}
void FontColorImpl(Ark_NativePointer node,
                   const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetFontColor(frameNode, convValue);
}
void FontSizeImpl(Ark_NativePointer node,
                  const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetFontSize(frameNode, convValue);
}
void FontStyleImpl(Ark_NativePointer node,
                   Ark_FontStyle value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetFontStyle(frameNode, convValue);
}
void FontWeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_FontWeight_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetFontWeight(frameNode, convValue);
}
void FontFamilyImpl(Ark_NativePointer node,
                    const Ark_ResourceStr* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetFontFamily(frameNode, convValue);
}
void InputFilterImpl(Ark_NativePointer node,
                     const Ark_ResourceStr* value,
                     const Opt_Function* error)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetInputFilter(frameNode, convValue);
}
void OnCopyImpl(Ark_NativePointer node,
                Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnCopy(frameNode, convValue);
}
void OnCutImpl(Ark_NativePointer node,
               Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnCut(frameNode, convValue);
}
void OnPasteImpl(Ark_NativePointer node,
                 Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnPaste(frameNode, convValue);
}
void CopyOptionImpl(Ark_NativePointer node,
                    Ark_CopyOptions value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetCopyOption(frameNode, convValue);
}
void ShowPasswordIconImpl(Ark_NativePointer node,
                          Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetShowPasswordIcon(frameNode, convValue);
}
void TextAlignImpl(Ark_NativePointer node,
                   Ark_TextAlign value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetTextAlign(frameNode, convValue);
}
void StyleImpl(Ark_NativePointer node,
               const Ark_Union_TextInputStyle_TextContentStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetStyle(frameNode, convValue);
}
void CaretStyleImpl(Ark_NativePointer node,
                    const Ark_CaretStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetCaretStyle(frameNode, convValue);
}
void SelectedBackgroundColorImpl(Ark_NativePointer node,
                                 const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetSelectedBackgroundColor(frameNode, convValue);
}
void CaretPositionImpl(Ark_NativePointer node,
                       const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetCaretPosition(frameNode, convValue);
}
void EnableKeyboardOnFocusImpl(Ark_NativePointer node,
                               Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetEnableKeyboardOnFocus(frameNode, convValue);
}
void PasswordIconImpl(Ark_NativePointer node,
                      const Ark_PasswordIcon* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetPasswordIcon(frameNode, convValue);
}
void ShowErrorImpl(Ark_NativePointer node,
                   const Opt_Union_ResourceStr_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TextInputModelNG::SetShowError(frameNode, convValue);
}
void ShowUnitImpl(Ark_NativePointer node,
                  const Ark_CustomBuilder* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetShowUnit(frameNode, convValue);
}
void ShowUnderlineImpl(Ark_NativePointer node,
                       Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetShowUnderline(frameNode, convValue);
}
void UnderlineColorImpl(Ark_NativePointer node,
                        const Ark_Union_ResourceColor_UnderlineColor_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetUnderlineColor(frameNode, convValue);
}
void SelectionMenuHiddenImpl(Ark_NativePointer node,
                             Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetSelectionMenuHidden(frameNode, convValue);
}
void BarStateImpl(Ark_NativePointer node,
                  Ark_BarState value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetBarState(frameNode, convValue);
}
void MaxLinesImpl(Ark_NativePointer node,
                  const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetMaxLines(frameNode, convValue);
}
void WordBreakImpl(Ark_NativePointer node,
                   Ark_WordBreak value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetWordBreak(frameNode, convValue);
}
void LineBreakStrategyImpl(Ark_NativePointer node,
                           Ark_LineBreakStrategy strategy)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(strategy);
    //auto convValue = Converter::OptConvert<type>(strategy); // for enums
    //TextInputModelNG::SetLineBreakStrategy(frameNode, convValue);
}
void CustomKeyboardImpl(Ark_NativePointer node,
                        const Ark_CustomBuilder* value,
                        const Opt_KeyboardOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetCustomKeyboard(frameNode, convValue);
}
void ShowCounterImpl(Ark_NativePointer node,
                     Ark_Boolean value,
                     const Opt_InputCounterOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetShowCounter(frameNode, convValue);
}
void CancelButtonImpl(Ark_NativePointer node,
                      const Ark_Literal_CancelButtonStyle_style_IconOptions_icon* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetCancelButton(frameNode, convValue);
}
void SelectAllImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetSelectAll(frameNode, convValue);
}
void MinFontSizeImpl(Ark_NativePointer node,
                     const Ark_Union_Number_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetMinFontSize(frameNode, convValue);
}
void MaxFontSizeImpl(Ark_NativePointer node,
                     const Ark_Union_Number_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetMaxFontSize(frameNode, convValue);
}
void HeightAdaptivePolicyImpl(Ark_NativePointer node,
                              Ark_TextHeightAdaptivePolicy value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TextInputModelNG::SetHeightAdaptivePolicy(frameNode, convValue);
}
void EnableAutoFillImpl(Ark_NativePointer node,
                        Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TextInputModelNG::SetEnableAutoFill(frameNode, convValue);
}
void DecorationImpl(Ark_NativePointer node,
                    const Ark_TextDecorationOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetDecoration(frameNode, convValue);
}
void LetterSpacingImpl(Ark_NativePointer node,
                       const Ark_Union_Number_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetLetterSpacing(frameNode, convValue);
}
void LineHeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TextInputModelNG::SetLineHeight(frameNode, convValue);
}
void PasswordRulesImpl(Ark_NativePointer node,
                       const Ark_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    [[maybe_unused]]
    auto convValue = Converter::Convert<std::string>(*value);
    //TextInputModelNG::SetPasswordRules(frameNode, convValue);
}
void FontFeatureImpl(Ark_NativePointer node,
                     const Ark_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    [[maybe_unused]]
    auto convValue = Converter::Convert<std::string>(*value);
    //TextInputModelNG::SetFontFeature(frameNode, convValue);
}
void ShowPasswordImpl(Ark_NativePointer node,
                      Ark_Boolean visible)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(visible);
    //TextInputModelNG::SetShowPassword(frameNode, convValue);
}
void OnSecurityStateChangeImpl(Ark_NativePointer node,
                               Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnSecurityStateChange(frameNode, convValue);
}
void OnWillInsertImpl(Ark_NativePointer node,
                      Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnWillInsert(frameNode, convValue);
}
void OnDidInsertImpl(Ark_NativePointer node,
                     Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnDidInsert(frameNode, convValue);
}
void OnWillDeleteImpl(Ark_NativePointer node,
                      Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnWillDelete(frameNode, convValue);
}
void OnDidDeleteImpl(Ark_NativePointer node,
                     Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TextInputModelNG::SetOnDidDelete(frameNode, convValue);
}
void EditMenuOptionsImpl(Ark_NativePointer node,
                         const Ark_Materialized* editMenu)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(editMenu);
    //auto convValue = Converter::OptConvert<type_name>(*editMenu);
    //TextInputModelNG::SetEditMenuOptions(frameNode, convValue);
}
void EnablePreviewTextImpl(Ark_NativePointer node,
                           Ark_Boolean enable)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(enable);
    //TextInputModelNG::SetEnablePreviewText(frameNode, convValue);
}
void EnableHapticFeedbackImpl(Ark_NativePointer node,
                              Ark_Boolean isEnabled)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(isEnabled);
    //TextInputModelNG::SetEnableHapticFeedback(frameNode, convValue);
}
} // TextInputAttributeModifier
const GENERATED_ArkUITextInputModifier* GetTextInputModifier()
{
    static const GENERATED_ArkUITextInputModifier ArkUITextInputModifierImpl {
        TextInputInterfaceModifier::SetTextInputOptionsImpl,
        TextInputAttributeModifier::TypeImpl,
        TextInputAttributeModifier::ContentTypeImpl,
        TextInputAttributeModifier::PlaceholderColorImpl,
        TextInputAttributeModifier::TextOverflowImpl,
        TextInputAttributeModifier::TextIndentImpl,
        TextInputAttributeModifier::PlaceholderFontImpl,
        TextInputAttributeModifier::EnterKeyTypeImpl,
        TextInputAttributeModifier::CaretColorImpl,
        TextInputAttributeModifier::OnEditChangedImpl,
        TextInputAttributeModifier::OnEditChangeImpl,
        TextInputAttributeModifier::OnSubmitImpl,
        TextInputAttributeModifier::OnChangeImpl,
        TextInputAttributeModifier::OnTextSelectionChangeImpl,
        TextInputAttributeModifier::OnContentScrollImpl,
        TextInputAttributeModifier::MaxLengthImpl,
        TextInputAttributeModifier::FontColorImpl,
        TextInputAttributeModifier::FontSizeImpl,
        TextInputAttributeModifier::FontStyleImpl,
        TextInputAttributeModifier::FontWeightImpl,
        TextInputAttributeModifier::FontFamilyImpl,
        TextInputAttributeModifier::InputFilterImpl,
        TextInputAttributeModifier::OnCopyImpl,
        TextInputAttributeModifier::OnCutImpl,
        TextInputAttributeModifier::OnPasteImpl,
        TextInputAttributeModifier::CopyOptionImpl,
        TextInputAttributeModifier::ShowPasswordIconImpl,
        TextInputAttributeModifier::TextAlignImpl,
        TextInputAttributeModifier::StyleImpl,
        TextInputAttributeModifier::CaretStyleImpl,
        TextInputAttributeModifier::SelectedBackgroundColorImpl,
        TextInputAttributeModifier::CaretPositionImpl,
        TextInputAttributeModifier::EnableKeyboardOnFocusImpl,
        TextInputAttributeModifier::PasswordIconImpl,
        TextInputAttributeModifier::ShowErrorImpl,
        TextInputAttributeModifier::ShowUnitImpl,
        TextInputAttributeModifier::ShowUnderlineImpl,
        TextInputAttributeModifier::UnderlineColorImpl,
        TextInputAttributeModifier::SelectionMenuHiddenImpl,
        TextInputAttributeModifier::BarStateImpl,
        TextInputAttributeModifier::MaxLinesImpl,
        TextInputAttributeModifier::WordBreakImpl,
        TextInputAttributeModifier::LineBreakStrategyImpl,
        TextInputAttributeModifier::CustomKeyboardImpl,
        TextInputAttributeModifier::ShowCounterImpl,
        TextInputAttributeModifier::CancelButtonImpl,
        TextInputAttributeModifier::SelectAllImpl,
        TextInputAttributeModifier::MinFontSizeImpl,
        TextInputAttributeModifier::MaxFontSizeImpl,
        TextInputAttributeModifier::HeightAdaptivePolicyImpl,
        TextInputAttributeModifier::EnableAutoFillImpl,
        TextInputAttributeModifier::DecorationImpl,
        TextInputAttributeModifier::LetterSpacingImpl,
        TextInputAttributeModifier::LineHeightImpl,
        TextInputAttributeModifier::PasswordRulesImpl,
        TextInputAttributeModifier::FontFeatureImpl,
        TextInputAttributeModifier::ShowPasswordImpl,
        TextInputAttributeModifier::OnSecurityStateChangeImpl,
        TextInputAttributeModifier::OnWillInsertImpl,
        TextInputAttributeModifier::OnDidInsertImpl,
        TextInputAttributeModifier::OnWillDeleteImpl,
        TextInputAttributeModifier::OnDidDeleteImpl,
        TextInputAttributeModifier::EditMenuOptionsImpl,
        TextInputAttributeModifier::EnablePreviewTextImpl,
        TextInputAttributeModifier::EnableHapticFeedbackImpl,
    };
    return &ArkUITextInputModifierImpl;
}

}

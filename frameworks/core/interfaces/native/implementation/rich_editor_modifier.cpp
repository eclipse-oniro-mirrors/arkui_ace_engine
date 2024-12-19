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

#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_styled_string_controller.h"
#include "core/components_ng/base/frame_node.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/converter2.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/generated/interface/node_api.h"
#include "rich_editor_controller_peer_impl.h"
#include "rich_editor_styled_string_controller_peer_impl.h"

namespace OHOS::Ace::NG::Converter {

void AssignArkValue(Ark_RichEditorSelection& dst, const BaseEventInfo& src)
{
    if (src.GetType() == "SelectionInfo") {
        auto selectionInfo = static_cast<const SelectionInfo*>(&src);
        if (selectionInfo) {
            auto selection = selectionInfo->GetSelection();
            dst.selection.value0 = Converter::ArkValue<Ark_Number>(selection.selection[0]);
            dst.selection.value1 = Converter::ArkValue<Ark_Number>(selection.selection[1]);
        }
    }
}

void AssignArkValue(Ark_RichEditorRange& dst, const BaseEventInfo& src)
{
    if (src.GetType() == "SelectionInfo") {
        auto selectionInfo = static_cast<const SelectionInfo*>(&src);
        if (selectionInfo) {
            auto selection = selectionInfo->GetSelection();
            dst.start = Converter::ArkValue<Opt_Number>(selection.selection[0]);
            dst.end = Converter::ArkValue<Opt_Number>(selection.selection[1]);
        }
    }
}

void AssignArkValue(Ark_RichEditorInsertValue& dst, const RichEditorInsertValue& src)
{
    dst.insertOffset = Converter::ArkValue<Ark_Number>(src.GetInsertOffset());
    dst.insertValue = Converter::ArkValue<Ark_String>(src.GetInsertValue());
    dst.previewText = Converter::ArkValue<Opt_String>(src.GetPreviewText());
}

void AssignArkValue(Ark_RichEditorSpanPosition& dst, const RichEditorAbstractSpanResult& src)
{
    dst.spanIndex = Converter::ArkValue<Ark_Number>(src.GetSpanIndex());
    dst.spanRange.value0 = Converter::ArkValue<Ark_Number>(src.GetSpanRangeStart());
    dst.spanRange.value1 = Converter::ArkValue<Ark_Number>(src.GetSpanRangeEnd());
}

void AssignArkValue(Ark_DecorationStyleResult& dst, const RichEditorAbstractSpanResult& src)
{
    dst.type = Converter::ArkValue<Ark_TextDecorationType>(src.GetTextDecoration());
    dst.color = Converter::ArkUnion<Ark_ResourceColor, Ark_String>(src.GetColor());
    dst.style.value = Converter::ArkValue<Ark_TextDecorationStyle>(src.GetTextDecorationStyle());
}

void AssignArkValue(Ark_String& dst, const FONT_FEATURES_LIST& src)
{
    CHECK_NULL_VOID(src.empty());
    JsonValue jsonValue;
    for (auto it = src.begin(); it != src.end(); it++) {
        jsonValue.Put((it->first.c_str()), it->second);
    }
    static std::string list = jsonValue.ToString();
    dst = Converter::ArkValue<Ark_String>(list);
}

void AssignArkValue(Ark_RichEditorTextStyleResult& dst, const RichEditorAbstractSpanResult& src)
{
    dst.fontColor = Converter::ArkUnion<Ark_ResourceColor, Ark_String>(src.GetFontColor());
    dst.fontSize = Converter::ArkValue<Ark_Number>(src.GetFontSize());
    dst.fontStyle = Converter::ArkValue<Ark_FontStyle>(src.GetFontStyle());
    dst.fontWeight = Converter::ArkValue<Ark_Number>(src.GetFontWeight());
    dst.fontFamily = Converter::ArkValue<Ark_String>(src.GetFontFamily());
    dst.decoration = Converter::ArkValue<Ark_DecorationStyleResult>(src);
    // dst.textShadow = implement it
    LOGW("RichEditor modifier :: textShadow conversion is not implemented yet.");
    dst.letterSpacing = Converter::ArkValue<Opt_Number>(src.GetLetterspacing());
    dst.lineHeight = Converter::ArkValue<Opt_Number>(src.GetLineHeight());
    dst.fontFeature.value = Converter::ArkValue<Ark_String>(src.GetFontFeatures());
}

void AssignArkValue(Ark_RichEditorSymbolSpanStyle& dst, const SymbolSpanStyle& src)
{
    dst.fontSize = Converter::ArkUnion<Opt_Union_Number_String_Resource, Ark_Number>(src.fontSize);
    dst.fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_Number>(src.fontWeight);
    dst.effectStrategy.value = static_cast<Ark_SymbolEffectStrategy>(src.effectStrategy);
    dst.renderingStrategy.value = static_cast<Ark_SymbolRenderingStrategy>(src.renderingStrategy);
}

void AssignArkValue(Ark_Resource& dst, const ResourceObject& src, ConvContext *ctx)
{
    dst.bundleName = Converter::ArkValue<Ark_String>(src.GetBundleName(), ctx);
    dst.moduleName = Converter::ArkValue<Ark_String>(src.GetModuleName(), ctx);
    dst.id = Converter::ArkValue<Ark_Number>(src.GetId());
    dst.type = Converter::ArkValue<Opt_Number>(src.GetType());
}

void AssignArkValue(Ark_RichEditorTextSpanResult& dst, const RichEditorAbstractSpanResult& src, ConvContext *ctx)
{
    dst.spanPosition = Converter::ArkValue<Ark_RichEditorSpanPosition>(src);
    dst.value = Converter::ArkValue<Ark_String>(src.GetValue());
    dst.textStyle = Converter::ArkValue<Ark_RichEditorTextStyleResult>(src);
    dst.symbolSpanStyle.value = Converter::ArkValue<Ark_RichEditorSymbolSpanStyle>(src.GetSymbolSpanStyle(), ctx);
    if (src.GetValueResource()) {
        dst.valueResource.value = Converter::ArkValue<Ark_Resource>(*src.GetValueResource(), ctx);
    }
    dst.previewText = Converter::ArkValue<Opt_String>(src.GetPreviewText());
}

void AssignArkValue(Ark_TextRange& dst, const TextRange& src)
{
    dst.start = Converter::ArkValue<Opt_Number>(src.start);
    dst.end = Converter::ArkValue<Opt_Number>(src.end);
}

void AssignArkValue(Ark_RichEditorDeleteValue& dst, const RichEditorDeleteValue& src)
{
    dst.offset = Converter::ArkValue<Ark_Number>(src.GetOffset());
    dst.direction = Converter::ArkValue<Ark_RichEditorDeleteDirection>(src.GetRichEditorDeleteDirection());
    dst.length = Converter::ArkValue<Ark_Number>(src.GetLength());
}

void AssignArkValue(Ark_RichEditorChangeValue& dst, const RichEditorChangeValue& src)
{
    auto rangeBefore = src.GetRangeBefore();
    dst.rangeBefore.start = Converter::ArkValue<Opt_Number>(rangeBefore.start);
    dst.rangeBefore.end = Converter::ArkValue<Opt_Number>(rangeBefore.end);
}

void AssignArkValue(Ark_SubmitEvent& dst, const NG::TextFieldCommonEvent& src, ConvContext *ctx)
{
    dst.text = Converter::ArkValue<Ark_String>(src.GetText(), ctx);
}

template<>
void AssignCast(std::optional<std::string>& dst, const Array_TextDataDetectorType& src)
{
    CHECK_NULL_VOID(src.array);
    std::string ret;
    for (int idx = 0; idx < src.length; idx++) {
        Ark_TextDataDetectorType type = src.array[idx];
        switch (type) {
            case ARK_TEXT_DATA_DETECTOR_TYPE_PHONE_NUMBER:
                ret += "phoneNum";
                break;
            case ARK_TEXT_DATA_DETECTOR_TYPE_URL:
                ret += "url";
                break;
            case ARK_TEXT_DATA_DETECTOR_TYPE_EMAIL:
                ret += "email";
                break;
            case ARK_TEXT_DATA_DETECTOR_TYPE_ADDRESS:
                ret += "location";
                break;
            case ARK_TEXT_DATA_DETECTOR_TYPE_DATE_TIME:
                ret += "datetime";
                break;
            default:
                break;
        }
        bool isLast = idx == (src.length - 1);
        if (!isLast) {
            ret += ",";
        }
    }
    dst = ret;
}

template<>
void AssignCast(std::optional<TextDecorationStyle>& dst, const Ark_DecorationStyleInterface& src)
{
    if (auto style = Converter::OptConvert<TextDecorationStyle>(src.style); style) {
        dst = style.value();
    }
}

template<>
TextDetectConfig Convert(const Ark_TextDataDetectorConfig& src)
{
    TextDetectConfig ret;
    ret.types = Converter::OptConvert<std::string>(src.types).value_or("");
    if (auto color = Converter::OptConvert<Color>(src.color); color) {
        ret.entityColor = color.value();
    }
    if (auto style = Converter::OptConvert<TextDecorationStyle>(src.decoration); style) {
        ret.entityDecorationStyle = style.value();
    }
    return ret;
}

template<>
void AssignCast(std::optional<PlaceholderOptions>& dst, const Ark_PlaceholderStyle& src)
{
    PlaceholderOptions ret;
    ret.fontSize = Converter::OptConvert<Dimension>(src.font.value.size);
    ret.fontWeight = Converter::OptConvert<FontWeight>(src.font.value.weight);
    ret.fontColor = Converter::OptConvert<Color>(src.fontColor);
    ret.fontStyle = Converter::OptConvert<OHOS::Ace::FontStyle>(src.font.value.style);
    dst = ret;
}

} // OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RichEditorModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = RichEditorModelNG::CreateFrameNode(id);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // RichEditorModifier
namespace RichEditorInterfaceModifier {
void SetRichEditorOptions0Impl(Ark_NativePointer node,
                               const Ark_RichEditorOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    // obtain the internal RichEditorController
    RefPtr<RichEditorBaseControllerBase> internalController = RichEditorModelNG::GetRichEditorController(frameNode);
    void *ptr = internalController.GetRawPtr();
    RichEditorController *rawPtr = reinterpret_cast<RichEditorController *>(ptr);
    CHECK_NULL_VOID(rawPtr);
    WeakPtr<RichEditorController> controller;
    controller = rawPtr;

    auto peerImplPtr = reinterpret_cast<GeneratedModifier::RichEditorControllerPeerImpl *>(value->controller.ptr);
    CHECK_NULL_VOID(peerImplPtr);

    // pass the internal controller to external management
    peerImplPtr->AddTargetController(controller);
}
void SetRichEditorOptions1Impl(Ark_NativePointer node,
                               const Ark_RichEditorStyledStringOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    RichEditorModelNG::SetStyledStringMode(frameNode, true);
    // obtain the internal Styled String RichEditorController
    RefPtr<RichEditorBaseControllerBase> internalController =
        RichEditorModelNG::GetRichEditorStyledStringController(frameNode);
    void *ptr = internalController.GetRawPtr();
    RichEditorStyledStringController *rawPtr = reinterpret_cast<RichEditorStyledStringController *>(ptr);
    CHECK_NULL_VOID(rawPtr);
    WeakPtr<RichEditorStyledStringController> controller;
    controller = rawPtr;

    auto peerImplPtr = reinterpret_cast<GeneratedModifier::RichEditorStyledStringControllerPeerImpl *>(
        options->controller.ptr);
    CHECK_NULL_VOID(peerImplPtr);

    // pass the internal controller to external management
    peerImplPtr->AddTargetController(controller);
}
} // RichEditorInterfaceModifier
namespace RichEditorAttributeModifier {
void OnReadyImpl(Ark_NativePointer node,
                 const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onReady(frameNode->GetId());
    };
    RichEditorModelNG::SetOnReady(frameNode, std::move(onCallback));
}
void OnSelectImpl(Ark_NativePointer node,
                  const Callback_RichEditorSelection_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const BaseEventInfo* event) {
        CHECK_NULL_VOID(event);
        auto selection = Converter::ArkValue<Ark_RichEditorSelection>(*event);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onSelect(
            frameNode->GetId(), selection);
    };
    RichEditorModelNG::SetOnSelect(frameNode, std::move(onCallback));
}
void OnSelectionChangeImpl(Ark_NativePointer node,
                           const Callback_RichEditorRange_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const BaseEventInfo* event) {
        CHECK_NULL_VOID(event);
        auto range = Converter::ArkValue<Ark_RichEditorRange>(*event);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onSelectionChange(frameNode->GetId(), range);
    };
    RichEditorModelNG::SetOnSelectionChange(frameNode, std::move(onCallback));
}
void AboutToIMEInputImpl(Ark_NativePointer node,
                         const Callback_RichEditorInsertValue_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const RichEditorInsertValue& param) -> bool {
        Ark_RichEditorInsertValue data = Converter::ArkValue<Ark_RichEditorInsertValue>(param);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->aboutToIMEInput(frameNode->GetId(), data);
        LOGW("Richeditor modifier, AboutToIMEInputImpl :: callback should return bool value back to the aceEngine");
        return true;
    };
    RichEditorModelNG::SetAboutToIMEInput(frameNode, std::move(onCallback));
}
void OnIMEInputCompleteImpl(Ark_NativePointer node,
                            const Callback_RichEditorTextSpanResult_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const RichEditorAbstractSpanResult& param) {
        Converter::ConvContext ctx;
        auto data = Converter::ArkValue<Ark_RichEditorTextSpanResult>(param, &ctx);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onIMEInputComplete(frameNode->GetId(), data);
    };
    RichEditorModelNG::SetOnIMEInputComplete(frameNode, std::move(onCallback));
}
void OnDidIMEInputImpl(Ark_NativePointer node,
                       const Callback_TextRange_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const TextRange& param) {
        auto data = Converter::ArkValue<Ark_TextRange>(param);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onDidIMEInput(frameNode->GetId(), data);
    };
    RichEditorModelNG::SetOnDidIMEInput(frameNode, std::move(onCallback));
}
void AboutToDeleteImpl(Ark_NativePointer node,
                       const Callback_RichEditorDeleteValue_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const RichEditorDeleteValue& param) -> bool {
        auto data = Converter::ArkValue<Ark_RichEditorDeleteValue>(param);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->aboutToDelete(frameNode->GetId(), data);
        LOGW("AboutToDeleteImpl :: callback should return bool value back to the aceEngine");
        return true;
    };
    RichEditorModelNG::SetAboutToDelete(frameNode, std::move(onCallback));
}
void OnDeleteCompleteImpl(Ark_NativePointer node,
                          const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onDeleteComplete(frameNode->GetId());
    };
    RichEditorModelNG::SetOnDeleteComplete(frameNode, std::move(onCallback));
}
void CopyOptionsImpl(Ark_NativePointer node,
                     Ark_CopyOptions value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto options = Converter::OptConvert<CopyOptions>(value);
    if (options) {
        RichEditorModelNG::SetCopyOption(frameNode, options.value());
    }
}
void OnPasteImpl(Ark_NativePointer node,
                 const PasteEventCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](NG::TextCommonEvent& param) {
        auto event = Converter::ArkValue<Opt_PasteEvent>(Ark_Empty());
        LOGW("RichEditor modifier:: Opt_PasteEvent usage is not implemented yet.");
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onPaste(frameNode->GetId(), event);
    };
    RichEditorModelNG::SetOnPaste(frameNode, std::move(onCallback));
}
void EnableDataDetectorImpl(Ark_NativePointer node,
                            Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    RichEditorModelNG::SetTextDetectEnable(frameNode, convValue);
}
void EnablePreviewTextImpl(Ark_NativePointer node,
                           Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    RichEditorModelNG::SetSupportPreviewText(frameNode, convValue);
}
void DataDetectorConfigImpl(Ark_NativePointer node,
                            const Ark_TextDataDetectorConfig* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto textDetectConfig = Converter::Convert<TextDetectConfig>(*value);
    RichEditorModelNG::SetTextDetectConfig(frameNode, textDetectConfig);
}
void CaretColorImpl(Ark_NativePointer node,
                    const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<Color>(*value);
    RichEditorModelNG::SetCaretColor(frameNode, convValue);
}
void SelectedBackgroundColorImpl(Ark_NativePointer node,
                                 const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<Color>(*value);
    RichEditorModelNG::SetSelectedBackgroundColor(frameNode, convValue);
}
void OnEditingChangeImpl(Ark_NativePointer node,
                         const Callback_Boolean_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const bool& param) {
        Ark_Boolean flag = Converter::ArkValue<Ark_Boolean>(param);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onEditingChange(frameNode->GetId(), flag);
    };
    RichEditorModelNG::SetOnEditingChange(frameNode, std::move(onCallback));
}
void EnterKeyTypeImpl(Ark_NativePointer node,
                      Ark_EnterKeyType value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<TextInputAction>(value);
    RichEditorModelNG::SetEnterKeyType(frameNode, convValue);
}
void OnSubmitImpl(Ark_NativePointer node,
                  const SubmitCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](int32_t param1, NG::TextFieldCommonEvent& param2) {
        Ark_EnterKeyType enterKey = static_cast<Ark_EnterKeyType>(param1);
        Converter::ConvContext ctx;
        Ark_SubmitEvent event = Converter::ArkValue<Ark_SubmitEvent>(param2, &ctx);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onSubmit(frameNode->GetId(), enterKey, event);
    };
    RichEditorModelNG::SetOnSubmit(frameNode, std::move(onCallback));
}
void OnWillChangeImpl(Ark_NativePointer node,
                      const Callback_RichEditorChangeValue_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const RichEditorChangeValue& param) -> bool {
        auto data = Converter::ArkValue<Ark_RichEditorChangeValue>(param);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onWillChange(frameNode->GetId(), data);
        LOGW("OnWillChangeImpl :: callback should return bool value back to the aceEngine");
        return true;
    };
    RichEditorModelNG::SetOnWillChange(frameNode, std::move(onCallback));
}
void OnDidChangeImpl(Ark_NativePointer node,
                     const OnDidChangeCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](const RichEditorChangeValue& param) {
        TextRange inBefore = param.GetRangeBefore();
        TextRange inAfter = param.GetRangeAfter();
        Ark_TextRange rangeBefore = Converter::ArkValue<Ark_TextRange>(inBefore);
        Ark_TextRange rangeAfter = Converter::ArkValue<Ark_TextRange>(inAfter);
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onDidChange(
            frameNode->GetId(), rangeBefore, rangeAfter);
    };
    RichEditorModelNG::SetOnDidChange(frameNode, std::move(onCallback));
}
void OnCutImpl(Ark_NativePointer node,
               const Callback_CutEvent_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](NG::TextCommonEvent& param) {
        Ark_CutEvent event;
        LOGW("RichEditor modifier :: Ark_CutEvent conversion is not implemented yet.");
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onCut(frameNode->GetId(), event);
    };
    RichEditorModelNG::SetOnCut(frameNode, std::move(onCallback));
}
void OnCopyImpl(Ark_NativePointer node,
                const Callback_CopyEvent_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto onCallback = [frameNode](NG::TextCommonEvent& param) {
        Ark_CopyEvent event;
        LOGW("RichEditor modifier :: Ark_CopyEvent conversion is not implemented yet.");
        GetFullAPI()->getEventsAPI()->getRichEditorEventsReceiver()->onCopy(frameNode->GetId(), event);
    };
    RichEditorModelNG::SetOnCopy(frameNode, std::move(onCallback));
}
void EditMenuOptionsImpl(Ark_NativePointer node,
                         const Ark_EditMenuOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //RichEditorModelNG::SetEditMenuOptions(frameNode, convValue);
    LOGW("RichEditor modifier :: EditMenuOptionsImpl() needs onCreateMenuCallback, onMenuItemClick input");
}
void EnableKeyboardOnFocusImpl(Ark_NativePointer node,
                               Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    RichEditorModelNG::SetRequestKeyboardOnFocus(frameNode, convValue);
}
void EnableHapticFeedbackImpl(Ark_NativePointer node,
                              Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    RichEditorModelNG::SetEnableHapticFeedback(frameNode, convValue);
}
void BarStateImpl(Ark_NativePointer node,
                  Ark_BarState value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<DisplayMode>(value);
    RichEditorModelNG::SetBarState(frameNode, convValue);
}
void BindSelectionMenuImpl(Ark_NativePointer node,
                           Ark_RichEditorSpanType spanType,
                           const CustomNodeBuilder* content,
                           const Ark_Union_ResponseType_RichEditorResponseType* responseType,
                           const Opt_SelectionMenuOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(responseType);
    CHECK_NULL_VOID(options);
    auto aceSpanType = Converter::OptConvert<TextSpanType>(spanType);
    CHECK_NULL_VOID(aceSpanType);
    auto aceResponseType = Converter::OptConvert<TextResponseType>(*responseType);
    CHECK_NULL_VOID(aceResponseType); // A required parameter
    auto arkMenuOptions = Converter::OptConvert<Ark_SelectionMenuOptions>(*options);
    SelectMenuParam menuParam;
    menuParam.onAppear = [](int32_t start, int32_t end) {};
    menuParam.onDisappear = []() {};
    if (arkMenuOptions) {
        auto appearCb = Converter::OptConvert<MenuOnAppearCallback>(arkMenuOptions->onAppear);
        auto disappearCb = Converter::OptConvert<Callback_Void>(arkMenuOptions->onDisappear);

        CHECK_NULL_VOID(appearCb);
        auto appearCbPtr = std::make_shared<MenuOnAppearCallback>(*appearCb); // Well captured as shared_ptr
        menuParam.onAppear =
            [appearCbPtr, arkCallback = CallbackHelper(*appearCbPtr)](int32_t start, int32_t end) {
            if (appearCbPtr) {
                arkCallback.Invoke(Converter::ArkValue<Ark_Number>(start), Converter::ArkValue<Ark_Number>(end));
            }
        };

        CHECK_NULL_VOID(disappearCb);
        auto disappearCbPtr = std::make_shared<Callback_Void>(*disappearCb); // Well captured as shared_ptr
        menuParam.onDisappear =
            [disappearCbPtr, arkCallback = CallbackHelper(*disappearCbPtr)]() {
            if (disappearCbPtr) {
                arkCallback.Invoke();
            }
        };
    }

    std::function<void()> buildFunc = [arkCallback = CallbackHelper(*content), parentNode = frameNode]() {
        Callback_Pointer_Void continuation;
        arkCallback.Invoke(parentNode, continuation);
    };

    RichEditorModelNG::BindSelectionMenu(
        frameNode, aceSpanType.value(), aceResponseType.value(), buildFunc, menuParam);
}
void CustomKeyboardImpl(Ark_NativePointer node,
                        const CustomNodeBuilder* value,
                        const Opt_KeyboardOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    auto convValue = Converter::OptConvert<Ark_KeyboardOptions>(*options);
    std::optional<bool> supportAvoidance;
    if (convValue) {
        supportAvoidance = Converter::OptConvert<bool>(convValue->supportAvoidance);
    }
    std::function<void()> callback = []() {};
    if (value) {
        callback = [arkCallback = CallbackHelper(*value), parentNode = frameNode]() -> void {
            Callback_Pointer_Void continuation;
            arkCallback.Invoke(parentNode, continuation);
        };
    }
    RichEditorModelNG::SetCustomKeyboard(frameNode, std::move(callback), supportAvoidance);
}
void PlaceholderImpl(Ark_NativePointer node,
                     const Ark_ResourceStr* value,
                     const Opt_PlaceholderStyle* style)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    CHECK_NULL_VOID(style);
    PlaceholderOptions options;
    if (auto value = Converter::OptConvert<PlaceholderOptions>(*style); value) {
        options = *value;
    }
    options.value = Converter::OptConvert<std::string>(*value);
    RichEditorModelNG::SetPlaceholder(frameNode, options);
}
} // RichEditorAttributeModifier
const GENERATED_ArkUIRichEditorModifier* GetRichEditorModifier()
{
    static const GENERATED_ArkUIRichEditorModifier ArkUIRichEditorModifierImpl {
        RichEditorModifier::ConstructImpl,
        RichEditorInterfaceModifier::SetRichEditorOptions0Impl,
        RichEditorInterfaceModifier::SetRichEditorOptions1Impl,
        RichEditorAttributeModifier::OnReadyImpl,
        RichEditorAttributeModifier::OnSelectImpl,
        RichEditorAttributeModifier::OnSelectionChangeImpl,
        RichEditorAttributeModifier::AboutToIMEInputImpl,
        RichEditorAttributeModifier::OnIMEInputCompleteImpl,
        RichEditorAttributeModifier::OnDidIMEInputImpl,
        RichEditorAttributeModifier::AboutToDeleteImpl,
        RichEditorAttributeModifier::OnDeleteCompleteImpl,
        RichEditorAttributeModifier::CopyOptionsImpl,
        RichEditorAttributeModifier::OnPasteImpl,
        RichEditorAttributeModifier::EnableDataDetectorImpl,
        RichEditorAttributeModifier::EnablePreviewTextImpl,
        RichEditorAttributeModifier::DataDetectorConfigImpl,
        RichEditorAttributeModifier::CaretColorImpl,
        RichEditorAttributeModifier::SelectedBackgroundColorImpl,
        RichEditorAttributeModifier::OnEditingChangeImpl,
        RichEditorAttributeModifier::EnterKeyTypeImpl,
        RichEditorAttributeModifier::OnSubmitImpl,
        RichEditorAttributeModifier::OnWillChangeImpl,
        RichEditorAttributeModifier::OnDidChangeImpl,
        RichEditorAttributeModifier::OnCutImpl,
        RichEditorAttributeModifier::OnCopyImpl,
        RichEditorAttributeModifier::EditMenuOptionsImpl,
        RichEditorAttributeModifier::EnableKeyboardOnFocusImpl,
        RichEditorAttributeModifier::EnableHapticFeedbackImpl,
        RichEditorAttributeModifier::BarStateImpl,
        RichEditorAttributeModifier::BindSelectionMenuImpl,
        RichEditorAttributeModifier::CustomKeyboardImpl,
        RichEditorAttributeModifier::PlaceholderImpl,
    };
    return &ArkUIRichEditorModifierImpl;
}

}

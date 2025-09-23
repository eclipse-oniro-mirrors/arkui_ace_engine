/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <utility>
#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#ifdef ARKUI_CAPI_UNITTEST
#include "test/unittest/capi/stubs/mock_text_picker_dialog_view.h"
#else
#include "core/components_ng/pattern/text_picker/textpicker_dialog_view.h"
#include "core/components_ng/pattern/text_picker/textpicker_model.h"
#endif
#include "core/interfaces/native/utility/callback_helper.h"
#include "arkoala_api_generated.h"
#include "text_picker_modifier.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace TextPickerDialogAccessor {

uint32_t CalculateKind(bool fromRangeContent, const std::vector<NG::RangeContent>& range)
{
    uint32_t kind = 0;
    if (!fromRangeContent) {
        kind = NG::TEXT;
    } else {
        kind |= NG::ICON;
        for (const auto& rangeContent : range) {
            if (!rangeContent.text_.empty()) {
                kind |= NG::TEXT;
                break;
            }
        }
    }
    return kind;
}
void ParseTextPickerOptions(const Ark_TextPickerDialogOptions& src, TextPickerOptions& dst)
{
    auto pickerValueOpt = Converter::OptConvert<Converter::PickerValueType>(src.value);
    if (pickerValueOpt) {
        auto pickerValue = pickerValueOpt.value();
        dst.hasValue = true;
        if (auto value = std::get_if<std::string>(&pickerValue); value) {
            dst.value = *value;
            dst.values.emplace_back(dst.value);
        } else {
            dst.values = std::move(std::get<std::vector<std::string>>(pickerValue));
        }
    }
    auto pickerSelectedOpt = Converter::OptConvert<Converter::PickerSelectedType>(src.selected);
    if (pickerSelectedOpt) {
        auto pickerSelected = pickerSelectedOpt.value();
        dst.hasSelected = true;
        if (auto selected = std::get_if<uint32_t>(&pickerSelected); selected) {
            dst.selected = *selected;
            dst.selecteds.emplace_back(dst.selected);
        } else {
            dst.selecteds = std::move(std::get<std::vector<uint32_t>>(pickerSelected));
        }
    }
    auto pickerRangeOpt = Converter::OptConvert<PickerRangeType>(src.range);
    if (pickerRangeOpt) {
        auto pickerRange = pickerRangeOpt.value();
        auto rangeVector = std::get_if<std::pair<bool, std::vector<NG::RangeContent>>>(&pickerRange);
        if (rangeVector) {
            auto fromRangeContent = rangeVector->first;
            dst.range = rangeVector->second;
            dst.kind = CalculateKind(fromRangeContent, dst.range);
            if (fromRangeContent) {
                dst.value = "";
                dst.hasValue = true;
            }
        } else {
            auto options = std::get<std::pair<bool, std::vector<NG::TextCascadePickerOptions>>>(pickerRange);
            dst.options = std::move(options.second);
            dst.isCascade = options.first;
            dst.kind = NG::TEXT;
        }
    }
}

void TextPickerOptions2SettingData(TextPickerOptions& textPickerOptions, TextPickerSettingData& settingData)
{
    // Verify the data to prevent triggering of exceptions
    if (!textPickerOptions.range.empty()) {
        ValidateSingleTextPickerOptions(textPickerOptions);
    } else if (!textPickerOptions.options.empty()) {
        ValidateMultiTextPickerOptions(textPickerOptions);
    }
    // data conversion
    settingData.rangeVector = std::move(textPickerOptions.range);
    settingData.selected = textPickerOptions.selected;
    settingData.columnKind = textPickerOptions.kind;
    settingData.selectedValues = std::move(textPickerOptions.selecteds);
    settingData.values = std::move(textPickerOptions.values);
    settingData.options = std::move(textPickerOptions.options);
    settingData.attr.isCascade = textPickerOptions.isCascade;
    settingData.attr.isHasSelectAttr = textPickerOptions.hasSelected;
}

void BuildTextPickerSettingData(const Ark_TextPickerDialogOptions& arkOptions,
                                TextPickerSettingData& settingData, TextPickerDialog& pickerDialog)
{
    // parse frontend input parameter type TextPickerOptions
    TextPickerOptions textPickerOptions;
    ParseTextPickerOptions(arkOptions, textPickerOptions);
    TextPickerOptions2SettingData(textPickerOptions, settingData);

    pickerDialog.isDefaultHeight = false;
    auto height = Converter::OptConvert<Dimension>(arkOptions.defaultPickerItemHeight);
    if (height) {
        settingData.height = height.value();
        pickerDialog.height = settingData.height;
        pickerDialog.isDefaultHeight = true;
    }
    settingData.canLoop = true;
    auto canLoop = Converter::OptConvert<bool>(arkOptions.canLoop);
    if (canLoop) {
        settingData.canLoop = canLoop.value();
    }
    // pickerBgStyle, temporarily set as default value
    settingData.pickerBgStyle.color = Color::TRANSPARENT;
    settingData.pickerBgStyle.borderRadius = NG::BorderRadiusProperty(8.0_vp);
    // property for text style
    auto disappearTextStyle = Converter::OptConvert<PickerTextStyle>(arkOptions.disappearTextStyle);
    if (disappearTextStyle) {
        settingData.properties.disappearTextStyle_ = disappearTextStyle.value();
    }
    auto textStyle = Converter::OptConvert<PickerTextStyle>(arkOptions.textStyle);
    if (textStyle) {
        settingData.properties.normalTextStyle_ = textStyle.value();
    }
    auto selectedTextStyle = Converter::OptConvert<PickerTextStyle>(arkOptions.selectedTextStyle);
    if (selectedTextStyle) {
        settingData.properties.selectedTextStyle_ = selectedTextStyle.value();
    }
    auto defaultTextStyle = Converter::OptConvert<PickerTextStyle>(arkOptions.defaultTextStyle);
    if (defaultTextStyle) {
        settingData.properties.defaultTextStyle_ = defaultTextStyle.value();
    }

    // disableTextStyleAnimation
    settingData.isDisableTextStyleAnimation = false;
    auto disableTextStyleAnimation = Converter::OptConvert<bool>(arkOptions.disableTextStyleAnimation);
    if (disableTextStyleAnimation) {
        settingData.isDisableTextStyleAnimation = disableTextStyleAnimation.value();
    }
    settingData.isEnableHapticFeedback = true;
    auto enableHapticFeedback = Converter::OptConvert<bool>(arkOptions.enableHapticFeedback);
    if (enableHapticFeedback) {
        settingData.isEnableHapticFeedback = enableHapticFeedback.value();
    }
}

DialogTextEvent BuildTextEvent(Callback_TextPickerResult_Void callback)
{
    return [arkCallback = CallbackHelper(callback)](const std::string& info) -> void {
        Ark_TextPickerResult textPickerRes;
        auto data = JsonUtil::ParseJsonString(info);

        std::vector<uint32_t> indexes;
        auto jsonIndex = data->GetValue("index");
        auto isIndexesArray = jsonIndex->IsArray();
        if (isIndexesArray) {
            for (auto i = 0; i < jsonIndex->GetArraySize(); i++) {
                indexes.push_back(jsonIndex->GetArrayItem(i)->GetUInt());
            }
        } else {
            indexes.push_back(jsonIndex->GetUInt());
        }
        Converter::ArkArrayHolder<Array_Number> indexesHolder(indexes);
        if (isIndexesArray) {
            Array_Number arkValues = indexesHolder.ArkValue();
            textPickerRes.index = Converter::ArkUnion<Ark_Union_Number_Array_Number, Array_Number>(arkValues);
        } else if (static_cast<int32_t>(indexes.size()) > 0) {
            textPickerRes.index = Converter::ArkUnion<Ark_Union_Number_Array_Number, Ark_Number>(
                Converter::ArkValue<Ark_Number>(indexes.at(0)));
        }

        std::vector<std::string> values;
        auto jsonValue = data->GetValue("value");
        auto isValuesArray = jsonValue->IsArray();
        if (isValuesArray) {
            for (auto i = 0; i < jsonValue->GetArraySize(); i++) {
                values.push_back(jsonValue->GetArrayItem(i)->GetString());
            }
        } else {
            values.push_back(data->GetString("value"));
        }
        Converter::ArkArrayHolder<Array_String> holder(values);
        if (isValuesArray) {
            Array_String arkValues = holder.ArkValue();
            textPickerRes.value = Converter::ArkUnion<Ark_Union_String_Array_String, Array_String>(arkValues);
        } else if (static_cast<int32_t>(values.size()) > 0) {
            textPickerRes.value = Converter::ArkUnion<Ark_Union_String_Array_String, Ark_String>(
                Converter::ArkValue<Ark_String>(values.at(0)));
        }

        arkCallback.Invoke(textPickerRes);
    };
}

void BuildTextPickerDialog(const Ark_TextPickerDialogOptions& options, TextPickerDialog& pickerDialog)
{
    // selectedValue getRangeVector
    auto enableHoverMode = Converter::OptConvert<bool>(options.enableHoverMode);
    if (enableHoverMode) {
        pickerDialog.enableHoverMode = enableHoverMode.value();
    }
    auto alignment = Converter::OptConvert<DialogAlignment>(options.alignment);
    if (alignment) {
        pickerDialog.alignment = alignment.value();
    }
    auto offset = Converter::OptConvert<DimensionOffset>(options.offset);
    if (offset) {
        pickerDialog.offset = offset.value();
    }
    pickerDialog.maskRect = Converter::OptConvert<DimensionRect>(options.maskRect);
    pickerDialog.backgroundColor = Converter::OptConvert<Color>(options.backgroundColor);
    pickerDialog.backgroundBlurStyle = static_cast<int32_t>(Converter::OptConvert<BlurStyle>(
        options.backgroundBlurStyle).value_or(BlurStyle::COMPONENT_REGULAR));
    pickerDialog.blurStyleOption = Converter::OptConvert<BlurStyleOption>(options.backgroundBlurStyleOptions);
    pickerDialog.effectOption =  Converter::OptConvert<EffectOption>(options.backgroundEffect);
    pickerDialog.shadow = Converter::OptConvert<Shadow>(options.shadow);
    auto hoverModeArea = Converter::OptConvert<HoverModeAreaType>(options.hoverModeArea);
    if (hoverModeArea) {
        pickerDialog.hoverModeArea = hoverModeArea.value();
    }
}

TextPickerDialogEvent BuildTextPickerDialogEvents(const Ark_TextPickerDialogOptions& options)
{
    TextPickerDialogEvent dialogEvent;
    auto didAppearCallbackOpt = Converter::OptConvert<Callback_Void>(options.onDidAppear);
    if (didAppearCallbackOpt) {
        auto onDidAppear = [arkCallback = CallbackHelper(*didAppearCallbackOpt)]() -> void {
            arkCallback.Invoke();
        };
        dialogEvent.onDidAppear = onDidAppear;
    }
    auto didDisappearCallbackOpt = Converter::OptConvert<Callback_Void>(options.onDidDisappear);
    if (didDisappearCallbackOpt) {
        auto onDidDisappear = [arkCallback = CallbackHelper(*didDisappearCallbackOpt)]() -> void {
            arkCallback.Invoke();
        };
        dialogEvent.onDidDisappear = onDidDisappear;
    }
    auto willAppearCallbackOpt = Converter::OptConvert<Callback_Void>(options.onWillAppear);
    if (willAppearCallbackOpt) {
        auto onWillAppear = [arkCallback = CallbackHelper(*willAppearCallbackOpt)]() -> void {
            arkCallback.Invoke();
        };
        dialogEvent.onWillAppear = onWillAppear;
    }
    auto willDisappearCallbackOpt = Converter::OptConvert<Callback_Void>(options.onWillDisappear);
    if (willDisappearCallbackOpt) {
        auto onWillDisappear = [arkCallback = CallbackHelper(*willDisappearCallbackOpt)]() -> void {
            arkCallback.Invoke();
        };
        dialogEvent.onWillDisappear = onWillDisappear;
    }
    return dialogEvent;
}

TextPickerInteractiveEvent BuildSelectInteractiveEvents(const Ark_TextPickerDialogOptions& arkOptions)
{
    TextPickerInteractiveEvent events;
    // onCancel
    auto cancelCallbackOpt = Converter::OptConvert<Callback_Void>(arkOptions.onCancel);
    if (cancelCallbackOpt) {
        events.cancelEvent = [arkCallback = CallbackHelper(*cancelCallbackOpt)]() -> void {
            arkCallback.Invoke();
        };
    }
    // onAccept
    auto acceptCallbackOpt = Converter::OptConvert<Callback_TextPickerResult_Void>(arkOptions.onAccept);
    if (acceptCallbackOpt) {
        events.acceptEvent = BuildTextEvent(*acceptCallbackOpt);
    }
    // onChange
    auto changeCallbackOpt = Converter::OptConvert<Callback_TextPickerResult_Void>(arkOptions.onChange);
    if (changeCallbackOpt) {
        events.changeEvent = BuildTextEvent(*changeCallbackOpt);
    }
    // onScrollStop
    auto scrollStopCallbackOpt = Converter::OptConvert<Callback_TextPickerResult_Void>(arkOptions.onScrollStop);
    if (scrollStopCallbackOpt) {
        events.scrollStopEvent = BuildTextEvent(*scrollStopCallbackOpt);
    }
    // onEnterSelectedAreaEvent
    auto enterSelectedAreaCallbackOpt =
        Converter::OptConvert<Callback_TextPickerResult_Void>(arkOptions.onEnterSelectedArea);
    if (enterSelectedAreaCallbackOpt) {
        events.enterSelectedAreaEvent = BuildTextEvent(*enterSelectedAreaCallbackOpt);
    }
    return events;
}

std::vector<ButtonInfo> BuildButtonInfos(const Ark_TextPickerDialogOptions& options)
{
    std::vector<ButtonInfo> buttonInfos;
    auto acceptButtonInfo = Converter::OptConvert<ButtonInfo>(options.acceptButtonStyle);
    if (acceptButtonInfo.has_value()) {
        buttonInfos.emplace_back(acceptButtonInfo.value());
        buttonInfos[0].isAcceptButton = true;
    } else {
        ButtonInfo buttonInfo;
        buttonInfos.emplace_back(buttonInfo);
    }
    auto cancelButtonInfo = Converter::OptConvert<ButtonInfo>(options.cancelButtonStyle);
    if (cancelButtonInfo.has_value()) {
        buttonInfos.emplace_back(cancelButtonInfo.value());
    }
    return buttonInfos;
}

void ShowImpl(const Opt_TextPickerDialogOptions* options)
{
    CHECK_NULL_VOID(options);
    auto arkOptionsOpt = Converter::OptConvert<Ark_TextPickerDialogOptions>(*options);
    if (!arkOptionsOpt.has_value()) { return; }

    Ark_TextPickerDialogOptions arkOptions = arkOptionsOpt.value();

    TextPickerSettingData settingData;
    TextPickerDialog textPickerDialog;
    BuildTextPickerSettingData(arkOptions, settingData, textPickerDialog);
    BuildTextPickerDialog(arkOptions, textPickerDialog);
    auto interEvents = BuildSelectInteractiveEvents(arkOptions);
    auto dialogEvents = BuildTextPickerDialogEvents(arkOptions);
    std::vector<ButtonInfo> buttonInfos = BuildButtonInfos(arkOptions);
    // WARING: don't use, only adapter backend function interface.
    RefPtr<AceType> pickerText = nullptr;
    TextPickerDialogModel::GetInstance()->SetTextPickerDialogShow(pickerText, settingData,
        std::move(interEvents.cancelEvent), std::move(interEvents.acceptEvent), std::move(interEvents.changeEvent),
        std::move(interEvents.scrollStopEvent), std::move(interEvents.enterSelectedAreaEvent),
        textPickerDialog, dialogEvents, buttonInfos);
}
} // TextPickerDialogAccessor

const GENERATED_ArkUITextPickerDialogAccessor* GetTextPickerDialogAccessor()
{
    static const GENERATED_ArkUITextPickerDialogAccessor TextPickerDialogAccessorImpl {
        TextPickerDialogAccessor::ShowImpl,
    };
    return &TextPickerDialogAccessorImpl;
}
}

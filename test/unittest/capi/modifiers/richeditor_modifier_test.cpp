/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "richeditor_modifier_test.h"
#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/native/implementation/rich_editor_controller_peer_impl.h"
#include "core/interfaces/native/implementation/rich_editor_styled_string_controller_peer_impl.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"

namespace OHOS::Ace::NG {
static constexpr int TEST_OFFSET = 5;
static constexpr int TEST_OFFSET_2 = 7;
static constexpr int TEST_INDEX = 1;
static constexpr auto TEST_COLOR = "#FFFF0000";
static const auto TEST_VALUE = "test value";
static const auto TEST_VALUE_2 = "test value 2";
static constexpr int TEST_INDEX_2 = 2;
static constexpr int TEST_FONT_SIZE = 30;
static constexpr int TEST_FONT_WEIGHT = static_cast<int>(FontWeight::BOLD);
static const std::string COLOR_TRANSPARENT = "#00000000";
static constexpr int TEST_RESOURCE_ID = 1000;

static const auto ATTRIBUTE_COPY_OPTIONS_NAME = "CopyOption";
static const auto ATTRIBUTE_COPY_OPTIONS_DEFAULT_VALUE = "CopyOptions.None";
static const auto ATTRIBUTE_COPY_OPTIONS_IN_APP_VALUE = "CopyOptions.InApp";
static const auto ATTRIBUTE_ENABLE_DATA_DETECTOR_NAME = "enableDataDetector";
static const auto ATTRIBUTE_ENABLE_PREVIEW_TEXT_NAME = "textPreviewSupported";
static const auto ATTRIBUTE_DATA_DETECTOR_CONFIG_NAME = "dataDetectorConfig";
static const auto ATTRIBUTE_DATA_DETECTOR_CONFIG_VALUE_ARR = {
    ARK_TEXT_DATA_DETECTOR_TYPE_PHONE_NUMBER,
    ARK_TEXT_DATA_DETECTOR_TYPE_URL,
    ARK_TEXT_DATA_DETECTOR_TYPE_EMAIL,
    ARK_TEXT_DATA_DETECTOR_TYPE_ADDRESS,
    ARK_TEXT_DATA_DETECTOR_TYPE_DATE_TIME,
};
static const auto ATTRIBUTE_DATA_DETECTOR_CONFIG_VALUE = "phoneNum,url,email,location,datetime";
static const auto ATTRIBUTE_DATA_DETECTOR_STYLE_VALUE = ARK_TEXT_DECORATION_STYLE_DASHED;
static const auto ATTRIBUTE_PLACEHOLDER_NAME = "placeholder";
static const auto ATTRIBUTE_PLACEHOLDER_VALUE_NAME = "value";
static const auto ATTRIBUTE_PLACEHOLDER_STYLE_NAME = "style";
static const auto ATTRIBUTE_PLACEHOLDER_FONT_COLOR_NAME = "fontColor";
static const auto ATTRIBUTE_PLACEHOLDER_FONT_NAME = "font";
static const auto ATTRIBUTE_PLACEHOLDER_FONT_SIZE_NAME = "size";
static const auto ATTRIBUTE_PLACEHOLDER_FONT_SIZE = "30.00px";
static const auto ATTRIBUTE_PLACEHOLDER_WEIGHT_NAME = "weight";
static const auto ATTRIBUTE_PLACEHOLDER_WEIGHT_NAME_VALUE = "FontWeight.Normal";
static const auto ATTRIBUTE_PLACEHOLDER_STYLE_VALUE = "FontStyle.Normal";
static const auto ATTRIBUTE_ENTER_KEY_TYPE_NAME = "enterKeyType";
static const auto ATTRIBUTE_ENTER_KEY_TYPE_DEFAULT_VALUE = "EnterKeyType.Begin";
static const auto ATTRIBUTE_ENTER_KEY_TYPE_NEWLINE_VALUE = "EnterKeyType.NEW_LINE";
static const auto ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_NAME = "enableKeyboardOnFocus";
static const auto ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_DEFAULT_VALUE = "true";
static const auto ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_NAME = "enableHapticFeedback";
static const auto ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_DEFAULT_VALUE = "true";
static const auto ATTRIBUTE_BAR_STATE_NAME = "barState";
static const auto ATTRIBUTE_BAR_STATE_DEFAULT = "BarState.Auto";
static const auto ATTRIBUTE_BAR_STATE_VALUE = "BarState.On";
static const auto ATTRIBUTE_CUSTOM_KB_NAME = "keyboardAvoidance";
static const auto ATTRIBUTE_CUSTOM_KB_DEFAULT_VALUE = "false";
static const auto ATTRIBUTE_CUSTOM_KB_VALUE = "true";

typedef std::tuple<Ark_ResourceColor, std::string> ColorTestStep;
static const std::vector<ColorTestStep> COLOR_TEST_PLAN = {
    { Converter::ArkUnion<Ark_ResourceColor, enum Ark_Color>(ARK_COLOR_BLUE), "#FF0000FF" },
    { Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
    { Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f), COLOR_TRANSPARENT },
    { Converter::ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
    { Converter::ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" }};

using namespace testing;
using namespace testing::ext;

class RichEditorModifierTest : public ModifierTestBase<GENERATED_ArkUIRichEditorModifier,
    &GENERATED_ArkUINodeModifiers::getRichEditorModifier, GENERATED_ARKUI_RICH_EDITOR> {
};

/**
 * @tc.name: setRichEditorOptions0Test
 * @tc.desc: Check the functionality of setRichEditorOptions0
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setRichEditorOptions0Test, TestSize.Level1)
{
    ASSERT_NE(modifier_->setRichEditorOptions0, nullptr);

    // assume nothing bad with invalid and empty options
    modifier_->setRichEditorOptions0(node_, nullptr);

    auto controllerUndef = Converter::ArkValue<Opt_RichEditorOptions>(Ark_Empty());
    modifier_->setRichEditorOptions0(node_, &controllerUndef.value);

    // Check the internal controller
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto internalController = RichEditorModelNG::GetRichEditorController(frameNode);
    ASSERT_NE(internalController, nullptr);
    void* ptr = internalController.GetRawPtr();
    RichEditorController *rawPtr = reinterpret_cast<RichEditorController *>(ptr);
    ImageSpanOptions imageOptions;
    rawPtr->AddImageSpan(imageOptions);

    GeneratedModifier::RichEditorControllerPeerImpl peer;
    auto controller = Converter::ArkValue<Ark_RichEditorOptions>(&peer);
    modifier_->setRichEditorOptions0(node_, &controller);

    TextSpanOptions textSpanOptions;
    textSpanOptions.offset = TEST_OFFSET;
    textSpanOptions.value = TEST_VALUE;
    int32_t result = peer.AddTextSpanImpl(textSpanOptions);
    ASSERT_EQ(result, TEST_INDEX);

    TextSpanOptions textSpanOptions2;
    textSpanOptions.offset = TEST_OFFSET_2;
    textSpanOptions.value = TEST_VALUE_2;
    result = peer.AddTextSpanImpl(textSpanOptions2);
    ASSERT_EQ(result, TEST_INDEX_2);
}

/**
 * @tc.name: setRichEditorOptions1Test
 * @tc.desc: Check the functionality of setRichEditorOptions1
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setRichEditorOptions1Test, TestSize.Level1)
{
    ASSERT_NE(modifier_->setRichEditorOptions1, nullptr);

    // assume nothing bad with invalid and empty options
    modifier_->setRichEditorOptions1(node_, nullptr);

    Ark_RichEditorStyledStringOptions controllerUndef;
    controllerUndef.controller.ptr = nullptr;
    modifier_->setRichEditorOptions1(node_, &controllerUndef);

    // Check the internal controller
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto internalController = RichEditorModelNG::GetRichEditorStyledStringController(frameNode);
    ASSERT_NE(internalController, nullptr);
    void* ptr = internalController.GetRawPtr();
    RichEditorStyledStringController *rawPtr = reinterpret_cast<RichEditorStyledStringController *>(ptr);
    ASSERT_NE(rawPtr->GetStyledString().GetRawPtr(), nullptr);

    GeneratedModifier::RichEditorStyledStringControllerPeerImpl peer;
    auto controller = Converter::ArkValue<Ark_RichEditorStyledStringOptions>(&peer);
    modifier_->setRichEditorOptions1(node_, &controller);

    auto string = peer.GetStyledString();
    ASSERT_NE(string.GetRawPtr(), nullptr);
    peer.SetStyledString(string);
    ASSERT_NE(string.GetRawPtr(), peer.GetStyledString().GetRawPtr());
}

/**
 * @tc.name: setCopyOptionTest
 * @tc.desc: Check the functionality of setCopyOption
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setCopyOptionTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setCopyOptions, nullptr);

    std::unique_ptr<JsonValue> jsonValue = GetLayoutJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_COPY_OPTIONS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_COPY_OPTIONS_DEFAULT_VALUE);

    auto optionsConverted = ARK_COPY_OPTIONS_IN_APP;
    modifier_->setCopyOptions(node_, optionsConverted);

    jsonValue = GetLayoutJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_COPY_OPTIONS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_COPY_OPTIONS_IN_APP_VALUE);
}

/**
 * @tc.name: setEnableDataDetectorTest
 * @tc.desc: Check the functionality of setEnableDataDetector
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setEnableDataDetectorTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEnableDataDetector, nullptr);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_DATA_DETECTOR_NAME);
    EXPECT_EQ(resultStr, "false");

    auto enabled = Converter::ArkValue<Ark_Boolean>(true);
    modifier_->setEnableDataDetector(node_, enabled);

    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_DATA_DETECTOR_NAME);
    EXPECT_EQ(resultStr, "true");
}

/**
 * @tc.name: setEnablePreviewTextTest
 * @tc.desc: Check the functionality of setEnablePreviewText
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setEnablePreviewTextTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEnablePreviewText, nullptr);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_PREVIEW_TEXT_NAME);
    EXPECT_EQ(resultStr, "true");

    auto enabled = Converter::ArkValue<Ark_Boolean>(false);
    modifier_->setEnablePreviewText(node_, enabled);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_PREVIEW_TEXT_NAME);
    EXPECT_EQ(resultStr, "false");

    enabled = Converter::ArkValue<Ark_Boolean>(true);
    modifier_->setEnablePreviewText(node_, enabled);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_PREVIEW_TEXT_NAME);
    EXPECT_EQ(resultStr, "true");
}

/**
 * @tc.name: setDataDetectorConfigTest
 * @tc.desc: Check the functionality of setDataDetectorConfig
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setDataDetectorConfigTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setDataDetectorConfig, nullptr);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DATA_DETECTOR_CONFIG_NAME);
    EXPECT_EQ(resultStr, "");

    Ark_TextDataDetectorConfig config;
    config.color = Converter::ArkUnion<Opt_ResourceColor, Ark_String>(TEST_COLOR);
    Converter::ArkArrayHolder<Array_TextDataDetectorType> types(ATTRIBUTE_DATA_DETECTOR_CONFIG_VALUE_ARR);
    config.types = types.ArkValue();
    config.decoration = Converter::ArkValue<Opt_DecorationStyleInterface>(Ark_DecorationStyleInterface{
        .style = Converter::ArkValue<Opt_TextDecorationStyle>(ATTRIBUTE_DATA_DETECTOR_STYLE_VALUE)
    });

    auto configConverted = Converter::ArkValue<Ark_TextDataDetectorConfig>(config);
    modifier_->setDataDetectorConfig(node_, &configConverted);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DATA_DETECTOR_CONFIG_NAME);
    auto jsonConfig = JsonUtil::ParseJsonString(resultStr);
    resultStr = GetAttrValue<std::string>(jsonConfig, "color");
    EXPECT_EQ(resultStr, TEST_COLOR);
    resultStr = GetAttrValue<std::string>(jsonConfig, "types");
    EXPECT_EQ(resultStr, ATTRIBUTE_DATA_DETECTOR_CONFIG_VALUE);
    resultStr = GetAttrValue<std::string>(jsonConfig, "decoration");
    auto jsonStyle = JsonUtil::ParseJsonString(resultStr);
    int testValue = GetAttrValue<int>(jsonStyle, "style");
    EXPECT_EQ(testValue, static_cast<int>(ATTRIBUTE_DATA_DETECTOR_STYLE_VALUE));
}

void FixValue(std::string& value)
{
    size_t pos = 0;
    auto issue1 = "\"{\"";
    while ((pos = value.find(issue1, pos)) != std::string::npos) {
        value.replace(pos, strlen(issue1), "{\"");
        pos++;
    }
    pos = 0;
    auto issue2 = "\"}\"";
    while ((pos = value.find(issue2, pos)) != std::string::npos) {
        value.replace(pos, strlen(issue2), "\"}");
        pos++;
    }
}

/**
 * @tc.name: setPlaceholderTest
 * @tc.desc: Check the functionality of setPlaceholder
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setPlaceholderTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setPlaceholder, nullptr);

    Ark_ResourceStr value = Converter::ArkUnion<Ark_ResourceStr, Ark_String>(TEST_VALUE);
    Ark_Font label;
    label.size = Converter::ArkValue<Opt_Length>(TEST_FONT_SIZE);
    label.weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(TEST_FONT_WEIGHT);
    label.style = Converter::ArkValue<Opt_FontStyle>(ARK_FONT_STYLE_NORMAL);
    Opt_PlaceholderStyle style;
    style.value.font = Converter::ArkValue<Opt_Font>(label);
    Ark_ResourceColor fontColor = Converter::ArkUnion<Ark_ResourceColor, Ark_String>(TEST_COLOR);
    style.value.fontColor = Converter::ArkValue<Opt_ResourceColor>(fontColor);

    modifier_->setPlaceholder(node_, &value, &style);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_PLACEHOLDER_NAME);
    FixValue(resultStr);
    std::unique_ptr<JsonValue> placeholderValue = JsonUtil::ParseJsonData(resultStr.c_str());

    resultStr = GetAttrValue<std::string>(placeholderValue, ATTRIBUTE_PLACEHOLDER_VALUE_NAME);
    EXPECT_EQ(resultStr, TEST_VALUE);

    std::unique_ptr<JsonValue> styleValue = placeholderValue->GetObject(ATTRIBUTE_PLACEHOLDER_STYLE_NAME);
    resultStr = GetAttrValue<std::string>(styleValue, ATTRIBUTE_PLACEHOLDER_FONT_COLOR_NAME);
    EXPECT_EQ(resultStr, TEST_COLOR);

    std::unique_ptr<JsonValue> fontValue = styleValue->GetObject(ATTRIBUTE_PLACEHOLDER_FONT_NAME);
    resultStr = GetAttrValue<std::string>(fontValue, ATTRIBUTE_PLACEHOLDER_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_PLACEHOLDER_FONT_SIZE);

    resultStr = GetAttrValue<std::string>(fontValue, ATTRIBUTE_PLACEHOLDER_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_PLACEHOLDER_WEIGHT_NAME_VALUE);

    resultStr = GetAttrValue<std::string>(fontValue, ATTRIBUTE_PLACEHOLDER_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_PLACEHOLDER_STYLE_VALUE);
}

/**
 * @tc.name: setCaretColorTest
 * @tc.desc: Check the functionality of setCaretColor
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setCaretColorTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setCaretColor, nullptr);

    for (const auto& [value, expectVal] : COLOR_TEST_PLAN) {
        modifier_->setCaretColor(node_, &value);
        auto checkVal = GetAttrValue<std::string>(node_, "caretColor");
        EXPECT_EQ(checkVal, expectVal);
    }
}

/**
 * @tc.name: setSelectedBackgroundColorTest
 * @tc.desc: Check the functionality of setSelectedBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setSelectedBackgroundColorTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setSelectedBackgroundColor, nullptr);

    for (const auto& [value, expectVal] : COLOR_TEST_PLAN) {
        modifier_->setSelectedBackgroundColor(node_, &value);
        auto checkVal = GetAttrValue<std::string>(node_, "selectedBackgroundColor");
        EXPECT_EQ(checkVal, expectVal);
    }
}

/**
 * @tc.name: setEnterKeyTypeTest
 * @tc.desc: Check the functionality of setEnterKeyType
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setEnterKeyTypeTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEnterKeyType, nullptr);
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENTER_KEY_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ENTER_KEY_TYPE_DEFAULT_VALUE);

    modifier_->setEnterKeyType(node_, Ark_EnterKeyType::ARK_ENTER_KEY_TYPE_NEW_LINE);

    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENTER_KEY_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ENTER_KEY_TYPE_NEWLINE_VALUE);
}

/**
 * @tc.name: setEnableKeyboardOnFocusTest
 * @tc.desc: Check the functionality of setEnableKeyboardOnFocus
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setEnableKeyboardOnFocusTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEnableKeyboardOnFocus, nullptr);
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);

    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_DEFAULT_VALUE);

    modifier_->setEnableKeyboardOnFocus(node_, false);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_NAME);
    EXPECT_EQ(resultStr, "false");

    modifier_->setEnableKeyboardOnFocus(node_, true);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_KEYBOARD_ON_FOCUS_NAME);
    EXPECT_EQ(resultStr, "true");
}

/**
 * @tc.name: setEnableHapticFeedbackTest
 * @tc.desc: Check the functionality of setEnableHapticFeedback
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setEnableHapticFeedbackTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEnableHapticFeedback, nullptr);
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);

    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_DEFAULT_VALUE);

    modifier_->setEnableHapticFeedback(node_, false);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_NAME);
    EXPECT_EQ(resultStr, "false");

    modifier_->setEnableHapticFeedback(node_, true);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_NAME);
    EXPECT_EQ(resultStr, "true");
}

/**
 * @tc.name: setBarStateTest
 * @tc.desc: Check the functionality of setBarState
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setBarStateTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBarState, nullptr);
    std::unique_ptr<JsonValue> jsonValue = GetLayoutJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BAR_STATE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BAR_STATE_DEFAULT);

    modifier_->setBarState(node_, Ark_BarState::ARK_BAR_STATE_ON);

    jsonValue = GetLayoutJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BAR_STATE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BAR_STATE_VALUE);
}

static bool g_onBuilt = false;
static bool g_onAppear = false;
static bool g_onDisappear = false;

/**
 * @tc.name: setBindSelectionMenuTest
 * @tc.desc: Check the functionality of setBindSelectionMenu
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setBindSelectionMenuTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(modifier_->setBindSelectionMenu, nullptr);
    // Prepare callbacks
    auto onBuiltCallback = [](const Ark_Int32 resourceId,
        const Ark_NativePointer parentNode, const Callback_Pointer_Void continuation) {
        g_onBuilt = true;
    };
    auto onAppearCallback = [](const Ark_Int32 resourceId, const Ark_Number start, const Ark_Number end) {
        g_onAppear = true;
    };
    auto onDisappearCallback = [](const Ark_Int32 resourceId) {
        g_onDisappear = true;
    };
    // Prepare options
    auto responseType = Converter::ArkUnion<Ark_Union_ResponseType_RichEditorResponseType, Ark_ResponseType>(
        Ark_ResponseType::ARK_RESPONSE_TYPE_LONG_PRESS);
    auto buildFunc = Converter::ArkValue<CustomNodeBuilder>(onBuiltCallback, TEST_RESOURCE_ID);
    Ark_SelectionMenuOptions value;
    value.menuType = Converter::ArkValue<Opt_MenuType>(Ark_MenuType::ARK_MENU_TYPE_PREVIEW_MENU);
    auto onAppearCb = Converter::ArkValue<MenuOnAppearCallback>(onAppearCallback, TEST_RESOURCE_ID);
    value.onAppear = Converter::ArkValue<Opt_MenuOnAppearCallback>(onAppearCb);
    auto onDisappearCb = Converter::ArkValue<Callback_Void>(onDisappearCallback, TEST_RESOURCE_ID);
    value.onDisappear = Converter::ArkValue<Opt_Callback_Void>(onDisappearCb);

    auto options = Converter::ArkValue<Opt_SelectionMenuOptions>(value);
    modifier_->setBindSelectionMenu(node_,
        Ark_RichEditorSpanType::ARK_RICH_EDITOR_SPAN_TYPE_TEXT, &buildFunc, &responseType, &options);

    // The testing part begins here:
    EXPECT_FALSE(g_onBuilt);
    EXPECT_FALSE(g_onAppear);
    EXPECT_FALSE(g_onDisappear);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetSelectedType(TextSpanType::TEXT); // Needed for logic of CopySelectionMenuParams()
    SelectOverlayInfo selectInfo;
    pattern->CopySelectionMenuParams(selectInfo, TextResponseType::LONG_PRESS);
    selectInfo.menuInfo.menuBuilder();
    EXPECT_TRUE(g_onBuilt);
    selectInfo.menuCallback.onAppear();
    EXPECT_TRUE(g_onAppear);
    selectInfo.menuCallback.onDisappear();
    EXPECT_TRUE(g_onDisappear);
}

static bool g_keyboardCallbackCalled = false;

/**
 * @tc.name: setCustomKeyboardTest
 * @tc.desc: Check the functionality of setCustomKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorModifierTest, setCustomKeyboardTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(modifier_->setCustomKeyboard, nullptr);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CUSTOM_KB_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CUSTOM_KB_DEFAULT_VALUE);

    auto onCallback = [](const Ark_Int32 resourceId,
        const Ark_NativePointer parentNode, const Callback_Pointer_Void continuation) {
        g_keyboardCallbackCalled = true;
    };
    auto keyboardBuilderCallback = Converter::ArkValue<CustomNodeBuilder>(onCallback, TEST_RESOURCE_ID);

    Ark_KeyboardOptions keyboardOptions;
    keyboardOptions.supportAvoidance = Converter::ArkValue<Opt_Boolean>(true);
    auto options = Converter::ArkValue<Opt_KeyboardOptions>(keyboardOptions);

    modifier_->setCustomKeyboard(node_, &keyboardBuilderCallback, &options);

    // Testing callback
    EXPECT_FALSE(g_keyboardCallbackCalled);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    bool built = pattern->RequestCustomKeyboard();
    EXPECT_TRUE(built);
    EXPECT_TRUE(g_keyboardCallbackCalled);

    // Testing value
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CUSTOM_KB_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CUSTOM_KB_VALUE);
}

} // namespace OHOS::Ace::NG

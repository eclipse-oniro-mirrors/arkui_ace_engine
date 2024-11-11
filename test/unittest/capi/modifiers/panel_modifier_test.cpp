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

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"

#include "arkoala_api_generated.h"
#include "core/components_ng/pattern/panel/sliding_panel_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_MODE_NAME = "mode";
    const auto ATTRIBUTE_MODE_DEFAULT_VALUE = "PanelMode.Half";
    const auto ATTRIBUTE_TYPE_NAME = "type";
    const auto ATTRIBUTE_TYPE_DEFAULT_VALUE = "PanelType.Foldable";
    const auto ATTRIBUTE_DRAG_BAR_NAME = "dragBar";
    const auto ATTRIBUTE_DRAG_BAR_DEFAULT_VALUE = "true";
    const auto ATTRIBUTE_SHOW_NAME = "show";
    const auto ATTRIBUTE_SHOW_DEFAULT_VALUE = "true";
    const auto ATTRIBUTE_BACKGROUND_MASK_COLOR_NAME = "backgroundMask";
    const auto ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE = "#08182431";
    const auto ATTRIBUTE_SHOW_CLOSE_ICON_NAME = "showCloseIcon";
    const auto ATTRIBUTE_SHOW_CLOSE_ICON_DEFAULT_VALUE = "false";
    const auto ATTRIBUTE_MINI_HEIGHT_NAME = "miniHeight";
    const auto ATTRIBUTE_MINI_HEIGHT_DEFAULT_VALUE = "48.00vp";
    const auto ATTRIBUTE_HALF_HEIGHT_NAME = "halfHeight";
    const auto ATTRIBUTE_HALF_HEIGHT_DEFAULT_VALUE = "0.00px";
    const auto ATTRIBUTE_FULL_HEIGHT_NAME = "fullHeight";
    const auto ATTRIBUTE_FULL_HEIGHT_DEFAULT_VALUE = "0.00px";
    const auto ATTRIBUTE_CUSTOM_HEIGHT_NAME = "customHeight";
    const auto ATTRIBUTE_CUSTOM_HEIGHT_DEFAULT_VALUE = "0.00px";

struct EventsTracker {
    static inline GENERATED_ArkUIPanelEventsReceiver panelEventReceiver {};

    static inline const GENERATED_ArkUIEventsAPI eventsApiImpl {
        .getPanelEventsReceiver = [] () -> const GENERATED_ArkUIPanelEventsReceiver* {
            return &panelEventReceiver;
        }
    };
}; // EventsTracker
} // namespace

class PanelModifierTest : public ModifierTestBase<
    GENERATED_ArkUIPanelModifier,
    &GENERATED_ArkUINodeModifiers::getPanelModifier,
    GENERATED_ARKUI_PANEL> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        // setup the test event handler
        fullAPI_->setArkUIEventsAPI(&EventsTracker::eventsApiImpl);
    }
};

/*
 * @tc.name: setModeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setModeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MODE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MODE_DEFAULT_VALUE);
}

// Valid values for attribute 'mode' of method 'mode'
static std::vector<std::tuple<std::string, Ark_PanelMode, std::string>> modeModeValidValues = {
    {"ARK_PANEL_MODE_MINI", Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_MINI), "PanelMode.Mini"},
    {"ARK_PANEL_MODE_HALF", Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_HALF), "PanelMode.Half"},
    {"ARK_PANEL_MODE_FULL", Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_FULL), "PanelMode.Full"},
};

/*
 * @tc.name: setModeTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setModeTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    // Verifying attribute's  values
    for (auto& [print, input, expected]: modeModeValidValues) {
        modifier_->setMode(node_, input);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MODE_NAME);
        EXPECT_EQ(resultStr, expected) << "Passed value is: " << print;
    }
}

// Invalid values for attribute 'mode' of method 'mode'
static std::vector<std::tuple<std::string, Ark_PanelMode>> modeModeInvalidValues = {
    {"static_cast<Ark_PanelMode>(-1)", Converter::ArkValue<Ark_PanelMode>(static_cast<Ark_PanelMode>(-1))},
};

/*
 * @tc.name: setModeTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setModeTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_PanelMode inputValueMode;
    Ark_PanelMode initValueMode;

    // Initial setup
    initValueMode = std::get<1>(modeModeValidValues[0]);

    // Verifying attribute's  values
    for (auto& [print, input]: modeModeInvalidValues) {
        inputValueMode = initValueMode;
        modifier_->setMode(node_, inputValueMode);
        inputValueMode = input;
        modifier_->setMode(node_, inputValueMode);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MODE_NAME);
        expectedStr = ATTRIBUTE_MODE_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << print;
    }
}

/*
 * @tc.name: setTypeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setTypeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

// Valid values for attribute 'type' of method 'type'
static std::vector<std::tuple<std::string, Ark_PanelType, std::string>> typeTypeValidValues = {
{"ARK_PANEL_TYPE_MINIBAR", Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_MINIBAR), "PanelType.Minibar"},
{"ARK_PANEL_TYPE_FOLDABLE", Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_FOLDABLE), "PanelType.Foldable"},
{"ARK_PANEL_TYPE_TEMPORARY", Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_TEMPORARY), "PanelType.Temporary"},
{"ARK_PANEL_TYPE_CUSTOM", Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_CUSTOM), "PanelType.CUSTOM"},
};

/*
 * @tc.name: DISABLED_setTypeTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setTypeTestValidValues, TestSize.Level1)
{
    // reason of disable: need fix array PANEL_TYPE in SlidingPanelPattern::ToJsonValue
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_PanelType inputValueType;
    Ark_PanelType initValueType;

    // Initial setup
    initValueType = std::get<1>(typeTypeValidValues[0]);

    // Verifying attribute's  values
    inputValueType = initValueType;
    for (auto&& value: typeTypeValidValues) {
        inputValueType = std::get<1>(value);
        modifier_->setType(node_, inputValueType);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'type' of method 'type'
static std::vector<std::tuple<std::string, Ark_PanelType>> typeTypeInvalidValues = {
    {"static_cast<Ark_PanelType>(-1)", Converter::ArkValue<Ark_PanelType>(static_cast<Ark_PanelType>(-1))},
};

/*
 * @tc.name: setTypeTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setTypeTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_PanelType inputValueType;
    Ark_PanelType initValueType;

    // Initial setup
    initValueType = std::get<1>(typeTypeValidValues[0]);

    // Verifying attribute's  values
    for (auto&& value: typeTypeInvalidValues) {
        inputValueType = initValueType;
        modifier_->setType(node_, inputValueType);
        inputValueType = std::get<1>(value);
        modifier_->setType(node_, inputValueType);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        expectedStr = ATTRIBUTE_TYPE_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setDragBarTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setDragBarTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DRAG_BAR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_DRAG_BAR_DEFAULT_VALUE);
}

// Valid values for attribute 'dragBar' of method 'dragBar'
static std::vector<std::tuple<std::string, Ark_Boolean, std::string>> dragBarDragBarValidValues = {
    {"true", Converter::ArkValue<Ark_Boolean>(true), "true"},
    {"false", Converter::ArkValue<Ark_Boolean>(false), "false"},
};

/*
 * @tc.name: setDragBarTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setDragBarTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    // Verifying attribute's  values
    for (auto& [print, input, expected]: dragBarDragBarValidValues) {
        modifier_->setDragBar(node_, input);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DRAG_BAR_NAME);
        EXPECT_EQ(resultStr, expected) << "Passed value is: " << print;
    }
}

/*
 * @tc.name: setShowTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setShowTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SHOW_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_SHOW_DEFAULT_VALUE);
}

// Valid values for attribute 'show' of method 'show'
static std::vector<std::tuple<std::string, Ark_Boolean, std::string>> showShowValidValues = {
    {"true", Converter::ArkValue<Ark_Boolean>(true), "true"},
    {"false", Converter::ArkValue<Ark_Boolean>(false), "false"},
};

/*
 * @tc.name: setShowTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setShowTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    for (auto& [print, input, expected]: showShowValidValues) {
        modifier_->setShow(node_, input);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SHOW_NAME);
        EXPECT_EQ(resultStr, expected) << "Passed value is: " << print;
    }
}

/*
 * @tc.name: DISABLED_setBackgroundMaskTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setBackgroundMaskTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BACKGROUND_MASK_COLOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE);
}

// Valid values for attribute 'backgroundMaskColor' of method 'backgroundMask'
static std::vector<std::tuple<std::string, Ark_ResourceColor, std::string>>
    backgroundMaskBackgroundMaskColorValidValues = {
    { "#FF0000FF", Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_BLUE), "#FF0000FF" },
    { "#FF123456", Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
    { Color::TRANSPARENT.ToString(), Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f),
        Color::TRANSPARENT.ToString() },
    { "#11223344", Converter::ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
    { "#FF00FFFF", Converter::ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" },
};

/*
 * @tc.name: DISABLED_setBackgroundMaskTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setBackgroundMaskTestValidValues, TestSize.Level1)
{
    // reason of disable: need fix set value in SlidingPanelPattern::ToJsonValue
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_ResourceColor inputValueBackgroundMaskColor;
    Ark_ResourceColor initValueBackgroundMaskColor;

    // Initial setup
    initValueBackgroundMaskColor = std::get<1>(backgroundMaskBackgroundMaskColorValidValues[0]);

    // Verifying attribute's  values
    inputValueBackgroundMaskColor = initValueBackgroundMaskColor;
    for (auto&& value: backgroundMaskBackgroundMaskColorValidValues) {
        inputValueBackgroundMaskColor = std::get<1>(value);
        modifier_->setBackgroundMask(node_, &inputValueBackgroundMaskColor);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BACKGROUND_MASK_COLOR_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

static std::vector<std::tuple<std::string, Ark_ResourceColor, std::string>>
    backgroundMaskBackgroundMaskColorInvalidValues = {
    { ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE, Converter::ArkUnion<Ark_ResourceColor, Ark_String>(""),
        ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE },
    { ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE,
        Converter::ArkUnion<Ark_ResourceColor, Ark_String>("incorrect_color"),
        ATTRIBUTE_BACKGROUND_MASK_COLOR_DEFAULT_VALUE }
};

/*
 * @tc.name: DISABLED_setBackgroundMaskTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setBackgroundMaskTestInvalidValues, TestSize.Level1)
{
    // reason of disable: need fix set value in SlidingPanelPattern::ToJsonValue
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_ResourceColor inputValueBackgroundMaskColor;
    Ark_ResourceColor initValueBackgroundMaskColor;

    // Initial setup
    initValueBackgroundMaskColor = std::get<1>(backgroundMaskBackgroundMaskColorInvalidValues[0]);

    // Verifying attribute's  values
    inputValueBackgroundMaskColor = initValueBackgroundMaskColor;
    for (auto&& value: backgroundMaskBackgroundMaskColorInvalidValues) {
        inputValueBackgroundMaskColor = std::get<1>(value);
        modifier_->setBackgroundMask(node_, &inputValueBackgroundMaskColor);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BACKGROUND_MASK_COLOR_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setShowCloseIconTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setShowCloseIconTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SHOW_CLOSE_ICON_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_SHOW_CLOSE_ICON_DEFAULT_VALUE);
}

// Valid values for attribute 'showCloseIcon' of method 'showCloseIcon'
static std::vector<std::tuple<std::string, Ark_Boolean, std::string>> showCloseIconValidValues = {
    {"true", Converter::ArkValue<Ark_Boolean>(true), "true"},
    {"false", Converter::ArkValue<Ark_Boolean>(false), "false"},
};

/*
 * @tc.name: setShowCloseIconTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setShowCloseIconTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    for (auto& [print, input, expected]: showCloseIconValidValues) {
        modifier_->setShowCloseIcon(node_, input);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SHOW_CLOSE_ICON_NAME);
        EXPECT_EQ(resultStr, expected) << "Passed value is: " << print;
    }
}

/*
 * @tc.name: DISABLED_setMiniHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setMiniHeightTestDefaultValues, TestSize.Level1)
{
    // reason of disable: need get default value from SlidingPanelLayoutAlgorithm
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_MINI));

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MINI_HEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MINI_HEIGHT_DEFAULT_VALUE);
}

typedef std::pair<Ark_Union_Number_String, std::string> OneTestStep;
const std::vector<OneTestStep> heightValidValues = {
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(1), "1.00vp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(0), "0.00vp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(2.45f), "2.45vp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("5px"), "5.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("22.35px"), "22.35px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("7vp"), "7.00vp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("1.65vp"), "1.65vp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("65fp"), "65.00fp" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("4.3fp"), "4.30fp" },
};

static const std::vector<OneTestStep> heightInvalidValues = {
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(-1), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(-2.45f), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-5px"), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-22.35px"), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-7vp"), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-1.65vp"), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-65fp"), "0.00px" },
    { Converter::ArkUnion<Ark_Union_Number_String, Ark_String>("-4.3fp"), "0.00px" },
};

/*
 * @tc.name: setMiniHeightTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setMiniHeightTestValidValues, TestSize.Level1)
{
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_MINI));

    for (const auto &[arkHeight, expected]: heightValidValues) {
        modifier_->setMiniHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_MINI_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: DISABLED_setMiniHeightTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setMiniHeightTestInvalidValues, TestSize.Level1)
{
    // reason of disable: need get default value for SlidingPanelLayoutAlgorithm
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_MINI));

    for (const auto &[arkHeight, expected]: heightInvalidValues) {
        modifier_->setMiniHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_MINI_HEIGHT_NAME);
        auto expectedStr = ATTRIBUTE_MINI_HEIGHT_DEFAULT_VALUE;
        EXPECT_EQ(strResult, expectedStr);
    }
}


/*
 * @tc.name: setHalfHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setHalfHeightTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_HALF));

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_HALF_HEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_HALF_HEIGHT_DEFAULT_VALUE);
}

/*
 * @tc.name: setHalfHeightTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setHalfHeightTestValidValues, TestSize.Level1)
{
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_HALF));

    for (const auto &[arkHeight, expected]: heightValidValues) {
        modifier_->setHalfHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_HALF_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setHalfHeightTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setHalfHeightTestInvalidValues, TestSize.Level1)
{
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_HALF));

    for (const auto &[arkHeight, expected]: heightInvalidValues) {
        modifier_->setHalfHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_HALF_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setFullHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setFullHeightTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_FULL));

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FULL_HEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FULL_HEIGHT_DEFAULT_VALUE);
}

/*
 * @tc.name: setFullHeightTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setFullHeightTestValidValues, TestSize.Level1)
{
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_FULL));

    for (const auto &[arkHeight, expected]: heightValidValues) {
        modifier_->setFullHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_FULL_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setFullHeightTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setFullHeightTestInvalidValues, TestSize.Level1)
{
    modifier_->setMode(node_, Converter::ArkValue<Ark_PanelMode>(ARK_PANEL_MODE_FULL));

    for (const auto &[arkHeight, expected]: heightInvalidValues) {
        modifier_->setFullHeight(node_, &arkHeight);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_FULL_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setCustomHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setCustomHeightTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    modifier_->setType(node_, Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_CUSTOM));

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CUSTOM_HEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CUSTOM_HEIGHT_DEFAULT_VALUE);
}

/*
 * @tc.name: setCustomHeightTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setCustomHeightTestValidValues, TestSize.Level1)
{
    typedef std::pair<Ark_Length, std::string> OneCustomHeightTestStep;
    static const std::vector<OneCustomHeightTestStep> testPlan = {
        { Converter::ArkValue<Ark_Length>(1.0f),  "1.00vp" },
        { Converter::ArkValue<Ark_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Ark_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Ark_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Ark_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Ark_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Ark_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Ark_Length>(4.3_fp), "4.30fp" },
    };

    modifier_->setType(node_, Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_CUSTOM));

    for (const auto &[lengthValue, expected]: testPlan) {
        auto value = Converter::ArkUnion<Ark_Union_Dimension_PanelHeight, Ark_Length>(lengthValue);
        modifier_->setCustomHeight(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_CUSTOM_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }

    auto value = Converter::ArkUnion<Ark_Union_Dimension_PanelHeight, Ark_PanelHeight>(
        Ark_PanelHeight::ARK_PANEL_HEIGHT_WRAP_CONTENT);
    modifier_->setCustomHeight(node_, &value);
    auto strResult = GetStringAttribute(node_, ATTRIBUTE_CUSTOM_HEIGHT_NAME);
    EXPECT_EQ(strResult, "wrapContentcalc");
}

/*
 * @tc.name: setCustomHeightTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, setCustomHeightTestInvalidValues, TestSize.Level1)
{
    typedef std::pair<Ark_Length, std::string> OneCustomHeightTestStep;
    static const std::vector<OneCustomHeightTestStep> testPlan = {
        { Converter::ArkValue<Ark_Length>(-1.0f),  ATTRIBUTE_CUSTOM_HEIGHT_DEFAULT_VALUE },
        { Converter::ArkValue<Ark_Length>(-2.45f), ATTRIBUTE_CUSTOM_HEIGHT_DEFAULT_VALUE },
        { Converter::ArkValue<Ark_Length>(-5.0_px), ATTRIBUTE_CUSTOM_HEIGHT_DEFAULT_VALUE },
    };
    modifier_->setType(node_, Converter::ArkValue<Ark_PanelType>(ARK_PANEL_TYPE_CUSTOM));

    for (const auto &[lengthValue, expected]: testPlan) {
        auto value = Converter::ArkUnion<Ark_Union_Dimension_PanelHeight, Ark_Length>(lengthValue);
        modifier_->setCustomHeight(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_CUSTOM_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setOnChangeTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setOnChangeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SlidingPanelEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t width;
        int32_t height;
        Ark_PanelMode mode;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    EventsTracker::panelEventReceiver.onChange = [](Ark_Int32 nodeId,
        const Ark_Number width,
        const Ark_Number height,
        const Ark_PanelMode mode)
    {
        checkEvent = {
            .nodeId = nodeId,
            .width = Converter::Convert<Ark_Int32>(width),
            .height = Converter::Convert<Ark_Int32>(height),
            .mode = Converter::Convert<Ark_PanelMode>(mode)
        };
    };

    ASSERT_NE(modifier_->setOnChange, nullptr);

    modifier_->setOnChange(node_, {});

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireSizeChangeEvent(PanelMode::HALF, 123, 321);
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->mode, Ark_PanelMode::ARK_PANEL_MODE_HALF);
    EXPECT_EQ(checkEvent->width, 123);
    EXPECT_EQ(checkEvent->height, 321);
}

/*
 * @tc.name: setOnHeightChangeTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PanelModifierTest, DISABLED_setOnHeightChangeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SlidingPanelEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t index;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    EventsTracker::panelEventReceiver.onHeightChange = [](Ark_Int32 nodeId, const Ark_Number index)
    {
        checkEvent = {
            .nodeId = nodeId,
            .index = Converter::Convert<Ark_Int32>(index)
        };
    };

    ASSERT_NE(modifier_->setOnHeightChange, nullptr);

    modifier_->setOnHeightChange(node_, {});

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireHeightChangeEvent(123);
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->index, 123);
}
} // namespace OHOS::Ace::NG

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

#include "gauge_modifier_test.h"

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "arkoala_api_generated.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_TRACK_SHADOW_NAME = "trackShadow";
    const auto ATTRIBUTE_INDICATOR_NAME = "indicator";
    const auto ATTRIBUTE_VALUE_NAME = "value";
    const auto ATTRIBUTE_VALUE_DEFAULT_VALUE = "0.00";
    const auto ATTRIBUTE_MIN_NAME = "min";
    const auto ATTRIBUTE_MIN_DEFAULT_VALUE = "0.00";
    const auto ATTRIBUTE_MAX_NAME = "max";
    const auto ATTRIBUTE_MAX_DEFAULT_VALUE = "100.00";
    const auto ATTRIBUTE_START_ANGLE_ANGLE_NAME = "startAngle";
    const auto ATTRIBUTE_START_ANGLE_ANGLE_DEFAULT_VALUE = "0.00";
    const auto ATTRIBUTE_END_ANGLE_ANGLE_NAME = "endAngle";
    const auto ATTRIBUTE_END_ANGLE_ANGLE_DEFAULT_VALUE = "360.00";
    const auto ATTRIBUTE_STROKE_WIDTH_LENGTH_NAME = "strokeWidth";
    const auto ATTRIBUTE_STROKE_WIDTH_LENGTH_DEFAULT_VALUE = "";
    const auto ATTRIBUTE_COLORS_NAME = "colors";
    const auto ATTRIBUTE_COLORS_DEFAULT_VALUE = "[\"#FF64BB5C\",\"#FFF7CE00\",\"#FFE84026\"]";
    const auto ATTRIBUTE_DESCRIPTION_NAME = "description";
    const auto ATTRIBUTE_DESCRIPTION_DEFAULT_VALUE = "";
    const auto ATTRIBUTE_TRACK_SHADOW_RADIUS_NAME = "radius";
    const auto ATTRIBUTE_TRACK_SHADOW_RADIUS_DEFAULT_VALUE = "20.000000";
    const auto ATTRIBUTE_TRACK_SHADOW_OFFSET_X_NAME = "offsetX";
    const auto ATTRIBUTE_TRACK_SHADOW_OFFSET_X_DEFAULT_VALUE = "5.000000";
    const auto ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_NAME = "offsetY";
    const auto ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_DEFAULT_VALUE = "5.000000";
    const auto ATTRIBUTE_INDICATOR_ICON_NAME = "icon";
    const auto ATTRIBUTE_INDICATOR_ICON_DEFAULT_VALUE = "SystemStyle";
    const auto ATTRIBUTE_INDICATOR_SPACE_NAME = "space";
    const auto ATTRIBUTE_INDICATOR_SPACE_DEFAULT_VALUE = "8.00vp";
    const auto ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_NAME = "isSensitive";
    const auto ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_DEFAULT_VALUE = "0";

    const auto COLOR_BY_NAME = Color(0xFF5A0F78);
    const auto COLOR_BY_ID = Color(0xABCDEF01);
    const auto RES_COLOR_NAME = NamedResourceId{"test_color", NodeModifier::ResourceType::COLOR};
    const auto RES_COLOR_ID = IntResourceId{54321, NodeModifier::ResourceType::COLOR};
    const auto RES_VALUE_NAME = NamedResourceId{"test_value", NodeModifier::ResourceType::FLOAT};
    const auto RES_VALUE_ID = IntResourceId{1, NodeModifier::ResourceType::FLOAT};
    const auto ICON_PATH = "path/test_icon.svg";
    const auto RES_ICON_NAME = NamedResourceId{"icon", NodeModifier::ResourceType::STRING};
} // namespace

class GaugeModifierTest : public ModifierTestBase<
    GENERATED_ArkUIGaugeModifier, &GENERATED_ArkUINodeModifiers::getGaugeModifier, GENERATED_ARKUI_GAUGE> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        // set test values to Theme Pattern as data for the Theme building
        AddResource(RES_COLOR_NAME, COLOR_BY_NAME);
        AddResource(RES_COLOR_ID, COLOR_BY_ID);
        AddResource(RES_VALUE_NAME, 5.0f);
        AddResource(RES_VALUE_ID, 1.0f);
        AddResource(RES_ICON_NAME, ICON_PATH);
    }
};

//     value: Ark_Number [false]
//     min: Opt_Number [true]
//     max: Opt_Number [true]

/*
 * @tc.name: setGaugeOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setGaugeOptionsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_VALUE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_VALUE_DEFAULT_VALUE);

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MIN_DEFAULT_VALUE);

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MAX_DEFAULT_VALUE);
}

// Valid values for attribute 'value' of method 'setGaugeOptions'
static std::vector<std::tuple<std::string, Ark_Number, std::string>> setGaugeOptionsValueValidValues = {
    {"20.15f", Converter::ArkValue<Ark_Number>(20.15f), "20.15"},
    {"499.0f", Converter::ArkValue<Ark_Number>(499.0f), "499.00"},
    {"-99.0f", Converter::ArkValue<Ark_Number>(-99.0f), "-99.00"},
    {"-100.0f", Converter::ArkValue<Ark_Number>(-100.0f), "-100.00"},
    {"500.0f", Converter::ArkValue<Ark_Number>(500.0f), "500.00"},
    {"-999.0f", Converter::ArkValue<Ark_Number>(-999.0f), "-100.00"},
    {"1000.0f", Converter::ArkValue<Ark_Number>(1000.0f), "-100.00"},
};

// Valid values for attribute 'min' of method 'setGaugeOptions'
static std::vector<std::tuple<std::string, Opt_Number, std::string>> setGaugeOptionsMinValidValues = {
    {"-100.0f", Converter::ArkValue<Opt_Number>(-100.0f), "-100.00"},
    {"20.15f", Converter::ArkValue<Opt_Number>(20.15f), "20.15"},
    {"-9999.0f", Converter::ArkValue<Opt_Number>(-9999.0f), "-9999.00"},
    {"undefined", Converter::ArkValue<Opt_Number>(), "0.00"},
    {"501.0f", Converter::ArkValue<Opt_Number>(501.0f), "0.00"},
};

// Valid values for attribute 'max' of method 'setGaugeOptions'
static std::vector<std::tuple<std::string, Opt_Number, std::string>> setGaugeOptionsMaxValidValues = {
    {"500.0f", Converter::ArkValue<Opt_Number>(500.0f), "500.00"},
    {"-99.0f", Converter::ArkValue<Opt_Number>(-99.0f), "-99.00"},
    {"100.0f", Converter::ArkValue<Opt_Number>(100.0f), "100.00"},
    {"undefined", Converter::ArkValue<Opt_Number>(), "100.00"},
    {"-110.0f", Converter::ArkValue<Opt_Number>(-110.0f), "100.00"},
};

/*
 * @tc.name: setGaugeOptionsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setGaugeOptionsTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeOptions inputValueOptions;
    Ark_GaugeOptions initValueOptions;

    // Initial setup
    initValueOptions.value = std::get<1>(setGaugeOptionsValueValidValues[0]);
    initValueOptions.min = std::get<1>(setGaugeOptionsMinValidValues[0]);
    initValueOptions.max = std::get<1>(setGaugeOptionsMaxValidValues[0]);

    // Verifying attribute's 'value'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setGaugeOptionsValueValidValues) {
        inputValueOptions.value = std::get<1>(value);
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_VALUE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'min'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setGaugeOptionsMinValidValues) {
        inputValueOptions.min = std::get<1>(value);
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'max'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setGaugeOptionsMaxValidValues) {
        inputValueOptions.max = std::get<1>(value);
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'min' of method 'setGaugeOptions'
static std::vector<std::tuple<std::string, Opt_Number>> setGaugeOptionsMinInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_Number>(Ark_Empty())},
};

// Invalid values for attribute 'max' of method 'setGaugeOptions'
static std::vector<std::tuple<std::string, Opt_Number>> setGaugeOptionsMaxInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_Number>(Ark_Empty())},
};

/*
 * @tc.name: setGaugeOptionsTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setGaugeOptionsTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeOptions inputValueOptions;
    Ark_GaugeOptions initValueOptions;

    // Initial setup
    initValueOptions.value = std::get<1>(setGaugeOptionsValueValidValues[0]);
    initValueOptions.min = std::get<1>(setGaugeOptionsMinValidValues[0]);
    initValueOptions.max = std::get<1>(setGaugeOptionsMaxValidValues[0]);

    // Verifying attribute's 'min'  values
    for (auto&& value: setGaugeOptionsMinInvalidValues) {
        inputValueOptions = initValueOptions;
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        inputValueOptions.min = std::get<1>(value);
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_NAME);
        expectedStr = ATTRIBUTE_MIN_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'max'  values
    for (auto&& value: setGaugeOptionsMaxInvalidValues) {
        inputValueOptions = initValueOptions;
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        inputValueOptions.max = std::get<1>(value);
        modifier_->setGaugeOptions(node_, &inputValueOptions);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_NAME);
        expectedStr = ATTRIBUTE_MAX_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setValueTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setValueTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_VALUE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_VALUE_DEFAULT_VALUE);
}

// Valid values for attribute 'value' of method 'value'
static std::vector<std::tuple<std::string, Ark_Number, std::string>> valueValueValidValues = {
    {"20.0f", Converter::ArkValue<Ark_Number>(20.0f), "20.00"},
    {"499.0f", Converter::ArkValue<Ark_Number>(499.0f), "499.00"},
    {"-100.0f", Converter::ArkValue<Ark_Number>(-100.0f), "-100.00"},
    {"0.50f", Converter::ArkValue<Ark_Number>(0.50f), "0.50"},
    {"99.0f", Converter::ArkValue<Ark_Number>(99.0f), "99.00"},
    {"100.0f", Converter::ArkValue<Ark_Number>(100.0f), "100.00"},
};

/*
 * @tc.name: setValueTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setValueTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Number inputValueValue;
    Ark_Number initValueValue;

    // Initial setup
    initValueValue = std::get<1>(valueValueValidValues[0]);

    // Verifying attribute's  values
    inputValueValue = initValueValue;
    for (auto&& value: valueValueValidValues) {
        inputValueValue = std::get<1>(value);
        modifier_->setValue(node_, &inputValueValue);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_VALUE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setStartAngleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setStartAngleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_START_ANGLE_ANGLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_START_ANGLE_ANGLE_DEFAULT_VALUE);
}

// Valid values for attribute 'startAngle' and 'endAngle' of method 'startAngle' and 'endAngle'
static std::vector<std::tuple<std::string, Ark_Number, std::string>> angleValidValues = {
    {"20.0f", Converter::ArkValue<Ark_Number>(20.0f), "20.00"},
    {"360.0f", Converter::ArkValue<Ark_Number>(360.0f), "360.00"},
    {"710.0f", Converter::ArkValue<Ark_Number>(710.0f), "710.00"},
    {"720.0f", Converter::ArkValue<Ark_Number>(0.0f), "0.00"},
    {"730.0f", Converter::ArkValue<Ark_Number>(10.0f), "10.00"},
    {"-730.0f", Converter::ArkValue<Ark_Number>(-10.0f), "-10.00"},
    {"-720.0f", Converter::ArkValue<Ark_Number>(0.0f), "0.00"},
    {"-600.0f", Converter::ArkValue<Ark_Number>(-600.0f), "-600.00"},
    {"-10.0f", Converter::ArkValue<Ark_Number>(-10.0f), "-10.00"},
};

/*
 * @tc.name: setStartAngleTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setStartAngleTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Number inputValueStartAngleAngle;
    Ark_Number initValueStartAngleAngle;

    // Initial setup
    initValueStartAngleAngle = std::get<1>(angleValidValues[0]);

    // Verifying attribute's  values
    inputValueStartAngleAngle = initValueStartAngleAngle;
    for (auto&& value: angleValidValues) {
        inputValueStartAngleAngle = std::get<1>(value);
        modifier_->setStartAngle(node_, &inputValueStartAngleAngle);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_START_ANGLE_ANGLE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setEndAngleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setEndAngleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_END_ANGLE_ANGLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_END_ANGLE_ANGLE_DEFAULT_VALUE);
}

/*
 * @tc.name: setEndAngleTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setEndAngleTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Number inputValueEndAngleAngle;
    Ark_Number initValueEndAngleAngle;

    // Initial setup
    initValueEndAngleAngle = std::get<1>(angleValidValues[0]);

    // Verifying attribute's  values
    inputValueEndAngleAngle = initValueEndAngleAngle;
    for (auto&& value: angleValidValues) {
        inputValueEndAngleAngle = std::get<1>(value);
        modifier_->setEndAngle(node_, &inputValueEndAngleAngle);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_END_ANGLE_ANGLE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setColorsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setColorsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_COLORS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_COLORS_DEFAULT_VALUE);
}

namespace {
    const auto COLORS_ENUM_RED = Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_RED);
    const auto COLORS_NUMBER_GREEN = Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0xF000FF00);
    const auto COLORS_STRING_BLUE = Converter::ArkUnion<Ark_ResourceColor, Ark_String>("rgba(0, 0, 255, 0.5)");
    const auto COLORS_RES_BY_NAME = Converter::ArkUnion<Ark_ResourceColor, Ark_Resource>(
        CreateResource(RES_COLOR_NAME));
    const auto COLORS_RES_BY_ID = Converter::ArkUnion<Ark_ResourceColor, Ark_Resource>(
        CreateResource(RES_COLOR_ID));

    const std::vector<std::tuple<Ark_Union_ResourceColor_LinearGradient, Ark_Number>> COLORS_TUPLE_ARRAY {
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_ENUM_RED),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_NUMBER_GREEN),
            Converter::ArkValue<Ark_Number>(0.25f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_STRING_BLUE),
            Converter::ArkValue<Ark_Number>(0.5f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_RES_BY_NAME),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_RES_BY_ID),
            Converter::ArkValue<Ark_Number>(1.0f)},
    };
    Converter::ArkArrayHolder<Array_Tuple_Union_ResourceColor_LinearGradient_Number>
        ColorsTupleArrayHolder(COLORS_TUPLE_ARRAY);

    const std::vector<std::tuple<Ark_Union_ResourceColor_LinearGradient, Ark_Number>> COLORS_TUPLE_BIG_ARRAY {
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_ENUM_RED),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_NUMBER_GREEN),
            Converter::ArkValue<Ark_Number>(2.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_STRING_BLUE),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_ENUM_RED),
            Converter::ArkValue<Ark_Number>(2.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_NUMBER_GREEN),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_STRING_BLUE),
            Converter::ArkValue<Ark_Number>(2.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_ENUM_RED),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_NUMBER_GREEN),
            Converter::ArkValue<Ark_Number>(2.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_STRING_BLUE),
            Converter::ArkValue<Ark_Number>(1.0f)},
        {Converter::ArkUnion<Ark_Union_ResourceColor_LinearGradient, Ark_ResourceColor>(COLORS_ENUM_RED),
            Converter::ArkValue<Ark_Number>(2.0f)},
    };
    Converter::ArkArrayHolder<Array_Tuple_Union_ResourceColor_LinearGradient_Number>
        ColorsTupleBigArrayHolder(COLORS_TUPLE_BIG_ARRAY);
} // namespace

// Valid values for attribute of method 'colors'
static std::vector<std::tuple<std::string, Ark_Type_GaugeAttribute_colors_colors, std::string>> colorsValidValues = {
    {
        "ARK_COLOR_RED",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors, Ark_ResourceColor>(COLORS_ENUM_RED),
        "#FFFF0000"
    },
    {
        "0xF000FF00",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors, Ark_ResourceColor>(COLORS_NUMBER_GREEN),
        "#F000FF00"
    },
    {
        "rgba(0, 0, 255, 0.5)",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors, Ark_ResourceColor>(COLORS_STRING_BLUE),
        "#800000FF"
    },
    {
        "RES_COLOR_NAME",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors, Ark_ResourceColor>(COLORS_RES_BY_NAME),
        COLOR_BY_NAME.ColorToString()
    },
    {
        "RES_COLOR_ID",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors, Ark_ResourceColor>(COLORS_RES_BY_ID),
        COLOR_BY_ID.ColorToString()
    },
    {
        "COLORS_TUPLE_ARRAY",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors,
            Array_Tuple_Union_ResourceColor_LinearGradient_Number>(ColorsTupleArrayHolder.ArkValue()),
        "[[[[\"#FFFF0000\",\"0.000000\"]],1],"
            "[[[\"#F000FF00\",\"0.000000\"]],0.25],"
            "[[[\"#800000FF\",\"0.000000\"]],0.5],"
            "[[[\"#FF5A0F78\",\"0.000000\"]],1],"
            "[[[\"#ABCDEF01\",\"0.000000\"]],1]]"
    },
    {
        "COLORS_TUPLE_BIG_ARRAY",
        Converter::ArkUnion<Ark_Type_GaugeAttribute_colors_colors,
            Array_Tuple_Union_ResourceColor_LinearGradient_Number>(ColorsTupleBigArrayHolder.ArkValue()),
        "[[[[\"#FFFF0000\",\"0.000000\"]],1],"
            "[[[\"#F000FF00\",\"0.000000\"]],2],"
            "[[[\"#800000FF\",\"0.000000\"]],1],"
            "[[[\"#FFFF0000\",\"0.000000\"]],2],"
            "[[[\"#F000FF00\",\"0.000000\"]],1],"
            "[[[\"#800000FF\",\"0.000000\"]],2],"
            "[[[\"#FFFF0000\",\"0.000000\"]],1],"
            "[[[\"#F000FF00\",\"0.000000\"]],2],"
            "[[[\"#800000FF\",\"0.000000\"]],1]]"
    },
};

/*
 * @tc.name: setColorsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setColorsTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Type_GaugeAttribute_colors_colors inputColors;
    Ark_Type_GaugeAttribute_colors_colors initColors;

    // Initial setup
    initColors = std::get<1>(colorsValidValues[0]);

    // Verifying attribute's colors
    inputColors = initColors;
    for (auto&& value: colorsValidValues) {
        inputColors = std::get<1>(value);
        modifier_->setColors(node_, &inputColors);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_COLORS_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setStrokeWidthTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setStrokeWidthTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_WIDTH_LENGTH_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_STROKE_WIDTH_LENGTH_DEFAULT_VALUE);
}

// Valid values for attribute 'strokeWidthLength' of method 'strokeWidth'
static std::vector<std::tuple<std::string, Ark_Length, std::string>> strokeWidthStrokeWidthLengthValidValues = {
    {"0.0f", Converter::ArkValue<Ark_Length>(0.0f), "0.00vp"},
    {"5.0f", Converter::ArkValue<Ark_Length>(5.0f), "5.00vp"},
    {"5.5f", Converter::ArkValue<Ark_Length>(5.5f), "5.50vp"},
    {"0.0_vp", Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp"},
    {"15.0_vp", Converter::ArkValue<Ark_Length>(15.0_vp), "15.00vp"},
    {"15.5_vp", Converter::ArkValue<Ark_Length>(15.5_vp), "15.50vp"},
    {"0.0_px", Converter::ArkValue<Ark_Length>(0.0_px), "0.00px"},
    {"25.0_px", Converter::ArkValue<Ark_Length>(25.0_px), "25.00px"},
    {"25.5_px", Converter::ArkValue<Ark_Length>(25.5_px), "25.50px"},
    {"0.0_fp", Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp"},
    {"35.0_fp", Converter::ArkValue<Ark_Length>(35.0_fp), "35.00fp"},
    {"35.5_fp", Converter::ArkValue<Ark_Length>(35.5_fp), "35.50fp"},
};

/*
 * @tc.name: setStrokeWidthTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setStrokeWidthTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Length inputValueStrokeWidthLength;
    Ark_Length initValueStrokeWidthLength;

    // Initial setup
    initValueStrokeWidthLength = std::get<1>(strokeWidthStrokeWidthLengthValidValues[0]);

    // Verifying attribute's  values
    inputValueStrokeWidthLength = initValueStrokeWidthLength;
    for (auto&& value: strokeWidthStrokeWidthLengthValidValues) {
        inputValueStrokeWidthLength = std::get<1>(value);
        modifier_->setStrokeWidth(node_, &inputValueStrokeWidthLength);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_WIDTH_LENGTH_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Valid values for attribute 'strokeWidthLength' of method 'strokeWidth'
static std::vector<std::tuple<std::string, Ark_Length>> strokeWidthStrokeWidthLengthInvalidValues = {
    {"10.0_pct", Converter::ArkValue<Ark_Length>(10.0_pct)},
    {"0.0_pct", Converter::ArkValue<Ark_Length>(0.0_pct)},
    {"-5.0f", Converter::ArkValue<Ark_Length>(-5.0f)},
    {"-15.5_vp", Converter::ArkValue<Ark_Length>(-15.5_vp)},
    {"-25.0_px", Converter::ArkValue<Ark_Length>(-25.0_px)},
    {"-0.5_fp", Converter::ArkValue<Ark_Length>(-0.5_fp)},
};

/*
 * @tc.name: setStrokeWidthTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setStrokeWidthTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Length inputValueStrokeWidthLength;
    Ark_Length initValueStrokeWidthLength;

    // Initial setup
    initValueStrokeWidthLength = std::get<1>(strokeWidthStrokeWidthLengthValidValues[0]);
    // Verifying attribute's 'min'  values
    for (auto&& value: strokeWidthStrokeWidthLengthInvalidValues) {
        inputValueStrokeWidthLength = initValueStrokeWidthLength;
        modifier_->setStrokeWidth(node_, &inputValueStrokeWidthLength);
        inputValueStrokeWidthLength = std::get<1>(value);
        modifier_->setStrokeWidth(node_, &inputValueStrokeWidthLength);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_WIDTH_LENGTH_NAME);
        expectedStr = ATTRIBUTE_STROKE_WIDTH_LENGTH_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setDescriptionTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setDescriptionTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DESCRIPTION_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_DESCRIPTION_DEFAULT_VALUE);
}

/*
 * @tc.name: setTrackShadowTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setTrackShadowTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultTrackShadow
        = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_RADIUS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TRACK_SHADOW_RADIUS_DEFAULT_VALUE);

    resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_X_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TRACK_SHADOW_OFFSET_X_DEFAULT_VALUE);

    resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_DEFAULT_VALUE);
}

// Valid values for attribute 'radius' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource, std::string>> trackShadowRadiusValidValues = {
    {"0.05f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(0.05f)),
        "0.050000"},
    {"10.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(10.0f)),
        "10.000000"},
    {"100.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(100.0f)),
        "100.000000"},
    {"5.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_NAME)),
        "5.000000"},
    {"1.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_ID)),
        "1.000000"},
};

// Valid values for attribute 'offsetX' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource, std::string>> trackShadowOffsetXValidValues = {
    {"0.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0f)),
        "0.000000"},
    {"10.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(10.0f)),
        "10.000000"},
    {"-0.5f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(-0.5f)),
        "-0.500000"},
    {"1.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_ID)),
        "1.000000"},
    {"5.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_NAME)),
        "5.000000"},
};

// Valid values for attribute 'offsetY' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource, std::string>> trackShadowOffsetYValidValues = {
    {"0.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0f)),
        "0.000000"},
    {"-100.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(-100.0f)),
        "-100.000000"},
    {"5.5f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(5.5f)),
        "5.500000"},
    {"5.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_NAME)),
        "5.000000"},
    {"1.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Resource>(CreateResource(RES_VALUE_ID)),
        "1.000000"},
};

/*
 * @tc.name: setTrackShadowTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setTrackShadowTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::unique_ptr<JsonValue> resultTrackShadow;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeShadowOptions inputValueTrackShadow;
    Ark_GaugeShadowOptions initValueTrackShadow;

    // Initial setup
    initValueTrackShadow.radius = std::get<1>(trackShadowRadiusValidValues[0]);
    initValueTrackShadow.offsetX = std::get<1>(trackShadowOffsetXValidValues[0]);
    initValueTrackShadow.offsetY = std::get<1>(trackShadowOffsetYValidValues[0]);

    // Verifying attribute's 'radius'  values
    inputValueTrackShadow = initValueTrackShadow;
    for (auto&& value: trackShadowRadiusValidValues) {
        inputValueTrackShadow.radius = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_RADIUS_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'offsetX'  values
    inputValueTrackShadow = initValueTrackShadow;
    for (auto&& value: trackShadowOffsetXValidValues) {
        inputValueTrackShadow.offsetX = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_X_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'offsetY'  values
    inputValueTrackShadow = initValueTrackShadow;
    for (auto&& value: trackShadowOffsetYValidValues) {
        inputValueTrackShadow.offsetY = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'radius' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource>> trackShadowRadiusInvalidValues = {
    {"Ark_Empty()", Converter::ArkUnion<Opt_Union_Number_Resource>(Ark_Empty())},
    {"nullptr", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Empty>(nullptr)},
    {"0.0f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0f))},
    {"-20.5f", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(Converter::ArkValue<Ark_Number>(-20.5f))},
};

// Invalid values for attribute 'offsetX' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource>> trackShadowOffsetXInvalidValues = {
    {"Ark_Empty()", Converter::ArkUnion<Opt_Union_Number_Resource>(Ark_Empty())},
    {"nullptr", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Empty>(nullptr)},
};

// Invalid values for attribute 'offsetY' of method 'trackShadow'
static std::vector<std::tuple<std::string, Opt_Union_Number_Resource>> trackShadowOffsetYInvalidValues = {
    {"Ark_Empty()", Converter::ArkUnion<Opt_Union_Number_Resource>(Ark_Empty())},
    {"nullptr", Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Empty>(nullptr)},
};

/*
 * @tc.name: setTrackShadowTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setTrackShadowTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::unique_ptr<JsonValue> resultTrackShadow;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeShadowOptions inputValueTrackShadow;
    Ark_GaugeShadowOptions initValueTrackShadow;

    // Initial setup
    initValueTrackShadow.radius = std::get<1>(trackShadowRadiusValidValues[0]);
    initValueTrackShadow.offsetX = std::get<1>(trackShadowOffsetXValidValues[0]);
    initValueTrackShadow.offsetY = std::get<1>(trackShadowOffsetYValidValues[0]);

    // Verifying attribute's 'radius'  values
    for (auto&& value: trackShadowRadiusInvalidValues) {
        inputValueTrackShadow = initValueTrackShadow;
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        inputValueTrackShadow.radius = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_RADIUS_NAME);
        expectedStr = ATTRIBUTE_TRACK_SHADOW_RADIUS_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'offsetX'  values
    for (auto&& value: trackShadowOffsetXInvalidValues) {
        inputValueTrackShadow = initValueTrackShadow;
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        inputValueTrackShadow.offsetX = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_X_NAME);
        expectedStr = ATTRIBUTE_TRACK_SHADOW_OFFSET_X_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'offsetY'  values
    for (auto&& value: trackShadowOffsetYInvalidValues) {
        inputValueTrackShadow = initValueTrackShadow;
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        inputValueTrackShadow.offsetY = std::get<1>(value);
        modifier_->setTrackShadow(node_, &inputValueTrackShadow);
        jsonValue = GetJsonValue(node_);
        resultTrackShadow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TRACK_SHADOW_NAME);
        resultStr = GetAttrValue<std::string>(resultTrackShadow, ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_NAME);
        expectedStr = ATTRIBUTE_TRACK_SHADOW_OFFSET_Y_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setIndicatorTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setIndicatorTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultIndicator
        = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_INDICATOR_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_ICON_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_INDICATOR_ICON_DEFAULT_VALUE);

    resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_SPACE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_INDICATOR_SPACE_DEFAULT_VALUE);
}

// Valid values for attribute 'icon' of method 'indicator'
static std::vector<std::tuple<std::string, Opt_ResourceStr, std::string>> indicatorIconValidValues = {
    {
        "test/path/icon.svg",
        Converter::ArkUnion<Opt_ResourceStr, Ark_String>(Converter::ArkValue<Ark_String>("test/path/icon.svg")),
        "test/path/icon.svg"
    },
    {ICON_PATH, Converter::ArkUnion<Opt_ResourceStr, Ark_Resource>(CreateResource(RES_ICON_NAME)), ICON_PATH},
};

// Valid values for attribute 'space' of method 'indicator'
static std::vector<std::tuple<std::string, Opt_Length, std::string>> indicatorSpaceValidValues = {
    {"5.0f", Converter::ArkValue<Opt_Length>(5.0f), "5.00vp"},
    {"5.5f", Converter::ArkValue<Opt_Length>(5.5f), "5.50vp"},
    {"15.0_vp", Converter::ArkValue<Opt_Length>(15.0_vp), "15.00vp"},
    {"15.5_vp", Converter::ArkValue<Opt_Length>(15.5_vp), "15.50vp"},
    {"25.0_px", Converter::ArkValue<Opt_Length>(25.0_px), "25.00px"},
    {"25.5_px", Converter::ArkValue<Opt_Length>(25.5_px), "25.50px"},
    {"35.0_fp", Converter::ArkValue<Opt_Length>(35.0_fp), "35.00fp"},
    {"35.5_fp", Converter::ArkValue<Opt_Length>(35.5_fp), "35.50fp"},
};

/*
 * @tc.name: setIndicatorTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setIndicatorTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::unique_ptr<JsonValue> resultIndicator;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeIndicatorOptions inputValueIndicator;
    Ark_GaugeIndicatorOptions initValueIndicator;

    // Initial setup
    initValueIndicator.icon = std::get<1>(indicatorIconValidValues[0]);
    initValueIndicator.space = std::get<1>(indicatorSpaceValidValues[0]);

    // Verifying attribute's 'icon'  values
    inputValueIndicator = initValueIndicator;
    for (auto&& value: indicatorIconValidValues) {
        inputValueIndicator.icon = std::get<1>(value);
        modifier_->setIndicator(node_, &inputValueIndicator);
        jsonValue = GetJsonValue(node_);
        resultIndicator = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_INDICATOR_NAME);
        resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_ICON_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'space'  values
    inputValueIndicator = initValueIndicator;
    for (auto&& value: indicatorSpaceValidValues) {
        inputValueIndicator.space = std::get<1>(value);
        modifier_->setIndicator(node_, &inputValueIndicator);
        jsonValue = GetJsonValue(node_);
        resultIndicator = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_INDICATOR_NAME);
        resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_SPACE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'icon' of method 'indicator'
static std::vector<std::tuple<std::string, Opt_ResourceStr>> indicatorIconInvalidValues = {
    {"Ark_Empty()", Converter::ArkUnion<Opt_ResourceStr>(Ark_Empty())},
    {"nullptr", Converter::ArkUnion<Opt_ResourceStr, Ark_Empty>(nullptr)},
};

// Invalid values for attribute 'space' of method 'indicator'
static std::vector<std::tuple<std::string, Opt_Length>> indicatorSpaceInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_Length>(Ark_Empty())},
    {"-0.5_vp", Converter::ArkValue<Opt_Length>(-0.5_vp)},
    {"50.0_pct", Converter::ArkValue<Opt_Length>(50.0_pct)},
};

/*
 * @tc.name: setIndicatorTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setIndicatorTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::unique_ptr<JsonValue> resultIndicator;
    std::string resultStr;
    std::string expectedStr;
    Ark_GaugeIndicatorOptions inputValueIndicator;
    Ark_GaugeIndicatorOptions initValueIndicator;

    // Initial setup
    initValueIndicator.icon = std::get<1>(indicatorIconValidValues[0]);
    initValueIndicator.space = std::get<1>(indicatorSpaceValidValues[0]);

    // Verifying attribute's 'icon'  values
    for (auto&& value: indicatorIconInvalidValues) {
        inputValueIndicator = initValueIndicator;
        modifier_->setIndicator(node_, &inputValueIndicator);
        inputValueIndicator.icon = std::get<1>(value);
        modifier_->setIndicator(node_, &inputValueIndicator);
        jsonValue = GetJsonValue(node_);
        resultIndicator = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_INDICATOR_NAME);
        resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_ICON_NAME);
        expectedStr = ATTRIBUTE_INDICATOR_ICON_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'space'  values
    for (auto&& value: indicatorSpaceInvalidValues) {
        inputValueIndicator = initValueIndicator;
        modifier_->setIndicator(node_, &inputValueIndicator);
        inputValueIndicator.space = std::get<1>(value);
        modifier_->setIndicator(node_, &inputValueIndicator);
        jsonValue = GetJsonValue(node_);
        resultIndicator = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_INDICATOR_NAME);
        resultStr = GetAttrValue<std::string>(resultIndicator, ATTRIBUTE_INDICATOR_SPACE_NAME);
        expectedStr = ATTRIBUTE_INDICATOR_SPACE_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setPrivacySensitiveTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setPrivacySensitiveTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_DEFAULT_VALUE);
}

// Valid values for attribute 'privacySensitiveIsPrivacySensitiveMode' of method 'privacySensitive'
static std::vector<std::tuple<std::string, Opt_Boolean, std::string>>
privacySensitivePrivacySensitiveIsPrivacySensitiveModeValidValues = {
    {"true", Converter::ArkValue<Opt_Boolean>(true), "1"},
    {"false", Converter::ArkValue<Opt_Boolean>(false), "0"},
};

/*
 * @tc.name: setPrivacySensitiveTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
#ifdef WRONG_OPT
HWTEST_F(GaugeModifierTest, setPrivacySensitiveTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Opt_Boolean inputValuePrivacySensitiveIsPrivacySensitiveMode;

    for (auto&& value: privacySensitivePrivacySensitiveIsPrivacySensitiveModeValidValues) {
        inputValuePrivacySensitiveIsPrivacySensitiveMode = std::get<1>(value);
        modifier_->setPrivacySensitive(node_, &inputValuePrivacySensitiveIsPrivacySensitiveMode);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'privacySensitiveIsPrivacySensitiveMode' of method 'privacySensitive'
static std::vector<std::tuple<std::string, Opt_Boolean>>
privacySensitivePrivacySensitiveIsPrivacySensitiveModeInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_Boolean>(Ark_Empty())},
};

/*
 * @tc.name: setPrivacySensitiveTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, setPrivacySensitiveTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Opt_Boolean inputValuePrivacySensitiveIsPrivacySensitiveMode;
    Opt_Boolean initValuePrivacySensitiveIsPrivacySensitiveMode;

    // Initial setup
    initValuePrivacySensitiveIsPrivacySensitiveMode
        = std::get<1>(privacySensitivePrivacySensitiveIsPrivacySensitiveModeValidValues[0]);

    // Verifying attribute's  values
    for (auto&& value: privacySensitivePrivacySensitiveIsPrivacySensitiveModeInvalidValues) {
        inputValuePrivacySensitiveIsPrivacySensitiveMode = initValuePrivacySensitiveIsPrivacySensitiveMode;
        modifier_->setPrivacySensitive(node_, &inputValuePrivacySensitiveIsPrivacySensitiveMode);
        inputValuePrivacySensitiveIsPrivacySensitiveMode = std::get<1>(value);
        modifier_->setPrivacySensitive(node_, &inputValuePrivacySensitiveIsPrivacySensitiveMode);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_NAME);
        expectedStr = ATTRIBUTE_PRIVACY_SENSITIVE_IS_PRIVACY_SENSITIVE_MODE_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}
#endif // WRONG_OPT

/*
 * @tc.name: setContentModifierTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GaugeModifierTest, DISABLED_setContentModifierTest, TestSize.Level1)
{
    // CustomObjects is not implemented yet!
}
} // namespace OHOS::Ace::NG
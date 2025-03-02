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

#include "text_modifier_test.h"

namespace OHOS::Ace::NG {
using namespace TestConst::Text;

/*
 * @tc.name: setTextOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setTextOptionsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultValue = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_VALUE_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTENT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CONTENT_DEFAULT_VALUE) << "Default value for attribute 'content'";

    resultStr = GetAttrValue<std::string>(resultValue, ATTRIBUTE_VALUE_I_CONTROLLER_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_VALUE_I_CONTROLLER_DEFAULT_VALUE) <<
        "Default value for attribute 'value.controller'";
}

/*
 * @tc.name: setTextOptionsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setTextOptionsTestValidValues, TestSize.Level1)
{
    FAIL() << "Need to properly configure fixtures in configuration file for proper test generation!";
}

/*
 * @tc.name: setFont0TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_SIZE_DEFAULT_VALUE) << "Default value for attribute 'font.size'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_WEIGHT_DEFAULT_VALUE) << "Default value for attribute 'font.weight'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_FAMILY_DEFAULT_VALUE) << "Default value for attribute 'font.family'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_STYLE_DEFAULT_VALUE) << "Default value for attribute 'font.style'";
}

/*
 * @tc.name: setFont0TestFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontSizeValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](
                          const std::string& input, const std::string& expectedStr, const Opt_Length& value) {
        Ark_Font inputValueFont = initValueFont;

        inputValueFont.size = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setFont0, attribute: font.size";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureLengthNonNegNonPctValidValues) {
        checkValue(input, expected, ArkValue<Opt_Length>(value));
    }
}

/*
 * @tc.name: setFont0TestFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](const std::string& input, const Opt_Length& value) {
        Ark_Font inputValueFont = initValueFont;

        modifier_->setFont0(node_, &inputValueFont);
        inputValueFont.size = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_SIZE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.size";
    };

    for (auto& [input, value] : Fixtures::testFixtureLengthNonNegNonPctInvalidValues) {
        checkValue(input, ArkValue<Opt_Length>(value));
    }
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Length>());
}

/*
 * @tc.name: setFont0TestFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontWeightValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](const std::string& input, const std::string& expectedStr,
                          const Opt_Union_FontWeight_Number_String& value) {
        Ark_Font inputValueFont = initValueFont;

        inputValueFont.weight = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.weight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value));
    }
}

/*
 * @tc.name: setFont0TestFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](
                          const std::string& input, const Opt_Union_FontWeight_Number_String& value) {
        Ark_Font inputValueFont = initValueFont;

        modifier_->setFont0(node_, &inputValueFont);
        inputValueFont.weight = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_WEIGHT_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.weight";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Union_FontWeight_Number_String>());
}

/*
 * @tc.name: setFont0TestFontFamilyValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontFamilyValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](const std::string& input, const std::string& expectedStr,
                          const Opt_Union_String_Resource& value) {
        Ark_Font inputValueFont = initValueFont;

        inputValueFont.family = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.family";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyResourceValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyStringValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setFont0TestFontFamilyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontFamilyInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](const std::string& input, const Opt_Union_String_Resource& value) {
        Ark_Font inputValueFont = initValueFont;

        modifier_->setFont0(node_, &inputValueFont);
        inputValueFont.family = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_FAMILY_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.family";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontFamilyResourceInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontFamilyStringInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Opt_Union_String_Resource, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Union_String_Resource>());
}

/*
 * @tc.name: setFont0TestFontStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontStyleValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](
                          const std::string& input, const std::string& expectedStr, const Opt_FontStyle& value) {
        Ark_Font inputValueFont = initValueFont;

        inputValueFont.style = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setFont0, attribute: font.style";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontStyleValidValues) {
        checkValue(input, expected, ArkValue<Opt_FontStyle>(value));
    }
}

/*
 * @tc.name: setFont0TestFontStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont0TestFontStyleInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueFont](const std::string& input, const Opt_FontStyle& value) {
        Ark_Font inputValueFont = initValueFont;

        modifier_->setFont0(node_, &inputValueFont);
        inputValueFont.style = value;
        modifier_->setFont0(node_, &inputValueFont);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_STYLE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont0, attribute: font.style";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontStyleInvalidValues) {
        checkValue(input, ArkValue<Opt_FontStyle>(value));
    }
}

/*
 * @tc.name: setFont1TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setFont1TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
    std::unique_ptr<JsonValue> resultOptions =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OPTIONS_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_SIZE_DEFAULT_VALUE) << "Default value for attribute 'font.size'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_WEIGHT_DEFAULT_VALUE) << "Default value for attribute 'font.weight'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_FAMILY_DEFAULT_VALUE) << "Default value for attribute 'font.family'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_STYLE_DEFAULT_VALUE) << "Default value for attribute 'font.style'";

    resultStr = GetAttrValue<std::string>(resultOptions, ATTRIBUTE_OPTIONS_I_ENABLE_VARIABLE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_OPTIONS_I_ENABLE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE) <<
        "Default value for attribute 'options.enableVariableFontWeight'";
}

/*
 * @tc.name: setFont1TestFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontSizeValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](
                          const std::string& input, const std::string& expectedStr, const Opt_Length& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueFont.size = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setFont1, attribute: font.size";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureLengthNonNegNonPctValidValues) {
        checkValue(input, expected, ArkValue<Opt_Length>(value));
    }
}

/*
 * @tc.name: setFont1TestFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](const std::string& input, const Opt_Length& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        inputValueFont.size = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_SIZE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.size";
    };

    for (auto& [input, value] : Fixtures::testFixtureLengthNonNegNonPctInvalidValues) {
        checkValue(input, ArkValue<Opt_Length>(value));
    }
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Length>());
}

/*
 * @tc.name: setFont1TestFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontWeightValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](const std::string& input,
                          const std::string& expectedStr, const Opt_Union_FontWeight_Number_String& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueFont.weight = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.weight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value));
    }
}

/*
 * @tc.name: setFont1TestFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](
                          const std::string& input, const Opt_Union_FontWeight_Number_String& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        inputValueFont.weight = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_WEIGHT_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.weight";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Union_FontWeight_Number_String>());
}

/*
 * @tc.name: setFont1TestFontFamilyValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontFamilyValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](const std::string& input,
                          const std::string& expectedStr, const Opt_Union_String_Resource& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueFont.family = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.family";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyResourceValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyStringValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setFont1TestFontFamilyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontFamilyInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](
                          const std::string& input, const Opt_Union_String_Resource& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        inputValueFont.family = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_FAMILY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_FAMILY_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.family";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontFamilyResourceInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontFamilyStringInvalidValues) {
        checkValue(input, ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Opt_Union_String_Resource, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Union_String_Resource>());
}

/*
 * @tc.name: setFont1TestFontStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontStyleValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](
                          const std::string& input, const std::string& expectedStr, const Opt_FontStyle& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueFont.style = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setFont1, attribute: font.style";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontStyleValidValues) {
        checkValue(input, expected, ArkValue<Opt_FontStyle>(value));
    }
}

/*
 * @tc.name: setFont1TestFontStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFont1TestFontStyleInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](const std::string& input, const Opt_FontStyle& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        inputValueFont.style = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_FONT_I_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_I_STYLE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont1, attribute: font.style";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontStyleInvalidValues) {
        checkValue(input, ArkValue<Opt_FontStyle>(value));
    }
}

/*
 * @tc.name: setFont1TestOptionsEnableVariableFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setFont1TestOptionsEnableVariableFontWeightValidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](
                          const std::string& input, const std::string& expectedStr, const Opt_Boolean& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        WriteTo(inputValueOptions).enableVariableFontWeight = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultOptions = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OPTIONS_NAME);
        auto resultStr = GetAttrValue<std::string>(resultOptions, ATTRIBUTE_OPTIONS_I_ENABLE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFont1, attribute: options.enableVariableFontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, expected, ArkValue<Opt_Boolean>(value));
    }
}

/*
 * @tc.name: setFont1TestOptionsEnableVariableFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setFont1TestOptionsEnableVariableFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Font initValueFont;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueFont.size = ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    initValueFont.weight = ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    initValueFont.family = ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    initValueFont.style = ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));
    WriteTo(initValueOptions).enableVariableFontWeight =
        ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));

    auto checkValue = [this, &initValueFont, &initValueOptions](const std::string& input, const Opt_Boolean& value) {
        Ark_Font inputValueFont = initValueFont;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        WriteTo(inputValueOptions).enableVariableFontWeight = value;
        modifier_->setFont1(node_, &inputValueFont, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultOptions = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OPTIONS_NAME);
        auto resultStr = GetAttrValue<std::string>(resultOptions, ATTRIBUTE_OPTIONS_I_ENABLE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_OPTIONS_I_ENABLE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFont1, attribute: options.enableVariableFontWeight";
    };

    // Check empty optional
    checkValue("undefined", ArkValue<Opt_Boolean>());
}

/*
 * @tc.name: setFontColorTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontColorTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE) << "Default value for attribute 'fontColor'";
}

/*
 * @tc.name: setFontColorTestFontColorValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontColorTestFontColorValidValues, TestSize.Level1)
{
    Ark_ResourceColor initValueFontColor;

    // Initial setup
    initValueFontColor =
        ArkUnion<Ark_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));

    auto checkValue = [this, &initValueFontColor](
                          const std::string& input, const std::string& expectedStr, const Ark_ResourceColor& value) {
        Ark_ResourceColor inputValueFontColor = initValueFontColor;

        inputValueFontColor = value;
        modifier_->setFontColor(node_, &inputValueFontColor);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFontColor, attribute: fontColor";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureColorsEnumValidValues) {
        checkValue(input, expected, ArkUnion<Ark_ResourceColor, Ark_Color>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsNumValidValues) {
        checkValue(input, expected, ArkUnion<Ark_ResourceColor, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsResValidValues) {
        checkValue(input, expected, ArkUnion<Ark_ResourceColor, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsStrValidValues) {
        checkValue(input, expected, ArkUnion<Ark_ResourceColor, Ark_String>(value));
    }
}

/*
 * @tc.name: setFontColorTestFontColorInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontColorTestFontColorInvalidValues, TestSize.Level1)
{
    Ark_ResourceColor initValueFontColor;

    // Initial setup
    initValueFontColor =
        ArkUnion<Ark_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));

    auto checkValue = [this, &initValueFontColor](const std::string& input, const Ark_ResourceColor& value) {
        Ark_ResourceColor inputValueFontColor = initValueFontColor;

        modifier_->setFontColor(node_, &inputValueFontColor);
        inputValueFontColor = value;
        modifier_->setFontColor(node_, &inputValueFontColor);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFontColor, attribute: fontColor";
    };

    for (auto& [input, value] : Fixtures::testFixtureColorsStrInvalidValues) {
        checkValue(input, ArkUnion<Ark_ResourceColor, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureColorsEnumInvalidValues) {
        checkValue(input, ArkUnion<Ark_ResourceColor, Ark_Color>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_ResourceColor, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontSizeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontSizeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE) << "Default value for attribute 'fontSize'";
}

/*
 * @tc.name: setFontSizeTestFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontSizeTestFontSizeValidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueFontSize;

    // Initial setup
    initValueFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueFontSize](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueFontSize = initValueFontSize;

        inputValueFontSize = value;
        modifier_->setFontSize(node_, &inputValueFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFontSize, attribute: fontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setFontSizeTestFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontSizeTestFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueFontSize;

    // Initial setup
    initValueFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueFontSize](
                          const std::string& input, const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueFontSize = initValueFontSize;

        modifier_->setFontSize(node_, &inputValueFontSize);
        inputValueFontSize = value;
        modifier_->setFontSize(node_, &inputValueFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFontSize, attribute: fontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setMinFontSizeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMinFontSizeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MIN_FONT_SIZE_DEFAULT_VALUE) << "Default value for attribute 'minFontSize'";
}

/*
 * @tc.name: setMinFontSizeTestMinFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setMinFontSizeTestMinFontSizeValidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueMinFontSize;

    // Initial setup
    initValueMinFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueMinFontSize](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueMinFontSize = initValueMinFontSize;

        inputValueMinFontSize = value;
        modifier_->setMinFontSize(node_, &inputValueMinFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setMinFontSize, attribute: minFontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setMinFontSizeTestMinFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMinFontSizeTestMinFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueMinFontSize;

    // Initial setup
    initValueMinFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueMinFontSize](
                          const std::string& input, const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueMinFontSize = initValueMinFontSize;

        modifier_->setMinFontSize(node_, &inputValueMinFontSize);
        inputValueMinFontSize = value;
        modifier_->setMinFontSize(node_, &inputValueMinFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_MIN_FONT_SIZE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setMinFontSize, attribute: minFontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setMaxFontSizeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxFontSizeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MAX_FONT_SIZE_DEFAULT_VALUE) << "Default value for attribute 'maxFontSize'";
}

/*
 * @tc.name: setMaxFontSizeTestMaxFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setMaxFontSizeTestMaxFontSizeValidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueMaxFontSize;

    // Initial setup
    initValueMaxFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueMaxFontSize](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueMaxFontSize = initValueMaxFontSize;

        inputValueMaxFontSize = value;
        modifier_->setMaxFontSize(node_, &inputValueMaxFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setMaxFontSize, attribute: maxFontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegNonPctValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setMaxFontSizeTestMaxFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxFontSizeTestMaxFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueMaxFontSize;

    // Initial setup
    initValueMaxFontSize = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueMaxFontSize](
                          const std::string& input, const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueMaxFontSize = initValueMaxFontSize;

        modifier_->setMaxFontSize(node_, &inputValueMaxFontSize);
        inputValueMaxFontSize = value;
        modifier_->setMaxFontSize(node_, &inputValueMaxFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_MAX_FONT_SIZE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setMaxFontSize, attribute: maxFontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegNonPctInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setMinFontScaleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMinFontScaleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SCALE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MIN_FONT_SCALE_DEFAULT_VALUE) << "Default value for attribute 'minFontScale'";
}

/*
 * @tc.name: setMinFontScaleTestMinFontScaleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setMinFontScaleTestMinFontScaleValidValues, TestSize.Level1)
{
    Ark_Union_Number_Resource initValueMinFontScale;

    // Initial setup
    initValueMinFontScale = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureMinFontScaleNumValidValues[0]));

    auto checkValue = [this, &initValueMinFontScale](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_Resource& value) {
        Ark_Union_Number_Resource inputValueMinFontScale = initValueMinFontScale;

        inputValueMinFontScale = value;
        modifier_->setMinFontScale(node_, &inputValueMinFontScale);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SCALE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setMinFontScale, attribute: minFontScale";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureMinFontScaleNumValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureMinFontScaleResValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_Resource, Ark_Resource>(value));
    }
}

/*
 * @tc.name: setMinFontScaleTestMinFontScaleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMinFontScaleTestMinFontScaleInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_Resource initValueMinFontScale;

    // Initial setup
    initValueMinFontScale = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureMinFontScaleNumValidValues[0]));

    auto checkValue = [this, &initValueMinFontScale](const std::string& input, const Ark_Union_Number_Resource& value) {
        Ark_Union_Number_Resource inputValueMinFontScale = initValueMinFontScale;

        modifier_->setMinFontScale(node_, &inputValueMinFontScale);
        inputValueMinFontScale = value;
        modifier_->setMinFontScale(node_, &inputValueMinFontScale);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MIN_FONT_SCALE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_MIN_FONT_SCALE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setMinFontScale, attribute: minFontScale";
    };

    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setMaxFontScaleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxFontScaleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SCALE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MAX_FONT_SCALE_DEFAULT_VALUE) << "Default value for attribute 'maxFontScale'";
}

/*
 * @tc.name: setMaxFontScaleTestMaxFontScaleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setMaxFontScaleTestMaxFontScaleValidValues, TestSize.Level1)
{
    Ark_Union_Number_Resource initValueMaxFontScale;

    // Initial setup
    initValueMaxFontScale = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureMaxFontScaleNumValidValues[0]));

    auto checkValue = [this, &initValueMaxFontScale](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_Resource& value) {
        Ark_Union_Number_Resource inputValueMaxFontScale = initValueMaxFontScale;

        inputValueMaxFontScale = value;
        modifier_->setMaxFontScale(node_, &inputValueMaxFontScale);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SCALE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setMaxFontScale, attribute: maxFontScale";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureMaxFontScaleNumValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureMaxFontScaleResValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_Resource, Ark_Resource>(value));
    }
}

/*
 * @tc.name: setMaxFontScaleTestMaxFontScaleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxFontScaleTestMaxFontScaleInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_Resource initValueMaxFontScale;

    // Initial setup
    initValueMaxFontScale = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureMaxFontScaleNumValidValues[0]));

    auto checkValue = [this, &initValueMaxFontScale](const std::string& input, const Ark_Union_Number_Resource& value) {
        Ark_Union_Number_Resource inputValueMaxFontScale = initValueMaxFontScale;

        modifier_->setMaxFontScale(node_, &inputValueMaxFontScale);
        inputValueMaxFontScale = value;
        modifier_->setMaxFontScale(node_, &inputValueMaxFontScale);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_FONT_SCALE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_MAX_FONT_SCALE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setMaxFontScale, attribute: maxFontScale";
    };

    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontStyleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontStyleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_STYLE_DEFAULT_VALUE) << "Default value for attribute 'fontStyle'";
}

/*
 * @tc.name: setFontStyleTestFontStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontStyleTestFontStyleValidValues, TestSize.Level1)
{
    Ark_FontStyle initValueFontStyle;

    // Initial setup
    initValueFontStyle = std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]);

    auto checkValue = [this, &initValueFontStyle](
                          const std::string& input, const std::string& expectedStr, const Ark_FontStyle& value) {
        Ark_FontStyle inputValueFontStyle = initValueFontStyle;

        inputValueFontStyle = value;
        modifier_->setFontStyle(node_, inputValueFontStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFontStyle, attribute: fontStyle";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontStyleValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setFontStyleTestFontStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontStyleTestFontStyleInvalidValues, TestSize.Level1)
{
    Ark_FontStyle initValueFontStyle;

    // Initial setup
    initValueFontStyle = std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]);

    auto checkValue = [this, &initValueFontStyle](const std::string& input, const Ark_FontStyle& value) {
        Ark_FontStyle inputValueFontStyle = initValueFontStyle;

        modifier_->setFontStyle(node_, inputValueFontStyle);
        inputValueFontStyle = value;
        modifier_->setFontStyle(node_, inputValueFontStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_STYLE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFontStyle, attribute: fontStyle";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontStyleInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setFontWeight0TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight0TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE) << "Default value for attribute 'fontWeight'";
}

/*
 * @tc.name: setFontWeight0TestFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight0TestFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        inputValueFontWeight = value;
        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFontWeight0, attribute: fontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value));
    }
}

/*
 * @tc.name: setFontWeight0TestFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight0TestFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](
                          const std::string& input, const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        inputValueFontWeight = value;
        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFontWeight0, attribute: fontWeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setLineSpacingTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setLineSpacingTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LINE_SPACING_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LINE_SPACING_DEFAULT_VALUE) << "Default value for attribute 'lineSpacing'";
}

/*
 * @tc.name: setLineSpacingTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setLineSpacingTestValidValues, TestSize.Level1)
{
    FAIL() << "Need to properly configure fixtures in configuration file for proper test generation!";
}

/*
 * @tc.name: setTextAlignTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextAlignTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TEXT_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TEXT_ALIGN_DEFAULT_VALUE) << "Default value for attribute 'textAlign'";
}

/*
 * @tc.name: setTextAlignTestTextAlignValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextAlignTestTextAlignValidValues, TestSize.Level1)
{
    Ark_TextAlign initValueTextAlign;

    // Initial setup
    initValueTextAlign = std::get<1>(Fixtures::testFixtureTextAlignTypeValidValues[0]);

    auto checkValue = [this, &initValueTextAlign](
                          const std::string& input, const std::string& expectedStr, const Ark_TextAlign& value) {
        Ark_TextAlign inputValueTextAlign = initValueTextAlign;

        inputValueTextAlign = value;
        modifier_->setTextAlign(node_, inputValueTextAlign);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TEXT_ALIGN_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setTextAlign, attribute: textAlign";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureTextAlignTypeValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setTextAlignTestTextAlignInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextAlignTestTextAlignInvalidValues, TestSize.Level1)
{
    Ark_TextAlign initValueTextAlign;

    // Initial setup
    initValueTextAlign = std::get<1>(Fixtures::testFixtureTextAlignTypeValidValues[0]);

    auto checkValue = [this, &initValueTextAlign](const std::string& input, const Ark_TextAlign& value) {
        Ark_TextAlign inputValueTextAlign = initValueTextAlign;

        modifier_->setTextAlign(node_, inputValueTextAlign);
        inputValueTextAlign = value;
        modifier_->setTextAlign(node_, inputValueTextAlign);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TEXT_ALIGN_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_TEXT_ALIGN_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setTextAlign, attribute: textAlign";
    };

    for (auto& [input, value] : Fixtures::testFixtureTextAlignTypeInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setLineHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setLineHeightTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LINE_HEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LINE_HEIGHT_DEFAULT_VALUE) << "Default value for attribute 'lineHeight'";
}

/*
 * @tc.name: setLineHeightTestLineHeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setLineHeightTestLineHeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueLineHeight;

    // Initial setup
    initValueLineHeight = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueLineHeight](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueLineHeight = initValueLineHeight;

        inputValueLineHeight = value;
        modifier_->setLineHeight(node_, &inputValueLineHeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LINE_HEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setLineHeight, attribute: lineHeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setLineHeightTestLineHeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setLineHeightTestLineHeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_String_Resource initValueLineHeight;

    // Initial setup
    initValueLineHeight = ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));

    auto checkValue = [this, &initValueLineHeight](
                          const std::string& input, const Ark_Union_Number_String_Resource& value) {
        Ark_Union_Number_String_Resource inputValueLineHeight = initValueLineHeight;

        modifier_->setLineHeight(node_, &inputValueLineHeight);
        inputValueLineHeight = value;
        modifier_->setLineHeight(node_, &inputValueLineHeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LINE_HEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LINE_HEIGHT_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setLineHeight, attribute: lineHeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setTextOverflowTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setTextOverflowTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultTextOverflow =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TEXT_OVERFLOW_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultTextOverflow, ATTRIBUTE_TEXT_OVERFLOW_I_OVERFLOW_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TEXT_OVERFLOW_I_OVERFLOW_DEFAULT_VALUE) <<
        "Default value for attribute 'textOverflow.overflow'";
}

/*
 * @tc.name: setTextOverflowTestTextOverflowOverflowValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setTextOverflowTestTextOverflowOverflowValidValues, TestSize.Level1)
{
    Ark_TextOverflowOptions initValueTextOverflow;

    // Initial setup
    initValueTextOverflow.overflow = std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]);

    auto checkValue = [this, &initValueTextOverflow](
                          const std::string& input, const std::string& expectedStr, const Ark_TextOverflow& value) {
        Ark_TextOverflowOptions inputValueTextOverflow = initValueTextOverflow;

        inputValueTextOverflow.overflow = value;
        modifier_->setTextOverflow(node_, &inputValueTextOverflow);
        auto jsonValue = GetJsonValue(node_);
        auto resultTextOverflow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TEXT_OVERFLOW_NAME);
        auto resultStr = GetAttrValue<std::string>(resultTextOverflow, ATTRIBUTE_TEXT_OVERFLOW_I_OVERFLOW_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setTextOverflow, attribute: textOverflow.overflow";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumTextOverflowValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setTextOverflowTestTextOverflowOverflowInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setTextOverflowTestTextOverflowOverflowInvalidValues, TestSize.Level1)
{
    Ark_TextOverflowOptions initValueTextOverflow;

    // Initial setup
    initValueTextOverflow.overflow = std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]);

    auto checkValue = [this, &initValueTextOverflow](const std::string& input, const Ark_TextOverflow& value) {
        Ark_TextOverflowOptions inputValueTextOverflow = initValueTextOverflow;

        modifier_->setTextOverflow(node_, &inputValueTextOverflow);
        inputValueTextOverflow.overflow = value;
        modifier_->setTextOverflow(node_, &inputValueTextOverflow);
        auto jsonValue = GetJsonValue(node_);
        auto resultTextOverflow = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TEXT_OVERFLOW_NAME);
        auto resultStr = GetAttrValue<std::string>(resultTextOverflow, ATTRIBUTE_TEXT_OVERFLOW_I_OVERFLOW_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_TEXT_OVERFLOW_I_OVERFLOW_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setTextOverflow, attribute: textOverflow.overflow";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumTextOverflowInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setFontFamilyTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontFamilyTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_FAMILY_DEFAULT_VALUE) << "Default value for attribute 'fontFamily'";
}

/*
 * @tc.name: setFontFamilyTestFontFamilyValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontFamilyTestFontFamilyValidValues, TestSize.Level1)
{
    Ark_Union_String_Resource initValueFontFamily;

    // Initial setup
    initValueFontFamily = ArkUnion<Ark_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));

    auto checkValue = [this, &initValueFontFamily](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_String_Resource& value) {
        Ark_Union_String_Resource inputValueFontFamily = initValueFontFamily;

        inputValueFontFamily = value;
        modifier_->setFontFamily(node_, &inputValueFontFamily);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setFontFamily, attribute: fontFamily";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyResourceValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyStringValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_String_Resource, Ark_String>(value));
    }
}

/*
 * @tc.name: setFontFamilyTestFontFamilyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontFamilyTestFontFamilyInvalidValues, TestSize.Level1)
{
    Ark_Union_String_Resource initValueFontFamily;

    // Initial setup
    initValueFontFamily = ArkUnion<Ark_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));

    auto checkValue = [this, &initValueFontFamily](const std::string& input, const Ark_Union_String_Resource& value) {
        Ark_Union_String_Resource inputValueFontFamily = initValueFontFamily;

        modifier_->setFontFamily(node_, &inputValueFontFamily);
        inputValueFontFamily = value;
        modifier_->setFontFamily(node_, &inputValueFontFamily);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_FAMILY_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setFontFamily, attribute: fontFamily";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontFamilyResourceInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontFamilyStringInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_String_Resource, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setMaxLinesTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxLinesTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_LINES_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_MAX_LINES_DEFAULT_VALUE) << "Default value for attribute 'maxLines'";
}

/*
 * @tc.name: setMaxLinesTestMaxLinesValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setMaxLinesTestMaxLinesValidValues, TestSize.Level1)
{
    Ark_Number initValueMaxLines;

    // Initial setup
    initValueMaxLines = std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]);

    auto checkValue = [this, &initValueMaxLines](
                          const std::string& input, const std::string& expectedStr, const Ark_Number& value) {
        Ark_Number inputValueMaxLines = initValueMaxLines;

        inputValueMaxLines = value;
        modifier_->setMaxLines(node_, &inputValueMaxLines);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_LINES_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setMaxLines, attribute: maxLines";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureNumberPosIntFloorValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setMaxLinesTestMaxLinesInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setMaxLinesTestMaxLinesInvalidValues, TestSize.Level1)
{
    Ark_Number initValueMaxLines;

    // Initial setup
    initValueMaxLines = std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]);

    auto checkValue = [this, &initValueMaxLines](const std::string& input, const Ark_Number& value) {
        Ark_Number inputValueMaxLines = initValueMaxLines;

        modifier_->setMaxLines(node_, &inputValueMaxLines);
        inputValueMaxLines = value;
        modifier_->setMaxLines(node_, &inputValueMaxLines);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_MAX_LINES_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_MAX_LINES_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setMaxLines, attribute: maxLines";
    };

    for (auto& [input, value] : Fixtures::testFixtureNumberPosIntFloorInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setDecorationTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultDecoration =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_TYPE_DEFAULT_VALUE) << "Default value for attribute 'decoration.type'";

    resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_COLOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_COLOR_DEFAULT_VALUE) <<
        "Default value for attribute 'decoration.color'";

    resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_STYLE_DEFAULT_VALUE) <<
        "Default value for attribute 'decoration.style'";
}

/*
 * @tc.name: setDecorationTestDecorationTypeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationTypeValidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](const std::string& input, const std::string& expectedStr,
                          const Ark_TextDecorationType& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        inputValueDecoration.type = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_TYPE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.type";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumTextDecorationTypeValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setDecorationTestDecorationTypeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationTypeInvalidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](const std::string& input, const Ark_TextDecorationType& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        modifier_->setDecoration(node_, &inputValueDecoration);
        inputValueDecoration.type = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_TYPE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_TYPE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.type";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumTextDecorationTypeInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setDecorationTestDecorationColorValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationColorValidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](
                          const std::string& input, const std::string& expectedStr, const Opt_ResourceColor& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        inputValueDecoration.color = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_COLOR_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.color";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureColorsEnumValidValues) {
        checkValue(input, expected, ArkUnion<Opt_ResourceColor, Ark_Color>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsNumValidValues) {
        checkValue(input, expected, ArkUnion<Opt_ResourceColor, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsResValidValues) {
        checkValue(input, expected, ArkUnion<Opt_ResourceColor, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsStrValidValues) {
        checkValue(input, expected, ArkUnion<Opt_ResourceColor, Ark_String>(value));
    }
}

/*
 * @tc.name: setDecorationTestDecorationColorInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationColorInvalidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](const std::string& input, const Opt_ResourceColor& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        modifier_->setDecoration(node_, &inputValueDecoration);
        inputValueDecoration.color = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_COLOR_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_COLOR_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.color";
    };

    for (auto& [input, value] : Fixtures::testFixtureColorsStrInvalidValues) {
        checkValue(input, ArkUnion<Opt_ResourceColor, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureColorsEnumInvalidValues) {
        checkValue(input, ArkUnion<Opt_ResourceColor, Ark_Color>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Opt_ResourceColor, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", ArkValue<Opt_ResourceColor>());
}

/*
 * @tc.name: setDecorationTestDecorationStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationStyleValidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](const std::string& input, const std::string& expectedStr,
                          const Opt_TextDecorationStyle& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        inputValueDecoration.style = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.style";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumTextDecorationStyleValidValues) {
        checkValue(input, expected, ArkValue<Opt_TextDecorationStyle>(value));
    }
}

/*
 * @tc.name: setDecorationTestDecorationStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setDecorationTestDecorationStyleInvalidValues, TestSize.Level1)
{
    Ark_DecorationStyleInterface initValueDecoration;

    // Initial setup
    initValueDecoration.type = std::get<1>(Fixtures::testFixtureEnumTextDecorationTypeValidValues[0]);
    initValueDecoration.color =
        ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));
    initValueDecoration.style =
        ArkValue<Opt_TextDecorationStyle>(std::get<1>(Fixtures::testFixtureEnumTextDecorationStyleValidValues[0]));

    auto checkValue = [this, &initValueDecoration](const std::string& input, const Opt_TextDecorationStyle& value) {
        Ark_DecorationStyleInterface inputValueDecoration = initValueDecoration;

        modifier_->setDecoration(node_, &inputValueDecoration);
        inputValueDecoration.style = value;
        modifier_->setDecoration(node_, &inputValueDecoration);
        auto jsonValue = GetJsonValue(node_);
        auto resultDecoration = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_DECORATION_NAME);
        auto resultStr = GetAttrValue<std::string>(resultDecoration, ATTRIBUTE_DECORATION_I_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_DECORATION_I_STYLE_DEFAULT_VALUE) <<
            "Input value is: " << input << ", method: setDecoration, attribute: decoration.style";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumTextDecorationStyleInvalidValues) {
        checkValue(input, ArkValue<Opt_TextDecorationStyle>(value));
    }
}

/*
 * @tc.name: setLetterSpacingTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, DISABLED_setLetterSpacingTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LETTER_SPACING_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LETTER_SPACING_DEFAULT_VALUE) << "Default value for attribute 'letterSpacing'";
}

/*
 * @tc.name: setLetterSpacingTestLetterSpacingValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setLetterSpacingTestLetterSpacingValidValues, TestSize.Level1)
{
    Ark_Union_Number_String initValueLetterSpacing;

    // Initial setup
    initValueLetterSpacing =
        ArkUnion<Ark_Union_Number_String, Ark_Number>(std::get<1>(Fixtures::testFixtureDimensionsNumAnyValidValues[0]));

    auto checkValue = [this, &initValueLetterSpacing](const std::string& input, const std::string& expectedStr,
                          const Ark_Union_Number_String& value) {
        Ark_Union_Number_String inputValueLetterSpacing = initValueLetterSpacing;

        inputValueLetterSpacing = value;
        modifier_->setLetterSpacing(node_, &inputValueLetterSpacing);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LETTER_SPACING_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setLetterSpacing, attribute: letterSpacing";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumAnyValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonPercValidValues) {
        checkValue(input, expected, ArkUnion<Ark_Union_Number_String, Ark_String>(value));
    }
}

} // namespace OHOS::Ace::NG

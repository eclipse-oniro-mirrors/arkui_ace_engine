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
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "arkoala_api_generated.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_ALLOW_SCALE_NAME = "allowScale";
    const auto ATTRIBUTE_ALLOW_SCALE_DEFAULT_VALUE = "false";
    const auto ATTRIBUTE_FONT_COLOR_NAME = "fontColor";
    const auto ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE = "#FF000000";
    const auto ATTRIBUTE_FONT_COLOR_INVALID_VALUE = "#00000000";
    const auto ATTRIBUTE_FONT_FAMILY_NAME("fontFamily");
    const auto ATTRIBUTE_FONT_SIZE_NAME("fontSize");
    const auto ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE("10.00vp");
    const auto ATTRIBUTE_FONT_WEIGHT_NAME("fontWeight");
    const auto ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE("FontWeight.Normal");
} // namespace

class MarqueeModifierTest : public ModifierTestBase<GENERATED_ArkUIMarqueeModifier,
    &GENERATED_ArkUINodeModifiers::getMarqueeModifier, GENERATED_ARKUI_MARQUEE> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
    }
};

/*
 * @tc.name: setAllowScaleTestDefaultValues
 * @tc.desc: Check the functionality of MarqueeModifier.setAllowScale
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setAllowScaleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ALLOW_SCALE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALLOW_SCALE_DEFAULT_VALUE);
}

// Valid values for attribute 'select' of method 'select'
static std::vector<std::tuple<std::string, Ark_Boolean, std::string>> allowScaleValidValues = {
    {"true", Converter::ArkValue<Ark_Boolean>(true), "true"},
    {"false", Converter::ArkValue<Ark_Boolean>(false), "false"},
};

/*
 * @tc.name: setAllowScaleTestValidValues
 * @tc.desc: Check the functionality of MarqueeModifier.setAllowScale
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setAllowScaleTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_Boolean inputValueAllowScale;
    Ark_Boolean initValueAllowScale;

    // Initial setup
    initValueAllowScale = std::get<1>(allowScaleValidValues[0]);

    // Verifying attribute's  values
    inputValueAllowScale = initValueAllowScale;
    for (auto&& value: allowScaleValidValues) {
        inputValueAllowScale = std::get<1>(value);
        modifier_->setAllowScale(node_, inputValueAllowScale);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ALLOW_SCALE_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setFontColorTestDefaultValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontColor
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontColorTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE);
}

// Valid values for attribute 'fontColor' of method 'fontColor'
static std::vector<std::tuple<std::string, Ark_ResourceColor, std::string>> fontColorFontColorValidValues = {
    { "ARK_COLOR_BLUE", Converter::ArkUnion<Ark_ResourceColor, enum Ark_Color>(ARK_COLOR_BLUE), "#FF0000FF" },
    { "ARK_COLOR_RED", Converter::ArkUnion<Ark_ResourceColor, enum Ark_Color>(ARK_COLOR_RED), "#FFFF0000" },
};

/*
 * @tc.name: setFontColorTestValidValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontColor
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontColorTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_ResourceColor inputValueFontColor;
    Ark_ResourceColor initValueFontColor;

    // Initial setup
    initValueFontColor = std::get<1>(fontColorFontColorValidValues[0]);

    // Verifying attribute's  values
    inputValueFontColor = initValueFontColor;
    for (auto&& value: fontColorFontColorValidValues) {
        inputValueFontColor = std::get<1>(value);
        modifier_->setFontColor(node_, &inputValueFontColor);
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
        expectedStr = std::get<2>(value);
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setFontColorTestInvalidValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontColor
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontColorTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    Ark_ResourceColor inputValueFontColor;

    // Verifying attribute's  values
    inputValueFontColor = Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0xffffffff + 1);
    modifier_->setFontColor(node_, &inputValueFontColor);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
    expectedStr = ATTRIBUTE_FONT_COLOR_INVALID_VALUE;
    EXPECT_EQ(resultStr, expectedStr);
}

/*
 * @tc.name: setFontFamilyTestValidValueson
 * @tc.desc: Check the functionality of MarqueeModifier.setFontFamily
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontFamilyTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    auto inputStringValue = "testFamily";
    auto inputArkStringValue = Converter::ArkValue<Ark_String>(inputStringValue);
    auto inputValue = Converter::ArkUnion<Ark_Union_String_Resource, Ark_String>(inputArkStringValue);

    modifier_->setFontFamily(node_, &inputValue);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
    EXPECT_EQ(resultStr, inputStringValue);

    inputStringValue = "testFamily1, testFamily2";
    inputArkStringValue = Converter::ArkValue<Ark_String>(inputStringValue);
    inputValue = Converter::ArkUnion<Ark_Union_String_Resource, Ark_String>(inputArkStringValue);

    modifier_->setFontFamily(node_, &inputValue);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
    EXPECT_EQ(resultStr, inputStringValue);

    inputStringValue = "testFamily1, testFamily2";
    inputArkStringValue = Converter::ArkValue<Ark_String>(inputStringValue);
    inputValue = Converter::ArkUnion<Ark_Union_String_Resource, Ark_String>(inputArkStringValue);

    modifier_->setFontFamily(node_, &inputValue);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
    EXPECT_EQ(resultStr, inputStringValue);
}

/*
 * @tc.name: setFontSizeDefaultValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontSize
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontSizeDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto size = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
    EXPECT_EQ(size, ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE);
}

/*
 * @tc.name: setFontSizeTestValidValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontSize
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontSizeTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    typedef std::pair<Ark_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testFontSizeValid = {
        { Converter::ArkValue<Ark_Length>(1.0f),  "1.00vp" },
        { Converter::ArkValue<Ark_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Ark_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Ark_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Ark_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Ark_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Ark_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Ark_Length>(4.3_fp), "4.30fp" },
    };

    for (const auto &[value, expectValue]: testFontSizeValid) {
        modifier_->setFontSize(node_, &value);
        jsonValue = GetJsonValue(node_);
        auto size = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(size, expectValue);
    }
}
/*
 * @tc.name: setFontSizeInvalidValues
 * @tc.desc: Check the functionality of MarqueeModifier.setFontSize
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontSizeInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;
    typedef std::pair<Ark_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testFontSizeInvalid = {
        { Converter::ArkValue<Ark_Length>(-0.1f), "10.00vp"},
        { Converter::ArkValue<Ark_Length>(-5.0_px), "10.00vp" },
    };
    for (const auto &[value, expectValue]: testFontSizeInvalid) {
        modifier_->setFontSize(node_, &value);
        jsonValue = GetJsonValue(node_);
        auto size = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(size, expectValue);
    }
}

using  ArkFontWeightTest = std::pair<Ark_Union_Number_FontWeight_String, std::string>;
namespace Converter {
const std::vector<ArkFontWeightTest> FONT_WEIGHT_TEST_PLAN = {
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_LIGHTER),
        "FontWeight.Lighter" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_NORMAL),
        "FontWeight.Normal" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_REGULAR),
        "FontWeight.Regular" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_MEDIUM),
        "FontWeight.Medium" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_BOLD),
        "FontWeight.Bold" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(Ark_FontWeight::ARK_FONT_WEIGHT_BOLDER),
        "FontWeight.Bolder" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(100)), "100" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(200)), "200" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(300)), "300" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(400)), "400" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(500)), "500" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(600)), "600" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(700)), "700" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(800)), "800" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(ArkValue<Ark_Number>(900)), "900" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("lighter")), "FontWeight.Lighter" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("normal")), "FontWeight.Normal" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("regular")), "FontWeight.Regular" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("medium")), "FontWeight.Medium" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("bold")), "FontWeight.Bold" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("bolder")), "FontWeight.Bolder" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("100")), "100" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("200")), "200" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("300")), "300" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("400")), "400" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("500")), "500" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("600")), "600" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("700")), "700" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("800")), "800" },
    { ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(ArkValue<Ark_String>("900")), "900" },
};
}

/**
 * @tc.name: setFontWeightTestDefaultValue
 * @tc.desc: Check the functionality of setFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontWeightTestDefaultValue, TestSize.Level1)
{
    auto jsonValue = GetJsonValue(node_);
    auto result = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
    EXPECT_EQ(result, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE);
}

/**
 * @tc.name: setFontWeightTestValidValues
 * @tc.desc: Check the functionality of setFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontWeightTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setFontWeight, nullptr);
    std::unique_ptr<JsonValue> jsonValue;
    std::string result;
    for (const auto &[weight, expectValue] : Converter::FONT_WEIGHT_TEST_PLAN) {
        modifier_->setFontWeight(node_, &weight);
        jsonValue = GetJsonValue(node_);
        result = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(result, expectValue);
    }
}

/**
 * @tc.name: setFontWeightTestInvalidValues
 * @tc.desc: Check the functionality of setFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MarqueeModifierTest, setFontWeightTestInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setFontWeight, nullptr);
    std::unique_ptr<JsonValue> jsonValue;
    std::string result;

    const std::vector<ArkFontWeightTest> testPlan = {
        { Converter::ArkUnion<Ark_Union_Number_FontWeight_String,
            Ark_Number>(Converter::ArkValue<Ark_Number>(1000)), "FontWeight.Normal" },
        { Converter::ArkUnion<Ark_Union_Number_FontWeight_String,
            Ark_String>(Converter::ArkValue<Ark_String>("1000")), "FontWeight.Normal" },
    };
    for (const auto &[weight, expectValue] : testPlan) {
        modifier_->setFontWeight(node_, &weight);
        jsonValue = GetJsonValue(node_);
        result = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(result, expectValue);
    }
}

} // namespace OHOS::Ace::NG
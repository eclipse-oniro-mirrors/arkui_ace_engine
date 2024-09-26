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

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::NG::Converter;

namespace OHOS::Ace::NG {
namespace  {
Ark_Resource ArkRes(Ark_String *name, int id = -1,
    NodeModifier::ResourceType type = NodeModifier::ResourceType::COLOR,
    const char *module = "", const char *bundle = "")
{
    return {
        .id = ArkValue<Ark_Number>(id),
        .type = ArkValue<Ark_Number>(static_cast<Ark_Int32>(type)),
        .moduleName = ArkValue<Ark_String>(module),
        .bundleName = ArkValue<Ark_String>(bundle),
        .params = { .tag = ARK_TAG_OBJECT, .value = {.array = name, .length = name ? 1 : 0} }
    };
}

// attributes name
const auto ATTRIBUTE_STROKE_LINE_CAP_NAME = "strokeLineCap";

// attributes default
const auto ATTRIBUTE_STROKE_LINE_CAP_DEFAULT_VALUE = "LineCapStyle.Butt";

using OneTestColorStep = std::pair<Ark_ResourceColor, std::string>;
static Ark_String RESOURCE_NAME = ArkValue<Ark_String>("aa.bb.cc");
static const std::string EXPECTED_RESOURCE_COLOR =
    Color::RED.ToString(); // Color::RED is result of ThemeConstants::GetColorXxxx stubs
static const std::vector<OneTestColorStep> TEST_COLOR_PLAN = {
    { ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
    { ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
    { ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f), "#00000000" },
    { ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
    { ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" },
    { ArkUnion<Ark_ResourceColor, Ark_Resource>(ArkRes(&RESOURCE_NAME)), EXPECTED_RESOURCE_COLOR },
    { ArkUnion<Ark_ResourceColor, Ark_Resource>(ArkRes(nullptr, 1234)), EXPECTED_RESOURCE_COLOR },
};
} // namespace

class CommonShapeMethodModifierTest : public ModifierTestBase<GENERATED_ArkUICommonShapeMethodModifier,
    &GENERATED_ArkUINodeModifiers::getCommonShapeMethodModifier,
    GENERATED_ARKUI_CIRCLE // test common shape methods on frameNode for Circle component
    > {
};

/**
 * @tc.name: setStrokeTest
 * @tc.desc: Check the functionality of CommonShapeMethodModifierTest.setStroke
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setStrokeTest, TestSize.Level1)
{
    static const std::string PROP_NAME("stroke");
    ASSERT_NE(modifier_->setStroke, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, Color::BLACK.ToString());

    for (const auto &[arkResColor, expected]: TEST_COLOR_PLAN) {
        modifier_->setStroke(node_, &arkResColor);
        auto checkColor = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkColor, expected);
    }
}

/**
 * @tc.name: setFillTest
 * @tc.desc: Check the functionality of CommonShapeMethodModifierTest.setFill
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setFillTest, TestSize.Level1)
{
    static const std::string PROP_NAME("fill");
    ASSERT_NE(modifier_->setFill, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, Color::BLACK.ToString());

    for (const auto &[arkResColor, expected]: TEST_COLOR_PLAN) {
        modifier_->setFill(node_, &arkResColor);
        auto checkColor = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkColor, expected);
    }
}

/*
 * @tc.name: setStrokeLineCapTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setStrokeLineCapTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_LINE_CAP_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_STROKE_LINE_CAP_DEFAULT_VALUE);
}

/*
 * @tc.name: setStrokeLineCapTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setStrokeLineCapTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    Ark_LineCapStyle inputValueStrokeLineCap;

    // Initial setup
    inputValueStrokeLineCap = ARK_LINE_CAP_STYLE_BUTT;

    // Test
    modifier_->setStrokeLineCap(node_, inputValueStrokeLineCap);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_LINE_CAP_NAME);
    EXPECT_EQ(resultStr, "LineCapStyle.Butt");

    // Verifying attribute's other values
    inputValueStrokeLineCap = ARK_LINE_CAP_STYLE_ROUND;
    modifier_->setStrokeLineCap(node_, inputValueStrokeLineCap);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_LINE_CAP_NAME);
    EXPECT_EQ(resultStr, "LineCapStyle.Round");

    inputValueStrokeLineCap = ARK_LINE_CAP_STYLE_SQUARE;
    modifier_->setStrokeLineCap(node_, inputValueStrokeLineCap);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_LINE_CAP_NAME);
    EXPECT_EQ(resultStr, "LineCapStyle.Square");
}

/*
 * @tc.name: setStrokeLineCapTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setStrokeLineCapTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    Ark_LineCapStyle inputValueStrokeLineCap;

    // Initial setup
    inputValueStrokeLineCap = static_cast<Ark_LineCapStyle>(-1);

    // Test
    modifier_->setStrokeLineCap(node_, inputValueStrokeLineCap);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_LINE_CAP_NAME);
    EXPECT_EQ(resultStr, "LineCapStyle.Butt");
}

/**
 * @tc.name: setAntiAliasTest
 * @tc.desc: Check the functionality of setAntiAlias
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierTest, setAntiAliasTest, TestSize.Level1)
{
    static const std::string PROP_NAME("antiAlias");
    ASSERT_NE(modifier_->setAntiAlias, nullptr);
    using OneBoolStep = std::tuple<Ark_Boolean, std::string>;
    const std::vector<OneBoolStep> BOOL_TEST_PLAN = {
        { false, "false" },
        { true, "true" },
        { 0, "false" },
        { -25, "true" },
        { 0, "false" },
        { 25, "true" },
        { false, "false" }
    };
    auto fullJson = GetJsonValue(node_);
    auto checkVal = GetAttrValue<std::string>(fullJson, PROP_NAME);
    EXPECT_EQ(checkVal, "true");

    for (const auto& [value, expectVal] : BOOL_TEST_PLAN) {
        modifier_->setAntiAlias(node_, value);
        auto fullJson = GetJsonValue(node_);
        checkVal = GetAttrValue<std::string>(fullJson, PROP_NAME);
        EXPECT_EQ(checkVal, expectVal);
    }
}
} // namespace OHOS::Ace::NG

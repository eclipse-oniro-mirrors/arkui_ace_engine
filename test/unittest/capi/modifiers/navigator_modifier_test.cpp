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

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_ACTIVE_NAME = "active";
    const auto ATTRIBUTE_ACTIVE_DEFAULT_VALUE = "false";
    const auto ATTRIBUTE_TYPE_NAME = "type";
    const auto ATTRIBUTE_TYPE_DEFAULT_VALUE = "NavigationType.Push";
    const auto ATTRIBUTE_TARGET_NAME = "target";
    const auto ATTRIBUTE_TARGET_DEFAULT_VALUE = "";
} // namespace


class NavigatorModifierTest : public ModifierTestBase<GENERATED_ArkUINavigatorModifier,
    &GENERATED_ArkUINodeModifiers::getNavigatorModifier, GENERATED_ARKUI_NAVIGATOR> {
};

/*
 * @tc.name: setNavigatorOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setNavigatorOptionsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TARGET_DEFAULT_VALUE);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

/*
 * @tc.name: setNavigatorOptions0TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setNavigatorOptions0TestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    ASSERT_NE(modifier_->setNavigatorOptions0, nullptr);

    // Initial setup
<<<<<<< HEAD
    Ark_Literal_String_target_NavigationType_type inputValueOptions = {
        .target = {"abc"},
=======
    Type_NavigatorInterface_setNavigatorOptions_Arg0 inputValueOptions = {
        .target = {.chars = "abc", .length = sizeof("abc")},
>>>>>>> 0c47493497 (add missing length)
        .type = Converter::ArkValue<Opt_NavigationType>(ARK_NAVIGATION_TYPE_REPLACE)
    };
    auto realInputValue = Converter::ArkValue<Opt_Literal_String_target_NavigationType_type>(inputValueOptions);

    // Test
    modifier_->setNavigatorOptions0(node_, &realInputValue);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, "abc");
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, "NavigationType.Replace");
}

/*
 * @tc.name: setNavigatorOptions0TestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setNavigatorOptions0TestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    ASSERT_NE(modifier_->setNavigatorOptions0, nullptr);

    // Initial setup
    Ark_Literal_String_target_NavigationType_type inputValueOptions = {
        .target = { .chars = nullptr },
        .type = { .tag = ARK_TAG_OBJECT, .value = static_cast<Ark_NavigationType>(INT_MIN) }
    };
    auto realInputValue = Converter::ArkValue<Opt_Literal_String_target_NavigationType_type>(inputValueOptions);

    // Test
    modifier_->setNavigatorOptions0(node_, &realInputValue);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TARGET_DEFAULT_VALUE);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

/*
 * @tc.name: setNavigatorOptions1Test
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setNavigatorOptions1, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;

    ASSERT_NE(modifier_->setNavigatorOptions1, nullptr);

    // Test
    modifier_->setNavigatorOptions1(node_);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TARGET_DEFAULT_VALUE);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

/*
 * @tc.name: setActiveTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setActiveTestDefaultValues, TestSize.Level1)
{
    auto resultStr = GetAttrValue<std::string>(node_, ATTRIBUTE_ACTIVE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ACTIVE_DEFAULT_VALUE);
}

/*
 * @tc.name: setActiveTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setActiveTestValidValues, TestSize.Level1)
{
    std::string resultStr;
    Ark_Boolean inputValueActive;

    ASSERT_NE(modifier_->setActive, nullptr);

    // Initial setup
    inputValueActive = Converter::ArkValue<Ark_Boolean>(true);

    // Test
    modifier_->setActive(node_, inputValueActive);

    // Initial verification
    resultStr = GetAttrValue<std::string>(node_, ATTRIBUTE_ACTIVE_NAME);
    EXPECT_EQ(resultStr, "true");

    // Verifying attribute's other values
    inputValueActive = Converter::ArkValue<Ark_Boolean>(false);
    modifier_->setActive(node_, inputValueActive);
    resultStr = GetAttrValue<std::string>(node_, ATTRIBUTE_ACTIVE_NAME);
    EXPECT_EQ(resultStr, "false");
}

/*
 * @tc.name: setTypeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setTypeTestDefaultValues, TestSize.Level1)
{
    auto resultStr = GetAttrValue<std::string>(node_, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

/*
 * @tc.name: setTypeTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setTypeTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    Ark_NavigationType inputValueType;

    ASSERT_NE(modifier_->setType, nullptr);

    // Initial setup
    inputValueType = ARK_NAVIGATION_TYPE_PUSH;

    // Test
    modifier_->setType(node_, inputValueType);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, "NavigationType.Push");

    // Verifying attribute's other values
    inputValueType = ARK_NAVIGATION_TYPE_BACK;
    modifier_->setType(node_, inputValueType);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, "NavigationType.Back");

    inputValueType = ARK_NAVIGATION_TYPE_REPLACE;
    modifier_->setType(node_, inputValueType);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, "NavigationType.Replace");
}

/*
 * @tc.name: setTypeTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setTypeTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    Ark_NavigationType inputValueType;

    ASSERT_NE(modifier_->setType, nullptr);

    // Initial setup
    inputValueType = static_cast<Ark_NavigationType>(INT_MIN);

    // Test
    modifier_->setType(node_, inputValueType);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE);
}

/*
 * @tc.name: setTargetTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setTargetTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TARGET_DEFAULT_VALUE);
}

/*
 * @tc.name: setTargetTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, setTargetTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    Ark_String inputValueTarget;

    ASSERT_NE(modifier_->setTarget, nullptr);

    // Initial setup
    inputValueTarget = Converter::ArkValue<Ark_String>("");

    // Test
    modifier_->setTarget(node_, &inputValueTarget);

    // Initial verification
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, "");

    // Verifying attribute's other values
    inputValueTarget = Converter::ArkValue<Ark_String>("abc");
    modifier_->setTarget(node_, &inputValueTarget);
    jsonValue = GetJsonValue(node_);
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TARGET_NAME);
    EXPECT_EQ(resultStr, "abc");
}

/*
 * @tc.name: DISABLED_setParamsTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(NavigatorModifierTest, DISABLED_setParamsTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setParams, nullptr);
    // CustomObjects is not implemented yet!
}
} // namespace OHOS::Ace::NG

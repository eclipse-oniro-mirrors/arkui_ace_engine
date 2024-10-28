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
#include "test_fixtures.h"
#include "type_helpers.h"

#include "core/components/button/button_theme.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
using namespace TypeHelper;
namespace {
const auto ATTRIBUTE_LABEL_STYLE_NAME = "labelStyle";
const auto ATTRIBUTE_LABEL_STYLE_FONT_NAME = "font";
const auto ATTRIBUTE_TYPE_NAME = "type";
const auto ATTRIBUTE_TYPE_DEFAULT_VALUE = "ButtonType.Capsule";
const auto ATTRIBUTE_STATE_EFFECT_NAME = "stateEffect";
const auto ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_BUTTON_STYLE_NAME = "buttonStyle";
const auto ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE = "ButtonStyleMode.EMPHASIZED";
const auto ATTRIBUTE_CONTROL_SIZE_NAME = "controlSize";
const auto ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE = "ControlSize.NORMAL";
const auto ATTRIBUTE_ROLE_NAME = "role";
const auto ATTRIBUTE_ROLE_DEFAULT_VALUE = "ButtonRole.NORMAL";
const auto ATTRIBUTE_LABEL_NAME = "label";
const auto ATTRIBUTE_LABEL_DEFAULT_VALUE = "";
const auto ATTRIBUTE_FONT_COLOR_NAME = "fontColor";
const auto ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE = "#FF000000";
const auto ATTRIBUTE_FONT_SIZE_NAME = "fontSize";
const auto ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_FONT_WEIGHT_NAME = "fontWeight";
const auto ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE = "FontWeight.Medium";
const auto ATTRIBUTE_FONT_STYLE_NAME = "fontStyle";
const auto ATTRIBUTE_FONT_STYLE_DEFAULT_VALUE = "FontStyle.Normal";
const auto ATTRIBUTE_FONT_FAMILY_NAME = "fontFamily";
const auto ATTRIBUTE_FONT_FAMILY_DEFAULT_VALUE = "Harmony"
                                                 "OS Sans";
const auto ATTRIBUTE_LABEL_STYLE_OVERFLOW_NAME = "overflow";
const auto ATTRIBUTE_LABEL_STYLE_OVERFLOW_DEFAULT_VALUE = "TextOverflow.Ellipsis";
const auto ATTRIBUTE_LABEL_STYLE_MAX_LINES_NAME = "maxLines";
const auto ATTRIBUTE_LABEL_STYLE_MAX_LINES_DEFAULT_VALUE = "1";
const auto ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_NAME = "minFontSize";
const auto ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_NAME = "maxFontSize";
const auto ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_NAME = "heightAdaptivePolicy";
const auto ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_DEFAULT_VALUE = "TextHeightAdaptivePolicy.MAX_LINES_FIRST";
const auto ATTRIBUTE_LABEL_STYLE_FONT_SIZE_NAME = "size";
const auto ATTRIBUTE_LABEL_STYLE_FONT_SIZE_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_NAME = "weight";
const auto ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_DEFAULT_VALUE = "FontWeight.Medium";
const auto ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_NAME = "family";
const auto ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_DEFAULT_VALUE = "Harmony"
                                                             "OS Sans";
const auto ATTRIBUTE_LABEL_STYLE_FONT_STYLE_NAME = "style";
const auto ATTRIBUTE_LABEL_STYLE_FONT_STYLE_DEFAULT_VALUE = "FontStyle.Normal";
} // namespace

class ButtonModifierTest : public ModifierTestBase<GENERATED_ArkUIButtonModifier,
                               &GENERATED_ArkUINodeModifiers::getButtonModifier, GENERATED_ARKUI_BUTTON> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<ButtonTheme>();

        for (auto& [id, strid, res] : Fixtures::resourceInitTable) {
            AddResource(id, res);
            AddResource(strid, res);
        }
    }
};

/*
 * @tc.name: setButtonOptions1TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE) << "Default value for attribute 'options.type'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE) << "Default value for attribute 'options.stateEffect'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE) << "Default value for attribute 'options.buttonStyle'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE) << "Default value for attribute 'options.controlSize'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE) << "Default value for attribute 'options.role'";
}

/*
 * @tc.name: setButtonOptions1TestOptionsTypeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsTypeValidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](
                          const std::string& input, const Opt_ButtonType& value, const std::string& expectedStr) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.type = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.type";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonTypeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonType>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsTypeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsTypeInvalidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](const std::string& input, const Opt_ButtonType& value) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.type = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.type";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonTypeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonType>(value));
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsStateEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsStateEffectValidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](
                          const std::string& input, const Opt_Boolean& value, const std::string& expectedStr) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.stateEffect = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.stateEffect";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, Converter::ArkValue<Opt_Boolean>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsStateEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsStateEffectInvalidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](const std::string& input, const Opt_Boolean& value) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.stateEffect = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.stateEffect";
    };

    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Boolean>());
}

/*
 * @tc.name: setButtonOptions1TestOptionsButtonStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsButtonStyleValidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](
                          const std::string& input, const Opt_ButtonStyleMode& value, const std::string& expectedStr) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.buttonStyle = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.buttonStyle";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonStyleModeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonStyleMode>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsButtonStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsButtonStyleInvalidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](const std::string& input, const Opt_ButtonStyleMode& value) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.buttonStyle = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.buttonStyle";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonStyleModeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonStyleMode>(value));
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsControlSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsControlSizeValidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](
                          const std::string& input, const Opt_ControlSize& value, const std::string& expectedStr) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.controlSize = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.controlSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumControlSizeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ControlSize>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsControlSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsControlSizeInvalidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](const std::string& input, const Opt_ControlSize& value) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.controlSize = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.controlSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumControlSizeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ControlSize>(value));
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsRoleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsRoleValidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](
                          const std::string& input, const Opt_ButtonRole& value, const std::string& expectedStr) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.role = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.role";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonRoleValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonRole>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions1TestOptionsRoleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions1TestOptionsRoleInvalidValues, TestSize.Level1)
{
    Ark_ButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    initValueOptions.stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    initValueOptions.buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    initValueOptions.controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    initValueOptions.role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueOptions](const std::string& input, const Opt_ButtonRole& value) {
        Ark_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueOptions.role = value;
        modifier_->setButtonOptions1(node, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions1, attribute: options.role";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonRoleInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonRole>(value));
    }
}

/*
 * @tc.name: setButtonOptions2TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LABEL_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_DEFAULT_VALUE) << "Default value for attribute 'label'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE) << "Default value for attribute 'options.type'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE) << "Default value for attribute 'options.stateEffect'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE) << "Default value for attribute 'options.buttonStyle'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE) << "Default value for attribute 'options.controlSize'";

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE) << "Default value for attribute 'options.role'";
}

/*
 * @tc.name: setButtonOptions2TestLabelValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestLabelValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Ark_ResourceStr& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueLabel = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LABEL_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: label";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureStringValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceStr, Ark_String>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureStringResValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceStr, Ark_Resource>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestLabelInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestLabelInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Ark_ResourceStr& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueLabel = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LABEL_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: label";
    };

    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Ark_ResourceStr, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setButtonOptions2TestOptionsTypeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsTypeValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonType& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).type = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.type";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonTypeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonType>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsTypeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsTypeInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonType& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).type = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.type";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonTypeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonType>(value));
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsStateEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsStateEffectValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_Boolean& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).stateEffect = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.stateEffect";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, Converter::ArkValue<Opt_Boolean>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsStateEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsStateEffectInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](const std::string& input, const Opt_Boolean& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).stateEffect = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.stateEffect";
    };

    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Boolean>());
}

/*
 * @tc.name: setButtonOptions2TestOptionsButtonStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsButtonStyleValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonStyleMode& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).buttonStyle = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.buttonStyle";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonStyleModeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonStyleMode>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsButtonStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsButtonStyleInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonStyleMode& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).buttonStyle = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.buttonStyle";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonStyleModeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonStyleMode>(value));
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsControlSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsControlSizeValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ControlSize& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).controlSize = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.controlSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumControlSizeValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ControlSize>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsControlSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsControlSizeInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ControlSize& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).controlSize = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.controlSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumControlSizeInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ControlSize>(value));
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsRoleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsRoleValidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonRole& value, const std::string& expectedStr) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).role = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.role";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonRoleValidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonRole>(value), expected);
    }
}

/*
 * @tc.name: setButtonOptions2TestOptionsRoleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonOptions2TestOptionsRoleInvalidValues, TestSize.Level1)
{
    Ark_ResourceStr initValueLabel;
    Opt_ButtonOptions initValueOptions;

    // Initial setup
    initValueLabel =
        Converter::ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0]));
    WriteTo(initValueOptions).type =
        Converter::ArkValue<Opt_ButtonType>(std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]));
    WriteTo(initValueOptions).stateEffect =
        Converter::ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]));
    WriteTo(initValueOptions).buttonStyle =
        Converter::ArkValue<Opt_ButtonStyleMode>(std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]));
    WriteTo(initValueOptions).controlSize =
        Converter::ArkValue<Opt_ControlSize>(std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]));
    WriteTo(initValueOptions).role =
        Converter::ArkValue<Opt_ButtonRole>(std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]));

    auto checkValue = [this, &initValueLabel, &initValueOptions](
                          const std::string& input, const Opt_ButtonRole& value) {
        Ark_ResourceStr inputValueLabel = initValueLabel;
        Opt_ButtonOptions inputValueOptions = initValueOptions;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        WriteTo(inputValueOptions).role = value;
        modifier_->setButtonOptions2(node, &inputValueLabel, &inputValueOptions);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonOptions2, attribute: options.role";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonRoleInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_ButtonRole>(value));
    }
}

/*
 * @tc.name: setTypeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setTypeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE) << "Default value for attribute 'type'";
}

/*
 * @tc.name: setTypeTestTypeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setTypeTestTypeValidValues, TestSize.Level1)
{
    Ark_ButtonType initValueType;

    // Initial setup
    initValueType = std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]);

    auto checkValue = [this, &initValueType](
                          const std::string& input, const Ark_ButtonType& value, const std::string& expectedStr) {
        Ark_ButtonType inputValueType = initValueType;

        inputValueType = value;
        modifier_->setType(node_, inputValueType);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setType, attribute: type";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonTypeValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setTypeTestTypeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setTypeTestTypeInvalidValues, TestSize.Level1)
{
    Ark_ButtonType initValueType;

    // Initial setup
    initValueType = std::get<1>(Fixtures::testFixtureEnumButtonTypeValidValues[0]);

    auto checkValue = [this, &initValueType](const std::string& input, const Ark_ButtonType& value) {
        Ark_ButtonType inputValueType = initValueType;

        modifier_->setType(node_, inputValueType);
        inputValueType = value;
        modifier_->setType(node_, inputValueType);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_TYPE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setType, attribute: type";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonTypeInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setStateEffectTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setStateEffectTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_STATE_EFFECT_DEFAULT_VALUE) << "Default value for attribute 'stateEffect'";
}

/*
 * @tc.name: setStateEffectTestStateEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setStateEffectTestStateEffectValidValues, TestSize.Level1)
{
    Ark_Boolean initValueStateEffect;

    // Initial setup
    initValueStateEffect = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueStateEffect](
                          const std::string& input, const Ark_Boolean& value, const std::string& expectedStr) {
        Ark_Boolean inputValueStateEffect = initValueStateEffect;

        inputValueStateEffect = value;
        modifier_->setStateEffect(node_, inputValueStateEffect);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setStateEffect, attribute: stateEffect";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setButtonStyleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonStyleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE) << "Default value for attribute 'buttonStyle'";
}

/*
 * @tc.name: setButtonStyleTestButtonStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonStyleTestButtonStyleValidValues, TestSize.Level1)
{
    Ark_ButtonStyleMode initValueButtonStyle;

    // Initial setup
    initValueButtonStyle = std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]);

    auto checkValue = [this, &initValueButtonStyle](
                          const std::string& input, const Ark_ButtonStyleMode& value, const std::string& expectedStr) {
        Ark_ButtonStyleMode inputValueButtonStyle = initValueButtonStyle;

        inputValueButtonStyle = value;
        modifier_->setButtonStyle(node_, inputValueButtonStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setButtonStyle, attribute: buttonStyle";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonStyleModeValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setButtonStyleTestButtonStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setButtonStyleTestButtonStyleInvalidValues, TestSize.Level1)
{
    Ark_ButtonStyleMode initValueButtonStyle;

    // Initial setup
    initValueButtonStyle = std::get<1>(Fixtures::testFixtureEnumButtonStyleModeValidValues[0]);

    auto checkValue = [this, &initValueButtonStyle](const std::string& input, const Ark_ButtonStyleMode& value) {
        Ark_ButtonStyleMode inputValueButtonStyle = initValueButtonStyle;

        modifier_->setButtonStyle(node_, inputValueButtonStyle);
        inputValueButtonStyle = value;
        modifier_->setButtonStyle(node_, inputValueButtonStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_BUTTON_STYLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setButtonStyle, attribute: buttonStyle";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonStyleModeInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setControlSizeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setControlSizeTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE) << "Default value for attribute 'controlSize'";
}

/*
 * @tc.name: setControlSizeTestControlSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setControlSizeTestControlSizeValidValues, TestSize.Level1)
{
    Ark_ControlSize initValueControlSize;

    // Initial setup
    initValueControlSize = std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]);

    auto checkValue = [this, &initValueControlSize](
                          const std::string& input, const Ark_ControlSize& value, const std::string& expectedStr) {
        Ark_ControlSize inputValueControlSize = initValueControlSize;

        inputValueControlSize = value;
        modifier_->setControlSize(node_, inputValueControlSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setControlSize, attribute: controlSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumControlSizeValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setControlSizeTestControlSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setControlSizeTestControlSizeInvalidValues, TestSize.Level1)
{
    Ark_ControlSize initValueControlSize;

    // Initial setup
    initValueControlSize = std::get<1>(Fixtures::testFixtureEnumControlSizeValidValues[0]);

    auto checkValue = [this, &initValueControlSize](const std::string& input, const Ark_ControlSize& value) {
        Ark_ControlSize inputValueControlSize = initValueControlSize;

        modifier_->setControlSize(node_, inputValueControlSize);
        inputValueControlSize = value;
        modifier_->setControlSize(node_, inputValueControlSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_CONTROL_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setControlSize, attribute: controlSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumControlSizeInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setRoleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setRoleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE) << "Default value for attribute 'role'";
}

/*
 * @tc.name: setRoleTestRoleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setRoleTestRoleValidValues, TestSize.Level1)
{
    Ark_ButtonRole initValueRole;

    // Initial setup
    initValueRole = std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]);

    auto checkValue = [this, &initValueRole](
                          const std::string& input, const Ark_ButtonRole& value, const std::string& expectedStr) {
        Ark_ButtonRole inputValueRole = initValueRole;

        inputValueRole = value;
        modifier_->setRole(node_, inputValueRole);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) << "Input value is: " << input << ", method: setRole, attribute: role";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumButtonRoleValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setRoleTestRoleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setRoleTestRoleInvalidValues, TestSize.Level1)
{
    Ark_ButtonRole initValueRole;

    // Initial setup
    initValueRole = std::get<1>(Fixtures::testFixtureEnumButtonRoleValidValues[0]);

    auto checkValue = [this, &initValueRole](const std::string& input, const Ark_ButtonRole& value) {
        Ark_ButtonRole inputValueRole = initValueRole;

        modifier_->setRole(node_, inputValueRole);
        inputValueRole = value;
        modifier_->setRole(node_, inputValueRole);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_ROLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setRole, attribute: role";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumButtonRoleInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setFontColorTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontColorTestDefaultValues, TestSize.Level1)
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
HWTEST_F(ButtonModifierTest, setFontColorTestFontColorValidValues, TestSize.Level1)
{
    Ark_ResourceColor initValueFontColor;

    // Initial setup
    initValueFontColor =
        Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));

    auto checkValue = [this, &initValueFontColor](
                          const std::string& input, const Ark_ResourceColor& value, const std::string& expectedStr) {
        Ark_ResourceColor inputValueFontColor = initValueFontColor;

        inputValueFontColor = value;
        modifier_->setFontColor(node_, &inputValueFontColor);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontColor, attribute: fontColor";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureColorsEnumValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsNumValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsResValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_Resource>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureColorsStrValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontColorTestFontColorInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontColorTestFontColorInvalidValues, TestSize.Level1)
{
    Ark_ResourceColor initValueFontColor;

    // Initial setup
    initValueFontColor =
        Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(std::get<1>(Fixtures::testFixtureColorsEnumValidValues[0]));

    auto checkValue = [this, &initValueFontColor](const std::string& input, const Ark_ResourceColor& value) {
        Ark_ResourceColor inputValueFontColor = initValueFontColor;

        modifier_->setFontColor(node_, &inputValueFontColor);
        inputValueFontColor = value;
        modifier_->setFontColor(node_, &inputValueFontColor);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_COLOR_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_COLOR_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontColor, attribute: fontColor";
    };

    for (auto& [input, value] : Fixtures::testFixtureColorsStrInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureColorsEnumInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(value));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Ark_ResourceColor, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontSizeTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontSizeTestDefaultValues, TestSize.Level1)
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
HWTEST_F(ButtonModifierTest, DISABLED_setFontSizeTestFontSizeValidValues, TestSize.Level1)
{
    Ark_Length initValueFontSize;

    // Initial setup
    initValueFontSize = std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]);

    auto checkValue = [this, &initValueFontSize](
                          const std::string& input, const Ark_Length& value, const std::string& expectedStr) {
        Ark_Length inputValueFontSize = initValueFontSize;

        inputValueFontSize = value;
        modifier_->setFontSize(node_, &inputValueFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontSize, attribute: fontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureLengthNonNegNonPctValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setFontSizeTestFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontSizeTestFontSizeInvalidValues, TestSize.Level1)
{
    Ark_Length initValueFontSize;

    // Initial setup
    initValueFontSize = std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]);

    auto checkValue = [this, &initValueFontSize](const std::string& input, const Ark_Length& value) {
        Ark_Length inputValueFontSize = initValueFontSize;

        modifier_->setFontSize(node_, &inputValueFontSize);
        inputValueFontSize = value;
        modifier_->setFontSize(node_, &inputValueFontSize);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontSize, attribute: fontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureLengthNonNegNonPctInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setFontWeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setFontWeightTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE) << "Default value for attribute 'fontWeight'";
}

/*
 * @tc.name: setFontWeightTestFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontWeightTestFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](const std::string& input,
                          const Ark_Union_Number_FontWeight_String& value, const std::string& expectedStr) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        inputValueFontWeight = value;
        modifier_->setFontWeight(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontWeight, attribute: fontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontWeightTestFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontWeightTestFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](
                          const std::string& input, const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        modifier_->setFontWeight(node_, &inputValueFontWeight);
        inputValueFontWeight = value;
        modifier_->setFontWeight(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontWeight, attribute: fontWeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontStyleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontStyleTestDefaultValues, TestSize.Level1)
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
HWTEST_F(ButtonModifierTest, setFontStyleTestFontStyleValidValues, TestSize.Level1)
{
    Ark_FontStyle initValueFontStyle;

    // Initial setup
    initValueFontStyle = std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]);

    auto checkValue = [this, &initValueFontStyle](
                          const std::string& input, const Ark_FontStyle& value, const std::string& expectedStr) {
        Ark_FontStyle inputValueFontStyle = initValueFontStyle;

        inputValueFontStyle = value;
        modifier_->setFontStyle(node_, inputValueFontStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontStyle, attribute: fontStyle";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontStyleValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setFontStyleTestFontStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontStyleTestFontStyleInvalidValues, TestSize.Level1)
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
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_STYLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontStyle, attribute: fontStyle";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontStyleInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setFontFamilyTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontFamilyTestDefaultValues, TestSize.Level1)
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
HWTEST_F(ButtonModifierTest, DISABLED_setFontFamilyTestFontFamilyValidValues, TestSize.Level1)
{
    Ark_Union_String_Resource initValueFontFamily;

    // Initial setup
    initValueFontFamily = Converter::ArkUnion<Ark_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));

    auto checkValue = [this, &initValueFontFamily](const std::string& input, const Ark_Union_String_Resource& value,
                          const std::string& expectedStr) {
        Ark_Union_String_Resource inputValueFontFamily = initValueFontFamily;

        inputValueFontFamily = value;
        modifier_->setFontFamily(node_, &inputValueFontFamily);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontFamily, attribute: fontFamily";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyResourceValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_String_Resource, Ark_Resource>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyStringValidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_String_Resource, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontFamilyTestFontFamilyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setFontFamilyTestFontFamilyInvalidValues, TestSize.Level1)
{
    Ark_Union_String_Resource initValueFontFamily;

    // Initial setup
    initValueFontFamily = Converter::ArkUnion<Ark_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));

    auto checkValue = [this, &initValueFontFamily](const std::string& input, const Ark_Union_String_Resource& value) {
        Ark_Union_String_Resource inputValueFontFamily = initValueFontFamily;

        modifier_->setFontFamily(node_, &inputValueFontFamily);
        inputValueFontFamily = value;
        modifier_->setFontFamily(node_, &inputValueFontFamily);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_FAMILY_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontFamily, attribute: fontFamily";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontFamilyResourceInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontFamilyStringInvalidValues) {
        checkValue(input, Converter::ArkUnion<Ark_Union_String_Resource, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Ark_Union_String_Resource, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setLabelStyleTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setLabelStyleTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultLabelStyle =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
    std::unique_ptr<JsonValue> resultFont =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_OVERFLOW_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_OVERFLOW_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.overflow'";

    resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_LINES_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MAX_LINES_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.maxLines'";

    resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.minFontSize'";

    resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.maxFontSize'";

    resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.heightAdaptivePolicy'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_SIZE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_SIZE_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.font.size'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.font.weight'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.font.family'";

    resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_STYLE_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_STYLE_DEFAULT_VALUE)
        << "Default value for attribute 'labelStyle.font.style'";
}

/*
 * @tc.name: setLabelStyleTestLabelStyleOverflowValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setLabelStyleTestLabelStyleOverflowValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_TextOverflow& value, const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        inputValueLabelStyle.overflow = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_OVERFLOW_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.overflow";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumTextOverflowValidValues) {
        checkValue(input, Converter::ArkValue<Opt_TextOverflow>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleOverflowInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setLabelStyleTestLabelStyleOverflowInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_TextOverflow& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        inputValueLabelStyle.overflow = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_OVERFLOW_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_OVERFLOW_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.overflow";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumTextOverflowInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_TextOverflow>(value));
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMaxLinesValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMaxLinesValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_Number& value, const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        inputValueLabelStyle.maxLines = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_LINES_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.maxLines";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureNumberPosIntFloorValidValues) {
        checkValue(input, Converter::ArkValue<Opt_Number>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMaxLinesInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMaxLinesInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Number& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        inputValueLabelStyle.maxLines = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_LINES_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MAX_LINES_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.maxLines";
    };

    for (auto& [input, value] : Fixtures::testFixtureNumberPosIntFloorInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_Number>(value));
    }
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Number>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMinFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMinFontSizeValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Union_Number_ResourceStr& value,
                          const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        inputValueLabelStyle.minFontSize = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.minFontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegNonPctValidValues) {
        checkValue(input,
            Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                Converter::ArkUnion<Ark_ResourceStr, Ark_Resource>(value)),
            expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegNonPctValidValues) {
        checkValue(input,
            Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                Converter::ArkUnion<Ark_ResourceStr, Ark_String>(value)),
            expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMinFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMinFontSizeInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_Union_Number_ResourceStr& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        inputValueLabelStyle.minFontSize = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MIN_FONT_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.minFontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegNonPctInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                              Converter::ArkUnion<Ark_ResourceStr, Ark_String>(value)));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegNonPctInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                              Converter::ArkUnion<Ark_ResourceStr, Ark_Resource>(value)));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Union_Number_ResourceStr>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMaxFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMaxFontSizeValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Union_Number_ResourceStr& value,
                          const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        inputValueLabelStyle.maxFontSize = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.maxFontSize";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsNumNonNegValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsResNonNegNonPctValidValues) {
        checkValue(input,
            Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                Converter::ArkUnion<Ark_ResourceStr, Ark_Resource>(value)),
            expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureDimensionsStrNonNegNonPctValidValues) {
        checkValue(input,
            Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                Converter::ArkUnion<Ark_ResourceStr, Ark_String>(value)),
            expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleMaxFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleMaxFontSizeInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_Union_Number_ResourceStr& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        inputValueLabelStyle.maxFontSize = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_MAX_FONT_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.maxFontSize";
    };

    for (auto& [input, value] : Fixtures::testFixtureDimensionsNumNonNegInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsStrNonNegNonPctInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                              Converter::ArkUnion<Ark_ResourceStr, Ark_String>(value)));
    }
    for (auto& [input, value] : Fixtures::testFixtureDimensionsResNonNegNonPctInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_ResourceStr>(
                              Converter::ArkUnion<Ark_ResourceStr, Ark_Resource>(value)));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Union_Number_ResourceStr>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleHeightAdaptivePolicyValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleHeightAdaptivePolicyValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_TextHeightAdaptivePolicy& value,
                          const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        inputValueLabelStyle.heightAdaptivePolicy = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.heightAdaptivePolicy";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues) {
        checkValue(input, Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleHeightAdaptivePolicyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleHeightAdaptivePolicyInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_TextHeightAdaptivePolicy& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        inputValueLabelStyle.heightAdaptivePolicy = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_HEIGHT_ADAPTIVE_POLICY_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.heightAdaptivePolicy";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumTextHeightAdaptivePolicyInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(value));
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontSizeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setLabelStyleTestLabelStyleFontSizeValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_Length& value, const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        WriteTo(inputValueLabelStyle.font).size = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.size";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureLengthNonNegNonPctValidValues) {
        checkValue(input, Converter::ArkValue<Opt_Length>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontSizeInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontSizeInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Length& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        WriteTo(inputValueLabelStyle.font).size = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_SIZE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_SIZE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.size";
    };

    for (auto& [input, value] : Fixtures::testFixtureLengthNonNegNonPctInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_Length>(value));
    }
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Length>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontWeightValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input,
                          const Opt_Union_FontWeight_Number_String& value, const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        WriteTo(inputValueLabelStyle.font).weight = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.weight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontWeightInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_Union_FontWeight_Number_String& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        WriteTo(inputValueLabelStyle.font).weight = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_WEIGHT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.weight";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_String>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Union_FontWeight_Number_String>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontFamilyValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, DISABLED_setLabelStyleTestLabelStyleFontFamilyValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Union_String_Resource& value,
                          const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        WriteTo(inputValueLabelStyle.font).family = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.family";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyResourceValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontFamilyStringValidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_String_Resource, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontFamilyInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontFamilyInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_Union_String_Resource& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        WriteTo(inputValueLabelStyle.font).family = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_FAMILY_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.family";
    };

    for (auto& [input, value] : Fixtures::testFixtureFontFamilyResourceInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontFamilyStringInvalidValues) {
        checkValue(input, Converter::ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", Converter::ArkUnion<Opt_Union_String_Resource, Ark_Empty>(nullptr));
    // Check empty optional
    checkValue("undefined", Converter::ArkValue<Opt_Union_String_Resource>());
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontStyleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontStyleValidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](
                          const std::string& input, const Opt_FontStyle& value, const std::string& expectedStr) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        WriteTo(inputValueLabelStyle.font).style = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_STYLE_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.style";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontStyleValidValues) {
        checkValue(input, Converter::ArkValue<Opt_FontStyle>(value), expected);
    }
}

/*
 * @tc.name: setLabelStyleTestLabelStyleFontStyleInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierTest, setLabelStyleTestLabelStyleFontStyleInvalidValues, TestSize.Level1)
{
    Ark_LabelStyle initValueLabelStyle;

    // Initial setup
    initValueLabelStyle.overflow =
        Converter::ArkValue<Opt_TextOverflow>(std::get<1>(Fixtures::testFixtureEnumTextOverflowValidValues[0]));
    initValueLabelStyle.maxLines =
        Converter::ArkValue<Opt_Number>(std::get<1>(Fixtures::testFixtureNumberPosIntFloorValidValues[0]));
    initValueLabelStyle.minFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.maxFontSize = Converter::ArkUnion<Opt_Union_Number_ResourceStr, Ark_Number>(
        std::get<1>(Fixtures::testFixtureDimensionsNumNonNegValidValues[0]));
    initValueLabelStyle.heightAdaptivePolicy = Converter::ArkValue<Opt_TextHeightAdaptivePolicy>(
        std::get<1>(Fixtures::testFixtureEnumTextHeightAdaptivePolicyValidValues[0]));
    WriteTo(initValueLabelStyle.font).size =
        Converter::ArkValue<Opt_Length>(std::get<1>(Fixtures::testFixtureLengthNonNegNonPctValidValues[0]));
    WriteTo(initValueLabelStyle.font).weight = Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    WriteTo(initValueLabelStyle.font).family = Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        std::get<1>(Fixtures::testFixtureFontFamilyResourceValidValues[0]));
    WriteTo(initValueLabelStyle.font).style =
        Converter::ArkValue<Opt_FontStyle>(std::get<1>(Fixtures::testFixtureEnumFontStyleValidValues[0]));

    auto checkValue = [this, &initValueLabelStyle](const std::string& input, const Opt_FontStyle& value) {
        Ark_LabelStyle inputValueLabelStyle = initValueLabelStyle;

        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        WriteTo(inputValueLabelStyle.font).style = value;
        modifier_->setLabelStyle(node_, &inputValueLabelStyle);
        auto jsonValue = GetJsonValue(node_);
        auto resultLabelStyle = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_LABEL_STYLE_NAME);
        auto resultFont = GetAttrValue<std::unique_ptr<JsonValue>>(resultLabelStyle, ATTRIBUTE_LABEL_STYLE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(resultFont, ATTRIBUTE_LABEL_STYLE_FONT_STYLE_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_LABEL_STYLE_FONT_STYLE_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setLabelStyle, attribute: labelStyle.font.style";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontStyleInvalidValues) {
        checkValue(input, Converter::ArkValue<Opt_FontStyle>(value));
    }
}
} // namespace OHOS::Ace::NG

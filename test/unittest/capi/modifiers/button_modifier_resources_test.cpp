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
#include "node_api.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/common/mock_theme_style.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "core/components/button/button_theme.h"

#include "core/interfaces/arkoala/utility/reverse_converter.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace Converter;

namespace {
    // attrs
    const auto ATTRIBUTE_LABEL_NAME("label");
    const auto ATTRIBUTE_TYPE_NAME("type");
    const auto ATTRIBUTE_ROLE_NAME("role");
    const auto ATTRIBUTE_STATE_EFFECT_NAME("stateEffect");
    const auto ATTRIBUTE_CONTROL_SIZE_NAME("controlSize");
    const auto ATTRIBUTE_BUTTON_STYLE_NAME("buttonStyle");

    using ButtonLabelResourceTest = std::tuple<ResourceStr, std::string>;

    // resource names and id
    const auto RES_NAME = NamedResourceId{"aa.bb.cc", NodeModifier::ResourceType::STRING};
    const auto RES_ID = IntResourceId{11111, NodeModifier::ResourceType::STRING};

    // resource values
    const auto RESOURCE_BY_STRING = "ResourceByString";
    const auto RESOURCE_BY_NUMBER = "ResourceByNumber";

    const std::vector<ButtonLabelResourceTest> BUTTON_LABEL_RESOURCES_TEST_PLAN = {
        { CreateResourceUnion<ResourceStr>(RES_NAME), RESOURCE_BY_STRING },
        { CreateResourceUnion<ResourceStr>(RES_ID), RESOURCE_BY_NUMBER },
    };
} // namespace

class ButtonModifierResourcesTest : public ModifierTestBase<GENERATED_ArkUIButtonModifier,
    &GENERATED_ArkUINodeModifiers::getButtonModifier, GENERATED_ARKUI_BUTTON> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<ButtonTheme>();

        // set test values to Theme Pattern as data for the Theme building
        AddResource(RES_ID, RESOURCE_BY_NUMBER);
        AddResource(RES_NAME, RESOURCE_BY_STRING);
    }
};

/*
 * @tc.name: SetButtonOptions2TestButtonRole
 * @tc.desc: Check the functionality of ButtonModifier.SetButtonOptions2
 * @tc.type: FUNC
 */
HWTEST_F(ButtonModifierResourcesTest, SetButtonOptions2TestLabelResource, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    // Initial setup
    Ark_ButtonOptions inputValueOptions;
    inputValueOptions.type = Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_CAPSULE);
    inputValueOptions.stateEffect = Converter::ArkValue<Opt_Boolean>(true);
    inputValueOptions.buttonStyle = Converter::ArkValue<Opt_ButtonStyleMode>(ARK_BUTTON_STYLE_MODE_NORMAL);
    inputValueOptions.controlSize = Converter::ArkValue<Opt_ControlSize>(ARK_CONTROL_SIZE_SMALL);
    inputValueOptions.role = Converter::ArkValue<Opt_ButtonRole>(ARK_BUTTON_ROLE_NORMAL);
    ResourceStr label;
    std::string expectValue;
    std::tie(label, expectValue) = BUTTON_LABEL_RESOURCES_TEST_PLAN.front();
    auto optInputValueOptions = Converter::ArkValue<Opt_ButtonOptions>(inputValueOptions);
    // Test
    modifier_->setButtonOptions2(node_, &label, &optInputValueOptions);
    // Initial verification
    jsonValue = GetJsonValue(node_);
    auto checkType = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_TYPE_NAME);
    auto checkStateEffect = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STATE_EFFECT_NAME);
    auto checkButtonStyle = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BUTTON_STYLE_NAME);
    auto checkControlSize = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTROL_SIZE_NAME);
    auto checkRole = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ROLE_NAME);
    auto checkLabel = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_LABEL_NAME);
    EXPECT_EQ(checkType, "ButtonType.Capsule");
    EXPECT_EQ(checkStateEffect, "true");
    EXPECT_EQ(checkButtonStyle, "ButtonStyleMode.NORMAL");
    EXPECT_EQ(checkControlSize, "ControlSize.SMALL");
    EXPECT_EQ(checkRole, "ButtonRole.NORMAL");
    EXPECT_EQ(checkLabel, expectValue);

    for (const auto& [label, expectValue] : BUTTON_LABEL_RESOURCES_TEST_PLAN) {
        modifier_->setButtonOptions2(node_, &label, &optInputValueOptions);
        checkLabel = GetAttrValue<std::string>(jsonValue, expectValue);
    }
}
} // namespace OHOS::Ace::NG

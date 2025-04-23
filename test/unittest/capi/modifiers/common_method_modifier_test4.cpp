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

#include "common_method_modifier_test.h"

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "test/mock/core/render/mock_render_context.h"

using namespace testing;
using namespace testing::ext;

void AssignArkValue(Ark_InvertOptions& dst, const float& value)
{
    auto arkVal = OHOS::Ace::NG::Converter::ArkValue<Ark_Number>(value);
    dst = {arkVal, arkVal, arkVal, arkVal};
}

void AssignArkValue(Ark_PixelStretchEffectOptions& dst, const Ark_Empty& value)
{
    dst.left = OHOS::Ace::NG::Converter::ArkValue<Opt_Length>();
    dst.top = OHOS::Ace::NG::Converter::ArkValue<Opt_Length>();
    dst.right = OHOS::Ace::NG::Converter::ArkValue<Opt_Length>();
    dst.bottom = OHOS::Ace::NG::Converter::ArkValue<Opt_Length>();
}

namespace OHOS::Ace {
void AssignArkValue(Ark_InvertOptions& dst, const InvertOption& value)
{
    dst.low = NG::Converter::ArkValue<Ark_Number>(value.low_);
    dst.high = NG::Converter::ArkValue<Ark_Number>(value.high_);
    dst.threshold = NG::Converter::ArkValue<Ark_Number>(value.threshold_);
    dst.thresholdRange = NG::Converter::ArkValue<Ark_Number>(value.thresholdRange_);
}
}

namespace OHOS::Ace::NG {
namespace {
    const auto ATTRIBUTE_COLOR_BLEND_NAME = "colorBlend";
    const auto ATTRIBUTE_COLOR_BLEND_DEFAULT_VALUE = "";
    const auto ATTRIBUTE_INVERT_NAME = "invert";
    const auto ATTRIBUTE_INVERT_DEFAULT_VALUE = "";
    const auto ATTRIBUTE_HUE_ROTATE_NAME = "hueRotate";
    const auto ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE = 0;
    const auto ATTRIBUTE_USE_EFFECT_NAME = "useEffect";
    const auto ATTRIBUTE_USE_EFFECT_DEFAULT_VALUE = false;
    const auto ATTRIBUTE_RENDER_GROUP_NAME = "renderGroup";
    const auto ATTRIBUTE_RENDER_GROUP_DEFAULT_VALUE = true;
    const auto ATTRIBUTE_FREEZE_NAME = "freeze";
    const auto ATTRIBUTE_FREEZE_DEFAULT_VALUE = false;
    const auto ATTRIBUTE_GRID_SPAN_NAME = "gridSpan";
    const auto ATTRIBUTE_GRID_SPAN_DEFAULT_VALUE = 1;
    const auto ATTRIBUTE_GRID_OFFSET_NAME = "gridOffset";
    const auto ATTRIBUTE_GRID_OFFSET_DEFAULT_VALUE = 0;
    const auto ATTRIBUTE_SPHERICAL_EFFECT_NAME = "sphericalEffect";
    const auto ATTRIBUTE_SPHERICAL_EFFECT_DEFAULT_VALUE = 0.0;
    const auto ATTRIBUTE_LIGHT_UP_EFFECT_NAME = "lightUpEffect";
    const auto ATTRIBUTE_LIGHT_UP_EFFECT_DEFAULT_VALUE = 0.0;
    const auto ATTRIBUTE_PIXEL_STRETCH_EFFECT_NAME = "pixelStretchEffect";
    const auto ATTRIBUTE_PIXEL_STRETCH_EFFECT_DEFAULT_VALUE =
        "{\"left\":\"0.00px\",\"right\":\"0.00px\",\"top\":\"0.00px\",\"bottom\":\"0.00px\"}";
    const auto ATTRIBUTE_ALIGN_RULES_NAME = "alignRules";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_NAME = "left";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_NAME = "right";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_NAME = "middle";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_NAME = "top";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_NAME = "bottom";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_NAME = "center";
const auto ATTRIBUTE_ALIGN_RULES_I_START_NAME = "start";
const auto ATTRIBUTE_ALIGN_RULES_I_END_NAME = "end";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_NAME = "bias";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_NAME = "horizontal";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_DEFAULT_VALUE = "0.5";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_NAME = "vertical";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_DEFAULT_VALUE = "0.5";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_OBSCURED_NAME = "obscured";
const auto ATTRIBUTE_EXPAND_SAFE_AREA_NAME = "expandSafeArea";
const auto ATTRIBUTE_EXPAND_SAFE_AREA_I_TYPES_NAME = "types";
const auto ATTRIBUTE_EXPAND_SAFE_AREA_I_EDGES_NAME = "edges";
const auto ATTRIBUTE_SAFE_AREA_PADDING_NAME = "safeAreaPadding";
const auto ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE = "0.00vp";
const auto ATTRIBUTE_SAFE_AREA_LEFT_NAME = "left";
const auto ATTRIBUTE_SAFE_AREA_TOP_NAME = "top";
const auto ATTRIBUTE_SAFE_AREA_RIGHT_NAME = "right";
const auto ATTRIBUTE_SAFE_AREA_BOTTOM_NAME = "bottom";
const auto ATTRIBUTE_ROTATE_NAME = "rotate";
const auto ATTRIBUTE_ROTATE_I_CENTER_Z_NAME = "centerZ";
const auto ATTRIBUTE_ROTATE_I_ANGLE_NAME = "angle";
const auto ATTRIBUTE_ROTATE_I_CENTER_Z_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_ARRAY_DEFAULT_SIZE = 0;
}
struct PixelStretchEffect {
    float left = 0.0;
    float top = 0.0;
    float right = 0.0;
    float bottom = 0.0;
};
void AssignArkValue(Ark_PixelStretchEffectOptions& dst, const PixelStretchEffect& value)
{
    auto left = Dimension(value.left);
    auto top = Dimension(value.top);
    auto right = Dimension(value.right);
    auto bottom = Dimension(value.bottom);
    dst.left = Converter::ArkValue<Opt_Length>(left);
    dst.top = Converter::ArkValue<Opt_Length>(top);
    dst.right = Converter::ArkValue<Opt_Length>(right);
    dst.bottom = Converter::ArkValue<Opt_Length>(bottom);
}
class CommonMethodModifierTest4 : public ModifierTestBase<GENERATED_ArkUICommonMethodModifier,
    &GENERATED_ArkUINodeModifiers::getCommonMethodModifier,
    GENERATED_ARKUI_BLANK // test common methods on frameNode for Blank component
    > {
public:
    RefPtr<RenderContext> render_;

    void *CreateNodeImpl() override
    {
        return nodeModifiers_->getBlankModifier()->construct(GetId(), 0);
    }
};

/*
 * @tc.name: setColorBlendTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setColorBlendTestDefaultValues, TestSize.Level1)
{
    std::string strResult = GetStringAttribute(node_, ATTRIBUTE_COLOR_BLEND_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_COLOR_BLEND_DEFAULT_VALUE);
}

/*
 * @tc.name: setColorBlendTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setColorBlendTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setColorBlend0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Color_String_Resource, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_WHITE), "#FFFFFFFF"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BLACK), "#FF000000"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BLUE), "#FF0000FF"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BROWN), "#FFA52A2A"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GRAY), "#FF808080"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GREEN), "#FF008000"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GREY), "#FF808080"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_ORANGE), "#FFFFA500"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_PINK), "#FFFFC0CB"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_RED), "#FFFF0000"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_YELLOW), "#FFFFFF00"},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_TRANSPARENT),
            "#00000000"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setColorBlend0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_COLOR_BLEND_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setColorBlend1TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setColorBlend1TestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setColorBlend1, nullptr);
    using OneTestStep = std::tuple<Opt_Union_Color_String_Resource, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_WHITE), "#FFFFFFFF"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BLACK), "#FF000000"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BLUE), "#FF0000FF"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_BROWN), "#FFA52A2A"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GRAY), "#FF808080"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GREEN), "#FF008000"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_GREY), "#FF808080"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_ORANGE), "#FFFFA500"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_PINK), "#FFFFC0CB"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_RED), "#FFFF0000"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_YELLOW), "#FFFFFF00"},
        {Converter::ArkUnion<Opt_Union_Color_String_Resource, Ark_Color>(Ark_Color::ARK_COLOR_TRANSPARENT),
            "#00000000"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setColorBlend1(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_COLOR_BLEND_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setColorBlendTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setColorBlendTestInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setColorBlend0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Color_String_Resource, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_String>(""), ""},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_String>("failValue"), ""},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setColorBlend0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_COLOR_BLEND_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setInvertTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setInvertTestDefaultValues, TestSize.Level1)
{
    std::string strResult = GetStringAttribute(node_, ATTRIBUTE_INVERT_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_INVERT_DEFAULT_VALUE);
}

/*
 * @tc.name: setInvertTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setInvertTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setInvert0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_InvertOptions, std::string>;
    InvertOption val1 = {1.0f, 2.0f, 3.0f, 4.0f};
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(0.0f), "0"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(0.0f),
            "{\"low\":0,\"high\":0,\"threshold\":0,\"thresholdRange\":0}"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(1.0f), "1"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(1.0f),
            "{\"low\":1,\"high\":1,\"threshold\":1,\"thresholdRange\":1}"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(
            Converter::ArkValue<Ark_InvertOptions>(val1)),
            "{\"low\":1,\"high\":2,\"threshold\":3,\"thresholdRange\":4}"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setInvert0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_INVERT_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setInvert1TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setInvert1TestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setInvert1, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_InvertOptions, std::string>;
    InvertOption val1 = {1.0f, 2.0f, 3.0f, 4.0f};
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(0.0f), "0"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(0.0f),
            "{\"low\":0,\"high\":0,\"threshold\":0,\"thresholdRange\":0}"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(1.0f), "1"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(1.0f),
            "{\"low\":1,\"high\":1,\"threshold\":1,\"thresholdRange\":1}"},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(
            Converter::ArkValue<Ark_InvertOptions>(val1)),
            "{\"low\":1,\"high\":2,\"threshold\":3,\"thresholdRange\":4}"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        Opt_Union_Number_InvertOptions optValue = { .value = inputValue };
        modifier_->setInvert1(node_, &optValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_INVERT_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setInvertTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setInvertTestInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setInvert0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_InvertOptions, std::string>;
    InvertOption val1 = {-1.0, 2.0, 3.0, 4.0};
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(Converter::ArkValue<Ark_Number>(-1.0)), ""},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(Converter::ArkValue<Ark_Number>(-2.0)), ""},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(
            Converter::ArkValue<Ark_InvertOptions>(val1)), ""},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setInvert0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_INVERT_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setHueRotateDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setHueRotateDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetInt(ATTRIBUTE_HUE_ROTATE_NAME, ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE);
}

/*
 * @tc.name: setHueRotateValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setHueRotateValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setHueRotate0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_String, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0)), 0.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(1.0)), 1.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("4.0")), 4.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("5.0")), 5.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("-90.0")), -90.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setHueRotate0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_HUE_ROTATE_NAME, ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setHueRotate1ValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setHueRotate1ValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setHueRotate1, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_String, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0)), 0.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(1.0)), 1.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("4.0")), 4.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("5.0")), 5.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("-90.0")), -90.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        Opt_Union_Number_String optValue = { .value = inputValue};
        modifier_->setHueRotate1(node_, &optValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_HUE_ROTATE_NAME, ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setHueRotateInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setHueRotateInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setHueRotate0, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_String, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("")), 0.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("badValue")), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setHueRotate0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_HUE_ROTATE_NAME, ATTRIBUTE_HUE_ROTATE_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
////////////// UseEffect
/*
 * @tc.name: setUseEffectDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setUseEffectDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_USE_EFFECT_NAME, ATTRIBUTE_USE_EFFECT_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_USE_EFFECT_DEFAULT_VALUE);
}

/*
 * @tc.name: setUseEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, DISABLED_setUseEffectValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setUseEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_Boolean, bool>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Boolean>(true), true},
        {Converter::ArkValue<Ark_Boolean>(false), false},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setUseEffect0(node_, inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetBool(ATTRIBUTE_USE_EFFECT_NAME, ATTRIBUTE_USE_EFFECT_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << (expectedValue ? "true" : "false");
    }
}
////////////// RenderGroup
/*
 * @tc.name: setRenderGroupDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRenderGroupDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_RENDER_GROUP_NAME, ATTRIBUTE_RENDER_GROUP_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_RENDER_GROUP_DEFAULT_VALUE);
}

/*
 * @tc.name: setRenderGroupValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, DISABLED_setRenderGroupValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setRenderGroup0, nullptr);
    using OneTestStep = std::tuple<Ark_Boolean, bool>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Boolean>(true), true},
        {Converter::ArkValue<Ark_Boolean>(false), false},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setRenderGroup0(node_, inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetBool(ATTRIBUTE_RENDER_GROUP_NAME, ATTRIBUTE_RENDER_GROUP_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << (expectedValue ? "true" : "false");
    }
}
////////////// Freeze
/*
 * @tc.name: setFreezeDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setFreezeDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_FREEZE_NAME, ATTRIBUTE_FREEZE_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_FREEZE_DEFAULT_VALUE);
}

/*
 * @tc.name: setFreezeValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, DISABLED_setFreezeValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setFreeze0, nullptr);
    using OneTestStep = std::tuple<Ark_Boolean, bool>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Boolean>(true), true},
        {Converter::ArkValue<Ark_Boolean>(false), false},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setFreeze0(node_, inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetBool(ATTRIBUTE_FREEZE_NAME, ATTRIBUTE_FREEZE_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << (expectedValue ? "true" : "false");
    }
}
////////////// GridSpan
/*
 * @tc.name: setGridSpanDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridSpanDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_GRID_SPAN_NAME, ATTRIBUTE_GRID_SPAN_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_GRID_SPAN_DEFAULT_VALUE);
}

/*
 * @tc.name: setGridSpanValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridSpanValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setGridSpan, nullptr);
    using OneTestStep = std::tuple<Ark_Number, int32_t>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0), 0},
        {Converter::ArkValue<Ark_Number>(1), 1},
        {Converter::ArkValue<Ark_Number>(2), 2},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setGridSpan(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_GRID_SPAN_NAME, ATTRIBUTE_GRID_SPAN_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setGridSpanInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridSpanInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setGridSpan, nullptr);
    using OneTestStep = std::tuple<Ark_Number, int32_t>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1), 1},
        {Converter::ArkValue<Ark_Number>(-2), 1},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setGridSpan(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_GRID_SPAN_NAME, ATTRIBUTE_GRID_SPAN_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
////////////// GridOffset
/*
 * @tc.name: setGridOffsetDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridOffsetDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_GRID_OFFSET_NAME, ATTRIBUTE_GRID_OFFSET_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_GRID_OFFSET_DEFAULT_VALUE);
}

/*
 * @tc.name: setGridOffsetValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridOffsetValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setGridOffset, nullptr);
    using OneTestStep = std::tuple<Ark_Number, int32_t>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0), 0},
        {Converter::ArkValue<Ark_Number>(1), 1},
        {Converter::ArkValue<Ark_Number>(2), 2},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setGridOffset(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_GRID_OFFSET_NAME, ATTRIBUTE_GRID_OFFSET_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setGridOffsetInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setGridOffsetInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setGridOffset, nullptr);
    using OneTestStep = std::tuple<Ark_Number, int32_t>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1), 0},
        {Converter::ArkValue<Ark_Number>(-2), 0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setGridOffset(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetInt(ATTRIBUTE_GRID_OFFSET_NAME, ATTRIBUTE_GRID_OFFSET_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
////////////// SphericalEffect
/*
 * @tc.name: setSphericalEffectDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSphericalEffectDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetBool(ATTRIBUTE_SPHERICAL_EFFECT_NAME, ATTRIBUTE_SPHERICAL_EFFECT_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_SPHERICAL_EFFECT_DEFAULT_VALUE);
}

/*
 * @tc.name: setSphericalEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSphericalEffectValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setSphericalEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0.0), 0.0},
        {Converter::ArkValue<Ark_Number>(0.4), 0.4},
        {Converter::ArkValue<Ark_Number>(0.5), 0.5},
        {Converter::ArkValue<Ark_Number>(0.6), 0.6},
        {Converter::ArkValue<Ark_Number>(1.0), 1.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setSphericalEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetDouble(ATTRIBUTE_SPHERICAL_EFFECT_NAME,
            ATTRIBUTE_SPHERICAL_EFFECT_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setSphericalEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSphericalEffectInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setSphericalEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1.0), 0.0},
        {Converter::ArkValue<Ark_Number>(-2.0), 0.0},
        {Converter::ArkValue<Ark_Number>(1.5), 0.0},
        {Converter::ArkValue<Ark_Number>(2.0), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setSphericalEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetDouble(ATTRIBUTE_SPHERICAL_EFFECT_NAME,
            ATTRIBUTE_SPHERICAL_EFFECT_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
////////////// LightUpEffect
/*
 * @tc.name: setLightUpEffectDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setLightUpEffectDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = jsonValue->GetDouble(ATTRIBUTE_LIGHT_UP_EFFECT_NAME, ATTRIBUTE_LIGHT_UP_EFFECT_DEFAULT_VALUE);
    EXPECT_EQ(resultValue, ATTRIBUTE_LIGHT_UP_EFFECT_DEFAULT_VALUE);
}

/*
 * @tc.name: setLightUpEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setLightUpEffectValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setLightUpEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0.0), 0.0},
        {Converter::ArkValue<Ark_Number>(0.4), 0.4},
        {Converter::ArkValue<Ark_Number>(0.5), 0.5},
        {Converter::ArkValue<Ark_Number>(0.6), 0.6},
        {Converter::ArkValue<Ark_Number>(1.0), 1.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setLightUpEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetDouble(ATTRIBUTE_LIGHT_UP_EFFECT_NAME, ATTRIBUTE_LIGHT_UP_EFFECT_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setLightUpEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setLightUpEffectInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setLightUpEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1.0), 0.0},
        {Converter::ArkValue<Ark_Number>(-2.0), 0.0},
        {Converter::ArkValue<Ark_Number>(1.5), 0.0},
        {Converter::ArkValue<Ark_Number>(2.0), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setLightUpEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = fullJson->GetDouble(ATTRIBUTE_LIGHT_UP_EFFECT_NAME, ATTRIBUTE_LIGHT_UP_EFFECT_DEFAULT_VALUE);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
////////////// PixelStretchEffect
/*
 * @tc.name: setPixelStretchEffectDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setPixelStretchEffectDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultValue = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_PIXEL_STRETCH_EFFECT_NAME);
    EXPECT_EQ(resultValue, ATTRIBUTE_PIXEL_STRETCH_EFFECT_DEFAULT_VALUE);
}

/*
 * @tc.name: setPixelStretchEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setPixelStretchEffectValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setPixelStretchEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_PixelStretchEffectOptions, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_PixelStretchEffectOptions>(
            PixelStretchEffect({.left = 0.0, .top = 0.0, .right = 0.0, .bottom = 0.0})),
            "{\"left\":\"0.00px\",\"right\":\"0.00px\",\"top\":\"0.00px\",\"bottom\":\"0.00px\"}"},
        {Converter::ArkValue<Ark_PixelStretchEffectOptions>(
            PixelStretchEffect({.left = -1.0, .top = -2.0, .right = -3.0, .bottom = -4.0})),
            "{\"left\":\"-1.00px\",\"right\":\"-3.00px\",\"top\":\"-2.00px\",\"bottom\":\"-4.00px\"}"},
        {Converter::ArkValue<Ark_PixelStretchEffectOptions>(
            PixelStretchEffect({.left = 1.0, .top = 2.0, .right = 3.0, .bottom = 4.0})),
            "{\"left\":\"1.00px\",\"right\":\"3.00px\",\"top\":\"2.00px\",\"bottom\":\"4.00px\"}"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setPixelStretchEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_PIXEL_STRETCH_EFFECT_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setPixelStretchEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setPixelStretchEffectInvalidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setPixelStretchEffect0, nullptr);
    using OneTestStep = std::tuple<Ark_PixelStretchEffectOptions, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {{.top = {}, .bottom = {}, .left = {}, .right = {}},
            "{\"left\":\"0.00px\",\"right\":\"0.00px\",\"top\":\"0.00px\",\"bottom\":\"0.00px\"}"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setPixelStretchEffect0(node_, &inputValue);
        auto fullJson = GetJsonValue(node_);
        auto resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_PIXEL_STRETCH_EFFECT_NAME);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}

/*
 * @tc.name: setAlignRules1TestDefaultValues1
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setAlignRules1TestDefaultValues1, TestSize.Level1)
{
    auto jsonValue = GetJsonValue(node_);
    auto resultAlignRules = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ALIGN_RULES_NAME);
    auto resultLeft = GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_LEFT_NAME);
    auto resultRight = GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_RIGHT_NAME);
    auto resultStart = GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_START_NAME);
    auto resultEnd = GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_END_NAME);
    auto resultMiddle =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_MIDDLE_NAME);
    auto resultBias = GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_BIAS_NAME);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(resultLeft, ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.anchor'";
    resultStr = GetAttrValue<std::string>(resultLeft, ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.anchor'";
    resultStr = GetAttrValue<std::string>(resultRight, ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.anchor'";
    resultStr = GetAttrValue<std::string>(resultRight, ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.anchor'";
    resultStr = GetAttrValue<std::string>(resultStart, ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.anchor'";
    resultStr = GetAttrValue<std::string>(resultStart, ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.start.align'";
    resultStr = GetAttrValue<std::string>(resultEnd, ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.end.anchor'";
    resultStr = GetAttrValue<std::string>(resultEnd, ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.end.align'";
    resultStr = GetAttrValue<std::string>(resultMiddle, ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.middle.anchor'";
    resultStr = GetAttrValue<std::string>(resultMiddle, ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.middle.align'";
    resultStr = GetAttrValue<std::string>(resultBias, ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.bias.horizontal'";
    resultStr = GetAttrValue<std::string>(resultBias, ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.bias.vertical'";
}

/*
 * @tc.name: setAlignRules1TestDefaultValues2
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setAlignRules1TestDefaultValues2, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultAlignRules =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ALIGN_RULES_NAME);

    std::unique_ptr<JsonValue> resultTop =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_TOP_NAME);
    std::unique_ptr<JsonValue> resultBottom =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_BOTTOM_NAME);
    std::unique_ptr<JsonValue> resultCenter =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_CENTER_NAME);
    std::unique_ptr<JsonValue> resultBias =
        GetAttrValue<std::unique_ptr<JsonValue>>(resultAlignRules, ATTRIBUTE_ALIGN_RULES_I_BIAS_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(resultTop, ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.top.anchor'";

    resultStr = GetAttrValue<std::string>(resultTop, ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.top.align'";

    resultStr = GetAttrValue<std::string>(resultBottom, ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.bottom.anchor'";

    resultStr = GetAttrValue<std::string>(resultBottom, ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.bottom.align'";

    resultStr = GetAttrValue<std::string>(resultCenter, ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.center.anchor'";

    resultStr = GetAttrValue<std::string>(resultCenter, ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_DEFAULT_VALUE) <<
        "Default value for attribute 'alignRules.center.align'";
}

/*
 * @tc.name: setObscuredTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setObscuredTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    auto jsonArray = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OBSCURED_NAME);
    EXPECT_EQ(jsonArray->GetArraySize(), ATTRIBUTE_ARRAY_DEFAULT_SIZE);
}

/*
 * @tc.name: setObscuredTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setObscuredTestValues, TestSize.Level1)
{
    std::vector<Ark_ObscuredReasons> vecReason = {ARK_OBSCURED_REASONS_PLACEHOLDER, ARK_OBSCURED_REASONS_PLACEHOLDER,
        ARK_OBSCURED_REASONS_PLACEHOLDER};
    std::vector<Ark_ObscuredReasons> vecInvalidReason = { static_cast<Ark_ObscuredReasons>(-100)};
    Converter::ArkArrayHolder<Array_ObscuredReasons> vecHolder(vecReason);
    Array_ObscuredReasons vecArkReason = vecHolder.ArkValue();
    modifier_->setObscured(node_, &vecArkReason);
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    auto jsonArray = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OBSCURED_NAME);
    EXPECT_EQ(jsonArray->GetArraySize(), vecReason.size());
    for (int i = 0; i < jsonArray->GetArraySize(); i++) {
        auto itemJson = jsonArray->GetArrayItem(i);
        auto value = std::to_string(static_cast<int32_t>(vecReason[i]));
        EXPECT_EQ(itemJson->GetString(), value);
    }
}

/*
 * @tc.name: setObscuredTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setObscuredTestInvalidValues, TestSize.Level1)
{
    std::vector<Ark_ObscuredReasons> vecReason = { static_cast<Ark_ObscuredReasons>(-100),
        static_cast<Ark_ObscuredReasons>(INT_MAX)};
    Converter::ArkArrayHolder<Array_ObscuredReasons> vecHolder(vecReason);
    Array_ObscuredReasons vecArkReason = vecHolder.ArkValue();
    modifier_->setObscured(node_, &vecArkReason);
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    auto jsonArray = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_OBSCURED_NAME);
    EXPECT_EQ(jsonArray->GetArraySize(), ATTRIBUTE_ARRAY_DEFAULT_SIZE);
}

void checkExpandSafeArea(Ark_NodeHandle node, std::vector<std::string> vecTypes,
    std::vector<std::string> vecEdges)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node);
    auto jsonExpSA = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_EXPAND_SAFE_AREA_NAME);
    auto jsonArray = GetAttrValue<std::unique_ptr<JsonValue>>(jsonExpSA, ATTRIBUTE_EXPAND_SAFE_AREA_I_TYPES_NAME);
    EXPECT_EQ(jsonArray->GetArraySize(), vecTypes.size());
    for (int i = 0; i < jsonArray->GetArraySize(); i++) {
        auto itemJson = jsonArray->GetArrayItem(i);
        EXPECT_EQ(itemJson->GetString(), vecTypes[i]);
    }
    jsonArray = GetAttrValue<std::unique_ptr<JsonValue>>(jsonExpSA, ATTRIBUTE_EXPAND_SAFE_AREA_I_EDGES_NAME);
    EXPECT_EQ(jsonArray->GetArraySize(), vecEdges.size());
    for (int i = 0; i < jsonArray->GetArraySize(); i++) {
        auto itemJson = jsonArray->GetArrayItem(i);
        EXPECT_EQ(itemJson->GetString(), vecEdges[i]);
    }
}
/*
 * @tc.name: setExpandSafeAreaTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setExpandSafeAreaTestDefaultValues, TestSize.Level1)
{
    std::vector<std::string> vecTypes = {"SafeAreaType.SYSTEM", "SafeAreaType.CUTOUT", "SafeAreaType.KEYBOARD"};
    std::vector<std::string> vecEdges = {"SafeAreaEdge.TOP", "SafeAreaEdge.BOTTOM", "SafeAreaEdge.START",
        "SafeAreaEdge.END"};
    checkExpandSafeArea(node_, vecTypes, vecEdges);
}

/*
 * @tc.name: setExpandSafeAreaTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setExpandSafeAreaTestValidValues, TestSize.Level1)
{
    std::vector<std::string> expectedTypes = {"SafeAreaType.SYSTEM", "SafeAreaType.CUTOUT",
        "SafeAreaType.KEYBOARD"};
    std::vector<std::string> expectedEdges = {"SafeAreaEdge.TOP", "SafeAreaEdge.BOTTOM", "SafeAreaEdge.START",
        "SafeAreaEdge.END"};
    std::vector<Ark_SafeAreaType> vecTypes = {ARK_SAFE_AREA_TYPE_SYSTEM, ARK_SAFE_AREA_TYPE_CUTOUT,
        ARK_SAFE_AREA_TYPE_KEYBOARD};
    Converter::ArkArrayHolder<Array_SafeAreaType> vecHolderT(vecTypes);
    auto optArrayArkTypes = vecHolderT.OptValue<Opt_Array_SafeAreaType>();
    std::vector<Ark_SafeAreaEdge> vecEdges = {ARK_SAFE_AREA_EDGE_TOP, ARK_SAFE_AREA_EDGE_BOTTOM,
        ARK_SAFE_AREA_EDGE_START, ARK_SAFE_AREA_EDGE_END};
    Converter::ArkArrayHolder<Array_SafeAreaEdge> vecHolderE(vecEdges);
    auto optArrayArkEdge = vecHolderE.OptValue<Opt_Array_SafeAreaEdge>();
    modifier_->setExpandSafeArea(node_, &optArrayArkTypes, &optArrayArkEdge);
    checkExpandSafeArea(node_, expectedTypes, expectedEdges);
}

/*
 * @tc.name: setExpandSafeAreaTestValidValues2
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setExpandSafeAreaTestValidValues2, TestSize.Level1)
{
    std::vector<std::string> expectedTypes = {"SafeAreaType.SYSTEM", "SafeAreaType.KEYBOARD"};
    std::vector<std::string> expectedEdges = {"SafeAreaEdge.TOP", "SafeAreaEdge.END"};
    std::vector<Ark_SafeAreaType> vecTypes = {ARK_SAFE_AREA_TYPE_SYSTEM, ARK_SAFE_AREA_TYPE_KEYBOARD};
    Converter::ArkArrayHolder<Array_SafeAreaType> vecHolderT(vecTypes);
    auto optArrayArkTypes = vecHolderT.OptValue<Opt_Array_SafeAreaType>();
    std::vector<Ark_SafeAreaEdge> vecEdges = {ARK_SAFE_AREA_EDGE_TOP, ARK_SAFE_AREA_EDGE_END};
    Converter::ArkArrayHolder<Array_SafeAreaEdge> vecHolderE(vecEdges);
    auto optArrayArkEdge = vecHolderE.OptValue<Opt_Array_SafeAreaEdge>();
    modifier_->setExpandSafeArea(node_, &optArrayArkTypes, &optArrayArkEdge);
    checkExpandSafeArea(node_, expectedTypes, expectedEdges);
}

/*
 * @tc.name: setExpandSafeAreaTestInvalidValues1
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setExpandSafeAreaTestInvalidValues1, TestSize.Level1)
{
    std::vector<std::string> expectedTypes = {"SafeAreaType.SYSTEM", "SafeAreaType.KEYBOARD"};
    std::vector<std::string> expectedEdges = {"SafeAreaEdge.TOP", "SafeAreaEdge.END"};
    std::vector<Ark_SafeAreaType> vecTypes = {ARK_SAFE_AREA_TYPE_SYSTEM, ARK_SAFE_AREA_TYPE_SYSTEM,
        ARK_SAFE_AREA_TYPE_SYSTEM, ARK_SAFE_AREA_TYPE_KEYBOARD, ARK_SAFE_AREA_TYPE_KEYBOARD};
    Converter::ArkArrayHolder<Array_SafeAreaType> vecHolderT(vecTypes);
    auto optArrayArkTypes = vecHolderT.OptValue<Opt_Array_SafeAreaType>();
    std::vector<Ark_SafeAreaEdge> vecEdges = {ARK_SAFE_AREA_EDGE_TOP, ARK_SAFE_AREA_EDGE_END};
    Converter::ArkArrayHolder<Array_SafeAreaEdge> vecHolderE(vecEdges);
    auto optArrayArkEdge = vecHolderE.OptValue<Opt_Array_SafeAreaEdge>();
    modifier_->setExpandSafeArea(node_, &optArrayArkTypes, &optArrayArkEdge);
    checkExpandSafeArea(node_, expectedTypes, expectedEdges);
}

/*
 * @tc.name: setExpandSafeAreaTestInvalidValues2
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setExpandSafeAreaTestInvalidValues2, TestSize.Level1)
{
    std::vector<std::string> expectedTypes = {"SafeAreaType.KEYBOARD"};
    std::vector<std::string> expectedEdges = {"SafeAreaEdge.TOP", "SafeAreaEdge.END"};
    std::vector<Ark_SafeAreaType> vecTypes = {static_cast<Ark_SafeAreaType>(-100), ARK_SAFE_AREA_TYPE_KEYBOARD,
        static_cast<Ark_SafeAreaType>(100), ARK_SAFE_AREA_TYPE_KEYBOARD, static_cast<Ark_SafeAreaType>(2)};
    Converter::ArkArrayHolder<Array_SafeAreaType> vecHolderT(vecTypes);
    auto optArrayArkTypes = vecHolderT.OptValue<Opt_Array_SafeAreaType>();
    std::vector<Ark_SafeAreaEdge> vecEdges = {ARK_SAFE_AREA_EDGE_TOP, ARK_SAFE_AREA_EDGE_END};
    Converter::ArkArrayHolder<Array_SafeAreaEdge> vecHolderE(vecEdges);
    auto optArrayArkEdge = vecHolderE.OptValue<Opt_Array_SafeAreaEdge>();
    modifier_->setExpandSafeArea(node_, &optArrayArkTypes, &optArrayArkEdge);
    checkExpandSafeArea(node_, expectedTypes, expectedEdges);
}

/*
 * @tc.name: setSafeAreaPaddingTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingTestDefaultValues, TestSize.Level1)
{
    std::string strResult;
    strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
}

typedef std::pair<Ark_LengthMetrics, std::string> LengthMetricsOneTestStep;
std::vector<LengthMetricsOneTestStep> testLengthMetricsValues = {
    { Converter::ArkValue<Ark_LengthMetrics>(123.0_vp), "123.00vp" },
    { Converter::ArkValue<Ark_LengthMetrics>(0.0_vp), "0.00vp" },
    { Converter::ArkValue<Ark_LengthMetrics>(1.23_vp), "1.23vp" },
    { Converter::ArkValue<Ark_LengthMetrics>(123.0_fp), "123.00fp" },
    { Converter::ArkValue<Ark_LengthMetrics>(0.0_fp), "0.00fp" },
    { Converter::ArkValue<Ark_LengthMetrics>(1.23_fp), "1.23fp" },
    { Converter::ArkValue<Ark_LengthMetrics>(123.0_px), "123.00px" },
    { Converter::ArkValue<Ark_LengthMetrics>(0.0_px), "0.00px" },
    { Converter::ArkValue<Ark_LengthMetrics>(1.23_px), "1.23px" },
    { Converter::ArkValue<Ark_LengthMetrics>(0.5_pct), "50.00%" },
    { Converter::ArkValue<Ark_LengthMetrics>(0.0_pct), "0.00%" },
};
/*
 * @tc.name: setSafeAreaPaddingLengthMetricsTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingLengthMetricsTestValues, TestSize.Level1)
{
    std::string strResult;
    strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);

    for (const auto &[arkPadding, expected]: testLengthMetricsValues) {
        auto value = Converter::ArkUnion<Ark_Union_Padding_LengthMetrics_LocalizedPadding,
            Ark_LengthMetrics>(arkPadding);
        modifier_->setSafeAreaPadding(node_, &value);
        strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

typedef std::pair<Ark_Length, std::string> lengthOneTestStep;
std::vector<lengthOneTestStep> testLengthValues = {
    { Converter::ArkValue<Ark_Length>(123.0_vp), "123.00vp" },
    { Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp" },
    { Converter::ArkValue<Ark_Length>(1.23_vp), "1.23vp" },
    { Converter::ArkValue<Ark_Length>(123.0_fp), "123.00fp" },
    { Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp" },
    { Converter::ArkValue<Ark_Length>(1.23_fp), "1.23fp" },
    { Converter::ArkValue<Ark_Length>(123.0_px), "123.00px" },
    { Converter::ArkValue<Ark_Length>(0.0_px), "0.00px" },
    { Converter::ArkValue<Ark_Length>(1.23_px), "1.23px" },
    { Converter::ArkValue<Ark_Length>(0.5_pct), "50.00%" },
    { Converter::ArkValue<Ark_Length>(0.0_pct), "0.00%" },
};
/*
 * @tc.name: setSafeAreaPaddingLeftArkPaddingTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingLeftArkPaddingTestValues, TestSize.Level1)
{
    auto initVal = Converter::ArkValue<Opt_Length>(Ark_Empty());
    Ark_Padding inputValue = {.left = initVal, .top = initVal, .right = initVal, .bottom= initVal };
    for (const auto &[arkPadding, expected]: testLengthValues) {
        inputValue.left = Converter::ArkValue<Opt_Length>(arkPadding);
        auto value = Converter::ArkUnion<Ark_Union_Padding_LengthMetrics_LocalizedPadding, Ark_Padding>(inputValue);
        modifier_->setSafeAreaPadding(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
        auto leftResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_LEFT_NAME);
        auto topResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_TOP_NAME);
        auto rightResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_RIGHT_NAME);
        auto bottomResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_BOTTOM_NAME);
        EXPECT_EQ(leftResult, expected);
        EXPECT_EQ(topResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(rightResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(bottomResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
    }
}

/*
 * @tc.name: setSafeAreaPaddingTopArkPaddingTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingTopArkPaddingTestValues, TestSize.Level1)
{
    auto initVal = Converter::ArkValue<Opt_Length>(Ark_Empty());
    Ark_Padding inputValue = {.left = initVal, .top = initVal, .right = initVal, .bottom= initVal };
    for (const auto &[arkPadding, expected]: testLengthValues) {
        inputValue.top = Converter::ArkValue<Opt_Length>(arkPadding);
        auto value = Converter::ArkUnion<Ark_Union_Padding_LengthMetrics_LocalizedPadding, Ark_Padding>(inputValue);
        modifier_->setSafeAreaPadding(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
        auto leftResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_LEFT_NAME);
        auto topResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_TOP_NAME);
        auto rightResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_RIGHT_NAME);
        auto bottomResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_BOTTOM_NAME);
        EXPECT_EQ(leftResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(topResult, expected);
        EXPECT_EQ(rightResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(bottomResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
    }
}

/*
 * @tc.name: setSafeAreaPaddingRightArkPaddingTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingRightArkPaddingTestValues, TestSize.Level1)
{
    auto initVal = Converter::ArkValue<Opt_Length>(Ark_Empty());
    Ark_Padding inputValue = {.left = initVal, .top = initVal, .right = initVal, .bottom= initVal };
    for (const auto &[arkPadding, expected]: testLengthValues) {
        inputValue.right = Converter::ArkValue<Opt_Length>(arkPadding);
        auto value = Converter::ArkUnion<Ark_Union_Padding_LengthMetrics_LocalizedPadding, Ark_Padding>(inputValue);
        modifier_->setSafeAreaPadding(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
        auto leftResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_LEFT_NAME);
        auto topResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_TOP_NAME);
        auto rightResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_RIGHT_NAME);
        auto bottomResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_BOTTOM_NAME);
        EXPECT_EQ(leftResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(topResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(rightResult, expected);
        EXPECT_EQ(bottomResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
    }
}

/*
 * @tc.name: setSafeAreaPaddingBottomArkPaddingTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setSafeAreaPaddingBottomArkPaddingTestValues, TestSize.Level1)
{
    auto initVal = Converter::ArkValue<Opt_Length>(Ark_Empty());
    Ark_Padding inputValue = {.left = initVal, .top = initVal, .right = initVal, .bottom= initVal };
    for (const auto &[arkPadding, expected]: testLengthValues) {
        inputValue.bottom = Converter::ArkValue<Opt_Length>(arkPadding);
        auto value = Converter::ArkUnion<Ark_Union_Padding_LengthMetrics_LocalizedPadding, Ark_Padding>(inputValue);
        modifier_->setSafeAreaPadding(node_, &value);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_SAFE_AREA_PADDING_NAME);
        auto leftResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_LEFT_NAME);
        auto topResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_TOP_NAME);
        auto rightResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_RIGHT_NAME);
        auto bottomResult = GetAttrValue<std::string>(strResult, ATTRIBUTE_SAFE_AREA_BOTTOM_NAME);
        EXPECT_EQ(leftResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(topResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(rightResult, ATTRIBUTE_SAFE_AREA_PADDING_DEFAULT_VALUE);
        EXPECT_EQ(bottomResult, expected);
    }
}

typedef std::pair<SizeF, SizeF> sizeOneTestStep;
std::vector<sizeOneTestStep> vecSizePair = {
    {SizeF(100.0f, 1000.0f), SizeF(50.0f, 1001.0f)},
    {SizeF(-100.0f, -1000.0f), SizeF(50.0f, 1001.0f)},
    {SizeF(100.0f, 1000.0f), SizeF(-50.0f, -1001.0f)},
};
/*
 * @tc.name: setOnSizeChangeTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setOnSizeChangeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    struct CheckEvent {
        int32_t nodeId;
        SizeF oldSize;
        SizeF newSize;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;

    auto callback = [](Ark_Int32 nodeId, const Ark_SizeOptions oldValue, const Ark_SizeOptions newValue) {
        checkEvent = {
            .nodeId = nodeId,
        };
        auto arkLen = Converter::OptConvert<Ark_Length>(oldValue.width);
        auto sizeW = arkLen.has_value() ? arkLen.value().value : 0.0f;
        arkLen = Converter::OptConvert<Ark_Length>(oldValue.height);
        auto sizeH = arkLen.has_value() ? arkLen.value().value : 0.0f;
        checkEvent->oldSize = SizeF(sizeW, sizeH);
        arkLen = Converter::OptConvert<Ark_Length>(newValue.width);
        sizeW = arkLen.has_value() ? arkLen.value().value : 0.0f;
        arkLen = Converter::OptConvert<Ark_Length>(newValue.height);
        sizeH = arkLen.has_value() ? arkLen.value().value : 0.0f;
        checkEvent->newSize = SizeF(sizeW, sizeH);
    };
    static constexpr int32_t contextId = 123;
    SizeChangeCallback callBackValue =
        Converter::ArkValue<SizeChangeCallback>(callback, contextId);
    auto test = [this, &callBackValue, eventHub, frameNode](const sizeOneTestStep testSize) {
        checkEvent = std::nullopt;
        modifier_->setOnSizeChange(node_, &callBackValue);
        auto offset = OffsetF(0.0f, 0.0f);
        eventHub->FireOnSizeChanged(RectF(offset, testSize.first), RectF(offset, testSize.second));
        ASSERT_TRUE(checkEvent.has_value());
        EXPECT_EQ(checkEvent->oldSize.Width(), testSize.first.Width());
        EXPECT_EQ(checkEvent->oldSize.Height(), testSize.first.Height());
        EXPECT_EQ(checkEvent->newSize.Width(), testSize.second.Width());
        EXPECT_EQ(checkEvent->newSize.Height(), testSize.second.Height());
    };
    for (int i = 0; i < vecSizePair.size(); i++) {
        test(vecSizePair[i]);
    }
}

std::vector<std::tuple<std::string, Ark_Number, std::string>> dimensionsFLoatValidValues = {
    { "123", Converter::ArkValue<Ark_Number>(123), "123.00vp" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0.00vp" },
    { "1.23", Converter::ArkValue<Ark_Number>(1.23), "1.23vp" },
    { "-2", Converter::ArkValue<Ark_Number>(-2), "-2.00vp" },
    { "-3.45", Converter::ArkValue<Ark_Number>(-3.45), "-3.45vp" },
};

/*
 * @tc.name: setRotate0TestRotateCenterZValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate0TestRotateCenterZValidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr, const Ark_Number& value) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(value);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(defNumValue);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = optValue;
        modifier_->setRotate0(node_, &inputValueRotate);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_CENTER_Z_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate0, attribute: rotate.centerZ";
    };

    for (auto& [input, value, expected] : dimensionsFLoatValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setRotate0TestRotateCenterZInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate0TestRotateCenterZInvalidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr,
                          const Ark_Number& value, const Opt_Number& invalidValue) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(value);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(defNumValue);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = invalidValue;
        modifier_->setRotate0(node_, &inputValueRotate);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_CENTER_Z_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate0, attribute: rotate.centerZ";
    };

    for (auto& [input, value, expected] : dimensionsFLoatValidValues) {
        checkValue(input, ATTRIBUTE_ROTATE_I_CENTER_Z_DEFAULT_VALUE, value, Converter::ArkValue<Opt_Number>());
    }
}

/*
 * @tc.name: setRotate1TestRotateCenterZValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate1TestRotateCenterZValidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr, const Ark_Number& value) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(value);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(defNumValue);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = optValue;

        auto optRotateOptions = Converter::ArkValue<Opt_RotateOptions>(inputValueRotate);
        modifier_->setRotate1(node_, &optRotateOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_CENTER_Z_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate1, attribute: rotate.centerZ";
    };

    for (auto& [input, value, expected] : dimensionsFLoatValidValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setRotate1TestRotateCenterZInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate1TestRotateCenterZInvalidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr,
                          const Ark_Number& value, const Opt_Number& invalidValue) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(value);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(defNumValue);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = invalidValue;
        auto optRotateOptions = Converter::ArkValue<Opt_RotateOptions>(inputValueRotate);
        modifier_->setRotate1(node_, &optRotateOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_CENTER_Z_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate1, attribute: rotate.centerZ";
    };

    for (auto& [input, value, expected] : dimensionsFLoatValidValues) {
        checkValue(input, ATTRIBUTE_ROTATE_I_CENTER_Z_DEFAULT_VALUE, value, Converter::ArkValue<Opt_Number>());
    }
}

std::vector<std::tuple<std::string, Ark_Number, std::string>> testNumberFloatValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100.000000" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0.000000" },
    { "-100", Converter::ArkValue<Ark_Number>(-100), "-100.000000" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "12.340000" },
    { "-56.73", Converter::ArkValue<Ark_Number>(-56.73), "-56.730000" },
};

std::vector<std::tuple<std::string, Ark_String, std::string>> testStringFloatValues = {
    { "100", Converter::ArkValue<Ark_String>("100"), "100.000000" },
    { "0", Converter::ArkValue<Ark_String>("0"), "0.000000" },
    { "-100", Converter::ArkValue<Ark_String>("-100"), "-100.000000" },
    { "12.34", Converter::ArkValue<Ark_String>("12.34"), "12.340000" },
    { "-56.73", Converter::ArkValue<Ark_String>("-56.73"), "-56.730000" },
};

/*
 * @tc.name: setRotate0TestRotateAngleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate0TestRotateAngleValidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr, const Ark_Number& value) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = optValue;
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = optValue;

        modifier_->setRotate0(node_, &inputValueRotate);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_ANGLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate0, attribute: rotate.angle";
    };

    for (auto& [input, value, expected] : testNumberFloatValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setRotate0TestRotateAngleStringValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate0TestRotateAngleStringValidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr, const Ark_String& value) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = optValue;
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(value);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>(value);
        inputValueRotate.centerZ = optValue;

        modifier_->setRotate0(node_, &inputValueRotate);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_ANGLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate0, attribute: rotate.angle";
    };

    for (auto& [input, value, expected] : testStringFloatValues) {
        checkValue(input, expected, value);
    }
}

/*
 * @tc.name: setRotate1TestRotateAngleValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, setRotate1TestRotateAngleValidValues, TestSize.Level1)
{
    Ark_Number defNumValue = std::get<1>(dimensionsFLoatValidValues[0]);
    auto checkValue = [this, defNumValue](
                          const std::string& input, const std::string& expectedStr, const Ark_Number& value) {
        Ark_RotateOptions inputValueRotate;
        auto optValue = Converter::ArkValue<Opt_Number>(defNumValue);
        inputValueRotate.x = optValue;
        inputValueRotate.y = optValue;
        inputValueRotate.z = optValue;
        inputValueRotate.perspective = optValue;
        inputValueRotate.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(value);
        inputValueRotate.centerZ = optValue;

        auto optRotateOptions = Converter::ArkValue<Opt_RotateOptions>(inputValueRotate);
        modifier_->setRotate1(node_, &optRotateOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultRotate = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_ROTATE_NAME);
        auto resultStr = GetAttrValue<std::string>(resultRotate, ATTRIBUTE_ROTATE_I_ANGLE_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setRotate1, attribute: rotate.angle";
    };

    for (auto& [input, value, expected] : testNumberFloatValues) {
        checkValue(input, expected, value);
    }
}

struct AutoTransitionEffectPeer {
    const GENERATED_ArkUITransitionEffectAccessor* const accessor_;
    TransitionEffectPeer* peer_;
    static constexpr auto translateX = 123.4567f;
    static constexpr auto translateY = 765.4321f;
    static constexpr auto translateZ = 77.001f;
    static constexpr auto angle = 10.f;
    static constexpr auto perspective = 45.f;

    explicit  AutoTransitionEffectPeer(const GENERATED_ArkUIFullNodeAPI* fullAPI,
            ChainedTransitionEffectType type = ChainedTransitionEffectType::TRANSLATE)
        :accessor_(fullAPI->getAccessors()->getTransitionEffectAccessor())
    {
        if (type == ChainedTransitionEffectType::TRANSLATE) {
            Ark_TranslateOptions value;
            value.x = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(translateX);
            value.y = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(translateY);
            value.z = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(translateZ);
            peer_ = accessor_->translate(&value);
        }
        if (type == ChainedTransitionEffectType::ROTATE) {
            Ark_RotateOptions value;
            value.x = Converter::ArkValue<Opt_Number>(translateX);
            value.y = Converter::ArkValue<Opt_Number>(translateY);
            value.z = Converter::ArkValue<Opt_Number>(translateZ);
            value.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(translateX);
            value.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(translateY);
            value.centerZ = Converter::ArkValue<Opt_Number>(translateZ);
            value.perspective = Converter::ArkValue<Opt_Number>(perspective);
            value.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(angle);
            peer_ = accessor_->rotate(&value);
        }
    }
    ~AutoTransitionEffectPeer() { accessor_->destroyPeer(peer_); }
    Ark_TransitionEffect GetArkValue() const
    {
        return peer_;
    }
    ACE_DISALLOW_COPY_AND_MOVE(AutoTransitionEffectPeer);
};

/*
 * @tc.name: CommonMethod Transition0TransitionEffectTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, Transition0TransitionEffectTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    const auto& rContext = AceType::DynamicCast<MockRenderContext>(frameNode->GetRenderContext());
    ASSERT_NE(rContext, nullptr);
    AutoTransitionEffectPeer peer(fullAPI_, ChainedTransitionEffectType::TRANSLATE);
    const auto materialized = peer.GetArkValue();
    auto effect = Converter::ArkUnion<Ark_Union_TransitionOptions_TransitionEffect,
    Ark_TransitionEffect>(materialized);
    modifier_->setTransition0(node_, &effect);
    auto rcEffect = rContext->chainedTransitionEffect_;
    ASSERT_NE(rcEffect, nullptr);
    EXPECT_EQ(rcEffect->GetType(), ChainedTransitionEffectType::TRANSLATE);
}

/*
 * @tc.name: CommonMethod Transition0TransitionEffect2Test
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, Transition0TransitionEffect2Test, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    const auto& rContext = AceType::DynamicCast<MockRenderContext>(frameNode->GetRenderContext());
    ASSERT_NE(rContext, nullptr);
    AutoTransitionEffectPeer peer(fullAPI_, ChainedTransitionEffectType::ROTATE);
    const auto materialized = peer.GetArkValue();
    auto effect = Converter::ArkUnion<Ark_Union_TransitionOptions_TransitionEffect,
    Ark_TransitionEffect>(materialized);
    modifier_->setTransition0(node_, &effect);
    auto rcEffect = rContext->chainedTransitionEffect_;
    ASSERT_NE(rcEffect, nullptr);
    EXPECT_EQ(rcEffect->GetType(), ChainedTransitionEffectType::ROTATE);
}

/*
 * @tc.name: CommonMethod Transition1TransitionEffectCbTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest4, Transition1TransitionEffectCbTest, TestSize.Level1)
{
    struct CheckEvent {
        int32_t nodeId;
        bool isTransitionIn;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    const auto& rContext = AceType::DynamicCast<MockRenderContext>(frameNode->GetRenderContext());
    ASSERT_NE(rContext, nullptr);
    AutoTransitionEffectPeer peer(fullAPI_);
    const auto materialized = peer.GetArkValue();
    auto callback = [](Ark_Int32 nodeId, const Ark_Boolean transitionIn) {
        checkEvent = {.nodeId = nodeId, .isTransitionIn = transitionIn};
    };
    static constexpr int32_t contextId = 123;
    ::TransitionFinishCallback callBackValue =
        Converter::ArkValue<::TransitionFinishCallback>(callback, contextId);
    auto optCb = Converter::ArkValue<Opt_TransitionFinishCallback>(callBackValue);
    modifier_->setTransition1(node_, materialized, &optCb);
    auto rcEffect = rContext->chainedTransitionEffect_;
    ASSERT_NE(rcEffect, nullptr);
    EXPECT_EQ(rcEffect->GetType(), ChainedTransitionEffectType::TRANSLATE);
    ASSERT_NE(rContext->transitionUserCallback_, nullptr);
    rContext->transitionUserCallback_(true);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_TRUE(checkEvent->isTransitionIn);
    rContext->transitionUserCallback_(false);
    EXPECT_FALSE(checkEvent->isTransitionIn);
    modifier_->setTransition1(node_, nullptr, &optCb);
    rContext->transitionUserCallback_ = nullptr;
    rContext->chainedTransitionEffect_ = nullptr;
}
}

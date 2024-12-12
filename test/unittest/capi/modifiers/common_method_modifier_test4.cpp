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

using namespace testing;
using namespace testing::ext;

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
const auto ATTRIBUTE_ARRAY_DEFAULT_SIZE = 0;
}
struct PixelStretchEffect {
    float left = 0.0;
    float top = 0.0;
    float right = 0.0;
    float bottom = 0.0;
};
namespace Converter {
    template<>
    Ark_InvertOptions ArkValue(const float& value)
    {
        auto arkVal = ArkValue<Ark_Number>(value);
        return {arkVal, arkVal, arkVal, arkVal};
    }
    template<>
    Ark_InvertOptions ArkValue(const InvertOption& value)
    {
        return {
            .low = ArkValue<Ark_Number>(value.low_),
            .high = ArkValue<Ark_Number>(value.high_),
            .threshold = ArkValue<Ark_Number>(value.threshold_),
            .thresholdRange = ArkValue<Ark_Number>(value.thresholdRange_),
        };
    }
    template<>
    Ark_PixelStretchEffectOptions ArkValue(const Ark_Empty& value)
    {
        Ark_PixelStretchEffectOptions dst;
        dst.left = Converter::ArkValue<Opt_Length>(Dimension(0.0));
        dst.top = Converter::ArkValue<Opt_Length>(Dimension(0.0));
        dst.right = Converter::ArkValue<Opt_Length>(Dimension(0.0));
        dst.bottom = Converter::ArkValue<Opt_Length>(Dimension(0.0));
        return dst;
    }
    template<>
    Ark_PixelStretchEffectOptions ArkValue(const PixelStretchEffect& value)
    {
        Ark_PixelStretchEffectOptions dst;
        auto left = Dimension(value.left);
        auto top = Dimension(value.top);
        auto right = Dimension(value.right);
        auto bottom = Dimension(value.bottom);
        dst.left = Converter::ArkValue<Opt_Length>(left);
        dst.top = Converter::ArkValue<Opt_Length>(top);
        dst.right = Converter::ArkValue<Opt_Length>(right);
        dst.bottom = Converter::ArkValue<Opt_Length>(bottom);
        return dst;
    }
}
class CommonMethodModifierTest4 : public ModifierTestBase<GENERATED_ArkUICommonMethodModifier,
    &GENERATED_ArkUINodeModifiers::getCommonMethodModifier,
    GENERATED_ARKUI_BLANK // test common methods on frameNode for Blank component
    > {
public:
    RefPtr<RenderContext> render_;
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
    ASSERT_NE(modifier_->setColorBlend, nullptr);
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
        modifier_->setColorBlend(node_, &inputValue);
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
    ASSERT_NE(modifier_->setColorBlend, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Color_String_Resource, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_String>(""), ""},
        {Converter::ArkUnion<Ark_Union_Color_String_Resource, Ark_String>("failValue"), ""},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setColorBlend(node_, &inputValue);
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
    ASSERT_NE(modifier_->setInvert, nullptr);
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
        modifier_->setInvert(node_, &inputValue);
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
    ASSERT_NE(modifier_->setInvert, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_InvertOptions, std::string>;
    InvertOption val1 = {-1.0, 2.0, 3.0, 4.0};
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(Converter::ArkValue<Ark_Number>(-1.0)), ""},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_Number>(Converter::ArkValue<Ark_Number>(-2.0)), ""},
        {Converter::ArkUnion<Ark_Union_Number_InvertOptions, Ark_InvertOptions>(
            Converter::ArkValue<Ark_InvertOptions>(val1)), ""},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setInvert(node_, &inputValue);
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
    ASSERT_NE(modifier_->setHueRotate, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_String, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(0.0)), 0.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(Converter::ArkValue<Ark_Number>(1.0)), 1.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("4.0")), 4.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("5.0")), 5.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("-90.0")), -90.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setHueRotate(node_, &inputValue);
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
    ASSERT_NE(modifier_->setHueRotate, nullptr);
    using OneTestStep = std::tuple<Ark_Union_Number_String, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("")), 0.0},
        {Converter::ArkUnion<Ark_Union_Number_String, Ark_String>(Converter::ArkValue<Ark_String>("badValue")), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setHueRotate(node_, &inputValue);
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
    ASSERT_NE(modifier_->setRenderGroup, nullptr);
    using OneTestStep = std::tuple<Ark_Boolean, bool>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Boolean>(true), true},
        {Converter::ArkValue<Ark_Boolean>(false), false},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setRenderGroup(node_, inputValue);
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
    ASSERT_NE(modifier_->setFreeze, nullptr);
    using OneTestStep = std::tuple<Ark_Boolean, bool>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Boolean>(true), true},
        {Converter::ArkValue<Ark_Boolean>(false), false},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setFreeze(node_, inputValue);
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
    ASSERT_NE(modifier_->setSphericalEffect, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0.0), 0.0},
        {Converter::ArkValue<Ark_Number>(0.4), 0.4},
        {Converter::ArkValue<Ark_Number>(0.5), 0.5},
        {Converter::ArkValue<Ark_Number>(0.6), 0.6},
        {Converter::ArkValue<Ark_Number>(1.0), 1.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setSphericalEffect(node_, &inputValue);
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
    ASSERT_NE(modifier_->setSphericalEffect, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1.0), 0.0},
        {Converter::ArkValue<Ark_Number>(-2.0), 0.0},
        {Converter::ArkValue<Ark_Number>(1.5), 0.0},
        {Converter::ArkValue<Ark_Number>(2.0), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setSphericalEffect(node_, &inputValue);
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
    ASSERT_NE(modifier_->setLightUpEffect, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(0.0), 0.0},
        {Converter::ArkValue<Ark_Number>(0.4), 0.4},
        {Converter::ArkValue<Ark_Number>(0.5), 0.5},
        {Converter::ArkValue<Ark_Number>(0.6), 0.6},
        {Converter::ArkValue<Ark_Number>(1.0), 1.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setLightUpEffect(node_, &inputValue);
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
    ASSERT_NE(modifier_->setLightUpEffect, nullptr);
    using OneTestStep = std::tuple<Ark_Number, float>;
    static const std::vector<OneTestStep> testPlan = {
        {Converter::ArkValue<Ark_Number>(-1.0), 0.0},
        {Converter::ArkValue<Ark_Number>(-2.0), 0.0},
        {Converter::ArkValue<Ark_Number>(1.5), 0.0},
        {Converter::ArkValue<Ark_Number>(2.0), 0.0},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setLightUpEffect(node_, &inputValue);
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
    ASSERT_NE(modifier_->setPixelStretchEffect, nullptr);
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
        modifier_->setPixelStretchEffect(node_, &inputValue);
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
    ASSERT_NE(modifier_->setPixelStretchEffect, nullptr);
    using OneTestStep = std::tuple<Ark_PixelStretchEffectOptions, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {{.top = {}, .bottom = {}, .left = {}, .right = {}},
            "{\"left\":\"0.00px\",\"right\":\"0.00px\",\"top\":\"0.00px\",\"bottom\":\"0.00px\"}"},
    };
    for (auto [inputValue, expectedValue]: testPlan) {
        modifier_->setPixelStretchEffect(node_, &inputValue);
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
}

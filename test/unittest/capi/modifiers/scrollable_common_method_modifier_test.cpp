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
#include "test/unittest/capi/modifiers/generated/test_fixtures.h"
#include "test/unittest/capi/modifiers/generated/type_helpers.h"

#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/pattern/scrollable/scrollable_theme.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/peer_utils.h"
#include "core/components/scroll/scrollable.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/interfaces/native/node/scrollable_modifier.h"
#include "native_node.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
using namespace Converter;
using namespace TypeHelper;
namespace {
const auto ATTRIBUTE_EDGE_EFFECT_NAME = "edgeEffect";
const auto ATTRIBUTE_EDGE_EFFECT_DEFAULT_VALUE = "EdgeEffect.None";
const auto ATTRIBUTE_EDGE_EFFECT_OPTIONS_NAME = "edgeEffectOptions";
const auto ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_NAME = "alwaysEnabled";
const auto ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_FRICTION_NAME = "friction";
const auto ATTRIBUTE_FRICTION_DEFAULT_VALUE = "0.6";
const auto ATTRIBUTE_BACK_TO_TOP_NAME = "backToTop";
const auto ATTRIBUTE_BACK_TO_TOP_DEFAULT_VALUE = "false";
#ifdef SUPPORT_DIGITAL_CROWN
const auto ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_DEFAULT_VALUE = "1";
const auto ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_NAME = "digitCrownSensitivity";
#endif
constexpr auto EXPECTED_TRUE = "true";
constexpr auto EXPECTED_FALSE = "false";
} // namespace

class ScrollableCommonMethodModifierTest
    : public ModifierTestBase<GENERATED_ArkUIScrollableCommonMethodModifier,
          &GENERATED_ArkUINodeModifiers::getScrollableCommonMethodModifier, GENERATED_ARKUI_WATER_FLOW> {
public:
    void *CreateNodeImpl() override
    {
        return nodeModifiers_->getWaterFlowModifier()->construct(GetId(), 0);
    }

    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<ScrollBarTheme>();
        SetupTheme<ScrollableTheme>();
    }

    void OnModifyDone()
    {
        auto frameNode = reinterpret_cast<FrameNode*>(node_);
        ASSERT_NE(frameNode, nullptr);
        auto pattern = frameNode->GetPattern();
        ASSERT_TRUE(pattern);
        pattern->OnModifyDone();
    }
};

/*
 * @tc.name: setEdgeEffectTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setEdgeEffectTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultOptions =
        GetAttrObject(jsonValue, ATTRIBUTE_EDGE_EFFECT_OPTIONS_NAME);
    std::optional<std::string> resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
    EXPECT_THAT(resultStr, Eq(ATTRIBUTE_EDGE_EFFECT_DEFAULT_VALUE)) <<
        "Default value for attribute 'edgeEffect.edgeEffect'";

    resultStr = GetAttrValue<std::string>(resultOptions, ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_NAME);
    EXPECT_THAT(resultStr, Eq(ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_DEFAULT_VALUE)) <<
        "Default value for attribute 'edgeEffect.edgeEffectOptions.alwaysEnabled'";
}

/*
 * @tc.name: setEdgeEffectTestEdgeEffectEdgeEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setEdgeEffectTestEdgeEffectEdgeEffectValidValues, TestSize.Level1)
{
    Opt_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    WriteTo(initValueEdgeEffect) = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_EdgeEffect& value, const std::string& expectedStr) {
        Opt_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        inputValueEdgeEffect = Converter::ArkValue<Opt_EdgeEffect>(value);
        modifier_->setEdgeEffect(node_, &inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
        EXPECT_THAT(resultStr, Eq(expectedStr)) <<
            "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.edgeEffect";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumEdgeEffectValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setEdgeEffectTestEdgeEffectEdgeEffectInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(
    ScrollableCommonMethodModifierTest, setEdgeEffectTestEdgeEffectEdgeEffectInvalidValues, TestSize.Level1)
{
    Opt_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    WriteTo(initValueEdgeEffect) = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_EdgeEffect& value) {
        Opt_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        modifier_->setEdgeEffect(node_, &inputValueEdgeEffect, &inputValueOptions);
        inputValueEdgeEffect = Converter::ArkValue<Opt_EdgeEffect>(value);
        modifier_->setEdgeEffect(node_, &inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
        EXPECT_THAT(resultStr, Eq(ATTRIBUTE_EDGE_EFFECT_DEFAULT_VALUE)) <<
            "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.edgeEffect";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumEdgeEffectInvalidValues) {
        checkValue(input, value);
    }
}

/*
 * @tc.name: setEdgeEffectTestEdgeEffectOptionsAlwaysEnabledValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setEdgeEffectTestEdgeEffectOptionsAlwaysEnabledValidValues,
    TestSize.Level1)
{
    Opt_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    WriteTo(initValueEdgeEffect) = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_Boolean& value, const std::string& expectedStr) {
        Opt_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        WriteTo(inputValueOptions).alwaysEnabled = value;
        modifier_->setEdgeEffect(node_, &inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultOptions =
            GetAttrObject(jsonValue, ATTRIBUTE_EDGE_EFFECT_OPTIONS_NAME);
        auto resultStr =
            GetAttrValue<std::string>(resultOptions, ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.options.alwaysEnabled";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, value, expected);
    }
}

/*
 * @tc.name: setFrictionTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setFrictionTestDefaultValues, TestSize.Level1)
{
    OnModifyDone();
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::optional<std::string> resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FRICTION_NAME);
    EXPECT_THAT(resultStr, Eq(ATTRIBUTE_FRICTION_DEFAULT_VALUE)) << "Default value for attribute 'friction'";
}

/*
 * @tc.name: setOnReachStartTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setOnReachStartTest, TestSize.Level1)
{
    const int32_t contextId = 123;

    static std::optional<int32_t> checkData;
    auto checkCallback = [](const Ark_Int32 resourceId) { checkData = resourceId; };
    ASSERT_FALSE(checkData.has_value());

    // setup the callback object via C-API
    auto arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_Callback_Void>(arkCallback);
    modifier_->setOnReachStart(node_, &optCallback);

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto onReachStart = eventHub->GetOnReachStart();
    ASSERT_NE(onReachStart, nullptr);
    onReachStart();

    ASSERT_TRUE(checkData.has_value());
    EXPECT_EQ(checkData.value(), contextId);
}

/*
 * @tc.name: setOnReachEndTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, DISABLED_setOnReachEndTest, TestSize.Level1)
{
    const int32_t contextId = 123;

    static std::optional<int32_t> checkData;
    auto checkCallback = [](const Ark_Int32 resourceId) { checkData = resourceId; };
    ASSERT_FALSE(checkData.has_value());

    // setup the callback object via C-API
    auto arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_Callback_Void>(arkCallback);
    modifier_->setOnReachEnd(node_, &optCallback);

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto onReachEnd = eventHub->GetOnReachEnd();
    onReachEnd();

    ASSERT_TRUE(checkData.has_value());
    EXPECT_EQ(checkData.value(), contextId);
}

/*
 * @tc.name: setOnScrollStartTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setOnScrollStartTest, TestSize.Level1)
{
    const int32_t contextId = 123;

    static std::optional<int32_t> checkData;
    auto checkCallback = [](const Ark_Int32 resourceId) { checkData = resourceId; };
    ASSERT_FALSE(checkData.has_value());

    // setup the callback object via C-API
    auto arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_Callback_Void>(arkCallback);
    modifier_->setOnScrollStart(node_, &optCallback);

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto onReachStart = eventHub->GetOnScrollStart();
    ASSERT_NE(onReachStart, nullptr);
    onReachStart();

    ASSERT_TRUE(checkData.has_value());
    EXPECT_EQ(checkData.value(), contextId);
}

/*
 * @tc.name: setOnScrollStopTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setOnScrollStopTest, TestSize.Level1)
{
    const int32_t contextId = 123;

    static std::optional<int32_t> checkData;
    auto checkCallback = [](const Ark_Int32 resourceId) { checkData = resourceId; };
    ASSERT_FALSE(checkData.has_value());

    // setup the callback object via C-API
    auto arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_Callback_Void>(arkCallback);
    modifier_->setOnScrollStop(node_, &optCallback);

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto onReachStart = eventHub->GetOnScrollStop();
    ASSERT_NE(onReachStart, nullptr);
    onReachStart();

    ASSERT_TRUE(checkData.has_value());
    EXPECT_EQ(checkData.value(), contextId);
}

#ifdef SUPPORT_DIGITAL_CROWN
std::vector<std::tuple<std::string, Opt_CrownSensitivity, std::string>> testFixtureEnumCrownSensitivityTestPlan = {
    { "CrownSensitivity.LOW", Converter::ArkValue<Opt_CrownSensitivity>(ARK_CROWN_SENSITIVITY_LOW),
        "0" },
    { "CrownSensitivity.MEDIUM", Converter::ArkValue<Opt_CrownSensitivity>(ARK_CROWN_SENSITIVITY_MEDIUM),
        "1" },
    { "CrownSensitivity.HIGH", Converter::ArkValue<Opt_CrownSensitivity>(ARK_CROWN_SENSITIVITY_HIGH),
        "2" },
    { "-1", Converter::ArkValue<Opt_CrownSensitivity>(static_cast<Ark_CrownSensitivity>(-1)),
        ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_DEFAULT_VALUE },
    { "INT_MAX", Converter::ArkValue<Opt_CrownSensitivity>(static_cast<Ark_CrownSensitivity>(INT_MAX)),
        ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_DEFAULT_VALUE },
    { "Ark_Empty", Converter::ArkValue<Opt_CrownSensitivity>(static_cast<Ark_CrownSensitivity>(INT_MAX)),
        ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_DEFAULT_VALUE },
    { "CrownSensitivity.MEDIUM", Converter::ArkValue<Opt_CrownSensitivity>(ARK_CROWN_SENSITIVITY_MEDIUM),
        "1" },
};
/*
 * @tc.name: setDigitalCrownSensitivityTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setDigitalCrownSensitivityTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_NAME);
    EXPECT_THAT(resultStr, Eq(ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_DEFAULT_VALUE)) <<
        "Default value for attribute 'digitalCrownSensitivity'";
}

/*
 * @tc.name: setDigitalCrownSensitivityTestValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setDigitalCrownSensitivityTestValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setDigitalCrownSensitivity, nullptr);
    auto checkValue = [this](
                        const std::string& input, const std::string& expectedStr, const Opt_CrownSensitivity& value) {
        Opt_CrownSensitivity inputValueDigitalCrownSensitivity = value;
        modifier_->setDigitalCrownSensitivity(node_, &inputValueDigitalCrownSensitivity);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_DIGITAL_CROWN_SENSITIVITY_NAME);
        EXPECT_THAT(resultStr, Eq(expectedStr)) << "Input value is: " << input <<
                                        ", method: setDigitalCrownSensitivity, attribute: digitalCrownSensitivity";
    };
    for (auto& [input, value, expected] : testFixtureEnumCrownSensitivityTestPlan) {
        checkValue(input, expected, value);
    }
}
#endif

/**
 * @tc.name: setBackToTopTest
 * @tc.desc: Check backToTop method of ScrollableCommonMethod modifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, DISABLED_setBackToTopTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBackToTop, nullptr);
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);

    auto checkVal = GetAttrValue<std::string>(node_, ATTRIBUTE_BACK_TO_TOP_NAME);
    EXPECT_THAT(checkVal, Eq(ATTRIBUTE_BACK_TO_TOP_DEFAULT_VALUE));
    auto value = Converter::ArkValue<Opt_Boolean>(false);
    modifier_->setBackToTop(node_, &value);
    checkVal = GetAttrValue<std::string>(node_, ATTRIBUTE_BACK_TO_TOP_NAME);
    EXPECT_THAT(checkVal, Eq(EXPECTED_FALSE));
    value = Converter::ArkValue<Opt_Boolean>(true);
    modifier_->setBackToTop(node_, &value);
    checkVal = GetAttrValue<std::string>(node_, ATTRIBUTE_BACK_TO_TOP_NAME);
    EXPECT_THAT(checkVal, Eq(EXPECTED_TRUE));
}
namespace {
constexpr ArkUI_Int32 ERROR_INT_CODE = -1;

const ArkUIScrollableModifier* GetScrollableModifier()
{
    return NodeModifier::GetScrollableModifier();
}

ArkUINodeHandle CreateScrollableNode(const char* tag, RefPtr<Pattern> pattern, std::vector<RefPtr<FrameNode>>& keep)
{
    auto frameNode = FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    keep.emplace_back(frameNode);
    return reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode));
}
} // namespace

class ScrollableGetScrollStateTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
    }
    static void TearDownTestCase()
    {
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: GetScrollStateNullNodeReturnsError
 * @tc.desc: NODE_SCROLL_STATE query on a null node handle returns ERROR_INT_CODE, not a valid state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateNullNodeReturnsError, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->getScrollState, nullptr);
    EXPECT_EQ(modifier->getScrollState(nullptr), ERROR_INT_CODE);
}

/**
 * @tc.name: GetScrollStateNonScrollableNodeReturnsError
 * @tc.desc: node without ScrollablePattern (base pattern only) returns ERROR_INT_CODE, not a valid state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateNonScrollableNodeReturnsError, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto node = CreateScrollableNode("plain", AceType::MakeRefPtr<Pattern>(), keep);
    EXPECT_EQ(modifier->getScrollState(node), ERROR_INT_CODE);
}

/**
 * @tc.name: GetScrollStateScrollIdleByDefault
 * @tc.desc: NODE_SCROLL_STATE on Scroll component returns IDLE before any scroll session starts
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateScrollIdleByDefault, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto node = CreateScrollableNode("scroll", AceType::MakeRefPtr<ScrollPattern>(), keep);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_IDLE));
}

/**
 * @tc.name: GetScrollStateListScrollingReturnsScroll
 * @tc.desc: NODE_SCROLL_STATE on List component returns SCROLL during an active scroll session (fix: was empty item)
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateListScrollingReturnsScroll, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto listPattern = AceType::MakeRefPtr<ListPattern>();
    auto node = CreateScrollableNode("list", listPattern, keep);
    listPattern->SetScrollSourceForQuery(SCROLL_FROM_UPDATE);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_SCROLL));
}

/**
 * @tc.name: GetScrollStateGridFlingReturnsFling
 * @tc.desc: NODE_SCROLL_STATE on Grid component returns FLING while snap/fling animation is running
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateGridFlingReturnsFling, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto gridPattern = AceType::MakeRefPtr<GridPattern>();
    auto node = CreateScrollableNode("grid", gridPattern, keep);
    gridPattern->SetScrollSourceForQuery(SCROLL_FROM_ANIMATION);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_FLING));
}

/**
 * @tc.name: GetScrollStateWaterFlowScrollingReturnsScroll
 * @tc.desc: NODE_SCROLL_STATE on WaterFlow component returns SCROLL during an active scroll session
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateWaterFlowScrollingReturnsScroll, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto waterFlowPattern = AceType::MakeRefPtr<WaterFlowPattern>();
    auto node = CreateScrollableNode("waterflow", waterFlowPattern, keep);
    waterFlowPattern->SetScrollSourceForQuery(SCROLL_FROM_UPDATE);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_SCROLL));
}

/**
 * @tc.name: GetScrollStateListResetToIdle
 * @tc.desc: session-level query source reset (SCROLL_FROM_NONE) brings NODE_SCROLL_STATE back to IDLE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableGetScrollStateTest, GetScrollStateListResetToIdle, TestSize.Level1)
{
    auto* modifier = GetScrollableModifier();
    ASSERT_NE(modifier, nullptr);
    std::vector<RefPtr<FrameNode>> keep;
    auto listPattern = AceType::MakeRefPtr<ListPattern>();
    auto node = CreateScrollableNode("list", listPattern, keep);
    listPattern->SetScrollSourceForQuery(SCROLL_FROM_UPDATE);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_SCROLL));
    listPattern->SetScrollSourceForQuery(SCROLL_FROM_NONE);
    EXPECT_EQ(modifier->getScrollState(node), static_cast<ArkUI_Int32>(ARKUI_SCROLL_STATE_IDLE));
}
} // namespace OHOS::Ace::NG

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
const auto ATTRIBUTE_FRICTION_DEFAULT_VALUE = "0.75";
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
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_EDGE_EFFECT_OPTIONS_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_EDGE_EFFECT_DEFAULT_VALUE)
        << "Default value for attribute 'edgeEffect.edgeEffect'";

    resultStr = GetAttrValue<std::string>(resultOptions, ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_DEFAULT_VALUE)
        << "Default value for attribute 'edgeEffect.edgeEffectOptions.alwaysEnabled'";
}

/*
 * @tc.name: setEdgeEffectTestEdgeEffectEdgeEffectValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setEdgeEffectTestEdgeEffectEdgeEffectValidValues, TestSize.Level1)
{
    Ark_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    initValueEdgeEffect = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_EdgeEffect& value, const std::string& expectedStr) {
        Ark_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        inputValueEdgeEffect = value;
        modifier_->setEdgeEffect(node_, inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.edgeEffect";
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
    Ark_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    initValueEdgeEffect = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_EdgeEffect& value) {
        Ark_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        modifier_->setEdgeEffect(node_, inputValueEdgeEffect, &inputValueOptions);
        inputValueEdgeEffect = value;
        modifier_->setEdgeEffect(node_, inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_EDGE_EFFECT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_EDGE_EFFECT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.edgeEffect";
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
    Ark_EdgeEffect initValueEdgeEffect;
    Opt_EdgeEffectOptions initValueOptions;

    // Initial setup
    initValueEdgeEffect = std::get<1>(Fixtures::testFixtureEnumEdgeEffectValidValues[0]);
    WriteTo(initValueOptions).alwaysEnabled = std::get<1>(Fixtures::testFixtureBooleanValidValues[0]);

    auto checkValue = [this, &initValueEdgeEffect, &initValueOptions](
                          const std::string& input, const Ark_Boolean& value, const std::string& expectedStr) {
        Ark_EdgeEffect inputValueEdgeEffect = initValueEdgeEffect;
        Opt_EdgeEffectOptions inputValueOptions = initValueOptions;

        WriteTo(inputValueOptions).alwaysEnabled = value;
        modifier_->setEdgeEffect(node_, inputValueEdgeEffect, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultOptions =
            GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_EDGE_EFFECT_OPTIONS_NAME);
        auto resultStr =
            GetAttrValue<std::string>(resultOptions, ATTRIBUTE_EDGE_EFFECT_OPTIONS_ALWAYS_ENABLED_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setEdgeEffect, attribute: edgeEffect.options.alwaysEnabled";
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
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FRICTION_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FRICTION_DEFAULT_VALUE) << "Default value for attribute 'friction'";
}

/*
 * @tc.name: setOnWillScrollTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setOnWillScrollTest, TestSize.Level1)
{
    using namespace Converter;
    static const int32_t expectedResId = 123;
    static const Dimension expectedOffset = 555.0_vp;
    static const ScrollState expectedState = ScrollState::SCROLL;
    
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto callSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId,
                            const Ark_Number xOffset, const Ark_Number yOffset,
                            Ark_ScrollState scrollState, Ark_ScrollSource scrollSource,
                            const Callback_OffsetResult_Void continuation) {
        EXPECT_EQ(Convert<int32_t>(resourceId), expectedResId);
        EXPECT_EQ(OptConvert<ScrollState>(scrollState).value_or(ScrollState()), expectedState);

        Ark_OffsetResult arkResult {
            .xOffset = xOffset,
            .yOffset = yOffset
        };
        CallbackHelper(continuation).Invoke(arkResult);
    };
    auto func = ArkValue<ScrollOnWillScrollCallback>(nullptr, callSyncFunc, expectedResId);
    auto funcOpt = ArkValue<Opt_ScrollOnWillScrollCallback>(func);
    modifier_->setOnWillScroll(node_, &funcOpt);

    auto fireOnWillScroll = eventHub->GetOnWillScroll();
    ASSERT_NE(fireOnWillScroll, nullptr);
    auto result = fireOnWillScroll(expectedOffset, expectedState, ScrollSource::SCROLL_BAR);
    EXPECT_EQ(result.offset, expectedOffset);
}

/**
 * @tc.name: setOnWillScrollTest_Callback_Void_return
 * @tc.desc: Test OnWillScrollImpl
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, setOnWillScrollTest_Callback_Void_return, testing::ext::TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct ScrollData
    {
        Ark_ScrollState state;
        Ark_ScrollSource source;
        Ark_Int32 nodeId;
    };
    static std::optional<ScrollData> otherState;

    auto callback = [](
        Ark_VMContext context,
        const Ark_Int32 resourceId,
        const Ark_Number xOffset,
        const Ark_Number yOffset,
        Ark_ScrollState scrollState,
        Ark_ScrollSource scrollSource,
        const Callback_OffsetResult_Void continuation) {
        otherState = {scrollState, scrollSource, resourceId};
    };

    auto id = Converter::ArkValue<Ark_Int32>(123);

    auto apiCall = Converter::ArkValue<Opt_ScrollOnWillScrollCallback>(
        Converter::ArkValue<ScrollOnWillScrollCallback>(nullptr, callback, id));
    ASSERT_FALSE(eventHub->GetOnWillScroll());

    ASSERT_NE(modifier_->setOnWillScroll, nullptr);
    modifier_->setOnWillScroll(node_, &apiCall);

    ASSERT_TRUE(eventHub->GetOnWillScroll());
    Dimension offset(1102);
    auto returnValue = eventHub->GetOnWillScroll()(offset, ScrollState::FLING, ScrollSource::SCROLL_BAR);
    EXPECT_EQ(returnValue.offset.Value(), offset.Value());
    ASSERT_TRUE(otherState.has_value());
    EXPECT_EQ(Ark_ScrollState::ARK_SCROLL_STATE_FLING, otherState->state);
    EXPECT_EQ(Ark_ScrollSource::ARK_SCROLL_SOURCE_SCROLL_BAR, otherState->source);
    EXPECT_EQ(id, otherState->nodeId);
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
    modifier_->setOnReachStart(node_, &arkCallback);

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
HWTEST_F(ScrollableCommonMethodModifierTest, setOnReachEndTest, TestSize.Level1)
{
    const int32_t contextId = 123;

    static std::optional<int32_t> checkData;
    auto checkCallback = [](const Ark_Int32 resourceId) { checkData = resourceId; };
    ASSERT_FALSE(checkData.has_value());

    // setup the callback object via C-API
    auto arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnReachEnd(node_, &arkCallback);

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
    modifier_->setOnScrollStart(node_, &arkCallback);

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
    modifier_->setOnScrollStop(node_, &arkCallback);

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


/*
 * @tc.name: OnDidScroll_SetCallback
 * @tc.desc: Test OnDidScrollImpl
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, OnDidScroll_SetCallback, testing::ext::TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct resultData {
        Ark_Int32 resourceId;
        Ark_Number x;
        Ark_Number y;
        Ark_ScrollState state;
    };
    static std::optional<resultData> result;

    auto callback = [](
        const Ark_Int32 resourceId, const Ark_Number xOffset, const Ark_Number yOffset, Ark_ScrollState scrollState) {
            result = {resourceId, xOffset, yOffset, scrollState};
    };

    auto id = Converter::ArkValue<Ark_Int32>(123);

    auto apiCall = Converter::ArkValue<ScrollOnScrollCallback>(callback, id);
    ASSERT_FALSE(eventHub->GetOnDidScroll());

    ASSERT_NE(modifier_->setOnDidScroll, nullptr);
    modifier_->setOnDidScroll(node_, &apiCall);

    ASSERT_TRUE(eventHub->GetOnDidScroll());
    int offsetIn = 333;
    Dimension offset(offsetIn);
    eventHub->GetOnDidScroll()(offset, ScrollState::SCROLL);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(Ark_ScrollState::ARK_SCROLL_STATE_SCROLL, result.value().state);
    auto resultValX = Converter::Convert<int>(result.value().x);
    auto resultValY = Converter::Convert<int>(result.value().y);
    EXPECT_EQ(offsetIn, resultValX);
    EXPECT_EQ(offsetIn, resultValY);
    EXPECT_EQ(id, result.value().resourceId);
}

/**
 * @tc.name: OnDidScroll_SetNullCallback
 * @tc.desc: Test OnDidScrolleImpl
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableCommonMethodModifierTest, OnDidScroll_SetNullCallback, testing::ext::TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::ScrollableEventHub>();
    ASSERT_NE(eventHub, nullptr);

    ASSERT_NE(modifier_->setOnDidScroll, nullptr);
    modifier_->setOnDidScroll(node_, nullptr);
    ASSERT_FALSE(eventHub->GetOnDidScroll());
}

} // namespace OHOS::Ace::NG

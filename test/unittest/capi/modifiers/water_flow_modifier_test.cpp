/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include "core/components_ng/pattern/waterflow/water_flow_event_hub.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
using namespace Converter;

const auto CASHED_COUNT_ATTRIBUTE_NAME = "cachedCount";
const auto CASHED_COUNT_ATTRIBUTE_DEFAULT_VALUE = "1";

static const std::vector<std::tuple<Opt_Int32, std::string>> CASHED_COUNT_TEST_PLAN = {
    { ArkValue<Opt_Int32>(10), "10"},
    { ArkValue<Opt_Int32>(-10), CASHED_COUNT_ATTRIBUTE_DEFAULT_VALUE},
    { ArkValue<Opt_Int32>(832), "832"},
    { ArkValue<Opt_Int32>(), CASHED_COUNT_ATTRIBUTE_DEFAULT_VALUE}
};

namespace Converter {
inline void AssignArkValue(Ark_OnScrollFrameBeginHandlerResult& dst, const ScrollFrameResult& src,
    ConvContext *ctx)
{
    dst.offsetRemain = Converter::ArkValue<Ark_Float64>(src.offset);
}
} // Converter

namespace {
    const float TEST_OFFSET = 10.0f;
    constexpr double SNAP_DENSITY = 2.0;
    constexpr double SNAP_VELOCITY_PX = 600.0;
    constexpr double SNAP_DECAY_PX = 300.0;
    constexpr double SNAP_APPROACH_VP = 125.0;
    constexpr double SNAP_TARGET_VP = 250.0;
}

class WaterFlowModifierTest : public ModifierTestBase<GENERATED_ArkUIWaterFlowModifier,
      &GENERATED_ArkUINodeModifiers::getWaterFlowModifier, GENERATED_ARKUI_WATER_FLOW> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
    }
};

HWTEST_F(WaterFlowModifierTest, DISABLED_setOnScrollIndexTestCachedCountValidValues, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<WaterFlowEventHub>();
    ASSERT_NE(eventHub, nullptr);

    // Define a structure to check events
    struct CheckEvent {
        int32_t nodeId;
        int32_t firstIndex;
        int32_t lastIndex;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;

    // Create an event handler for `onScrollIndex`
    auto onScrollIndex = [](Ark_Int32 nodeId, const Ark_Int32 first, const Ark_Int32 last) {
        checkEvent = {
            .nodeId = nodeId,
            .firstIndex = Converter::Convert<int32_t>(first),
            .lastIndex = Converter::Convert<int32_t>(last),
        };
    };

    // Set the callback for scroll index change
    auto optCallback = Converter::ArkCallback<Opt_Callback_I32_I32_Void>(onScrollIndex, frameNode->GetId());
    modifier_->setOnScrollIndex(node_, &optCallback);

    // Check that the event has not been triggered yet
    EXPECT_FALSE(checkEvent.has_value());

    // Trigger the scroll index change event with valid values
    auto onScrollIndx = eventHub->GetOnScrollIndex();
    onScrollIndx(10, 20);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->firstIndex, 10);
    EXPECT_EQ(checkEvent->lastIndex, 20);

    // Reset the state for the next test
    checkEvent.reset();

    // Trigger another scroll index change event with different values
    onScrollIndx(15, 25);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->firstIndex, 15);
    EXPECT_EQ(checkEvent->lastIndex, 25);

    // Reset the state for the next test
    checkEvent.reset();

    // Trigger event with zero values to test edge cases
    onScrollIndx = eventHub->GetOnScrollIndex();
    onScrollIndx(0, 0);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->firstIndex, 0);
    EXPECT_EQ(checkEvent->lastIndex, 0);
}

/*
 * @tc.name: setOnScrollFrameBeginTest
 * @tc.desc: test function of setOnScrollFrameBegin
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, setOnScrollFrameBeginTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<WaterFlowEventHub>();
    ASSERT_NE(eventHub, nullptr);
    ASSERT_NE(modifier_->setOnScrollFrameBegin, nullptr);
    static const Ark_Int32 expectedResId = 123;
    auto onScrollFrameBegin = [](Ark_VMContext context, const Ark_Int32 resourceId, const Ark_Float64 offset,
        Ark_ScrollState state, const Callback_OnScrollFrameBeginHandlerResult_Void cbReturn) {
        EXPECT_EQ(resourceId, expectedResId);
        EXPECT_EQ(Converter::Convert<float>(offset), TEST_OFFSET);
        ScrollFrameResult result;
        result.offset = Converter::Convert<Dimension>(offset);
        CallbackHelper(cbReturn).InvokeSync(Converter::ArkValue<Ark_OnScrollFrameBeginHandlerResult>(result));
    };
    auto arkFunc = Converter::ArkCallback<Opt_OnScrollFrameBeginCallback>(onScrollFrameBegin, expectedResId);
    modifier_->setOnScrollFrameBegin(node_, &arkFunc);

    Dimension dimension(TEST_OFFSET);
    ScrollState state = ScrollState::SCROLL;
    ScrollFrameResult result = eventHub->GetOnScrollFrameBegin()(dimension, state);
    EXPECT_EQ(result.offset.ConvertToPx(), dimension.ConvertToPx());
}

/*
 * @tc.name: setCachedCount0Test
 * @tc.desc: test function of SetCachedCount
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, setCachedCount0Test, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(modifier_->setCachedCount0, nullptr);
    auto checkVal = GetAttrValue<std::string>(node_, CASHED_COUNT_ATTRIBUTE_NAME);
    EXPECT_THAT(checkVal, Eq(CASHED_COUNT_ATTRIBUTE_DEFAULT_VALUE));
    for (const auto& [value, expectVal] : CASHED_COUNT_TEST_PLAN) {
        modifier_->setCachedCount0(node_, &value);
        checkVal = GetAttrValue<std::string>(node_, CASHED_COUNT_ATTRIBUTE_NAME);
        EXPECT_THAT(checkVal, Eq(expectVal));
    }
}

/**
 * @tc.name: ScrollSnapStrategyAlignAndReset
 * @tc.desc: The static modifier maps alignments and clears NONE, invalid, omitted and null strategies.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, ScrollSnapStrategyAlignAndReset, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<WaterFlowLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_NE(modifier_->setScrollSnapStrategy, nullptr);
    for (auto align : { ARK_SCROLL_SNAP_ALIGN_START, ARK_SCROLL_SNAP_ALIGN_CENTER, ARK_SCROLL_SNAP_ALIGN_END }) {
        auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
            Ark_ScrollSnapAlign>(align);
        modifier_->setScrollSnapStrategy(node_, &value);
        ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
        EXPECT_EQ(property->GetScrollSnapStrategy()->align, static_cast<ScrollSnapAlign>(align));
        EXPECT_FALSE(property->GetScrollSnapStrategy()->hasProvider);
    }
    for (auto align : { ARK_SCROLL_SNAP_ALIGN_NONE, static_cast<Ark_ScrollSnapAlign>(-1) }) {
        auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
            Ark_ScrollSnapAlign>(align);
        modifier_->setScrollSnapStrategy(node_, &value);
        EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
    }
    auto start = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapAlign>(ARK_SCROLL_SNAP_ALIGN_START);
    auto reset = Converter::ArkValue<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy>();
    modifier_->setScrollSnapStrategy(node_, &start);
    modifier_->setScrollSnapStrategy(node_, &reset);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
    modifier_->setScrollSnapStrategy(node_, &start);
    modifier_->setScrollSnapStrategy(node_, nullptr);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
    modifier_->setScrollSnapStrategy(nullptr, &start);
}

/**
 * @tc.name: ScrollSnapStrategyProviderConvertsUnits
 * @tc.desc: Both callbacks are synchronous and convert px inputs and vp results at the bridge.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, ScrollSnapStrategyProviderConvertsUnits, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<WaterFlowLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64 velocity, Ark_Float64 decay,
                        synthetic_Callback_F64_Void continuation) {
        EXPECT_DOUBLE_EQ(velocity, -SNAP_VELOCITY_PX / SNAP_DENSITY);
        EXPECT_DOUBLE_EQ(decay, SNAP_DECAY_PX / SNAP_DENSITY);
        CallbackHelper(continuation).InvokeSync(SNAP_APPROACH_VP);
    };
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64 velocity, synthetic_Callback_F64_Void continuation) {
        EXPECT_DOUBLE_EQ(velocity, -SNAP_VELOCITY_PX / SNAP_DENSITY);
        CallbackHelper(continuation).InvokeSync(SNAP_TARGET_VP);
    };
    Ark_ScrollSnapOffsetProviderProxy provider {
        .calculateApproachOffset = Converter::ArkValue<ScrollSnapApproachCallback>(nullptr, approach),
        .calculateSnapOffset = Converter::ArkValue<ScrollSnapTargetCallback>(nullptr, target),
    };
    auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
    const auto strategy = property->GetScrollSnapStrategy().value();
    ASSERT_TRUE(strategy.hasProvider);
    ASSERT_TRUE(strategy.calculateApproachOffset);
    ASSERT_TRUE(strategy.calculateSnapOffset);
    auto pipeline = MockPipelineContext::GetCurrent();
    const double scale = pipeline->GetDipScale();
    pipeline->SetDipScale(SNAP_DENSITY);
    EXPECT_DOUBLE_EQ(strategy.calculateApproachOffset(-SNAP_VELOCITY_PX, SNAP_DECAY_PX),
        SNAP_APPROACH_VP * SNAP_DENSITY);
    EXPECT_DOUBLE_EQ(strategy.calculateSnapOffset(-SNAP_VELOCITY_PX), SNAP_TARGET_VP * SNAP_DENSITY);
    pipeline->SetDipScale(scale);
}

/**
 * @tc.name: ScrollSnapStrategyProviderMissingResultUsesFallback
 * @tc.desc: No continuation result yields NaN; providers missing either required callback clear the strategy.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, ScrollSnapStrategyProviderMissingResultUsesFallback, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<WaterFlowLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64, Ark_Float64, synthetic_Callback_F64_Void) {};
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64, synthetic_Callback_F64_Void) {};
    Ark_ScrollSnapOffsetProviderProxy provider {
        .calculateApproachOffset = Converter::ArkValue<ScrollSnapApproachCallback>(nullptr, approach),
        .calculateSnapOffset = Converter::ArkValue<ScrollSnapTargetCallback>(nullptr, target),
    };
    auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
    const auto strategy = property->GetScrollSnapStrategy().value();
    EXPECT_TRUE(std::isnan(strategy.calculateApproachOffset(SNAP_VELOCITY_PX, SNAP_DECAY_PX)));
    EXPECT_TRUE(std::isnan(strategy.calculateSnapOffset(SNAP_VELOCITY_PX)));
    provider.calculateApproachOffset.callSync = nullptr;
    value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
    provider.calculateApproachOffset = Converter::ArkValue<ScrollSnapApproachCallback>(nullptr, approach);
    provider.calculateSnapOffset.callSync = nullptr;
    value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
}

/**
 * @tc.name: ScrollSnapStrategyProviderDoesNotRetainNode
 * @tc.desc: Saved callbacks must not retain the WaterFlow or invoke providers after disposal.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, ScrollSnapStrategyProviderDoesNotRetainNode, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto weak = AceType::WeakClaim(frameNode);
    auto property = frameNode->GetLayoutProperty<WaterFlowLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64, Ark_Float64, synthetic_Callback_F64_Void) {
        ADD_FAILURE() << "disposed WaterFlow invoked approach provider";
    };
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64, synthetic_Callback_F64_Void) {
        ADD_FAILURE() << "disposed WaterFlow invoked snap provider";
    };
    Ark_ScrollSnapOffsetProviderProxy provider {
        .calculateApproachOffset = Converter::ArkValue<ScrollSnapApproachCallback>(nullptr, approach),
        .calculateSnapOffset = Converter::ArkValue<ScrollSnapTargetCallback>(nullptr, target),
    };
    auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
    const auto strategy = property->GetScrollSnapStrategy().value();
    DisposeNode(node_);
    EXPECT_EQ(weak.Upgrade(), nullptr);
    EXPECT_TRUE(std::isnan(strategy.calculateApproachOffset(SNAP_VELOCITY_PX, SNAP_DECAY_PX)));
    EXPECT_TRUE(std::isnan(strategy.calculateSnapOffset(SNAP_VELOCITY_PX)));
}

/**
 * @tc.name: ScrollSnapStrategyProviderTargetRange
 * @tc.desc: Zero stays valid, non-finite results use fallback, and finite overflow remains clampable.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowModifierTest, ScrollSnapStrategyProviderTargetRange, TestSize.Level1)
{
    static double targetOffset = 0.0;
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<WaterFlowLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64, Ark_Float64 decay,
                        synthetic_Callback_F64_Void continuation) {
        CallbackHelper(continuation).InvokeSync(decay);
    };
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64, synthetic_Callback_F64_Void continuation) {
        CallbackHelper(continuation).InvokeSync(targetOffset);
    };
    Ark_ScrollSnapOffsetProviderProxy provider {
        .calculateApproachOffset = Converter::ArkValue<ScrollSnapApproachCallback>(nullptr, approach),
        .calculateSnapOffset = Converter::ArkValue<ScrollSnapTargetCallback>(nullptr, target),
    };
    auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
    const auto strategy = property->GetScrollSnapStrategy().value();
    targetOffset = 0.0;
    EXPECT_DOUBLE_EQ(strategy.calculateSnapOffset(SNAP_VELOCITY_PX), 0.0);
    for (auto invalid : { std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN() }) {
        targetOffset = invalid;
        EXPECT_TRUE(std::isnan(strategy.calculateSnapOffset(SNAP_VELOCITY_PX)));
    }
    auto pipeline = MockPipelineContext::GetCurrent();
    const double scale = pipeline->GetDipScale();
    pipeline->SetDipScale(SNAP_DENSITY);
    targetOffset = std::numeric_limits<double>::max();
    EXPECT_DOUBLE_EQ(strategy.calculateSnapOffset(SNAP_VELOCITY_PX), targetOffset);
    targetOffset = -std::numeric_limits<double>::max();
    EXPECT_DOUBLE_EQ(strategy.calculateSnapOffset(SNAP_VELOCITY_PX), targetOffset);
    pipeline->SetDipScale(scale);
}

} // namespace OHOS::Ace::NG

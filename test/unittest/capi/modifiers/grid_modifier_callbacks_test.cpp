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
#include <climits>
#include <cmath>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/grid/grid_event_hub.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"

#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace Converter {
inline void AssignArkValue(Ark_OnScrollFrameBeginHandlerResult& dst, const ScrollFrameResult& src,
    ConvContext *ctx)
{
    dst.offsetRemain = Converter::ArkValue<Ark_Float64>(src.offset);
}
} // Converter

class GridModifierCallbacksTest : public ModifierTestBase<GENERATED_ArkUIGridModifier,
    &GENERATED_ArkUINodeModifiers::getGridModifier, GENERATED_ARKUI_GRID> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
    }
};

namespace {
    const float TEST_OFFSET = 10.0f;
}

/*
 * @tc.name: setOnScrollBarUpdateTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnScrollBarUpdateTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    ASSERT_NE(eventHub, nullptr);

    auto callSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId, const Ark_Int32 index,
        const Ark_Float64 offset, const Callback_ComputedBarAttribute_Void continuation) {
        Ark_ComputedBarAttribute arkResult {
            .totalOffset = offset,
            .totalLength = index
        };
        CallbackHelper(continuation).InvokeSync(arkResult);
    };
    auto func = Converter::ArkValue<Callback_I32_F64_ComputedBarAttribute>(nullptr, callSyncFunc);
    auto optFunc = Converter::ArkValue<Opt_Callback_I32_F64_ComputedBarAttribute>(func);
    modifier_->setOnScrollBarUpdate(node_, &optFunc);

    auto result = eventHub->FireOnScrollBarUpdate(1000, Dimension(2, DimensionUnit::VP));
    EXPECT_EQ(result.first, 2);
    EXPECT_EQ(result.second, 1000);
}

/*
 * @tc.name: setOnScrollBarUpdateTestInvalid
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, DISABLED_setOnScrollBarUpdateTestInvalid, TestSize.Level1)
{
    // test is disabled because onScrollBarUpdate callback should return value
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent { int32_t nodeId; int32_t index; Dimension offset; };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    Callback_I32_F64_ComputedBarAttribute onScrollBarUpdate = {
        .resource = {.resourceId = frameNode->GetId()},
        .call = [](const Ark_Int32 resourceId,
            const Ark_Int32 index, const Ark_Float64 offset, const Callback_ComputedBarAttribute_Void continuation)
            {
                checkEvent = {
                    .nodeId = resourceId,
                    .index = Converter::Convert<int32_t>(index),
                    .offset = Converter::Convert<Dimension>(offset)
                };
        }
    };

    auto optOnScrollBarUpdate = Converter::ArkValue<Opt_Callback_I32_F64_ComputedBarAttribute>(onScrollBarUpdate);
    modifier_->setOnScrollBarUpdate(node_, &optOnScrollBarUpdate);

    // index: 11, offset: 12 invalid
    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireOnScrollBarUpdate(11, Dimension(12, DimensionUnit::INVALID));
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->index, 11);
    EXPECT_EQ(checkEvent->offset.Value(), 0);
    EXPECT_EQ(checkEvent->offset.Unit(), DimensionUnit::VP);

    // index: 12, offset: 14%
    eventHub->FireOnScrollBarUpdate(13, Dimension(14, DimensionUnit::PERCENT));
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->index, 13);
    EXPECT_EQ(checkEvent->offset.Value(), 0);
    EXPECT_EQ(checkEvent->offset.Unit(), DimensionUnit::VP);

    // index: 15, offset: 16 auto
    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireOnScrollBarUpdate(15, Dimension(16, DimensionUnit::AUTO));
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->index, 15);
    EXPECT_EQ(checkEvent->offset.Value(), 0);
    EXPECT_EQ(checkEvent->offset.Unit(), DimensionUnit::VP);

    // index: 17, offset: 18 calc
    eventHub->FireOnScrollBarUpdate(17, Dimension(18, DimensionUnit::CALC));
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->index, 17);
    EXPECT_EQ(checkEvent->offset.Value(), 0);
    EXPECT_EQ(checkEvent->offset.Unit(), DimensionUnit::VP);
}

/*
 * @tc.name: setOnScrollIndexTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnScrollIndexTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();

    struct CheckEvent {
        int32_t nodeId;
        int32_t first;
        int32_t last;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId, const Ark_Int32 first, const Ark_Int32 last) {
        isCalled = true;
        checkEvent = {
            .nodeId = resourceId,
            .first = Converter::Convert<int32_t>(first),
            .last = Converter::Convert<int32_t>(last)
        };
    };

    auto arkCallback =
        Converter::ArkValue<Callback_I32_I32_Void>(checkCallback, contextId);

    auto optCallback = Converter::ArkValue<Opt_Callback_I32_I32_Void>(arkCallback);
    modifier_->setOnScrollIndex(node_, &optCallback);

    auto onScrollIndex = eventHub->GetOnScrollIndex();
    EXPECT_NE(onScrollIndex, nullptr);
    EXPECT_EQ(checkEvent.has_value(), false);

    // first: 0, last: 10
    isCalled = false;
    onScrollIndex(0, 10);
    ASSERT_TRUE(isCalled);
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->first, 0);
    EXPECT_EQ(checkEvent->last, 10);

    // first: 5, last: 25
    isCalled = false;
    onScrollIndex(5, 25);
    ASSERT_TRUE(isCalled);
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->first, 5);
    EXPECT_EQ(checkEvent->last, 25);
}

/*
 * @tc.name: setOnItemDragStartTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, DISABLED_setOnItemDragStartTest, TestSize.Level1)
{
    using namespace Converter;
    static const int32_t expectedX = 357;
    static const int32_t expectedY = 468;
    static const int32_t expectedIdx = 7;
    static const int32_t expectedResourceId = 123;

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    static const auto expectedParentNode = frameNode;
    static auto expectedCustomNode = CreateNode();
    ASSERT_NE(expectedCustomNode, nullptr);

    // set callback to model
    auto onItemDragStartSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId,
        const Ark_ItemDragInfo event, const Ark_Int32 itemIndex,
        const Callback_Opt_CustomBuilder_Void continuation
    ) {
        // check input values
        EXPECT_EQ(resourceId, expectedResourceId);
        auto dragInfo = Convert<ItemDragInfo>(event);
        EXPECT_EQ(dragInfo.GetX(), expectedX);
        EXPECT_EQ(dragInfo.GetY(), expectedY);
        auto index = Convert<int32_t>(itemIndex);
        EXPECT_EQ(index, expectedIdx);

        // construct the result CustomBuilder
        auto builderSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId,
            const Ark_NativePointer parentNode, const Callback_Pointer_Void continuation) {
            EXPECT_EQ(reinterpret_cast<FrameNode*>(parentNode), expectedParentNode);
            CallbackHelper(continuation).InvokeSync(expectedCustomNode);
        };
        auto builder = ArkValue<CustomNodeBuilder>(nullptr, builderSyncFunc);
        auto optBuilder = ArkValue<Opt_CustomNodeBuilder>(builder);

        // return result
        CallbackHelper(continuation).InvokeSync(optBuilder);
    };
    auto arkCallback =
        ArkValue<OnItemDragStartCallback>(nullptr, onItemDragStartSyncFunc, expectedResourceId);
    auto optCallback = Converter::ArkValue<Opt_OnItemDragStartCallback>(arkCallback);
    modifier_->setOnItemDragStart(node_, &optCallback);

    // imitate the test case
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    ASSERT_NE(eventHub, nullptr);
    ItemDragInfo dragInfo;
    dragInfo.SetX(expectedX);
    dragInfo.SetY(expectedY);
    auto resultNode = eventHub->FireOnItemDragStart(dragInfo, expectedIdx);

    // check result
    EXPECT_EQ(resultNode, reinterpret_cast<UINode *>(expectedCustomNode));

    DisposeNode(expectedCustomNode);
}

/*
 * @tc.name: setOnItemDragStartTestInvalid
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnItemDragStartTestInvalid, TestSize.Level1)
{
    using namespace Converter;
    static const int32_t expectedX = 357;
    static const int32_t expectedY = 468;
    static const int32_t expectedIdx = 7;
    static const int32_t expectedResourceId = 123;

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    static const auto expectedParentNode = frameNode;
    static auto expectedCustomNode = nullptr;

    // set callback to model
    auto onItemDragStartSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId,
        const Ark_ItemDragInfo event, const Ark_Int32 itemIndex,
        const Callback_Opt_CustomBuilder_Void continuation
    ) {
        // check input values
        EXPECT_EQ(resourceId, expectedResourceId);
        auto dragInfo = Convert<ItemDragInfo>(event);
        EXPECT_EQ(dragInfo.GetX(), expectedX);
        EXPECT_EQ(dragInfo.GetY(), expectedY);
        auto index = Convert<int32_t>(itemIndex);
        EXPECT_EQ(index, expectedIdx);

        // construct the result CustomBuilder
        auto builderSyncFunc = [](Ark_VMContext context, const Ark_Int32 resourceId,
            const Ark_NativePointer parentNode, const Callback_Pointer_Void continuation) {
            EXPECT_EQ(reinterpret_cast<FrameNode*>(parentNode), expectedParentNode);
            CallbackHelper(continuation).Invoke(expectedCustomNode);
        };
        auto builder = ArkValue<CustomNodeBuilder>(nullptr, builderSyncFunc);
        auto optBuilder = ArkValue<Opt_CustomNodeBuilder>(builder);

        // return result
        CallbackHelper(continuation).Invoke(optBuilder);
    };
    auto arkCallback =
        ArkValue<OnItemDragStartCallback>(nullptr, onItemDragStartSyncFunc, expectedResourceId);
    auto optCallback = Converter::ArkValue<Opt_OnItemDragStartCallback>(arkCallback);
    modifier_->setOnItemDragStart(node_, &optCallback);

    // imitate the test case
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    ASSERT_NE(eventHub, nullptr);
    ItemDragInfo dragInfo;
    dragInfo.SetX(expectedX);
    dragInfo.SetY(expectedY);
    auto resultNode = eventHub->FireOnItemDragStart(dragInfo, expectedIdx);

    // check result
    EXPECT_EQ(resultNode, nullptr);
}

/*
 * @tc.name: setOnItemDragEnterTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnItemDragEnterTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    auto dragInfo = ItemDragInfo();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        ItemDragInfo dragInfo;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;

    Callback_ItemDragInfo_Void onItemDragEnter = {
        .resource = {.resourceId = frameNode->GetId()},
        .call = [](Ark_Int32 nodeId, const Ark_ItemDragInfo event) {
            checkEvent = {
                .nodeId = nodeId,
                .dragInfo = Converter::Convert<ItemDragInfo>(event),
            };
        }
    };
    auto optOnItemDragEnter = Converter::ArkValue<Opt_Callback_ItemDragInfo_Void>(onItemDragEnter);
    modifier_->setOnItemDragEnter(node_, &optOnItemDragEnter);

    dragInfo.SetX(234);
    dragInfo.SetY(567);

    EXPECT_FALSE(checkEvent.has_value());
    eventHub->FireOnItemDragEnter(dragInfo);
    EXPECT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->dragInfo.GetX(), 234);
    EXPECT_EQ(checkEvent->dragInfo.GetY(), 567);
}

/*
 * @tc.name: setOnItemDragMoveTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnItemDragMoveTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    auto dragInfo = ItemDragInfo();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        ItemDragInfo dragInfo;
        int32_t itemIndex;
        int32_t insertIndex;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    Callback_ItemDragInfo_I32_I32_Void onItemDragMove = {
        .resource = {.resourceId = frameNode->GetId()},
        .call = [](Ark_Int32 nodeId, const Ark_ItemDragInfo event, const Ark_Int32 itemIndex,
            const Ark_Int32 insertIndex)
            {
                checkEvent = {
                    .nodeId = nodeId,
                    .dragInfo = Converter::Convert<ItemDragInfo>(event),
                    .itemIndex = Converter::Convert<int32_t>(itemIndex),
                    .insertIndex = Converter::Convert<int32_t>(insertIndex)
                };
            }
    };

    auto optOnItemDragMove = Converter::ArkValue<Opt_Callback_ItemDragInfo_I32_I32_Void>(onItemDragMove);
    modifier_->setOnItemDragMove(node_, &optOnItemDragMove);

    dragInfo.SetX(987);
    dragInfo.SetY(654);

    EXPECT_FALSE(checkEvent.has_value());
    eventHub->FireOnItemDragMove(dragInfo, 4, 35);
    EXPECT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->dragInfo.GetX(), 987);
    EXPECT_EQ(checkEvent->dragInfo.GetY(), 654);
    EXPECT_EQ(checkEvent->itemIndex, 4);
    EXPECT_EQ(checkEvent->insertIndex, 35);
}

/*
 * @tc.name: setOnItemDragLeaveTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnItemDragLeaveTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    auto dragInfo = ItemDragInfo();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        ItemDragInfo dragInfo;
        int32_t itemIndex;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    Callback_ItemDragInfo_I32_Void onItemDragLeave = {
        .resource = {.resourceId = frameNode->GetId()},
        .call = [](Ark_Int32 nodeId,
            const Ark_ItemDragInfo event, const Ark_Int32 itemIndex)
            {
                checkEvent = {
                    .nodeId = nodeId,
                    .dragInfo = Converter::Convert<ItemDragInfo>(event),
                    .itemIndex = Converter::Convert<int32_t>(itemIndex)
            };
        }
    };

    auto optOnItemDragLeave = Converter::ArkValue<Opt_Callback_ItemDragInfo_I32_Void>(onItemDragLeave);
    modifier_->setOnItemDragLeave(node_, &optOnItemDragLeave);

    dragInfo.SetX(135);
    dragInfo.SetY(246);

    EXPECT_FALSE(checkEvent.has_value());
    eventHub->FireOnItemDragLeave(dragInfo, 6);
    EXPECT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->dragInfo.GetX(), 135);
    EXPECT_EQ(checkEvent->dragInfo.GetY(), 246);
    EXPECT_EQ(checkEvent->itemIndex, 6);
}

/*
 * @tc.name: setOnItemDropTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnItemDropTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    auto dragInfo = ItemDragInfo();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        ItemDragInfo dragInfo;
        int32_t itemIndex;
        int32_t insertIndex;
        bool isSuccess;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    Callback_ItemDragInfo_I32_I32_Boolean_Void onItemDrop = {
        .resource = {.resourceId = frameNode->GetId()},
        .call = [](Ark_Int32 nodeId, const Ark_ItemDragInfo event,
        const Ark_Int32 itemIndex, const Ark_Int32 insertIndex, const Ark_Boolean isSuccess)
        {
            checkEvent = {
                .nodeId = nodeId,
                .dragInfo = Converter::Convert<ItemDragInfo>(event),
                .itemIndex = Converter::Convert<int32_t>(itemIndex),
                .insertIndex = Converter::Convert<int32_t>(insertIndex),
                .isSuccess = Converter::Convert<bool>(isSuccess),
            };
        }
    };

    auto optOnItemDrop = Converter::ArkValue<Opt_Callback_ItemDragInfo_I32_I32_Boolean_Void>(onItemDrop);
    modifier_->setOnItemDrop(node_, &optOnItemDrop);

    dragInfo.SetX(975);
    dragInfo.SetY(864);

    EXPECT_FALSE(checkEvent.has_value());
    eventHub->FireOnItemDrop(dragInfo, 81, 99, true);
    EXPECT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->dragInfo.GetX(), 975);
    EXPECT_EQ(checkEvent->dragInfo.GetY(), 864);
    EXPECT_EQ(checkEvent->itemIndex, 81);
    EXPECT_EQ(checkEvent->insertIndex, 99);
    EXPECT_TRUE(checkEvent->isSuccess);
}

/*
 * @tc.name: setOnScrollFrameBeginTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, setOnScrollFrameBeginTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<GridEventHub>();
    ASSERT_NE(eventHub, nullptr);

    ASSERT_NE(modifier_->setOnScrollFrameBegin, nullptr);
    modifier_->setOnScrollFrameBegin(node_, nullptr);

    static const Ark_Int32 expectedResId = 123;
    auto onScrollFrameBegin = [](Ark_VMContext context, const Ark_Int32 resourceId,
        const Ark_Float64 offset, Ark_ScrollState state,
        const Callback_OnScrollFrameBeginHandlerResult_Void cbReturn) {
        EXPECT_EQ(resourceId, expectedResId);
        EXPECT_EQ(Converter::Convert<float>(offset), TEST_OFFSET);
        ScrollFrameResult result;
        result.offset = Converter::Convert<Dimension>(offset);
        CallbackHelper(cbReturn).InvokeSync(Converter::ArkValue<Ark_OnScrollFrameBeginHandlerResult>(result));
    };

    auto arkFunc = Converter::ArkValue<OnScrollFrameBeginCallback>(
        nullptr, onScrollFrameBegin, expectedResId);
    auto arkFuncOpt = Converter::ArkValue<Opt_OnScrollFrameBeginCallback>(arkFunc);
    modifier_->setOnScrollFrameBegin(node_, &arkFuncOpt);

    Dimension dimension(TEST_OFFSET);
    ScrollState state = ScrollState::SCROLL;
    ScrollFrameResult result = eventHub->GetOnScrollFrameBegin()(dimension, state);
    EXPECT_EQ(result.offset.ConvertToPx(), dimension.ConvertToPx());
}
/**
 * @tc.name: ScrollSnapStrategyAlignAndReset
 * @tc.desc: The static modifier maps all built-in alignments and clears an omitted strategy.
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, ScrollSnapStrategyAlignAndReset, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<GridLayoutProperty>();
    ASSERT_NE(property, nullptr);
    for (auto align : { ARK_SCROLL_SNAP_ALIGN_NONE, ARK_SCROLL_SNAP_ALIGN_START,
             ARK_SCROLL_SNAP_ALIGN_CENTER, ARK_SCROLL_SNAP_ALIGN_END }) {
        auto value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
            Ark_ScrollSnapAlign>(align);
        modifier_->setScrollSnapStrategy(node_, &value);
        ASSERT_TRUE(property->GetScrollSnapStrategy().has_value());
        EXPECT_EQ(property->GetScrollSnapStrategy()->align, static_cast<ScrollSnapAlign>(align));
        EXPECT_FALSE(property->GetScrollSnapStrategy()->hasProvider);
    }
    auto reset = Converter::ArkValue<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy>();
    modifier_->setScrollSnapStrategy(node_, &reset);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
    modifier_->setScrollSnapStrategy(nullptr, &reset);
}

/**
 * @tc.name: ScrollSnapStrategyProviderConvertsUnits
 * @tc.desc: Both provider callbacks are synchronous and convert px inputs and vp results at the bridge.
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, ScrollSnapStrategyProviderConvertsUnits, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<GridLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64 velocity, Ark_Float64 decay,
                        synthetic_Callback_F64_Void continuation) {
        EXPECT_DOUBLE_EQ(velocity, -300.0);
        EXPECT_DOUBLE_EQ(decay, 150.0);
        CallbackHelper(continuation).InvokeSync(125.0);
    };
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64 velocity, synthetic_Callback_F64_Void continuation) {
        EXPECT_DOUBLE_EQ(velocity, -300.0);
        CallbackHelper(continuation).InvokeSync(250.0);
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
    pipeline->SetDipScale(2.0);
    EXPECT_DOUBLE_EQ(strategy.calculateApproachOffset(-600.0, 300.0), 250.0);
    EXPECT_DOUBLE_EQ(strategy.calculateSnapOffset(-600.0), 500.0);
    pipeline->SetDipScale(scale);
}

/**
 * @tc.name: ScrollSnapStrategyProviderMissingResultUsesFallback
 * @tc.desc: A callback with no continuation result must yield NaN rather than a valid zero target.
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, ScrollSnapStrategyProviderMissingResultUsesFallback, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto property = frameNode->GetLayoutProperty<GridLayoutProperty>();
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
    EXPECT_TRUE(std::isnan(strategy.calculateApproachOffset(600.0, 300.0)));
    EXPECT_TRUE(std::isnan(strategy.calculateSnapOffset(600.0)));
    provider.calculateSnapOffset.callSync = nullptr;
    value = Converter::ArkUnion<Opt_Union_ScrollSnapAlign_ScrollSnapOffsetProviderProxy,
        Ark_ScrollSnapOffsetProviderProxy>(provider);
    modifier_->setScrollSnapStrategy(node_, &value);
    EXPECT_FALSE(property->GetScrollSnapStrategy().has_value());
}

/**
 * @tc.name: ScrollSnapStrategyProviderDoesNotRetainNode
 * @tc.desc: Saved callbacks must not keep the Grid alive and must not invoke providers after disposal.
 * @tc.type: FUNC
 */
HWTEST_F(GridModifierCallbacksTest, ScrollSnapStrategyProviderDoesNotRetainNode, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto weak = AceType::WeakClaim(frameNode);
    auto property = frameNode->GetLayoutProperty<GridLayoutProperty>();
    auto approach = [](Ark_VMContext, Ark_Int32, Ark_Float64, Ark_Float64, synthetic_Callback_F64_Void) {
        ADD_FAILURE() << "disposed Grid invoked approach provider";
    };
    auto target = [](Ark_VMContext, Ark_Int32, Ark_Float64, synthetic_Callback_F64_Void) {
        ADD_FAILURE() << "disposed Grid invoked snap provider";
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
    EXPECT_TRUE(std::isnan(strategy.calculateApproachOffset(600.0, 300.0)));
    EXPECT_TRUE(std::isnan(strategy.calculateSnapOffset(600.0)));
}

} // namespace OHOS::Ace::NG

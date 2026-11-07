/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <map>
#include <vector>

#include "gtest/gtest.h"

#include "event_converter.h"
#include "node_extened.h"
#include "node_model.h"

using namespace testing::ext;
using namespace OHOS::Ace::NodeModel;

namespace {
constexpr int32_t PRIMARY_RECEIVER_ID = 1;
constexpr int32_t SECONDARY_RECEIVER_ID = 2;
constexpr int32_t MUTATED_TARGET_ID = 42;

struct CallbackState {
    std::vector<int32_t> calls;
    ArkUI_NodeEvent* received = nullptr;
};

void NodeReceiver(ArkUI_NodeEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(PRIMARY_RECEIVER_ID);
    state->received = event;
    event->targetId = MUTATED_TARGET_ID;
}

void GlobalReceiver(ArkUI_NodeEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(SECONDARY_RECEIVER_ID);
    EXPECT_EQ(event->targetId, MUTATED_TARGET_ID);
}

void RemoveBothReceivers(ArkUI_NodeEvent* event);

void RemoveOtherReceiver(ArkUI_NodeEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(SECONDARY_RECEIVER_ID);
    if (event->node->eventListeners) {
        RemoveNodeEventReceiver(event->node, RemoveBothReceivers);
        RemoveNodeEventReceiver(event->node, RemoveOtherReceiver);
    }
}

void RemoveBothReceivers(ArkUI_NodeEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(PRIMARY_RECEIVER_ID);
    if (event->node->eventListeners) {
        RemoveNodeEventReceiver(event->node, RemoveBothReceivers);
        RemoveNodeEventReceiver(event->node, RemoveOtherReceiver);
    }
}

void CustomReceiver(ArkUI_NodeCustomEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(PRIMARY_RECEIVER_ID);
    event->targetId = MUTATED_TARGET_ID;
}

void GlobalCustomReceiver(ArkUI_NodeCustomEvent* event)
{
    auto* state = static_cast<CallbackState*>(event->userData);
    state->calls.push_back(SECONDARY_RECEIVER_ID);
    EXPECT_EQ(event->targetId, MUTATED_TARGET_ID);
}
} // namespace

class CallbackDfxTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ASSERT_TRUE(InitialFullImpl());
    }

    void SetUp() override
    {
        // These dispatch tests have no PipelineContext; Text initialization requires one.
        node_ = CreateNode(ARKUI_NODE_STACK);
        ASSERT_NE(node_, nullptr);
        UnregisterOnEvent();
        UnregisterNodeCustomEventReceiver();
    }

    void TearDown() override
    {
        UnregisterOnEvent();
        UnregisterNodeCustomEventReceiver();
        if (node_) {
            DisposeNode(node_);
        }
    }

protected:
    ArkUI_NodeHandle node_ = nullptr;
};

/**
 * @tc.name: NodeReceiverOrderAndArguments
 * @tc.desc: Diagnostic scopes preserve the node/global receiver order and event mutation.
 * @tc.type: FUNC
 */
HWTEST_F(CallbackDfxTest, NodeReceiverOrderAndArguments, TestSize.Level1)
{
    CallbackState state;
    ArkUI_NodeEvent event {};
    event.node = node_;
    event.userData = &state;
    ASSERT_EQ(AddNodeEventReceiver(node_, NodeReceiver), 0);
    RegisterOnEvent(GlobalReceiver);
    HandleNodeEvent(&event);
    EXPECT_EQ(state.calls, (std::vector<int32_t> { PRIMARY_RECEIVER_ID, SECONDARY_RECEIVER_ID }));
    EXPECT_EQ(state.received, &event);
    EXPECT_EQ(event.targetId, MUTATED_TARGET_ID);
}

/**
 * @tc.name: RemovalDoesNotInvokeCallback
 * @tc.desc: Removal neither invokes the callback nor leaves it in the next dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(CallbackDfxTest, RemovalDoesNotInvokeCallback, TestSize.Level1)
{
    CallbackState state;
    ArkUI_NodeEvent event {};
    event.node = node_;
    event.userData = &state;
    ASSERT_EQ(AddNodeEventReceiver(node_, NodeReceiver), 0);
    HandleNodeEvent(&event);
    ASSERT_EQ(state.calls.size(), 1u);
    ASSERT_EQ(RemoveNodeEventReceiver(node_, NodeReceiver), 0);
    EXPECT_EQ(state.calls.size(), 1u);
    HandleNodeEvent(&event);
    EXPECT_EQ(state.calls.size(), 1u);
    EXPECT_EQ(node_->eventListeners, nullptr);
}

/**
 * @tc.name: ReceiverRemovalPreservesCurrentSnapshot
 * @tc.desc: Both copied listeners execute once even when the first removes both registrations.
 * @tc.type: FUNC
 */
HWTEST_F(CallbackDfxTest, ReceiverRemovalPreservesCurrentSnapshot, TestSize.Level1)
{
    CallbackState state;
    ArkUI_NodeEvent event {};
    event.node = node_;
    event.userData = &state;
    ASSERT_EQ(AddNodeEventReceiver(node_, RemoveBothReceivers), 0);
    ASSERT_EQ(AddNodeEventReceiver(node_, RemoveOtherReceiver), 0);
    HandleNodeEvent(&event);
    ASSERT_EQ(state.calls.size(), 2u);
    EXPECT_NE(state.calls[0], state.calls[1]);
    EXPECT_EQ(node_->eventListeners, nullptr);
    HandleNodeEvent(&event);
    EXPECT_EQ(state.calls.size(), 2u);
}

/**
 * @tc.name: CommonEventDispatchPreservesArguments
 * @tc.desc: The three common event types preserve callback arguments and selection.
 * @tc.type: FUNC
 */
HWTEST_F(CallbackDfxTest, CommonEventDispatchPreservesArguments, TestSize.Level1)
{
    CallbackState state;
    ArkUI_NodeEvent event {};
    event.node = node_;
    event.userData = &state;
    for (const auto type : { NODE_ON_CLICK_EVENT, NODE_VISIBLE_AREA_APPROXIMATE_CHANGE_EVENT,
        NODE_EVENT_ON_AREA_CHANGE }) {
        std::map<uint32_t, void (*)(ArkUI_NodeEvent*)> listeners { { type, NodeReceiver } };
        state.calls.clear();
        TriggerNodeCommonEvent(&event, type, &listeners);
        EXPECT_EQ(state.calls.size(), 1u);
        EXPECT_EQ(state.received, &event);
        EXPECT_EQ(event.targetId, MUTATED_TARGET_ID);
        listeners.clear();
        TriggerNodeCommonEvent(&event, type, &listeners);
        EXPECT_EQ(state.calls.size(), 1u);
    }
}

/**
 * @tc.name: CustomReceiverOrderAndArguments
 * @tc.desc: Node and global custom receivers receive the same event in the original order.
 * @tc.type: FUNC
 */
HWTEST_F(CallbackDfxTest, CustomReceiverOrderAndArguments, TestSize.Level1)
{
    CallbackState state;
    ArkUI_NodeCustomEvent event {};
    event.node = node_;
    event.userData = &state;
    ASSERT_EQ(AddNodeCustomEventReceiver(node_, CustomReceiver), 0);
    RegisterNodeCustomReceiver(GlobalCustomReceiver);
    HandleCustomEvent(&event);
    EXPECT_EQ(state.calls, (std::vector<int32_t> { PRIMARY_RECEIVER_ID, SECONDARY_RECEIVER_ID }));
    EXPECT_EQ(event.targetId, MUTATED_TARGET_ID);
}

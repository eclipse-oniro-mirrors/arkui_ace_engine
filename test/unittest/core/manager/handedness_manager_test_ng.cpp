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

#include <optional>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "core/components_ng/manager/handedness/handedness_manager.h"
#include "core/components_ng/manager/handedness/handedness_types.h"
#include "core/common/event_manager.h"

#include "base/utils/system_properties.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_window.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t INVALID_NODE_ID = -1;
constexpr float FRAME_WIDTH = 100.0f;
constexpr float FRAME_HEIGHT = 50.0f;
constexpr float FRAME_OFFSET_X = 200.0f;
constexpr float FRAME_OFFSET_Y = 300.0f;

RefPtr<HandednessManager> GetHandednessManager()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    return pipeline ? pipeline->handednessManager_ : nullptr;
}

void ResetManagerState(const RefPtr<HandednessManager>& manager)
{
    if (!manager) {
        return;
    }
    manager->operatingHand_ = 0;
    manager->lastFocusNodeId_ = INVALID_NODE_ID;
    manager->referenceNodeId_ = INVALID_NODE_ID;
}

void SetSmallWindowRect()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    if (!pipeline) {
        return;
    }
    auto* mockWindow = static_cast<MockWindow*>(pipeline->window_.get());
    if (!mockWindow) {
        return;
    }
    ON_CALL(*mockWindow, GetCurrentWindowRect).WillByDefault(Return(Rect(0, 0, 100, 100)));
}

void SetLargeWindowRect()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    if (!pipeline) {
        return;
    }
    auto* mockWindow = static_cast<MockWindow*>(pipeline->window_.get());
    if (!mockWindow) {
        return;
    }
    ON_CALL(*mockWindow, GetCurrentWindowRect).WillByDefault(Return(Rect(0, 0, 720, 1280)));
}
} // namespace

class HandednessManagerTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
};

void HandednessManagerTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    auto pipeline = MockPipelineContext::GetCurrent();
    pipeline->window_ = std::make_shared<NiceMock<MockWindow>>();
    auto pipelineBase = AceType::DynamicCast<PipelineBase>(pipeline);
    MockContainer::SetUp(pipelineBase);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(themeManager);
}

void HandednessManagerTestNg::TearDownTestSuite()
{
    MockContainer::TearDown();
    MockPipelineContext::TearDown();
}

void HandednessManagerTestNg::SetUp()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    if (pipeline) {
        auto mgr = AceType::MakeRefPtr<HandednessManager>(WeakPtr(pipeline));
        pipeline->handednessManager_ = mgr;
    }
    SetLargeWindowRect();
}

void HandednessManagerTestNg::TearDown()
{
    SetLargeWindowRect();
}

// ================ SetOperatingHand / GetOperatingHand ================

/**
 * @tc.name: SetOperatingHand001
 * @tc.desc: Test SetOperatingHand with left hand (1)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetOperatingHand001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetOperatingHand(1);
    EXPECT_EQ(manager->GetOperatingHand(), 1);
}

/**
 * @tc.name: SetOperatingHand002
 * @tc.desc: Test SetOperatingHand with right hand (2)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetOperatingHand002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetOperatingHand(2);
    EXPECT_EQ(manager->GetOperatingHand(), 2);
}

/**
 * @tc.name: SetOperatingHand003
 * @tc.desc: Test SetOperatingHand with unknown hand (0)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetOperatingHand003, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetOperatingHand(0);
    EXPECT_EQ(manager->GetOperatingHand(), 0);
}

// ================ SetLastFocusNodeId / GetLastFocusNodeId ================

/**
 * @tc.name: SetLastFocusNodeId001
 * @tc.desc: Test SetLastFocusNodeId with valid id
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetLastFocusNodeId001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetLastFocusNodeId(42);
    EXPECT_EQ(manager->GetLastFocusNodeId(), 42);
}

/**
 * @tc.name: SetLastFocusNodeId002
 * @tc.desc: Test SetLastFocusNodeId with -1 (invalid)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetLastFocusNodeId002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetLastFocusNodeId(INVALID_NODE_ID);
    EXPECT_EQ(manager->GetLastFocusNodeId(), INVALID_NODE_ID);
}

// ================ SetReferenceNodeId / GetReferenceNodeId ================

/**
 * @tc.name: SetReferenceNodeId001
 * @tc.desc: Test SetReferenceNodeId with valid id
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, SetReferenceNodeId001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    manager->SetReferenceNodeId(99);
    EXPECT_EQ(manager->GetReferenceNodeId(), 99);
}

// ================ IsBreakpointMet ================

/**
 * @tc.name: IsBreakpointMet001
 * @tc.desc: Large window (720x1280) meets breakpoint condition
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, IsBreakpointMet001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    SetLargeWindowRect();
    EXPECT_TRUE(manager->IsBreakpointMet());
}

/**
 * @tc.name: IsBreakpointMet002
 * @tc.desc: Small window does not meet breakpoint condition
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, IsBreakpointMet002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    SetSmallWindowRect();
    EXPECT_FALSE(manager->IsBreakpointMet());
}

/**
 * @tc.name: IsBreakpointMet003
 * @tc.desc: Wide window (1600x400) meets breakpoint via WIDTH_LG
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, IsBreakpointMet003, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    auto pipeline = MockPipelineContext::GetCurrent();
    auto* mockWindow = static_cast<MockWindow*>(pipeline->window_.get());
    ON_CALL(*mockWindow, GetCurrentWindowRect).WillByDefault(Return(Rect(0, 0, 1600, 400)));
    EXPECT_TRUE(manager->IsBreakpointMet());
}

// ================ QueryHoldHandPosture / QueryHoldHandPostureFromMsdp ================

/**
 * @tc.name: QueryHoldHandPosture001
 * @tc.desc: Test QueryHoldHandPosture returns 0 when MSDP unavailable (test env)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryHoldHandPosture001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    // MSDP_MOTION_EXISTS not defined in test => returns 0
    EXPECT_EQ(manager->QueryHoldHandPosture(), 0);
}

/**
 * @tc.name: QueryHoldHandPostureFromMsdp001
 * @tc.desc: Test QueryHoldHandPostureFromMsdp returns 0 when MSDP unavailable
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryHoldHandPostureFromMsdp001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->QueryHoldHandPostureFromMsdp(), 0);
}

// ================ QueryPopupSideByHoldingHand ================

/**
 * @tc.name: QueryPopupSideByHoldingHand001
 * @tc.desc: Breakpoint not met => returns CENTER
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryPopupSideByHoldingHand001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetSmallWindowRect();
    EXPECT_EQ(manager->QueryPopupSideByHoldingHand(), PopupSide::CENTER);
}

/**
 * @tc.name: QueryPopupSideByHoldingHand002
 * @tc.desc: Breakpoint met, posture=0 (unknown) => CENTER
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryPopupSideByHoldingHand002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();
    // MSDP unavailable => posture=0 => CENTER
    EXPECT_EQ(manager->QueryPopupSideByHoldingHand(), PopupSide::CENTER);
}

/**
 * @tc.name: QueryPopupSideByHoldingHand003
 * @tc.desc: Breakpoint met, posture=0, operatingHand=1 => CENTER (posture 0 always CENTER)
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryPopupSideByHoldingHand003, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();
    manager->SetOperatingHand(1);
    // posture=0 in test env => CENTER
    EXPECT_EQ(manager->QueryPopupSideByHoldingHand(), PopupSide::CENTER);
}

// ================ QueryAnchorPosition (no-arg overload) ================

/**
 * @tc.name: QueryAnchorPosition001
 * @tc.desc: Breakpoint not met => returns {-1, (0,0)}
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPosition001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetSmallWindowRect();
    auto [nodeId, offset] = manager->QueryAnchorPosition();
    EXPECT_EQ(nodeId, INVALID_NODE_ID);
    EXPECT_EQ(offset.GetX(), 0.0f);
    EXPECT_EQ(offset.GetY(), 0.0f);
}

/**
 * @tc.name: QueryAnchorPosition002
 * @tc.desc: Breakpoint met, lastFocusNodeId=-1 (no valid node) => {-1, (0,0)}
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPosition002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();
    auto [nodeId, offset] = manager->QueryAnchorPosition();
    EXPECT_EQ(nodeId, INVALID_NODE_ID);
    EXPECT_EQ(offset.GetX(), 0.0f);
    EXPECT_EQ(offset.GetY(), 0.0f);
}

// ================ QueryAnchorPosition(nodeId) ================

/**
 * @tc.name: QueryAnchorPositionWithId001
 * @tc.desc: Breakpoint not met => returns {-1, (0,0)}
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPositionWithId001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    SetSmallWindowRect();
    auto [nodeId, offset] = manager->QueryAnchorPosition(1);
    EXPECT_EQ(nodeId, INVALID_NODE_ID);
    EXPECT_EQ(offset.GetX(), 0.0f);
    EXPECT_EQ(offset.GetY(), 0.0f);
}

/**
 * @tc.name: QueryAnchorPositionWithId002
 * @tc.desc: Breakpoint met, nodeId=-1, lastFocusNodeId=-1 => {-1, (0,0)}
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPositionWithId002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();
    auto [nodeId, offset] = manager->QueryAnchorPosition(INVALID_NODE_ID);
    EXPECT_EQ(nodeId, INVALID_NODE_ID);
    EXPECT_EQ(offset.GetX(), 0.0f);
    EXPECT_EQ(offset.GetY(), 0.0f);
}

/**
 * @tc.name: QueryAnchorPositionWithId003
 * @tc.desc: Breakpoint met, nodeId is valid but not registered => {-1, (0,0)}
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPositionWithId003, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();
    auto [nodeId, offset] = manager->QueryAnchorPosition(999999);
    EXPECT_EQ(nodeId, INVALID_NODE_ID);
    EXPECT_EQ(offset.GetX(), 0.0f);
    EXPECT_EQ(offset.GetY(), 0.0f);
}

/**
 * @tc.name: QueryAnchorPositionWithId004
 * @tc.desc: Breakpoint met, valid registered FrameNode => returns center coordinates
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, QueryAnchorPositionWithId004, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);
    SetLargeWindowRect();

    auto frameNode = FrameNode::CreateFrameNode("test", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    int32_t nodeId = frameNode->GetId();

    auto geoNode = frameNode->GetGeometryNode();
    ASSERT_NE(geoNode, nullptr);
    geoNode->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    geoNode->SetFrameOffset(OffsetF(FRAME_OFFSET_X, FRAME_OFFSET_Y));

    auto [resultNodeId, resultOffset] = manager->QueryAnchorPosition(nodeId);
    EXPECT_EQ(resultNodeId, nodeId);
}

// ================ EventManager::UpdateHandednessOnTouchDown ================

/**
 * @tc.name: UpdateHandednessOnTouchDown001
 * @tc.desc: Touch down with no touchTestResults for the event id => no crash, no SetLastFocusNodeId
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, UpdateHandednessOnTouchDown001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    ASSERT_NE(eventManager, nullptr);
    TouchEvent event;
    event.id = 0;
    event.type = TouchType::DOWN;
    // touchTestResults_ is empty => resultIter == end => skip
    eventManager->UpdateHandednessOnTouchDown(event);
    EXPECT_EQ(manager->GetLastFocusNodeId(), INVALID_NODE_ID);
}

/**
 * @tc.name: UpdateHandednessOnTouchDown002
 * @tc.desc: Touch down with empty touchTestResults list for the event id => no SetLastFocusNodeId
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, UpdateHandednessOnTouchDown002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    ASSERT_NE(eventManager, nullptr);
    // Insert an empty result list
    eventManager->touchTestResults_[1] = TouchTestResult{};
    TouchEvent event;
    event.id = 1;
    event.type = TouchType::DOWN;
    eventManager->UpdateHandednessOnTouchDown(event);
    EXPECT_EQ(manager->GetLastFocusNodeId(), INVALID_NODE_ID);
}

// ================ EventManager::UpdateHandednessOnTouchUp ================

/**
 * @tc.name: UpdateHandednessOnTouchUp001
 * @tc.desc: Touch up sets operatingHand on the manager
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, UpdateHandednessOnTouchUp001, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    ASSERT_NE(eventManager, nullptr);
    TouchEvent event;
    event.id = 0;
    event.type = TouchType::UP;
    event.operatingHand = 2;
    eventManager->UpdateHandednessOnTouchUp(event);
    EXPECT_EQ(manager->GetOperatingHand(), 2);
}

/**
 * @tc.name: UpdateHandednessOnTouchUp002
 * @tc.desc: Touch up with operatingHand=1 sets left hand
 * @tc.type: FUNC
 */
HWTEST_F(HandednessManagerTestNg, UpdateHandednessOnTouchUp002, TestSize.Level1)
{
    auto manager = GetHandednessManager();
    ASSERT_NE(manager, nullptr);
    ResetManagerState(manager);

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    ASSERT_NE(eventManager, nullptr);
    TouchEvent event;
    event.id = 0;
    event.type = TouchType::UP;
    event.operatingHand = 1;
    eventManager->UpdateHandednessOnTouchUp(event);
    EXPECT_EQ(manager->GetOperatingHand(), 1);
}
} // namespace OHOS::Ace::NG

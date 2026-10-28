/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "scroll_test_ng.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "core/common/container.h"
#include "core/common/resource/resource_parse_utils.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

namespace OHOS::Ace::NG {
class ScrollControllerTestNg : public ScrollTestNg, public testing::WithParamInterface<bool> {};

/**
 * @tc.name: AnimateTo001
 * @tc.desc: Test AnimateTo
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, AnimateTo001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. AnimateTo the position in the scroll
     * @tc.expected: AnimateTo the position
     */
    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can not over scroll
     */
    AnimateTo(Dimension(CONTENT_MAIN_SIZE), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo002
 * @tc.desc: Test AnimateTo in Horizontal Layout
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, AnimateTo002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), HORIZONTAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. AnimateTo the position in the scroll
     * @tc.expected: AnimateTo the position
     */
    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can not over scroll
     */
    AnimateTo(Dimension(10000.0f), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-HORIZONTAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo003
 * @tc.desc: Test AnimateTo in Horizontal and RTL Layout
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, AnimateTo003, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), HORIZONTAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. AnimateTo the position in the scroll
     * @tc.expected: AnimateTo the position
     */
    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can not over scroll
     */
    AnimateTo(Dimension(10000.0f), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-HORIZONTAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollBy001
 * @tc.desc: Test ScrollBy
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, ScrollBy001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. ScrollBy the position
     * @tc.expected: ScrollBy the position
     */
    bool smooth = GetParam();
    ScrollableUtilsTestNG::ScrollBy(0, ITEM_MAIN_SIZE, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. ScrollBy the position over the scroll
     * @tc.expected: ScrollBy the bottom, can not over scroll
     */
    ScrollableUtilsTestNG::ScrollBy(0, CONTENT_MAIN_SIZE, smooth);
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step3. ScrollBy the position 0
     * @tc.expected: Not scroll
     */
    ScrollableUtilsTestNG::ScrollBy(0, 0, smooth);
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step4. ScrollBy the position to top
     * @tc.expected: ScrollBy the top
     */
    ScrollableUtilsTestNG::ScrollBy(0, -CONTENT_MAIN_SIZE, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollPage001
 * @tc.desc: Test ScrollPage
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, ScrollPage001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. ScrollPage down with animation
     * @tc.expected: Scroll down with animation
     */
    bool smooth = GetParam();
    ScrollPage(false, smooth);
    EXPECT_TRUE(TickPosition(-HEIGHT));

    /**
     * @tc.steps: step1. ScrollPage up with animation
     * @tc.expected: Scroll up with animation
     */
    pattern_->SetIsOverScroll(true);
    pattern_->SetCanStayOverScroll(true);
    ScrollPage(true, smooth);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_FALSE(pattern_->GetIsOverScroll());
    EXPECT_FALSE(pattern_->GetCanStayOverScroll());
}

/**
 * @tc.name: ScrollPage002
 * @tc.desc: Test ScrollPage
 * @tc.type: FUNC
 */
HWTEST_P(ScrollControllerTestNg, ScrollPage002, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. ScrollPage down with animation
     * @tc.expected: Scroll down with animation
     */
    auto scrollable = AceType::MakeRefPtr<Scrollable>();
    auto propertyCallback = [](float offset) {};
    scrollable->springOffsetProperty_ =
        AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    scrollable->state_ = Scrollable::AnimationState::SPRING;
    pattern_->scrollAbort_ = false;
    ASSERT_NE(pattern_->scrollableEvent_, nullptr);
    pattern_->scrollableEvent_->scrollable_ = scrollable;

    /**
     * @tc.steps: step2. Test ScrollPage
     * @tc.expected: Verify the scrollAbort_ status
     */
    pattern_->ScrollPage(false, true, AccessibilityScrollType::SCROLL_HALF);
    EXPECT_TRUE(pattern_->scrollAbort_);
}

INSTANTIATE_TEST_SUITE_P(Smooth, ScrollControllerTestNg, testing::Bool());

/**
 * @tc.name: AnimateTo004
 * @tc.desc: Test AnimateTo with duration animation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, AnimateTo004, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. AnimateTo the position in the scroll
     * @tc.expected: AnimateTo the position
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 1000.f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE / TICK));
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can not over scroll
     */
    AnimateTo(Dimension(CONTENT_MAIN_SIZE), 1000.f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(-(CONTENT_MAIN_SIZE + ITEM_MAIN_SIZE) / 2));
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 1000.f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE / TICK));
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo005
 * @tc.desc: Test AnimateTo with duration animation, canOverScroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, AnimateTo005, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can over scroll
     */
    bool canOverScroll = true;
    MockAnimationManager::GetInstance().SetTicks(5);
    AnimateTo(Dimension(1000.f), 1000.f, Curves::EASE, false, canOverScroll);
    EXPECT_TRUE(TickPosition(-200.f));
    EXPECT_TRUE(TickPosition(-400.f));
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));
    EXPECT_TRUE(TickPosition(-800.f)); // Tick doesn't advance new animations created within the same tick
    EXPECT_TRUE(TickPosition(-760.f));
    EXPECT_TRUE(TickPosition(-720.f));
    EXPECT_TRUE(TickPosition(-680.f));
    EXPECT_TRUE(TickPosition(-640.f));
    EXPECT_TRUE(TickPosition(-VERTICAL_SCROLLABLE_DISTANCE));
}

/**
 * @tc.name: ScrollToEdge001
 * @tc.desc: Test ScrollToEdge
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, ScrollToEdge001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. SCROLL_BOTTOM
     * @tc.expected: Scroll to bottom with animation
     */
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, 200.f);
    EXPECT_TRUE(Position(-VERTICAL_SCROLLABLE_DISTANCE));

    /**
     * @tc.steps: step2. SCROLL_TOP
     * @tc.expected: Scroll to top with animation
     */
    pattern_->SetAnimateCanOverScroll(true);
    pattern_->SetIsOverScroll(true);
    pattern_->SetCanStayOverScroll(true);
    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, 200.f);
    EXPECT_TRUE(Position(0));
    EXPECT_FALSE(pattern_->GetIsOverScroll());
    EXPECT_FALSE(pattern_->GetCanStayOverScroll());
    EXPECT_FALSE(pattern_->animateCanOverScroll_);
}

/**
 * @tc.name: Fling001
 * @tc.desc: Test Fling
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, Fling001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. Fling, the flingVelocity greater than 0
     * @tc.expected: Scroll down
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    const float finalPosition = 100.f;
    const float flingVelocity = finalPosition * FRICTION * FRICTION_SCALE;
    Fling(flingVelocity);
    EXPECT_TRUE(TickPosition(-finalPosition / TICK));
    EXPECT_TRUE(TickPosition(-finalPosition));

    /**
     * @tc.steps: step2. Fling, the flingVelocity less than 0
     * @tc.expected: Scroll up
     */
    Fling(-flingVelocity);
    EXPECT_TRUE(TickPosition(-finalPosition / TICK));
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: GetInfo001
 * @tc.desc: Test non-action GetCurrentOffset/GetScrollDirection/IsAtEnd/GetItemRect
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetInfo001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);
    EXPECT_EQ(GetScrollDirection(), Axis::VERTICAL);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset()));
    EXPECT_FALSE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(0), Rect(0, 0, WIDTH, CONTENT_MAIN_SIZE)));

    /**
     * @tc.steps: step1. AnimateTo the position
     */
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, false);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset(0, ITEM_MAIN_SIZE)));
    EXPECT_FALSE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(0), Rect(0, -ITEM_MAIN_SIZE, WIDTH, CONTENT_MAIN_SIZE)));

    /**
     * @tc.steps: step2. AnimateTo bottom
     */
    AnimateTo(Dimension(CONTENT_MAIN_SIZE), 0, nullptr, false);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset(0, VERTICAL_SCROLLABLE_DISTANCE)));
    EXPECT_TRUE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(0), Rect(0, -VERTICAL_SCROLLABLE_DISTANCE, WIDTH, CONTENT_MAIN_SIZE)));
}

/**
 * @tc.name: CreateWithResourceObjScrollBarColor
 * @tc.desc: Test CreateWithResourceObjScrollBarColor in ScrollModelNG
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, CreateWithResourceObjScrollBarColor001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_EQ(pattern_->resourceMgr_, nullptr);

    RefPtr<ResourceObject> invalidResObj = AceType::MakeRefPtr<ResourceObject>("", "", 0);
    model.CreateWithResourceObjScrollBarColor(invalidResObj);
    ASSERT_NE(pattern_->resourceMgr_, nullptr);
    EXPECT_NE(pattern_->resourceMgr_->resMap_.size(), 0);

    std::vector<ResourceObjectParams> params;
    AddMockResourceData(0, Color::BLUE);
    auto resObjWithString = AceType::MakeRefPtr<ResourceObject>(
        0, static_cast<int32_t>(ResourceType::COLOR), params, "", "", Container::CurrentIdSafely());
    model.CreateWithResourceObjScrollBarColor(resObjWithString);
    pattern_->resourceMgr_->ReloadResources();
    uint32_t color = ScrollModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_));
    EXPECT_EQ(color, Color::BLUE.GetValue());

    model.CreateWithResourceObjScrollBarColor(resObjWithString);
    ASSERT_NE(pattern_->resourceMgr_, nullptr);
    EXPECT_NE(pattern_->resourceMgr_->resMap_.size(), 0);

    color = ScrollModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_));
    EXPECT_NE(color, Color::RED.GetValue());
}

/**
 * @tc.name: CreateWithResourceObjScrollBarColor
 * @tc.desc: Test CreateWithResourceObjScrollBarColor in ScrollModelNG
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, CreateWithResourceObjScrollBarColor002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_EQ(pattern_->resourceMgr_, nullptr);

    RefPtr<ResourceObject> invalidResObj = AceType::MakeRefPtr<ResourceObject>("", "", 0);
    ScrollModelNG::CreateWithResourceObjScrollBarColor(AceType::RawPtr(frameNode_), invalidResObj);
    model.CreateWithResourceObjScrollBarColor(invalidResObj);
    ASSERT_NE(pattern_->resourceMgr_, nullptr);
    EXPECT_NE(pattern_->resourceMgr_->resMap_.size(), 0);

    std::vector<ResourceObjectParams> params;
    AddMockResourceData(0, Color::BLUE);
    auto resObjWithString = AceType::MakeRefPtr<ResourceObject>(
        0, static_cast<int32_t>(ResourceType::COLOR), params, "", "", Container::CurrentIdSafely());
    ScrollModelNG::CreateWithResourceObjScrollBarColor(AceType::RawPtr(frameNode_), resObjWithString);
    pattern_->resourceMgr_->ReloadResources();
    uint32_t color = ScrollModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_));
    EXPECT_EQ(color, Color::BLUE.GetValue());

    model.CreateWithResourceObjScrollBarColor(resObjWithString);
    ASSERT_NE(pattern_->resourceMgr_, nullptr);
    EXPECT_NE(pattern_->resourceMgr_->resMap_.size(), 0);

    color = ScrollModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_));
    EXPECT_NE(color, Color::RED.GetValue());
}

/**
 * @tc.name: OnInjectionEventTest001
 * @tc.desc: test OnInjectionEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, OnInjectionEventTest001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::NONE, true, EffectEdge::START);
    CreateContent();
    CreateScrollDone();

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    command = R"({"cmd":"scrollBackward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, -40);

    command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    command = R"({"cmd":"scrollward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    command = R"({"cmd":"scrollForward","eventId":123123,"ratio":1.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    command = R"({"cmd":"scrollBackward"})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, -400);

    command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, -360);

    command = R"({"cmd":"scrollForward"})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 40);

    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    command = R"({"cmd":"scrollBackward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, -600);
}

/**
 * @tc.name: OnInjectionEventTest002
 * @tc.desc: test OnInjectionEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, OnInjectionEventTest002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::NONE, true, EffectEdge::START);
    CreateContent(30);
    CreateScrollDone();
    EXPECT_TRUE(pattern_->IsAtTop());
    EXPECT_TRUE(pattern_->IsAtBottom());

    std::string command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);
}

/**
 * @tc.name: OnInjectionEventTest003
 * @tc.desc: test OnInjectionEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, OnInjectionEventTest003, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::NONE, true, EffectEdge::START);
    CreateContent();
    CreateScrollDone();

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, 0);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":-20})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, 0);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":20})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -20);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":-10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    command = R"({"cmd":"scrolloffset","eventId":123123,"offset":10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    command = R"({"cmd":"scrollByOffset","eventId":123123)";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);

    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -600);
    EXPECT_EQ(pattern_->GetFirstIndex(), -1);
}

/**
 * @tc.name: ReportComponentChangeEventTest001
 * @tc.desc: Test ReportComponentChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, ReportComponentChangeEventTest001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::NONE, true, EffectEdge::START);
    CreateContent(30);
    CreateScrollDone();
    EXPECT_TRUE(pattern_->IsAtTop());
    MockUiSessionManager* mockUiSessionManager =
        reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mockUiSessionManager, GetComponentChangeEventRegistered()).WillRepeatedly(Return(true));

    pattern_->ReportScroll(false, ScrollError::SCROLL_ERROR_OTHER, 123);
    pattern_->ReportScroll(true, ScrollError::SCROLL_NO_ERROR, 123);
    pattern_->ReportOnItemScrollEvent("onReachStart");

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->currentOffset_, 0);
}

/**
 * @tc.name: GetBindingFrameNodeId001
 * @tc.desc: Test GetBindingFrameNodeId returns valid node id after scroll is created
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetBindingFrameNodeId001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Get the binding frame node id from controller
     * @tc.expected: The node id should match the scroll frame node's id
     */
    auto nodeId = positionController_->GetBindingFrameNodeId();
    EXPECT_EQ(nodeId, frameNode_->GetId());
}

/**
 * @tc.name: GetBindingFrameNodeId002
 * @tc.desc: Test GetBindingFrameNodeId returns -1 when pattern is not set
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetBindingFrameNodeId002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a standalone controller without binding to any pattern
     * @tc.expected: GetBindingFrameNodeId should return -1
     */
    auto controller = AceType::MakeRefPtr<ScrollableController>();
    EXPECT_EQ(controller->GetBindingFrameNodeId(), -1);
}

/**
 * @tc.name: GetBindingFrameNodeId003
 * @tc.desc: Test ScrollControllerBase default GetBindingFrameNodeId returns -1
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetBindingFrameNodeId003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Call base class GetBindingFrameNodeId
     * @tc.expected: Default implementation should return -1
     */
    auto baseController = AceType::MakeRefPtr<ScrollControllerBase>();
    EXPECT_EQ(baseController->GetBindingFrameNodeId(), -1);
}
/**
 * @tc.name: GetScrollState001
 * @tc.desc: Test GetScrollState returns IDLE by default
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState002
 * @tc.desc: Test GetScrollState returns SCROLL when scrollSource is UPDATE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState002, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
}

/**
 * @tc.name: GetScrollState003
 * @tc.desc: Test GetScrollState returns FLING when scrollSource is ANIMATION
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState003, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_ANIMATION);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::FLING);
}

/**
 * @tc.name: GetScrollState004
 * @tc.desc: Test ScrollableController GetScrollState delegates to pattern
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState004, TestSize.Level1)
{
    auto controller = AceType::MakeRefPtr<ScrollableController>();
    ASSERT_NE(controller, nullptr);
    EXPECT_EQ(controller->GetScrollState(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState005
 * @tc.desc: Test ScrollControllerBase default GetScrollState returns IDLE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState005, TestSize.Level1)
{
    auto baseController = AceType::MakeRefPtr<ScrollControllerBase>();
    EXPECT_EQ(baseController->GetScrollState(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState006
 * @tc.desc: Test GetScrollState returns SCROLL for all event-driven scroll sources
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState006, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    const std::vector<int32_t> scrollSources = { SCROLL_FROM_UPDATE, SCROLL_FROM_AXIS, SCROLL_FROM_BAR,
        SCROLL_FROM_CROWN, SCROLL_FROM_BAR_OVER_DRAG };
    for (auto source : scrollSources) {
        pattern_->SetScrollSource(source);
        EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
    }
}

/**
 * @tc.name: GetScrollState007
 * @tc.desc: Test GetScrollState returns FLING for all animation-driven scroll sources
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState007, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    const std::vector<int32_t> animationSources = { SCROLL_FROM_ANIMATION, SCROLL_FROM_ANIMATION_SPRING,
        SCROLL_FROM_ANIMATION_CONTROLLER, SCROLL_FROM_BAR_FLING };
    for (auto source : animationSources) {
        pattern_->SetScrollSource(source);
        EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::FLING);
    }
}

/**
 * @tc.name: GetScrollState008
 * @tc.desc: Test GetScrollState returns IDLE for non-scrolling sources and default scrollSource_
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState008, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
    const std::vector<int32_t> idleSources = { SCROLL_FROM_NONE, SCROLL_FROM_JUMP, SCROLL_FROM_CHILD,
        SCROLL_FROM_FOCUS_JUMP, SCROLL_FROM_ROTATE, SCROLL_FROM_INDEXER, SCROLL_FROM_START, SCROLL_FROM_STATUSBAR,
        SCROLL_FROM_LAYOUT };
    for (auto source : idleSources) {
        pattern_->SetScrollSource(source);
        EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
    }
}

/**
 * @tc.name: GetScrollState009
 * @tc.desc: Test scrollSource_ persists across frames and switches with source changes
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState009, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
    pattern_->SetScrollSource(SCROLL_FROM_ANIMATION);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::FLING);
    pattern_->SetScrollSource(SCROLL_FROM_AXIS);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
}

/**
 * @tc.name: GetScrollState010
 * @tc.desc: Test FireOnScrollStop resets scroll state to IDLE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState010, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
    pattern_->FireOnScrollStop([]() {}, []() {});
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState011
 * @tc.desc: Test JUMP source maps to IDLE for query API
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState011, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_JUMP);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState012
 * @tc.desc: Test layout-level reset affects GetScrollState only, GetScrollStateForQuery keeps session-level state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState012, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollState(), ScrollState::SCROLL);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
    /**
     * @tc.steps: Layout reset touches only the frame-level source.
     * @tc.expected: GetScrollState becomes IDLE while GetScrollStateForQuery stays SCROLL.
     */
    pattern_->SetScrollSourceLayout(SCROLL_FROM_NONE);
    EXPECT_EQ(pattern_->GetScrollState(), ScrollState::IDLE);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
}

/**
 * @tc.name: GetScrollState013
 * @tc.desc: Test zero-delta nested delegation frame does not pollute query state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState013, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    /**
     * @tc.steps: step1. Simulate nested delegation: child hands a zero-delta UPDATE frame to parent.
     * @tc.expected: Query state stays IDLE because no real scroll happened,
     * while frame-level state keeps original behavior.
     */
    pattern_->HandleScroll(0.0f, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL, 0.0f);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
    EXPECT_EQ(pattern_->GetScrollState(), ScrollState::SCROLL);
}

/**
 * @tc.name: GetScrollState014
 * @tc.desc: Test zero-delta frame preserves previous session-level query state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState014, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    pattern_->SetScrollSource(SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
    /**
     * @tc.steps: step1. A zero-delta delegation frame arrives during an active drag session.
     * @tc.expected: Query state keeps SCROLL, neither polluted nor reset.
     */
    pattern_->HandleScroll(0.0f, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL, 0.0f);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::SCROLL);
}

/**
 * @tc.name: GetScrollState015
 * @tc.desc: Test zero-delta AXIS delegation frame does not pollute query state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState015, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    /**
     * @tc.steps: step1. Simulate a zero-delta AXIS frame delegated from child.
     * @tc.expected: Query state stays IDLE.
     */
    pattern_->HandleScroll(0.0f, SCROLL_FROM_AXIS, NestedState::CHILD_SCROLL, 0.0f);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
}

/**
 * @tc.name: GetScrollState016
 * @tc.desc: Test zero-delta CROWN delegation frame does not pollute query state
 * @tc.type: FUNC
 */
HWTEST_F(ScrollControllerTestNg, GetScrollState016, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();
    /**
     * @tc.steps: step1. Simulate a zero-delta CROWN frame delegated from child on wearables.
     * @tc.expected: Query state stays IDLE, same as UPDATE/AXIS drag frames.
     */
    pattern_->HandleScroll(0.0f, SCROLL_FROM_CROWN, NestedState::CHILD_SCROLL, 0.0f);
    EXPECT_EQ(pattern_->GetScrollStateForQuery(), ScrollState::IDLE);
}

} // namespace OHOS::Ace::NG

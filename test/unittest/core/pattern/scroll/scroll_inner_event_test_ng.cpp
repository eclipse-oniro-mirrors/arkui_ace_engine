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

#include "mock_task_executor.h"
#include "scroll_test_ng.h"
#include "test/mock/core/animation/mock_animation_manager.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/rosen/mock_canvas.h"

#include "core/components/scroll/scroll_bar_theme.h"

namespace OHOS::Ace::NG {
namespace {
const Offset OUT_SCROLLBAR_OFFSET = Offset(100.f, 100.f);
const Offset IN_ACTIVE_BAR_OFFSET = Offset(238.f, 80.f);
const Offset BELOW_ACTIVE_BAR_OFFSET = Offset(238.f, 300.f);
const Point OUT_SCROLLBAR_POINT = Point(100.f, 100.f);
const Point IN_ACTIVE_BAR_POINT = Point(238.f, 80.f);
const Point BELOW_ACTIVE_BAR_POINT = Point(238.f, 300.f);
const float VERTICAL_RATIO = HEIGHT / CONTENT_MAIN_SIZE;
const float HORIZONTAL_RATIO = WIDTH / CONTENT_MAIN_SIZE;
} // namespace

class ScrollInnerEventTestNg : public ScrollTestNg {
private:
    void MouseOnScroll(MouseButton mouseButton, MouseAction mouseAction, Offset localLocation);
    void TouchOnScroll(TouchType touchType);
    void HoverOnScrollBar(bool isHover);
    void TouchOnScrollBar(TouchType touchType, SourceType sourceType, Offset localLocation);
    void MouseOnScrollBar(MouseButton mouseButton, MouseAction mouseAction, Offset localLocation);
    void DragScrollBarStart(GestureEvent& gesture);
    void DragScrollBarUpdate(GestureEvent& gesture);
    void DragScrollBarEnd(GestureEvent& gesture);
    void DragScrollBarAction(Offset startOffset, float dragDelta, float velocity = 0);
    void CollectTouchTarget(Point point);
};

void ScrollInnerEventTestNg::MouseOnScroll(MouseButton mouseButton, MouseAction mouseAction, Offset localLocation)
{
    auto mouseEvent = pattern_->mouseEvent_->GetOnMouseEventFunc();
    MouseInfo mouseInfo;
    mouseInfo.SetButton(mouseButton);
    mouseInfo.SetAction(mouseAction);
    mouseInfo.SetLocalLocation(localLocation);
    mouseEvent(mouseInfo);
}

void ScrollInnerEventTestNg::TouchOnScroll(TouchType touchType)
{
    TouchLocationInfo locationInfo(1);
    locationInfo.SetTouchType(touchType);
    TouchEventInfo eventInfo("touch");
    eventInfo.AddTouchLocationInfo(std::move(locationInfo));
    auto touchEvent = pattern_->touchEvent_->GetTouchEventCallback();
    touchEvent(eventInfo);
}

void ScrollInnerEventTestNg::HoverOnScrollBar(bool isHover)
{
    auto hoverEvent = scrollBar_->GetHoverEvent()->GetOnHoverEventFunc();
    hoverEvent(isHover);
}

void ScrollInnerEventTestNg::TouchOnScrollBar(TouchType touchType, SourceType sourceType, Offset localLocation)
{
    TouchLocationInfo locationInfo(1);
    locationInfo.SetTouchType(touchType);
    locationInfo.SetLocalLocation(localLocation);
    TouchEventInfo eventInfo("touch");
    eventInfo.SetSourceDevice(sourceType);
    eventInfo.AddTouchLocationInfo(std::move(locationInfo));
    auto touchEvent = scrollBar_->GetTouchEvent()->GetTouchEventCallback();
    touchEvent(eventInfo);
}

void ScrollInnerEventTestNg::MouseOnScrollBar(MouseButton mouseButton, MouseAction mouseAction, Offset localLocation)
{
    MouseInfo mouseInfo;
    mouseInfo.SetButton(mouseButton);
    mouseInfo.SetAction(mouseAction);
    mouseInfo.SetLocalLocation(localLocation);
    auto mouseEvent = scrollBar_->GetMouseEvent()->GetOnMouseEventFunc();
    mouseEvent(mouseInfo);
}

void ScrollInnerEventTestNg::DragScrollBarStart(GestureEvent& gesture)
{
    gesture.SetSourceTool(SourceTool::FINGER);
    gesture.SetInputEventType(InputEventType::TOUCH_SCREEN);
    TouchOnScrollBar(TouchType::DOWN, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    auto HandleDragStart = *(scrollBar_->panRecognizer_->onActionStart_);
    HandleDragStart(gesture);
}

void ScrollInnerEventTestNg::DragScrollBarUpdate(GestureEvent& gesture)
{
    float dragDelta = gesture.GetMainDelta();
    Axis axis = pattern_->GetAxis();
    Offset deltaOffset = axis == Axis::VERTICAL ? Offset(0, dragDelta) : Offset(dragDelta, 0);
    gesture.SetGlobalPoint(gesture.GetGlobalPoint() + deltaOffset);
    gesture.SetGlobalLocation(gesture.GetGlobalLocation() + deltaOffset);
    gesture.SetLocalLocation(gesture.GetLocalLocation() + deltaOffset);
    double velocity = dragDelta > 0 ? DRAG_VELOCITY : -DRAG_VELOCITY;
    gesture.SetMainVelocity(velocity);
    auto HandleDragUpdate = *(scrollBar_->panRecognizer_->onActionUpdate_);
    HandleDragUpdate(gesture);
    FlushUITasks();
}

void ScrollInnerEventTestNg::DragScrollBarEnd(GestureEvent& gesture)
{
    gesture.SetMainDelta(0);
    gesture.SetGlobalPoint(gesture.GetGlobalPoint());
    gesture.SetGlobalLocation(gesture.GetGlobalLocation());
    gesture.SetLocalLocation(gesture.GetLocalLocation());
    TouchOnScrollBar(TouchType::UP, SourceType::TOUCH, gesture.GetLocalLocation());
    auto HandleDragEnd = *(scrollBar_->panRecognizer_->onActionEnd_);
    HandleDragEnd(gesture);
    FlushUITasks();
}

void ScrollInnerEventTestNg::DragScrollBarAction(Offset startOffset, float dragDelta, float velocity)
{
    GestureEvent gesture;
    gesture.SetGlobalPoint(Point() + startOffset);
    gesture.SetGlobalLocation(startOffset);
    gesture.SetLocalLocation(startOffset);
    DragScrollBarStart(gesture);

    gesture.SetMainDelta(dragDelta);
    DragScrollBarUpdate(gesture);

    gesture.SetMainVelocity(velocity);
    DragScrollBarEnd(gesture);
}

void ScrollInnerEventTestNg::CollectTouchTarget(Point point)
{
    PointF localPoint = PointF(point.GetX(), point.GetY());
    RefPtr<FrameNode> frameNode = frameNode_;
    OffsetF coordinateOffset;
    TouchRestrict touchRestrict;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    RefPtr<TargetComponent> targetComponent;
    ResponseLinkResult responseLinkResult;
    auto scrollableActuator = frameNode_->GetOrCreateGestureEventHub()->scrollableActuator_;
    scrollableActuator->CollectTouchTarget(coordinateOffset, touchRestrict, getEventTargetImpl, result, localPoint,
        frameNode, targetComponent, responseLinkResult, 1);
}

/**
 * @tc.name: HandleClick001
 * @tc.desc: Test click action, move mouse in activeBar and click, than move out
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleClick001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Mouse is out of scrollBar
     */
    MouseOnScroll(MouseButton::NONE_BUTTON, MouseAction::MOVE, OUT_SCROLLBAR_OFFSET);
    EXPECT_EQ(pattern_->locationInfo_, OUT_SCROLLBAR_OFFSET);

    /**
     * @tc.steps: step2. Move mouse in activeBar
     * @tc.expected: PlayScrollBarGrowAnimation
     */
    MouseOnScrollBar(MouseButton::NONE_BUTTON, MouseAction::HOVER, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(scrollBar_->IsHover());
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::GROW);
    MouseOnScroll(MouseButton::NONE_BUTTON, MouseAction::HOVER, IN_ACTIVE_BAR_OFFSET);
    EXPECT_EQ(pattern_->locationInfo_, IN_ACTIVE_BAR_OFFSET);
    HoverOnScrollBar(true);

    /**
     * @tc.steps: step3. Press in activeBar
     */
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(scrollBar_->isMousePressed_);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(pattern_->isMousePressed_);

    /**
     * @tc.steps: step4. Trigger HandleClickEvent
     * @tc.expected: Not trigger ScrollPage because in the activeBar
     */
    MockAnimationManager::GetInstance().SetTicks(1);
    pattern_->HandleClickEvent();
    EXPECT_TRUE(TickPosition(0));

    /**
     * @tc.steps: step5. Release in activeBar
     */
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::RELEASE, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->isMousePressed_);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::RELEASE, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(pattern_->isMousePressed_);
    HoverOnScrollBar(true);

    /**
     * @tc.steps: step6. Move mouse out of activeBar
     * @tc.expected: PlayScrollBarShrinkAnimation
     */
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::MOVE, OUT_SCROLLBAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsHover());
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::MOVE, OUT_SCROLLBAR_OFFSET);
    EXPECT_EQ(pattern_->locationInfo_, OUT_SCROLLBAR_OFFSET);
    HoverOnScrollBar(false);
}

/**
 * @tc.name: HandleClick002
 * @tc.desc: Test click action, click below/above the activeBar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleClick002, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Press below activeBar, trigger HandleClickEvent
     * @tc.expected: Scroll page down
     */
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, BELOW_ACTIVE_BAR_OFFSET);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, BELOW_ACTIVE_BAR_OFFSET);
    MockAnimationManager::GetInstance().SetTicks(TICK);
    pattern_->HandleClickEvent();
    EXPECT_TRUE(TickPosition(-HEIGHT / TICK));
    EXPECT_TRUE(TickPosition(-HEIGHT));

    /**
     * @tc.steps: step2. Press above activeBar, trigger HandleClickEvent
     * @tc.expected: Scroll page up
     */
    const Offset aboveActiveBarPoint = Offset(238.f, 0);
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, aboveActiveBarPoint);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, aboveActiveBarPoint);
    pattern_->HandleClickEvent();
    EXPECT_TRUE(TickPosition(-HEIGHT / TICK));
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: HandleClick003
 * @tc.desc: Test click action, click not on the scrollBar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleClick003, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Press not in scrollBar
     * @tc.expected: Not scroll
     */
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, OUT_SCROLLBAR_OFFSET);
    MockAnimationManager::GetInstance().SetTicks(1);
    pattern_->HandleClickEvent();
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: HandleLongPress001
 * @tc.desc: Test HandleLongPress action, move touch in activeBar and long press
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleLongPress001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. LongPress in scrollBar
     * @tc.expected: Not trigger ScrollPage because in the activeBar
     */
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, IN_ACTIVE_BAR_OFFSET);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, IN_ACTIVE_BAR_OFFSET);
    MockAnimationManager::GetInstance().SetTicks(1);
    scrollBar_->HandleLongPress(true);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: HandleLongPress002
 * @tc.desc: Test HandleLongPress action, long press below/above the activeBar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleLongPress002, TestSize.Level1)
{
    auto mockTaskExecutor = AceType::MakeRefPtr<MockScrollTaskExecutor>();
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = mockTaskExecutor;
    ScrollModelNG model = CreateScroll();
    model.SetDisplayMode(static_cast<int>(DisplayMode::ON));
    CreateContent(2000.f);
    CreateScrollDone();

    /**
     * @tc.steps: step1. Press below activeBar, longPress in scrollBar
     * @tc.expected: Trigger Scroll page down until the scrollBar scroll to mouse position
     */
    const Offset belowActiveBarPoint = Offset(238.f, 200.f);
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, belowActiveBarPoint);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, belowActiveBarPoint);
    MockAnimationManager::GetInstance().SetTicks(1);
    scrollBar_->HandleLongPress(true);
    EXPECT_TRUE(TickPosition(-HEIGHT));
    mockTaskExecutor->RunDelayTask();
    EXPECT_TRUE(TickPosition(-HEIGHT * 2));
    mockTaskExecutor->RunDelayTask();
    EXPECT_TRUE(TickPosition(-HEIGHT * 2));

    /**
     * @tc.steps: step2. Press above activeBar, longPress in scrollBar
     * @tc.expected: Trigger Scroll page up until the scrollBar scroll to mouse position
     */
    const Offset aboveActiveBarPoint = Offset(238.f, 0);
    MouseOnScrollBar(MouseButton::LEFT_BUTTON, MouseAction::PRESS, aboveActiveBarPoint);
    MouseOnScroll(MouseButton::LEFT_BUTTON, MouseAction::PRESS, aboveActiveBarPoint);
    scrollBar_->HandleLongPress(true);
    EXPECT_TRUE(TickPosition(-HEIGHT * 1));
    mockTaskExecutor->RunDelayTask();
    EXPECT_TRUE(TickPosition(0));
    mockTaskExecutor->RunDelayTask();
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: TouchEvent001
 * @tc.desc: Test touch event, will GROW when touch down the activeBar, will SHRINK when touch up
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, TouchEvent001, TestSize.Level1)
{
    auto mockTaskExecutor = AceType::MakeRefPtr<MockScrollTaskExecutor>();
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = mockTaskExecutor;
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Touch down in activeBar
     * @tc.expected: PlayScrollBarGrowAnimation
     */
    TouchOnScroll(TouchType::DOWN);
    EXPECT_TRUE(scrollable_->isTouching_);
    TouchOnScrollBar(TouchType::DOWN, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(scrollBar_->IsPressed());
    EXPECT_NE(scrollBar_->fingerId_, -1);
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::APPEAR);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_ACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::GROW);

    /**
     * @tc.steps: step2. Touch move in activeBar
     * @tc.expected: No change
     */
    TouchOnScrollBar(TouchType::MOVE, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::APPEAR);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_ACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::GROW);

    /**
     * @tc.steps: step3. Touch move out of activeBar
     * @tc.expected: No change
     */
    TouchOnScrollBar(TouchType::MOVE, SourceType::TOUCH, OUT_SCROLLBAR_OFFSET);
    EXPECT_TRUE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::APPEAR);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_ACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::GROW);

    /**
     * @tc.steps: step4. Touch up
     * @tc.expected: PlayScrollBarShrinkAnimation
     */
    TouchOnScroll(TouchType::UP);
    EXPECT_FALSE(scrollable_->isTouching_);
    TouchOnScrollBar(TouchType::UP, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->fingerId_, -1);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_INACTIVE_WIDTH);
    mockTaskExecutor->RunDelayTask();
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::DISAPPEAR);
}

/**
 * @tc.name: TouchEvent002
 * @tc.desc: Test touch event, touch with SourceType::MOUSE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, TouchEvent002, TestSize.Level1)
{
    auto mockTaskExecutor = AceType::MakeRefPtr<MockScrollTaskExecutor>();
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = mockTaskExecutor;
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Touch down in activeBar
     * @tc.expected: PlayScrollBarGrowAnimation
     */
    TouchOnScrollBar(TouchType::DOWN, SourceType::MOUSE, IN_ACTIVE_BAR_OFFSET);
    EXPECT_TRUE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::APPEAR);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_ACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::GROW);

    /**
     * @tc.steps: step2. Touch up
     * @tc.expected: PlayScrollBarShrinkAnimation
     */
    TouchOnScrollBar(TouchType::UP, SourceType::MOUSE, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_INACTIVE_WIDTH);
    mockTaskExecutor->RunDelayTask();
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::DISAPPEAR);
}

/**
 * @tc.name: TouchEvent003
 * @tc.desc: Test touch event, touch down not on the activeBar, than move to activeBar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, TouchEvent003, TestSize.Level1)
{
    auto mockTaskExecutor = AceType::MakeRefPtr<MockScrollTaskExecutor>();
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = mockTaskExecutor;
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Touch down below activeBar
     * @tc.expected: No change
     */
    TouchOnScrollBar(TouchType::DOWN, SourceType::TOUCH, BELOW_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_INACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::NONE);

    /**
     * @tc.steps: step2. Touch move in activeBar
     * @tc.expected: No change
     */
    TouchOnScrollBar(TouchType::MOVE, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_INACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::NONE);

    /**
     * @tc.steps: step3. Touch up
     * @tc.expected: No change
     */
    TouchOnScrollBar(TouchType::UP, SourceType::TOUCH, IN_ACTIVE_BAR_OFFSET);
    EXPECT_FALSE(scrollBar_->IsPressed());
    EXPECT_EQ(scrollBar_->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar_->GetActiveRect().Width(), DEFAULT_INACTIVE_WIDTH);
    EXPECT_EQ(scrollBar_->GetHoverAnimationType(), HoverAnimationType::NONE);
}

/**
 * @tc.name: HandleDragScrollBar001
 * @tc.desc: Test handleDrag
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag
     * @tc.expected: Scroll with drag
     */
    float dragDelta = 10;
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, dragDelta);
    float expectOffset = -dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));

    /**
     * @tc.steps: step2. Drag end with velocity
     * @tc.expected: Continue scroll after end
     */
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET + Offset(0, expectOffset), dragDelta, 10);
    EXPECT_LT(GetChildY(frameNode_, 0), expectOffset * 2);
}

/**
 * @tc.name: HandleDragScrollBar002
 * @tc.desc: Test handleDrag in RTL layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar002, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag
     * @tc.expected: Scroll with drag
     */
    float dragDelta = 10;
    DragScrollBarAction(Offset(2, 80), dragDelta);
    float expectOffset = -dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));

    /**
     * @tc.steps: step2. Drag end with velocity
     * @tc.expected: Continue scroll after end
     */
    DragScrollBarAction(Offset(2, 80) + Offset(0, expectOffset), dragDelta, 10);
    EXPECT_LT(GetChildY(frameNode_, 0), expectOffset * 2);
}

/**
 * @tc.name: HandleDragScrollBar003
 * @tc.desc: Test handleDrag in Horizontal
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar003, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag
     * @tc.expected: Scroll with drag
     */
    float dragDelta = 24;
    DragScrollBarAction(Offset(1, 398), dragDelta);
    float expectOffset = -dragDelta / HORIZONTAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));

    /**
     * @tc.steps: step2. Drag end with velocity
     * @tc.expected: Continue scroll after end
     */
    DragScrollBarAction(Offset(1, 398) + Offset(expectOffset, 0), dragDelta, 10);
    EXPECT_LT(GetChildX(frameNode_, 0), expectOffset * 2);
}

/**
 * @tc.name: HandleDragScrollBar004
 * @tc.desc: Test handleDrag in Horizontal and RTL layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar004, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), HORIZONTAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. Drag
     * @tc.expected: Scroll with drag
     */
    float dragDelta = -24;
    DragScrollBarAction(Offset(239, 398), dragDelta);
    float expectOffset = dragDelta / HORIZONTAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));

    /**
     * @tc.steps: step2. Drag end with velocity
     * @tc.expected: Continue scroll after end
     */
    DragScrollBarAction(Offset(239, 398) + Offset(expectOffset, 0), dragDelta, -10);
    expectOffset = -HORIZONTAL_SCROLLABLE_DISTANCE - dragDelta / HORIZONTAL_RATIO * 2;
    EXPECT_GT(GetChildX(frameNode_, 0), expectOffset);
}

/**
 * @tc.name: HandleDragScrollBar005
 * @tc.desc: Test handleDrag with snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar005, TestSize.Level1)
{
    float intervalSize = 100;
    std::vector<Dimension> snapPaginations = {};
    std::pair<bool, bool> enableSnapToSide = { true, true };
    ScrollModelNG model = CreateScroll();
    model.SetScrollSnap(ScrollSnapAlign::START, Dimension(intervalSize), snapPaginations, enableSnapToSide);
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag up less than half of intervalSize
     * @tc.expected: Scroll back
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    float dragDelta = 10;
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, dragDelta);
    float expectOffset = -dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));
    EXPECT_TRUE(TickPosition(expectOffset / TICK));
    EXPECT_TRUE(TickPosition(0));

    /**
     * @tc.steps: step2. Drag up greater than half of intervalSize
     * @tc.expected: Scroll to intervalSize
     */
    dragDelta = 30;
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, dragDelta);
    expectOffset = -dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));
    EXPECT_TRUE(TickPosition((-intervalSize + expectOffset) / TICK));
    EXPECT_TRUE(TickPosition(-intervalSize));

    /**
     * @tc.steps: step3. Drag down with velocity less than half of intervalSize
     * @tc.expected: Scroll back
     */
    dragDelta = -10;
    float velocity = 5;
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, dragDelta, velocity);
    expectOffset = -intervalSize - dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));
    EXPECT_TRUE(TickPosition((-intervalSize + expectOffset) / TICK));
    EXPECT_TRUE(TickPosition(-intervalSize));

    /**
     * @tc.steps: step4. Drag down greater than half of intervalSize
     * @tc.expected: Scroll to intervalSize
     */
    dragDelta = -30;
    velocity = 10;
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, dragDelta, velocity);
    expectOffset = -intervalSize - dragDelta / VERTICAL_RATIO;
    EXPECT_TRUE(Position(expectOffset));
    EXPECT_TRUE(TickPosition(expectOffset / TICK));
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: HandleDragScrollBar006
 * @tc.desc: Test handleDrag with snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar006, TestSize.Level1)
{
    float intervalSize = 100;
    std::vector<Dimension> snapPaginations = {};
    std::pair<bool, bool> enableSnapToSide = { true, true };
    ScrollModelNG model = CreateScroll();
    model.SetScrollSnap(ScrollSnapAlign::START, Dimension(intervalSize), snapPaginations, enableSnapToSide);

    ScrollSource scrollSource;
    auto willEvent = [&scrollSource](Dimension x, Dimension y, ScrollState state, ScrollSource source) {
        scrollSource = source;
        TwoDimensionScrollResult result;
        result.xOffset = x;
        result.yOffset = y;
        return result;
    };
    model.SetOnWillScroll(std::move(willEvent));
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag update inner scrollbar
     * @tc.expected: the source of onWillScroll is SCROLL_FROM_BAR_FLING
     */
    DragScrollBarAction(IN_ACTIVE_BAR_OFFSET, 10.f, 1000.f);
    EXPECT_EQ(scrollSource, ScrollSource::SCROLL_BAR);

    /**
     * @tc.steps: step2. Drag End inner scrollbar
     * @tc.expected: the source of onWillScroll is SCROLL_FROM_BAR_FLING
     */
    MockAnimationManager::GetInstance().Tick();
    EXPECT_EQ(scrollSource, ScrollSource::SCROLL_BAR_FLING);
}

/**
 * @tc.name: HandleDragScrollBar007
 * @tc.desc: Test handleDrag with snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragScrollBar007, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag inner scrollbar
     * @tc.expected: IsDriving is true
     */
    MockAnimationManager::GetInstance().SetTicks(1);
    GestureEvent info;
    scrollBar_->HandleDragStart(info);
    EXPECT_TRUE(scrollBar_->IsDriving());
    
    info.SetMainDelta(10.f);
    info.SetMainVelocity(0.f);
    scrollBar_->HandleDragEnd(info);
    EXPECT_FALSE(scrollBar_->IsDriving());

    /**
     * @tc.steps: step2. Trigger scrollbar fling animation
     * @tc.expected: IsDriving is true
     */
    scrollBar_->HandleDragStart(info);
    EXPECT_TRUE(scrollBar_->IsDriving());
    info.SetMainVelocity(1000.f);
    scrollBar_->HandleDragEnd(info);
    EXPECT_TRUE(scrollBar_->IsDriving());
}

/**
 * @tc.name: RegisterEventByClick001
 * @tc.desc: Test Register Event By Click(CollectTouchTarget)
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, RegisterEventByClick001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetDisplayMode(static_cast<int>(DisplayMode::ON));
    model.SetScrollBarWidth(Dimension(HEIGHT + 1.f)); // will be default
    CreateContent();
    CreateScrollDone();
    EXPECT_TRUE(IsEqual(scrollBar_->activeRect_, Rect(236, 0, 4, 160)));

    int32_t nodeId = 123456;
    frameNode_->UpdateRecycleElmtId(nodeId);
    EXPECT_EQ(frameNode_->GetId(), nodeId);

    /**
     * @tc.steps: step1. Click activeBar
     * @tc.expected: Trigger BarCollectTouchTarget
     */
    CollectTouchTarget(IN_ACTIVE_BAR_POINT);
    EXPECT_EQ(scrollBar_->panRecognizer_->nodeId_, nodeId);

    /**
     * @tc.steps: step2. Click scrollBar
     * @tc.expected: Trigger BarCollectLongPressTarget,CollectScrollableTouchTarget
     */
    CollectTouchTarget(BELOW_ACTIVE_BAR_POINT);
    EXPECT_EQ(scrollBar_->longPressRecognizer_->nodeId_, nodeId);
    EXPECT_EQ(scrollable_->panRecognizerNG_->nodeId_, nodeId);

    /**
     * @tc.steps: step3. Click out of scrollBar
     * @tc.expected: Trigger CollectScrollableTouchTarget
     */
    scrollable_->panRecognizerNG_->SetNodeId(0);
    CollectTouchTarget(OUT_SCROLLBAR_POINT);
    EXPECT_EQ(scrollable_->panRecognizerNG_->nodeId_, nodeId);
}

/**
 * @tc.name: HandleDragEndScrollBar001
 * @tc.desc: Test handleDragEnd in Horizontal and RTL layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, HandleDragEndScrollBar001, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), HORIZONTAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. Drag end with velocity
     * @tc.expected: Continue scroll after end
     */
    DragScrollBarAction(Offset(239, 398), 0.f, -1000.f);
    EXPECT_LE(GetChildX(frameNode_, 0), 60.f);
}

/**
 * @tc.name: MouseEventScrollBar001
 * @tc.desc: Test scroll bar hover.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, MouseEventScrollBar001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. hover in bar slider.
     * @tc.expected: IsHover is true, IsHoverSlider is true.
     */
    MouseOnScrollBar(MouseButton::NONE_BUTTON, MouseAction::MOVE, Offset(239, 10));
    EXPECT_TRUE(scrollBar_->IsHover());
    EXPECT_TRUE(scrollBar_->IsHoverSlider());

    /**
     * @tc.steps: step2. hover in bar but out of slider.
     * @tc.expected: IsHover is true, IsHoverSlider is false.
     */
    MouseOnScrollBar(MouseButton::NONE_BUTTON, MouseAction::MOVE, Offset(239, 350));
    EXPECT_TRUE(scrollBar_->IsHover());
    EXPECT_FALSE(scrollBar_->IsHoverSlider());

    /**
     * @tc.steps: step3. hover out of bar and out of slider.
     * @tc.expected: IsHover is false, IsHoverSlider is false.
     */
    MouseOnScrollBar(MouseButton::NONE_BUTTON, MouseAction::MOVE, Offset(200, 350));
    EXPECT_FALSE(scrollBar_->IsHover());
    EXPECT_FALSE(scrollBar_->IsHoverSlider());
}

/**
 * @tc.name: ScrollBarSetOpacity001
 * @tc.desc: Test scrollBar SetOpacity
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollBarSetOpacity001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetDisplayMode(static_cast<int>(DisplayMode::ON));
    CreateContent();
    CreateScrollDone();
    EXPECT_EQ(pattern_->scrollBarOverlayModifier_->GetOpacity(), UINT8_MAX);

    AnimationOption option;
    option.SetCurve(Curves::SHARP);
    option.SetDuration(300);
    auto propertyCallback = [this]() {
        EXPECT_TRUE(MockAnimationManager::GetInstance().IsAnimationOpen());
        pattern_->scrollBarOverlayModifier_->SetOpacity(0);
        EXPECT_FALSE(MockAnimationManager::GetInstance().IsAnimationOpen());
    };
    auto finishCallback = []() {};
    auto animation = AnimationUtils::StartAnimation(option, propertyCallback, finishCallback);
    EXPECT_EQ(pattern_->scrollBarOverlayModifier_->GetOpacity(), 0);
}

/**
 * @tc.name: ScrollBarOpacityAnimation001
 * @tc.desc: Test scrollBar OpacityAnimation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollBarOpacityAnimation001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetDisplayMode(static_cast<int>(DisplayMode::ON));
    CreateContent();
    CreateScrollDone();
    auto scrollBarOverlayModifier = pattern_->scrollBarOverlayModifier_;
    EXPECT_NE(scrollBarOverlayModifier, nullptr);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacity(), UINT8_MAX);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacityAnimatingType(), OpacityAnimationType::NONE);

    /**
     * @tc.steps: step1. Trigger scrollbar disappear animation
     * @tc.expected: scrollbar opacity will be 0 after animation finish
     */
    scrollBarOverlayModifier->StartOpacityAnimation(OpacityAnimationType::DISAPPEAR);
    EXPECT_NE(scrollBarOverlayModifier->opacityAnimation_, nullptr);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacity(), 0);

    /**
     * @tc.steps: step2. Trigger scrollbar appear animation
     * @tc.expected: scrollbar opacity will be 255 after animation finish
     */
    scrollBarOverlayModifier->opacityAnimation_ = nullptr;
    scrollBarOverlayModifier->StartOpacityAnimation(OpacityAnimationType::APPEAR);
    EXPECT_NE(scrollBarOverlayModifier->opacityAnimation_, nullptr);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacity(), UINT8_MAX);
}

/**
 * @tc.name: ScrollBarOpacityAnimation002
 * @tc.desc: Test scrollBar OpacityAnimation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollBarOpacityAnimation002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetDisplayMode(static_cast<int>(DisplayMode::ON));
    CreateContent();
    CreateScrollDone();
    auto scrollBarOverlayModifier = pattern_->scrollBarOverlayModifier_;
    EXPECT_NE(scrollBarOverlayModifier, nullptr);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacity(), UINT8_MAX);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacityAnimatingType(), OpacityAnimationType::NONE);

    /**
     * @tc.steps: step1. Trigger scrollbar disappear animation
     * @tc.expected: scrollbar opacity is 0
     */
    scrollBarOverlayModifier->SetNavDestinationShow(false);
    scrollBarOverlayModifier->StartOpacityAnimation(OpacityAnimationType::DISAPPEAR);
    EXPECT_EQ(scrollBarOverlayModifier->opacityAnimation_, nullptr);
    EXPECT_EQ(scrollBarOverlayModifier->GetOpacity(), 0);
}

/**
 * @tc.name: ScrollSnap001
 * @tc.desc: Test handleDrag with snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollSnap001, TestSize.Level1)
{
    float intervalSize = 100;
    std::vector<Dimension> snapPaginations = {};
    std::pair<bool, bool> enableSnapToSide = { true, true };
    ScrollModelNG model = CreateScroll();
    model.SetScrollSnap(ScrollSnapAlign::START, Dimension(intervalSize), snapPaginations, enableSnapToSide);
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. Drag end scroll
     * @tc.expected: the source of onWillScroll is SCROLL_FROM_BAR_FLING
     */
    GestureEvent info;
    info.SetMainDelta(-10.f);
    info.SetMainVelocity(-1000000.f);
    auto scrollable = pattern_->GetScrollable();
    scrollable->HandleDragEnd(info);
    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::SNAP);
}

/**
 * @tc.name: ScrollSnap002
 * @tc.desc: Test handleDrag with enablePaging and spring
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollSnap002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEnablePaging(true);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. drag update
     * @tc.expected: the currentOffset of scroll greater than 0
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    DragStart(frameNode_, Offset());
    DragUpdate(10);
    EXPECT_TRUE(Position(10));

    DragUpdate(-5);
    EXPECT_LT(GetChildY(frameNode_, 0), 10);
    EXPECT_GT(GetChildY(frameNode_, 0), 5);

    float velocityDelta = -1000;
    DragEnd(velocityDelta);
    auto scrollable = pattern_->GetScrollable();

    /**
     * @tc.steps: step2. drag end and trigger spring animation
     * @tc.expected: the currentOffset of scroll is 0
     */
    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::SPRING);
    MockAnimationManager::GetInstance().TickByVelocity(velocityDelta);
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, 0.f);

    /**
     * @tc.steps: step3. spring end and trigger snap animation
     * @tc.expected: the currentOffset of scroll is scrollHeight
     */
    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::SNAP);
    MockAnimationManager::GetInstance().TickByVelocity(velocityDelta);
    FlushUITasks();
    EXPECT_EQ(pattern_->currentOffset_, -600);
}

/**
 * @tc.name: ScrollSnap003
 * @tc.desc: Test handleTouchUp and snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollInnerEventTestNg, ScrollSnap003, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEnablePaging(true);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateScrollDone();

    /**
     * @tc.steps: step1. handelTouchUp
     * @tc.expected: trigger snap animation
     */
    pattern_->currentOffset_ = -100;
    auto scrollable = pattern_->GetScrollable();
    scrollable->HandleTouchUp();
    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::SNAP);
    MockAnimationManager::GetInstance().Tick();
    EXPECT_EQ(pattern_->currentOffset_, 0);
}
} // namespace OHOS::Ace::NG

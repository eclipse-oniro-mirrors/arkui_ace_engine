/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License" << std::endl;
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

#include "scroll_bar_test_ng.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

namespace OHOS::Ace::NG {

class ScrollBarEventTestNg : public ScrollBarTestNg {
public:
    void HandleDragStart(GestureEvent info)
    {
        auto handleDragStart = *(pattern_->panRecognizer_->onActionStart_);
        handleDragStart(info);
    }

    void HandleDragUpdate(GestureEvent info)
    {
        auto handleDragUpdate = *(pattern_->panRecognizer_->onActionUpdate_);
        handleDragUpdate(info);
    }

    void HandleDragEnd(GestureEvent info)
    {
        auto handleDragEnd = *(pattern_->panRecognizer_->onActionEnd_);
        handleDragEnd(info);
    }

    void HandleMouseEvent(MouseInfo info)
    {
        auto handleMouseEvent = pattern_->mouseEvent_->GetOnMouseEventFunc();
        handleMouseEvent(info);
    }

    void HandleTouchEvent(TouchEventInfo& info)
    {
        auto touchEvent = pattern_->scrollBar_->GetTouchEvent();
        ASSERT_NE(touchEvent, nullptr);
        (*touchEvent)(info);
    }

    void PrepareTouchEventTest()
    {
        CreateStack();
        CreateScroll();
        CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
        CreateScrollBarChild();
        CreateDone();
        ASSERT_NE(pattern_, nullptr);
        ASSERT_NE(pattern_->scrollBar_, nullptr);
        pattern_->UpdateScrollBarOffset(SCROLL_FROM_NONE);
        pattern_->scrollBar_->SetScrollable(true);
    }

    Offset GetScrollBarActiveCenter() const
    {
        auto activeRect = pattern_->scrollBar_->GetActiveRect();
        return Offset(activeRect.Left() + activeRect.Width() / 2.f, activeRect.Top() + activeRect.Height() / 2.f);
    }

    void SendTouchEvent(TouchType type, int32_t fingerId, const Offset& point,
        SourceType source = SourceType::TOUCH)
    {
        TouchEventInfo info("touch");
        info.SetSourceDevice(source);
        TouchLocationInfo touch(fingerId);
        touch.SetTouchType(type);
        touch.SetLocalLocation(point);
        info.AddTouchLocationInfo(std::move(touch));
        HandleTouchEvent(info);
    }

    void HandleClickEvent()
    {
        ClickInfo info(1);
        auto handleClickEvent = pattern_->clickRecognizer_->onClick_;
        handleClickEvent(info);
    }

    void HandleLongPress()
    {
        GestureEvent info;
        auto handleLongPress = *(pattern_->longPressRecognizer_->onAction_);
        handleLongPress(info);
    }
};

/**
 * @tc.name: HandleDrag001
 * @tc.desc: Test ScrollBar about HandleDrag, the scrollBar and scroll position will change by drag
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 0), RectF(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT)));
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 1), RectF(460.f, 0, SCROLL_BAR_CHILD_WIDTH, SCROLL_HEIGHT)));
    float controlDistance = pattern_->GetControlDistance();
    float scrollableDistance = pattern_->GetScrollableDistance();
    EXPECT_EQ(controlDistance, CONTENT_MAIN_SIZE - SCROLL_HEIGHT);          // 200.f
    EXPECT_EQ(scrollableDistance, SCROLL_HEIGHT - SCROLL_BAR_CHILD_HEIGHT); // 640.f

    /**
     * @tc.steps: step1. HandleDragStart, drag on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, drag up
     * @tc.expected: Can not scroll up because at top
     */
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0.f);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step3. HandleDragUpdate, drag down
     * @tc.expected: Scroll down
     */
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);
    float expectOffset = delta * controlDistance / scrollableDistance;
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset); // 50.f

    /**
     * @tc.steps: step4. HandleDragUpdate, drag down continue
     * @tc.expected: Scroll down
     */
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta * 2);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset * 2); // 100.f

    /**
     * @tc.steps: step5. HandleDragUpdate, drag up
     * @tc.expected: Scroll up
     */
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset); // 50.f

    /**
     * @tc.steps: step6. HandleDragEnd, drag end
     */
    HandleDragEnd(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset);
}

/**
 * @tc.name: HandleDrag002
 * @tc.desc: Test ScrollBar about HandleDrag with InputEventType::AXIS
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag002, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateScrollBarChild();
    CreateDone();
    float controlDistance = pattern_->GetControlDistance();
    float scrollableDistance = pattern_->GetScrollableDistance();

    /**
     * @tc.steps: step1. HandleDragStart, mouse wheel on scrollBar
     */
    auto scrollable = scrollPattern_->GetScrollable();
    auto context = scrollable->context_.Upgrade();
    ASSERT_NE(context, nullptr);
    scrollable->InitAxisAnimator();
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    info.SetInputEventType(InputEventType::AXIS);
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, mouse wheel down
     * @tc.expected: Scroll down
     */
    float delta = -SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    float expectBarPosition = -delta / controlDistance * scrollableDistance;
    EXPECT_EQ(pattern_->GetCurrentPosition(), expectBarPosition); // 512.f
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), -delta);
    scrollable->axisAnimator_->StopAxisAnimation();

    /**
     * @tc.steps: step3. HandleDragUpdate, mouse wheel down continue
     * @tc.expected: Scroll down
     */
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance); // 640.f
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance);
    scrollable->axisAnimator_->StopAxisAnimation();

    /**
     * @tc.steps: step4. HandleDragUpdate, mouse wheel up
     * @tc.expected: Scroll up
     */
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance - expectBarPosition); // 138
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance + delta);              // 40.f

    /**
     * @tc.steps: step5. HandleDragEnd, mouse wheel end
     */
    HandleDragEnd(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance - expectBarPosition); // 138
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance + delta);              // 40.f
}

/**
 * @tc.name: HandleDrag003
 * @tc.desc: Test ScrollBar about HandleDrag in HORIZONTAL Layout and RTL Layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag003, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateStack(Alignment::BOTTOM_CENTER);
    CreateScroll(CONTENT_MAIN_SIZE, Axis::HORIZONTAL);
    CreateScrollBar(true, true, Axis::HORIZONTAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 0), RectF(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT)));
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 1), RectF(0.f, 780.f, SCROLL_WIDTH, SCROLL_BAR_CHILD_WIDTH)));
    float controlDistance = pattern_->GetControlDistance();
    float scrollableDistance = pattern_->GetScrollableDistance();
    EXPECT_EQ(controlDistance, CONTENT_MAIN_SIZE - SCROLL_WIDTH);          // 520.f
    EXPECT_EQ(scrollableDistance, SCROLL_WIDTH - SCROLL_BAR_CHILD_HEIGHT); // 320.f
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance);

    /**
     * @tc.steps: step1. HandleDragStart, drag on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(1.f, SCROLL_HEIGHT - 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, drag left
     * @tc.expected: Scroll left
     */
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(delta, 0.f)));

    /**
     * @tc.steps: step3. HandleDragUpdate, drag left continue
     * @tc.expected: Scroll left
     */
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0.f);

    /**
     * @tc.steps: step4. HandleDragUpdate, drag right
     * @tc.expected: Scroll right
     */
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);

    /**
     * @tc.steps: step5. HandleDragEnd, drag end
     */
    HandleDragEnd(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), delta);
}

/**
 * @tc.name: HandleDrag004
 * @tc.desc: Test ScrollBar about HandleDrag with InputEventType::AXIS in unScrollable Scroll, can not scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag004, TestSize.Level1)
{
    CreateStack();
    CreateScroll(SCROLL_HEIGHT);
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    float scrollableDistance = scrollPattern_->GetScrollableDistance();
    EXPECT_EQ(scrollableDistance, 0.f);

    /**
     * @tc.steps: step1. HandleDragStart, mouse wheel scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    info.SetInputEventType(InputEventType::AXIS);
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, mouse wheel down
     * @tc.expected: Not scroll
     */
    float delta = -SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0.f);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step3. HandleDragEnd, mouse wheel end
     */
    HandleDragEnd(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0.f);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: HandleDrag005
 * @tc.desc: Test ScrollBar about HandleDrag in unScrollable Scroll, can not scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag005, TestSize.Level1)
{
    CreateStack();
    CreateScroll(SCROLL_HEIGHT);
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    float scrollableDistance = scrollPattern_->GetScrollableDistance();
    EXPECT_EQ(scrollableDistance, 0.f);

    /**
     * @tc.steps: step1. HandleDragStart, mouse wheel scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, mouse wheel down
     * @tc.expected: Not scroll
     */
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step3. HandleDragEnd, mouse wheel end
     */
    HandleDragEnd(info);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 0);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: HandleDrag006
 * @tc.desc: Test ScrollBar about HandleDrag with velocity
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDrag006, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. HandleDragEnd with velocity
     * @tc.expected: Trigger animation
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    HandleDragStart(info);
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushUITasks();
    info.SetMainVelocity(1200.f);
    HandleDragEnd(info);
    pattern_->frictionMotion_->NotifyListener(0.f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->frictionController_->IsRunning());

    /**
     * @tc.steps: step2. HandleDragEnd with velocity again
     * @tc.expected: Trigger animation
     */
    HandleDragStart(info);
    HandleDragUpdate(info);
    FlushUITasks();
    HandleDragEnd(info);
    FlushUITasks();
    EXPECT_TRUE(pattern_->frictionController_->IsRunning());
}

/**
 * @tc.name: InBarRegion001
 * @tc.desc: Test InBarRegion
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, InBarRegion001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    EXPECT_TRUE(IsEqual(pattern_->childRect_, RectF(0, 0, SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT)));
    EXPECT_TRUE(pattern_->scrollableEvent_->InBarRegion(PointF(), SourceType::TOUCH));
    EXPECT_FALSE(pattern_->scrollableEvent_->InBarRegion(PointF(0, SCROLL_BAR_CHILD_HEIGHT + 1.f), SourceType::TOUCH));
}

/**
 * @tc.name: InBarRegion002
 * @tc.desc: Test InBarRegion without scrollBar child in VERSION_TWELVE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, InBarRegion002, TestSize.Level1)
{
    const int32_t apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateDone();
    EXPECT_TRUE(IsEqual(pattern_->scrollBar_->hoverRegion_, Rect(8.f, 0, 8.f, 640.f)));
    EXPECT_TRUE(IsEqual(pattern_->scrollBar_->touchRegion_, Rect(-16.f, 0, 32.f, 640.f)));
    EXPECT_TRUE(pattern_->scrollableEvent_->InBarRegion(PointF(), SourceType::TOUCH));
    EXPECT_FALSE(pattern_->scrollableEvent_->InBarRegion(PointF(-20.f, 0), SourceType::TOUCH));
    EXPECT_TRUE(pattern_->scrollableEvent_->InBarRegion(PointF(8.f, 0), SourceType::MOUSE));
    EXPECT_FALSE(pattern_->scrollableEvent_->InBarRegion(PointF(), SourceType::MOUSE));
    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: BarCollectTouchTarget001
 * @tc.desc: Test BarCollectTouchTarget
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, BarCollectTouchTarget001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateDone();

    OffsetF coordinateOffset;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    pattern_->scrollableEvent_->BarCollectTouchTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode_, responseLinkResult);
    EXPECT_FLOAT_EQ(pattern_->panRecognizer_->GetCoordinateOffset().GetX(), coordinateOffset.GetX());
    EXPECT_FLOAT_EQ(pattern_->panRecognizer_->GetCoordinateOffset().GetY(), coordinateOffset.GetY());
    EXPECT_EQ(result.size(), 1);
}

/**
 * @tc.name: BarCollectTouchTarget002
 * @tc.desc: Test BarCollectTouchTarget in VERSION_TWELVE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, BarCollectTouchTarget002, TestSize.Level1)
{
    const int32_t apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateDone();

    OffsetF coordinateOffset;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    pattern_->scrollableEvent_->BarCollectTouchTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode_, responseLinkResult);
    EXPECT_EQ(result.size(), 1);
    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: BarCollectLongPressTarget001
 * @tc.desc: Test BarCollectLongPressTarget
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, BarCollectLongPressTarget001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    OffsetF coordinateOffset;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::LIST_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    pattern_->scrollableEvent_->BarCollectLongPressTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode, responseLinkResult);
    EXPECT_FLOAT_EQ(pattern_->longPressRecognizer_->GetCoordinateOffset().GetX(), coordinateOffset.GetX());
    EXPECT_FLOAT_EQ(pattern_->longPressRecognizer_->GetCoordinateOffset().GetY(), coordinateOffset.GetY());
    EXPECT_EQ(result.size(), 2);
}

/**
 * @tc.name: InBarRectRegion001
 * @tc.desc: Test InBarRectRegion
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, InBarRectRegion001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    EXPECT_TRUE(pattern_->scrollableEvent_->InBarRectRegion(PointF(), SourceType::KEYBOARD));
}

/**
 * @tc.name: ScrollScrollBar001
 * @tc.desc: Test scrollCallback event in VERSION_TWELVE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, ScrollScrollBar001, TestSize.Level1)
{
    const int32_t apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    auto scrollCallback = pattern_->scrollBar_->GetScrollPositionCallback();
    scrollCallback(-100.f, SCROLL_FROM_BAR, false);
    FlushUITasks();
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 100.f);

    auto scrollEnd = pattern_->scrollBar_->GetScrollEndCallback();
    scrollEnd();
    FlushUITasks();

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleDragEnd001
 * @tc.desc: Test ScrollBar about HandleDragEnd in HORIZONTAL Layout and RTL Layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleDragEnd001, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateStack(Alignment::BOTTOM_CENTER);
    CreateScroll(CONTENT_MAIN_SIZE, Axis::HORIZONTAL);
    CreateScrollBar(true, true, Axis::HORIZONTAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 0), RectF(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT)));
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 1), RectF(0.f, 780.f, SCROLL_WIDTH, SCROLL_BAR_CHILD_WIDTH)));
    float controlDistance = pattern_->GetControlDistance();
    float scrollableDistance = pattern_->GetScrollableDistance();
    EXPECT_EQ(controlDistance, CONTENT_MAIN_SIZE - SCROLL_WIDTH);          // 520.f
    EXPECT_EQ(scrollableDistance, SCROLL_WIDTH - SCROLL_BAR_CHILD_HEIGHT); // 320.f
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance);

    /**
     * @tc.steps: step1. HandleDragStart, drag on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(1.f, SCROLL_HEIGHT - 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragEnd, drag left
     * @tc.expected: Scroll left
     */
    info.SetMainVelocity(-1000.f);
    HandleDragEnd(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(stackNode_);
    EXPECT_LE(pattern_->GetCurrentPosition(), 60.f);

    /**
     * @tc.steps: step2. HandleDragEnd, drag right
     * @tc.expected: Scroll right
     */
    info.SetMainVelocity(1000.f);
    HandleDragEnd(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(stackNode_);
    EXPECT_EQ(pattern_->GetCurrentPosition(), scrollableDistance);
}

/**
 * @tc.name: HandleClickEvent001
 * @tc.desc: HandleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleClickEvent001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. Click in scrollBar
     * @tc.expected: Can not scroll
     */
    MouseInfo mouseInfo;
    mouseInfo.SetButton(MouseButton::LEFT_BUTTON);
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetLocalLocation(Offset());
    HandleMouseEvent(mouseInfo);
    HandleClickEvent();
    FlushUITasks();
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step2. Click below scrollBar
     * @tc.expected: Scroll down
     */
    mouseInfo.SetLocalLocation(Offset(0, SCROLL_BAR_CHILD_HEIGHT + 1.f));
    HandleMouseEvent(mouseInfo);
    HandleClickEvent();
    FlushUITasks();
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step3. Click above scrollBar
     * @tc.expected: Scroll up
     */
    // scroll down
    pattern_->UpdateCurrentOffset(1.f, SCROLL_FROM_BAR);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCurrentPosition(), 1.f);
    // click above scrollBar
    mouseInfo.SetLocalLocation(Offset());
    HandleMouseEvent(mouseInfo);
    HandleClickEvent();
    FlushUITasks();
    EXPECT_GT(scrollPattern_->GetTotalOffset(), 0.f);
    EXPECT_LT(scrollPattern_->GetTotalOffset(), 1.f);
}

/**
 * @tc.name: HandleLongPress001
 * @tc.desc: HandleLongPress, Press scrollBar will scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleLongPress001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. Press below scrollBar
     * @tc.expected: Scroll down
     */
    MouseInfo mouseInfo;
    mouseInfo.SetButton(MouseButton::LEFT_BUTTON);
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetLocalLocation(Offset(-100.f, SCROLL_BAR_CHILD_HEIGHT + 1.f));
    HandleMouseEvent(mouseInfo);
    HandleLongPress();
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_TRUE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step2. Press in scrollBar
     * @tc.expected: Can not scroll
     */
    mouseInfo.SetLocalLocation(Offset());
    HandleMouseEvent(mouseInfo);
    HandleLongPress();
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_FALSE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step3. Press above scrollBar
     * @tc.expected: Scroll up
     */
    mouseInfo.SetLocalLocation(Offset(0, -1.f));
    HandleMouseEvent(mouseInfo);
    HandleLongPress();
    EXPECT_TRUE(pattern_->scrollingUp_);
    EXPECT_FALSE(pattern_->scrollingDown_);
}

/**
 * @tc.name: HandleLongPress002
 * @tc.desc: HandleLongPress
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleLongPress002, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. When scrollingUp_
     * @tc.expected: Can not scroll down
     */
    MouseInfo mouseInfo;
    mouseInfo.SetButton(MouseButton::LEFT_BUTTON);
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetLocalLocation(Offset(-100.f, SCROLL_BAR_CHILD_HEIGHT + 1.f));
    HandleMouseEvent(mouseInfo);
    pattern_->scrollingUp_ = true;
    HandleLongPress();
    EXPECT_TRUE(pattern_->scrollingUp_);
    EXPECT_FALSE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step2. When scrollingDown_
     * @tc.expected: Can not scroll up
     */
    mouseInfo.SetLocalLocation(Offset(0, -1.f));
    HandleMouseEvent(mouseInfo);
    pattern_->scrollingUp_ = false;
    pattern_->scrollingDown_ = true;
    HandleLongPress();
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_TRUE(pattern_->scrollingDown_);
}

/**
 * @tc.name: HandleLongPress003
 * @tc.desc: HandleLongPress, When use other mouse action, stop scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleLongPress003, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. Press below scrollBar
     * @tc.expected: Scroll down
     */
    MouseInfo mouseInfo;
    mouseInfo.SetButton(MouseButton::LEFT_BUTTON);
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetLocalLocation(Offset(-100.f, SCROLL_BAR_CHILD_HEIGHT + 1.f));
    HandleMouseEvent(mouseInfo);
    HandleLongPress();
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_TRUE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step2. Mouse move
     * @tc.expected: Stop scroll
     */
    mouseInfo.SetAction(MouseAction::MOVE);
    HandleMouseEvent(mouseInfo);
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_FALSE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step3. Press below scrollBar
     * @tc.expected: Scroll down
     */
    mouseInfo.SetAction(MouseAction::PRESS);
    HandleMouseEvent(mouseInfo);
    HandleLongPress();
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_TRUE(pattern_->scrollingDown_);

    /**
     * @tc.steps: step4. Press by other button
     * @tc.expected: Stop scroll
     */
    mouseInfo.SetButton(MouseButton::RIGHT_BUTTON);
    HandleMouseEvent(mouseInfo);
    EXPECT_FALSE(pattern_->scrollingUp_);
    EXPECT_FALSE(pattern_->scrollingDown_);
}

/**
 * @tc.name: IsScrolling001
 * @tc.desc: Test isScrolling in scrollBar without child
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, IsScrolling001, TestSize.Level1)
{
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateDone();
    EXPECT_FALSE(pattern_->isScrolling_);

    /**
     * @tc.steps: step2. inner scrollbar HandleDragStart
     * @tc.expected: isScrolling_ is true
     */
    GestureEvent info;
    pattern_->scrollBar_->HandleDragStart(info);
    EXPECT_TRUE(pattern_->isScrolling_);

    /**
     * @tc.steps: step2. inner scrollbar HandleDragEnd
     * @tc.expected: isScrolling_ is false
     */
    info.SetMainVelocity(-1000.f);
    pattern_->scrollBar_->HandleDragEnd(info);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_FALSE(pattern_->isScrolling_);
}

/**
 * @tc.name: PanDirection001
 * @tc.desc: Test ScrollBar PanDirection with axis changing
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, PanDirection001, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();
    EXPECT_EQ(pattern_->panRecognizer_->direction_.type, PanDirection::VERTICAL);

    /**
     * @tc.steps: step1. change scrollbar axis to HORIZONTAL
     * @tc.expected: scrollBar panDirection is HORIZONTAL
     */
    layoutProperty_->UpdateAxis(Axis::HORIZONTAL);
    pattern_->OnModifyDone();
    FlushUITasks();
    EXPECT_EQ(pattern_->panRecognizer_->direction_.type, PanDirection::HORIZONTAL);
}

/**
 * @tc.name: OnScrollStartStop001
 * @tc.desc: Test OnScrollStart and OnScrollStart in drag scrollBar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, OnScrollStartStop001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize variables and callback
     * @tc.expected: Variables initialized successfully.
     */
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateDone();

    int32_t isScrollStartCalled = 0;
    OnScrollStartEvent scrollStart = [&isScrollStartCalled]() { isScrollStartCalled++; };
    int32_t isScrollStopCalled = 0;
    int32_t stopHasStart = 0;
    OnScrollStopEvent scrollStop = [&isScrollStopCalled, &isScrollStartCalled, &stopHasStart]() {
        if (isScrollStartCalled - isScrollStopCalled == 1) {
            stopHasStart++;
        }
        isScrollStopCalled++;
    };
    auto eventHub = scrollNode_->GetEventHub<ScrollEventHub>();
    EXPECT_NE(eventHub, nullptr);
    eventHub->SetOnScrollStart(std::move(scrollStart));
    eventHub->SetOnScrollStop(std::move(scrollStop));

    /**
     * @tc.steps: step2. scrollBar dragStart
     * @tc.expected: isScrollStartCalled should be 1.
     */
    GestureEvent info;
    pattern_->scrollBar_->HandleDragStart(info);
    FlushUITasks();
    EXPECT_EQ(isScrollStartCalled, 1);
    EXPECT_EQ(isScrollStopCalled, 0);

    /**
     * @tc.steps: step3. scrollBar dragUpdate.
     * @tc.expected: isScrollStartCalled should be 1.
     */
    info.SetMainDelta(10.f);
    pattern_->scrollBar_->HandleDragUpdate(info);
    FlushUITasks();
    EXPECT_EQ(isScrollStartCalled, 1);
    EXPECT_EQ(isScrollStopCalled, 0);

    /**
     * @tc.steps: step4. scrollBar dragEnd.
     * @tc.expected: isScrollStopCalled should be 1.
     */
    info.SetMainVelocity(0.f);
    pattern_->scrollBar_->HandleDragEnd(info);
    FlushUITasks();
    EXPECT_EQ(isScrollStartCalled, 1);
    EXPECT_EQ(isScrollStopCalled, 1);
    EXPECT_EQ(stopHasStart, 1);
}

/**
 * @tc.name: IsScrollSnapTrigger001
 * @tc.desc: Test drag scrollbar with scroll snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, IsScrollSnapTrigger001, TestSize.Level1)
{
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    scrollPattern_->SetEnablePaging(ScrollPagingStatus::VALID);
    CreateDone();

    /**
     * @tc.steps: step1. scroll set enablePaging and scrollBar dragStart
     * @tc.expected: IsScrollSnapTrigger should be true.
     */
    GestureEvent info;
    pattern_->scrollBar_->HandleDragStart(info);
    EXPECT_TRUE(pattern_->scrollBarProxy_->IsScrollSnapTrigger());

    /**
     * @tc.steps: step2. scrollBar dragEnd.
     * @tc.expected: trigger scroll snap animation.
     */
    info.SetMainDelta(10.f);
    info.SetMainVelocity(1000.f);
    pattern_->scrollBar_->HandleDragEnd(info);
    auto scrollable = scrollPattern_->GetScrollable();
    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::SNAP);
}

/**
 * @tc.name: IsScrollSnapTrigger002
 * @tc.desc: Test drag scrollbar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, IsScrollSnapTrigger002, TestSize.Level1)
{
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateScrollBarChild();
    CreateDone();

    /**
     * @tc.steps: step1. scroll set enablePaging and scrollBar dragStart
     * @tc.expected: IsScrollSnapTrigger should be true.
     */
    GestureEvent info;
    pattern_->HandleDragStart(info);
    EXPECT_TRUE(pattern_->scrollBarProxy_->IsScrollSnapTrigger());
    info.SetMainDelta(10.f);
    info.SetMainVelocity(0.f);
    pattern_->HandleDragEnd(info);
    EXPECT_FALSE(pattern_->scrollBarProxy_->IsScrollSnapTrigger());

    /**
     * @tc.steps: step2. scrollBar dragEnd.
     * @tc.expected: trigger scroll snap animation.
     */
    pattern_->HandleDragStart(info);
    EXPECT_TRUE(pattern_->scrollBarProxy_->IsScrollSnapTrigger());
    info.SetMainVelocity(1000.f);
    pattern_->HandleDragEnd(info);
    EXPECT_TRUE(pattern_->scrollBarProxy_->IsScrollSnapTrigger());
}

/**
 * @tc.name: HandleLongPress004
 * @tc.desc: HandleLongPress early-returns when GetScrollBarInteractive() is false (rect scroll bar with
 *           scrollBarInteractive_=false), so the scroll page callback is never invoked.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleLongPress004, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::ON);
    CreateScrollBarChild();
    CreateDone();

    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(pattern_->scrollBar_, nullptr);
    auto scrollBar = pattern_->scrollBar_;
    EXPECT_EQ(scrollBar->GetShapeMode(), ShapeMode::RECT);
    scrollBar->SetScrollBarInteractive(false);
    EXPECT_FALSE(scrollBar->GetScrollBarInteractive());

    bool callbackCalled = false;
    scrollBar->SetScrollPageCallback(
        [&callbackCalled](bool reverse, bool smooth) {
            callbackCalled = true;
        });
    scrollBar->SetIsMousePressed(true);
    // location below the bar so AnalysisUpOrDown would return true if reached
    scrollBar->SetLocationInfo(Offset(-100.f, SCROLL_BAR_CHILD_HEIGHT + 1.f));

    scrollBar->HandleLongPress(true);
    // CHECK_NULL_VOID(GetScrollBarInteractive()) early-returns, callback must NOT be called
    EXPECT_FALSE(callbackCalled);

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: TouchStateCallbackTouchDownAndUp001
 * @tc.desc: Rect scroll bar invokes the registered touch state callback with true on touch down and false on
 *           touch up when the touch point is inside the bar touch region.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, TouchStateCallbackTouchDownAndUp001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateScrollBarChild();
    CreateDone();

    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(pattern_->scrollBar_, nullptr);
    auto scrollBar = pattern_->scrollBar_;
    pattern_->UpdateScrollBarOffset(SCROLL_FROM_NONE);
    scrollBar->SetScrollable(true);
    ASSERT_TRUE(scrollBar->IsScrollable());
    ASSERT_TRUE(scrollBar->GetScrollBarInteractive());
    auto touchRegion = scrollBar->GetTouchRegion();
    ASSERT_GT(touchRegion.Width(), 0.0f);
    ASSERT_GT(touchRegion.Height(), 0.0f);
    auto touchPoint = Offset(touchRegion.Left() + touchRegion.Width() / 2.f,
        touchRegion.Top() + touchRegion.Height() / 2.f);

    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) {
        pressedStates.push_back(isPressed);
    });

    TouchEventInfo downInfo("touch");
    downInfo.SetSourceDevice(SourceType::TOUCH);
    TouchLocationInfo downTouch(0);
    downTouch.SetTouchType(TouchType::DOWN);
    downTouch.SetLocalLocation(touchPoint);
    downInfo.AddTouchLocationInfo(std::move(downTouch));
    HandleTouchEvent(downInfo);

    ASSERT_EQ(pressedStates.size(), 1);
    EXPECT_TRUE(pressedStates[0]);
    EXPECT_TRUE(scrollBar->IsPressed());

    TouchEventInfo upInfo("touch");
    upInfo.SetSourceDevice(SourceType::TOUCH);
    TouchLocationInfo upTouch(0);
    upTouch.SetTouchType(TouchType::UP);
    upTouch.SetLocalLocation(touchPoint);
    upInfo.AddTouchLocationInfo(std::move(upTouch));
    HandleTouchEvent(upInfo);

    ASSERT_EQ(pressedStates.size(), 2);
    EXPECT_FALSE(pressedStates[1]);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: TouchStateCallbackNullOnTouch001
 * @tc.desc: Touch handling still updates the pressed state and does not crash when no touch state callback is
 *           registered, covering the null callback branch.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, TouchStateCallbackNullOnTouch001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateScrollBarChild();
    CreateDone();

    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(pattern_->scrollBar_, nullptr);
    auto scrollBar = pattern_->scrollBar_;
    pattern_->UpdateScrollBarOffset(SCROLL_FROM_NONE);
    scrollBar->SetScrollable(true);
    ASSERT_TRUE(scrollBar->IsScrollable());
    scrollBar->touchStateCallback_ = nullptr;
    auto touchRegion = scrollBar->GetTouchRegion();
    ASSERT_GT(touchRegion.Width(), 0.0f);
    ASSERT_GT(touchRegion.Height(), 0.0f);
    auto touchPoint = Offset(touchRegion.Left() + touchRegion.Width() / 2.f,
        touchRegion.Top() + touchRegion.Height() / 2.f);

    TouchEventInfo downInfo("touch");
    downInfo.SetSourceDevice(SourceType::TOUCH);
    TouchLocationInfo downTouch(0);
    downTouch.SetTouchType(TouchType::DOWN);
    downTouch.SetLocalLocation(touchPoint);
    downInfo.AddTouchLocationInfo(std::move(downTouch));
    HandleTouchEvent(downInfo);
    EXPECT_TRUE(scrollBar->IsPressed());

    TouchEventInfo upInfo("touch");
    upInfo.SetSourceDevice(SourceType::TOUCH);
    TouchLocationInfo upTouch(0);
    upTouch.SetTouchType(TouchType::UP);
    upTouch.SetLocalLocation(touchPoint);
    upInfo.AddTouchLocationInfo(std::move(upTouch));
    HandleTouchEvent(upInfo);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: TouchStateCallbackDisappearAnimator001
 * @tc.desc: The touch state callback registered by ScrollBarPattern stops the disappear delay task on press and
 *           restarts it on release only for AUTO mode while not scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, TouchStateCallbackDisappearAnimator001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateStack();
    CreateScroll();
    CreateScrollBar(true, true, Axis::VERTICAL, DisplayMode::AUTO);
    CreateScrollBarChild();
    CreateDone();

    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(pattern_->scrollBar_, nullptr);
    auto scrollBar = pattern_->scrollBar_;
    ASSERT_TRUE(scrollBar->touchStateCallback_);
    ASSERT_GT(pattern_->GetControlDistance(), 0.0f);

    auto context = MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto oldTaskExecutor = context->taskExecutor_;
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>(true);

    pattern_->StartDisappearAnimator();
    EXPECT_TRUE(pattern_->disapplearDelayTask_);

    scrollBar->touchStateCallback_(true);
    EXPECT_FALSE(pattern_->disapplearDelayTask_);

    scrollBar->touchStateCallback_(false);
    EXPECT_TRUE(pattern_->disapplearDelayTask_);

    pattern_->disapplearDelayTask_.Cancel();
    EXPECT_FALSE(pattern_->disapplearDelayTask_);
    pattern_->isScrolling_ = true;
    scrollBar->touchStateCallback_(false);
    EXPECT_FALSE(pattern_->disapplearDelayTask_);
    pattern_->isScrolling_ = false;

    pattern_->displayMode_ = DisplayMode::ON;
    scrollBar->touchStateCallback_(false);
    EXPECT_FALSE(pattern_->disapplearDelayTask_);

    context->taskExecutor_ = oldTaskExecutor;
    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchEventEmptyTouches001
 * @tc.desc: HandleTouchEvent ignores an event without touch points and keeps the scroll bar unpressed.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchEventEmptyTouches001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    bool called = false;
    scrollBar->SetTouchStateCallback([&called](bool) { called = true; });

    TouchEventInfo emptyInfo("touch");
    emptyInfo.SetSourceDevice(SourceType::TOUCH);
    HandleTouchEvent(emptyInfo);

    EXPECT_FALSE(called);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchEventGuardBranches001
 * @tc.desc: HandleTouchEvent ignores touch events when the scroll bar is not scrollable or not interactive.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchEventGuardBranches001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    int32_t callbackCount = 0;
    scrollBar->SetTouchStateCallback([&callbackCount](bool) { ++callbackCount; });
    auto touchPoint = GetScrollBarActiveCenter();

    scrollBar->SetScrollable(false);
    ASSERT_FALSE(scrollBar->IsScrollable());
    SendTouchEvent(TouchType::DOWN, 0, touchPoint);
    EXPECT_EQ(callbackCount, 0);
    EXPECT_FALSE(scrollBar->IsPressed());

    scrollBar->SetScrollable(true);
    scrollBar->SetScrollBarInteractive(false);
    ASSERT_FALSE(scrollBar->GetScrollBarInteractive());
    SendTouchEvent(TouchType::DOWN, 0, touchPoint);
    EXPECT_EQ(callbackCount, 0);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchDownMouseSource001
 * @tc.desc: A mouse press inside the hover region also reports pressed state and releases it on mouse up.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchDownMouseSource001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();
    ASSERT_TRUE(scrollBar->InBarHoverRegion(Point(touchPoint.GetX(), touchPoint.GetY())));

    SendTouchEvent(TouchType::DOWN, 5, touchPoint, SourceType::MOUSE);
    ASSERT_EQ(pressedStates.size(), 1);
    EXPECT_TRUE(pressedStates[0]);
    EXPECT_TRUE(scrollBar->IsPressed());
    EXPECT_EQ(scrollBar->fingerId_, 5);

    SendTouchEvent(TouchType::UP, 5, touchPoint, SourceType::MOUSE);
    ASSERT_EQ(pressedStates.size(), 2);
    EXPECT_FALSE(pressedStates[1]);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchDownUnknownSource001
 * @tc.desc: A touch down from an unsupported source device is ignored without changing the pressed state.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchDownUnknownSource001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    int32_t callbackCount = 0;
    scrollBar->SetTouchStateCallback([&callbackCount](bool) { ++callbackCount; });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 0, touchPoint, SourceType::NONE);
    EXPECT_EQ(callbackCount, 0);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchDownWhenHovered001
 * @tc.desc: A touch down while the mouse is hovering over the bar sets pressed but skips the touch state callback.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchDownWhenHovered001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    int32_t callbackCount = 0;
    scrollBar->SetTouchStateCallback([&callbackCount](bool) { ++callbackCount; });
    auto touchPoint = GetScrollBarActiveCenter();

    scrollBar->SetHover(true);
    SendTouchEvent(TouchType::DOWN, 0, touchPoint);
    EXPECT_EQ(callbackCount, 0);
    EXPECT_TRUE(scrollBar->IsPressed());
    scrollBar->SetHover(false);

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchDownRepeated001
 * @tc.desc: A repeated touch down keeps the pressed state and reports the pressed callback again.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchDownRepeated001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 2, touchPoint);
    SendTouchEvent(TouchType::DOWN, 2, touchPoint);
    ASSERT_EQ(pressedStates.size(), 2);
    EXPECT_TRUE(pressedStates[0]);
    EXPECT_TRUE(pressedStates[1]);
    EXPECT_TRUE(scrollBar->IsPressed());
    EXPECT_EQ(scrollBar->fingerId_, 2);

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchUpFingerMismatch001
 * @tc.desc: A touch up from a different finger is ignored and does not release the pressed scroll bar.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchUpFingerMismatch001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 7, touchPoint);
    ASSERT_EQ(pressedStates.size(), 1);

    SendTouchEvent(TouchType::UP, 8, touchPoint);
    EXPECT_EQ(pressedStates.size(), 1);
    EXPECT_TRUE(scrollBar->IsPressed());
    EXPECT_EQ(scrollBar->fingerId_, 7);

    SendTouchEvent(TouchType::UP, 7, touchPoint);
    ASSERT_EQ(pressedStates.size(), 2);
    EXPECT_FALSE(pressedStates[1]);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchUpInactiveBranches001
 * @tc.desc: A touch up while the bar is not pressed or is hovered does not report a released touch state.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchUpInactiveBranches001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 4, touchPoint);
    ASSERT_EQ(pressedStates.size(), 1);
    scrollBar->SetPressed(false);
    SendTouchEvent(TouchType::UP, 4, touchPoint);
    EXPECT_EQ(pressedStates.size(), 1);
    EXPECT_EQ(scrollBar->fingerId_, -1);

    SendTouchEvent(TouchType::DOWN, 4, touchPoint);
    ASSERT_EQ(pressedStates.size(), 2);
    scrollBar->SetHover(true);
    SendTouchEvent(TouchType::UP, 4, touchPoint);
    EXPECT_EQ(pressedStates.size(), 2);
    EXPECT_FALSE(scrollBar->IsPressed());
    EXPECT_EQ(scrollBar->fingerId_, -1);
    scrollBar->SetHover(false);

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchEventCancel001
 * @tc.desc: A CANCEL event releases the pressed scroll bar and reports the released touch state.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchEventCancel001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 3, touchPoint);
    SendTouchEvent(TouchType::CANCEL, 3, touchPoint);
    ASSERT_EQ(pressedStates.size(), 2);
    EXPECT_TRUE(pressedStates[0]);
    EXPECT_FALSE(pressedStates[1]);
    EXPECT_FALSE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: HandleTouchEventOtherTypeIgnored001
 * @tc.desc: A touch event whose type is neither down nor up/cancel leaves the pressed state unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, HandleTouchEventOtherTypeIgnored001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    std::vector<bool> pressedStates;
    scrollBar->SetTouchStateCallback([&pressedStates](bool isPressed) { pressedStates.push_back(isPressed); });
    auto touchPoint = GetScrollBarActiveCenter();

    SendTouchEvent(TouchType::DOWN, 3, touchPoint);
    ASSERT_EQ(pressedStates.size(), 1);
    SendTouchEvent(TouchType::MOVE, 3, touchPoint);
    EXPECT_EQ(pressedStates.size(), 1);
    EXPECT_TRUE(scrollBar->IsPressed());

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}

/**
 * @tc.name: InitTouchStateCallbackNullScrollBar001
 * @tc.desc: InitTouchStateCallback returns safely when the scroll bar instance has not been created.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarEventTestNg, InitTouchStateCallbackNullScrollBar001, TestSize.Level1)
{
    auto apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    PrepareTouchEventTest();

    auto scrollBar = pattern_->scrollBar_;
    pattern_->scrollBar_ = nullptr;
    pattern_->InitTouchStateCallback();
    EXPECT_EQ(pattern_->scrollBar_, nullptr);
    pattern_->scrollBar_ = scrollBar;

    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}
} // namespace OHOS::Ace::NG

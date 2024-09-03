/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/animation/mock_animation_manager.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/pattern/text/text_model_ng.h"

namespace OHOS::Ace::NG {
void ScrollTestNg::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto themeConstants = CreateThemeConstants(THEME_PATTERN_SCROLL_BAR);
    auto scrollBarTheme = ScrollBarTheme::Builder().Build(themeConstants);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(scrollBarTheme));
    scrollBarTheme->normalWidth_ = Dimension(NORMAL_WIDTH);
    scrollBarTheme->padding_ = Edge(0.0);
    scrollBarTheme->scrollBarMargin_ = Dimension(0.0);
    scrollBarTheme->touchWidth_ = Dimension(DEFAULT_TOUCH_WIDTH, DimensionUnit::VP);
    scrollBarTheme->activeWidth_ = Dimension(DEFAULT_ACTIVE_WIDTH, DimensionUnit::VP);
    scrollBarTheme->normalWidth_ = Dimension(DEFAULT_NORMAL_WIDTH, DimensionUnit::VP);
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockAnimationManager::Enable(true);
}

void ScrollTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

void ScrollTestNg::SetUp() {}

void ScrollTestNg::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    paintProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
    scrollBar_ = nullptr;
    ClearOldNodes(); // Each testCase will create new list at begin
    AceApplicationInfo::GetInstance().isRightToLeft_ = false;
    MockAnimationManager::GetInstance().Reset();
}

void ScrollTestNg::GetScroll()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    frameNode_ = AceType::DynamicCast<FrameNode>(element);
    pattern_ = frameNode_->GetPattern<ScrollPattern>();
    eventHub_ = frameNode_->GetEventHub<ScrollEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<ScrollLayoutProperty>();
    paintProperty_ = frameNode_->GetPaintProperty<ScrollablePaintProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<ScrollAccessibilityProperty>();
}

ScrollModelNG ScrollTestNg::CreateScroll()
{
    ResetElmtId();
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    ScrollModelNG model;
    model.Create();
    auto proxy = model.CreateScrollBarProxy();
    model.SetScrollBarProxy(proxy);
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    GetScroll();
    return model;
}

void ScrollTestNg::CreateSnapScroll(ScrollSnapAlign scrollSnapAlign, const Dimension& intervalSize,
    const std::vector<Dimension>& snapPaginations, const std::pair<bool, bool>& enableSnapToSide)
{
    ScrollModelNG model = CreateScroll();
    ViewAbstract::SetHeight(CalcLength(800.f));
    model.SetScrollSnap(scrollSnapAlign, intervalSize, snapPaginations, enableSnapToSide);
    CreateContent(3000.f);
    CreateDone();
}

void ScrollTestNg::CreateContent(float mainSize)
{
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    auto axis = layoutProperty_->GetAxis();
    if (axis.has_value() && axis.value() == Axis::HORIZONTAL) {
        RowModelNG rowModel;
        rowModel.Create(Dimension(0), nullptr, "");
    } else {
        ColumnModelNG colModel;
        colModel.Create(Dimension(0), nullptr, "");
    }
    SetSize(axis, CalcLength(FILL_LENGTH), CalcLength(mainSize));
}

void ScrollTestNg::CreateContentChild(int32_t childNumber)
{
    contentChildren_.clear();
    for (int32_t index = 0; index < childNumber; index++) {
        TextModelNG textModel;
        textModel.Create("text");
        SetSize(layoutProperty_->GetAxis(), CalcLength(FILL_LENGTH), CalcLength(ITEM_MAIN_SIZE));
        contentChildren_.emplace_back(ViewStackProcessor::GetInstance()->GetMainFrameNode());
        ViewStackProcessor::GetInstance()->Pop();
    }
}

void ScrollTestNg::Touch(TouchType touchType, Offset offset, SourceType sourceType)
{
    TouchLocationInfo locationInfo(1);
    locationInfo.SetTouchType(touchType);
    locationInfo.SetLocalLocation(offset);
    TouchEventInfo eventInfo("touch");
    eventInfo.SetSourceDevice(sourceType);
    eventInfo.AddTouchLocationInfo(std::move(locationInfo));
    auto touchEvent = pattern_->GetScrollBar()->touchEvent_->GetTouchEventCallback();
    touchEvent(eventInfo);
}

void ScrollTestNg::Mouse(Offset location, MouseButton mouseButton, MouseAction mouseAction)
{
    MouseInfo mouseInfo;
    mouseInfo.SetLocalLocation(location);
    mouseInfo.SetButton(mouseButton);
    mouseInfo.SetAction(mouseAction);
    auto mouseEventHub = frameNode_->GetOrCreateInputEventHub();
    RefPtr<InputEvent> inputEvent = mouseEventHub->mouseEventActuator_->inputEvents_.front();
    auto mouseEvent = inputEvent->GetOnMouseEventFunc();
    mouseEvent(mouseInfo);
}

void ScrollTestNg::Hover(bool isHover)
{
    auto hoverEventHub = frameNode_->GetOrCreateInputEventHub();
    RefPtr<InputEvent> inputEvent = hoverEventHub->hoverEventActuator_->inputEvents_.front();
    auto hoverEvent = inputEvent->GetOnHoverEventFunc();
    hoverEvent(isHover);
}

bool ScrollTestNg::OnScrollCallback(float offset, int32_t source)
{
    bool result = pattern_->OnScrollCallback(offset, source);
    FlushLayoutTask(frameNode_);
    return result;
}

void ScrollTestNg::ScrollToEdge(ScrollEdgeType scrollEdgeType)
{
    pattern_->ScrollToEdge(scrollEdgeType, false);
    FlushLayoutTask(frameNode_);
}

void ScrollTestNg::ScrollTo(float offset)
{
    pattern_->ScrollTo(offset);
    FlushLayoutTask(frameNode_);
}

AssertionResult ScrollTestNg::UpdateAndVerifyPosition(float delta, int32_t source, float expectOffset)
{
    pattern_->UpdateCurrentOffset(delta, source);
    FlushLayoutTask(frameNode_);
    return IsEqual(pattern_->GetTotalOffset(), expectOffset);
}

AssertionResult ScrollTestNg::ScrollToNode(const RefPtr<FrameNode>& focusFrameNode, float expectOffset)
{
    pattern_->ScrollToNode(focusFrameNode);
    FlushLayoutTask(frameNode_);
    return IsEqual(pattern_->GetTotalOffset(), expectOffset);
}

AssertionResult ScrollTestNg::VerifyTickPosition(float expectOffset)
{
    MockAnimationManager::GetInstance().Tick();
    FlushLayoutTask(frameNode_);
    return IsEqual(GetChildY(frameNode_, 0), expectOffset);
}

void ScrollTestNg::DragStart(GestureEvent& gesture)
{
    gesture.SetSourceTool(SourceTool::FINGER);
    gesture.SetInputEventType(InputEventType::TOUCH_SCREEN);
    gesture.SetGlobalPoint(Point());
    gesture.SetGlobalLocation(Offset());
    gesture.SetLocalLocation(Offset());
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchDown();
    scrollable->isDragging_ = true;
    scrollable->HandleDragStart(gesture);
}

void ScrollTestNg::DragUpdate(GestureEvent& gesture)
{
    float dragDelta = gesture.GetMainDelta();
    gesture.SetGlobalPoint(Point(0, dragDelta));
    gesture.SetGlobalLocation(Offset(0, dragDelta));
    gesture.SetLocalLocation(Offset(0, dragDelta));
    double velocity = dragDelta > 0 ? DRAG_VELOCITY : -DRAG_VELOCITY;
    gesture.SetMainVelocity(velocity);
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleDragUpdate(gesture);
    FlushLayoutTask(frameNode_);
}

void ScrollTestNg::DragEnd(GestureEvent& gesture)
{
    gesture.SetMainDelta(0);
    gesture.SetGlobalPoint(gesture.GetGlobalPoint());
    gesture.SetGlobalLocation(gesture.GetGlobalLocation());
    gesture.SetLocalLocation(gesture.GetLocalLocation());
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchUp();
    scrollable->HandleDragEnd(gesture);
    scrollable->isDragging_ = false;
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: HandleDrag001
 * @tc.desc: Handle drag
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDrag001, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step2. DragUpdate over scroll
     * @tc.expected: Scroll with drag
     */
    const float dragDelta = -10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);

    /**
     * @tc.steps: step3. DragEnd
     * @tc.expected: Scroll back with animation
     */
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);
}

/**
 * @tc.name: HandleDragOverScroll001
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::SPRING, will scroll back with animation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateDone();

    /**
     * @tc.steps: step1. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step2. DragUpdate over scroll
     * @tc.expected: Scroll with drag
     */
    const float dragDelta = 10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);

    /**
     * @tc.steps: step3. DragEnd
     * @tc.expected: Scroll back with animation
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);
    EXPECT_TRUE(VerifyTickPosition(dragDelta / TICK));
    EXPECT_TRUE(VerifyTickPosition(0));
}

/**
 * @tc.name: HandleDragOverScroll002
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::SPRING, will scroll back with animation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step1. Scroll to bottom
     */
    ScrollTo(CONTENT_MAIN_SIZE);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step2. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step3. DragUpdate over scroll
     * @tc.expected: Scroll with drag
     */
    const float dragDelta = -10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE + dragDelta);

    /**
     * @tc.steps: step4. DragEnd
     * @tc.expected: Scroll back with animation
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE + dragDelta);
    EXPECT_TRUE(VerifyTickPosition(-VERTICAL_SCROLLABLE_DISTANCE + dragDelta / TICK));
    EXPECT_TRUE(VerifyTickPosition(-VERTICAL_SCROLLABLE_DISTANCE));
}

/**
 * @tc.name: HandleDragOverScroll003
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::SPRING, will scroll back with animation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Has no scrollable distance, and AlwaysEnabled
     * @tc.expected: Still can drag over
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent(SCROLL_HEIGHT);
    CreateDone();
    EXPECT_TRUE(pattern_->GetAlwaysEnabled());
    EXPECT_EQ(pattern_->GetScrollableDistance(), 0);

    /**
     * @tc.steps: step2. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step3. DragUpdate over scroll
     * @tc.expected: Scroll with drag
     */
    const float dragDelta = 10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);

    /**
     * @tc.steps: step4. DragEnd
     * @tc.expected: Scroll back with animation
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);
    EXPECT_TRUE(VerifyTickPosition(dragDelta / TICK));
    EXPECT_TRUE(VerifyTickPosition(0));
}

/**
 * @tc.name: HandleDragOverScroll004
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::SPRING, will scroll back with animation
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Has no scrollable distance, and !AlwaysEnabled
     * @tc.expected: Still can drag over
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, false);
    CreateContent(SCROLL_HEIGHT);
    CreateDone();
    EXPECT_FALSE(pattern_->GetAlwaysEnabled());
    EXPECT_EQ(pattern_->GetScrollableDistance(), 0);

    /**
     * @tc.steps: step2. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step3. DragUpdate over scroll
     * @tc.expected: Scroll with drag
     */
    const float dragDelta = 10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);

    /**
     * @tc.steps: step4. DragEnd
     * @tc.expected: Scroll back with animation
     */
    MockAnimationManager::GetInstance().SetTicks(TICK);
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), dragDelta);
    EXPECT_TRUE(VerifyTickPosition(dragDelta / TICK));
    EXPECT_TRUE(VerifyTickPosition(0));
}

/**
 * @tc.name: HandleDragOverScroll005
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::FADE, can not drag over
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. EdgeEffect::FADE
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::FADE, true);
    CreateContent();
    CreateDone();

    /**
     * @tc.steps: step2. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step3. DragUpdate over scroll
     * @tc.expected: Can not drag over
     */
    const float dragDelta = 10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), 0);

    /**
     * @tc.steps: step4. DragEnd
     * @tc.expected: Can not drag over
     */
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), 0);
}

/**
 * @tc.name: HandleDragOverScroll006
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::FADE, can not drag over
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. EdgeEffect::FADE
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::FADE, true);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step2. Scroll to bottom
     */
    ScrollTo(CONTENT_MAIN_SIZE);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step3. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step4. DragUpdate over scroll
     * @tc.expected: Can not drag over
     */
    const float dragDelta = -10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step5. DragEnd
     * @tc.expected: Can not drag over
     */
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);
}

/**
 * @tc.name: HandleDragOverScroll007
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::NONE, can not drag over
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. EdgeEffect::NONE
     */
    CreateScroll();
    CreateContent();
    CreateDone();

    /**
     * @tc.steps: step2. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step3. DragUpdate over scroll
     * @tc.expected: Can not drag over
     */
    const float dragDelta = 10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), 0);

    /**
     * @tc.steps: step4. DragEnd
     * @tc.expected: Can not drag over
     */
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), 0);
}

/**
 * @tc.name: HandleDragOverScroll008
 * @tc.desc: Handle drag over scrollable distance in EdgeEffect::NONE, can not drag over
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, HandleDragOverScroll008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. EdgeEffect::NONE
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step2. Scroll to bottom
     */
    ScrollTo(CONTENT_MAIN_SIZE);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step3. DragStart
     */
    GestureEvent gesture;
    DragStart(gesture);

    /**
     * @tc.steps: step4. DragUpdate over scroll
     * @tc.expected: Can not drag over
     */
    const float dragDelta = -10.f;
    gesture.SetMainDelta(dragDelta);
    DragUpdate(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step5. DragEnd
     * @tc.expected: Can not drag over
     */
    gesture.SetMainVelocity(DRAG_VELOCITY);
    DragEnd(gesture);
    EXPECT_EQ(GetChildY(frameNode_, 0), -VERTICAL_SCROLLABLE_DISTANCE);
}

/**
 * @tc.name: AttrScrollable001
 * @tc.desc: Test attribute about scrollable,
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, AttrScrollable001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Text default value: VERTICAL
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. Text set value: HORIZONTAL
     */
    ClearOldNodes();
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step3. Text set value: NONE
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetAxis(Axis::NONE);
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));
}

/**
 * @tc.name: UpdateCurrentOffset001
 * @tc.desc: Test UpdateCurrentOffset that return
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, UpdateCurrentOffset001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When unScrollable
     * @tc.expected: currentOffset would not change
     */
    CreateScroll();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_UPDATE, 0.f));

    /**
     * @tc.steps: step2. When !HandleEdgeEffect and !IsOutOfBoundary
     * @tc.expected: currentOffset would not change
     */
    ClearOldNodes();
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_UPDATE, 0.f));

    /**
     * @tc.steps: step3. When !HandleEdgeEffect and IsOutOfBoundary
     * @tc.expected: currentOffset would not change
     */
    ClearOldNodes();
    CreateScroll();
    CreateContent();
    CreateDone();
    pattern_->currentOffset_ = 10.f;
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_UPDATE, 0.f));
}

/**
 * @tc.name: UpdateCurrentOffset002
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, UpdateCurrentOffset002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When Axis::VERTICAL
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_BAR, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ROTATE, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION_SPRING, ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. When Axis::HORIZONTAL
     */
    ClearOldNodes();
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_BAR, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ROTATE, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION_SPRING, ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step3. When Axis::HORIZONTAL and ROW_REVERSE
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();
    pattern_->SetDirection(FlexDirection::ROW_REVERSE);
    FlushLayoutTask(frameNode_, true);
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_BAR, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ROTATE, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_ANIMATION_SPRING, 0.f));

    /**
     * @tc.steps: step4. When EdgeEffect::SPRING, Test ValidateOffset
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateDone();
    EXPECT_FALSE(pattern_->IsRestrictBoundary());
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_JUMP, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_BAR, 0.f));
    EXPECT_TRUE(UpdateAndVerifyPosition(-ITEM_MAIN_SIZE, SCROLL_FROM_BAR_FLING, ITEM_MAIN_SIZE));
    EXPECT_TRUE(UpdateAndVerifyPosition(ITEM_MAIN_SIZE, SCROLL_FROM_ROTATE, 0.f));

    pattern_->currentOffset_ = -10.f;
    pattern_->UpdateScrollBarOffset();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetScrollBarOutBoundaryExtent(), 0);

    pattern_->currentOffset_ = -1000.f;
    pattern_->UpdateScrollBarOffset();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetScrollBarOutBoundaryExtent(), -pattern_->currentOffset_ - VERTICAL_SCROLLABLE_DISTANCE);

    pattern_->currentOffset_ = -100.f;
    pattern_->UpdateScrollBarOffset();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetScrollBarOutBoundaryExtent(), 0.0f);
}

/**
 * @tc.name: UpdateCurrentOffset003
 * @tc.desc: Test whether the isAnimateOverScroll_ can be set right.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, UpdateCurrentOffset003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll model with spring edgeEffect.
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateDone();
    pattern_->isAnimationStop_ = false;

    /**
     * @tc.steps: step2. Make animateCanOverScroll_ true, UpdateCurrentOffset to a position where over the boundary.
     * @tc.expected: pattern_->isAnimateOverScroll_ can be set to true.
     */
    pattern_->animateCanOverScroll_ = true;
    pattern_->isAnimateOverScroll_ = false;
    pattern_->UpdateCurrentOffset(100, SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->isAnimateOverScroll_, true);

    /**
     * @tc.steps: step3. Make animateCanOverScroll_ false, UpdateCurrentOffset to a position where over the boundary.
     * @tc.expected: pattern_->isAnimateOverScroll_ can't be set to true.
     */
    pattern_->animateCanOverScroll_ = false;
    pattern_->isAnimateOverScroll_ = false;
    pattern_->UpdateCurrentOffset(100, SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->isAnimateOverScroll_, false);
}

/**
 * @tc.name: UpdateCurrentOffset004
 * @tc.desc: Test return value of UpdateCurrentOffset.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, UpdateCurrentOffset004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll model with spring edgeEffect.
     */
    ScrollModelNG model = CreateScroll();
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateContent();
    CreateDone();
    /**
     * @tc.steps: step2. Make animateCanOverScroll_ true, UpdateCurrentOffset to a position where over the boundary.
     * @tc.expected: the return value of UpdateCurrentOffset is true.
     */
    pattern_->animateCanOverScroll_ = true;
    EXPECT_EQ(pattern_->UpdateCurrentOffset(100, SCROLL_FROM_ANIMATION_CONTROLLER), true);

    /**
     * @tc.steps: step3. Make animateCanOverScroll_ false, UpdateCurrentOffset to a position where over the boundary.
     * @tc.expected: the return value of UpdateCurrentOffset is false.
     */
    pattern_->animateCanOverScroll_ = false;
    EXPECT_EQ(pattern_->UpdateCurrentOffset(100, SCROLL_FROM_ANIMATION_CONTROLLER), false);
}

/**
 * @tc.name: Layout001
 * @tc.desc: Test Layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Layout001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::NONE);
    CreateContent();
    CreateDone();
    layoutProperty_->UpdateAlignment(Alignment::CENTER);
    FlushLayoutTask(frameNode_);
    auto col = frameNode_->GetChildAtIndex(0);
    auto colNode = AceType::DynamicCast<FrameNode>(col);
    auto colOffset = colNode->GetGeometryNode()->GetMarginFrameOffset();
    auto expectOffset = OffsetF(0, 0);
    EXPECT_EQ(colOffset, expectOffset) << "colOffset: " << colOffset.ToString()
                                       << " expectOffset: " << expectOffset.ToString();
}

/**
 * @tc.name: ScrollToNode001
 * @tc.desc: Test ScrollToNode
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollToNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. ScrollToNode content child
     * @tc.expected: currentOffset_ is correct
     */
    CreateScroll();
    CreateContent();
    CreateContentChild();
    CreateDone();
    EXPECT_TRUE(ScrollToNode(contentChildren_[3], 0));
    EXPECT_TRUE(ScrollToNode(contentChildren_[4], ITEM_MAIN_SIZE));
    EXPECT_TRUE(ScrollToNode(contentChildren_[5], ITEM_MAIN_SIZE * 2));
    EXPECT_TRUE(ScrollToNode(contentChildren_[3], ITEM_MAIN_SIZE * 2));
    EXPECT_TRUE(ScrollToNode(contentChildren_[0], 0));

    /**
     * @tc.steps: step2. ScrollToNode itSelf
     * @tc.expected: currentOffset_ is zero
     */
    EXPECT_TRUE(ScrollToNode(frameNode_, 0));
    EXPECT_TRUE(ScrollToNode(GetChildFrameNode(frameNode_, 0), 0));
}

/**
 * @tc.name: ScrollToNode002
 * @tc.desc: Test ScrollToNode in HORIZONTAL
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollToNode002, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateContentChild();
    CreateDone();
    EXPECT_TRUE(ScrollToNode(contentChildren_[3], 160.f));
    EXPECT_TRUE(ScrollToNode(contentChildren_[4], 260.f));
    EXPECT_TRUE(ScrollToNode(contentChildren_[5], 360.f));
    EXPECT_TRUE(ScrollToNode(contentChildren_[3], 300.f));
    EXPECT_TRUE(ScrollToNode(contentChildren_[0], 0));
}

/**
 * @tc.name: ScrollToNode003
 * @tc.desc: Test ScrollToNode when animate is running, can not scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollToNode003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Play animate
     * @tc.expected: ScrollToNode can not scroll
     */
    CreateScroll();
    CreateContent();
    CreateContentChild();
    CreateDone();
    pattern_->isAnimationStop_ = false;
    EXPECT_FALSE(pattern_->AnimateStoped());
    EXPECT_TRUE(ScrollToNode(contentChildren_[8], 0.f));
}

/**
 * @tc.name: Pattern003
 * @tc.desc: Test HandleScrollBarOutBoundary
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern003, TestSize.Level1)
{
    CreateScroll();
    CreateContent();
    CreateDone();

    /**
     * @tc.steps: step1. When scrollBar is not OFF
     * @tc.expected: outBoundary_ would be set
     */
    pattern_->HandleScrollBarOutBoundary(100.f);
    auto scrollBar = pattern_->GetScrollBar();
    EXPECT_EQ(scrollBar->outBoundary_, 100.f);

    /**
     * @tc.steps: step1. When scrollBar is OFF
     * @tc.expected: outBoundary_ would not be set
     */
    scrollBar->displayMode_ = DisplayMode::OFF;
    pattern_->HandleScrollBarOutBoundary(200.f);
    EXPECT_EQ(scrollBar->outBoundary_, 100.f);
}

/**
 * @tc.name: Test001
 * @tc.desc: Test GetOverScrollOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Test001, TestSize.Level1)
{
    CreateScroll();
    CreateContent(600.f);
    CreateDone();

    OverScrollOffset offset = pattern_->GetOverScrollOffset(ITEM_MAIN_SIZE);
    OverScrollOffset expectOffset = { ITEM_MAIN_SIZE, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_MAIN_SIZE);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->currentOffset_ = -ITEM_MAIN_SIZE;
    offset = pattern_->GetOverScrollOffset(ITEM_MAIN_SIZE * 2);
    expectOffset = { ITEM_MAIN_SIZE, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_MAIN_SIZE * 2);
    expectOffset = { 0, -ITEM_MAIN_SIZE };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->currentOffset_ = -ITEM_MAIN_SIZE * 2;
    offset = pattern_->GetOverScrollOffset(ITEM_MAIN_SIZE);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_MAIN_SIZE);
    expectOffset = { 0, -ITEM_MAIN_SIZE };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->currentOffset_ = ITEM_MAIN_SIZE;
    offset = pattern_->GetOverScrollOffset(ITEM_MAIN_SIZE);
    expectOffset = { ITEM_MAIN_SIZE, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_MAIN_SIZE * 2);
    expectOffset = { -ITEM_MAIN_SIZE, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->currentOffset_ = -ITEM_MAIN_SIZE * 3;
    offset = pattern_->GetOverScrollOffset(ITEM_MAIN_SIZE * 2);
    expectOffset = { 0, ITEM_MAIN_SIZE };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_MAIN_SIZE);
    expectOffset = { 0, -ITEM_MAIN_SIZE };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
}

/**
 * @tc.name: ScrollTest006
 * @tc.desc: When setting a fixed length and width, verify the related functions in the scroll pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest006, TestSize.Level1)
{
    double touchPosX = 150.0;
    double touchPosY = 500.0;
    float offset = 10.0f;
    float velocity = 1200.0f;
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    model.SetDisplayMode(static_cast<int>(DisplayMode::OFF));
    auto scrollProxy = model.CreateScrollBarProxy();
    model.SetScrollBarProxy(scrollProxy);
    CreateContent();
    CreateDone();

    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);

    GestureEvent info;
    info.SetMainVelocity(velocity);
    info.SetGlobalPoint(Point(touchPosX, touchPosY));
    info.SetGlobalLocation(Offset(touchPosX, touchPosY));
    info.SetSourceTool(SourceTool::FINGER);
    info.SetInputEventType(InputEventType::TOUCH_SCREEN);
    pattern_->scrollableEvent_->GetScrollable()->HandleDragStart(info);

    // Update 1 finger position.
    info.SetGlobalLocation(Offset(touchPosX, touchPosY + offset));
    info.SetGlobalPoint(Point(touchPosX, touchPosY + offset));
    info.SetMainVelocity(velocity);
    info.SetMainDelta(offset);
    pattern_->scrollableEvent_->GetScrollable()->HandleDragUpdate(info);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTotalOffset(), 10.0f);

    // Update 2 finger position.
    info.SetGlobalLocation(Offset(touchPosX, touchPosY + offset));
    info.SetGlobalPoint(Point(touchPosX, touchPosY + offset));
    info.SetMainVelocity(velocity);
    info.SetMainDelta(offset);
    pattern_->scrollableEvent_->GetScrollable()->HandleDragUpdate(info);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTotalOffset(), 20.0f);

    // Lift finger and end List sliding.
    info.SetMainVelocity(0.0);
    info.SetMainDelta(0.0);
    pattern_->scrollableEvent_->GetScrollable()->HandleDragEnd(info);
    pattern_->scrollableEvent_->GetScrollable()->isDragging_ = false;
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTotalOffset(), 20.0f);
}

/**
 * @tc.name: ScrollSetFrictionTest001
 * @tc.desc: Test SetFriction
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollSetFrictionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set friction less than 0
     * @tc.expected: should be more than 0.0,if out of range,should be default value.
     */
    auto pipelineContext = PipelineContext::GetCurrentContext();
    pipelineContext->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    double friction = -1;
    ScrollModelNG model = CreateScroll();
    model.SetFriction(friction);
    CreateDone();
    EXPECT_DOUBLE_EQ(pattern_->GetFriction(), DEFAULT_FRICTION);

    /**
     * @tc.steps: step1. set friction more than 0
     * @tc.expected: friction should be more than 0.0,if out of range,should be default value.
     */
    friction = 10;
    ClearOldNodes();
    model = CreateScroll();
    model.SetFriction(friction);
    CreateDone();
    EXPECT_DOUBLE_EQ(pattern_->GetFriction(), friction);
}

/**
 * @tc.name: Snap001
 * @tc.desc: Test snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Snap001, TestSize.Level1)
{
    Dimension intervalSize = Dimension(10.f);
    std::vector<Dimension> snapPaginations = {
        Dimension(10.f),
        Dimension(20.f),
        Dimension(30.f),
    };

    // snapOffsets_: { 0.f, -10.f, -20.f, -30.f, -2200.f }
    std::pair<bool, bool> enableSnapToSide = { false, false };
    CreateSnapScroll(ScrollSnapAlign::START, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-20.f).has_value());
    pattern_->currentOffset_ = -20.f;
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(0.f).has_value());

    pattern_->currentOffset_ = -10.f;
    EXPECT_TRUE(pattern_->NeedScrollSnapToSide(-10.f));
    EXPECT_FALSE(pattern_->NeedScrollSnapToSide(10.f));
    pattern_->currentOffset_ = -20.f;
    EXPECT_FALSE(pattern_->NeedScrollSnapToSide(0.f));

    enableSnapToSide = { true, false };
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::START, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    pattern_->currentOffset_ = 20.f;
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());

    pattern_->currentOffset_ = -30.f;
    EXPECT_TRUE(pattern_->NeedScrollSnapToSide(10.f));
    EXPECT_FALSE(pattern_->NeedScrollSnapToSide(-10.f));
    pattern_->currentOffset_ = -20.f;
    EXPECT_FALSE(pattern_->NeedScrollSnapToSide(0.f));

    // snapOffsets_: { 0.f, -10.f, -20.f, -30.f, -40.f, ... , -180.f, -190.f, -200.f }
    snapPaginations = {};
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::START, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(10.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-(2200.f + 10.f)).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-2.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-158.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-10.f).has_value());
}

/**
 * @tc.name: Snap002
 * @tc.desc: Test snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Snap002, TestSize.Level1)
{
    const float scrollHeight = 800.f;
    Dimension intervalSize = Dimension(10.f / scrollHeight, DimensionUnit::PERCENT);
    std::vector<Dimension> snapPaginations = {
        Dimension(0.f, DimensionUnit::PERCENT),
        Dimension(10.f / scrollHeight, DimensionUnit::PERCENT),
        Dimension(20.f / scrollHeight, DimensionUnit::PERCENT),
        Dimension(30.f / scrollHeight, DimensionUnit::PERCENT),
        Dimension((200.f + 10.f) / scrollHeight, DimensionUnit::PERCENT),
    };

    // snapOffsets_: { 0.f, -1205.f, -2200.f }
    std::pair<bool, bool> enableSnapToSide = { false, false };
    CreateSnapScroll(ScrollSnapAlign::CENTER, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-20.f).has_value());
    pattern_->currentOffset_ = -20.f;
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());

    enableSnapToSide = { true, false };
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::CENTER, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    pattern_->currentOffset_ = -1200.f;
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(0.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(0.f).value(), -5.f);

    // snapOffsets_: { 0.f, -5.f, -15.f, -25.f, -35.f, ... , -2185.f, -2195.f }
    snapPaginations = {};
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::CENTER, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(10.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-(2200.f + 10.f)).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-2.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-158.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-10.f).has_value());
}

/**
 * @tc.name: Snap003
 * @tc.desc: Test snap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Snap003, TestSize.Level1)
{
    Dimension intervalSize = Dimension(10.f);
    std::vector<Dimension> snapPaginations = {
        Dimension(10.f),
        Dimension(20.f),
        Dimension(30.f),
    };

    // snapOffsets_: { 0.f, -2200.f }
    std::pair<bool, bool> enableSnapToSide = { false, false };
    CreateSnapScroll(ScrollSnapAlign::END, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-20.f).has_value());
    pattern_->currentOffset_ = -20.f;
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());

    enableSnapToSide = { true, false };
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::END, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    pattern_->currentOffset_ = 20.f;
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-40.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(0.f).has_value());

    // snapOffsets_: { 0.f, -10.f, -20.f, -30.f, -40.f, ... , -180.f, -190.f, -200.f }
    snapPaginations = {};
    ClearOldNodes();
    CreateSnapScroll(ScrollSnapAlign::END, intervalSize, snapPaginations, enableSnapToSide);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(10.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-(2200.f + 10.f)).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-2.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-158.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-10.f).has_value());
}

/**
 * @tc.name: CaleSnapOffsetsByPaginations001
 * @tc.desc: Test CaleSnapOffsetsByPaginations where the page size is lesser than the scroll size.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, CaleSnapOffsetsByPaginations001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init snapPaginations.
     */
    Dimension intervalSize = Dimension(0.f);
    std::vector<Dimension> snapPaginations = {
        Dimension(400.f),
        Dimension(800.f),
        Dimension(1400.f),
        Dimension(1600.f),
        Dimension(2200.f),
    };

    /**
     * @tc.steps: step2. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::START.
     * @tc.expected: SnapOffsets is correct.
     */
    // snapPaginations: { 400.f, 800.f, 1400.f, 1600.f, 2200.f }
    std::pair<bool, bool> enableSnapToSide = { true, true };
    CreateSnapScroll(ScrollSnapAlign::START, intervalSize, snapPaginations, enableSnapToSide);
    pattern_->CaleSnapOffsets();
    auto snapOffsets = pattern_->GetSnapOffsets();
    std::vector<float> testSnapOffsets = { 0.f, -400.f, -800.f, -1400.f, -1600.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-200.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-200.f).value(), -400.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-2000.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-2000.f).value(), -2200.f);

    /**
     * @tc.steps: step3. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-200.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-2000.f).has_value());

    /**
     * @tc.steps: step4. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::CENTER.
     * @tc.expected: SnapOffsets is correct.
     */
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->SetEnableSnapToSide({ true, true });
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ScrollLayoutProperty, ScrollSnapAlign, ScrollSnapAlign::CENTER, host);
    pattern_->CaleSnapOffsets();
    snapOffsets = pattern_->GetSnapOffsets();
    testSnapOffsets = { 0.f, -200.f, -700.f, -1100.f, -1500.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-99.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-99.f).value(), 0.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-1700.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-1700.f).value(), -1500.f);

    /**
     * @tc.steps: step5. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-99.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-1700.f).has_value());

    /**
     * @tc.steps: step6. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::END.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ true, true });
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ScrollLayoutProperty, ScrollSnapAlign, ScrollSnapAlign::END, host);
    pattern_->CaleSnapOffsets();
    snapOffsets = pattern_->GetSnapOffsets();
    testSnapOffsets = { 0.f, -600.f, -800.f, -1400.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-300.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-300.f).value(), -600.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-1799.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-1799.f).value(), -1400.f);

    /**
     * @tc.steps: step7. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-300.f).has_value());
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-1799.f).has_value());
}

/**
 * @tc.name: CaleSnapOffsetsByPaginations002
 * @tc.desc: Test CaleSnapOffsetsByPaginations where the page size is greater than the scroll size.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, CaleSnapOffsetsByPaginations002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init snapPaginations.
     */
    Dimension intervalSize = Dimension(0.f);
    std::vector<Dimension> snapPaginations = {
        Dimension(1000.f),
        Dimension(1200.f),
        Dimension(2000.f),
    };

    /**
     * @tc.steps: step2. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::START.
     * @tc.expected: SnapOffsets is correct.
     */
    // snapPaginations: { 1000.f, 1200.f, 2000.f }
    std::pair<bool, bool> enableSnapToSide = { true, true };
    CreateSnapScroll(ScrollSnapAlign::START, intervalSize, snapPaginations, enableSnapToSide);
    pattern_->CaleSnapOffsets();
    auto snapOffsets = pattern_->GetSnapOffsets();
    std::vector<float> testSnapOffsets = { 0.f, -1000.f, -1200.f, -2000.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-500.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-500.f).value(), -1000.f);
    pattern_->currentOffset_ = -2000.f;
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-100.f).value(), -200.f);
    // The midpoint of -1200.f and -2000.f is -1600.f
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(400.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(400.f).value(), 0.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(401.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(401.f).value(), 800.f);

    /**
     * @tc.steps: step3. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    pattern_->currentOffset_ = -1900.f;
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-100.f).value(), -100.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-101.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(900.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(900.f).value(), 900.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(901.f).has_value());

    /**
     * @tc.steps: step4. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::CENTER.
     * @tc.expected: SnapOffsets is correct.
     */
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->currentOffset_ = 0.f;
    pattern_->SetEnableSnapToSide({ true, true });
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ScrollLayoutProperty, ScrollSnapAlign, ScrollSnapAlign::CENTER, host);
    pattern_->CaleSnapOffsets();
    snapOffsets = pattern_->GetSnapOffsets();
    testSnapOffsets = { 0.f, -100.f, -700.f, -1200.f, -2100.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-49.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-49.f).value(), 0.f);
    pattern_->currentOffset_ = -2000.f;
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-100.f).value(), -100.f);
    // The midpoint of -1200.f and -2100.f is -1650.f
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(350.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(350.f).value(), -100.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(351.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(351.f).value(), 800.f);

    /**
     * @tc.steps: step5. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-100.f).value(), -100.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-101.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(1900.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(1900.f).value(), 1900.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(1901.f).has_value());

    /**
     * @tc.steps: step6. EnableSnapToSide is { true, true } and SnapAlign is ScrollSnapAlign::END.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->currentOffset_ = 0.f;
    pattern_->SetEnableSnapToSide({ true, true });
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ScrollLayoutProperty, ScrollSnapAlign, ScrollSnapAlign::END, host);
    pattern_->CaleSnapOffsets();
    snapOffsets = pattern_->GetSnapOffsets();
    testSnapOffsets = { 0.f, -200.f, -400.f, -1200.f, -2200.f };
    EXPECT_TRUE(snapOffsets == testSnapOffsets);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-100.f).value(), -200.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-1200.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-1200.f).value(), -1200.f);
    pattern_->currentOffset_ = -300.f;
    // The midpoint of 0.f and -200.f is -100.f
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(200.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(200.f).value(), 100.f);
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(201.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(201.f).value(), 300.f);

    /**
     * @tc.steps: step7. EnableSnapToSide is { false, false }.
     * @tc.expected: SnapOffsets is correct.
     */
    pattern_->SetEnableSnapToSide({ false, false });
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(100.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(100.f).value(), 100.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(101.f).has_value());
    EXPECT_TRUE(pattern_->CalcPredictSnapOffset(-900.f).has_value());
    EXPECT_DOUBLE_EQ(pattern_->CalcPredictSnapOffset(-900.f).value(), -900.f);
    EXPECT_FALSE(pattern_->CalcPredictSnapOffset(-901.f).has_value());
}
/**
 * @tc.name: Distributed001
 * @tc.desc: Test the distributed capability of Scroll.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Distributed001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize Scroll node
     */
    CreateScroll();
    CreateContent();
    CreateDone();

    // need dpi to be 1
    /**
     * @tc.steps: step2. get pattern .
     * @tc.expected: function ProvideRestoreInfo is called.
     */
    pattern_->currentOffset_ = 1.0f;
    std::string ret = pattern_->ProvideRestoreInfo();

    /**
     * @tc.steps: step3. function OnRestoreInfo is called.
     * @tc.expected: Passing JSON format.
     */
    pattern_->OnRestoreInfo(ret);
    EXPECT_DOUBLE_EQ(pattern_->currentOffset_, 1.0f);
}

/**
 * @tc.name: ScrollGetItemRect001
 * @tc.desc: Test Scroll GetItemRect function.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollGetItemRect001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize Scroll.
     */
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();

    /**
     * @tc.steps: step2. Get invalid ScrollItem Rect.
     * @tc.expected: Return 0 when input invalid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(-1), Rect()));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(1), Rect()));

    /**
     * @tc.steps: step3. Get valid ScrollItem Rect.
     * @tc.expected: Return actual Rect when input valid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, 0, CONTENT_MAIN_SIZE, FILL_LENGTH.Value() * SCROLL_HEIGHT)));
}

/**
 * @tc.name: ScrollWidth001
 * @tc.desc: Test the usability of scroll width property and its get and set function.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollWidth001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. verify the scroll width property
     * of scroll layout property.
     * @tc.expected: Default value is ought to be false.
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    float scrollWidth = 150.0f;
    EXPECT_FALSE(layoutProperty_->GetScrollWidth().has_value());
    layoutProperty_->UpdateScrollWidth(scrollWidth);
    EXPECT_EQ(layoutProperty_->GetScrollWidth().value(), scrollWidth);
}

/**
 * @tc.name: SelectScroll001
 * @tc.desc: Test the flags of select scroll that determines whether it belong to or be modified by a select
 * and their get and set functions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, SelectScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. verify the default value of the flags
     * which inform whether the scroll belongs to or is modified by a select.
     * @tc.expected: Default value is ought to be false.
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_FALSE(pattern_->IsWidthModifiedBySelect());
    EXPECT_FALSE(pattern_->IsSelectScroll());
    /**
     * @tc.steps: step2. Set both flags to be true and verify the usability of their get and set functions in
     * select pattern.
     * @tc.expected: After setting the value should be true.
     */
    pattern_->SetIsWidthModifiedBySelect(true);
    pattern_->SetIsSelectScroll(true);
    EXPECT_TRUE(pattern_->IsWidthModifiedBySelect());
    EXPECT_TRUE(pattern_->IsSelectScroll());
}

/**
 * @tc.name: Measure002
 * @tc.desc: Test ScrollLayoutAlgorithm Measure when the scroll belongs to a select.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Measure002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll model and set the width, height, axis of the scroll, create the content of
     * the scroll and get its instance.
     * @tc.expected: Objects are created successfully.
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    RefPtr<LayoutWrapperNode> layoutWrapper = frameNode_->CreateLayoutWrapper(false, false);
    pattern_->SetIsSelectScroll(true);
    FlushLayoutTask(frameNode_);
    layoutWrapper->MountToHostOnMainThread();

    RefPtr<GridColumnInfo> columnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::MENU);
    columnInfo->GetParent()->BuildColumnWidth();
    auto defaultWidth = static_cast<float>(columnInfo->GetWidth(2));
    auto scrollSize = frameNode_->GetGeometryNode()->GetFrameSize();
    auto expectSize = SizeF(defaultWidth, 600.f);
    EXPECT_NE(scrollSize, expectSize) << "scrollSize: " << scrollSize.ToString()
                                      << " expectSize: " << expectSize.ToString();
}

/**
 * @tc.name: Measure003
 * @tc.desc: Test ScrollLayoutAlgorithm Measure.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Measure003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll without children
     */
    CreateScroll();
    CreateDone();
    auto scrollSize = frameNode_->GetGeometryNode()->GetFrameSize();
    auto expectSize = SizeF(SCROLL_WIDTH, SCROLL_HEIGHT);
    EXPECT_TRUE(IsEqual(scrollSize, expectSize));

    /**
     * @tc.steps: step1. set idealSize
     * @tc.expected: The frameSize would be idealSize
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(300.f));
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(500.f));
    FlushLayoutTask(frameNode_);

    scrollSize = frameNode_->GetGeometryNode()->GetFrameSize();
    expectSize = SizeF(300.f, 500.f);
    EXPECT_TRUE(IsEqual(scrollSize, expectSize));
}

/**
 * @tc.name: SelectScroll002
 * @tc.desc: Test select scroll default width.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, SelectScroll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the width of select scroll without setting it, this case is meant to test the correctness
     * of its default value.
     * @tc.expected: Default width of select scroll should be 0.0.
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    auto ScrollWidth = pattern_->GetSelectScrollWidth();
    ASSERT_NE(ScrollWidth, 0.0);
}

/**
 * @tc.name: EnablePaging001
 * @tc.desc: Test enablePaging
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, EnablePaging001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll and initialize related properties.
     */
    ScrollModelNG model = CreateScroll();
    model.SetEnablePaging(true);
    CreateContent();
    CreateDone();
    auto viewPortLength = pattern_->GetMainContentSize();
    pattern_->scrollableDistance_ = viewPortLength * 10;
    pattern_->currentOffset_ = -viewPortLength * 5 - 10.0f;
    SizeF viewPortExtent(SCROLL_WIDTH, viewPortLength * 11);
    pattern_->viewPortExtent_ = viewPortExtent;
    pattern_->SetIntervalSize(Dimension(static_cast<double>(viewPortLength)));
    pattern_->CaleSnapOffsets();

    /**
     * @tc.steps: step2. dragDistance and dragSpeed less than threshold
     * @tc.expected: predictSnapOffset.value() less than 0
     */
    auto dragDistance = viewPortLength * 0.5 - 1;
    auto dragSpeed = SCROLL_PAGING_SPEED_THRESHOLD - 1;
    auto predictSnapOffset = pattern_->CalcPredictSnapOffset(0.f, dragDistance, dragSpeed);
    EXPECT_TRUE(predictSnapOffset.has_value());
    EXPECT_LT(predictSnapOffset.value(), 0);

    /**
     * @tc.steps: step3. dragDistance and dragSpeed larger than threshold
     * @tc.expected: the absolute value of predictSnapOffset.value() less than viewPortLength
     */
    dragDistance = viewPortLength * 0.5 * 5;
    dragSpeed = SCROLL_PAGING_SPEED_THRESHOLD * 5;
    predictSnapOffset = pattern_->CalcPredictSnapOffset(0.f, dragDistance, dragSpeed);
    EXPECT_TRUE(predictSnapOffset.has_value());
    EXPECT_LT(abs(predictSnapOffset.value()), viewPortLength);
    EXPECT_GT(predictSnapOffset.value(), 0);

    /**
     * @tc.steps: step4. dragDistance equals threshold and dragSpeed less than threshold
     * @tc.expected: the absolute value of predictSnapOffset.value() less than viewPortLength
     */
    dragDistance = viewPortLength * 0.5;
    dragSpeed = SCROLL_PAGING_SPEED_THRESHOLD - 1;
    predictSnapOffset = pattern_->CalcPredictSnapOffset(0.f, dragDistance, dragSpeed);
    EXPECT_TRUE(predictSnapOffset.has_value());
    EXPECT_LT(abs(predictSnapOffset.value()), viewPortLength);
    EXPECT_GT(predictSnapOffset.value(), 0);

    /**
     * @tc.steps: step5. dragDistance less than threshold and dragSpeed equals threshold
     * @tc.expected: the absolute value of predictSnapOffset.value() less than viewPortLength
     */
    dragDistance = viewPortLength * 0.5 - 1;
    dragSpeed = SCROLL_PAGING_SPEED_THRESHOLD;
    predictSnapOffset = pattern_->CalcPredictSnapOffset(0.f, dragDistance, dragSpeed);
    EXPECT_TRUE(predictSnapOffset.has_value());
    EXPECT_LT(abs(predictSnapOffset.value()), viewPortLength);
    EXPECT_GT(predictSnapOffset.value(), 0);
}

/**
 * @tc.name: EnablePaging002
 * @tc.desc: Test enablePaging
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, EnablePaging002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll and set enablePaging.
     * @tc.expected: the value of GetEnablePaging() is VALID
     */
    ScrollModelNG model = CreateScroll();
    model.SetEnablePaging(true);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetEnablePaging(), ScrollPagingStatus::VALID);
    EXPECT_EQ(pattern_->IsEnablePagingValid(), true);

    /**
     * @tc.steps: step2. Create scroll, first set enablePaging and than set snap.
     * @tc.expected: the value of IsEnablePagingValid() is false
     */
    Dimension intervalSize = Dimension(10.f);
    std::vector<Dimension> snapPaginations = {
        Dimension(10.f),
        Dimension(20.f),
        Dimension(30.f),
    };
    std::pair<bool, bool> enableSnapToSide = { false, false };
    auto scrollSnapAlign = ScrollSnapAlign::START;
    ClearOldNodes();
    model = CreateScroll();
    model.SetEnablePaging(true);
    model.SetScrollSnap(scrollSnapAlign, intervalSize, snapPaginations, enableSnapToSide);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->IsEnablePagingValid(), false);

    /**
     * @tc.steps: step3. Create scroll, first set snap and than set enablePaging.
     * @tc.expected: the value of IsEnablePagingValid() is false
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetScrollSnap(scrollSnapAlign, intervalSize, snapPaginations, enableSnapToSide);
    model.SetEnablePaging(true);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->IsEnablePagingValid(), false);

    /**
     * @tc.steps: step4. Create scroll, set enablePaging true and than set enablePaging false.
     * @tc.expected: the value of GetEnablePaging() is INVALID
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetEnablePaging(true);
    model.SetEnablePaging(false);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetEnablePaging(), ScrollPagingStatus::INVALID);
    EXPECT_EQ(pattern_->IsEnablePagingValid(), false);

    /**
     * @tc.steps: step5. Create scroll, set enablePaging false and than set enablePaging true.
     * @tc.expected: the value of GetEnablePaging() is VALID
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetEnablePaging(false);
    model.SetEnablePaging(true);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetEnablePaging(), ScrollPagingStatus::VALID);
    EXPECT_EQ(pattern_->IsEnablePagingValid(), true);
}

/**
 * @tc.name: InitialOffset001
 * @tc.desc: Test initialOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, InitialOffset001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll.
     * @tc.expected: the value of currentOffset_ is 0
     */
    CreateScroll();
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, 0.f);

    /**
     * @tc.steps: step2. Create scroll and set initialOffset ITEM_MAIN_SIZE.
     * @tc.expected: the value of currentOffset_ is -ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    ScrollModelNG model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(ITEM_MAIN_SIZE)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -ITEM_MAIN_SIZE);

    /**
     * @tc.steps: step3. Create scroll , set axis HORIZONTAL and set initialOffset ITEM_MAIN_SIZE.
     * @tc.expected: the value of currentOffset_ is -ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(ITEM_MAIN_SIZE), CalcDimension(0.f)));
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -ITEM_MAIN_SIZE);

    /**
     * @tc.steps: step4. Create scroll , set initialOffset 10%.
     * @tc.expected: the value of currentOffset_ is -ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    model = CreateScroll();
    auto offset = Dimension(0.1, DimensionUnit::PERCENT);
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(offset)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -SCROLL_HEIGHT * 0.1f);

    /**
     * @tc.steps: step5. Create scroll , set axis HORIZONTAL and set initialOffset 10%.
     * @tc.expected: the value of currentOffset_ is -ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(offset), CalcDimension(0.f)));
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -SCROLL_WIDTH * 0.1f);
}

/**
 * @tc.name: InitialOffset002
 * @tc.desc: Test initialOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, InitialOffset002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll and set initialOffset 2*ITEM_MAIN_SIZE.
     * @tc.expected: the value of currentOffset_ is -2*ITEM_MAIN_SIZE
     */
    ScrollModelNG model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(2 * ITEM_MAIN_SIZE)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -2 * ITEM_MAIN_SIZE);

    /**
     * @tc.steps: step2. Create scroll and set initialOffset 3*ITEM_MAIN_SIZE.
     * @tc.expected: the value of currentOffset_ is -2*ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(100 * ITEM_MAIN_SIZE)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -VERTICAL_SCROLLABLE_DISTANCE);

    /**
     * @tc.steps: step3. Create scroll and set initialOffset -ITEM_MAIN_SIZE.
     * @tc.expected: the value of currentOffset_ is 0
     */
    ClearOldNodes();
    model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(-ITEM_MAIN_SIZE)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, 0.f);

    /**
     * @tc.steps: step4. Create scroll , set initialOffset 100%.
     * @tc.expected: the value of currentOffset_ is -2*ITEM_MAIN_SIZE
     */
    ClearOldNodes();
    model = CreateScroll();
    auto offset = Dimension(100, DimensionUnit::PERCENT);
    model.SetInitialOffset(OffsetT(CalcDimension(0.f), CalcDimension(offset)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->currentOffset_, -VERTICAL_SCROLLABLE_DISTANCE);
}

/**
 * @tc.name: Model001
 * @tc.desc: Test scroll model
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Model001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    EXPECT_NE(model.GetOrCreateController(), nullptr);
    pattern_->positionController_ = nullptr;
    EXPECT_NE(model.GetOrCreateController(), nullptr);
    EXPECT_NE(model.GetOrCreateController(AceType::RawPtr(frameNode_)), nullptr);
    pattern_->positionController_ = nullptr;
    EXPECT_NE(model.GetOrCreateController(AceType::RawPtr(frameNode_)), nullptr);
    EXPECT_EQ(model.GetAxis(AceType::RawPtr(frameNode_)), 0);
    model.SetAxis(Axis::VERTICAL);
    EXPECT_EQ(model.GetAxis(AceType::RawPtr(frameNode_)), 0);
    EXPECT_EQ(model.GetScrollEnabled(AceType::RawPtr(frameNode_)), 1);
    model.SetScrollEnabled(AceType::RawPtr(frameNode_), false);
    EXPECT_EQ(model.GetScrollEnabled(AceType::RawPtr(frameNode_)), 0);
    model.SetEnablePaging(AceType::RawPtr(frameNode_), true);
    EXPECT_EQ(pattern_->GetEnablePaging(), ScrollPagingStatus::VALID);
    EXPECT_EQ(pattern_->IsEnablePagingValid(), true);
    model.SetEnablePaging(AceType::RawPtr(frameNode_), false);
    EXPECT_EQ(pattern_->GetEnablePaging(), ScrollPagingStatus::INVALID);
    EXPECT_EQ(pattern_->IsEnablePagingValid(), false);
    CreateContent();
    CreateDone();

    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_TOP);
    ScrollTo(ITEM_MAIN_SIZE);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_NONE);
    ScrollTo(CONTENT_MAIN_SIZE);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_BOTTOM);

    ScrollTo(0.f);
    pattern_->SetAxis(Axis::HORIZONTAL);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_LEFT);
    ScrollTo(ITEM_MAIN_SIZE);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_NONE);
    ScrollTo(CONTENT_MAIN_SIZE);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_RIGHT);

    pattern_->SetAxis(Axis::NONE);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(model.GetOnScrollEdge(AceType::RawPtr(frameNode_)), ScrollEdgeType::SCROLL_NONE);
}

/**
 * @tc.name: Alignment001
 * @tc.desc: Test UpdateScrollAlignment in RTL Layout, content size less than scroll size
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Alignment001, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateScroll();
    CreateContent(100.f); // Set content height less than scroll height
    CreateDone();

    /**
     * @tc.steps: step1. Set content width less than scroll width
     */
    float contentWidth = SCROLL_WIDTH / 2;
    auto contentNode = GetChildFrameNode(frameNode_, 0);
    ViewAbstract::SetWidth(AceType::RawPtr(contentNode), CalcLength(contentWidth));
    FlushLayoutTask(frameNode_);
    float centerPosition = (SCROLL_HEIGHT - ITEM_MAIN_SIZE) / 2;
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(0.f, centerPosition)));

    layoutProperty_->UpdateAlignment(Alignment::TOP_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(contentWidth, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::TOP_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF()));

    layoutProperty_->UpdateAlignment(Alignment::BOTTOM_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(contentWidth, SCROLL_HEIGHT - ITEM_MAIN_SIZE)));

    layoutProperty_->UpdateAlignment(Alignment::BOTTOM_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(0.f, SCROLL_HEIGHT - ITEM_MAIN_SIZE)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(0.f, centerPosition)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(contentWidth, centerPosition)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(contentWidth / 2, centerPosition)));
}

/**
 * @tc.name: Alignment002
 * @tc.desc: Test UpdateScrollAlignment in RTL Layout, content size greater than scroll size
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Alignment002, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateScroll();
    CreateContent(); // Set content height less than scroll height
    CreateDone();

    /**
     * @tc.steps: step1. Set content width greater than scroll width
     */
    float contentWidth = SCROLL_WIDTH * 2;
    auto contentNode = GetChildFrameNode(frameNode_, 0);
    ViewAbstract::SetWidth(AceType::RawPtr(contentNode), CalcLength(contentWidth));
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF()));

    layoutProperty_->UpdateAlignment(Alignment::TOP_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::TOP_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::BOTTOM_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::BOTTOM_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER_RIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER_LEFT);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));

    layoutProperty_->UpdateAlignment(Alignment::CENTER);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(-SCROLL_WIDTH, 0.f)));
}

/**
 * @tc.name: ToJsonValue001
 * @tc.desc: Test ToJsonValue
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ToJsonValue001, TestSize.Level1)
{
    ScrollModelNG model = CreateScroll();
    model.SetInitialOffset(OffsetT(CalcDimension(10.f), CalcDimension(20.f)));
    CreateContent();
    CreateDone();
    EXPECT_EQ(pattern_->GetInitialOffset().GetX().ToString(), "10.00px");
    EXPECT_EQ(pattern_->GetInitialOffset().GetY().ToString(), "20.00px");

    /**
     * @tc.steps: step1. !IsFastFilter
     */
    InspectorFilter filter;
    auto json = JsonUtil::Create(true);
    pattern_->ToJsonValue(json, filter);
    auto initialOffset = json->GetObject("initialOffset");
    EXPECT_EQ(initialOffset->GetString("xOffset"), "10.00px");
    EXPECT_EQ(initialOffset->GetString("yOffset"), "20.00px");

    /**
     * @tc.steps: step2. IsFastFilter
     */
    std::string attr = "id";
    filter.AddFilterAttr(attr);
    json = JsonUtil::Create(true);
    pattern_->ToJsonValue(json, filter);
    initialOffset = json->GetObject("initialOffset");
    EXPECT_EQ(initialOffset->GetString("xOffset"), "");
    EXPECT_EQ(initialOffset->GetString("yOffset"), "");
}

/**
 * @tc.name: RTL001
 * @tc.desc: Test horizontal scroll in RTL Layout, content size less than scroll size
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, RTL001, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    ScrollModelNG model = CreateScroll();
    model.SetAxis(Axis::HORIZONTAL);
    CreateContent(SCROLL_WIDTH / 2);
    CreateDone(frameNode_);

    /**
     * @tc.steps: step1. Set content width less than scroll width
     */
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 0), OffsetF(SCROLL_WIDTH/4, 0.f)));
}
} // namespace OHOS::Ace::NG

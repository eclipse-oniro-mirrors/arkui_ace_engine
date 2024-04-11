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

#include "swiper_test_ng.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class SwiperCommonTestNg : public SwiperTestNg {
public:
    AssertionResult IsEqualNextFocusNode(FocusStep step,
        const RefPtr<FrameNode>& currentNode, const RefPtr<FrameNode>& expectNextNode);
    void OnKeyEvent(KeyCode keyCode, KeyAction keyAction);
};

AssertionResult SwiperCommonTestNg::IsEqualNextFocusNode(FocusStep step,
    const RefPtr<FrameNode>& currentNode, const RefPtr<FrameNode>& expectNextNode)
{
    RefPtr<FocusHub> currentFocusNode = currentNode->GetOrCreateFocusHub();
    currentFocusNode->RequestFocusImmediately();
    RefPtr<FocusHub> nextFocusNode = pattern_->GetNextFocusNode(step, currentFocusNode).Upgrade();
    if (expectNextNode == nullptr && nextFocusNode != nullptr) {
        return AssertionFailure() << "Next focusNode is not null";
    }
    if (expectNextNode != nullptr && nextFocusNode != expectNextNode->GetOrCreateFocusHub()) {
        return AssertionFailure() << "Next focusNode is not as expected";
    }
    return AssertionSuccess();
}

void SwiperCommonTestNg::OnKeyEvent(KeyCode keyCode, KeyAction keyAction)
{
    auto focusHub = frameNode_->GetFocusHub();
    focusHub->ProcessOnKeyEventInternal(KeyEvent(keyCode, keyAction));
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: HandleTouchEvent001
 * @tc.desc: Test HandleTouchEvent invalid args
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent001, TestSize.Level1)
{
    /**
     * @tc.cases: Call HandleTouchEvent with empty TouchLocationInfo
     * @tc.expected: isTouchDown_ is still false when touch
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->isTouchDown_);

    pattern_->HandleTouchEvent(TouchEventInfo("touch"));
    EXPECT_FALSE(pattern_->isTouchDown_);
}

/**
 * @tc.name: HandleTouchEvent002
 * @tc.desc: Test HandleTouchEvent invalid args
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent002, TestSize.Level1)
{
    /**
     * @tc.cases: Call HandleTouchEvent with invalid TouchType::UNKNOWN
     * @tc.expected: isTouchDown_ is still false when touch
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->isTouchDown_);

    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UNKNOWN, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
}

/**
 * @tc.name: HandleTouchEvent003
 * @tc.desc: When touch down, the animation will stop. When touch up, the animation will run.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Swipe to item(index:1), set animation to true
     * @tc.expected: When touch up, will trigger UpdateAnimationProperty and stop animation
     */
    CreateWithItem([](SwiperModelNG model) {});
    ShowNext();
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);
    pattern_->springAnimationIsRunning_ = true;
    pattern_->childScrolling_ = true;

    /**
     * @tc.steps: step1. Touch down a location
     * @tc.expected: isTouchDown_ is true, Animation stop
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset()));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_FALSE(pattern_->springAnimationIsRunning_);
    EXPECT_FALSE(pattern_->childScrolling_);

    /**
     * @tc.steps: step2. Touch up
     * @tc.expected: isTouchDown_ is false, Animation resume
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
    EXPECT_TRUE(pattern_->moveDirection_);
}

/**
 * @tc.name: HandleTouchEvent004
 * @tc.desc: When touch down, the animation will stop. When touch cancel, the animation will run.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent004, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->fadeAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down a location
     * @tc.expected: Animation stoped
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset()));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_FALSE(pattern_->fadeAnimationIsRunning_);

    /**
     * @tc.steps: step2. Touch cancel
     * @tc.expected: Animation resume
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::CANCEL, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->fadeAnimationIsRunning_);
}

/**
 * @tc.name: HandleTouchEvent005
 * @tc.desc: When touch down on indicatorNode, the animation will not stop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent005, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->springAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down on indicatorNode
     * @tc.expected: Animation still running
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset(SWIPER_WIDTH / 2, SWIPER_HEIGHT)));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
    
    /**
     * @tc.steps: step2. Touch up
     * @tc.expected: Animation still running
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, Offset(SWIPER_WIDTH / 2, SWIPER_HEIGHT)));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
}

/**
 * @tc.name: HandleTouchEvent006
 * @tc.desc: When touch down on indicatorNode area, but no indicatorNode, the animation will stop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent006, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetShowIndicator(false); // not show indicator
    });
    pattern_->translateAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down on indicatorNode area
     * @tc.expected: Animation stoped
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset(SWIPER_WIDTH / 2, SWIPER_HEIGHT)));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_FALSE(pattern_->translateAnimationIsRunning_);
}

/**
 * @tc.name: AccessibilityProperty001
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(accessibilityProperty_->GetCurrentIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetBeginIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetEndIndex(), 0);
    AccessibilityValue result = accessibilityProperty_->GetAccessibilityValue();
    EXPECT_EQ(result.min, 0);
    EXPECT_EQ(result.max, 3);
    EXPECT_EQ(result.current, 0);
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());
    EXPECT_EQ(accessibilityProperty_->GetCollectionItemCounts(), 4);
}

/**
 * @tc.name: AccessibilityProperty002
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Current is first page
     * @tc.expected: ACTION_SCROLL_FORWARD
     */
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty003
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Show next page, Current is second(middle) page
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    ShowNext();
    accessibilityProperty_->ResetSupportAction();
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty004
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Show last page, Current is last page
     * @tc.expected: ACTION_SCROLL_BACKWARD
     */
    ChangeIndex(3);
    accessibilityProperty_->ResetSupportAction();
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty005
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty005, TestSize.Level1)
{
    /**
     * @tc.cases: Swiper is loop
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    CreateWithItem([](SwiperModelNG model) {});
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty006
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty006, TestSize.Level1)
{
    /**
     * @tc.cases: Create unscrollable swiepr
     * @tc.expected: exptectActions is 0
     */
    Create([](SwiperModelNG model) {
        model.SetLoop(false);
        CreateItem(1);
    });
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: PerformActionTest001
 * @tc.desc: Swiper AccessibilityProperty PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, PerformActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Scrollable swiper
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());

    /**
     * @tc.steps: step2. call ActActionScrollForward
     * @tc.expected: ShowNext is triggered
     */
    accessibilityProperty_->ActActionScrollForward();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 1);

    /**
     * @tc.steps: step3. call ActActionScrollBackward
     * @tc.expected: ShowPrevious is triggered
     */
    accessibilityProperty_->ActActionScrollBackward();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 0);
}

/**
 * @tc.name: PerformActionTest002
 * @tc.desc: Swiper AccessibilityProperty PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, PerformActionTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. UnScrollable swiper
     */
    Create([](SwiperModelNG model) {
        model.SetLoop(false);
        CreateItem(1);
    });
    EXPECT_FALSE(accessibilityProperty_->IsScrollable());

    /**
     * @tc.steps: step2. call ActActionScrollForward
     * @tc.expected: ShowNext is not triggered
     */
    accessibilityProperty_->ActActionScrollForward();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 0);

    /**
     * @tc.steps: step3. call ActActionScrollBackward
     * @tc.expected: ShowPrevious is not triggered
     */
    accessibilityProperty_->ActActionScrollBackward();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 0);
}

/**
 * @tc.name: FocusStep001
 * @tc.desc: Test FocusStep with arrow
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDisplayArrow(true); // show arrow
        model.SetHoverShow(false);
        model.SetArrowStyle(ARROW_PARAMETERS);
    });
    auto indicatorNode = GetChildFrameNode(frameNode_, 4);
    auto leftArrowNode = GetChildFrameNode(frameNode_, 5);
    auto rightArrowNode = GetChildFrameNode(frameNode_, 6);

    /**
     * @tc.cases: GetNextFocusNode from indicatorNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, indicatorNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, indicatorNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, rightArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, indicatorNode, rightArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, indicatorNode, nullptr));

    /**
     * @tc.cases: GetNextFocusNode from leftArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, leftArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, leftArrowNode, nullptr));

    /**
     * @tc.cases: GetNextFocusNode from rightArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, rightArrowNode, nullptr));
}

/**
 * @tc.name: FocusStep002
 * @tc.desc: Test FocusStep with arrow and VERTICAL layout
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL); // set VERTICAL
        model.SetDisplayArrow(true); // show arrow
        model.SetHoverShow(false);
        model.SetArrowStyle(ARROW_PARAMETERS);
    });
    auto indicatorNode = GetChildFrameNode(frameNode_, 4);
    auto leftArrowNode = GetChildFrameNode(frameNode_, 5);
    auto rightArrowNode = GetChildFrameNode(frameNode_, 6);

    /**
     * @tc.cases: GetNextFocusNode from indicatorNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, indicatorNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, indicatorNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, indicatorNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, indicatorNode, rightArrowNode));

    /**
     * @tc.cases: GetNextFocusNode from leftArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, leftArrowNode, indicatorNode));

    /**
     * @tc.cases: GetNextFocusNode from rightArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::SHIFT_TAB, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::LEFT, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::TAB, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::RIGHT, rightArrowNode, nullptr));
}

/**
 * @tc.name: FocusStep003
 * @tc.desc: Test FocusStep with arrow and loop:false
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep003, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false); // set false loop
        model.SetDisplayArrow(true); // show arrow
        model.SetHoverShow(false);
        model.SetArrowStyle(ARROW_PARAMETERS);
    });
    auto indicatorNode = GetChildFrameNode(frameNode_, 4);
    auto leftArrowNode = GetChildFrameNode(frameNode_, 5);
    auto rightArrowNode = GetChildFrameNode(frameNode_, 6);

    /**
     * @tc.cases: LoopIndex is first item(index:0)
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, nullptr)); // PreviousFocus
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, rightArrowNode)); // NextFocus

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, indicatorNode));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));

    /**
     * @tc.cases: LoopIndex is middle item(index:1)
     */
    ChangeIndex(1);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 1);
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, rightArrowNode));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, indicatorNode));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));

    /**
     * @tc.cases: LoopIndex is last item(index:3)
     */
    ChangeIndex(3);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 3);
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, nullptr));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, indicatorNode));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, indicatorNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));
}

/**
 * @tc.name: FocusStep004
 * @tc.desc: Test FocusStep with arrow and loop:false and without indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep004, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetShowIndicator(false); // without indicator
        model.SetLoop(false);
        model.SetDisplayArrow(true); // show arrow
        model.SetHoverShow(false);
        model.SetArrowStyle(ARROW_PARAMETERS);
    });
    auto leftArrowNode = GetChildFrameNode(frameNode_, 4);
    auto rightArrowNode = GetChildFrameNode(frameNode_, 5);

    /**
     * @tc.cases: LoopIndex is last item(index:0)
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr)); // PreviousFocus
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, rightArrowNode)); // NextFocus

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));

    /**
     * @tc.cases: LoopIndex is middle item(index:1)
     */
    ChangeIndex(1);
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, rightArrowNode));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));

    /**
     * @tc.cases: LoopIndex is last item(index:3)
     */
    ChangeIndex(3);
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, nullptr));

    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));
}

/**
 * @tc.name: FocusStep005
 * @tc.desc: Test FocusStep without arrow
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep005, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorNode = GetChildFrameNode(frameNode_, 4);

    /**
     * @tc.cases: GetNextFocusNode from indicatorNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, indicatorNode, nullptr)); // PreviousFocus
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, indicatorNode, nullptr)); // NextFocus
}

/**
 * @tc.name: FocusStep006
 * @tc.desc: Test FocusStep without indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, FocusStep006, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetShowIndicator(false); // without indicator
        model.SetDisplayArrow(true); // show arrow
        model.SetHoverShow(false);
        model.SetArrowStyle(ARROW_PARAMETERS);
    });
    auto leftArrowNode = GetChildFrameNode(frameNode_, 4);
    auto rightArrowNode = GetChildFrameNode(frameNode_, 5);
    
    /**
     * @tc.cases: GetNextFocusNode from leftArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, leftArrowNode, nullptr)); // PreviousFocus
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, leftArrowNode, rightArrowNode)); // NextFocus
    
    /**
     * @tc.cases: GetNextFocusNode from rightArrowNode
     */
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::UP, rightArrowNode, leftArrowNode));
    EXPECT_TRUE(IsEqualNextFocusNode(FocusStep::DOWN, rightArrowNode, nullptr));
}

/**
 * @tc.name: DumpAdvanceInfo001
 * @tc.desc: DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, DumpAdvanceInfo001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step1. Test DumpAdvanceInfo function.
     * @tc.expected: SwiperIndicatorType::DOT.
     */
    pattern_->lastSwiperIndicatorType_ = SwiperIndicatorType::DOT;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->lastSwiperIndicatorType_, SwiperIndicatorType::DOT);

    /**
     * @tc.steps: step2. Test DumpAdvanceInfo function.
     * @tc.expected: SwiperIndicatorType::DIGIT.
     */
    pattern_->lastSwiperIndicatorType_ = SwiperIndicatorType::DIGIT;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->lastSwiperIndicatorType_, SwiperIndicatorType::DIGIT);

    /**
     * @tc.steps: step3. Test DumpAdvanceInfo function.
     * @tc.expected: PanDirection::NONE.
     */
    pattern_->panDirection_.type = PanDirection::NONE;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::NONE);

    pattern_->panDirection_.type = PanDirection::LEFT;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::LEFT);

    pattern_->panDirection_.type = PanDirection::RIGHT;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::RIGHT);

    pattern_->panDirection_.type = PanDirection::HORIZONTAL;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::HORIZONTAL);

    pattern_->panDirection_.type = PanDirection::UP;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::UP);

    pattern_->panDirection_.type = PanDirection::DOWN;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::DOWN);

    pattern_->panDirection_.type = PanDirection::VERTICAL;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::VERTICAL);

    pattern_->panDirection_.type = PanDirection::ALL;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->panDirection_.type, PanDirection::ALL);
}

/**
 * @tc.name: DumpAdvanceInfo002
 * @tc.desc: DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, DumpAdvanceInfo002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    pattern_->direction_ = Axis::NONE;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->direction_, Axis::NONE);

    pattern_->direction_ = Axis::HORIZONTAL;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->direction_, Axis::HORIZONTAL);

    pattern_->direction_ = Axis::FREE;
    pattern_->DumpAdvanceInfo();
    EXPECT_EQ(pattern_->direction_, Axis::FREE);

    pattern_->direction_ = Axis::VERTICAL;
    pattern_->DumpAdvanceInfo();
    ASSERT_EQ(pattern_->direction_, Axis::VERTICAL);
}

/**
 * @tc.name: Distributed001
 * @tc.desc: Test the distributed capability of Swiper
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, Distributed001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and get pattern_.
     */
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step2. Set Index.
     * @tc.expected: Function ProvideRestoreInfo is called.
     */
    layoutProperty_->UpdateIndex(1);
    std::string ret = pattern_->ProvideRestoreInfo();
    EXPECT_EQ(ret, R"({"Index":1})");

    /**
     * @tc.steps: step3. Function OnRestoreInfo is called.
     * @tc.expected: Passing invalid & valid JSON format.
     */
    std::string restoreInfo_ = R"({"Index":1})";
    pattern_->OnRestoreInfo(restoreInfo_);
    EXPECT_EQ(layoutProperty_->GetIndexValue(0), 1);
    restoreInfo_ = R"({"Index":0})";
    pattern_->OnRestoreInfo(restoreInfo_);
    EXPECT_EQ(layoutProperty_->GetIndexValue(0), 0);
    restoreInfo_ = "invalid_json_string";
    pattern_->OnRestoreInfo(restoreInfo_);
    EXPECT_EQ(layoutProperty_->GetIndexValue(0), 0);
}

/**
 * @tc.name: OnKeyEvent001
 * @tc.desc: InitOnKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, OnKeyEvent001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step1. Call OnKeyEvent KEY_DPAD_LEFT
     * @tc.expected: Trigger ShowPrevious
     */
    OnKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);

    /**
     * @tc.steps: step2. Call OnKeyEvent KEY_DPAD_RIGHT
     * @tc.expected: Trigger ShowNext
     */
    OnKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
}

/**
 * @tc.name: OnKeyEvent002
 * @tc.desc: InitOnKeyEvent in VERTICAL layout
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, OnKeyEvent002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL);
    });

    /**
     * @tc.steps: step1. Call OnKeyEvent KEY_DPAD_UP
     * @tc.expected: Trigger ShowPrevious
     */
    OnKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);

    /**
     * @tc.steps: step2. Call OnKeyEvent KEY_DPAD_DOWN
     * @tc.expected: Trigger ShowNext
     */
    OnKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
}

/**
 * @tc.name: OnKeyEvent003
 * @tc.desc: InitOnKeyEvent with DisplayCount>1, swipe only when focus first/last item in page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, OnKeyEvent003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. DisplayCount>1, focus item(index:0)
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetDisplayCount(2);
    });
    GetChildFocusHub(frameNode_, 0)->RequestFocusImmediately();

    /**
     * @tc.steps: step2. Call OnKeyEvent KEY_DPAD_LEFT
     * @tc.expected: ShowPrevious and Focus pre item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 3)->IsCurrentFocus());

    /**
     * @tc.steps: step3. Call OnKeyEvent KEY_DPAD_RIGHT
     * @tc.expected: Focus next item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 0)->IsCurrentFocus());

    /**
     * @tc.steps: step4. Call OnKeyEvent KEY_DPAD_RIGHT
     * @tc.expected: ShowNext and Focus next item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 1)->IsCurrentFocus());

    /**
     * @tc.steps: step5. Call OnKeyEvent KEY_DPAD_LEFT
     * @tc.expected: Focus pre item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 0)->IsCurrentFocus());
}

/**
 * @tc.name: OnKeyEvent004
 * @tc.desc: InitOnKeyEvent with DisplayCount>1, swipe only when focus first/last item in page in VERTICAL layout
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, OnKeyEvent004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. DisplayCount>1, focus item(index:0)
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL);
        model.SetDisplayCount(2);
    });
    GetChildFocusHub(frameNode_, 0)->RequestFocusImmediately();

    /**
     * @tc.steps: step2. Call OnKeyEvent KEY_DPAD_UP
     * @tc.expected: ShowPrevious and Focus pre item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 3)->IsCurrentFocus());

    /**
     * @tc.steps: step3. Call OnKeyEvent KEY_DPAD_DOWN
     * @tc.expected: Focus next item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 0)->IsCurrentFocus());

    /**
     * @tc.steps: step4. Call OnKeyEvent KEY_DPAD_DOWN
     * @tc.expected: ShowNext and Focus next item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 1)->IsCurrentFocus());

    /**
     * @tc.steps: step5. Call OnKeyEvent KEY_DPAD_UP
     * @tc.expected: Focus pre item
     */
    OnKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
    EXPECT_TRUE(GetChildFocusHub(frameNode_, 0)->IsCurrentFocus());
}
} // namespace OHOS::Ace::NG

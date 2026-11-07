/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_accessibility_property.h"

namespace OHOS::Ace::NG {
class SwiperAccessibilityTestNg : public SwiperTestNg {};

/**
 * @tc.name: GetInfo001
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, GetInfo001, TestSize.Level1)
{
    CreateSwiper();
    CreateSwiperItems();
    CreateSwiperDone();
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());
    EXPECT_EQ(accessibilityProperty_->GetCollectionItemCounts(), 4);
    EXPECT_EQ(accessibilityProperty_->GetCurrentIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetBeginIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetEndIndex(), 0);
    AccessibilityValue result = accessibilityProperty_->GetAccessibilityValue();
    EXPECT_EQ(result.min, 0);
    EXPECT_EQ(result.max, 3);
    EXPECT_EQ(result.current, 0);

    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    EXPECT_EQ(indicatorAcc->GetCollectionItemCounts(), ITEM_NUMBER);

    ChangeIndex(1);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 1);

    ChangeIndex(2);
    EXPECT_EQ(indicatorAcc->GetBeginIndex(), 2);
    EXPECT_EQ(indicatorAcc->GetEndIndex(), 2);
}

/**
 * @tc.name: AccessibilityProperty001
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, AccessibilityProperty001, TestSize.Level1)
{
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems();
    CreateSwiperDone();

    /**
     * @tc.steps: step1. Current is first page
     * @tc.expected: ACTION_SCROLL_FORWARD
     */
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_FORWARD };
    EXPECT_EQ(accessibilityProperty_->GetSupportAction(), expectedActions);

    /**
     * @tc.steps: step2. Show next page, Current is second(middle) page
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    ShowNext();
    accessibilityProperty_->ResetSupportAction();
    expectedActions = { AceAction::ACTION_SCROLL_FORWARD, AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(accessibilityProperty_->GetSupportAction(), expectedActions);

    /**
     * @tc.steps: step3. Show last page, Current is last page
     * @tc.expected: ACTION_SCROLL_BACKWARD
     */
    ChangeIndex(3);
    accessibilityProperty_->ResetSupportAction();
    expectedActions = { AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(accessibilityProperty_->GetSupportAction(), expectedActions);

    /**
     * @tc.cases: Swiper is loop
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    model.SetLoop(AceType::RawPtr(frameNode_), true);
    accessibilityProperty_->ResetSupportAction();
    expectedActions = { AceAction::ACTION_SCROLL_FORWARD, AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(accessibilityProperty_->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: AccessibilityProperty002
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, AccessibilityProperty002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. UnScrollable swiper
     */
    SwiperModelNG model = CreateSwiper();
    model.SetDisableSwipe(true);
    CreateSwiperItems();
    CreateSwiperDone();
    EXPECT_FALSE(accessibilityProperty_->IsScrollable());
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = {};
    EXPECT_EQ(accessibilityProperty_->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: PerformActionTest001
 * @tc.desc: Swiper AccessibilityProperty PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, PerformActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Scrollable swiper
     */
    CreateSwiper();
    CreateSwiperItems();
    CreateSwiperDone();
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());

    /**
     * @tc.steps: step2. Call ActActionScrollForward
     * @tc.expected: ShowNext is triggered
     */
    accessibilityProperty_->ActActionScrollForward();
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(1));

    /**
     * @tc.steps: step3. Call ActActionScrollBackward
     * @tc.expected: ShowPrevious is triggered
     */
    accessibilityProperty_->ActActionScrollBackward();
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(0));
}

/**
 * @tc.name: PerformActionTest002
 * @tc.desc: Swiper AccessibilityProperty PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, PerformActionTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. UnScrollable swiper
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems(1);
    CreateSwiperDone();
    EXPECT_FALSE(accessibilityProperty_->IsScrollable());

    /**
     * @tc.steps: step2. Call ActActionScrollForward
     * @tc.expected: ShowNext is not triggered
     */
    accessibilityProperty_->ActActionScrollForward();
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(0));

    /**
     * @tc.steps: step3. Call ActActionScrollBackward
     * @tc.expected: ShowPrevious is not triggered
     */
    accessibilityProperty_->ActActionScrollBackward();
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(0));
}

/**
 * @tc.name: UpdateFocusable001
 * @tc.desc: Test UpdateFocusable, indicatorNode is unFocusable when swiper has no items
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, UpdateFocusable001, TestSize.Level1)
{
    SwiperModelNG model = CreateSwiper();
    CreateSwiperItems(3);
    CreateSwiperDone();
    auto totalCount = pattern_->TotalCount();
    EXPECT_EQ(totalCount, 3);

    auto indicatorFocusHub = indicatorNode_->GetOrCreateFocusHub();
    EXPECT_TRUE(indicatorFocusHub->GetFocusable());
    auto accessibilityProperty = indicatorNode_->GetAccessibilityProperty<AccessibilityProperty>();
    EXPECT_EQ(accessibilityProperty->GetAccessibilityLevel(), "yes");

    frameNode_->RemoveChildAtIndex(0);
    FlushUITasks();
    EXPECT_TRUE(indicatorFocusHub->GetFocusable());

    frameNode_->RemoveChildAtIndex(0);
    frameNode_->RemoveChildAtIndex(0);
    FlushUITasks();
    EXPECT_EQ(pattern_->TotalCount(), 0);

    FlushUITasks();
    EXPECT_FALSE(indicatorFocusHub->GetFocusable());
}

/**
 * @tc.name: IndicatorAccessibilityProperty001
 * @tc.desc: Test IndicatorAccessibilityProperty GetAccessibilityLevel
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityProperty001, TestSize.Level1)
{
    auto indicatorAccessibilityProperty = AceType::MakeRefPtr<IndicatorAccessibilityProperty>();
    EXPECT_EQ(indicatorAccessibilityProperty->GetAccessibilityLevel(), "auto");
}

/**
 * @tc.name: IndicatorAccessibilityPropertyIsScrollable001
 * @tc.desc: Test SwiperIndicatorAccessibilityProperty::IsScrollable when swiper is scrollable (default loop, multiple
 * items)
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertyIsScrollable001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with default settings (loop=true, 4 items)
     * @tc.expected: IsScrollable returns true
     */
    CreateSwiper();
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_TRUE(indicatorAcc->IsScrollable());
}

/**
 * @tc.name: IndicatorAccessibilityPropertyIsScrollable002
 * @tc.desc: Test SwiperIndicatorAccessibilityProperty::IsScrollable when swiper is not scrollable (disableSwipe=true)
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertyIsScrollable002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with disableSwipe=true
     * @tc.expected: IsScrollable returns false
     */
    SwiperModelNG model = CreateSwiper();
    model.SetDisableSwipe(true);
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_FALSE(indicatorAcc->IsScrollable());
}

/**
 * @tc.name: IndicatorAccessibilityPropertyIsScrollable003
 * @tc.desc: Test SwiperIndicatorAccessibilityProperty::IsScrollable when swiper is not scrollable (loop=false, single
 * item)
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertyIsScrollable003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false and only 1 item
     * @tc.expected: IsScrollable returns false
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems(1);
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_FALSE(indicatorAcc->IsScrollable());
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction001
 * @tc.desc: Test SetSpecificSupportAction when swiper is not scrollable, no actions should be added
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create non-scrollable swiper (disableSwipe=true)
     * @tc.expected: No support actions are set on indicator
     */
    SwiperModelNG model = CreateSwiper();
    model.SetDisableSwipe(true);
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = {};
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction002
 * @tc.desc: Test SetSpecificSupportAction when swiper is scrollable and loop=true, both forward and backward actions
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollable swiper with loop=true (default)
     * @tc.expected: Both ACTION_SCROLL_FORWARD and ACTION_SCROLL_BACKWARD are set
     */
    CreateSwiper();
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = {
        AceAction::ACTION_SCROLL_FORWARD,
        AceAction::ACTION_SCROLL_BACKWARD
    };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction003
 * @tc.desc: Test SetSpecificSupportAction when loop=false and current is first page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, current index is 0 (first page)
     * @tc.expected: Only ACTION_SCROLL_FORWARD is set (no ACTION_SCROLL_BACKWARD)
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 0);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_FORWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction004
 * @tc.desc: Test SetSpecificSupportAction when loop=false and current is middle page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, navigate to middle page (index=1)
     * @tc.expected: Both ACTION_SCROLL_FORWARD and ACTION_SCROLL_BACKWARD are set
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems();
    CreateSwiperDone();
    ChangeIndex(1);
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 1);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = {
        AceAction::ACTION_SCROLL_FORWARD,
        AceAction::ACTION_SCROLL_BACKWARD
    };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction005
 * @tc.desc: Test SetSpecificSupportAction when loop=false and current is last page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, navigate to last page (index=3)
     * @tc.expected: Only ACTION_SCROLL_BACKWARD is set (no ACTION_SCROLL_FORWARD)
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems();
    CreateSwiperDone();
    ChangeIndex(3);
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 3);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction006
 * @tc.desc: Test SetSpecificSupportAction when loop=false, displayCount=2, current at first page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, displayCount=2, 4 items, current index=0
     * @tc.expected: Only ACTION_SCROLL_FORWARD (0 < 4-2=2), no ACTION_SCROLL_BACKWARD (index=0)
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    model.SetDisplayCount(2);
    CreateSwiperItems();
    CreateSwiperDone();
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 0);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_FORWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction007
 * @tc.desc: Test SetSpecificSupportAction when loop=false, displayCount=2, current at last valid page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, displayCount=2, 4 items, navigate to index=2 (last valid page)
     * @tc.expected: Only ACTION_SCROLL_BACKWARD (2 >= 4-2=2, no forward), index>0 so backward
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    model.SetDisplayCount(2);
    CreateSwiperItems();
    CreateSwiperDone();
    ChangeIndex(2);
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 2);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction008
 * @tc.desc: Test SetSpecificSupportAction when loop=false, displayCount=2, current at middle page
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, displayCount=2, 4 items, navigate to index=1 (middle page)
     * @tc.expected: Both ACTION_SCROLL_FORWARD (1 < 4-2=2) and ACTION_SCROLL_BACKWARD (1 > 0)
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    model.SetDisplayCount(2);
    CreateSwiperItems();
    CreateSwiperDone();
    ChangeIndex(1);
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    EXPECT_EQ(indicatorAcc->GetCurrentIndex(), 1);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = {
        AceAction::ACTION_SCROLL_FORWARD,
        AceAction::ACTION_SCROLL_BACKWARD
    };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}

/**
 * @tc.name: IndicatorAccessibilityPropertySetSpecificSupportAction009
 * @tc.desc: Test SetSpecificSupportAction when loop is toggled from false to true at runtime
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAccessibilityTestNg, IndicatorAccessibilityPropertySetSpecificSupportAction009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create swiper with loop=false, navigate to last page
     * @tc.expected: Only ACTION_SCROLL_BACKWARD at last page with loop=false
     */
    SwiperModelNG model = CreateSwiper();
    model.SetLoop(false);
    CreateSwiperItems();
    CreateSwiperDone();
    ChangeIndex(3);
    auto indicatorAcc = indicatorNode_->GetAccessibilityProperty<SwiperIndicatorAccessibilityProperty>();
    ASSERT_NE(indicatorAcc, nullptr);
    indicatorAcc->ResetSupportAction();
    std::unordered_set<AceAction> expectedActions = { AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);

    /**
     * @tc.steps: step2. Toggle loop to true
     * @tc.expected: Both ACTION_SCROLL_FORWARD and ACTION_SCROLL_BACKWARD are set
     */
    model.SetLoop(AceType::RawPtr(frameNode_), true);
    indicatorAcc->ResetSupportAction();
    expectedActions = { AceAction::ACTION_SCROLL_FORWARD, AceAction::ACTION_SCROLL_BACKWARD };
    EXPECT_EQ(indicatorAcc->GetSupportAction(), expectedActions);
}
} // namespace OHOS::Ace::NG

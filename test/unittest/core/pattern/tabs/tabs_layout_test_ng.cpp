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

#include "tabs_test_ng.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t CUSTOM_TRANSITION_TIMEOUT_MS = 3000;
constexpr int32_t POST_CHANGE_LAYOUT_FRAME_COUNT = 2;
constexpr uint32_t TABS_START_EVENT = 9;
constexpr uint32_t TABS_END_EVENT = 10;
constexpr uint64_t INVALID_SUBSCRIPTION_ID = 0;
enum class TabsSwitchTrigger { CONTROLLER, TAB_BAR, SWIPE, ALLOWED_CONTROLLER, ZERO_DURATION_CONTROLLER };
} // namespace

class TabsLayoutTestNg : public TabsTestNg {
public:
    void CheckCustomTabsAttributeEnd(bool validTransition);
    void CheckTabsIndexWriteback(TabsSwitchTrigger trigger);
};

/**
 * @tc.name: TabsSize001
 * @tc.desc: Test tabs size
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsSize001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents();
    CreateTabsDone(model);

    // Tabs
    EXPECT_EQ(frameNode_->GetTag(), V2::TABS_ETS_TAG);
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TABS_HEIGHT)));

    // TabBar
    EXPECT_EQ(tabBarNode_->GetTag(), V2::TAB_BAR_ETS_TAG);
    EXPECT_TRUE(IsEqual(tabBarNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TAB_BAR_SIZE)));
    EXPECT_EQ(tabBarNode_->GetTotalChildCount(), TABCONTENT_NUMBER + MASK_COUNT + IMAGE_INDICATOR_COUNT);
    float barItemWidth = TABS_WIDTH / TABCONTENT_NUMBER;
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, barItemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(barItemWidth * 1, 0, barItemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(barItemWidth * 2, 0, barItemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(barItemWidth * 3, 0, barItemWidth, TAB_BAR_SIZE)));
    // TabBar Item
    auto barItem = GetChildFrameNode(tabBarNode_, 0);
    EXPECT_EQ(barItem->GetTag(), V2::COLUMN_ETS_TAG);
    EXPECT_EQ(barItem->GetTotalChildCount(), 2);
    auto imageNode = GetChildFrameNode(barItem, 0);
    EXPECT_EQ(imageNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto textNode = GetChildFrameNode(barItem, 1);
    EXPECT_EQ(textNode->GetTag(), V2::TEXT_ETS_TAG);

    // Divider
    EXPECT_EQ(dividerNode_->GetTag(), V2::DIVIDER_ETS_TAG);
    EXPECT_TRUE(IsEqual(dividerNode_->GetGeometryNode()->GetFrameRect(), RectF(0, TAB_BAR_SIZE, TABS_WIDTH, 0)));

    // Swiper
    EXPECT_EQ(swiperNode_->GetTag(), V2::SWIPER_ETS_TAG);
    EXPECT_TRUE(IsEqual(swiperNode_->GetGeometryNode()->GetFrameRect(),
        RectF(0, TAB_BAR_SIZE, TABS_WIDTH, TABS_HEIGHT - TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(swiperNode_, 0), RectF(0, 0, TABS_WIDTH, TABS_HEIGHT - TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(swiperNode_, 1), RectF()));
    EXPECT_TRUE(IsEqual(GetChildRect(swiperNode_, 2), RectF()));
    EXPECT_TRUE(IsEqual(GetChildRect(swiperNode_, 3), RectF()));
}

/**
 * @tc.name: BarPosition001
 * @tc.desc: Test Tabs BarPosition and vertical, affect the tabBar position
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, BarPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set BarPosition::START and Vertical:false
     * @tc.expected: The tabBar would at the top of tabs
     */
    BarPosition barPosition = BarPosition::START;
    TabsModelNG model = CreateTabs(barPosition);
    CreateTabContents();
    CreateTabsDone(model);
    EXPECT_EQ(swiperPattern_->GetDirection(), Axis::HORIZONTAL);
    EXPECT_TRUE(IsEqual(tabBarNode_->GetGeometryNode()->GetFrameRect(), RectF(0, 0, TABS_WIDTH, TAB_BAR_SIZE)));

    /**
     * @tc.steps: step2. Set BarPosition::END and Vertical:false
     * @tc.expected: The tabBar would at the bottom of tabs
     */
    model.SetTabBarPosition(AceType::RawPtr(frameNode_), BarPosition::END);
    FlushUITasks();
    EXPECT_EQ(swiperPattern_->GetDirection(), Axis::HORIZONTAL);
    EXPECT_TRUE(IsEqual(tabBarNode_->GetGeometryNode()->GetFrameRect(),
        RectF(0, TABS_HEIGHT - TAB_BAR_SIZE, TABS_WIDTH, TAB_BAR_SIZE)));

    /**
     * @tc.steps: step3. Set BarPosition::START and Vertical:true
     * @tc.expected: The tabBar would at the left of tabs
     */
    model.SetTabBarPosition(AceType::RawPtr(frameNode_), BarPosition::START);
    model.SetIsVertical(AceType::RawPtr(frameNode_), true);
    FlushUITasks();
    EXPECT_EQ(swiperPattern_->GetDirection(), Axis::VERTICAL);
    EXPECT_TRUE(IsEqual(tabBarNode_->GetGeometryNode()->GetFrameRect(), RectF(0, 0, TAB_BAR_SIZE, TABS_HEIGHT)));

    /**
     * @tc.steps: step4. Set BarPosition::END and Vertical:true
     * @tc.expected: The tabBar would at the right of tabs
     */
    model.SetTabBarPosition(AceType::RawPtr(frameNode_), BarPosition::END);
    FlushUITasks();
    EXPECT_EQ(swiperPattern_->GetDirection(), Axis::VERTICAL);
    EXPECT_TRUE(IsEqual(tabBarNode_->GetGeometryNode()->GetFrameRect(),
        RectF(TABS_WIDTH - TAB_BAR_SIZE, 0, TAB_BAR_SIZE, TABS_HEIGHT)));
}

/**
 * @tc.name: InitIndex001
 * @tc.desc: Test init index, affect the swiper show item
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, InitIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set init index is 0
     * @tc.expected: The swiper show the first item
     */
    TabsModelNG model = CreateTabs(BarPosition::START, 0);
    CreateTabContents();
    CreateTabsDone(model);
    EXPECT_TRUE(CurrentIndex(0));

    /**
     * @tc.steps: step2. Set init index is 1
     * @tc.expected: The swiper show the second item
     */
    layoutProperty_->UpdateIndexSetByUser(1);
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(1));

    /**
     * @tc.steps: step3. Set init index is -1
     * @tc.expected: The swiper show item is not change
     */
    layoutProperty_->UpdateIndexSetByUser(-1);
    FlushUITasks();
    EXPECT_TRUE(CurrentIndex(1));
}

/**
 * @tc.name: TabBarMode_Fixed001
 * @tc.desc: The tabBar items always average split barWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabBarMode_Fixed001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Set total tabBar items width < TABS_WIDTH
     * @tc.expected: The tabBar items always average split barWidth
     */
    TabsModelNG model = CreateTabs();
    CreateTabContentsWithBuilder();
    CreateTabsDone(model);
    EXPECT_EQ(tabBarPattern_->axis_, Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty_->GetTabBarModeValue(TabBarMode::FIXED), TabBarMode::FIXED);
    EXPECT_EQ(tabBarNode_->GetGeometryNode()->GetFrameRect().Width(), TABS_WIDTH);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 0), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 1), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 2), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 3), TABS_WIDTH / TABCONTENT_NUMBER);

    /**
     * @tc.steps: steps2. Set total tabBar items width > TABS_WIDTH
     * @tc.expected: The tabBar items always average split barWidth
     */
    auto barItem = GetChildFrameNode(tabBarNode_, 2);
    ViewAbstract::SetWidth(AceType::RawPtr(barItem), CalcLength(10000.0f));
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(tabBarNode_, 0), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 1), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 2), TABS_WIDTH / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 3), TABS_WIDTH / TABCONTENT_NUMBER);
}

/**
 * @tc.name: TabBarMode_Fixed002
 * @tc.desc: The tabBar items always average split barHeight
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabBarMode_Fixed002, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Set total tabBar items height < TABS_HEIGHT
     * @tc.expected: The tabBar items always average split barHeight
     */
    TabsModelNG model = CreateTabs();
    model.SetIsVertical(true);
    CreateTabContentsWithBuilder();
    CreateTabsDone(model);
    EXPECT_EQ(tabBarPattern_->axis_, Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty_->GetTabBarModeValue(TabBarMode::FIXED), TabBarMode::FIXED);
    EXPECT_EQ(tabBarNode_->GetGeometryNode()->GetFrameRect().Height(), TABS_HEIGHT);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 0), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 1), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 2), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 3), TABS_HEIGHT / TABCONTENT_NUMBER);

    /**
     * @tc.steps: steps2. Set total tabBar items height > TABS_HEIGHT
     * @tc.expected: The tabBar items always average split barHeight
     */
    auto barItem = GetChildFrameNode(tabBarNode_, 2);
    ViewAbstract::SetHeight(AceType::RawPtr(barItem), CalcLength(10000.0f));
    FlushUITasks();
    EXPECT_EQ(GetChildHeight(tabBarNode_, 0), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 1), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 2), TABS_HEIGHT / TABCONTENT_NUMBER);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 3), TABS_HEIGHT / TABCONTENT_NUMBER);
}

/**
 * @tc.name: TabBarMode_Scrollable001
 * @tc.desc: TabBar items width is its actual width
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabBarMode_Scrollable001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Set total tabBar items width < TABS_WIDTH
     * @tc.expected: TabBar items width is its actual width
     */
    TabsModelNG model = CreateTabs();
    model.SetTabBarMode(TabBarMode::SCROLLABLE);
    CreateTabContentsWithBuilder();
    CreateTabsDone(model);
    EXPECT_EQ(tabBarPattern_->axis_, Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty_->GetTabBarModeValue(TabBarMode::FIXED), TabBarMode::SCROLLABLE);
    EXPECT_EQ(tabBarNode_->GetGeometryNode()->GetFrameRect().Width(), TABS_WIDTH);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 0), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 1), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 2), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 3), BAR_ITEM_SIZE);

    /**
     * @tc.steps: steps2. Set total tabBar items width > TABS_WIDTH
     * @tc.expected: TabBar items width is its actual width
     */
    auto barItem = GetChildFrameNode(tabBarNode_, 2);
    ViewAbstract::SetWidth(AceType::RawPtr(barItem), CalcLength(10000.0f));
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(tabBarNode_, 0), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 1), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 2), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildWidth(tabBarNode_, 3), BAR_ITEM_SIZE);
}

/**
 * @tc.name: TabBarMode_Scrollable002
 * @tc.desc: TabBar items height is its actual height
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabBarMode_Scrollable002, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Set total tabBar items height < TABS_HEIGHT
     * @tc.expected: TabBar items height is its actual height
     */
    TabsModelNG model = CreateTabs();
    model.SetTabBarMode(TabBarMode::SCROLLABLE);
    model.SetIsVertical(true);
    CreateTabContentsWithBuilder();
    CreateTabsDone(model);
    EXPECT_EQ(tabBarPattern_->axis_, Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty_->GetTabBarModeValue(TabBarMode::FIXED), TabBarMode::SCROLLABLE);
    EXPECT_EQ(tabBarNode_->GetGeometryNode()->GetFrameRect().Height(), TABS_HEIGHT);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 0), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 1), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 2), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 3), BAR_ITEM_SIZE);

    /**
     * @tc.steps: steps2. Set total tabBar items height > TABS_HEIGHT
     * @tc.expected: TabBar items height is its actual height
     */
    auto barItem = GetChildFrameNode(tabBarNode_, 2);
    ViewAbstract::SetHeight(AceType::RawPtr(barItem), CalcLength(10000.0f));
    FlushUITasks();
    EXPECT_EQ(GetChildHeight(tabBarNode_, 0), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 1), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 2), BAR_ITEM_SIZE);
    EXPECT_EQ(GetChildHeight(tabBarNode_, 3), BAR_ITEM_SIZE);
}

/**
 * @tc.name: Divider001
 * @tc.desc: Test divider
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, Divider001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Not set divider
     * @tc.expected: Not show divider
     */
    TabsModelNG model = CreateTabs();
    CreateTabContents();
    CreateTabsDone(model);
    EXPECT_TRUE(IsEqual(dividerNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, 0)));

    /**
     * @tc.steps: steps2. Set Divider
     * @tc.expected: Has divider
     */
    TabsItemDivider divider;
    divider.strokeWidth = Dimension(4);
    divider.startMargin = Dimension(5);
    divider.endMargin = Dimension(5);
    divider.color = Color::BLUE;
    divider.isNull = false;
    model.SetDivider(AceType::RawPtr(frameNode_), divider);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(dividerNode_->GetGeometryNode()->GetFrameSize(), SizeF(710.0f, 4.0f)));

    /**
     * @tc.steps: steps3. Set Divider and isNull is true
     * @tc.expected: Create a new divider replace
     */
    divider.isNull = true;
    model.SetDivider(AceType::RawPtr(frameNode_), divider);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(dividerNode_->GetGeometryNode()->GetFrameSize(), SizeF(710.0f, 1.0f)));
}

/**
 * @tc.name: BarGridAlign001
 * @tc.desc: Test Tabs BarGridAlign
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, BarGridAlign001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Default
     * @tc.expected: TabBarItem width are full of tabsWidth
     */
    TabsModelNG model = CreateTabs();
    CreateTabContents();
    CreateTabsDone(model);
    float itemWidth = TABS_WIDTH / TABCONTENT_NUMBER;
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, itemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(itemWidth, 0, itemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(itemWidth * 2, 0, itemWidth, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(itemWidth * 3, 0, itemWidth, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps2. Set BarGridColumnOptions, tabsWidth is md
     * @tc.expected: TabBarItem position changed
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(800.0f));
    BarGridColumnOptions options;
    options.sm = 4;
    options.md = 3;
    options.lg = 4;
    options.gutter = Dimension(5);
    options.margin = Dimension(10);
    model.SetBarGridAlign(AceType::RawPtr(frameNode_), options);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(200.0f, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(400.0f, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(600.0f, 0, 200.0f, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps3. TabsWidth is sm
     * @tc.expected: TabBarItem position changed
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(400.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(10.0f, 0, 95.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(105.0f, 0, 95.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(200.0f, 0, 95.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(295.0f, 0, 95.0f, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps4. TabsWidth is lg
     * @tc.expected: TabBarItem position changed
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(900.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(305.0f, 0, 72.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(377.5f, 0, 72.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(450.0f, 0, 72.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(522.5f, 0, 72.5f, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps4. TabsWidth is > lg
     * @tc.expected: The margin between items would be 0
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(1500.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(505.0f, 0, 122.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(627.5f, 0, 122.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(750.0f, 0, 122.5f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(872.5f, 0, 122.5f, TAB_BAR_SIZE)));
}

/**
 * @tc.name: BarGridAlign002
 * @tc.desc: Test Tabs BarGridAlign
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, BarGridAlign002, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Set columnNum > maxNum
     * @tc.expected: The margin between items would be 0
     */
    TabsModelNG model = CreateTabs();
    BarGridColumnOptions options;
    options.sm = 6;  // > SM_COLUMN_NUM
    options.md = 10; // > MD_COLUMN_NUM
    options.lg = 14; // > LG_COLUMN_NUM
    options.gutter = Dimension(5);
    options.margin = Dimension(10);
    model.SetBarGridAlign(options);
    CreateTabContents();
    CreateTabsDone(model);

    /**
     * @tc.steps: steps2. TabsWidth is md
     * @tc.expected: The margin between items would be 0
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(800.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(200.0f, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(400.0f, 0, 200.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(600.0f, 0, 200.0f, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps3. TabsWidth is sm
     * @tc.expected: The margin between items would be 0
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(400.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, 100.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(100.0f, 0, 100.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(200.0f, 0, 100.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(300.0f, 0, 100.0f, TAB_BAR_SIZE)));

    /**
     * @tc.steps: steps4. TabsWidth is lg
     * @tc.expected: The margin between items would be 0
     */
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(900.0f));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 0), RectF(0, 0, 225.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 1), RectF(225.0f, 0, 225.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 2), RectF(450.0f, 0, 225.0f, TAB_BAR_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildRect(tabBarNode_, 3), RectF(675.0f, 0, 225.0f, TAB_BAR_SIZE)));
}

/**
 * @tc.name: SetWidthAuto001
 * @tc.desc: test SetWidthAuto and SetHeightAuto
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, SetWidthAuto001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    model.SetWidthAuto(true);
    model.SetHeightAuto(true);
    CreateTabContents();
    CreateTabsDone(model);
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TABS_HEIGHT)));

    model.SetBarOverlap(AceType::RawPtr(frameNode_), true);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TABS_HEIGHT)));
}

/**
 * @tc.name: SetWidthAuto002
 * @tc.desc: test SetWidthAuto and SetHeightAuto in vertical
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, SetWidthAuto002, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    model.SetWidthAuto(true);
    model.SetHeightAuto(true);
    model.SetIsVertical(true);
    CreateTabContents();
    CreateTabsDone(model);
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TABS_HEIGHT)));

    model.SetBarOverlap(AceType::RawPtr(frameNode_), true);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameSize(), SizeF(TABS_WIDTH, TABS_HEIGHT)));
}
/**
 * @tc.name: CustomTabsAttributeChangeReportsEnd
 * @tc.desc: Attribute jumps in custom mode report after layout without invoking a transition proxy.
 * @tc.type: FUNC
 */
void TabsLayoutTestNg::CheckCustomTabsAttributeEnd(bool validTransition)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    auto model = CreateTabs();
    model.SetIsCustomAnimation(true);
    int transitions = 0;
    model.SetOnCustomAnimation([&transitions, validTransition](int32_t, int32_t) {
        ++transitions;
        TabContentAnimatedTransition transition;
        if (validTransition) {
            transition.timeout = CUSTOM_TRANSITION_TIMEOUT_MS;
            transition.transition = [](const RefPtr<TabContentTransitionProxy>&) {};
        }
        return transition;
    });
    CreateTabContents();
    CreateTabsDone(model);
    std::vector<uint32_t> a;
    std::vector<uint32_t> b;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &a, sink, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);
    for (int index : { 1, 2 }) {
        a.clear();
        b.clear();
        layoutProperty_->UpdateIndexSetByUser(index);
        FlushUITasks();
        manager->OnVsyncEnd(RectF());
        EXPECT_TRUE(CurrentIndex(index));
        EXPECT_EQ(transitions, 0);
        EXPECT_EQ(a, (std::vector<uint32_t> { 9, 10 }));
        EXPECT_EQ(b, (std::vector<uint32_t> { 10 }));
        a.clear();
        b.clear();
        layoutProperty_->UpdateIndexSetByUser(index);
        FlushUITasks();
        manager->OnVsyncEnd(RectF());
        EXPECT_TRUE(a.empty());
        EXPECT_TRUE(b.empty());
    }
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

/**
 * @tc.name: CustomTabsAttributeChangeReportsEnd
 * @tc.desc: Preserve the original delegate-returning-undefined property path.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, CustomTabsAttributeChangeReportsEnd, TestSize.Level1)
{
    CheckCustomTabsAttributeEnd(false);
}

/**
 * @tc.name: ValidCustomTabsAttributeChangeReportsEnd
 * @tc.desc: A valid transition delegate must not suppress property-driven END.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, ValidCustomTabsAttributeChangeReportsEnd, TestSize.Level1)
{
    CheckCustomTabsAttributeEnd(true);
}

/**
 * @tc.name: CustomTabsControllerEndIsNotDuplicatedByLayout
 * @tc.desc: The controller creates a real proxy; only its completion produces END.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, CustomTabsControllerEndIsNotDuplicatedByLayout, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    auto model = CreateTabs();
    model.SetIsCustomAnimation(true);
    RefPtr<TabContentTransitionProxy> proxy;
    model.SetOnCustomAnimation([&proxy](int32_t, int32_t) {
        TabContentAnimatedTransition transition;
        transition.timeout = CUSTOM_TRANSITION_TIMEOUT_MS;
        transition.transition = [&proxy](const RefPtr<TabContentTransitionProxy>& value) { proxy = value; };
        return transition;
    });
    CreateTabContents();
    CreateTabsDone(model);
    auto oldExecutor = pipeline->GetTaskExecutor();
    pipeline->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>(true));
    std::vector<uint32_t> a;
    std::vector<uint32_t> b;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &a, sink, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);
    swiperPattern_->SwipeTo(1);
    FlushUITasks();
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(a, (std::vector<uint32_t> { 9 }));
    EXPECT_TRUE(b.empty());
    EXPECT_NE(proxy, nullptr);
    if (proxy) {
        proxy->FinishTransition();
        FlushUITasks();
        manager->OnVsyncEnd(RectF());
        EXPECT_TRUE(CurrentIndex(1));
        EXPECT_EQ(a, (std::vector<uint32_t> { 9, 10 }));
        EXPECT_EQ(b, (std::vector<uint32_t> { 10 }));
        // Repeated completion and a later layout cannot synthesize another END.
        proxy->FinishTransition();
        FlushUITasks();
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(a, (std::vector<uint32_t> { 9, 10 }));
        EXPECT_EQ(b, (std::vector<uint32_t> { 10 }));
    }
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
    pipeline->SetTaskExecutor(oldExecutor);
}

void TabsLayoutTestNg::CheckTabsIndexWriteback(TabsSwitchTrigger trigger)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    MockAnimationManager::Enable(true);
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(1);
    auto model = CreateTabs();
    model.SetAnimationDuration(trigger == TabsSwitchTrigger::ZERO_DURATION_CONTROLLER ? 0.0f : 800.0f);
    int32_t changedIndex = -1;
    model.SetOnChange([&changedIndex](const BaseEventInfo* info) {
        changedIndex = TypeInfoHelper::DynamicCast<TabContentChangeEvent>(info)->GetIndex();
    });
    if (trigger == TabsSwitchTrigger::ALLOWED_CONTROLLER) {
        model.SetOnContentWillChange([](int32_t, int32_t) { return true; });
    }
    CreateTabContents();
    CreateTabsDone(model);
    manager->OnVsyncEnd(RectF());
    using Events = std::vector<std::pair<uint32_t, std::string>>;
    Events a;
    Events b;
    Events lateTrue;
    Events lateFalse;
    auto sink = [](uint32_t type, uint64_t, const char* json, uint32_t, void* data) {
        auto event = JsonUtil::ParseJsonString(json);
        static_cast<Events*>(data)->emplace_back(type, event->GetObject("targetNode")->GetString("id"));
    };
    uint64_t lateTrueId = INVALID_SUBSCRIPTION_ID;
    uint64_t lateFalseId = INVALID_SUBSCRIPTION_ID;
    std::function<void(uint32_t, std::string)> onEvent = [&](uint32_t type, std::string id) {
        a.emplace_back(type, id);
        if (type == TABS_START_EVENT && lateTrueId == INVALID_SUBSCRIPTION_ID) {
            lateTrueId = manager->RegisterCapiContentChangeEvent(true, 1, 0, &lateTrue, sink, nullptr);
            lateFalseId = manager->RegisterCapiContentChangeEvent(false, 1, 0, &lateFalse, sink, nullptr);
        }
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &onEvent,
        [](uint32_t type, uint64_t, const char* json, uint32_t, void* data) {
            auto event = JsonUtil::ParseJsonString(json);
            (*static_cast<std::function<void(uint32_t, std::string)>*>(data))(
                type, event->GetObject("targetNode")->GetString("id"));
        }, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);

    // Start at index 0; use the real controller, tab bar or drag entry point to switch to index 1.
    if (trigger == TabsSwitchTrigger::SWIPE) {
        auto info = CreateDragInfo(true);
        swiperPattern_->HandleDragStart(info);
        swiperPattern_->HandleDragUpdate(info);
        FlushUITasks();
        swiperPattern_->HandleDragEnd(info.GetMainVelocity());
        FlushUITasks();
    } else if (trigger == TabsSwitchTrigger::TAB_BAR) {
        HandleClick(1);
    } else {
        ChangeIndex(1);
    }
    MockAnimationManager::GetInstance().Tick();
    // MockRenderContext does not animate TranslateXY; deliver its normal completion callback explicitly.
    if (swiperPattern_->propertyAnimationIsRunning_) {
        swiperPattern_->PropertyPrefMonitor(false);
        swiperPattern_->OnPropertyTranslateAnimationFinish(OffsetF(-TABS_WIDTH, 0.0f));
        swiperPattern_->FireScrollStateEvent(ScrollState::IDLE);
    }
    pipeline->taskScheduler_->FlushAfterLayoutCallbackInImplicitAnimationTask();
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(changedIndex, 1);

    // Apply the one-way index binding after onChange, then allow two more layout/vsync cycles.
    layoutProperty_->UpdateIndexSetByUser(changedIndex);
    for (int frame = 0; frame < POST_CHANGE_LAYOUT_FRAME_COUNT; ++frame) {
        FlushUITasks();
        FlushUITasks(swiperNode_);
        manager->OnVsyncEnd(RectF());
    }
    EXPECT_TRUE(CurrentIndex(1));
    const auto nodeId = std::to_string(frameNode_->GetId());
    EXPECT_EQ(a, (Events { { TABS_START_EVENT, nodeId }, { TABS_END_EVENT, nodeId } }));
    EXPECT_EQ(b, (Events { { TABS_END_EVENT, nodeId } }));
    EXPECT_EQ(lateTrue, (Events { { TABS_END_EVENT, nodeId } }));
    EXPECT_EQ(lateFalse, (Events { { TABS_END_EVENT, nodeId } }));
    for (auto id : { aid, bid, lateTrueId, lateFalseId }) {
        manager->UnregisterCapiContentChangeEvent(id);
    }
    MockAnimationManager::Enable(false);
}

/**
 * @tc.name: TabsControllerIndexWritebackReportsOnce
 * @tc.desc: Controller completion and onChange writeback produce one cycle, including late and END-only listeners.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsControllerIndexWritebackReportsOnce, TestSize.Level1)
{
    CheckTabsIndexWriteback(TabsSwitchTrigger::CONTROLLER);
}

/**
 * @tc.name: TabsTabBarIndexWritebackReportsOnce
 * @tc.desc: Clicking tab 1 and writing back its index must not report a content replacement.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsTabBarIndexWritebackReportsOnce, TestSize.Level1)
{
    CheckTabsIndexWriteback(TabsSwitchTrigger::TAB_BAR);
}

/**
 * @tc.name: TabsSwipeIndexWritebackReportsOnce
 * @tc.desc: A completed content swipe and its index writeback must produce only one Tabs cycle.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsSwipeIndexWritebackReportsOnce, TestSize.Level1)
{
    CheckTabsIndexWriteback(TabsSwitchTrigger::SWIPE);
}

/**
 * @tc.name: TabsAllowedControllerIndexWritebackReportsOnce
 * @tc.desc: Allowing a controller switch must preserve single-cycle reporting after index writeback.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsAllowedControllerIndexWritebackReportsOnce, TestSize.Level1)
{
    CheckTabsIndexWriteback(TabsSwitchTrigger::ALLOWED_CONTROLLER);
}

/**
 * @tc.name: TabsZeroDurationControllerIndexWritebackReportsOnce
 * @tc.desc: A zero-duration controller jump and its index writeback produce only one cycle, as in W06.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsZeroDurationControllerIndexWritebackReportsOnce, TestSize.Level1)
{
    CheckTabsIndexWriteback(TabsSwitchTrigger::ZERO_DURATION_CONTROLLER);
}

/**
 * @tc.name: TabsSameIndexReplacementReportsOnce
 * @tc.desc: Replacing the selected TabContent without changing index still reports one cycle.
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutTestNg, TabsSameIndexReplacementReportsOnce, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    auto model = CreateTabs();
    CreateTabContents();
    CreateTabsDone(model);
    manager->OnVsyncEnd(RectF());
    std::vector<uint32_t> a;
    std::vector<uint32_t> b;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &a, sink, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);
    auto original = swiperNode_->GetChildAtIndex(0);
    auto replacement = FrameNode::CreateFrameNode(V2::TAB_CONTENT_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TabContentPattern>(nullptr));
    swiperNode_->ReplaceChild(original, replacement);
    swiperNode_->MarkNeedSyncRenderTree();
    for (int frame = 0; frame < POST_CHANGE_LAYOUT_FRAME_COUNT; ++frame) {
        FlushUITasks(swiperNode_);
        manager->OnVsyncEnd(RectF());
    }
    EXPECT_EQ(swiperPattern_->GetCurrentIndex(), 0);
    EXPECT_EQ(a, (std::vector<uint32_t> { TABS_START_EVENT, TABS_END_EVENT }));
    EXPECT_EQ(b, (std::vector<uint32_t> { TABS_END_EVENT }));
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

} // namespace OHOS::Ace::NG

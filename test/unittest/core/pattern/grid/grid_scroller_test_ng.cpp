/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "grid_test_ng.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class GridScrollerTestNg : public GridTestNg {
public:
    void CreateWithNoProxy(const std::function<void(GridModelNG)>& callback = nullptr);
    AssertionResult ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, float expectOffset);
};

void GridScrollerTestNg::CreateWithNoProxy(const std::function<void(GridModelNG)>& callback)
{
    GridModelNG model;
    RefPtr<ScrollControllerBase> positionController = model.CreatePositionController();
    model.Create(positionController, nullptr);
    ViewAbstract::SetWidth(CalcLength(GRID_WIDTH));
    ViewAbstract::SetHeight(CalcLength(GRID_HEIGHT));
    if (callback) {
        callback(model);
    }
    GetInstance();
    FlushLayoutTask(frameNode_);
}

AssertionResult GridScrollerTestNg::ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, float expectOffset)
{
    // After every call to ScrollToIndex(), reset currentOffset_
    float startOffset = pattern_->GetTotalOffset();
    pattern_->ScrollToIndex(index, smooth, align);
    FlushLayoutTask(frameNode_);
    if (smooth) {
        // Straight to the end of the anmiation
    }
    float currentOffset = pattern_->GetTotalOffset();
    pattern_->gridLayoutInfo_.currentOffset_ = 0; // for avoid some error
    FlushLayoutTask(frameNode_);
    pattern_->ScrollTo(startOffset); // reset currentOffset_
    FlushLayoutTask(frameNode_);
    return IsEqual(currentOffset, expectOffset);
}

/**
 * @tc.name: ScrollToIndex001
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex001, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at top, ScrollTo index:0, text each ScrollAlign
     * @tc.expected: Each test grid does not scroll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    EXPECT_TRUE(pattern_->IsAtTop());
    int32_t index = 0;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, 0.f));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, 0.f));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, 0.f));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, 0.f));
}

/**
 * @tc.name: ScrollToIndex002
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex002, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at top, ScrollTo index:8, index:8 item is in the view, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    int32_t index = 8;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 2));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 0.5));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, 0));
}

/**
 * @tc.name: ScrollToIndex003
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex003, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at top, ScrollTo index:16, index:16 item is below the view, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    int32_t index = 16;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 4));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 2.5));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT));
}

/**
 * @tc.name: ScrollToIndex004
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex004, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at top, ScrollTo index:LAST_ITEM, index:LAST_ITEM item is below the view, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    int32_t index = LAST_ITEM;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT * 6));
}

/**
 * @tc.name: ScrollToIndex005
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex005, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at middle(scroll a little distance), ScrollTo index:0, index:0 item is above the view,
     *            text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    pattern_->ScrollTo(ITEM_HEIGHT * 4);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTotalOffset(), ITEM_HEIGHT * 4);
    EXPECT_EQ(accessibilityProperty_->GetScrollOffSet(), pattern_->GetTotalOffset());
    EXPECT_FALSE(pattern_->IsAtTop());
    EXPECT_FALSE(pattern_->IsAtBottom());
    int32_t index = 0;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, 0));
}

/**
 * @tc.name: ScrollToIndex006
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex006, TestSize.Level1)
{
    /**
     * @tc.cases: Grid at bottom, ScrollTo index:LAST_ITEM, index:LAST_ITEM item is in the view, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(40);
    });
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTotalOffset(), ITEM_HEIGHT * 6);
    EXPECT_TRUE(pattern_->IsAtBottom());
    int32_t index = LAST_ITEM;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT * 6));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT * 6));
}

/**
 * @tc.name: ScrollToIndex007
 * @tc.desc: Test ScrollToIndex invalid
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex007, TestSize.Level1)
{
    /**
     * @tc.cases: ScrollTo invalid index:-2, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(24);
    });
    int32_t index = -2;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, 0));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, 0));
}

/**
 * @tc.name: ScrollToIndex008
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex008, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(31);
    });
    /**
     * @tc.steps: step1. Call ScrollToIndex()
     * @tc.expected: pattern_->targetIndex.value() is 30.
     * @tc.expected: pattern_->scrollAlign_ is ScrollAlign::END.
     */
    pattern_->ScrollToIndex(30, true, ScrollAlign::END);
    EXPECT_EQ(pattern_->targetIndex_.value(), 30);
    EXPECT_EQ(pattern_->scrollAlign_, ScrollAlign::END);
}

/**
 * @tc.name: ScrollToIndex009
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex009, TestSize.Level1)
{
    /**
     * @tc.cases: Set gap, ScrollTo index:16, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetColumnsGap(Dimension(COL_GAP));
        model.SetRowsGap(Dimension(ROW_GAP));
        CreateFixedItem(40);
    });
    int32_t index = 16;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 4 + ROW_GAP * 4));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 2.5 + ROW_GAP * 4));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT + ROW_GAP * 4));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT + ROW_GAP * 4));
}

/**
 * @tc.name: ScrollToIndex010
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex010, TestSize.Level1)
{
    /**
     * @tc.cases: Set GridLayoutOptions:irregularIndexes, ScrollTo index:7, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    GridLayoutOptions option;
    option.irregularIndexes = { 6, 1, 3, 4, 5, 0 };
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetLayoutOptions(option);
        model.SetColumnsGap(Dimension(COL_GAP));
        model.SetRowsGap(Dimension(ROW_GAP));
        CreateFixedItem(10);
    });
    int32_t index = 5;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 4 + ROW_GAP * 7));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 3.5 + ROW_GAP * 5));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT * 2 + ROW_GAP * 5));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT * 2 + ROW_GAP * 5));
}

/**
 * @tc.name: ScrollToIndex011
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, DISABLED_ScrollToIndex011, TestSize.Level1)
{
    // to do ... ScrollToIndex not support getSizeByIndex at now!!!!
    /**
     * @tc.cases: Set GridLayoutOptions:irregularIndexes getSizeByIndex,
     *            set gap, ScrollTo index:7, text each ScrollAlign
     * @tc.expected: Each test scroll the correct distance
     */
    GridLayoutOptions option;
    option.irregularIndexes = { 6, 1, 3, 4, 5, 0 };
    GetSizeByIndex onGetIrregularSizeByIndex = [](int32_t index) {
        if (index == 3) {
            return GridItemSize { 1, 2 };
        }
        return GridItemSize { 1, 4 };
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetLayoutOptions(option);
        model.SetColumnsGap(Dimension(COL_GAP));
        model.SetRowsGap(Dimension(ROW_GAP));
        CreateFixedItem(10);
    });
    int32_t index = 5;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::START, ITEM_HEIGHT * 4 + ROW_GAP * 3));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, ITEM_HEIGHT * 5.5 + ROW_GAP * 4));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::END, ITEM_HEIGHT + ROW_GAP * 3));
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT + ROW_GAP * 3));
}

/**
 * @tc.name: ScrollToIndex012
 * @tc.desc: Test ScrollToIndex with big gap(lineHeight + gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex012, TestSize.Level1)
{
    /**
     * @tc.cases: Set BIG_ROW_GAP, ScrollTo index:10, text ScrollAlign::AUTO
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetRowsGap(Dimension(BIG_ROW_GAP));
        CreateFixedItem(30);
    });
    int32_t index = 10;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, ITEM_HEIGHT * 3 + BIG_ROW_GAP * 2 - GRID_HEIGHT));
    /**
     * @tc.cases: use ScrollTo to make item 10 in the last line, ScrollTo index:10, text ScrollAlign::AUTO
     * @tc.expected: scrollToIndex don't change grid offset
     */
    auto autoPosition = ITEM_HEIGHT * 3 + BIG_ROW_GAP * 2 - GRID_HEIGHT + ITEM_HEIGHT;
    pattern_->ScrollTo(autoPosition);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, autoPosition));
}

/**
 * @tc.name: ScrollToIndex013
 * @tc.desc: Test ScrollToIndex with big gap(lineHeight + gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex013, TestSize.Level1)
{
    /**
     * @tc.cases: Set BIG_ROW_GAP, ScrollTo index:11, text each ScrollAlign::CENTER
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetRowsGap(Dimension(BIG_ROW_GAP));
        CreateFixedItem(30);
    });
    int32_t index = 11;
    auto enddPosition = ITEM_HEIGHT * 3 + BIG_ROW_GAP * 2 - GRID_HEIGHT;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::CENTER, enddPosition + (GRID_HEIGHT - ITEM_HEIGHT) / 2));
}

/**
 * @tc.name: ScrollToIndex014
 * @tc.desc: Test ScrollToIndex with medium gap(3*lineHeight + 3*gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToIndex014, TestSize.Level1)
{
    /**
     * @tc.cases: Set MEDIUM_ROW_GAP and scroll to specified position, ScrollTo index:12, text each ScrollAlign::AUTO
     * @tc.expected: Each test scroll the correct distance
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetRowsGap(Dimension(MEDIUM_ROW_GAP));
        CreateFixedItem(30);
    });
    auto position = ITEM_HEIGHT + 5 * COL_GAP;
    pattern_->ScrollTo(position);
    FlushLayoutTask(frameNode_);
    int32_t index = 12;
    EXPECT_TRUE(ScrollToIndex(index, false, ScrollAlign::AUTO, position));
}

/**
 * @tc.name: ScrollTo001
 * @tc.desc: Test ScrollTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollTo001, TestSize.Level1)
{
    /**
     * @tc.cases: UnScrollable grid
     * @tc.expected: Unroll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(4);
    });
    pattern_->ScrollTo(ITEM_HEIGHT);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: ScrollTo002
 * @tc.desc: Test ScrollTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollTo002, TestSize.Level1)
{
    /**
     * @tc.cases: Scrollable grid
     * @tc.expected: Rolled
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(10);
    });
    pattern_->ScrollTo(ITEM_HEIGHT);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT);
}

/**
 * @tc.name: ScrollTo003
 * @tc.desc: Test ScrollTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollTo003, TestSize.Level1)
{
    /**
     * @tc.cases: Scrollable grid, scroll position greater than GRID_HEIGHT
     * @tc.expected: Rolled
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(10);
    });
    pattern_->ScrollTo(ITEM_HEIGHT * 5);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT * 5);
}

/**
 * @tc.name: ScrollTo004
 * @tc.desc: Test ScrollTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollTo004, TestSize.Level1)
{
    /**
     * @tc.cases: When isConfigScrollable_ is false
     * @tc.expected: Can not roll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        model.SetRowsTemplate("1fr");
        CreateFixedItem(10);
    });
    EXPECT_FALSE(pattern_->isConfigScrollable_);

    pattern_->ScrollTo(ITEM_HEIGHT * 5);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0.f);
}

/**
 * @tc.name: AnimateTo001
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo001, TestSize.Level1)
{
    /**
     * @tc.cases: UnScrollable grid
     * @tc.expected: Unroll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(4);
    });
    pattern_->AnimateTo(ITEM_HEIGHT, 200.f, Curves::LINEAR, true);
    float endValue = pattern_->GetFinalPosition();
    pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - endValue,
        SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: AnimateTo002
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo002, TestSize.Level1)
{
    /**
     * @tc.cases: Scrollable grid
     * @tc.expected: Rolled
     */
    // smooth is true
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(10);
    });
    pattern_->AnimateTo(ITEM_HEIGHT, 200.f, Curves::LINEAR, true);
    float endValue = pattern_->GetFinalPosition();
    pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - endValue,
        SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT);
}

/**
 * @tc.name: AnimateTo003
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo003, TestSize.Level1)
{
    /**
     * @tc.cases: Scrollable grid, scroll position greater than GRID_HEIGHT
     * @tc.expected: Rolled
     */
    // smooth is true
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        CreateFixedItem(10);
    });
    pattern_->AnimateTo(ITEM_HEIGHT * 9, 200.f, Curves::LINEAR, true);
    float endValue = pattern_->GetFinalPosition();
    pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - endValue,
        SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT * 9);
}

/**
 * @tc.name: AnimateTo004
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When Animation is not running, call AnimateTo
     * @tc.expected: onScrollStart event will be triggered
     */
    bool isTrigger = false;
    auto onScrollStart = [&isTrigger]() { isTrigger = true; };
    Create([=](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        model.SetOnScrollStart(onScrollStart);
        model.SetScrollBarMode(DisplayMode::ON);
        CreateFixedItem(10);
    });
    pattern_->AnimateTo(ITEM_HEIGHT, 200.f, Curves::LINEAR, true);
    EXPECT_TRUE(isTrigger);

    float endValue = pattern_->GetFinalPosition();
    pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - endValue,
        SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT);

    /**
     * @tc.steps: step2. When Animation is running, call AnimateTo
     * @tc.expected: onScrollStart event will not be triggered
     */
    isTrigger = false; // reset val
    pattern_->scrollAbort_ = true; // set running
    pattern_->AnimateTo(ITEM_HEIGHT * 2, 200.f, Curves::LINEAR, true);
    EXPECT_FALSE(isTrigger);
}

/**
 * @tc.name: AnimateTo005
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo005, TestSize.Level1)
{
    /**
     * @tc.cases: When isConfigScrollable_ is false
     * @tc.expected: Can not roll
     */
    Create([=](GridModelNG model) {
        model.SetColumnsTemplate("1fr");
        model.SetRowsTemplate("1fr");
        CreateFixedItem(10);
    });
    EXPECT_FALSE(pattern_->isConfigScrollable_);

    pattern_->AnimateTo(ITEM_HEIGHT, 200.f, Curves::LINEAR, true);
    float endValue = pattern_->GetFinalPosition();
    pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - endValue,
        SCROLL_FROM_ANIMATION_CONTROLLER);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0.f);
}

/**
 * @tc.name: ScrollablePattern001
 * @tc.desc: Test scrollable_pattern
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollablePattern001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Test OnScrollPosition/OnScrollEnd.
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    double itemHeight = ITEM_HEIGHT;
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_START));
    pattern_->OnScrollEnd();
    auto coordinationEvent = AceType::MakeRefPtr<ScrollableCoordinationEvent>();
    auto event1 = [](float, float) { return true; };
    auto event2 = [](bool, float) {};
    auto event3 = [](float) {};
    coordinationEvent->SetOnScrollEvent(event1);
    coordinationEvent->SetOnScrollStartEvent(event2);
    coordinationEvent->SetOnScrollEndEvent(event3);
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_START));
    pattern_->OnScrollEnd();
    EXPECT_TRUE(pattern_->OnScrollPosition(itemHeight, SCROLL_FROM_UPDATE));
    pattern_->OnScrollEnd();

    /**
     * @tc.steps: step2. Test OnScrollCallback.
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    EXPECT_TRUE(pattern_->OnScrollCallback(-ITEM_HEIGHT, SCROLL_FROM_START));
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    EXPECT_TRUE(pattern_->OnScrollCallback(-ITEM_HEIGHT, SCROLL_FROM_UPDATE));
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT);
    EXPECT_TRUE(pattern_->OnScrollCallback(-ITEM_HEIGHT, SCROLL_FROM_UPDATE));
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -ITEM_HEIGHT * 2);
}

/**
 * @tc.name: PositionController001
 * @tc.desc: Test positionController func in VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, PositionController001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Unscrollable
     * @tc.expected: jumpIndex_ not change
     */
    Create([](GridModelNG model) {
        CreateFixedItem(14);
    });
    EXPECT_FALSE(pattern_->isConfigScrollable_);
    auto controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::START, 3);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, EMPTY_JUMP_INDEX);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);

    /**
     * @tc.steps: step2. Test JumpTo func.
     * @tc.expected: Verify return value.
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::START, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(19, false, ScrollAlign::START, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(11, false, ScrollAlign::START, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::CENTER, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(12, false, ScrollAlign::CENTER, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(11, false, ScrollAlign::CENTER, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -100.f);

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::END, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(12, false, ScrollAlign::END, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(11, false, ScrollAlign::END, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::AUTO, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(12, false, ScrollAlign::AUTO, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(11, false, ScrollAlign::AUTO, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    controller = pattern_->positionController_;
    controller->JumpTo(1, false, ScrollAlign::NONE, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(12, false, ScrollAlign::NONE, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    controller->JumpTo(11, false, ScrollAlign::NONE, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: PositionController002
 * @tc.desc: Test positionController func in VERTICAL Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, PositionController002, TestSize.Level1)
{
    /**
     * @tc.steps: step2. Test AnimateTo func.
     * @tc.expected: Verify return value.
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    auto controller = pattern_->positionController_;
    controller->AnimateTo(Dimension(100.f, DimensionUnit::PX), 200.f, Curves::LINEAR, false);
    ASSERT_NE(pattern_->curveAnimation_, nullptr);

    /**
     * @tc.steps: step3. Test GetScrollDirection func.
     * @tc.expected: Verify return value.
     */
    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);

    /**
     * @tc.steps: step4. Test GetCurrentOffset func.
     * @tc.expected: Verify return value.
     */
    // When finger moves up, offset is negative.
    pattern_->UpdateCurrentOffset(-20.f, SCROLL_FROM_UPDATE);
    Offset currentOffset = controller->GetCurrentOffset();
    EXPECT_EQ(currentOffset, Offset(0, 20.f));

    /**
     * @tc.steps: step5. Test ScrollToEdge func.
     * @tc.expected: Verify return value.
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, EMPTY_JUMP_INDEX);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, EMPTY_JUMP_INDEX);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, LAST_ITEM);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 0);

    /**
     * @tc.steps: step6. Test ScrollPage func.
     * @tc.expected: Verify currentOffset.
     */
    FlushLayoutTask(frameNode_);
    controller->ScrollPage(false, false);
    EXPECT_TRUE(IsEqual(controller->GetCurrentOffset(), Offset(0, GRID_HEIGHT)));

    // scroll to previous page
    controller->ScrollPage(true, false);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset(0, 0));

    /**
     * @tc.steps: step7. Test IsAtEnd func.
     */
    EXPECT_FALSE(controller->IsAtEnd());
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(controller->IsAtEnd());
}

/**
 * @tc.name: PositionController003
 * @tc.desc: Test positionController func in HORIZONTAL Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, PositionController003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Change Grid width to 300.
     */
    Create([](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step2. Test GetScrollDirection func.
     * @tc.expected: Verify return value.
     */
    auto controller = pattern_->positionController_;
    EXPECT_EQ(controller->GetScrollDirection(), Axis::HORIZONTAL);

    /**
     * @tc.steps: step4. Test ScrollToEdge func.
     * @tc.expected: Verify return value.
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, false);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, EMPTY_JUMP_INDEX);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, false);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, EMPTY_JUMP_INDEX);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 0);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, LAST_ITEM);

    /**
     * @tc.steps: step5. Test ScrollPage func.
     * @tc.expected: Verify currentOffset.
     */
    pattern_->UpdateCurrentOffset(-20.f, SCROLL_FROM_UPDATE);
    controller->ScrollPage(true, false);
    EXPECT_TRUE(IsEqual(controller->GetCurrentOffset(), Offset(20 - GRID_WIDTH, 0)));
    EXPECT_EQ(pattern_->GetMainContentSize(), GRID_WIDTH);
    controller->ScrollPage(false, false);
    EXPECT_TRUE(IsEqual(controller->GetCurrentOffset(), Offset(20.f, 0)));
}

/**
 * @tc.name: PositionController004
 * @tc.desc: Test positionController func in Axis::NONE Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, PositionController004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Supplement ScrollPage, GetCurrentOffset branch,
     * has no condition that axis_ is Axis::NONE.
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    pattern_->SetAxis(Axis::NONE);
    auto controller = pattern_->positionController_;
    controller->ScrollPage(true, false);
    controller->GetCurrentOffset();
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: PositionController005
 * @tc.desc: Test positionController func
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, PositionController005, TestSize.Level1)
{
    /**
     * @tc.cases: When isConfigScrollable_ is false
     * @tc.expected: Can not roll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetRowsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    EXPECT_FALSE(pattern_->isConfigScrollable_);

    auto controller = pattern_->positionController_;
    controller->ScrollPage(true, false);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: ScrollToFocusNodeIndex001
 * @tc.desc: Test ScrollToFocusNodeIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToFocusNodeIndex001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        CreateFixedItem(10);
    });

    /**
     * @tc.steps: step1. Focus node outside the viewport
     * @tc.expected: scroll to the node
     */
    int32_t focusNodeIndex = 6;
    pattern_->ScrollToFocusNodeIndex(focusNodeIndex);
    FlushLayoutTask(frameNode_);
    RefPtr<FocusHub> focusNode = GetChildFocusHub(frameNode_, focusNodeIndex);
    EXPECT_TRUE(focusNode->IsCurrentFocus());
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);

    /**
     * @tc.steps: step2. Focus node index not exist
     * @tc.expected: do nothing
     */
    focusNodeIndex = 10;
    pattern_->ScrollToFocusNodeIndex(focusNodeIndex);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: ScrollToNode001
 * @tc.desc: Test ScrollToNode
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToNode001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        CreateFixedItem(10);
    });

    /**
     * @tc.steps: step1. Focus node outside the viewport
     * @tc.expected: scroll to the node
     */
    int32_t focusNodeIndex = 6;
    RefPtr<FrameNode> focusNode = GetChildFrameNode(frameNode_, focusNodeIndex);
    pattern_->ScrollToNode(focusNode);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: GetOverScrollOffset001
 * @tc.desc: Test GetOverScrollOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetOverScrollOffset001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        CreateFixedItem(10);
    });

    OverScrollOffset offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    OverScrollOffset expectOffset = { ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT * 2);
    expectOffset = { ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT * 2);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT * 2;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = ITEM_HEIGHT;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    expectOffset = { ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT * 2);
    expectOffset = { -ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT * 3;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT * 2);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
}

/**
 * @tc.name: GetOverScrollOffset002
 * @tc.desc: Test GetOverScrollOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetOverScrollOffset002, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        CreateFixedItem(8);
    });

    OverScrollOffset offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    OverScrollOffset expectOffset = { ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, -ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT * 2);
    expectOffset = { ITEM_HEIGHT, ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT * 2);
    expectOffset = { 0, -ITEM_HEIGHT * 2 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT * 2;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    expectOffset = { 0, ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, -ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = ITEM_HEIGHT;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT);
    expectOffset = { ITEM_HEIGHT, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT * 2);
    expectOffset = { -ITEM_HEIGHT, -ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));

    pattern_->gridLayoutInfo_.currentOffset_ = -ITEM_HEIGHT * 3;
    offset = pattern_->GetOverScrollOffset(ITEM_HEIGHT * 2);
    expectOffset = { 0, ITEM_HEIGHT  * 2 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(0.f);
    expectOffset = { 0, 0 };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
    offset = pattern_->GetOverScrollOffset(-ITEM_HEIGHT);
    expectOffset = { 0, -ITEM_HEIGHT };
    EXPECT_TRUE(IsEqual(offset, expectOffset));
}

/**
 * @tc.name: UpdateCurrentOffset001
 * @tc.desc: Test grid pattern UpdateCurrentOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, UpdateCurrentOffset001, TestSize.Level1)
{
    /**
     * @tc.cases: Test SCROLL_FROM_UPDATE
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    UpdateCurrentOffset(-100.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -100.f);
    UpdateCurrentOffset(100.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    UpdateCurrentOffset(-200.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -200.f);
}

/**
 * @tc.name: UpdateCurrentOffset002
 * @tc.desc: Test grid pattern UpdateCurrentOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, UpdateCurrentOffset002, TestSize.Level1)
{
    /**
     * @tc.cases: Test SCROLL_FROM_BAR
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    UpdateCurrentOffset(-100.f, SCROLL_FROM_BAR);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -100.f);
    UpdateCurrentOffset(100.f, SCROLL_FROM_BAR);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    UpdateCurrentOffset(-200.f, SCROLL_FROM_BAR);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -200.f);
}

/**
 * @tc.name: UpdateCurrentOffset003
 * @tc.desc: Test grid pattern UpdateCurrentOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, UpdateCurrentOffset003, TestSize.Level1)
{
    /**
     * @tc.cases: Test SCROLL_FROM_UPDATE and EdgeEffect::FADE
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    pattern_->scrollEffect_ = AceType::MakeRefPtr<ScrollEdgeEffect>(EdgeEffect::FADE);
    UpdateCurrentOffset(-100.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -100.f);
    UpdateCurrentOffset(100.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    UpdateCurrentOffset(-200.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -200.f);
}

/**
 * @tc.name: UpdateCurrentOffset004
 * @tc.desc: Test grid pattern UpdateCurrentOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, UpdateCurrentOffset004, TestSize.Level1)
{
    /**
     * @tc.cases: When isConfigScrollable_ is false
     * @tc.expected: Can not roll
     */
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetRowsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedItem(20);
    });
    EXPECT_FALSE(pattern_->isConfigScrollable_);

    UpdateCurrentOffset(-100.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0.f);
}

/**
 * @tc.name: GridEventTestNg001
 * @tc.desc: Test scroll callback
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg001, TestSize.Level1)
{
    Dimension offsetY;
    ScrollState scrollState;
    auto event = [&offsetY, &scrollState](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
    };
    Create([event](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(event);
        CreateFixedItem(10);
    });

    UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_UPDATE);
    EXPECT_EQ(offsetY.ConvertToPx(), ITEM_HEIGHT);
    EXPECT_EQ(scrollState, ScrollState::SCROLL);

    UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    EXPECT_EQ(offsetY.ConvertToPx(), -ITEM_HEIGHT);
    EXPECT_EQ(scrollState, ScrollState::FLING);

    UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    EXPECT_EQ(offsetY.ConvertToPx(), ITEM_HEIGHT);
    EXPECT_EQ(scrollState, ScrollState::FLING);

    UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_NONE);
    EXPECT_EQ(offsetY.ConvertToPx(), -ITEM_HEIGHT);
    EXPECT_EQ(scrollState, ScrollState::IDLE);

    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_AXIS);
    FlushLayoutTask(frameNode_);
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR);
    FlushLayoutTask(frameNode_);
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushLayoutTask(frameNode_);
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR_FLING);
    FlushLayoutTask(frameNode_);

    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_UPDATE);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(offsetY.ConvertToPx(), 0);
    EXPECT_EQ(scrollState, ScrollState::IDLE);

    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(offsetY.ConvertToPx(), 0);
    EXPECT_EQ(scrollState, ScrollState::IDLE);

    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(offsetY.ConvertToPx(), 0);
    EXPECT_EQ(scrollState, ScrollState::IDLE);

    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_NONE);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(offsetY.ConvertToPx(), -ITEM_HEIGHT);
    EXPECT_EQ(scrollState, ScrollState::IDLE);

    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_AXIS);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR);
    pattern_->scrollStop_ = true;
    FlushLayoutTask(frameNode_);
    pattern_->SetScrollAbort(true);
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_NONE);
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: GridEventTestNg002
 * @tc.desc: Test scroll callback
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg002, TestSize.Level1)
{
    int32_t startIndex;
    int32_t endIndex;
    auto event = [&startIndex, &endIndex](int32_t start, int32_t end) {
        startIndex = start;
        endIndex = end;
    };
    Create([event](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScrollIndex(event);
        CreateFixedItem(12);
    });

    UpdateCurrentOffset(-ITEM_HEIGHT);
    EXPECT_EQ(startIndex, 0);
    EXPECT_EQ(endIndex, 9);

    UpdateCurrentOffset(-ITEM_HEIGHT);
    EXPECT_EQ(startIndex, 2);
    EXPECT_EQ(endIndex, 11);
}

/**
 * @tc.name: GridEventTestNg003
 * @tc.desc: Test scroll callback
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg003, TestSize.Level1)
{
    bool isTrigger = false;
    auto event = [&isTrigger]() { isTrigger = true; };
    Create([event](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnReachStart(event);
        CreateFixedItem(12);
    });
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    UpdateCurrentOffset(-ITEM_HEIGHT);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    UpdateCurrentOffset(ITEM_HEIGHT);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_AXIS);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_AXIS);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_JUMP);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_JUMP);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_BAR);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_BAR_FLING);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR_FLING);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
}

/**
 * @tc.name: GridEventTestNg004
 * @tc.desc: Test scroll callback
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg004, TestSize.Level1)
{
    bool isTrigger = false;
    auto event = [&isTrigger]() { isTrigger = true; };
    Create([event](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnReachEnd(event);
        CreateFixedItem(10);
    });

    isTrigger = false;
    UpdateCurrentOffset(-ITEM_HEIGHT * 3);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    UpdateCurrentOffset(ITEM_HEIGHT);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_AXIS);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_AXIS);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION_SPRING);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_JUMP);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_JUMP);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_BAR);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_BAR_FLING);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_BAR_FLING);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);

    isTrigger = false;
    pattern_->UpdateCurrentOffset(-ITEM_HEIGHT, SCROLL_FROM_NONE);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isTrigger);
    isTrigger = false;
    pattern_->UpdateCurrentOffset(ITEM_HEIGHT, SCROLL_FROM_NONE);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isTrigger);
}

/**
 * @tc.name: GridEventTestNg005
 * @tc.desc: Verify onScrollStart, onScrollStop callback
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg005, TestSize.Level1)
{
    bool isScrollStartCalled = false;
    bool isScrollStopCalled = false;
    auto scrollStart = [&isScrollStartCalled]() { isScrollStartCalled = true; };
    auto scrollStop = [&isScrollStopCalled]() { isScrollStopCalled = true; };
    Create([scrollStart, scrollStop](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScrollStart(scrollStart);
        model.SetOnScrollStop(scrollStop);
        CreateFixedItem(10);
    });

    pattern_->OnScrollCallback(100.f, SCROLL_FROM_START);
    EXPECT_TRUE(isScrollStartCalled);

    pattern_->SetScrollAbort(true);
    pattern_->OnScrollEndCallback();
    EXPECT_FALSE(isScrollStopCalled);
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isScrollStopCalled);
}

/**
 * @tc.name: GridEventTestNg006
 * @tc.desc: Test other condition
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg006, TestSize.Level1)
{
    auto event = [](Dimension, ScrollState) {
        ScrollFrameResult result;
        return result;
    };
    Create([event](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScrollFrameBegin(event);
        CreateFixedItem(10);
    });

    auto scrollableEvent = pattern_->GetScrollableEvent();
    ASSERT_NE(scrollableEvent, nullptr);
    EXPECT_NE(scrollableEvent->GetScrollable()->callback_, nullptr);
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: GridEventTestNg007
 * @tc.desc: Emulate consecutive swipes
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GridEventTestNg007, TestSize.Level1)
{
    bool isScrollStartCalled = false;
    bool isScrollStopCalled = false;
    auto scrollStart = [&isScrollStartCalled]() { isScrollStartCalled = true; };
    auto scrollStop = [&isScrollStopCalled]() { isScrollStopCalled = true; };
    Create([scrollStart, scrollStop](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScrollStart(scrollStart);
        model.SetOnScrollStop(scrollStop);
        CreateFixedItem(10);
    });

    pattern_->OnScrollCallback(100.f, SCROLL_FROM_START);
    EXPECT_TRUE(isScrollStartCalled);

    // reset
    isScrollStartCalled = false;

    pattern_->OnScrollEndCallback();
    EXPECT_TRUE(pattern_->scrollStop_);
    EXPECT_FALSE(isScrollStopCalled);

    // scrollStart again, before layout
    pattern_->OnScrollCallback(100.f, SCROLL_FROM_START);
    EXPECT_FALSE(isScrollStartCalled);
    EXPECT_FALSE(pattern_->scrollStop_);
    EXPECT_FALSE(isScrollStopCalled);

    pattern_->OnScrollEndCallback();
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isScrollStopCalled);
}

/**
 * @tc.name: VerticalGridWithoutScrollBar001
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll Without Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithoutScrollBarWithoutAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set seven kinds of scroll events: onScroll, onWillScroll, onDidScroll,
     *                   onScrollStart, onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onWillScroll, onDidScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd,
                          this](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll call back function should be triggered
     */
    pattern_->ScrollTo(ITEM_HEIGHT * 1);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnWillScrollCallBack);
    EXPECT_TRUE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY, willScrollOffset);
    EXPECT_EQ(offsetY, didScrollOffset);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithoutScrollBar002
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll Without Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithoutScrollBarWithoutAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd, this](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll and onReachEnd should be triggered
     */
    pattern_->ScrollTo(ITEM_HEIGHT * 9);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithoutScrollBarWithAnimation001
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll Without Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithoutScrollBarWithAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;
    /**
     * @tc.steps: step2. Scroll to grid's center with animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: OnScroll OnScrollStart and onScrollStop call back functions should be triggered
     */
    pattern_->AnimateTo(-5*ITEM_HEIGHT, 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate(); pattern_->SetScrollAbort(false); pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-5*ITEM_HEIGHT, SCROLL_FROM_ANIMATION);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithoutScrollBarWithAnimation002
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll Without Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithoutScrollBarWithAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end without animation, and check the
     *                   five kinds of call back functions is triggered or not.
     * @tc.expected:All call back functions except onReachStart should be triggered
     */
    pattern_->AnimateTo(-20*ITEM_HEIGHT, 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-20*ITEM_HEIGHT, SCROLL_FROM_ANIMATION);

    EXPECT_EQ(pattern_->GetTotalOffset(), ITEM_HEIGHT * 20);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithoutScrollBarWithoutAnimation001
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll Without Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithoutScrollBarWithoutAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll call back function should be triggered
     */
    pattern_->ScrollTo(ITEM_WIDTH * 1);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithoutScrollBarWithoutAnimation002
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll Without Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithoutScrollBarWithoutAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

   /**
     * @tc.steps: step2. Scroll to grid's end position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll and onReachEnd should be triggered
     */
    pattern_->ScrollTo(ITEM_WIDTH * 10);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithoutScrollBarWithAnimation001
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll Without Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithoutScrollBarWithAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;
    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center with animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: OnScroll OnScrollStart and onScrollStop call back functions should be triggered
     */
    pattern_->AnimateTo(-5*ITEM_WIDTH, 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-5*ITEM_WIDTH, SCROLL_FROM_ANIMATION);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithoutScrollBarWithAnimation002
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll Without Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithoutScrollBarWithAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;
    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    CreateWithNoProxy([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end without animation, and check the
     *                   five kinds of call back functions is triggered or not.
     * @tc.expected:All call back functions except onReachStart should be triggered
     */
    pattern_->AnimateTo(-10*ITEM_WIDTH, 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-10*ITEM_WIDTH, SCROLL_FROM_ANIMATION);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithScrollBarWithoutAnimation001
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll With Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithScrollBarWithoutAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd, this](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll call back function should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->JumpTo(5, false, ScrollAlign::CENTER, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithScrollBarWithoutAnimation002
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll With Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithScrollBarWithoutAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd, this](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll and onReachEnd should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->JumpTo(19, false, ScrollAlign::END, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_FALSE(isOnScrollStartCallBack); EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithScrollBarWithAnimation001
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll With Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithScrollBarWithAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center with animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: OnScroll OnScrollStart and onScrollStop call back functions should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->AnimateTo(Dimension(-5*ITEM_HEIGHT, DimensionUnit::PX), 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-5*ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: VerticalGridWithScrollBarWithAnimation002
 * @tc.desc: Test Grid(Axis::VERTICAL) Scroll With Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, VerticalGridWithScrollBarWithAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end without animation, and check the
     *                   five kinds of call back functions is triggered or not.
     * @tc.expected:All call back functions except onReachStart should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->AnimateTo(Dimension(-10*ITEM_HEIGHT, DimensionUnit::PX), 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-10*ITEM_HEIGHT, SCROLL_FROM_ANIMATION_CONTROLLER);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack); EXPECT_TRUE(isOnScrollStartCallBack); EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack); EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithScrollBarWithoutAnimation001
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll With Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithScrollBarWithoutAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset; scrollState = state; isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll call back function should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->JumpTo(5, false, ScrollAlign::CENTER, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithScrollBarWithoutAnimation002
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll With Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithScrollBarWithoutAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step3. Scroll to grid's end position without animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: Only onScroll and onReachEnd should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->JumpTo(19, false, ScrollAlign::END, 3);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_FALSE(isOnScrollStartCallBack);
    EXPECT_FALSE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithScrollBarWithAnimation001
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll With Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithScrollBarWithAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's center with animation, and check the
     *                   five kinds of call back functions are triggered or not.
     * @tc.expected: OnScroll OnScrollStart and onScrollStop call back functions should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->AnimateTo(Dimension(-5 * ITEM_WIDTH, DimensionUnit::PX), 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-5 * ITEM_WIDTH, SCROLL_FROM_ANIMATION_CONTROLLER);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnScrollStartCallBack);
    EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_FALSE(isOnReachEndCallBack);
}

/**
 * @tc.name: HorizontalGridWithScrollBarWithAnimation002
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll With Scroll Bar With Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, HorizontalGridWithScrollBarWithAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set five kinds of scroll events: onScroll, onScrollStart,
     *                   onScrollStop, onReachStart, onReachEnd
     */
    bool isOnScrollCallBack = false;
    bool isOnScrollStartCallBack = false;
    bool isOnScrollStopCallBack = false;
    bool isOnReachStartCallBack = false;
    bool isOnReachEndCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    auto onScrollStart = [&isOnScrollStartCallBack]() { isOnScrollStartCallBack = true; };
    auto onScrollStop = [&isOnScrollStopCallBack]() { isOnScrollStopCallBack = true; };
    auto onReachStart = [&isOnReachStartCallBack]() { isOnReachStartCallBack = true; };
    auto onReachEnd = [&isOnReachEndCallBack]() { isOnReachEndCallBack = true; };

    Create([onScroll, onScrollStart, onScrollStop, onReachStart, onReachEnd](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetOnScrollStart(onScrollStart);
        model.SetOnScrollStop(onScrollStop);
        model.SetOnReachStart(onReachStart);
        model.SetOnReachEnd(onReachEnd);
        model.SetSupportAnimation(true);
        CreateFixedItem(20);
    });
    EXPECT_TRUE(isOnReachStartCallBack);
    isOnReachStartCallBack = false;

    /**
     * @tc.steps: step2. Scroll to grid's end without animation, and check the
     *                   five kinds of call back functions is triggered or not.
     * @tc.expected:All call back functions except onReachStart should be triggered
     */
    auto controller = pattern_->positionController_;
    controller->AnimateTo(Dimension(-10 * ITEM_WIDTH, DimensionUnit::PX), 1.f, Curves::LINEAR, false);
    pattern_->StopAnimate();
    pattern_->SetScrollAbort(false);
    pattern_->OnScrollEndCallback();
    pattern_->UpdateCurrentOffset(-10 * ITEM_WIDTH, SCROLL_FROM_ANIMATION_CONTROLLER);

    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnScrollStartCallBack);
    EXPECT_TRUE(isOnScrollStopCallBack);
    EXPECT_FALSE(isOnReachStartCallBack);
    EXPECT_TRUE(isOnReachEndCallBack);
}

/**
 * @tc.name: ScrollBy
 * @tc.desc: Test ScrollBy
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollBy001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        CreateFixedItem(10);
    });

    /**
     * @tc.steps: step1. ScrollBy information function call
     * @tc.expected: scroll to the node
     */
    pattern_->ScrollBy(1.2);
    EXPECT_TRUE(pattern_->UpdateCurrentOffset(-1.2, 3));
    pattern_->OnAnimateStop();
    EXPECT_TRUE(pattern_->scrollStop_);

    pattern_->OutBoundaryCallback();
    EXPECT_FALSE(pattern_->IsOutOfBoundary());

    /**
     * @tc.steps: step2. DumpAdvanceInfo information function call
     */
    pattern_->DumpAdvanceInfo();

    /**
     * @tc.steps: step3. Test AnimateTo function
     * @tc.expected: pattern_->isAnimationStop_ is false
     */
    pattern_->AnimateTo(1.5, 1.f, Curves::LINEAR, false);
    EXPECT_FALSE(pattern_->isAnimationStop_);
}

/**
 * @tc.name: GetGridItemAnimatePos001
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateItem(30, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    });

    int32_t targetIndex = 20;
    ScrollAlign align = ScrollAlign::START;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, 0.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos002
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos002, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateItem(30, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    });

    int32_t targetIndex = 20;
    ScrollAlign align = ScrollAlign::CENTER;

    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, -400.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos003
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos003, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateItem(30, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    });

    int32_t targetIndex = 20;
    ScrollAlign align = ScrollAlign::END;

    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, -800.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos004
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos004, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateFixedItem(100);
    });

    pattern_->ScrollToIndex(30, true, ScrollAlign::AUTO);
    AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());

    int32_t targetIndex = 60;
    ScrollAlign align = ScrollAlign::AUTO;

    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 0;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 10;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, 0.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos005
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos005, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateItem(100, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    });

    int32_t targetIndex = 30;
    ScrollAlign align = ScrollAlign::AUTO;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 60;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 0;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 300;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, 0.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos006
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos006, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateFixedItem(1);
    });

    int32_t targetIndex = 0;
    ScrollAlign align = ScrollAlign::START;
    targetIndex = 0;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, 0.0f);
}

/**
 * @tc.name: GetGridItemAnimatePos007
 * @tc.desc: Test GetGridItemAnimatePos
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetGridItemAnimatePos007, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateItem(100, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    });

    int32_t targetIndex = 20;
    ScrollAlign align = ScrollAlign::AUTO;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 30;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);

    targetIndex = 10;
    pattern_->ScrollToIndex(targetIndex, true, align);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->finalPosition_, 0.0f);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll001
 * @tc.desc: Test Grid(Axis::Vertical) Scroll, onWillScroll and onDidScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset * 2;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset * 3;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to grid's end position without animation, and check the
     *                   three kinds of call back functions are triggered or not.
     * @tc.expected: All functions should be triggered
     */
    pattern_->ScrollTo(ITEM_HEIGHT * 5);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_EQ(offsetY.Value(), ITEM_HEIGHT * 5);
    EXPECT_EQ(willScrollOffset.Value(), ITEM_HEIGHT * 5 * 2);
    EXPECT_EQ(didScrollOffset.Value(), ITEM_HEIGHT * 5 * 3);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll002
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll, onWillScroll and onDidScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset * 2;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset * 3;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to grid's end position without animation, and check the
     *                   three kinds of call back functions are triggered or not.
     * @tc.expected: All functions should be triggered
     */
    pattern_->ScrollTo(ITEM_WIDTH * 5);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnWillScrollCallBack);
    EXPECT_TRUE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY.Value(), ITEM_WIDTH * 5);
    EXPECT_EQ(willScrollOffset.Value(), ITEM_WIDTH * 5 * 2);
    EXPECT_EQ(didScrollOffset.Value(), ITEM_WIDTH * 5 * 3);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll003
 * @tc.desc: Test Grid(Axis::HORIZONTAL) Scroll Without Scroll Bar Without Animation
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset * 2;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset * 3;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetRowsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to a large offset beyond the total height of the grid without animation, and check
     *                   three kinds of call back functions are triggered and values are correct.
     * @tc.expected: All functions should be triggered
     */
    pattern_->ScrollBy(ITEM_WIDTH);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnWillScrollCallBack);
    EXPECT_TRUE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY.Value(), ITEM_WIDTH);
    EXPECT_EQ(willScrollOffset.Value(), ITEM_WIDTH * 2);
    EXPECT_EQ(didScrollOffset.Value(), ITEM_WIDTH * 3);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll004
 * @tc.desc: Test Grid(Axis::Vertical) Scroll, onWillScroll and onDidScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to a large offset beyond the total height of the grid without animation, and check
     *                   three kinds of call back functions are triggered and values are correct.
     * @tc.expected: All functions should be triggered
     */
    pattern_->ScrollTo(ITEM_HEIGHT * 10);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isOnScrollCallBack);
    EXPECT_TRUE(isOnWillScrollCallBack);
    EXPECT_TRUE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY.Value(), ITEM_HEIGHT * 6);
    EXPECT_EQ(willScrollOffset.Value(), ITEM_HEIGHT * 10);
    EXPECT_EQ(didScrollOffset.Value(), ITEM_HEIGHT * 6);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll005
 * @tc.desc: Test Grid(Axis::Vertical) Scroll, onWillScroll and onDidScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to a negative position without animation, and check
     *                   three kinds of call back functions are triggered and values are correct.
     * @tc.expected: All functions should be triggered
     */
    pattern_->ScrollTo(-ITEM_HEIGHT);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isOnScrollCallBack);
    EXPECT_FALSE(isOnWillScrollCallBack);
    EXPECT_FALSE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY.Value(), 0);
    EXPECT_EQ(willScrollOffset.Value(), 0);
    EXPECT_EQ(didScrollOffset.Value(), 0);
    EXPECT_EQ(scrollState, willScrollState);
    EXPECT_EQ(scrollState, didScrollState);
}

/**
 * @tc.name: onWillScrollAndOnDidScroll006
 * @tc.desc: Test Grid(Axis::Vertical) Scroll, onWillScroll and onDidScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, onWillScrollAndOnDidScroll006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set three kinds of scroll events: onScroll, onWillScroll, onDidScroll
     * @tc.expected: creat grid
     */
    bool isOnScrollCallBack = false;
    bool isOnWillScrollCallBack = false;
    bool isOnDidScrollCallBack = false;

    Dimension offsetY;
    ScrollState scrollState;
    auto onScroll = [&offsetY, &scrollState, &isOnScrollCallBack](Dimension offset, ScrollState state) {
        offsetY = offset;
        scrollState = state;
        isOnScrollCallBack = true;
    };
    Dimension willScrollOffset;
    ScrollState willScrollState;
    auto onWillScroll = [&willScrollOffset, &willScrollState, &isOnWillScrollCallBack](
                            Dimension offset, ScrollState state) {
        willScrollOffset = offset;
        willScrollState = state;
        isOnWillScrollCallBack = true;
    };
    Dimension didScrollOffset;
    ScrollState didScrollState;
    auto onDidScroll = [&didScrollOffset, &didScrollState, &isOnDidScrollCallBack](
                           Dimension offset, ScrollState state) {
        didScrollOffset = offset;
        didScrollState = state;
        isOnDidScrollCallBack = true;
    };

    CreateWithNoProxy([onScroll](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetOnScroll(onScroll);
        model.SetEdgeEffect(EdgeEffect::SPRING, true);
        CreateFixedItem(20);
    });
    eventHub_->SetOnWillScroll(onWillScroll);
    eventHub_->SetOnDidScroll(onDidScroll);

    /**
     * @tc.steps: step2. Scroll to a negative position without animation, and check
     *                   three kinds of call back functions are triggered and values are correct.
     * @tc.expected: All functions should be triggered
     */
    pattern_->OnScrollCallback(ITEM_HEIGHT, SCROLL_FROM_UPDATE);
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(isOnScrollCallBack);
    EXPECT_TRUE(isOnWillScrollCallBack);
    EXPECT_FALSE(isOnDidScrollCallBack);
    EXPECT_EQ(offsetY.Value(), 0);
    EXPECT_EQ(willScrollOffset.Value(), -ITEM_HEIGHT);
    EXPECT_EQ(didScrollOffset.Value(), 0);
    EXPECT_EQ(willScrollState, ScrollState::SCROLL);
    EXPECT_EQ(scrollState, didScrollState);
}
} // namespace OHOS::Ace::NG

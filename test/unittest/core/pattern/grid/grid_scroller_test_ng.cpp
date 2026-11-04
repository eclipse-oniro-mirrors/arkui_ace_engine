/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>
#include "core/components_ng/pattern/grid/grid_custom/grid_custom_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_with_options_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/scroll/scroll_edge_effect.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

namespace OHOS::Ace::NG {
class GridScrollerTestNg : public GridTestNg, public testing::WithParamInterface<bool> {};

namespace {
class FastScrollRecordingGridPattern final : public GridPattern {
    DECLARE_ACE_TYPE(FastScrollRecordingGridPattern, GridPattern);

public:
    void ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, std::optional<float> offset) override
    {
        ++legacyCalls_;
        lastIndex_ = index;
        lastSmooth_ = smooth;
        lastAlign_ = align;
        lastOffset_ = offset;
        lastMode_ = FastScrollMode::NONE;
    }

    void ScrollToIndexFastScroll(int32_t index, bool smooth, ScrollAlign align,
        std::optional<float> offset, FastScrollMode mode) override
    {
        ++fastCalls_;
        lastIndex_ = index;
        lastSmooth_ = smooth;
        lastAlign_ = align;
        lastOffset_ = offset;
        lastMode_ = mode;
    }

    int32_t legacyCalls_ = 0;
    int32_t fastCalls_ = 0;
    int32_t lastIndex_ = 0;
    bool lastSmooth_ = false;
    ScrollAlign lastAlign_ = ScrollAlign::NONE;
    std::optional<float> lastOffset_;
    FastScrollMode lastMode_ = FastScrollMode::NONE;
};
} // namespace

/**
 * @tc.name: FastScrollModelDispatch
 * @tc.desc: The Grid model uses the real controller and preserves mode, AUTO and signed offsets.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, FastScrollModelDispatch, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<FastScrollRecordingGridPattern>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    for (const auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        GridModelNG::SetScrollToIndexWithOptions(
            frameNode.GetRawPtr(), 90, true, ScrollAlign::NONE, -13.0f, mode);
        EXPECT_EQ(pattern->lastIndex_, 90);
        EXPECT_TRUE(pattern->lastSmooth_);
        EXPECT_EQ(pattern->lastAlign_, ScrollAlign::AUTO);
        EXPECT_EQ(pattern->lastOffset_, std::optional<float>(-13.0f));
        EXPECT_EQ(pattern->lastMode_, mode);
    }
    EXPECT_EQ(pattern->fastCalls_, 2);
    EXPECT_EQ(pattern->legacyCalls_, 0);

    GridModelNG::SetScrollToIndexWithOptions(frameNode.GetRawPtr(), LAST_ITEM, true,
        ScrollAlign::END, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_EQ(pattern->fastCalls_, 3);
    EXPECT_EQ(pattern->lastIndex_, LAST_ITEM);
    EXPECT_EQ(pattern->lastAlign_, ScrollAlign::END);

    auto controller = pattern->GetOrCreatePositionController();
    controller->ScrollToIndex(4, false, ScrollAlign::NONE, std::nullopt);
    EXPECT_EQ(pattern->fastCalls_, 3);
    EXPECT_EQ(pattern->legacyCalls_, 1);
    EXPECT_EQ(pattern->lastMode_, FastScrollMode::NONE);
    EXPECT_EQ(pattern->lastIndex_, 4);
    EXPECT_EQ(pattern->lastAlign_, ScrollAlign::AUTO);
    EXPECT_FALSE(pattern->lastOffset_.has_value());
}

/**
 * @tc.name: FastScrollModelLegacyFallback
 * @tc.desc: Invalid, disabled and non-animated options use the old virtual call; null is safe.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, FastScrollModelLegacyFallback, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<FastScrollRecordingGridPattern>();
    auto frameNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    for (const auto mode : { FastScrollMode::NONE, static_cast<FastScrollMode>(-1), static_cast<FastScrollMode>(3) }) {
        GridModelNG::SetScrollToIndexWithOptions(
            frameNode.GetRawPtr(), 8, true, ScrollAlign::CENTER, 7.0f, mode);
        EXPECT_EQ(pattern->lastAlign_, ScrollAlign::CENTER);
        EXPECT_EQ(pattern->lastOffset_, std::optional<float>(7.0f));
    }
    for (const auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        GridModelNG::SetScrollToIndexWithOptions(frameNode.GetRawPtr(), 8, false, ScrollAlign::END, std::nullopt, mode);
        EXPECT_FALSE(pattern->lastSmooth_);
        EXPECT_EQ(pattern->lastAlign_, ScrollAlign::END);
        EXPECT_FALSE(pattern->lastOffset_.has_value());
    }
    EXPECT_EQ(pattern->fastCalls_, 0);
    EXPECT_EQ(pattern->legacyCalls_, 5);
    GridModelNG::SetScrollToIndexWithOptions(nullptr, 8, true, ScrollAlign::AUTO, 0.0f, FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: ScrollToIndex_Align001
 * @tc.desc: Test ScrollToIndex with ScrollAlign::START
 * @tc.desc: Scroll to (first/inView/outOfView/last) item (with/without) animation
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Align001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    // Scroll to the first item
    bool smooth = GetParam();
    ScrollAlign align = ScrollAlign::START;
    ScrollToIndex(0, smooth, align);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_TRUE(pattern_->IsAtTop());

    // Scroll to the first item in row in view
    ScrollToIndex(2, smooth, align);
    EXPECT_TRUE(TickPosition(-100.0f));

    // Scroll to the last item in row in view
    ScrollToIndex(5, smooth, align);
    EXPECT_TRUE(TickPosition(-200.0f));

    // Scroll to the item out of view
    ScrollToIndex(12, smooth, align);
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(pattern_->IsAtBottom());

    // Scroll back
    ScrollToIndex(3, smooth, align);
    EXPECT_TRUE(TickPosition(-100.0f));

    // Scroll with invalid index
    ScrollToIndex(-100, smooth, align);
    EXPECT_TRUE(TickPosition(-100.0f));
    ScrollToIndex(100, smooth, align);
    EXPECT_TRUE(TickPosition(-100.0f));

    // Scroll to the last item
    ScrollToIndex(LAST_ITEM, smooth, align);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-100.0f));
        EXPECT_FALSE(pattern_->IsAtBottom());
    } else {
        EXPECT_TRUE(TickPosition(-600.0f));
        EXPECT_TRUE(pattern_->IsAtBottom());
    }
}

/**
 * @tc.name: ScrollToIndex_Align002
 * @tc.desc: Test ScrollToIndex with ScrollAlign::CENTER
 * @tc.desc: Scroll to (first/inView/outOfView/last) item (with/without) animation
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Align002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    // Scroll to the first item
    bool smooth = GetParam();
    ScrollAlign align = ScrollAlign::CENTER;
    ScrollToIndex(0, smooth, align);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_TRUE(pattern_->IsAtTop());

    // Scroll to the first item in row in view
    ScrollToIndex(4, smooth, align);
    EXPECT_TRUE(TickPosition(-50.0f));

    // Scroll to the last item in row in view
    ScrollToIndex(7, smooth, align);
    EXPECT_TRUE(TickPosition(-150.0f));

    // Scroll to the item out of view
    ScrollToIndex(12, smooth, align);
    EXPECT_TRUE(TickPosition(-450.0f));

    // Scroll back
    ScrollToIndex(5, smooth, align);
    EXPECT_TRUE(TickPosition(-50.0f));

    // Scroll to the last item
    ScrollToIndex(LAST_ITEM, smooth, align);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-50.0f));
        EXPECT_FALSE(pattern_->IsAtBottom());
    } else {
        EXPECT_TRUE(TickPosition(-600.0f));
        EXPECT_TRUE(pattern_->IsAtBottom());
    }
}

/**
 * @tc.name: ScrollToIndex_Align003
 * @tc.desc: Test ScrollToIndex with ScrollAlign::END
 * @tc.desc: Scroll to (first/inView/outOfView/last) item (with/without) animation
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Align003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    // Scroll to the first item
    bool smooth = GetParam();
    ScrollAlign align = ScrollAlign::END;
    ScrollToIndex(0, smooth, align);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_TRUE(pattern_->IsAtTop());

    // Scroll to the first item in row in view
    ScrollToIndex(4, smooth, align);
    EXPECT_TRUE(TickPosition(0));

    // Scroll to the first item in row out of view
    ScrollToIndex(12, smooth, align);
    EXPECT_TRUE(TickPosition(-300.0f));

    // Scroll to the last item in row out of view
    ScrollToIndex(15, smooth, align);
    EXPECT_TRUE(TickPosition(-400.0f));

    // Scroll back
    ScrollToIndex(5, smooth, align);
    EXPECT_TRUE(TickPosition(0));

    // Scroll to the last item
    ScrollToIndex(LAST_ITEM, smooth, align);
    if (smooth) {
        EXPECT_TRUE(TickPosition(0));
        EXPECT_FALSE(pattern_->IsAtBottom());
    } else {
        EXPECT_TRUE(TickPosition(-600.0f));
        EXPECT_TRUE(pattern_->IsAtBottom());
    }
}

/**
 * @tc.name: ScrollToIndex_Align004
 * @tc.desc: Test ScrollToIndex with ScrollAlign::AUTO
 * @tc.desc: Scroll to (first/inView/outOfView/last) item (with/without) animation
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Align004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    // Scroll to the first item
    bool smooth = GetParam();
    ScrollAlign align = ScrollAlign::AUTO;
    ScrollToIndex(0, smooth, align);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_TRUE(pattern_->IsAtTop());

    // Scroll to the first item in row in view
    ScrollToIndex(4, smooth, align);
    EXPECT_TRUE(TickPosition(0));

    // Scroll to the first item in row out of view
    ScrollToIndex(12, smooth, align);
    EXPECT_TRUE(TickPosition(-300.0f));

    // Scroll to the last item in row out of view
    ScrollToIndex(15, smooth, align);
    EXPECT_TRUE(TickPosition(-400.0f));

    // Scroll back
    ScrollToIndex(5, smooth, align);
    EXPECT_TRUE(TickPosition(-200.0f));

    // Scroll to the last item
    ScrollToIndex(LAST_ITEM, smooth, align);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-200.0f));
        EXPECT_FALSE(pattern_->IsAtBottom());
    } else {
        EXPECT_TRUE(TickPosition(-600.0f));
        EXPECT_TRUE(pattern_->IsAtBottom());
    }
}

/**
 * @tc.name: ScrollToIndex_Gap001
 * @tc.desc: Test ScrollToIndex with Gap
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Gap001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetColumnsGap(Dimension(COL_GAP));
    model.SetRowsGap(Dimension(ROW_GAP));
    CreateFixedItems(20);
    CreateDone();

    // ScrollAlign::START
    bool smooth = GetParam();
    ScrollToIndex(2, smooth, ScrollAlign::START);
    EXPECT_TRUE(TickPosition(-105.0f));

    // ScrollAlign::CENTER
    ScrollToIndex(7, smooth, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-165.0f));

    // ScrollAlign::END
    ScrollToIndex(12, smooth, ScrollAlign::END);
    EXPECT_TRUE(TickPosition(-330.0f));

    // ScrollAlign::AUTO
    ScrollToIndex(LAST_ITEM, smooth, ScrollAlign::AUTO);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-330.0f));
        EXPECT_FALSE(pattern_->IsAtBottom());
    } else {
        EXPECT_TRUE(TickPosition(-645.0f));
        EXPECT_TRUE(pattern_->IsAtBottom());
    }
}

/**
 * @tc.name: ScrollToIndex_Gap002
 * @tc.desc: Test ScrollToIndex with big gap(lineHeight + gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Gap002, TestSize.Level1)
{
    /**
     * @tc.cases: Set BIG_ROW_GAP, ScrollTo index:5, text ScrollAlign::AUTO
     * @tc.expected: Each test scroll the correct distance
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(BIG_ROW_GAP));
    CreateFixedItems(15);
    CreateDone();

    bool smooth = GetParam();
    int32_t index = 5;
    ScrollToIndex(index, smooth, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-(ITEM_MAIN_SIZE * 3 + BIG_ROW_GAP * 2 - HEIGHT)));
    /**
     * @tc.cases: use ScrollTo to make item 5 in the last line, ScrollTo index:5, text ScrollAlign::AUTO
     * @tc.expected: scrollToIndex don't change grid offset
     */
    auto autoPosition = ITEM_MAIN_SIZE * 3 + BIG_ROW_GAP * 2 - HEIGHT + ITEM_MAIN_SIZE;
    ScrollTo(autoPosition);
    ScrollToIndex(index, smooth, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-autoPosition));
}

/**
 * @tc.name: ScrollToIndex_Gap003
 * @tc.desc: Test ScrollToIndex with big gap(lineHeight + gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Gap003, TestSize.Level1)
{
    /**
     * @tc.cases: Set BIG_ROW_GAP, ScrollTo index, text each ScrollAlign::CENTER
     * @tc.expected: Each test scroll the correct distance
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(BIG_ROW_GAP));
    CreateFixedItems(15);
    CreateDone();

    auto endPosition = ITEM_MAIN_SIZE * 3 + BIG_ROW_GAP * 2 - HEIGHT;
    bool smooth = GetParam();
    ScrollToIndex(5, smooth, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-(endPosition + (HEIGHT - ITEM_MAIN_SIZE) / 2)));
}

/**
 * @tc.name: ScrollToIndex_Gap004
 * @tc.desc: Test ScrollToIndex with medium gap(3*lineHeight + 3*gap > mainSize)
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Gap004, TestSize.Level1)
{
    /**
     * @tc.cases: Set MEDIUM_ROW_GAP and scroll to specified position, ScrollTo index, text each ScrollAlign::AUTO
     * @tc.expected: Each test scroll the correct distance
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(MEDIUM_ROW_GAP));
    CreateFixedItems(15);
    CreateDone();

    auto position = ITEM_MAIN_SIZE + 5 * COL_GAP;
    ScrollTo(position);
    EXPECT_TRUE(TickPosition(-position));

    bool smooth = GetParam();
    ScrollToIndex(6, smooth, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-position));
}

/**
 * @tc.name: ScrollToIndex_Irregular001
 * @tc.desc: Test ScrollToIndex with irregular
 * @tc.desc: The irregularIndexes index take up a whole row
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Irregular001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = { 6, 1, 3, 4, 5, 0 };
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    model.SetColumnsGap(Dimension(COL_GAP));
    model.SetRowsGap(Dimension(ROW_GAP));
    CreateFixedItems(10);
    CreateDone();

    bool smooth = GetParam();
    ScrollToIndex(1, smooth, ScrollAlign::START);
    EXPECT_TRUE(TickPosition(-105.0f));

    ScrollToIndex(3, smooth, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-165.0f));

    ScrollToIndex(5, smooth, ScrollAlign::END);
    EXPECT_TRUE(TickPosition(-225.0f));

    ScrollToIndex(LAST_ITEM, smooth, ScrollAlign::AUTO);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-225.0f));
    } else {
        EXPECT_TRUE(TickPosition(-435.0f));
    }
}

/**
 * @tc.name: ScrollToIndex_Irregular002
 * @tc.desc: Test ScrollToIndex with irregular
 * @tc.desc: OnGetIrregularSizeByIndex will set item (rows/cols) that in irregularIndexes
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_Irregular002, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = { 6, 1, 3, 4, 5, 0 };
    GetSizeByIndex onGetIrregularSizeByIndex = [](int32_t index) {
        if (index == 3) {
            return GridItemSize { 1, 2 };
        }
        return GridItemSize { 1, 4 };
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    model.SetColumnsGap(Dimension(COL_GAP));
    model.SetRowsGap(Dimension(ROW_GAP));
    CreateFixedItems(10);
    CreateDone();

    bool smooth = GetParam();
    ScrollToIndex(1, smooth, ScrollAlign::START);
    EXPECT_TRUE(TickPosition(-105.0f));

    ScrollToIndex(3, smooth, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-60.0f));

    ScrollToIndex(5, smooth, ScrollAlign::END);
    EXPECT_TRUE(TickPosition(-120.0f));
}

/**
 * @tc.name: ScrollToIndex_ExtraOffset001
 * @tc.desc: Test ScrollToIndex with extraOffset
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToIndex_ExtraOffset001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    /**
     * @tc.steps: step1. extraOffset < 0
     */
    bool smooth = GetParam();
    float extraOffset = -100.0f;
    ScrollToIndex(2, smooth, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(0));

    ScrollToIndex(17, smooth, ScrollAlign::START, extraOffset);
    FlushUITasks();
    EXPECT_TRUE(TickPosition(-600.0f));

    ScrollToIndex(17, smooth, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-400.0f));

    ScrollToIndex(LAST_ITEM, smooth, ScrollAlign::END, extraOffset);
    if (smooth) {
        EXPECT_TRUE(TickPosition(-300.0f));
    } else {
        EXPECT_TRUE(TickPosition(-500.0f));
    }

    /**
     * @tc.steps: step2. Reset position
     */
    ScrollTo(0);
    EXPECT_TRUE(TickPosition(0));

    /**
     * @tc.steps: step3. extraOffset > 0
     */
    extraOffset = 100.0f;
    ScrollToIndex(2, smooth, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(-200.0f));

    ScrollToIndex(2, smooth, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(0));

    ScrollToIndex(17, smooth, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-600.0f));

    ScrollToIndex(LAST_ITEM, smooth, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-600.0f));
}

/**
 * @tc.name: AnimateTo001
 * @tc.desc: Test AnimateTo Function.
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, AnimateTo001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

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
    AnimateTo(Dimension(1000.0f), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-600.0f));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo002
 * @tc.desc: Test AnimateTo with duration animation
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, AnimateTo002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. AnimateTo the position in the scroll
     * @tc.expected: AnimateTo the position
     */
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 1000.0f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can not over scroll
     */
    AnimateTo(Dimension(1000.0f), 1000.0f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(-600.0f));

    /**
     * @tc.steps: step3. AnimateTo the top
     * @tc.expected: AnimateTo the top
     */
    AnimateTo(Dimension(0), 1000.0f, Curves::EASE, false);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo003
 * @tc.desc: Test unScrollable, can not scroll
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, AnimateTo003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(4);
    CreateDone();

    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo004
 * @tc.desc: Test scroll position greater than HEIGHT
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, AnimateTo004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();

    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE * 5), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE * 5));
}

/**
 * @tc.name: AnimateTo005
 * @tc.desc: Test unScrollable, set col/row template at same time, can not scroll
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, AnimateTo005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetRowsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_FALSE(pattern_->isConfigScrollable_);

    bool smooth = GetParam();
    AnimateTo(Dimension(ITEM_MAIN_SIZE * 5), 0, nullptr, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollBy001
 * @tc.desc: Test ScrollBy
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollBy001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. ScrollBy the position
     * @tc.expected: ScrollBy the position
     */
    bool smooth = GetParam();
    ScrollBy(0, ITEM_MAIN_SIZE, smooth);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE));

    /**
     * @tc.steps: step2. ScrollBy the position over the scroll
     * @tc.expected: ScrollBy the bottom, can not over scroll
     */
    ScrollBy(0, 1000.0f, smooth);
    EXPECT_TRUE(TickPosition(-600.0f));

    /**
     * @tc.steps: step3. ScrollBy the position 0
     * @tc.expected: Not scroll
     */
    ScrollBy(0, 0, smooth);
    EXPECT_TRUE(TickPosition(-600.0f));

    /**
     * @tc.steps: step4. ScrollBy the position to top
     * @tc.expected: ScrollBy the top
     */
    ScrollBy(0, -1000.0f, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollToEdge001
 * @tc.desc: Test ScrollToEdge
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollToEdge001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. SCROLL_BOTTOM
     * @tc.expected: Scroll to bottom with animation
     */
    bool smooth = GetParam();
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, smooth);
    EXPECT_TRUE(TickPosition(-600.0f));

    /**
     * @tc.steps: step2. SCROLL_TOP
     * @tc.expected: Scroll to top with animation
     */
    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollPage001
 * @tc.desc: Test ScrollPage
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, ScrollPage001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. ScrollPage down
     * @tc.expected: Scroll down
     */
    bool smooth = GetParam();
    ScrollPage(false, smooth);
    EXPECT_TRUE(TickPosition(-HEIGHT));

    /**
     * @tc.steps: step2. ScrollPage up
     * @tc.expected: Scroll up
     */
    ScrollPage(true, smooth);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: FastScrollModelActualLayout
 * @tc.desc: The model and controller options path drives a real Grid animation for either fast mode.
 * @tc.type: FUNC
 */
HWTEST_P(GridScrollerTestNg, FastScrollModelActualLayout, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    const auto mode = GetParam() ? FastScrollMode::SKIP_MEASURE : FastScrollMode::SKIP_CREATE_AND_MEASURE;
    GridModelNG::SetScrollToIndexWithOptions(frameNode_.GetRawPtr(), 90, true, ScrollAlign::START, 25.0f, mode);
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    for (int32_t tick = 0; tick < 32 && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest()); ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 9025.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(90).Top(), -25.0f, 0.1f);
    positionController_->ScrollToIndex(5, false, ScrollAlign::START, 0.0f);
    FlushUITasks(frameNode_);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
}

INSTANTIATE_TEST_SUITE_P(Smooth, GridScrollerTestNg, testing::Bool());

/**
 * @tc.name: ScrollPage002
 * @tc.desc: Test ScrollPage with smooth half-page focus scroll.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollPage002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    pattern_->ScrollPage(false, true, AccessibilityScrollType::SCROLL_HALF);
    EXPECT_TRUE(TickPosition(-HEIGHT / 2));

    pattern_->ScrollPage(true, true, AccessibilityScrollType::SCROLL_HALF);
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: AnimateTo006
 * @tc.desc: Test AnimateTo with duration animation, canOverScroll
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo006, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. AnimateTo the position over the scroll
     * @tc.expected: AnimateTo the bottom, can over scroll
     */
    MockAnimationManager::GetInstance().SetTicks(5);
    bool canOverScroll = true;
    bool smooth = false;
    AnimateTo(Dimension(1000.0f), 1000.0f, Curves::EASE, smooth, canOverScroll);
    EXPECT_TRUE(TickPosition(-200.0f));
    EXPECT_TRUE(TickPosition(-400.0f));
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(TickPosition(-800.0f)); // Tick doesn't advance new animations created within the same tick
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(TickPosition(-600.0f));
    EXPECT_TRUE(TickPosition(-600.0f));
}

/**
 * @tc.name: AnimateTo007
 * @tc.desc: Test AnimateTo events.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, AnimateTo007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When Animation is not running, call AnimateTo
     * @tc.expected: onScrollStart event will be triggered
     */
    bool isTrigger = false;
    auto onScrollStart = [&isTrigger]() { isTrigger = true; };
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetOnScrollStart(onScrollStart);
    model.SetScrollBarMode(DisplayMode::ON);
    CreateFixedItems(10);
    CreateDone();

    MockAnimationManager::GetInstance().SetTicks(2);
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, true);
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE / 2));
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step2. When Animation is running, call AnimateTo
     * @tc.expected: onScrollStart event will not be triggered
     */
    isTrigger = false; // reset val
    pattern_->AnimateTo(ITEM_MAIN_SIZE * 2, 200.f, Curves::LINEAR, true);
    EXPECT_FALSE(isTrigger);
    EXPECT_TRUE(TickPosition(-125.0f));
    EXPECT_TRUE(TickPosition(-ITEM_MAIN_SIZE * 2));
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
}

/**
 * @tc.name: ScrollToEdge002
 * @tc.desc: Test ScrollToEdge
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToEdge002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

    /**
     * @tc.steps: step1. SCROLL_BOTTOM
     * @tc.expected: Scroll to bottom with animation
     */
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, 200.0f);
    EXPECT_TRUE(Position(-592.0f));

    /**
     * @tc.steps: step2. SCROLL_TOP
     * @tc.expected: Scroll to top with animation
     */
    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, 200.0f);
    EXPECT_TRUE(Position(0));
}

/**
 * @tc.name: Fling001
 * @tc.desc: Test Fling
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, Fling001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);

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
HWTEST_F(GridScrollerTestNg, GetInfo001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetScrollableDistance(), 600.0f);
    EXPECT_EQ(GetScrollDirection(), Axis::VERTICAL);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset()));
    EXPECT_FALSE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(0), Rect(0, 0, WIDTH, ITEM_MAIN_SIZE)));
    EXPECT_TRUE(IsEqual(GetItemRect(4), Rect()));

    /**
     * @tc.steps: step1. AnimateTo the position
     */
    AnimateTo(Dimension(ITEM_MAIN_SIZE), 0, nullptr, false);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset(0, ITEM_MAIN_SIZE)));
    EXPECT_FALSE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(0), Rect()));
    EXPECT_TRUE(IsEqual(GetItemRect(1), Rect(0, 0, WIDTH, ITEM_MAIN_SIZE)));
    EXPECT_TRUE(IsEqual(GetItemRect(5), Rect()));

    /**
     * @tc.steps: step2. AnimateTo bottom
     */
    AnimateTo(Dimension(600.0f), 0, nullptr, false);
    EXPECT_TRUE(IsEqual(GetCurrentOffset(), Offset(0, 600.0f)));
    EXPECT_TRUE(IsAtEnd());
    EXPECT_TRUE(IsEqual(GetItemRect(5), Rect()));
    EXPECT_TRUE(IsEqual(GetItemRect(6), Rect(0, 0, WIDTH, ITEM_MAIN_SIZE)));
}

/**
 * @tc.name: ScrollToFocusNodeIndex001
 * @tc.desc: Test ScrollToFocusNodeIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToFocusNodeIndex001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFocusableGridItems(10, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    /**
     * @tc.steps: step1. Focus node outside the viewport
     * @tc.expected: scroll to the node
     */
    int32_t focusNodeIndex = 6;
    pattern_->ScrollToFocusNodeIndex(focusNodeIndex);
    FlushUITasks();
    RefPtr<FocusHub> focusNode = GetChildFocusHub(frameNode_, focusNodeIndex);
    EXPECT_TRUE(focusNode->IsCurrentFocus());
    EXPECT_TRUE(Position(0));

    /**
     * @tc.steps: step2. Focus node index not exist
     * @tc.expected: do nothing
     */
    focusNodeIndex = 10;
    pattern_->ScrollToFocusNodeIndex(focusNodeIndex);
    FlushUITasks();
    EXPECT_TRUE(Position(0));
}

/**
 * @tc.name: ScrollToNode001
 * @tc.desc: Test ScrollToNode
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ScrollToNode001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. Focus node outside the viewport
     * @tc.expected: scroll to the node
     */
    int32_t focusNodeIndex = 6;
    RefPtr<FrameNode> focusNode = GetChildFrameNode(frameNode_, focusNodeIndex);
    pattern_->ScrollToNode(focusNode);
    FlushUITasks();
    EXPECT_TRUE(Position(0));
}

/**
 * @tc.name: OnInjectionEventTest001
 * @tc.desc: test OnInjectionEvent
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, OnInjectionEventTest001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_TRUE(pattern_->IsAtTop());

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_-> info_.currentOffset_, 0);

    command = R"({"cmd":"scrollForward"})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_-> info_.currentOffset_, 0);

    command = R"({"cmd":"scrollBackward"})";
    pattern_->OnInjectionEvent(command);
    EXPECT_NE(pattern_->info_.currentOffset_, 0);
}

/**
 * @tc.name: OnInjectionEventTest001
 * @tc.desc: Test CreateWithResourceObjScrollBarColor in GridModelNG
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, OnInjectionEventTest002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_TRUE(pattern_->IsAtTop());

    std::string command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);

    command = R"({"cmd":"scrollBackward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, -40);

    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_TRUE(pattern_->IsAtBottom());

    command = R"({"cmd":"scrollBackward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);

    command = R"({"cmd":"scrollward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);

    command = R"({"cmd":"scrollForward","eventId":123123,"ratio":1.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);

    command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 40);
}

/**
 * @tc.name: OnInjectionEventTest002
 * @tc.desc: test OnInjectionEvent
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, OnInjectionEventTest003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr");
    model.SetColumnsTemplate("1fr");
    CreateDone();
    EXPECT_FALSE(pattern_->IsScrollable());

    std::string command = R"({"cmd":"scrollForward","eventId":123123,"ratio":0.1})";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);

    command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
}

/**
 * @tc.name: OnInjectionEventTest004
 * @tc.desc: Test CreateWithResourceObjScrollBarColor in GridModelNG
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, OnInjectionEventTest004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_TRUE(pattern_->IsAtTop());

    std::string command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":20})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, -20);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":-10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);

    command = R"({"cmd":"scrolloffset","eventId":123123,"offset":10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);

    command = R"({"cmd":"scrollByOffset","eventId":123123)";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, -10);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);

    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_TRUE(pattern_->IsAtBottom());

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":10})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->GetFirstIndex(), 6);
}

/**
 * @tc.name: OnInjectionEventTest005
 * @tc.desc: Test CreateWithResourceObjScrollBarColor in GridModelNG
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, OnInjectionEventTest005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_TRUE(pattern_->IsAtTop());

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);

    command = R"({"cmd":"scrollByOffset","eventId":123123,"offset":-20})";
    pattern_->OnInjectionEvent(command);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->GetFirstIndex(), 0);
}

/**
 * @tc.name: ReportComponentChangeEventTest001
 * @tc.desc: ReportComponentChangeEventTest
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, ReportComponentChangeEventTest001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr");
    model.SetColumnsTemplate("1fr");
    CreateDone();
    EXPECT_FALSE(pattern_->IsScrollable());
    MockUiSessionManager* mockUiSessionManager =
        reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mockUiSessionManager, GetComponentChangeEventRegistered()).WillRepeatedly(Return(true));

    pattern_->ReportScroll(false, ScrollError::SCROLL_ERROR_OTHER, 123);
    pattern_->ReportScroll(true, ScrollError::SCROLL_NO_ERROR, 123);
    pattern_->ReportOnItemGridEvent("onReachStart");

    std::string command = R"()";
    pattern_->OnInjectionEvent(command);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
}

/**
 * @tc.name: GetBindingFrameNodeId001
 * @tc.desc: Test GetBindingFrameNodeId returns valid node id for Grid component
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollerTestNg, GetBindingFrameNodeId001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(20);
    CreateDone();

    /**
     * @tc.steps: step1. Get the binding frame node id from controller
     * @tc.expected: The node id should match the grid frame node's id
     */
    auto nodeId = positionController_->GetBindingFrameNodeId();
    EXPECT_EQ(nodeId, frameNode_->GetId());
}
namespace {
constexpr int32_t FRAME_ITEM_COUNT = 150;
constexpr float FRAME_EPSILON = 0.1f;
constexpr int32_t FRAME_DIRECTION_COUNT = 2;
constexpr size_t FRAME_LAYOUT_PARAM = 0;
constexpr size_t FRAME_MODE_PARAM = 1;
constexpr size_t FRAME_SCENARIO_PARAM = 2;
constexpr int32_t FRAME_COLUMN_COUNT = 3;
constexpr int32_t FRAME_HEIGHT_VARIANTS = 5;
constexpr int32_t FRAME_TRACE_PRECISION = 9;
constexpr int32_t FRAME_GROUP_ROWS = 2;
constexpr int32_t FRAME_HEAD_STOP_TICK = 2;
constexpr int32_t FRAME_SETTLED_TICKS = 2;
constexpr int32_t FRAME_TICK_LIMIT = 256;
constexpr size_t FRAME_PREVIOUS_SAMPLE = 2;
constexpr int32_t FRAME_ANIMATION_TICKS = 32;
constexpr int32_t FRAME_SETTLED_MARKER = 1000;
constexpr int32_t FRAME_LARGE_ITEM = 60;
constexpr int32_t FRAME_LARGE_GROUP = FRAME_LARGE_ITEM / FRAME_COLUMN_COUNT;
constexpr int32_t FRAME_FIRST_SPAN = 39;
constexpr int32_t FRAME_SECOND_SPAN = 40;
constexpr int32_t FRAME_FAR_INDEX = 120;
constexpr int32_t FRAME_NEAR_INDEX = 15;
enum FrameLayout : int32_t { FRAME_ORDINARY, FRAME_OPTIONS, FRAME_IRREGULAR, FRAME_CUSTOM };
enum FrameScenario : int32_t {
    FRAME_START, FRAME_CENTER_HALF, FRAME_END_MINUS_TWO, FRAME_LARGE_PLUS_TWO,
    FRAME_AUTO_VISIBLE, FRAME_AT_THRESHOLD, FRAME_OVER_THRESHOLD, FRAME_STOP_HEAD, FRAME_STOP_TAIL,
    FRAME_RTL_START, FRAME_RTL_END, FRAME_HALF_ACCEPTED, FRAME_ZERO_ACCEPTED, FRAME_AUTO_FAR,
};
struct FastScrollFrameRequest {
    int32_t startIndex;
    int32_t target;
    ScrollAlign align = ScrollAlign::START;
    float extra = 0.0f;
    float start = 0.0f;
    float destination = 0.0f;
    bool limited = false;
};
const std::vector<std::string> FRAME_LAYOUTS { "Ordinary", "Options", "Irregular", "Custom" };
const std::vector<std::string> FRAME_SCENARIOS { "Start", "CenterPlusHalf", "EndMinus2", "LargePlus2",
    "AutoVisible", "At4V", "Over4V", "StopHead", "StopTail", "RtlStart", "RtlEndMinus2",
    "HalfAccepted", "ZeroAccepted", "AutoFar" };
struct FastScrollFrameItem {
    float main;
    float cross;
    float mainSize;
    float crossSize;
};
struct FastScrollLayoutFrame {
    int32_t tick = 0;
    bool skipped = false;
    bool running = false;
    bool pending = false;
    float estimate = 0.0f;
    float physical = 0.0f;
    float willReturn = 0.0f;
    float reported = 0.0f;
    bool geometryReady = false;
    float preparedOffset = 0.0f;
    float preparedSize = 0.0f;
    float preparedStart = 0.0f;
    float cachedTargetSize = 0.0f;
    std::map<int32_t, FastScrollFrameItem> items;
    std::vector<int32_t> built;
    std::vector<int32_t> measured;
};
} // namespace

class GridFastScrollFrameTest : public GridTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, int32_t, int32_t>> {
public:
    void SetUp() override
    {
        previousIrregular_ = SystemProperties::gridIrregularLayoutEnable_;
        GridTestNg::SetUp();
        layout_ = std::get<FRAME_LAYOUT_PARAM>(GetParam());
        mode_ = static_cast<FastScrollMode>(std::get<FRAME_MODE_PARAM>(GetParam()));
        scenario_ = std::get<FRAME_SCENARIO_PARAM>(GetParam()) / FRAME_DIRECTION_COUNT;
        reverse_ = std::get<FRAME_SCENARIO_PARAM>(GetParam()) % FRAME_DIRECTION_COUNT != 0;
        horizontal_ = scenario_ == FRAME_RTL_START || scenario_ == FRAME_RTL_END;
    }
    void TearDown() override
    {
        GridTestNg::TearDown();
        SystemProperties::gridIrregularLayoutEnable_ = previousIrregular_;
    }

protected:
    bool ConstantRows() const { return scenario_ == FRAME_AT_THRESHOLD || scenario_ == FRAME_OVER_THRESHOLD; }
    float GroupHeight(int32_t group) const
    {
        return scenario_ == FRAME_LARGE_PLUS_TWO && group == FRAME_LARGE_GROUP ? 320.0f :
            (ConstantRows() ? 100.0f : 80.0f + (group % FRAME_HEIGHT_VARIANTS) * 20.0f);
    }
    float GroupPrefix(int32_t group) const
    {
        float result = 0.0f;
        for (int32_t i = 0; i < group; ++i) {
            result += 2.0f * GroupHeight(i);
        }
        return result;
    }
    bool Spanning(int32_t index) const
    {
        return layout_ == FRAME_CUSTOM ? index % FRAME_COLUMN_COUNT == 0 :
            layout_ != FRAME_ORDINARY &&
                (index == FRAME_FIRST_SPAN || index == FRAME_SECOND_SPAN || index == FRAME_FAR_INDEX);
    }
    float ItemMainSize(int32_t index) const
    {
        if (layout_ == FRAME_CUSTOM) {
            // The spanning frame may be smaller than its two occupied rows; neighboring items define the row height.
            return GroupHeight(index / FRAME_COLUMN_COUNT) *
                (scenario_ == FRAME_LARGE_PLUS_TWO && Spanning(index) ? 2.0f : 1.0f);
        }
        if (scenario_ == FRAME_LARGE_PLUS_TWO && index == FRAME_LARGE_ITEM) {
            return 800.0f;
        }
        return ConstantRows() ? (layout_ == FRAME_IRREGULAR && Spanning(index) ? 200.0f : 100.0f) :
            80.0f + (index % FRAME_HEIGHT_VARIANTS) * 20.0f;
    }
    void ConfigureCustomFrameOptions(GridLayoutOptions& options)
    {
        options.getStartIndexByIndex = [this](int32_t index) {
            const int32_t group = index / FRAME_COLUMN_COUNT;
            const float shift = index % FRAME_COLUMN_COUNT == FRAME_COLUMN_COUNT - 1 ? GroupHeight(group) : 0.0f;
            return GridStartLineInfo { group * FRAME_COLUMN_COUNT, group * FRAME_GROUP_ROWS,
                -shift, GroupPrefix(group) + shift };
        };
        options.getStartIndexByOffset = [this](float offset) {
            int32_t group = 0;
            while (group + 1 < FRAME_ITEM_COUNT / FRAME_COLUMN_COUNT && GroupPrefix(group + 1) <= offset) {
                ++group;
            }
            return GridStartLineInfo { group * FRAME_COLUMN_COUNT, group * FRAME_GROUP_ROWS,
                GroupPrefix(group) - offset, offset };
        };
    }
    GridLayoutOptions MakeFrameOptions()
    {
        GridLayoutOptions options;
        for (int32_t i = 0; i < FRAME_ITEM_COUNT; ++i) {
            if (Spanning(i)) {
                options.irregularIndexes.insert(i);
            }
        }
        options.getSizeByIndex = [this](int32_t) {
            return layout_ == FRAME_OPTIONS ? (horizontal_ ? GridItemSize { 2, 1 } : GridItemSize { 1, 2 }) :
                GridItemSize { 2, 2 };
        };
        if (layout_ == FRAME_CUSTOM) {
            ConfigureCustomFrameOptions(options);
        }
        return options;
    }
    void CheckFrameAlgorithm()
    {
        auto algorithm = pattern_->CreateLayoutAlgorithm();
        if (layout_ == FRAME_ORDINARY) {
            ASSERT_NE(AceType::DynamicCast<GridScrollLayoutAlgorithm>(algorithm), nullptr);
            ASSERT_EQ(AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(algorithm), nullptr);
        } else if (layout_ == FRAME_OPTIONS) {
            ASSERT_NE(AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(algorithm), nullptr);
        } else if (layout_ == FRAME_IRREGULAR) {
            ASSERT_NE(AceType::DynamicCast<GridIrregularLayoutAlgorithm>(algorithm), nullptr);
        } else {
            ASSERT_NE(AceType::DynamicCast<GridCustomLayoutAlgorithm>(algorithm), nullptr);
        }
    }
    void BuildGrid(bool reference)
    {
        SystemProperties::gridIrregularLayoutEnable_ = layout_ == FRAME_IRREGULAR;
        auto model = CreateGrid();
        model.SetCachedCount(0);
        if (horizontal_) {
            model.SetRowsTemplate("1fr 1fr 1fr");
            ViewAbstract::SetWidth(CalcLength(reference ? 30000.0f : WIDTH));
            layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
        } else {
            model.SetColumnsTemplate("1fr 1fr 1fr");
            ViewAbstract::SetHeight(CalcLength(reference ? 30000.0f : HEIGHT));
        }
        if (layout_ != FRAME_ORDINARY) {
            model.SetLayoutOptions(MakeFrameOptions());
        }
        CreateItemsInLazyForEach(FRAME_ITEM_COUNT, [this, reference](uint32_t index) {
            if (!reference) {
                built_.push_back(index);
                auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
                node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
                    [measured = measured_, index](RefPtr<Kit::FrameNode>&) { measured->push_back(index); });
            }
            if (horizontal_) {
                ViewAbstract::SetWidth(CalcLength(ItemMainSize(index)));
                return HEIGHT / 3.0f * (Spanning(index) ? 2.0f : 1.0f);
            }
            return ItemMainSize(index);
        });
        CreateDone();
        CheckFrameAlgorithm();
    }
    FastScrollFrameItem Normalize(const RectF& rectangle, float viewport) const
    {
        return horizontal_ ? FastScrollFrameItem { viewport - rectangle.Right(), rectangle.Top(),
            rectangle.Width(), rectangle.Height() } : FastScrollFrameItem { rectangle.Top(), rectangle.Left(),
            rectangle.Height(), rectangle.Width() };
    }
    FastScrollLayoutFrame ReadFrame(int32_t tick, bool skipped)
    {
        FastScrollLayoutFrame frame;
        frame.tick = tick;
        frame.skipped = skipped;
        frame.running = pattern_->AnimateRunning();
        frame.pending = pattern_->GetFastScrollRequest() != nullptr;
        frame.estimate = pattern_->GetTotalOffset();
        frame.willReturn = willReturn_;
        frame.reported = reported_;
        const auto builtBefore = built_.size();
        const auto measuredBefore = measured_->size();
        for (int32_t index = 0; index < FRAME_ITEM_COUNT; ++index) {
            // needBuild=false in FrameNode::GetChildByIndex; sampling must not create or measure children.
            auto child = AceType::DynamicCast<FrameNode>(frameNode_->GetChildByIndex(index));
            if (!child || !child->IsActive()) {
                continue;
            }
            const auto item = Normalize(child->GetGeometryNode()->GetFrameRect(), viewport_);
            if (item.main + item.mainSize > FRAME_EPSILON && item.main < viewport_ - FRAME_EPSILON) {
                frame.items.emplace(index, item);
            }
        }
        const auto& geometry = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        if (geometry) {
            frame.geometryReady = geometry->ready;
            frame.preparedOffset = geometry->targetOffsetPx;
            frame.preparedSize = horizontal_ ? geometry->targetRect.Width() : geometry->targetRect.Height();
            frame.preparedStart = horizontal_ ? geometry->targetRect.Left() : geometry->targetRect.Top();
        }
        if (traceTarget_ >= 0) {
            auto target = frameNode_->GetChildByIndex(traceTarget_, true);
            if (target) {
                frame.cachedTargetSize = target->GetGeometryNode()->GetFrameSize().MainSize(infoAxis_);
            }
        }
        EXPECT_EQ(built_.size(), builtBefore);
        EXPECT_EQ(measured_->size(), measuredBefore);
        frame.built.assign(built_.begin() + builtCount_, built_.end());
        frame.measured.assign(measured_->begin() + measuredCount_, measured_->end());
        builtCount_ = built_.size();
        measuredCount_ = measured_->size();
        if (!frame.items.empty()) {
            const auto& [index, item] = *frame.items.begin();
            frame.physical = reference_.at(index).main - item.main;
        }
        return frame;
    }
    void CheckFrame(const FastScrollLayoutFrame& frame)
    {
        ASSERT_FALSE(frame.items.empty()) << "tick=" << frame.tick;
        EXPECT_TRUE(std::isfinite(frame.estimate));
        EXPECT_TRUE(std::isfinite(frame.physical));
        if (frame.physical > FRAME_EPSILON && frame.physical < contentSize_ - viewport_ - FRAME_EPSILON) {
            EXPECT_NEAR(frame.reported, frame.willReturn, FRAME_EPSILON) << "tick=" << frame.tick;
        }
        for (const auto& [index, item] : frame.items) {
            SCOPED_TRACE(std::to_string(frame.tick) + ":item=" + std::to_string(index));
            const auto& ref = reference_.at(index);
            EXPECT_NEAR(item.main, ref.main - frame.physical, FRAME_EPSILON);
            EXPECT_NEAR(item.cross, ref.cross, FRAME_EPSILON);
            EXPECT_NEAR(item.mainSize, ref.mainSize, FRAME_EPSILON);
            EXPECT_NEAR(item.crossSize, ref.crossSize, FRAME_EPSILON);
        }
        for (const auto& [index, ref] : reference_) {
            const float start = ref.main - frame.physical;
            if (start + ref.mainSize > FRAME_EPSILON && start < viewport_ - FRAME_EPSILON) {
                EXPECT_NE(frame.items.find(index), frame.items.end()) << "tick=" << frame.tick << ":missing=" << index;
            }
        }
    }
    float ExpectedPosition(int32_t target, ScrollAlign align, float extra, float start) const
    {
        const auto& item = reference_.at(target);
        float result = item.main;
        if (align == ScrollAlign::CENTER) {
            result += (item.mainSize - viewport_) / 2.0f;
        } else if (align == ScrollAlign::END) {
            result += item.mainSize - viewport_;
        } else if (align == ScrollAlign::AUTO) {
            const float top = item.main - start;
            const float end = top + item.mainSize;
            if ((top >= 0.0f && end <= viewport_) || (top <= 0.0f && end >= viewport_)) {
                result = start;
            } else if (top >= 0.0f) {
                result += item.mainSize - viewport_;
            }
        }
        return std::clamp(result + extra, 0.0f, contentSize_ - viewport_);
    }
    void CheckWork(int32_t target, ScrollAlign align, float destination)
    {
        // Callback consumption checks bounded usability rather than the original landing/work bound.
        if (scenario_ == FRAME_HALF_ACCEPTED || scenario_ == FRAME_ZERO_ACCEPTED) {
            return;
        }
        float closure = 0.0f;
        for (const auto& [index, item] : reference_) {
            closure = std::max(closure, item.mainSize);
        }
        const auto& targetItem = reference_.at(target);
        const float unoffset = ExpectedPosition(target, align, 0.0f, frames_.front().physical);
        const float prepareStart = std::min({ targetItem.main, destination, unoffset }) - closure;
        const float prepareEnd = std::max({ targetItem.main + targetItem.mainSize,
            destination + viewport_, unoffset + viewport_ }) + closure;
        std::set<int32_t> forbidden;
        for (const auto& [index, item] : reference_) {
            bool allowed = item.main < prepareEnd && item.main + item.mainSize > prepareStart;
            for (const auto& frame : frames_) {
                allowed = allowed || (item.main < frame.physical + viewport_ + closure &&
                    item.main + item.mainSize > frame.physical - closure);
            }
            if (!allowed) {
                forbidden.insert(index);
            }
        }
        for (const auto& frame : frames_) {
            for (int32_t index : frame.measured) {
                EXPECT_EQ(forbidden.count(index), 0) << "tick=" << frame.tick << ":measure=" << index;
            }
            if (mode_ == FastScrollMode::SKIP_CREATE_AND_MEASURE) {
                for (int32_t index : frame.built) {
                    EXPECT_EQ(forbidden.count(index), 0) << "tick=" << frame.tick << ":build=" << index;
                }
            }
        }
        RecordProperty("unvisited_work_candidates", static_cast<int32_t>(forbidden.size()));
    }
    void RecordTraceSources()
    {
        RecordProperty("input_revision", "frames-r4-custom-frame-smaller-than-occupied-rows");
        RecordProperty("display_frames_observed", "false");
        RecordProperty("creation_source", "GridMockLazy callback; pool reuse not separated");
        RecordProperty("reference_source", "independent full ordinary Grid layout; no fast-scroll estimate reused");
    }
    void SaveTrace(int32_t target, ScrollAlign align, float extra, float destination)
    {
        std::ostringstream out;
        out << std::setprecision(FRAME_TRACE_PRECISION) << "{\"layout\":\"" << FRAME_LAYOUTS[layout_]
            << "\",\"scenario\":\""
            << FRAME_SCENARIOS[scenario_] << "\",\"mode\":" << static_cast<int32_t>(mode_)
            << ",\"reverse\":" << reverse_ << ",\"horizontal_rtl\":" << horizontal_
            << ",\"viewport\":" << viewport_ << ",\"target\":" << target << ",\"align\":" << static_cast<int>(align)
            << ",\"extra\":" << extra << ",\"expected_position\":" << destination << ",\"reference\":[";
        bool first = true;
        for (const auto& [index, item] : reference_) {
            out << (first ? "" : ",") << "[" << index << "," << item.main << "," << item.cross << ","
                << item.mainSize << "," << item.crossSize << "]";
            first = false;
        }
        out << "],\"frames\":[";
        first = true;
        const auto indices = [](std::ostream& stream, const std::vector<int32_t>& values) {
            for (size_t i = 0; i < values.size(); ++i) {
                stream << (i == 0 ? "" : ",") << values[i];
            }
        };
        for (const auto& frame : frames_) {
            out << (first ? "" : ",") << "{\"tick\":" << frame.tick << ",\"skip\":" << frame.skipped
                << ",\"running\":" << frame.running << ",\"pending\":" << frame.pending
                << ",\"estimate\":" << frame.estimate << ",\"physical\":" << frame.physical
                << ",\"will_returned\":" << frame.willReturn << ",\"reported\":" << frame.reported
                << ",\"geometry_ready\":" << frame.geometryReady
                << ",\"prepared_offset\":" << frame.preparedOffset << ",\"prepared_size\":" << frame.preparedSize
                << ",\"prepared_start\":" << frame.preparedStart << ",\"cached_target_size\":" << frame.cachedTargetSize
                << ",\"items\":[";
            bool firstItem = true;
            for (const auto& [index, item] : frame.items) {
                out << (firstItem ? "" : ",") << "[" << index << "," << item.main << "," << item.cross << ","
                    << item.mainSize << "," << item.crossSize << "]";
                firstItem = false;
            }
            out << "],\"built\":[";
            indices(out, frame.built);
            out << "],\"measured\":[";
            indices(out, frame.measured);
            out << "]}";
            first = false;
        }
        out << "]}";
        RecordProperty("host_frame_trace", out.str());
        RecordTraceSources();
    }
    void BuildFrameReference()
    {
        BuildGrid(true);
        ASSERT_FALSE(HasFatalFailure());
        const float referenceViewport = pattern_->GetMainContentSize();
        for (int32_t index = 0; index < FRAME_ITEM_COUNT; ++index) {
            auto child = AceType::DynamicCast<FrameNode>(frameNode_->GetChildByIndex(index));
            ASSERT_NE(child, nullptr) << "full reference index=" << index;
            const auto item = Normalize(child->GetGeometryNode()->GetFrameRect(), referenceViewport);
            ASSERT_GT(item.mainSize, 0.0f);
            ASSERT_GT(item.crossSize, 0.0f);
            reference_[index] = item;
            contentSize_ = std::max(contentSize_, item.main + item.mainSize);
        }
        GridTestNg::TearDown();
        GridTestNg::SetUp();
    }
    void PrepareFrameScenario(int32_t startIndex)
    {
        BuildGrid(false);
        ASSERT_FALSE(HasFatalFailure());
        viewport_ = pattern_->GetMainContentSize();
        ScrollToIndex(startIndex, false, ScrollAlign::START);
        built_.clear();
        measured_->clear();
        builtCount_ = measuredCount_ = 0;
        frames_.clear();
        frames_.push_back(ReadFrame(-1, false));
        CheckFrame(frames_.back());
    }
    void SetFrameThresholdTarget(FastScrollFrameRequest& request) const
    {
        const float multiple = scenario_ == FRAME_AT_THRESHOLD ? 4.0f : 4.01f;
        const float desired = request.start + (reverse_ ? -1.0f : 1.0f) * viewport_ * multiple;
        float closest = std::numeric_limits<float>::max();
        for (const auto& [index, item] : reference_) {
            if (std::abs(item.main - desired) < closest) {
                closest = std::abs(item.main - desired);
                request.target = index;
            }
        }
        request.extra = desired - reference_.at(request.target).main;
    }
    FastScrollFrameRequest MakeFrameRequest(int32_t startIndex) const
    {
        FastScrollFrameRequest request { startIndex,
            scenario_ == FRAME_LARGE_PLUS_TWO ? FRAME_LARGE_ITEM : (reverse_ ? FRAME_NEAR_INDEX : FRAME_FAR_INDEX) };
        request.start = frames_.front().physical;
        if (scenario_ == FRAME_CENTER_HALF) {
            request.align = ScrollAlign::CENTER;
            request.extra = viewport_ * 0.5f;
        } else if (scenario_ == FRAME_END_MINUS_TWO || scenario_ == FRAME_RTL_END) {
            request.align = ScrollAlign::END;
            request.extra = -viewport_ * 2.0f;
        } else if (scenario_ == FRAME_LARGE_PLUS_TWO) {
            request.align = ScrollAlign::CENTER;
            request.extra = viewport_ * 2.0f;
        } else if (scenario_ == FRAME_AUTO_VISIBLE || scenario_ == FRAME_AUTO_FAR) {
            request.target = scenario_ == FRAME_AUTO_VISIBLE ? startIndex + 1 : request.target;
            request.align = ScrollAlign::AUTO;
        } else if (ConstantRows()) {
            SetFrameThresholdTarget(request);
        }
        request.destination = ExpectedPosition(request.target, request.align, request.extra, request.start);
        request.limited = scenario_ == FRAME_STOP_HEAD || scenario_ == FRAME_STOP_TAIL ||
            scenario_ == FRAME_HALF_ACCEPTED || scenario_ == FRAME_ZERO_ACCEPTED;
        return request;
    }
    void ResolveFrameAutoReference(FastScrollFrameRequest& request)
    {
        if (request.align != ScrollAlign::AUTO) {
            return;
        }
        // Obtain the original AUTO rule, then discard the warmed tree before measuring the tested request.
        ScrollToIndex(request.target, false, ScrollAlign::AUTO, request.extra);
        const auto originalAuto = ReadFrame(-2, false);
        CheckFrame(originalAuto);
        ASSERT_FALSE(HasFailure());
        request.destination = originalAuto.physical;
        GridTestNg::TearDown();
        GridTestNg::SetUp();
        PrepareFrameScenario(request.startIndex);
        ASSERT_FALSE(HasFatalFailure());
        ASSERT_NEAR(frames_.back().physical, request.start, FRAME_EPSILON);
    }
    void StartFrameRequest(const FastScrollFrameRequest& request)
    {
        eventHub_->SetOnWillScroll([this](Dimension offset, ScrollState, ScrollSource) {
            ScrollFrameResult result;
            const float ratio = scenario_ == FRAME_HALF_ACCEPTED ? 0.5f :
                (scenario_ == FRAME_ZERO_ACCEPTED ? 0.0f : 1.0f);
            result.offset = Dimension(offset.Value() * ratio, offset.Unit());
            willReturn_ += result.offset.ConvertToPx();
            return result;
        });
        eventHub_->SetOnDidScroll([this](Dimension offset, ScrollState) { reported_ += offset.ConvertToPx(); });
        traceTarget_ = request.target;
        infoAxis_ = horizontal_ ? Axis::HORIZONTAL : Axis::VERTICAL;
        MockAnimationManager::GetInstance().SetTicks(FRAME_ANIMATION_TICKS);
        pattern_->ScrollToIndexFastScroll(request.target, true, request.align, request.extra, mode_);
        FlushUITasks(frameNode_);
        frames_.push_back(ReadFrame(0, false));
        CheckFrame(frames_.back());
    }
    void CheckFrameMovement(const FastScrollFrameRequest& request, int32_t tick, bool skipped, int32_t& skips)
    {
        const auto& previous = frames_[frames_.size() - FRAME_PREVIOUS_SAMPLE];
        const auto& current = frames_.back();
        const float movement = current.physical - previous.physical;
        if (!skipped) {
            EXPECT_LE(std::abs(movement), viewport_ + FRAME_EPSILON) << "tick=" << tick;
            EXPECT_NEAR(movement, current.reported, FRAME_EPSILON) << "tick=" << tick;
        }
        if (ConstantRows() && !request.limited && std::abs(request.destination - request.start) > FRAME_EPSILON) {
            EXPECT_GE(movement * (request.destination > request.start ? 1.0f : -1.0f), -FRAME_EPSILON) <<
                "tick=" << tick;
        }
        if (skipped) {
            if (skips == 0 && !request.limited && std::abs(request.destination - request.start) > 4.0f * viewport_) {
                EXPECT_GE(std::abs(previous.physical - request.start), 1.5f * viewport_ - FRAME_EPSILON);
            }
            ++skips;
        }
    }
    int32_t RunFrameRequest(const FastScrollFrameRequest& request, bool& stopped)
    {
        int32_t skips = 0;
        for (int32_t tick = 1; tick <= FRAME_TICK_LIMIT && !HasFailure(); ++tick) {
            willReturn_ = 0.0f;
            reported_ = 0.0f;
            MockAnimationManager::GetInstance().Tick();
            const bool skipped = pattern_->fastScrollSkip_;
            FlushUITasks(frameNode_);
            frames_.push_back(ReadFrame(tick, skipped));
            CheckFrame(frames_.back());
            CheckFrameMovement(request, tick, skipped, skips);
            if (!stopped && ((scenario_ == FRAME_STOP_HEAD && tick == FRAME_HEAD_STOP_TICK) ||
                (scenario_ == FRAME_STOP_TAIL && skipped))) {
                pattern_->StopAnimate();
                stopped = true;
            }
            if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
                break;
            }
        }
        return skips;
    }
    void CheckFrameCompletion(const FastScrollFrameRequest& request, int32_t skips, bool stopped)
    {
        if (!request.limited) {
            EXPECT_NEAR(frames_.back().physical, request.destination, FRAME_EPSILON);
            if (scenario_ == FRAME_AT_THRESHOLD) {
                EXPECT_EQ(skips, 0);
            } else if (std::abs(request.destination - request.start) > 4.0f * viewport_ + FRAME_EPSILON) {
                EXPECT_GT(skips, 0);
            }
        } else if (scenario_ == FRAME_STOP_HEAD || scenario_ == FRAME_STOP_TAIL) {
            EXPECT_TRUE(stopped);
        }
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        const float finalPosition = frames_.back().physical;
        for (int32_t settled = 0; settled < FRAME_SETTLED_TICKS && !HasFailure(); ++settled) {
            willReturn_ = reported_ = 0.0f;
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
            frames_.push_back(ReadFrame(FRAME_SETTLED_MARKER + settled, false));
            CheckFrame(frames_.back());
            EXPECT_NEAR(frames_.back().physical, finalPosition, FRAME_EPSILON);
            EXPECT_NEAR(frames_.back().reported, 0.0f, FRAME_EPSILON);
            EXPECT_FALSE(frames_.back().pending);
            EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
            EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
        }
    }
    int32_t layout_ = FRAME_ORDINARY;
    int32_t scenario_ = 0;
    FastScrollMode mode_ = FastScrollMode::NONE;
    bool reverse_ = false;
    bool horizontal_ = false;
    bool previousIrregular_ = false;
    int32_t traceTarget_ = -1;
    Axis infoAxis_ = Axis::VERTICAL;
    float viewport_ = 0.0f;
    float contentSize_ = 0.0f;
    float willReturn_ = 0.0f;
    float reported_ = 0.0f;
    size_t builtCount_ = 0;
    size_t measuredCount_ = 0;
    std::vector<int32_t> built_;
    std::shared_ptr<std::vector<int32_t>> measured_ = std::make_shared<std::vector<int32_t>>();
    std::map<int32_t, FastScrollFrameItem> reference_;
    std::vector<FastScrollLayoutFrame> frames_;
};

/**
 * @tc.name: FastScrollFrames
 * @tc.desc: Verify every Host layout frame against an independent full layout for all four algorithms.
 * @tc.type: FUNC
 */
HWTEST_P(GridFastScrollFrameTest, FastScrollFrames, TestSize.Level1)
{
    BuildFrameReference();
    ASSERT_FALSE(HasFatalFailure());
    const int32_t startIndex = reverse_ ? 120 : 15;
    PrepareFrameScenario(startIndex);
    ASSERT_FALSE(HasFailure());
    auto request = MakeFrameRequest(startIndex);
    ResolveFrameAutoReference(request);
    ASSERT_FALSE(HasFatalFailure());
    StartFrameRequest(request);
    bool stopped = false;
    const int32_t skips = RunFrameRequest(request, stopped);
    CheckFrameCompletion(request, skips, stopped);
    CheckWork(request.target, request.align, request.destination);
    SaveTrace(request.target, request.align, request.extra, request.destination);
}

INSTANTIATE_TEST_SUITE_P(Frames, GridFastScrollFrameTest,
    testing::Combine(testing::Values(FRAME_ORDINARY, FRAME_OPTIONS, FRAME_IRREGULAR, FRAME_CUSTOM),
        testing::Values(1, 2),
        testing::Range(0, static_cast<int32_t>(FRAME_SCENARIOS.size()) * FRAME_DIRECTION_COUNT)),
    [](const testing::TestParamInfo<GridFastScrollFrameTest::ParamType>& info) {
        const auto scenario = std::get<FRAME_SCENARIO_PARAM>(info.param);
        return FRAME_LAYOUTS[std::get<FRAME_LAYOUT_PARAM>(info.param)] + "_Mode" +
            std::to_string(std::get<FRAME_MODE_PARAM>(info.param)) +
            (scenario % FRAME_DIRECTION_COUNT ? "_Reverse_" : "_Forward_") +
            FRAME_SCENARIOS[scenario / FRAME_DIRECTION_COUNT];
    });

} // namespace OHOS::Ace::NG

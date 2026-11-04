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

#include "custom_layout_options.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/unittest/core/pattern/grid/grid_test_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"

#include "core/components_ng/pattern/grid/grid_custom/grid_custom_layout_algorithm.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t COLD_ITEM_COUNT = 150;
constexpr int32_t REVERSE_ITEM_COUNT = 120;
constexpr int32_t REVERSE_TARGET_INDEX = 18;
constexpr int32_t CUSTOM_GROUP_ITEMS = 3;
} // namespace

class GridCustomScrollerTestNg : public GridTestNg {
protected:
    void BuildColdSpanGrid(const std::shared_ptr<std::vector<int32_t>>& created,
        const std::shared_ptr<std::vector<int32_t>>& measured,
        const std::shared_ptr<std::vector<int32_t>>& queried)
    {
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetCachedCount(0);
        auto options = Get3LinesIrregularDemoOptions(COLD_ITEM_COUNT, 100.0f);
        const auto getStart = options.getStartIndexByIndex;
        options.getStartIndexByIndex = [queried, getStart](int32_t index) {
            queried->push_back(index);
            return getStart(index);
        };
        model.SetLayoutOptions(options);
        CreateItemsInLazyForEach(COLD_ITEM_COUNT, [created, measured](uint32_t index) {
            created->push_back(static_cast<int32_t>(index));
            auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
            node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
                [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
            return index % CUSTOM_GROUP_ITEMS == 0 ? 200.0f : 100.0f;
        });
        CreateDone();
    }

protected:
    void BuildReverseColdGrid(const std::shared_ptr<std::vector<int32_t>>& created,
        const std::shared_ptr<WeakPtr<FrameNode>>& lastTarget)
    {
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetCachedCount(0);
        auto options = Get3LinesIrregularDemoOptions(REVERSE_ITEM_COUNT, 100.0f);
        model.SetLayoutOptions(options);
        CreateItemsInLazyForEach(REVERSE_ITEM_COUNT, [created, lastTarget](uint32_t index) {
            created->push_back(static_cast<int32_t>(index));
            if (index == REVERSE_TARGET_INDEX) {
                *lastTarget = ViewStackProcessor::GetInstance()->GetMainFrameNode();
            }
            return index % CUSTOM_GROUP_ITEMS == 0 ? 200.0f : 100.0f;
        });
        CreateDone();
    }
};

/**
 * @tc.name: ScrollToIndexStartWithContentOffset
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::Start without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexStartWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::START;
    bool smooth = false;
    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->info_.prevOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->info_.prevOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 180);

    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->info_.prevOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 580);

    ScrollToIndex(17, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->info_.prevOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->info_.prevOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: AnimateToIndexStartWithContentOffsetTest
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::Start with animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, AnimateToIndexStartWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::START;

    AnimateToIndexWithTicks(19, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    AnimateToIndexWithTicks(5, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 180);

    AnimateToIndexWithTicks(13, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 580);

    AnimateToIndexWithTicks(17, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    AnimateToIndexWithTicks(0, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: ScrollToIndexEndWithContentOffset
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::End without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexEndWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::END;
    bool smooth = false;

    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);

    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    ScrollToIndex(17, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 530);

    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: AnimateToIndexEndWithContentOffsetTest
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::End with animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, AnimateToIndexEndWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::END;

    AnimateToIndexWithTicks(19, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    AnimateToIndexWithTicks(5, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);

    AnimateToIndexWithTicks(13, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    AnimateToIndexWithTicks(17, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 530);

    AnimateToIndexWithTicks(0, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: ScrollToIndexAutoWithContentOffset
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::Auto without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexAutoWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::AUTO;
    bool smooth = false;

    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    // Scroll forward, item 5 is at top
    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 180);

    // Scroll backward, item 13 is at end
    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    // item 11 is in view
    ScrollToIndex(11, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    ScrollBy(0, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->info_.startIndex_, 6);
    EXPECT_EQ(pattern_->info_.endIndex_, 13);
    ScrollToIndex(7, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 280);

    ScrollTo(300);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->info_.startIndex_, 6);
    EXPECT_EQ(pattern_->info_.endIndex_, 13);
    ScrollToIndex(13, false, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: AnimateToIndexAutoWithContentOffsetTest
 * @tc.desc: Test ScrollToIndex with ContentOffset and ScrollAlign::Auto with animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, AnimateToIndexAutoWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::AUTO;

    AnimateToIndexWithTicks(19, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    AnimateToIndexWithTicks(5, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 180);

    AnimateToIndexWithTicks(13, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    AnimateToIndexWithTicks(11, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    ScrollBy(0, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->info_.startIndex_, 6);
    EXPECT_EQ(pattern_->info_.endIndex_, 13);
    AnimateToIndexWithTicks(7, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -ITEM_MAIN_SIZE + CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 280);

    ScrollTo(300);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_EQ(pattern_->info_.startIndex_, 6);
    EXPECT_EQ(pattern_->info_.endIndex_, 13);
    AnimateToIndexWithTicks(13, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -CONTENT_END_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), 330);

    AnimateToIndexWithTicks(0, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, CONTENT_START_OFFSET);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: ScrollToIndexStart
 * @tc.desc: Test ScrollToIndex with ScrollAlign::Start without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexStartTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::START;
    bool smooth = false;
    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 200);

    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);
    ScrollToIndex(17, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: ScrollToIndexCenter
 * @tc.desc: Test ScrollToIndex with ScrollAlign::Center without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexCenterTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::CENTER;
    bool smooth = false;
    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -50.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -50.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 50);

    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -50.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -50.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 450);
    ScrollToIndex(17, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);
    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: ScrollToIndexEnd
 * @tc.desc: Test ScrollToIndex with ScrollAlign::End without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollToIndexEndTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    CreateFixedItems(20);
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::END;
    bool smooth = false;
    ScrollToIndex(19, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToIndex(5, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0);

    ScrollToIndex(13, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 300);

    ScrollToIndex(17, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 500);
    ScrollToIndex(0, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: ScrollEdgeWithoutAnimationTest
 * @tc.desc: Test ScrollEdge without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollEdgeWithoutAnimationTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    CreateFixedItems(20);
    CreateDone();
    bool smooth = false;
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, smooth);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, smooth);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0);
}

/**
 * @tc.name: ScrollEdgeWithAnimationTest
 * @tc.desc: Test ScrollEdge with animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollEdgeWithAnimationTest, TestSize.Level1)
{
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(10);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    CreateFixedItems(20);
    CreateDone();
    bool smooth = true;
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, smooth);
    TickToFinish();
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 600);

    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, smooth);
    TickToFinish();
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 0.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0);
}

/**
 * @tc.name: ScrollEdgeWithContentOffsetTest
 * @tc.desc: Test ScrollEdge with ContentOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, ScrollEdgeWithContentOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(GetRegularDemoOptions(2, ITEM_MAIN_SIZE));
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    CreateFixedItems(20);
    CreateDone();
    bool smooth = false;
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, smooth);
    EXPECT_EQ(pattern_->info_.currentOffset_, -30.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -30.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, smooth);
    EXPECT_EQ(pattern_->info_.currentOffset_, 20.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 20.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);

    smooth = true;
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, smooth);
    TickToFinish();
    EXPECT_EQ(pattern_->info_.currentOffset_, -30.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -30.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 630);

    ScrollToEdge(ScrollEdgeType::SCROLL_TOP, smooth);
    TickToFinish();
    EXPECT_EQ(pattern_->info_.currentOffset_, 20.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, 20.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), -20);
}

/**
 * @tc.name: ScrollToIndexStart
 * @tc.desc: Test ScrollToIndex with ScrollAlign::Start without animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, IrregularScrollToIndexStartTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetLayoutOptions(Get3LinesIrregularDemoOptions(50, ITEM_MAIN_SIZE));
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return idx % 3 == 0 ? ITEM_MAIN_SIZE * 2 : ITEM_MAIN_SIZE; });
    CreateDone();
    ScrollAlign scrollAlign = ScrollAlign::START;
    bool smooth = false;
    ScrollToIndex(20, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -100.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -100.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1300);
    EXPECT_EQ(pattern_->info_.startIndex_, 18);

    ScrollToIndex(20, smooth, scrollAlign);
    EXPECT_EQ(pattern_->info_.currentOffset_, -100.f);
    EXPECT_EQ(pattern_->info_.prevOffset_, -100.f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1300);
    EXPECT_EQ(pattern_->info_.startIndex_, 18);
}
/**
 * @tc.name: FastScrollCustomColdPreparation
 * @tc.desc: Two-dimensional cold spans use metadata without creating or measuring the intermediate path.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomColdPreparation, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto queried = std::make_shared<std::vector<int32_t>>();
    BuildColdSpanGrid(created, measured, queried);
    ASSERT_NE(AceType::DynamicCast<GridCustomLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    ASSERT_EQ(std::count(created->begin(), created->end(), 39), 0);
    ASSERT_EQ(std::count(created->begin(), created->end(), 40), 0);
    const auto source = pattern_->GetItemRect(0);
    created->clear();
    measured->clear();
    queried->clear();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 15.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    const auto prepared = *pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    EXPECT_TRUE(prepared.ready);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
    EXPECT_EQ(pattern_->GetItemRect(0), source);
    EXPECT_NE(std::find(queried->begin(), queried->end(), 90), queried->end());
    EXPECT_NE(std::find(measured->begin(), measured->end(), 90), measured->end());
    const auto middle = [](int32_t index) { return index >= 24 && index < 72; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    std::string createdIndices;
    std::string measuredIndices;
    for (auto index : *created) {
        createdIndices += std::to_string(index) + ",";
    }
    for (auto index : *measured) {
        measuredIndices += std::to_string(index) + ",";
    }
    RecordProperty("created_indices", createdIndices);
    RecordProperty("measured_indices", measuredIndices);
    RecordProperty("first_prepared_index", prepared.firstPreparedIndex);
    RecordProperty("last_prepared_index", prepared.lastPreparedIndex);
    pattern_->CancelFastScroll();
    ScrollToIndex(90, false, ScrollAlign::START, 15.0f);
    const auto reference = pattern_->GetItemRect(90);
    ASSERT_GT(reference.Width(), 0.0f);
    EXPECT_FLOAT_EQ(prepared.targetRect.Left(), reference.Left());
    EXPECT_FLOAT_EQ(prepared.targetRect.Top() - prepared.targetOffsetPx, reference.Top());
    EXPECT_FLOAT_EQ(prepared.targetRect.Width(), reference.Width());
    EXPECT_FLOAT_EQ(prepared.targetRect.Height(), reference.Height());
}

/**
 * @tc.name: FastScrollCustomColdMode1
 * @tc.desc: Two-dimensional cold spans use metadata without creating or measuring the intermediate path.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomColdMode1, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto queried = std::make_shared<std::vector<int32_t>>();
    BuildColdSpanGrid(created, measured, queried);
    ASSERT_NE(AceType::DynamicCast<GridCustomLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    ASSERT_EQ(std::count(created->begin(), created->end(), 39), 0);
    ASSERT_EQ(std::count(created->begin(), created->end(), 40), 0);
    const auto source = pattern_->GetItemRect(0);
    created->clear();
    measured->clear();
    queried->clear();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 15.0f,
        FastScrollMode::SKIP_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    const auto prepared = *pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    EXPECT_TRUE(prepared.ready);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
    EXPECT_EQ(pattern_->GetItemRect(0), source);
    EXPECT_NE(std::find(queried->begin(), queried->end(), 90), queried->end());
    EXPECT_NE(std::find(measured->begin(), measured->end(), 90), measured->end());
    const auto middle = [](int32_t index) { return index >= 24 && index < 72; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    std::string createdIndices;
    std::string measuredIndices;
    for (auto index : *created) {
        createdIndices += std::to_string(index) + ",";
    }
    for (auto index : *measured) {
        measuredIndices += std::to_string(index) + ",";
    }
    RecordProperty("created_indices", createdIndices);
    RecordProperty("measured_indices", measuredIndices);
    RecordProperty("first_prepared_index", prepared.firstPreparedIndex);
    RecordProperty("last_prepared_index", prepared.lastPreparedIndex);
    pattern_->CancelFastScroll();
    ScrollToIndex(90, false, ScrollAlign::START, 15.0f);
    const auto reference = pattern_->GetItemRect(90);
    ASSERT_GT(reference.Width(), 0.0f);
    EXPECT_FLOAT_EQ(prepared.targetRect.Left(), reference.Left());
    EXPECT_FLOAT_EQ(prepared.targetRect.Top() - prepared.targetOffsetPx, reference.Top());
    EXPECT_FLOAT_EQ(prepared.targetRect.Width(), reference.Width());
    EXPECT_FLOAT_EQ(prepared.targetRect.Height(), reference.Height());
}

/**
 * @tc.name: FastScrollCustomReverseCold
 * @tc.desc: Both modes rebuild a reverse cold target with valid options and preserve the current viewport.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomReverseCold, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto lastTarget = std::make_shared<WeakPtr<FrameNode>>();
    BuildReverseColdGrid(created, lastTarget);
    ScrollToIndex(18, false, ScrollAlign::START);
    const auto reference = pattern_->GetItemRect(18);
    ASSERT_GT(reference.Width(), 0.0f);
    auto lazy = AceType::DynamicCast<LazyForEachNode>(frameNode_->GetChildren().front());
    ASSERT_NE(lazy, nullptr);
    auto old = lastTarget->Upgrade();
    ASSERT_NE(old, nullptr);
    WeakPtr<FrameNode> weak(old);
    old.Reset();
    ScrollToIndex(90, false, ScrollAlign::START);
    const auto sourceIndex = pattern_->GetGridLayoutInfo().startIndex_;
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        SCOPED_TRACE(static_cast<int32_t>(mode));
        lazy->CleanCache(true);
        // A non-cache lookup would restore an expiring item to the active cache and change the test state.
        ASSERT_EQ(lazy->GetFrameChildByIndex(18, false, true, false), nullptr);
        created->clear();
        pattern_->ScrollToIndexFastScroll(18, true, ScrollAlign::START, 0.0f, mode);
        FlushUITasks(frameNode_);
        const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        ASSERT_TRUE(prepared.has_value());
        EXPECT_TRUE(prepared->ready);
        EXPECT_FLOAT_EQ(prepared->targetRect.Left(), reference.Left());
        EXPECT_FLOAT_EQ(prepared->targetRect.Top() - prepared->targetOffsetPx, reference.Top());
        EXPECT_FLOAT_EQ(prepared->targetRect.Width(), reference.Width());
        EXPECT_FLOAT_EQ(prepared->targetRect.Height(), reference.Height());
        auto rebuilt = lastTarget->Upgrade();
        ASSERT_NE(rebuilt, nullptr);
        EXPECT_NE(rebuilt, weak.Upgrade());
        EXPECT_FALSE(rebuilt->IsActive());
        EXPECT_EQ(std::count(created->begin(), created->end(), 18), 1);
        EXPECT_EQ(std::count_if(created->begin(), created->end(), [](int32_t i) { return i >= 36 && i < 84; }), 0);
        EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, sourceIndex);
        weak = rebuilt;
    }
}

/**
 * @tc.name: FastScrollCustomHorizontalRtl
 * @tc.desc: Spanning static items preserve RTL, padding, alignment and signed offsets.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomHorizontalRtl, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr");
    model.SetColumnsGap(Dimension(10.0f));
    model.SetRowsGap(Dimension(5.0f));
    model.SetCachedCount(1, true);
    ViewAbstract::SetPadding(CalcLength(10.0f));
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    auto options = Get3LinesIrregularDemoOptions(150, 110.0f);
    model.SetLayoutOptions(options);
    for (int32_t i = 0; i < 150; ++i) {
        CreateGridItem(i % 3 == 0 ? 200.0f : 100.0f, FILL_VALUE);
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();
    ASSERT_NE(AceType::DynamicCast<GridCustomLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO }) {
        SCOPED_TRACE(static_cast<int32_t>(align));
        ScrollToIndex(0, false, ScrollAlign::START);
        pattern_->ScrollToIndexFastScroll(90, true, align, -25.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks(frameNode_);
        ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
        const auto prepared = *pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
        pattern_->CancelFastScroll();
        ScrollToIndex(90, false, align, -25.0f);
        const auto reference = pattern_->GetItemRect(90);
        ASSERT_GT(reference.Width(), 0.0f);
        float expectedLeft = reference.Left(); // START/AUTO retain their original layout rule.
        const float padding = layoutProperty_->CreatePaddingAndBorder().left.value_or(0.0f);
        if (align == ScrollAlign::CENTER) {
            expectedLeft = padding + (pattern_->GetMainContentSize() - reference.Width()) / 2.0f - 25.0f;
        } else if (align == ScrollAlign::END) {
            expectedLeft = padding - 25.0f;
        }
        EXPECT_NEAR(prepared.targetRect.Left() + prepared.targetOffsetPx, expectedLeft, 0.1f);
        EXPECT_FLOAT_EQ(prepared.targetRect.Top(), reference.Top());
        EXPECT_FLOAT_EQ(prepared.targetRect.Width(), reference.Width());
        EXPECT_FLOAT_EQ(prepared.targetRect.Height(), reference.Height());
    }
}

/**
 * @tc.name: FastScrollCustomRelocation
 * @tc.desc: A relocation batch lays out the target, schedules the existing cache and resumes ordinary offsets.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomRelocation, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1, true);
    auto options = Get3LinesIrregularDemoOptions(150, 100.0f);
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(150, [created](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        return index % 3 == 0 ? 200.0f : 100.0f;
    });
    CreateDone();
    created->clear();
    pattern_->ClearPreloadItemList();
    auto algorithm = AceType::MakeRefPtr<GridCustomLayoutAlgorithm>(pattern_->GetGridLayoutInfo());
    algorithm->SetFastScrollTargetForLayout({ 90, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE });
    algorithm->Measure(AceType::RawPtr(frameNode_));
    algorithm->Layout(AceType::RawPtr(frameNode_));
    auto target = frameNode_->GetChildByIndex(90, true);
    ASSERT_NE(target, nullptr);
    EXPECT_FLOAT_EQ(target->GetGeometryNode()->GetFrameOffset().GetY(), 0.0f);
    EXPECT_GT(target->GetGeometryNode()->GetFrameSize().Width(), 0.0f);
    const int32_t nextIndex = algorithm->info_.endIndex_ + 1;
    const auto pending = pattern_->MovePreloadItemList();
    EXPECT_NE(std::find(pending.begin(), pending.end(), GridPreloadItem(nextIndex)), pending.end());
    EXPECT_EQ(std::count_if(created->begin(), created->end(), [](int32_t i) { return i >= 24 && i < 72; }), 0);
    algorithm->info_.currentOffset_ -= 30.0f;
    algorithm->info_.currentDelta_ = -30.0f;
    algorithm->Measure(AceType::RawPtr(frameNode_));
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_FLOAT_EQ(target->GetGeometryNode()->GetFrameOffset().GetY(), -30.0f);
}

/**
 * @tc.name: FastScrollCustomLastBeforeFirstLayout
 * @tc.desc: A pre-layout last-item request becomes ready after measuring real children and survives no stale result.
 * @tc.type: FUNC
 */
HWTEST_F(GridCustomScrollerTestNg, FastScrollCustomLastBeforeFirstLayout, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    auto options = Get3LinesIrregularDemoOptions(150, 100.0f);
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(150, [](uint32_t index) { return index % 3 == 0 ? 200.0f : 100.0f; });
    pattern_->ScrollToIndexFastScroll(LAST_ITEM, true, ScrollAlign::END, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    CreateDone();
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    const auto prepared = *pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    EXPECT_TRUE(prepared.ready);
    pattern_->CancelFastScroll();
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    ScrollToIndex(LAST_ITEM, false, ScrollAlign::END);
    const auto reference = pattern_->GetItemRect(149);
    ASSERT_GT(reference.Width(), 0.0f);
    EXPECT_FLOAT_EQ(prepared.targetRect.Top() - prepared.targetOffsetPx, reference.Top());
    EXPECT_FLOAT_EQ(prepared.targetRect.Left(), reference.Left());
    EXPECT_FLOAT_EQ(prepared.targetRect.Width(), reference.Width());
    EXPECT_FLOAT_EQ(prepared.targetRect.Height(), reference.Height());
}

} // namespace OHOS::Ace::NG

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

#include "grid_test_ng.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/syntax/lazy_for_each_model_ng.h"
#include "test/unittest/core/syntax/mock_lazy_for_each_builder.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float PRELOAD_TEST_OFFSET = -20.0f;

RefPtr<FrameNode> CreateImageFrameNode()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto frameNode = FrameNode::CreateFrameNode("Image", nodeId, AceType::MakeRefPtr<ImagePattern>());
    return frameNode;
}
} // namespace

namespace {
constexpr float ITEM_PADDING = 5.0f;
constexpr float CHILD_WIDTH = 40.0f;
constexpr float CHILD_HEIGHT = 30.0f;

// LazyForEach builder whose GridItems carry a padded subtree: each item is a GridItem
// with 5px padding containing a fixed-size 40x30 child, so a missing subtree layout of
// a cache item is observable through the child's frame rect (before the layout it stays
// at the (0, 0) origin of the GridItem).
class GridMockLazyWithChild : public Framework::MockLazyForEachBuilder {
public:
    GridMockLazyWithChild(int32_t itemCnt, std::function<float(int32_t)>&& getHeight)
        : itemCnt_(itemCnt), getHeight_(getHeight)
    {}

protected:
    int32_t OnGetTotalCount() override
    {
        return itemCnt_;
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, NG::LazyForEachCacheChild>& expiringItems) override
    {
        GridItemModelNG itemModel;
        itemModel.Create(GridItemStyle::NONE);
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(getHeight_(index)));
        ViewAbstract::SetPadding(CalcLength(ITEM_PADDING));
        // pin the content to the padding corner (the in-item default is Alignment::CENTER)
        ViewAbstract::SetAlign(Alignment::TOP_LEFT);
        // fixed-size child nested inside the padded GridItem
        auto child = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        ViewStackProcessor::GetInstance()->Push(child);
        ViewAbstract::SetWidth(CalcLength(CHILD_WIDTH));
        ViewAbstract::SetHeight(CalcLength(CHILD_HEIGHT));
        ViewStackProcessor::GetInstance()->Pop();
        auto node = ViewStackProcessor::GetInstance()->Finish();
        node->nodeId_ = index;
        return { std::to_string(index), node };
    }

private:
    int32_t itemCnt_ = 0;
    const std::function<float(int32_t)> getHeight_;
};
} // namespace

class GridCacheLayoutTestNg : public GridTestNg {
public:
    void CreateItems1();
    void CreateItems2();
    void CheckSyncPreloadLineShift(
        int32_t startIndex, int32_t lineOffset, int32_t cacheCount, bool recordedTail, int32_t crossCount = 1);
    void CheckSyncPreloadBigItem(bool moveRecordedRows, int32_t cacheCount);
    void InitSyncPreloadAlgorithm(GridScrollLayoutAlgorithm& algorithm, int32_t itemCount);
    void CheckSyncPreloadRange(
        const GridLayoutInfo& info, int32_t startIndex, int32_t endIndex, int32_t startLine, int32_t endLine);
};

void GridCacheLayoutTestNg::CreateItems1()
{
    /**
     * 0:  [0],  [0],  [0],  [0]
     * 1:  [1],  [1],  [1],  [1]
     * 2:  [2],  [2],  [2],  [2]
     * 3:  [3],  [3],  [4],  [4]
     * 4:  [5],  [6],  [4],  [4]
     * 5:  [7],  [8],  [9],  [9]
     * 6:  [10], [11], [12], [13]
     * 7:  [14], [14], [14], [14]
     * 8:  [15], [15], [16], [17]
     * 9:  [18], [18], [18], [18]
     */
    CreateBigItem(0, 0, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(1, 1, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(2, 2, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 0, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 4, 2, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE * 2);
    CreateBigItem(4, 4, 0, 0, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 1, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(5, 5, 0, 0, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(5, 5, 1, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(5, 5, 2, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(6, 6, 0, 0, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(6, 6, 1, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(6, 6, 2, 2, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(6, 6, 3, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(7, 7, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(8, 8, 0, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(8, 8, 2, 2, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(8, 8, 3, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(9, 9, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
}

void GridCacheLayoutTestNg::CreateItems2()
{
    /**
     * 0:  [0],  [0],  [0],  [0],  [0],  [0]
     * 1:  [1],  [1],  [1],  [1],  [2],  [2]
     * 2:  [3],  [3],  [4],  [5],  [2],  [2]
     * 3:  [6],  [7],  [8],  [8],  [9],  [10]
     * 4:  [11], [12], [13], [13], [14], [15]
     * 5:  [16], [16], [16], [16], [17], [17]
     * 6:  [18], [18], [18], [18]
     */
    CreateBigItem(0, 0, 0, 5, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(1, 1, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(1, 2, 4, 5, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE * 2);
    CreateBigItem(2, 2, 0, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(2, 2, 2, 2, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(2, 2, 3, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 0, 0, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 1, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 2, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 4, 4, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(3, 3, 5, 5, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 0, 0, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 1, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 2, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 4, 4, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(4, 4, 5, 5, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(5, 5, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(5, 5, 4, 5, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateBigItem(6, 6, 0, 3, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
}

/**
 * @tc.name: Layout forward cache lines for regular
 * @tc.desc: Test forward cache lines layout
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutForwardCachedLines001, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(30, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(10); // 10 lines
    model.SetLayoutOptions({});
    CreateDone();
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 30);
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 11);

    for (int32_t i = 0; i < 10; i++) {
        pattern_->ScrollBy(ITEM_MAIN_SIZE);
        FlushUITasks();
    }

    int32_t colsNumber = 3;
    float itemWidth = 80;
    float totalHeight = ITEM_MAIN_SIZE * 10;
    for (int32_t index = 29; index >= 0; index--) {
        RectF childRect = GetChildRect(frameNode_, index);
        float offsetX = index % colsNumber * itemWidth;
        float offsetY = -totalHeight + floor(index / colsNumber) * ITEM_MAIN_SIZE + HEIGHT;
        RectF expectRect = RectF(offsetX, offsetY, itemWidth, ITEM_MAIN_SIZE);
        EXPECT_TRUE(IsEqual(childRect, expectRect)) << "index: " << index;
    }
}

/**
 * @tc.name: Layout forward cache lines for irregular item with columnStart
 * @tc.desc: Test forward cache lines layout
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutForwardCachedLines002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateItems1();
    model.SetCachedCount(10); // 10 lines
    CreateDone();

    for (int32_t i = 0; i < 25; i++) {
        pattern_->ScrollBy(50);
        FlushUITasks();
    }

    float totalHeight = ITEM_MAIN_SIZE * 10;
    for (int32_t index = 18; index >= 0; index--) {
        auto child = frameNode_->GetChildByIndex(index);
        auto childProps = AceType::DynamicCast<GridItemLayoutProperty>(child->GetLayoutProperty());
        RectF childRect = GetChildRect(frameNode_, index);
        float width = childProps->GetCrossSpan(Axis::VERTICAL) * WIDTH / 4;
        float height = childProps->GetMainSpan(Axis::VERTICAL) * ITEM_MAIN_SIZE;
        float offsetY = -totalHeight + childProps->GetMainIndex().value_or(0) * ITEM_MAIN_SIZE + HEIGHT;
        // multi-line item's mainIndex may be translated to its last line during scrolling.
        if (index == 4) {
            offsetY -= ITEM_MAIN_SIZE;
        }
        float offsetX = childProps->GetCrossIndex().value_or(0) * WIDTH / 4;
        RectF expectRect = RectF(offsetX, offsetY, width, height);
        EXPECT_TRUE(IsEqual(childRect, expectRect)) << "index: " << index;
    }
}

/**
 * @tc.name: Layout forward cache lines for irregular item with columnStart
 * @tc.desc: Test forward cache lines layout
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutForwardCachedLines003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr 1fr 1fr");
    CreateItems2();
    model.SetCachedCount(6); // 6 lines
    CreateDone();

    for (int32_t i = 0; i < 14; i++) {
        pattern_->ScrollBy(100);
        FlushUITasks();
    }

    float totalHeight = ITEM_MAIN_SIZE * 7;
    for (int32_t index = 18; index >= 0; index--) {
        auto child = frameNode_->GetChildByIndex(index);
        auto childProps = AceType::DynamicCast<GridItemLayoutProperty>(child->GetLayoutProperty());
        RectF childRect = GetChildRect(frameNode_, index);
        float width = childProps->GetCrossSpan(Axis::VERTICAL) * WIDTH / 6;
        float height = childProps->GetMainSpan(Axis::VERTICAL) * ITEM_MAIN_SIZE;
        float offsetY = -totalHeight + childProps->GetMainIndex().value_or(0) * ITEM_MAIN_SIZE + HEIGHT;
        // multi-line item's mainIndex may be translated to its last line during scrolling.
        if (index == 2) {
            offsetY -= ITEM_MAIN_SIZE;
        }
        float offsetX = childProps->GetCrossIndex().value_or(0) * WIDTH / 6;
        RectF expectRect = RectF(offsetX, offsetY, width, height);
        EXPECT_TRUE(IsEqual(childRect, expectRect)) << "index: " << index;
    }
}

/**
 * @tc.name: Create001
 * @tc.desc: Test creating items in front
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, Create001, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(100, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetLayoutOptions({});
    model.SetCachedCount(1); // 6 lines
    CreateDone();
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.endIndex_, 11);

    ScrollToIndex(99, false, ScrollAlign::END);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 89));
    EXPECT_EQ(info.startIndex_, 90);
    const std::list<int32_t> preload = { 89, 88, 87 };
    CheckPreloadListEqual(preload);

    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    ASSERT_TRUE(GetChildFrameNode(frameNode_, 88));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 88)->IsActive());
}

/**
 * @tc.name: Create001
 * @tc.desc: Test creating items in front
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, Create005, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(100, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetLayoutOptions({});
    model.SetCachedCount(1, true); // 6 lines
    CreateDone();
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.endIndex_, 11);

    ScrollToIndex(99, false, ScrollAlign::END);
    ASSERT_TRUE(GetChildFrameNode(frameNode_, 88));
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 88)->IsActive());
    EXPECT_EQ(GetChildHeight(frameNode_, 88), 100);
}

/**
 * @tc.name: ShowCache001
 * @tc.desc: Test Grid showCache items
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache001, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(50, [](uint32_t idx) { return 100.0f; });
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions({});
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    model.SetCachedCount(1, true);
    CreateDone();
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    FlushUITasks();
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 8)->IsActive());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 9)->IsActive());
    EXPECT_EQ(GetChildWidth(frameNode_, 8), 115);
    EXPECT_EQ(GetChildY(frameNode_, 8), 440);
    EXPECT_EQ(GetChildY(frameNode_, 9), 440);

    UpdateCurrentOffset(-200);
    EXPECT_EQ(info.startIndex_, 2);
    EXPECT_EQ(info.endIndex_, 11);
    FlushUITasks();
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 12)->IsActive());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 13)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 10), 350);
    EXPECT_EQ(GetChildY(frameNode_, 11), 350);
    EXPECT_EQ(GetChildY(frameNode_, 12), 460);
    EXPECT_EQ(GetChildY(frameNode_, 13), 460);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 0)->IsActive());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 1)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 0), -200);
    EXPECT_EQ(GetChildY(frameNode_, 1), -200);
}

/**
 * @tc.name: ShowCache002
 * @tc.desc: Test Grid showCache items and change width
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache002, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(50, [](uint32_t idx) { return 100.0f; });
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions({});
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    model.SetCachedCount(2, true);
    CreateDone();
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(GetChildY(frameNode_, 9), 440);
    EXPECT_EQ(GetChildY(frameNode_, 11), 550);

    UpdateCurrentOffset(-275);
    EXPECT_EQ(info.startIndex_, 4);
    EXPECT_EQ(info.endIndex_, 13);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(frameNode_, 15), 115.0f);
    EXPECT_EQ(GetChildX(frameNode_, 15), 125.0f);

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(100.0f), CalcLength(400.0f)));
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(frameNode_, 14), 45.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 15), 45.0f);
    EXPECT_EQ(GetChildX(frameNode_, 15), 55.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 3), 45.0f);
    EXPECT_EQ(GetChildX(frameNode_, 3), 55.0f);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 3)->IsActive());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 14)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 2), -165.0f);
    EXPECT_EQ(GetChildY(frameNode_, 14), 495.0f);
    EXPECT_EQ(info.gridMatrix_.size(), 9);

    UpdateCurrentOffset(-175.0f);
    EXPECT_EQ(info.startIndex_, 8);
    EXPECT_EQ(info.endIndex_, 15);
    EXPECT_EQ(GetChildY(frameNode_, 4), -230.0f);
    EXPECT_EQ(GetChildX(frameNode_, 5), 55.0f);
    EXPECT_EQ(GetChildY(frameNode_, 16), 430.0f);
    EXPECT_EQ(GetChildY(frameNode_, 19), 540.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 19), 45.0f);
}

/**
 * @tc.name: ShowCache003
 * @tc.desc: Test Grid showCache items with padding
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache003, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(50, [](uint32_t idx) { return 100.0f; });
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions({});
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    model.SetCachedCount(3, true);
    ViewAbstract::SetPadding(CalcLength(5));
    CreateDone();
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(GetChildX(frameNode_, 0), 5.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 110);
    EXPECT_EQ(GetChildWidth(frameNode_, 1), 110);
    EXPECT_EQ(GetChildX(frameNode_, 1), 125);
    EXPECT_EQ(GetChildX(frameNode_, 8), 5.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 8), 110);
    EXPECT_EQ(GetChildX(frameNode_, 9), 125);
    EXPECT_EQ(GetChildWidth(frameNode_, 9), 110);
    EXPECT_EQ(GetChildY(frameNode_, 9), 445);
    EXPECT_EQ(GetChildY(frameNode_, 11), 555);
    EXPECT_EQ(GetChildWidth(frameNode_, 11), 110);
}

/**
 * @tc.name: Cache001
 * @tc.desc: Test Grid preload items
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, Cache001, TestSize.Level1)
{
    GridModelNG model = CreateRepeatGrid(50, [](uint32_t idx) { return 100.0f; });
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    model.SetCachedCount(2); // 2 lines
    CreateDone();
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 50);
    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 11);
    const std::list<int32_t> preloadList = { 12, 13, 14 };
    for (const int32_t i : preloadList) {
        EXPECT_FALSE(frameNode_->GetChildByIndex(i));
    }
    CheckPreloadListEqual(preloadList);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    for (const int32_t i : preloadList) {
        EXPECT_TRUE(frameNode_->GetChildByIndex(i));
        EXPECT_EQ(GetChildRect(frameNode_, i).Height(), 100);
    }
    FlushUITasks();
    // preload next line
    const std::list<int32_t> preloadList2 = { 15, 16, 17 };
    CheckPreloadListEqual(preloadList2);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    for (const int32_t i : preloadList2) {
        EXPECT_TRUE(frameNode_->GetChildByIndex(i));
        EXPECT_EQ(GetChildRect(frameNode_, i).Height(), 100);
    }
    FlushUITasks();
    EXPECT_TRUE(pattern_->preloadItemList_.empty());

    ScrollToIndex(49, false, ScrollAlign::AUTO);
    EXPECT_EQ(info.startIndex_, 39);
    // GridScroll algo currently not capable of preloading backward
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
}

/**
 * @tc.name: Cache004
 * @tc.desc: Test Grid layout cache with no scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, Cache004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions({});
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 50.0f; });
    CreateDone();

    EXPECT_EQ(pattern_->info_.endIndex_, 23);
    EXPECT_FALSE(GetItem(24, true));
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    auto item = GetItem(24, true);
    ASSERT_TRUE(item);
    auto ctx = AceType::DynamicCast<MockRenderContext>(item->GetRenderContext());
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 400.00) - [80.00 x 50.00]");

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(600.0f)));
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 400.00) - [80.00 x 50.00]");
    EXPECT_EQ(ctx->paintRect_, item->GetGeometryNode()->GetFrameRect()); // should be synced with layout
    EXPECT_EQ(GetChildFrameNode(frameNode_, 24), item);
}

/**
 * @tc.name: ShowCache004
 * @tc.desc: Test Grid layout cache
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(2, true);
    model.SetLayoutOptions({});
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 50.0f; });
    CreateDone();
    UpdateCurrentOffset(-200.0f);
    UpdateCurrentOffset(60.0f);
    EXPECT_EQ(pattern_->info_.startIndex_, 6);
    EXPECT_EQ(pattern_->info_.endIndex_, 32);
    for (int i = 0; i < 50; ++i) {
        if (i > 38) {
            if (GetItem(i, true)) {
                EXPECT_FALSE(GetItem(i, true)->IsActive());
            }
        } else {
            EXPECT_TRUE(GetItem(i, true));
            EXPECT_TRUE(GetItem(i, true)->IsActive());
        }
    }
}

/**
 * @tc.name: ShowCache005
 * @tc.desc: Test Grid layout cache
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2, true);
    model.SetLayoutOptions({});
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 50.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    for (int i = 0; i < 20; ++i) {
        UpdateCurrentOffset(-300.0f);
    }
    EXPECT_TRUE(GetItem(9, true)->IsActive());
    EXPECT_LT(GetChildY(frameNode_, 9), -50.0f);
    EXPECT_TRUE(GetItem(8, true)->IsActive());
    EXPECT_TRUE(GetItem(7, true)->IsActive());
    EXPECT_TRUE(GetItem(6, true)->IsActive());
    EXPECT_FALSE(GetItem(3, true)->IsActive());
    EXPECT_FALSE(GetItem(2, true)->IsActive());
}

/**
 * @tc.name: ShowCache006
 * @tc.desc: Test cached items when last line isn't filled
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache006, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(2, true);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 50.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    for (int i = 0; i < 20; ++i) {
        UpdateCurrentOffset(-300.0f);
    }
    EXPECT_TRUE(GetItem(9, true)->IsActive());
    EXPECT_LT(GetChildY(frameNode_, 9), -50.0f);
    EXPECT_TRUE(GetItem(8, true)->IsActive());
    EXPECT_TRUE(GetItem(7, true)->IsActive());
    EXPECT_TRUE(GetItem(6, true)->IsActive());
    EXPECT_FALSE(GetItem(5, true)->IsActive());
    EXPECT_FALSE(GetItem(4, true)->IsActive());
}

/**
 * @tc.name: LayoutCachedItem001
 * @tc.desc: Test LayoutCachedItem
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutCachedItem001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set CachedCount:1
     * @tc.expected: The item(index:16) below view is active, no item above view
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetCachedCount(1);
    CreateFixedItems(40);
    CreateDone();
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 16)->IsActive()); // the fifth row
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 20)->IsActive()); // the sixth row

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: The item(index:0) above view is active, the item(index:20) below view is active
     */
    pattern_->UpdateCurrentOffset(-ITEM_MAIN_SIZE, SCROLL_FROM_UPDATE);
    ScrollToIndex(4, false, ScrollAlign::START);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 0)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 20)->IsActive());

    /**
     * @tc.steps: step3. Scroll down
     * @tc.expected: The item(index:4) above view is active, the item(index:24) below view is active
     */
    ScrollToIndex(8, false, ScrollAlign::START);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 0)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 4)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 24)->IsActive()); // th seventh row

    /**
     * @tc.steps: step4. Scroll up
     * @tc.expected: The item(index:0) above view is active, the item(index:20) below view is active
     */
    ScrollToIndex(4, false, ScrollAlign::START);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 0)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 20)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 24)->IsActive());

    /**
     * @tc.steps: step5. Scroll up
     * @tc.expected: The item(index:16) below view is active, no item above view
     */
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 16)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 20)->IsActive());
}

/**
 * @tc.name: LayoutCachedItem002
 * @tc.desc: Change dataSource when overScroll at top.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutCachedItem002, TestSize.Level1)
{
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(1);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    model.SetCachedCount(3, true);
    CreateFixedItems(40);
    CreateDone();
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 7);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 3);

    GestureEvent info;
    info.SetMainVelocity(1200.f);
    info.SetMainDelta(400.f);
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchDown();
    scrollable->HandleDragStart(info);
    scrollable->HandleDragUpdate(info);
    FlushUITasks();

    EXPECT_FLOAT_EQ(pattern_->info_.currentOffset_, 400);
    EXPECT_EQ(GetChildY(frameNode_, 0), 400);
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 1);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 0);

    // remove the last child.
    frameNode_->RemoveChildAtIndex(39);
    frameNode_->ChildrenUpdatedFrom(39);

    scrollable->HandleTouchUp();
    scrollable->HandleDragEnd(info);
    FlushUITasks();
    EXPECT_FLOAT_EQ(pattern_->info_.currentOffset_, 402.69519);
    EXPECT_FLOAT_EQ(GetChildY(frameNode_, 0), 402.69519);
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 1);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 0);

    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_NEAR(pattern_->info_.currentOffset_, 0, 0.0001);
    EXPECT_NEAR(GetChildY(frameNode_, 0), 0, 0.0001);
    EXPECT_FLOAT_EQ(GetChildY(frameNode_, 2), 100);
    EXPECT_FLOAT_EQ(GetChildY(frameNode_, 4), 200);
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 7);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 3);
}

/**
 * @tc.name: ShowCache007
 * @tc.desc: change dataSource when overScroll at bottom.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCache007, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1, true);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateFixedItems(7);
    CreateDone();
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 6);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 0);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 3);

    UpdateCurrentOffset(-300.0f);
    frameNode_->RemoveChildAtIndex(0);
    frameNode_->ChildrenUpdatedFrom(0);
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.startIndex_, 4);
    EXPECT_EQ(pattern_->info_.endIndex_, 5);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 3);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 3);
    decltype(pattern_->info_.gridMatrix_) cmp = { {2, { {0, 2}, {1, 3} }}, {3, { {0, 4}, {1, 5} }} };
    EXPECT_EQ(pattern_->info_.gridMatrix_, cmp);

    // FillNewLineForward in syncPreload.
    UpdateCurrentOffset(100.0f);
    EXPECT_EQ(pattern_->info_.startIndex_, 2);
    EXPECT_EQ(pattern_->info_.endIndex_, 5);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 1);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 2);
    cmp = { {0, { {0, 0}, {1, 1} }}, {1, { {0, 2}, {1, 3} }}, {2, { {0, 4}, {1, 5} }} };
    EXPECT_EQ(pattern_->info_.gridMatrix_, cmp);
}

/**
 * @tc.name: LayoutCachedItem003
 * @tc.desc: Test preLoadList when use ForEach.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, LayoutCachedItem003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set CachedCount:10
     * @tc.expected: The item(index:16) below view is active, no item above view
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(10);
    CreateFixedItems(40);
    CreateDone();
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);

    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 7);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 7)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 8)->IsActive());

    EXPECT_EQ(GetChildHeight(frameNode_, 7), 100);
    EXPECT_EQ(GetChildRect(frameNode_, 7).Bottom(), 400.0f);
    EXPECT_FALSE(frameNode_->GetChildByIndex(7)->CheckNeedForceMeasureAndLayout());

    EXPECT_EQ(GetChildHeight(frameNode_, 8), 100);
    EXPECT_FALSE(frameNode_->GetChildByIndex(8)->CheckNeedForceMeasureAndLayout());
}

/**
 * @tc.name: ShowCacheWithMultiLineItem
 * @tc.desc: ShowCacheWithMultiLineItem
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCacheWithMultiLineItem, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2, true);
    CreateBigItem(0, 1, 0, 1, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE * 2);
    CreateFixedItems(20);
    CreateDone();
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 4);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 0);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 3);

    UpdateCurrentOffset(-ITEM_MAIN_SIZE * 0.75);
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 6);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 0);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 4);

    UpdateCurrentOffset(-ITEM_MAIN_SIZE * 0.75);
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 8);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 1);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 5);

    UpdateCurrentOffset(-ITEM_MAIN_SIZE * 0.75);
    FlushUITasks();
    EXPECT_EQ(pattern_->info_.startIndex_, 1);
    EXPECT_EQ(pattern_->info_.endIndex_, 10);
    EXPECT_EQ(pattern_->info_.startMainLineIndex_, 2);
    EXPECT_EQ(pattern_->info_.endMainLineIndex_, 6);
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount001
 * @tc.desc: Test overScroll at top with cachedCount=0, initial state at top
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, 10 items height=100 each
     * @tc.expected: All items visible on screen (viewport height=400), at initial position
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr"); // 2 columns
    model.SetCachedCount(0); // No cache
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7); // 2 columns * 4 rows = 8 items fit in 400px viewport
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3); // 4 rows (0, 1, 2, 3)

    // Verify all on-screen items exist and are active
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        EXPECT_TRUE(item->IsActive()) << "Item " << i << " should be active";
        EXPECT_GE(GetChildY(frameNode_, i), 0) << "Item " << i << " Y position should be >= 0";
        EXPECT_LE(GetChildY(frameNode_, i), 400) << "Item " << i << " Y position should be <= 400";
    }

    // Verify off-screen items do not exist
    for (int32_t i = 8; i < 10; ++i) {
        auto item = GetItem(i, true);
        EXPECT_FALSE(item) << "Item " << i << " should not exist";
    }

    EXPECT_EQ(info.currentOffset_, 0);
    EXPECT_NEAR(GetChildY(frameNode_, 0), 0, 0.1); // First item at top edge
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount002
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 50px at top
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 50px at top (overScroll)
     * @tc.expected: First row pushed down to Y=50, last row partially off-screen at bottom
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(50.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(info.currentOffset_, 50);

    // Items 0-1: pushed down to Y=50 (overScroll effect)
    EXPECT_NEAR(GetChildY(frameNode_, 0), 50, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 1), 50, 0.1);

    // Items 6-7: partially off-screen at bottom (Y=350)
    EXPECT_NEAR(GetChildY(frameNode_, 6), 350, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 7), 350, 0.1);

    // Verify all items 0-7 are active during touch
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        bool isOnScreen = (GetChildY(frameNode_, i) >= 0 && GetChildY(frameNode_, i) < 400);
        EXPECT_EQ(item->IsActive(), isOnScreen) << "Item " << i << " active state mismatch at Y="
                                                << GetChildY(frameNode_, i);
    }

    // Items 8-9: still not created
    for (int32_t i = 8; i < 10; ++i) {
        EXPECT_FALSE(GetItem(i, true)) << "Item " << i << " should not exist";
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount003
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 100px at top
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 100px at top (overScroll)
     * @tc.expected: First row pushed to Y=100, items 6-7 completely off-screen at bottom
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(100.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 5);
    EXPECT_EQ(info.currentOffset_, 100);

    // Items 0-1: pushed to Y=100
    EXPECT_NEAR(GetChildY(frameNode_, 0), 100, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 1), 100, 0.1);

    // Items 2-3: at Y=200
    EXPECT_NEAR(GetChildY(frameNode_, 2), 200, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 3), 200, 0.1);

    // Verify active state: items 6-7 should be inactive (Y >= 400)
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 6) {
            EXPECT_EQ(item->IsActive(), false);
        } else {
            EXPECT_EQ(item->IsActive(), true);
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount004
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 200px at top
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 200px at top (overScroll)
     * @tc.expected: First two rows pushed down, last two rows completely off-screen
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(200.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 3);
    EXPECT_EQ(info.currentOffset_, 200);

    // Items 0-1: pushed to Y=200
    EXPECT_NEAR(GetChildY(frameNode_, 0), 200, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 1), 200, 0.1);

    // Items 2-3: at Y=300
    EXPECT_NEAR(GetChildY(frameNode_, 2), 300, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 3), 300, 0.1);

    // Items 4-7: off-screen items may not have Y values updated, only verify active state
    // Verify active state: only items 0-3 should be active (on or partially on screen)
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 4) {
            EXPECT_EQ(item->IsActive(), false);
        } else {
            EXPECT_EQ(item->IsActive(), true);
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount005
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 300px at top
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 300px at top (overScroll)
     * @tc.expected: Most items off-screen at bottom
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(300.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 1);
    EXPECT_EQ(info.currentOffset_, 300);

    // Items 0-1: pushed to Y=300
    EXPECT_NEAR(GetChildY(frameNode_, 0), 300, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 1), 300, 0.1);

    // Items 2-7: off-screen items may not have Y values updated, only verify active state
    // Only items 0-1 should be active (partially visible)
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 2) {
            EXPECT_EQ(item->IsActive(), false);
        } else {
            EXPECT_EQ(item->IsActive(), true);
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount006
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 400px (extreme case)
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 400px at top (overScroll)
     * @tc.expected: First row at Y=400, almost all items off-screen
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(400.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 1);
    EXPECT_EQ(info.currentOffset_, 400);

    // All items should be inactive as Y >= 400
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 2) {
            EXPECT_EQ(item->IsActive(), false) << "Item " << i << " should be inactive (off-screen)";
        } else {
            EXPECT_EQ(item->IsActive(), true);  // must have one row
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount007
 * @tc.desc: Test overScroll at top with cachedCount=0, drag down 500px (extreme case)
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down 500px at top (overScroll)
     * @tc.expected: All items completely off-screen below viewport
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(500.0f); // Positive: drag down at top
    FlushUITasks();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 1);
    EXPECT_EQ(info.currentOffset_, 500);

    // Off-screen items may not have Y values updated, only verify active state
    // All items should be inactive
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 2) {
            EXPECT_EQ(item->IsActive(), false) << "Item " << i << " should be inactive when completely off-screen";
        } else {
            EXPECT_EQ(item->IsActive(), true);  // must have one row
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount008
 * @tc.desc: Test overScroll at top with cachedCount=0, spring effect large drag and bounce back
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, apply large spring drag at top
     * @tc.expected: Items dragged down significantly with spring effect
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(info.currentOffset_, 0);

    // Simulate large spring drag at top (positive offset)
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(150.0f); // Drag down at top
    FlushUITasks();

    EXPECT_NEAR(info.currentOffset_, 150, 0.1);

    // All items should be pushed down
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 6) {
            EXPECT_EQ(item->IsActive(), false);
        } else {
            EXPECT_EQ(item->IsActive(), true);
        }
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount009
 * @tc.desc: Test overScroll at top with cachedCount=0, spring drag and release animation
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down and release for spring bounce back
     * @tc.expected: Items animate back to original position
     */
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(2);

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);

    // Drag down at top
    GestureEvent gesture;
    gesture.SetMainVelocity(200.0f);
    gesture.SetMainDelta(200.0f);
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchDown();
    scrollable->HandleDragStart(gesture);
    scrollable->HandleDragUpdate(gesture);
    FlushUITasks();

    EXPECT_TRUE(pattern_->OutBoundaryCallback());
    EXPECT_NEAR(info.currentOffset_, 200, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 0), 200, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 1), 200, 0.1);

    // Release and animate back
    scrollable->HandleTouchUp();
    scrollable->HandleDragEnd(gesture);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());

    // Should settle at or near 0
    EXPECT_NEAR(info.currentOffset_, 0, 0.1);
    EXPECT_NEAR(GetChildY(frameNode_, 0), 0, 0.1);

    // Verify all items in correct position and active
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item);
        EXPECT_TRUE(item->IsActive());
        int32_t row = i / 2;
        EXPECT_NEAR(GetChildY(frameNode_, i), row * 100.0f, 0.1);
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount010
 * @tc.desc: Test overScroll at top with cachedCount=0, drag and verify index range stability
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, verify index range remains stable during overScroll
     * @tc.expected: startIndex and endIndex don't change when dragging down at top
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;

    // Initial state
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);

    // Drag down at top (overScroll) - index range should not change
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(300.0f);
    FlushUITasks();

    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 1);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 0);

    // Verify items are pushed down but index range unchanged
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item);
        if (i >= 2) {
            EXPECT_EQ(item->IsActive(), false);
        } else {
            EXPECT_EQ(item->IsActive(), true);
        }
    }

    // Release and bounce back
    pattern_->scrollableEvent_->scrollable_->isTouching_ = false;
    UpdateCurrentOffset(-300.0f); // Move back towards 0
    FlushUITasks();

    // Should return to initial state
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);

    // Verify first 8 items are active again
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item);
        EXPECT_TRUE(item->IsActive());
        int32_t row = i / 2;
        EXPECT_NEAR(GetChildY(frameNode_, i), row * 100.0f, 0.1);
    }
}

/**
 * @tc.name: OverScrollAtTopWithCacheCount011
 * @tc.desc: Test overScroll at top with cachedCount=0, verify all items off-screen at bottom
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopWithCacheCount011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=0, drag down to push all items off-screen
     * @tc.expected: All items 0-7 become inactive (pushed below viewport)
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;

    // Drag down to push all items 0-7 below viewport (Y >= 400)
    UpdateCurrentOffset(600.0f);
    FlushUITasks();

    // Items 0-7: all pushed below viewport (Y >= 600)
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item) << "Item " << i << " should exist";
        if (i >= 2) {
            EXPECT_FALSE(item->IsActive()) << "Item " << i << " should be inactive when off-screen";
        } else {
            EXPECT_TRUE(item->IsActive());  // must have one row
        }
    }

    // Items 8-9: should not exist yet (not created)
    for (int32_t i = 8; i < 10; ++i) {
        auto item = GetItem(i, true);
        EXPECT_FALSE(item) << "Item " << i << " should not exist";
    }
}

/**
 * @tc.name: OverScrollAtTopNegativeEndMainLineIndex001
 * @tc.desc: When all content is pushed below viewport via top overscroll (endMainLineIndex_ < 0),
 *           the active range anchor stays at -1, -1 so no item is marked active while content is
 *           off-screen. With cachedCount > 0, FillCacheLineAtEnd must not trigger negative-main
 *           guard in CheckGridPlaced. After rollback, items must reappear correctly.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopNegativeEndMainLineIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with cachedCount=2, spring edge effect, 10 items, rowsGap=10
     * @tc.expected: Initial layout fills viewport with 8 items (0-7, lines 0-3)
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2, false);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    model.SetRowsGap(Dimension(10));
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);

    /**
     * @tc.steps: step2. Drag down 400px (= mainSize) to push all content below viewport.
     *             endMainLineIndex_ goes to -1 only when the offset falls in
     *             [mainSize, mainSize + mainGap). With rowsGap=10 this interval is [400, 410).
     * @tc.expected: endMainLineIndex_ goes negative (-1), active range stays -1, -1 (no active item)
     */
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(400.0f);
    FlushUITasks();

    // endMainLineIndex_ must be -1: the while-loop in UseCurrentLines never executed
    EXPECT_EQ(info.endMainLineIndex_, -1);

    // No item is active: the fallback branch keeps the anchor at -1, -1 when content is off-screen
    auto item0 = GetItem(0, true);
    ASSERT_TRUE(item0);
    EXPECT_FALSE(item0->IsActive()) << "Item 0 should not be active when all content is below viewport";

    /**
     * @tc.steps: step3. Release and bounce back to original position
     * @tc.expected: Items reappear, endMainLineIndex_ returns to 3, all visible items active
     */
    pattern_->scrollableEvent_->scrollable_->isTouching_ = false;
    UpdateCurrentOffset(-400.0f);
    FlushUITasks();

    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);

    // All visible items must be active again
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item);
        EXPECT_TRUE(item->IsActive()) << "Item " << i << " should be active after rollback";
    }
}

/**
 * @tc.name: OverScrollAtTopLargeDataScrollTo001
 * @tc.desc: With large data and scrollToIndex(0) at top overscroll, verify no persistent
 *           negative currentMainLineIndex_ state. First line must recover after rollback.
 *           This regression test covers the FillCacheLineAtEnd guard fix that prevents
 *           the negative-main guard in CheckGridPlaced from firing every frame.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, OverScrollAtTopLargeDataScrollTo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with large data (100 items), cachedCount=2, spring effect, rowsGap=10
     * @tc.expected: Initial layout fills viewport with 8 items (0-7, lines 0-3)
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2, false);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    model.SetRowsGap(Dimension(10));
    CreateItemsInLazyForEach(100, [](uint32_t idx) { return 100.0f; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);
    EXPECT_EQ(info.GetChildrenCount(), 100);

    /**
     * @tc.steps: step2. Drag down 400px (= mainSize) to push all content below viewport.
     *             endMainLineIndex_ goes to -1 only when offset is in [mainSize, mainSize+mainGap).
     * @tc.expected: endMainLineIndex_ = -1, active range stays -1, -1 (no active item)
     */
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    UpdateCurrentOffset(400.0f);
    FlushUITasks();

    EXPECT_EQ(info.endMainLineIndex_, -1);
    auto item0 = GetItem(0, true);
    ASSERT_TRUE(item0);
    EXPECT_FALSE(item0->IsActive());

    /**
     * @tc.steps: step3. ScrollToIndex(0) while in overscroll state
     * @tc.expected: Grid returns to top, endMainLineIndex_ recovers to non-negative,
     *               first line is properly laid out
     */
    pattern_->ScrollToIndex(0, false, ScrollAlign::START);
    FlushUITasks();

    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_GE(info.endMainLineIndex_, 0) << "endMainLineIndex_ must recover to non-negative after ScrollToIndex(0)";
    EXPECT_EQ(info.startIndex_, 0);

    // First line items must be active and properly positioned
    auto item = GetItem(0, true);
    ASSERT_TRUE(item);
    EXPECT_TRUE(item->IsActive());
    EXPECT_NEAR(GetChildY(frameNode_, 0), 0.0f, 0.1f);
    item = GetItem(1, true);
    ASSERT_TRUE(item);
    EXPECT_TRUE(item->IsActive());
    EXPECT_NEAR(GetChildY(frameNode_, 1), 0.0f, 0.1f);

    /**
     * @tc.steps: step4. Release touch and verify stable state
     * @tc.expected: Layout is stable, all visible items active
     */
    pattern_->scrollableEvent_->scrollable_->isTouching_ = false;
    FlushUITasks();

    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);
    for (int32_t i = 0; i <= 7; ++i) {
        auto item = GetItem(i, true);
        ASSERT_TRUE(item);
        EXPECT_TRUE(item->IsActive());
    }
}

// The sync-cache-in-layout strategy is implemented by GridScrollLayoutAlgorithm and
// GridScrollWithOptionsLayoutAlgorithm; in the TEST_IRREGULAR_GRID build the
// GridIrregularLayoutAlgorithm takes over (gridIrregularLayoutEnable_ + layout options
// set) and still fills its cache through the asynchronous preload task, so the sync
// tests that rely on SetLayoutOptions are excluded from that build.
#ifndef TEST_IRREGULAR_GRID
/**
 * @tc.name: SyncCacheLayout001
 * @tc.desc: Test sync-cache-in-layout: cache items are built in the same frame instead of
 *           being preloaded asynchronously on idle
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Same setup as Cache004, but hold the sync-cache-in-layout reference
     *             on the pattern (as an attached Repeat with allowReuseInAnimation=false
     *             does via ToggleParentSyncCacheLayout)
     * @tc.expected: the cache item below the view is built, measured and deactivated within
     *               the same frame, without running the idle preload task
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions({});
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    EXPECT_EQ(pattern_->info_.endIndex_, 23);
    auto item = GetItem(24, true);
    ASSERT_TRUE(item) << "Cache item should be built synchronously during Layout";
    EXPECT_FALSE(item->IsActive());
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 400.00) - [80.00 x 50.00]");

    /**
     * @tc.steps: step2. Release the reference: cache filling falls back to the asynchronous
     *             preload strategy
     * @tc.expected: after scrolling two rows, the new cache row (items 30-32) is not built
     *               until OnIdle runs
     */
    pattern_->RemoveSyncCacheLayoutChild();
    pattern_->UpdateCurrentOffset(-100.0f, SCROLL_FROM_UPDATE);
    FlushUITasks();
    auto nextCacheItem = GetItem(30, true);
    EXPECT_FALSE(nextCacheItem) << "Without the sync strategy the cache item is deferred to the idle task";
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(GetItem(30, true));
}

/**
 * @tc.name: SyncCacheLayout002
 * @tc.desc: Test GridScrollWithOptionsLayoutAlgorithm::GenerateSyncCacheItemConstraint with
 *           irregular indexes: a sync-built cache item spanning two columns is measured
 *           with the full cross size in the same frame
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Every item spans both columns of a two-column grid (irregular
     *             1x2), so each line holds exactly one item; hold the sync-cache-in-layout
     *             reference on the pattern
     * @tc.expected: the cache item below the view is built synchronously and its width is
     *               the full grid width (span from the layout options), not the single
     *               column size
     */
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    for (int32_t i = 0; i < 20; ++i) {
        option.irregularIndexes.insert(i);
    }
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(20, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    // one item per line, 8 lines visible in the 400px viewport
    EXPECT_EQ(pattern_->info_.endIndex_, 7);
    auto item = GetItem(8, true);
    ASSERT_TRUE(item) << "Irregular cache item should be built synchronously during Layout";
    EXPECT_FALSE(item->IsActive());
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 400.00) - [240.00 x 50.00]");
}

/**
 * @tc.name: SyncCacheLayout003
 * @tc.desc: Test the early-return branch of SyncLayoutCacheItems: with the grid cache
 *           feature disabled it returns even while the pattern holds the sync-cache
 *           reference, so no cache item is built in-frame and the asynchronous preload
 *           stays suppressed
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions({});
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    SystemProperties::gridCacheEnabled_ = false;
    CreateDone();
    SystemProperties::gridCacheEnabled_ = true;

    // viewport is laid out, but the cache item is neither built nor preloaded
    EXPECT_EQ(pattern_->info_.endIndex_, 23);
    EXPECT_FALSE(GetItem(24, true));
}

/**
 * @tc.name: SyncCacheLayout005
 * @tc.desc: A multi-row item straddling the visible/cache boundary keeps its active
 *           state: the sync-cache deactivation skips items that also cover a visible
 *           row (they are recorded in every covered gridMatrix_ row), because the
 *           SetActiveChildRange fallback is skipped for grids with multi-line items
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Single column of 100px items in the 400px viewport; item 3
     *             spans 2 rows, so it covers the last visible line 3 and the cache
     *             line 4; hold the sync-cache-in-layout reference
     * @tc.expected: item 3 stays active (one of its rows is visible) while the regular
     *               cache item below (item 4) is deactivated
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    option.irregularIndexes.insert(3);
    option.getSizeByIndex = [](int32_t index) {
        return index == 3 ? GridItemSize { 2, 1 } : GridItemSize { 1, 1 };
    };

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(2);
    model.SetLayoutOptions(option);
    CreateFixedHeightItems(10, ITEM_MAIN_SIZE);
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    FlushUITasks();

    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    // item 3 starts on the last visible line; the fill may account its span through
    // line 3 or line 4, both keep line 3 visible
    EXPECT_GE(pattern_->info_.endIndex_, 3);
    EXPECT_LE(pattern_->info_.endIndex_, 4);
    auto spanningItem = GetItem(3, true);
    ASSERT_TRUE(spanningItem) << "spanning item built with the viewport";
    EXPECT_TRUE(spanningItem->IsActive()) << "multi-row item covering a visible row must stay active";
    EXPECT_EQ(GetChildY(frameNode_, 3), 300.0f);
    auto cacheItem = GetItem(4, true);
    ASSERT_TRUE(cacheItem);
    EXPECT_FALSE(cacheItem->IsActive());
}

/**
 * @tc.name: SyncCacheLayout006
 * @tc.desc: Horizontal counterpart of SyncCacheLayout005: a multi-column item spanning
 *           the last visible column line and the cache line keeps its active state
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Rows-template-only grid (main axis horizontal), 4 cross lines of
     *             100px, viewport width 240px; items 0-7 fill the first two main lines,
     *             then item 8 spans 2 main lines (lines 2-3), so it covers the last
     *             visible line 2 and the first cache line 3
     * @tc.expected: item 8 stays active while the first regular item on a pure cache
     *               line (item 12, line 3) is deactivated
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    option.irregularIndexes.insert(8);
    option.getSizeByIndex = [](int32_t index) {
        return index == 8 ? GridItemSize { 1, 2 } : GridItemSize { 1, 1 };
    };

    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetCachedCount(2);
    model.SetLayoutOptions(option);
    CreateFixedItems(13);
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    FlushUITasks();

    auto spanningItem = GetItem(8, true);
    ASSERT_TRUE(spanningItem) << "spanning item built with the viewport";
    EXPECT_TRUE(spanningItem->IsActive()) << "multi-column item covering a visible line must stay active";
    EXPECT_EQ(GetChildX(frameNode_, 8), 200.0f);
    auto cacheItem = GetItem(12, true);
    ASSERT_TRUE(cacheItem);
    EXPECT_FALSE(cacheItem->IsActive());
}
#endif // !TEST_IRREGULAR_GRID

/**
 * @tc.name: SyncCacheLayout004
 * @tc.desc: Test the front fill of GridScrollLayoutAlgorithm::SyncLayoutCacheItems (no
 *           layout options set, so the base GridScrollLayoutAlgorithm and its
 *           GenerateSyncCacheItemConstraint run): after a jump the never-built head cache
 *           line above the viewport is built and positioned in the same frame
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Plain scrollable grid without layout options, sync strategy held;
     *             100 items in 3 columns of 50px, 8 visible lines
     * @tc.expected: initial layout builds the viewport (0-23) plus the tail cache line
     *               (24-26) in-frame
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    CreateItemsInLazyForEach(100, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    EXPECT_EQ(pattern_->info_.endIndex_, 23);
    EXPECT_TRUE(GetItem(24, true));

    /**
     * @tc.steps: step2. Jump down by 1000px (20 lines) with the sync strategy still held
     * @tc.expected: the head cache line above the viewport (line 19, items 57-59, never
     *               built before) is built, measured, positioned and deactivated in the
     *               same frame by the front fill; the async strategy would defer it to the
     *               idle preload task
     */
    pattern_->UpdateCurrentOffset(-1000.0f, SCROLL_FROM_UPDATE);
    FlushUITasks();

    EXPECT_EQ(pattern_->info_.startIndex_, 60);
    EXPECT_EQ(pattern_->info_.endIndex_, 83);
    EXPECT_TRUE(GetItem(60, true)->IsActive());
    auto headCache = GetItem(57, true);
    ASSERT_TRUE(headCache) << "Head cache line should be built synchronously during Layout";
    EXPECT_FALSE(headCache->IsActive());
    EXPECT_EQ(headCache->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, -50.00) - [80.00 x 50.00]");
}

/**
 * @tc.name: SyncCacheLayout007
 * @tc.desc: The front fill only measures the head cache items; the sync-cache pass also
 *           lays out their subtrees in the same frame, so a fresh head cache item with a
 *           padded fixed-size child positions the child at the padding offset
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Same jump as SyncCacheLayout004, but every item carries a
     *             padded subtree (GridMockLazyWithChild): GridItem with 5px padding
     *             containing a fixed 40x30 child
     * @tc.expected: after the jump the head cache item's child is positioned at the
     *               padding offset (5, 5) within the GridItem in the same frame;
     *               without the subtree layout its frame rect would stay at (0, 0)
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions({});
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazyForEachModelNG;
    RefPtr<LazyForEachActuator> mockLazy =
        AceType::MakeRefPtr<GridMockLazyWithChild>(100, [](int32_t) { return 50.0f; });
    lazyForEachModelNG.Create(mockLazy);
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    pattern_->UpdateCurrentOffset(-1000.0f, SCROLL_FROM_UPDATE);
    FlushUITasks();

    EXPECT_EQ(pattern_->info_.startIndex_, 60);
    auto headCache = GetItem(57, true);
    ASSERT_TRUE(headCache) << "Head cache line should be built synchronously during Layout";
    EXPECT_FALSE(headCache->IsActive());
    EXPECT_EQ(headCache->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, -50.00) - [80.00 x 50.00]");
    auto child = AceType::DynamicCast<FrameNode>(headCache->GetChildAtIndex(0));
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->GetGeometryNode()->GetFrameRect().ToString(),
        "RectT (5.00, 5.00) - [40.00 x 30.00]") << "head cache subtree laid out in-frame";
}

/**
 * @tc.name: SyncCacheLayout008
 * @tc.desc: A recorded tail cache line whose nodes were recycled by a far jump is
 *           rebuilt in-frame by the sync-cache fill after jumping back: the complete
 *           recorded line branch (RebuildRecordedLineItems) creates the missing nodes
 *           without waiting for the idle preload task
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayout008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. 100 items in 3 columns of 50px, cachedCount 1, sync strategy
     *             held: the initial frame builds the viewport (0-23) and the tail cache
     *             line 8 (items 24-26), whose placement is recorded in gridMatrix_
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions({});
    CreateItemsInLazyForEach(100, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    EXPECT_EQ(pattern_->info_.endIndex_, 23);
    ASSERT_TRUE(GetItem(24, true));
    EXPECT_FALSE(GetItem(24, true)->IsActive());

    /**
     * @tc.steps: step2. Jump to the end and let the builder's idle pass run: the tail
     *             cache nodes are recycled while their matrix records survive
     */
    ScrollToIndex(99, false, ScrollAlign::END);
    // line 33 bottom-aligned in an 8-line viewport: first visible line is 26
    EXPECT_EQ(pattern_->info_.startIndex_, 78);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_FALSE(GetItem(24, true)) << "recorded tail line node recycled by the jump";

    /**
     * @tc.steps: step3. Jump back to the top: line 8 is still complete in gridMatrix_
     *             but its nodes are gone; the recorded-line branch rebuilds them in the
     *             same frame
     * @tc.expected: items 24-26 exist, are positioned at the recorded line and stay
     *               inactive, without any pending idle preload task
     */
    ScrollToIndex(23, false, ScrollAlign::END);
    EXPECT_EQ(pattern_->info_.startIndex_, 0);
    auto rebuilt = GetItem(24, true);
    ASSERT_TRUE(rebuilt) << "recorded tail cache line rebuilt in-frame after jumping back";
    EXPECT_FALSE(rebuilt->IsActive());
    FlushUITasks();
    EXPECT_EQ(rebuilt->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 400.00) - [80.00 x 50.00]");
    EXPECT_TRUE(GetItem(25, true));
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
}

/**
 * @tc.name: SyncCacheLayoutIrregular001
 * @tc.desc: [GridIrregularLayoutAlgorithm] sync-cache-in-layout: with the irregular
 *           layout algorithm selected (system property + layout options), the cache
 *           items on both sides of the viewport are built, laid out and deactivated in
 *           the same frame instead of being preloaded asynchronously on idle
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayoutIrregular001, TestSize.Level1)
{
    // select the GridIrregularLayoutAlgorithm explicitly (same condition as
    // production), so this runs in both grid test builds
    const bool savedFlag = SystemProperties::gridIrregularLayoutEnable_;
    SystemProperties::gridIrregularLayoutEnable_ = true;

    /**
     * @tc.steps: step1. Two columns; every third item (0, 3, 6, ...) spans the full
     *             line, the rest are 1x1; hold the sync-cache-in-layout reference
     * @tc.expected: the tail cache items below the viewport are built, measured and
     *               deactivated in the same frame and the async preload list stays empty
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    for (int32_t i = 0; i < 30; i += 3) {
        option.irregularIndexes.insert(i);
    }
    option.getSizeByIndex = [](int32_t index) { return GridItemSize { 1, 2 }; };

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2);
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(30, [](uint32_t idx) { return 50.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    const int32_t tailIdx = info.endIndex_ + 1;
    ASSERT_LT(tailIdx, 30);
    auto tailCache = GetItem(tailIdx, true);
    ASSERT_TRUE(tailCache) << "irregular tail cache built synchronously during Layout";
    EXPECT_FALSE(tailCache->IsActive());
    EXPECT_EQ(GetChildHeight(frameNode_, tailIdx), 50.0f);

    /**
     * @tc.steps: step2. Scroll down: the new tail cache items are built in-frame on the
     *             irregular path as well
     */
    pattern_->UpdateCurrentOffset(-100.0f, SCROLL_FROM_UPDATE);
    FlushUITasks();
    const int32_t newTailIdx = info.endIndex_ + 1;
    ASSERT_LT(newTailIdx, 30);
    EXPECT_TRUE(GetItem(newTailIdx, true));
    EXPECT_FALSE(GetItem(newTailIdx, true)->IsActive());
    EXPECT_TRUE(pattern_->preloadItemList_.empty());

    /**
     * @tc.steps: step3. Jump back up: the head cache items above the viewport are
     *             laid out and deactivated in-frame
     */
    ScrollToIndex(4, false, ScrollAlign::START);
    EXPECT_GT(info.startIndex_, 0);
    const int32_t headIdx = info.startIndex_ - 1;
    ASSERT_GE(headIdx, 0);
    auto headCache = GetItem(headIdx, true);
    ASSERT_NE(headCache, nullptr) << "irregular head cache kept in-frame";
    EXPECT_FALSE(headCache->IsActive());

    SystemProperties::gridIrregularLayoutEnable_ = savedFlag;
}

/**
 * @tc.name: SyncCacheLayoutIrregular002
 * @tc.desc: [GridIrregularLayoutAlgorithm] the sync cache window is index-based; when the
 *           starting row is backtracked to a straddling item's top row, head cache line
 *           items outside the index window are skipped by the layout loop instead of
 *           being built and laid out
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncCacheLayoutIrregular002, TestSize.Level1)
{
    // select the GridIrregularLayoutAlgorithm explicitly (same condition as production)
    const bool savedFlag = SystemProperties::gridIrregularLayoutEnable_;
    SystemProperties::gridIrregularLayoutEnable_ = true;

    /**
     * @tc.steps: step1. Two columns, items of 100px, viewport 240x400 (4 lines), item 21
     *             spans 3 main lines at (10, 1), so the matrix around the jump target is
     *             line 9:  [18][19]
     *             line 10: [20][21]  (21 spans lines 10-12, col 1)
     *             line 11: [22][-21]
     *             cachedCount = 1 -> cacheCnt = 2
     * @tc.expected: initial layout fills lines 0-3 (items 0-7)
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    option.irregularIndexes.insert(21);
    option.getSizeByIndex = [](int32_t index) {
        return index == 21 ? GridItemSize { 3, 1 } : GridItemSize { 1, 1 };
    };

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1);
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(30, [](uint32_t idx) { return 100.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    /**
     * @tc.steps: step2. Jump to item 22 (line 11) with ScrollAlign::START: the covered tile
     *             of item 21 backtracks the starting row to line 10 (startIndex_ = 21), so
     *             the head cache index window is {19, 20} while the head cache line 9 also
     *             holds item 18
     * @tc.expected: item 18 stays unbuilt (outside the index window, only inside the
     *               row-based cache range), while the in-window items are built, laid out
     *               and deactivated in the same frame
     */
    ScrollToIndex(22, false, ScrollAlign::START);

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startMainLineIndex_, 10);
    EXPECT_EQ(info.startIndex_, 21);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    EXPECT_FALSE(GetItem(18, true)) << "out-of-window head cache item must not be built";
    auto headCache = GetItem(19, true);
    ASSERT_TRUE(headCache) << "in-window head cache item built synchronously during Layout";
    EXPECT_FALSE(headCache->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 19), -200.0f);
    ASSERT_TRUE(GetItem(20, true));
    EXPECT_EQ(GetChildY(frameNode_, 20), -100.0f);
    EXPECT_TRUE(GetItem(21, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 21), -100.0f);
    EXPECT_TRUE(GetItem(22, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 22), 0.0f);

    SystemProperties::gridIrregularLayoutEnable_ = savedFlag;
}

/**
 * @tc.name: ShowCacheIrregular001
 * @tc.desc: [GridIrregularLayoutAlgorithm] with showCachedItems = true and a
 *           sync-cache-layout child attached (the adjustments are isolated to the feature)
 *           the shown preload window is index-based (SyncPreloadItems): when the starting
 *           row is backtracked to a straddling item's top row, head cache line items
 *           outside the index window are neither built by the preload nor laid out by the
 *           layout loop, while the in-window cache items stay attached, active and
 *           positioned
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, ShowCacheIrregular001, TestSize.Level1)
{
    // select the GridIrregularLayoutAlgorithm explicitly (same condition as production)
    const bool savedFlag = SystemProperties::gridIrregularLayoutEnable_;
    SystemProperties::gridIrregularLayoutEnable_ = true;

    /**
     * @tc.steps: step1. Two columns, items of 100px, viewport 240x400 (4 lines), item 21
     *             spans 3 main lines at (10, 1), so the matrix around the jump target is
     *             line 9:  [18][19]
     *             line 10: [20][21]  (21 spans lines 10-12, col 1)
     *             line 11: [22][-21]
     *             showCache = true, cachedCount = 1 -> cacheCnt = 2, sync-cache-layout
     *             child attached
     * @tc.expected: initial layout fills lines 0-3 (items 0-7)
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    option.irregularIndexes.insert(21);
    option.getSizeByIndex = [](int32_t index) {
        return index == 21 ? GridItemSize { 3, 1 } : GridItemSize { 1, 1 };
    };

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1, true);
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(30, [](uint32_t idx) { return 100.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    /**
     * @tc.steps: step2. Jump to item 22 (line 11) with ScrollAlign::START: the covered tile
     *             of item 21 backtracks the starting row to line 10 (startIndex_ = 21), so
     *             the head cache index window is {19, 20} while the head cache line 9 also
     *             holds item 18
     * @tc.expected: item 18 stays unbuilt (outside the index window, only inside the
     *               row-based cache range), while the in-window cache items are attached,
     *               active and positioned in the same frame
     */
    ScrollToIndex(22, false, ScrollAlign::START);

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startMainLineIndex_, 10);
    EXPECT_EQ(info.startIndex_, 21);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    EXPECT_FALSE(GetItem(18, true)) << "out-of-window head cache item must not be built";
    auto headCache = GetItem(19, true);
    ASSERT_TRUE(headCache) << "in-window head cache item built by the shown preload";
    EXPECT_TRUE(headCache->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 19), -200.0f);
    ASSERT_TRUE(GetItem(20, true));
    EXPECT_TRUE(GetItem(20, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 20), -100.0f);
    EXPECT_TRUE(GetItem(21, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 21), -100.0f);
    EXPECT_TRUE(GetItem(22, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 22), 0.0f);

    /**
     * @tc.steps: step3. Check the tail side of the shown window ({28, 29} below the
     *             viewport on line 15)
     * @tc.expected: both tail cache items are built, active and laid out at line 15
     */
    ASSERT_TRUE(GetItem(28, true));
    EXPECT_TRUE(GetItem(28, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 28), 400.0f);
    ASSERT_TRUE(GetItem(29, true));
    EXPECT_TRUE(GetItem(29, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 29), 400.0f);

    SystemProperties::gridIrregularLayoutEnable_ = savedFlag;
}

/**
 * @tc.name: IrregularStartLineBacktrack001
 * @tc.desc: [GridIrregularLayoutAlgorithm] with a sync-cache-layout child attached (the
 *           adjustment is isolated to the feature) and cachedCount = 0, when the starting
 *           line is backtracked to a straddling item's top row (which lies above the
 *           display top), the straddler's row-mates on that line are outside
 *           [startIndex_, endIndex_] and are no longer laid out by the layout loop even
 *           though their line is walked as a viewport line
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, IrregularStartLineBacktrack001, TestSize.Level1)
{
    // select the GridIrregularLayoutAlgorithm explicitly (same condition as production)
    const bool savedFlag = SystemProperties::gridIrregularLayoutEnable_;
    SystemProperties::gridIrregularLayoutEnable_ = true;

    /**
     * @tc.steps: step1. Two columns, items of 100px, viewport 240x400 (4 lines), item 21
     *             spans 3 main lines at (10, 1), so the matrix around the jump target is
     *             line 9:  [18][19]
     *             line 10: [20][21]  (21 spans lines 10-12, col 1)
     *             line 11: [22][-21]
     *             cachedCount = 0 with a sync-cache-layout child attached (sync strategy,
     *             empty cache window)
     * @tc.expected: initial layout fills lines 0-3 (items 0-7)
     */
    GridLayoutOptions option;
    option.regularSize = GridItemSize { 1, 1 };
    option.irregularIndexes.insert(21);
    option.getSizeByIndex = [](int32_t index) {
        return index == 21 ? GridItemSize { 3, 1 } : GridItemSize { 1, 1 };
    };

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(30, [](uint32_t idx) { return 100.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    /**
     * @tc.steps: step2. Jump to item 22 (line 11) with ScrollAlign::START: the covered tile
     *             of item 21 backtracks the starting line to line 10 above the display top
     *             (startIndex_ = 21), so line 10 is walked as a viewport line although item
     *             20 on it is outside the empty index window
     * @tc.expected: item 20 is measured by the jump solve (the height of line 10 needs it)
     *               but never laid out — a freshly built node keeps the zero offset — and
     *               is deactivated by the controller in the same frame; the viewport items
     *               and the straddler crossing the display top are laid out normally
     */
    ScrollToIndex(22, false, ScrollAlign::START);

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startMainLineIndex_, 10);
    EXPECT_EQ(info.startIndex_, 21);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_TRUE(pattern_->preloadItemList_.empty());
    // no cache line items exist with cachedCount = 0: lines 9 and 15 stay unbuilt
    EXPECT_FALSE(GetItem(19, true));
    EXPECT_FALSE(GetItem(28, true));
    // measured for the line 10 height, but outside [startIndex_, endIndex_]: never laid
    // out, so the freshly built node keeps the zero offset
    ASSERT_TRUE(GetItem(20, true));
    EXPECT_EQ(GetChildY(frameNode_, 20), 0.0f);
    EXPECT_FALSE(GetItem(20, true)->IsActive());
    // the straddler crossing the display top and the viewport items are laid out normally
    EXPECT_TRUE(GetItem(21, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 21), -100.0f);
    EXPECT_TRUE(GetItem(22, true)->IsActive());
    EXPECT_EQ(GetChildY(frameNode_, 22), 0.0f);
    EXPECT_EQ(GetChildY(frameNode_, 27), 300.0f);

    SystemProperties::gridIrregularLayoutEnable_ = savedFlag;
}

/**
 * @tc.name: SyncPreloadItemsIrregularGuard001
 * @tc.desc: [GridIrregularLayoutAlgorithm::SyncPreloadItems] the head-cache index loop
 *           skips an item whose resolved matrix tile is not a valid placement, covering
 *           every sign combination of the pos.first / pos.second guard: an item missing
 *           from the matrix, an item at a negative column and an item at a negative row
 *           are all skipped without being built, while an in-window item placed at a
 *           valid (col >= 0, row >= 0) tile is measured and built. The negative-column
 *           and negative-row entries are planted into gridMatrix_ directly (production
 *           fillers only place non-negative keys), so the defensive combinations of
 *           the guard can be exercised.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadItemsIrregularGuard001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Two columns, items of 100px, viewport 240x400 (4 lines),
     *             cachedCount = 2 with showCachedItems -> production cacheCnt = 4;
     *             sync-cache-layout child attached so the head window takes the
     *             index-based loop. Initial layout fills lines 0-3 (items 0-7), so
     *             the head-side items 16-19 stay unbuilt
     * @tc.expected: the grid starts at the top with items 0-7 in the viewport
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2, true);
    CreateItemsInLazyForEach(30, [](uint32_t idx) { return 100.0f; });
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 7);
    EXPECT_FALSE(GetItem(16, true));

    /**
     * @tc.steps: step2. Drive the algorithm directly (the same direct-call pattern as
     *             CheckSyncPreloadLineShift): viewport at lines 10-13 (items 20-27), so
     *             the head cache index window is {16..19}. Plant the guarded tiles:
     *             item 19 at (col 1, row 9) — the valid control tile,
     *             item 18 at (col -2, row 9) — negative column,
     *             item 17 at (col 1, row -1) — negative row,
     *             item 16 — no matrix entry at all
     * @tc.expected: GetItemPos resolves all four sign combinations of the guard
     */
    GridIrregularLayoutAlgorithm algorithm(pattern_->info_);
    algorithm.wrapper_ = AceType::RawPtr(frameNode_);
    algorithm.crossLens_ = { 120.0f, 120.0f };
    algorithm.crossGap_ = 0.0f;
    algorithm.mainGap_ = 0.0f;
    auto& algInfo = algorithm.info_;
    algInfo.gridMatrix_.clear();
    algInfo.lineHeightMap_.clear();
    algInfo.startIndex_ = 20;
    algInfo.endIndex_ = 27;
    algInfo.startMainLineIndex_ = 10;
    algInfo.endMainLineIndex_ = 13;
    // viewport lines 10-13
    for (int32_t row = 10; row <= 13; ++row) {
        algInfo.gridMatrix_[row][0] = 2 * row;
        algInfo.gridMatrix_[row][1] = 2 * row + 1;
        algInfo.lineHeightMap_[row] = 100.0f;
    }
    // head cache line 9: the valid control tile and the negative-column tile
    algInfo.gridMatrix_[9][1] = 19;
    algInfo.gridMatrix_[9][-2] = 18;
    algInfo.lineHeightMap_[9] = 100.0f;
    // negative row above the solved range: the negative-row tile
    algInfo.gridMatrix_[-1][1] = 17;

    // (pos.first < 0 = false, pos.second < 0 = false)
    EXPECT_EQ(algInfo.GetItemPos(19), (std::pair<int32_t, int32_t>(1, 9)));
    // (pos.first < 0 = true, pos.second < 0 = false)
    EXPECT_EQ(algInfo.GetItemPos(18), (std::pair<int32_t, int32_t>(-2, 9)));
    // (pos.first < 0 = false, pos.second < 0 = true)
    EXPECT_EQ(algInfo.GetItemPos(17), (std::pair<int32_t, int32_t>(1, -1)));
    // (pos.first < 0 = true, pos.second < 0 = true)
    EXPECT_EQ(algInfo.GetItemPos(16), (std::pair<int32_t, int32_t>(-1, -1)));

    /**
     * @tc.steps: step3. Run the shown preload with the production cache window
     * @tc.expected: the guarded items (16, 17, 18) are skipped and stay unbuilt, while
     *               the valid tile (19) is measured and built; the planted entries
     *               survive the pass untouched
     */
    algorithm.SyncPreloadItems(4);

    EXPECT_FALSE(GetItem(16, true)) << "missing-entry item is skipped";
    EXPECT_FALSE(GetItem(17, true)) << "negative-row item is skipped";
    EXPECT_FALSE(GetItem(18, true)) << "negative-column item is skipped";
    ASSERT_TRUE(GetItem(19, true)) << "valid-tile item is measured by the head window";
    EXPECT_EQ(algInfo.GetItemPos(18), (std::pair<int32_t, int32_t>(-2, 9)));
    EXPECT_EQ(algInfo.GetItemPos(17), (std::pair<int32_t, int32_t>(1, -1)));
}

/**
 * @tc.name: SyncPreloadAfterDataReloadAndScrollToIndex
 * @tc.desc: When Grid is single column with showCached=true, after data reload triggers
 *           ScrollToIndex, verify that the cache line below the viewport is correctly created.
 *           moveToEndLineIndex_ set during ScrollToIndex(AUTO) should not leak into SyncPreload
 *           and block cache line creation in FillNewLineBackward.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadAfterDataReloadAndScrollToIndex, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid with 1 column, showCached=true, cachedCount=1,
     *             20 items each 100px height in viewport of 400px
     * @tc.expected: Initial layout fills viewport with 4 items (indices 0-3, lines 0-3)
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(1, true);
    CreateItemsInLazyForEach(20, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    const auto& info = pattern_->info_;
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 3);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 3);

    /**
     * @tc.steps: step2. Simulate data reload by calling ChildrenUpdatedFrom(0)
     *             (equivalent to data source reset), then immediately call ScrollToIndex
     *             before the next layout pass.
     * @tc.expected: Both signals are queued; next Measure will process reload + scroll together
     */
    frameNode_->ChildrenUpdatedFrom(0);

    ScrollToIndex(5, false, ScrollAlign::AUTO);
    FlushUITasks();

    /**
     * @tc.steps: step3. After layout is done, verify the cache line below the viewport
     *             (line after endMainLineIndex_, containing endIndex_+1) has been created.
     *             This verifies that moveToEndLineIndex_ was cleared before SyncPreload ran.
     * @tc.expected: Item at endIndex_+1 (the first cache line) has a wrapper node.
     */
    EXPECT_FALSE(info.reachEnd_);
    int32_t cacheItemIdx = info.endIndex_ + 1;
    EXPECT_TRUE(GetItem(cacheItemIdx, true))
        << "Cache item " << cacheItemIdx << " should exist after data reload + ScrollToIndex";
}

void GridCacheLayoutTestNg::InitSyncPreloadAlgorithm(GridScrollLayoutAlgorithm& algorithm, int32_t itemCount)
{
    algorithm.wrapper_ = AceType::RawPtr(frameNode_);
    algorithm.frameSize_ = frameNode_->GetGeometryNode()->GetFrameSize();
    algorithm.syncLoad_ = true;
    algorithm.InitialItemsCrossSize(
        layoutProperty_, algorithm.frameSize_, itemCount, algorithm.frameSize_.Width());
    auto& info = algorithm.info_;
    info.gridMatrix_.clear();
    info.lineHeightMap_.clear();
    info.ResetPositionFlags();
    info.currentOffset_ = PRELOAD_TEST_OFFSET;
    info.prevOffset_ = info.currentOffset_;
}

void GridCacheLayoutTestNg::CheckSyncPreloadRange(
    const GridLayoutInfo& info, int32_t startIndex, int32_t endIndex, int32_t startLine, int32_t endLine)
{
    EXPECT_EQ(info.startIndex_, startIndex);
    EXPECT_EQ(info.endIndex_, endIndex);
    EXPECT_EQ(info.startMainLineIndex_, startLine);
    EXPECT_EQ(info.endMainLineIndex_, endLine);
    EXPECT_FLOAT_EQ(info.currentOffset_, PRELOAD_TEST_OFFSET);
    EXPECT_FLOAT_EQ(info.prevOffset_, PRELOAD_TEST_OFFSET);
}

void GridCacheLayoutTestNg::CheckSyncPreloadLineShift(
    int32_t startIndex, int32_t lineOffset, int32_t cacheCount, bool recordedTail, int32_t crossCount)
{
    constexpr int32_t SINGLE_COLUMN_COUNT = 1;
    constexpr int32_t DOUBLE_COLUMN_COUNT = 2;
    constexpr int32_t VISIBLE_LINE_COUNT = 3;
    if (crossCount != SINGLE_COLUMN_COUNT && crossCount != DOUBLE_COLUMN_COUNT) {
        ADD_FAILURE() << "crossCount must be one or two";
        return;
    }
    const int32_t columnCount = crossCount == SINGLE_COLUMN_COUNT ? SINGLE_COLUMN_COUNT : DOUBLE_COLUMN_COUNT;
    ASSERT_EQ(startIndex % columnCount, 0);
    const int32_t endIndex = startIndex + VISIBLE_LINE_COUNT * columnCount - 1;
    const int32_t itemCount = endIndex + 1 + std::max(cacheCount, 1) * columnCount;
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate(columnCount == SINGLE_COLUMN_COUNT ? "1fr" : "1fr 1fr");
    model.SetCachedCount(cacheCount, true);
    CreateFixedItems(itemCount);
    CreateDone();

    // Reproduce the partial matrix left by reload. Exercise the real forward
    // merge and backward fill; only their input state is supplied by the test.
    GridScrollLayoutAlgorithm algorithm(pattern_->info_);
    InitSyncPreloadAlgorithm(algorithm, itemCount);
    auto& info = algorithm.info_;
    info.startIndex_ = startIndex;
    info.endIndex_ = endIndex;
    info.startMainLineIndex_ = startIndex / columnCount + lineOffset;
    info.endMainLineIndex_ = endIndex / columnCount + lineOffset;
    const int32_t recordedEnd = endIndex + (recordedTail ? columnCount : 0);
    for (int32_t index = startIndex; index <= recordedEnd; ++index) {
        const int32_t row = index / columnCount + lineOffset;
        info.gridMatrix_[row][index % columnCount] = index;
        info.lineHeightMap_[row] = ITEM_MAIN_SIZE;
    }

    algorithm.SyncPreload(AceType::RawPtr(frameNode_), cacheCount,
        algorithm.frameSize_.Width(), algorithm.frameSize_.Height());

    // For regular items the canonical row is the item index divided by the column count.
    // Compare the entire matrix to catch gaps, duplicates and extra cache rows.
    decltype(info.gridMatrix_) expectedMatrix;
    decltype(info.lineHeightMap_) expectedHeights;
    const int32_t expectedStart = std::max(startIndex - cacheCount * columnCount, 0);
    const int32_t expectedEnd = endIndex + cacheCount * columnCount;
    for (int32_t index = expectedStart; index <= expectedEnd; ++index) {
        expectedMatrix[index / columnCount][index % columnCount] = index;
        expectedHeights[index / columnCount] = ITEM_MAIN_SIZE;
    }
    EXPECT_EQ(info.gridMatrix_, expectedMatrix);
    EXPECT_EQ(info.lineHeightMap_, expectedHeights);
    CheckSyncPreloadRange(info, startIndex, endIndex, startIndex / columnCount, endIndex / columnCount);
    EXPECT_FALSE(algorithm.measureInNextFrame_);
}

/**
 * @tc.name: SyncPreloadAfterForwardLineShift
 * @tc.desc: A five-item reload must place the last item on row 4 after prepending item 0.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadAfterForwardLineShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 1, 1, false);
}

/**
 * @tc.name: SyncPreloadMultipleLineShift
 * @tc.desc: Backward preload must follow a forward merge that removes three excess rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleLineShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 3, 1, false);
}

/**
 * @tc.name: SyncPreloadMultipleLineShiftTwoCachedLines
 * @tc.desc: A three-row shift must preserve both consecutive backward cache rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleLineShiftTwoCachedLines, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 3, 2, false);
}

/**
 * @tc.name: SyncPreloadMultipleLineShiftThreeCachedLines
 * @tc.desc: A three-row shift must not accumulate extra gaps across three cache iterations.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleLineShiftThreeCachedLines, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 3, 3, false);
}

/**
 * @tc.name: SyncPreloadMultipleCachedLinesWithoutShift
 * @tc.desc: Three backward cache rows stay consecutive when the forward merge does not shift rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleCachedLinesWithoutShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 0, 3, false);
}

/**
 * @tc.name: SyncPreloadShiftWithRecordedTail
 * @tc.desc: Reuse a shifted tail cache row before appending the remaining two cache rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadShiftWithRecordedTail, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 3, 3, true);
}

/**
 * @tc.name: SyncPreloadRepeatedForwardShift
 * @tc.desc: Each prepended item shifts the matrix, including tail rows added in previous iterations.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadRepeatedForwardShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(3, -3, 3, false);
}

/**
 * @tc.name: SyncPreloadZeroCachedLines
 * @tc.desc: A zero cache count preserves the partial matrix and the visible range.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadZeroCachedLines, TestSize.Level1)
{
    CheckSyncPreloadLineShift(1, 0, 0, false);
}

/**
 * @tc.name: SyncPreloadMultipleColumnsLineShift
 * @tc.desc: Two-column rows remain continuous after removing three excess row indices.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleColumnsLineShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(2, 3, 1, false, 2);
}

/**
 * @tc.name: SyncPreloadMultipleColumnsCachedLines
 * @tc.desc: Reuse a two-column tail row and append three cache rows after a multi-row shift.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleColumnsCachedLines, TestSize.Level1)
{
    CheckSyncPreloadLineShift(2, 3, 3, true, 2);
}

/**
 * @tc.name: SyncPreloadMultipleColumnsRepeatedShift
 * @tc.desc: Each forward iteration shifts both visible and previously cached two-column rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadMultipleColumnsRepeatedShift, TestSize.Level1)
{
    CheckSyncPreloadLineShift(6, -3, 3, false, 2);
}

void GridCacheLayoutTestNg::CheckSyncPreloadBigItem(bool moveRecordedRows, int32_t cacheCount)
{
    constexpr int32_t CROSS_COUNT = 2;
    constexpr int32_t MAIN_SPAN = 3;
    constexpr int32_t VISIBLE_LINE_COUNT = 2;
    const int32_t bigItem = moveRecordedRows ? 0 : 1;
    const int32_t startIndex = bigItem + 1;
    const int32_t endIndex = startIndex + VISIBLE_LINE_COUNT * CROSS_COUNT - 1;
    const int32_t startLine = moveRecordedRows ? 1 : bigItem + MAIN_SPAN;
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(cacheCount, true);
    CreateFixedItems(bigItem);
    CreateBigItem(bigItem, bigItem + MAIN_SPAN - 1, 0, CROSS_COUNT - 1,
        NULL_VALUE, MAIN_SPAN * ITEM_MAIN_SIZE);
    CreateFixedItems((VISIBLE_LINE_COUNT + cacheCount) * CROSS_COUNT);
    CreateDone();

    GridScrollLayoutAlgorithm algorithm(pattern_->info_);
    InitSyncPreloadAlgorithm(algorithm, endIndex + 1 + cacheCount * CROSS_COUNT);
    auto& info = algorithm.info_;
    info.startIndex_ = startIndex;
    info.endIndex_ = endIndex;
    info.startMainLineIndex_ = startLine;
    info.endMainLineIndex_ = startLine + VISIBLE_LINE_COUNT - 1;
    for (int32_t index = startIndex; index <= endIndex; ++index) {
        const int32_t row = startLine + (index - startIndex) / CROSS_COUNT;
        info.gridMatrix_[row][(index - startIndex) % CROSS_COUNT] = index;
        info.lineHeightMap_[row] = ITEM_MAIN_SIZE;
    }

    algorithm.SyncPreload(AceType::RawPtr(frameNode_), cacheCount,
        algorithm.frameSize_.Width(), algorithm.frameSize_.Height());

    // The leading item occupies three complete rows. The two visible rows and
    // every tail cache row must follow it, irrespective of the temporary input row numbers.
    decltype(info.gridMatrix_) expectedMatrix;
    decltype(info.lineHeightMap_) expectedHeights;
    for (int32_t row = bigItem; row < bigItem + MAIN_SPAN; ++row) {
        expectedMatrix[row] = { { 0, bigItem }, { 1, bigItem } };
        expectedHeights[row] = ITEM_MAIN_SIZE;
    }
    for (int32_t index = startIndex; index <= endIndex + cacheCount * CROSS_COUNT; ++index) {
        const int32_t row = bigItem + MAIN_SPAN + (index - startIndex) / CROSS_COUNT;
        expectedMatrix[row][(index - startIndex) % CROSS_COUNT] = index;
        expectedHeights[row] = ITEM_MAIN_SIZE;
    }
    EXPECT_EQ(info.gridMatrix_, expectedMatrix);
    EXPECT_EQ(info.lineHeightMap_, expectedHeights);
    CheckSyncPreloadRange(
        info, startIndex, endIndex, bigItem + MAIN_SPAN, bigItem + MAIN_SPAN + VISIBLE_LINE_COUNT - 1);
    EXPECT_FALSE(algorithm.measureInNextFrame_);
}

/**
 * @tc.name: SyncPreloadBigItemMovesRecordedRows
 * @tc.desc: Prepending a three-row item moves the old rows and the next tail cache row together.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadBigItemMovesRecordedRows, TestSize.Level1)
{
    CheckSyncPreloadBigItem(true, 1);
}

/**
 * @tc.name: SyncPreloadBigItemMovesRecordedRowsMultipleCachedLines
 * @tc.desc: A positive multi-row shift must retain three consecutive tail cache rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadBigItemMovesRecordedRowsMultipleCachedLines, TestSize.Level1)
{
    CheckSyncPreloadBigItem(true, 3);
}

/**
 * @tc.name: SyncPreloadBigItemOverlappingMerge
 * @tc.desc: The overlapping merge preserves old row numbers while shifting the newly filled big item.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadBigItemOverlappingMerge, TestSize.Level1)
{
    CheckSyncPreloadBigItem(false, 1);
}

/**
 * @tc.name: SyncPreloadBigItemOverlappingMergeMultipleCachedLines
 * @tc.desc: An overlapping big-item merge preserves three tail cache rows without shifting them twice.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadBigItemOverlappingMergeMultipleCachedLines, TestSize.Level1)
{
    CheckSyncPreloadBigItem(false, 3);
}

/**
 * @tc.name: SyncPreloadColumnChange
 * @tc.desc: Five items remain consecutive after changing from two columns to one with visible cache rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, SyncPreloadColumnChange, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1, true);
    CreateFixedItems(5);
    CreateDone();

    const decltype(pattern_->info_.gridMatrix_) expectedMatrix = {
        { 0, { { 0, 0 } } }, { 1, { { 0, 1 } } }, { 2, { { 0, 2 } } },
        { 3, { { 0, 3 } } }, { 4, { { 0, 4 } } }
    };
    for (int32_t cacheCount = 1; cacheCount <= 3; ++cacheCount) {
        SCOPED_TRACE(cacheCount);
        layoutProperty_->UpdateColumnsTemplate("1fr 1fr");
        layoutProperty_->UpdateCachedCount(cacheCount);
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        FlushUITasks();
        ASSERT_EQ(pattern_->info_.crossCount_, 2);

        layoutProperty_->UpdateColumnsTemplate("1fr");
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        FlushUITasks();
        ASSERT_EQ(pattern_->info_.crossCount_, 1);
        EXPECT_EQ(pattern_->info_.gridMatrix_, expectedMatrix);
        for (int32_t index = 0; index < 5; ++index) {
            EXPECT_FLOAT_EQ(GetChildY(frameNode_, index), index * ITEM_MAIN_SIZE);
        }
    }
}

/**
 * @tc.name: UpdateGridDecodeCacheWindow001
 * @tc.desc: Test UpdateGridDecodeCacheWindow guard conditions do not crash.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, UpdateGridDecodeCacheWindow001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(20);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindow(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateGridDecodeCacheWindow002
 * @tc.desc: Test UpdateGridDecodeCacheWindow with showCachedItems=true skips gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, UpdateGridDecodeCacheWindow002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5, true); // 5 : cacheCount, true : showCachedItems
    CreateFixedItems(20);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindow(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateGridDecodeCacheWindow003
 * @tc.desc: Test UpdateGridDecodeCacheWindow with cacheCount=0 skips gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, UpdateGridDecodeCacheWindow003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    CreateFixedItems(20);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindow(AceType::RawPtr(wrapper), 0);
}

/**
 * @tc.name: UpdateGridDecodeCacheWindow004
 * @tc.desc: Test UpdateGridDecodeCacheWindow does not crash after scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, UpdateGridDecodeCacheWindow004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(30);
    CreateDone();
    for (int32_t i = 0; i < 5; ++i) {
        pattern_->ScrollBy(ITEM_MAIN_SIZE);
        FlushUITasks();
    }
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindow(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: GridCalcDecodeWindowCount001
 * @tc.desc: Test CalcDecodeWindowCount returns correct window size for various cacheCount values.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridCalcDecodeWindowCount001, TestSize.Level1)
{
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(0), 0);   // 0 : cacheCount
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(1), 1);   // 1 : cacheCount
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(2), 1);   // 2 : cacheCount
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(5), 1);   // 5 : cacheCount
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(6), 2);   // 6 : cacheCount, 2 : decodeWindow
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(10), 2);  // 10 : cacheCount
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(11), 3);  // 11 : cacheCount, 3 : decodeWindow
    EXPECT_EQ(ScrollableUtils::CalcDecodeWindowCount(25), 5);  // 25 : cacheCount, 5 : decodeWindow
}

/**
 * @tc.name: GridIsLowMemoryDevice001
 * @tc.desc: Test IsLowMemoryDevice threshold logic (<=8GiB is low memory).
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridIsLowMemoryDevice001, TestSize.Level1)
{
    EXPECT_FALSE(ScrollableUtils::IsLowMemoryDevice(0));  // 0 : invalid
    EXPECT_TRUE(ScrollableUtils::IsLowMemoryDevice(1));   // 1 : GiB, low memory
    EXPECT_TRUE(ScrollableUtils::IsLowMemoryDevice(8));   // 8 : GiB, boundary
    EXPECT_FALSE(ScrollableUtils::IsLowMemoryDevice(9));  // 9 : GiB, not low memory
    EXPECT_FALSE(ScrollableUtils::IsLowMemoryDevice(12)); // 12 : GiB, not low memory
}

/**
 * @tc.name: GridCollectDecodeItems001
 * @tc.desc: Test CollectDecodeItems with empty gridMatrix returns empty set.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridCollectDecodeItems001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.gridMatrix_.clear();
    auto result = layoutAlgorithm->CollectDecodeItems(0, 5);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GridCollectDecodeItems002
 * @tc.desc: Test CollectDecodeItems collects items within range and excludes items outside.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridCollectDecodeItems002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
        {1, {{0, 2}, {1, 3}}},
        {2, {{0, 4}, {1, 5}}},
    };
    auto result = layoutAlgorithm->CollectDecodeItems(0, 1);
    EXPECT_EQ(result.size(), 4u);
    EXPECT_TRUE(result.count(0) > 0);
    EXPECT_TRUE(result.count(1) > 0);
    EXPECT_TRUE(result.count(2) > 0);
    EXPECT_TRUE(result.count(3) > 0);
    EXPECT_TRUE(result.count(4) == 0);
    EXPECT_TRUE(result.count(5) == 0);
}

/**
 * @tc.name: GridCollectDecodeItems003
 * @tc.desc: Test CollectDecodeItems handles negative itemIdx (non-origin cells of multi-row items).
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridCollectDecodeItems003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    // Item 1 spans rows 0-1: origin at (0,1) idx=1, non-origin at (1,1) idx=-1
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
        {1, {{0, 2}, {1, -1}}},
    };
    auto result = layoutAlgorithm->CollectDecodeItems(0, 1);
    EXPECT_EQ(result.size(), 3u);
    EXPECT_TRUE(result.count(0) > 0);
    EXPECT_TRUE(result.count(1) > 0);  // abs(-1) = 1
    EXPECT_TRUE(result.count(2) > 0);
}

/**
 * @tc.name: GridApplyItemDecodeState001
 * @tc.desc: Test ApplyItemDecodeState: item in decodeItems stays active, item not in decodeItems deactivated.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    auto item1 = GetItem(1, true);
    ASSERT_NE(item0, nullptr);
    ASSERT_NE(item1, nullptr);
    auto image0 = CreateImageFrameNode();
    auto image1 = CreateImageFrameNode();
    ASSERT_NE(image0, nullptr);
    ASSERT_NE(image1, nullptr);
    item0->AddChild(image0);
    item1->AddChild(image1);
    auto pattern0 = image0->GetPattern<ImagePattern>();
    auto pattern1 = image1->GetPattern<ImagePattern>();
    ASSERT_NE(pattern0, nullptr);
    ASSERT_NE(pattern1, nullptr);

    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
    };
    std::set<int32_t> decodeItems = {0};
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, decodeItems);
    EXPECT_TRUE(pattern0->decodeCacheActive_);   // in decodeItems, stays active
    EXPECT_FALSE(pattern1->decodeCacheActive_);  // not in decodeItems, deactivated
}

/**
 * @tc.name: GridApplyItemDecodeState002
 * @tc.desc: Test ApplyItemDecodeState marks items NOT in decodeItems as inactive.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);

    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
    };
    std::set<int32_t> decodeItems;  // empty: no items in decode window
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, decodeItems);
    EXPECT_FALSE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridApplyItemDecodeState003
 * @tc.desc: Test ApplyItemDecodeState deduplicates items by itemIdx (multi-row items processed once).
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item1 = GetItem(1, true);
    ASSERT_NE(item1, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item1->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);

    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    // Overwrite gridMatrix_ to simulate item 1 spanning rows 0-1
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
        {1, {{0, 2}, {1, -1}}},
    };
    std::set<int32_t> decodeItems;
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, decodeItems);
    EXPECT_FALSE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridSetDecodeCacheActive001
 * @tc.desc: Test SetDecodeCacheActive(false) deactivates ImagePattern and clears state.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridSetDecodeCacheActive001, TestSize.Level1)
{
    auto frameNode = CreateImageFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImagePattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->decodeCacheActive_);
    pattern->SetDecodeCacheActive(false);
    EXPECT_FALSE(pattern->decodeCacheActive_);
}

/**
 * @tc.name: GridSetDecodeCacheActive002
 * @tc.desc: Test SetDecodeCacheActive(true) reactivates ImagePattern after deactivation.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridSetDecodeCacheActive002, TestSize.Level1)
{
    auto frameNode = CreateImageFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImagePattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetDecodeCacheActive(false);
    EXPECT_FALSE(pattern->decodeCacheActive_);
    pattern->SetDecodeCacheActive(true);
    EXPECT_TRUE(pattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateCachedItemDecodeState001
 * @tc.desc: Test UpdateCachedItemDecodeState recursively updates nested Image nodes.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateCachedItemDecodeState001, TestSize.Level1)
{
    auto parent = CreateImageFrameNode();
    ASSERT_NE(parent, nullptr);
    auto child = CreateImageFrameNode();
    ASSERT_NE(child, nullptr);
    parent->AddChild(child);
    auto parentPattern = parent->GetPattern<ImagePattern>();
    ASSERT_NE(parentPattern, nullptr);
    auto childPattern = child->GetPattern<ImagePattern>();
    ASSERT_NE(childPattern, nullptr);
    EXPECT_TRUE(parentPattern->decodeCacheActive_);
    EXPECT_TRUE(childPattern->decodeCacheActive_);
    ScrollableUtils::UpdateCachedItemDecodeState(parent, false);
    EXPECT_FALSE(parentPattern->decodeCacheActive_);
    EXPECT_FALSE(childPattern->decodeCacheActive_);
}

/**
 * @tc.name: GridApplyItemDecodeState_ExpandFullMatrix001
 * @tc.desc: Test ApplyItemDecodeState covers all items outside decode window.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState_ExpandFullMatrix001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1);
    CreateFixedItems(20);
    CreateDone();

    auto item0 = GetItem(0, true);
    auto item10 = GetItem(10, true);
    ASSERT_NE(item0, nullptr);
    ASSERT_NE(item10, nullptr);
    auto image0 = CreateImageFrameNode();
    auto image10 = CreateImageFrameNode();
    ASSERT_NE(image0, nullptr);
    ASSERT_NE(image10, nullptr);
    item0->AddChild(image0);
    item10->AddChild(image10);
    auto pattern0 = image0->GetPattern<ImagePattern>();
    auto pattern10 = image10->GetPattern<ImagePattern>();
    ASSERT_NE(pattern0, nullptr);
    ASSERT_NE(pattern10, nullptr);

    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    // Overwrite gridMatrix_ to simulate preloaded items beyond cache window
    layoutAlgorithm->info_.gridMatrix_ = {
        {0, {{0, 0}, {1, 1}}},
        {1, {{0, 2}, {1, 3}}},
        {2, {{0, 4}, {1, 5}}},
        {3, {{0, 6}, {1, 7}}},
        {4, {{0, 8}, {1, 9}}},
        {5, {{0, 10}, {1, 11}}},
    };
    layoutAlgorithm->info_.startMainLineIndex_ = 2;
    layoutAlgorithm->info_.endMainLineIndex_ = 3;
    // All items outside decode window → marked inactive
    std::set<int32_t> emptyDecodeItems;
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, emptyDecodeItems);
    EXPECT_FALSE(pattern0->decodeCacheActive_);
    EXPECT_FALSE(pattern10->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateGridDecodeCacheWindowInternal_ShowCached001
 * @tc.desc: Test B5 — showCachedItems=true skips gracefully, no decode state change.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateGridDecodeCacheWindowInternal_ShowCached001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5, true); // 5 : cacheCount, true : showCachedItems
    CreateFixedItems(20);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
    // showCachedItems=true → early return, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateGridDecodeCacheWindowInternal_CacheCountZero001
 * @tc.desc: Test B6 — cacheCount=0 skips gracefully, no decode state change.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateGridDecodeCacheWindowInternal_CacheCountZero001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0);
    CreateFixedItems(20);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 0);
    // cacheCount=0 → early return, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateGridDecodeCacheWindowInternal_EmptyMatrix001
 * @tc.desc: Test B7 — empty gridMatrix skips gracefully, no decode state change.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateGridDecodeCacheWindowInternal_EmptyMatrix001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(20);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.gridMatrix_.clear();
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateGridDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
    // gridMatrix empty → early return, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateGridDecodeCacheWindowInternal_NormalPath001
 * @tc.desc: Test B9 — normal path via internal method, ApplyItemDecodeState called.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateGridDecodeCacheWindowInternal_NormalPath001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);

    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->Measure(AceType::RawPtr(wrapper));
    layoutAlgorithm->Layout(AceType::RawPtr(wrapper));
    layoutAlgorithm->UpdateGridDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
    // Item 0 is in viewport (startIndex_=0) → in decodeItems → stays active
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateGridDecodeCacheWindow_NullWrapper001
 * @tc.desc: Test B1 — UpdateGridDecodeCacheWindow with null layoutWrapper does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateGridDecodeCacheWindow_NullWrapper001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->UpdateGridDecodeCacheWindow(nullptr, 5); // 5 : cacheCount
    // null layoutWrapper → early return, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridApplyItemDecodeState_NullHost001
 * @tc.desc: Test D1 — ApplyItemDecodeState with null host does not crash, no items processed.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState_NullHost001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    std::set<int32_t> decodeItems;
    RefPtr<FrameNode> nullHost = nullptr;
    layoutAlgorithm->ApplyItemDecodeState(nullHost, decodeItems);
    // null host → early return, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridApplyItemDecodeState_UnbuiltItem001
 * @tc.desc: Test D2 — unbuilt items return null from GetFrameNodeChildByIndexWithoutBuild, skipped.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState_UnbuiltItem001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    // Set childrenCount_ higher than actual built items (10) to simulate unbuilt items
    layoutAlgorithm->info_.childrenCount_ = 20; // 20 : simulate 20 items, only 10 built
    std::set<int32_t> emptyDecodeItems;
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, emptyDecodeItems);
    // Built item 0 (not in decodeItems) → deactivated; unbuilt items 10..19 → skipped
    EXPECT_FALSE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridApplyItemDecodeState_EmptyCount001
 * @tc.desc: Test D6 — ApplyItemDecodeState with totalItemCount=0, loop never executes.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridApplyItemDecodeState_EmptyCount001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    CreateFixedItems(10);
    CreateDone();
    auto item0 = GetItem(0, true);
    ASSERT_NE(item0, nullptr);
    auto imageNode = CreateImageFrameNode();
    ASSERT_NE(imageNode, nullptr);
    item0->AddChild(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
    auto layoutAlgorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->info_.childrenCount_ = 0;
    std::set<int32_t> decodeItems = {0};
    layoutAlgorithm->ApplyItemDecodeState(frameNode_, decodeItems);
    // totalItemCount=0 → loop never executes, no decode state change
    EXPECT_TRUE(imagePattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateCachedItemDecodeState_NullRoot001
 * @tc.desc: Test U1 — UpdateCachedItemDecodeState with null root does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateCachedItemDecodeState_NullRoot001, TestSize.Level1)
{
    auto frameNode = CreateImageFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImagePattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->decodeCacheActive_);
    RefPtr<UINode> nullNode = nullptr;
    ScrollableUtils::UpdateCachedItemDecodeState(nullNode, false);
    // null root → early return, existing node unaffected
    EXPECT_TRUE(pattern->decodeCacheActive_);
}

/**
 * @tc.name: GridUpdateCachedItemDecodeState_NoChildren001
 * @tc.desc: Test U6 — UpdateCachedItemDecodeState on node with no children, no recursion.
 * @tc.type: FUNC
 */
HWTEST_F(GridCacheLayoutTestNg, GridUpdateCachedItemDecodeState_NoChildren001, TestSize.Level1)
{
    auto frameNode = CreateImageFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImagePattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->decodeCacheActive_);
    ScrollableUtils::UpdateCachedItemDecodeState(frameNode, false);
    EXPECT_FALSE(pattern->decodeCacheActive_);
}

} // namespace OHOS::Ace::NG

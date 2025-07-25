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
#include "test/mock/core/render/mock_render_context.h"

#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "test/mock/core/animation/mock_animation_manager.h"

namespace OHOS::Ace::NG {

class GridCacheLayoutTestNg : public GridTestNg {
public:
    void CreateItems1();
    void CreateItems2();
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
    EXPECT_FLOAT_EQ(pattern_->info_.currentOffset_, 0);
    EXPECT_FLOAT_EQ(GetChildY(frameNode_, 0), 0);
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
} // namespace OHOS::Ace::NG
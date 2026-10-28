/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/licenses-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lazy_grid_layout_syntax_test.h"

#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/unittest/core/syntax/mock_lazy_for_each_builder.h"

#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_model.h"
#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_property.h"
#include "core/components_ng/pattern/waterflow/water_flow_item_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components_ng/syntax/lazy_for_each_model_ng.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_model_ng.h"

namespace OHOS::Ace::NG {

using GetChildByIndexCallback = std::function<std::pair<std::string, RefPtr<NG::UINode>>(int32_t index)>;

class CustomLazyForEachBuilder : public Framework::MockLazyForEachBuilder {
public:
    CustomLazyForEachBuilder(int32_t itemCnt, GetChildByIndexCallback&& callback)
        : itemCnt_(itemCnt), getChildByIndexCallback_(std::move(callback)) {}

    void SetTotalCount(int32_t itemCnt)
    {
        itemCnt_ = itemCnt;
    }

protected:
    int32_t OnGetTotalCount() override
    {
        return itemCnt_;
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, NG::LazyForEachCacheChild>& expiringItems) override
    {
        if (getChildByIndexCallback_) {
            return getChildByIndexCallback_(index);
        }
        return { std::to_string(index), nullptr };
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndexNew(int32_t index,
        std::map<int32_t, NG::LazyForEachChild>& cachedItems,
        std::unordered_map<std::string, NG::LazyForEachCacheChild>& expiringItems) override
    {
        return OnGetChildByIndex(index, expiringItems);
    }

private:
    int32_t itemCnt_ = 0;
    GetChildByIndexCallback getChildByIndexCallback_ = nullptr;
};

namespace {
constexpr size_t PARAM_LANES = 0;
constexpr size_t PARAM_BELOW_VIEWPORT = 1;
constexpr size_t PARAM_DATASET_CHANGE = 2;
constexpr size_t PARAM_SLIDING_WINDOW = 3;
constexpr int32_t GRID_ITEM_COUNT = 50;
constexpr int32_t INSERTED_KEY = 50;
constexpr int32_t MAX_PREDICT_PASSES = 10;
constexpr int64_t PREDICT_BUDGET_NS = 16 * 1000000;
constexpr float POS_TOLERANCE = 0.5f;
// FlowItem indices of the three-grid WaterFlow; the middle one holds the grid under test.
constexpr int32_t FIRST_FLOW_ITEM = 0;
constexpr int32_t MIDDLE_FLOW_ITEM = 1;
constexpr int32_t LAST_FLOW_ITEM = 2;

float RegularExtent(int32_t count, int32_t lanes)
{
    return lanes > 0 ? ((count + lanes - 1) / lanes) * ITEM_HEIGHT : 0.0f;
}

// Equal-height items: any misplaced anchor shows up as a whole-row offset.
void ExpectRegularRows(const RefPtr<LazyGridLayoutInfo>& info, int32_t lanes)
{
    EXPECT_GT(lanes, 0);
    const int32_t laneCount = lanes > 0 ? lanes : 1;
    for (const auto& [index, pos] : info->posMap_) {
        EXPECT_EQ(pos.laneIdx, index % laneCount) << index;
        EXPECT_NEAR(pos.startPos, (index / laneCount) * ITEM_HEIGHT, POS_TOLERANCE) << index;
        EXPECT_NEAR(pos.endPos, (index / laneCount + 1) * ITEM_HEIGHT, POS_TOLERANCE) << index;
    }
}

// The grid must hold every item at its regular row, keep a positive extent and leave no prediction pending.
void ExpectConvergedLayout(const RefPtr<FrameNode>& grid, const RefPtr<LazyGridLayoutInfo>& info,
    int32_t count, int32_t lanes)
{
    SCOPED_TRACE(testing::Message() << "count=" << count << " start=" << info->startIndex_
        << " end=" << info->endIndex_ << " laid=" << info->layoutedStartIndex_ << ","
        << info->layoutedEndIndex_ << " cache=" << info->cacheStartPos_ << "," << info->cacheEndPos_);
    EXPECT_EQ(info->totalItemCount_, count);
    EXPECT_GT(info->totalMainSize_, 0.0f);
    EXPECT_GT(grid->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    EXPECT_FALSE(info->NeedPredict());
    EXPECT_LE(info->cachedStartIndex_, info->cachedEndIndex_);
    ExpectRegularRows(info, lanes);
}
} // namespace

class LazyGridOffscreenMutationTest : public LazyGridLayoutSyntaxTest,
    public testing::WithParamInterface<std::tuple<int32_t, bool, bool, bool>> {
public:
    using CheckStep = void (LazyGridOffscreenMutationTest::*)();

    int32_t Lanes() const
    {
        return std::get<PARAM_LANES>(GetParam());
    }

    bool BelowViewport() const
    {
        return std::get<PARAM_BELOW_VIEWPORT>(GetParam());
    }

    WaterFlowLayoutMode FlowMode() const
    {
        return std::get<PARAM_SLIDING_WINDOW>(GetParam()) ? WaterFlowLayoutMode::SLIDING_WINDOW
                                                           : WaterFlowLayoutMode::TOP_DOWN;
    }

    int32_t KeyCount() const
    {
        return static_cast<int32_t>(keys_->size());
    }

    // Creates the grid under test in the current FlowItem; keys_ backs its LazyForEach data.
    void CreateMutableGrid()
    {
        keys_ = std::make_shared<std::vector<int32_t>>(GRID_ITEM_COUNT);
        std::iota(keys_->begin(), keys_->end(), 0);
        CreateLazyGridLayout();
        layoutProperty_->UpdateColumnsTemplate(Lanes() == 1 ? "1fr" : "1fr 1fr 1fr");
        builder_ = AceType::MakeRefPtr<CustomLazyForEachBuilder>(KeyCount(),
            [keys = keys_](int32_t index) -> std::pair<std::string, RefPtr<NG::UINode>> {
                StackModelNG model;
                model.Create();
                ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
                ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
                return { std::to_string(keys->at(index)), ViewStackProcessor::GetInstance()->Finish() };
            });
        RefPtr<NG::LazyForEachBuilder> actuator = builder_;
        LazyForEachModelNG().Create(actuator);
        grid_ = frameNode_;
        gridPattern_ = pattern_;
    }

    void CreateFixedGridItem()
    {
        WaterFlowItemModelNG().Create();
        CreateLazyGridLayout();
        layoutProperty_->UpdateColumnsTemplate("1fr");
        CreateContent(GRID_ITEM_COUNT);
    }

    // Three FlowItems with a grid each; the middle one is the grid under test.
    void CreateFlowWithMiddleGrid()
    {
        CreateWaterFlow(FlowMode());
        CreateFixedGridItem();
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->Pop();
        WaterFlowItemModelNG().Create();
        CreateMutableGrid();
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->Pop();
        CreateFixedGridItem();
        CreateDone();
    }

    bool BindLazyNode()
    {
        lazy_ = AceType::DynamicCast<LazyForEachNode>(grid_->GetChildAtIndex(0));
        return lazy_ != nullptr;
    }

    // Lays the grid out once, then scrolls it just outside the viewport, above or below it.
    void ScrollGridOffscreen()
    {
        scrollablePattern_->ScrollToIndex(MIDDLE_FLOW_ITEM, false, ScrollAlign::START);
        FlushUITasks();
        scrollablePattern_->UpdateCurrentOffset(-200.0f, SCROLL_FROM_UPDATE);
        FlushUITasks();
        FlushIdleTask(gridPattern_);
        ASSERT_GT(gridPattern_->layoutInfo_->endIndex_, 0);

        const bool below = BelowViewport();
        scrollablePattern_->ScrollToIndex(below ? FIRST_FLOW_ITEM : LAST_FLOW_ITEM, false,
            below ? ScrollAlign::END : ScrollAlign::START);
        FlushUITasks();
        scrollablePattern_->UpdateCurrentOffset(below ? 100.0f : -100.0f, SCROLL_FROM_UPDATE);
        FlushUITasks();
        FlushIdleTask(gridPattern_);
        auto waterFlow = scrollableFrameNode_->GetPattern<WaterFlowPattern>();
        ASSERT_NE(waterFlow, nullptr);
        ASSERT_TRUE(below ? waterFlow->GetEndIndex() < MIDDLE_FLOW_ITEM : waterFlow->GetBeginIndex() > MIDDLE_FLOW_ITEM)
            << "visible=" << waterFlow->GetBeginIndex() << "," << waterFlow->GetEndIndex()
            << " offset=" << waterFlow->GetTotalOffset();
    }

    // Supplies the same parent constraint as a grid just outside the viewport, but inside its cache window.
    void InitNearViewportConstraint()
    {
        constraint_ = grid_->GetGeometryNode()->GetParentLayoutConstraint().value();
        view_ = {};
        view_.axis = Axis::VERTICAL;
        view_.viewPosEnd = SCROLL_HEIGHT;
        view_.referenceEdge = BelowViewport() ? ReferenceEdge::END : ReferenceEdge::START;
    }

    // Measures the grid directly so a parent relayout cannot repair a bad position map first.
    void MeasureGrid(bool predict)
    {
        auto info = gridPattern_->layoutInfo_;
        grid_->ResetLayoutAlgorithm();
        grid_->GetLayoutProperty()->UpdatePropertyChangeFlag(
            predict ? PROPERTY_UPDATE_MEASURE_SELF : PROPERTY_UPDATE_MEASURE);
        if (predict) {
            info->deadline_ = GetSysTimestamp() + PREDICT_BUDGET_NS;
        } else {
            info->deadline_.reset();
            const auto height = grid_->GetGeometryNode()->GetFrameSize().Height();
            view_.referencePos = BelowViewport() ? height + 600.0f : -height - 100.0f;
            constraint_.viewPosRef = view_;
        }
        grid_->Measure(constraint_);
        grid_->Layout();
    }

    void FlushAndCheck()
    {
        FlushUITasks();
        FlushIdleTask(gridPattern_);
        ExpectConvergedLayout(grid_, gridPattern_->layoutInfo_, KeyCount(), Lanes());
    }

    void MeasureAndCheck()
    {
        MeasureGrid(false);
        auto info = gridPattern_->layoutInfo_;
        for (int32_t pass = 0; pass < MAX_PREDICT_PASSES && info->NeedPredict(); ++pass) {
            MeasureGrid(true);
        }
        ExpectConvergedLayout(grid_, info, KeyCount(), Lanes());
    }

    // Deletes the first item, deletes the last item, then inserts a new head, checking after each change.
    void MutateFirstLastFirst(CheckStep check)
    {
        keys_->erase(keys_->begin());
        NotifyMutation(0, true);
        (this->*check)();
        keys_->pop_back();
        NotifyMutation(KeyCount(), true);
        (this->*check)();
        keys_->insert(keys_->begin(), INSERTED_KEY);
        NotifyMutation(0, false);
        (this->*check)();
    }

protected:
    void NotifyMutation(int32_t index, bool isDelete)
    {
        builder_->UpdateHistoricalTotalCount(builder_->GetTotalCount());
        builder_->SetTotalCount(KeyCount());
        if (std::get<PARAM_DATASET_CHANGE>(GetParam())) {
            V2::Operation operation {};
            operation.type = isDelete ? "delete" : "add";
            operation.index = index;
            operation.count = 1;
            lazy_->OnDatasetChange({ operation });
        } else if (isDelete) {
            lazy_->OnDataDeleted(index);
        } else {
            lazy_->OnDataAdded(index);
        }
        MockPipelineContext::GetCurrent()->FlushBuild();
    }

    std::shared_ptr<std::vector<int32_t>> keys_;
    RefPtr<CustomLazyForEachBuilder> builder_;
    RefPtr<LazyForEachNode> lazy_;
    RefPtr<FrameNode> grid_;
    RefPtr<LazyGridLayoutPattern> gridPattern_;
    LayoutConstraintF constraint_;
    ViewPosReference view_ {};
};

/**
 * @tc.name: DeleteFirstDeleteLastInsertFirstOffscreen
 * @tc.desc: Offscreen data changes must preserve item positions, extent and a convergent prediction range.
 * @tc.type: FUNC
 */
TEST_P(LazyGridOffscreenMutationTest, DeleteFirstDeleteLastInsertFirstOffscreen)
{
    CreateFlowWithMiddleGrid();
    ASSERT_TRUE(BindLazyNode());
    ASSERT_NO_FATAL_FAILURE(ScrollGridOffscreen());
    MutateFirstLastFirst(&LazyGridOffscreenMutationTest::FlushAndCheck);

    // Re-enter from both ends: every surviving item must recover its regular row position and total extent.
    scrollablePattern_->ScrollToIndex(MIDDLE_FLOW_ITEM, false, ScrollAlign::START);
    FlushUITasks();
    FlushIdleTask(gridPattern_);
    auto info = gridPattern_->layoutInfo_;
    EXPECT_NEAR(info->totalMainSize_, RegularExtent(KeyCount(), Lanes()), POS_TOLERANCE);
    EXPECT_EQ(info->visibleStartIndex_, 0);
    FlushAndCheck();
}

/**
 * @tc.name: OffscreenMutationsWithOppositeReferenceEdge
 * @tc.desc: The parent's reference edge must not turn the before-first sentinel into a tail prediction anchor.
 * @tc.type: FUNC
 */
TEST_P(LazyGridOffscreenMutationTest, OffscreenMutationsWithOppositeReferenceEdge)
{
    CreateWaterFlow(FlowMode());
    WaterFlowItemModelNG().Create();
    CreateMutableGrid();
    CreateDone();
    ASSERT_TRUE(BindLazyNode());
    InitNearViewportConstraint();
    MeasureGrid(false);
    auto info = gridPattern_->layoutInfo_;
    ASSERT_TRUE(BelowViewport() ? info->endIndex_ < 0 : info->startIndex_ >= info->totalItemCount_);
    MutateFirstLastFirst(&LazyGridOffscreenMutationTest::MeasureAndCheck);
}

INSTANTIATE_TEST_SUITE_P(OneAndThreeLanes, LazyGridOffscreenMutationTest,
    testing::Combine(testing::Values(1, 3), testing::Bool(), testing::Bool(), testing::Bool()));

class LazyGridPredictAnchorTest : public LazyGridLayoutSyntaxTest, public testing::WithParamInterface<int32_t> {
public:
    void CreateGridInFlowItem()
    {
        CreateWaterFlow();
        WaterFlowItemModelNG().Create();
        CreateLazyGridLayout();
        layoutProperty_->UpdateColumnsTemplate(GetParam() == 1 ? "1fr" : "1fr 1fr 1fr");
        CreateContent(GRID_ITEM_COUNT);
        CreateDone();
        constraint_ = frameNode_->GetGeometryNode()->GetParentLayoutConstraint().value();
    }

    void MeasureWith(const std::optional<ViewPosReference>& view, PropertyChangeFlag flag)
    {
        frameNode_->ResetLayoutAlgorithm();
        layoutProperty_->UpdatePropertyChangeFlag(flag);
        if (view.has_value()) {
            constraint_.viewPosRef = view;
        }
        frameNode_->Measure(constraint_);
        frameNode_->Layout();
    }

    float Extent() const
    {
        return RegularExtent(GRID_ITEM_COUNT, GetParam());
    }

    LayoutConstraintF constraint_;
};

/**
 * @tc.name: PastLastAnchorIgnoresStaleExtent
 * @tc.desc: Backward prediction from the past-last sentinel must anchor at the laid-out last row, not at a stored
 *           extent left over from before a tail deletion.
 * @tc.type: FUNC
 */
TEST_P(LazyGridPredictAnchorTest, PastLastAnchorIgnoresStaleExtent)
{
    CreateGridInFlowItem();
    auto info = pattern_->layoutInfo_;
    ViewPosReference view {};
    view.axis = Axis::VERTICAL;
    view.viewPosEnd = SCROLL_HEIGHT;
    view.referenceEdge = ReferenceEdge::START;
    // The grid ends 100 above the viewport, so its last rows sit in the leading cache window.
    view.referencePos = -Extent() - 100.0f;
    MeasureWith(view, PROPERTY_UPDATE_MEASURE);
    ASSERT_GE(info->startIndex_, info->totalItemCount_);
    // Let idle prediction lay out the tail rows in the cache window.
    for (int32_t pass = 0; pass < MAX_PREDICT_PASSES && info->NeedPredict(); ++pass) {
        info->deadline_ = GetSysTimestamp() + PREDICT_BUDGET_NS;
        MeasureWith(std::nullopt, PROPERTY_UPDATE_MEASURE_SELF);
    }
    ASSERT_NE(info->posMap_.find(GRID_ITEM_COUNT - 1), info->posMap_.end());
    ASSERT_NEAR(info->posMap_.at(GRID_ITEM_COUNT - 1).endPos, Extent(), POS_TOLERANCE);

    // Past-last sentinel whose stored anchor still holds an extent one row longer, as after an unrelaid tail deletion.
    info->layoutedStartIndex_ = GRID_ITEM_COUNT;
    info->layoutedStart_ = Extent() + ITEM_HEIGHT;
    info->deadline_ = GetSysTimestamp() + PREDICT_BUDGET_NS;
    MeasureWith(std::nullopt, PROPERTY_UPDATE_MEASURE_SELF);

    EXPECT_LT(info->layoutedStartIndex_, GRID_ITEM_COUNT);
    EXPECT_NEAR(info->posMap_.at(GRID_ITEM_COUNT - 1).endPos, Extent(), POS_TOLERANCE);
    ExpectRegularRows(info, GetParam());
}

/**
 * @tc.name: UnmeasuredEndReferencePredictsTail
 * @tc.desc: A grid with no baseline whose first measure is predictive under an end reference must build the tail
 *           rows that border the viewport, then rebase them and its window onto the estimated extent.
 * @tc.type: FUNC
 */
TEST_P(LazyGridPredictAnchorTest, UnmeasuredEndReferencePredictsTail)
{
    CreateGridInFlowItem();
    for (auto flag : { PROPERTY_UPDATE_MEASURE, PROPERTY_UPDATE_MEASURE_SELF }) {
        SCOPED_TRACE(testing::Message() << "flag=" << flag);
        pattern_->layoutInfo_ = AceType::MakeRefPtr<LazyGridLayoutInfo>();
        auto info = pattern_->layoutInfo_;
        frameNode_->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 0.0f));
        ViewPosReference view {};
        view.axis = Axis::VERTICAL;
        view.viewPosEnd = SCROLL_HEIGHT;
        view.referenceEdge = ReferenceEdge::END;
        // The grid ends 100 above the viewport; the leading cache window covers its last 125.
        view.referencePos = -100.0f;
        view.deadline = GetSysTimestamp() + PREDICT_BUDGET_NS;
        MeasureWith(view, flag);

        ASSERT_FALSE(info->posMap_.empty());
        EXPECT_EQ(info->posMap_.rbegin()->first, GRID_ITEM_COUNT - 1);
        EXPECT_GT(info->posMap_.begin()->first, 0);
        EXPECT_NEAR(info->totalMainSize_, Extent(), POS_TOLERANCE);
        ExpectRegularRows(info, GetParam());
        EXPECT_EQ(info->layoutedEndIndex_, GRID_ITEM_COUNT - 1);
        EXPECT_NEAR(info->layoutedStart_, info->posMap_.at(info->layoutedStartIndex_).startPos, POS_TOLERANCE);
        EXPECT_FALSE(info->NeedPredict());
    }
}

INSTANTIATE_TEST_SUITE_P(OneAndThreeLanes, LazyGridPredictAnchorTest, testing::Values(1, 3));

void LazyGridLayoutSyntaxTest::CreateRepeatVirtualScrollNode(
    int32_t itemNumber, const std::function<std::pair<uint32_t, uint32_t>(int32_t, bool, bool)>& onGetRid4Index)
{
    RepeatVirtualScroll2ModelNG repeatModel;
    auto onActiveRange = [](int32_t start, int32_t end, int32_t cacheStart,
        int32_t cacheEnd, bool isCache, bool isImmediate) {};
    auto onPurge = []() {};
    auto onPurgeAll = []() {};
    auto onUpdateDirty = []() {};
    auto onRecycleItems = [](int32_t start, int32_t end) {};
    repeatModel.Create(
        itemNumber,
        itemNumber,
        0,
        onGetRid4Index,
        onRecycleItems,
        onActiveRange,
        nullptr,
        onPurge,
        onPurgeAll,
        onUpdateDirty);
}

/**
 * @tc.name: LazyForEachTest001
 * @tc.desc: Test LazyVGridLayout with LazyForEach generated children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutSyntaxTest, LazyForEachTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout with LazyForEach
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    auto mockLazy = AceType::MakeRefPtr<CustomLazyForEachBuilder>(30,
        [](int32_t index) -> std::pair<std::string, RefPtr<NG::UINode>> {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
            auto node = ViewStackProcessor::GetInstance()->Finish();
            return { std::to_string(index), node };
        }
    );
    LazyForEachModelNG lazyForEachModelNG;
    RefPtr<NG::LazyForEachBuilder> builder = mockLazy;
    lazyForEachModelNG.Create(builder);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 200);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 13);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 13);
    EXPECT_EQ(mockLazy->startIndex_, 0);
    EXPECT_EQ(mockLazy->endIndex_, 13);

    /**
     * @tc.steps: UpdateColumnsTemplate 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 3000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 6);
    EXPECT_EQ(mockLazy->startIndex_, 0);
    EXPECT_EQ(mockLazy->endIndex_, 6);

    /**
     * @tc.steps: UpdateColumnsTemplate 1fr 1fr 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 14);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 20);
    EXPECT_EQ(mockLazy->startIndex_, 0);
    EXPECT_EQ(mockLazy->endIndex_, 20);
}

/**
 * @tc.name: RepeatVirtualScrollTest001
 * @tc.desc: Test LazyVGridLayout with Repeat generated children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutSyntaxTest, RepeatVirtualScrollTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout with LazyForEach
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateRepeatVirtualScrollNode(30,
        [](uint32_t index, bool inAnimation, bool isRestoreCache) -> std::pair<uint32_t, uint32_t> {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
            ViewStackProcessor::GetInstance()->Pop();
            return { 1, OnGetRid4IndexResult::CREATED_NEW_NODE };
        }
    );
    CreateDone();
    auto repeat = AceType::DynamicCast<RepeatVirtualScroll2Node>(frameNode_->GetChildAtIndex(0));
    ASSERT_NE(repeat, nullptr);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 200);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 13);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 13);
    EXPECT_EQ(repeat->prevActiveRangeStart_, 0);
    EXPECT_EQ(repeat->prevActiveRangeEnd_, 13);

    /**
     * @tc.steps: UpdateColumnsTemplate 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 3000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 6);
    EXPECT_EQ(repeat->prevActiveRangeStart_, 0);
    EXPECT_EQ(repeat->prevActiveRangeEnd_, 6);

    /**
     * @tc.steps: UpdateColumnsTemplate 1fr 1fr 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 14);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 20);
    EXPECT_EQ(repeat->prevActiveRangeStart_, 0);
    EXPECT_EQ(repeat->prevActiveRangeEnd_, 20);
}
} // namespace OHOS::Ace::NG

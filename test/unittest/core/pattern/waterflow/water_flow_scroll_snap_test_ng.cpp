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

#include "water_flow_test_ng.h"

#include <algorithm>
#include <array>
#include <deque>
#include <limits>

#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"
#include "core/components_ng/pattern/waterflow/water_flow_model_static.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/components_ng/pattern/mock_nestable_scroll_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

// Feat-20260806915269: WaterFlow scrollSnapStrategy. WaterFlow supplies real laid-out item candidates and the
// legal scroll range; selection, animation and cancellation belong to the shared item snap mechanism.
// The file is part of COMMON_SRC, so every case runs for the top-down, segmented and sliding-window targets.
namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SNAP_ITEM_COUNT = 30;
constexpr int32_t UNDERESTIMATED_ITEM_COUNT = 100;
constexpr int32_t UNDERESTIMATED_SHORT_ITEM_COUNT = 10;
constexpr int32_t UNDERESTIMATED_LAST_ITEM_INDEX = UNDERESTIMATED_ITEM_COUNT - 1;
constexpr int32_t PROVIDER_TARGET_ITEM_INDEX = 6;
constexpr int32_t MAX_SNAP_ANIMATION_TICKS = 100;
constexpr size_t MIN_SNAP_CANDIDATE_COUNT = 4;
constexpr size_t POST_TASK_NAME_ARGUMENT = 3;
constexpr double PROVIDER_TARGET = 320.0;
constexpr float PROVIDER_NATURAL_OFFSET = 500.0f;
constexpr float PROVIDER_INITIAL_VELOCITY = 1300.0f;
constexpr std::array<float, 6> SNAP_ITEM_SIZES = { 100.0f, 170.0f, 80.0f, 120.0f, 150.0f, 90.0f };
constexpr std::array<SnapCandidate, 3> EXPECTED_START_ANCHORS = { {
    { 0.0f, 0 }, { 100.0f, 2 }, { 170.0f, 3 }
} };

// Captures the shared provider handoff task so that the two-stage provider path can be driven in UT.
class WaterFlowSnapTaskExecutor : public MockTaskExecutor {
public:
    MOCK_METHOD(bool, OnPostTask,
        (Task&& task, TaskType type, uint32_t delayTime, const std::string& name, PriorityType priority,
            VsyncBarrierOption barrier), (const, override));

    bool CaptureTask(Task&& task, TaskType type, const std::string& name) const
    {
        if (type == TaskType::UI && name == "ArkUIScrollItemSnapHandoff") {
            pending_.push_back(std::move(task));
            return true;
        }
        return MockTaskExecutor::OnPostTask(std::move(task), type, 0, name);
    }

    bool HasPending() const
    {
        return !pending_.empty();
    }

    void RunPending()
    {
        auto tasks = std::move(pending_);
        pending_.clear();
        for (auto& task : tasks) {
            task();
        }
    }

private:
    mutable std::deque<Task> pending_;
};

ScrollSnapStrategy MakeAlign(ScrollSnapAlign align)
{
    ScrollSnapStrategy strategy;
    strategy.align = align;
    return strategy;
}

ScrollSnapStrategy MakeProvider(SnapOffsetFunc snap, SnapApproachOffsetFunc approach = nullptr)
{
    ScrollSnapStrategy strategy;
    strategy.hasProvider = true;
    strategy.calculateSnapOffset = std::move(snap);
    strategy.calculateApproachOffset = std::move(approach);
    return strategy;
}
} // namespace

class WaterFlowScrollSnapTestNg : public WaterFlowTestNg {
protected:
    void SetUp() override
    {
        WaterFlowTestNg::SetUp();
        auto pipeline = MockPipelineContext::GetCurrent();
        previousExecutor_ = pipeline->GetTaskExecutor();
        snapExecutor_ = AceType::MakeRefPtr<WaterFlowSnapTaskExecutor>();
        EXPECT_CALL(*snapExecutor_, OnPostTask).Times(testing::AnyNumber()).WillRepeatedly(
            testing::WithArgs<0, 1, POST_TASK_NAME_ARGUMENT>(testing::Invoke(snapExecutor_.GetRawPtr(),
                &WaterFlowSnapTaskExecutor::CaptureTask)));
        pipeline->SetTaskExecutor(snapExecutor_);
    }

    void TearDown() override
    {
        WaterFlowTestNg::TearDown();
        MockPipelineContext::GetCurrent()->SetTaskExecutor(previousExecutor_);
        snapExecutor_.Reset();
        previousExecutor_.Reset();
    }

    // Two lanes, 300px viewport, item heights repeat 100/170/80/120/150/90.
    // Lane 0: item0 [0,100], item2 [100,180], item4 [180,330]; lane 1: item1 [0,170], item3 [170,290].
    void CreateUnequalItems()
    {
        auto model = CreateWaterFlow();
        model.SetColumnsTemplate("1fr 1fr");
        ViewAbstract::SetHeight(CalcLength(300.0f));
        for (int32_t i = 0; i < SNAP_ITEM_COUNT; ++i) {
            CreateItemWithHeight(SNAP_ITEM_SIZES[i % SNAP_ITEM_SIZES.size()]);
        }
        CreateDone();
    }

    void CreateUnderestimatedItems()
    {
        auto model = CreateWaterFlow();
        model.SetColumnsTemplate("1fr 1fr");
        ViewAbstract::SetHeight(CalcLength(300.0f));
        for (int32_t i = 0; i < UNDERESTIMATED_ITEM_COUNT; ++i) {
            CreateItemWithHeight(i < UNDERESTIMATED_SHORT_ITEM_COUNT ? 100.0f : 200.0f);
        }
        CreateDone();
    }

    void CreateDirectionalItems(FlexDirection direction, TextDirection textDirection = TextDirection::LTR)
    {
        auto model = CreateWaterFlow();
        model.SetLayoutDirection(direction);
        model.SetColumnsTemplate("1fr 1fr");
        model.SetRowsTemplate("1fr 1fr");
        ViewAbstract::SetLayoutDirection(textDirection);
        ViewAbstract::SetWidth(CalcLength(300.0f));
        ViewAbstract::SetHeight(CalcLength(300.0f));
        model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
        for (int32_t i = 0; i < SNAP_ITEM_COUNT; ++i) {
            CreateItemWithHeight(SNAP_ITEM_SIZES[i % SNAP_ITEM_SIZES.size()]);
        }
        CreateDone();
    }

    void SetStrategy(const ScrollSnapStrategy& strategy)
    {
        WaterFlowModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
        FlushUITasks();
    }

    void FinishSnapAnimation()
    {
        for (int32_t tick = 0; tick < MAX_SNAP_ANIMATION_TICKS &&
            (!MockAnimationManager::GetInstance().AllFinished() || snapExecutor_->HasPending()); ++tick) {
            MockAnimationManager::GetInstance().Tick();
            snapExecutor_->RunPending();
            FlushUITasks();
        }
        ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
        EXPECT_FALSE(snapExecutor_->HasPending());
        EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
    }

    void DestroyWaterFlow()
    {
        WeakPtr<FrameNode> weakNode = frameNode_;
        RemoveFromStageNode();
        ViewStackProcessor::GetInstance()->ClearStack();
        frameNode_.Reset();
        pattern_.Reset();
        ClearOldNodes();
        ASSERT_EQ(weakNode.Upgrade(), nullptr);
    }

    void FinishDetachedSnapAnimation(const RefPtr<Scrollable>& scrollable)
    {
        for (int32_t tick = 0; tick < MAX_SNAP_ANIMATION_TICKS &&
            (!MockAnimationManager::GetInstance().AllFinished() || snapExecutor_->HasPending()); ++tick) {
            MockAnimationManager::GetInstance().Tick();
            snapExecutor_->RunPending();
        }
        EXPECT_FALSE(snapExecutor_->HasPending());
        EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
        EXPECT_TRUE(scrollable->IsStopped());
    }

    RefPtr<TaskExecutor> previousExecutor_;
    RefPtr<WaterFlowSnapTaskExecutor> snapExecutor_;
};

/**
 * @tc.name: SnapTypeFollowsStrategy
 * @tc.desc: AC-3.1/AC-3.2: snap stays NONE when unconfigured and follows set/reset through the FrameNode entry.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, SnapTypeFollowsStrategy, TestSize.Level1)
{
    CreateUnequalItems();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    EXPECT_FALSE(pattern_->StartSnapAnimation({ .snapDelta = -10.0f }));
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
    WaterFlowModelNG::ResetScrollSnapStrategy(AceType::RawPtr(frameNode_));
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
}

/**
 * @tc.name: StrategyCloneResetAndDefault
 * @tc.desc: AC-3.2: Clone keeps the configured strategy; property Reset restores the disabled default.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, StrategyCloneResetAndDefault, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    auto clone = AceType::DynamicCast<WaterFlowLayoutProperty>(layoutProperty_->Clone());
    ASSERT_TRUE(clone);
    ASSERT_TRUE(clone->GetScrollSnapStrategy());
    EXPECT_EQ(clone->GetScrollSnapStrategy()->align, ScrollSnapAlign::CENTER);
    clone->Reset();
    EXPECT_FALSE(clone->GetScrollSnapStrategy());
    EXPECT_EQ(pattern_->GetScrollSnapStrategy().align, ScrollSnapAlign::CENTER);
}

/**
 * @tc.name: InvalidStrategyClearsConfiguration
 * @tc.desc: AC-3.2: NONE, an out-of-range align and a provider without calculateSnapOffset clear the strategy.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, InvalidStrategyClearsConfiguration, TestSize.Level1)
{
    CreateUnequalItems();
    const ScrollSnapStrategy invalids[] = { MakeAlign(ScrollSnapAlign::NONE),
        MakeAlign(static_cast<ScrollSnapAlign>(99)), MakeProvider(nullptr) };
    for (const auto& invalid : invalids) {
        SetStrategy(MakeAlign(ScrollSnapAlign::END));
        ASSERT_TRUE(layoutProperty_->GetScrollSnapStrategy());
        SetStrategy(invalid);
        EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
        EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    }
}

/**
 * @tc.name: StaticEntrySetsAndResets
 * @tc.desc: AC-3.2: the static (ArkTS 1.2) entry stores a strategy and clears it for an absent value.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, StaticEntrySetsAndResets, TestSize.Level1)
{
    CreateUnequalItems();
    WaterFlowModelStatic::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeAlign(ScrollSnapAlign::START));
    ASSERT_TRUE(layoutProperty_->GetScrollSnapStrategy());
    EXPECT_EQ(layoutProperty_->GetScrollSnapStrategy()->align, ScrollSnapAlign::START);
    WaterFlowModelStatic::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), std::nullopt);
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
}

/**
 * @tc.name: ProviderReplacementKeepsNewCallback
 * @tc.desc: AC-4.2: a real snap after provider replacement invokes only the new callback and reaches its target.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderReplacementKeepsNewCallback, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t oldSnapCalls = 0;
    int32_t oldApproachCalls = 0;
    int32_t newSnapCalls = 0;
    SetStrategy(MakeProvider([&](double) { ++oldSnapCalls; return 100.0; },
        [&](double, double) { ++oldApproachCalls; return 50.0; }));
    SetStrategy(MakeProvider([&](double) { ++newSnapCalls; return 170.0; }));
    EXPECT_FALSE(pattern_->GetScrollSnapStrategy().calculateApproachOffset);
    ScrollTo(60.0f);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(oldSnapCalls, 0);
    EXPECT_EQ(oldApproachCalls, 0);
    EXPECT_EQ(newSnapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 170.0f, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, 3).GetY(), 0.0f, 0.5f);
}

/**
 * @tc.name: UnequalLaneStartCandidates
 * @tc.desc: AC-1.2/AC-1.5/AC-4.3: START candidates are the real masonry item starts across unequal lanes,
 *           ordered by offset with the smaller index kept for equal offsets.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, UnequalLaneStartCandidates, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    const auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(candidates.size(), 4u);
    EXPECT_FLOAT_EQ(candidates[0].offset, 0.0f);
    EXPECT_EQ(candidates[0].index, 0);
    EXPECT_FLOAT_EQ(candidates[1].offset, 100.0f);
    EXPECT_EQ(candidates[1].index, 2);
    EXPECT_FLOAT_EQ(candidates[2].offset, 170.0f);
    EXPECT_EQ(candidates[2].index, 3);
    EXPECT_FLOAT_EQ(candidates[3].offset, 180.0f);
    EXPECT_EQ(candidates[3].index, 4);
}

/**
 * @tc.name: UnequalCenterAndEndCandidates
 * @tc.desc: AC-1.1/AC-1.2: items with equal starts keep distinct CENTER and END anchors for unequal heights.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, UnequalCenterAndEndCandidates, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    auto findOffset = [](const std::vector<SnapCandidate>& candidates, int32_t index) -> std::optional<float> {
        for (const auto& candidate : candidates) {
            if (candidate.index == index) {
                return candidate.offset;
            }
        }
        return std::nullopt;
    };
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_TRUE(findOffset(centers, 0) && findOffset(centers, 1));
    ASSERT_TRUE(findOffset(ends, 0) && findOffset(ends, 1));
    EXPECT_FLOAT_EQ(*findOffset(centers, 0), -100.0f);
    EXPECT_FLOAT_EQ(*findOffset(centers, 1), -65.0f);
    EXPECT_FLOAT_EQ(*findOffset(ends, 0), -200.0f);
    EXPECT_FLOAT_EQ(*findOffset(ends, 1), -130.0f);
}

/**
 * @tc.name: ColumnReverseLogicalAnchors
 * @tc.desc: AC-1.2: column-reverse maps physical geometry to the same logical anchors.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ColumnReverseLogicalAnchors, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN_REVERSE);
    ASSERT_TRUE(pattern_->IsReverse());
    const auto starts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(starts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(starts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(starts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(centers.empty());
    ASSERT_FALSE(ends.empty());
    EXPECT_FLOAT_EQ(centers.front().offset, -100.0f);
    EXPECT_EQ(centers.front().index, 0);
    EXPECT_FLOAT_EQ(ends.front().offset, -200.0f);
    EXPECT_EQ(ends.front().index, 0);
}

/**
 * @tc.name: RowLogicalAnchors
 * @tc.desc: AC-1.2: a horizontal layout uses the horizontal main axis for anchors.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, RowLogicalAnchors, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::ROW);
    ASSERT_EQ(pattern_->GetAxis(), Axis::HORIZONTAL);
    const auto starts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(starts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(starts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(starts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(centers.empty());
    ASSERT_FALSE(ends.empty());
    EXPECT_FLOAT_EQ(centers.front().offset, -100.0f);
    EXPECT_EQ(centers.front().index, 0);
    EXPECT_FLOAT_EQ(ends.front().offset, -200.0f);
    EXPECT_EQ(ends.front().index, 0);
}

/**
 * @tc.name: RowRtlLogicalAnchors
 * @tc.desc: AC-1.2: RTL rows map mirrored physical positions to the same logical anchors.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, RowRtlLogicalAnchors, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::ROW, TextDirection::RTL);
    ASSERT_TRUE(pattern_->IsReverse());
    const auto starts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(starts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(starts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(starts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(centers.empty());
    ASSERT_FALSE(ends.empty());
    EXPECT_FLOAT_EQ(centers.front().offset, -100.0f);
    EXPECT_EQ(centers.front().index, 0);
    EXPECT_FLOAT_EQ(ends.front().offset, -200.0f);
    EXPECT_EQ(ends.front().index, 0);
}

/**
 * @tc.name: PaddingAndItemMarginsUseOuterGeometry
 * @tc.desc: AC-1.2: anchors are relative to the content start and use the item margin frame.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PaddingAndItemMarginsUseOuterGeometry, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN);
    PaddingProperty padding;
    padding.top = CalcLength(13.0f);
    padding.bottom = CalcLength(17.0f);
    layoutProperty_->UpdatePadding(padding);
    MarginProperty margin;
    margin.top = CalcLength(10.0f);
    margin.bottom = CalcLength(20.0f);
    auto itemProperty = GetChildLayoutProperty<WaterFlowItemLayoutProperty>(frameNode_, 0);
    ASSERT_TRUE(itemProperty);
    itemProperty->UpdateMargin(margin);
    FlushUITasks();
    const auto starts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_GE(starts.size(), 3u);
    EXPECT_FLOAT_EQ(starts[0].offset, 0.0f);
    EXPECT_FLOAT_EQ(starts[1].offset, 130.0f);
    EXPECT_EQ(starts[1].index, 2);
    EXPECT_FLOAT_EQ(centers.front().offset, -70.0f);
    EXPECT_FLOAT_EQ(ends.front().offset, -140.0f);
}

/**
 * @tc.name: PendingDeltaDoesNotMoveAnchors
 * @tc.desc: AC-4.3: a scroll delta that is not laid out yet leaves the content-space anchors unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingDeltaDoesNotMoveAnchors, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    const auto before = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_FALSE(before.empty());
    pattern_->UpdateCurrentOffset(-37.0f, SCROLL_FROM_UPDATE);
    const auto after = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_EQ(after.size(), before.size());
    for (size_t i = 0; i < before.size(); ++i) {
        EXPECT_FLOAT_EQ(after[i].offset, before[i].offset);
        EXPECT_EQ(after[i].index, before[i].index);
    }
}

/**
 * @tc.name: AnchorsStableAfterScroll
 * @tc.desc: AC-4.3: after a laid-out scroll the anchors of items that stay laid out keep their content offsets.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, AnchorsStableAfterScroll, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-150.0f);
    ASSERT_NEAR(pattern_->GetTotalOffset(), 150.0f, 0.5f);
    bool foundItem4 = false;
    for (const auto& candidate : pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START)) {
        if (candidate.index == 4) {
            foundItem4 = true;
            EXPECT_FLOAT_EQ(candidate.offset, 180.0f);
        }
    }
    EXPECT_TRUE(foundItem4);
}

/**
 * @tc.name: LazyCandidatesDoNotBuildDistantItems
 * @tc.desc: AC-3.3/AC-4.3: LazyForEach candidates come from the laid-out window without building more items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, LazyCandidatesDoNotBuildDistantItems, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2);
    ViewAbstract::SetHeight(CalcLength(300.0f));
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
    int32_t builds = 0;
    auto builder = CreateItemsInLazyForEach(100000, [&builds](int32_t index) {
        ++builds;
        return index % 2 == 0 ? 100.0f : 170.0f;
    });
    ASSERT_TRUE(builder);
    CreateDone();
    const int32_t before = builds;
    ASSERT_GT(before, 0);
    ASSERT_LT(before, 64);
    for (auto align : { ScrollSnapAlign::START, ScrollSnapAlign::CENTER, ScrollSnapAlign::END }) {
        const auto candidates = pattern_->BuildItemSnapCandidates(align);
        EXPECT_FALSE(candidates.empty());
        EXPECT_LT(candidates.size(), 64u);
    }
    EXPECT_EQ(builds, before);
}

/**
 * @tc.name: RepeatCandidatesDoNotBuildDistantItems
 * @tc.desc: AC-3.3/AC-4.3: Repeat candidates come from the laid-out window without building more items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, RepeatCandidatesDoNotBuildDistantItems, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr 1fr");
    ViewAbstract::SetHeight(CalcLength(300.0f));
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    int32_t builds = 0;
    CreateItemsInRepeat(100000, [&builds](uint32_t index) {
        ++builds;
        return index % 2 == 0 ? 100.0f : 170.0f;
    });
    CreateDone();
    const int32_t before = builds;
    ASSERT_GT(before, 0);
    ASSERT_LT(before, 64);
    EXPECT_FALSE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER).empty());
    EXPECT_EQ(builds, before);
}

/**
 * @tc.name: DataChangeInvalidatesCandidatesUntilLayout
 * @tc.desc: AC-4.3: a data change drops the old anchors; the next valid layout captures new ones.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, DataChangeInvalidatesCandidatesUntilLayout, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    ASSERT_FALSE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START).empty());
    AddItemsAtSlot(1, 60.0f, 2);
    frameNode_->ChildrenUpdatedFrom(2);
    pattern_->NotifyDataChange(2, 1);
    EXPECT_TRUE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START).empty());
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks();
    // The inserted item occupies [100,160], so the next item now starts at 160, not 170.
    const auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    const auto changed = std::find_if(candidates.begin(), candidates.end(),
        [](const SnapCandidate& candidate) { return candidate.index == 3; });
    ASSERT_NE(changed, candidates.end());
    EXPECT_FLOAT_EQ(changed->offset, 160.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 3).GetY(), 160.0f);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), SNAP_ITEM_COUNT + 1);
}

/**
 * @tc.name: LayoutModeChangeInvalidatesOldAnchors
 * @tc.desc: AC-4.3: a layout mode switch drops the old anchors and the snap stays off until the next layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, LayoutModeChangeInvalidatesOldAnchors, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN);
    ASSERT_FALSE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START).empty());
    const auto newMode = pattern_->GetLayoutMode() == WaterFlowLayoutMode::SLIDING_WINDOW
                             ? WaterFlowLayoutMode::TOP_DOWN
                             : WaterFlowLayoutMode::SLIDING_WINDOW;
    pattern_->SetLayoutMode(newMode);
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    EXPECT_TRUE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START).empty());
    FlushUITasks();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
    EXPECT_FALSE(pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START).empty());
}

/**
 * @tc.name: SameLayoutModePreservesSnapCandidates
 * @tc.desc: Reapplying the active layout mode must not disable snapping before another layout is scheduled.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, SameLayoutModePreservesSnapCandidates, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN);
    const auto beforeStarts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(beforeStarts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(beforeStarts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(beforeStarts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto beforeCenters = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto beforeEnds = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(beforeCenters.empty());
    ASSERT_FALSE(beforeEnds.empty());
    EXPECT_FLOAT_EQ(beforeCenters.front().offset, -100.0f);
    EXPECT_EQ(beforeCenters.front().index, 0);
    EXPECT_FLOAT_EQ(beforeEnds.front().offset, -200.0f);
    EXPECT_EQ(beforeEnds.front().index, 0);
    const auto maxOffset = pattern_->GetItemSnapMaxOffset();

    pattern_->SetLayoutMode(pattern_->GetLayoutMode());

    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
    const auto afterStarts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(afterStarts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(afterStarts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(afterStarts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto afterCenters = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto afterEnds = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(afterCenters.empty());
    ASSERT_FALSE(afterEnds.empty());
    EXPECT_FLOAT_EQ(afterCenters.front().offset, -100.0f);
    EXPECT_EQ(afterCenters.front().index, 0);
    EXPECT_FLOAT_EQ(afterEnds.front().offset, -200.0f);
    EXPECT_EQ(afterEnds.front().index, 0);
    ASSERT_EQ(afterStarts.size(), beforeStarts.size());
    for (size_t i = 0; i < beforeStarts.size(); ++i) {
        EXPECT_FLOAT_EQ(afterStarts[i].offset, beforeStarts[i].offset);
        EXPECT_EQ(afterStarts[i].index, beforeStarts[i].index);
    }
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), maxOffset);
}

/**
 * @tc.name: SameLayoutModePreservesRetainedHeight
 * @tc.desc: An unsynchronized sliding window keeps its sampled height and legal bound after same-mode replay.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, SameLayoutModePreservesRetainedHeight, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN);
    // Exercise the sliding-window contract in every COMMON_SRC target, after an explicit layout.
    pattern_->SetLayoutMode(WaterFlowLayoutMode::SLIDING_WINDOW);
    FlushUITasks();
    ASSERT_EQ(pattern_->GetLayoutMode(), WaterFlowLayoutMode::SLIDING_WINDOW);
    ASSERT_TRUE(pattern_->snapTotalHeight_.has_value());
    const float retainedHeight = pattern_->snapTotalHeight_.value();
    const float maxOffset = pattern_->GetItemSnapMaxOffset();
    ASSERT_GT(retainedHeight, 300.0f);
    ASSERT_FALSE(pattern_->layoutInfo_->itemEnd_);
    ASSERT_NEAR(GetChildRect(frameNode_, 0).GetY(), 0.0f, 0.01f);

    // A real pending drag invalidates the SW estimate before the next layout consumes it.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-37.0f, SCROLL_FROM_UPDATE));
    ASSERT_FLOAT_EQ(pattern_->layoutInfo_->EstimateTotalHeight(), 0.0f);
    ASSERT_NEAR(pattern_->GetItemSnapCurrentOffset(), 37.0f, 0.01f);
    ASSERT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), maxOffset);

    pattern_->SetLayoutMode(WaterFlowLayoutMode::SLIDING_WINDOW);

    ASSERT_TRUE(pattern_->snapTotalHeight_.has_value());
    EXPECT_FLOAT_EQ(pattern_->snapTotalHeight_.value(), retainedHeight);
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), maxOffset);
    EXPECT_NEAR(pattern_->GetItemSnapCurrentOffset(), 37.0f, 0.01f);
    const auto starts = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_GE(starts.size(), MIN_SNAP_CANDIDATE_COUNT);
    for (size_t i = 0; i < EXPECTED_START_ANCHORS.size(); ++i) {
        EXPECT_FLOAT_EQ(starts[i].offset, EXPECTED_START_ANCHORS[i].offset);
        EXPECT_EQ(starts[i].index, EXPECTED_START_ANCHORS[i].index);
    }
    const auto centers = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    const auto ends = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_FALSE(centers.empty());
    ASSERT_FALSE(ends.empty());
    EXPECT_FLOAT_EQ(centers.front().offset, -100.0f);
    EXPECT_EQ(centers.front().index, 0);
    EXPECT_FLOAT_EQ(ends.front().offset, -200.0f);
    EXPECT_EQ(ends.front().index, 0);
    FlushUITasks();
    EXPECT_NEAR(GetChildRect(frameNode_, 0).GetY(), -37.0f, 0.01f);
}

/**
 * @tc.name: SameLayoutModeKeepsNextDragSnap
 * @tc.desc: A drag after a redundant mode update still settles on the same anchor as an unchanged component.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, SameLayoutModeKeepsNextDragSnap, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-51.0f);
    ASSERT_NEAR(pattern_->GetTotalOffset(), 51.0f, 0.01f);
    // Replay after the last measured drag. No layout may repopulate the cache before release.
    pattern_->SetLayoutMode(pattern_->GetLayoutMode());
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, 2).GetY(), 0.0f, 0.5f);
}

/**
 * @tc.name: ShortContentHasOneLegalOffset
 * @tc.desc: AC-2.1: content shorter than the viewport has one legal offset and never starts a snap.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ShortContentHasOneLegalOffset, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr 1fr");
    ViewAbstract::SetHeight(CalcLength(300.0f));
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
    CreateItemWithHeight(100.0f);
    CreateItemWithHeight(170.0f);
    CreateDone();
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMinOffset(), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 1).Bottom(), 170.0f);
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    EXPECT_FALSE(pattern_->StartSnapAnimation({ .snapDelta = -10.0f }));
}

/**
 * @tc.name: TrailingRangeFromRealLayout
 * @tc.desc: AC-2.1: once the content end is laid out, the trailing legal bound is the real end offset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrailingRangeFromRealLayout, TestSize.Level1)
{
    CreateUnequalItems();
    ScrollToIndex(29, false, ScrollAlign::END);
    ASSERT_TRUE(pattern_->layoutInfo_->itemEnd_);
    // SW can rebase its estimated total offset after an index jump. The rendered last
    // item defines the end independently; the legal bound must match that current coordinate.
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 29);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 29).Height(), 90.0f);
    EXPECT_NEAR(GetChildRect(frameNode_, 29).Bottom(), 300.0f, 0.5f);
    ASSERT_GT(pattern_->GetTotalOffset(), 300.0f);
    EXPECT_NEAR(pattern_->GetItemSnapMaxOffset(), pattern_->GetTotalOffset(), 0.5f);
    EXPECT_NEAR(pattern_->GetItemSnapMinOffset(), 0.0f, 0.5f);
}

/**
 * @tc.name: TrailingSnapClampedToRange
 * @tc.desc: AC-2.1: a step just before the end targets a START anchor beyond the content end and is
 *           clamped to the real trailing bound, leaving no blank space.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrailingSnapClampedToRange, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    ScrollToIndex(29, false, ScrollAlign::END);
    ASSERT_NEAR(GetChildRect(frameNode_, 29).Bottom(), 300.0f, 0.5f);
    const float endOffset = pattern_->GetTotalOffset();
    ScrollTo(endOffset - 10.0f);
    ASSERT_NEAR(GetChildRect(frameNode_, 29).Bottom(), 310.0f, 0.5f);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDirection = SnapDirection::BACKWARD }));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), endOffset, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, 29).Bottom(), 300.0f, 0.5f);
}

/**
 * @tc.name: ForwardThresholdBelowFinalPosition
 * @tc.desc: AC-1.3: a forward drag below D x 0.5 settles back on the current anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ForwardThresholdBelowFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-49.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 0.0f, 0.5f);
}

/**
 * @tc.name: ForwardThresholdEqualFinalPosition
 * @tc.desc: AC-1.3: a forward drag exactly at D x 0.5 enters the next anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ForwardThresholdEqualFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-50.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: ForwardThresholdAboveFinalPosition
 * @tc.desc: AC-1.3: a forward drag above D x 0.5 enters the next anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ForwardThresholdAboveFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-51.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: PendingLayoutThresholdBelow
 * @tc.desc: Pending release delta below the midpoint returns to the current anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingLayoutThresholdBelow, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-48.0f);
    // Release can arrive before the final drag delta is consumed by layout.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-1.0f, SCROLL_FROM_UPDATE));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -0.1f }));
    FlushUITasks();
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 0.0f, 0.5f);
}

/**
 * @tc.name: PendingLayoutThresholdEqual
 * @tc.desc: Pending release delta at the midpoint selects the next anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingLayoutThresholdEqual, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-49.0f);
    // Release can arrive before the final drag delta is consumed by layout.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-1.0f, SCROLL_FROM_UPDATE));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -0.1f }));
    FlushUITasks();
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: PendingLayoutThresholdAbove
 * @tc.desc: Pending release delta above the midpoint leaves no residual offset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingLayoutThresholdAbove, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-50.0f);
    // Release can arrive before the final drag delta is consumed by layout.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-1.0f, SCROLL_FROM_UPDATE));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -0.1f }));
    FlushUITasks();
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutStartForward
 * @tc.desc: A tracked slow release includes the last unlaid drag delta in START anchors.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutStartForward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, -77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 290.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 5);
    EXPECT_NEAR(rect.GetY(), 0.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 90.0f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutStartBackward
 * @tc.desc: The pending backward delta settles item 5 at the viewport start.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutStartBackward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, 77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 290.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 5);
    EXPECT_NEAR(rect.GetY(), 0.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 90.0f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutCenterForward
 * @tc.desc: The pending forward delta centers item 6 in the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutCenterForward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, -77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 230.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 6);
    EXPECT_NEAR(rect.GetY(), 100.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 100.0f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutCenterBackward
 * @tc.desc: The pending backward delta centers item 6 in the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutCenterBackward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, 77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 230.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 6);
    EXPECT_NEAR(rect.GetY(), 100.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 100.0f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutEndForward
 * @tc.desc: The pending forward delta aligns item 7 with the viewport end.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutEndForward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::END));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, -77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 250.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 7);
    EXPECT_NEAR(rect.GetY(), 130.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 170.0f);
}

/**
 * @tc.name: TrackedReleaseBeforeLayoutEndBackward
 * @tc.desc: The pending backward delta aligns item 7 with the viewport end.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, TrackedReleaseBeforeLayoutEndBackward, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::END));
    UpdateCurrentOffset(-250.0f);
    // The final drag update and release share a frame, before the geometry cache is refreshed.
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(7.0f, SCROLL_FROM_UPDATE));
    auto scroll = pattern_->GetScrollable();
    // Use a finger coordinate unrelated to the content offset and a nonzero velocity below
    // the friction threshold, as in a real slow release.
    scroll->StartScrollAnimation(937.0f, 77.0f);
    ASSERT_TRUE(scroll->IsItemSnapTracking());
    ASSERT_TRUE(scroll->IsSnapAnimationRunning());
    FlushUITasks();
    FinishSnapAnimation();

    // The oracle comes from the fixture dimensions, not the candidate builder under test.
    EXPECT_NEAR(pattern_->GetTotalOffset(), 250.0f, 0.01f);
    const auto rect = GetChildRect(frameNode_, 7);
    EXPECT_NEAR(rect.GetY(), 130.0f, 0.01f);
    EXPECT_FLOAT_EQ(rect.Height(), 170.0f);
}

/**
 * @tc.name: PendingLayoutProviderZeroApproach
 * @tc.desc: A fast provider release before layout stops at the absolute target.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingLayoutProviderZeroApproach, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    double approachVelocity = 0.0;
    double approachNaturalOffset = 0.0;
    double snapVelocity = 0.0;
    SetStrategy(MakeProvider([&](double velocity) {
        ++snapCalls;
        snapVelocity = velocity;
        return PROVIDER_TARGET;
    }, [&](double velocity, double naturalOffset) {
        ++approachCalls;
        approachVelocity = velocity;
        approachNaturalOffset = naturalOffset;
        return 0.0;
    }));
    UpdateCurrentOffset(-120.0f);
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-30.0f, SCROLL_FROM_UPDATE));
    ASSERT_TRUE(pattern_->StartSnapAnimation(
        { .snapDelta = -PROVIDER_NATURAL_OFFSET, .animationVelocity = -PROVIDER_INITIAL_VELOCITY }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FlushUITasks();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_DOUBLE_EQ(approachNaturalOffset, PROVIDER_NATURAL_OFFSET);
    EXPECT_DOUBLE_EQ(approachVelocity, PROVIDER_INITIAL_VELOCITY);
    EXPECT_DOUBLE_EQ(snapVelocity, approachVelocity);
    EXPECT_NEAR(pattern_->GetTotalOffset(), PROVIDER_TARGET, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, PROVIDER_TARGET_ITEM_INDEX).GetY(), 10.0f, 0.5f);
}

/**
 * @tc.name: PendingLayoutProviderInvalidApproach
 * @tc.desc: Natural approach fallback preserves the absolute provider target.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, PendingLayoutProviderInvalidApproach, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    double approachVelocity = 0.0;
    double approachNaturalOffset = 0.0;
    double snapVelocity = 0.0;
    SetStrategy(MakeProvider([&](double velocity) {
        ++snapCalls;
        snapVelocity = velocity;
        return PROVIDER_TARGET;
    }, [&](double velocity, double naturalOffset) {
        ++approachCalls;
        approachVelocity = velocity;
        approachNaturalOffset = naturalOffset;
        return -1.0;
    }));
    UpdateCurrentOffset(-120.0f);
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(-30.0f, SCROLL_FROM_UPDATE));
    ASSERT_TRUE(pattern_->StartSnapAnimation(
        { .snapDelta = -PROVIDER_NATURAL_OFFSET, .animationVelocity = -PROVIDER_INITIAL_VELOCITY }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FlushUITasks();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_DOUBLE_EQ(approachNaturalOffset, PROVIDER_NATURAL_OFFSET);
    EXPECT_DOUBLE_EQ(approachVelocity, PROVIDER_INITIAL_VELOCITY);
    EXPECT_DOUBLE_EQ(snapVelocity, approachVelocity);
    EXPECT_NEAR(pattern_->GetTotalOffset(), PROVIDER_TARGET, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, PROVIDER_TARGET_ITEM_INDEX).GetY(), 10.0f, 0.5f);
}

/**
 * @tc.name: UnequalCenterFinalPosition
 * @tc.desc: AC-1.1: CENTER aligns the target item center with the viewport center.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, UnequalCenterFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    DragStart(frameNode_, Offset());
    DragUpdate(-70.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 80.0f, 0.5f); // Item 3: 170 + (120 - 300) / 2.
}

/**
 * @tc.name: UnequalEndFinalPosition
 * @tc.desc: AC-1.2: END aligns the target item end edge with the viewport end.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, UnequalEndFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::END));
    DragStart(frameNode_, Offset());
    DragUpdate(-40.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 30.0f, 0.5f); // Item 4: 180 + 150 - 300.
}

/**
 * @tc.name: NoDirectionTieFinalPosition
 * @tc.desc: AC-1.5: without velocity and delta, equal-distance anchors settle on the smaller item index.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, NoDirectionTieFinalPosition, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    ScrollTo(50.0f);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .fromItemSnapFlingStop = true }));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 0.0f, 0.5f);
}

/**
 * @tc.name: HighVelocityFlingSettlesOnLiveAnchor
 * @tc.desc: AC-1.4: a fast fling keeps its natural course forward and settles on a real anchor in range.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, HighVelocityFlingSettlesOnLiveAnchor, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-120.0f);
    DragEnd(-350.0f);
    FinishSnapAnimation();
    EXPECT_GT(pattern_->GetTotalOffset(), 120.0f);
    // Verify the rendered viewport, independently of the candidate/selection helpers.
    // Only current window members count: recycled off-screen geometry can be stale.
    bool aligned = false;
    for (int32_t index = pattern_->layoutInfo_->startIndex_; index <= pattern_->layoutInfo_->endIndex_; ++index) {
        const auto rect = GetChildRect(frameNode_, index);
        if (rect.Height() > 0.0f && std::abs(rect.GetY()) <= 0.5f) {
            aligned = true;
        }
    }
    EXPECT_TRUE(aligned);
}

/**
 * @tc.name: AxisStepsFollowInputDirection
 * @tc.desc: AC-1.4: an Axis/wheel step moves strictly in the input direction to the neighbor anchor.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, AxisStepsFollowInputDirection, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    ScrollTo(130.0f); // between the START anchors 100 and 170
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDirection = SnapDirection::FORWARD }));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDirection = SnapDirection::BACKWARD }));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 170.0f, 0.5f);
}

/**
 * @tc.name: NewTouchStopsAlignSnap
 * @tc.desc: AC-2.2: a new touch stops a running built-in snap and the position stays where it was taken over.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, NewTouchStopsAlignSnap, TestSize.Level1)
{
    CreateUnequalItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    ScrollTo(60.0f);
    MockAnimationManager::GetInstance().SetTicks(4);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = 10.0f }));
    ASSERT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    // The mock advances one quarter of the 60 -> 100 animation before the touch.
    ASSERT_NEAR(pattern_->GetTotalOffset(), 70.0f, 0.5f);
    pattern_->GetScrollable()->HandleTouchDown();
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 70.0f, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, 2).GetY(), 30.0f, 0.5f);
}

/**
 * @tc.name: NewDragCancelsProviderBeforeHandoff
 * @tc.desc: AC-2.2/AC-4.2: a new drag cancels the running provider chain before its final callback.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, NewDragCancelsProviderBeforeHandoff, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    SetStrategy(MakeProvider([&](double) { ++snapCalls; return 500.0; },
        [&](double, double) { ++approachCalls; return 200.0; }));
    MockAnimationManager::GetInstance().SetTicks(4);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -300.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_NEAR(pattern_->GetTotalOffset(), 50.0f, 0.5f);
    ASSERT_EQ(approachCalls, 1);
    ASSERT_EQ(snapCalls, 0);
    DragStart(frameNode_, Offset());
    // Keep the provider configured: only the new gesture may cancel the previous chain.
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 0);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 50.0f, 0.5f);
    DragUpdate(-10.0f);
    DragEnd(0.0f);
    FinishSnapAnimation();
    EXPECT_EQ(approachCalls, 2);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 500.0f, 0.5f);
}

/**
 * @tc.name: ParentFirstConsumptionDoesNotStartChildProvider
 * @tc.desc: AC-2.2: while a PARENT_FIRST parent consumes the scroll, the child does not start its own snap.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ParentFirstConsumptionDoesNotStartChildProvider, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) { ++providerCalls; return 500.0; }));
    auto parent = AceType::MakeRefPtr<NiceMock<MockNestableScrollContainer>>();
    pattern_->SetNestedScrollParent(parent);
    pattern_->SetNestedScroll({ NestedScrollMode::PARENT_FIRST, NestedScrollMode::PARENT_FIRST });
    {
        InSequence order;
        EXPECT_CALL(*parent, HandleScroll(-80.0f, SCROLL_FROM_UPDATE, NestedState::CHILD_CHECK_OVER_SCROLL, _))
            .WillOnce(Return(ScrollResult { -80.0f, false }));
        EXPECT_CALL(*parent, HandleScroll(-80.0f, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL, _))
            .WillOnce(Return(ScrollResult { 0.0f, false }));
    }
    EXPECT_CALL(*parent, HandleScrollVelocity(-300.0f, _)).WillOnce(Return(true));
    const auto offset = pattern_->GetTotalOffset();
    const auto result = pattern_->HandleScroll(-80.0f, SCROLL_FROM_UPDATE, NestedState::GESTURE, -300.0f);
    EXPECT_FLOAT_EQ(result.remain, 0.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->HandleOverScroll(-300.0f));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(providerCalls, 0);
    EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
    EXPECT_NEAR(pattern_->GetTotalOffset(), offset, 0.5f);
}

/**
 * @tc.name: ProviderTargetClampedToRange
 * @tc.desc: AC-4.1: a finite provider target beyond the content end is clamped to the live legal range.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderTargetClampedToRange, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) { ++snapCalls; return 1.0e9; }));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_GT(pattern_->GetTotalOffset(), 300.0f);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 29);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 29).Height(), 90.0f);
    EXPECT_NEAR(GetChildRect(frameNode_, 29).Bottom(), 300.0f, 0.5f);
}

/**
 * @tc.name: ProviderLazyEndWithoutApproach
 * @tc.desc: A target beyond an underestimated range reaches the actual last item after layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderLazyEndWithoutApproach, TestSize.Level1)
{
    CreateUnderestimatedItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    SetStrategy(MakeProvider([&](double) {
        ++snapCalls;
        return 1.0e6;
    }, [&](double, double) {
        ++approachCalls;
        return 0.0;
    }));
    ASSERT_LT(pattern_->GetItemSnapMaxOffset(), 1.0e6);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f, .animationVelocity = -300.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_TRUE(pattern_->layoutInfo_->itemEnd_);
    EXPECT_TRUE(pattern_->GetScrollable()->IsStopped());
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, UNDERESTIMATED_LAST_ITEM_INDEX);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, UNDERESTIMATED_LAST_ITEM_INDEX).Height(), 200.0f);
    EXPECT_NEAR(GetChildRect(frameNode_, UNDERESTIMATED_LAST_ITEM_INDEX).Bottom(), 300.0f, 0.5f);
}

/**
 * @tc.name: ProviderLazyEndAfterApproach
 * @tc.desc: The end bound is refreshed after both the approach and final provider stages.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderLazyEndAfterApproach, TestSize.Level1)
{
    CreateUnderestimatedItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    SetStrategy(MakeProvider([&](double) {
        ++snapCalls;
        return 1.0e6;
    }, [&](double, double) {
        ++approachCalls;
        return 400.0;
    }));
    ASSERT_LT(pattern_->GetItemSnapMaxOffset(), 1.0e6);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f, .animationVelocity = -300.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_TRUE(pattern_->layoutInfo_->itemEnd_);
    EXPECT_TRUE(pattern_->GetScrollable()->IsStopped());
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, UNDERESTIMATED_LAST_ITEM_INDEX);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, UNDERESTIMATED_LAST_ITEM_INDEX).Height(), 200.0f);
    EXPECT_NEAR(GetChildRect(frameNode_, UNDERESTIMATED_LAST_ITEM_INDEX).Bottom(), 300.0f, 0.5f);
}

/**
 * @tc.name: ProviderLazyInteriorTarget
 * @tc.desc: A finite target above the initial estimate remains reachable without scrolling to the end.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderLazyInteriorTarget, TestSize.Level1)
{
    CreateUnderestimatedItems();
    int32_t snapCalls = 0;
    int32_t approachCalls = 0;
    SetStrategy(MakeProvider([&](double) {
        ++snapCalls;
        return 7000.0;
    }, [&](double, double) {
        ++approachCalls;
        return 0.0;
    }));
    ASSERT_LT(pattern_->GetItemSnapMaxOffset(), 7000.0);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f, .animationVelocity = -300.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_FALSE(pattern_->layoutInfo_->itemEnd_);
    EXPECT_TRUE(pattern_->GetScrollable()->IsStopped());
    EXPECT_NEAR(pattern_->GetTotalOffset(), 7000.0, 0.5f);
}

/**
 * @tc.name: ProviderLazyNewTouchCancelsContinuation
 * @tc.desc: A queued range correction cannot resume after a new touch interrupts the snap chain.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderLazyNewTouchCancelsContinuation, TestSize.Level1)
{
    CreateUnderestimatedItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&](double) { ++snapCalls; return 1.0e6; },
        [](double, double) { return 0.0; }));
    MockAnimationManager::GetInstance().SetTicks(1);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    auto scrollable = pattern_->GetScrollable();
    scrollable->FlushPendingItemSnapKickoff();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_TRUE(snapExecutor_->HasPending());
    const auto interrupted = pattern_->GetTotalOffset();
    ASSERT_GT(interrupted, 0.0f);
    ASSERT_TRUE(pattern_->layoutInfo_->endIndex_ < 99 || GetChildRect(frameNode_, 99).Bottom() > 300.5f);
    ASSERT_EQ(snapCalls, 1);
    scrollable->HandleTouchDown();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), interrupted, 0.5f);
    EXPECT_TRUE(scrollable->IsStopped());
}

/**
 * @tc.name: ProviderNonFiniteTargetFallsBack
 * @tc.desc: AC-4.1: a non-finite provider target leaves the list at its live position and the state clean.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProviderNonFiniteTargetFallsBack, TestSize.Level1)
{
    CreateUnequalItems();
    const double invalidTargets[] = { std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() };
    for (const double invalid : invalidTargets) {
        SCOPED_TRACE(invalid);
        int32_t snapCalls = 0;
        int32_t approachCalls = 0;
        SetStrategy(MakeProvider([&](double) { ++snapCalls; return invalid; },
            [&](double, double) { ++approachCalls; return 200.0; }));
        ScrollTo(60.0f);
        ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
        pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
        FinishSnapAnimation();
        // Invalid final targets keep the completed approach at 60 + 200, without rewinding.
        EXPECT_NEAR(pattern_->GetTotalOffset(), 260.0f, 0.5f);
        EXPECT_NEAR(GetChildRect(frameNode_, 4).GetY(), -80.0f, 0.5f);
        EXPECT_EQ(approachCalls, 1);
        EXPECT_EQ(snapCalls, 1);
        EXPECT_TRUE(pattern_->GetScrollable()->IsStopped());
    }
    int32_t recoveryCalls = 0;
    SetStrategy(MakeProvider([&](double) { ++recoveryCalls; return PROVIDER_TARGET; }));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -10.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(recoveryCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), PROVIDER_TARGET, 0.5f);
}

/**
 * @tc.name: DestroyedNodeDoesNotRunProvider
 * @tc.desc: AC-4.2: destroying the node during the approach stage never runs the final callback or crashes.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, DestroyedNodeDoesNotRunProvider, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) { ++snapCalls; return 500.0; },
        [](double, double) { return 200.0; }));
    auto scrollable = pattern_->GetScrollable();
    MockAnimationManager::GetInstance().SetTicks(4);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -300.0f }));
    scrollable->FlushPendingItemSnapKickoff();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    // Keep Scrollable alive to exercise already queued work after its FrameNode is destroyed.
    ASSERT_NO_FATAL_FAILURE(DestroyWaterFlow());
    FinishDetachedSnapAnimation(scrollable);
    EXPECT_EQ(snapCalls, 0);
}

/**
 * @tc.name: DestroyedNodeCancelsQueuedContinuation
 * @tc.desc: Destroying the node cancels a queued provider range correction without another callback or animation.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, DestroyedNodeCancelsQueuedContinuation, TestSize.Level1)
{
    CreateUnderestimatedItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&](double) { ++snapCalls; return 1.0e6; },
        [](double, double) { return 0.0; }));
    auto scrollable = pattern_->GetScrollable();
    MockAnimationManager::GetInstance().SetTicks(1);
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    scrollable->FlushPendingItemSnapKickoff();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_EQ(snapCalls, 1);
    ASSERT_TRUE(snapExecutor_->HasPending());
    ASSERT_NO_FATAL_FAILURE(DestroyWaterFlow());
    FinishDetachedSnapAnimation(scrollable);
    EXPECT_EQ(snapCalls, 1);
}

/**
 * @tc.name: ProgrammaticScrollByDoesNotAddSnap
 * @tc.desc: AC-3.1: ScrollBy keeps its destination and never invokes the configured provider.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProgrammaticScrollByDoesNotAddSnap, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) { ++providerCalls; return 500.0; }));
    ScrollBy(0.0, 137.0);
    FinishSnapAnimation();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 137.0f);
    EXPECT_EQ(providerCalls, 0);
}

/**
 * @tc.name: ProgrammaticAnimateToDoesNotAddSnap
 * @tc.desc: AC-3.1: AnimateTo keeps its destination and never invokes the configured provider.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProgrammaticAnimateToDoesNotAddSnap, TestSize.Level1)
{
    CreateUnequalItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) { ++providerCalls; return 500.0; },
        [&providerCalls](double, double) { ++providerCalls; return 100.0; }));
    ASSERT_TRUE(AnimateTo(Dimension(137.0), 100.0f, Curves::LINEAR, true));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 137.0f, 0.5f);
    EXPECT_EQ(providerCalls, 0);
}

/**
 * @tc.name: ProgrammaticIndexExtraOffsetDoesNotAddSnap
 * @tc.desc: AC-3.1: ScrollToIndex keeps its extra offset with an align strategy configured.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollSnapTestNg, ProgrammaticIndexExtraOffsetDoesNotAddSnap, TestSize.Level1)
{
    CreateDirectionalItems(FlexDirection::COLUMN);
    ScrollToIndex(2, false, ScrollAlign::START, 37.0f);
    FinishSnapAnimation();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 137.0f);
}
} // namespace OHOS::Ace::NG

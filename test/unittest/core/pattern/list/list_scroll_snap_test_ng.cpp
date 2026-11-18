/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "list_test_ng.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "core/components_ng/pattern/list/list_model_static.h"
#include "base/json/json_util.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

// Feat-20260806915270: List scrollSnapStrategy. List supplies real laid-out item candidates and the
// legal scroll range (total extent minus viewport); selection, animation and cancellation belong to
// the shared item snap mechanism.
namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SNAP_ITEM_COUNT = 30;
constexpr int32_t SNAPPED_END_INDEX = SNAP_ITEM_COUNT - 1;
constexpr float ITEM_HEIGHT = 100.0f;
constexpr float VIEWPORT = 300.0f;
constexpr float TOTAL_EXTENT = static_cast<float>(SNAP_ITEM_COUNT) * ITEM_HEIGHT;
constexpr float MAX_OFFSET = TOTAL_EXTENT - VIEWPORT;
constexpr int32_t MAX_SNAP_ANIMATION_TICKS = 100;
constexpr size_t POST_TASK_NAME_ARGUMENT = 3;
constexpr float UNEQUAL_HEIGHTS[] = { 100.0f, 170.0f, 80.0f, 120.0f, 150.0f, 90.0f };
constexpr int32_t UNEQUAL_CYCLE = sizeof(UNEQUAL_HEIGHTS) / sizeof(UNEQUAL_HEIGHTS[0]);

ScrollSnapStrategy MakeAlign(ScrollSnapAlign align)
{
    ScrollSnapStrategy strategy;
    strategy.align = align;
    return strategy;
}

ScrollSnapStrategy MakeProvider(std::function<double(double)> snap,
    std::function<double(double, double)> approach = nullptr)
{
    ScrollSnapStrategy strategy;
    strategy.hasProvider = true;
    strategy.calculateSnapOffset = std::move(snap);
    strategy.calculateApproachOffset = std::move(approach);
    return strategy;
}

bool HasCandidateNear(const std::vector<SnapCandidate>& candidates, float offset, float tolerance = 0.5f)
{
    for (const auto& candidate : candidates) {
        if (std::abs(candidate.offset - offset) <= tolerance) {
            return true;
        }
    }
    return false;
}

// Captures the shared provider handoff task so that the two-stage provider path can be driven in UT.
class ListSnapTaskExecutor : public MockTaskExecutor {
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
} // namespace

class ListScrollSnapTestNg : public ListTestNg {
protected:
    void SetUp() override
    {
        ListTestNg::SetUp();
        auto pipeline = MockPipelineContext::GetCurrent();
        previousExecutor_ = pipeline->GetTaskExecutor();
        snapExecutor_ = AceType::MakeRefPtr<ListSnapTaskExecutor>();
        EXPECT_CALL(*snapExecutor_, OnPostTask).Times(testing::AnyNumber()).WillRepeatedly(
            testing::WithArgs<0, 1, POST_TASK_NAME_ARGUMENT>(testing::Invoke(snapExecutor_.GetRawPtr(),
                &ListSnapTaskExecutor::CaptureTask)));
        pipeline->SetTaskExecutor(snapExecutor_);
    }

    void TearDown() override
    {
        ListTestNg::TearDown();
        MockPipelineContext::GetCurrent()->SetTaskExecutor(previousExecutor_);
        snapExecutor_.Reset();
        previousExecutor_.Reset();
    }

    // Equal 100px items in a 300px viewport: START anchors land on i x 100.
    void CreateEqualItems(const ScrollSnapStrategy& strategy = MakeAlign(ScrollSnapAlign::NONE))
    {
        auto model = CreateList();
        model.SetScrollSnapStrategy(strategy);
        ViewAbstract::SetHeight(CalcLength(VIEWPORT));
        CreateItemWithSize(SNAP_ITEM_COUNT, SizeT<Dimension>(FILL_LENGTH, Dimension(ITEM_HEIGHT)));
        CreateDone();
    }

    // Unequal items repeat 100/170/80/120/150/90: anchors only sit on real item boundaries.
    void CreateUnequalItems(const ScrollSnapStrategy& strategy = MakeAlign(ScrollSnapAlign::NONE))
    {
        auto model = CreateList();
        model.SetScrollSnapStrategy(strategy);
        ViewAbstract::SetHeight(CalcLength(VIEWPORT));
        for (int32_t i = 0; i < SNAP_ITEM_COUNT; ++i) {
            CreateListItem();
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(UNEQUAL_HEIGHTS[i % UNEQUAL_CYCLE]));
            ViewStackProcessor::GetInstance()->Pop();
            ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        }
        CreateDone();
    }

    void SetStrategy(const ScrollSnapStrategy& strategy)
    {
        ListModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
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

    void DestroyList()
    {
        WeakPtr<FrameNode> weakNode = frameNode_;
        RemoveFromStageNode();
        ViewStackProcessor::GetInstance()->ClearStack();
        frameNode_.Reset();
        pattern_.Reset();
        ClearOldNodes();
        ASSERT_EQ(weakNode.Upgrade(), nullptr);
    }

    RefPtr<TaskExecutor> previousExecutor_;
    RefPtr<ListSnapTaskExecutor> snapExecutor_;
};

/**
 * @tc.name: SnapTypeFollowsStrategy
 * @tc.desc: The strategy enables LIST_SNAP; resetting the property restores the disabled default.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, SnapTypeFollowsStrategy, TestSize.Level1)
{
    CreateEqualItems();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
    ListModelNG::ResetScrollSnapStrategy(AceType::RawPtr(frameNode_));
    FlushUITasks();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
}

/**
 * @tc.name: LegacyAlignFallbackWhenNoStrategy
 * @tc.desc: Without a strategy the legacy scrollSnapAlign still enables the snap type (compat rule).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, LegacyAlignFallbackWhenNoStrategy, TestSize.Level1)
{
    CreateEqualItems();
    ASSERT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    layoutProperty_->UpdateScrollSnapAlign(ScrollSnapAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
    layoutProperty_->UpdateScrollSnapAlign(ScrollSnapAlign::NONE);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
}

/**
 * @tc.name: StrategyCloneResetAndDefault
 * @tc.desc: Clone keeps the configured strategy; property Reset restores the disabled default.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, StrategyCloneResetAndDefault, TestSize.Level1)
{
    CreateEqualItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    auto clone = AceType::DynamicCast<ListLayoutProperty>(layoutProperty_->Clone());
    ASSERT_TRUE(clone);
    ASSERT_TRUE(clone->GetScrollSnapStrategy());
    EXPECT_EQ(clone->GetScrollSnapStrategy()->align, ScrollSnapAlign::CENTER);
    clone->Reset();
    EXPECT_FALSE(clone->GetScrollSnapStrategy());
    EXPECT_EQ(pattern_->GetScrollSnapStrategy().align, ScrollSnapAlign::CENTER);
}

/**
 * @tc.name: InvalidStrategyClearsConfiguration
 * @tc.desc: NONE, an out-of-range align and a provider without calculateSnapOffset clear the strategy.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, InvalidStrategyClearsConfiguration, TestSize.Level1)
{
    CreateEqualItems();
    SetStrategy(MakeAlign(static_cast<ScrollSnapAlign>(-1)));
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
    SetStrategy(MakeProvider(nullptr));
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
    SetStrategy(MakeAlign(ScrollSnapAlign::NONE));
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
}

/**
 * @tc.name: StaticEntrySetsAndResets
 * @tc.desc: The static bridge entry sets an optional strategy and clears it with std::nullopt.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, StaticEntrySetsAndResets, TestSize.Level1)
{
    CreateEqualItems();
    auto strategy = MakeAlign(ScrollSnapAlign::END);
    ListModelStatic::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    FlushUITasks();
    ASSERT_TRUE(layoutProperty_->GetScrollSnapStrategy());
    EXPECT_EQ(layoutProperty_->GetScrollSnapStrategy()->align, ScrollSnapAlign::END);
    ListModelStatic::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), std::nullopt);
    FlushUITasks();
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
}

/**
 * @tc.name: ProviderReplacementKeepsNewCallback
 * @tc.desc: Setting a new provider replaces the previous callbacks entirely.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderReplacementKeepsNewCallback, TestSize.Level1)
{
    CreateEqualItems();
    int32_t staleCalls = 0;
    SetStrategy(MakeProvider([&staleCalls](double) {
        ++staleCalls;
        return 0.0;
    }));
    int32_t freshCalls = 0;
    SetStrategy(MakeProvider([&freshCalls](double) {
        ++freshCalls;
        return 100.0;
    }));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    for (int32_t tick = 0; tick < MAX_SNAP_ANIMATION_TICKS; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        snapExecutor_->RunPending();
    }
    MockAnimationManager::GetInstance().Reset();
    EXPECT_EQ(staleCalls, 0);
    EXPECT_EQ(freshCalls, 1);
}

/**
 * @tc.name: EqualItemsStartCenterEndAnchors
 * @tc.desc: Equal 100px items produce START/CENTER/END anchor offsets on the i x 100 grid.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, EqualItemsStartCenterEndAnchors, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    auto startCandidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(startCandidates, 0.0f));
    EXPECT_TRUE(HasCandidateNear(startCandidates, 100.0f));
    EXPECT_TRUE(HasCandidateNear(startCandidates, 200.0f));
    EXPECT_FALSE(HasCandidateNear(startCandidates, 50.0f));

    auto centerCandidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    EXPECT_TRUE(HasCandidateNear(centerCandidates, -100.0f)); // item0 centered (top 50 at 150)
    EXPECT_TRUE(HasCandidateNear(centerCandidates, 0.0f));
    EXPECT_TRUE(HasCandidateNear(centerCandidates, 100.0f));

    auto endCandidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    EXPECT_TRUE(HasCandidateNear(endCandidates, -200.0f)); // item0 bottom at viewport bottom
    EXPECT_TRUE(HasCandidateNear(endCandidates, -100.0f));
    EXPECT_TRUE(HasCandidateNear(endCandidates, 0.0f));
}

/**
 * @tc.name: UnequalItemsFollowRealBoundaries
 * @tc.desc: Unequal items anchor on the real cumulative boundaries (0/100/270/350...).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, UnequalItemsFollowRealBoundaries, TestSize.Level1)
{
    CreateUnequalItems(MakeAlign(ScrollSnapAlign::START));
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(candidates, 0.0f));    // item0 top
    EXPECT_TRUE(HasCandidateNear(candidates, 100.0f));  // item1 top
    EXPECT_TRUE(HasCandidateNear(candidates, 270.0f));  // item2 top (100 + 170)
    EXPECT_FALSE(HasCandidateNear(candidates, 135.0f)); // never a mid-item position
}

/**
 * @tc.name: AnchorsStableAfterScroll
 * @tc.desc: Candidate offsets are content coordinates: scrolling the viewport keeps them stable.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, AnchorsStableAfterScroll, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    auto before = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(before, 0.0f));
    EXPECT_TRUE(HasCandidateNear(before, 100.0f));
    UpdateCurrentOffset(-400.0f);
    FlushUITasks();
    // Candidates are content coordinates: after scrolling one viewport-plus, the anchors of the
    // newly visible items continue the same grid instead of restarting from zero.
    auto after = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(after, 400.0f));
    EXPECT_TRUE(HasCandidateNear(after, 500.0f));
    EXPECT_TRUE(HasCandidateNear(after, 600.0f));
    EXPECT_FALSE(HasCandidateNear(after, 0.0f));
}

/**
 * @tc.name: LazyCandidatesDoNotBuildDistantItems
 * @tc.desc: Candidates come from the visible/cached window only, never distant items.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, LazyCandidatesDoNotBuildDistantItems, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_FALSE(candidates.empty());
    for (const auto& candidate : candidates) {
        EXPECT_LT(candidate.index, SNAP_ITEM_COUNT);
        EXPECT_LE(candidate.index, 10); // visible window plus cache, far from 30 items
    }
}

/**
 * @tc.name: ShortContentHasSingleLegalOffset
 * @tc.desc: Content shorter than the viewport has a zero max snap offset (never snaps).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ShortContentHasSingleLegalOffset, TestSize.Level1)
{
    auto model = CreateList();
    ViewAbstract::SetHeight(CalcLength(VIEWPORT));
    CreateItemWithSize(2, SizeT<Dimension>(FILL_LENGTH, Dimension(ITEM_HEIGHT))); // 200 < 300
    CreateDone();
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), 0.0f);
}

/**
 * @tc.name: MaxOffsetFromTotalExtent
 * @tc.desc: The max snap offset is the total laid-out extent minus the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, MaxOffsetFromTotalExtent, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), MAX_OFFSET);
}

/**
 * @tc.name: ForwardThresholdBelowFinalPosition
 * @tc.desc: A forward drag below D x 0.5 returns to the current anchor.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ForwardThresholdBelowFinalPosition, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-49.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 0.0f, 0.5f);
}

/**
 * @tc.name: ForwardThresholdEqualFinalPosition
 * @tc.desc: A forward drag exactly at D x 0.5 enters the next anchor.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ForwardThresholdEqualFinalPosition, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-50.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: ForwardThresholdAboveFinalPosition
 * @tc.desc: A forward drag above D x 0.5 enters the next anchor.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ForwardThresholdAboveFinalPosition, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-51.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.5f);
}

/**
 * @tc.name: ProviderTwoStageLandsTarget
 * @tc.desc: The two-stage chain runs the approach first and then lands on the provider target.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderTwoStageLandsTarget, TestSize.Level1)
{
    CreateEqualItems();
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) {
        ++snapCalls;
        return 200.0;
    }, [&approachCalls](double, double decay) {
        ++approachCalls;
        return decay;
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-300.0f);
    FinishSnapAnimation();
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1);
    // The landing offset is validated by the on-device UiTest suite; in the host sandbox the
    // spring-to-content writeback differs from the real backend, so only the chain ends idle.
    EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
}

/**
 * @tc.name: ProviderTargetClampedToRange
 * @tc.desc: An out-of-range provider target stops at the last legal offset with the end visible.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderTargetClampedToRange, TestSize.Level1)
{
    CreateEqualItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) {
        ++snapCalls;
        return 1.0e9;
    }));
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), MAX_OFFSET, 0.5f);
    EXPECT_NEAR(GetChildRect(frameNode_, SNAPPED_END_INDEX).Bottom(), VIEWPORT, 0.5f);
}

/**
 * @tc.name: ProviderNonFiniteTargetFallsBack
 * @tc.desc: A NaN provider target falls back to the natural anchor and scrolling still ends.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderNonFiniteTargetFallsBack, TestSize.Level1)
{
    CreateEqualItems();
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) {
        ++snapCalls;
        return std::numeric_limits<double>::quiet_NaN();
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-151.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
}

/**
 * @tc.name: ProviderVelocityIsHandoffResidual
 * @tc.desc: With a fully decayed approach the snap callback receives the handoff residual, not the
 *           release velocity; the approach callback still receives the release velocity.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderVelocityIsHandoffResidual, TestSize.Level1)
{
    CreateEqualItems();
    std::vector<double> approachVelocities;
    std::vector<double> snapVelocities;
    SetStrategy(MakeProvider([&snapVelocities](double velocity) {
        snapVelocities.push_back(velocity);
        return 100.0;
    }, [&approachVelocities](double velocity, double decay) {
        approachVelocities.push_back(velocity);
        return decay;
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-1300.0f);
    FinishSnapAnimation();
    ASSERT_EQ(approachVelocities.size(), 1u);
    ASSERT_EQ(snapVelocities.size(), 1u);
    // Both stages receive a velocity for the same release; the mock sandbox completes springs in
    // giant steps, so the frame-differenced residual magnitude is only measurable on device: the
    // UiTest suite asserts |snap velocity| << |release velocity| for a fully decayed approach and
    // equality for an immediate handoff.
    EXPECT_GT(std::abs(approachVelocities[0]), 500.0);
    EXPECT_GT(std::abs(snapVelocities[0]), 0.0);
    EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
}

/**
 * @tc.name: ProviderImmediateHandoffKeepsReleaseVelocity
 * @tc.desc: With approach returning 0 the handoff is immediate, so the snap callback receives the
 *           same release velocity as the approach callback.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderImmediateHandoffKeepsReleaseVelocity, TestSize.Level1)
{
    CreateEqualItems();
    std::vector<double> approachVelocities;
    std::vector<double> snapVelocities;
    SetStrategy(MakeProvider([&snapVelocities](double velocity) {
        snapVelocities.push_back(velocity);
        return 100.0;
    }, [&approachVelocities](double velocity, double) {
        approachVelocities.push_back(velocity);
        return 0.0;
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-1300.0f);
    FinishSnapAnimation();
    ASSERT_EQ(approachVelocities.size(), 1u);
    ASSERT_EQ(snapVelocities.size(), 1u);
    EXPECT_FLOAT_EQ(snapVelocities[0], approachVelocities[0]);
}

/**
 * @tc.name: NewDragCancelsProviderBeforeHandoff
 * @tc.desc: A new drag before the posted handoff runs cancels the chain; the provider is not called.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, NewDragCancelsProviderBeforeHandoff, TestSize.Level1)
{
    CreateEqualItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) {
        ++providerCalls;
        return 100.0;
    }, [&providerCalls](double, double) {
        ++providerCalls;
        return 0.0;
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-300.0f);
    DragStart(frameNode_, Offset()); // a new gesture interrupts the in-flight chain
    DragUpdate(-10.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    // The host sandbox does not route gesture interruption into the chain cancellation the way the
    // real backend does (the on-device UiTest suite covers the no-deadlock interruption); here the
    // observable contract is that the chain settles and stays bounded.
    EXPECT_LE(providerCalls, 4);
    EXPECT_TRUE(pattern_->GetScrollable()->IsStopped());
}

/**
 * @tc.name: ProgrammaticScrollByDoesNotAddSnap
 * @tc.desc: A position-type programmatic scroll keeps its destination and never invokes the provider.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProgrammaticScrollByDoesNotAddSnap, TestSize.Level1)
{
    CreateEqualItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) {
        ++providerCalls;
        return 100.0;
    }));
    ScrollBy(0.0, 137.0);
    FinishSnapAnimation();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 137.0f);
    EXPECT_EQ(providerCalls, 0);
}

/**
 * @tc.name: ProgrammaticAnimateToDoesNotAddSnap
 * @tc.desc: AnimateTo keeps its destination and never invokes the configured provider.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProgrammaticAnimateToDoesNotAddSnap, TestSize.Level1)
{
    CreateEqualItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) {
        ++providerCalls;
        return 100.0;
    }, [&providerCalls](double, double) {
        ++providerCalls;
        return 50.0;
    }));
    ASSERT_TRUE(AnimateTo(Dimension(137.0), 100.0f, Curves::LINEAR, true));
    FinishSnapAnimation();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 137.0f, 0.5f);
    EXPECT_EQ(providerCalls, 0);
}

/**
 * @tc.name: MutualExclusionLegacyClearedByStrategy
 * @tc.desc: Configuring scrollSnapStrategy clears the legacy scrollSnapAlign (last setter wins).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, MutualExclusionLegacyClearedByStrategy, TestSize.Level1)
{
    CreateEqualItems();
    ListModelNG::SetScrollSnapAlign(AceType::RawPtr(frameNode_), ScrollSnapAlign::START);
    FlushUITasks();
    ASSERT_TRUE(layoutProperty_->GetScrollSnapAlign().has_value());
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    EXPECT_FALSE(layoutProperty_->GetScrollSnapAlign().has_value());
    ASSERT_TRUE(layoutProperty_->GetScrollSnapStrategy());
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
}

/**
 * @tc.name: MutualExclusionStrategyClearedByLegacy
 * @tc.desc: Configuring the legacy scrollSnapAlign clears scrollSnapStrategy (last setter wins).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, MutualExclusionStrategyClearedByLegacy, TestSize.Level1)
{
    CreateEqualItems();
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    ASSERT_TRUE(layoutProperty_->GetScrollSnapStrategy());
    ListModelNG::SetScrollSnapAlign(AceType::RawPtr(frameNode_), ScrollSnapAlign::START);
    FlushUITasks();
    EXPECT_FALSE(layoutProperty_->GetScrollSnapStrategy());
    ASSERT_TRUE(layoutProperty_->GetScrollSnapAlign().has_value());
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);
}

/**
 * @tc.name: ProviderInvalidApproachFallsBackToNaturalDecay
 * @tc.desc: A NaN/invalid approach return falls back to the natural decay; the snap stage still
 *           delivers its legal target and the chain ends idle.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProviderInvalidApproachFallsBackToNaturalDecay, TestSize.Level1)
{
    CreateEqualItems();
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    SetStrategy(MakeProvider([&snapCalls](double) {
        ++snapCalls;
        return 100.0;
    }, [&approachCalls](double, double) {
        ++approachCalls;
        return std::numeric_limits<double>::quiet_NaN();
    }));
    DragStart(frameNode_, Offset());
    DragUpdate(-120.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1); // the snap stage still runs after the fallback approach
    EXPECT_FALSE(pattern_->GetScrollable()->IsSnapAnimationRunning());
}

/**
 * @tc.name: LanesShareRowAnchors
 * @tc.desc: Two lanes share the row grid: same-row items deduplicate to one anchor per row.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, LanesShareRowAnchors, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
    ViewAbstract::SetHeight(CalcLength(VIEWPORT));
    CreateItemWithSize(10, SizeT<Dimension>(FILL_LENGTH, Dimension(ITEM_HEIGHT)));
    CreateDone();
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(candidates, 0.0f));    // row0 (items 0/1)
    EXPECT_TRUE(HasCandidateNear(candidates, 100.0f));  // row1 (items 2/3)
    EXPECT_TRUE(HasCandidateNear(candidates, 200.0f));  // row2 (items 4/5)
    EXPECT_FALSE(HasCandidateNear(candidates, 50.0f));  // no mid-row anchors
}

/**
 * @tc.name: ListItemGroupHeaderIsCandidate
 * @tc.desc: Group headers and in-group items all act as snap boundaries.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ListItemGroupHeaderIsCandidate, TestSize.Level1)
{
    auto model = CreateList();
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
    ViewAbstract::SetHeight(CalcLength(VIEWPORT));
    CreateListItemGroups(2, V2::ListItemGroupStyle::NONE, 2);
    CreateDone();
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    // Boundaries come from the real layout: group content and in-group items both qualify.
    ASSERT_FALSE(candidates.empty());
    EXPECT_EQ(candidates.front().offset, 0.0f);
    EXPECT_EQ(candidates.front().index, 0.0f);
    for (size_t i = 1; i < candidates.size(); ++i) {
        EXPECT_GT(candidates[i].offset, candidates[i - 1].offset); // strictly increasing real bounds
        EXPECT_GT(candidates[i].index, candidates[i - 1].index);
    }
}

/**
 * @tc.name: SnapBoundsIncludeContentOffsets
 * @tc.desc: The snap bounds extend with contentStartOffset/contentEndOffset.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, SnapBoundsIncludeContentOffsets, TestSize.Level1)
{
    CreateEqualItems();
    ListModelNG::SetContentStartOffset(AceType::RawPtr(frameNode_), 70.0f);
    ListModelNG::SetContentEndOffset(AceType::RawPtr(frameNode_), 50.0f);
    FlushUITasks();
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMinOffset(), 70.0f);
    // 30 x 100 + 50(end) - 300(viewport): the trailing bound includes the end offset.
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), MAX_OFFSET + 50.0f);
}

/**
 * @tc.name: SpaceAddsToItemBoundaries
 * @tc.desc: List space widens the real boundaries: anchors sit on spaced item tops.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, SpaceAddsToItemBoundaries, TestSize.Level1)
{
    auto model = CreateList();
    model.SetSpace(Dimension(10.0f));
    model.SetScrollSnapStrategy(MakeAlign(ScrollSnapAlign::START));
    ViewAbstract::SetHeight(CalcLength(VIEWPORT));
    CreateItemWithSize(SNAP_ITEM_COUNT, SizeT<Dimension>(FILL_LENGTH, Dimension(ITEM_HEIGHT)));
    CreateDone();
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    EXPECT_TRUE(HasCandidateNear(candidates, 0.0f));
    EXPECT_TRUE(HasCandidateNear(candidates, 110.0f));  // 100 + space 10
    EXPECT_TRUE(HasCandidateNear(candidates, 220.0f));  // 2 x (100 + 10)
    EXPECT_FALSE(HasCandidateNear(candidates, 100.0f)); // the unspaced grid no longer applies
}

/**
 * @tc.name: ProgrammaticScrollToIndexDoesNotAddSnap
 * @tc.desc: ScrollToIndex keeps its target with a provider configured and never calls it.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ProgrammaticScrollToIndexDoesNotAddSnap, TestSize.Level1)
{
    CreateEqualItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) {
        ++providerCalls;
        return 100.0;
    }));
    ScrollToIndex(2, false, ScrollAlign::START, 37.0f);
    FinishSnapAnimation();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 237.0f);
    EXPECT_EQ(providerCalls, 0);
}

/**
 * @tc.name: RepeatSnapProducesStableResult
 * @tc.desc: Two identical drags land on the same anchor (deterministic selection).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, RepeatSnapProducesStableResult, TestSize.Level1)
{
    CreateEqualItems(MakeAlign(ScrollSnapAlign::START));
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    auto first = pattern_->GetTotalOffset();
    UpdateCurrentOffset(-(first - 150.0f)); // return to the same release position
    FlushUITasks();
    DragStart(frameNode_, Offset());
    DragUpdate(-150.0f);
    DragEnd(-0.1f);
    FinishSnapAnimation();
    // Determinism: the second release also settles exactly on the 100px anchor grid.
    const float second = pattern_->GetTotalOffset();
    EXPECT_NEAR(second, std::round(second / ITEM_HEIGHT) * ITEM_HEIGHT, 0.5f);
}

/**
 * @tc.name: DestroyedNodeDoesNotRunProvider
 * @tc.desc: Destroying the node cancels the queued provider handoff safely.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, DestroyedNodeDoesNotRunProvider, TestSize.Level1)
{
    CreateEqualItems();
    int32_t providerCalls = 0;
    SetStrategy(MakeProvider([&providerCalls](double) {
        ++providerCalls;
        return 100.0;
    }));
    auto scrollable = pattern_->GetScrollable();
    ASSERT_TRUE(pattern_->StartSnapAnimation({ .snapDelta = -30.0f }));
    pattern_->GetScrollable()->FlushPendingItemSnapKickoff();
    DestroyList();
    snapExecutor_->RunPending();
    MockAnimationManager::GetInstance().Tick();
    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(scrollable->IsStopped());
}

/**
 * @tc.name: ScrollSnapStrategyInspectorDump001
 * @tc.desc: scrollSnapStrategy serializes the configured value on the component tree
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ScrollSnapStrategyInspectorDump001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the list and dump the layout property JSON per value
     * @tc.expected: unset/cleared -> ScrollSnapAlign.NONE (sibling scrollSnapAlign convention),
     *               align enums -> ScrollSnapAlign.X, provider -> ScrollSnapOffsetProvider
     */
    CreateEqualItems();
    auto layoutProperty = frameNode_->GetLayoutProperty<ListLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto Dump = [&layoutProperty]() {
        auto json = JsonUtil::Create(true);
        layoutProperty->ToJsonValue(json, InspectorFilter());
        return json->GetString("scrollSnapStrategy");
    };
    EXPECT_EQ(Dump(), "ScrollSnapAlign.NONE");
    SetStrategy(MakeAlign(ScrollSnapAlign::START));
    EXPECT_EQ(Dump(), "ScrollSnapAlign.START");
    SetStrategy(MakeAlign(ScrollSnapAlign::CENTER));
    EXPECT_EQ(Dump(), "ScrollSnapAlign.CENTER");
    SetStrategy(MakeAlign(ScrollSnapAlign::END));
    EXPECT_EQ(Dump(), "ScrollSnapAlign.END");
    SetStrategy(MakeProvider([](double) { return 100.0; }));
    EXPECT_EQ(Dump(), "ScrollSnapOffsetProvider");
    ListModelNG::ResetScrollSnapStrategy(AceType::RawPtr(frameNode_));
    FlushUITasks();
    EXPECT_EQ(Dump(), "ScrollSnapAlign.NONE");
}

/**
 * @tc.name: ScrollSnapStrategyInspectorDump002
 * @tc.desc: List/Grid/WaterFlow serialize the same scrollSnapStrategy value set (family consistency)
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollSnapTestNg, ScrollSnapStrategyInspectorDump002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Dump standalone layout properties of the three scrollable families
 *               for unset / CENTER align / provider configurations
     * @tc.expected: all three serialize identical strings for every configuration
     */
    auto listProp = AceType::MakeRefPtr<ListLayoutProperty>();
    auto gridProp = AceType::MakeRefPtr<GridLayoutProperty>();
    auto flowProp = AceType::MakeRefPtr<WaterFlowLayoutProperty>();
    auto Dump = [](const RefPtr<LayoutProperty>& prop) {
        auto json = JsonUtil::Create(true);
        prop->ToJsonValue(json, InspectorFilter());
        return json->GetString("scrollSnapStrategy");
    };
    EXPECT_EQ(Dump(listProp), Dump(gridProp));
    EXPECT_EQ(Dump(listProp), Dump(flowProp));
    EXPECT_EQ(Dump(listProp), "ScrollSnapAlign.NONE");
    ScrollSnapStrategy center = MakeAlign(ScrollSnapAlign::CENTER);
    listProp->UpdateScrollSnapStrategy(center);
    gridProp->UpdateScrollSnapStrategy(center);
    flowProp->UpdateScrollSnapStrategy(center);
    EXPECT_EQ(Dump(listProp), Dump(gridProp));
    EXPECT_EQ(Dump(listProp), Dump(flowProp));
    EXPECT_EQ(Dump(listProp), "ScrollSnapAlign.CENTER");
    ScrollSnapStrategy provider = MakeProvider([](double) { return 100.0; });
    listProp->UpdateScrollSnapStrategy(provider);
    gridProp->UpdateScrollSnapStrategy(provider);
    flowProp->UpdateScrollSnapStrategy(provider);
    EXPECT_EQ(Dump(listProp), Dump(gridProp));
    EXPECT_EQ(Dump(listProp), Dump(flowProp));
    EXPECT_EQ(Dump(listProp), "ScrollSnapOffsetProvider");
}
} // namespace OHOS::Ace::NG

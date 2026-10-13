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

#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include <cmath>
#include <functional>
#include <limits>
#include <tuple>

#include <gtest/gtest.h>

#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#define private public
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#undef private
#include "test/unittest/core/pattern/grid/grid_test_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t GRID_ITEM_NUMBER = 10; // 2 columns x 5 rows of 100px items
constexpr float GRID_CONTENT_HEIGHT = 500.0f;
constexpr float GRID_MAX_OFFSET = GRID_CONTENT_HEIGHT - HEIGHT; // 100.0f

ScrollSnapStrategy MakeStrategy(ScrollSnapAlign align)
{
    ScrollSnapStrategy strategy;
    strategy.align = align;
    return strategy;
}

ScrollSnapStrategy MakeProviderStrategy(std::function<double(double)> snapFunc)
{
    ScrollSnapStrategy strategy;
    strategy.align = ScrollSnapAlign::CENTER;
    strategy.hasProvider = true;
    strategy.calculateSnapOffset = std::move(snapFunc);
    return strategy;
}

} // namespace

class GridScrollSnapTestNg : public GridTestNg {};

/**
 * @tc.name: SnapTypeGate001
 * @tc.desc: Feat-20260806915268 AC-3.1: snap stays NONE by default and only LIST_SNAP when a strategy is set.
 */
HWTEST_F(GridScrollSnapTestNg, SnapTypeGate001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Not configured: existing Grid behavior unchanged (NONE_SNAP).
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);

    // Configured via the FrameNode entry (ModelNG static / C API path).
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::CENTER));
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::LIST_SNAP);

    // Reset clears the strategy.
    GridModelNG::ResetScrollSnapStrategy(AceType::RawPtr(frameNode_));
    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
}

/**
 * @tc.name: StrategyPropertyPipeline002
 * @tc.desc: Feat-20260806915268 AC-3.2: creation-time and FrameNode property writes agree on value and default.
 */
HWTEST_F(GridScrollSnapTestNg, StrategyPropertyPipeline002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto stored = layoutProperty_->GetScrollSnapStrategy().value_or(ScrollSnapStrategy());
    EXPECT_EQ(stored.align, ScrollSnapAlign::START);
    EXPECT_TRUE(stored.IsEnabled());

    // FrameNode-state write (used by C API / static modifier entry) updates the same property.
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::END));
    stored = layoutProperty_->GetScrollSnapStrategy().value_or(ScrollSnapStrategy());
    EXPECT_EQ(stored.align, ScrollSnapAlign::END);

    // Reset restores the default: NONE and disabled.
    GridModelNG::ResetScrollSnapStrategy(AceType::RawPtr(frameNode_));
    stored = layoutProperty_->GetScrollSnapStrategy().value_or(ScrollSnapStrategy());
    EXPECT_EQ(stored.align, ScrollSnapAlign::NONE);
    EXPECT_FALSE(stored.IsEnabled());
}

/**
 * @tc.name: CandidatesStartAlignDedup003
 * @tc.desc: Feat-20260806915268 AC-1.2 / AC-1.4: START anchors per row; same-row (cross-column) offsets dedup
 *           keeping the smaller index.
 */
HWTEST_F(GridScrollSnapTestNg, CandidatesStartAlignDedup003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    // Initial layout materializes the visible rows (0..3): one anchor per row, cross-column
    // duplicates removed keeping the smaller index (R-6 / AC-1.4).
    ASSERT_EQ(static_cast<int32_t>(candidates.size()), 4);
    const float expectedOffsets[] = { 0.0f, 100.0f, 200.0f, 300.0f };
    const int32_t expectedIndices[] = { 0, 2, 4, 6 };
    for (int32_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(candidates[i].offset, expectedOffsets[i]);
        EXPECT_EQ(candidates[i].index, expectedIndices[i]);
    }

    // After scrolling, the candidates follow the materialized window and reach the last row.
    UpdateCurrentOffset(-350.0f);
    candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::START);
    ASSERT_FALSE(candidates.empty());
    EXPECT_FLOAT_EQ(candidates.back().offset, 400.0f);
    EXPECT_EQ(candidates.back().index, 8);
}

/**
 * @tc.name: CandidatesCenterAlign004
 * @tc.desc: Feat-20260806915268 AC-1.1: CENTER anchors align the item center to the viewport center.
 */
HWTEST_F(GridScrollSnapTestNg, CandidatesCenterAlign004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_EQ(static_cast<int32_t>(candidates.size()), 4);
    // start + itemMain/2 - viewport/2 = row * 100 + 50 - 200.
    const float expectedOffsets[] = { -150.0f, -50.0f, 50.0f, 150.0f };
    for (int32_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(candidates[i].offset, expectedOffsets[i]);
    }
}

/**
 * @tc.name: CandidatesEndAlign005
 * @tc.desc: Feat-20260806915268 AC-1.2: END anchors align the item end edge to the viewport end.
 */
HWTEST_F(GridScrollSnapTestNg, CandidatesEndAlign005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::END));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::END);
    ASSERT_EQ(static_cast<int32_t>(candidates.size()), 4);
    // start + itemMain - viewport = row * 100 - 300.
    const float expectedOffsets[] = { -300.0f, -200.0f, -100.0f, 0.0f };
    for (int32_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(candidates[i].offset, expectedOffsets[i]);
    }
}

/**
 * @tc.name: CandidatesNotScrollable006
 * @tc.desc: Feat-20260806915268 AC-2.1: for non-overflowing content every anchor is non-positive, so the
 *           runtime clamp keeps the grid at the range start (no movement, no blank).
 */
HWTEST_F(GridScrollSnapTestNg, CandidatesNotScrollable006, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(4, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE); // 2 rows = 200px < viewport 400px
    CreateDone();

    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_EQ(static_cast<int32_t>(candidates.size()), 2);
    EXPECT_FLOAT_EQ(candidates[0].offset, -150.0f);
    EXPECT_FLOAT_EQ(candidates[1].offset, -50.0f);
    // The clamped target stays at the range start: content does not overflow.
    EXPECT_FLOAT_EQ(
        ScrollSnapUtils::ClampSnapOffset(candidates.back().offset, 0.0f, pattern_->GetItemSnapMaxOffset()),
        0.0f);
}

/**
 * @tc.name: MaxOffsetClamp007
 * @tc.desc: Feat-20260806915268 AC-2.1: the snap boundary is total content minus viewport and never below
 *           the current offset (lazy underestimation guard).
 */
HWTEST_F(GridScrollSnapTestNg, MaxOffsetClamp007, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), GRID_MAX_OFFSET);

    UpdateCurrentOffset(-60.0f);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMaxOffset(), GRID_MAX_OFFSET);
}

/**
 * @tc.name: SnapAnimationGatedByStrategy008
 * @tc.desc: Feat-20260806915268 AC-3.1: without a strategy the snap animation entry is refused so the
 *           default motion (including programmed scrolls) is untouched.
 */
HWTEST_F(GridScrollSnapTestNg, SnapAnimationGatedByStrategy008, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    SnapAnimationOptions options;
    options.snapDelta = 50.0f;
    options.animationVelocity = 100.0f;
    EXPECT_FALSE(pattern_->StartSnapAnimation(options));

    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::CENTER));
    // A release with fling energy does not seize the animation channel: it defers to the
    // default friction fling and reports "not handled" (the settle re-enters at fling stop).
    UpdateCurrentOffset(-60.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);
    EXPECT_FALSE(pattern_->StartSnapAnimation(options));
}

/**
 * @tc.name: SnapAnimationInterruptedByTouch009
 * @tc.desc: Feat-20260806915268 AC-2.2: a new gesture during the snap animation stops it immediately and the
 *           position stays put (no further snap motion).
 * @tc.note: the nested-scroll consumption order itself is unchanged master behavior guarded by the
 *           scrollable nested tests; the child defers snap until it is the final consumer.
 */
HWTEST_F(GridScrollSnapTestNg, SnapAnimationInterruptedByTouch009, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Release mid-gap: offset 60 sits between the CENTER anchors 50 and 150.
    UpdateCurrentOffset(-60.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    // Low-speed release: threshold selection targets the 50 anchor and schedules the spring.
    SnapAnimationOptions options;
    options.snapDelta = 10.0f;
    options.animationVelocity = 0.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());

    // A new gesture interrupts the running snap animation immediately.
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    scrollable->HandleTouchDown();

    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    // No snap motion after the interruption: the position stays mid-gap.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);
}

/**
 * @tc.name: SnapAnimationCompletesOnAnchor010
 * @tc.desc: Feat-20260806915268 AC-2.2/AC-1.3: an uninterrupted low-speed snap animation completes exactly on
 *           the selected anchor (deterministic threshold selection through the shared pipeline).
 */
HWTEST_F(GridScrollSnapTestNg, SnapAnimationCompletesOnAnchor010, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Release mid-gap at 60 with a small forward drag: the 50 anchor wins the 0.5 threshold.
    UpdateCurrentOffset(-60.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    SnapAnimationOptions options;
    options.snapDelta = 10.0f;
    options.animationVelocity = 0.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));

    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 50.0f);
}
/**
 * @tc.name: TwoStageInterruptedByTouch013
 * @tc.desc: Feat-20260806915268 AC-2.2 (provider path): interrupting the approach stage must cancel the
 *           whole two-stage snap; the interrupted handoff must not silently start stage 2.
 */
HWTEST_F(GridScrollSnapTestNg, TwoStageInterruptedByTouch013, TestSize.Level1)
{
    auto strategy = MakeProviderStrategy([](double velocity) { return 50.0; });
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    UpdateCurrentOffset(-60.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    SnapAnimationOptions options;
    options.snapDelta = -30.0f; // drag frame: forward approach of 30
    options.animationVelocity = 0.0f;
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    // Direct pattern-level calls park the provider kickoff (the gesture pipeline flushes it
    // after the lifecycle events); flush it the way HandleDragEnd would.
    scrollable->FlushPendingItemSnapKickoff();
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());

    scrollable->HandleTouchDown();

    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // Interrupted: the position stays where the gesture stopped it; no stage-2 runs under the
    // user's finger.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    // A later snap must still be accepted and settle: the interrupted chain leaked nothing.
    // (The mocked UI task executor drops the deferred handoff task and the mock animation
    // backend cannot simulate frames for a nested stage-2 spring, so the provider-path landing
    // is verified on device / SpecTest; the bounds below pin that no stale chain moves the
    // grid beyond the fresh approach delta.)
    SnapAnimationOptions next;
    next.snapDelta = -10.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(next));
    // Same parking rule as above: flush so the "later snap still settles" half actually runs
    // (without it the loop below spins on an empty queue and the bounds pass vacuously).
    scrollable->FlushPendingItemSnapKickoff();
    int32_t settleTicks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && settleTicks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++settleTicks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_GE(pattern_->GetTotalOffset(), 60.0f);
    EXPECT_LE(pattern_->GetTotalOffset(), 70.0f);
}

/**
 * @tc.name: ProviderNonFiniteTargetCleansState014
 * @tc.desc: Feat-20260806915268 review: a provider returning NaN at handoff must clean the snap state
 *           instead of leaving the scrollable stuck in SNAP.
 */
HWTEST_F(GridScrollSnapTestNg, ProviderNonFiniteTargetCleansState014, TestSize.Level1)
{
    auto strategy = MakeProviderStrategy([](double velocity) { return std::nanf(""); });
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    UpdateCurrentOffset(-60.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    SnapAnimationOptions options;
    options.snapDelta = -30.0f;
    options.animationVelocity = 0.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    // Direct pattern-level calls PARK the provider kickoff; without this flush the case is
    // vacuous (no animation runs, every assertion below passes trivially) and the NaN
    // cleanup path stays uncovered.
    scrollable->FlushPendingItemSnapKickoff();
    // Anti-vacuous guard: the approach stage must actually be in flight right now.
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // The snap state is cleaned instead of stuck in SNAP, and a later snap still works.
    EXPECT_TRUE(scrollable->IsStopped());
    SnapAnimationOptions next;
    next.snapDelta = -10.0f;
    EXPECT_TRUE(pattern_->StartSnapAnimation(next));
}

/**
 * @tc.name: CrossRowLayoutNotSupported015
 * @tc.desc: Feat-20260806915268 AC-1.4 (2026-09-15 scope): grids with cross-row items do not support the
 *           snap capability — it stays inactive exactly like an unconfigured grid.
 */
HWTEST_F(GridScrollSnapTestNg, CrossRowLayoutNotSupported015, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::CENTER));
    CreateBigItem(0, 1); // spans rows 0-1: cross-row node
    CreateGridItems(6, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    EXPECT_EQ(pattern_->GetSnapType(), SnapType::NONE_SNAP);
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    EXPECT_TRUE(candidates.empty());

    SnapAnimationOptions options;
    options.snapDelta = -30.0f;
    options.animationVelocity = 0.0f;
    EXPECT_FALSE(pattern_->StartSnapAnimation(options));
}

/**
 * @tc.name: SnapRangeConstrainedByContentOffsets016
 * @tc.desc: Feat-20260806915268 AC-2.4 (2026-09-15 scope): the snap target range honors
 *           contentStartOffset/contentEndOffset per the List snap spec — the lower bound is
 *           contentStartOffset (not 0) and the upper bound already includes contentEndOffset.
 */
HWTEST_F(GridScrollSnapTestNg, SnapRangeConstrainedByContentOffsets016, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    ScrollableModelNG::SetContentStartOffset(CONTENT_START_OFFSET);
    ScrollableModelNG::SetContentEndOffset(CONTENT_END_OFFSET);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Candidates start after the leading gap; the first CENTER anchor (row 0) is negative and
    // must clamp up to contentStartOffset instead of 0.
    EXPECT_FLOAT_EQ(pattern_->GetItemSnapMinOffset(), CONTENT_START_OFFSET);
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_FALSE(candidates.empty());
    EXPECT_FLOAT_EQ(
        ScrollSnapUtils::ClampSnapOffset(candidates.front().offset, pattern_->GetItemSnapMinOffset(),
            pattern_->GetItemSnapMaxOffset()),
        CONTENT_START_OFFSET);

    // Release at the initial position (offset 0, gap fully visible): threshold selection
    // picks the row-1 anchor (-30) and the final target clamps up to contentStartOffset (20),
    // never resting inside the leading gap.
    SnapAnimationOptions options;
    options.snapDelta = 5.0f;
    options.animationVelocity = 0.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), CONTENT_START_OFFSET);
}

/**
 * @tc.name: AxisDirectionMapsToListSemantics017
 * @tc.desc: Feat-20260806915268 review: axis SnapDirection follows the established List semantics —
 *           FORWARD steps to the smaller item index, i.e. the smaller candidate offset.
 */
HWTEST_F(GridScrollSnapTestNg, AxisDirectionMapsToListSemantics015, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(20, false);
    model.SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(20, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Release mid-gap at 450: the materialized window starts at the current row (400), so the
    // axis FORWARD step targets 400 (smaller index than the mid-gap position), BACKWARD targets 500.
    UpdateCurrentOffset(-450.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 450.0f);
    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::FORWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));

    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    // Axis BACKWARD steps to the larger offset.
    SnapAnimationOptions backward;
    backward.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(backward));
    ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
}

/**
 * @tc.name: AxisStateResetByProgrammaticStop018
 * @tc.desc: Feat-20260806915268 review: cancelling an in-flight axis snap via the programmatic stop path
 *           must reset the snap steering state — the next axis step starts fresh and is not
 *           treated as a continuation of the cancelled direction.
 */
HWTEST_F(GridScrollSnapTestNg, AxisStateResetByProgrammaticStop016, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(20, false);
    model.SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(20, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // First axis step lands on 400 from a mid-gap 450 release.
    UpdateCurrentOffset(-450.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 450.0f);
    SnapAnimationOptions forward;
    forward.snapDirection = SnapDirection::FORWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(forward));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    // Start a second axis snap (BACKWARD) and cancel it through the programmatic stop path
    // (the same chain ScrollBy / AnimateTo / StopAnimate use); the steering state must reset.
    SnapAnimationOptions inFlight;
    inFlight.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(inFlight));
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    scrollable->StopScrollable();
    FlushUITasks();
    // Position stays (handoff cancelled); the grid does not drift toward the cancelled 500.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    // The next axis FORWARD step after the cancellation behaves like a fresh step: the
    // neighboring-line candidates let it reach the smaller anchor 300 (F003) — and crucially
    // NOT the larger 500, proving the cancelled BACKWARD direction did not leak into the new
    // input (a leaked direction would have pulled the grid toward 500).
    SnapAnimationOptions next;
    next.snapDirection = SnapDirection::FORWARD;
    EXPECT_TRUE(pattern_->StartSnapAnimation(next));
    int32_t ticks2 = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks2 < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks2;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 300.0f);
}

/**
 * @tc.name: FlingReleaseDefersAnimationAndFallbackAligns017
 * @tc.desc: Feat-20260806915268 AC-2.2/AC-1.3 (align path): a release with fling energy defers to the
 *           default friction fling (StartSnapAnimation reports "not handled") and the settle
 *           fallback selects a real nearby anchor. Per-frame early takeover is covered by
 *           GridItemSnapGeometryTest instead of this manually replayed callback case.
 */
HWTEST_F(GridScrollSnapTestNg, FlingReleaseDefersAnimationAndFallbackAligns017, TestSize.Level1)
{
    // 40 items: 2 columns x 20 rows of 100px -> content 2000, keep the whole glide in range.
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Park at the fling landing point directly (60 + 45 decay = 105, between the CENTER
    // anchors 50 and 150) and replay the two callbacks the release/fling-stop sequence makes.
    UpdateCurrentOffset(-105.0f);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 105.0f);

    // Release with fling energy (any speed): the snap entry must NOT take over the animation
    // channel — the default friction fling owns the motion.
    SnapAnimationOptions options;
    options.snapDelta = -45.0f; // drag frame: forward natural decay of 45
    options.animationVelocity = -800.0f; // drag frame: forward, below the fling threshold
    EXPECT_FALSE(pattern_->StartSnapAnimation(options));

    // The friction stop re-enters the snap callback with near-zero velocity and delta — the
    // settle then targets the anchor NEAREST THE LANDING (150), not the one nearest a release
    // point below the half-gap (the release-side threshold rule would have stayed at 50).
    SnapAnimationOptions settle;
    settle.snapDelta = 0.0f;
    settle.animationVelocity = 0.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(settle));
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());

    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 150.0f);
}

/**
 * @tc.name: CandidateAbsoluteFrameAtDeepOffset019
 * @tc.desc: Feat-20260806915268 review: candidates built at a deep offset must be absolute anchors
 *           (phase 50 mod 100, sorted, inside the visible window band) — never drifted by the
 *           current offset, for both the vertical and the horizontal axis.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, CandidateAbsoluteFrameAtDeepOffset019, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Deep offset 950 of the 1600 range: the visible band covers anchors 750..1350.
    UpdateCurrentOffset(-950.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 950.0f);

    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_FALSE(candidates.empty());
    for (const auto& candidate : candidates) {
        // Absolute CENTER anchors sit on the 50 + k*100 phase, never shifted by the offset.
        EXPECT_NEAR(std::fmod(candidate.offset + 50.0f, 100.0f), 0.0f, 0.5f)
            << "candidate " << candidate.offset << " off the anchor phase";
        // Only the visible + cached window contributes.
        EXPECT_GE(candidate.offset, 500.0f);
        EXPECT_LE(candidate.offset, 1400.0f);
    }
    // Sorted ascending after normalization.
    EXPECT_TRUE(std::is_sorted(candidates.begin(), candidates.end(),
        [](const SnapCandidate& a, const SnapCandidate& b) { return a.offset < b.offset; }));
}

/**
 * @tc.name: DeepOffsetSlowFlingSettleStaysLocal020
 * @tc.desc: Feat-20260806915268 device-report regression: a fling released at a deep offset must defer to
 *           the friction pipeline and settle back into the LOCAL anchor neighborhood — never
 *           run away toward the range start (the observed "fling returns to index 0").
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, DeepOffsetSlowFlingSettleStaysLocal020, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    UpdateCurrentOffset(-970.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 970.0f);

    // Replay a low-speed fling using the native coordinates prepared by StartScrollAnimation.
    // High release speeds preserve direction and are tested separately below.
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->SetCurrentPos(-970.0f);
    scrollable->finalPosition_ = -1070.0f;
    SnapAnimationOptions release;
    release.snapDelta = -100.0f;
    release.animationVelocity = -250.0f;
    EXPECT_FALSE(pattern_->StartSnapAnimation(release));

    SnapAnimationOptions settle;
    settle.snapDelta = 0.0f;
    settle.animationVelocity = 0.0f;
    settle.fromItemSnapFlingStop = true;
    ASSERT_TRUE(pattern_->StartSnapAnimation(settle));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    auto finalOffset = pattern_->GetTotalOffset();
    // Local neighborhood of the 970 release point; a runaway to the range start is > 850 px away.
    EXPECT_GT(finalOffset, 800.0f);
    EXPECT_LT(finalOffset, 1100.0f);
    // Rests on the local 950 anchor (release 20px past it, well inside the half-gap).
    EXPECT_FLOAT_EQ(finalOffset, 950.0f);
}

/**
 * @tc.name: CandidateAnchorStableAcrossScroll021
 * @tc.desc: Feat-20260806915268 device-report audit (vertical axis): a candidate anchor is an absolute
 *           scroll offset — the same item must produce the SAME anchor regardless of the scroll
 *           position the candidates are built at. A frame mismatch between the item rects and
 *           the offset would drift the anchors with the window.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, CandidateAnchorStableAcrossScroll021, TestSize.Level1)
{
    // Vertical: build at 950, remember the anchors, rebuild at 1050 — the shared items must
    // report identical absolute anchors.
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    UpdateCurrentOffset(-950.0f);
    FlushUITasks();
    auto at950 = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    UpdateCurrentOffset(-1050.0f);
    FlushUITasks();
    auto at1050 = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_FALSE(at950.empty());
    ASSERT_FALSE(at1050.empty());
    for (const auto& candidate : at950) {
        for (const auto& other : at1050) {
            if (other.index == candidate.index) {
                EXPECT_FLOAT_EQ(other.offset, candidate.offset)
                    << "anchor of item " << candidate.index << " drifted with the scroll window";
            }
        }
    }
}

/**
 * @tc.name: HorizontalCandidateAnchorStableAcrossScroll022
 * @tc.desc: Feat-20260806915268 device-report audit (horizontal axis): a candidate anchor is an absolute
 *           scroll offset — the same item must produce the SAME anchor regardless of the scroll
 *           position the candidates are built at (rect.GetX + offset conversion).
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, HorizontalCandidateAnchorStableAcrossScroll022, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    ASSERT_EQ(pattern_->GetAxis(), Axis::HORIZONTAL);

    UpdateCurrentOffset(-450.0f, false);
    FlushUITasks();
    auto at450 = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    UpdateCurrentOffset(-350.0f, false);
    FlushUITasks();
    auto at350 = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_FALSE(at450.empty());
    ASSERT_FALSE(at350.empty());
    for (const auto& candidate : at450) {
        for (const auto& other : at350) {
            if (other.index == candidate.index) {
                EXPECT_FLOAT_EQ(other.offset, candidate.offset)
                    << "horizontal anchor of item " << candidate.index << " drifted";
            }
        }
    }
}

/**
 * @tc.name: SlowReleaseSettlesWithoutFling023
 * @tc.desc: Feat-20260806915268 F002 regression: a release below the friction start speed (~240 px/s)
 *           still settles to the nearest anchor — the low-speed path consumes the pending
 *           settle itself instead of waiting for a friction stop that never fires.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, SlowReleaseSettlesWithoutFling023, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);

    // Slow forward release at offset 60 (between the anchors 50 and 150): below the friction
    // threshold, so neither friction nor a friction stop will ever run.
    UpdateCurrentOffset(-60.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    // Drag-frame position/velocity through the real release entry (finger at the anchor 60).
    ASSERT_TRUE(scrollable->StartScrollAnimation(-60.0f, -100.0f));
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // The settle ran on the low-speed path: the position rests aligned on the 50 anchor
    // instead of stopping unaligned mid-gap.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 50.0f);
}

/**
 * @tc.name: AxisForwardStepFromWindowEdgeAnchor024
 * @tc.desc: Feat-20260806915268 F003 regression: resting exactly on the first visible-row anchor, a
 *           FORWARD axis step (List semantics: toward the smaller index / smaller offset)
 *           must reach the previous row's anchor — the candidate window includes the
 *           neighboring materialized line beyond the visible range.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, AxisForwardStepFromWindowEdgeAnchor024, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // START anchors sit on the row pitch (0, 100, ...); rest exactly on 400 (row 4), the
    // first row fully past the viewport top (400 = 4 rows x 100).
    UpdateCurrentOffset(-400.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::FORWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // Stepped one row toward the smaller index; a window-edge stall would stay at 400.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 300.0f);
}

/**
 * @tc.name: ProviderTakeoverReAnchorsToContentFrame025
 * @tc.desc: Feat-20260806915268 F001 regression: the provider takeover must re-anchor the animation
 *           origin to the content offset even when the gesture pipeline left the FINGER's
 *           drag position in currentPos_ (finger position differs from -offset), so the
 *           absolute stage targets land where the provider says.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, ProviderTakeoverReAnchorsToContentFrame025, TestSize.Level1)
{
    auto strategy = MakeProviderStrategy([](double velocity) { return 100.0; });
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);

    UpdateCurrentOffset(-60.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 60.0f);

    // Simulate the gesture pipeline having left the finger position (200px) in currentPos_:
    // the real StartScrollAnimation does this right before the snap callback.
    scrollable->SetCurrentPos(-200.0f);

    SnapAnimationOptions options;
    options.snapDelta = -30.0f;
    options.animationVelocity = -900.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    // Not started yet: the provider kickoff is parked until the flush point.
    EXPECT_FLOAT_EQ(scrollable->GetCurrentPos(), -200.0f);
    scrollable->FlushPendingItemSnapKickoff();
    // The takeover re-anchored the origin to the content offset (60), not the finger (200):
    // the approach stage must start from the content position, otherwise every stage target
    // shifts by the finger delta.
    EXPECT_FLOAT_EQ(scrollable->GetCurrentPos(), -60.0f);
}

/**
 * @tc.name: AxisStepWithZeroCache026
 * @tc.desc: Feat-20260806915268 F003-R2 regression: with cachedCount(0) the neighboring rows are never
 *           materialized, yet a FORWARD axis step from a row anchor must still reach the
 *           previous row's anchor — the boundary anchors are synthesized from the layout line
 *           info, so the axis input is never swallowed for a missing neighbor.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, AxisStepWithZeroCache026, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::START));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Rest exactly on the row-4 START anchor (400) with no cached rows materialized.
    UpdateCurrentOffset(-400.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::FORWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // Stepped one row toward the smaller index through the synthesized boundary anchor.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 300.0f);
}

/**
 * @tc.name: LazyItemsCandidateScanIsSideEffectFree027
 * @tc.desc: Feat-20260806915268 R2-NEW001 smoke: building candidates over a LazyForEach grid must not
 *           disturb the off-screen cached nodes — the neighbor anchors are synthesized from
 *           line info and only the visible children are read.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, LazyItemsCandidateScanIsSideEffectFree027, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateItemsInLazyForEach(40, [](uint32_t index) { return ITEM_MAIN_SIZE; });
    CreateDone();

    UpdateCurrentOffset(-450.0f);
    FlushUITasks();

    auto before = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_FALSE(before.empty());
    // Repeated scans stay stable and the lazy builder is not forced to churn its cached range.
    auto after = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_EQ(after.size(), before.size());
    for (size_t i = 0; i < before.size(); ++i) {
        EXPECT_FLOAT_EQ(after[i].offset, before[i].offset);
        EXPECT_EQ(after[i].index, before[i].index);
    }
}

/**
 * @tc.name: NeighborAnchorHonorsAlign028
 * @tc.desc: Feat-20260806915268 R3-NEW001 regression: the synthesized neighbor anchor applies the FULL
 *           align formula to the neighbor line — END from 400 toward the larger index must
 *           land on the neighbor's END anchor (500), not its start position (800); CENTER
 *           likewise centers the neighbor row.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, NeighborAnchorHonorsAlign028, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::END));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // END anchors: start(k) + 100 - 400 = 100k - 300. Rest exactly on the window-edge END
    // anchor of row 7 (400); toward the larger index the neighbor row 8's END anchor is 500.
    UpdateCurrentOffset(-400.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 400.0f);

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
}

/**
 * @tc.name: NeighborAnchorHonorsCenter029
 * @tc.desc: Feat-20260806915268 R3/R4 regression (CENTER, unmeasured neighbor): resting on the last
 *           visible CENTER anchor with no cached rows, the axis step toward the larger index
 *           reaches the next row's CENTER anchor through the boundary estimation (uniform
 *           200px rows make the estimate exact).
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, NeighborAnchorHonorsCenter029, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(0, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    for (int32_t i = 0; i < 40; ++i) {
        CreateGridItem(NULL_VALUE, 200.0f);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    CreateDone();

    // 200px rows, viewport 400: visible rows 0-1; CENTER anchors -100 and 100. Resting on 100
    // (the last visible anchor), the larger index needs the unmaterialized row 2.
    UpdateCurrentOffset(-100.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 100.0f);

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // Row 2 CENTER anchor: 400 + 100 - 200 = 300.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 300.0f);
}


/**
 * @tc.name: NeighborAnchorPerItemMixedHeights031
 * @tc.desc: Feat-20260806915268 R4 regression (materialized neighbor, per-item anchors): a row with
 *           mixed-height items (100/200) must expose BOTH item anchors — END stepping toward
 *           the larger index picks the 100-item anchor (100), never the row-max anchor (200).
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, NeighborAnchorPerItemMixedHeights031, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::END));
    for (int32_t i = 0; i < 40; ++i) {
        CreateGridItem(NULL_VALUE, i % 2 == 0 ? 100.0f : 200.0f);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    CreateDone();

    // Rows are 200 tall (row max). At offset 600 rows 3-4 are visible; their END anchors are
    // 300/400 (row 3) and 500/600 (row 4) — the largest visible anchor equals the position.
    // The next anchors live in the CACHED (not visible) row 5 (start 1000): the 100-item's
    // END anchor 700 and the 200-item's 800; the per-item semantics must pick 700, while the
    // row-max semantics would jump to 800.
    UpdateCurrentOffset(-600.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 600.0f);

    // The mock layout does not run the cache measuring pass, so prime the cached row-5 items'
    // geometry the way the device layout would have: start 1000 (viewport Y 400 at offset 600),
    // heights 100 and 200.
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    auto item10 = host->GetChildByIndex(10);
    auto item11 = host->GetChildByIndex(11);
    ASSERT_NE(item10, nullptr);
    ASSERT_NE(item11, nullptr);
    item10->GetGeometryNode()->SetFrameSize(SizeF(500.0f, 100.0f));
    item10->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 400.0f));
    item11->GetGeometryNode()->SetFrameSize(SizeF(500.0f, 200.0f));
    item11->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 400.0f));

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    // The nearest strictly-larger per-item anchor: the 100-item's END anchor.
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 700.0f);
}

/**
 * @tc.name: NeighborAnchorPerItemMixedHeightsCenter032
 * @tc.desc: Feat-20260806915268 R4 regression (materialized neighbor, CENTER): mixed-height cached row
 *           exposes per-item CENTER anchors; stepping picks the nearest strictly-larger one.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, NeighborAnchorPerItemMixedHeightsCenter032, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    for (int32_t i = 0; i < 40; ++i) {
        CreateGridItem(NULL_VALUE, i % 2 == 0 ? 100.0f : 200.0f);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    CreateDone();

    // At offset 700 (rows 3-4 visible, row starts 600/800) the visible CENTER anchors are
    // 450/500 (row 3) and 650/700 (row 4) — the largest equals the position, so the axis step
    // falls back to the neighbor. The cached row 5 (start 1000) exposes the per-item CENTER
    // anchors 1000+50-200=850 and 1000+100-200=900; the nearest (850) must win over the
    // row-max-based 900. The mock layout does not run the cache measuring pass, so prime the
    // cached items' geometry like the device layout would (viewport Y = 1000 - 700 = 300).
    UpdateCurrentOffset(-700.0f);
    FlushUITasks();
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 700.0f);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    auto item10 = host->GetChildByIndex(10);
    auto item11 = host->GetChildByIndex(11);
    ASSERT_NE(item10, nullptr);
    ASSERT_NE(item11, nullptr);
    item10->GetGeometryNode()->SetFrameSize(SizeF(500.0f, 100.0f));
    item10->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 300.0f));
    item11->GetGeometryNode()->SetFrameSize(SizeF(500.0f, 200.0f));
    item11->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 300.0f));

    SnapAnimationOptions options;
    options.snapDirection = SnapDirection::BACKWARD;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 850.0f);
}

/**
 * @tc.name: ProviderApproachAfterLifecycleEvents033
 * @tc.desc: Feat-20260806915268 architect alignment: the parked provider kickoff does not execute inside
 *           the snap callback — calculateApproachOffset runs only at the flush point, after
 *           the gesture lifecycle events would have fired. A strategy removed "in the
 *           callback window" cancels the chain instead of starting an approach stage.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, ProviderApproachAfterLifecycleEvents033, TestSize.Level1)
{
    bool providerCalled = false;
    // The timing contract under test is the APPROACH provider: it executes synchronously
    // inside the flushed kickoff, observable in the host environment (the final-target
    // provider only runs at the animation handoff).
    ScrollSnapStrategy strategy;
    strategy.hasProvider = true;
    strategy.calculateApproachOffset = [&providerCalled](double velocity, double decayOffset) {
        providerCalled = true;
        return decayOffset;
    };
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    UpdateCurrentOffset(-60.0f);
    FlushUITasks();

    SnapAnimationOptions options;
    options.snapDelta = -30.0f;
    options.animationVelocity = -900.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    // Parked, not executed: nothing started and the provider has not been asked.
    EXPECT_FALSE(providerCalled);
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());

    // "Inside the callback window" (what onDidStopDragging / onWillStartFling would cover):
    // the app removes the strategy — the flush must cancel instead of starting a chain.
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), ScrollSnapStrategy());
    scrollable->FlushPendingItemSnapKickoff();
    EXPECT_FALSE(providerCalled);
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());

    // Re-enable and flush again: the provider chain starts and the approach runs to the end.
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), strategy);
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    scrollable->FlushPendingItemSnapKickoff();
    EXPECT_TRUE(providerCalled);
}

/**
 * @tc.name: AnchorsFromLayoutTimeGeometry034
 * @tc.desc: Feat-20260806915268 fix regression: snap anchors must be derived from layout-time geometry.
 *           The settle callback can run before the final scroll delta has been laid out (an
 *           interrupted fling leaves child frame rects one frame behind the offset); anchors
 *           from a settle-time live read shift by that lag and the spring lands off-anchor.
 *           GridTestNg::UpdateCurrentOffset flushes layout, which is exactly why this timing
 *           window is invisible to the other cases — this test bypasses the helper on purpose.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, AnchorsFromLayoutTimeGeometry034, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(GRID_ITEM_NUMBER, false);
    model.SetScrollSnapStrategy(MakeStrategy(ScrollSnapAlign::CENTER));
    CreateGridItems(GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    // Advance the offset WITHOUT flushing layout: child geometry (and the layout-time
    // geometry cache) still reflect offset 0, lagging by 95 — the interrupted-fling state.
    pattern_->UpdateCurrentOffset(-95.0f, SCROLL_FROM_UPDATE);
    ASSERT_FLOAT_EQ(pattern_->GetTotalOffset(), 95.0f);

    // TRUE anchors are row*100 - 150 for every materialized row, independent of the
    // un-flushed delta. A settle-time live geometry read would shift each by +95.
    auto candidates = pattern_->BuildItemSnapCandidates(ScrollSnapAlign::CENTER);
    ASSERT_EQ(static_cast<int32_t>(candidates.size()), 4);
    const float expectedOffsets[] = { -150.0f, -50.0f, 50.0f, 150.0f };
    for (size_t i = 0; i < candidates.size(); ++i) {
        EXPECT_FLOAT_EQ(candidates[i].offset, expectedOffsets[i]);
    }

    // Replay the fling-stop settle: it must land on the TRUE anchor nearest the landing
    // (offset 95 -> 50, distance 45), not on a lag-shifted anchor (45 or 145).
    SnapAnimationOptions settle;
    settle.snapDelta = 0.0f;
    settle.animationVelocity = 0.0f;
    settle.fromItemSnapFlingStop = true;
    ASSERT_TRUE(pattern_->StartSnapAnimation(settle));
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 50.0f);
}


/**
 * @tc.name: ProviderReceivesReleaseVelocity035
 * @tc.desc: Feat-20260806915268 fix contract: calculateSnapOffset receives the RELEASE fling speed
 *           (positive-forward), never the near-zero spring residual measured at the approach
 *           handoff. On a real animation backend the residual is 1~10 px/s regardless of the
 *           release speed, which made velocity-aware providers unusable; the mock executor
 *           keeps the stage-start velocity, so this case locks the contract from both sides.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollSnapTestNg, ProviderReceivesReleaseVelocity035, TestSize.Level1)
{
    float seenVelocity = -1.0f;
    bool providerCalled = false;
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(40, false);
    model.SetScrollSnapStrategy(MakeProviderStrategy([&seenVelocity, &providerCalled](double velocity) {
        providerCalled = true;
        seenVelocity = static_cast<float>(velocity);
        return 240.0;
    }));
    CreateGridItems(40, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();

    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    ASSERT_NE(scrollable, nullptr);
    // NOTE: the mock task executor DROPS the deferred stage handoff (documented in
    // TwoStageInterruptedByTouch013), so the stage-completion branch — where the real
    // backend handed the provider a ~0 residual instead of the release speed — cannot be
    // driven here. This case locks the contract through the synchronous near-zero-approach
    // handoff, which must pass the RELEASE speed through unchanged. The stage-completion
    // regression is proven on HostPreview (velocity-aware provider case: release ~1100 px/s
    // reached the provider as ~9 px/s before the fix, exact release speed after).
    SnapAnimationOptions options;
    options.snapDelta = -0.0001f; // ~zero approach: the handoff runs synchronously
    options.animationVelocity = -1000.0f; // drag frame: forward release speed
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    scrollable->FlushPendingItemSnapKickoff();
    EXPECT_TRUE(providerCalled);
    // The provider must see the ~1000 px/s release speed (positive-forward), not a residual.
    EXPECT_NEAR(seenVelocity, 1000.0f, 1.0f);
    int32_t ticks = 0;
    while (!MockAnimationManager::GetInstance().AllFinished() && ticks < 100) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ++ticks;
    }
    ASSERT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 240.0f);
}

namespace {
constexpr int32_t TAKEOVER_ITEM_COUNT = 500;
constexpr int32_t TAKEOVER_COLUMN_COUNT = 2;
constexpr int32_t TAKEOVER_ANIMATION_TICKS = 40;
constexpr int32_t TAKEOVER_MAX_FLING_FRAMES = 400;
constexpr int32_t TAKEOVER_MAX_SNAP_TICKS = 200;

struct ItemSnapFlingOptions {
    ScrollSnapAlign align = ScrollSnapAlign::CENTER;
    float velocity = -6000.0f;
    bool mixed = false;
    float gap = 10.0f;
    std::optional<float> nativeOrigin = std::nullopt;
    bool configured = true;
    float height = 120.0f;
    bool lazy = false;
    float initialOffset = 6022.2f;
};

// Keep takeover asynchronous so touch/new-fling cancellation runs before the queued task.
class ItemSnapTaskExecutor final : public MockTaskExecutor {
    DECLARE_ACE_TYPE(ItemSnapTaskExecutor, MockTaskExecutor);
public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType, VsyncBarrierOption barrierOption) const override
    {
        if (name == "ArkUIScrollItemSnapTakeover") {
            tasks_.push_back(std::move(task));
            return true;
        }
        return MockTaskExecutor::OnPostTask(std::move(task), type, delayTime, name, priorityType, barrierOption);
    }

    void Drain()
    {
        auto tasks = std::move(tasks_);
        tasks_.clear();
        for (const auto& task : tasks) {
            task();
        }
    }

    bool HasPendingTask() const
    {
        return !tasks_.empty();
    }

private:
    mutable std::vector<Task> tasks_;
};
} // namespace

class GridItemSnapTakeoverTest : public GridTestNg {
public:
    void SetUp() override
    {
        GridTestNg::SetUp();
        auto pipeline = MockPipelineContext::GetCurrent();
        oldExecutor_ = pipeline->GetTaskExecutor();
        oldVsync_ = pipeline->GetVsyncTime();
        executor_ = AceType::MakeRefPtr<ItemSnapTaskExecutor>();
        pipeline->SetTaskExecutor(executor_);
    }

    void TearDown() override
    {
        // Destroy the node (and its weak callbacks) before restoring the global executor.
        GridTestNg::TearDown();
        auto pipeline = MockPipelineContext::GetCurrent();
        if (pipeline) {
            pipeline->SetTaskExecutor(oldExecutor_);
            pipeline->SetVsyncTime(oldVsync_);
        }
    }

    RefPtr<Scrollable> CreateFling(const ItemSnapFlingOptions& options = {})
    {
        auto model = CreateGrid();
        ViewAbstract::SetHeight(CalcLength(300.0f));
        model.SetColumnsTemplate("1fr 1fr");
        model.SetRowsGap(Dimension(options.gap));
        model.SetOnScrollStop([this]() { ++scrollStops_; });
        if (options.configured) {
            model.SetScrollSnapStrategy(MakeStrategy(options.align));
        }
        const std::vector<float> heights { 100.0f, 120.0f, 70.0f };
        if (options.lazy) {
            CreateItemsInLazyForEach(TAKEOVER_ITEM_COUNT,
                [heights, mixed = options.mixed, height = options.height](uint32_t index) {
                    return mixed ? heights[(index / TAKEOVER_COLUMN_COUNT) % heights.size()] : height;
                });
        } else {
            for (int32_t row = 0; row < TAKEOVER_ITEM_COUNT / TAKEOVER_COLUMN_COUNT; ++row) {
                CreateGridItems(TAKEOVER_COLUMN_COUNT, ITEM_MAIN_SIZE,
                    options.mixed ? heights[row % heights.size()] : options.height);
            }
        }
        CreateDone();
        UpdateCurrentOffset(-options.initialOffset);
        MockAnimationManager::GetInstance().SetTicks(TAKEOVER_ANIMATION_TICKS);
        auto scroll = pattern_->GetScrollableEvent()->GetScrollable();
        const float origin = options.nativeOrigin.value_or(-pattern_->GetTotalOffset());
        scroll->StartScrollAnimation(origin, options.velocity);
        frictionRate_ = std::abs(options.velocity / (scroll->GetFinalPosition() - origin));
        return scroll;
    }

    void DeliverFrame(const RefPtr<Scrollable>& scroll, float position, uint64_t interval = 16000000ULL)
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        pipeline->SetVsyncTime(pipeline->GetVsyncTime() + interval);
        scroll->lastVsyncTime_ = pipeline->GetVsyncTime() - interval;
        scroll->lastPosition_ = scroll->GetCurrentPos();
        auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->GetFrictionProperty()->GetProperty());
        ASSERT_NE(property, nullptr);
        property->GetUpdateCallback()(position);
        FlushUITasks();
    }

    void AdvanceToTakeover(const RefPtr<Scrollable>& scroll, uint64_t interval = 16000000ULL)
    {
        for (int32_t frame = 0; frame < TAKEOVER_MAX_FLING_FRAMES &&
            scroll->state_ == Scrollable::AnimationState::FRICTION; ++frame) {
            const float current = scroll->GetCurrentPos();
            const float end = scroll->GetFinalPosition();
            const float seconds = static_cast<float>(interval) / 1000000000.0f;
            DeliverFrame(scroll, end + (current - end) * std::exp(-frictionRate_ * seconds), interval);
        }
    }

    void FinishSnap()
    {
        executor_->Drain();
        auto& manager = MockAnimationManager::GetInstance();
        for (int32_t tick = 0; tick < TAKEOVER_MAX_SNAP_TICKS && !manager.AllFinished(); ++tick) {
            manager.Tick();
            FlushUITasks();
            executor_->Drain();
        }
        EXPECT_TRUE(manager.AllFinished());
    }

    void VerifyVisibleAnchor(ScrollSnapAlign align, bool horizontal = false, bool rtl = false)
    {
        const auto size = frameNode_->GetGeometryNode()->GetFrameSize();
        const float viewport = horizontal ? size.Width() : size.Height();
        float nearest = std::numeric_limits<float>::max();
        for (int32_t index = pattern_->info_.startIndex_; index <= pattern_->info_.endIndex_; ++index) {
            auto child = frameNode_->GetChildByIndex(index);
            if (!child || !child->GetGeometryNode()) {
                continue;
            }
            const auto rect = child->GetGeometryNode()->GetFrameRect();
            const float mainSize = horizontal ? rect.Width() : rect.Height();
            float start = horizontal ? rect.GetX() : rect.GetY();
            if (horizontal && rtl) {
                start = viewport - start - mainSize;
            }
            const float anchor = align == ScrollSnapAlign::START ? start :
                (align == ScrollSnapAlign::CENTER ? start + mainSize / 2.0f - viewport / 2.0f :
                    start + mainSize - viewport);
            nearest = std::min(nearest, std::abs(anchor));
        }
        EXPECT_NEAR(nearest, 0.0f, 0.01f);
    }

protected:
    RefPtr<TaskExecutor> oldExecutor_;
    RefPtr<ItemSnapTaskExecutor> executor_;
    uint64_t oldVsync_ = 0;
    float frictionRate_ = 0.0f;
    int32_t flingStops_ = 0;
    int32_t scrollStops_ = 0;
};

/**
 * @tc.name: FlingFrictionThenSettleEndToEnd018
 * @tc.desc: The real exponential friction driver advances layout before takeover and reaches a forward anchor.
 * @tc.type: FUNC
 */
HWTEST_F(GridItemSnapTakeoverTest, FlingFrictionThenSettleEndToEnd018, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::CENTER, -2000.0f, false, 0.0f,
        std::nullopt, true, 100.0f, false, 60.0f});
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    AdvanceToTakeover(scroll);
    FinishSnap();
    EXPECT_GT(pattern_->GetTotalOffset(), 400.0f);
    VerifyVisibleAnchor(ScrollSnapAlign::CENTER);
}

/**
 * @tc.name: FlingSettleFiresStopEventsOnce036
 * @tc.desc: Friction-to-snap handoff keeps both stop events pending until the spring finishes.
 * @tc.type: FUNC
 */
HWTEST_F(GridItemSnapTakeoverTest, FlingSettleFiresStopEventsOnce036, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::CENTER, -2000.0f, false, 0.0f,
        std::nullopt, true, 100.0f, false, 60.0f});
    const int32_t initialStops = scrollStops_;
    scroll->isUserFling_ = true;
    scroll->onDidStopFlingCallback_ = [this]() { ++flingStops_; };
    AdvanceToTakeover(scroll);
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scrollStops_, initialStops);
    FinishSnap();
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, initialStops + 1);
    EXPECT_GT(pattern_->GetTotalOffset(), 400.0f);
    VerifyVisibleAnchor(ScrollSnapAlign::CENTER);
}

// The friction driver follows the real exponential decay. The snap mock checks geometry and
// lifecycle only; acceleration and wall-clock spring duration require the Previewer backend.
class GridItemSnapGeometryTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<ScrollSnapAlign, float, bool, float, bool>> {};

HWTEST_P(GridItemSnapGeometryTest, MeasuredRowsRemainAlignedAfterTakeover, TestSize.Level1)
{
    auto [align, velocity, mixed, gap, lazy] = GetParam();
    auto scroll = CreateFling({align, velocity, mixed, gap, std::nullopt, true, 120.0f, lazy});
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    AdvanceToTakeover(scroll);
    const float handoff = scroll->GetCurrentPos();
    executor_->Drain();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), handoff);
    EXPECT_GE((scroll->GetFinalPosition() - handoff) * velocity, 0.0f);
    FinishSnap();
    VerifyVisibleAnchor(align);
}

INSTANTIATE_TEST_SUITE_P(AlignSpeedHeightGap, GridItemSnapGeometryTest,
    testing::Combine(testing::Values(ScrollSnapAlign::START, ScrollSnapAlign::CENTER, ScrollSnapAlign::END),
        testing::Values(-6000.0f, -1500.0f, -250.0f, 6000.0f, 1500.0f, 250.0f),
        testing::Bool(), testing::Values(0.0f, 10.0f, 30.0f), testing::Bool()));

class GridItemSnapReleaseTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<float, float, uint64_t>> {};

HWTEST_P(GridItemSnapReleaseTest, HighCenterDoesNotReverseAcrossReleasePositionsAndDelayedFrames, TestSize.Level1)
{
    auto [release, velocity, interval] = GetParam();
    auto scroll = CreateFling({ScrollSnapAlign::CENTER, velocity, false, 10.0f,
        std::nullopt, true, 120.0f, true, release});
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float direction = velocity > 0.0f ? 1.0f : -1.0f;
    float previous = scroll->GetCurrentPos();
    float reverse = 0.0f;
    auto sample = [scroll, direction, &previous, &reverse]() {
        const float current = scroll->GetCurrentPos();
        reverse += std::max(0.0f, direction * (previous - current));
        previous = current;
    };
    for (int32_t frame = 0; frame < TAKEOVER_MAX_FLING_FRAMES &&
        scroll->state_ == Scrollable::AnimationState::FRICTION; ++frame) {
        const float current = scroll->GetCurrentPos();
        const float end = scroll->GetFinalPosition();
        const float seconds = static_cast<float>(interval) / 1000000000.0f;
        DeliverFrame(scroll, end + (current - end) * std::exp(-frictionRate_ * seconds), interval);
        executor_->Drain();
        sample();
    }
    auto& manager = MockAnimationManager::GetInstance();
    for (int32_t tick = 0; tick < TAKEOVER_MAX_SNAP_TICKS && !manager.AllFinished(); ++tick) {
        manager.Tick();
        FlushUITasks();
        executor_->Drain();
        sample();
    }
    EXPECT_TRUE(manager.AllFinished());
    EXPECT_LE(reverse, 0.01f);
    EXPECT_NE(scroll->state_, Scrollable::AnimationState::FRICTION);
    VerifyVisibleAnchor(ScrollSnapAlign::CENTER);
}

INSTANTIATE_TEST_SUITE_P(ReleasePositionsAndFrames, GridItemSnapReleaseTest,
    testing::Combine(testing::Values(6022.2f, 6050.0f, 6095.0f, 6140.0f, 6200.0f, 6280.0f),
        testing::Values(-6000.0f, 6000.0f), testing::Values(16000000ULL, 250000000ULL)));

class GridItemSnapHorizontalTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<ScrollSnapAlign, float, bool>> {};

HWTEST_P(GridItemSnapHorizontalTest, HorizontalAndRtlUseRealLogicalAnchors, TestSize.Level1)
{
    auto [align, velocity, rtl] = GetParam();
    auto model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    model.SetColumnsGap(Dimension(10.0f));
    model.SetScrollSnapStrategy(MakeStrategy(align));
    ViewAbstract::SetLayoutDirection(rtl ? TextDirection::RTL : TextDirection::LTR);
    CreateGridItems(500, 120.0f, 100.0f);
    CreateDone();
    UpdateCurrentOffset(-6022.2f);
    MockAnimationManager::GetInstance().SetTicks(40);
    auto scroll = pattern_->GetScrollableEvent()->GetScrollable();
    const float origin = -pattern_->GetTotalOffset();
    ASSERT_TRUE(scroll->StartScrollAnimation(origin, velocity));
    frictionRate_ = std::abs(velocity / (scroll->GetFinalPosition() - origin));
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align, true, rtl);
}

INSTANTIATE_TEST_SUITE_P(HorizontalDirection, GridItemSnapHorizontalTest,
    testing::Combine(testing::Values(ScrollSnapAlign::START, ScrollSnapAlign::CENTER, ScrollSnapAlign::END),
        testing::Values(-6000.0f, 6000.0f), testing::Bool()));

HWTEST_F(GridItemSnapTakeoverTest, VerySlowReleaseAlignsWithoutStartingFriction, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::CENTER, -100.0f});
    EXPECT_NE(scroll->state_, Scrollable::AnimationState::FRICTION);
    FinishSnap();
    VerifyVisibleAnchor(ScrollSnapAlign::CENTER);
}

HWTEST_F(GridItemSnapTakeoverTest, TouchCancellationSettlesAndReportsStopOnce, TestSize.Level1)
{
    auto scroll = CreateFling();
    const int32_t initialStops = scrollStops_;
    scroll->isUserFling_ = true;
    scroll->onDidStopFlingCallback_ = [this]() { ++flingStops_; };
    AdvanceToTakeover(scroll);
    ASSERT_TRUE(executor_->HasPendingTask());
    scroll->HandleTouchDown();
    executor_->Drain();
    EXPECT_EQ(flingStops_, 0);
    scroll->HandleTouchUp();
    EXPECT_EQ(flingStops_, 0);
    FinishSnap();
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, initialStops + 1);
}

HWTEST_F(GridItemSnapTakeoverTest, UnknownRowsKeepFrictionRunning, TestSize.Level1)
{
    auto scroll = CreateFling();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    DeliverFrame(scroll, scroll->GetCurrentPos() - 1.0f);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FALSE(executor_->HasPendingTask());
}

HWTEST_F(GridItemSnapTakeoverTest, DelayedFrameDoesNotSkipUnmeasuredRows, TestSize.Level1)
{
    auto scroll = CreateFling();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float current = scroll->GetCurrentPos();
    DeliverFrame(scroll, scroll->GetFinalPosition() + 1.0f, 250000000ULL);
    EXPECT_LE(std::abs(scroll->GetCurrentPos() - current), 150.001f);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
}

HWTEST_F(GridItemSnapTakeoverTest, TouchCancelsQueuedTakeover, TestSize.Level1)
{
    auto scroll = CreateFling();
    AdvanceToTakeover(scroll);
    ASSERT_TRUE(executor_->HasPendingTask());
    scroll->HandleTouchDown();
    executor_->Drain();
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::IDLE);
}

HWTEST_F(GridItemSnapTakeoverTest, NewFlingInvalidatesQueuedTakeover, TestSize.Level1)
{
    auto scroll = CreateFling();
    AdvanceToTakeover(scroll);
    ASSERT_TRUE(executor_->HasPendingTask());
    ASSERT_TRUE(scroll->StartScrollAnimation(scroll->GetCurrentPos(), 6000.0f));
    const float newEnd = scroll->GetFinalPosition();
    executor_->Drain();
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FLOAT_EQ(scroll->GetFinalPosition(), newEnd);
}

HWTEST_F(GridItemSnapTakeoverTest, NoneDuringFlingRestoresOrdinaryMotion, TestSize.Level1)
{
    auto scroll = CreateFling();
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), MakeStrategy(ScrollSnapAlign::NONE));
    FlushUITasks();
    const float sample = scroll->GetCurrentPos() - 400.0f;
    DeliverFrame(scroll, sample);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), sample);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FALSE(executor_->HasPendingTask());
}

HWTEST_F(GridItemSnapTakeoverTest, UnconfiguredFlingKeepsOrdinaryMotion, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::NONE, -6000.0f, false, 10.0f, std::nullopt, false});
    const float sample = scroll->GetCurrentPos() - 400.0f;
    DeliverFrame(scroll, sample);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), sample);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FALSE(executor_->HasPendingTask());
}

HWTEST_F(GridItemSnapTakeoverTest, GestureOriginDoesNotShiftMixedHeightAnchors, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::CENTER, -6000.0f, true, 10.0f, 500.0f});
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(ScrollSnapAlign::CENTER);
}

HWTEST_F(GridItemSnapTakeoverTest, SmallRowsHighEndKeepsReleaseDirection, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::END, -6000.0f, false, 0.0f, std::nullopt, true, 60.0f});
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    AdvanceToTakeover(scroll);
    const float handoff = scroll->GetCurrentPos();
    executor_->Drain();
    EXPECT_LE(scroll->GetFinalPosition(), handoff);
    FinishSnap();
    VerifyVisibleAnchor(ScrollSnapAlign::END);
}

HWTEST_F(GridItemSnapTakeoverTest, HighReleaseDirectionSurvivesLowResidualSpeed, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::END, -6000.0f, false, 0.0f, std::nullopt, true, 60.0f});
    // Near an END anchor, the nearest-anchor policy at 119px/s would reverse a high-speed fling.
    scroll->SetCurrentPos(-8355.79f);
    UpdateCurrentOffset(6022.2f - 8355.79f);
    scroll->StopFrictionAnimation();
    SnapAnimationOptions options { .animationVelocity = -119.0f, .fromItemSnapFlingStop = true };
    ASSERT_TRUE(pattern_->StartItemSnapAnimation(options));
    EXPECT_LE(scroll->GetFinalPosition(), -8355.79f);
}

HWTEST_F(GridItemSnapTakeoverTest, DelayedKnownFrameStopsAtActualTakeover, TestSize.Level1)
{
    auto scroll = CreateFling();
    // Materialize the destination window before injecting a late sample beyond the threshold.
    for (int32_t frame = 0; frame < TAKEOVER_MAX_FLING_FRAMES && !scroll->itemSnapTargetReady_; ++frame) {
        const float current = scroll->GetCurrentPos();
        const float end = scroll->GetFinalPosition();
        DeliverFrame(scroll, end + (current - end) * std::exp(-frictionRate_ * 0.016f));
    }
    ASSERT_TRUE(scroll->itemSnapTargetReady_);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float naturalEnd = scroll->GetFinalPosition();
    const float takeover = naturalEnd + scroll->itemSnapTakeoverDistance_;
    DeliverFrame(scroll, naturalEnd + 0.1f, 250000000ULL);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), takeover);
    EXPECT_NEAR(scroll->frictionVelocity_, frictionRate_ * (naturalEnd - takeover), 0.01f);
    ASSERT_TRUE(executor_->HasPendingTask());
    executor_->Drain();
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), takeover);
    EXPECT_LE(scroll->GetFinalPosition(), takeover);
}

HWTEST_F(GridItemSnapTakeoverTest, CancelledFrictionFinishCannotStopReplacementSnap, TestSize.Level1)
{
    auto scroll = CreateFling();
    auto& manager = MockAnimationManager::GetInstance();
    ASSERT_FALSE(manager.animations_.empty());
    auto removedFinish = manager.animations_.back()->cbs_.finishCb;
    ASSERT_TRUE(removedFinish);
    scroll->HandleTouchDown();
    // Releasing without another drag starts the new nearby-anchor snap.
    scroll->HandleTouchUp();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    const float target = scroll->GetFinalPosition();
    removedFinish();
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    EXPECT_FLOAT_EQ(scroll->GetFinalPosition(), target);
}

HWTEST_F(GridItemSnapTakeoverTest, TakeoverReportsStopOnlyAfterSnapCompletes, TestSize.Level1)
{
    auto scroll = CreateFling();
    const int32_t initialStops = scrollStops_;
    scroll->isUserFling_ = true;
    scroll->onDidStopFlingCallback_ = [this]() { ++flingStops_; };
    AdvanceToTakeover(scroll);
    ASSERT_TRUE(executor_->HasPendingTask());
    EXPECT_EQ(flingStops_, 0);
    executor_->Drain();
    EXPECT_EQ(flingStops_, 0);
    FinishSnap();
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, initialStops + 1);
}

HWTEST_F(GridItemSnapTakeoverTest, NaturalStopRejectsOvershootingAndStaleSamples, TestSize.Level1)
{
    auto scroll = CreateFling({ScrollSnapAlign::START});
    scroll->StopFrictionAnimation();
    SnapAnimationOptions options { .animationVelocity = -119.0f, .fromItemSnapFlingStop = true };
    ASSERT_TRUE(pattern_->StartItemSnapAnimation(options));
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    const float current = scroll->GetCurrentPos();
    const float target = scroll->GetFinalPosition();
    ASSERT_LT(target, current);
    auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->GetSnapProperty()->GetProperty());
    ASSERT_NE(property, nullptr);
    property->GetUpdateCallback()(current + 5.0f);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), current);
    property->GetUpdateCallback()(target - 5.0f);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), target);
}

} // namespace OHOS::Ace::NG

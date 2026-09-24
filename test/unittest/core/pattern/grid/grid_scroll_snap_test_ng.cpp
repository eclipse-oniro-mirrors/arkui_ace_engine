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

} // namespace OHOS::Ace::NG

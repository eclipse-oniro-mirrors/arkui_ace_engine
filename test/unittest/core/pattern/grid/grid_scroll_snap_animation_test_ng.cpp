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

#include "test/unittest/core/pattern/grid/grid_test_ng.h"

#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include <cmath>
#include <functional>
#include <limits>
#include <tuple>

#include <gtest/gtest.h>

#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_animation_consts.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/animation/mock_animation_proxy.h"

namespace OHOS::Ace::NG {
namespace {
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

namespace {
constexpr int32_t SNAP_GRID_ITEM_NUMBER = 30;
constexpr int32_t PROVIDER_BUDGET_ITEM_COUNT = 60;
constexpr int32_t TAKEOVER_ITEM_COUNT = 500;
constexpr int32_t TAKEOVER_COLUMN_COUNT = 2;
constexpr int32_t TAKEOVER_ANIMATION_TICKS = 40;
constexpr int32_t TAKEOVER_MAX_FLING_FRAMES = 400;
constexpr int32_t TAKEOVER_MAX_SNAP_TICKS = 200;
constexpr int32_t SCROLL_BAR_ITEM_COUNT = 200;
constexpr double SCROLL_BAR_TEST_VELOCITY_LIMIT = 100000.0;

enum class ScrollBarEvent {
    DRAG_STOP,
    DRAG_STOP_WITH_FLING,
    FLING_START,
};

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
    float contentStartOffset = 0.0f;
    float contentEndOffset = 0.0f;
};

// Keep takeover asynchronous so touch/new-fling cancellation runs before the queued task.
class ItemSnapTaskExecutor final : public MockTaskExecutor {
    DECLARE_ACE_TYPE(ItemSnapTaskExecutor, MockTaskExecutor);
public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType, VsyncBarrierOption barrierOption) const override
    {
        if (name == "ArkUIScrollItemSnapTakeover" || name == "ArkUIScrollItemSnapHandoff") {
            if (rejectHandoffs_) {
                return false;
            }
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

    void RejectHandoffs(bool reject)
    {
        rejectHandoffs_ = reject;
    }

private:
    mutable std::vector<Task> tasks_;
    bool rejectHandoffs_ = false;
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
        ScrollableModelNG::SetContentStartOffset(options.contentStartOffset);
        ScrollableModelNG::SetContentEndOffset(options.contentEndOffset);
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

    void DeliverSnapFrame(const RefPtr<Scrollable>& scroll, float position, uint64_t interval = 16000000ULL)
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        pipeline->SetVsyncTime(pipeline->GetVsyncTime() + interval);
        scroll->lastVsyncTime_ = pipeline->GetVsyncTime() - interval;
        auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->snapOffsetProperty_->GetProperty());
        ASSERT_NE(property, nullptr);
        property->GetUpdateCallback()(position);
        FlushUITasks();
    }

    void FinishSnap()
    {
        executor_->Drain();
        FlushUITasks();
        auto& manager = MockAnimationManager::GetInstance();
        for (int32_t tick = 0; tick < TAKEOVER_MAX_SNAP_TICKS && !manager.AllFinished(); ++tick) {
            manager.Tick();
            FlushUITasks();
            executor_->Drain();
            FlushUITasks();
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
            const float anchor = align == ScrollSnapAlign::START ? start - pattern_->GetContentStartOffset() :
                (align == ScrollSnapAlign::CENTER ? start + mainSize / 2.0f - viewport / 2.0f :
                    start + mainSize - viewport + pattern_->GetContentEndOffset());
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

class GridItemSnapContentOffsetsTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<ScrollSnapAlign, float>> {};

HWTEST_P(GridItemSnapContentOffsetsTest, FlingSettlesWithContentOffsets, TestSize.Level1)
{
    auto [align, velocity] = GetParam();
    ItemSnapFlingOptions options;
    options.align = align;
    options.velocity = velocity;
    options.mixed = true;
    options.lazy = true;
    options.contentStartOffset = CONTENT_START_OFFSET;
    options.contentEndOffset = CONTENT_END_OFFSET;
    auto scroll = CreateFling(options);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
}

INSTANTIATE_TEST_SUITE_P(AlignAndVelocity, GridItemSnapContentOffsetsTest,
    testing::Combine(testing::Values(ScrollSnapAlign::START, ScrollSnapAlign::END, ScrollSnapAlign::CENTER),
        testing::Values(-1500.0f, 1500.0f)));

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
    auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->snapOffsetProperty_->GetProperty());
    ASSERT_NE(property, nullptr);
    property->GetUpdateCallback()(current + 5.0f);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), current);
    property->GetUpdateCallback()(target - 5.0f);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), target);
}

/**
 * @tc.name: ProviderNonFiniteTargetCleansState014
 * @tc.desc: Exercise the actual deferred callback; an invalid target stays at the approach endpoint and stops once.
 */
HWTEST_F(GridItemSnapTakeoverTest, ProviderNonFiniteTargetCleansState014, TestSize.Level1)
{
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return std::nan("");
    });
    strategy.align = ScrollSnapAlign::NONE;
    strategy.calculateApproachOffset = [](double, double) { return 30.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    model.SetOnScrollStop([this]() { ++scrollStops_; });
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    UpdateCurrentOffset(-60.0f);
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(-60.0f, -1000.0f));
    FinishSnap();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_TRUE(scroll->IsStopped());
    EXPECT_NEAR(pattern_->GetTotalOffset(), 90.0f, 0.01f);
    EXPECT_EQ(scrollStops_, 1);
}

/**
 * @tc.name: ProviderApproachHandsOffAtExactDistance
 * @tc.desc: An overshooting approach frame is clipped before the single deferred target callback.
 */
HWTEST_F(GridItemSnapTakeoverTest, ProviderApproachHandsOffAtExactDistance, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    double seenOffset = -1.0;
    double seenVelocity = 0.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset, &seenVelocity](double velocity) {
        ++snapCalls;
        seenOffset = pattern_->GetTotalOffset();
        seenVelocity = velocity;
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 200.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    model.SetOnScrollStop([this]() { ++scrollStops_; });
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    ASSERT_EQ(approachCalls, 1);
    ASSERT_EQ(snapCalls, 0);
    EXPECT_EQ(scrollStops_, 0) << "after start";
    DeliverSnapFrame(scroll, -180.0f);
    EXPECT_EQ(scrollStops_, 0) << "before distance threshold";
    constexpr uint64_t handoffIntervalNs = 40000000ULL;
    DeliverSnapFrame(scroll, -220.0f, handoffIntervalNs);
    EXPECT_EQ(scrollStops_, 0) << "queued handoff, before UI task";
    EXPECT_NEAR(pattern_->GetTotalOffset(), 200.0f, 0.01f);
    EXPECT_EQ(snapCalls, 0);
    EXPECT_TRUE(executor_->HasPendingTask());
    executor_->Drain();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_DOUBLE_EQ(seenOffset, 200.0);
    EXPECT_DOUBLE_EQ(seenVelocity, 1200.0);
    EXPECT_FLOAT_EQ(MockAnimationManager::GetInstance().params_.velocity, -1000.0f);
    EXPECT_EQ(scrollStops_, 0);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(scrollStops_, 1);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderReverseApproachKeepsDistanceAndVelocity, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    double seenOffset = -1.0;
    double seenVelocity = 0.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset, &seenVelocity](double velocity) {
        ++snapCalls;
        seenOffset = pattern_->GetTotalOffset();
        seenVelocity = velocity;
        return 200.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 160.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    UpdateCurrentOffset(-600.0f);
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(-600.0f, 1200.0f));
    DeliverSnapFrame(scroll, -460.0f);
    constexpr uint64_t handoffIntervalNs = 40000000ULL;
    DeliverSnapFrame(scroll, -420.0f, handoffIntervalNs);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 440.0f, 0.01f);
    EXPECT_EQ(snapCalls, 0);
    executor_->Drain();
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_DOUBLE_EQ(seenOffset, 440.0);
    EXPECT_DOUBLE_EQ(seenVelocity, -1200.0);
    EXPECT_FLOAT_EQ(MockAnimationManager::GetInstance().params_.velocity, 1000.0f);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 200.0f, 0.01f);
    EXPECT_EQ(snapCalls, 1);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderApproachMayExceedNaturalDecay, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    double natural = 0.0;
    double seenOffset = -1.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset](double) {
        ++snapCalls;
        seenOffset = pattern_->GetTotalOffset();
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approachCalls, &natural](double, double decay) {
        ++approachCalls;
        natural = decay;
        return 300.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -100.0f));
    ASSERT_LT(natural, 300.0);
    DeliverSnapFrame(scroll, -350.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 300.0f, 0.01f);
    executor_->Drain();
    EXPECT_EQ(snapCalls, 0);
    FinishSnap();
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_DOUBLE_EQ(seenOffset, 300.0);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderPositiveApproachWithZeroReleaseVelocity, TestSize.Level1)
{
    int32_t snapCalls = 0;
    double seenOffset = -1.0;
    double seenVelocity = -1.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset, &seenVelocity](double velocity) {
        ++snapCalls;
        seenOffset = pattern_->GetTotalOffset();
        seenVelocity = velocity;
        return 400.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 60.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, 0.0f));
    EXPECT_EQ(snapCalls, 0);
    DeliverSnapFrame(scroll, -90.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 60.0f, 0.01f);
    executor_->Drain();
    EXPECT_EQ(snapCalls, 0);
    FinishSnap();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_DOUBLE_EQ(seenOffset, 60.0);
    EXPECT_DOUBLE_EQ(seenVelocity, 0.0);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 400.0f, 0.01f);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderTouchCancelsQueuedDistanceHandoff, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 200.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    model.SetOnScrollStop([this]() { ++scrollStops_; });
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    scroll->isUserFling_ = true;
    scroll->onDidStopFlingCallback_ = [this]() { ++flingStops_; };
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    DeliverSnapFrame(scroll, -250.0f);
    ASSERT_TRUE(executor_->HasPendingTask());
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scrollStops_, 0);
    scroll->HandleTouchDown();
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scrollStops_, 0);
    executor_->Drain();
    FinishSnap();
    EXPECT_EQ(snapCalls, 0);
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scrollStops_, 0);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 200.0f, 0.01f);
    scroll->HandleTouchUp();
    EXPECT_EQ(approachCalls, 2);
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scrollStops_, 0);
    FinishSnap();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
    EXPECT_FALSE(scroll->isUserFling_);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderFinalSpringClampsOvershoot, TestSize.Level1)
{
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 600.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 200.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    DeliverSnapFrame(scroll, -250.0f);
    executor_->Drain();
    ASSERT_EQ(snapCalls, 1);
    DeliverSnapFrame(scroll, -650.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    DeliverSnapFrame(scroll, -620.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    FinishSnap();
    EXPECT_EQ(snapCalls, 1);
}


HWTEST_F(GridItemSnapTakeoverTest, ProviderZeroApproachCallsEachProviderOnce, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    double seenVelocity = 0.0;
    auto strategy = MakeProviderStrategy([&snapCalls, &seenVelocity](double velocity) {
        ++snapCalls;
        seenVelocity = velocity;
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 0.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    model.SetOnScrollStop([this]() { ++scrollStops_; });
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_DOUBLE_EQ(seenVelocity, 1200.0);
    EXPECT_FLOAT_EQ(MockAnimationManager::GetInstance().params_.velocity, -1200.0f);
    EXPECT_FALSE(executor_->HasPendingTask());
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_EQ(scrollStops_, 1);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderRestartInvalidatesQueuedHandoff, TestSize.Level1)
{
    int32_t oldCalls = 0;
    int32_t newCalls = 0;
    auto strategy = MakeProviderStrategy([&oldCalls](double) {
        ++oldCalls;
        return 600.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 200.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    DeliverSnapFrame(scroll, -250.0f);
    ASSERT_TRUE(executor_->HasPendingTask());
    auto replacement = MakeProviderStrategy([&newCalls](double) {
        ++newCalls;
        return 400.0;
    });
    replacement.calculateApproachOffset = [](double, double) { return 0.0; };
    GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), replacement);
    ASSERT_TRUE(scroll->StartScrollAnimation(-200.0f, -800.0f));
    executor_->Drain();
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 1);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 400.0f, 0.01f);
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 1);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderRejectedHandoffStopsAndCanRetry, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 200.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    model.SetOnScrollStop([this]() { ++scrollStops_; });
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    executor_->RejectHandoffs(true);
    DeliverSnapFrame(scroll, -250.0f);
    FinishSnap();
    EXPECT_TRUE(scroll->IsStopped());
    EXPECT_EQ(snapCalls, 0);
    EXPECT_EQ(scrollStops_, 1);
    executor_->RejectHandoffs(false);
    ASSERT_TRUE(scroll->StartScrollAnimation(-200.0f, -1200.0f));
    DeliverSnapFrame(scroll, -450.0f);
    FinishSnap();
    EXPECT_EQ(approachCalls, 2);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    EXPECT_EQ(scrollStops_, 2);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderCanChooseTargetBehindApproach, TestSize.Level1)
{
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 100.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 200.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1200.0f));
    DeliverSnapFrame(scroll, -250.0f);
    executor_->Drain();
    ASSERT_EQ(snapCalls, 1);
    EXPECT_FLOAT_EQ(MockAnimationManager::GetInstance().params_.velocity, 0.0f);
    DeliverSnapFrame(scroll, -240.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 200.0f, 0.01f);
    DeliverSnapFrame(scroll, -80.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 100.0f, 0.01f);
    FinishSnap();
    EXPECT_EQ(snapCalls, 1);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderInvalidApproachFallsBackToNaturalDistance, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    double natural = 0.0;
    double seenOffset = 0.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset](double) {
        seenOffset = pattern_->GetTotalOffset();
        return ++snapCalls == 1 ? 600.0 : 200.0;
    });
    strategy.calculateApproachOffset = [&approachCalls, &natural](double, double decay) {
        natural = decay;
        return ++approachCalls == 1 ? std::numeric_limits<double>::infinity() : -1.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1000.0f));
    const float firstDistance = natural;
    DeliverSnapFrame(scroll, -firstDistance - 10.0f);
    executor_->Drain();
    EXPECT_EQ(snapCalls, 0);
    FinishSnap();
    EXPECT_NEAR(seenOffset, firstDistance, 0.01f);
    ASSERT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    ASSERT_TRUE(scroll->StartScrollAnimation(-600.0f, 1000.0f));
    const float secondDistance = natural;
    DeliverSnapFrame(scroll, -600.0f + secondDistance + 10.0f);
    executor_->Drain();
    EXPECT_EQ(snapCalls, 1);
    FinishSnap();
    EXPECT_NEAR(seenOffset, 600.0f - secondDistance, 0.01f);
    EXPECT_EQ(approachCalls, 2);
    EXPECT_EQ(snapCalls, 2);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 200.0f, 0.01f);
}

HWTEST_F(GridItemSnapTakeoverTest, ProviderApproachCurveValidForExtremeFriction, TestSize.Level1)
{
    auto strategy = MakeProviderStrategy([](double) { return 400.0; });
    strategy.calculateApproachOffset = [](double, double) { return 60.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    scroll->SetUnstaticFriction(std::numeric_limits<double>::max());
    SnapAnimationOptions options;
    options.snapDelta = -60.0f;
    options.animationVelocity = -1000.0f;
    ASSERT_TRUE(pattern_->StartSnapAnimation(options));
    scroll->FlushPendingItemSnapKickoff();
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(MockAnimationManager::GetInstance().params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_TRUE(std::isfinite(curve->GetResponse()));
    EXPECT_GT(curve->GetResponse(), 0.0f);
    scroll->HandleTouchDown();
    executor_->Drain();
    FinishSnap();
}


class GridProviderSynchronousTest : public GridItemSnapTakeoverTest {
public:
    void SetUp() override
    {
        GridItemSnapTakeoverTest::SetUp();
        oldVersion_ = MockAnimationManager::GetVersion();
        MockAnimationManager::SetVersion(MockAnimationManager::Version::V1);
    }

    void TearDown() override
    {
        MockAnimationManager::SetVersion(oldVersion_);
        GridItemSnapTakeoverTest::TearDown();
    }

private:
    MockAnimationManager::Version oldVersion_ = MockAnimationManager::Version::V0;
};

HWTEST_F(GridProviderSynchronousTest, ProviderInitializationDoesNotDispatchScroll, TestSize.Level1)
{
    int32_t approachCalls = 0;
    int32_t snapCalls = 0;
    int32_t willScrollCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 400.0;
    });
    strategy.calculateApproachOffset = [&approachCalls](double, double) {
        ++approachCalls;
        return 100.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    UpdateCurrentOffset(-60.0f);
    eventHub_->SetOnWillScroll([&willScrollCalls](Dimension offset, ScrollState, ScrollSource) {
        ++willScrollCalls;
        return ScrollFrameResult { .offset = offset };
    });
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(-60.0f, -1000.0f));
    EXPECT_EQ(willScrollCalls, 0) << "Initializing a property is not a scroll frame";
    EXPECT_EQ(approachCalls, 1);
    EXPECT_EQ(snapCalls, 0);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 60.0f, 0.01f);
    FinishSnap();
    EXPECT_GT(willScrollCalls, 0);
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 400.0f, 0.01f);
    eventHub_->SetOnWillScroll(nullptr);
}

HWTEST_F(GridProviderSynchronousTest, ProviderSynchronousStopDiscardsOldApproach, TestSize.Level1)
{
    int32_t snapCalls = 0;
    auto strategy = MakeProviderStrategy([&snapCalls](double) {
        ++snapCalls;
        return 400.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 100.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    UpdateCurrentOffset(-60.0f);
    auto scroll = pattern_->GetScrollable();
    auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->GetSnapProperty()->GetProperty());
    ASSERT_NE(property, nullptr);
    auto originalCallback = property->GetUpdateCallback();
    int32_t reentries = 0;
    // V1 invokes this synchronously from the initial Set, outside an animation scope.
    property->SetUpdateCallback(
        [weak = WeakPtr<Scrollable>(scroll), originalCallback, &reentries](const float& position) {
            originalCallback(position);
            auto current = weak.Upgrade();
            ASSERT_NE(current, nullptr);
            if (reentries++ == 0) {
                current->StopScrollable();
            }
        });
    ASSERT_TRUE(scroll->StartScrollAnimation(-60.0f, -1000.0f));
    EXPECT_GE(reentries, 1);
    EXPECT_TRUE(scroll->IsStopped());
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished()) << "Cancelled approach must never be submitted";
    property->SetUpdateCallback(std::move(originalCallback));
    FinishSnap();
    EXPECT_EQ(snapCalls, 0);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 60.0f, 0.01f);
}

HWTEST_F(GridProviderSynchronousTest, ProviderSynchronousFlingKeepsReplacement, TestSize.Level1)
{
    int32_t oldCalls = 0;
    int32_t newCalls = 0;
    auto strategy = MakeProviderStrategy([&oldCalls](double) {
        ++oldCalls;
        return 600.0;
    });
    strategy.calculateApproachOffset = [](double, double) { return 100.0; };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    UpdateCurrentOffset(-60.0f);
    auto scroll = pattern_->GetScrollable();
    auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->GetSnapProperty()->GetProperty());
    ASSERT_NE(property, nullptr);
    auto originalCallback = property->GetUpdateCallback();
    int32_t reentries = 0;
    property->SetUpdateCallback(
        [this, weak = WeakPtr<Scrollable>(scroll), originalCallback, &reentries, &newCalls](const float& position) {
            originalCallback(position);
            if (reentries++ != 0) {
                return;
            }
            auto replacement = MakeProviderStrategy([&newCalls](double) {
                ++newCalls;
                return 400.0;
            });
            replacement.calculateApproachOffset = [](double, double) { return 0.0; };
            GridModelNG::SetScrollSnapStrategy(AceType::RawPtr(frameNode_), replacement);
            auto current = weak.Upgrade();
            ASSERT_NE(current, nullptr);
            EXPECT_TRUE(current->StartScrollAnimation(current->GetCurrentPos(), -800.0f));
        });
    ASSERT_TRUE(scroll->StartScrollAnimation(-60.0f, -1000.0f));
    EXPECT_GE(reentries, 1);
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 1);
    EXPECT_FLOAT_EQ(MockAnimationManager::GetInstance().params_.velocity, -800.0f);
    EXPECT_FLOAT_EQ(scroll->snapOffsetProperty_->GetStagingValue(), -400.0f);
    property->SetUpdateCallback(std::move(originalCallback));
    FinishSnap();
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 1);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 400.0f, 0.01f);
}

class GridProviderFlingBudgetTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<float, bool>> {
public:
    void SetUp() override
    {
        GridItemSnapTakeoverTest::SetUp();
        ratio_ = std::get<0>(GetParam());
        direction_ = std::get<1>(GetParam()) ? -1.0f : 1.0f;
        startOffset_ = direction_ < 0.0f ? 1600.0f : 0.0f;
        auto strategy = MakeProviderStrategy([this](double velocity) {
            ++snapCalls_;
            seenOffset_ = pattern_->GetTotalOffset();
            seenVelocity_ = velocity;
            return startOffset_ + direction_ * (approach_ + finalDistance_);
        });
        strategy.calculateApproachOffset = [this](double, double decay) {
            ++approachCalls_;
            natural_ = decay;
            approach_ = decay * ratio_;
            return approach_;
        };
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr");
        model.SetScrollSnapStrategy(strategy);
        model.SetOnScrollStop([this]() { ++scrollStops_; });
        CreateGridItems(PROVIDER_BUDGET_ITEM_COUNT, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
        CreateDone();
        UpdateCurrentOffset(-startOffset_);
        MockAnimationManager::GetInstance().SetTicks(TAKEOVER_ANIMATION_TICKS);
    }

protected:
    const float releaseSpeed_ = 1200.0f;
    const float finalDistance_ = 100.0f;
    float ratio_ = 0.0f;
    float direction_ = 1.0f;
    float startOffset_ = 0.0f;
    int32_t approachCalls_ = 0;
    int32_t snapCalls_ = 0;
    double natural_ = 0.0;
    double approach_ = 0.0;
    double seenOffset_ = 0.0;
    double seenVelocity_ = 0.0;
};

/**
 * @tc.name: ProviderBudgetUsesFlingCurveAndDestination
 * @tc.desc: Preserve the fling destination inside budget and extend it only for a farther approach.
 */
HWTEST_P(GridProviderFlingBudgetTest, ProviderBudgetUsesFlingCurveAndDestination, TestSize.Level1)
{
    auto scroll = pattern_->GetScrollable();
    auto& manager = MockAnimationManager::GetInstance();
    ASSERT_TRUE(scroll->StartScrollAnimation(-startOffset_, -direction_ * releaseSpeed_));
    auto property = AceType::DynamicCast<AnimatablePropertyFloat>(scroll->snapOffsetProperty_->GetProperty());
    ASSERT_NE(property, nullptr);
    const float expectedEnd = -startOffset_ - direction_ * std::max(natural_, approach_);
    EXPECT_NEAR(MockAnimationProxy<float>::GetInstance().GetEndValue(property), expectedEnd, 0.01f);
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(manager.params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_NEAR(curve->GetResponse(), std::abs(2.0 * ACE_PI / (FRICTION_SCALE * scroll->GetFriction())), 0.0001f);
    EXPECT_FLOAT_EQ(curve->GetDampingRatio(), 1.0f);
    EXPECT_FLOAT_EQ(manager.params_.velocity, -direction_ * releaseSpeed_);
    EXPECT_EQ(approachCalls_, 1);
    EXPECT_EQ(snapCalls_, 0);
    scroll->HandleTouchDown();
    FinishSnap();
    EXPECT_EQ(snapCalls_, 0);
}

/**
 * @tc.name: ProviderBudgetDestinationAndHandoff
 * @tc.desc: Clip at approach, wait for endpoint completion when required, and run the final snap once.
 */
HWTEST_P(GridProviderFlingBudgetTest, ProviderBudgetDestinationAndHandoff, TestSize.Level1)
{
    auto scroll = pattern_->GetScrollable();
    auto& manager = MockAnimationManager::GetInstance();
    ASSERT_TRUE(scroll->StartScrollAnimation(-startOffset_, -direction_ * releaseSpeed_));
    auto approachFinish = manager.params_.callbacks.finishCb;
    ASSERT_TRUE(approachFinish);
    const float approachPosition = -startOffset_ - direction_ * approach_;
    DeliverSnapFrame(scroll, approachPosition - direction_ * 10.0f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), startOffset_ + direction_ * approach_, 0.01f);
    EXPECT_EQ(snapCalls_, 0);
    if (ratio_ < 1.0f) {
        EXPECT_TRUE(executor_->HasPendingTask());
    } else {
        EXPECT_FALSE(executor_->HasPendingTask());
        executor_->Drain();
        EXPECT_EQ(snapCalls_, 0) << "An endpoint handoff waits for animation completion";
        approachFinish();
    }
    executor_->Drain();
    ASSERT_EQ(snapCalls_, 1);
    EXPECT_NEAR(seenOffset_, startOffset_ + direction_ * approach_, 0.01);
    EXPECT_DOUBLE_EQ(seenVelocity_, direction_ * releaseSpeed_);
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(manager.params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_FLOAT_EQ(curve->GetResponse(), 0.7f);
    EXPECT_FLOAT_EQ(curve->GetDampingRatio(), 2.5f);
    approachFinish();
    executor_->Drain();
    EXPECT_EQ(snapCalls_, 1) << "Stale completion cannot run the target provider again";
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), startOffset_ + direction_ * (approach_ + finalDistance_), 0.01f);
    EXPECT_EQ(approachCalls_, 1);
    EXPECT_EQ(snapCalls_, 1);
    EXPECT_EQ(scrollStops_, 1);
}

INSTANTIATE_TEST_SUITE_P(ProviderBudget, GridProviderFlingBudgetTest,
    testing::Combine(testing::Values(0.5f, 1.0f, 2.0f), testing::Bool()));


HWTEST_F(GridItemSnapTakeoverTest, ProviderSlowGestureUsesPreparedFlingFriction, TestSize.Level1)
{
    double natural = 0.0;
    auto strategy = MakeProviderStrategy([](double) { return 400.0; });
    strategy.calculateApproachOffset = [&natural](double, double decay) {
        natural = decay;
        return 40.0;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    scroll->isSlow_ = true;
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1000.0f));
    const auto friction = scroll->slidInfo_.friction;
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(MockAnimationManager::GetInstance().params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_NEAR(curve->GetResponse(), std::abs(2.0 * ACE_PI / (FRICTION_SCALE * friction)), 0.0001f);
    EXPECT_NEAR(natural, 1000.0 / std::abs(FRICTION_SCALE * friction), 0.001);
    scroll->HandleTouchDown();
    FinishSnap();
}


class GridProviderEndpointTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<bool> {};

HWTEST_P(GridProviderEndpointTest, ProviderEndpointCompletion, TestSize.Level1)
{
    const bool interrupted = GetParam();
    int32_t snapCalls = 0;
    double approach = 0.0;
    double seenOffset = 0.0;
    auto strategy = MakeProviderStrategy([this, &snapCalls, &seenOffset](double) {
        ++snapCalls;
        seenOffset = pattern_->GetTotalOffset();
        return 600.0;
    });
    strategy.calculateApproachOffset = [&approach](double, double decay) {
        approach = decay;
        return approach;
    };
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetScrollSnapStrategy(strategy);
    CreateGridItems(SNAP_GRID_ITEM_NUMBER, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    auto scroll = pattern_->GetScrollable();
    ASSERT_TRUE(scroll->StartScrollAnimation(0.0f, -1000.0f));
    auto finish = MockAnimationManager::GetInstance().params_.callbacks.finishCb;
    ASSERT_TRUE(finish);
    const float remaining = SNAP_SPRING_MINIMUM_AMPLITUDE_PX / 2.0f;
    DeliverSnapFrame(scroll, -approach + remaining);
    EXPECT_EQ(snapCalls, 0);
    EXPECT_FALSE(executor_->HasPendingTask());
    if (interrupted) {
        // A boundary can terminate the property animation before its actual target is available.
        scroll->StopSnapAnimation();
    }
    finish();
    executor_->Drain();
    EXPECT_EQ(snapCalls, 1);
    EXPECT_NEAR(seenOffset, approach - (interrupted ? remaining : 0.0f), 0.01);
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 600.0f, 0.01f);
    EXPECT_EQ(snapCalls, 1);
}

INSTANTIATE_TEST_SUITE_P(ProviderEndpoint, GridProviderEndpointTest, testing::Bool());

class GridScrollBarSnapTest : public GridItemSnapTakeoverTest,
    public testing::WithParamInterface<std::tuple<ScrollSnapAlign, bool, float>> {
public:
    void TearDown() override
    {
        // The legacy Animator mock retains motions; observers must not outlive this fixture.
        if (eventHub_) {
            eventHub_->SetOnWillScroll(nullptr);
            eventHub_->SetOnScrollStop(nullptr);
        }
        if (pattern_) {
            auto scroll = pattern_->GetScrollable();
            scroll->onDidStopDraggingCallback_ = nullptr;
            scroll->onWillStartFlingCallback_ = nullptr;
            scroll->onDidStopFlingCallback_ = nullptr;
            pattern_->StopScrollableAndAnimate();
            auto bar = pattern_->GetScrollBar();
            if (bar && bar->frictionController_ && bar->frictionController_->IsRunning()) {
                bar->frictionController_->Stop();
            }
        }
        GridItemSnapTakeoverTest::TearDown();
    }

    void CreateBarGrid(const ScrollSnapStrategy& strategy)
    {
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr");
        model.SetScrollBarMode(DisplayMode::ON);
        model.SetScrollSnapStrategy(strategy);
        model.SetOnScrollStop([this]() { ++scrollStops_; });
        CreateGridItems(SCROLL_BAR_ITEM_COUNT, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
        CreateDone();
        UpdateCurrentOffset(-3000.0f);
        auto scroll = pattern_->GetScrollable();
        scroll->onDidStopDraggingCallback_ = [this](bool fling) {
            events_.push_back(fling ? ScrollBarEvent::DRAG_STOP_WITH_FLING : ScrollBarEvent::DRAG_STOP);
        };
        scroll->onWillStartFlingCallback_ = [this]() { events_.push_back(ScrollBarEvent::FLING_START); };
        scroll->onDidStopFlingCallback_ = [this]() { ++flingStops_; };
        scroll->maxFlingVelocity_ = SCROLL_BAR_TEST_VELOCITY_LIMIT;
        scrollStops_ = 0;
        MockAnimationManager::GetInstance().SetTicks(TAKEOVER_ANIMATION_TICKS);
    }

    void ReleaseBar(bool touch, float velocity)
    {
        auto bar = pattern_->GetScrollBar();
        ASSERT_NE(bar, nullptr);
        GestureEvent info;
        info.SetInputEventType(touch ? InputEventType::TOUCH_SCREEN : InputEventType::MOUSE_BUTTON);
        info.SetGlobalPoint(Point(0.0f, 100.0f));
        if (physicalTouch_) {
            pattern_->GetScrollable()->HandleTouchDown();
        }
        bar->HandleDragStart(info);
        info.SetMainDelta(velocity > 0.0f ? 2.0f : -2.0f);
        bar->HandleDragUpdate(info);
        FlushUITasks();
        releaseOffset_ = pattern_->GetTotalOffset();
        contentVelocity_ = bar->CalcPatternOffset(velocity);
        eventHub_->SetOnWillScroll([this](Dimension offset, ScrollState, ScrollSource source) {
            if (!NearZero(offset.Value())) {
                motionSources_.push_back(source);
            }
            return ScrollFrameResult { offset };
        });
        info.SetGlobalPoint(Point(0.0f, 100.0f + info.GetMainDelta()));
        info.SetMainVelocity(velocity);
        if (physicalTouch_) {
            pattern_->GetScrollable()->HandleTouchUp();
            competingSnap_ = pattern_->GetScrollable()->IsSnapAnimationRunning();
        }
        bar->HandleDragEnd(info);
    }

protected:
    bool physicalTouch_ = false;
    bool competingSnap_ = false;
    std::vector<ScrollBarEvent> events_;
    std::vector<ScrollSource> motionSources_;
    float releaseOffset_ = 0.0f;
    float contentVelocity_ = 0.0f;
    int32_t approachCalls_ = 0;
    int32_t snapCalls_ = 0;
    float decisionVelocity_ = 0.0f;
};

/**
 * @tc.name: InnerScrollBarBuiltInFling
 * @tc.desc: The real scrollbar release transfers its budget to tracked friction and settles on an anchor.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarBuiltInFling, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    ReleaseBar(touch, velocity);
    auto scroll = pattern_->GetScrollable();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FLOAT_EQ(scroll->GetCurrentPos(), -releaseOffset_);
    EXPECT_NEAR(scroll->initVelocity_, contentVelocity_, 0.01f);
    auto bar = pattern_->GetScrollBar();
    const float budget = contentVelocity_ / (-FRICTION_SCALE * scroll->GetFlingFriction());
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(MockAnimationManager::GetInstance().params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_NEAR(curve->GetResponse(), 2.0 * ACE_PI / (-FRICTION_SCALE * scroll->GetFlingFriction()), 0.0001f);
    EXPECT_FLOAT_EQ(curve->GetDampingRatio(), 1.0f);
    EXPECT_NEAR(scroll->GetFinalPosition(), -releaseOffset_ + budget, 0.01f);
    EXPECT_FALSE(bar->frictionController_ && bar->frictionController_->IsRunning());
    EXPECT_EQ(events_,
        (std::vector<ScrollBarEvent> { ScrollBarEvent::DRAG_STOP_WITH_FLING, ScrollBarEvent::FLING_START }));
    frictionRate_ = std::abs(contentVelocity_ / budget);
    const float direction = velocity > 0.0f ? 1.0f : -1.0f;
    for (int32_t i = 0; i < TAKEOVER_MAX_FLING_FRAMES &&
        scroll->state_ == Scrollable::AnimationState::FRICTION; ++i) {
        const float before = pattern_->GetTotalOffset();
        const float end = scroll->GetFinalPosition();
        DeliverFrame(scroll, end + (scroll->GetCurrentPos() - end) * std::exp(-frictionRate_ * 0.016f));
        EXPECT_GE(direction * (pattern_->GetTotalOffset() - before), -0.01f);
        EXPECT_LE(std::abs(pattern_->GetTotalOffset() - before), std::abs(contentVelocity_) * 0.016f + 1.0f);
    }
    EXPECT_EQ(flingStops_, 0);
    FinishSnap();
    VerifyVisibleAnchor(align);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
    EXPECT_FALSE(scroll->isScrollBarDragging_);
    ASSERT_FALSE(motionSources_.empty());
    EXPECT_TRUE(std::all_of(motionSources_.begin(), motionSources_.end(),
        [](ScrollSource source) { return source == ScrollSource::SCROLL_BAR_FLING; }));
    EXPECT_FALSE(scroll->isUserFling_);
}

/**
 * @tc.name: InnerScrollBarProviderFling
 * @tc.desc: Scrollbar lifecycle dispatch starts both provider stages with content velocity exactly once.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarProviderFling, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    auto strategy = MakeProviderStrategy([this](double speed) {
        ++snapCalls_;
        decisionVelocity_ = speed;
        return releaseOffset_ + (speed > 0.0 ? 600.0f : -600.0f);
    });
    strategy.calculateApproachOffset = [this, snapAlign = align](double speed, double decay) {
        ++approachCalls_;
        EXPECT_EQ(events_,
            (std::vector<ScrollBarEvent> { ScrollBarEvent::DRAG_STOP_WITH_FLING, ScrollBarEvent::FLING_START }));
        EXPECT_NEAR(speed, -contentVelocity_, 0.01f);
        EXPECT_NEAR(decay, std::abs(contentVelocity_) /
            (-FRICTION_SCALE * pattern_->GetScrollable()->GetFlingFriction()), 0.01f);
        const float approach = snapAlign == ScrollSnapAlign::START ? 0.0f :
            static_cast<float>(decay * (snapAlign == ScrollSnapAlign::CENTER ? 0.5f : 1.0f));
        return approach;
    };
    CreateBarGrid(strategy);
    ReleaseBar(touch, velocity);
    auto scroll = pattern_->GetScrollable();
    ASSERT_EQ(approachCalls_, 1);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    EXPECT_FALSE(scroll->pendingItemSnapKickoff_);
    FinishSnap();
    EXPECT_EQ(snapCalls_, 1);
    EXPECT_NEAR(decisionVelocity_, -contentVelocity_, 0.01f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), releaseOffset_ + (velocity > 0.0f ? 600.0f : -600.0f), 0.01f);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
    EXPECT_FALSE(scroll->isScrollBarDragging_);
    ASSERT_FALSE(motionSources_.empty());
    EXPECT_TRUE(std::all_of(motionSources_.begin(), motionSources_.end(),
        [](ScrollSource source) { return source == ScrollSource::SCROLL_BAR_FLING; }));
}

/**
 * @tc.name: InnerScrollBarUnconfiguredFling
 * @tc.desc: Without item snap, the scrollbar retains its own friction controller and content scaling.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarUnconfiguredFling, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(ScrollSnapStrategy());
    ReleaseBar(touch, velocity);
    auto bar = pattern_->GetScrollBar();
    auto scroll = pattern_->GetScrollable();
    ASSERT_NE(bar->frictionController_, nullptr);
    EXPECT_TRUE(bar->frictionController_->IsRunning());
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::IDLE);
    EXPECT_FALSE(scroll->pendingItemSnapKickoff_);
    EXPECT_FALSE(scroll->needItemSnapSettle_);
    const float step = velocity > 0.0f ? 1.0f : -1.0f;
    const float contentStep = bar->CalcPatternOffset(step);
    bar->ProcessFrictionMotion(step);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetTotalOffset(), releaseOffset_ - contentStep, 0.01f);
    bar->frictionController_->Stop();
}

/**
 * @tc.name: InnerScrollBarProviderWithoutLifecycleListeners
 * @tc.desc: Provider startup and state cleanup do not require application drag/fling listeners.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarProviderWithoutLifecycleListeners, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    auto strategy = MakeProviderStrategy([this](double speed) {
        ++snapCalls_;
        return releaseOffset_ + (speed > 0.0 ? 100.0f : -100.0f);
    });
    strategy.calculateApproachOffset = [](double, double) { return 0.0; };
    CreateBarGrid(strategy);
    auto scroll = pattern_->GetScrollable();
    scroll->onDidStopDraggingCallback_ = nullptr;
    scroll->onWillStartFlingCallback_ = nullptr;
    ReleaseBar(touch, velocity);
    ASSERT_EQ(snapCalls_, 1);
    EXPECT_TRUE(scroll->isUserFling_);
    FinishSnap();
    EXPECT_FALSE(scroll->isUserFling_);
    EXPECT_FALSE(scroll->isScrollBarDragging_);
    EXPECT_EQ(flingStops_, 1);
}

/**
 * @tc.name: InnerScrollBarTouchCancelsProvider
 * @tc.desc: A new touch cancels the provider approach without allowing an old queued snap to move content.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarTouchCancelsProvider, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    auto strategy = MakeProviderStrategy([this](double speed) {
        ++snapCalls_;
        return releaseOffset_ + (speed > 0.0 ? 600.0f : -600.0f);
    });
    strategy.calculateApproachOffset = [](double, double) { return 100.0; };
    CreateBarGrid(strategy);
    ReleaseBar(touch, velocity);
    auto scroll = pattern_->GetScrollable();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::SNAP);
    const float delta = contentVelocity_ > 0.0f ? 50.0f : -50.0f;
    DeliverSnapFrame(scroll, scroll->GetCurrentPos() + delta);
    scroll->HandleTouchDown();
    const float offset = pattern_->GetTotalOffset();
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), offset, 0.01f);
    EXPECT_EQ(snapCalls_, 0);
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::IDLE);
}

/**
 * @tc.name: InnerScrollBarTouchCancelsBuiltInFling
 * @tc.desc: A touch interrupts the transferred scrollbar friction without a stale takeover or position jump.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarTouchCancelsBuiltInFling, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    ReleaseBar(touch, velocity);
    auto scroll = pattern_->GetScrollable();
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    DeliverFrame(scroll, scroll->GetCurrentPos() + contentVelocity_ * 0.016f);
    scroll->HandleTouchDown();
    const float offset = pattern_->GetTotalOffset();
    FinishSnap();
    EXPECT_NEAR(pattern_->GetTotalOffset(), offset, 0.01f);
    EXPECT_FALSE(scroll->needItemSnapSettle_);
    EXPECT_FALSE(scroll->IsItemSnapTracking());
    EXPECT_EQ(flingStops_, 0);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::IDLE);
}

/**
 * @tc.name: InnerScrollBarZeroBudgetSettles
 * @tc.desc: A nonzero release below scrollbar friction's threshold must still align between anchors.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarZeroBudgetSettles, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    ReleaseBar(touch, velocity > 0.0f ? 1.0f : -1.0f);
    auto scroll = pattern_->GetScrollable();
    auto bar = pattern_->GetScrollBar();
    ASSERT_FLOAT_EQ(bar->frictionMotion_->GetFinalPosition(), 0.0f);
    EXPECT_FALSE(scroll->needItemSnapSettle_);
    FinishSnap();
    VerifyVisibleAnchor(align);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_FALSE(scroll->isUserFling_);
    if (bar->frictionController_ && bar->frictionController_->IsRunning()) {
        bar->frictionController_->Stop();
    }
}

/**
 * @tc.name: InnerScrollBarBelowLegacyThresholdStillFlings
 * @tc.desc: A thumb release below 45 px/s still has a nonzero page budget after content scaling.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarBelowLegacyThresholdStillFlings, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    auto scroll = pattern_->GetScrollable();
    scroll->SetUnstaticFriction(0.7);
    ReleaseBar(touch, velocity > 0.0f ? 30.0f : -30.0f);
    ASSERT_FLOAT_EQ(pattern_->GetScrollBar()->frictionMotion_->GetFinalPosition(), 0.0f);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float budget = contentVelocity_ / (-FRICTION_SCALE * 0.7f);
    EXPECT_NEAR(scroll->GetFinalPosition(), -releaseOffset_ + budget, 0.01f);
    frictionRate_ = -FRICTION_SCALE * 0.7f;
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
    EXPECT_EQ(flingStops_, 1);
}

/**
 * @tc.name: InnerScrollBarPageVelocityLimit
 * @tc.desc: Apply the page velocity cap to the scaled content velocity without consecutive-fling gain.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarPageVelocityLimit, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    auto scroll = pattern_->GetScrollable();
    scroll->SetUnstaticFriction(0.7);
    scroll->maxFlingVelocity_ = 500.0;
    scroll->slipFactor_ = 20.0;
    ReleaseBar(touch, velocity);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float expected = contentVelocity_ > 0.0f ? 480.0f : -480.0f;
    EXPECT_FLOAT_EQ(scroll->initVelocity_, expected);
    EXPECT_NEAR(scroll->GetFinalPosition() + releaseOffset_, expected / (-FRICTION_SCALE * 0.7f), 0.01f);
    frictionRate_ = -FRICTION_SCALE * 0.7f;
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
}

/**
 * @tc.name: InnerScrollBarPassesLegacyEndpoint
 * @tc.desc: Reaching the old thumb-45px/s endpoint must not stop the page fling or fire stop events.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarPassesLegacyEndpoint, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    UpdateCurrentOffset(-1500.0f); // Keep both natural endpoints clear of the content boundaries.
    auto scroll = pattern_->GetScrollable();
    scroll->SetUnstaticFriction(0.3);
    ReleaseBar(touch, velocity);
    ASSERT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    const float oldBudget = std::abs(pattern_->GetScrollBar()->frictionMotion_->GetFinalPosition() *
        contentVelocity_ / velocity);
    frictionRate_ = -FRICTION_SCALE * 0.3f;
    for (int32_t i = 0; i < 100 && std::abs(pattern_->GetTotalOffset() - releaseOffset_) <= oldBudget; ++i) {
        const float end = scroll->GetFinalPosition();
        DeliverFrame(scroll, end + (scroll->GetCurrentPos() - end) * std::exp(-frictionRate_ * 0.016f));
    }
    EXPECT_GT(std::abs(pattern_->GetTotalOffset() - releaseOffset_), oldBudget);
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_EQ(flingStops_, 0);
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
}

/**
 * @tc.name: InnerScrollBarProviderUsesPageVelocityLimit
 * @tc.desc: Provider decision velocity and decay distance use the same capped page fling as built-in snap.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarProviderUsesPageVelocityLimit, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    auto strategy = MakeProviderStrategy([this](double speed) {
        ++snapCalls_;
        decisionVelocity_ = speed;
        return releaseOffset_ + (speed > 0.0 ? 100.0f : -100.0f);
    });
    strategy.calculateApproachOffset = [this](double speed, double decay) {
        ++approachCalls_;
        EXPECT_NEAR(std::abs(speed), 480.0, 0.01);
        EXPECT_NEAR(decay, 480.0 / (-FRICTION_SCALE * 0.7), 0.01);
        return 0.0;
    };
    CreateBarGrid(strategy);
    auto scroll = pattern_->GetScrollable();
    scroll->SetUnstaticFriction(0.7);
    scroll->maxFlingVelocity_ = 500.0;
    scroll->slipFactor_ = 20.0;
    ReleaseBar(touch, velocity);
    EXPECT_EQ(approachCalls_, 1);
    EXPECT_NEAR(decisionVelocity_, velocity > 0.0f ? 480.0f : -480.0f, 0.01f);
    FinishSnap();
    EXPECT_EQ(snapCalls_, 1);
    EXPECT_EQ(flingStops_, 1);
}

/**
 * @tc.name: InnerScrollBarRefreshesSlowState
 * @tc.desc: The previous gesture's slow flag cannot alter the next scrollbar release's page curve.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarRefreshesSlowState, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    auto scroll = pattern_->GetScrollable();
    const bool slow = align == ScrollSnapAlign::START;
    scroll->SetUnstaticFriction(-1.0);
    scroll->defaultFriction_ = 0.7;
    scroll->isSlow_ = !slow;
    const float magnitude = slow ? 30.0f : 150.0f;
    ReleaseBar(touch, velocity > 0.0f ? magnitude : -magnitude);
    EXPECT_EQ(scroll->isSlow_, slow);
    const float friction = slow ? 1.0f : 0.7f;
    auto curve = AceType::DynamicCast<ResponsiveSpringMotion>(MockAnimationManager::GetInstance().params_.curve);
    ASSERT_NE(curve, nullptr);
    EXPECT_NEAR(curve->GetResponse(), 2.0 * ACE_PI / (-FRICTION_SCALE * friction), 0.0001f);
    EXPECT_NEAR(scroll->GetFinalPosition() + releaseOffset_, contentVelocity_ / (-FRICTION_SCALE * friction), 0.01f);
    frictionRate_ = -FRICTION_SCALE * friction;
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
}

/**
 * @tc.name: InnerScrollBarPhysicalTouchRelease
 * @tc.desc: The content touch-up dispatched before scrollbar drag-end cannot start a competing snap.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarPhysicalTouchRelease, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    physicalTouch_ = true;
    ReleaseBar(touch, velocity);
    EXPECT_FALSE(competingSnap_);
    EXPECT_EQ(flingStops_, 0);
    auto scroll = pattern_->GetScrollable();
    frictionRate_ = -FRICTION_SCALE * scroll->GetFlingFriction();
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
}

/**
 * @tc.name: InnerScrollBarPhysicalProviderRelease
 * @tc.desc: Native touch-up/drag-end dispatch calls each provider once after the drag lifecycle.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarPhysicalProviderRelease, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    auto strategy = MakeProviderStrategy([this](double speed) {
        ++snapCalls_;
        return releaseOffset_ + (speed > 0.0 ? 100.0f : -100.0f);
    });
    strategy.calculateApproachOffset = [this](double, double decay) {
        ++approachCalls_;
        EXPECT_EQ(events_,
            (std::vector<ScrollBarEvent> { ScrollBarEvent::DRAG_STOP_WITH_FLING, ScrollBarEvent::FLING_START }));
        return decay * 0.5;
    };
    CreateBarGrid(strategy);
    physicalTouch_ = true;
    ReleaseBar(touch, velocity);
    EXPECT_FALSE(competingSnap_);
    EXPECT_EQ(approachCalls_, 1);
    EXPECT_EQ(flingStops_, 0);
    FinishSnap();
    EXPECT_EQ(snapCalls_, 1);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
}

/**
 * @tc.name: InnerScrollBarTouchUpAfterDragEnd
 * @tc.desc: The reverse native dispatch order also preserves the already-started scrollbar fling.
 */
HWTEST_P(GridScrollBarSnapTest, InnerScrollBarTouchUpAfterDragEnd, TestSize.Level1)
{
    auto [align, touch, velocity] = GetParam();
    CreateBarGrid(MakeStrategy(align));
    ReleaseBar(touch, velocity);
    auto scroll = pattern_->GetScrollable();
    scroll->HandleTouchDown(); // Interrupt the first fling before a new scrollbar release.
    ASSERT_TRUE(scroll->GetIsTouchStopAnimation());
    ReleaseBar(touch, velocity);
    const float end = scroll->GetFinalPosition();
    scroll->HandleTouchUp();
    EXPECT_FALSE(scroll->GetIsTouchStopAnimation());
    EXPECT_EQ(scroll->state_, Scrollable::AnimationState::FRICTION);
    EXPECT_FLOAT_EQ(scroll->GetFinalPosition(), end);
    EXPECT_EQ(flingStops_, 0);
    frictionRate_ = -FRICTION_SCALE * scroll->GetFlingFriction();
    AdvanceToTakeover(scroll);
    FinishSnap();
    VerifyVisibleAnchor(align);
    EXPECT_EQ(flingStops_, 1);
    EXPECT_EQ(scrollStops_, 1);
}

INSTANTIATE_TEST_SUITE_P(AlignInputDirection, GridScrollBarSnapTest,
    testing::Combine(testing::Values(ScrollSnapAlign::START, ScrollSnapAlign::CENTER, ScrollSnapAlign::END),
        testing::Bool(), testing::Values(-150.0f, 150.0f)));

} // namespace OHOS::Ace::NG

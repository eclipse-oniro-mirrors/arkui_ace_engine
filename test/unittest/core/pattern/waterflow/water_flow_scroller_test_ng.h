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

#ifndef FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_WATERFLOW_SCROLLER_TEST_NG_H
#define FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_WATERFLOW_SCROLLER_TEST_NG_H

#include "water_flow_test_ng.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"

namespace OHOS::Ace::NG {
class WaterFlowScrollerTestNg : public WaterFlowTestNg {
protected:
    static constexpr int32_t FAST_SCROLL_ITEM_COUNT = 1000;
    static constexpr int32_t FAST_SCROLL_TARGET = 800;
    static constexpr int32_t FAST_SCROLL_ANIMATION_TICKS = 8;
    static constexpr int32_t SHORT_ANIMATION_TICKS = 4;
    static constexpr int32_t MAX_FAST_SCROLL_TICKS = 40;
    static constexpr int32_t SETTLE_TICKS = 3;
    static constexpr int32_t PREPARATION_FRAME_BUDGET = 16;
    static constexpr int32_t MAX_PREPARATION_FRAMES = 100;
    static constexpr int32_t RETAINED_WINDOW_ITEMS = 32;
    static constexpr int32_t VISIBLE_ITEMS = 8;
    // Static route roots alive at once during preparation stay within five viewports of VISIBLE_ITEMS.
    static constexpr int32_t MAX_STATIC_ROUTE_ITEMS = 40;
    static constexpr int32_t MAX_MEASURED_ITEMS = 100;
    enum class DetachPhase { BEFORE_PREPARATION, RUNNING, AFTER_FINISH };
#ifdef TEST_WATER_FLOW_SW
    void CheckFastScrollRecycledTarget(FastScrollMode mode)
    {
        // Arrange: a cold lazy window, with no offscreen cache retained.
        auto model = CreateWaterFlow();
        model.SetCachedCount(0);
        auto lazyBuilder = CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t) { return 100.0f; });
        int32_t starts = 0;
        int32_t stops = 0;
        model.SetOnScrollStart([&starts]() { ++starts; });
        model.SetOnScrollStop([&stops]() { ++stops; });
        CreateDone();
        MockAnimationManager::GetInstance().SetTicks(1);

        // Act: prepare the distant target, then run production idle recycling.
        pattern_->ScrollToIndexFastScroll(FAST_SCROLL_TARGET, true, ScrollAlign::START, 0.0f, mode);
        FlushUITasks();
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
        PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
        auto lazyNode = AceType::DynamicCast<LazyForEachNode>(frameNode_->GetChildAtIndex(0));
        ASSERT_NE(lazyNode, nullptr);
        lazyNode->GetChildren();
        ASSERT_TRUE(lazyBuilder->PreBuild(INT64_MAX, std::nullopt, true));
        ASSERT_EQ(GetItem(FAST_SCROLL_TARGET), nullptr) << "The prepared target must actually have been recycled";
        // A delayed next animation frame can reach its endpoint in one update.
        FinishFastScroll();

        // Assert: real active geometry and one event pair, not only estimated offset.
        const auto item = GetItem(FAST_SCROLL_TARGET);
        ASSERT_NE(item, nullptr);
        EXPECT_TRUE(item->IsActive());
        EXPECT_NEAR(item->GetGeometryNode()->GetFrameRect().Top(), 0.0f, 0.2f);
        EXPECT_NEAR(item->GetGeometryNode()->GetFrameRect().Height(), 100.0f, 0.2f);
        EXPECT_EQ(starts, 1);
        EXPECT_EQ(stops, 1);
    }
    void CheckFastScrollPreparationYield(int32_t from, int32_t target);
    void CheckFastScrollStaticPreparation(int32_t from, int32_t target);
    void CheckFastScrollRepeatV2BuildWindow(int32_t from, int32_t target);
    void CheckFastScrollWork(int32_t count, FastScrollMode mode);
    void CheckFastScrollLargeOffsetMixedSizes(FastScrollMode mode);
    void CheckFastScrollDetach(DetachPhase phase);
    void CheckNewPageAfterDetach();
    void CheckFastScrollLayoutModeInvalidation();
    void CheckFastScrollInvalidIndex(FastScrollMode mode, int32_t index, bool running);
    void ContinueFastScrollPreparation(int32_t from, const RectF& initialRect, float initialOffset,
        const int32_t& created, const int32_t& starts);
#endif

    RectF FastScrollItemRect(int32_t index)
    {
        EXPECT_LE(pattern_->layoutInfo_->startIndex_, index);
        EXPECT_GE(pattern_->layoutInfo_->endIndex_, index);
        auto item = GetItem(index + (pattern_->layoutInfo_->footerIndex_ == 0 ? 1 : 0));
        EXPECT_NE(item, nullptr) << "Missing item " << index;
        return item ? item->GetGeometryNode()->GetFrameRect() : RectF();
    }

    void FinishFastScroll()
    {
        for (int32_t tick = 0;
             tick < MAX_FAST_SCROLL_TICKS && !MockAnimationManager::GetInstance().AllFinished(); ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks();
        }
        EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->fastScrollState_);
        EXPECT_FALSE(pattern_->AnimateRunning());
        // Observe queued layout/finish work before resetting mock bookkeeping. A
        // late production callback must not restart an already completed request.
        const float settledOffset = pattern_->GetTotalOffset();
        for (int32_t tick = 0; tick < SETTLE_TICKS; ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks();
            EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
            EXPECT_FALSE(pattern_->GetFastScrollRequest());
            EXPECT_FALSE(pattern_->fastScrollState_);
            EXPECT_FALSE(pattern_->AnimateRunning());
            EXPECT_NEAR(pattern_->GetTotalOffset(), settledOffset, 0.2f);
        }
        // AllFinished prunes the mock's animation list but its weak property map can
        // still reference the completed animation retained by the pattern.
        MockAnimationManager::GetInstance().Reset();
    }
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_WATERFLOW_SCROLLER_TEST_NG_H

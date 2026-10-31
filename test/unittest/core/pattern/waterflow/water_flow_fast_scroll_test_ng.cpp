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

#include "water_flow_scroller_test_ng.h"

#include <chrono>
#include <map>
#include <set>

#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include "core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_sw.h"
#include "core/components_ng/syntax/arkoala_lazy_node.h"
#include "core/components_ng/syntax/lazy_for_each_model_ng.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_node.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
#ifdef TEST_WATER_FLOW_SW
namespace {
struct StaticFastScrollPool {
    std::map<int32_t, RefPtr<UINode>> pool;
    std::set<int32_t> visited;
    int32_t created = 0;
    size_t peak = 0;

    RefPtr<UINode> Build(int32_t index)
    {
        visited.insert(index);
        auto found = pool.find(index);
        if (found != pool.end()) {
            return found->second;
        }
        ScopedViewStackProcessor scope;
        WaterFlowItemModelNG itemModel;
        itemModel.Create();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(100.0f));
        // LazyItemNode.getPeerPtr() returns the first peer (FlowItem here),
        // not an extra native syntax wrapper around that peer.
        auto root = ViewStackProcessor::GetInstance()->Finish();
        pool.emplace(index, root);
        peak = std::max(peak, pool.size());
        ++created;
        MockPipelineContext::GetCurrent()->DecResponseTime();
        return root;
    }

    void SetActiveRange(int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd)
    {
        for (auto item = pool.begin(); item != pool.end();) {
            if (item->first < start - cacheStart || item->first > end + cacheEnd) {
                // Drop frontend ownership; native node/cache lifetime is managed
                // by the real ArkoalaLazyNode, rather than manual test cleanup.
                item = pool.erase(item);
            } else {
                ++item;
            }
        }
    }
};

struct RepeatFastScrollPool {
    WeakPtr<RepeatVirtualScroll2Node> node;
    std::set<int32_t> requested;
    size_t peakNodes = 0;
    RIDType nextRid = 0;
    std::function<void()> createItem;

    std::pair<RIDType, uint32_t> Get(IndexType index)
    {
        requested.insert(index);
        if (auto repeat = node.Upgrade()) {
            for (const auto& [rid, item] : repeat->caches_.cacheItem4Rid_) {
                if (item && !item->isL1_) {
                    return { rid, OnGetRid4IndexResult::UPDATED_NODE };
                }
            }
            peakNodes = std::max(peakNodes, repeat->caches_.cacheItem4Rid_.size() + 1);
        }
        createItem();
        return { ++nextRid, OnGetRid4IndexResult::CREATED_NEW_NODE };
    }
};
} // namespace

void WaterFlowScrollerTestNg::CheckFastScrollStaticPreparation(int32_t from, int32_t target)
{
    // Real static native node and layout path. Only the ArkTS pool is represented
    // by callbacks that retain created roots and dispose them on range updates.
    auto model = CreateWaterFlow();
    model.SetCachedCount(0);
    auto tracker = std::make_shared<StaticFastScrollPool>();
    auto lazy = AceType::MakeRefPtr<ArkoalaLazyNode>(GetElmtId());
    lazy->SetTotalCount(FAST_SCROLL_ITEM_COUNT);
    lazy->SetCallbacks(
        [tracker](int32_t index, bool) { return tracker->Build(index); },
        [tracker](int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool) {
            tracker->SetActiveRange(start, end, cacheStart, cacheEnd);
        }, []() {}, [](int32_t) {});
    ViewStackProcessor::GetInstance()->Push(lazy);
    ViewStackProcessor::GetInstance()->Pop();
    auto events = std::make_shared<std::pair<int32_t, int32_t>>(0, 0);
    model.SetOnScrollStart([events]() { ++events->first; });
    model.SetOnScrollStop([events]() { ++events->second; });
    CreateDone();
    ScrollToIndex(from, false, ScrollAlign::START);
    auto context = MockPipelineContext::GetCurrent();
    context->OnIdle(INT64_MAX);
    const auto initialRect = FastScrollItemRect(from);
    const auto initialOffset = pattern_->GetTotalOffset();
    tracker->visited.clear();
    *events = { 0, 0 };
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);

    positionController_->ScrollToIndexWithOptions(target, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_MEASURE);
    ASSERT_NO_FATAL_FAILURE(
        ContinueFastScrollPreparation(from, initialRect, initialOffset, tracker->created, events->first));
    const int32_t firstUncached = target > from ? from + VISIBLE_ITEMS : target;
    const int32_t lastUncached = target > from ? target : from - 1;
    for (int32_t index = firstUncached; index <= lastUncached; ++index) {
        EXPECT_EQ(tracker->visited.count(index), 1u) << "Omitted intermediate index " << index;
    }
    // Route roots must reach the frontend for disposal in batches even though the
    // visible range never changes; otherwise the whole route stays alive.
    EXPECT_LE(tracker->peak, static_cast<size_t>(MAX_STATIC_ROUTE_ITEMS));
    EXPECT_EQ(events->first, 1);
    EXPECT_EQ(events->second, 0);
    // This case ends at animation start. Static frontend disposal and target
    // lifetime follow the shared ArkoalaLazyNode contract.
    context->SetResponseTime(INT32_MAX);
}

/**
 * @tc.name: WFFastScrollStaticPreparationForward
 * @tc.desc: Static forward mode-1 preparation yields and visits every uncached item.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollStaticPreparationForward, TestSize.Level1)
{
    CheckFastScrollStaticPreparation(0, 800);
}

/**
 * @tc.name: WFFastScrollStaticPreparationBackward
 * @tc.desc: Static backward mode-1 preparation yields while preserving the current view.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollStaticPreparationBackward, TestSize.Level1)
{
    CheckFastScrollStaticPreparation(900, 200);
}

/**
 * @tc.name: WFFastScrollStaticRangeContract
 * @tc.desc: Static route release relies on ArkoalaLazyNode forwarding a range whose only change is the
 *           trailing cache count, while an identical range is skipped. Guard that shared behavior here.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollStaticRangeContract, TestSize.Level1)
{
    auto lazy = AceType::MakeRefPtr<ArkoalaLazyNode>(GetElmtId());
    lazy->SetTotalCount(FAST_SCROLL_ITEM_COUNT);
    int32_t updates = 0;
    lazy->SetCallbacks([](int32_t, bool) { return RefPtr<UINode>(); },
        [&updates](int32_t, int32_t, int32_t, int32_t, bool) { ++updates; }, []() {}, [](int32_t) {});
    lazy->DoSetActiveChildRange(0, VISIBLE_ITEMS - 1, 0, 0, false);
    ASSERT_EQ(updates, 1);
    lazy->DoSetActiveChildRange(0, VISIBLE_ITEMS - 1, 0, 0, false);
    EXPECT_EQ(updates, 1) << "Identical range is expected to be skipped";
    lazy->DoSetActiveChildRange(0, VISIBLE_ITEMS - 1, 0, 1, false);
    EXPECT_EQ(updates, 2) << "A cache-only change must still reach the frontend; WaterFlow static "
                             "fast-scroll preparation depends on it to dispose route roots";
    lazy->DoSetActiveChildRange(0, VISIBLE_ITEMS - 1, 0, 0, false);
    EXPECT_EQ(updates, 3);
}

/**
 * @tc.name: WFFastScrollEstimationEndsWithRequest
 * @tc.desc: A request that never applies a jump window must not keep average-only height estimation afterwards.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollEstimationEndsWithRequest, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t index) { return index % 2 ? 300.0f : 100.0f; });
    CreateDone();
    auto info = AceType::DynamicCast<WaterFlowLayoutInfoSW>(pattern_->layoutInfo_);
    ASSERT_NE(info, nullptr);
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    // The target is visible, so no jump window replaces the measured height cache.
    pattern_->ScrollToIndexFastScroll(2, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    EXPECT_TRUE(info->IsFastScrollEstimating());
    FinishFastScroll();
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_FALSE(info->IsFastScrollEstimating());
}

/**
 * @tc.name: WFFastScrollEstimateAcrossSectionMargins
 * @tc.desc: The estimated position of an item in a later section includes that section's leading margin.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollEstimateAcrossSectionMargins, TestSize.Level1)
{
    constexpr int32_t target = 500;
    constexpr int32_t firstCount = 200;
    CreateWaterFlow();
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t) { return 100.0f; });
    // Different leading margins expose a missing margin of the target section.
    MarginProperty first;
    first.top = CalcLength(30.0f);
    first.bottom = CalcLength(20.0f);
    MarginProperty second;
    second.top = CalcLength(70.0f);
    second.bottom = CalcLength(20.0f);
    std::vector<WaterFlowSections::Section> data(2);
    data[0].itemsCount = firstCount;
    data[0].crossCount = 1;
    data[0].margin = first;
    data[1].itemsCount = FAST_SCROLL_ITEM_COUNT - firstCount;
    data[1].crossCount = 1;
    data[1].margin = second;
    pattern_->GetOrCreateWaterFlowSections()->ChangeData(0, 0, data);
    CreateDone();
    auto info = AceType::DynamicCast<WaterFlowLayoutInfoSW>(pattern_->layoutInfo_);
    ASSERT_NE(info, nullptr);
    const auto estimated = info->EstimateFastScrollPosition(target);
    ASSERT_TRUE(estimated.has_value());
    // Uniform items make the content position exact: first section with both margins, then the
    // leading margin of the second section and its items before the target.
    const float expected = 30.0f + firstCount * 100.0f + 20.0f + 70.0f + (target - firstCount) * 100.0f;
    EXPECT_NEAR(*estimated, expected, 1.0f);
}

/**
 * @tc.name: WFFastScrollNonFiniteOffsetStopsPrior
 * @tc.desc: A non-finite extraOffset is rejected like an invalid index and stops the running request.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollNonFiniteOffsetStopsPrior, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t) { return 100.0f; });
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    const auto previous = pattern_->fastScrollState_;
    ASSERT_NE(previous, nullptr);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    const float before = pattern_->GetTotalOffset();
    pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::START, std::numeric_limits<float>::quiet_NaN(),
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_TRUE(previous->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before);
}

void WaterFlowScrollerTestNg::ContinueFastScrollPreparation(int32_t from, const RectF& initialRect,
    float initialOffset, const int32_t& created, const int32_t& starts)
{
    auto context = MockPipelineContext::GetCurrent();
    // Flush only the tasks queued by production. Manually marking the WaterFlow
    // dirty here would conceal a missing continuation after the first yield.
    for (int32_t frame = 0; frame < MAX_PREPARATION_FRAMES && !pattern_->AnimateRunning(); ++frame) {
        const int32_t frameBefore = created;
        context->SetResponseTime(PREPARATION_FRAME_BUDGET);
        context->FlushUITasks();
        if (!pattern_->AnimateRunning()) {
            EXPECT_LE(created - frameBefore, PREPARATION_FRAME_BUDGET);
            EXPECT_EQ(starts, 0);
            EXPECT_EQ(FastScrollItemRect(from), initialRect);
            EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), initialOffset);
        }
    }
    ASSERT_TRUE(pattern_->AnimateRunning()) << "The queued continuation must eventually prepare the target";
}

void WaterFlowScrollerTestNg::CheckFastScrollPreparationYield(int32_t from, int32_t target)
{
    // Model each frontend build as one unit of response budget. The real LazyForEach,
    // layout, asynchronous continuation and scroll animation paths remain in use.
    auto model = CreateWaterFlow();
    model.SetCachedCount(0);
    auto built = std::make_shared<std::set<int32_t>>();
    auto created = std::make_shared<int32_t>(0);
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [built, created](int32_t index) {
        built->insert(index);
        ++*created;
        MockPipelineContext::GetCurrent()->DecResponseTime();
        return 100.0f;
    });
    int32_t starts = 0;
    int32_t stops = 0;
    model.SetOnScrollStart([&starts]() { ++starts; });
    model.SetOnScrollStop([&stops]() { ++stops; });
    CreateDone();
    ScrollToIndex(from, false, ScrollAlign::START);
    const auto initialRect = FastScrollItemRect(from);
    const auto initialOffset = pattern_->GetTotalOffset();
    const int32_t first = std::min(from, target);
    const int32_t last = std::max(pattern_->layoutInfo_->endIndex_, target);
    starts = stops = 0;
    auto context = MockPipelineContext::GetCurrent();
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    const int32_t before = *created;
    context->SetResponseTime(PREPARATION_FRAME_BUDGET);

    positionController_->ScrollToIndexWithOptions(target, true, ScrollAlign::CENTER, 20.0f,
        FastScrollMode::SKIP_MEASURE);
    context->FlushUITasks();

    EXPECT_GT(*created, before);
    EXPECT_LE(*created - before, PREPARATION_FRAME_BUDGET)
        << "Preparation must yield when frontend work consumes the budget";
    ASSERT_FALSE(pattern_->AnimateRunning()) << "Do not start an animation before preparation completes";
    EXPECT_EQ(starts, 0);
    EXPECT_EQ(stops, 0);
    EXPECT_EQ(FastScrollItemRect(from), initialRect);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), initialOffset);

    ASSERT_NO_FATAL_FAILURE(ContinueFastScrollPreparation(from, initialRect, initialOffset, *created, starts));
    for (int32_t index = first; index <= last; ++index) {
        EXPECT_EQ(built->count(index), 1u) << "Skip-measure omitted intermediate index " << index;
    }
    context->SetResponseTime(INT32_MAX);
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(target).Top(), 330.0f, 0.2f);
    EXPECT_NEAR(FastScrollItemRect(target).Height(), 100.0f, 0.2f);
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
}

/**
 * @tc.name: WFFastScrollPreparationYieldForward
 * @tc.desc: Budgeted mode-1 preparation resumes every intermediate build and reaches CENTER with +20px offset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollPreparationYieldForward, TestSize.Level1)
{
    CheckFastScrollPreparationYield(0, 800);
}

/**
 * @tc.name: WFFastScrollPreparationYieldBackward
 * @tc.desc: Backward preparation yields without moving the visible window and retains every intermediate build.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollPreparationYieldBackward, TestSize.Level1)
{
    CheckFastScrollPreparationYield(900, 200);
}

/**
 * @tc.name: WFFastScrollPreparationYieldCancel
 * @tc.desc: A touch cancels a yielded request; its queued task cannot restart it or interfere with the next request.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollPreparationYieldCancel, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    auto created = std::make_shared<int32_t>(0);
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [created](int32_t) {
        ++*created;
        MockPipelineContext::GetCurrent()->DecResponseTime();
        return 100.0f;
    });
    int32_t starts = 0;
    int32_t stops = 0;
    model.SetOnScrollStart([&starts]() { ++starts; });
    model.SetOnScrollStop([&stops]() { ++stops; });
    CreateDone();
    auto context = MockPipelineContext::GetCurrent();
    context->SetResponseTime(PREPARATION_FRAME_BUDGET);
    positionController_->ScrollToIndexWithOptions(800, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_MEASURE);
    context->FlushUITasks();
    ASSERT_LT(*created, 100) << "The long route must remain pending after one frame";
    ASSERT_FALSE(pattern_->AnimateRunning());

    TouchEventInfo touch("touch");
    pattern_->OnTouchDown(touch);
    context->SetResponseTime(INT32_MAX);
    for (int32_t frame = 0; frame < 3; ++frame) {
        context->FlushUITasks();
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    }
    EXPECT_EQ(starts, 0);
    EXPECT_EQ(stops, 0);
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    positionController_->ScrollToIndexWithOptions(100, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    context->FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 0.0f, 0.2f);
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
}

void WaterFlowScrollerTestNg::CheckFastScrollRepeatV2BuildWindow(int32_t from, int32_t target)
{
    // Arrange: the real Repeat v2 node/cache path, with a frontend adapter that
    // creates equal-height items and reuses only nodes released from native L1.
    auto model = CreateWaterFlow();
    model.SetCachedCount(0);
    auto tracker = std::make_shared<RepeatFastScrollPool>();
    tracker->createItem = [this]() { CreateItemWithHeight(100.0f); };
    auto events = std::make_shared<std::pair<int32_t, int32_t>>(0, 0);
    auto onGet = [tracker](IndexType index, bool, bool) { return tracker->Get(index); };
    auto repeat = RepeatVirtualScroll2Node::GetOrCreateRepeatNode(
        GetElmtId(), FAST_SCROLL_ITEM_COUNT, FAST_SCROLL_ITEM_COUNT, 0, onGet,
        [](IndexType, IndexType) {}, [](int32_t, int32_t, int32_t, int32_t, bool, bool) {},
        [](IndexType, IndexType) {}, []() {}, []() {}, []() {});
    tracker->node = WeakPtr<RepeatVirtualScroll2Node>(repeat);
    ViewStackProcessor::GetInstance()->Push(repeat);
    ViewStackProcessor::GetInstance()->Pop();
    model.SetOnScrollStart([events]() { ++events->first; });
    model.SetOnScrollStop([events]() { ++events->second; });
    CreateDone();
    ScrollToIndex(from, false, ScrollAlign::START);
    const auto initialRect = FastScrollItemRect(from);
    const auto initialOffset = pattern_->GetTotalOffset();
    tracker->requested.clear();
    tracker->peakNodes = repeat->caches_.cacheItem4Rid_.size();
    *events = { 0, 0 };
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);

    // Act: use the production request and layout path; no manual recycle call.
    pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks();

    // Eight 100px rows fill the 800px viewport. Preparation may retain nearby
    // windows, but must not accumulate hundreds of intermediate Repeat nodes.
    EXPECT_LE(tracker->peakNodes, RETAINED_WINDOW_ITEMS);
    const int32_t firstUncached = target > from ? from + VISIBLE_ITEMS : target;
    const int32_t lastUncached = target > from ? target : from - 1;
    for (int32_t index = firstUncached; index <= lastUncached; ++index) {
        EXPECT_EQ(tracker->requested.count(index), 1u) << "Skip-measure omitted intermediate index " << index;
    }
    EXPECT_EQ(FastScrollItemRect(from), initialRect);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), initialOffset);
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(target).Top(), 0.0f, 0.2f);
    EXPECT_NEAR(FastScrollItemRect(target).Height(), 100.0f, 0.2f);
    EXPECT_EQ(events->first, 1);
    EXPECT_EQ(events->second, 1);
}

/**
 * @tc.name: WFFastScrollRepeatV2BuildWindowForward
 * @tc.desc: Skip-measure visits every intermediate index with bounded Repeat v2 retention and correct arrival.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollRepeatV2BuildWindowForward, TestSize.Level1)
{
    CheckFastScrollRepeatV2BuildWindow(0, 800);
}

/**
 * @tc.name: WFFastScrollRepeatV2BuildWindowBackward
 * @tc.desc: Backward preparation keeps the current window and reuses offscreen Repeat v2 nodes before animation.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollRepeatV2BuildWindowBackward, TestSize.Level1)
{
    CheckFastScrollRepeatV2BuildWindow(900, 200);
}

void WaterFlowScrollerTestNg::CheckFastScrollLargeOffsetMixedSizes(FastScrollMode mode)
{
    constexpr int32_t HEIGHT_CHANGE_INDEX = 400;
    constexpr int32_t ARRIVAL_INDEX = 600;
    constexpr int32_t MIN_OFFSET_REGION_ITEMS = 200;
    constexpr int32_t MAX_OFFSET_REGION_ITEMS = 220;
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr");
    auto created = std::make_shared<std::set<int32_t>>();
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [created](int32_t index) {
        created->insert(index);
        return index < HEIGHT_CHANGE_INDEX ? 100.0f : 200.0f;
    });
    CreateDone();
    const int32_t before = created->size();
    MockAnimationManager::GetInstance().SetTicks(SHORT_ANIMATION_TICKS);
    pattern_->ScrollToIndexFastScroll(FAST_SCROLL_TARGET, true, ScrollAlign::START, -40000.0f, mode);
    FlushUITasks();
    const int32_t preparationCreated = created->size() - before;
    RecordProperty("preparation_created", preparationCreated);
    if (mode == FastScrollMode::SKIP_CREATE_AND_MEASURE) {
        EXPECT_GE(preparationCreated, MIN_OFFSET_REGION_ITEMS);
        EXPECT_LE(preparationCreated, MAX_OFFSET_REGION_ITEMS);
    }
    FinishFastScroll();
    // Item 800 starts at 120000px; subtracting 40000px reaches item 600 at 80000px.
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, ARRIVAL_INDEX);
    EXPECT_NEAR(FastScrollItemRect(ARRIVAL_INDEX).Top(), 0.0f, 0.2f);
    if (mode == FastScrollMode::SKIP_CREATE_AND_MEASURE) {
        for (int32_t index = RETAINED_WINDOW_ITEMS; index < ARRIVAL_INDEX - RETAINED_WINDOW_ITEMS; ++index) {
            EXPECT_EQ(created->count(index), 0u) << index;
        }
    }
}

/**
 * @tc.name: WFFastScrollLargeOffsetMixedSizesMode1
 * @tc.desc: Mode 1 uses actual geometry across a large extraOffset with different item heights.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollLargeOffsetMixedSizesMode1, TestSize.Level1)
{
    CheckFastScrollLargeOffsetMixedSizes(FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: WFFastScrollLargeOffsetMixedSizesMode2
 * @tc.desc: Mode 2 measures the offset region accurately while skipping unrelated intermediate items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollLargeOffsetMixedSizesMode2, TestSize.Level1)
{
    CheckFastScrollLargeOffsetMixedSizes(FastScrollMode::SKIP_CREATE_AND_MEASURE);
}

namespace {
struct FastScrollWorkCounts {
    std::set<int32_t> created;
    std::set<int32_t> measured;
    std::set<int32_t> everCreated;
    uint64_t createCalls = 0;
    uint64_t measureCalls = 0;

    static constexpr int32_t WINDOW_MARGIN = 32;
    static constexpr int32_t MAX_CREATED_ITEMS = 100;

    void CheckIntermediate(int32_t target, FastScrollMode mode, const std::string& key) const
    {
        // Uniform 100px items, 800px viewport, zero cachedCount: 32 items on each
        // side conservatively include both animated visible windows and preparation.
        int32_t intermediateCreated = 0;
        int32_t intermediateMeasured = 0;
        for (int32_t index : created) {
            intermediateCreated += index >= WINDOW_MARGIN && index < target - WINDOW_MARGIN;
        }
        for (int32_t index : measured) {
            intermediateMeasured += index >= WINDOW_MARGIN && index < target - WINDOW_MARGIN;
        }
        EXPECT_EQ(intermediateMeasured, 0);
        if (mode == FastScrollMode::SKIP_CREATE_AND_MEASURE) {
            EXPECT_EQ(intermediateCreated, 0);
            EXPECT_LT(created.size(), MAX_CREATED_ITEMS);
        } else {
            for (int32_t index = 0; index <= target; ++index) {
                ASSERT_EQ(everCreated.count(index), 1);
            }
        }
        testing::Test::RecordProperty("total_created_" + key, std::to_string(created.size()));
        testing::Test::RecordProperty("total_measured_" + key, std::to_string(measured.size()));
        testing::Test::RecordProperty("total_create_calls_" + key, std::to_string(createCalls));
        testing::Test::RecordProperty("total_measure_calls_" + key, std::to_string(measureCalls));
        EXPECT_GE(createCalls, created.size());
        EXPECT_GE(measureCalls, measured.size());
        testing::Test::RecordProperty("intermediate_created_" + key, std::to_string(intermediateCreated));
        testing::Test::RecordProperty("intermediate_measured_" + key, std::to_string(intermediateMeasured));
    }
};

class FastScrollCountingLazy : public WaterFlowMockLazy {
public:
    FastScrollCountingLazy(int32_t count, const std::shared_ptr<FastScrollWorkCounts>& work)
        : WaterFlowMockLazy(count, [](int32_t) { return 100.0f; }), work_(work)
    {}

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        auto result = WaterFlowMockLazy::OnGetChildByIndex(index, expiringItems);
        ++work_->createCalls;
        work_->created.insert(index);
        work_->everCreated.insert(index);
        auto frame = AceType::DynamicCast<FrameNode>(result.second);
        frame->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [work = work_, index](RefPtr<Kit::FrameNode>&) {
                ++work->measureCalls;
                work->measured.insert(index);
            });
        return result;
    }

private:
    std::shared_ptr<FastScrollWorkCounts> work_;
};
} // namespace

/**
 * @tc.name: WFFastScrollIndexCounts
 * @tc.desc: Exact skipped-index creation/measurement counts remain zero over 1k/10k/100k spans.
 * @tc.type: FUNC
 */
void WaterFlowScrollerTestNg::CheckFastScrollWork(int32_t count, FastScrollMode mode)
{
    auto model = CreateWaterFlow();
    model.SetCachedCount(0);
    auto work = std::make_shared<FastScrollWorkCounts>();
    auto lazy = AceType::MakeRefPtr<FastScrollCountingLazy>(count, work);
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    RefPtr<LazyForEachActuator> actuator = lazy;
    LazyForEachModelNG().Create(actuator);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateDone();
    constexpr int32_t TARGET_NUMERATOR = 4;
    constexpr int32_t TARGET_DENOMINATOR = 5;
    const int32_t target = count * TARGET_NUMERATOR / TARGET_DENOMINATOR;
    ScrollToIndex(0, false, ScrollAlign::START);
    work->created.clear();
    work->measured.clear();
    work->createCalls = 0;
    work->measureCalls = 0;
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    const auto started = std::chrono::steady_clock::now();
    pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::CENTER, 0.0f, mode);
    FlushUITasks();
    const auto preparation = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - started).count();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    EXPECT_LT(work->measured.size(), MAX_MEASURED_ITEMS);
    const auto key = std::to_string(static_cast<int32_t>(mode)) + "_" + std::to_string(target);
    RecordProperty("preparation_us_" + key, std::to_string(preparation));
    RecordProperty("preparation_created_" + key, std::to_string(work->created.size()));
    RecordProperty("preparation_measured_" + key, std::to_string(work->measured.size()));
    RecordProperty("preparation_create_calls_" + key, std::to_string(work->createCalls));
    RecordProperty("preparation_measure_calls_" + key, std::to_string(work->measureCalls));
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(target).Top(), 350.0f, 0.2f);
    EXPECT_LT(work->measured.size(), MAX_MEASURED_ITEMS);
    ASSERT_NO_FATAL_FAILURE(work->CheckIntermediate(target, mode, key));
}

/**
 * @tc.name: WFFastScrollIndexCountsMode1Span1000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode1Span1000, TestSize.Level1)
{
    CheckFastScrollWork(1000, FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: WFFastScrollIndexCountsMode1Span10000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode1Span10000, TestSize.Level1)
{
    CheckFastScrollWork(10000, FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: WFFastScrollIndexCountsMode1Span100000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode1Span100000, TestSize.Level1)
{
    CheckFastScrollWork(100000, FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: WFFastScrollIndexCountsMode2Span1000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode2Span1000, TestSize.Level1)
{
    CheckFastScrollWork(1000, FastScrollMode::SKIP_CREATE_AND_MEASURE);
}

/**
 * @tc.name: WFFastScrollIndexCountsMode2Span10000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode2Span10000, TestSize.Level1)
{
    CheckFastScrollWork(10000, FastScrollMode::SKIP_CREATE_AND_MEASURE);
}

/**
 * @tc.name: WFFastScrollIndexCountsMode2Span100000
 * @tc.desc: A fresh lazy source records skipped-index work at the specified scale.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollIndexCountsMode2Span100000, TestSize.Level1)
{
    CheckFastScrollWork(100000, FastScrollMode::SKIP_CREATE_AND_MEASURE);
}
void WaterFlowScrollerTestNg::CheckFastScrollLayoutModeInvalidation()
{
    constexpr int32_t nextTargetIndex = 100;
    pattern_->ScrollToIndexFastScroll(FAST_SCROLL_TARGET, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto old = pattern_->fastScrollState_;
    ASSERT_NE(old, nullptr);
    const auto layoutInfo = pattern_->layoutInfo_;
    const auto snapCount = pattern_->snapItemGeometryCache_.size();
    const auto snapHeight = pattern_->snapTotalHeight_;
    ASSERT_GT(snapCount, 0u);
    ASSERT_TRUE(snapHeight.has_value());

    // Replaying the same mode must preserve both the pending fast scroll and snap geometry.
    pattern_->SetLayoutMode(WaterFlowLayoutMode::SLIDING_WINDOW);
    EXPECT_EQ(pattern_->fastScrollState_, old);
    EXPECT_FALSE(old->cancelled);
    EXPECT_EQ(pattern_->layoutInfo_, layoutInfo);
    EXPECT_EQ(pattern_->snapItemGeometryCache_.size(), snapCount);
    EXPECT_EQ(pattern_->snapTotalHeight_, snapHeight);

    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(FAST_SCROLL_TARGET).Top(), 250.0f, 0.2f);
    pattern_->ScrollToIndexFastScroll(nextTargetIndex, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    old = pattern_->fastScrollState_;
    ASSERT_NE(old, nullptr);

    // A real mode switch invalidates both kinds of state before replacing the layout.
    pattern_->SetLayoutMode(WaterFlowLayoutMode::TOP_DOWN);
    EXPECT_TRUE(old->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->fastScrollState_);
    EXPECT_TRUE(pattern_->snapItemGeometryCache_.empty());
    EXPECT_FALSE(pattern_->snapTotalHeight_.has_value());
}

/**
 * @tc.name: WFFastScrollLayoutInvalidation
 * @tc.desc: Geometry changes and layout-mode changes discard a prepared window.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollLayoutInvalidation, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    ScrollSnapStrategy snap;
    snap.align = ScrollSnapAlign::START;
    model.SetScrollSnapStrategy(snap);
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    auto info = AceType::DynamicCast<WaterFlowLayoutInfoSW>(pattern_->layoutInfo_);
    ASSERT_NE(info, nullptr);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    auto old = pattern_->fastScrollState_;
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr");
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_TRUE(old->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->fastScrollState_);
    // The measurement that cancels the request no longer estimates heights from averages.
    EXPECT_FALSE(info->IsFastScrollEstimating());

    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    old = pattern_->fastScrollState_;
    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(600.0f)));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_TRUE(old->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(info->IsFastScrollEstimating());
    FinishFastScroll();
    pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 250.0f, 0.2f);

    ASSERT_NO_FATAL_FAILURE(CheckFastScrollLayoutModeInvalidation());
}

void WaterFlowScrollerTestNg::CheckNewPageAfterDetach()
{
    WeakPtr<FrameNode> oldNode = frameNode_;
    WeakPtr<WaterFlowPattern> oldPattern = pattern_;
    RemoveFromStageNode();
    frameNode_.Reset();
    pattern_.Reset();
    eventHub_.Reset();
    layoutProperty_.Reset();
    paintProperty_.Reset();
    accessibilityProperty_.Reset();
    positionController_.Reset();
    MockPipelineContext::GetCurrent()->FlushUITasks();
    EXPECT_FALSE(oldNode.Upgrade());
    EXPECT_FALSE(oldPattern.Upgrade());

    // A new page is independent of callbacks retained by the old animation.
    ClearOldNodes();
    CreateWaterFlow();
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t) { return 100.0f; });
    CreateDone();
    for (int32_t tick = 0; tick < SETTLE_TICKS; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
    }
    EXPECT_NEAR(FastScrollItemRect(0).Top(), 0.0f, 0.2f);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
}

// All detach phases use real node removal/reattachment and animation ticks.
void WaterFlowScrollerTestNg::CheckFastScrollDetach(DetachPhase phase)
{
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr");
    CreateItemsInLazyForEach(FAST_SCROLL_ITEM_COUNT, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN);
    ASSERT_TRUE(frameNode_->IsOnMainTree());
    MockAnimationManager::GetInstance().SetTicks(phase == DetachPhase::AFTER_FINISH ? 1 : SHORT_ANIMATION_TICKS);
    pattern_->ScrollToIndexFastScroll(FAST_SCROLL_TARGET, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto state = pattern_->fastScrollState_;
    ASSERT_NE(state, nullptr);
    if (phase != DetachPhase::BEFORE_PREPARATION) {
        FlushUITasks();
        ASSERT_TRUE(pattern_->AnimateRunning());
    }
    if (phase == DetachPhase::AFTER_FINISH) {
        MockAnimationManager::GetInstance().Tick(); // Deliberately do not flush the last layout yet.
        ASSERT_TRUE(state->finishing);
        ASSERT_FALSE(pattern_->GetFastScrollRequest());
    }

    // Act: production UINode/FrameNode detach dispatch, not a direct finish callback.
    RemoveFromStageNode();
    EXPECT_FALSE(frameNode_->IsOnMainTree());
    EXPECT_TRUE(state->cancelled);
    EXPECT_FALSE(pattern_->fastScrollState_);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());

    // A cached page may reattach the same node: queued work must not revive the old target.
    MountToStageNode(frameNode_);
    FlushUITasks();
    const float detachedOffset = pattern_->GetTotalOffset();
    for (int32_t tick = 0; tick < FAST_SCROLL_ANIMATION_TICKS; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        EXPECT_NEAR(pattern_->GetTotalOffset(), detachedOffset, 0.2f);
        EXPECT_FALSE(pattern_->fastScrollState_);
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->AnimateRunning());
    }
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    state.reset();
    ASSERT_NO_FATAL_FAILURE(CheckNewPageAfterDetach());
}

/**
 * @tc.name: WFFastScrollDetachBeforePrepare
 * @tc.desc: Removing a node cancels preparation and cannot revive it after reattach.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollDetachBeforePrepare, TestSize.Level1)
{
    CheckFastScrollDetach(DetachPhase::BEFORE_PREPARATION);
}

/**
 * @tc.name: WFFastScrollDetachAnimating
 * @tc.desc: Removing an animating node drains old work and releases the old page.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollDetachAnimating, TestSize.Level1)
{
    CheckFastScrollDetach(DetachPhase::RUNNING);
}

/**
 * @tc.name: WFFastScrollDetachAfterFinish
 * @tc.desc: Removal between a finish callback and final layout cancels the retained target.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollDetachAfterFinish, TestSize.Level1)
{
    CheckFastScrollDetach(DetachPhase::AFTER_FINISH);
}

/**
 * @tc.name: WFFastScrollRecycledTargetMode1
 * @tc.desc: Skip-measure restores a recycled target even when the next frame reaches the endpoint.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollRecycledTargetMode1, TestSize.Level1)
{
    CheckFastScrollRecycledTarget(FastScrollMode::SKIP_MEASURE);
}

/**
 * @tc.name: WFFastScrollRecycledTargetMode2
 * @tc.desc: Skip-create-and-measure restores real target geometry after idle recycling.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollRecycledTargetMode2, TestSize.Level1)
{
    CheckFastScrollRecycledTarget(FastScrollMode::SKIP_CREATE_AND_MEASURE);
}

/**
 * @tc.name: WFFastScrollAlignments
 * @tc.desc: Both fast modes settle at measured target geometry in both directions.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollAlignments, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr 1fr");
    CreateItemsInLazyForEach(1000, [](int32_t index) { return 100.0f + (index % 3) * 20.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO }) {
            for (int32_t index : { 800, 100 }) {
                for (float extra : { 0.0f, 20.0f, -20.0f }) {
                    SCOPED_TRACE(std::to_string(static_cast<int>(mode)) + ":" +
                        std::to_string(static_cast<int>(align)) + ":" + std::to_string(index) + ":" +
                        std::to_string(extra));
                    // Begin away from the destination for each alignment/offset combination.
                    ScrollToIndex(index == 800 ? 50 : 900, false, ScrollAlign::START);
                    MockAnimationManager::GetInstance().SetTicks(4);
                    pattern_->ScrollToIndexFastScroll(index, true, align, extra, mode);
                    FlushUITasks();
                    FinishFastScroll();
                    ASSERT_TRUE(AceType::DynamicCast<WaterFlowLayoutInfoSW>(pattern_->layoutInfo_)->ItemInView(index));
                    const auto rect = FastScrollItemRect(index);
                    auto resolved = align == ScrollAlign::AUTO ?
                        (index == 800 ? ScrollAlign::END : ScrollAlign::START) : align;
                    const float expected = resolved == ScrollAlign::START ? -extra :
                        (resolved == ScrollAlign::CENTER ? (WATER_FLOW_HEIGHT - rect.Height()) / 2.0f - extra :
                            WATER_FLOW_HEIGHT - rect.Height() - extra);
                    EXPECT_NEAR(rect.Top(), expected, 0.2f);
                    EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
                    EXPECT_FALSE(pattern_->targetIndex_);
                }
            }
        }
    }
}

/**
 * @tc.name: WFFastScrollInterruptAndReplace
 * @tc.desc: Touch, data changes and same-target replacement invalidate previous preparation.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollInterruptAndReplace, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f, FastScrollMode::SKIP_MEASURE);
    auto old = pattern_->fastScrollState_;
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_NE(old, pattern_->fastScrollState_);
    EXPECT_TRUE(old->cancelled);
    TouchEventInfo touch("touch");
    pattern_->OnTouchDown(touch);
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->fastScrollState_);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);

    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    const float stopped = pattern_->GetTotalOffset();
    pattern_->OnTouchDown(touch);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), stopped);

    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f, FastScrollMode::SKIP_MEASURE);
    pattern_->NotifyDataChange(0, 1);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->fastScrollState_);
    EXPECT_FALSE(pattern_->layoutInfo_->targetIndex_);
    EXPECT_FALSE(pattern_->layoutInfo_->duringPositionCalc_);
}

/**
 * @tc.name: WFFastScrollReentrantCallback
 * @tc.desc: An old spring frame cannot apply its delta or stop a replacement request from onWillScroll.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollReentrantCallback, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    auto replace = std::make_shared<bool>(true);
    eventHub_->SetOnWillScroll([this, replace](Dimension offset, ScrollState, ScrollSource) {
        if (*replace) {
            *replace = false;
            pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
        }
        return ScrollFrameResult { .offset = offset };
    });
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, 100);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    EXPECT_FALSE(pattern_->fastScrollSkip_);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 0.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollSingleFrameAndLargeOffset
 * @tc.desc: A final callback before layout preserves alignment; extraOffset measures only its required region.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollSingleFrameAndLargeOffset, TestSize.Level1)
{
    CreateWaterFlow();
    auto created = std::make_shared<std::set<int32_t>>();
    CreateItemsInLazyForEach(1000, [created](int32_t index) { created->insert(index); return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    MockAnimationManager::GetInstance().SetTicks(1);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(800).Top(), 350.0f, 0.2f);
    const int32_t before = created->size();
    pattern_->ScrollToIndexFastScroll(500, true, ScrollAlign::START, -40000.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    const int32_t offsetRegionCreated = created->size() - before;
    RecordProperty("offset_region_created", offsetRegionCreated);
    EXPECT_GE(offsetRegionCreated, 400);
    EXPECT_LE(offsetRegionCreated, 450);
    for (int32_t index = 532; index < 768; ++index) {
        EXPECT_EQ(created->count(index), 0u) << index;
    }
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 0.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollModeAndNegativeTarget
 * @tc.desc: Mode 1 preserves intermediate creation; an absolute target of -1 is a valid coordinate.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollModeAndNegativeTarget, TestSize.Level1)
{
    CreateWaterFlow();
    ScrollableModelNG::SetContentStartOffset(40.0f);
    ScrollableModelNG::SetContentEndOffset(60.0f);
    auto created = std::make_shared<int32_t>(0);
    CreateItemsInLazyForEach(1000, [created](int32_t) { ++*created; return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    MockAnimationManager::GetInstance().SetTicks(1);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::START, 39.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(pattern_->GetTotalOffset(), -1.0f, 0.1f);
    EXPECT_NEAR(FastScrollItemRect(0).Top(), 1.0f, 0.1f);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks();
    EXPECT_GE(*created, 801);
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(800).Top(), 40.0f, 0.1f);
}

/**
 * @tc.name: WFFastScrollBoundedWork
 * @tc.desc: A ten-thousand-item data source does not create or measure the skipped interval in mode 2.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollBoundedWork, TestSize.Level1)
{
    CreateWaterFlow();
    auto created = std::make_shared<int32_t>(0);
    CreateItemsInLazyForEach(10000, [created](int32_t) { ++*created; return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    const int32_t before = *created;
    auto context = MockPipelineContext::GetCurrent();
    context->SetResponseTime(200);
    MockAnimationManager::GetInstance().SetTicks(8);
    positionController_->ScrollToIndexWithOptions(8000, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    FinishFastScroll();
    EXPECT_LT(*created - before, 100);
    EXPECT_FALSE(context->ReachResponseDeadline());
    EXPECT_NEAR(FastScrollItemRect(8000).Top(), 350.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollHorizontal
 * @tc.desc: Fast scrolling uses horizontal main-axis geometry in both directions.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollHorizontal, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetLayoutDirection(FlexDirection::ROW);
    model.SetRowsTemplate("1fr 1fr");
    CreateWaterFlowItems(1000);
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (auto direction : { TextDirection::LTR, TextDirection::RTL }) {
        layoutProperty_->UpdateLayoutDirection(direction);
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        FlushUITasks();
        for (int32_t index : { 800, 100 }) {
            MockAnimationManager::GetInstance().SetTicks(4);
            pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::CENTER, 20.0f,
                FastScrollMode::SKIP_CREATE_AND_MEASURE);
            FlushUITasks();
            FinishFastScroll();
            const auto rect = FastScrollItemRect(index);
            EXPECT_NEAR(rect.Left(), (WATER_FLOW_WIDTH - rect.Width()) / 2.0f +
                (direction == TextDirection::RTL ? 20.0f : -20.0f), 0.2f);
        }
    }
}


/**
 * @tc.name: WFFastScrollSections
 * @tc.desc: Sliding-window section changes in lane count, gaps and margins preserve measured target alignment.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollSections, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t index) { return 100.0f + (index % 3) * 20.0f; });
    auto sections = pattern_->GetOrCreateWaterFlowSections();
    MarginProperty margin;
    margin.top = CalcLength(30.0f);
    margin.bottom = CalcLength(20.0f);
    std::vector<WaterFlowSections::Section> data(3);
    data[0].itemsCount = 200;
    data[0].crossCount = 1;
    data[1].itemsCount = 300;
    data[1].crossCount = 3;
    data[2].itemsCount = 500;
    data[2].crossCount = 2;
    for (auto& section : data) {
        section.rowsGap = 10.0_vp;
        section.margin = margin;
    }
    sections->ChangeData(0, 0, data);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (int32_t index : { 750, 100, 350 }) {
        MockAnimationManager::GetInstance().SetTicks(4);
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::CENTER, -20.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        FinishFastScroll();
        const auto rect = FastScrollItemRect(index);
        EXPECT_NEAR(rect.Top(), (WATER_FLOW_HEIGHT - rect.Height()) / 2.0f + 20.0f, 0.2f);
    }
}

/**
 * @tc.name: WFFastScrollFirstLayoutAndFollowingCommand
 * @tc.desc: Preparation obtains first-layout samples and leaves ordinary commands usable after a fast jump.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollFirstLayoutAndFollowingCommand, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    if (!(TOP_TO_DOWN)) {
        pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
    }
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(800).Top(), 350.0f, 0.2f);
    ScrollToIndex(20, false, ScrollAlign::START);
    EXPECT_NEAR(FastScrollItemRect(20).Top(), 0.0f, 0.2f);
    pattern_->ScrollToIndexFastScroll(LAST_ITEM, true, ScrollAlign::END, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, 999);
    FlushUITasks();
    auto lastState = pattern_->fastScrollState_;
    ASSERT_NE(lastState, nullptr);
    ASSERT_TRUE(lastState->targetOffset);
    const float initialTarget = *lastState->targetOffset;
    const int32_t preparedEnd = lastState->preparedWindow ? lastState->preparedWindow->endIndex_ : -1;
    FinishFastScroll();
    SCOPED_TRACE(testing::Message() << "initialTarget=" << initialTarget << " preparedEnd=" << preparedEnd
        << " finalTarget=" << lastState->targetOffset.value_or(0.0f) << " offset=" << pattern_->GetTotalOffset());
    EXPECT_NEAR(FastScrollItemRect(999).Bottom(), WATER_FLOW_HEIGHT, 0.2f);
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_NEAR(FastScrollItemRect(0).Top(), 0.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollThresholdAndVisibleAuto
 * @tc.desc: Exactly four viewports do not skip, and AUTO applies extraOffset once to a visible item.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollThresholdAndVisibleAuto, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (int32_t index : { 32, 33 }) {
        ScrollToIndex(0, false, ScrollAlign::START);
        MockAnimationManager::GetInstance().SetTicks(4);
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_EQ(!NearZero(pattern_->GetFastScrollRequest()->skipDistance), index == 33);
        FinishFastScroll();
        EXPECT_NEAR(FastScrollItemRect(index).Top(), 0.0f, 0.2f);
    }
    ScrollToIndex(0, false, ScrollAlign::START);
    pattern_->ScrollToIndexFastScroll(2, true, ScrollAlign::AUTO, 20.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(2).Top(), 180.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollOversizedItem
 * @tc.desc: Measured alignment remains correct when the destination is larger than the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollOversizedItem, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t index) { return index == 500 ? 2000.0f : 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END }) {
        ScrollToIndex(10, false, ScrollAlign::START);
        MockAnimationManager::GetInstance().SetTicks(4);
        pattern_->ScrollToIndexFastScroll(500, true, align, 20.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        FinishFastScroll();
        const float expected = align == ScrollAlign::START ? -20.0f :
            (align == ScrollAlign::CENTER ? -620.0f : -1220.0f);
        EXPECT_NEAR(FastScrollItemRect(500).Top(), expected, 0.2f);
    }
}

/**
 * @tc.name: WFFastScrollReverse
 * @tc.desc: Reversed layout transforms the final main-axis geometry and extra offset consistently.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollReverse, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetLayoutDirection(FlexDirection::COLUMN_REVERSE);
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    for (int32_t index : { 800, 100 }) {
        MockAnimationManager::GetInstance().SetTicks(4);
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::CENTER, 20.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        FinishFastScroll();
        EXPECT_NEAR(FastScrollItemRect(index).Top(), 370.0f, 0.2f);
    }
}

/**
 * @tc.name: WFFastScrollReentrantPreparation
 * @tc.desc: A request replaced during item creation survives the old layout and starts in the next layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollReentrantPreparation, TestSize.Level1)
{
    CreateWaterFlow();
    auto replace = std::make_shared<bool>(false);
    CreateItemsInLazyForEach(1000, [this, replace](int32_t index) {
        if (*replace && index >= 790) {
            *replace = false;
            pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::START, 0.0f,
                FastScrollMode::SKIP_CREATE_AND_MEASURE);
        }
        return 100.0f;
    });
    CreateDone();
    ASSERT_FALSE(TOP_TO_DOWN) << "This test must execute in water_flow_test_sw";
    *replace = true;
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto old = pattern_->fastScrollState_;
    FlushUITasks();
    EXPECT_TRUE(old->cancelled);
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, 100);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 0.0f, 0.2f);
}


/**
 * @tc.name: WFFastScrollFooterAndFollowingScroll
 * @tc.desc: A fast jump preserves footer geometry and ordinary scrolling across the target window.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollFooterAndFollowingScroll, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetFooter(GetDefaultHeaderBuilder());
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    for (float delta : { -300.0f, -300.0f, 300.0f, 300.0f }) {
        UpdateCurrentOffset(delta);
        const int32_t start = pattern_->layoutInfo_->startIndex_;
        const int32_t end = pattern_->layoutInfo_->endIndex_;
        EXPECT_LT(start, end);
        EXPECT_LE(FastScrollItemRect(start).Top(), 0.0f);
        EXPECT_GE(FastScrollItemRect(end).Bottom(), WATER_FLOW_HEIGHT);
        for (int32_t index = start; index < end; ++index) {
            EXPECT_NEAR(FastScrollItemRect(index).Bottom(), FastScrollItemRect(index + 1).Top(), 0.1f);
        }
    }
    pattern_->ScrollToIndexFastScroll(LAST_ITEM, true, ScrollAlign::END, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(999).Bottom(), WATER_FLOW_HEIGHT, 0.2f);
    const auto footerRect = GetChildRect(frameNode_, pattern_->layoutInfo_->footerIndex_);
    EXPECT_NEAR(footerRect.Top(), FastScrollItemRect(999).Bottom(), 0.2f);
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_NEAR(FastScrollItemRect(0).Top(), 0.0f, 0.2f);
}

void WaterFlowScrollerTestNg::CheckFastScrollInvalidIndex(FastScrollMode mode, int32_t index, bool running)
{
    constexpr int32_t CANCEL_SETTLE_TICKS = 2;
    MockAnimationManager::GetInstance().SetTicks(FAST_SCROLL_ANIMATION_TICKS);
    pattern_->ScrollToIndexFastScroll(FAST_SCROLL_TARGET, true, ScrollAlign::START, 0.0f, mode);
    const auto previous = pattern_->fastScrollState_;
    ASSERT_NE(previous, nullptr);
    if (running) {
        FlushUITasks();
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
    }
    const float before = pattern_->GetTotalOffset();
    pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::START, 0.0f, mode);
    EXPECT_TRUE(previous->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    for (int32_t tick = 0; tick < CANCEL_SETTLE_TICKS; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before);
    }
    FinishFastScroll();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before);
}

/**
 * @tc.name: WFFastScrollInvalidInputAndZeroViewport
 * @tc.desc: Invalid indices stop prior work like the legacy entry, and zero-height layout cancels safely.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollInvalidInputAndZeroViewport, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    for (auto mode : { FastScrollMode::NONE, static_cast<FastScrollMode>(-1), static_cast<FastScrollMode>(3) }) {
        pattern_->ScrollToIndexFastScroll(2, true, ScrollAlign::START, 0.0f, mode);
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        pattern_->StopAnimate();
    }
    for (int32_t index : { -2, 1000 }) {
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
    }
    // Invalid indices retain the old entry's stop behavior in both preparation and animation.
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (int32_t index : { -2, 1000 }) {
            for (bool running : { false, true }) {
                ASSERT_NO_FATAL_FAILURE(CheckFastScrollInvalidIndex(mode, index, running));
            }
        }
    }
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::START,
        std::numeric_limits<float>::infinity(), FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(0.0f)));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
    layoutProperty_->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(WATER_FLOW_HEIGHT)));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 0.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollSignedThreshold
 * @tc.desc: Both directions distinguish 4V-1, 4V, and 4V+1 without an initial opposite delta.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollSignedThreshold, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    for (int32_t direction : { -1, 1 }) {
        for (float extra : { -1.0f, 0.0f, 1.0f }) {
            ScrollToIndex(100, false, ScrollAlign::START);
            const float before = pattern_->GetTotalOffset();
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndexFastScroll(100 + direction * 32, true, ScrollAlign::START, extra,
                FastScrollMode::SKIP_CREATE_AND_MEASURE);
            FlushUITasks();
            ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
            EXPECT_EQ(!NearZero(pattern_->GetFastScrollRequest()->skipDistance), direction * extra > 0.0f);
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks();
            EXPECT_GE(direction * (pattern_->GetTotalOffset() - before), 0.0f);
            FinishFastScroll();
            EXPECT_NEAR(FastScrollItemRect(100 + direction * 32).Top(), -extra, 0.2f);
        }
    }
}
/**
 * @tc.name: WFFastScrollOrdinaryTakeover
 * @tc.desc: Ordinary scrollTo, scrollBy and scrollEdge own the viewport after replacing pending or active fast work.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollOrdinaryTakeover, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    CreateDone();
    for (bool animating : { false, true }) {
        for (int32_t operation = 0; operation < 3; ++operation) {
            SCOPED_TRACE(std::to_string(animating) + ":" + std::to_string(operation));
            ScrollToIndex(50, false, ScrollAlign::START);
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
                FastScrollMode::SKIP_CREATE_AND_MEASURE);
            auto old = pattern_->fastScrollState_;
            if (animating) {
                FlushUITasks();
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks();
            }
            float expected = 0.0f;
            if (operation == 0) {
                expected = 200.0f;
                pattern_->ScrollTo(expected);
                FlushUITasks();
            } else if (operation == 1) {
                expected = pattern_->GetTotalOffset() + 120.0f;
                ScrollBy(0.0f, 120.0f);
            } else {
                ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
            }
            EXPECT_TRUE(old->cancelled);
            EXPECT_FALSE(pattern_->fastScrollState_);
            EXPECT_FALSE(pattern_->GetFastScrollRequest());
            FinishFastScroll();
            EXPECT_NEAR(pattern_->GetTotalOffset(), expected, 0.2f);
        }
    }
}

/**
 * @tc.name: WFFastScrollSectionAndDeletionRecovery
 * @tc.desc: Section changes and deletion of an animated target invalidate its window and allow a new request.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollSectionAndDeletionRecovery, TestSize.Level1)
{
    CreateWaterFlow();
    auto lazy = CreateItemsInLazyForEach(1000, [](int32_t) { return 100.0f; });
    auto sections = pattern_->GetOrCreateWaterFlowSections();
    std::vector<WaterFlowSections::Section> data(2);
    data[0].itemsCount = 500;
    data[1].itemsCount = 500;
    data[0].crossCount = 1;
    data[1].crossCount = 1;
    sections->ChangeData(0, 0, data);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    auto old = pattern_->fastScrollState_;
    ASSERT_NE(old, nullptr);
    data[0].crossCount = 2;
    sections->ChangeData(0, 1, { data[0] });
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushUITasks();
    EXPECT_TRUE(old->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    FinishFastScroll();

    pattern_->ScrollToIndexFastScroll(900, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    old = pattern_->fastScrollState_;
    ASSERT_NE(old, nullptr);
    lazy->SetTotalCount(999);
    DeleteItemInLazyForEach(900);
    data[1].itemsCount = 499;
    sections->ChangeData(1, 1, { data[1] });
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushUITasks();
    EXPECT_TRUE(old->cancelled);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    FinishFastScroll();
    pattern_->ScrollToIndexFastScroll(100, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    FinishFastScroll();
    EXPECT_NEAR(FastScrollItemRect(100).Top(), 350.0f, 0.2f);
}

/**
 * @tc.name: WFFastScrollRepeatAndAccessibility
 * @tc.desc: Virtual Repeat settles in both directions and the forward page action remains usable afterward.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollRepeatAndAccessibility, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemsInRepeat(1000, [](uint32_t index) { return 100.0f + (index % 3) * 20.0f; });
    CreateDone();
    for (int32_t index : { 800, 100 }) {
        MockAnimationManager::GetInstance().SetTicks(4);
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::CENTER, 20.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        FinishFastScroll();
        const auto rect = FastScrollItemRect(index);
        EXPECT_NEAR(rect.Top(), (WATER_FLOW_HEIGHT - rect.Height()) / 2.0f - 20.0f, 0.2f);
    }
    const auto before = pattern_->GetTotalOffset();
    accessibilityProperty_->ActActionScrollForward();
    FlushUITasks();
    FinishFastScroll();
    EXPECT_GT(pattern_->GetTotalOffset(), before);
    EXPECT_FALSE(pattern_->fastScrollState_);
}

/**
 * @tc.name: WFFastScrollEmptyData
 * @tc.desc: An empty WaterFlow rejects content and last-item indices without retaining animation state.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollerTestNg, WFFastScrollEmptyData, TestSize.Level1)
{
    CreateWaterFlow();
    CreateDone();
    for (int32_t index : { 0, LAST_ITEM }) {
        pattern_->ScrollToIndexFastScroll(index, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks();
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->fastScrollState_);
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
    }
}
#endif
} // namespace OHOS::Ace::NG

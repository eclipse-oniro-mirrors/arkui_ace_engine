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

#include "list_test_ng.h"
#include "core/components_ng/event/state_style_manager.h"
#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/list/list_item_layout_property.h"
#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "core/animation/velocity_motion.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/common/multi_thread_build_manager.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/syntax/if_else_node.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"

#define private public
#define protected public
#include "core/components_ng/pattern/list/list_item_group_paint_method.h"
#undef private
#undef protected

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t FAST_SCROLL_ITEM_COUNT = 100;
constexpr int32_t FAST_SCROLL_TARGET_INDEX = 60;
constexpr int32_t REVERSED_ITEM_COUNT = 47;
constexpr int32_t ROW_SIZE_PERIOD = 6;
constexpr float LANE_SIZE_STEP = 15.0f;
constexpr int32_t REVERSED_TARGET_INDEX = 27;
constexpr int32_t INITIAL_SCROLL_SAMPLE_COUNT = 2;
constexpr int32_t SHORTCUT_INITIAL_INDEX = 20;
constexpr int32_t ALTERNATING_HEIGHT_PERIOD = 2;
constexpr float RELOADED_SHORT_HEIGHT = 60.0f;
constexpr float RELOADED_TALL_HEIGHT = 180.0f;
constexpr int32_t MULTI_LANE_COUNT = 2;
constexpr int32_t CONSTRAINED_LANE_MODE = 2;
constexpr int32_t LONG_ANIMATION_TICKS = 8;
constexpr float OFFSET_SHORT_HEIGHT = 60.0f;
constexpr float OFFSET_TALL_HEIGHT = 140.0f;
constexpr int32_t ANIMATION_TICKS = 4;
constexpr int32_t CHANGE_STACK_FROM_END = 2;
constexpr int32_t CHANGE_CONTENT_START_OFFSET = 3;
constexpr int32_t CHANGE_CONTENT_END_OFFSET = 4;
constexpr int32_t CHANGE_ITEM_SPACE = 5;
constexpr int32_t CHANGE_LIST_HEIGHT = 6;
constexpr int32_t CHANGE_ESTIMATED_SPACE = 7;
constexpr int32_t UNEVEN_LANE_COUNT = 3;
constexpr int32_t UNEVEN_INITIAL_INDEX = 90;
constexpr int32_t UNEVEN_ITEM_COUNT = 120;
constexpr int32_t KNOWN_GROUP_PERIOD = 13;
constexpr int32_t KNOWN_GROUP_REMAINDER = 2;
constexpr int32_t KNOWN_GROUP_CHILD_COUNT = 3;
constexpr int32_t MIXED_ITEM_COUNT = 360;
constexpr float MIXED_DEFAULT_SIZE = 80.0f;
constexpr int32_t FIRST_GROUP_PERIOD = 17;
constexpr int32_t FIRST_GROUP_REMAINDER = 3;
constexpr int32_t SECOND_GROUP_PERIOD = 29;
constexpr int32_t SECOND_GROUP_REMAINDER = 11;
constexpr int32_t MIXED_GROUP_ROW_COUNT = 2;
constexpr int32_t MIXED_INITIAL_INDEX = 150;
constexpr int32_t MIXED_TARGET_INDEX = 144;
constexpr int32_t MIXED_MAX_ANIMATION_FRAMES = 64;
constexpr int32_t MIXED_ORDINARY_SCROLL_STEPS = 48;
constexpr float MIXED_SCROLL_STEP = 30.0f;
constexpr int32_t MIXED_THREE_LANES = 3;
constexpr float MAX_GROUP_HEIGHT_STEP = 24.0f;
constexpr float GROUP_HEIGHT_STEP = 12.0f;
constexpr int32_t FALLING_GROUP_MIN_CHILDREN = 7;
constexpr int32_t GROUP_CHILD_COUNT_PERIOD = 3;
constexpr int32_t HEIGHT_SECTION_COUNT = 90;
constexpr int32_t ITEM_HEIGHT_PERIOD = 3;
constexpr float ITEM_HEIGHT_STEP = 8.0f;
constexpr int32_t GROUP_HEIGHT_PERIOD = 3;
constexpr int32_t MIXED_TWO_LANES = 2;
constexpr int32_t RISING_GROUP_MIN_CHILDREN = 5;
constexpr int32_t TRACE_START_INDEX = 142;
constexpr int32_t TRACE_END_INDEX = 152;
} // namespace

class ListScrollerTestNg : public ListTestNg {
protected:
    void CreateGroupWithSettingChildrenMainSize(int32_t groupNumber);
    void CreateGroupChildrenMainSize(int32_t groupNumber);
    void CreateGroupWithItem(int32_t groupNumber, Axis axis = Axis::VERTICAL);
};

class ListFastScrollerTestNg : public ListScrollerTestNg {
public:
    void SetUp() override
    {
        version_ = MockAnimationManager::GetVersion();
        MockAnimationManager::SetVersion(MockAnimationManager::Version::V2);
        ListScrollerTestNg::SetUp();
    }

    void TearDown() override
    {
        ListScrollerTestNg::TearDown();
        MockAnimationManager::SetVersion(version_);
    }

protected:
    void CreateDirectBuilderItems(const RefPtr<LazyForEachBuilder>& builder, int32_t count)
    {
        const auto parent = ViewStackProcessor::GetInstance()->GetMainElementNode();
        ASSERT_NE(parent, nullptr);
        for (int32_t index = 0; index < count; ++index) {
            ScopedViewStackProcessor scoped;
            auto item = builder->GetChildByIndex(index, true).second;
            ASSERT_NE(item, nullptr);
            item->MountToParent(parent);
        }
    }

    void RunFastScrollFrames(int32_t maxFrames = 24)
    {
        if (!pattern_->GetFastScrollRequest() && !pattern_->AnimateRunning() && !pattern_->scrollStop_) {
            return;
        }
        std::ostringstream trace;
        for (int32_t frame = 0; frame < maxFrames; ++frame) {
            if (!MockAnimationManager::GetInstance().AllFinished()) {
                MockAnimationManager::GetInstance().Tick();
            }
            FlushUITasks();
            trace << "\nframe=" << frame << " offset=" << pattern_->GetTotalOffset()
                  << " endpoint=" << pattern_->GetFinalPosition();
            if (!pattern_->itemPosition_.empty()) {
                const auto& first = *pattern_->itemPosition_.begin();
                const auto& last = *pattern_->itemPosition_.rbegin();
                trace << " visible=" << first.first << ":" << first.second.startPos
                      << ".." << last.first << ":" << last.second.endPos;
            }
            AppendFastScrollGroupTrace(trace);
            if (!pattern_->GetFastScrollRequest() && !pattern_->AnimateRunning() && !pattern_->scrollStop_) {
                RecordProperty("fastScrollFrames", frame + 1);
                RecordProperty("fastScrollFrameTrace", trace.str());
                return;
            }
        }
        float targetPosition = 0.0f;
        if (pattern_->GetFastScrollRequest()) {
            pattern_->GetFastScrollTargetPosition(targetPosition);
        }
        FAIL() << "Fast scroll did not finish within " << maxFrames << " frames; pending="
               << pattern_->HasFastScrollRequest() << ", running=" << pattern_->AnimateRunning()
               << ", stop=" << pattern_->scrollStop_ << ", offset=" << pattern_->GetTotalOffset()
               << ", target=" << targetPosition << trace.str();
    }

    void AppendFastScrollGroupTrace(std::ostringstream& trace)
    {
        if (auto request = pattern_->GetFastScrollRequest()) {
            trace << " skip=" << request->skipDistance << " skipPending=" << request->skipPending;
            const auto item = pattern_->itemPosition_.find(request->index);
            if (item != pattern_->itemPosition_.end() && item->second.isGroup) {
                const auto group = GetChildPattern<ListItemGroupPattern>(frameNode_, request->index);
                trace << " group=" << item->second.startPos << ".." << item->second.endPos;
                if (group && !group->GetItemPosition().empty()) {
                    const auto& start = *group->GetItemPosition().begin();
                    const auto& end = *group->GetItemPosition().rbegin();
                    trace << " internal=" << start.first << ":" << start.second.startPos
                          << ".." << end.first << ":" << end.second.endPos;
                }
            }
        }
    }

private:
    MockAnimationManager::Version version_ = MockAnimationManager::Version::V0;
};

namespace {
std::pair<std::string, RefPtr<UINode>> WithConditionalBranch(std::pair<std::string, RefPtr<UINode>> item)
{
    auto branch = IfElseNode::GetOrCreateIfElseNode(ElementRegister::GetInstance()->MakeUniqueId());
    item.second->MountToParent(branch);
    item.second = branch;
    return item;
}
} // namespace

using FastScrollParameters = std::tuple<bool, bool, ScrollAlign, float>;

class ListFastScrollEstimationTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<int32_t> {
protected:
    void CreateEstimationRequest(bool knownSize = false)
    {
        auto model = CreateList();
        if (knownSize) {
            model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        CreateListItems(FAST_SCROLL_ITEM_COUNT);
        CreateDone();
        pattern_->InitializeFastScrollRequest(FAST_SCROLL_TARGET_INDEX, ScrollAlign::START, 25.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
    }

    float GetInvalidSize() const
    {
        if (GetParam() == 0) {
            return std::numeric_limits<float>::quiet_NaN();
        }
        return GetParam() == 1 ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max();
    }

    void CheckCancelledEstimation()
    {
        EXPECT_FALSE(pattern_->HasFastScrollRequest());
        EXPECT_FALSE(pattern_->scrollTarget_.has_value());
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
        pattern_->UpdateFastScrollPositionMap();
        pattern_->TryStartFastScroll();
        auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
        ASSERT_NE(algorithm, nullptr);
        EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    }
};

class ListFastScrollPreparedEstimationTestNg : public ListFastScrollEstimationTestNg {};

class FastScrollMapObservingAlgorithm final : public ListLanesLayoutAlgorithm {
    DECLARE_ACE_TYPE(FastScrollMapObservingAlgorithm, ListLanesLayoutAlgorithm);
public:
    std::function<void(const RefPtr<ListPositionMap>&)> onMeasure;
    WeakPtr<ListPositionMap> measuredMap;
    int32_t measureCount = 0;

    void MeasureList(LayoutWrapper* wrapper) override
    {
        ++measureCount;
        measuredMap = posMap_;
        if (onMeasure) {
            onMeasure(posMap_);
        }
        ListLayoutAlgorithm::MeasureList(wrapper);
    }
};

class ListFastScrollMapScopeTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<int32_t> {
protected:
    using Rows = std::map<int32_t, ListPositionInfo>;
    using ScrollState = std::pair<double, ListLayoutAlgorithm::PositionMap>;

    void CreateKnownReversedList()
    {
        auto model = CreateList();
        model.SetStackFromEnd(true);
        model.SetInitialIndex(0);
        model.SetLanes(GetParam());
        model.SetSpace(Dimension(5.0f));
        const float rowSizes[] = { 80.0f, 240.0f, 90.0f, 500.0f, 120.0f, 700.0f };
        std::vector<float> sizes;
        for (int32_t index = 0; index < REVERSED_ITEM_COUNT; ++index) {
            CreateListItem();
            sizes.emplace_back(rowSizes[(index / GetParam()) % ROW_SIZE_PERIOD] + index % GetParam() * LANE_SIZE_STEP);
            ViewAbstract::SetHeight(CalcLength(sizes.back()));
            ViewStackProcessor::GetInstance()->Pop();
            ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        }
        model.GetOrCreateListChildrenMainSize()->ChangeData(0, 0, sizes);
        CreateDone();
    }

    void ExpectPublicRows(const Rows& expected)
    {
        ASSERT_EQ(pattern_->posMap_->posMap_.size(), expected.size());
        for (const auto& [index, info] : expected) {
            const auto actual = pattern_->posMap_->GetPositionInfo(index);
            EXPECT_FLOAT_EQ(actual.mainPos, info.mainPos) << index;
            EXPECT_FLOAT_EQ(actual.mainSize, info.mainSize) << index;
            EXPECT_EQ(actual.isGroup, info.isGroup) << index;
            EXPECT_EQ(actual.isLazyChild, info.isLazyChild) << index;
        }
    }

    RefPtr<FastScrollMapObservingAlgorithm> CreateObservedAlgorithm(
        bool mirror = true, std::function<void()> duringMeasure = nullptr)
    {
        auto algorithm = AceType::MakeRefPtr<FastScrollMapObservingAlgorithm>();
        algorithm->SetLanes(GetParam());
        pattern_->SetLayoutAlgorithmParams(algorithm, layoutProperty_);
        const auto rows = pattern_->posMap_->posMap_;
        algorithm->onMeasure = [this, rows, mirror, duringMeasure](const RefPtr<ListPositionMap>& working) {
            if (mirror) {
                EXPECT_NE(working, pattern_->posMap_);
                ExpectPublicRows(rows);
            } else {
                EXPECT_EQ(working, pattern_->posMap_);
            }
            if (duringMeasure) {
                duringMeasure();
            }
        };
        return algorithm;
    }

    void InitializeKnownRequest()
    {
        pattern_->InitializeFastScrollRequest(REVERSED_TARGET_INDEX, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
    }

    void CheckOrdinaryScroll(const Rows& rows)
    {
        const auto before = pattern_->GetTotalOffset();
        pattern_->ScrollBy(25.0f);
        FlushUITasks();
        EXPECT_NEAR(pattern_->GetTotalOffset(), before + 25.0f, 0.1f);
        ExpectPublicRows(rows);
        auto next = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
        ASSERT_NE(next, nullptr);
        EXPECT_EQ(next->GetFastScrollState(), nullptr);
    }

    std::vector<ScrollState> CollectOrdinaryScroll(const Rows& rows)
    {
        std::vector<ScrollState> states = { { pattern_->GetTotalOffset(), pattern_->itemPosition_ } };
        for (float distance : { 25.0f, -25.0f, 800.0f, -800.0f, 25.0f }) {
            SCOPED_TRACE(distance);
            const auto offset = pattern_->GetTotalOffset();
            pattern_->ScrollBy(distance);
            FlushUITasks();
            if (states.size() <= INITIAL_SCROLL_SAMPLE_COUNT) {
                EXPECT_NEAR(pattern_->GetTotalOffset(), offset + distance, 0.1f);
            }
            ExpectPublicRows(rows);
            EXPECT_FALSE(pattern_->HasFastScrollRequest());
            auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
            EXPECT_NE(algorithm, nullptr);
            CHECK_NULL_RETURN(algorithm, states);
            EXPECT_EQ(algorithm->GetFastScrollState(), nullptr);
            states.emplace_back(pattern_->GetTotalOffset(), pattern_->itemPosition_);
        }
        return states;
    }

    void ExpectOrdinaryScrollMatches(const std::vector<ScrollState>& expected, const std::vector<ScrollState>& actual)
    {
        ASSERT_EQ(actual.size(), expected.size());
        for (size_t step = 0; step < expected.size(); ++step) {
            SCOPED_TRACE(step);
            EXPECT_NEAR(actual[step].first, expected[step].first, 0.1f);
            ASSERT_EQ(actual[step].second.size(), expected[step].second.size());
            auto item = actual[step].second.begin();
            for (const auto& [index, info] : expected[step].second) {
                ASSERT_EQ(item->first, index);
                EXPECT_FLOAT_EQ(item->second.startPos, info.startPos);
                EXPECT_FLOAT_EQ(item->second.endPos, info.endPos);
                EXPECT_EQ(item->second.isGroup, info.isGroup);
                EXPECT_EQ(item->second.isLazyChild, info.isLazyChild);
                ++item;
            }
        }
    }
};

/**
 * @tc.name: FastScrollMeasureOnlyKeepsPublicMap001
 * @tc.desc: Repeated Measure without Layout or algorithm destruction never leaves the shared table mirrored.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollMeasureOnlyKeepsPublicMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    InitializeKnownRequest();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm();
    for (int32_t attempt = 0; attempt < 2; ++attempt) {
        algorithm->Measure(AceType::RawPtr(frameNode_));
        ExpectPublicRows(rows);
        EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
        EXPECT_EQ(algorithm->measuredMap.Upgrade(), nullptr);
    }
    EXPECT_EQ(algorithm->measureCount, 2);
    algorithm.Reset();
    ExpectPublicRows(rows);
    pattern_->StopAnimate();
    FlushUITasks();
    CheckOrdinaryScroll(rows);
}

/**
 * @tc.name: FastScrollCancelBeforeLayoutKeepsPublicMap001
 * @tc.desc: Cancellation between stages does not change the measured algorithm's coordinate policy or public table.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollCancelBeforeLayoutKeepsPublicMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    InitializeKnownRequest();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm();
    algorithm->Measure(AceType::RawPtr(frameNode_));
    pattern_->StopAnimate();
    ASSERT_FALSE(pattern_->HasFastScrollRequest());
    ExpectPublicRows(rows);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
    ExpectPublicRows(rows);
    FlushUITasks();
    CheckOrdinaryScroll(rows);
}

/**
 * @tc.name: FastScrollCancelDuringMeasureKeepsPublicMap001
 * @tc.desc: An application-side stop during Measure cannot expose the temporary mirror to the Pattern.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollCancelDuringMeasureKeepsPublicMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    InitializeKnownRequest();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm(true, [this]() { pattern_->StopAnimate(); });
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
    ExpectPublicRows(rows);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    ExpectPublicRows(rows);
    FlushUITasks();
    CheckOrdinaryScroll(rows);
}

/**
 * @tc.name: FastScrollFinishDuringMeasureKeepsPublicMap001
 * @tc.desc: Natural completion during the last layout leaves its local input intact and the shared table canonical.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollFinishDuringMeasureKeepsPublicMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    InitializeKnownRequest();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm(true, [this]() { pattern_->OnAnimateStop(); });
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(algorithm->GetFastScrollState()->input.useKnownMainSize);
    ExpectPublicRows(rows);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
    ExpectPublicRows(rows);
    FlushUITasks();
    CheckOrdinaryScroll(rows);
}

/**
 * @tc.name: FastScrollLayoutEarlyReturnKeepsPublicMap001
 * @tc.desc: An early Layout return cannot leak its mirror, and the same measured algorithm can complete afterward.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollLayoutEarlyReturnKeepsPublicMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    InitializeKnownRequest();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm();
    algorithm->Measure(AceType::RawPtr(frameNode_));
    LayoutWrapperNode invalid(frameNode_, frameNode_->GetGeometryNode()->Clone(),
        AceType::MakeRefPtr<LayoutProperty>());
    algorithm->Layout(&invalid);
    EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
    ExpectPublicRows(rows);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    ExpectPublicRows(rows);
    pattern_->StopAnimate();
    FlushUITasks();
    CheckOrdinaryScroll(rows);
}

/**
 * @tc.name: FastScrollFinalFrameKeepsOrdinaryScrolling001
 * @tc.desc: The last animation frame preserves the public table and subsequent scrolling matches a fresh legacy List.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, FastScrollFinalFrameKeepsOrdinaryScrolling001, TestSize.Level1)
{
    CreateKnownReversedList();
    const auto rows = pattern_->posMap_->posMap_;
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(27, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    int32_t frames = 0;
    while ((pattern_->HasFastScrollRequest() || pattern_->AnimateRunning() || pattern_->scrollStop_) && frames < 48) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ExpectPublicRows(rows);
        ++frames;
    }
    ASSERT_GT(frames, 0);
    ASSERT_FALSE(pattern_->HasFastScrollRequest());
    ASSERT_FALSE(pattern_->AnimateRunning());
    const auto actual = CollectOrdinaryScroll(rows);
    // Reset registered IDs, mounted nodes and animation state before creating the independent control.
    ListScrollerTestNg::TearDown();
    ListScrollerTestNg::SetUp();
    CreateKnownReversedList();
    pattern_->ScrollToIndex(27, false, ScrollAlign::START);
    FlushUITasks();
    ExpectOrdinaryScrollMatches(CollectOrdinaryScroll(rows), actual);
}

/**
 * @tc.name: OrdinaryLayoutNeverUsesFastScrollMap001
 * @tc.desc: A List that has not requested FastScroll continues using its original shared position map.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMapScopeTestNg, OrdinaryLayoutNeverUsesFastScrollMap001, TestSize.Level1)
{
    CreateKnownReversedList();
    const auto rows = pattern_->posMap_->posMap_;
    auto algorithm = CreateObservedAlgorithm(false);
    EXPECT_EQ(algorithm->GetFastScrollState(), nullptr);
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->posMap_, pattern_->posMap_);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    ExpectPublicRows(rows);
}

INSTANTIATE_TEST_SUITE_P(KnownReversedRows, ListFastScrollMapScopeTestNg, testing::Values(1, 3));

/**
 * @tc.name: FastScrollPositionUpdateUsesValidPrefix001
 * @tc.desc: FastScroll skips incomplete prefix metadata, then ordinary scrolling resumes legacy offset updates.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollPositionUpdateUsesValidPrefix001, TestSize.Level1)
{
    CreateEstimationRequest();
    pattern_->spaceWidth_ = 10.0f;
    pattern_->posMap_->ClearPosMap();
    pattern_->posMap_->UpdatePos(0, { 0.0f, 100.0f });
    pattern_->posMap_->UpdatePos(1, { -1.0f, -1.0f, true });
    pattern_->posMap_->UpdatePos(2, { 200.0f, -1.0f });
    pattern_->posMap_->UpdatePos(3, { -1.0f, 50.0f });
    pattern_->posMap_->UpdatePos(4, { 400.0f, 100.0f });
    pattern_->itemPosition_ = { { 4, { -1, 0.0f, 100.0f, false } } };
    pattern_->currentOffset_ = 100.0;
    pattern_->CalculateCurrentOffset(0.0f, {});
    EXPECT_DOUBLE_EQ(pattern_->currentOffset_, 110.0);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(4).mainPos, 110.0f);
    pattern_->CancelFastScroll();
    EXPECT_EQ(pattern_->posMap_->posMap_.size(), 2);
    pattern_->CalculateCurrentOffset(10.0f, {});
    EXPECT_DOUBLE_EQ(pattern_->currentOffset_, 120.0);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(4).mainPos, 120.0f);
}

/**
 * @tc.name: FastScrollPositionUpdateKeepsUnmeasuredSuffix001
 * @tc.desc: FastScroll ignores unpositioned suffix metadata and cancellation leaves only legacy-compatible records.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollPositionUpdateKeepsUnmeasuredSuffix001, TestSize.Level1)
{
    CreateEstimationRequest();
    pattern_->posMap_->ClearPosMap();
    pattern_->posMap_->UpdatePos(3, { 100.0f, 100.0f });
    pattern_->posMap_->UpdatePos(79, { -1.0f, 50.0f, true });
    pattern_->posMap_->UpdatePos(80, { 150.0f, 80.0f, false, true });
    pattern_->currentOffset_ = 100.0;
    const ListLayoutAlgorithm::PositionMap items = { { 3, { -1, 0.0f, 100.0f, false } } };
    pattern_->UpdatePosMap(items);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(79).mainPos, -1.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainPos, 200.0f);
    pattern_->CancelFastScroll();
    EXPECT_EQ(pattern_->posMap_->posMap_.count(79), 0);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, 80.0f);
    EXPECT_TRUE(pattern_->posMap_->GetPositionInfo(80).isLazyChild);
    pattern_->currentOffset_ = 120.0;
    pattern_->UpdatePosMap(items);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainPos, 220.0f);
}

/**
 * @tc.name: FastScrollNaturalStopRemovesUnpositionedEntries001
 * @tc.desc: Retargeting preserves preparation metadata, while natural completion removes it before ordinary scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollNaturalStopRemovesUnpositionedEntries001, TestSize.Level1)
{
    CreateEstimationRequest();
    const auto first = pattern_->posMap_->GetPositionInfo(0);
    pattern_->posMap_->UpdatePos(80, { -1.0f, 50.0f, true });
    pattern_->startingFastScroll_ = true;
    pattern_->OnAnimateStop();
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, 50.0f);
    pattern_->startingFastScroll_ = false;
    pattern_->OnAnimateStop();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->posMap_->posMap_.count(80), 0);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(0).mainPos, first.mainPos);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(0).mainSize, first.mainSize);
}

/**
 * @tc.name: LegacyScrollStopKeepsPositionMap001
 * @tc.desc: Ordinary stop and cancellation do not scan or discard position records without FastScroll preparation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, LegacyScrollStopKeepsPositionMap001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    ASSERT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
    pattern_->posMap_->UpdatePos(80, { -1.0f, 50.0f, true });
    pattern_->CancelFastScroll();
    pattern_->OnAnimateStop();
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainPos, -1.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, 50.0f);
    EXPECT_TRUE(pattern_->posMap_->GetPositionInfo(80).isGroup);
    EXPECT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
}

/**
 * @tc.name: FastScrollEstimateUsesPositionMapAverage001
 * @tc.desc: Missing entries use the average of visible, historical and offscreen prepared sizes.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollEstimateUsesPositionMapAverage001, TestSize.Level1)
{
    CreateEstimationRequest();
    ASSERT_FALSE(pattern_->itemPosition_.empty());
    EXPECT_FLOAT_EQ(pattern_->itemPosition_.begin()->second.endPos -
        pattern_->itemPosition_.begin()->second.startPos, ITEM_MAIN_SIZE);
    pattern_->posMap_->ClearPosMap();
    pattern_->posMap_->UpdatePos(0, { 0.0f, 100.0f });
    pattern_->posMap_->UpdatePos(4, { 400.0f, 300.0f });
    pattern_->posMap_->UpdatePos(80, { -1.0f, 500.0f, false, true });
    ASSERT_TRUE(pattern_->EstimateFastScrollPositions(80, 80));
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(1).mainSize, 300.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(4).mainSize, 300.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, 500.0f);
    EXPECT_TRUE(pattern_->posMap_->GetPositionInfo(80).isLazyChild);
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->estimated);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
}

/**
 * @tc.name: FastScrollEstimateKnownSizeBypassesAverage001
 * @tc.desc: ChildrenMainSize retains its accurate position table without mean-based estimation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollEstimateKnownSizeBypassesAverage001, TestSize.Level1)
{
    CreateEstimationRequest(true);
    ASSERT_NE(pattern_->childrenSize_, nullptr);
    const auto target = pattern_->posMap_->GetPositionInfo(60);
    ASSERT_TRUE(pattern_->EstimateFastScrollPositions());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainPos, target.mainPos);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainSize, ITEM_MAIN_SIZE);
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->estimated);
}

/**
 * @tc.name: FastScrollEstimateWaitsForVisibleSample001
 * @tc.desc: Missing visible layout or viewport is a wait condition, not a fatal estimation failure.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollEstimationTestNg, FastScrollEstimateWaitsForVisibleSample001, TestSize.Level1)
{
    CreateEstimationRequest();
    pattern_->itemPosition_.clear();
    pattern_->UpdateFastScrollPositionMap();
    pattern_->TryStartFastScroll();
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_FALSE(pattern_->AnimateRunning());
    pattern_->itemPosition_[0] = { -1, 0.0f, ITEM_MAIN_SIZE, false };
    pattern_->contentMainSize_ = 0.0f;
    pattern_->TryStartFastScroll();
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
}

/**
 * @tc.name: FastScrollEstimateFailureCancels
 * @tc.desc: Invalid sizes, coordinate overflow, absent metadata and invalid spacing cannot leave a pending request.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollEstimationTestNg, FastScrollEstimateFailureCancels, TestSize.Level1)
{
    CreateEstimationRequest();
    pattern_->posMap_->ClearPosMap();
    if (GetParam() != 3) {
        pattern_->posMap_->UpdatePos(0, { 0.0f, 100.0f });
        if (GetParam() < 3) {
            pattern_->posMap_->UpdatePos(1, { 100.0f, GetInvalidSize() });
            pattern_->posMap_->UpdatePos(2, { 200.0f, GetInvalidSize() });
        } else {
            pattern_->spaceWidth_ = -1.0f;
        }
    }
    pattern_->UpdateFastScrollPositionMap();
    pattern_->TryStartFastScroll();
    ASSERT_NO_FATAL_FAILURE(CheckCancelledEstimation());
}

/**
 * @tc.name: FastScrollPreparedEstimateFailureDoesNotRetry
 * @tc.desc: Invalid prepared suffix metadata cancels instead of preparing the same extraOffset window again.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollPreparedEstimationTestNg, FastScrollPreparedEstimateFailureDoesNotRetry, TestSize.Level1)
{
    CreateEstimationRequest();
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->SetFastScrollTarget(60, ScrollAlign::START);
    algorithm->SetFastScrollPreparationOffset(25.0f);
    algorithm->GetOrCreateFastScrollState().result.status = FastScrollMeasureStatus::COMPLETE;
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[60] = { 100.0f };
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[65] = { GetInvalidSize() };
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[66] = { GetInvalidSize() };
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[67] = { 100.0f };
    pattern_->ApplyFastScrollLayoutResult(algorithm);
    ASSERT_NO_FATAL_FAILURE(CheckCancelledEstimation());
}

INSTANTIATE_TEST_SUITE_P(InvalidMetadata, ListFastScrollEstimationTestNg, testing::Values(0, 1, 2, 3, 4));
INSTANTIATE_TEST_SUITE_P(PreparedMetadata, ListFastScrollPreparedEstimationTestNg, testing::Values(0, 1, 2));

class ListFastScrollTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<FastScrollParameters> {};

using FastScrollDirectionParameters = std::tuple<Axis, bool, bool, bool, ScrollAlign>;

class ListFastScrollDirectionTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<FastScrollDirectionParameters> {};

class ListFastScrollModeTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, bool, bool, bool>> {};

/**
 * @tc.name: FastScrollModeForwarding
 * @tc.desc: Both enabled modes are preserved; disabled, invalid and nonanimated options use the legacy path.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollModeTestNg, FastScrollModeForwarding, TestSize.Level1)
{
    const auto [value, smooth, useModel, knownSize] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    const auto mode = static_cast<FastScrollMode>(value);
    if (useModel) {
        ListModelNG::SetScrollToIndexWithOptions(
            AceType::RawPtr(frameNode_), 80, smooth, ScrollAlign::START, 25.0f, mode);
    } else {
        positionController_->ScrollToIndexWithOptions(80, smooth, ScrollAlign::START, 25.0f, mode);
    }
    const bool enabled = smooth && (value == 1 || value == 2);
    ASSERT_EQ(pattern_->HasFastScrollRequest(), enabled);
    if (enabled) {
        EXPECT_EQ(pattern_->GetFastScrollRequest()->fastScrollMode, mode);
    }
    FlushUITasks();
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_NEAR(pattern_->GetTotalOffset(), 8025.0f, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(Options, ListFastScrollModeTestNg,
    testing::Combine(testing::Values(0, 1, 2, -1, 3, 100), testing::Bool(), testing::Bool(), testing::Bool()));

class ListFastScrollVisibleShortcutTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<Axis, bool, bool, bool, bool, ScrollAlign>> {
protected:
    void CreateShortcutList()
    {
        const auto [axis, rtl, stackFromEnd, knownSize, backward, align] = GetParam();
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        ListModelNG model;
        model.Create();
        ViewAbstract::SetWidth(CalcLength(WIDTH));
        ViewAbstract::SetHeight(CalcLength(HEIGHT));
        GetList();
        model.SetScroller(model.CreateScrollController(), AceType::MakeRefPtr<ScrollBarProxy>());
        model.SetListDirection(axis);
        model.SetStackFromEnd(stackFromEnd);
        model.SetInitialIndex(SHORTCUT_INITIAL_INDEX);
        layoutProperty_->UpdateLayoutDirection(rtl ? TextDirection::RTL : TextDirection::LTR);
        if (knownSize) {
            model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        for (int32_t index = 0; index < FAST_SCROLL_ITEM_COUNT; ++index) {
            auto child = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ListItemPattern>(nullptr));
            const CalcLength mainSize(ITEM_MAIN_SIZE);
            const CalcLength crossSize(FILL_LENGTH);
            child->GetLayoutProperty()->UpdateUserDefinedIdealSize(axis == Axis::VERTICAL ?
                CalcSize(crossSize, mainSize) : CalcSize(mainSize, crossSize));
            child->MountToParent(frameNode_);
        }
        CreateDone();
    }
};

/**
 * @tc.name: FastScrollVisibleTargetStartsImmediately
 * @tc.desc: A clean ordinary target uses current geometry without requesting neighborhood premeasurement.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollVisibleShortcutTestNg, FastScrollVisibleTargetStartsImmediately, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateShortcutList());
    const bool backward = std::get<4>(GetParam());
    const auto align = std::get<5>(GetParam());
    ASSERT_GE(pattern_->itemPosition_.size(), 3);
    const int32_t target = backward ? pattern_->itemPosition_.begin()->first + 1 :
        pattern_->itemPosition_.rbegin()->first - 1;
    const auto positions = pattern_->itemPosition_;
    const float origin = pattern_->GetTotalOffset();
    const float available = pattern_->contentMainSize_ - ITEM_MAIN_SIZE;
    float alignedStart = align == ScrollAlign::CENTER ? available / 2.0f : 0.0f;
    if (align == ScrollAlign::END) {
        alignedStart = available;
    }
    const float expected = align == ScrollAlign::AUTO ? origin : target * ITEM_MAIN_SIZE - alignedStart;
    auto child = GetChildFrameNode(frameNode_, target);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(ElementRegister::GetInstance()->GetUINodeById(child->GetId()), child);
    pattern_->ScrollToIndexFastScroll(target, true, align, std::nullopt, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    if (!NearEqual(expected, origin)) {
        ASSERT_TRUE(pattern_->AnimateRunning());
        ASSERT_TRUE(pattern_->HasFastScrollRequest());
        EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
        EXPECT_NEAR(pattern_->GetFinalPosition(), expected, 0.1f);
        auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
        ASSERT_NE(algorithm, nullptr);
        EXPECT_FALSE(algorithm->GetOrCreateFastScrollState().input.preparationOffset.has_value());
    }
    EXPECT_EQ(pattern_->itemPosition_.size(), positions.size());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), origin);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), expected, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(CurrentWindow, ListFastScrollVisibleShortcutTestNg,
    testing::Combine(testing::Values(Axis::VERTICAL, Axis::HORIZONTAL), testing::Bool(), testing::Bool(),
        testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO)));

class ListFastScrollShortcutGuardTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<bool> {};

/**
 * @tc.name: FastScrollVisibleTargetKeepsPreparationGuards
 * @tc.desc: Extra offset still prepares; ordinary targets reuse recorded positions even when List is dirty.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollShortcutGuardTestNg, FastScrollVisibleTargetKeepsPreparationGuards, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    const int32_t target = 1;
    const bool withExtraOffset = GetParam();
    if (!withExtraOffset) {
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    }
    pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, withExtraOffset ? 25.0f : 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->AnimateRunning(), !withExtraOffset);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->NeedsPreparation(), withExtraOffset);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), target * ITEM_MAIN_SIZE + (withExtraOffset ? 25.0f : 0.0f), 0.1f);
}

INSTANTIATE_TEST_SUITE_P(Preparation, ListFastScrollShortcutGuardTestNg, testing::Bool());

class ListFastScrollVisiblePositionTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, int32_t>> {};

/**
 * @tc.name: FastScrollVisibleTargetMatchesLegacyPositionSource
 * @tc.desc: Both entrances start from itemPosition despite pending child layout or absent constraint metadata.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollVisiblePositionTestNg, FastScrollVisibleTargetMatchesLegacyPositionSource, TestSize.Level1)
{
    const auto [fast, knownSize, state] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    constexpr int32_t target = 1;
    auto child = GetChildFrameNode(frameNode_, target);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(pattern_->itemPosition_.count(target), 1);
    const auto position = pattern_->itemPosition_.at(target);
    if (state == 0) {
        child->MarkDirtyNode(true, true, PROPERTY_UPDATE_MEASURE_SELF);
    } else if (state == 1) {
        child->isLayoutComplete_ = false;
    } else {
        child->GetGeometryNode()->ResetParentLayoutConstraint();
    }
    ASSERT_FALSE(frameNode_->IsLayoutDirtyMarked());
    if (fast) {
        pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
        auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
        ASSERT_NE(algorithm, nullptr);
        EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    } else {
        pattern_->ScrollToIndex(target, true, ScrollAlign::START, 0.0f);
    }
    ASSERT_TRUE(pattern_->AnimateRunning());
    EXPECT_NEAR(pattern_->GetFinalPosition(), position.startPos, 0.1f);
    EXPECT_FLOAT_EQ(pattern_->itemPosition_.at(target).startPos, position.startPos);
    EXPECT_FLOAT_EQ(pattern_->itemPosition_.at(target).endPos, position.endPos);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), ITEM_MAIN_SIZE, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(PositionSource, ListFastScrollVisiblePositionTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Values(0, 1, 2)));

/**
 * @tc.name: FastScrollVisibleLargeItemKeepsCompression001
 * @tc.desc: A visible item's distant END uses immediate fast animation rather than the uncompressed legacy path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollVisibleLargeItemKeepsCompression001, TestSize.Level1)
{
    CreateList();
    CreateListItem();
    ViewAbstract::SetHeight(CalcLength(5000.0f));
    ViewStackProcessor::GetInstance()->Pop();
    CreateListItems(20);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::END, std::nullopt, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->AnimateRunning());
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_GT(pattern_->GetFastScrollRequest()->skipDistance, 0.0f);
    EXPECT_NEAR(pattern_->GetFinalPosition(), 5000.0f - HEIGHT, 0.1f);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetItemRect(0).Bottom(), HEIGHT, 0.1f);
}

/**
 * @tc.name: FastScrollVisibleTargetUsesUpdatedLayout001
 * @tc.desc: A visible ordinary target uses normal layout updates without requesting offscreen preparation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollVisibleTargetUsesUpdatedLayout001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(1, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->AnimateRunning());
    ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(WIDTH / 2.0f));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_NEAR(pattern_->GetItemRect(1).Top(), 0.0f, 0.1f);
}

/**
 * @tc.name: FastScrollVisibleGroupStillPrepares001
 * @tc.desc: Presence of a visible Group is not a guarantee of complete internal boundary geometry.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollVisibleGroupStillPrepares001, TestSize.Level1)
{
    CreateList();
    CreateListItemGroups(10);
    CreateDone();
    ASSERT_EQ(pattern_->itemPosition_.count(1), 1);
    ASSERT_TRUE(pattern_->itemPosition_.at(1).isGroup);
    pattern_->ScrollToIndexFastScroll(1, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->AnimateRunning());
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetItemRect(1).Top(), 0.0f, 0.1f);
}

class ListFastScrollOffsetTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, float>> {};

class ListFastScrollReplacementTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool, bool>> {};

class ListFastScrollTouchTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

/**
 * @tc.name: FastScrollTouchDownKeepsControllerAnimation
 * @tc.desc: Touch down preserves controller animation; a subsequent drag displacement takes over fast scroll.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollTouchTestNg, FastScrollTouchDownKeepsControllerAnimation, TestSize.Level1)
{
    const auto [fast, drag] = GetParam();
    CreateList();
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(8);
    positionController_->ScrollToIndexWithOptions(80, true, ScrollAlign::START, std::nullopt,
        fast ? FastScrollMode::SKIP_CREATE_AND_MEASURE : FastScrollMode::NONE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    auto animation = pattern_->springAnimation_;
    ASSERT_TRUE(pattern_->touchEvent_);
    TouchLocationInfo touch(0);
    touch.SetTouchType(TouchType::DOWN);
    TouchLocationInfo changedTouch(touch);
    TouchEventInfo info("fastScrollTouch");
    info.AddTouchLocationInfo(std::move(touch));
    info.AddChangedTouchLocationInfo(std::move(changedTouch));
    pattern_->touchEvent_->GetTouchEventCallback()(info);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->springAnimation_, animation);
    EXPECT_EQ(pattern_->HasFastScrollRequest(), fast);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_GT(pattern_->GetTotalOffset(), 0.0f);
    if (fast && drag) {
        pattern_->OnScrollCallback(-40.0f, SCROLL_FROM_UPDATE);
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_FALSE(pattern_->HasFastScrollRequest());
        FlushUITasks();
        const float offset = pattern_->GetTotalOffset();
        ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
        EXPECT_NEAR(pattern_->GetTotalOffset(), offset, 0.1f);
    } else {
        ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
        EXPECT_NEAR(pattern_->GetTotalOffset(), 80 * ITEM_MAIN_SIZE, 0.1f);
    }
    pattern_->GetScrollable()->HandleTouchUp();
}

INSTANTIATE_TEST_SUITE_P(Interaction, ListFastScrollTouchTestNg,
    testing::Values(std::make_tuple(false, false), std::make_tuple(true, false), std::make_tuple(true, true)));

/**
 * @tc.name: FastScrollTouchDownKeepsPreparation001
 * @tc.desc: Touch down while waiting for neighborhood measurement does not cancel the pending request.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollTouchDownKeepsPreparation001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    positionController_->ScrollToIndexWithOptions(80, true, ScrollAlign::START, 600.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    ASSERT_FALSE(pattern_->AnimateRunning());
    TouchEventInfo info("fastScrollPreparation");
    pattern_->OnTouchDown(info);
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), 80 * ITEM_MAIN_SIZE + 600.0f, 0.1f);
}

/**
 * @tc.name: FastScrollSequentialReplacement
 * @tc.desc: A normal subsequent fast or legacy request replaces a running request and reaches its own target.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollReplacementTestNg, FastScrollSequentialReplacement, TestSize.Level1)
{
    const auto [knownSize, withExtraOffset, fast, smooth] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, withExtraOffset ? 600.0f : 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());

    constexpr int32_t replacementIndex = 20;
    constexpr float extraOffset = 35.0f;
    positionController_->ScrollToIndexWithOptions(replacementIndex, smooth, ScrollAlign::START, extraOffset,
        fast ? FastScrollMode::SKIP_CREATE_AND_MEASURE : FastScrollMode::NONE);
    if (fast && smooth) {
        ASSERT_TRUE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(pattern_->GetFastScrollRequest()->index, replacementIndex);
        EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, extraOffset);
    } else {
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
    }
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_NEAR(pattern_->GetTotalOffset(), replacementIndex * ITEM_MAIN_SIZE + extraOffset, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(replacementIndex).Top(), -extraOffset, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(Sequential, ListFastScrollReplacementTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollDisabledKeepsZeroCallbackBehavior001
 * @tc.desc: A zero displacement returned to a legacy animation does not cancel it as a fast-scroll request.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollDisabledKeepsZeroCallbackBehavior001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(8);
    positionController_->ScrollToIndexWithOptions(80, true, ScrollAlign::START, std::nullopt, FastScrollMode::NONE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    const float delta = pattern_->currentDelta_;
    eventHub_->SetOnWillScroll([](Dimension, ScrollState, ScrollSource) {
        return ScrollFrameResult { Dimension(0.0f) };
    });
    EXPECT_TRUE(pattern_->UpdateCurrentOffset(-20.0f, SCROLL_FROM_ANIMATION_CONTROLLER));
    EXPECT_FLOAT_EQ(pattern_->currentDelta_, delta);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    eventHub_->SetOnWillScroll(nullptr);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), 0.0f, 0.1f);
}

enum class FastScrollCallbackMode { ZERO, HALF, HALF_ONCE, UNCHANGED };

class ListFastScrollCallbackTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, FastScrollCallbackMode>> {};

class FastScrollCountingLazy : public ListItemMockLazy {
public:
    explicit FastScrollCountingLazy(int32_t count) : ListItemMockLazy(count, ITEM_MAIN_SIZE) {}

    const std::vector<int32_t>& GetCreatedIndices() const
    {
        return createdIndices_;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        createdIndices_.push_back(index);
        return ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
    }

private:
    std::vector<int32_t> createdIndices_;
};

class FastScrollPrecisionLazy final : public Framework::MockLazyForEachBuilder {
public:
    static constexpr int32_t TOTAL_COUNT = 1000;
    static constexpr int32_t TARGET_INDEX = 800;
    static constexpr float itemHeight = 128.0078125f;
    static constexpr float targetHeight = 208.0078125f;

    int32_t OnGetTotalCount() override
    {
        return TOTAL_COUNT;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& /* expiringItems */) override
    {
        ListItemModelNG model;
        model.Create();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        const float height = index == TARGET_INDEX ? targetHeight : itemHeight;
        ViewAbstract::SetHeight(CalcLength(height));
        return { std::to_string(index), ViewStackProcessor::GetInstance()->Finish() };
    }
};

class FastScrollLifecycleLazy final : public ListItemMockLazy {
public:
    FastScrollLifecycleLazy() : ListItemMockLazy(FAST_SCROLL_ITEM_COUNT, ITEM_MAIN_SIZE) {}

    void ReloadSizes()
    {
        reloaded_ = true;
    }

    float GetMainSize(int32_t index) const
    {
        return reloaded_ ?
            (index % ALTERNATING_HEIGHT_PERIOD == 0 ? RELOADED_SHORT_HEIGHT : RELOADED_TALL_HEIGHT) : ITEM_MAIN_SIZE;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& /* expiringItems */) override
    {
        ListItemModelNG model;
        model.Create();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(GetMainSize(index)));
        auto node = ViewStackProcessor::GetInstance()->Finish();
        return { std::to_string(reloaded_) + ":" + std::to_string(index), node };
    }

private:
    bool reloaded_ = false;
};

class ListFastScrollLifecycleTestNg : public ListFastScrollerTestNg {
protected:
    void CreateLifecycleList(bool knownSize)
    {
        auto model = CreateList();
        if (knownSize) {
            model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        builder_ = AceType::MakeRefPtr<FastScrollLifecycleLazy>();
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(builder_));
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        lazyNode_ = AceType::DynamicCast<LazyForEachNode>(frameNode_->GetChildAtIndex(0));
        ASSERT_NE(lazyNode_, nullptr);
    }

    void ReloadLifecycleSizes(bool knownSize)
    {
        builder_->ReloadSizes();
        if (knownSize) {
            std::vector<float> sizes;
            for (int32_t index = 0; index < FAST_SCROLL_ITEM_COUNT; ++index) {
                sizes.push_back(builder_->GetMainSize(index));
            }
            pattern_->childrenSize_->ChangeData(0, FAST_SCROLL_ITEM_COUNT, sizes);
        }
        lazyNode_->OnDataReloaded();
        MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    }

    RefPtr<FastScrollLifecycleLazy> builder_;
    RefPtr<LazyForEachNode> lazyNode_;
};

class ListFastScrollMutationTestNg : public ListFastScrollLifecycleTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool, int32_t>> {};

class ListFastScrollReloadTestNg : public ListFastScrollLifecycleTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

class ListFastScrollDetachTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

/**
 * @tc.name: FastScrollLifecycleTargetRemoved
 * @tc.desc: An invalidated pending or running request cannot revive when data returns.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMutationTestNg, FastScrollLifecycleTargetRemoved, TestSize.Level1)
{
    const auto [knownSize, pending, reload, remainingCount] = GetParam();
    CreateLifecycleList(knownSize);
    if (!pending) {
        CreateDone();
    }
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    if (!pending) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ASSERT_TRUE(pattern_->AnimateRunning());
    }
    builder_->SetTotalCount(remainingCount);
    if (reload) {
        lazyNode_->OnDataReloaded();
    } else {
        lazyNode_->OnDataBulkDeleted(remainingCount, 100 - remainingCount);
    }
    if (pending) {
        CreateDone();
    } else {
        FlushUITasks();
    }
    EXPECT_EQ(pattern_->maxListItemIndex_, remainingCount - 1);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    builder_->SetTotalCount(100);
    lazyNode_->OnDataReloaded();
    FlushUITasks();
    const auto restoredOffset = pattern_->GetTotalOffset();
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_NEAR(pattern_->GetTotalOffset(), restoredOffset, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(Lifecycle, ListFastScrollMutationTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool(), testing::Values(0, 40)));

/**
 * @tc.name: FastScrollLifecycleSameCountReload
 * @tc.desc: Reloaded sizes converge to the actual target without reviving a finished request.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollReloadTestNg, FastScrollLifecycleSameCountReload, TestSize.Level1)
{
    const auto [knownSize, pending] = GetParam();
    CreateLifecycleList(knownSize);
    if (!pending) {
        CreateDone();
        ScrollTo(20 * ITEM_MAIN_SIZE);
    }
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    if (!pending) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
    }
    ReloadLifecycleSizes(knownSize);
    if (pending) {
        CreateDone();
    } else {
        FlushUITasks();
    }
    RunFastScrollFrames(64);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), 0.0f, 0.1f);
    if (knownSize) {
        EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * (60.0f + 180.0f), 0.1f);
    }
}

INSTANTIATE_TEST_SUITE_P(Lifecycle, ListFastScrollReloadTestNg,
    testing::Combine(testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollLifecycleModifiedOffsetRebase001
 * @tc.desc: A layout rebase after nonzero callback editing must not end the physical segment early.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollLifecycleModifiedOffsetRebase001, TestSize.Level1)
{
    auto model = CreateList();
    auto sizes = model.GetOrCreateListChildrenMainSize();
    sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
    CreateListItems(100);
    CreateDone();
    ScrollTo(20 * ITEM_MAIN_SIZE);
    auto stops = std::make_shared<int32_t>(0);
    eventHub_->SetOnScrollStop([stops]() { ++*stops; });
    eventHub_->SetOnWillScroll([](Dimension offset, ScrollState, ScrollSource) {
        return ScrollFrameResult { Dimension(offset.Value() / 2, offset.Unit()) };
    });
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    auto firstItem = GetChildFrameNode(frameNode_, 0);
    ViewAbstract::SetHeight(AceType::RawPtr(firstItem), CalcLength(2 * ITEM_MAIN_SIZE));
    sizes->ChangeData(0, 1, { 2 * ITEM_MAIN_SIZE });
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushUITasks();
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(*stops, 0);
    RunFastScrollFrames();
    EXPECT_EQ(*stops, 1);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    eventHub_->SetOnWillScroll(nullptr);
    eventHub_->SetOnScrollStop(nullptr);
}

/**
 * @tc.name: FastScrollLifecycleDetach
 * @tc.desc: Main-tree detach cancels pending/running requests, including thread-safe List nodes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollDetachTestNg, FastScrollLifecycleDetach, TestSize.Level1)
{
    const auto [threadSafe, pending] = GetParam();
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(threadSafe);
    CreateList();
    CreateListItems(100);
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(false);
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    MountToStageNode(frameNode_);
    if (!pending) {
        FlushUITasks();
    }
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    if (!pending) {
        FlushUITasks();
    }
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->AnimateRunning(), !pending);
    RemoveFromStageNode();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    MountToStageNode(frameNode_);
    FlushUITasks();
    const auto offset = pattern_->GetTotalOffset();
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), offset, 0.1f);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
}

INSTANTIATE_TEST_SUITE_P(Lifecycle, ListFastScrollDetachTestNg,
    testing::Combine(testing::Bool(), testing::Bool()));

class ListFastScrollFreeNodeTestNg : public ListFastScrollerTestNg {
public:
    void SetUp() override
    {
        ListFastScrollerTestNg::SetUp();
        auto context = MockPipelineContext::GetCurrent();
        density_ = context->GetDensity();
        dipScale_ = context->GetDipScale();
        threadSafeScope_ = MultiThreadBuildManager::IsThreadSafeNodeScope();
    }

    void TearDown() override
    {
        auto context = MockPipelineContext::GetCurrent();
        context->SetDensity(density_);
        context->PipelineBase::dipScale_ = dipScale_;
        MultiThreadBuildManager::SetIsThreadSafeNodeScope(threadSafeScope_);
        ListFastScrollerTestNg::TearDown();
    }

protected:
    void SetTestDensity(double density)
    {
        auto context = MockPipelineContext::GetCurrent();
        context->SetDensity(density);
        context->PipelineBase::dipScale_ = density;
    }

    bool threadSafeScope_ = false;

private:
    double density_ = 1.0;
    double dipScale_ = 1.0;
};

/**
 * @tc.name: FastScrollWillScrollFloatRoundTrip001
 * @tc.desc: Float round-trip and a half-distance callback apply their returned displacement.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollFreeNodeTestNg, FastScrollWillScrollFloatRoundTrip001, TestSize.Level1)
{
    constexpr float testDensity = 1.5f;
    constexpr float requestedOffset = 250703.328125f;
    SetTestDensity(testDensity);
    CreateList();
    CreateListItems(100);
    CreateDone();
    eventHub_->SetOnWillScroll([](Dimension offset, ScrollState, ScrollSource) {
        return ScrollFrameResult { Dimension(static_cast<float>(offset.Value()), offset.Unit()) };
    });
    pattern_->scrollTarget_.emplace();
    pattern_->scrollTarget_->fastScrollMode = FastScrollMode::SKIP_CREATE_AND_MEASURE;
    pattern_->currentDelta_ = 0.0f;
    EXPECT_TRUE(pattern_->UpdateCurrentOffset(-requestedOffset, SCROLL_FROM_ANIMATION_CONTROLLER));
    EXPECT_NEAR(pattern_->currentDelta_, requestedOffset, 0.1f);

    eventHub_->SetOnWillScroll([](Dimension offset, ScrollState, ScrollSource) {
        return ScrollFrameResult { Dimension(offset.Value() / 2.0, offset.Unit()) };
    });
    pattern_->currentDelta_ = 0.0f;
    EXPECT_TRUE(pattern_->UpdateCurrentOffset(-requestedOffset, SCROLL_FROM_ANIMATION_CONTROLLER));
    EXPECT_NEAR(pattern_->currentDelta_, requestedOffset / 2.0f, 0.1f);
    eventHub_->SetOnWillScroll(nullptr);
}

/**
 * @tc.name: FastScrollWillScrollFloatRoundTripRetarget001
 * @tc.desc: ArkTS float round-trip precision loss must not suppress target geometry correction.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollFreeNodeTestNg, FastScrollWillScrollFloatRoundTripRetarget001, TestSize.Level1)
{
    SetTestDensity(1.3);
    CreateList();
    auto builder = AceType::MakeRefPtr<FastScrollPrecisionLazy>();
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazyModel;
    lazyModel.Create(RefPtr<LazyForEachBuilder>(builder));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateDone();

    auto precisionLossCount = std::make_shared<int32_t>(0);
    eventHub_->SetOnWillScroll([precisionLossCount](Dimension offset, ScrollState, ScrollSource) {
        Dimension floatOffset(static_cast<float>(offset.Value()), offset.Unit());
        if (!NearEqual(offset.ConvertToPx(), floatOffset.ConvertToPx())) {
            ++*precisionLossCount;
        }
        return ScrollFrameResult { floatOffset };
    });
    // Non-power-of-two sampling avoids only exactly representable round trips after retargeting.
    MockAnimationManager::GetInstance().SetTicks(7);
    pattern_->ScrollToIndexFastScroll(
        FastScrollPrecisionLazy::TARGET_INDEX, true, ScrollAlign::END, std::nullopt,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();

    EXPECT_GT(*precisionLossCount, 0);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_EQ(pattern_->itemPosition_.count(FastScrollPrecisionLazy::TARGET_INDEX), 1);
    const auto targetRect = pattern_->GetItemRect(FastScrollPrecisionLazy::TARGET_INDEX);
    EXPECT_NEAR(targetRect.Height(), FastScrollPrecisionLazy::targetHeight, 0.1f);
    EXPECT_NEAR(targetRect.Bottom(), pattern_->contentMainSize_, 0.1f);
    eventHub_->SetOnWillScroll(nullptr);
}

/**
 * @tc.name: FastScrollLifecycleFreeNodeOffsetUnit001
 * @tc.desc: Deferred fast pixel offsets are unaffected by attach-time density, as are legacy offsets.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollFreeNodeTestNg, FastScrollLifecycleFreeNodeOffsetUnit001, TestSize.Level1)
{
    SetTestDensity(1.0);
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(true);
    CreateList();
    CreateListItems(100);
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(threadSafeScope_);
    ASSERT_TRUE(frameNode_->IsFree());
    const auto taskCount = frameNode_->afterAttachMainTreeTasks_.size();
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, true,
        ScrollAlign::START, 12.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_EQ(frameNode_->afterAttachMainTreeTasks_.size(), taskCount + 1);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    SetTestDensity(2.0);
    MockAnimationManager::GetInstance().SetTicks(4);
    CreateDone();
    EXPECT_FALSE(frameNode_->IsFree());
    EXPECT_TRUE(frameNode_->afterAttachMainTreeTasks_.empty());
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, 12.0f);
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -12.0f, 0.1f);
    ListModelNG::SetScrollToIndex(AceType::RawPtr(frameNode_), 20, 1,
        static_cast<int32_t>(ScrollAlign::START), 12.0f);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    FlushUITasks();
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(20), 1);
    EXPECT_NEAR(pattern_->GetItemRect(20).Top(), -12.0f, 0.1f);
}

class ListFastScrollContextTestNg : public ListFastScrollFreeNodeTestNg {
public:
    void SetUp() override
    {
        ListFastScrollFreeNodeTestNg::SetUp();
        apiTargetVersion_ = MockContainer::Current()->GetApiTargetVersion();
        MockContainer::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWENTY_SIX));
    }

    void TearDown() override
    {
        ListFastScrollFreeNodeTestNg::TearDown();
        MockContainer::Current()->SetApiTargetVersion(apiTargetVersion_);
    }

protected:
    void CreateUnattachedFreeList(bool nested, int32_t laneMode = 0)
    {
        MultiThreadBuildManager::SetIsThreadSafeNodeScope(true);
        auto model = CreateList();
        if (laneMode == 1) {
            model.SetLanes(MULTI_LANE_COUNT);
        } else if (laneMode == CONSTRAINED_LANE_MODE) {
            model.SetLaneMinLength(Dimension(WIDTH / MULTI_LANE_COUNT, DimensionUnit::PX));
            model.SetLaneMaxLength(Dimension(WIDTH / MULTI_LANE_COUNT, DimensionUnit::PX));
        }
        CreateListItems(FAST_SCROLL_ITEM_COUNT);
        ViewStackProcessor::GetInstance()->Finish();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        root_ = frameNode_;
        if (nested) {
            root_ = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StackPattern>());
            root_->AddChild(frameNode_);
        }
        MultiThreadBuildManager::SetIsThreadSafeNodeScope(threadSafeScope_);
        ASSERT_TRUE(frameNode_->IsFree());
        ASSERT_EQ(frameNode_->GetAttachedContext(), nullptr);
    }

    void DrainBeforeAttach()
    {
        root_->MarkNodeNotFree();
        ASSERT_FALSE(frameNode_->IsFree());
        EXPECT_TRUE(frameNode_->afterAttachMainTreeTasks_.empty());
        ASSERT_EQ(frameNode_->GetAttachedContext(), nullptr);
        EXPECT_FALSE(pattern_->AnimateRunning());
    }

    void AttachAndLayout(double density)
    {
        SetTestDensity(density);
        MockAnimationManager::GetInstance().SetTicks(LONG_ANIMATION_TICKS);
        MountToStageNode(root_);
        auto context = MockPipelineContext::GetCurrent();
        ASSERT_EQ(frameNode_->GetAttachedContext(), AceType::RawPtr(context));
        FlushUITasks();
        // The initial jump can leave a legacy target waiting for its own layout before animation starts.
        if (pattern_->targetIndex_ || pattern_->jumpIndex_) {
            FlushUITasks();
        }
    }

    RefPtr<FrameNode> root_;

private:
    int32_t apiTargetVersion_ = 0;
};

class ListFastScrollAttachedUnitTestNg : public ListFastScrollContextTestNg,
    public testing::WithParamInterface<std::tuple<bool, DimensionUnit>> {};

/**
 * @tc.name: FastScrollFreeNodeKeepsPixelOffset
 * @tc.desc: Preconverted pixel offsets survive deferred dispatch and attachment to a different-density context.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollAttachedUnitTestNg, FastScrollFreeNodeKeepsPixelOffset, TestSize.Level1)
{
    const auto [nested, unit] = GetParam();
    const float offset = unit == DimensionUnit::VP ? 18.0f : 12.0f;
    SetTestDensity(1.5);
    ASSERT_NO_FATAL_FAILURE(CreateUnattachedFreeList(nested));
    const auto taskCount = frameNode_->afterAttachMainTreeTasks_.size();
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, true,
        ScrollAlign::START, offset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_EQ(frameNode_->afterAttachMainTreeTasks_.size(), taskCount + 1);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_NO_FATAL_FAILURE(DrainBeforeAttach());
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, offset);
    pattern_->TryStartFastScroll();
    EXPECT_FALSE(pattern_->AnimateRunning());
    ASSERT_NO_FATAL_FAILURE(AttachAndLayout(2.5));
    const float expected = offset;
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, expected);
    SetTestDensity(3.0);
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, expected);
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -expected, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 80 * ITEM_MAIN_SIZE + expected, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(FinalContext, ListFastScrollAttachedUnitTestNg,
    testing::Combine(testing::Bool(), testing::Values(DimensionUnit::VP, DimensionUnit::PX)));

class ListFastScrollLegacyUnitTestNg : public ListFastScrollContextTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

/**
 * @tc.name: FastScrollFreeNodeLegacyOffset
 * @tc.desc: False or non-animated options retain all legacy arguments, including the raw 12px offset.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLegacyUnitTestNg, FastScrollFreeNodeLegacyOffset, TestSize.Level1)
{
    const auto [smooth, enabled] = GetParam();
    SetTestDensity(1.5);
    ASSERT_NO_FATAL_FAILURE(CreateUnattachedFreeList(true));
    const auto taskCount = frameNode_->afterAttachMainTreeTasks_.size();
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, smooth,
        ScrollAlign::START, 12.0f, enabled ? FastScrollMode::SKIP_CREATE_AND_MEASURE : FastScrollMode::NONE);
    EXPECT_EQ(frameNode_->afterAttachMainTreeTasks_.size(), taskCount + 1);
    ASSERT_NO_FATAL_FAILURE(DrainBeforeAttach());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->smooth_, smooth);
    EXPECT_EQ(pattern_->scrollAlign_, ScrollAlign::START);
    if (smooth) {
        ASSERT_TRUE(pattern_->targetIndex_);
        EXPECT_EQ(pattern_->targetIndex_.value(), 80);
        ASSERT_TRUE(pattern_->GetExtraOffset());
        EXPECT_FLOAT_EQ(pattern_->GetExtraOffset().value(), 12.0f);
    } else {
        ASSERT_TRUE(pattern_->jumpIndex_);
        EXPECT_EQ(pattern_->jumpIndex_.value(), 80);
        EXPECT_FLOAT_EQ(pattern_->currentDelta_, 12.0f);
    }
    ASSERT_NO_FATAL_FAILURE(AttachAndLayout(2.5));
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -12.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 8012.0f, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(FinalContext, ListFastScrollLegacyUnitTestNg,
    testing::Values(std::make_tuple(true, false), std::make_tuple(false, true), std::make_tuple(false, false)));

class ListFastScrollMultiLaneUnitTestNg : public ListFastScrollContextTestNg,
    public testing::WithParamInterface<std::tuple<bool, DimensionUnit>> {};

/**
 * @tc.name: FastScrollFreeNodeMultiLaneOffset
 * @tc.desc: Multi-lane preparation preserves preconverted pixels across attachment and density changes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneUnitTestNg, FastScrollFreeNodeMultiLaneOffset, TestSize.Level1)
{
    const auto [inferredLanes, unit] = GetParam();
    const float offset = unit == DimensionUnit::VP ? 18.0f : 12.0f;
    SetTestDensity(1.5);
    ASSERT_NO_FATAL_FAILURE(CreateUnattachedFreeList(true, inferredLanes ? 2 : 1));
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, true,
        ScrollAlign::START, offset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_NO_FATAL_FAILURE(DrainBeforeAttach());
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, offset);
    ASSERT_NO_FATAL_FAILURE(AttachAndLayout(2.5));
    EXPECT_EQ(pattern_->lanes_, 2);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, offset);
    SetTestDensity(3.0);
    FlushUITasks();
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    const float expected = offset;
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -expected, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * ITEM_MAIN_SIZE + expected, 0.1f);
}

/**
 * @tc.name: FastScrollAttachedMultiLaneOffset
 * @tc.desc: An already laid-out multi-lane List uses the same pixel-only contract as a single-lane List.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneUnitTestNg, FastScrollAttachedMultiLaneOffset, TestSize.Level1)
{
    const auto [inferredLanes, unit] = GetParam();
    const float offset = unit == DimensionUnit::VP ? 30.0f : 12.0f;
    ASSERT_NO_FATAL_FAILURE(CreateUnattachedFreeList(true, inferredLanes ? 2 : 1));
    ASSERT_NO_FATAL_FAILURE(DrainBeforeAttach());
    ASSERT_NO_FATAL_FAILURE(AttachAndLayout(2.5));
    ASSERT_EQ(pattern_->lanes_, 2);
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, true,
        ScrollAlign::START, offset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, 80);
    const float expected = offset;
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, expected);
    SetTestDensity(3.0);
    FlushUITasks();
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -expected, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * ITEM_MAIN_SIZE + expected, 0.1f);
}

/**
 * @tc.name: FastScrollMultiLaneLegacyOffset
 * @tc.desc: Disabled and non-animated multi-lane requests keep the old raw offset for either declared unit.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneUnitTestNg, FastScrollMultiLaneLegacyOffset, TestSize.Level1)
{
    const auto [inferredLanes, unit] = GetParam();
    SCOPED_TRACE(static_cast<int32_t>(unit));
    ASSERT_NO_FATAL_FAILURE(CreateUnattachedFreeList(true, inferredLanes ? 2 : 1));
    ASSERT_NO_FATAL_FAILURE(DrainBeforeAttach());
    ASSERT_NO_FATAL_FAILURE(AttachAndLayout(2.5));
    ASSERT_EQ(pattern_->lanes_, 2);
    int32_t index = 80;
    for (const auto& [smooth, enabled] : { std::make_tuple(true, false),
        std::make_tuple(false, true), std::make_tuple(false, false) }) {
        SCOPED_TRACE(testing::Message() << smooth << "," << enabled);
        ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), index, smooth,
            ScrollAlign::START, 12.0f, enabled ? FastScrollMode::SKIP_CREATE_AND_MEASURE : FastScrollMode::NONE);
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(pattern_->smooth_, smooth);
        if (smooth) {
            ASSERT_TRUE(pattern_->GetExtraOffset());
            EXPECT_FLOAT_EQ(pattern_->GetExtraOffset().value(), 12.0f);
        } else {
            EXPECT_FLOAT_EQ(pattern_->currentDelta_, 12.0f);
        }
        FlushUITasks();
        RunFastScrollFrames();
        ASSERT_EQ(pattern_->itemPosition_.count(index), 1);
        EXPECT_NEAR(pattern_->GetItemRect(index).Top(), -12.0f, 0.1f);
        EXPECT_NEAR(pattern_->GetTotalOffset(), index / 2 * ITEM_MAIN_SIZE + 12.0f, 0.1f);
        index -= 20;
    }
}

INSTANTIATE_TEST_SUITE_P(FinalContext, ListFastScrollMultiLaneUnitTestNg,
    testing::Combine(testing::Bool(), testing::Values(DimensionUnit::VP, DimensionUnit::PX)));

/**
 * @tc.name: FastScrollPixelOffsetSurvivesContextReattachment
 * @tc.desc: A pixel request keeps its offset through detachment and reattachment without density conversion.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollFreeNodeTestNg, FastScrollPixelOffsetSurvivesContextReattachment, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    SetTestDensity(1.5);
    frameNode_->DetachContext();
    ASSERT_EQ(frameNode_->GetAttachedContext(), nullptr);
    ASSERT_NE(frameNode_->GetContext(), nullptr);
    ASSERT_TRUE(pattern_->isInitialized_);
    ASSERT_FALSE(pattern_->itemPosition_.empty());
    ListModelNG::SetScrollToIndexWithOptions(AceType::RawPtr(frameNode_), 80, true,
        ScrollAlign::START, 18.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, 18.0f);
    EXPECT_FALSE(pattern_->AnimateRunning());
    SetTestDensity(2.5);
    auto context = MockPipelineContext::GetCurrent();
    frameNode_->AttachContext(AceType::RawPtr(context));
    MockAnimationManager::GetInstance().SetTicks(8);
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, 18.0f);
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), -18.0f, 0.1f);
}

/**
 * @tc.name: FastScrollLazySkipsMiddle001
 * @tc.desc: Count every generator invocation, including nodes later recycled, across a long jump.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollLazySkipsMiddle001, TestSize.Level1)
{
    CreateList();
    auto builder = AceType::MakeRefPtr<FastScrollCountingLazy>(1000);
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazyModel;
    lazyModel.Create(RefPtr<LazyForEachBuilder>(builder));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateDone();
    const auto initialCount = builder->GetCreatedIndices().size();
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(800, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_EQ(builder->GetCreatedIndices().size(), initialCount);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 800 * ITEM_MAIN_SIZE, 0.1f);
    ASSERT_EQ(pattern_->itemPosition_.count(800), 1);
    EXPECT_NEAR(pattern_->GetItemRect(800).Top(), 0.0f, 0.1f);
    for (int32_t index : builder->GetCreatedIndices()) {
        EXPECT_TRUE(index < 100 || index > 700) << "Unexpected intermediate generator call: " << index;
    }
}

/**
 * @tc.name: FastScrollSingleLaneAlignment
 * @tc.desc: Real layout reaches the target rectangle in both size modes and directions with one start/stop pair.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollTestNg, FastScrollSingleLaneAlignment, TestSize.Level1)
{
    const auto [knownSize, backward, align, extraOffset] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    if (backward) {
        ScrollTo(80 * ITEM_MAIN_SIZE);
    }
    auto starts = std::make_shared<int32_t>(0);
    auto stops = std::make_shared<int32_t>(0);
    eventHub_->SetOnScrollStart([starts]() { ++*starts; });
    eventHub_->SetOnScrollStop([stops]() { ++*stops; });
    MockAnimationManager::GetInstance().SetTicks(4);
    const int32_t targetIndex = backward ? 20 : 80;
    const float available = pattern_->contentMainSize_ - ITEM_MAIN_SIZE;
    float alignedTop = align == ScrollAlign::CENTER ? available / 2 : 0.0f;
    if (align == ScrollAlign::END || (align == ScrollAlign::AUTO && !backward)) {
        alignedTop = available;
    }
    positionController_->ScrollToIndexWithOptions(targetIndex, true, align, extraOffset,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), targetIndex * ITEM_MAIN_SIZE - alignedTop + extraOffset, 0.1f);
    ASSERT_EQ(pattern_->itemPosition_.count(targetIndex), 1);
    EXPECT_NEAR(pattern_->GetItemRect(targetIndex).Top(), alignedTop - extraOffset, 0.1f);
    EXPECT_EQ(*starts, 1);
    EXPECT_EQ(*stops, 1);
    eventHub_->SetOnScrollStart(nullptr);
    eventHub_->SetOnScrollStop(nullptr);
}

INSTANTIATE_TEST_SUITE_P(SingleLane, ListFastScrollTestNg,
    testing::Combine(testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO),
        testing::Values(-25.0f, 0.0f, 25.0f)));

/**
 * @tc.name: FastScrollDirectionAlignment
 * @tc.desc: Logical target alignment also reaches the correct physical rectangle in RTL and stackFromEnd.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollDirectionTestNg, FastScrollDirectionAlignment, TestSize.Level1)
{
    const auto [axis, rtl, stackFromEnd, knownSize, align] = GetParam();
    auto model = CreateList();
    model.SetListDirection(axis);
    model.SetStackFromEnd(stackFromEnd);
    layoutProperty_->UpdateLayoutDirection(rtl ? TextDirection::RTL : TextDirection::LTR);
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    const int32_t targetIndex = stackFromEnd ? 20 : 80;
    const float available = pattern_->contentMainSize_ - ITEM_MAIN_SIZE;
    float alignedStart = align == ScrollAlign::CENTER ? available / 2 : 0.0f;
    if (align == ScrollAlign::END) {
        alignedStart = available;
    }
    pattern_->ScrollToIndexFastScroll(targetIndex, true, align, std::nullopt, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), targetIndex * ITEM_MAIN_SIZE - alignedStart, 0.1f);
    ASSERT_EQ(pattern_->itemPosition_.count(targetIndex), 1);
    const auto rect = pattern_->GetItemRect(targetIndex);
    const float physicalStart = axis == Axis::HORIZONTAL ? rect.Left() : rect.Top();
    EXPECT_NEAR(physicalStart, axis == Axis::HORIZONTAL && rtl ? available - alignedStart : alignedStart, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(SingleLane, ListFastScrollDirectionTestNg,
    testing::Combine(testing::Values(Axis::VERTICAL, Axis::HORIZONTAL), testing::Bool(), testing::Bool(),
        testing::Bool(), testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END)));

/**
 * @tc.name: FastScrollModelOptions001
 * @tc.desc: The native model forwards explicit options, while disabled and nonanimated calls stay on the old path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollModelOptions001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    auto* node = AceType::RawPtr(frameNode_);
    ListModelNG::SetScrollToIndexWithOptions(node, 80, true, ScrollAlign::START, 25.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 8025.0f, 0.1f);
    ListModelNG::SetScrollToIndexWithOptions(node, 20, false, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 2000.0f, 0.1f);
    ListModelNG::SetScrollToIndexWithOptions(node, 21, true, ScrollAlign::START, std::nullopt, FastScrollMode::NONE);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 2100.0f, 0.1f);
}

/**
 * @tc.name: FastScrollLargeTargetRetargets001
 * @tc.desc: A much larger unmeasured target corrects the endpoint and may skip again before END alignment.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollLargeTargetRetargets001, TestSize.Level1)
{
    CreateList();
    CreateListItems(80);
    CreateListItem();
    ViewAbstract::SetHeight(CalcLength(5000.0f));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(19);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(80), 1);
    EXPECT_NEAR(pattern_->GetItemRect(80).Bottom(), pattern_->contentMainSize_, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 13000.0f - pattern_->contentMainSize_, 0.1f);
}

/**
 * @tc.name: FastScrollPreparesLargeTargetBeforeAnimation001
 * @tc.desc: The large target is measured before animation and does not need a post-stop repair animation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollPreparesLargeTargetBeforeAnimation001, TestSize.Level1)
{
    auto model = CreateList();
    int32_t starts = 0;
    int32_t stops = 0;
    model.SetOnScrollStart([&starts]() { ++starts; });
    model.SetOnScrollStop([&stops]() { ++stops; });
    CreateListItems(80);
    CreateListItem();
    ViewAbstract::SetHeight(CalcLength(5000.0f));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(19);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_FALSE(pattern_->AnimateRunning());
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, 5000.0f);
    const float preparedEnd = pattern_->GetFinalPosition();
    EXPECT_NEAR(preparedEnd, 13000.0f - pattern_->contentMainSize_, 0.1f);
    ASSERT_EQ(pattern_->itemPosition_.count(80), 0);
    pattern_->ProcessSpringOffsetCallback(preparedEnd);
    ASSERT_FALSE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    FlushUITasks();
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetItemRect(80).Bottom(), pattern_->contentMainSize_, 0.1f);
    EXPECT_EQ(starts, stops);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollDisabledKeepsSingleCorrection001
 * @tc.desc: Disabling fast scroll preserves the existing target reset on the first animation correction.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollDisabledKeepsSingleCorrection001, TestSize.Level1)
{
    CreateList();
    CreateListItems(20);
    CreateDone();
    positionController_->ScrollToIndexWithOptions(3, true, ScrollAlign::START, std::nullopt, FastScrollMode::NONE);
    ASSERT_TRUE(pattern_->scrollTarget_);
    auto info = pattern_->posMap_->GetPositionInfo(3);
    info.mainPos += 50.0f;
    pattern_->posMap_->UpdatePos(3, info);
    pattern_->CheckAndUpdateAnimateTo(0.0f, 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), info.mainPos);
    EXPECT_FALSE(pattern_->scrollTarget_);
    info.mainPos += 50.0f;
    pattern_->posMap_->UpdatePos(3, info);
    pattern_->CheckAndUpdateAnimateTo(0.0f, 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), info.mainPos - 50.0f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollOnlySkippedBatchUsesPositionMap001
 * @tc.desc: Large continuous offsets keep normal layout; only a consumed compression skip may use an estimate.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollOnlySkippedBatchUsesPositionMap001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    algorithm->contentMainSize_ = HEIGHT;
    algorithm->totalItemCount_ = 100;
    algorithm->currentDelta_ = HEIGHT * 1.5f;
    algorithm->PrepareFastScrollSkipStart();
    EXPECT_FALSE(algorithm->GetFastScrollState()->skipStart);
    algorithm->SetFastScrollSkip(true);
    algorithm->PrepareFastScrollSkipStart();
    EXPECT_TRUE(algorithm->GetFastScrollState()->skipStart);
}

/**
 * @tc.name: FastScrollSkipSurvivesUntilLayout001
 * @tc.desc: A normal callback following a skip cannot erase the pending layout's skip marker.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollSkipSurvivesUntilLayout001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    const float end = pattern_->GetFinalPosition();
    const float viewport = pattern_->contentMainSize_;
    pattern_->ProcessSpringOffsetCallback(end - viewport);
    ASSERT_TRUE(pattern_->fastScrollSkip_);
    pattern_->ProcessSpringOffsetCallback(end - viewport / 2.0f);
    ASSERT_TRUE(pattern_->fastScrollSkip_);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().input.skip);
    EXPECT_FALSE(pattern_->fastScrollSkip_);
}

/**
 * @tc.name: FastScrollRetargetPreservesApproachProgress001
 * @tc.desc: Endpoint corrections cannot repeatedly defer the compressed skip by another two screens.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollRetargetPreservesApproachProgress001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    const float viewport = pattern_->contentMainSize_;
    for (int32_t i = 0; i < 3; ++i) {
        const float nextPosition = pattern_->lastPosition_ + viewport / 2.0f;
        pattern_->ProcessSpringOffsetCallback(nextPosition + pattern_->GetFastScrollRequest()->skipDistance);
        pattern_->StartFastScrollAnimation(pattern_->GetFinalPosition() + 10.0f);
        EXPECT_FALSE(pattern_->startingFastScroll_);
        const auto& request = *pattern_->GetFastScrollRequest();
        EXPECT_NEAR(request.targetOffset - pattern_->lastPosition_ - request.skipDistance,
            (3.5f - i * 0.5f) * viewport, 0.1f);
    }
    const auto& request = *pattern_->GetFastScrollRequest();
    pattern_->ProcessSpringOffsetCallback(request.targetOffset - 2.0f * viewport);
    EXPECT_TRUE(pattern_->fastScrollSkip_);
}

class ListFastScrollSkipWindowTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<bool> {};

/**
 * @tc.name: FastScrollRetainsTwoScreensOnEachSide
 * @tc.desc: Forward and backward compression skip at the boundary of two terminal animation screens.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollSkipWindowTestNg, FastScrollRetainsTwoScreensOnEachSide, TestSize.Level1)
{
    auto model = CreateList();
    model.SetInitialIndex(GetParam() ? 80 : 0);
    CreateListItems(100);
    CreateDone();
    const float direction = GetParam() ? -1.0f : 1.0f;
    const float viewport = pattern_->contentMainSize_;
    const float origin = pattern_->GetTotalOffset();
    ScrollToIndexTarget request;
    request.fastScrollMode = FastScrollMode::SKIP_CREATE_AND_MEASURE;
    request.viewportSize = viewport;
    request.targetOffset = origin + direction * 10.0f * viewport;
    request.skipDistance = direction * 6.0f * viewport;
    request.skipPending = true;
    pattern_->scrollTarget_ = request;
    const float boundary = request.targetOffset - direction * 2.0f * viewport;
    const float before = boundary - direction;
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(origin), -request.skipDistance);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(before), -request.skipDistance);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary + direction), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary - direction * 0.0005f), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary - direction * 0.0025f), -request.skipDistance);
    EXPECT_FLOAT_EQ(direction * (boundary - request.skipDistance - origin), 2.0f * viewport);
    EXPECT_FLOAT_EQ(direction * (request.targetOffset - boundary), 2.0f * viewport);
}

/**
 * @tc.name: FastScrollRetargetKeepsTwoTerminalScreens
 * @tc.desc: A pending skip retains at least two terminal screens after an endpoint correction.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollSkipWindowTestNg, FastScrollRetargetKeepsTwoTerminalScreens, TestSize.Level1)
{
    auto model = CreateList();
    model.SetInitialIndex(GetParam() ? 80 : 0);
    CreateListItems(100);
    CreateDone();
    const float direction = GetParam() ? -1.0f : 1.0f;
    const float viewport = pattern_->contentMainSize_;
    const float target = pattern_->GetTotalOffset() + direction * 10.0f * viewport;
    pattern_->scrollTarget_.emplace();
    pattern_->scrollTarget_->fastScrollMode = FastScrollMode::SKIP_CREATE_AND_MEASURE;
    pattern_->StartFastScrollAnimation(target);
    ASSERT_TRUE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->startingFastScroll_);
    ASSERT_TRUE(pattern_->GetFastScrollRequest()->skipPending);
    pattern_->ProcessSpringOffsetCallback(pattern_->lastPosition_ +
        pattern_->GetFastScrollRequest()->skipDistance + direction * 1.5f * viewport);
    ASSERT_TRUE(pattern_->GetFastScrollRequest()->skipPending);
    // A shorter corrected endpoint leaves less than two screens before the pending skip.
    pattern_->GetFastScrollRequest()->targetOffset -= direction * viewport;
    const float origin = pattern_->GetTotalOffset();
    pattern_->StartFastScrollAnimation(pattern_->GetFastScrollRequest()->targetOffset + direction * 10.0f);
    EXPECT_FALSE(pattern_->startingFastScroll_);
    const auto& request = *pattern_->GetFastScrollRequest();
    EXPECT_FALSE(request.skipPending);
    EXPECT_TRUE(pattern_->fastScrollSkip_);
    EXPECT_NEAR(direction * (request.targetOffset - origin - request.skipDistance),
        2.0f * viewport, 0.1f);
    EXPECT_NEAR(direction * (request.targetOffset - pattern_->lastPosition_),
        2.0f * viewport, 0.1f);
    const float boundary = request.targetOffset - direction * 2.0f * viewport;
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->CalcSpringAdjustOffset(boundary - direction), -request.skipDistance);
}

INSTANTIATE_TEST_SUITE_P(FastScrollDirections, ListFastScrollSkipWindowTestNg, testing::Bool());

class ListFastScrollPreparationRangeTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<ScrollAlign, bool, int32_t, bool>> {};

/**
 * @tc.name: FastScrollPreparesTwoScreensFromAlignmentLine
 * @tc.desc: Preparation connects extraOffset to the aligned boundary and covers the two-screen approach segment.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollPreparationRangeTestNg, FastScrollPreparesTwoScreensFromAlignmentLine, TestSize.Level1)
{
    const auto [align, forward, offsetCase, stackFromEnd] = GetParam();
    const std::array<float, 5> offsets { -1500.0f, -100.0f, 0.0f, 100.0f, 1500.0f };
    const std::array<std::pair<float, float>, 5> forwardRanges {
        std::pair { -2700.0f, 0.0f }, { -1300.0f, 0.0f }, { -1200.0f, 0.0f },
        { -1100.0f, 100.0f }, { 0.0f, 1500.0f }
    };
    const std::array<std::pair<float, float>, 5> backwardRanges {
        std::pair { -1500.0f, 0.0f }, { -100.0f, 1100.0f }, { 0.0f, 1200.0f },
        { 0.0f, 1300.0f }, { 0.0f, 2700.0f }
    };
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->contentMainSize_ = 600.0f;
    algorithm->contentStartOffset_ = 40.0f;
    algorithm->contentEndOffset_ = 30.0f;
    algorithm->isStackFromEnd_ = stackFromEnd;
    algorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm->posMap_->UpdatePos(10, { 4000.0f, 900.0f });
    algorithm->SetFastScrollTarget(10, align);
    algorithm->SetFastScrollPreparationOffset(offsets[offsetCase]);
    const float alignmentLine = align == ScrollAlign::START ? 0.0f :
        (align == ScrollAlign::END ? 900.0f : 450.0f);
    const float viewportLine = align == ScrollAlign::START ? (stackFromEnd ? 30.0f : 40.0f) :
        (align == ScrollAlign::END ? 600.0f - (stackFromEnd ? 40.0f : 30.0f) : 300.0f);
    const float finalStart = alignmentLine + offsets[offsetCase] - viewportLine;
    algorithm->SetTotalOffset(4000.0f + finalStart + (forward ? -5000.0f : 5000.0f));
    const auto range = algorithm->GetFastScrollPreparationRange(900.0f);
    const auto expected = forward ? forwardRanges[offsetCase] : backwardRanges[offsetCase];
    EXPECT_FLOAT_EQ(range.first, alignmentLine + expected.first);
    EXPECT_FLOAT_EQ(range.second, alignmentLine + expected.second);
    for (const float remaining : { 0.0f, 600.0f, 1200.0f }) {
        const float windowStart = finalStart + (forward ? -remaining : remaining);
        EXPECT_TRUE(LessOrEqual(range.first, windowStart + 600.0f));
        EXPECT_TRUE(GreatOrEqual(range.second, windowStart));
    }
}

INSTANTIATE_TEST_SUITE_P(AlignmentDirectionOffsetStack, ListFastScrollPreparationRangeTestNg,
    testing::Combine(testing::Values(ScrollAlign::START, ScrollAlign::END, ScrollAlign::CENTER),
        testing::Bool(), testing::Values(0, 1, 2, 3, 4), testing::Bool()));

/**
 * @tc.name: FastScrollUnpositionedTargetUsesOffsetDirection001
 * @tc.desc: Offset can reverse an unpositioned target's approach direction relative to its index order.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollUnpositionedTargetUsesOffsetDirection001, TestSize.Level1)
{
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->contentMainSize_ = 600.0f;
    algorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm->posMap_->UpdatePos(10, { 1000.0f, 100.0f });
    algorithm->SetTotalOffset(1000.0f);
    algorithm->SetFastScrollTarget(20, ScrollAlign::START);
    algorithm->SetFastScrollPreparationOffset(-1500.0f);
    EXPECT_EQ(algorithm->GetFastScrollPreparationRange(100.0f), std::make_pair(-1500.0f, 0.0f));
    algorithm->SetFastScrollTarget(0, ScrollAlign::START);
    algorithm->SetFastScrollPreparationOffset(1500.0f);
    EXPECT_EQ(algorithm->GetFastScrollPreparationRange(100.0f), std::make_pair(0.0f, 1500.0f));
    EXPECT_EQ(algorithm->posMap_->GetEntryAtOrBeforeIndex(20), 10);
    EXPECT_EQ(algorithm->posMap_->GetEntryAtOrAfterIndex(0), 10);
}

/**
 * @tc.name: FastScrollPreparedTargetKeepsPhysicalApproachDirection001
 * @tc.desc: Measuring a large preceding target moves both its END and the current anchor, not the scroll direction.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollPreparedTargetKeepsPhysicalApproachDirection001, TestSize.Level1)
{
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->contentMainSize_ = 600.0f;
    algorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm->posMap_->UpdatePos(80, { 8000.0f, 100.0f });
    algorithm->posMap_->UpdatePos(156, { 15600.0f, 60.0f });
    algorithm->itemPosition_[156] = { -1, 0.0f, 60.0f };
    algorithm->SetTotalOffset(15600.0f);
    algorithm->SetFastScrollTarget(80, ScrollAlign::END);
    algorithm->SetFastScrollPreparationOffset(6000.0f);
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[80] = { 5000.0f, false, false };
    EXPECT_EQ(algorithm->GetFastScrollPreparationRange(5000.0f), std::make_pair(5000.0f, 12200.0f));
    EXPECT_FLOAT_EQ(algorithm->posMap_->GetPositionInfo(156).mainPos, 15600.0f);
}

/**
 * @tc.name: FastScrollCompressionThresholdUsesFloatTolerance
 * @tc.desc: Four-screen distances within tolerance do not skip, while a larger excess does in either direction.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollSkipWindowTestNg, FastScrollCompressionThresholdUsesFloatTolerance, TestSize.Level1)
{
    auto model = CreateList();
    model.SetInitialIndex(GetParam() ? 80 : 0);
    CreateListItems(100);
    CreateDone();
    const float direction = GetParam() ? -1.0f : 1.0f;
    const float viewport = pattern_->contentMainSize_;
    const float origin = pattern_->GetTotalOffset();
    for (const auto& [excess, skip] : { std::pair { 0.0005f, false }, std::pair { 0.0025f, true } }) {
        pattern_->scrollTarget_.emplace();
        pattern_->scrollTarget_->fastScrollMode = FastScrollMode::SKIP_CREATE_AND_MEASURE;
        pattern_->StartFastScrollAnimation(origin + direction * (4.0f * viewport + excess));
        ASSERT_TRUE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(pattern_->GetFastScrollRequest()->skipPending, skip);
        if (skip) {
            EXPECT_TRUE(Positive(direction * pattern_->GetFastScrollRequest()->skipDistance));
        } else {
            EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->skipDistance, 0.0f);
        }
        pattern_->StopAnimate();
    }
}

/**
 * @tc.name: FastScrollZeroOffsetPreparesTwoApproachScreens001
 * @tc.desc: START preparation covers two screens on the actual approach side, without adding the target extent.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollZeroOffsetPreparesTwoApproachScreens001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    algorithm->contentMainSize_ = 600.0f;
    algorithm->SetFastScrollPreparationOffset(0.0f);
    algorithm->SetTotalOffset(0.0f);
    EXPECT_EQ(algorithm->GetFastScrollPreparationRange(100.0f), std::make_pair(-1200.0f, 0.0f));
    algorithm->SetTotalOffset(10000.0f);
    EXPECT_EQ(algorithm->GetFastScrollPreparationRange(100.0f), std::make_pair(0.0f, 1200.0f));
}

/**
 * @tc.name: FastScrollOffsetOutsideTarget
 * @tc.desc: An offset that excludes the target from the final viewport still finishes at its actual aligned position.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollOffsetTestNg, FastScrollOffsetOutsideTarget, TestSize.Level1)
{
    const auto [knownSize, extraOffset] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        auto sizes = model.GetOrCreateListChildrenMainSize();
        sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
        sizes->ChangeData(80, 1, { 5000.0f });
    }
    CreateListItems(80);
    CreateListItem();
    ViewAbstract::SetHeight(CalcLength(5000.0f));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(19);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 13000.0f - pattern_->contentMainSize_ + extraOffset, 0.1f);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->itemPosition_.count(80), 0);
}

INSTANTIATE_TEST_SUITE_P(SingleLane, ListFastScrollOffsetTestNg,
    testing::Combine(testing::Bool(), testing::Values(-6000.0f, 1000.0f)));

namespace {
constexpr int32_t OFFSET_ITEM_COUNT = 160;
constexpr int32_t OFFSET_TARGET_INDEX = 80;
constexpr float OFFSET_LARGE_ITEM = 5000.0f;
constexpr float OFFSET_SHORT_ITEM = 8.0f;
// Rebased cumulative coordinates below 32768px can differ by one float ULP (1/512px).
constexpr float OFFSET_POSITION_EPSILON = 0.002f;

enum class FastScrollItemSizes { SHORT_TARGET, LARGE_TARGET, LARGE_PREVIOUS, LARGE_BOTH };

struct FastScrollItemWork {
    std::vector<int32_t> measures = std::vector<int32_t>(OFFSET_ITEM_COUNT, 0);
    std::vector<int32_t> layouts = std::vector<int32_t>(OFFSET_ITEM_COUNT, 0);
};

class FastScrollItemProbeAlgorithm final : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(FastScrollItemProbeAlgorithm, BoxLayoutAlgorithm);
public:
    FastScrollItemProbeAlgorithm(const std::shared_ptr<FastScrollItemWork>& work, int32_t index)
        : work_(work), index_(index) {}

    void Measure(LayoutWrapper* wrapper) override
    {
        ++work_->measures[index_];
        BoxLayoutAlgorithm::Measure(wrapper);
    }

    void Layout(LayoutWrapper* wrapper) override
    {
        ++work_->layouts[index_];
        BoxLayoutAlgorithm::Layout(wrapper);
    }

private:
    std::shared_ptr<FastScrollItemWork> work_;
    int32_t index_;
};

class FastScrollItemProbePattern final : public ListItemPattern {
    DECLARE_ACE_TYPE(FastScrollItemProbePattern, ListItemPattern);
public:
    FastScrollItemProbePattern(const std::shared_ptr<FastScrollItemWork>& work, int32_t index)
        : ListItemPattern(nullptr), work_(work), index_(index) {}

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        // Ordinary items without swipe/edit controls use BoxLayoutAlgorithm in production too.
        return AceType::MakeRefPtr<FastScrollItemProbeAlgorithm>(work_, index_);
    }

private:
    std::shared_ptr<FastScrollItemWork> work_;
    int32_t index_;
};

} // namespace

class ListFastScrollItemOffsetFixture : public ListFastScrollerTestNg {
protected:
    void SetOffsetItemSizes(FastScrollItemSizes sizes)
    {
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            heights_.push_back(index % ALTERNATING_HEIGHT_PERIOD == 0 ? OFFSET_SHORT_HEIGHT : OFFSET_TALL_HEIGHT);
        }
        heights_[OFFSET_TARGET_INDEX] =
            sizes == FastScrollItemSizes::LARGE_TARGET || sizes == FastScrollItemSizes::LARGE_BOTH
                ? OFFSET_LARGE_ITEM : OFFSET_SHORT_ITEM;
        if (sizes == FastScrollItemSizes::LARGE_PREVIOUS || sizes == FastScrollItemSizes::LARGE_BOTH) {
            heights_[OFFSET_TARGET_INDEX - 1] = OFFSET_LARGE_ITEM;
        }
        float position = 0.0f;
        for (float height : heights_) {
            starts_.push_back(position);
            position += height + space_;
        }
        contentSize_ = position - space_;
    }

    void CreateOffsetItem(int32_t index)
    {
        auto item = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG, GetElmtId(),
            AceType::MakeRefPtr<FastScrollItemProbePattern>(work_, index));
        ViewStackProcessor::GetInstance()->Push(item);
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(heights_[index]));
        ViewStackProcessor::GetInstance()->Pop();
        items_.push_back(item);
    }

    void CreateOffsetList(bool knownSize, bool backward, bool spaced, FastScrollItemSizes sizes)
    {
        backward_ = backward;
        space_ = spaced ? 13.0f : 0.0f;
        startInset_ = spaced ? 23.0f : 0.0f;
        endInset_ = spaced ? 37.0f : 0.0f;
        SetOffsetItemSizes(sizes);
        auto model = CreateList();
        model.SetCachedCount(0);
        model.SetSpace(Dimension(space_));
        model.SetContentStartOffset(startInset_);
        model.SetContentEndOffset(endInset_);
        model.SetInitialIndex(backward ? OFFSET_ITEM_COUNT - 1 : 0);
        if (knownSize) {
            auto childrenSize = model.GetOrCreateListChildrenMainSize();
            childrenSize->UpdateDefaultSize(ITEM_MAIN_SIZE);
            childrenSize->ChangeData(0, OFFSET_ITEM_COUNT, heights_);
        }
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            CreateOffsetItem(index);
        }
        CreateDone();
        ASSERT_EQ(pattern_->itemPosition_.count(OFFSET_TARGET_INDEX), 0);
        ASSERT_EQ(work_->measures[OFFSET_TARGET_INDEX], 0);
        ASSERT_NO_FATAL_FAILURE(ExpectOffsetGeometry(
            backward ? contentSize_ - HEIGHT + endInset_ : -startInset_));
    }

    float GetOffsetOracle(ScrollAlign align, float extraOffset, float viewport = HEIGHT) const
    {
        const float start = starts_[OFFSET_TARGET_INDEX] - startInset_;
        const float end = starts_[OFFSET_TARGET_INDEX] + heights_[OFFSET_TARGET_INDEX] - viewport + endInset_;
        float aligned = start;
        if (align == ScrollAlign::CENTER) {
            aligned = starts_[OFFSET_TARGET_INDEX] + (heights_[OFFSET_TARGET_INDEX] - viewport) / 2.0f;
        } else if (align == ScrollAlign::END) {
            aligned = end;
        } else if (align == ScrollAlign::AUTO) {
            // The target starts wholly offscreen: AUTO chooses its nearer real edge.
            aligned = backward_ ? std::max(start, end) : std::min(start, end);
        }
        return std::clamp(aligned + extraOffset, -startInset_, contentSize_ - viewport + endInset_);
    }

    std::vector<int32_t> GetVisibleOffsetItems(float viewport = HEIGHT) const
    {
        std::vector<int32_t> visible;
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            const auto rect = items_[index]->GetGeometryNode()->GetFrameRect();
            if (items_[index]->IsActive() && rect.Height() > 0.0f && rect.Bottom() > 0.0f && rect.Top() < viewport) {
                visible.push_back(index);
            }
        }
        return visible;
    }

    void ExpectOffsetGeometry(float offset, float viewport = HEIGHT)
    {
        SCOPED_TRACE(testing::Message() << "true cumulative offset=" << offset);
        SCOPED_TRACE(testing::Message() << "observed offset=" << pattern_->GetTotalOffset()
            << " visible first=" << (pattern_->itemPosition_.empty() ? -1 : pattern_->itemPosition_.begin()->first)
            << " visible last=" << (pattern_->itemPosition_.empty() ? -1 : pattern_->itemPosition_.rbegin()->first)
            << " target pos=" << pattern_->posMap_->GetPositionInfo(OFFSET_TARGET_INDEX).mainPos);
        std::vector<int32_t> expected;
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            const float top = starts_[index] - offset;
            const float bottom = top + heights_[index];
            if (bottom <= 0.0f || top >= viewport) {
                continue;
            }
            SCOPED_TRACE(testing::Message() << "visible item=" << index);
            expected.push_back(index);
            ASSERT_EQ(pattern_->itemPosition_.count(index), 1);
            EXPECT_TRUE(items_[index]->IsActive());
            const auto rect = items_[index]->GetGeometryNode()->GetFrameRect();
            EXPECT_NEAR(rect.Top(), top, 0.1f);
            EXPECT_NEAR(rect.Bottom(), bottom, 0.1f);
            EXPECT_NEAR(rect.Height(), heights_[index], 0.1f);
        }
        ASSERT_FALSE(expected.empty());
        EXPECT_EQ(GetVisibleOffsetItems(viewport), expected);
        const float targetTop = starts_[OFFSET_TARGET_INDEX] - offset;
        if (targetTop >= viewport || targetTop + heights_[OFFSET_TARGET_INDEX] <= 0.0f) {
            EXPECT_EQ(pattern_->itemPosition_.count(OFFSET_TARGET_INDEX), 0);
            EXPECT_FALSE(items_[OFFSET_TARGET_INDEX]->IsActive());
        }
    }

    void StartOffsetWithoutDisplay(ScrollAlign align, std::optional<float> extraOffset)
    {
        const auto visible = GetVisibleOffsetItems();
        std::vector<RectF> rectangles;
        for (int32_t index : visible) {
            rectangles.push_back(items_[index]->GetGeometryNode()->GetFrameRect());
        }
        auto notifications = std::make_shared<int32_t>(0);
        eventHub_->SetOnScrollVisibleContentChange(
            [notifications](const ListItemIndex&, const ListItemIndex&) { ++*notifications; });
        const auto measures = work_->measures;
        const auto layouts = work_->layouts;
        MockAnimationManager::GetInstance().SetTicks(ANIMATION_TICKS);
        positionController_->ScrollToIndexWithOptions(OFFSET_TARGET_INDEX, true, align, extraOffset,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        if (NearZero(extraOffset.value_or(0.0f))) {
            ASSERT_TRUE(pattern_->GetFastScrollRequest());
            EXPECT_EQ(pattern_->AnimateRunning(), pattern_->childrenSize_ != nullptr);
            EXPECT_EQ(work_->measures, measures);
            EXPECT_EQ(work_->layouts, layouts);
        }
        FlushUITasks(); // No animation tick: offscreen preparation must not publish another viewport.
        EXPECT_EQ(GetVisibleOffsetItems(), visible);
        for (size_t index = 0; index < visible.size(); ++index) {
            SCOPED_TRACE(testing::Message() << "unchanged viewport item=" << visible[index]);
            const auto rect = items_[visible[index]]->GetGeometryNode()->GetFrameRect();
            EXPECT_EQ(rect.Left(), rectangles[index].Left());
            EXPECT_EQ(rect.GetSize(), rectangles[index].GetSize());
            EXPECT_NEAR(rect.Top(), rectangles[index].Top(), OFFSET_POSITION_EPSILON);
        }
        EXPECT_EQ(pattern_->itemPosition_.count(OFFSET_TARGET_INDEX), 0);
        EXPECT_FALSE(items_[OFFSET_TARGET_INDEX]->IsActive());
        EXPECT_EQ(*notifications, 0);
        eventHub_->SetOnScrollVisibleContentChange(nullptr);
    }

    std::shared_ptr<FastScrollItemWork> work_ = std::make_shared<FastScrollItemWork>();
    std::vector<RefPtr<FrameNode>> items_;
    std::vector<float> heights_;
    std::vector<float> starts_;
    float contentSize_ = 0.0f;
    float space_ = 0.0f;
    float startInset_ = 0.0f;
    float endInset_ = 0.0f;
    bool backward_ = false;
};

using FastScrollItemOffsetParameters = std::tuple<bool, bool, ScrollAlign, bool, FastScrollItemSizes, float>;

class ListFastScrollItemOffsetTestNg : public ListFastScrollItemOffsetFixture,
    public testing::WithParamInterface<FastScrollItemOffsetParameters> {};

/**
 * @tc.name: FastScrollExtraOffsetItemGeometry
 * @tc.desc: Unequal ordinary items reach the real final viewport, including an invisible target and boundary clamps.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollItemOffsetTestNg, FastScrollExtraOffsetItemGeometry, TestSize.Level1)
{
    const auto [knownSize, backward, align, spaced, sizes, extraOffset] = GetParam();
    ASSERT_NO_FATAL_FAILURE(CreateOffsetList(knownSize, backward, spaced, sizes));
    const float expectedOffset = GetOffsetOracle(align, extraOffset);
    const float initialOffset = pattern_->GetTotalOffset();
    const auto firstVisible = pattern_->itemPosition_.begin();
    const auto initialAnchor = pattern_->posMap_->GetPositionInfo(firstVisible->first);
    SCOPED_TRACE(testing::Message() << "initial offset=" << initialOffset
        << " first index=" << firstVisible->first << " first viewport pos=" << firstVisible->second.startPos
        << " first map pos=" << initialAnchor.mainPos << " first map size=" << initialAnchor.mainSize);
    ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(align, extraOffset));
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->scrollStop_);
    ExpectOffsetGeometry(expectedOffset);
    FlushUITasks();
    ExpectOffsetGeometry(expectedOffset);
}

INSTANTIATE_TEST_SUITE_P(OutsideTarget, ListFastScrollItemOffsetTestNg,
    testing::Combine(testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO), testing::Bool(),
        testing::Values(FastScrollItemSizes::SHORT_TARGET, FastScrollItemSizes::LARGE_TARGET,
            FastScrollItemSizes::LARGE_PREVIOUS, FastScrollItemSizes::LARGE_BOTH),
        testing::Values(-50000.0f, -6000.0f, 6000.0f, 50000.0f)));

INSTANTIATE_TEST_SUITE_P(SmallOffset, ListFastScrollItemOffsetTestNg,
    testing::Combine(testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO), testing::Bool(),
        testing::Values(FastScrollItemSizes::SHORT_TARGET, FastScrollItemSizes::LARGE_PREVIOUS),
        testing::Values(-16.0f, 16.0f)));

class ListFastScrollGapOffsetTestNg : public ListFastScrollItemOffsetFixture,
    public testing::WithParamInterface<std::tuple<bool, ScrollAlign, bool>> {
protected:
    void CreateGapList(bool knownSize)
    {
        space_ = HEIGHT - ITEM_MAIN_SIZE;
        heights_.assign(OFFSET_ITEM_COUNT, ITEM_MAIN_SIZE);
        auto model = CreateList();
        model.SetCachedCount(0);
        model.SetSpace(Dimension(space_));
        if (knownSize) {
            model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            starts_.push_back(index * (ITEM_MAIN_SIZE + space_));
            CreateOffsetItem(index);
        }
        contentSize_ = starts_.back() + ITEM_MAIN_SIZE;
        CreateDone();
    }

    void ExpectGapPreparation(const FastScrollItemWork& before, const std::vector<int32_t>& visible)
    {
        ASSERT_TRUE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
        EXPECT_FALSE(pattern_->targetIndex_);
        EXPECT_TRUE(pattern_->AnimateRunning());
        EXPECT_EQ(work_->measures[OFFSET_TARGET_INDEX] > before.measures[OFFSET_TARGET_INDEX],
            !pattern_->childrenSize_);
        EXPECT_EQ(work_->layouts[OFFSET_TARGET_INDEX], before.layouts[OFFSET_TARGET_INDEX]);
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            if (index == OFFSET_TARGET_INDEX || std::find(visible.begin(), visible.end(), index) != visible.end()) {
                continue;
            }
            // Two approach screens include two predecessors for START, one for END.
            const auto align = pattern_->GetFastScrollRequest()->align;
            const int32_t first = OFFSET_TARGET_INDEX - (align == ScrollAlign::START ? 2 : 1);
            const int32_t expectedExtra = !pattern_->childrenSize_ && index >= first &&
                index < OFFSET_TARGET_INDEX ? 1 : 0;
            EXPECT_EQ(work_->measures[index], before.measures[index] + expectedExtra) << "offscreen measure=" << index;
            EXPECT_EQ(work_->layouts[index], before.layouts[index]) << "offscreen layout=" << index;
        }
    }

    void ExpectGapDestination(float expectedOffset)
    {
        // Preparation may measure the approach-side neighbor, but it must not display that neighbor.
        ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_FALSE(pattern_->scrollStop_);
        EXPECT_NEAR(pattern_->GetTotalOffset(), expectedOffset, 0.1f);
        ASSERT_NO_FATAL_FAILURE(ExpectOffsetGeometry(expectedOffset));
        EXPECT_EQ(GetVisibleOffsetItems(), std::vector<int32_t> { OFFSET_TARGET_INDEX });
        FlushUITasks();
        ExpectOffsetGeometry(expectedOffset);
    }
};

/**
 * @tc.name: FastScrollExtraOffsetGapPreparesApproachNeighbor
 * @tc.desc: Gaps exclude the opposite neighbor from preparation; the approach neighbor remains offscreen.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGapOffsetTestNg, FastScrollExtraOffsetGapPreparesApproachNeighbor, TestSize.Level1)
{
    const auto [knownSize, align, atBoundary] = GetParam();
    ASSERT_NO_FATAL_FAILURE(CreateGapList(knownSize));
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    ASSERT_EQ(before.measures[OFFSET_TARGET_INDEX - 1], 0);
    ASSERT_EQ(before.measures[OFFSET_TARGET_INDEX], 0);
    ASSERT_EQ(before.measures[OFFSET_TARGET_INDEX + 1], 0);

    // For the 400px viewport and 300px gaps, neither neighboring item intersects the destination window.
    const float displacement = atBoundary ? space_ : ITEM_MAIN_SIZE / 2.0f;
    const float extraOffset = align == ScrollAlign::START ? -displacement : displacement;
    const float expectedOffset = GetOffsetOracle(align, extraOffset);
    ASSERT_LE(starts_[OFFSET_TARGET_INDEX - 1] + ITEM_MAIN_SIZE, expectedOffset);
    ASSERT_GE(starts_[OFFSET_TARGET_INDEX + 1], expectedOffset + HEIGHT);
    ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(align, extraOffset));
    RecordProperty("preparedTarget",
        testing::PrintToString(pattern_->posMap_->GetPositionInfo(OFFSET_TARGET_INDEX).mainPos));
    RecordProperty("preparedOffset", pattern_->GetTotalOffset());
    ASSERT_NO_FATAL_FAILURE(ExpectGapPreparation(before, visible));
    ASSERT_NO_FATAL_FAILURE(ExpectGapDestination(expectedOffset));
}

INSTANTIATE_TEST_SUITE_P(GapBoundary, ListFastScrollGapOffsetTestNg,
    testing::Combine(testing::Bool(), testing::Values(ScrollAlign::START, ScrollAlign::END), testing::Bool()));

class ListFastScrollZeroOffsetTestNg : public ListFastScrollItemOffsetFixture,
    public testing::WithParamInterface<std::tuple<bool, bool, ScrollAlign, bool, bool>> {};

/**
 * @tc.name: FastScrollZeroOffsetPreparesWithoutDisplaying
 * @tc.desc: Absent and zero offsets prepare the target neighborhood without laying out offscreen items.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollZeroOffsetTestNg, FastScrollZeroOffsetPreparesWithoutDisplaying, TestSize.Level1)
{
    const auto [knownSize, backward, align, spaced, explicitZero] = GetParam();
    ASSERT_NO_FATAL_FAILURE(CreateOffsetList(knownSize, backward, spaced, FastScrollItemSizes::LARGE_BOTH));
    const auto measures = work_->measures;
    const auto layouts = work_->layouts;
    const auto visible = GetVisibleOffsetItems();
    const std::optional<float> extraOffset = explicitZero ? std::optional<float>(0.0f) : std::nullopt;
    ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(align, extraOffset));
    for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
        if (std::find(visible.begin(), visible.end(), index) == visible.end()) {
            if (knownSize) {
                EXPECT_EQ(work_->measures[index], measures[index]) << "known-size measure=" << index;
            }
            EXPECT_EQ(work_->layouts[index], layouts[index]) << "offscreen layout=" << index;
        }
    }
    EXPECT_EQ(work_->measures[OFFSET_TARGET_INDEX] > 0, !knownSize);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(work_->measures[10], measures[10]);
    EXPECT_EQ(work_->measures[120], measures[120]);
    EXPECT_FALSE(pattern_->targetIndex_);
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_GT(work_->measures[OFFSET_TARGET_INDEX], 0);
}

INSTANTIATE_TEST_SUITE_P(TargetNeighborhood, ListFastScrollZeroOffsetTestNg,
    testing::Combine(testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO),
        testing::Bool(), testing::Bool()));

class ListFastScrollGeometryTestNg : public ListFastScrollItemOffsetFixture {
protected:
    void CreatePreparedOffset(ScrollAlign align = ScrollAlign::START, float extraOffset = 900.0f)
    {
        ASSERT_NO_FATAL_FAILURE(CreateOffsetList(false, false, false, FastScrollItemSizes::SHORT_TARGET));
        ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(align, extraOffset));
        ASSERT_TRUE(pattern_->GetFastScrollRequest());
        ASSERT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
        ASSERT_GT(work_->measures[OFFSET_TARGET_INDEX], 0);
        ASSERT_TRUE(pattern_->AnimateRunning());
    }

    void ChangeOffsetItemHeight(int32_t index, float height)
    {
        heights_[index] = height;
        float position = 0.0f;
        for (int32_t item = 0; item < OFFSET_ITEM_COUNT; ++item) {
            starts_[item] = position;
            position += heights_[item] + space_;
        }
        contentSize_ = position - space_;
        ViewAbstract::SetHeight(AceType::RawPtr(items_[index]), CalcLength(height));
        items_[index]->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }

    void FinishOffsetGeometry(
        ScrollAlign align = ScrollAlign::START, float extraOffset = 900.0f, float viewport = HEIGHT)
    {
        RunFastScrollFrames();
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->AnimateRunning());
        ASSERT_NO_FATAL_FAILURE(ExpectOffsetGeometry(GetOffsetOracle(align, extraOffset, viewport), viewport));
        FlushUITasks();
        ExpectOffsetGeometry(GetOffsetOracle(align, extraOffset, viewport), viewport);
    }

    void FinishAfterDataChange()
    {
        ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
        for (const auto& [index, position] : pattern_->itemPosition_) {
            EXPECT_GE(index, 0);
            EXPECT_LT(index, OFFSET_ITEM_COUNT);
            EXPECT_TRUE(std::isfinite(position.startPos));
            EXPECT_TRUE(std::isfinite(position.endPos));
            EXPECT_GE(position.endPos, position.startPos);
        }
        pattern_->ScrollToIndex(0, false, ScrollAlign::START);
        FlushUITasks();
        ExpectOffsetGeometry(0.0f);
    }

    RefPtr<ListLayoutAlgorithm> MakePreparedResult(float height)
    {
        auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
        const auto& target = *pattern_->GetFastScrollRequest();
        algorithm->SetFastScrollTarget(target.index, target.align);
        algorithm->SetFastScrollPreparationOffset(target.extraOffset);
        algorithm->GetOrCreateFastScrollState().result.measuredSizes[OFFSET_TARGET_INDEX] = { height, false, false };
        algorithm->GetOrCreateFastScrollState().result.status = FastScrollMeasureStatus::COMPLETE;
        return algorithm;
    }

    RefPtr<ListLayoutAlgorithm> MakeUnsupportedResult()
    {
        auto algorithm = MakePreparedResult(heights_[OFFSET_TARGET_INDEX]);
        algorithm->GetOrCreateFastScrollState().result.measuredSizes.clear();
        algorithm->GetOrCreateFastScrollState().result.status = FastScrollMeasureStatus::UNSUPPORTED;
        return algorithm;
    }

    void ExpectNoOffscreenWork(const FastScrollItemWork& before, const std::vector<int32_t>& visible)
    {
        for (int32_t index = 0; index < OFFSET_ITEM_COUNT; ++index) {
            if (std::find(visible.begin(), visible.end(), index) != visible.end()) {
                continue;
            }
            EXPECT_EQ(work_->measures[index], before.measures[index]) << "offscreen measure=" << index;
            EXPECT_EQ(work_->layouts[index], before.layouts[index]) << "offscreen layout=" << index;
        }
    }
};

/**
 * @tc.name: FastScrollExtraOffsetNotifyDataChange001
 * @tc.desc: Data notification does not reprepare the whole window; the request terminates and scrolling recovers.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetNotifyDataChange001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    ASSERT_FALSE(pattern_->maintainVisibleContentPosition_);
    const auto before = *work_;
    ChangeOffsetItemHeight(OFFSET_TARGET_INDEX, 700.0f);
    pattern_->NotifyDataChange(OFFSET_TARGET_INDEX, 0);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(work_->measures, before.measures);
    FinishAfterDataChange();
}

/**
 * @tc.name: FastScrollExtraOffsetChildrenMainSize001
 * @tc.desc: Switching to supplied child sizes keeps preparation and updates positions through the known-size path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetChildrenMainSize001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto before = *work_;
    ChangeOffsetItemHeight(OFFSET_TARGET_INDEX + 1, 700.0f);
    auto childrenSize = pattern_->GetOrCreateListChildrenMainSize();
    childrenSize->ChangeData(0, OFFSET_ITEM_COUNT, heights_);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(work_->measures, before.measures);
    FinishAfterDataChange();
}

/**
 * @tc.name: FastScrollExtraOffsetKnownChildrenMainSize001
 * @tc.desc: Updating a supplied size table does not synchronously remeasure the prepared window.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetKnownChildrenMainSize001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateOffsetList(true, false, false, FastScrollItemSizes::SHORT_TARGET));
    ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(ScrollAlign::START, 900.0f));
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    const auto before = *work_;
    ChangeOffsetItemHeight(OFFSET_TARGET_INDEX + 1, 700.0f);
    pattern_->childrenSize_->ChangeData(OFFSET_TARGET_INDEX + 1, 1, { heights_[OFFSET_TARGET_INDEX + 1] });
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(work_->measures, before.measures);
    FinishAfterDataChange();
}

/**
 * @tc.name: FastScrollExtraOffsetItemResize001
 * @tc.desc: One offscreen size change does not remeasure the prepared range and normal layout can finish safely.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetItemResize001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    ASSERT_GT(work_->measures[OFFSET_TARGET_INDEX + 1], 0);
    ChangeOffsetItemHeight(OFFSET_TARGET_INDEX + 1, 700.0f);
    MockPipelineContext::GetCurrent()->FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    ExpectNoOffscreenWork(before, visible);
    FinishAfterDataChange();
}

/**
 * @tc.name: FastScrollExtraOffsetViewportResize001
 * @tc.desc: Resizing the viewport changes END alignment and the actual visible items after preparation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetViewportResize001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset(ScrollAlign::END, -900.0f));
    constexpr float viewport = HEIGHT / 2.0f;
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(viewport));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_NEAR(pattern_->contentMainSize_, viewport, 0.1f);
    FinishOffsetGeometry(ScrollAlign::END, -900.0f, viewport);
}

/**
 * @tc.name: FastScrollPosMapReestimateKeepsPreparation001
 * @tc.desc: Reestimating the outer origin does not remeasure the prepared offset range.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollPosMapReestimateKeepsPreparation001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    pattern_->needReEstimateOffset_ = true;
    pattern_->MarkDirtyNodeSelf();
    MockPipelineContext::GetCurrent()->FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    ExpectNoOffscreenWork(before, visible);
    FinishAfterDataChange();
}

/**
 * @tc.name: FastScrollExtraOffsetCanceledResultIgnored001
 * @tc.desc: Results delivered after cancellation cannot update the position table or revive the request.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetCanceledResultIgnored001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    auto stale = MakePreparedResult(987.0f);
    pattern_->StopAnimate();
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_NE(pattern_->posMap_, nullptr);
    const float size = pattern_->posMap_->GetPositionInfo(OFFSET_TARGET_INDEX).mainSize;
    pattern_->ApplyFastScrollLayoutResult(stale);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(OFFSET_TARGET_INDEX).mainSize, size);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    ExpectOffsetGeometry(0.0f);
}

class ListFastScrollPreparationTargetTestNg : public ListFastScrollGeometryTestNg,
    public testing::WithParamInterface<int32_t> {};

/**
 * @tc.name: FastScrollPreparationMatchesTargetParameters
 * @tc.desc: Only a matching index, alignment and offset can publish preparation completion for the current target.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollPreparationTargetTestNg, FastScrollPreparationMatchesTargetParameters, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    pattern_->GetOrCreateFastScrollPreparation().preparation = FastScrollPreparation::NEED_WINDOW;
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    auto result = MakePreparedResult(987.0f);
    const auto target = *pattern_->GetFastScrollRequest();
    const float oldSize = pattern_->posMap_->GetPositionInfo(target.index).mainSize;
    const auto mode = GetParam();
    if (mode == 0) {
        result->SetFastScrollTarget(target.index + 1, target.align);
    } else if (mode == 1) {
        result->SetFastScrollTarget(target.index, ScrollAlign::END);
    } else {
        result->SetFastScrollPreparationOffset(target.extraOffset + 1.0f);
    }
    pattern_->ApplyFastScrollLayoutResult(result);
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(target.index).mainSize, oldSize);
    pattern_->ApplyFastScrollLayoutResult(MakePreparedResult(heights_[target.index]));
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    FinishOffsetGeometry();
}

INSTANTIATE_TEST_SUITE_P(PreparationTarget, ListFastScrollPreparationTargetTestNg, testing::Values(0, 1, 2));

class ListFastScrollLayoutChangeTestNg : public ListFastScrollGeometryTestNg,
    public testing::WithParamInterface<int32_t> {
protected:
    void ChangeLayoutInput(int32_t mode)
    {
        auto property = frameNode_->GetLayoutProperty<ListLayoutProperty>();
        switch (mode) {
            case 1:
                ViewAbstract::SetWidth(AceType::RawPtr(frameNode_), CalcLength(WIDTH / 2.0f));
                break;
            case CHANGE_STACK_FROM_END:
                property->UpdateStackFromEnd(!pattern_->isStackFromEnd_);
                break;
            case CHANGE_CONTENT_START_OFFSET:
                property->UpdateContentStartOffset(pattern_->contentStartOffset_ + 1.0f);
                break;
            case CHANGE_CONTENT_END_OFFSET:
                property->UpdateContentEndOffset(pattern_->contentEndOffset_ + 1.0f);
                break;
            case CHANGE_ITEM_SPACE:
                property->UpdateSpace(Dimension(space_ + 1.0f));
                break;
            case CHANGE_LIST_HEIGHT:
                ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(HEIGHT / 2.0f));
                break;
            case CHANGE_ESTIMATED_SPACE:
                pattern_->GetOrCreateFastScrollPreparation().estimatedSpace += 1.0f;
                break;
            default:
                break;
        }
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
};

/**
 * @tc.name: FastScrollLayoutChangeDoesNotReprepareOffsetRange
 * @tc.desc: Normal layout changes do not automatically remeasure the prepared offscreen target.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLayoutChangeTestNg, FastScrollLayoutChangeDoesNotReprepareOffsetRange, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto targetMeasures = work_->measures[OFFSET_TARGET_INDEX];
    ChangeLayoutInput(GetParam());
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_EQ(work_->measures[OFFSET_TARGET_INDEX], targetMeasures);
    EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
    pattern_->StopAnimate();
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
}

INSTANTIATE_TEST_SUITE_P(LayoutChange, ListFastScrollLayoutChangeTestNg, testing::Range(0, 8));

/**
 * @tc.name: FastScrollExtraOffsetUnsupportedUsesLegacy001
 * @tc.desc: An unsupported result for the current target transfers the unchanged request to legacy scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetUnsupportedUsesLegacy001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    auto unsupported = MakeUnsupportedResult();
    ASSERT_TRUE(unsupported->GetFastScrollMeasureResult()->measuredSizes.empty());
    pattern_->ApplyFastScrollLayoutResult(unsupported);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::UNSUPPORTED);
    EXPECT_TRUE(pattern_->FallbackFastScroll());
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_TRUE(pattern_->targetIndex_);
    EXPECT_EQ(pattern_->targetIndex_.value(), OFFSET_TARGET_INDEX);
    EXPECT_EQ(pattern_->scrollAlign_, ScrollAlign::START);
    ASSERT_TRUE(pattern_->GetExtraOffset());
    EXPECT_FLOAT_EQ(pattern_->GetExtraOffset().value(), 900.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->AnimateRunning());
    FinishOffsetGeometry();
}

/**
 * @tc.name: FastScrollExtraOffsetRejectsStaleUnsupported001
 * @tc.desc: Unsupported results with different target parameters cannot trigger fallback.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetRejectsStaleUnsupported001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    auto stale = MakeUnsupportedResult();
    constexpr float replacementOffset = 16.0f;
    pattern_->ScrollToIndexFastScroll(OFFSET_TARGET_INDEX, true, ScrollAlign::START, replacementOffset,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    pattern_->ApplyFastScrollLayoutResult(stale);
    EXPECT_FALSE(pattern_->FallbackFastScroll());
    pattern_->TryStartFastScroll();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->extraOffset, replacementOffset);
    EXPECT_NE(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::UNSUPPORTED);
    EXPECT_FALSE(pattern_->targetIndex_);
    FinishOffsetGeometry(ScrollAlign::START, replacementOffset);
}

/**
 * @tc.name: FastScrollCallbackModifiedAnimationDoesNotRestart001
 * @tc.desc: A callback-modified request awaiting its last layout stops without replaying the remaining distance.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollCallbackModifiedAnimationDoesNotRestart001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    auto displacement = std::make_shared<float>(0.0f);
    eventHub_->SetOnWillScroll([displacement](Dimension offset, ScrollState, ScrollSource) {
        auto applied = Dimension(offset.Value() * 0.5f, offset.Unit());
        *displacement += applied.ConvertToPx();
        return ScrollFrameResult { applied };
    });
    constexpr int32_t maxFrames = 24;
    for (int32_t frame = 0; frame < maxFrames; ++frame) {
        MockAnimationManager::GetInstance().Tick();
        if (pattern_->AnimateStoped()) {
            break;
        }
        FlushUITasks();
    }
    // A late layout result cannot revive an already completed animation.
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    ASSERT_TRUE(pattern_->AnimateStoped());
    const float appliedOffset = *displacement;
    ASSERT_GT(appliedOffset, 0.0f);
    pattern_->TryStartFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->targetIndex_);
    EXPECT_FALSE(pattern_->AnimateRunning());
    FlushUITasks();
    const auto visible = GetVisibleOffsetItems();
    ASSERT_FALSE(visible.empty());
    // The skipped prefix is estimated, so callback displacement is not a true cumulative item coordinate.
    const float visibleOffset = starts_[visible.front()] -
        items_[visible.front()]->GetGeometryNode()->GetFrameRect().Top();
    ASSERT_NO_FATAL_FAILURE(ExpectOffsetGeometry(visibleOffset));
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_NEAR(*displacement, appliedOffset, 0.1f);
    ExpectOffsetGeometry(visibleOffset);
}

/**
 * @tc.name: FastScrollExtraOffsetCancelBeforePrepare001
 * @tc.desc: Cancelling before preparation prevents offscreen work and stale results cannot revive it.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetCancelBeforePrepare001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateOffsetList(false, false, false, FastScrollItemSizes::SHORT_TARGET));
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    pattern_->ScrollToIndexFastScroll(OFFSET_TARGET_INDEX, true, ScrollAlign::START, 900.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(work_->measures, before.measures);
    auto stale = MakePreparedResult(heights_[OFFSET_TARGET_INDEX]);
    pattern_->StopAnimate();
    pattern_->ApplyFastScrollLayoutResult(stale);
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    ExpectNoOffscreenWork(before, visible);
    ExpectOffsetGeometry(0.0f);
}

/**
 * @tc.name: FastScrollZeroOffsetLayoutKeepsPreparation001
 * @tc.desc: A subsequent layout keeps the zero-offset preparation without remeasuring the offscreen target.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollZeroOffsetLayoutKeepsPreparation001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateOffsetList(false, false, false, FastScrollItemSizes::SHORT_TARGET));
    ASSERT_NO_FATAL_FAILURE(StartOffsetWithoutDisplay(ScrollAlign::START, 0.0f));
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparedLayouts, nullptr);
    EXPECT_GT(work_->measures[OFFSET_TARGET_INDEX], 0);
    EXPECT_EQ(work_->layouts[OFFSET_TARGET_INDEX], 0);
    ExpectNoOffscreenWork(before, visible);
    FinishOffsetGeometry(ScrollAlign::START, 0.0f);
}

/**
 * @tc.name: FastScrollExtraOffsetSelfDirtyReusesPreparation001
 * @tc.desc: Animation-style MEASURE_SELF without a geometry change keeps preparation and does no offscreen work.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollExtraOffsetSelfDirtyReusesPreparation001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto before = *work_;
    const auto visible = GetVisibleOffsetItems();
    for (int32_t frame = 0; frame < 3; ++frame) {
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        MockPipelineContext::GetCurrent()->FlushUITasks();
        ASSERT_TRUE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
        ExpectNoOffscreenWork(before, visible);
    }
    FinishOffsetGeometry();
}

/**
 * @tc.name: FastScrollAnimationEndKeepsPreparedWindow001
 * @tc.desc: Ending an animation segment does not measure the same offscreen target again.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGeometryTestNg, FastScrollAnimationEndKeepsPreparedWindow001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreatePreparedOffset());
    const auto measures = work_->measures[OFFSET_TARGET_INDEX];
    ASSERT_GT(measures, 0);
    pattern_->StopAnimation(pattern_->springAnimation_);
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    FlushUITasks();
    EXPECT_EQ(pattern_->itemPosition_.count(OFFSET_TARGET_INDEX), 0);
    EXPECT_EQ(work_->measures[OFFSET_TARGET_INDEX], measures);
}

/**
 * @tc.name: FastScrollEstimateStartsWithoutTarget001
 * @tc.desc: Estimate an unmeasured target without requesting layout of the complete path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollEstimateStartsWithoutTarget001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    const auto visibleCount = pattern_->itemPosition_.size();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    FlushUITasks();
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->targetIndex_.has_value());
    EXPECT_EQ(pattern_->itemPosition_.size(), visibleCount);
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 80 * ITEM_MAIN_SIZE);
    EXPECT_EQ(pattern_->itemPosition_.count(80), 0);
}

/**
 * @tc.name: FastScrollRetargetsTwice001
 * @tc.desc: Two geometry corrections retain the target and use normal animation interruption semantics.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollRetargetsTwice001, TestSize.Level1)
{
    auto model = CreateList();
    int32_t starts = 0;
    int32_t stops = 0;
    model.SetOnScrollStart([&starts]() { ++starts; });
    model.SetOnScrollStop([&stops]() { ++stops; });
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    for (const float correction : { 120.0f, 240.0f }) {
        auto info = pattern_->posMap_->GetPositionInfo(80);
        info.mainPos = 80 * ITEM_MAIN_SIZE + correction;
        pattern_->posMap_->UpdatePos(80, info);
        pattern_->UpdateFastScrollAnimation();
        ASSERT_TRUE(pattern_->HasFastScrollRequest());
        EXPECT_TRUE(pattern_->AnimateRunning());
        EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), info.mainPos);
        EXPECT_TRUE(pattern_->GetScrollAbort());
        pattern_->OnScrollStop([&stops]() { ++stops; }, nullptr);
    }
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 0);
    pattern_->StopAnimate();
    FlushUITasks();
    EXPECT_EQ(stops, 1);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollEndpointStopsBeforeLayout001
 * @tc.desc: Reaching a segment endpoint performs normal stop handling before the final layout check.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollEndpointStopsBeforeLayout001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    pattern_->ProcessSpringOffsetCallback(pattern_->GetFinalPosition());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_TRUE(pattern_->scrollStop_);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    FlushUITasks();
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 80 * ITEM_MAIN_SIZE);
}

/**
 * @tc.name: FastScrollStartCallbackOrdinaryAnimation001
 * @tc.desc: The internal handoff guard must not leak into a public animation started by the start callback.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollStartCallbackOrdinaryAnimation001, TestSize.Level1)
{
    auto model = CreateList();
    bool replaced = false;
    model.SetOnScrollStart([this, &replaced]() {
        if (!replaced) {
            replaced = true;
            pattern_->AnimateTo(250.0f, -1, nullptr, true);
        }
    });
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    EXPECT_TRUE(replaced);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 250.0f);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 250.0f, 0.1f);
}

/**
 * @tc.name: FastScrollEndpointControllerTakeover001
 * @tc.desc: A legacy index request before the endpoint layout discards the old delta and reaches its own target.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollEndpointControllerTakeover001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, 25.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning());
    ASSERT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    pattern_->ProcessSpringOffsetCallback(pattern_->GetFinalPosition());
    ASSERT_FALSE(pattern_->HasFastScrollRequest());
    ASSERT_FALSE(pattern_->AnimateRunning());
    ASSERT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);

    constexpr int32_t replacementIndex = 20;
    constexpr float replacementOffset = 35.0f;
    positionController_->ScrollToIndexWithOptions(replacementIndex, true, ScrollAlign::START, replacementOffset,
        FastScrollMode::NONE);
    ASSERT_TRUE(pattern_->targetIndex_);
    EXPECT_EQ(pattern_->targetIndex_.value(), replacementIndex);
    EXPECT_EQ(pattern_->scrollAlign_, ScrollAlign::START);
    EXPECT_FLOAT_EQ(pattern_->GetExtraOffset().value_or(0.0f), replacementOffset);
    EXPECT_FLOAT_EQ(pattern_->currentDelta_, 0.0f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    const bool skipBeforeLayout = pattern_->fastScrollSkip_;
    FlushUITasks();
    ASSERT_TRUE(pattern_->AnimateRunning()) << "skipBeforeLayout=" << skipBeforeLayout
        << ", targetMeasured=" << pattern_->itemPosition_.count(replacementIndex)
        << ", targetPending=" << pattern_->targetIndex_.has_value()
        << ", offset=" << pattern_->GetTotalOffset();
    const float replacementPosition = replacementIndex * ITEM_MAIN_SIZE + replacementOffset;
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), replacementPosition);
    EXPECT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), replacementPosition, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(replacementIndex).Top(), -replacementOffset, 0.1f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetTotalOffset(), replacementPosition, 0.1f);
}

/**
 * @tc.name: FastScrollDirectStopClearsPendingRequest001
 * @tc.desc: An animator stop notification clears a waiting request without requiring a spring stop callback.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollDirectStopClearsPendingRequest001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    pattern_->OnAnimateStop();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    CreateDone();
    EXPECT_FALSE(pattern_->AnimateRunning());
}

/**
 * @tc.name: FastScrollPendingCancellation001
 * @tc.desc: Stop a request before the first layout, when there is no animation handle yet.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollPendingCancellation001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::CENTER, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    pattern_->StopAnimate();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    CreateDone();
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
}

/**
 * @tc.name: FastScrollPendingLatestRequest001
 * @tc.desc: The last valid request starts only after the first viewport has been measured.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollPendingLatestRequest001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    pattern_->ScrollToIndexFastScroll(50, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->AnimateRunning());
    CreateDone();
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, 50);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 50 * ITEM_MAIN_SIZE);
    EXPECT_FALSE(pattern_->targetIndex_.has_value());
}

/**
 * @tc.name: FastScrollFirstLayoutStartsBeforeEvents001
 * @tc.desc: A pending END request starts with current geometry and indexes before first-layout events.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollFirstLayoutStartsBeforeEvents001, TestSize.Level1)
{
    CreateList();
    auto events = std::make_shared<std::vector<std::string>>();
    eventHub_->SetOnScrollStart([this, events]() {
        events->emplace_back("start");
        EXPECT_FALSE(pattern_->isInitialized_);
        EXPECT_FLOAT_EQ(pattern_->contentMainSize_, HEIGHT);
        EXPECT_EQ(pattern_->startIndex_, 0);
        EXPECT_EQ(pattern_->endIndex_, 3);
    });
    eventHub_->SetOnReachStart([events]() { events->emplace_back("reach"); });
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->AnimateRunning());
    CreateDone();
    ASSERT_GE(events->size(), 2);
    EXPECT_EQ(events->at(0), "start");
    EXPECT_EQ(events->at(1), "reach");
    EXPECT_TRUE(pattern_->isInitialized_);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 81 * ITEM_MAIN_SIZE - HEIGHT);
    eventHub_->SetOnScrollStart(nullptr);
    eventHub_->SetOnReachStart(nullptr);
}

/**
 * @tc.name: FastScrollWillScrollInterception001
 * @tc.desc: A zero displacement from the application does not cancel an in-flight animation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollWillScrollInterception001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    eventHub_->SetOnWillScroll([](Dimension, ScrollState, ScrollSource) {
        ScrollFrameResult result;
        result.offset = Dimension(0.0f);
        return result;
    });
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    pattern_->UpdateCurrentOffset(-100.0f, SCROLL_FROM_ANIMATION_CONTROLLER);
    FlushUITasks();
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    RunFastScrollFrames();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
}

/**
 * @tc.name: FastScrollCallbackDoesNotRestart
 * @tc.desc: Apply callback displacement without replacing the running segment or retrying at its end.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollCallbackTestNg, FastScrollCallbackDoesNotRestart, TestSize.Level1)
{
    const auto [knownSize, backward, mode] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    const float initialOffset = backward ? 80 * ITEM_MAIN_SIZE : 0.0f;
    ScrollTo(initialOffset);
    auto displacement = std::make_shared<float>(0.0f);
    auto calls = std::make_shared<int32_t>(0);
    auto stops = std::make_shared<int32_t>(0);
    eventHub_->SetOnWillScroll([mode = mode, displacement, calls](Dimension offset, ScrollState, ScrollSource) {
        if (!NearZero(offset.Value())) {
            ++*calls;
        }
        float ratio = mode == FastScrollCallbackMode::ZERO ? 0.0f : 1.0f;
        if (mode == FastScrollCallbackMode::HALF || (mode == FastScrollCallbackMode::HALF_ONCE && *calls == 1)) {
            ratio = 0.5f;
        }
        auto applied = Dimension(offset.Value() * ratio, offset.Unit());
        *displacement += applied.ConvertToPx();
        return ScrollFrameResult { applied };
    });
    eventHub_->SetOnScrollStop([stops]() { ++*stops; });
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(backward ? 20 : 80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_NEAR(pattern_->GetTotalOffset(), initialOffset + *displacement, 0.1f);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), initialOffset + *displacement, 0.1f);
    EXPECT_EQ(*stops, 1);
    eventHub_->SetOnWillScroll(nullptr);
    eventHub_->SetOnScrollStop(nullptr);
    pattern_->ScrollToIndexFastScroll(backward ? 10 : 90, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), (backward ? 10 : 90) * ITEM_MAIN_SIZE, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(SingleLane, ListFastScrollCallbackTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Values(FastScrollCallbackMode::ZERO,
        FastScrollCallbackMode::HALF, FastScrollCallbackMode::HALF_ONCE, FastScrollCallbackMode::UNCHANGED)));

/**
 * @tc.name: FastScrollMultiLaneEnabled001
 * @tc.desc: Multi-lane preparation starts an optimized animation and preserves the target and extra offset.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollMultiLaneEnabled001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, 25.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    FlushUITasks();
    EXPECT_TRUE(pattern_->AnimateRunning());
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * ITEM_MAIN_SIZE + 25.0f, 0.1f);
}

/**
 * @tc.name: FastScrollMultiLaneBeforeLayout001
 * @tc.desc: A multi-lane request before first layout waits for visible row samples.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollMultiLaneBeforeLayout001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    CreateDone();
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * ITEM_MAIN_SIZE, 0.1f);
}

/**
 * @tc.name: FastScrollAutoLanesBeforeLayout001
 * @tc.desc: A pending request uses the measured lane count, not the default value in the pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollAutoLanesBeforeLayout001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLaneMinLength(Dimension(WIDTH / 2));
    model.SetLaneMaxLength(Dimension(WIDTH / 2));
    CreateListItems(100);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    EXPECT_FALSE(pattern_->AnimateRunning());
    CreateDone();
    FlushUITasks();
    EXPECT_EQ(pattern_->GetLanes(), 2);
    RunFastScrollFrames();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 40 * ITEM_MAIN_SIZE, 0.1f);
}

/**
 * @tc.name: FastScrollLaneChangeKeepsScrollingUsable001
 * @tc.desc: Changing lanes during fast scroll can lose alignment but must leave valid layout and scrolling state.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollLaneChangeKeepsScrollingUsable001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    pattern_->currentDelta_ = 5000.0f;
    layoutProperty_->UpdateLanes(2);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetLanes(), 2);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
    ASSERT_FALSE(pattern_->itemPosition_.empty());
    for (const auto& [index, position] : pattern_->itemPosition_) {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, 100);
        EXPECT_TRUE(std::isfinite(position.startPos));
        EXPECT_TRUE(std::isfinite(position.endPos));
        EXPECT_TRUE(GreatOrEqual(position.endPos, position.startPos));
    }
    pattern_->ScrollToIndex(10, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetItemRect(10).Top(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(11).Top(), 0.0f, 0.1f);
    const float offsetBeforeScroll = pattern_->GetTotalOffset();
    pattern_->ScrollBy(25.0f);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetTotalOffset(), offsetBeforeScroll + 25.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(10).Top(), -25.0f, 0.1f);
    pattern_->ScrollToIndex(0, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetTotalOffset(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(0).Top(), 0.0f, 0.1f);
}

class ListFastScrollMultiLaneTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool, FastScrollMode, ScrollAlign>> {
protected:
    static float UnevenItemHeight(int32_t index)
    {
        const int32_t row = index / 3;
        const float base = row < 6 ? 45.0f : (row < 21 ? 400.0f : (row < 32 ? 75.0f : 180.0f));
        return base + index % UNEVEN_LANE_COUNT * LANE_SIZE_STEP;
    }

    void CreateUnevenMultiLaneList(bool known, bool stack, bool backward)
    {
        auto model = CreateList();
        model.SetLanes(UNEVEN_LANE_COUNT);
        model.SetSpace(Dimension(SPACE));
        model.SetStackFromEnd(stack);
        model.SetInitialIndex(backward ? UNEVEN_INITIAL_INDEX : 0);
        std::vector<float> heights;
        for (int32_t index = 0; index < UNEVEN_ITEM_COUNT; ++index) {
            CreateListItem();
            heights.emplace_back(UnevenItemHeight(index));
            ViewAbstract::SetHeight(CalcLength(heights.back()));
            ViewStackProcessor::GetInstance()->Pop();
            ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        }
        if (known) {
            model.GetOrCreateListChildrenMainSize()->ChangeData(0, heights.size(), heights);
        }
        CreateDone();
    }

    void ExpectUnevenLegacyControl(int32_t target, ScrollAlign align, float extra, const std::vector<float>& actual)
    {
        ListScrollerTestNg::TearDown();
        ListScrollerTestNg::SetUp();
        CreateUnevenMultiLaneList(true, true, true);
        pattern_->ScrollToIndex(target, false, align, extra);
        FlushUITasks();
        std::ostringstream trace;
        trace << pattern_->GetItemRect(target).Top();
        EXPECT_NEAR(pattern_->GetItemRect(target).Top(), actual.front(), 0.1f);
        size_t step = 1;
        for (float delta : { 23.0f, -46.0f, 23.0f }) {
            pattern_->ScrollBy(delta);
            FlushUITasks();
            const auto top = pattern_->GetItemRect(target).Top();
            trace << "," << top;
            EXPECT_NEAR(top, actual.at(step++), 0.1f);
        }
        RecordProperty("legacyControlTargetTops", trace.str());
    }
};

class ListFastScrollUnevenRowTestNg : public ListFastScrollMultiLaneTestNg {
public:
    static std::vector<ParamType> GetRegressionParameters()
    {
        std::vector<ParamType> parameters;
        for (bool known : { false, true }) {
            for (bool stack : { false, true }) {
                for (bool backward : { false, true }) {
                    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
                        for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END }) {
                            // These ordinary-scroll failures are tracked outside this feature.
                            if (known && stack && backward && align != ScrollAlign::END) {
                                continue;
                            }
                            parameters.emplace_back(known, stack, backward, mode, align);
                        }
                    }
                }
            }
        }
        return parameters;
    }
};

/**
 * @tc.name: FastScrollMultiLaneUnevenRowContinuesScrolling
 * @tc.desc: Piecewise uneven rows retain their maximum height, target alignment and normal scrolling after fast scroll.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollUnevenRowTestNg, FastScrollMultiLaneUnevenRowContinuesScrolling, TestSize.Level1)
{
    const auto [known, stack, backward, mode, align] = GetParam();
    CreateUnevenMultiLaneList(known, stack, backward);
    const int32_t target = backward ? 25 : 70;
    const float extra = backward ? -35.0f : 35.0f;
    const float rowHeight = UnevenItemHeight(target / 3 * 3 + 2);
    float top = -extra;
    if (align == ScrollAlign::END) {
        top += HEIGHT - rowHeight;
    } else if (align == ScrollAlign::CENTER) {
        top += (HEIGHT - rowHeight) / 2.0f;
    }
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(target, true, align, extra, mode);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(64));
    EXPECT_NEAR(pattern_->GetItemRect(target).Top(), top, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(target).Height(), UnevenItemHeight(target), 0.1f);
    const int32_t rowStart = target / 3 * 3;
    for (int32_t index = rowStart; index < rowStart + 3; ++index) {
        EXPECT_NEAR(pattern_->GetItemRect(index).Top(), top, 0.1f);
        EXPECT_NEAR(pattern_->GetItemRect(index).Height(), UnevenItemHeight(index), 0.1f);
    }
    std::vector<float> actual = { pattern_->GetItemRect(target).Top() };
    for (float delta : { 23.0f, -46.0f, 23.0f }) {
        pattern_->ScrollBy(delta);
        FlushUITasks();
        top -= delta;
        EXPECT_NEAR(pattern_->GetItemRect(target).Top(), top, 0.1f);
        actual.emplace_back(pattern_->GetItemRect(target).Top());
        EXPECT_FALSE(pattern_->HasFastScrollRequest());
    }
    if (known && stack && backward) {
        ExpectUnevenLegacyControl(target, align, extra, actual);
    }
}

/**
 * @tc.name: FastScrollMultiLaneAlignment
 * @tc.desc: Whole-row preparation and skipping preserve alignment in both directions, with known sizes and reversal.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneTestNg, FastScrollMultiLaneAlignment, TestSize.Level1)
{
    const auto [known, stack, backward, mode, align] = GetParam();
    auto model = CreateList();
    model.SetLanes(3);
    model.SetStackFromEnd(stack);
    model.SetInitialIndex(backward ? 90 : 0);
    if (known) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(120);
    CreateDone();
    const int32_t target = backward ? 25 : 70;
    const float extra = backward ? -175.0f : 35.0f;
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(target, true, align, extra, mode);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    RunFastScrollFrames();
    float expected = target / 3 * ITEM_MAIN_SIZE + extra;
    if (align == ScrollAlign::END) {
        expected -= HEIGHT - ITEM_MAIN_SIZE;
    } else if (align == ScrollAlign::CENTER) {
        expected -= (HEIGHT - ITEM_MAIN_SIZE) / 2;
    }
    EXPECT_NEAR(pattern_->GetTotalOffset(), expected, 0.1f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
}

INSTANTIATE_TEST_SUITE_P(KnownAndEstimated, ListFastScrollMultiLaneTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool(),
        testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END)));

INSTANTIATE_TEST_SUITE_P(KnownAndEstimated, ListFastScrollUnevenRowTestNg,
    testing::ValuesIn(ListFastScrollUnevenRowTestNg::GetRegressionParameters()),
    ([](const testing::TestParamInfo<ListFastScrollUnevenRowTestNg::ParamType>& info) {
        const auto [known, stack, backward, mode, align] = info.param;
        const int32_t modeIndex = mode == FastScrollMode::SKIP_MEASURE ? 0 : 1;
        const int32_t alignIndex = align == ScrollAlign::START ? 0 : (align == ScrollAlign::CENTER ? 1 : 2);
        const int32_t originalIndex = (((known * 2 + stack) * 2 + backward) * 2 + modeIndex) * 3 + alignIndex;
        return std::to_string(originalIndex);
    }));

class ListFastScrollMultiLaneGroupTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool, ScrollAlign>> {};

/**
 * @tc.name: FastScrollMultiLaneGroupTarget
 * @tc.desc: A group interrupts a partial outer row and its internal multi-lane geometry survives preparation and skip.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneGroupTestNg, FastScrollMultiLaneGroupTarget, TestSize.Level1)
{
    const auto [known, stack, backward, align] = GetParam();
    auto model = CreateList();
    model.SetLanes(3);
    model.SetStackFromEnd(stack);
    model.SetInitialIndex(backward ? 120 : 0);
    constexpr int32_t groupIndex = 25;
    constexpr float groupHeight = 29 * ITEM_MAIN_SIZE;
    if (known) {
        auto sizes = model.GetOrCreateListChildrenMainSize();
        sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
        sizes->ChangeData(groupIndex, 1, { groupHeight });
    }
    CreateListItems(groupIndex);
    auto group = CreateListItemGroup();
    if (known) {
        group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(85);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(95);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(groupIndex, true, align, 0.0f, FastScrollMode::SKIP_MEASURE);
    RunFastScrollFrames(64);
    const auto rect = pattern_->GetItemRect(groupIndex);
    const float expected = align == ScrollAlign::START ? 0.0f :
        (align == ScrollAlign::END ? HEIGHT - groupHeight : (HEIGHT - groupHeight) / 2);
    EXPECT_NEAR(rect.Height(), groupHeight, 0.1f);
    EXPECT_NEAR(rect.Top(), expected, 0.1f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

INSTANTIATE_TEST_SUITE_P(MixedOuterRows, ListFastScrollMultiLaneGroupTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END)));

class ListFastScrollKnownGroupControlTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<ScrollAlign> {};

/**
 * @tc.name: LegacyKnownStackGroupAlignmentControl
 * @tc.desc: The ordinary API controls the existing CMS reversal geometry without invoking FastScroll.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollKnownGroupControlTestNg, LegacyKnownStackGroupAlignmentControl, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(3);
    model.SetStackFromEnd(true);
    model.SetInitialIndex(120);
    auto sizes = model.GetOrCreateListChildrenMainSize();
    sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
    sizes->ChangeData(25, 1, { 29 * ITEM_MAIN_SIZE });
    CreateListItems(25);
    auto group = CreateListItemGroup();
    group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    CreateListItems(85);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(95);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndex(25, true, GetParam());
    FlushUITasks();
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(64));
    const auto rect = pattern_->GetItemRect(25);
    const float expected = GetParam() == ScrollAlign::START ? 0.0f : HEIGHT - 29 * ITEM_MAIN_SIZE;
    EXPECT_NEAR(rect.Height(), 29 * ITEM_MAIN_SIZE, 0.1f);
    EXPECT_NEAR(rect.Top(), expected, 0.1f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

INSTANTIATE_TEST_SUITE_P(OriginalCMS, ListFastScrollKnownGroupControlTestNg,
    testing::Values(ScrollAlign::START, ScrollAlign::END),
    ([](const testing::TestParamInfo<ScrollAlign>& info) {
        return std::to_string(info.param == ScrollAlign::START ? 0 : 2);
    }));

/**
 * @tc.name: FastScrollMultiLaneHomogeneousGroups
 * @tc.desc: The no-build mode treats every Group as one outer row and keeps internal lanes, spacing and headers.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollMultiLaneGroupTestNg, FastScrollMultiLaneHomogeneousGroups, TestSize.Level1)
{
    const auto [known, stack, backward, align] = GetParam();
    auto model = CreateList();
    model.SetLanes(3);
    model.SetStackFromEnd(stack);
    model.SetInitialIndex(backward ? 60 : 0);
    constexpr float groupHeight = 3 * ITEM_MAIN_SIZE + 2 * SPACE + 2 * GROUP_HEADER_LEN;
    if (known) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(groupHeight);
    }
    for (int32_t index = 0; index < 70; ++index) {
        auto group = CreateListItemGroup();
        group.SetSpace(Dimension(SPACE));
        group.SetHeader(GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN)));
        group.SetFooter(GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN)));
        if (known) {
            group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        CreateListItems(7);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    CreateDone();
    const int32_t target = backward ? 10 : 40;
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(target, true, align, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(64));
    const float top = align == ScrollAlign::START ? 0.0f :
        (align == ScrollAlign::END ? HEIGHT - groupHeight : (HEIGHT - groupHeight) / 2.0f);
    EXPECT_NEAR(pattern_->GetItemRect(target).Top(), top, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(target).Height(), groupHeight, 0.1f);
    pattern_->ScrollBy(23.0f);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetItemRect(target).Top(), top - 23.0f, 0.1f);
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollWaitingLayoutStop001
 * @tc.desc: Cancellation after the physical animation ends still closes the logical request.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollWaitingLayoutStop001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    pattern_->StopAnimation(pattern_->springAnimation_);
    ASSERT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(pattern_->scrollStop_);
    pattern_->StopAnimate();
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_TRUE(pattern_->scrollStop_);
    FlushUITasks();
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollLargeGroupCenter001
 * @tc.desc: A target group may be fully measured for exact center alignment after an estimated start.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollLargeGroupCenter001, TestSize.Level1)
{
    CreateList();
    CreateListItems(5);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 100);
    CreateListItems(5);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(5, true, ScrollAlign::CENTER, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    RunFastScrollFrames();
    ASSERT_EQ(pattern_->itemPosition_.count(5), 1);
    const float alignedTop = (pattern_->contentMainSize_ - 100 * ITEM_MAIN_SIZE) / 2;
    EXPECT_NEAR(pattern_->GetItemRect(5).Top(), alignedTop, 0.1f);
    EXPECT_NEAR(pattern_->GetTotalOffset(), 5 * ITEM_MAIN_SIZE - alignedTop, 0.1f);
}

void ListScrollerTestNg::CreateGroupWithSettingChildrenMainSize(int32_t groupNumber)
{
    for (int32_t index = 0; index < groupNumber; ++index) {
        auto header = GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN));
        auto footer = GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN));
        ListItemGroupModelNG groupModel = CreateListItemGroup();
        groupModel.SetSpace(Dimension(SPACE));
        groupModel.SetDivider(ITEM_DIVIDER);
        groupModel.SetHeader(std::move(header));
        groupModel.SetFooter(std::move(footer));
        auto childrenSize = groupModel.GetOrCreateListChildrenMainSize();
        childrenSize->UpdateDefaultSize(ITEM_MAIN_SIZE);
        const int32_t itemNumber = 2;
        childrenSize->ChangeData(1, itemNumber, { 50.f, 200.f });
        CreateListItems(1);
        CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.f)));
        CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.f)));
        CreateListItems(1);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
}

void ListScrollerTestNg::CreateGroupChildrenMainSize(int32_t groupNumber)
{
    for (int32_t index = 0; index < groupNumber; index++) {
        ListItemGroupModelNG groupModel = CreateListItemGroup();
        auto childrenSize = groupModel.GetOrCreateListChildrenMainSize();
        childrenSize->UpdateDefaultSize(ITEM_MAIN_SIZE);
        const int32_t itemNumber = 2;
        childrenSize->ChangeData(1, itemNumber, { 50.f, 200.f });
        CreateListItems(1);
        CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.f)));
        CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.f)));
        CreateListItems(1);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
}

void ListScrollerTestNg::CreateGroupWithItem(int32_t groupNumber, Axis axis)
{
    for (int32_t index = 0; index < groupNumber; index++) {
        if (index & 1) {
            CreateListItems(1);
        } else {
            CreateListItemGroups(1);
        }
    }
}

/**
 * @tc.name: ScrollToIndex001
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex001, TestSize.Level1)
{
    /**
     * @tc.cases: Set ContentStartOffset and ContentEndOffset, ScrollTo index:3
     * @tc.expected: Each test scroll the correct distance
     */
    float offset = 25.0f;
    ListModelNG model = CreateList();
    model.SetContentStartOffset(offset);
    model.SetContentEndOffset(offset);
    CreateListItemGroups(8);
    CreateDone();
    int32_t index = 3;
    auto groupNode = frameNode_->GetChildByIndex(index)->GetHostNode();
    auto groupPattern = groupNode->GetPattern<ListItemGroupPattern>();
    auto groupLayoutProperty = groupNode->GetLayoutProperty<ListItemGroupLayoutProperty>();
    groupLayoutProperty->UpdateVisibility(VisibleType::GONE);

    float groupHeight = ITEM_MAIN_SIZE * GROUP_ITEM_NUMBER; // 200.0f
    ScrollToIndex(index, true, ScrollAlign::START);
    EXPECT_TRUE(TickPosition(-(index * groupHeight - offset)));
    ScrollToIndex(index, true, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-400.0f));
    ScrollToIndex(index, true, ScrollAlign::END);
    EXPECT_TRUE(TickPosition(-(200.0f + offset)));

    layoutProperty_->UpdateScrollSnapAlign(ScrollSnapAlign::CENTER);
    ScrollToIndex(index, true, ScrollAlign::START);
    EXPECT_TRUE(TickPosition(-index * groupHeight));
    ScrollToIndex(index, true, ScrollAlign::CENTER);
    EXPECT_TRUE(TickPosition(-400.0f));
    ScrollToIndex(index, true, ScrollAlign::END);
    EXPECT_TRUE(TickPosition(-200.0f));
}

/**
 * @tc.name: ScrollToIndex002
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex002, TestSize.Level1)
{
    /**
     * @tc.cases: ScrollToIndex(1, true, ScrollAlign::AUTO)
     * @tc.expected: Each test scroll the correct distance
     */
    SizeT<Dimension> itemSize = SizeT<Dimension>(FILL_LENGTH, Dimension(1000.0f));
    CreateList();
    CreateItemWithSize(4, itemSize);
    CreateDone();

    /**
     * @tc.cases: bottomOffset > topOffset > 0
     * @tc.expected: top Align
     */
    ScrollTo(800);
    EXPECT_EQ(pattern_->GetTotalOffset(), 800);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1000.0f));

    /**
     * @tc.cases: bottomOffset > topOffset == 0
     * @tc.expected: top Align
     */
    ScrollTo(1000);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1000);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1000.0f));

    /**
     * @tc.cases: bottomOffset > 0 > topOffset and |topOffset| < |bottomOffset|
     * @tc.expected: top Align
     */
    ScrollTo(1050);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1050);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1050.0f));

    /**
     * @tc.cases: bottomOffset > 0 > topOffset and |topOffset| == |bottomOffset|
     * @tc.expected: bottom Align
     */
    ScrollTo(1300);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetTotalOffset(), 1300);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1600.0f));

    /**
     * @tc.cases: bottomOffset == 0 > topOffset
     * @tc.expected: bottom Align
     */
    ScrollTo(1400);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1400);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1600.0f));

    /**
     * @tc.cases: 0 > bottomOffset > topOffset
     * @tc.expected: bottom Align
     */
    ScrollTo(1700);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1700);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-1600.0f));
}

/**
 * @tc.name: ScrollToIndex003
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create list.
     * @tc.cases: the height of listItemGroup is more than one screen
     */
    ListModelNG model = CreateList();
    model.SetContentStartOffset(50.0f);
    model.SetContentEndOffset(50.0f);
    model.SetInitialIndex(1);
    CreateGroupWithSetting(4, V2::ListItemGroupStyle::NONE, 8);
    CreateDone();
    EXPECT_EQ(pattern_->contentStartOffset_, 50.0f);
    EXPECT_EQ(pattern_->contentEndOffset_, 50.0f);

    /**
     * @tc.steps: step2. scroll to the group(index:0).
     * @tc.cases: jumpIndex < StartIndex
     * @tc.expected: pattern_->GetTotalOffset() == 620.
     *     GroupHight = Header(50) + footer(50) + (item(100) + space(10))*count(8) - space(10) = 970
     *     Offset = GroupHight(970) - (ListHeight(400) - ContentEndOffset(50)) = 620
     */
    ScrollToIndex(0, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->GetTotalOffset(), 620.0f);
    auto itemHeight = GetChildFrameNode(frameNode_, 0)->GetGeometryNode()->GetMarginFrameSize().Height();
    EXPECT_GT(itemHeight, pattern_->contentMainSize_);

    /**
     * @tc.steps: step2. scroll to group(index:1).
     * @tc.cases: jumpIndex < StartIndex
     * @tc.expected: pattern_->GetTotalOffset() == 920.0f.
     *     Offset = GroupHight(970) - ContentStartOffset(50) = 920
     */
    ScrollToIndex(1, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->GetTotalOffset(), 920.0f);
    itemHeight = GetChildFrameNode(frameNode_, 1)->GetGeometryNode()->GetMarginFrameSize().Height();
    EXPECT_GT(itemHeight, pattern_->contentMainSize_);
}

/**
 * @tc.name: ScrollToIndex004
 * @tc.desc: Test ScrollToIndex with extraOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex004, TestSize.Level1)
{
    /**
     * @tc.cases: ScrollToIndex without animation
     * @tc.expected: GetTotalOffset is right
     */
    CreateList();
    CreateListItems(20);
    CreateDone();
    std::optional<float> extraOffset = -200.0f;
    ScrollToIndex(1, false, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(Position(0));
    ScrollToIndex(18, false, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(Position(-1600.0f));
    ScrollToIndex(18, false, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(Position(-1300.0f));
    ScrollToIndex(LAST_ITEM, false, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(Position(-1400.0f));

    extraOffset = 200.0f;
    ScrollToIndex(1, false, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(Position(-300.0f));
    ScrollToIndex(1, false, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(Position(0));
    ScrollToIndex(18, false, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(Position(-1600.0f));
    ScrollToIndex(LAST_ITEM, false, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(Position(-1600.0f));
}

/**
 * @tc.name: ScrollToIndex005
 * @tc.desc: Test ScrollToIndex with extraOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex005, TestSize.Level1)
{
    /**
     * @tc.cases: ScrollToIndex with animation
     * @tc.expected: GetFinalPosition is right
     */
    CreateList();
    CreateListItems(20);
    CreateDone();
    std::optional<float> extraOffset = -200.0f;
    ScrollToIndex(1, true, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(0));
    ScrollToIndex(18, true, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(-1600.0f));
    ScrollToIndex(18, true, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-1300.0f));
    ScrollToIndex(LAST_ITEM, true, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-1300.0f));

    extraOffset = 200.0f;
    ScrollToIndex(1, true, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(-300.0f));
    ScrollToIndex(1, true, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(0));
    ScrollToIndex(18, true, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-1600.0f));
    ScrollToIndex(LAST_ITEM, true, ScrollAlign::END, extraOffset);
    EXPECT_TRUE(TickPosition(-1600.0f));
}

/**
 * @tc.name: ScrollToIndex006
 * @tc.desc: Test whether the extraOffset has been reset normally.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex006, TestSize.Level1)
{
    /**
     * @tc.cases: ScrollToIndex with animation
     * @tc.expected: GetFinalPosition is right
     */
    CreateList();
    CreateListItems(20);
    CreateDone();
    ASSERT_NE(pattern_, nullptr);
    std::optional<float> extraOffset = -200.0f;

    /**
     * @tc.steps: step2. scroll to the outside item.
     * @tc.expected: The extraOffset_ of List will be reset.
     */
    ScrollToIndex(-1, true, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(0));
    EXPECT_FALSE(pattern_->extraOffset_.has_value());

    ScrollToIndex(18, true, ScrollAlign::START, extraOffset);
    EXPECT_TRUE(TickPosition(-1600.0f));
    ScrollToIndex(20, true, ScrollAlign::START, extraOffset);
    EXPECT_FALSE(pattern_->extraOffset_.has_value());
}

/**
 * @tc.name: ScrollToIndex007
 * @tc.desc: Test ScrollToIndex with update estimate offset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex007, TestSize.Level1)
{
    /**
     * @tc.cases: Test ScrollToIndex with update estimate offset
     * @tc.expected: List will update Animation.
     */
    auto model = CreateList();
    model.SetInitialIndex(4);
    for (int32_t i = 0; i < 3; i++) {
        ListItemModelNG itemModel;
        itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
        SetSize(Axis::VERTICAL, CalcLength(FILL_LENGTH), CalcLength(200));
        ViewStackProcessor::GetInstance()->Pop();
    }
    for (int32_t i = 0; i < 3; i++) {
        ListItemModelNG itemModel;
        itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
        SetSize(Axis::VERTICAL, CalcLength(FILL_LENGTH), CalcLength(300));
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 1200);

    ScrollToIndex(2, true, ScrollAlign::CENTER, 0);
    EXPECT_TRUE(TickPosition(-400.0f)); // estimate:400, real:300
    MockAnimationManager::GetInstance().SetTicks(4);
    ScrollToIndex(1, true, ScrollAlign::CENTER, 0);
    EXPECT_TRUE(TickPosition(-350.0f));
    EXPECT_TRUE(TickPosition(-300.0f));
    EXPECT_TRUE(TickPosition(-150.0f));
    EXPECT_TRUE(TickPosition(-137.5f)); // Update Animation
    EXPECT_TRUE(TickPosition(-125.0f));
    EXPECT_TRUE(TickPosition(-112.5f));
    EXPECT_TRUE(TickPosition(-100.0f));
}

/**
 * @tc.name: ScrollToIndex008
 * @tc.desc: Test ScrollTo top with update estimate offset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex008, TestSize.Level1)
{
    /**
     * @tc.cases: Test ScrollTo top with update estimate offset
     * @tc.expected: List will scroll to top.
     */
    auto model = CreateList();
    model.SetInitialIndex(4);
    for (int32_t i = 0; i < 3; i++) {
        ListItemModelNG itemModel;
        itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
        SetSize(Axis::VERTICAL, CalcLength(FILL_LENGTH), CalcLength(180));
        ViewStackProcessor::GetInstance()->Pop();
    }
    for (int32_t i = 0; i < 4; i++) {
        ListItemModelNG itemModel;
        itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
        SetSize(Axis::VERTICAL, CalcLength(FILL_LENGTH), CalcLength(150));
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 600); // estimate:600, real:690

    MockAnimationManager::GetInstance().SetTicks(12);
    AnimateTo(Dimension(0), 100.0f, nullptr, true);
    for (int32_t i = 0; i < 11; i++) {
        MockAnimationManager::GetInstance().TickByVelocity(-550.0f + i * 50);
        FlushUITasks();
    }
    EXPECT_TRUE(TickPosition(0));
}

/**
 * @tc.name: ScrollToIndex009
 * @tc.desc: Test the onReachStart event after executing ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex009, TestSize.Level1)
{
    /**
     * @tc.cases: Create the List
     * @tc.expected: Create the List successfully
     */
    SizeT<Dimension> itemSize = SizeT<Dimension>(FILL_LENGTH, Dimension(1000.0f));
    ListModelNG model = CreateList();
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(12);
    CreateDone();

    /**
     * @tc.cases: Scroll to the 7 index
     * @tc.expected: the startIndex of List is 7 and startIndexChanged_ is true
     */
    ScrollToIndex(7, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->GetStartIndex(), 7);
    EXPECT_EQ(pattern_->prevStartOffset_, 0.f);
    EXPECT_EQ(pattern_->contentStartOffset_, 0.f);
    EXPECT_TRUE(pattern_->startIndexChanged_);

    /**
     * @tc.cases: Scroll to the 0 index
     * @tc.expected: the startIndex of List is 0, startIndexChanged_ and scrollDownToStart are true
     */
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->GetStartIndex(), 0);
    EXPECT_EQ(pattern_->prevStartOffset_, 0.f);
    EXPECT_EQ(pattern_->contentStartOffset_, 0.f);
    EXPECT_TRUE(pattern_->startIndexChanged_);
    auto scrollDownToStart =
        (pattern_->startIndexChanged_ || LessNotEqual(pattern_->prevStartOffset_, pattern_->contentStartOffset_) ||
            !pattern_->isInitialized_) &&
        GreatOrEqual(pattern_->startMainPos_, pattern_->contentStartOffset_);
    EXPECT_TRUE(scrollDownToStart);

    /**
     * @tc.cases: Scroll to the 0 index again
     * @tc.expected: the startIndex of List is 0, startIndexChanged_ and scrollDownToStart are false
     */
    ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->GetStartIndex(), 0);
    EXPECT_EQ(pattern_->prevStartOffset_, 0.f);
    EXPECT_EQ(pattern_->contentStartOffset_, 0.f);
    EXPECT_FALSE(pattern_->startIndexChanged_);
    scrollDownToStart =
        (pattern_->startIndexChanged_ || LessNotEqual(pattern_->prevStartOffset_, pattern_->contentStartOffset_) ||
            !pattern_->isInitialized_) &&
        GreatOrEqual(pattern_->startMainPos_, pattern_->contentStartOffset_);
    EXPECT_FALSE(scrollDownToStart);
}

/**
 * @tc.name: ScrollToIndex010
 * @tc.desc: Test ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndex010, TestSize.Level1)
{
    ListModelNG model = CreateList();
    CreateListItems(8);
    CreateDone();
    int32_t index = 7;

    ScrollToIndex(index, true, ScrollAlign::START);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();

    EXPECT_FALSE(pattern_->animateOverScrollEnd_);
}

/**
 * @tc.name: ScrollToIndexTwice001
 * @tc.desc: Test the state after calling ScrollToIndex twice
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndexTwice001, TestSize.Level1)
{
    /**
     * @tc.cases: Create the List
     * @tc.expected: Create the List successfully and index range is 0-3
     */
    ListModelNG model = CreateList();
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(12);
    CreateDone();
    EXPECT_EQ(pattern_->GetStartIndex(), 0);
    EXPECT_EQ(pattern_->GetEndIndex(), 3);

    /**
     * @tc.cases: ScrollToIndex with the 7 index and then ScrollToIndex with the 0 index
     * @tc.expected: targetIndex_ is 7, and then changed to 0
     */
    pattern_->ScrollToIndex(7, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->targetIndex_.value_or(-1), 7);
    pattern_->ScrollToIndex(0, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->targetIndex_.value_or(-1), -1);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetStartIndex(), 0);
    EXPECT_EQ(pattern_->GetEndIndex(), 3);

    /**
     * @tc.cases: ScrollToIndex with the 1 index and then ScrollToIndex with the 0 index
     * @tc.expected: targetIndex_ invalid, but aniamte state change from true to false
     */
    pattern_->ScrollToIndex(1, true, ScrollAlign::START);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->targetIndex_.value_or(-1), -1);
    pattern_->ScrollToIndex(0, true, ScrollAlign::START);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->targetIndex_.value_or(-1), -1);
}

/**
 * @tc.name: JumpToItemInGroup001
 * @tc.desc: Test JumpToItemInGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup001, TestSize.Level1)
{
    CreateList();
    CreateListItemGroups(8);
    CreateDone();

    /**
     * @tc.steps: step2. index is -2.
     * @tc.expected: not scroll.
     */
    JumpToItemInGroup(-2, 1, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 1);
    EXPECT_EQ(pattern_->itemPosition_[0].startPos, 0);
    EXPECT_EQ(pattern_->itemPosition_[1].endPos, 400);

    /**
     * @tc.steps: step3. index is -1.
     * @tc.expected: not scroll.
     */
    JumpToItemInGroup(ListLayoutAlgorithm::LAST_ITEM, 1, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 1);
    EXPECT_EQ(pattern_->itemPosition_[0].startPos, 0);
    EXPECT_EQ(pattern_->itemPosition_[1].endPos, 400);

    /**
     * @tc.steps: step4. indexInGroup is std::nullopt.
     * @tc.expected: startIndex_ = 0 and endIndex_ = 2.
     */
    ScrollToIndex(1, true, ScrollAlign::CENTER);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[0].startPos, -100);
    EXPECT_EQ(pattern_->itemPosition_[1].endPos, 300);

    /**
     * @tc.steps: step5. group within the screen.
     * @tc.expected: not scroll.
     */
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[0].startPos, -100);
    EXPECT_EQ(pattern_->itemPosition_[1].endPos, 300);
}

/**
 * @tc.name: JumpToItemInGroup002
 * @tc.desc: Test JumpToItemInGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup002, TestSize.Level1)
{
    const int32_t number = 8;
    CreateList();
    CreateGroupWithItem(number);
    CreateDone();

    /**
     * @tc.steps: step2. index is item.
     * @tc.expected: not scroll.
     */
    JumpToItemInGroup(1, 1, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);

    /**
     * @tc.steps: step2. index is greater than list size.
     * @tc.expected: not scroll.
     */
    JumpToItemInGroup(8, 1, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);

    /**
     * @tc.steps: step3. indexInGroup is greater than item size in group.
     * @tc.expected: not scroll.
     */
    JumpToItemInGroup(2, 4, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);

    /**
     * @tc.steps: step4. index is greater than the size in list.
     * @tc.expected: not scroll.
     */
    ScrollToIndex(number, true, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 2);

    /**
     * @tc.steps: step5. index is -1.
     * @tc.expected: not scroll.
     */
    ScrollToIndex(ListLayoutAlgorithm::LAST_ITEM, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
}

/**
 * @tc.name: JumpToItemInGroup003
 * @tc.desc: Test JumpToItemInGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup003, TestSize.Level1)
{
    ListModelNG model = CreateList();
    model.SetSticky(V2::StickyStyle::BOTH);
    CreateGroupWithSetting(8, V2::ListItemGroupStyle::NONE);
    CreateDone();
    JumpToItemInGroup(2, 1, true, ScrollAlign::AUTO);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, -220);
    EXPECT_EQ(pattern_->itemPosition_[2].endPos, 400);

    JumpToItemInGroup(2, 1, true, ScrollAlign::END);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, -220);
    EXPECT_EQ(pattern_->itemPosition_[2].endPos, 400);

    layoutProperty_->UpdateScrollSnapAlign(ScrollSnapAlign::CENTER);
    layoutProperty_->UpdateStickyStyle(V2::StickyStyle::HEADER);
    JumpToItemInGroup(2, 1, true, ScrollAlign::START);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[2].startPos, 90);
    EXPECT_EQ(pattern_->itemPosition_[3].endPos, 0);

    JumpToItemInGroup(2, 1, true, ScrollAlign::AUTO);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[2].startPos, 90);
    EXPECT_EQ(pattern_->itemPosition_[3].endPos, 0);

    layoutProperty_->UpdateStickyStyle(V2::StickyStyle::FOOTER);
    JumpToItemInGroup(2, 1, true, ScrollAlign::AUTO);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[2].startPos, 90);
    EXPECT_EQ(pattern_->itemPosition_[3].endPos, 0);

    JumpToItemInGroup(2, 1, true, ScrollAlign::END);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 2);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, -220);
    EXPECT_EQ(pattern_->itemPosition_[2].endPos, 400);
}

/**
 * @tc.name: JumpToItemInGroup004
 * @tc.desc: Test JumpToItemInGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create list.
     */
    ListModelNG model = CreateList();
    model.SetContentStartOffset(50.0f);
    model.SetContentEndOffset(50.0f);
    CreateListItemGroups(5);
    CreateDone();
    EXPECT_EQ(pattern_->contentStartOffset_, 50.0f);
    EXPECT_EQ(pattern_->contentEndOffset_, 50.0f);

    /**
     * @tc.steps: step2. scroll to the last index.
     * @tc.expected: pattern_->itemPosition_[4].endPos == 750.
     */
    JumpToItemInGroup(4, 1, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 3);
    EXPECT_EQ(pattern_->endIndex_, 4);
    EXPECT_EQ(pattern_->itemPosition_[3].startPos, -50);
    EXPECT_EQ(pattern_->itemPosition_[4].endPos, 350);

    JumpToItemInGroup(4, 1, false, ScrollAlign::CENTER);
    EXPECT_EQ(pattern_->startIndex_, 3);
    EXPECT_EQ(pattern_->endIndex_, 4);
    EXPECT_EQ(pattern_->itemPosition_[3].startPos, -50);
    EXPECT_EQ(pattern_->itemPosition_[4].endPos, 350);

    ScrollTo(0);
    JumpToItemInGroup(4, 1, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->startIndex_, 3);
    EXPECT_EQ(pattern_->endIndex_, 4);
    EXPECT_EQ(pattern_->itemPosition_[3].startPos, -50);
    EXPECT_EQ(pattern_->itemPosition_[4].endPos, 350);

    /**
     * @tc.steps: step3. scroll to the 1st index.
     * @tc.cases: jumpIndex < StartIndex
     * @tc.expected: pattern_->GetTotalOffset() == -50.
     */
    ScrollToIndex(0, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->GetTotalOffset(), -50);
    auto itemHeight = GetChildFrameNode(frameNode_, 0)->GetGeometryNode()->GetMarginFrameSize().Height();
    EXPECT_LT(itemHeight, pattern_->contentMainSize_);

    /**
     * @tc.steps: step4. scroll to the 3rd index.
     * @tc.cases: jumpIndex > EndIndex
     * @tc.expected: pattern_->GetTotalOffset() == 450.
     */
    ScrollToIndex(3, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->GetTotalOffset(), 450);
    itemHeight = GetChildFrameNode(frameNode_, 3)->GetGeometryNode()->GetMarginFrameSize().Height();
    EXPECT_LT(itemHeight, pattern_->contentMainSize_);
}

/**
 * @tc.name: JumpToItemInGroup005
 * @tc.desc: Test ScrollToItemInGroup With Margin
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create list with margin.
     */
    ListModelNG model = CreateList();
    CreateListItemGroups(4);
    CreateDone();

    MarginProperty groupMargin = { CalcLength(10), CalcLength(10), CalcLength(0), CalcLength(10) };
    MarginProperty itemMargin = { CalcLength(5), CalcLength(5), CalcLength(0), CalcLength(5) };
    for (int i = 0; i < 4; ++i) {
        auto groupNode = GetChildFrameNode(frameNode_, i);
        for (int j = 0; j < GROUP_ITEM_NUMBER; ++j) {
            auto itemLayoutProperty = GetChildLayoutProperty<ListItemLayoutProperty>(groupNode, j);
            itemLayoutProperty->UpdateMargin(itemMargin);
        }
        auto itemGroupLayoutProperty = groupNode->GetLayoutProperty();
        itemGroupLayoutProperty->UpdateMargin(groupMargin);
    }
    FlushUITasks();
    EXPECT_EQ(GetChildHeight(frameNode_, 0), 210); // GROUP_ITEM_NUMBER * (ITEM_MAIN_SIZE + 5)
    EXPECT_EQ(pattern_->GetTotalHeight(), 880);    // 4 * (210 + 10)
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, 220);

    /**
     * @tc.steps: step2. JumpToItemInGroup.
     * @tc.expected: Each test scroll the correct distance.
     */
    JumpToItemInGroup(1, 1, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, 0);

    JumpToItemInGroup(2, 1, false, ScrollAlign::CENTER);
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_FLOAT_EQ(pattern_->itemPosition_[1].startPos, -72.5f);

    JumpToItemInGroup(1, 1, false, ScrollAlign::END);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 1);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, 190);
}

/**
 * @tc.name: JumpToItemInGroup006
 * @tc.desc: Test ScrollToItemInGroup With Padding
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, JumpToItemInGroup006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create list with padding.
     */
    ListModelNG model = CreateList();
    CreateListItemGroups(4);
    CreateDone();

    MarginProperty listPadding = { CalcLength(10), CalcLength(10), CalcLength(10), CalcLength(10) };
    MarginProperty groupPadding = { CalcLength(5), CalcLength(5), CalcLength(5), CalcLength(5) };
    layoutProperty_->UpdatePadding(listPadding);
    for (int i = 0; i < 4; ++i) {
        auto groupLayoutProperty = GetChildLayoutProperty<ListItemGroupLayoutProperty>(frameNode_, i);
        groupLayoutProperty->UpdatePadding(groupPadding);
    }
    FlushUITasks();
    EXPECT_EQ(GetChildHeight(frameNode_, 0), 210); // GROUP_ITEM_NUMBER * ITEM_MAIN_SIZE + groupPadding
    EXPECT_EQ(pattern_->GetTotalHeight(), 840);    // 4 * 210
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, 210);

    /**
     * @tc.steps: step2. JumpToItemInGroup.
     * @tc.expected: Each test scroll the correct distance.
     */
    JumpToItemInGroup(1, 1, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, -5);

    JumpToItemInGroup(2, 1, false, ScrollAlign::CENTER);
    EXPECT_EQ(pattern_->startIndex_, 1);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, -75);

    JumpToItemInGroup(1, 1, false, ScrollAlign::END);
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 1);
    EXPECT_EQ(pattern_->itemPosition_[1].startPos, 175);
}

/**
 * @tc.name: PositionController007
 * @tc.desc: Test PositionController function with Axis::VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PositionController007, TestSize.Level1)
{
    constexpr int32_t itemNumber = 20;
    ListModelNG model = CreateList();
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(itemNumber);
    CreateDone();
    auto controller = pattern_->positionController_;

    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset::Zero());

    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, SCROLL_FIXED_VELOCITY);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, SCROLL_FIXED_VELOCITY);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, SCROLL_FIXED_VELOCITY);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), -SCROLL_FIXED_VELOCITY);
    int32_t offsetTime = OFFSET_TIME;
    for (int i = 0; i < TIME_CHANGED_COUNTS; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtBottom());
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, SCROLL_FIXED_VELOCITY);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), SCROLL_FIXED_VELOCITY);
    offsetTime = OFFSET_TIME;
    for (int i = 0; i < TIME_CHANGED_COUNTS; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtTop());
}

namespace {
constexpr float SCROLL_FIXED_VELOCITY_008 = 400.f;
constexpr float OFFSET_TIME_008 = 200.f;
constexpr int32_t TIME_CHANGED_COUNTS_008 = 20 * 10;
} // namespace
/**
 * @tc.name: PositionController008
 * @tc.desc: Test PositionController function with ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PositionController008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List Item
     */
    constexpr int32_t itemNumber = 20;
    ListModelNG model = CreateList();

    /**
     * @tc.steps: step2. Set the direction to VERTICAL
     * expected: 1.The direction to VERTICAL
     *           2.The current Offset is 0
     */
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(itemNumber);
    CreateDone();
    auto controller = pattern_->positionController_;
    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset::Zero());

    /**
     * @tc.steps: step3. Scroll to the left edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, SCROLL_FIXED_VELOCITY_008);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step4. Scroll to the right edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, SCROLL_FIXED_VELOCITY_008);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step5. Scroll to the bottom edge
     * expected: 1.Return fixed verify
     * expected: 2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, SCROLL_FIXED_VELOCITY_008);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), -SCROLL_FIXED_VELOCITY_008);

    /**
     * @tc.steps: step6. 1. Set offset time
     *                   2. Set changed count
     *                   3. Flush layout
     * expected: Scroll to the bottom edge
     */
    int32_t offsetTime = OFFSET_TIME_008;
    for (int i = 0; i < TIME_CHANGED_COUNTS_008; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_008;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtBottom());

    /**
     * @tc.steps: step7. Scroll to the top edge
     * expected: 1.Fixed Verify is is non-zero.
     *           2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, SCROLL_FIXED_VELOCITY_008);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), SCROLL_FIXED_VELOCITY_008);

    /**
     * @tc.steps: step8. 1. Set offset time
     *                   2. Set changed count
     *                   3. Flush layout
     * expected: Scroll to the top edge
     */
    offsetTime = OFFSET_TIME_008;
    for (int i = 0; i < TIME_CHANGED_COUNTS_008; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_008;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtTop());

    /**
     * @tc.steps: step9. index is -1.
     * @tc.expected: not scroll.
     */
    ScrollToIndex(ListLayoutAlgorithm::LAST_ITEM, true, ScrollAlign::START);
    EXPECT_EQ(pattern_->startIndex_, 0);
}

namespace {
constexpr float SCROLL_FIXED_VELOCITY_009 = 600.f;
constexpr float OFFSET_TIME_009 = 100.f;
constexpr int32_t TIME_CHANGED_COUNTS_009 = 20 * 60;
} // namespace
/**
 * @tc.name: PositionController009
 * @tc.desc: Test PositionController function with ScrollTo
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PositionController009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List Item
     */
    constexpr int32_t itemNumber = 20;
    ListModelNG model = CreateList();

    /**
     * @tc.steps: step2. Set the direction to VERTICAL
     * expected: 1.The direction to VERTICAL
     *           2.The current Offset is 0
     */
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(itemNumber);
    CreateDone();
    auto controller = pattern_->positionController_;

    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset::Zero());

    /**
     * @tc.steps: step3. Scroll to the left edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, SCROLL_FIXED_VELOCITY_009);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step4. Scroll to the right edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, SCROLL_FIXED_VELOCITY_009);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step5. Scroll to the bottom edge
     * expected: 1.Return fixed verify
     * expected: 2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, SCROLL_FIXED_VELOCITY_009);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), -SCROLL_FIXED_VELOCITY_009);

    /**
     * @tc.steps: step6. 1. Set offset time
     *                   2. Set changed count
     *                   3. Flush layout
     * expected: Scroll to the bottom edge
     */
    int32_t offsetTime = OFFSET_TIME_009;
    for (int i = 0; i < TIME_CHANGED_COUNTS_009; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_009;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtBottom());

    /**
     * @tc.steps: step7. Scroll to the top edge
     * expected: 1.Fixed Verify is is non-zero.
     *           2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, SCROLL_FIXED_VELOCITY_009);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), SCROLL_FIXED_VELOCITY_009);
    offsetTime = OFFSET_TIME_009;
    for (int i = 0; i < TIME_CHANGED_COUNTS_009; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_009;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtTop());

    /**
     * @tc.steps: step8. bottomOffset > topOffset == 0
     * expected: top Align
     */
    ScrollTo(1000);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1000);
}

namespace {
constexpr float SCROLL_FIXED_VELOCITY_010 = 800.f;
constexpr float OFFSET_TIME_010 = 400.f;
constexpr int32_t TIME_CHANGED_COUNTS_010 = 20 * 40;
} // namespace
/**
 * @tc.name: PositionController010
 * @tc.desc: Test PositionController function with ScrollToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PositionController010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List Item
     */
    constexpr int32_t itemNumber = 20;
    ListModelNG model = CreateList();

    /**
     * @tc.steps: step2. Set the direction to VERTICAL
     * expected: 1.The direction to VERTICAL
     *           2.The current Offset is 0
     */
    model.SetListDirection(Axis::VERTICAL);
    CreateListItems(itemNumber);
    CreateDone();
    auto controller = pattern_->positionController_;

    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset::Zero());

    /**
     * @tc.steps: step3. Scroll to the left edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, SCROLL_FIXED_VELOCITY_010);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step4. Scroll to the right edge
     * expected: Return fixed verify
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, SCROLL_FIXED_VELOCITY_010);
    EXPECT_FALSE(pattern_->fixedVelocityMotion_);

    /**
     * @tc.steps: step5. Scroll to the bottom edge
     * expected: 1.Return fixed verify
     * expected: 2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, SCROLL_FIXED_VELOCITY_010);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), -SCROLL_FIXED_VELOCITY_010);

    /**
     * @tc.steps: step6. 1. Set offset time
     *                   2. Set changed count
     *                   3. Flush layout
     * expected: Scroll to the bottom edge
     */
    int32_t offsetTime = OFFSET_TIME_010;
    for (int i = 0; i < TIME_CHANGED_COUNTS_010; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_010;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtBottom());

    /**
     * @tc.steps: step7. Scroll to the top edge
     * expected: 1.Fixed Verify is is non-zero.
     *           2.The current Velocity is equal to the set fixed velocity
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, SCROLL_FIXED_VELOCITY_010);
    EXPECT_TRUE(pattern_->fixedVelocityMotion_);
    EXPECT_EQ(pattern_->fixedVelocityMotion_->GetCurrentVelocity(), SCROLL_FIXED_VELOCITY_010);

    /**
     * @tc.steps: step8. 1. Set offset time
     *                   2. Set changed count
     *                   3. Flush layout
     * expected: Scroll to the top edge
     */
    offsetTime = OFFSET_TIME_010;
    for (int i = 0; i < TIME_CHANGED_COUNTS_010; i++) {
        pattern_->fixedVelocityMotion_->OnTimestampChanged(offsetTime, 0.0f, false);
        offsetTime = offsetTime + OFFSET_TIME_010;
        FlushUITasks();
    }
    EXPECT_TRUE(pattern_->IsAtTop());

    /**
     * @tc.steps: step1. When has animator_ and not stop, call OnScrollCallback.
     * @tc.expected: Would stop.
     */
    pattern_->AnimateTo(1, 0, nullptr, true);
    EXPECT_FALSE(pattern_->AnimateStoped());
    double offset = 100.0;
    pattern_->OnScrollPosition(offset, SCROLL_FROM_START);
    EXPECT_TRUE(pattern_->scrollAbort_);
    pattern_->OnScrollCallback(100.f, SCROLL_FROM_START);
    EXPECT_TRUE(pattern_->scrollAbort_);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0);
    EXPECT_TRUE(pattern_->AnimateStoped());
}

/**
 * @tc.name: Pattern012
 * @tc.desc: Test ScrollToNode
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern012, TestSize.Level1)
{
    // test ScrollToNode
    CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    auto listItem = GetChildFrameNode(frameNode_, 5);
    bool ret = pattern_->ScrollToNode(listItem);
    EXPECT_TRUE(ret);

    // test GetTotalHeight
    ClearOldNodes();
    CreateList();
    CreateDone();
    EXPECT_EQ(pattern_->GetTotalHeight(), 0.0f);
    ClearOldNodes();
    CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    EXPECT_EQ(pattern_->GetTotalHeight(), TOTAL_ITEM_NUMBER * ITEM_MAIN_SIZE);

    // test ToJsonValue/FromJson
    ClearOldNodes();
    CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    auto json = JsonUtil::Create(true);
    InspectorFilter filter;
    pattern_->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetBool("multiSelectable"), false);
    EXPECT_EQ(json->GetInt("startIndex"), 0);
    EXPECT_EQ(json->GetDouble("itemStartPos"), 0.0);
    EXPECT_EQ(json->GetDouble("friction"), DEFAULT_FRICTION);
    pattern_->FromJson(json);
    EXPECT_EQ(pattern_->GetTotalOffset(), 0);
}

/**
 * @tc.name: Pattern013
 * @tc.desc: Test ScrollTo and ScrollBy
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create List and Set lanes
     */
    int32_t itemNumber = 10;
    SizeT<Dimension> itemSize = SizeT<Dimension>(FILL_LENGTH, FILL_LENGTH);
    ListModelNG model = CreateList();
    model.SetListDirection(Axis::VERTICAL);
    model.SetScrollBar(DisplayMode::OFF);
    CreateItemWithSize(itemNumber, itemSize);
    CreateDone();

    /**
     * @tc.steps: step2. swipe forward 3 listItem
     */
    ScrollTo(3 * HEIGHT);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(3), Rect(0, 0, WIDTH, HEIGHT)));

    /**
     * @tc.steps: step3. swipe backward 2.5 listItem
     */
    pattern_->ScrollBy(-2.5 * HEIGHT);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, -HEIGHT / 2.0, WIDTH, HEIGHT)));

    /**
     * @tc.steps: step4. swipe forward 3 listItem
     */
    pattern_->ScrollBy(3 * HEIGHT);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(3), Rect(0, -HEIGHT / 2.0, WIDTH, HEIGHT)));

    /**
     * @tc.steps: step5. swipe backward 2.5 listItem
     */
    ScrollTo(HEIGHT);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(1), Rect(0, 0, WIDTH, HEIGHT)));

    /**
     * @tc.cases: bottomOffset == topOffset > 0
     * @tc.expected: top Align
     */
    ScrollTo(400);
    EXPECT_EQ(pattern_->GetTotalOffset(), 400);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-400.0f));

    /**
     * @tc.cases: 0 > bottomOffset == topOffset
     * @tc.expected: top Align
     */
    ScrollTo(1200);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1200);
    ScrollToIndex(1, true, ScrollAlign::AUTO);
    EXPECT_TRUE(TickPosition(-400.0f));

    /**
     * @tc.cases: JumpIndex == StartIndex == EndIndex
     * @tc.expected: top Align
     */
    ScrollTo(800);
    ScrollToIndex(1, false, ScrollAlign::AUTO);
    EXPECT_EQ(pattern_->GetTotalOffset(), 400.0f);
}

/**
 * @tc.name: Pattern014
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern014, TestSize.Level1)
{
    int32_t itemNumber = 16;
    ListModelNG model = CreateList();
    model.SetScrollBar(DisplayMode::ON);
    model.SetEdgeEffect(EdgeEffect::NONE, true);
    CreateListItems(itemNumber);
    CreateDone();

    auto scrollBar = pattern_->GetScrollBar();
    Rect barRect = scrollBar->GetBarRect();
    Rect activeRectInit = scrollBar->GetActiveRect();
    Rect activeRectBot = Rect(WIDTH, HEIGHT - activeRectInit.Height(), 0, activeRectInit.Height());

    std::vector<int32_t> scrollFromVector = { SCROLL_FROM_NONE, SCROLL_FROM_UPDATE, SCROLL_FROM_ANIMATION,
        SCROLL_FROM_JUMP, SCROLL_FROM_ANIMATION_SPRING, SCROLL_FROM_BAR, SCROLL_FROM_ANIMATION_CONTROLLER,
        SCROLL_FROM_BAR_FLING };

    for (int32_t form : scrollFromVector) {
        bool isAtBottom = pattern_->IsAtBottom();
        float offset = isAtBottom ? 3 * HEIGHT : -3 * HEIGHT;
        EXPECT_TRUE(pattern_->UpdateCurrentOffset(offset, form));
        FlushUITasks();
        EXPECT_TRUE(IsEqual(scrollBar->GetBarRect(), barRect));
        if (isAtBottom) {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), 0);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectInit));
        } else {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), -1200);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectBot));
        }
    }
}

/**
 * @tc.name: Pattern015
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern015, TestSize.Level1)
{
    int32_t itemNumber = 16;
    ListModelNG model = CreateList();
    model.SetScrollBar(DisplayMode::ON);
    model.SetEdgeEffect(EdgeEffect::FADE, false);
    CreateListItems(itemNumber);
    CreateDone();

    auto scrollBar = pattern_->GetScrollBar();
    Rect barRect = scrollBar->GetBarRect();
    Rect activeRectInit = scrollBar->GetActiveRect();
    Rect activeRectBot = Rect(WIDTH, HEIGHT - activeRectInit.Height(), 0, activeRectInit.Height());

    std::vector<int32_t> scrollFromVector = { SCROLL_FROM_NONE, SCROLL_FROM_UPDATE, SCROLL_FROM_ANIMATION,
        SCROLL_FROM_JUMP, SCROLL_FROM_ANIMATION_SPRING, SCROLL_FROM_BAR, SCROLL_FROM_ANIMATION_CONTROLLER,
        SCROLL_FROM_BAR_FLING };

    for (int32_t form : scrollFromVector) {
        bool isAtBottom = pattern_->IsAtBottom();
        float offset = isAtBottom ? 3 * HEIGHT : -3 * HEIGHT;
        EXPECT_TRUE(pattern_->UpdateCurrentOffset(offset, form));
        FlushUITasks();
        EXPECT_TRUE(IsEqual(scrollBar->GetBarRect(), barRect));
        if (isAtBottom) {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), 0);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectInit));
        } else {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), -1200);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectBot));
        }
    }
}

/**
 * @tc.name: Pattern016
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern016, TestSize.Level1)
{
    int32_t itemNumber = 16;
    ListModelNG model = CreateList();
    model.SetScrollBar(DisplayMode::ON);
    model.SetEdgeEffect(EdgeEffect::SPRING, false);
    CreateListItems(itemNumber);
    CreateDone();

    auto scrollBar = pattern_->GetScrollBar();
    Rect barRect = scrollBar->GetBarRect();
    Rect activeRectInit = scrollBar->GetActiveRect();
    Rect activeRectBot = Rect(WIDTH, HEIGHT - activeRectInit.Height(), 0, activeRectInit.Height());

    std::vector<int32_t> scrollFromVector = { SCROLL_FROM_NONE, SCROLL_FROM_UPDATE, SCROLL_FROM_ANIMATION,
        SCROLL_FROM_JUMP, SCROLL_FROM_ANIMATION_SPRING, SCROLL_FROM_BAR, SCROLL_FROM_BAR_FLING };

    for (int32_t form : scrollFromVector) {
        bool isAtBottom = pattern_->IsAtBottom();
        float offset = isAtBottom ? 3 * HEIGHT : -3 * HEIGHT;
        EXPECT_TRUE(pattern_->UpdateCurrentOffset(offset, form));
        FlushUITasks();
        EXPECT_TRUE(IsEqual(scrollBar->GetBarRect(), barRect));
        if (isAtBottom) {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), 0);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectInit));
        } else {
            EXPECT_FLOAT_EQ(pattern_->GetBarOffset(), -1200);
            EXPECT_TRUE(IsEqual(scrollBar->GetActiveRect(), activeRectBot));
        }
    }
}

/**
 * @tc.name: Pattern017
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, Pattern017, TestSize.Level1)
{
    CreateList();
    CreateListItems(16);
    CreateDone();
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    pattern_->ScrollPage(false, false);
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 4);
    EXPECT_EQ(pattern_->endIndex_, 7);
    EXPECT_EQ(pattern_->currentOffset_, 400);

    pattern_->ScrollPage(true, false);
    FlushUITasks();
    EXPECT_EQ(pattern_->startIndex_, 0);
    EXPECT_EQ(pattern_->endIndex_, 3);
    EXPECT_EQ(pattern_->currentOffset_, 0);

    pattern_->ScrollPage(false, true);
    FlushUITasks();
    EXPECT_EQ(pattern_->finalPosition_, 400);

    pattern_->ScrollPage(true, true);
    FlushUITasks();
    EXPECT_EQ(pattern_->finalPosition_, -400);
}

/**
 * @tc.name: PostListItemPressStyleTask_scroll001
 * @tc.desc: Test list layout with PostListItemPressStyleTask scroll.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PostListItemPressStyleTask_scroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init List.
     */
    ListModelNG model = CreateList();
    model.SetDivider(ITEM_DIVIDER);
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    int cur = 0;
    for (auto& child : pattern_->itemPosition_) {
        child.second.id += cur;
        cur++;
    }

    auto listItemNode = GetChildFrameNode(frameNode_, 0);
    auto listItemNodeId = listItemNode->GetId();
    auto stateStyleMgr = AceType::MakeRefPtr<StateStyleManager>(listItemNode);
    stateStyleMgr->PostListItemPressStyleTask(UI_STATE_PRESSED);
    RefPtr<NodePaintMethod> paint = pattern_->CreateNodePaintMethod();
    RefPtr<ListPaintMethod> listPaint = AceType::DynamicCast<ListPaintMethod>(paint);
    for (auto child : listPaint->itemPosition_) {
        if (child.second.id == listItemNodeId) {
            EXPECT_TRUE(child.second.isPressed);
        }
    }

    stateStyleMgr->PostListItemPressStyleTask(UI_STATE_NORMAL);
    paint = pattern_->CreateNodePaintMethod();
    listPaint = AceType::DynamicCast<ListPaintMethod>(paint);
    for (auto child : listPaint->itemPosition_) {
        if (child.second.id == listItemNodeId) {
            EXPECT_FALSE(child.second.isPressed);
        }
    }
}

/**
 * @tc.name: PostListItemPressStyleTask_scroll002
 * @tc.desc: Test listItemGroup layout with PostListItemPressStyleTask scroll.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, PostListItemPressStyleTask_scroll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init List.
     */
    ListModelNG model = CreateList();
    model.SetDivider(ITEM_DIVIDER);
    CreateListItemGroups(TOTAL_ITEM_NUMBER);
    CreateDone();
    auto groupFrameNode = GetChildFrameNode(frameNode_, 0);
    auto groupPattern = groupFrameNode->GetPattern<ListItemGroupPattern>();
    int cur = 0;
    for (auto& child : groupPattern->itemPosition_) {
        child.second.id += cur;
        cur++;
    }

    auto listItemNode = GetChildFrameNode(groupFrameNode, 0);
    auto listItemNodeId = listItemNode->GetId();
    auto stateStyleMgr = AceType::MakeRefPtr<StateStyleManager>(listItemNode);
    stateStyleMgr->PostListItemPressStyleTask(UI_STATE_PRESSED);
    RefPtr<NodePaintMethod> paint = groupPattern->CreateNodePaintMethod();
    RefPtr<ListItemGroupPaintMethod> groupPaint = AceType::DynamicCast<ListItemGroupPaintMethod>(paint);
    for (auto child : groupPaint->itemPosition_) {
        if (child.second.id == listItemNodeId) {
            EXPECT_TRUE(child.second.isPressed);
        }
    }

    stateStyleMgr->PostListItemPressStyleTask(UI_STATE_NORMAL);
    paint = groupPattern->CreateNodePaintMethod();
    groupPaint = AceType::DynamicCast<ListItemGroupPaintMethod>(paint);
    for (auto child : groupPaint->itemPosition_) {
        if (child.second.id == listItemNodeId) {
            EXPECT_FALSE(child.second.isPressed);
        }
    }
}

/**
 * @tc.name: ChildrenMainSize001
 * @tc.desc: Test childrenMainSize layout
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ChildrenMainSize001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollable list, change some items height
     * @tc.expected: Items height are unequal
     */
    ListModelNG model = CreateList();
    auto childrenSize = model.GetOrCreateListChildrenMainSize();
    childrenSize->UpdateDefaultSize(ITEM_MAIN_SIZE);
    childrenSize->ChangeData(2, 3, { 50.0f, 100.0f, 200.0f });
    childrenSize->ChangeData(15, 3, { 50.0f, 100.0f, 200.0f });
    CreateListItems(2); // ITEM_MAIN_SIZE
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(100.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.0f)));
    CreateListItems(10);
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(100.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.0f)));
    CreateListItems(3);
    CreateDone();
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(2), 50.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(17), 200.0f);

    /**
     * @tc.steps: step2. ScrollToIndex, index:3 is in the view
     */
    int32_t index = 3;
    ScrollToIndex(index, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-250.0f));
    ScrollToIndex(index, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-100.0f));
    ScrollToIndex(index, false, ScrollAlign::END);
    EXPECT_TRUE(Position(0.0f));
    ScrollToIndex(index, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(0.0f));

    /**
     * @tc.steps: step3. ScrollToIndex, index:17 is out of the view, and will scroll beyond one screen
     */
    index = 17;
    ScrollToIndex(index, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1700.0f));
    ScrollToIndex(index, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1600.0f));
    ScrollToIndex(index, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-1500.0f));
    ScrollToIndex(index, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-1500.0f));

    /**
     * @tc.steps: step4. ScrollTo, 400.0f is in the view
     */
    ScrollTo(400.0f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 400.0f);

    /**
     * @tc.steps: step5. ScrollTo, 1400.0f is out of the view
     */
    ScrollTo(1400.0f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1400.0f);
}

/**
 * @tc.name: ChildrenMainSize002
 * @tc.desc: Test childrenMainSize layout with itemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ChildrenMainSize002, TestSize.Level1)
{
    ListModelNG model = CreateList();
    auto childrenSize = model.GetOrCreateListChildrenMainSize();
    childrenSize->UpdateDefaultSize(200.0f);
    childrenSize->ChangeData(1, 2, { 450.0f, 450.0f });
    childrenSize->ChangeData(4, 1, { 450.0f });
    CreateListItemGroups(1);
    CreateGroupChildrenMainSize(2);
    CreateListItemGroups(1);
    CreateGroupChildrenMainSize(1);
    CreateDone();

    /**
     * @tc.steps: step2. ScrollToIndex, index:1 is in the view
     */
    int32_t index = 1;
    ScrollToIndex(index, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-200.0f));
    ScrollToIndex(index, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-225.0f));
    ScrollToIndex(index, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-250.0f));
    ScrollToIndex(index, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-250.0f));

    /**
     * @tc.steps: step3. ScrollToIndex, index:4 is out of the view, and will scroll beyond one screen
     */
    index = 4;
    ScrollToIndex(index, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1300.0f));
    ScrollToIndex(index, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1325.0f));
    ScrollToIndex(index, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-1350.0f));
    ScrollToIndex(index, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-1350.0f));

    /**
     * @tc.steps: step4. JumpToItemInGroup, index:1 is in the view
     */
    index = 1;
    int32_t indexInGroup = 2;
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-350.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-250.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-150.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-150.0f));

    /**
     * @tc.steps: step5. JumpToItemInGroup, index:4 is out of the view
     */
    index = 4;
    indexInGroup = 3;
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1350.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1350.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-1350.0f));
    JumpToItemInGroup(index, indexInGroup, false, ScrollAlign::AUTO);
    EXPECT_TRUE(Position(-1350.0f));

    /**
     * @tc.steps: step4. ScrollTo, 400.0f is in the view
     */
    ScrollTo(400.0f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 400.0f);

    /**
     * @tc.steps: step5. ScrollTo, 1400.0f is out of the view
     */
    ScrollTo(1400.0f);
    EXPECT_EQ(pattern_->GetTotalOffset(), 1350.0f);
}

/**
 * @tc.name: ChildrenMainSize003
 * @tc.desc: Test childrenMainSize layout
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ChildrenMainSize003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create list
     * @tc.cases: listItemGroup  with header and footer and space
     */
    ListItemIndex endInfo;
    auto onVisibleChange = [&endInfo](ListItemIndex start, ListItemIndex end) { endInfo = end; };
    ListModelNG model = CreateList();
    model.SetOnScrollVisibleContentChange(onVisibleChange);
    auto childrenSize = model.GetOrCreateListChildrenMainSize();
    childrenSize->UpdateDefaultSize(100.0f);
    childrenSize->ChangeData(4, 1, { 530.0f });
    childrenSize->ChangeData(5, 1, { 580.0f });
    CreateListItems(4);
    CreateGroupWithSetting(1, V2::ListItemGroupStyle::NONE);
    CreateGroupWithSettingChildrenMainSize(1);
    CreateListItems(4);
    CreateDone();
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(4), 530.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(5), 580.0f);
    ListItemIndex endExpect = { 3, -1, -1 };
    EXPECT_TRUE(IsEqual(endInfo, endExpect));

    /**
     * @tc.steps: step2. ScrollTo 120
     * @tc.expected: endExpect.area == footer
     */
    ScrollTo(120);
    endExpect = { 4, 1, 0 };
    EXPECT_TRUE(IsEqual(endInfo, endExpect));

    /**
     * @tc.steps: step3. ScrollTo 300
     * @tc.expected: index:5 indexInGroup:1 height == 50
     */
    ScrollTo(300);
    auto groupPattern = frameNode_->GetChildByIndex(5)->GetHostNode()->GetPattern<ListItemGroupPattern>();
    EXPECT_EQ(groupPattern->childrenSize_->GetChildSize(1), 50.0f);
    endExpect = { 4, 3, -1 };
    EXPECT_TRUE(IsEqual(endInfo, endExpect));

    /**
     * @tc.steps: step4. ScrollTo 540
     * @tc.expected: index:5 indexInGroup:2 height == 200
     */
    ScrollTo(540);
    EXPECT_EQ(groupPattern->childrenSize_->GetChildSize(2), 200.0f);
    endExpect = { 5, 2, -1 };
    EXPECT_TRUE(IsEqual(endInfo, endExpect));
}

/**
 * @tc.name: ChildrenMainSize004
 * @tc.desc: Test childrenMainSize layout
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ChildrenMainSize004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create list with multi lanes and space
     * @tc.expected: GetItemRect is right
     */
    ListModelNG model = CreateList();
    model.SetSpace(Dimension(10.0f));
    model.SetLanes(2);
    auto childrenSize = model.GetOrCreateListChildrenMainSize();
    childrenSize->UpdateDefaultSize(ITEM_MAIN_SIZE);
    childrenSize->ChangeData(4, 1, { 200.0f });
    childrenSize->ChangeData(9, 1, { 450.0f });
    childrenSize->ChangeData(10, 4, { 50.0f, 50.0f, 200.0f, 200.0f });
    childrenSize->ChangeData(18, 4, { 50.0f, 200.0f, 50.0f, 200.0f });
    CreateListItems(4); // 0~3
    CreateListItemGroups(1);
    CreateListItems(4); // 5~8
    CreateGroupChildrenMainSize(1);
    CreateItemWithSize(2, SizeT<Dimension>(FILL_LENGTH, Dimension(50.0f)));
    CreateItemWithSize(2, SizeT<Dimension>(FILL_LENGTH, Dimension(200.0f)));
    CreateListItems(4); // 14~17
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(50.0f)));
    CreateItemWithSize(1, SizeT<Dimension>(FILL_LENGTH, Dimension(200.0f)));
    CreateListItems(8);
    CreateDone();
    JumpToItemInGroup(4, 1, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-220.0f));
    JumpToItemInGroup(4, 1, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-70.0f));
    JumpToItemInGroup(4, 1, false, ScrollAlign::END);
    EXPECT_TRUE(Position(0));
    JumpToItemInGroup(9, 1, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-650.0f));
    JumpToItemInGroup(9, 1, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-500.0f));
    JumpToItemInGroup(9, 1, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-350.0f));

    /**
     * @tc.steps: step2. ScrollToIndex 12 / ScrollToIndex 13
     * @tc.expected: GetTotalOffset is right
     */
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(10), 50.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(11), 50.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(12), 200.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(13), 200.0f);
    ScrollToIndex(10, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1110.0f));
    ScrollToIndex(10, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-935.0f));
    ScrollToIndex(10, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-760.0f));
    ScrollToIndex(13, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1170.0f));
    ScrollToIndex(13, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1070.0f));
    ScrollToIndex(13, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-970.0f));

    /**
     * @tc.steps: step3. ScrollToIndex 18 / ScrollToIndex 21
     * @tc.expected: GetTotalOffset is right
     */
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(18), 50.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(19), 200.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(20), 50.0f);
    EXPECT_EQ(pattern_->childrenSize_->GetChildSize(21), 200.0f);
    ScrollToIndex(18, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1600.0f));
    ScrollToIndex(18, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1500.0f));
    ScrollToIndex(18, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-1400.0f));
    ScrollToIndex(21, false, ScrollAlign::START);
    EXPECT_TRUE(Position(-1810.0f));
    ScrollToIndex(21, false, ScrollAlign::CENTER);
    EXPECT_TRUE(Position(-1710.0f));
    ScrollToIndex(21, false, ScrollAlign::END);
    EXPECT_TRUE(Position(-1610.0f));
}

/**
 * @tc.name: GetScrollEnabled001
 * @tc.desc: Test List model ng GetScrollEnabled
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, GetScrollEnabled001, TestSize.Level1)
{
    ListModelNG model = CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    EXPECT_EQ(model.GetScrollEnabled(AceType::RawPtr(frameNode_)), 1);
    model.SetScrollEnabled(AceType::RawPtr(frameNode_), false);
    EXPECT_EQ(model.GetScrollEnabled(AceType::RawPtr(frameNode_)), 0);
}

/**
 * @tc.name: SetScrollBy001
 * @tc.desc: Test List model ng SetScrollBy
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, SetScrollBy001, TestSize.Level1)
{
    ListModelNG model = CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    const double x = 1.0;
    const double y = 2.0;
    model.SetScrollBy(AceType::RawPtr(frameNode_), x, y);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetTotalOffset(), y);
    model.SetScrollBy(AceType::RawPtr(frameNode_), x, 0.0);
    EXPECT_EQ(pattern_->GetTotalOffset(), y);
}

/**
 * @tc.name: SetScrollBy002
 * @tc.desc: Test List model ng SetScrollBy before List layout
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, SetScrollBy002, TestSize.Level1)
{
    ListModelNG model = CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    const double y = 200.f;
    model.SetScrollBy(AceType::RawPtr(frameNode_), 0, y);
    CreateDone();
    EXPECT_EQ(pattern_->GetTotalOffset(), y);
}

/**
 * @tc.name: SetAutoScale001
 * @tc.desc: Test List item model ng SetAutoScale
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, SetAutoScale001, TestSize.Level1)
{
    ListItemModelNG itemModel;
    itemModel.Create([](int32_t) {}, V2::ListItemStyle::CARD, true);
    itemModel.SetAutoScale(true);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    auto pattern = frameNode->GetPattern<ListItemPattern>();
    itemModel.SetAutoScale(Referenced::RawPtr(frameNode), false);
    itemModel.CreateFrameNode(0, true);
    EXPECT_EQ(pattern->GetListItemStyle(), V2::ListItemStyle::CARD);
}

/**
 * @tc.name: BigItemScrollWithScrollbar
 * @tc.desc: Test BigItemScrollWithScrollbar
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, BigItemScrollWithScrollbar, TestSize.Level1)
{
    ListModelNG model = CreateList();
    model.SetScrollBar(DisplayMode::ON);
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    ListItemModelNG itemModel;
    itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
    Axis axis = layoutProperty_->GetListDirection().value_or(Axis::VERTICAL);
    SetSize(axis, CalcLength(FILL_LENGTH), CalcLength(1000));
    CreateDone();

    auto scrollBar = pattern_->GetScrollBar();
    GestureEvent info;
    info.SetMainVelocity(-1200.f);
    info.SetMainDelta(-2000.f);
    scrollBar->HandleDragUpdate(info);
    auto scrollableEvent = pattern_->GetScrollableEvent();
    ASSERT_NE(scrollableEvent, nullptr);
    auto scrollable = scrollableEvent->GetScrollable();
    EXPECT_NE(scrollable->callback_, nullptr);
    scrollable->HandleTouchUp();

    EXPECT_EQ(scrollable->state_, Scrollable::AnimationState::IDLE);
}

/**
 * @tc.name: GetBindingFrameNodeId001
 * @tc.desc: Test GetBindingFrameNodeId returns valid node id for List component
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, GetBindingFrameNodeId001, TestSize.Level1)
{
    CreateList();
    CreateListItems();
    CreateDone();

    /**
     * @tc.steps: step1. Get the binding frame node id from controller
     * @tc.expected: The node id should match the list frame node's id
     */
    auto nodeId = positionController_->GetBindingFrameNodeId();
    EXPECT_EQ(nodeId, frameNode_->GetId());
}

class ListFastScrollLayoutHandoffTestNg : public ListFastScrollerTestNg {
protected:
    RefPtr<ListLayoutAlgorithm> CreateHandoffResult(bool complete)
    {
        CreateList();
        CreateListItems(FAST_SCROLL_ITEM_COUNT);
        CreateDone();
        pattern_->InitializeFastScrollRequest(FAST_SCROLL_TARGET_INDEX, ScrollAlign::START, 25.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
        algorithm->SetFastScrollTarget(FAST_SCROLL_TARGET_INDEX, ScrollAlign::START);
        algorithm->SetFastScrollPreparationOffset(25.0f);
        algorithm->GetOrCreateFastScrollState().result.measuredSizes[FAST_SCROLL_TARGET_INDEX] = { 250.0f };
        algorithm->GetOrCreateFastScrollState().result.status = complete ?
            FastScrollMeasureStatus::COMPLETE : FastScrollMeasureStatus::PENDING;
        return algorithm;
    }
};

/**
 * @tc.name: FastScrollPartialLayoutPublishesMeasuredSizes001
 * @tc.desc: Interrupted layout publishes partial sizes without starting an animation or reporting displacement.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg, FastScrollPartialLayoutPublishesMeasuredSizes001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(false);
    pattern_->prevMeasureBreak_ = true;
    EXPECT_FLOAT_EQ(pattern_->UpdateTotalOffset(algorithm, false), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainSize, 250.0f);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::NEED_WINDOW);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_TRUE(pattern_->HasFastScrollRequest());
}

/**
 * @tc.name: FastScrollLayoutPublishesAfterNormalReestimate001
 * @tc.desc: Normal reestimation cannot erase the prepared target size or its connected positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg, FastScrollLayoutPublishesAfterNormalReestimate001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(true);
    pattern_->needReEstimateOffset_ = true;
    EXPECT_FLOAT_EQ(pattern_->UpdateTotalOffset(algorithm, false), 0.0f);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainSize, 250.0f);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->estimated);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_TRUE(std::isfinite(pattern_->posMap_->GetPositionInfo(60).mainPos));
}

/**
 * @tc.name: FastScrollPreparationCorrectionIsNotScrollDisplacement001
 * @tc.desc: Connecting a negative prepared prefix adjusts the origin without reporting it in this or next frame.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg, FastScrollPreparationCorrectionIsNotScrollDisplacement001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(true);
    pattern_->itemPosition_.clear();
    pattern_->itemPosition_[10] = { -1, 0.0f, 100.0f, false };
    pattern_->itemPosition_[11] = { -1, 100.0f, 200.0f, false };
    pattern_->currentOffset_ = 1000.0f;
    pattern_->posMap_->ClearPosMap();
    pattern_->posMap_->UpdatePos(10, { 1000.0f, 100.0f });
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[2] = { 2000.0f };
    EXPECT_FLOAT_EQ(pattern_->UpdateTotalOffset(algorithm, false), 0.0f);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_GT(pattern_->GetTotalOffset(), 1000.0f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(10).mainPos, pattern_->GetTotalOffset());
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(2).mainSize, 2000.0f);
    const float corrected = pattern_->GetTotalOffset();
    auto next = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    next->SetFastScrollTarget(60, ScrollAlign::START);
    EXPECT_FLOAT_EQ(pattern_->UpdateTotalOffset(next, false), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), corrected);
    EXPECT_FALSE(pattern_->AnimateRunning());
}

/**
 * @tc.name: FastScrollRunningTargetUpdatesWithoutCoordinateCorrection001
 * @tc.desc: Running animation checks the refreshed endpoint even when the visible origin is unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg,
    FastScrollRunningTargetUpdatesWithoutCoordinateCorrection001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(60, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    ASSERT_TRUE(pattern_->AnimateRunning());
    const float endpoint = pattern_->GetFinalPosition();
    auto info = pattern_->posMap_->GetPositionInfo(60);
    info.mainPos += 120.0f;
    pattern_->posMap_->UpdatePos(60, info);
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->SetFastScrollTarget(60, ScrollAlign::START);
    EXPECT_FLOAT_EQ(pattern_->UpdateTotalOffset(algorithm, false), 0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), endpoint + 120.0f);
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_TRUE(pattern_->AnimateRunning());
}

/**
 * @tc.name: FastScrollNewCountWaitsForCurrentLayout001
 * @tc.desc: A target added after the previous layout is not rejected by the previous maximum index.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollNewCountWaitsForCurrentLayout001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    const int32_t oldMaximum = pattern_->maxListItemIndex_;
    auto added = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ListItemPattern>(nullptr));
    added->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FILL_LENGTH), CalcLength(ITEM_MAIN_SIZE)));
    added->MountToParent(frameNode_);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    ASSERT_EQ(frameNode_->GetTotalChildCount(), oldMaximum + 2);
    pattern_->ScrollToIndexFastScroll(oldMaximum + 1, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollRequest()->index, oldMaximum + 1);
    EXPECT_FALSE(pattern_->AnimateRunning());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_EQ(pattern_->GetEndIndexInItemPosition(), oldMaximum + 1);
    EXPECT_TRUE(pattern_->IsAtBottom());
}

/**
 * @tc.name: FastScrollPreparedResultWaitsForStartStage001
 * @tc.desc: Data handoff prepares positions; the initialized geometry stage starts the first animation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg, FastScrollPreparedResultWaitsForStartStage001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(true);
    pattern_->UpdateTotalOffset(algorithm, false);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->viewportSize, 0.0f);
    pattern_->TryStartFastScroll();
    ASSERT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->viewportSize, pattern_->contentMainSize_);
    EXPECT_NEAR(pattern_->GetFinalPosition(),
        pattern_->posMap_->GetPositionInfo(60).mainPos + 25.0f, 0.1f);
}

/**
 * @tc.name: FastScrollOffscreenTargetUsesPreparedSample001
 * @tc.desc: Entry does not fill the table; preparation contributes to the mean without replacing measured sizes.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutHandoffTestNg, FastScrollOffscreenTargetUsesPreparedSample001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(true);
    pattern_->posMap_->ClearPosMap();
    pattern_->UpdatePosMap(pattern_->itemPosition_);
    pattern_->ScrollToIndexFastScroll(60, true, ScrollAlign::START, 25.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::NEED_WINDOW);
    EXPECT_TRUE(Negative(pattern_->posMap_->GetPositionInfo(59).mainSize));

    algorithm->GetOrCreateFastScrollState().result.measuredSizes[60] = { 5000.0f };
    const float count = pattern_->itemPosition_.size();
    const float sample = (count * ITEM_MAIN_SIZE + 5000.0f) / (count + 1.0f);
    pattern_->UpdateTotalOffset(algorithm, false);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(59).mainSize, sample);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(0).mainSize, ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainSize, 5000.0f);
    for (int32_t index = 1; index <= 60; ++index) {
        const auto previous = pattern_->posMap_->GetPositionInfo(index - 1);
        EXPECT_NEAR(pattern_->posMap_->GetPositionInfo(index).mainPos,
            previous.mainPos + previous.mainSize, 0.1f);
    }
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_FALSE(pattern_->AnimateRunning());
}

class ListFastScrollPreparedNeighborTestNg : public ListFastScrollLayoutHandoffTestNg,
    public testing::WithParamInterface<int32_t> {};

/**
 * @tc.name: FastScrollPreparedLargeNeighborUsesUpdatedSample001
 * @tc.desc: Ordinary, Group and Lazy whole sizes contribute to missing-item estimates and retain their own sizes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollPreparedNeighborTestNg, FastScrollPreparedLargeNeighborUsesUpdatedSample001, TestSize.Level1)
{
    auto algorithm = CreateHandoffResult(true);
    pattern_->posMap_->ClearPosMap();
    pattern_->UpdatePosMap(pattern_->itemPosition_);
    pattern_->ScrollToIndexFastScroll(60, true, ScrollAlign::START, 25.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    algorithm->GetOrCreateFastScrollState().result.measuredSizes[61] = { 1000000.0f, GetParam() == 1, GetParam() == 2 };
    const float count = pattern_->itemPosition_.size();
    const float sample = (count * ITEM_MAIN_SIZE + 250.0f + 1000000.0f) / (count + 2.0f);
    pattern_->UpdateTotalOffset(algorithm, false);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(59).mainSize, sample);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(0).mainSize, ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(60).mainSize, 250.0f);
    const auto neighbor = pattern_->posMap_->GetPositionInfo(61);
    EXPECT_FLOAT_EQ(neighbor.mainSize, 1000000.0f);
    EXPECT_EQ(neighbor.isGroup, GetParam() == 1);
    EXPECT_EQ(neighbor.isLazyChild, GetParam() == 2);
    EXPECT_FLOAT_EQ(neighbor.mainPos, pattern_->posMap_->GetPositionInfo(60).mainPos + 250.0f);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_FALSE(pattern_->AnimateRunning());
}

INSTANTIATE_TEST_SUITE_P(SectionKind, ListFastScrollPreparedNeighborTestNg, testing::Values(0, 1, 2));

class ListFastScrollKnownStartTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, ScrollAlign, float, FastScrollMode, int32_t>> {};

/**
 * @tc.name: FastScrollKnownSizeStartsWithoutPreparation001
 * @tc.desc: Exact sizes start at entry or the first normal layout, without preparing the target or offset window.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollKnownStartTestNg, FastScrollKnownSizeStartsWithoutPreparation001, TestSize.Level1)
{
    const auto [layouted, align, extraOffset, mode, lanes] = GetParam();
    auto model = CreateList();
    model.SetLanes(lanes);
    model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    CreateListItems(100);
    if (layouted) {
        CreateDone();
    }
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, align, extraOffset, mode);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_EQ(pattern_->AnimateRunning(), layouted);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    if (!layouted) {
        CreateDone();
        EXPECT_TRUE(pattern_->AnimateRunning());
    }
    EXPECT_FALSE(pattern_->GetFastScrollPreparation()->estimated);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    float position = (80 / lanes) * ITEM_MAIN_SIZE + extraOffset;
    if (align == ScrollAlign::END || align == ScrollAlign::AUTO) {
        position += ITEM_MAIN_SIZE - HEIGHT;
    } else if (align == ScrollAlign::CENTER) {
        position += (ITEM_MAIN_SIZE - HEIGHT) / 2.0f;
    }
    EXPECT_NEAR(pattern_->GetTotalOffset(), position, 0.1f);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(80).mainSize, ITEM_MAIN_SIZE);
}

INSTANTIATE_TEST_SUITE_P(ExactTable, ListFastScrollKnownStartTestNg,
    testing::Combine(testing::Bool(),
        testing::Values(ScrollAlign::START, ScrollAlign::END, ScrollAlign::CENTER, ScrollAlign::AUTO),
        testing::Values(0.0f, 25.0f, -25.0f),
        testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE),
        testing::Values(1, 2, 3)));

class ListFastScrollKnownMixedStartTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, ScrollAlign, float, FastScrollMode>> {
protected:
    void CreateKnownMixedList(int32_t lanes)
    {
        if (lanes <= 0) {
            FAIL() << "Lane count must be positive.";
        }
        auto model = CreateList();
        model.SetLanes(lanes);
        std::vector<float> sizes(FAST_SCROLL_ITEM_COUNT, ITEM_MAIN_SIZE);
        for (int32_t index = 0; index < FAST_SCROLL_ITEM_COUNT; ++index) {
            if (index % KNOWN_GROUP_PERIOD != KNOWN_GROUP_REMAINDER) {
                CreateListItems(1);
                continue;
            }
            auto group = CreateListItemGroup();
            group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
            CreateListItems(KNOWN_GROUP_CHILD_COUNT);
            ViewStackProcessor::GetInstance()->Pop();
            ViewStackProcessor::GetInstance()->StopGetAccessRecording();
            sizes[index] = ((KNOWN_GROUP_CHILD_COUNT + lanes - 1) / lanes) * ITEM_MAIN_SIZE;
        }
        model.GetOrCreateListChildrenMainSize()->ChangeData(0, 0, sizes);
        CreateDone();
    }
};

/**
 * @tc.name: FastScrollKnownDirectMixedUsesOriginalTable
 * @tc.desc: Direct Group and ordinary CMS children use the original exact table without Fast preparation.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollKnownMixedStartTestNg, FastScrollKnownDirectMixedUsesOriginalTable, TestSize.Level1)
{
    const auto [lanes, align, extraOffset, mode] = GetParam();
    CreateKnownMixedList(lanes);
    const auto target = pattern_->posMap_->GetPositionInfo(80);
    ASSERT_EQ(GetChildFrameNode(frameNode_, 80)->GetTag(), V2::LIST_ITEM_GROUP_ETS_TAG);
    ASSERT_FALSE(Negative(target.mainSize));
    float expected = target.mainPos + extraOffset;
    if (align == ScrollAlign::END || align == ScrollAlign::AUTO) {
        expected += target.mainSize - HEIGHT;
    } else if (align == ScrollAlign::CENTER) {
        expected += (target.mainSize - HEIGHT) / 2.0f;
    }
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, align, extraOffset, mode);
    ASSERT_TRUE(pattern_->HasFastScrollRequest());
    ASSERT_NE(pattern_->GetFastScrollPreparation(), nullptr);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_TRUE(pattern_->AnimateRunning());
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames());
    EXPECT_NEAR(pattern_->GetTotalOffset(), expected, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), target.mainPos - expected, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(ExactMixedTable, ListFastScrollKnownMixedStartTestNg,
    testing::Combine(testing::Values(1, 2, 3),
        testing::Values(ScrollAlign::START, ScrollAlign::END, ScrollAlign::CENTER, ScrollAlign::AUTO),
        testing::Values(0.0f, 25.0f, -25.0f),
        testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE)));

class MixedFastScrollLazyBuilder final : public ListItemMockLazy {
public:
    explicit MixedFastScrollLazyBuilder(int32_t lanes, bool conditional = false)
        : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE), lanes_(lanes), conditional_(conditional) {}

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        if (index % FIRST_GROUP_PERIOD != FIRST_GROUP_REMAINDER &&
            index % SECOND_GROUP_PERIOD != SECOND_GROUP_REMAINDER) {
            return WrapConditional(ListItemMockLazy::OnGetChildByIndex(index, expiringItems));
        }
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        ListItemGroupModelNG group;
        group.Create({});
        group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(40.0f);
        stack->StopGetAccessRecording();
        for (int32_t child = 0; child < lanes_ * MIXED_GROUP_ROW_COUNT; ++child) {
            stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
            ListItemModelNG item;
            item.Create();
            ViewAbstract::SetHeight(CalcLength(40.0f));
            stack->Pop();
            stack->StopGetAccessRecording();
        }
        return WrapConditional({ std::to_string(index), stack->Finish() });
    }

private:
    std::pair<std::string, RefPtr<UINode>> WrapConditional(std::pair<std::string, RefPtr<UINode>> item) const
    {
        if (conditional_) {
            auto branch = IfElseNode::GetOrCreateIfElseNode(ElementRegister::GetInstance()->MakeUniqueId());
            item.second->MountToParent(branch);
            item.second = branch;
        }
        return item;
    }

    int32_t lanes_;
    bool conditional_;
};

class ListMixedFastScrollTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, int32_t>> {
protected:
    void RunMixedLazyScroll(bool stackFromEnd);

    std::string DescribeMixedWindow() const
    {
        std::ostringstream stream;
        stream << " offset=" << pattern_->GetTotalOffset();
        for (const auto& [index, item] : pattern_->itemPosition_) {
            stream << " [" << index << ":" << item.startPos << "," << item.endPos << "," << item.isGroup << "]";
        }
        for (const auto& [start, end] : pattern_->lanesItemRange_) {
            stream << " rowRange=" << start << "," << end;
        }
        return stream.str();
    }
};

void ListMixedFastScrollTestNg::RunMixedLazyScroll(bool stackFromEnd)
{
    const auto [known, backward, lanes] = GetParam();
    auto model = CreateList();
    model.SetLanes(lanes);
    model.SetStackFromEnd(stackFromEnd);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    model.SetInitialIndex(backward ? MIXED_INITIAL_INDEX : 0);
    if (known) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(80.0f);
    }
    auto builder = AceType::MakeRefPtr<MixedFastScrollLazyBuilder>(lanes, !known);
    if (known) {
        CreateDirectBuilderItems(builder, MIXED_ITEM_COUNT);
    } else {
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazy;
        lazy.Create(RefPtr<LazyForEachBuilder>(builder));
        stack->Pop();
        stack->StopGetAccessRecording();
    }
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(ANIMATION_TICKS);
    pattern_->ScrollToIndexFastScroll(MIXED_TARGET_INDEX, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(MIXED_MAX_ANIMATION_FRAMES));
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_NEAR(pattern_->GetItemRect(MIXED_TARGET_INDEX).Top(), 0.0f, 0.1f);
    for (int32_t step = 0; step < MIXED_ORDINARY_SCROLL_STEPS; ++step) {
        const int32_t anchor = pattern_->GetEndIndex();
        const float previous = pattern_->GetItemRect(anchor).Top();
        SCOPED_TRACE("before" + DescribeMixedWindow());
        pattern_->UpdateCurrentOffset(-MIXED_SCROLL_STEP, SCROLL_FROM_UPDATE);
        FlushUITasks();
        EXPECT_NEAR(pattern_->GetItemRect(anchor).Top(), previous - MIXED_SCROLL_STEP, 0.1f)
            << "step=" << step << " after" << DescribeMixedWindow();
        if (step < INITIAL_SCROLL_SAMPLE_COUNT) {
            EXPECT_NEAR(pattern_->GetItemRect(MIXED_TARGET_INDEX).Top(), -(step + 1) * MIXED_SCROLL_STEP, 0.1f);
        }
    }
}

/**
 * @tc.name: FastScrollMixedLazyCompletesAndContinues
 * @tc.desc: Mixed lazy rows finish SKIP_MEASURE and remain scrollable across subsequent Group boundaries.
 * @tc.type: FUNC
 */
HWTEST_P(ListMixedFastScrollTestNg, FastScrollMixedLazyCompletesAndContinues, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(RunMixedLazyScroll(false));
}

INSTANTIATE_TEST_SUITE_P(KnownAndEstimated, ListMixedFastScrollTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Values(2, 3, 4)));

class ListMixedFastScrollStackTestNg : public ListMixedFastScrollTestNg {};

/**
 * @tc.name: FastScrollMixedLazyStackCompletesAndContinues
 * @tc.desc: Stack-from-end mixed lazy rows also retain confirmed boundaries after FastScroll stops.
 * @tc.type: FUNC
 */
HWTEST_P(ListMixedFastScrollStackTestNg, FastScrollMixedLazyStackCompletesAndContinues, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(RunMixedLazyScroll(true));
}

INSTANTIATE_TEST_SUITE_P(KnownAndEstimated, ListMixedFastScrollStackTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Values(2, 3, 4)));

class ListFastScrollKnownLegacyMixedTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<FastScrollMode, bool>> {};

/**
 * @tc.name: LegacyKnownDirectMixedStackAlignmentControl001
 * @tc.desc: Compare the original controller with the same direct CMS mixed rows and initial window.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, LegacyKnownDirectMixedStackAlignmentControl001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(4);
    model.SetStackFromEnd(true);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    model.SetInitialIndex(150);
    model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(80.0f);
    CreateDirectBuilderItems(AceType::MakeRefPtr<MixedFastScrollLazyBuilder>(4), 360);
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(ANIMATION_TICKS);
    pattern_->ScrollToIndex(144, true, ScrollAlign::START);
    FlushUITasks();
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(MIXED_MAX_ANIMATION_FRAMES));
    EXPECT_NEAR(pattern_->GetItemRect(MIXED_TARGET_INDEX).Top(), 0.0f, 0.1f);
    for (int32_t step = 0; step < MIXED_ORDINARY_SCROLL_STEPS; ++step) {
        const int32_t anchor = pattern_->GetEndIndex();
        const float previous = pattern_->GetItemRect(anchor).Top();
        pattern_->UpdateCurrentOffset(-30.0f, SCROLL_FROM_UPDATE);
        FlushUITasks();
        EXPECT_NEAR(pattern_->GetItemRect(anchor).Top(), previous - 30.0f, 0.1f) << "step=" << step;
    }
}

/**
 * @tc.name: FastScrollKnownMixedLazyTerminatesWithoutRebuildingTable
 * @tc.desc: Legacy homogeneous CMS inference may be inaccurate for mixed Lazy, but must not cause endless retargeting.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollKnownLegacyMixedTestNg,
    FastScrollKnownMixedLazyTerminatesWithoutRebuildingTable, TestSize.Level1)
{
    const auto [mode, backward] = GetParam();
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(backward ? 150 : 0);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(80.0f);
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<MixedFastScrollLazyBuilder>(2)));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    const auto target = pattern_->posMap_->GetPositionInfo(144);
    int32_t starts = 0;
    int32_t stops = 0;
    eventHub_->SetOnScrollStart([&starts]() { ++starts; });
    eventHub_->SetOnScrollStop([&stops]() { ++stops; });
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(144, true, ScrollAlign::START, 0.0f, mode);
    ASSERT_NE(pattern_->GetFastScrollPreparation(), nullptr);
    EXPECT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(64));
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(144).mainPos, target.mainPos);
    EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(144).mainSize, target.mainSize);
    eventHub_->SetOnScrollStart(nullptr);
    eventHub_->SetOnScrollStop(nullptr);
}

INSTANTIATE_TEST_SUITE_P(LegacyCMSBoundary, ListFastScrollKnownLegacyMixedTestNg,
    testing::Combine(testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE),
        testing::Bool()));

/**
 * @tc.name: FastScrollKnownRowMismatchDoesNotRepairOriginalTable001
 * @tc.desc: Detect incompatible visible rows without changing the CMS table or affecting
 * unknown-size/single-lane paths.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollKnownRowMismatchDoesNotRepairOriginalTable001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    CreateListItems(4);
    CreateDone();
    ASSERT_EQ(pattern_->itemPosition_.size(), 4u);
    EXPECT_FALSE(pattern_->HasKnownFastScrollRowMismatch());
    const auto positions = pattern_->posMap_->posMap_;
    pattern_->itemPosition_.at(3).startPos += ITEM_MAIN_SIZE;
    pattern_->itemPosition_.at(3).endPos += ITEM_MAIN_SIZE;
    EXPECT_TRUE(pattern_->HasKnownFastScrollRowMismatch());
    EXPECT_EQ(pattern_->posMap_->posMap_.size(), positions.size());
    for (const auto& [index, expected] : positions) {
        const auto actual = pattern_->posMap_->GetPositionInfo(index);
        EXPECT_FLOAT_EQ(actual.mainPos, expected.mainPos);
        EXPECT_FLOAT_EQ(actual.mainSize, expected.mainSize);
        EXPECT_EQ(actual.isGroup, expected.isGroup);
    }
    pattern_->lanes_ = 1;
    EXPECT_FALSE(pattern_->HasKnownFastScrollRowMismatch());
    pattern_->lanes_ = 2;
    pattern_->childrenSize_.Reset();
    EXPECT_FALSE(pattern_->HasKnownFastScrollRowMismatch());
}

class FallingMixedLazyBuilder final : public ListItemMockLazy {
public:
    FallingMixedLazyBuilder() : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE) {}

    static bool IsGroup(int32_t index)
    {
        return index % FIRST_GROUP_PERIOD == FIRST_GROUP_REMAINDER ||
            index % SECOND_GROUP_PERIOD == SECOND_GROUP_REMAINDER;
    }

    static float BaseSize(int32_t index)
    {
        constexpr float sizes[] = { 320.0f, 180.0f, 48.0f, 24.0f };
        return sizes[index / HEIGHT_SECTION_COUNT];
    }

    static float MainSize(int32_t index)
    {
        if (!IsGroup(index)) {
            return BaseSize(index) + index % ITEM_HEIGHT_PERIOD * ITEM_HEIGHT_STEP;
        }
        const int32_t count = FALLING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD;
        return (count / MIXED_THREE_LANES) * (BaseSize(index) + MAX_GROUP_HEIGHT_STEP) +
            (count % MIXED_THREE_LANES ? BaseSize(index) + (count % MIXED_THREE_LANES - 1) * GROUP_HEIGHT_STEP : 0.0f);
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        if (!IsGroup(index)) {
            auto result = ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
            auto node = AceType::DynamicCast<FrameNode>(result.second);
            node->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(std::nullopt, CalcLength(MainSize(index))));
            return result;
        }
        auto stack = ViewStackProcessor::GetInstance();
        ListItemGroupModelNG group;
        group.Create({});
        auto childrenSize = group.GetOrCreateListChildrenMainSize();
        childrenSize->UpdateDefaultSize(0.0f);
        std::vector<float> sizes;
        for (int32_t child = 0; child < FALLING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD; ++child) {
            sizes.emplace_back(BaseSize(index) + child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP);
            ListItemModelNG item;
            item.Create();
            ViewAbstract::SetHeight(CalcLength(sizes.back()));
            stack->Pop();
        }
        childrenSize->ChangeData(0, sizes.size(), sizes);
        return { std::to_string(index), stack->Finish() };
    }
};

class ListMixedFastScrollOffsetTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<ScrollAlign> {
protected:
    static std::vector<float> ExpectedTops()
    {
        std::vector<float> tops(MIXED_ITEM_COUNT);
        float position = 0.0f;
        for (int32_t index = 0; index < MIXED_ITEM_COUNT;) {
            const int32_t start = index++;
            if (!FallingMixedLazyBuilder::IsGroup(start)) {
                while (index < MIXED_ITEM_COUNT && index - start < MIXED_THREE_LANES &&
                    !FallingMixedLazyBuilder::IsGroup(index)) {
                    ++index;
                }
            }
            float height = 0.0f;
            for (int32_t item = start; item < index; ++item) {
                tops[item] = position;
                height = std::max(height, FallingMixedLazyBuilder::MainSize(item));
            }
            position += height;
        }
        return tops;
    }
};

/**
 * @tc.name: FastScrollMixedKnownOffsetBeyondTarget
 * @tc.desc: Positive extraOffset beyond a confirmed target retains exact mixed rows and one start/stop pair.
 * @tc.type: FUNC
 */
HWTEST_P(ListMixedFastScrollOffsetTestNg, FastScrollMixedKnownOffsetBeyondTarget, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(3);
    model.SetInitialIndex(30);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    std::vector<float> sizes;
    for (int32_t index = 0; index < 360; ++index) {
        sizes.emplace_back(FallingMixedLazyBuilder::MainSize(index));
    }
    model.GetOrCreateListChildrenMainSize()->ChangeData(0, sizes.size(), sizes);
    CreateDirectBuilderItems(AceType::MakeRefPtr<FallingMixedLazyBuilder>(), 360);
    CreateDone();
    int32_t starts = 0;
    int32_t stops = 0;
    eventHub_->SetOnScrollStart([&starts]() { ++starts; });
    eventHub_->SetOnScrollStop([&stops]() { ++stops; });
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(286, true, GetParam(), 1600.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(64));
    const auto tops = ExpectedTops();
    const float adjustment = GetParam() == ScrollAlign::CENTER ? (40.0f - HEIGHT) / 2.0f : 40.0f - HEIGHT;
    const float offset = tops[286] + adjustment + 1600.0f;
    ASSERT_FALSE(pattern_->itemPosition_.empty());
    for (const auto& [index, item] : pattern_->itemPosition_) {
        EXPECT_NEAR(item.startPos, tops[index] - offset, 0.1f) << "index=" << index;
    }
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
}

INSTANTIATE_TEST_SUITE_P(KnownMixed, ListMixedFastScrollOffsetTestNg,
    testing::Values(ScrollAlign::CENTER, ScrollAlign::END));

class RisingMixedCallbackBuilder final : public ListItemMockLazy {
public:
    RisingMixedCallbackBuilder() : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE) {}

    static float BaseSize(int32_t index)
    {
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        return sizes[index / HEIGHT_SECTION_COUNT];
    }

    static float MainSize(int32_t index)
    {
        if (!FallingMixedLazyBuilder::IsGroup(index)) {
            return BaseSize(index) + index % ITEM_HEIGHT_PERIOD * ITEM_HEIGHT_STEP;
        }
        float height = 0.0f;
        const int32_t count = RISING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD;
        for (int32_t child = 0; child < count; child += MIXED_TWO_LANES) {
            const float first = BaseSize(index) + child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP;
            const float second = child + 1 < count ?
                BaseSize(index) + (child + 1) % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP : 0.0f;
            height += std::max(first, second);
        }
        return height;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        if (!FallingMixedLazyBuilder::IsGroup(index)) {
            auto result = ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
            AceType::DynamicCast<FrameNode>(result.second)->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(std::nullopt, CalcLength(MainSize(index))));
            return WithConditionalBranch(std::move(result));
        }
        auto stack = ViewStackProcessor::GetInstance();
        ListItemGroupModelNG group;
        group.Create({});
        auto childrenSize = group.GetOrCreateListChildrenMainSize();
        childrenSize->UpdateDefaultSize(0.0f);
        std::vector<float> sizes;
        for (int32_t child = 0; child < RISING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD; ++child) {
            sizes.emplace_back(BaseSize(index) + child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP);
            ListItemModelNG item;
            item.Create();
            ViewAbstract::SetHeight(CalcLength(sizes.back()));
            stack->Pop();
        }
        childrenSize->ChangeData(0, sizes.size(), sizes);
        return WithConditionalBranch({ std::to_string(index), stack->Finish() });
    }
};

/**
 * @tc.name: FastScrollMixedKnownCallbackMatchesVisibleMovement001
 * @tc.desc: Correcting confirmed row coordinates must not enter the physical onDidScroll displacement.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollMixedKnownCallbackMatchesVisibleMovement001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(300);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    std::vector<float> sizes;
    for (int32_t index = 0; index < 360; ++index) {
        sizes.emplace_back(RisingMixedCallbackBuilder::MainSize(index));
    }
    model.GetOrCreateListChildrenMainSize()->ChangeData(0, sizes.size(), sizes);
    CreateDirectBuilderItems(AceType::MakeRefPtr<RisingMixedCallbackBuilder>(), 360);
    CreateDone();
    auto previous = std::make_shared<std::map<int32_t, float>>();
    auto comparisons = std::make_shared<int32_t>(0);
    eventHub_->SetOnDidScroll([this, previous, comparisons](Dimension offset, ScrollState) {
        std::map<int32_t, float> current;
        for (const auto& [index, item] : pattern_->itemPosition_) {
            if (item.isGroup || !Positive(item.endPos) || GreatOrEqual(item.startPos, HEIGHT)) {
                continue;
            }
            current[index] = item.startPos;
            if (const auto iter = previous->find(index); iter != previous->end()) {
                ++*comparisons;
                EXPECT_NEAR(offset.ConvertToPx(), iter->second - item.startPos, 0.1f) << "index=" << index;
            }
        }
        *previous = std::move(current);
    });
    MockAnimationManager::GetInstance().SetTicks(64);
    pattern_->ScrollToIndexFastScroll(70, true, ScrollAlign::END, -24.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    EXPECT_GT(*comparisons, 20);
    eventHub_->SetOnDidScroll(nullptr);
}

class RisingGroupLazyBuilder final : public ListItemMockLazy {
public:
    RisingGroupLazyBuilder() : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE) {}

    static float ChildTop(int32_t index, int32_t child)
    {
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        float top = 0.0f;
        for (int32_t first = 0; first < child / MIXED_TWO_LANES * MIXED_TWO_LANES; first += MIXED_TWO_LANES) {
            top += sizes[index / HEIGHT_SECTION_COUNT] +
                std::max(first % GROUP_HEIGHT_PERIOD, (first + 1) % GROUP_HEIGHT_PERIOD) * GROUP_HEIGHT_STEP;
        }
        return top;
    }

    static float MainSize(int32_t index)
    {
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        const float base = sizes[index / HEIGHT_SECTION_COUNT];
        float height = 0.0f;
        const int32_t count = RISING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD;
        for (int32_t child = 0; child < count; child += MIXED_TWO_LANES) {
            const float first = base + child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP;
            const float second = child + 1 < count ?
                base + (child + 1) % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP : 0.0f;
            height += std::max(first, second);
        }
        return height;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        auto stack = ViewStackProcessor::GetInstance();
        ListItemGroupModelNG group;
        group.Create({});
        for (int32_t child = 0; child < RISING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD; ++child) {
            ListItemModelNG item;
            item.Create();
            ViewAbstract::SetHeight(CalcLength(sizes[index / HEIGHT_SECTION_COUNT] +
                child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP));
            stack->Pop();
        }
        return { std::to_string(index), stack->Finish() };
    }
};

class ListGroupFastScrollOffsetTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<FastScrollMode, int32_t>> {
protected:
    std::string DescribeOffsetPositions() const
    {
        std::ostringstream trace;
        trace << "\noffset=" << pattern_->GetTotalOffset() << " endpoint=" << pattern_->GetFinalPosition();
        for (int32_t index = TRACE_START_INDEX; index < TRACE_END_INDEX; ++index) {
            const auto info = pattern_->posMap_->GetPositionInfo(index);
            trace << " [" << index << ":" << info.mainPos << "," << info.mainSize << "]";
            if (const auto item = pattern_->itemPosition_.find(index); item != pattern_->itemPosition_.end()) {
                trace << " visible=" << item->second.startPos << ':' << item->second.endPos;
                if (item->second.groupInfo) {
                    trace << ':' << item->second.groupInfo->atStart << ':' << item->second.groupInfo->atEnd;
                }
            }
        }
        return trace.str();
    }

    void AdvanceRecordedFrames()
    {
        FlushUITasks();
        std::string trace = DescribeOffsetPositions();
        for (float delta : { -160.4844f, -153.7865f, -396.1224f, -630.5521f }) {
            MockAnimationManager::GetInstance().TickByVelocity(delta);
            FlushUITasks();
            trace += DescribeOffsetPositions();
        }
        RecordProperty("groupOffsetPreparationTrace", trace);
    }
};

/**
 * @tc.name: FastScrollVisibleGroupEndWithNegativeOffset
 * @tc.desc: Preparing a visible uneven Group preserves its real tail before scrolling to preceding Groups.
 * @tc.type: FUNC
 */
HWTEST_P(ListGroupFastScrollOffsetTestNg, FastScrollVisibleGroupEndWithNegativeOffset, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(150);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<RisingGroupLazyBuilder>()));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    std::vector<float> tops(361);
    for (int32_t index = 0; index < 360; ++index) {
        tops[index + 1] = tops[index] + RisingGroupLazyBuilder::MainSize(index);
    }
    ASSERT_EQ(pattern_->GetStartIndex(), 150);
    EXPECT_NEAR(pattern_->GetItemRect(151).Height(), 204.0f, 0.1f);
    const auto [mode, ticks] = GetParam();
    MockAnimationManager::GetInstance().SetTicks(ticks > 0 ? ticks : 32);
    pattern_->ScrollToIndexFastScroll(151, true, ScrollAlign::END, -1600.0f, mode);
    if (ticks == 0) {
        AdvanceRecordedFrames();
    }
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    RecordProperty("groupOffsetFinalPositions", DescribeOffsetPositions());
    const float offset = tops[152] - HEIGHT - 1600.0f;
    ASSERT_FALSE(pattern_->itemPosition_.empty());
    for (const auto& [index, item] : pattern_->itemPosition_) {
        const auto group = GetChildPattern<ListItemGroupPattern>(frameNode_, index);
        ASSERT_NE(group, nullptr);
        ASSERT_FALSE(group->GetItemPosition().empty());
        for (const auto& [child, position] : group->GetItemPosition()) {
            const auto rect = pattern_->GetItemRectInGroup(index, child);
            EXPECT_NEAR(rect.Top(), tops[index] + RisingGroupLazyBuilder::ChildTop(index, child) - offset, 0.1f)
                << "index=" << index << " child=" << child;
        }
    }
    EXPECT_FALSE(pattern_->HasFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
}

INSTANTIATE_TEST_SUITE_P(EstimatedGroups, ListGroupFastScrollOffsetTestNg,
    testing::Combine(testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE),
        testing::Values(0, 4, 12, 32)));

class RisingItemLazyBuilder final : public ListItemMockLazy {
public:
    RisingItemLazyBuilder() : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE) {}

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        auto result = ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
        auto node = AceType::DynamicCast<FrameNode>(result.second);
        node->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(std::nullopt, CalcLength(sizes[index / HEIGHT_SECTION_COUNT] +
                index % ITEM_HEIGHT_PERIOD * ITEM_HEIGHT_STEP)));
        return result;
    }
};

class ListMixedFastScrollBoundaryTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<FastScrollMode> {};

class ListFastScrollIdleLanesTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<FastScrollMode> {};

class ListMixedFastScrollOrdinaryTestNg : public ListFastScrollerTestNg,
    public testing::WithParamInterface<std::tuple<FastScrollMode, bool>> {};

/**
 * @tc.name: CompletedMixedScrollKeepsHeadRowsAligned
 * @tc.desc: Legacy jumps after a completed mixed request must not split the head row's geometry.
 * @tc.type: FUNC
 */
HWTEST_P(ListMixedFastScrollOrdinaryTestNg, CompletedMixedScrollKeepsHeadRowsAligned, TestSize.Level1)
{
    const auto [mode, backward] = GetParam();
    auto model = CreateList();
    ViewAbstract::SetHeight(CalcLength(400.0f));
    model.SetLanes(2);
    model.SetInitialIndex(backward ? 300 : 30);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<RisingMixedCallbackBuilder>()));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(32);
    pattern_->ScrollToIndexFastScroll(backward ? 70 : 286, true, ScrollAlign::END, -24.0f, mode);
    FlushUITasks();
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    for (int32_t step = 0; backward && step < 8; ++step) {
        const auto anchor = std::find_if(pattern_->itemPosition_.begin(), pattern_->itemPosition_.end(),
            [](const auto& item) { return !item.second.isGroup && Positive(item.second.endPos); });
        ASSERT_NE(anchor, pattern_->itemPosition_.end());
        const int32_t index = anchor->first;
        const float previous = pattern_->GetItemRect(index).Top();
        pattern_->UpdateCurrentOffset(30.0f, SCROLL_FROM_UPDATE);
        FlushUITasks();
        EXPECT_NEAR(pattern_->GetItemRect(index).Top(), previous + 30.0f, 0.1f) << "step=" << step;
    }
    pattern_->ScrollToIndex(110, false, ScrollAlign::CENTER);
    FlushUITasks();
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetItemRect(0).Top(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(1).Top(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(2).Top(), 32.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(3).Top(), 72.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(0).Left(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(1).Left(), WIDTH / 2.0f, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(OrdinaryAfterMixed, ListMixedFastScrollOrdinaryTestNg,
    testing::Combine(testing::Values(FastScrollMode::NONE, FastScrollMode::SKIP_MEASURE), testing::Bool()));

/**
 * @tc.name: IdleLaneChangeRebuildsHomogeneousRows
 * @tc.desc: Completed requests must not retain an old row phase across idle lane changes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollIdleLanesTestNg, IdleLaneChangeRebuildsHomogeneousRows, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(30);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<RisingItemLazyBuilder>()));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(32);
    pattern_->ScrollToIndexFastScroll(286, true, ScrollAlign::END, -24.0f, GetParam());
    FlushUITasks();
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    pattern_->ScrollToIndex(155, false, ScrollAlign::CENTER);
    FlushUITasks();
    for (int32_t lanes : { 3, 1, 4, 2 }) {
        SCOPED_TRACE(lanes);
        layoutProperty_->UpdateLanes(lanes);
        FlushUITasks();
        ASSERT_FALSE(pattern_->itemPosition_.empty());
        EXPECT_EQ(pattern_->itemPosition_.begin()->first % lanes, 0);
        const int32_t target = lanes == 3 ? 70 : 280;
        pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::END, 24.0f, GetParam());
        FlushUITasks();
        ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
        const int32_t row = target - target % lanes;
        float height = 0.0f;
        constexpr float sizes[] = { 24.0f, 48.0f, 180.0f, 320.0f };
        for (int32_t index = row; index < row + lanes; ++index) {
            height = std::max(height, sizes[index / 90] + index % 3 * 8.0f);
        }
        EXPECT_NEAR(pattern_->GetItemRect(target).Bottom(), HEIGHT - 24.0f - height +
            sizes[target / 90] + target % 3 * 8.0f, 0.1f);
        pattern_->ScrollToIndex(155, false, ScrollAlign::CENTER);
        FlushUITasks();
    }
}

INSTANTIATE_TEST_SUITE_P(IdleLanes, ListFastScrollIdleLanesTestNg,
    testing::Values(FastScrollMode::NONE, FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE));

/**
 * @tc.name: FastScrollSkipToHeadClampsVisibleWindow
 * @tc.desc: An estimated skip beyond the head clamps the same frame and reports a visible center index.
 * @tc.type: FUNC
 */
HWTEST_P(ListMixedFastScrollBoundaryTestNg, FastScrollSkipToHeadClampsVisibleWindow, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(325);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    int32_t indexEvents = 0;
    model.SetOnScrollIndex([&indexEvents](int32_t start, int32_t end, int32_t center) {
        ++indexEvents;
        EXPECT_GE(center, start);
        EXPECT_LE(center, end);
    });
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<RisingItemLazyBuilder>()));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(32);
    pattern_->ScrollToIndexFastScroll(70, true, ScrollAlign::END, -1600.0f, GetParam());
    FlushUITasks();
    for (float delta : { -156.5f, -144.3984f, -550.0f }) {
        MockAnimationManager::GetInstance().TickByVelocity(delta);
        FlushUITasks();
        if (pattern_->GetStartIndex() == 0) {
            EXPECT_LE(pattern_->GetItemRect(0).Top(), 0.1f);
        }
    }
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    EXPECT_GT(indexEvents, 0);
    EXPECT_NEAR(pattern_->GetItemRect(0).Top(), 0.0f, 0.1f);
}

INSTANTIATE_TEST_SUITE_P(EstimatedBoundary, ListMixedFastScrollBoundaryTestNg,
    testing::Values(FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE));

/**
 * @tc.name: FastScrollSkipHeadWindowWithPositiveAnchor001
 * @tc.desc: A skip resolving before row zero must not leave a blank head or an unfilled viewport.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollSkipHeadWindowWithPositiveAnchor001, TestSize.Level1)
{
    CreateList();
    CreateListItems(20);
    CreateDone();
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    algorithm->Measure(AceType::RawPtr(frameNode_));
    algorithm->currentOffset_ = 0.0f;
    algorithm->startMainPos_ = 0.0f;
    algorithm->endMainPos_ = HEIGHT;
    algorithm->canOverScrollStart_ = false;
    algorithm->GetOrCreateFastScrollState().skipStart = { 0, 244.0f };
    algorithm->MeasureFastScrollSkipWindow(AceType::RawPtr(frameNode_));
    ASSERT_FALSE(algorithm->itemPosition_.empty());
    EXPECT_NEAR(algorithm->GetStartPosition() - algorithm->currentOffset_, 0.0f, 0.1f);
    EXPECT_GE(algorithm->GetEndPosition() - algorithm->currentOffset_, HEIGHT);
}

class FallingTwoLaneMixedBuilder final : public ListItemMockLazy {
public:
    FallingTwoLaneMixedBuilder() : ListItemMockLazy(MIXED_ITEM_COUNT, MIXED_DEFAULT_SIZE) {}

    static float MainSize(int32_t index)
    {
        constexpr float sizes[] = { 320.0f, 180.0f, 48.0f, 24.0f };
        const float base = sizes[index / 90];
        if (!FallingMixedLazyBuilder::IsGroup(index)) {
            return base + index % ITEM_HEIGHT_PERIOD * ITEM_HEIGHT_STEP;
        }
        float size = 0.0f;
        const int32_t count = 5 + index % 3;
        for (int32_t child = 0; child < count; child += MIXED_TWO_LANES) {
            const float first = base + child % 3 * 12.0f;
            const float second = child + 1 < count ? base + (child + 1) % 3 * 12.0f : 0.0f;
            size += std::max(first, second);
        }
        return size;
    }

    static std::vector<std::pair<float, float>> ExpectedRows()
    {
        std::vector<std::pair<float, float>> rows(MIXED_ITEM_COUNT);
        float position = 0.0f;
        for (int32_t index = 0; index < MIXED_ITEM_COUNT;) {
            const int32_t first = index++;
            if (!FallingMixedLazyBuilder::IsGroup(first) && index < MIXED_ITEM_COUNT &&
                !FallingMixedLazyBuilder::IsGroup(index)) {
                ++index;
            }
            float height = MainSize(first);
            for (int32_t item = first + 1; item < index; ++item) {
                height = std::max(height, MainSize(item));
            }
            for (int32_t item = first; item < index; ++item) {
                rows[item] = { position, height };
            }
            position += height;
        }
        return rows;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        constexpr float sizes[] = { 320.0f, 180.0f, 48.0f, 24.0f };
        const float base = sizes[index / 90];
        if (!FallingMixedLazyBuilder::IsGroup(index)) {
            auto result = ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
            auto node = AceType::DynamicCast<FrameNode>(result.second);
            node->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(std::nullopt, CalcLength(base + index % ITEM_HEIGHT_PERIOD * ITEM_HEIGHT_STEP)));
            return WithConditionalBranch(std::move(result));
        }
        auto stack = ViewStackProcessor::GetInstance();
        ListItemGroupModelNG group;
        group.Create({});
        for (int32_t child = 0; child < RISING_GROUP_MIN_CHILDREN + index % GROUP_CHILD_COUNT_PERIOD; ++child) {
            ListItemModelNG item;
            item.Create();
            ViewAbstract::SetHeight(CalcLength(base + child % GROUP_HEIGHT_PERIOD * GROUP_HEIGHT_STEP));
            stack->Pop();
        }
        return WithConditionalBranch({ std::to_string(index), stack->Finish() });
    }
};

/**
 * @tc.name: FastScrollMixedUnknownPreparationRowContinuesBackward001
 * @tc.desc: Backfilling a viewport continues confirmed preparation rows instead of splitting a normal row.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollMixedUnknownPreparationRowContinuesBackward001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(2);
    model.SetInitialIndex(150);
    model.SetEdgeEffect(EdgeEffect::NONE, false);
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(AceType::MakeRefPtr<FallingTwoLaneMixedBuilder>()));
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    MockAnimationManager::GetInstance().SetTicks(32);
    pattern_->ScrollToIndexFastScroll(148, true, ScrollAlign::END, -1600.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_NO_FATAL_FAILURE(RunFastScrollFrames(256));
    // Group127 starts segment128; Group139 leaves item138 in the preceding incomplete row.
    const auto rows = FallingTwoLaneMixedBuilder::ExpectedRows();
    const float expectedOffset = rows[148].first + rows[148].second - HEIGHT - 1600.0f;
    for (int32_t index = 136; index <= 138; ++index) {
        EXPECT_NEAR(pattern_->GetItemRect(index).Top(), rows[index].first - expectedOffset, 0.1f);
    }
    EXPECT_NEAR(pattern_->GetItemRect(136).Left(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRect(137).Left(), WIDTH / 2.0f, 0.1f);
}

/**
 * @tc.name: FastScrollSkipBoundaryHonorsStackAndOverscroll001
 * @tc.desc: Skip-window clamping respects reverse layout and does not suppress explicitly permitted overscroll.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollerTestNg, FastScrollSkipBoundaryHonorsStackAndOverscroll001, TestSize.Level1)
{
    CreateList();
    CreateListItems(20);
    CreateDone();
    for (bool stack : { false, true }) {
        for (bool overscroll : { false, true }) {
            SCOPED_TRACE(testing::Message() << "stack=" << stack << ", overscroll=" << overscroll);
            auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
            ASSERT_NE(algorithm, nullptr);
            algorithm->Measure(AceType::RawPtr(frameNode_));
            algorithm->currentOffset_ = 0.0f;
            algorithm->startMainPos_ = 0.0f;
            algorithm->endMainPos_ = HEIGHT;
            algorithm->isStackFromEnd_ = stack;
            algorithm->canOverScrollStart_ = overscroll;
            algorithm->canOverScrollEnd_ = overscroll;
            algorithm->GetOrCreateFastScrollState().skipStart = { 0, 244.0f };
            algorithm->MeasureFastScrollSkipWindow(AceType::RawPtr(frameNode_));
            ASSERT_FALSE(algorithm->itemPosition_.empty());
            const float boundary = stack ? algorithm->GetEndPosition() : algorithm->GetStartPosition();
            const float expected = stack ? HEIGHT - (overscroll ? 244.0f : 0.0f) :
                (overscroll ? 244.0f : 0.0f);
            EXPECT_NEAR(boundary, expected, 0.1f);
        }
    }
}

/**
 * @tc.name: ScrollToIndexJumpSourceKept
 * @tc.desc: Test non-smooth ScrollToIndex keeps JUMP scroll source for jump semantics.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollerTestNg, ScrollToIndexJumpSourceKept, TestSize.Level1)
{
    CreateList();
    CreateListItems(5);
    CreateDone();

    /**
     * @tc.steps: step1. ScrollToIndex without smooth animation.
     * @tc.expected: The scroll source is SCROLL_FROM_JUMP.
     */
    pattern_->ScrollToIndex(3, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->GetScrollSource(), SCROLL_FROM_JUMP);
}
} // namespace OHOS::Ace::NG

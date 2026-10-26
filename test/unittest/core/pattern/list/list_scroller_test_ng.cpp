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

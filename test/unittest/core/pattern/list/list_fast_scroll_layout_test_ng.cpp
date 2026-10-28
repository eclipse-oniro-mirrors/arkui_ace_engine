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

#include "list_test_ng.h"
#include "core/components_ng/pattern/lazy_layout/prepared_layout_geometry.h"
#include "base/utils/time_util.h"
#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_model.h"
#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_pattern.h"
#include "core/components_ng/pattern/list/list_item_group_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t HEIGHT_SECTION_COUNT = 2;
constexpr int32_t OUTER_ITEM_COUNT = 240;
constexpr int32_t OUTER_TARGET_INDEX = 80;
constexpr int32_t PRECEDING_TALL_INDEX = 79;
constexpr int32_t FOLLOWING_TALL_INDEX = 81;
constexpr float OUTER_TALL_SIZE = 5000.0f;
constexpr int32_t TRAILING_ITEM_COUNT = 5;
constexpr int32_t OVERLAP_CONSTRAINT_CHANGED = 2;
constexpr int32_t OVERLAP_LAYOUT_INCOMPLETE = 3;
constexpr int32_t OVERLAP_EXPAND_SAFE_AREA = 4;
constexpr int32_t OVERLAP_GROUP_ITEM = 5;
constexpr int32_t OVERLAP_LAZY_ITEM = 6;
constexpr int32_t OVERLAP_POSITION_MISSING = 7;
constexpr int32_t OVERLAP_NODE_DETACHED = 8;
constexpr int32_t OVERLAP_DEFERRED_MEASURE = 9;
constexpr int32_t HUGE_LAZY_ITEM_COUNT = 3000;
constexpr int32_t ANCHOR_GRID_INDEX = 9;
constexpr int32_t ANCHOR_ITEM_COUNT = 10;
constexpr int32_t ANCHOR_INNER_INDEX = 7;
constexpr int32_t PREMEASURE_ANIMATION_TICKS = 4;
constexpr int32_t SUFFIX_START_INDEX = 900;
constexpr int32_t MIDDLE_START_INDEX = 100;
} // namespace

namespace {
constexpr int32_t INNER_ITEM_COUNT = 1000;
constexpr float TALL_ITEM_SIZE = 500.0f;

class FastScrollCountingWrapper final : public LayoutWrapperNode {
    DECLARE_ACE_TYPE(FastScrollCountingWrapper, LayoutWrapperNode);
public:
    explicit FastScrollCountingWrapper(const RefPtr<FrameNode>& host)
        : LayoutWrapperNode(host, host->GetGeometryNode(), host->GetLayoutProperty()) {}

    RefPtr<LayoutWrapper> GetOrCreateChildByIndex(uint32_t index, bool addToRenderTree = true,
        bool isCache = false, bool allowPlaceholderSubstitution = false) override
    {
        ++requests;
        requestedIndices.push_back(index);
        return GetHostNode()->GetOrCreateChildByIndex(index, addToRenderTree, isCache, allowPlaceholderSubstitution);
    }

    int32_t requests = 0;
    std::vector<uint32_t> requestedIndices;
};

enum class GroupDeferredPart { BODY, HEADER, FOOTER };

struct GroupDeferredMeasureState {
    bool deferred = false;
    int32_t attempts = 0;
};

class GroupDeferredMeasureAlgorithm final : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(GroupDeferredMeasureAlgorithm, BoxLayoutAlgorithm);
public:
    explicit GroupDeferredMeasureAlgorithm(const std::shared_ptr<GroupDeferredMeasureState>& state) : state_(state) {}

    void Measure(LayoutWrapper* wrapper) override
    {
        if (state_->deferred) {
            ++state_->attempts;
        }
        BoxLayoutAlgorithm::Measure(wrapper);
    }

    bool MeasureInNextFrame() const override
    {
        return state_->deferred;
    }

private:
    std::shared_ptr<GroupDeferredMeasureState> state_;
};

class GroupDeferredMeasurePattern final : public ListItemPattern {
    DECLARE_ACE_TYPE(GroupDeferredMeasurePattern, ListItemPattern);
public:
    explicit GroupDeferredMeasurePattern(const std::shared_ptr<GroupDeferredMeasureState>& state)
        : ListItemPattern(nullptr), state_(state) {}

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return AceType::MakeRefPtr<GroupDeferredMeasureAlgorithm>(state_);
    }

private:
    std::shared_ptr<GroupDeferredMeasureState> state_;
};

class FastScrollInnerBuilder final : public ListItemMockLazy {
public:
    explicit FastScrollInnerBuilder(bool uneven = false, Axis axis = Axis::VERTICAL)
        : ListItemMockLazy(INNER_ITEM_COUNT, ITEM_MAIN_SIZE), uneven_(uneven), axis_(axis) {}

    const std::set<int32_t>& GetCreatedIndices() const
    {
        return createdIndices_;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        createdIndices_.insert(index);
        if (!uneven_) {
            return ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
        }
        constexpr float heights[] = { 200.0f, 600.0f, 300.0f, 500.0f };
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        ListItemModelNG model;
        model.Create();
        const CalcLength size(index < INNER_ITEM_COUNT / 2 ? ITEM_MAIN_SIZE : heights[index % 4]);
        ViewAbstract::SetWidth(axis_ == Axis::VERTICAL ? CalcLength(FILL_LENGTH) : size);
        ViewAbstract::SetHeight(axis_ == Axis::VERTICAL ? size : CalcLength(FILL_LENGTH));
        auto node = stack->Finish();
        stack->StopGetAccessRecording();
        return { std::to_string(index), node };
    }

private:
    std::set<int32_t> createdIndices_;
    bool uneven_ = false;
    Axis axis_ = Axis::VERTICAL;
};

class FastScrollUnevenGridBuilder final : public Framework::MockLazyForEachBuilder {
public:
    explicit FastScrollUnevenGridBuilder(int32_t count = INNER_ITEM_COUNT) : count_(count) {}

    const std::set<int32_t>& GetCreatedIndices() const
    {
        return createdIndices_;
    }

    RefPtr<FrameNode> GetBuiltItem(int32_t index) const
    {
        auto item = builtItems_.find(index);
        return item == builtItems_.end() ? nullptr : item->second.Upgrade();
    }

protected:
    int32_t OnGetTotalCount() override
    {
        return count_;
    }

    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        StackModelNG model;
        createdIndices_.insert(index);
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        model.Create();
        stack->StopGetAccessRecording();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(index < count_ / HEIGHT_SECTION_COUNT ? ITEM_MAIN_SIZE : TALL_ITEM_SIZE));
        auto node = ViewStackProcessor::GetInstance()->Finish();
        builtItems_[index] = AceType::DynamicCast<FrameNode>(node);
        return { std::to_string(index), node };
    }

private:
    int32_t count_;
    std::map<int32_t, WeakPtr<FrameNode>> builtItems_;
    std::set<int32_t> createdIndices_;
};

class FastScrollOuterGroupBuilder final : public ListItemMockLazy {
public:
    explicit FastScrollOuterGroupBuilder(const RefPtr<FastScrollInnerBuilder>& inner)
        : ListItemMockLazy(OUTER_ITEM_COUNT, ITEM_MAIN_SIZE), inner_(inner) {}

    const std::set<int32_t>& GetGroupIds() const
    {
        return groupIds_;
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        if (index != OUTER_TARGET_INDEX) {
            return ListItemMockLazy::OnGetChildByIndex(index, expiringItems);
        }
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        ListItemGroupModelNG group;
        group.Create({});
        stack->StopGetAccessRecording();
        groupIds_.insert(stack->GetMainFrameNode()->GetId());
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        LazyForEachModelNG lazy;
        lazy.Create(RefPtr<LazyForEachBuilder>(inner_));
        stack->Pop();
        stack->StopGetAccessRecording();
        return { std::to_string(index), stack->Finish() };
    }

private:
    RefPtr<FastScrollInnerBuilder> inner_;
    std::set<int32_t> groupIds_;
};

class FastScrollOuterGridBuilder final : public Framework::MockLazyForEachBuilder {
public:
    explicit FastScrollOuterGridBuilder(const RefPtr<FastScrollUnevenGridBuilder>& inner) : inner_(inner) {}

    const std::set<int32_t>& GetGridIds() const
    {
        return gridIds_;
    }

    RefPtr<FrameNode> GetGrid() const
    {
        return grid_.Upgrade();
    }

protected:
    int32_t OnGetTotalCount() override
    {
        return OUTER_ITEM_COUNT;
    }

    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& /* expiringItems */) override
    {
        auto stack = ViewStackProcessor::GetInstance();
        if (index == OUTER_TARGET_INDEX) {
            stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
            LazyVGridLayoutModel::Create();
            stack->StopGetAccessRecording();
            LazyVGridLayoutModel::SetColumnsTemplate("1fr 1fr 1fr");
            ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
            auto grid = stack->GetMainFrameNode();
            stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
            LazyForEachModelNG lazyModel;
            lazyModel.Create(RefPtr<LazyForEachBuilder>(inner_));
            stack->Pop();
            stack->StopGetAccessRecording();
            grid_ = grid;
            gridIds_.insert(grid->GetId());
            return { std::to_string(index), stack->Finish() };
        }
        ListItemModelNG model;
        stack->StartGetAccessRecordingFor(ElementRegister::GetInstance()->MakeUniqueId());
        model.Create();
        stack->StopGetAccessRecording();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(index == PRECEDING_TALL_INDEX ||
            index == FOLLOWING_TALL_INDEX ? OUTER_TALL_SIZE : ITEM_MAIN_SIZE));
        return { std::to_string(index), stack->Finish() };
    }

private:
    RefPtr<FastScrollUnevenGridBuilder> inner_;
    WeakPtr<FrameNode> grid_;
    std::set<int32_t> gridIds_;
};
} // namespace

class ListFastScrollLayoutTestNg : public ListTestNg {
public:
    void SetUp() override
    {
        ListTestNg::SetUp();
        animationVersion_ = MockAnimationManager::GetVersion();
        MockAnimationManager::SetVersion(MockAnimationManager::Version::V2);
    }

    void TearDown() override
    {
        ListTestNg::TearDown();
        MockAnimationManager::SetVersion(animationVersion_);
    }

protected:
    RefPtr<ListLayoutAlgorithm> CreateOffsetPremeasureAlgorithm()
    {
        auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
        CHECK_NULL_RETURN(algorithm, nullptr);
        algorithm->GetOrCreateFastScrollState();
        algorithm->axis_ = Axis::VERTICAL;
        algorithm->listLayoutProperty_ = layoutProperty_;
        algorithm->contentMainSize_ = HEIGHT;
        algorithm->startMainPos_ = 0.0f;
        algorithm->endMainPos_ = HEIGHT;
        algorithm->childLayoutConstraint_ = layoutProperty_->CreateChildConstraint();
        OptionalSizeF listSize;
        listSize.SetWidth(WIDTH);
        listSize.SetHeight(HEIGHT);
        algorithm->UpdateListItemConstraint(Axis::VERTICAL, listSize, algorithm->childLayoutConstraint_);
        return algorithm;
    }

    RefPtr<FastScrollInnerBuilder> CreateLargeLazyGroup(
        bool knownInnerSize = false, int32_t leadingItems = 0, bool knownOuterSize = true, bool uneven = false)
    {
        auto model = CreateList();
        if (knownOuterSize) {
            auto sizes = model.GetOrCreateListChildrenMainSize();
            sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
            sizes->ChangeData(leadingItems, 1, { INNER_ITEM_COUNT * ITEM_MAIN_SIZE });
        }
        CreateListItems(leadingItems);
        auto group = CreateListItemGroup();
        if (knownInnerSize) {
            group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        auto builder = AceType::MakeRefPtr<FastScrollInnerBuilder>(uneven);
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(builder));
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        CreateListItems(TRAILING_ITEM_COUNT);
        CreateDone();
        return builder;
    }

    void DriveFastScroll(const RefPtr<FrameNode>& grid = nullptr,
        const RefPtr<FastScrollUnevenGridBuilder>& builder = nullptr)
    {
        constexpr int32_t maxFrames = 24;
        // Preserve legacy/zero-offset scheduling; extra-offset preparation also requires real prediction.
        const bool drivePrediction = pattern_->GetFastScrollRequest() &&
            pattern_->GetFastScrollPreparation()->preparation != FastScrollPreparation::NONE;
        std::ostringstream trace;
        AppendFastScrollTrace(trace, -1);
        AppendLazyGridTrace(trace, grid, builder);
        for (int32_t frame = 0; frame < maxFrames; ++frame) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks();
            AppendFastScrollTrace(trace, frame);
            trace << " phase=layout";
            AppendLazyGridTrace(trace, grid, builder);
            if (drivePrediction) {
                ASSERT_NO_FATAL_FAILURE(FlushFastScrollIdleTasks());
                AppendFastScrollTrace(trace, frame);
                trace << " phase=idle";
                AppendLazyGridTrace(trace, grid, builder);
            }
            if (!pattern_->GetFastScrollRequest() && !pattern_->AnimateRunning() && !pattern_->scrollStop_) {
                RecordProperty("fastScrollTrace", trace.str());
                return;
            }
        }
        ADD_FAILURE() << "Fast scroll did not finish within " << maxFrames << " frames" << trace.str();
    }

    void FlushFastScrollIdleTasks()
    {
        constexpr int64_t idleBudget = 16 * 1000000; // 16ms in nanoseconds, matching the Lazy layout fixtures.
        auto pipeline = pattern_->GetContext();
        ASSERT_NE(pipeline, nullptr);
        pipeline->OnIdle(GetSysTimestamp() + idleBudget);
        // Consume prediction-triggered layout without introducing another forced parent Measure.
        pipeline->FlushUITasks();
    }

    void AppendFastScrollTrace(std::ostringstream& trace, int32_t frame)
    {
        trace << "\nframe=" << frame << " offset=" << pattern_->GetTotalOffset()
              << " animation=" << pattern_->AnimateRunning() << " stop=" << pattern_->scrollStop_;
        if (!pattern_->GetFastScrollRequest()) {
            trace << " request=none";
            return;
        }
        const auto& request = *pattern_->GetFastScrollRequest();
        float target = 0.0f;
        const bool targetValid = pattern_->posMap_ && pattern_->GetFastScrollTargetPosition(target);
        trace << " preparation=" << static_cast<int32_t>(pattern_->GetFastScrollPreparation()->preparation)
              << " skipLayout=" << pattern_->fastScrollSkip_
              << " targetValid=" << targetValid
              << " target=" << target << " destination=" << request.targetOffset
              << " residual=" << target - pattern_->GetTotalOffset();
        auto item = pattern_->itemPosition_.find(request.index);
        if (item != pattern_->itemPosition_.end() && item->second.isGroup) {
            auto node = AceType::DynamicCast<FrameNode>(
                ElementRegister::GetInstance()->GetUINodeById(item->second.id));
            auto group = node ? node->GetPattern<ListItemGroupPattern>() : nullptr;
            if (group) {
                trace << " group=" << node->GetGeometryNode()->GetFrameRect().ToString();
                for (const auto& [index, position] : group->GetItemPosition()) {
                    trace << " inner" << index << "=" << position.startPos << ":" << position.endPos;
                }
            }
        }
    }

    void AppendLazyGridTrace(std::ostringstream& trace, const RefPtr<FrameNode>& grid,
        const RefPtr<FastScrollUnevenGridBuilder>& builder)
    {
        if (!grid || !builder) {
            return;
        }
        const auto info = grid->GetPattern<LazyGridLayoutPattern>()->layoutInfo_;
        trace << " grid=" << grid->GetGeometryNode()->GetFrameRect().ToString()
              << " total=" << info->totalMainSize_ << " predict=" << info->NeedPredict()
              << " visible=" << info->visibleStartIndex_ << ":" << info->visibleEndIndex_
              << " cached=" << info->cachedStartIndex_ << ":" << info->cachedEndIndex_
              << " layouted=" << info->layoutedStartIndex_ << ":" << info->layoutedEndIndex_
              << " coverage=" << info->layoutedStart_ << ":" << info->layoutedEnd_
              << " cacheWindow=" << info->cacheStartPos_ << ":" << info->cacheEndPos_
              << " adjust=" << info->adjustOffset_.start << ":" << info->adjustOffset_.end;
        for (int32_t index = INNER_ITEM_COUNT - 6; index < INNER_ITEM_COUNT; ++index) {
            auto item = builder->GetBuiltItem(index);
            trace << " item" << index << "=";
            if (!item) {
                trace << "absent";
                continue;
            }
            trace << item->GetGeometryNode()->GetFrameRect().ToString() << "/active:" << item->IsActive()
                  << "/dirty:" << item->GetLayoutProperty()->GetPropertyChangeFlag()
                  << "/constraint:" << item->GetGeometryNode()->GetParentLayoutConstraint().has_value();
        }
    }

private:
    MockAnimationManager::Version animationVersion_ = MockAnimationManager::Version::V0;
};

/**
 * @tc.name: OrdinaryListDoesNotAllocateFastScrollState001
 * @tc.desc: Ordinary layout and disabled fast-scroll setters keep request-only storage absent.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, OrdinaryListDoesNotAllocateFastScrollState001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateDone();
    EXPECT_EQ(pattern_->fastScrollPreparation_, nullptr);
    EXPECT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_EQ(algorithm->fastScrollState_, nullptr);
    algorithm->SetFastScrollKnownMainSize(false);
    algorithm->SetFastScrollSkip(false);
    algorithm->SetFastScrollPreparedLayouts(nullptr);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    EXPECT_EQ(algorithm->GetFastScrollState(), nullptr);
    EXPECT_EQ(algorithm->GetFastScrollMeasureResult(), nullptr);
    EXPECT_FALSE(algorithm->MatchesFastScrollTarget(0, ScrollAlign::START, 0.0f));
    EXPECT_EQ(algorithm->GetFastScrollPreparedLayouts(), nullptr);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    algorithm->PrepareFastScrollTargetWindow(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->fastScrollState_, nullptr);
    pattern_->CancelFastScroll();
    FlushUITasks();
    EXPECT_EQ(pattern_->fastScrollPreparation_, nullptr);
}

/**
 * @tc.name: FastScrollAllocatesAndReleasesPreparationState001
 * @tc.desc: A fast request owns its preparation state; cancellation does not leave it on an ordinary list.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollAllocatesAndReleasesPreparationState001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, 100.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_NE(pattern_->fastScrollPreparation_, nullptr);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    ASSERT_NE(algorithm->fastScrollState_, nullptr);
    EXPECT_TRUE(algorithm->HasFastScrollPreparationRequest());
    pattern_->StopAnimate();
    EXPECT_EQ(pattern_->fastScrollPreparation_, nullptr);
    FlushUITasks();
    auto ordinary = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(ordinary, nullptr);
    EXPECT_EQ(ordinary->fastScrollState_, nullptr);
}

/**
 * @tc.name: FastScrollPreparedLayoutsTransferWithoutPremeasurement001
 * @tc.desc: Prepared geometry transfers to normal layout without scheduling another premeasurement.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollPreparedLayoutsTransferWithoutPremeasurement001, TestSize.Level1)
{
    CreateList();
    CreateListItems(100);
    CreateDone();
    pattern_->InitializeFastScrollRequest(80, ScrollAlign::END, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto& preparation = pattern_->GetOrCreateFastScrollPreparation();
    preparation.preparation = FastScrollPreparation::READY;
    auto layouts = std::make_shared<FastScrollPreparedLayouts>();
    auto geometry = std::make_shared<PreparedLayoutGeometry>();
    layouts->emplace(80, geometry);
    preparation.preparedLayouts = layouts;
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_EQ(algorithm->GetFastScrollPreparedLayouts(), layouts);
    EXPECT_EQ(algorithm->GetFastScrollPreparedLayouts()->at(80), geometry);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
    pattern_->CancelFastScroll();
    EXPECT_EQ(pattern_->fastScrollPreparation_, nullptr);
    EXPECT_EQ(algorithm->GetFastScrollPreparedLayouts(), layouts);
}

class ListFastScrollInvalidMeasureTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, float>> {};

/**
 * @tc.name: InvalidPreparationCancelsWithoutLegacyMeasurement
 * @tc.desc: Invalid preparation is terminal, never falls back to measuring the entire path.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollInvalidMeasureTestNg, InvalidPreparationCancelsWithoutLegacyMeasurement, TestSize.Level1)
{
    const auto [lanes, extent] = GetParam();
    auto model = CreateList();
    model.SetLanes(lanes);
    CreateListItems(100);
    CreateDone();
    pattern_->InitializeFastScrollRequest(80, ScrollAlign::END, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    float size = 0.0f;
    EXPECT_FALSE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, extent, false, size));
    ASSERT_NE(algorithm->GetFastScrollMeasureResult(), nullptr);
    EXPECT_EQ(algorithm->GetFastScrollMeasureResult()->status, FastScrollMeasureStatus::INVALID);
    EXPECT_TRUE(algorithm->GetFastScrollMeasureResult()->measuredSizes.empty());
    pattern_->ApplyFastScrollLayoutResult(algorithm);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(pattern_->GetFastScrollPreparation(), nullptr);
    EXPECT_FALSE(pattern_->targetIndex_);
    EXPECT_FALSE(pattern_->FallbackFastScroll());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
}

INSTANTIATE_TEST_SUITE_P(InvalidExtent, ListFastScrollInvalidMeasureTestNg,
    testing::Combine(testing::Values(1, 2), testing::Values(std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN())));

class ListFastScrollConfirmedRowLayoutTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool>> {};

/**
 * @tc.name: FastScrollConfirmedRowCrossPosition
 * @tc.desc: A real Group resets the following row's cross position in horizontal, mirrored and RTL layouts.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollConfirmedRowLayoutTestNg, FastScrollConfirmedRowCrossPosition, TestSize.Level1)
{
    const auto [horizontal, stack, reverse] = GetParam();
    auto model = CreateList();
    model.SetLanes(3);
    model.SetInitialIndex(0);
    const int32_t groupIndex = stack ? 3 : 4;
    CreateListItems(groupIndex);
    CreateListItemGroup();
    CreateListItems(2);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateListItems(7 - groupIndex);
    CreateDone();
    auto algorithm = AceType::DynamicCast<ListLanesLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    algorithm->lanes_ = 3;
    algorithm->axis_ = horizontal ? Axis::HORIZONTAL : Axis::VERTICAL;
    algorithm->isStackFromEnd_ = stack;
    algorithm->isReverse_ = reverse;
    algorithm->totalItemCount_ = 8;
    algorithm->contentMainSize_ = HEIGHT;
    int32_t startIndex = 4;
    for (int32_t index = 4; index < 8; ++index) {
        const bool isGroup = index == 4;
        const int32_t physical = stack ? 7 - index : index;
        auto wrapper = frameNode_->GetOrCreateChildByIndex(physical);
        ASSERT_NE(wrapper, nullptr);
        ASSERT_EQ(wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG, isGroup);
        const float childCrossSize = isGroup ? WIDTH : WIDTH / 3;
        wrapper->GetGeometryNode()->SetFrameSize(
            horizontal ? SizeF(80.0f, childCrossSize) : SizeF(childCrossSize, 80.0f));
        const ListItemInfo info = { wrapper->GetHostNode()->GetId(), index == 4 ? 0.0f : 80.0f,
            index == 4 ? 80.0f : 160.0f, isGroup };
        algorithm->LayoutItem(wrapper, index, info, startIndex, WIDTH);
        const auto offset = wrapper->GetGeometryNode()->GetMarginFrameOffset();
        const float cross = (index == 4 ? 0 : index - 5) * WIDTH / 3;
        EXPECT_NEAR(horizontal ? offset.GetY() : offset.GetX(),
            !horizontal && reverse ? WIDTH - childCrossSize - cross : cross, 0.1f);
        EXPECT_EQ(startIndex, 5);
    }
}

INSTANTIATE_TEST_SUITE_P(PartialConfirmedRows, ListFastScrollConfirmedRowLayoutTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollVisibleMultiLaneTargetStartsWithoutPreparation001
 * @tc.desc: An already laid-out ordinary target starts immediately, without building types or filling positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollVisibleMultiLaneTargetStartsWithoutPreparation001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(3);
    model.SetInitialIndex(0);
    CreateListItems(100);
    CreateDone();
    ASSERT_EQ(pattern_->itemPosition_.count(4), 1u);
    const auto ranges = pattern_->lanesItemRange_;
    const auto positions = pattern_->posMap_->posMap_;
    pattern_->InitializeFastScrollRequest(4, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_TRUE(pattern_->MarkFastScrollVisibleTargetPrepared());
    pattern_->TryStartFastScroll();
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_TRUE(Positive(pattern_->GetFastScrollRequest()->viewportSize));
    EXPECT_EQ(pattern_->lanesItemRange_, ranges);
    ASSERT_EQ(pattern_->posMap_->posMap_.size(), positions.size());
    for (const auto& [index, info] : positions) {
        EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(index).mainPos, info.mainPos);
        EXPECT_FLOAT_EQ(pattern_->posMap_->GetPositionInfo(index).mainSize, info.mainSize);
    }
    pattern_->StopScrollableAndAnimate();
}

/**
 * @tc.name: FastScrollMultiLaneExtraOffsetStillRequiresPreparation001
 * @tc.desc: A visible target does not bypass the preparation required by an extraOffset window.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollMultiLaneExtraOffsetStillRequiresPreparation001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetLanes(3);
    CreateListItems(100);
    CreateDone();
    pattern_->InitializeFastScrollRequest(4, ScrollAlign::START, -20.0f, FastScrollMode::SKIP_MEASURE);
    EXPECT_FALSE(pattern_->MarkFastScrollVisibleTargetPrepared());
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    pattern_->CancelFastScroll();
}

class ListFastScrollOverlapReuseTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<bool, int32_t>> {
protected:
    RefPtr<FrameNode> ConfigureReuseCase(const RefPtr<ListLayoutAlgorithm>& algorithm, int32_t mode)
    {
        auto child = GetChildFrameNode(frameNode_, 1);
        CHECK_NULL_RETURN(child, nullptr);
        if (mode == 1) {
            child->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        } else if (mode == OVERLAP_CONSTRAINT_CHANGED) {
            algorithm->childLayoutConstraint_.maxSize.SetWidth(WIDTH / 2.0f);
        } else if (mode == OVERLAP_LAYOUT_INCOMPLETE) {
            child->isLayoutComplete_ = false;
        } else if (mode == OVERLAP_EXPAND_SAFE_AREA) {
            algorithm->expandSafeArea_ = true;
        } else if (mode == OVERLAP_GROUP_ITEM) {
            algorithm->itemPosition_.at(1).isGroup = true;
        } else if (mode == OVERLAP_LAZY_ITEM) {
            algorithm->itemPosition_.at(1).isLazyChild = true;
        } else if (mode == OVERLAP_POSITION_MISSING) {
            algorithm->itemPosition_.erase(1);
        } else if (mode == OVERLAP_NODE_DETACHED) {
            auto detached = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ListItemPattern>(nullptr));
            detached->SetGeometryNode(child->GetGeometryNode()->Clone());
            detached->SetLayoutProperty(child->GetLayoutProperty()->Clone());
            detached->isLayoutComplete_ = true;
            algorithm->itemPosition_.at(1).id = detached->GetId();
            child = detached;
        } else if (mode == OVERLAP_DEFERRED_MEASURE) {
            auto state = std::make_shared<GroupDeferredMeasureState>();
            state->deferred = true;
            child->layoutAlgorithm_ = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
                AceType::MakeRefPtr<GroupDeferredMeasureAlgorithm>(state));
        }
        return child;
    }
};

/**
 * @tc.name: FastScrollPreparationReusesOnlyValidVisibleItem
 * @tc.desc: Clean ordinary geometry avoids wrapper acquisition, but invalid or special geometry does not.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollOverlapReuseTestNg, FastScrollPreparationReusesOnlyValidVisibleItem, TestSize.Level1)
{
    const auto [knownSize, mode] = GetParam();
    auto model = CreateList();
    if (knownSize) {
        model.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
    }
    CreateListItems(100);
    CreateDone();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    auto child = ConfigureReuseCase(algorithm, mode);
    ASSERT_NE(child, nullptr);
    auto itemPattern = child->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    const bool reserveSpace = algorithm->NeedReserveEditModeCheckBoxSpace();
    itemPattern->SetNeedReserveEditModeCheckBoxSpace(!reserveSpace);
    auto wrapper = AceType::MakeRefPtr<FastScrollCountingWrapper>(frameNode_);
    float size = 0.0f;
    const bool complete = algorithm->MeasureFastScrollItem(AceType::RawPtr(wrapper), 1, HEIGHT, true, size);
    EXPECT_EQ(wrapper->requests, mode == 0 ? 0 : 1);
    if (mode == 9 && !knownSize) {
        EXPECT_FALSE(complete);
        EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().result.measuredSizes.empty());
        EXPECT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.count(1), 1);
        algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
        return;
    }
    ASSERT_TRUE(complete);
    EXPECT_FLOAT_EQ(size, ITEM_MAIN_SIZE);
    ASSERT_EQ(algorithm->GetOrCreateFastScrollState().result.measuredSizes.count(1), 1);
    if (mode == 0) {
        EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().premeasuredItems.empty());
        EXPECT_EQ(itemPattern->GetNeedReserveEditModeCheckBoxSpace(), knownSize ? !reserveSpace : reserveSpace);
    }
}

INSTANTIATE_TEST_SUITE_P(CurrentWindow, ListFastScrollOverlapReuseTestNg,
    testing::Combine(testing::Bool(), testing::Values(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)));

class ListFastScrollGroupPremeasureTestNg : public ListFastScrollLayoutTestNg {
protected:
    void PushGroupChild(float height, bool selected)
    {
        auto state = selected ? deferred_ : std::make_shared<GroupDeferredMeasureState>();
        auto child = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG, GetElmtId(),
            AceType::MakeRefPtr<GroupDeferredMeasurePattern>(state));
        ViewStackProcessor::GetInstance()->Push(child);
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(height));
        if (selected) {
            deferredChild_ = child;
        }
    }

    void CreateDeferredGroup(GroupDeferredPart part)
    {
        auto model = CreateList();
        model.SetCachedCount(0);
        auto group = CreateListItemGroup();
        group.SetHeader([this, part]() { PushGroupChild(groupHeaderSize, part == GroupDeferredPart::HEADER); });
        group.SetFooter([this, part]() { PushGroupChild(groupFooterSize, part == GroupDeferredPart::FOOTER); });
        for (int32_t index = 0; index < groupItemCount; ++index) {
            PushGroupChild(ITEM_MAIN_SIZE, part == GroupDeferredPart::BODY &&
                index == groupItemCount / HEIGHT_SECTION_COUNT - HEIGHT_SECTION_COUNT);
            ViewStackProcessor::GetInstance()->Pop();
        }
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
        CreateDone();
        ASSERT_NE(deferredChild_, nullptr);
    }

    void ExpectDeferredGroupGeometry(ScrollAlign align, float extraOffset)
    {
        const float total = groupHeaderSize + groupItemCount * ITEM_MAIN_SIZE + groupFooterSize;
        const float offset = std::clamp((align == ScrollAlign::END ? total - HEIGHT : 0.0f) + extraOffset,
            0.0f, total - HEIGHT);
        auto group = GetChildFrameNode(frameNode_, 0);
        ASSERT_NE(group, nullptr);
        EXPECT_NEAR(group->GetGeometryNode()->GetFrameRect().Top(), -offset, 0.1f);
        EXPECT_NEAR(group->GetGeometryNode()->GetFrameRect().Height(), total, 0.1f);
        for (int32_t index = 0; index < groupItemCount; ++index) {
            const float top = groupHeaderSize + index * ITEM_MAIN_SIZE - offset;
            if (top >= HEIGHT || top + ITEM_MAIN_SIZE <= 0.0f) {
                continue;
            }
            const auto rect = pattern_->GetItemRectInGroup(0, index);
            EXPECT_NEAR(rect.Top(), top, 0.1f) << "inner item=" << index;
            EXPECT_NEAR(rect.Height(), ITEM_MAIN_SIZE, 0.1f);
        }
    }

    static constexpr int32_t groupItemCount = 20;
    static constexpr float groupHeaderSize = 30.0f;
    static constexpr float groupFooterSize = 70.0f;
    std::shared_ptr<GroupDeferredMeasureState> deferred_ = std::make_shared<GroupDeferredMeasureState>();
    RefPtr<FrameNode> deferredChild_;
};

class ListFastScrollGroupDeferredTestNg : public ListFastScrollGroupPremeasureTestNg,
    public testing::WithParamInterface<std::tuple<GroupDeferredPart, ScrollAlign>> {};

/**
 * @tc.name: FastScrollGroupDeferredChildDoesNotCompletePreparation
 * @tc.desc: Real child measurement reporting another frame cannot publish a completed extra-offset preparation.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupDeferredTestNg, FastScrollGroupDeferredChildDoesNotCompletePreparation, TestSize.Level1)
{
    const auto [part, align] = GetParam();
    ASSERT_NO_FATAL_FAILURE(CreateDeferredGroup(part));
    deferred_->deferred = true;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    const float extraOffset = align == ScrollAlign::START ? 900.0f : -900.0f;
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, align, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_GT(deferred_->attempts, 0);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    EXPECT_FALSE(pattern_->AnimateRunning());
    deferred_->deferred = false;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    DriveFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    ExpectDeferredGroupGeometry(align, extraOffset);
    FlushUITasks();
    ExpectDeferredGroupGeometry(align, extraOffset);
}

INSTANTIATE_TEST_SUITE_P(DeferredPart, ListFastScrollGroupDeferredTestNg,
    testing::Combine(testing::Values(GroupDeferredPart::BODY, GroupDeferredPart::HEADER, GroupDeferredPart::FOOTER),
        testing::Values(ScrollAlign::START, ScrollAlign::END)));

class ListFastScrollGroupDeferredHeaderFooterTestNg : public ListFastScrollGroupPremeasureTestNg,
    public testing::WithParamInterface<GroupDeferredPart> {};

/**
 * @tc.name: OrdinaryGroupDeferredHeaderFooterKeepsLegacyMeasurement
 * @tc.desc: Ordinary measurement updates both decoration sizes even when a child requests another frame.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupDeferredHeaderFooterTestNg, OrdinaryGroupDeferredHeaderFooterKeepsLegacyMeasurement,
    TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateDeferredGroup(GetParam()));
    auto group = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(group, nullptr);
    auto groupPattern = group->GetPattern<ListItemGroupPattern>();
    ASSERT_NE(groupPattern, nullptr);
    auto algorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(groupPattern->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    auto header = GetChildFrameNode(group, algorithm->headerIndex_);
    auto footer = GetChildFrameNode(group, algorithm->footerIndex_);
    ASSERT_NE(header, nullptr);
    ASSERT_NE(footer, nullptr);
    constexpr float headerSize = groupHeaderSize + 10.0f;
    constexpr float footerSize = groupFooterSize + 20.0f;
    ViewAbstract::SetHeight(AceType::RawPtr(header), CalcLength(headerSize));
    ViewAbstract::SetHeight(AceType::RawPtr(footer), CalcLength(footerSize));
    deferred_->deferred = true;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    algorithm->MeasureHeaderFooter(AceType::RawPtr(group));
    EXPECT_GT(deferred_->attempts, 0);
    EXPECT_FLOAT_EQ(algorithm->headerMainSize_, headerSize);
    EXPECT_FLOAT_EQ(algorithm->footerMainSize_, footerSize);
    EXPECT_EQ(algorithm->fastScrollState_, nullptr);
}

/**
 * @tc.name: FastScrollGroupDeferredHeaderFooterKeepsBodyAnchors
 * @tc.desc: Failed header/footer preparation must stop before replacing valid body anchors and Group geometry.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupDeferredHeaderFooterTestNg, FastScrollGroupDeferredHeaderFooterKeepsBodyAnchors,
    TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateDeferredGroup(GetParam()));
    auto group = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(group, nullptr);
    auto groupPattern = group->GetPattern<ListItemGroupPattern>();
    ASSERT_NE(groupPattern, nullptr);
    const auto positions = groupPattern->GetItemPosition();
    ASSERT_FALSE(positions.empty());
    const auto frame = group->GetGeometryNode()->GetFrameRect();
    const auto constraint = group->GetGeometryNode()->GetParentLayoutConstraint();
    ASSERT_TRUE(constraint);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    algorithm->listLayoutProperty_ = layoutProperty_;
    algorithm->childLayoutConstraint_ = constraint.value();
    algorithm->axis_ = Axis::VERTICAL;
    algorithm->contentMainSize_ = HEIGHT;
    algorithm->startMainPos_ = 0.0f;
    algorithm->endMainPos_ = HEIGHT;
    algorithm->SetFastScrollTarget(0, ScrollAlign::END);
    deferred_->deferred = true;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    EXPECT_FALSE(algorithm->PremeasureFastScrollGroup(group, 0, HEIGHT + 900.0f, false));
    EXPECT_GT(deferred_->attempts, 0);
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    EXPECT_FALSE(groupAlgorithm->IsPremeasureComplete());
    EXPECT_EQ(group->GetGeometryNode()->GetFrameRect(), frame);
    const auto& actual = groupAlgorithm->GetItemPosition();
    ASSERT_EQ(actual.size(), positions.size());
    for (const auto& [index, position] : positions) {
        const auto item = actual.find(index);
        ASSERT_NE(item, actual.end());
        EXPECT_EQ(item->second.startPos, position.startPos) << "inner item=" << index;
        EXPECT_EQ(item->second.endPos, position.endPos) << "inner item=" << index;
    }
}

INSTANTIATE_TEST_SUITE_P(DeferredHeaderFooter, ListFastScrollGroupDeferredHeaderFooterTestNg,
    testing::Values(GroupDeferredPart::HEADER, GroupDeferredPart::FOOTER));

/**
 * @tc.name: FastScrollGroupDeferredCleanChildNeedsMeasure001
 * @tc.desc: Equal cached constraints and clean property flags do not make deferred geometry reusable in preparation.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollGroupPremeasureTestNg, FastScrollGroupDeferredCleanChildNeedsMeasure001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateDeferredGroup(GroupDeferredPart::HEADER));
    const auto constraint = deferredChild_->GetGeometryNode()->GetParentLayoutConstraint();
    ASSERT_TRUE(constraint);
    deferred_->deferred = true;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    deferredChild_->Measure(constraint);
    ASSERT_TRUE(deferredChild_->GetLayoutAlgorithm()->MeasureInNextFrame());
    // Isolate the reuse branch: retain the actual deferred algorithm, but remove the separate dirty signal.
    deferredChild_->GetLayoutProperty()->CleanDirty();
    ASSERT_FALSE(deferredChild_->CheckNeedForceMeasureAndLayout());
    ListItemGroupLayoutAlgorithm algorithm(-1, -1, 0);
    algorithm.childLayoutConstraint_ = constraint.value();
    EXPECT_FALSE(algorithm.CheckNeedMeasure(deferredChild_));
    algorithm.SetPremeasureOnly(true);
    EXPECT_TRUE(algorithm.CheckNeedMeasure(deferredChild_));
    EXPECT_FALSE(algorithm.MeasureChild(deferredChild_, constraint.value()));
    EXPECT_FALSE(algorithm.IsPremeasureComplete());
    EXPECT_TRUE(algorithm.fastScrollState_->premeasureFailed);
    deferred_->deferred = false;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    algorithm.SetPremeasureOnly(true);
    EXPECT_TRUE(algorithm.MeasureChild(deferredChild_, constraint.value()));
    EXPECT_FALSE(algorithm.fastScrollState_->premeasureFailed);
    deferredChild_->GetLayoutProperty()->CleanDirty();
    EXPECT_FALSE(algorithm.CheckNeedMeasure(deferredChild_));
    algorithm.SetPremeasureOnly(false);
    deferred_->deferred = true;
    EXPECT_FALSE(algorithm.CheckNeedMeasure(deferredChild_));
}

class ListFastScrollLazyGridTestNg : public ListFastScrollLayoutTestNg {
protected:
    RefPtr<FastScrollOuterGridBuilder> CreateOuterLazyGrid()
    {
        auto model = CreateList();
        model.SetCachedCount(0);
        builder_ = AceType::MakeRefPtr<FastScrollUnevenGridBuilder>(HUGE_LAZY_ITEM_COUNT);
        auto outer = AceType::MakeRefPtr<FastScrollOuterGridBuilder>(builder_);
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(outer));
        stack->Pop();
        stack->StopGetAccessRecording();
        CreateDone();
        return outer;
    }

    void CreateUnevenLazyGrid(
        bool knownSize = true, int32_t leadingItems = 0, int32_t lanes = 1, int32_t innerCount = INNER_ITEM_COUNT)
    {
        auto model = CreateList();
        model.SetCachedCount(0);
        if (knownSize) {
            auto sizes = model.GetOrCreateListChildrenMainSize();
            sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
            sizes->ChangeData(leadingItems, 1, { innerCount / (2 * lanes) * (ITEM_MAIN_SIZE + TALL_ITEM_SIZE) });
        }
        CreateListItems(leadingItems);
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyVGridLayoutModel::Create();
        LazyVGridLayoutModel::SetColumnsTemplate(lanes == 1 ? "1fr" : "1fr 1fr 1fr");
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        grid_ = stack->GetMainFrameNode();
        builder_ = AceType::MakeRefPtr<FastScrollUnevenGridBuilder>(innerCount);
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(builder_));
        stack->Pop();
        stack->StopGetAccessRecording();
        stack->Pop();
        stack->StopGetAccessRecording();
        CreateListItems(TRAILING_ITEM_COUNT);
        CreateDone();
    }

    void ExpectLazyItemAt(int32_t index, float top)
    {
        auto item = builder_->GetBuiltItem(index);
        ASSERT_NE(item, nullptr);
        EXPECT_TRUE(item->IsActive());
        EXPECT_NEAR(grid_->GetGeometryNode()->GetFrameOffset().GetY() +
            item->GetGeometryNode()->GetFrameRect().Top(), top, 0.1f);
        EXPECT_NEAR(item->GetGeometryNode()->GetFrameSize().Height(),
            index < INNER_ITEM_COUNT / HEIGHT_SECTION_COUNT ? ITEM_MAIN_SIZE : TALL_ITEM_SIZE, 0.1f);
    }

    RefPtr<FrameNode> grid_;
    RefPtr<FastScrollUnevenGridBuilder> builder_;
};

class ListFastScrollRepeatedPremeasureTestNg : public ListFastScrollLazyGridTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool, bool>> {
protected:
    void PrepareExistingLazyAnchor(bool decorated)
    {
        CreateUnevenLazyGrid(false, ANCHOR_GRID_INDEX, 1, ANCHOR_ITEM_COUNT);
        auto info = grid_->GetPattern<LazyGridLayoutPattern>()->layoutInfo_;
        info->lanes_ = 1;
        info->totalItemCount_ = ANCHOR_ITEM_COUNT;
        info->estimateItemSize_ = 100.0f;
        info->totalMainSize_ = 1000.0f;
        info->posMap_.clear();
        info->posMap_[ANCHOR_INNER_INDEX] = { 0, 700.0f, 800.0f };
        info->startIndex_ = info->layoutedStartIndex_ = info->cachedStartIndex_ = info->visibleStartIndex_ =
            ANCHOR_INNER_INDEX;
        info->endIndex_ = info->layoutedEndIndex_ = info->cachedEndIndex_ = info->visibleEndIndex_ = ANCHOR_INNER_INDEX;
        info->layoutedStart_ = info->cacheStartPos_ = 700.0f;
        info->layoutedEnd_ = info->cacheEndPos_ = 800.0f;
        grid_->GetGeometryNode()->SetFrameSize(SizeF(WIDTH, 1000.0f));
        if (decorated) {
            PaddingProperty padding;
            padding.top = CalcLength(13.0f);
            padding.bottom = CalcLength(19.0f);
            MarginProperty margin;
            margin.top = CalcLength(11.0f);
            margin.bottom = CalcLength(17.0f);
            grid_->GetLayoutProperty()->UpdatePadding(padding);
            grid_->GetLayoutProperty()->UpdateMargin(margin);
        }
    }
};

/**
 * @tc.name: FastScrollRepeatedLazyPremeasureKeepsReference
 * @tc.desc: Repeated preparation retains the original anchor shift and consumes it once before normal Measure.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollRepeatedPremeasureTestNg, FastScrollRepeatedLazyPremeasureKeepsReference, TestSize.Level1)
{
    const auto [forward, stackFromEnd, decorated] = GetParam();
    PrepareExistingLazyAnchor(decorated);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    float size = 0.0f;
    const float startInset = decorated ? 24.0f : 0.0f;
    const float totalInset = decorated ? 60.0f : 0.0f;
    for (int32_t attempt = 0; attempt < 3; ++attempt) {
        SCOPED_TRACE(attempt);
        ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 9, 10000.0f, forward, size));
        const auto& pending = algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment;
        ASSERT_TRUE(pending);
        EXPECT_FLOAT_EQ(pending->start, 800.0f + startInset);
        EXPECT_FLOAT_EQ(pending->end, 1200.0f + totalInset - startInset);
        EXPECT_FLOAT_EQ(size, 3000.0f + totalInset);
    }
    algorithm->isStackFromEnd_ = stackFromEnd;
    const bool physicalForward = forward != stackFromEnd;
    const float oldReference = physicalForward ? -700.0f : 300.0f;
    const float expected = physicalForward ? -1500.0f - startInset : 1500.0f + totalInset - startInset;
    float reference = stackFromEnd ? HEIGHT - oldReference : oldReference;
    algorithm->MeasureLazyChild(grid_, 9, reference, forward);
    EXPECT_FLOAT_EQ(reference, stackFromEnd ? HEIGHT - expected : expected);
    EXPECT_FALSE(algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment);
    algorithm->MeasureLazyChild(grid_, 9, reference, forward);
    EXPECT_FLOAT_EQ(reference, stackFromEnd ? HEIGHT - expected : expected);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
}

INSTANTIATE_TEST_SUITE_P(ReferenceEdges, ListFastScrollRepeatedPremeasureTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollGrowingLazyPremeasureAccumulatesAnchor001
 * @tc.desc: Expanding the real measured range composes consecutive nonzero anchor shifts.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollRepeatedPremeasureTestNg, FastScrollGrowingLazyPremeasureAccumulatesAnchor001, TestSize.Level1)
{
    PrepareExistingLazyAnchor(false);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 9, 600.0f, true, size));
    auto pending = algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment;
    ASSERT_TRUE(pending);
    EXPECT_FLOAT_EQ(pending->start, 400.0f);
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 9, 1500.0f, true, size));
    pending = algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment;
    ASSERT_TRUE(pending);
    EXPECT_FLOAT_EQ(pending->start, 800.0f);
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 9, 10000.0f, true, size));
    pending = algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment;
    ASSERT_TRUE(pending);
    EXPECT_FLOAT_EQ(pending->start, 800.0f);
    EXPECT_FLOAT_EQ(pending->end, 1200.0f);
    EXPECT_FLOAT_EQ(size, 3000.0f);
    float reference = -700.0f;
    algorithm->MeasureLazyChild(grid_, 9, reference, true);
    EXPECT_FLOAT_EQ(reference, -1500.0f);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
}

/**
 * @tc.name: FastScrollDirectionChangePremeasuresNeighborAgain001
 * @tc.desc: Real neighboring sizes reverse the approach and repeat the Lazy neighbor within one preparation call.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollRepeatedPremeasureTestNg, FastScrollDirectionChangePremeasuresNeighborAgain001, TestSize.Level1)
{
    PrepareExistingLazyAnchor(false);
    pattern_->InitializeFastScrollRequest(10, ScrollAlign::START, -3800.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    algorithm->SetFastScrollTarget(10, ScrollAlign::START);
    algorithm->SetFastScrollPreparationOffset(-3800.0f);
    algorithm->totalOffset_ = 200.0f;
    algorithm->itemPosition_.clear();
    algorithm->itemPosition_[2] = { -1, 0.0f, 100.0f };
    algorithm->posMap_->ClearPosMap();
    for (int32_t index = 0; index < 15; ++index) {
        algorithm->posMap_->UpdatePosWithCheck(index,
            { index * 100.0f + (index > 9 ? 4900.0f : 0.0f), index == 9 ? 5000.0f : 100.0f, false });
    }
    ASSERT_TRUE(algorithm->IsFastScrollForward(100.0f));
    auto wrapper = AceType::MakeRefPtr<FastScrollCountingWrapper>(frameNode_);
    algorithm->PrepareFastScrollTargetWindow(AceType::RawPtr(wrapper));
    ASSERT_EQ(algorithm->GetFastScrollMeasureResult()->status, FastScrollMeasureStatus::COMPLETE);
    EXPECT_FALSE(algorithm->IsFastScrollForward(100.0f));
    EXPECT_EQ(std::count(wrapper->requestedIndices.begin(), wrapper->requestedIndices.end(), 9u), 2);
    const auto& pending = algorithm->GetFastScrollState()->premeasuredItems.at(9).pendingLazyAdjustment;
    ASSERT_TRUE(pending);
    EXPECT_FLOAT_EQ(pending->start, 800.0f);
    EXPECT_FLOAT_EQ(pending->end, 1200.0f);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
}

class ListLazyGridOffsetTestNg : public ListFastScrollLazyGridTestNg,
    public testing::WithParamInterface<float> {};

/**
 * @tc.name: FastScrollOffscreenItemPremeasureClearsLayoutAlgorithm001
 * @tc.desc: A measured outer item outside the layout window is reset in the same frame.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollOffscreenItemPremeasureClearsLayoutAlgorithm001, TestSize.Level1)
{
    CreateList();
    CreateListItems(12);
    CreateDone();
    auto item = GetChildFrameNode(frameNode_, 9);
    ASSERT_NE(item, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    item->ResetLayoutAlgorithm();
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 9, HEIGHT, true, size));
    EXPECT_TRUE(item->HasLayoutAlgorithm());
    EXPECT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.count(9), 1);

    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->noLayoutedItems_.count(9), 0);
    EXPECT_FALSE(item->HasLayoutAlgorithm());
}

/**
 * @tc.name: FastScrollPremeasureCleanupReleasesOwnedReferences001
 * @tc.desc: Cleanup releases its wrapper, clears surviving measured algorithms, and tolerates expired weak nodes.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollPremeasureCleanupReleasesOwnedReferences001, TestSize.Level1)
{
    CreateList();
    CreateListItems(2);
    CreateDone();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    auto item = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    auto child = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    auto expired = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    WeakPtr<FrameNode> itemWeak(item);
    WeakPtr<FrameNode> expiredWeak(expired);
    child->GetLayoutAlgorithm();
    algorithm->TrackFastScrollPremeasuredItem(item, 10, true);
    auto& nodes = algorithm->GetOrCreateFastScrollState().premeasuredItems.at(10).measuredNodes;
    nodes = { child, child, expired };
    expired.Reset();
    EXPECT_EQ(expiredWeak.Upgrade(), nullptr);
    item.Reset();
    ASSERT_NE(itemWeak.Upgrade(), nullptr);
    algorithm->ResetFastScrollPremeasuredItems();
    EXPECT_EQ(itemWeak.Upgrade(), nullptr);
    EXPECT_FALSE(child->HasLayoutAlgorithm());
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
    algorithm->ResetFastScrollPremeasuredItems();
    EXPECT_NE(child, nullptr);
}

/**
 * @tc.name: FastScrollOffscreenGroupPremeasureClearsLayoutAlgorithm001
 * @tc.desc: Group and descendant algorithms measured only for extraOffset are cleared together.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollOffscreenGroupPremeasureClearsLayoutAlgorithm001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 3);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 8);
    ASSERT_NE(group, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    group->ResetLayoutAlgorithm();
    float size = 0.0f;
    algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 8, HEIGHT, true, size);
    EXPECT_TRUE(group->HasLayoutAlgorithm());
    EXPECT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.count(8), 1);

    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->noLayoutedItems_.count(8), 0);
    EXPECT_FALSE(group->HasLayoutAlgorithm());
}

/**
 * @tc.name: FastScrollGroupGeometryHandoffDoesNotPremeasure001
 * @tc.desc: Geometry handoff does not measure nodes or create offscreen cleanup work.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollGroupGeometryHandoffDoesNotPremeasure001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 3);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 8);
    ASSERT_NE(group, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->SetFastScrollTarget(8, ScrollAlign::END);
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 8, HEIGHT, false, size));
    auto prepared = algorithm->GetFastScrollPreparedLayouts();
    ASSERT_NE(prepared, nullptr);
    ASSERT_EQ(prepared->count(8), 1u);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_FALSE(group->HasLayoutAlgorithm());
    algorithm->noLayoutedItems_.clear();
    algorithm->ApplyFastScrollPreparedGeometry(group, 8);
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
    EXPECT_TRUE(algorithm->noLayoutedItems_.empty());
    EXPECT_FALSE(group->HasLayoutAlgorithm());
}

class ListFastScrollGroupCleanupTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<bool> {};

/**
 * @tc.name: FastScrollGroupCachedChildrenClearAfterPremeasure
 * @tc.desc: Group premeasurement clears detached LazyForEach children without activating or laying them out.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupCleanupTestNg, FastScrollGroupCachedChildrenClearAfterPremeasure, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false, true);
    auto group = GetChildFrameNode(frameNode_, 80);
    ASSERT_NE(group, nullptr);
    auto lazy = AceType::DynamicCast<LazyForEachNode>(group->GetChildAtIndex(0));
    ASSERT_NE(lazy, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, 700.0f, GetParam(), size));
    std::vector<RefPtr<FrameNode>> measured;
    for (int32_t index : builder->GetCreatedIndices()) {
        auto node = AceType::DynamicCast<FrameNode>(builder->GetChildByIndex(index, false, true).second);
        ASSERT_NE(node, nullptr);
        EXPECT_EQ(builder->GetAllChildren().at(index).second, nullptr);
        EXPECT_FALSE(node->IsActive());
        EXPECT_FALSE(node->IsOnMainTree());
        EXPECT_TRUE(node->HasLayoutAlgorithm());
        measured.push_back(node);
    }
    ASSERT_FALSE(measured.empty());
    EXPECT_TRUE(lazy->GetChildren().empty());
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    for (const auto& node : measured) {
        EXPECT_FALSE(node->HasLayoutAlgorithm());
        EXPECT_FALSE(node->IsActive());
        EXPECT_FALSE(node->IsOnMainTree());
        EXPECT_FALSE(node->IsLayoutComplete());
    }
}

INSTANTIATE_TEST_SUITE_P(Boundary, ListFastScrollGroupCleanupTestNg, testing::Bool());

class ListFastScrollGroupFailedCleanupTestNg : public ListFastScrollGroupPremeasureTestNg,
    public testing::WithParamInterface<GroupDeferredPart> {};

/**
 * @tc.name: FastScrollGroupFailedPremeasureRetainsCleanupNodes
 * @tc.desc: Incomplete body/header/footer measurement still transfers cleanup ownership to the List.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupFailedCleanupTestNg, FastScrollGroupFailedPremeasureRetainsCleanupNodes, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateDeferredGroup(GetParam()));
    auto group = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(group, nullptr);
    deferred_->deferred = true;
    deferredChild_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    EXPECT_FALSE(algorithm->PremeasureFastScrollGroup(group, 0, 3000.0f, true));
    ASSERT_GT(deferred_->attempts, 0);
    ASSERT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.count(0), 1);
    const auto& nodes = algorithm->GetOrCreateFastScrollState().premeasuredItems.at(0).measuredNodes;
    EXPECT_TRUE(std::any_of(nodes.begin(), nodes.end(), [this](const auto& weak) {
        return weak.Upgrade() == deferredChild_;
    }));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    EXPECT_TRUE(groupAlgorithm->TakePremeasuredNodes().empty());
    ASSERT_TRUE(deferredChild_->HasLayoutAlgorithm());
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_FALSE(deferredChild_->HasLayoutAlgorithm());
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().premeasuredItems.empty());
}

INSTANTIATE_TEST_SUITE_P(FailedPart, ListFastScrollGroupFailedCleanupTestNg,
    testing::Values(GroupDeferredPart::BODY, GroupDeferredPart::HEADER, GroupDeferredPart::FOOTER));

/**
 * @tc.name: FastScrollGroupCleanupKeepsLaidOutChild001
 * @tc.desc: Frame cleanup preserves a premeasured child's completed Layout and releases unlaid-out siblings.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollGroupCleanupKeepsLaidOutChild001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false, true);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, 700.0f, true, size));
    ASSERT_GT(builder->GetCreatedIndices().size(), 1);
    auto child = AceType::DynamicCast<FrameNode>(
        builder->GetChildByIndex(*builder->GetCreatedIndices().begin(), false, true).second);
    ASSERT_NE(child, nullptr);
    child->Layout();
    ASSERT_TRUE(child->IsLayoutComplete());
    ASSERT_FALSE(child->HasLayoutAlgorithm());
    const auto rect = child->GetGeometryNode()->GetFrameRect();
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_TRUE(child->IsLayoutComplete());
    EXPECT_EQ(child->GetGeometryNode()->GetFrameRect(), rect);
    for (int32_t index : builder->GetCreatedIndices()) {
        auto node = AceType::DynamicCast<FrameNode>(builder->GetChildByIndex(index, false, true).second);
        ASSERT_NE(node, nullptr);
        EXPECT_FALSE(node->HasLayoutAlgorithm());
    }
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().premeasuredItems.empty());
}

class ListFastScrollGroupBoundarySampleTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, bool, bool>> {};

/**
 * @tc.name: FastScrollGroupBoundaryPremeasureHasBodySample
 * @tc.desc: A one-pixel preparation inside header/footer still measures a whole boundary row, not just decorations.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupBoundarySampleTestNg, FastScrollGroupBoundaryPremeasureHasBodySample, TestSize.Level1)
{
    const auto [lanes, stack, forward] = GetParam();
    auto model = CreateList();
    model.SetLanes(lanes);
    model.SetStackFromEnd(stack);
    model.SetInitialIndex(0);
    CreateListItems(30);
    auto groupModel = CreateListItemGroup();
    groupModel.SetSpace(Dimension(SPACE));
    groupModel.SetHeader(GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN)));
    groupModel.SetFooter(GetRowOrColBuilder(FILL_LENGTH, Dimension(GROUP_HEADER_LEN)));
    CreateListItems(7);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 30);
    ASSERT_NE(group, nullptr);
    ASSERT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->isStackFromEnd_ = stack;
    algorithm->totalItemCount_ = 31;
    algorithm->SetFastScrollTarget(30, ScrollAlign::START);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, stack ? 0 : 30, 1.0f, forward));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    const auto geometry = groupAlgorithm->GetPreparedGeometry();
    ASSERT_NE(geometry, nullptr);
    EXPECT_EQ(geometry->lanes, lanes);
    EXPECT_EQ(geometry->fromEnd, forward == stack);
    EXPECT_EQ(geometry->positions.size(), forward != stack ? static_cast<size_t>(lanes) : 1u);
    const int32_t rows = (7 + lanes - 1) / lanes;
    EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(),
        rows * ITEM_MAIN_SIZE + (rows - 1) * SPACE + 2 * GROUP_HEADER_LEN);
    algorithm->ResetFastScrollPremeasuredItems();
}

INSTANTIATE_TEST_SUITE_P(DecoratedBoundaryRows, ListFastScrollGroupBoundarySampleTestNg,
    testing::Combine(testing::Values(1, 3), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollGroupPremeasureBackwardSummaryStable001
 * @tc.desc: Repeated suffix measurements keep the summary and children in the same estimated coordinate space.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollGroupPremeasureBackwardSummaryStable001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 10);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 8);
    ASSERT_NE(group, nullptr);
    ASSERT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->SetFastScrollTarget(8, ScrollAlign::END);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 150.0f, false));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 2u);
    ASSERT_TRUE(groupAlgorithm->layoutedItemInfo_);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->startIndex, 8);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->endIndex, 9);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->startPos, 800.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->endPos, 1000.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(8).startPos, 800.0f);
    EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 1000.0f);

    // A narrower second suffix must not merge the old origin with the translated end (old estimate: 5000).
    group->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 50.0f, false));
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 1u);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().begin()->first, 9);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(9).startPos, 900.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(9).endPos, 1000.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetLayoutInfo().averageHeight, 100.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->totalMainSize_, 1000.0f);
    EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 1000.0f);
}

/**
 * @tc.name: FastScrollGroupPremeasureForwardSummaryStable001
 * @tc.desc: Growing a prefix's estimated total size must not translate the prefix summary or child positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollGroupPremeasureForwardSummaryStable001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 10);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 8);
    ASSERT_NE(group, nullptr);
    ASSERT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->SetFastScrollTarget(8, ScrollAlign::START);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 150.0f, true));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 2u);
    ASSERT_TRUE(groupAlgorithm->layoutedItemInfo_);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->startIndex, 0);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->endIndex, 1);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->startPos, 0.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->endPos, 200.0f);
    EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 1000.0f);

    group->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 50.0f, true));
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 1u);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().begin()->first, 0);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(0).startPos, 0.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(0).endPos, 100.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->startPos, 0.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->endPos, 200.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetLayoutInfo().averageHeight, 100.0f);
    EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 1000.0f);
}

/**
 * @tc.name: FastScrollGroupPremeasureNullSummary001
 * @tc.desc: A supplied size is safe without a summary and translates children only for backward measurement.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollGroupPremeasureNullSummary001, TestSize.Level1)
{
    auto property = AceType::MakeRefPtr<ListItemGroupLayoutProperty>();
    for (bool forward : { false, true }) {
        SCOPED_TRACE(forward);
        ListItemGroupLayoutAlgorithm algorithm(-1, -1, 0);
        algorithm.SetFastScroll(true, 1000.0f);
        algorithm.forwardLayout_ = forward;
        algorithm.totalMainSize_ = 200.0f;
        algorithm.itemPosition_[9] = { -1, 100.0f, 200.0f };
        ASSERT_FALSE(algorithm.layoutedItemInfo_);
        algorithm.UpdatePremeasuredMainSize(property);
        EXPECT_FALSE(algorithm.layoutedItemInfo_);
        EXPECT_FLOAT_EQ(algorithm.totalMainSize_, 1000.0f);
        EXPECT_FLOAT_EQ(algorithm.itemPosition_.at(9).startPos, forward ? 100.0f : 900.0f);
        EXPECT_FLOAT_EQ(algorithm.itemPosition_.at(9).endPos, forward ? 200.0f : 1000.0f);
        algorithm.UpdatePremeasuredMainSize(property);
        EXPECT_FALSE(algorithm.layoutedItemInfo_);
        EXPECT_FLOAT_EQ(algorithm.itemPosition_.at(9).endPos, forward ? 200.0f : 1000.0f);
    }
}

/**
 * @tc.name: FastScrollCachedGroupPremeasureKeepsItemPositions001
 * @tc.desc: Reset a premeasured cached Group without discarding its cached child positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollCachedGroupPremeasureKeepsItemPositions001, TestSize.Level1)
{
    CreateList();
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 3);
    CreateListItems(8);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(group, nullptr);
    auto groupPattern = group->GetPattern<ListItemGroupPattern>();
    ASSERT_NE(groupPattern, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    float size = 0.0f;
    algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 0, HEIGHT, true, size);
    ASSERT_FALSE(groupPattern->GetItemPosition().empty());
    const auto positions = groupPattern->GetItemPosition();
    algorithm->itemPosition_.erase(0);
    algorithm->cachedItemPosition_[0] = { group->GetId(), 0.0f, size, true };

    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->noLayoutedItems_.count(0), 0);
    ASSERT_EQ(groupPattern->GetItemPosition().size(), positions.size());
    for (const auto& [index, position] : positions) {
        const auto& cached = groupPattern->GetItemPosition();
        auto iter = cached.find(index);
        ASSERT_NE(iter, cached.end());
        EXPECT_EQ(iter->second.id, position.id);
        EXPECT_FLOAT_EQ(iter->second.startPos, position.startPos);
        EXPECT_FLOAT_EQ(iter->second.endPos, position.endPos);
    }
    EXPECT_FALSE(group->HasLayoutAlgorithm());
}

/**
 * @tc.name: FastScrollOffscreenLazyGridPremeasureClearsChildAlgorithm001
 * @tc.desc: A temporary lazy premeasure leaves no measured child algorithm in an offscreen grid.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollOffscreenLazyGridPremeasureClearsChildAlgorithm001,
    TestSize.Level1)
{
    CreateUnevenLazyGrid(false, 80, 3, 3000);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    float size = 0.0f;
    algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, 500.0f, false, size);
    auto last = builder_->GetBuiltItem(2999);
    ASSERT_NE(last, nullptr);
    EXPECT_TRUE(last->HasLayoutAlgorithm());
    EXPECT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.count(80), 1);
    EXPECT_FALSE(algorithm->GetOrCreateFastScrollState().premeasuredItems.at(80).measuredNodes.empty());

    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->noLayoutedItems_.count(80), 0);
    EXPECT_FALSE(last->HasLayoutAlgorithm());
}

/**
 * @tc.name: FastScrollLazyLayoutRectUsesLegacy001
 * @tc.desc: A real Lazy root with an explicit layout rectangle transfers unsupported preparation to legacy.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollLazyLayoutRectUsesLegacy001, TestSize.Level1)
{
    CreateUnevenLazyGrid(false);
    grid_->GetLayoutProperty()->SetLayoutRect(RectF(0.0f, 0.0f, WIDTH, 3.0f * HEIGHT));
    grid_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    MockAnimationManager::GetInstance().SetTicks(4);
    auto events = std::make_shared<std::vector<std::string>>();
    eventHub_->SetOnScrollStart([this, events]() {
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        events->emplace_back("start");
    });
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::START, 25.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(pattern_->GetFastScrollPreparation()->NeedsPreparation());
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 25.0f);
    ASSERT_EQ(events->size(), 1);
    EXPECT_EQ(events->at(0), "start");
    eventHub_->SetOnScrollStart(nullptr);
    DriveFastScroll(grid_, builder_);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(pattern_->GetFinalPosition(), 25.0f);
    EXPECT_NEAR(grid_->GetGeometryNode()->GetFrameSize().Height(), 3.0f * HEIGHT, 0.1f);
}

/**
 * @tc.name: FastScrollLazyEndOffsetUsesRealSuffix
 * @tc.desc: A partial unequal suffix reaches its real inner item without measuring the unknown middle.
 * @tc.type: FUNC
 */
HWTEST_P(ListLazyGridOffsetTestNg, FastScrollLazyEndOffsetUsesRealSuffix, TestSize.Level1)
{
    const float extraOffset = GetParam();
    CreateUnevenLazyGrid(false);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::END, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll(grid_, builder_);
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    const float total = INNER_ITEM_COUNT / 2 * (ITEM_MAIN_SIZE + TALL_ITEM_SIZE);
    const float windowStart = total - pattern_->contentMainSize_ + extraOffset;
    const auto info = grid_->GetPattern<LazyGridLayoutPattern>()->layoutInfo_;
    EXPECT_FALSE(info->NeedPredict());
    for (int32_t index = INNER_ITEM_COUNT - 10; index < INNER_ITEM_COUNT; ++index) {
        const float top = total - (INNER_ITEM_COUNT - index) * TALL_ITEM_SIZE - windowStart;
        if (top >= pattern_->contentMainSize_ || top + TALL_ITEM_SIZE <= 0.0f) {
            continue;
        }
        ASSERT_GE(index, info->visibleStartIndex_);
        ASSERT_LE(index, info->visibleEndIndex_);
        auto item = grid_->GetChildByIndex(index);
        ASSERT_NE(item, nullptr);
        EXPECT_TRUE(item->GetHostNode()->IsActive());
        EXPECT_NEAR(grid_->GetGeometryNode()->GetFrameOffset().GetY() +
            item->GetGeometryNode()->GetFrameRect().Top(), top, 0.1f);
    }
    const auto& created = builder_->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 900;
    }), 0);
}

INSTANTIATE_TEST_SUITE_P(PartialSuffix, ListLazyGridOffsetTestNg, testing::Values(-500.0f, -900.0f, -2100.0f));

/**
 * @tc.name: FastScrollColdLazyGridEndOffsetSkipsInnerMiddle001
 * @tc.desc: A distant, never-displayed lazy target reuses its prepared suffix during animated layout.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollColdLazyGridEndOffsetSkipsInnerMiddle001, TestSize.Level1)
{
    constexpr int32_t innerCount = 3000;
    CreateUnevenLazyGrid(false, 80, 3, innerCount);
    ASSERT_TRUE(builder_->GetCreatedIndices().empty());
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, -500.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll(grid_, builder_);
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    const auto& created = builder_->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < innerCount - 100;
    }), 0);
    auto item = builder_->GetBuiltItem(innerCount - 6);
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->IsActive());
    EXPECT_NEAR(grid_->GetGeometryNode()->GetFrameOffset().GetY() +
        item->GetGeometryNode()->GetFrameRect().Top(), pattern_->contentMainSize_ - TALL_ITEM_SIZE, 0.1f);
}

/**
 * @tc.name: FastScrollOuterLazyGridRebuildRestoresSuffix001
 * @tc.desc: A cold grid released after preparation restores its suffix on rebuild without creating the middle.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollOuterLazyGridRebuildRestoresSuffix001, TestSize.Level1)
{
    auto outer = CreateOuterLazyGrid();
    ASSERT_TRUE(builder_->GetCreatedIndices().empty());
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, -500.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    const auto& created = builder_->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 2900;
    }), 0);
    EXPECT_EQ(outer->GetGridIds().size(), 2);
    EXPECT_EQ(outer->GetGridIds().count(ElementRegister::UndefinedElementId), 0);
    auto grid = outer->GetGrid();
    auto item = builder_->GetBuiltItem(2994);
    ASSERT_NE(grid, nullptr);
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->IsActive());
    EXPECT_NEAR(grid->GetGeometryNode()->GetFrameOffset().GetY() +
        item->GetGeometryNode()->GetFrameRect().Top(), pattern_->contentMainSize_ - TALL_ITEM_SIZE, 0.1f);
}

/**
 * @tc.name: FastScrollColdTargetGridReestablishesSuffix001
 * @tc.desc: A newly built target grid must establish its suffix before measuring a distant viewport.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollColdTargetGridReestablishesSuffix001, TestSize.Level1)
{
    CreateUnevenLazyGrid(false, 80, 3, 3000);
    ASSERT_TRUE(builder_->GetCreatedIndices().empty());
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->axis_ = Axis::VERTICAL;
    algorithm->contentMainSize_ = HEIGHT;
    algorithm->startMainPos_ = 697750.0f;
    algorithm->endMainPos_ = algorithm->startMainPos_ + HEIGHT;
    algorithm->listLayoutProperty_ = layoutProperty_;
    algorithm->childLayoutConstraint_ = frameNode_->GetLayoutProperty<ListLayoutProperty>()->CreateChildConstraint();
    OptionalSizeF listSize;
    listSize.SetWidth(WIDTH);
    listSize.SetHeight(HEIGHT);
    algorithm->UpdateListItemConstraint(Axis::VERTICAL, listSize, algorithm->childLayoutConstraint_);
    algorithm->SetFastScrollTarget(80, ScrollAlign::END);
    algorithm->SetFastScrollPreparationOffset(-500.0f);
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, 500.0f, false, size));
    auto prepared = algorithm->GetFastScrollPreparedLayouts();
    ASSERT_NE(prepared, nullptr);
    ASSERT_EQ(prepared->count(80), 1u);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    auto gridPattern = grid_->GetPattern<LazyGridLayoutPattern>();
    gridPattern->layoutInfo_ = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    float referencePos = 0.0f;
    algorithm->MeasureLazyChild(grid_, 80, referencePos, true);
    EXPECT_EQ(gridPattern->GetPreparedGeometry(), prepared->at(80));
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
    const auto& created = builder_->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 2900;
    }), 0);
    EXPECT_NE(builder_->GetBuiltItem(2999), nullptr);
}

/**
 * @tc.name: FastScrollColdTargetGroupReestablishesSuffix001
 * @tc.desc: A newly built target Group restores its measured tail before laying out a distant viewport.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollColdTargetGroupReestablishesSuffix001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false);
    ASSERT_TRUE(builder->GetCreatedIndices().empty());
    auto group = GetChildFrameNode(frameNode_, 80);
    ASSERT_NE(group, nullptr);
    auto algorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    algorithm->axis_ = Axis::VERTICAL;
    algorithm->contentMainSize_ = HEIGHT;
    algorithm->startMainPos_ = 0.0f;
    algorithm->endMainPos_ = HEIGHT;
    algorithm->listLayoutProperty_ = layoutProperty_;
    algorithm->childLayoutConstraint_ = layoutProperty_->CreateChildConstraint();
    OptionalSizeF listSize;
    listSize.SetWidth(WIDTH);
    listSize.SetHeight(HEIGHT);
    algorithm->UpdateListItemConstraint(Axis::VERTICAL, listSize, algorithm->childLayoutConstraint_);
    algorithm->SetFastScrollTarget(80, ScrollAlign::END);
    float size = 0.0f;
    ASSERT_TRUE(algorithm->MeasureFastScrollItem(AceType::RawPtr(frameNode_), 80, 2500.0f, false, size));
    auto prepared = algorithm->GetFastScrollPreparedLayouts();
    ASSERT_NE(prepared, nullptr);
    ASSERT_EQ(prepared->count(80), 1u);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    algorithm->SetListItemGroupParam(group, 80, -97500.0f, true, layoutProperty_, false);
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    EXPECT_EQ(groupAlgorithm->GetPreparedGeometry(), prepared->at(80));
    group->Measure(algorithm->childLayoutConstraint_);
    EXPECT_TRUE(algorithm->GetFastScrollState()->premeasuredItems.empty());
    EXPECT_FALSE(groupAlgorithm->IsPremeasureOnly());
    const auto& created = builder->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 900;
    }), 0);
    EXPECT_NE(created.count(INNER_ITEM_COUNT - 1), 0);
}

/**
 * @tc.name: FastScrollColdOuterGroupEndOffsetSkipsInnerMiddle001
 * @tc.desc: A distant Group retains its suffix across the full animated extra-offset request.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollColdOuterGroupEndOffsetSkipsInnerMiddle001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false);
    ASSERT_TRUE(builder->GetCreatedIndices().empty());
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, -2100.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    const auto& created = builder->GetCreatedIndices();
    EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 900;
    }), 0);
    EXPECT_NEAR(pattern_->GetItemRectInGroup(80, 975).Top(), 0.0f, 0.1f);
}

/**
 * @tc.name: FastScrollOuterGroupRebuildRestoresSuffix001
 * @tc.desc: Actual LazyForEach recycling rebuilds an uneven Group and restores its suffix without the middle.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollOuterGroupRebuildRestoresSuffix001, TestSize.Level1)
{
    auto model = CreateList();
    model.SetCachedCount(0);
    auto inner = AceType::MakeRefPtr<FastScrollInnerBuilder>(true);
    auto outer = AceType::MakeRefPtr<FastScrollOuterGroupBuilder>(inner);
    auto stack = ViewStackProcessor::GetInstance();
    stack->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(outer));
    stack->Pop();
    stack->StopGetAccessRecording();
    CreateDone();
    ASSERT_TRUE(outer->GetGroupIds().empty());
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, -2100.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(outer->GetGroupIds().size(), 2);
    EXPECT_EQ(outer->GetGroupIds().count(ElementRegister::UndefinedElementId), 0);
    EXPECT_EQ(std::count_if(inner->GetCreatedIndices().begin(), inner->GetCreatedIndices().end(),
        [](int32_t index) { return index >= 100 && index < 900; }), 0);
    const auto item = pattern_->GetItemRectInGroup(80, 994);
    EXPECT_NEAR(item.Height(), 300.0f, 0.1f);
    EXPECT_NEAR(item.Top(), HEIGHT - 300.0f, 1.0f);
}

/**
 * @tc.name: FastScrollUnevenGroupEndOffsetKeepsVisibleSuffix001
 * @tc.desc: The Group's own tail coordinates keep the real suffix aligned as its size estimate changes.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollUnevenGroupEndOffsetKeepsVisibleSuffix001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false, true);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, -2100.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_EQ(std::count_if(builder->GetCreatedIndices().begin(), builder->GetCreatedIndices().end(),
        [](int32_t index) { return index >= 100 && index < 900; }), 0);
    EXPECT_NEAR(pattern_->GetItemRectInGroup(80, 994).Top(), HEIGHT - 300.0f, 1.0f);
}

class ListFastScrollGroupBoundaryTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<Axis, TextDirection, bool, bool>> {
public:
    RefPtr<FastScrollInnerBuilder> CreateBoundaryGroup()
    {
        const auto [axis, direction, stackFromEnd, decorated] = GetParam();
        auto model = CreateList();
        model.SetListDirection(axis);
        model.SetStackFromEnd(stackFromEnd);
        ViewAbstract::SetLayoutDirection(direction);
        CreateListItems(OUTER_TARGET_INDEX);
        auto group = CreateListItemGroup();
        if (decorated) {
            group.SetSpace(Dimension(SPACE));
            group.SetHeader(GetRowOrColBuilder(FILL_LENGTH, Dimension(30.0f)));
            group.SetFooter(GetRowOrColBuilder(FILL_LENGTH, Dimension(70.0f)));
            SetBoundaryInsets(axis);
        }
        auto builder = AceType::MakeRefPtr<FastScrollInnerBuilder>(true, axis);
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazy;
        lazy.Create(RefPtr<LazyForEachBuilder>(builder));
        stack->Pop();
        stack->StopGetAccessRecording();
        stack->Pop();
        stack->StopGetAccessRecording();
        CreateListItems(TRAILING_ITEM_COUNT);
        CreateDone();
        MockAnimationManager::GetInstance().SetTicks(PREMEASURE_ANIMATION_TICKS);
        return builder;
    }

    void SetBoundaryInsets(Axis axis)
    {
        auto property = ViewStackProcessor::GetInstance()->GetMainFrameNode()->GetLayoutProperty();
        MarginProperty margin;
        PaddingProperty padding;
        if (axis == Axis::VERTICAL) {
            margin.top = CalcLength(11.0f);
            margin.bottom = CalcLength(17.0f);
            padding.top = CalcLength(13.0f);
            padding.bottom = CalcLength(19.0f);
        } else {
            margin.left = CalcLength(11.0f);
            margin.right = CalcLength(17.0f);
            padding.left = CalcLength(13.0f);
            padding.right = CalcLength(19.0f);
        }
        property->UpdateMargin(margin);
        property->UpdatePadding(padding);
    }

    void ExpectSuffix(float extraOffset)
    {
        constexpr float sizes[] = { 200.0f, 600.0f, 300.0f, 500.0f };
        const auto [axis, direction, stackFromEnd, decorated] = GetParam();
        const float viewport = pattern_->contentMainSize_;
        const bool reverse = axis == Axis::HORIZONTAL && direction == TextDirection::RTL;
        const float trailingInset = reverse ? 11.0f + 13.0f : 17.0f + 19.0f;
        float suffix = decorated ? 70.0f + trailingInset : 0.0f;
        int32_t checked = 0;
        for (int32_t index = INNER_ITEM_COUNT - 1; index >= SUFFIX_START_INDEX; --index) {
            const float size = sizes[index % 4];
            suffix += size;
            const float start = viewport - extraOffset - suffix;
            if (start < viewport && start + size > 0.0f) {
                const auto rect = pattern_->GetItemRectInGroup(80, index);
                EXPECT_NEAR(axis == Axis::VERTICAL ? rect.Height() : rect.Width(), size, 0.1f) << index;
                EXPECT_NEAR(axis == Axis::VERTICAL ? rect.Top() : rect.Left(),
                    reverse ? viewport - start - size : start, 1.0f) << index;
                ++checked;
            }
            suffix += decorated ? SPACE : 0.0f;
        }
        EXPECT_GT(checked, 0);
    }
};

/**
 * @tc.name: FastScrollUnevenGroupBoundaryRoundTrip001
 * @tc.desc: Real suffix geometry survives repeated END offsets, both axes, RTL and stack-from-end.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupBoundaryTestNg, FastScrollUnevenGroupBoundaryRoundTrip001, TestSize.Level1)
{
    auto builder = CreateBoundaryGroup();
    for (float extraOffset : { -2100.0f, -900.0f, -2100.0f }) {
        SCOPED_TRACE(testing::Message() << "extraOffset=" << extraOffset);
        pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::END, extraOffset,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        DriveFastScroll();
        ASSERT_FALSE(pattern_->GetFastScrollRequest());
        ExpectSuffix(extraOffset);
        EXPECT_EQ(std::count_if(builder->GetCreatedIndices().begin(), builder->GetCreatedIndices().end(),
            [](int32_t index) { return index >= 100 && index < 900; }), 0)
            << testing::PrintToString(builder->GetCreatedIndices());
    }
}

INSTANTIATE_TEST_SUITE_P(Direction, ListFastScrollGroupBoundaryTestNg,
    testing::Combine(testing::Values(Axis::VERTICAL, Axis::HORIZONTAL),
        testing::Values(TextDirection::LTR, TextDirection::RTL), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollLazySuffixPreparationMeasuresBody001
 * @tc.desc: Suffix preparation measures real body nodes without displaying them or replacing the current viewport.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollLazySuffixPreparationMeasuresBody001, TestSize.Level1)
{
    CreateUnevenLazyGrid(false);
    auto first = builder_->GetBuiltItem(0);
    ASSERT_NE(first, nullptr);
    const auto firstRect = first->GetGeometryNode()->GetFrameRect();
    const auto info = grid_->GetPattern<LazyGridLayoutPattern>()->layoutInfo_;
    const auto visible = std::make_pair(info->visibleStartIndex_, info->visibleEndIndex_);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::END, -2100.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_TRUE(pattern_->GetFastScrollRequest());
    ASSERT_EQ(pattern_->GetFastScrollPreparation()->preparation, FastScrollPreparation::READY);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    EXPECT_EQ(std::make_pair(info->visibleStartIndex_, info->visibleEndIndex_), visible);
    EXPECT_EQ(first->GetGeometryNode()->GetFrameRect(), firstRect);
    for (int32_t index = INNER_ITEM_COUNT - 5; index < INNER_ITEM_COUNT; ++index) {
        auto item = builder_->GetBuiltItem(index);
        ASSERT_NE(item, nullptr) << "prepared suffix item=" << index;
        EXPECT_FALSE(item->IsActive()) << "prepared suffix item=" << index;
        EXPECT_NEAR(item->GetGeometryNode()->GetFrameSize().Height(), TALL_ITEM_SIZE, 0.1f) << "item=" << index;
        EXPECT_TRUE(item->GetGeometryNode()->GetParentLayoutConstraint().has_value()) << "item=" << index;
    }
    DriveFastScroll(grid_, builder_);
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_FALSE(info->NeedPredict());
    ExpectLazyItemAt(INNER_ITEM_COUNT - 5, 0.0f);
}

class ListLazyGridStraddlingTestNg : public ListFastScrollLazyGridTestNg,
    public testing::WithParamInterface<bool> {};

/**
 * @tc.name: FastScrollLazyStartOffsetStraddlesPreviousItem
 * @tc.desc: A START offset keeps both the preceding ordinary item and the lazy prefix at their real coordinates.
 * @tc.type: FUNC
 */
HWTEST_P(ListLazyGridStraddlingTestNg, FastScrollLazyStartOffsetStraddlesPreviousItem, TestSize.Level1)
{
    CreateUnevenLazyGrid(GetParam(), 2);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(2, true, ScrollAlign::START, -50.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll(grid_, builder_);
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    auto previous = GetChildFrameNode(frameNode_, 1);
    ASSERT_NE(previous, nullptr);
    EXPECT_TRUE(previous->IsActive());
    EXPECT_NEAR(previous->GetGeometryNode()->GetFrameRect().Top(), -50.0f, 0.1f);
    EXPECT_NEAR(previous->GetGeometryNode()->GetFrameRect().Bottom(), 50.0f, 0.1f);
    for (int32_t index = 0; index < 4; ++index) {
        ExpectLazyItemAt(index, 50.0f + index * ITEM_MAIN_SIZE);
    }
}

/**
 * @tc.name: FastScrollLazyEndOffsetStraddlesFollowingItems
 * @tc.desc: An END offset keeps the real lazy suffix and the following ordinary items in the same viewport.
 * @tc.type: FUNC
 */
HWTEST_P(ListLazyGridStraddlingTestNg, FastScrollLazyEndOffsetStraddlesFollowingItems, TestSize.Level1)
{
    CreateUnevenLazyGrid(GetParam(), 2);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(2, true, ScrollAlign::END, 150.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll(grid_, builder_);
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    ExpectLazyItemAt(INNER_ITEM_COUNT - 1, HEIGHT - 150.0f - TALL_ITEM_SIZE);
    for (int32_t index = 0; index < 2; ++index) {
        auto following = GetChildFrameNode(frameNode_, 3 + index);
        ASSERT_NE(following, nullptr);
        EXPECT_TRUE(following->IsActive());
        EXPECT_NEAR(following->GetGeometryNode()->GetFrameRect().Top(),
            HEIGHT - 150.0f + index * ITEM_MAIN_SIZE, 0.1f);
        EXPECT_NEAR(following->GetGeometryNode()->GetFrameSize().Height(), ITEM_MAIN_SIZE, 0.1f);
    }
}

INSTANTIATE_TEST_SUITE_P(OuterSize, ListLazyGridStraddlingTestNg, testing::Bool());

/**
 * @tc.name: FastScrollLazyChildCompletesWithPendingCache001
 * @tc.desc: A reached target finishes without waiting for its own or a neighboring lazy cache to be filled.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollLazyChildCompletesWithPendingCache001, TestSize.Level1)
{
    CreateUnevenLazyGrid();
    auto gridPattern = grid_->GetPattern<LazyGridLayoutPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto info = gridPattern->layoutInfo_;
    ASSERT_NE(info, nullptr);
    info->totalItemCount_ = INNER_ITEM_COUNT;
    info->layoutedEndIndex_ = 0;
    info->layoutedEnd_ = 0.0f;
    info->cacheEndPos_ = ITEM_MAIN_SIZE;
    auto lazyItem = pattern_->itemPosition_.at(0);
    pattern_->itemPosition_.at(0).isLazyChild = false;
    pattern_->itemPosition_[1] = lazyItem;

    ASSERT_TRUE(info->NeedPredict());
    pattern_->InitializeFastScrollRequest(0, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    pattern_->GetOrCreateFastScrollPreparation().preparation = FastScrollPreparation::READY;
    EXPECT_TRUE(pattern_->HandleFastScrollTargetReached(pattern_->currentOffset_));
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(info->NeedPredict());

    pattern_->itemPosition_.erase(1);
    pattern_->itemPosition_.at(0) = lazyItem;
    pattern_->InitializeFastScrollRequest(0, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    pattern_->GetOrCreateFastScrollPreparation().preparation = FastScrollPreparation::READY;
    EXPECT_TRUE(pattern_->HandleFastScrollTargetReached(pattern_->currentOffset_));
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    EXPECT_TRUE(info->NeedPredict());
}

/**
 * @tc.name: FastScrollLayoutRebaseKeepsCompressedSegment001
 * @tc.desc: Estimation rebasing must not restart an active compressed animation segment.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollLayoutRebaseKeepsCompressedSegment001, TestSize.Level1)
{
    CreateList();
    CreateListItems(TOTAL_ITEM_NUMBER);
    CreateDone();
    pattern_->InitializeFastScrollRequest(TOTAL_ITEM_NUMBER - 1, ScrollAlign::END, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->EstimateFastScrollPositions());
    pattern_->StartFastScrollAnimation(TOTAL_ITEM_NUMBER * ITEM_MAIN_SIZE - HEIGHT);
    pattern_->currentOffset_ = ITEM_MAIN_SIZE;

    pattern_->CheckAndUpdateAnimateTo(0.0f, 0.0f);

    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_NEAR(pattern_->GetFastScrollRequest()->targetOffset,
        TOTAL_ITEM_NUMBER * ITEM_MAIN_SIZE - HEIGHT, 0.1f);
}

/**
 * @tc.name: FastScrollRetainedAnimationRequeues001
 * @tc.desc: Restarting a property after its finished animation was pruned must drive the new segment.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollRetainedAnimationRequeues001, TestSize.Level1)
{
    auto& manager = MockAnimationManager::GetInstance();
    auto property = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    manager.SetTicks(1);
    manager.OpenAnimation();
    manager.AddActiveProp(property);
    const auto firstHandles = manager.CloseAnimation();
    ASSERT_EQ(firstHandles.size(), 1);
    manager.Tick();
    ASSERT_TRUE(manager.AllFinished());
    manager.OpenAnimation();
    manager.AddActiveProp(property);
    const auto nextHandles = manager.CloseAnimation();
    EXPECT_EQ(nextHandles.size(), 1);
    EXPECT_FALSE(manager.AllFinished());
    EXPECT_FALSE(firstHandles.front()->Finished());
    manager.Tick();
    EXPECT_TRUE(firstHandles.front()->Finished());
    EXPECT_TRUE(manager.AllFinished());
}

class ListFastScrollGroupTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<bool> {
protected:
    RefPtr<FastScrollInnerBuilder> CreateSpacedGroup()
    {
        auto model = CreateList();
        if (GetParam()) {
            auto sizes = model.GetOrCreateListChildrenMainSize();
            sizes->UpdateDefaultSize(ITEM_MAIN_SIZE);
            sizes->ChangeData(0, 1, { INNER_ITEM_COUNT * ITEM_MAIN_SIZE + (INNER_ITEM_COUNT - 1) * SPACE });
        }
        auto group = CreateListItemGroup();
        group.SetSpace(Dimension(SPACE));
        if (GetParam()) {
            group.GetOrCreateListChildrenMainSize()->UpdateDefaultSize(ITEM_MAIN_SIZE);
        }
        auto builder = AceType::MakeRefPtr<FastScrollInnerBuilder>();
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(builder));
        stack->Pop();
        stack->StopGetAccessRecording();
        stack->Pop();
        stack->StopGetAccessRecording();
        CreateListItems(TRAILING_ITEM_COUNT);
        CreateDone();
        return builder;
    }

    int32_t CountMiddleItems(const RefPtr<FastScrollInnerBuilder>& builder) const
    {
        const auto& created = builder->GetCreatedIndices();
        return std::count_if(created.begin(), created.end(), [](int32_t index) {
            return index >= MIDDLE_START_INDEX && index < SUFFIX_START_INDEX;
        });
    }
};

/**
 * @tc.name: FastScrollGroupEndSkipsInnerMiddle001
 * @tc.desc: A fast END request skips inner lazy items while aligning the actual last child.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupTestNg, FastScrollGroupEndSkipsInnerMiddle001, TestSize.Level1)
{
    const auto builder = CreateLargeLazyGroup(GetParam(), 0, GetParam());
    ASSERT_EQ(builder->GetCreatedIndices().count(0), 1);
    ASSERT_EQ(builder->GetCreatedIndices().count(INNER_ITEM_COUNT - 1), 0);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::END, std::nullopt, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    const auto lastItem = pattern_->GetItemRectInGroup(0, INNER_ITEM_COUNT - 1);
    EXPECT_NEAR(lastItem.Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_NEAR(lastItem.Bottom(), pattern_->contentMainSize_, 0.1f);
    EXPECT_EQ(builder->GetCreatedIndices().count(INNER_ITEM_COUNT - 1), 1);
    const auto& created = builder->GetCreatedIndices();
    const auto middleCount = std::count_if(created.begin(), created.end(), [](int32_t index) {
        return index >= 100 && index < 900;
    });
    EXPECT_EQ(middleCount, 0) << "Intermediate group children were created during the skipped interval";
}

/**
 * @tc.name: FastScrollGroupStartSkipsInnerMiddle001
 * @tc.desc: A backward fast request uses the group end reference without creating skipped children.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupTestNg, FastScrollGroupStartSkipsInnerMiddle001, TestSize.Level1)
{
    const auto builder = CreateLargeLazyGroup(GetParam(), 0, GetParam());
    pattern_->ScrollToItemInGroup(0, INNER_ITEM_COUNT - 1, false, ScrollAlign::END);
    FlushUITasks();
    ASSERT_NEAR(pattern_->GetItemRectInGroup(0, INNER_ITEM_COUNT - 1).Bottom(), pattern_->contentMainSize_, 0.1f);
    ASSERT_EQ(CountMiddleItems(builder), 0);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    EXPECT_FALSE(pattern_->GetFastScrollRequest());
    const auto firstItem = pattern_->GetItemRectInGroup(0, 0);
    EXPECT_NEAR(firstItem.Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_NEAR(firstItem.Top(), 0.0f, 0.1f);
    EXPECT_EQ(CountMiddleItems(builder), 0);
}

/**
 * @tc.name: FastScrollGroupSpacedRoundTrip001
 * @tc.desc: Forward and backward window anchors include group spacing and retain the correct edge rows.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupTestNg, FastScrollGroupSpacedRoundTrip001, TestSize.Level1)
{
    const auto builder = CreateSpacedGroup();
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::END, std::nullopt, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    const auto lastItem = pattern_->GetItemRectInGroup(0, INNER_ITEM_COUNT - 1);
    EXPECT_NEAR(lastItem.Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_NEAR(lastItem.Bottom(), pattern_->contentMainSize_, 0.1f);
    EXPECT_EQ(CountMiddleItems(builder), 0);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    const auto firstItem = pattern_->GetItemRectInGroup(0, 0);
    EXPECT_NEAR(firstItem.Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_NEAR(firstItem.Top(), 0.0f, 0.1f);
    EXPECT_EQ(CountMiddleItems(builder), 0);
}

/**
 * @tc.name: FastScrollGroupSmallDistance001
 * @tc.desc: A sub-viewport request retains normal group layout with known and unknown sizes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupTestNg, FastScrollGroupSmallDistance001, TestSize.Level1)
{
    const auto builder = CreateSpacedGroup();
    pattern_->ScrollToItemInGroup(0, 2, false, ScrollAlign::START);
    FlushUITasks();
    ASSERT_NEAR(pattern_->GetItemRectInGroup(0, 2).Top(), 0.0f, 0.1f);
    ASSERT_LT(pattern_->GetTotalOffset(), pattern_->contentMainSize_);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::START, std::nullopt,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    EXPECT_NEAR(pattern_->GetItemRectInGroup(0, 0).Top(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRectInGroup(0, 0).Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_EQ(CountMiddleItems(builder), 0);
}

/**
 * @tc.name: BackToTopGroupKeepsLegacyTraversal001
 * @tc.desc: BackToTop must not acquire the new known-size fast-scroll group optimization.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollGroupTestNg, BackToTopGroupKeepsLegacyTraversal001, TestSize.Level1)
{
    const auto builder = CreateLargeLazyGroup(GetParam());
    pattern_->ScrollToItemInGroup(0, INNER_ITEM_COUNT - 1, false, ScrollAlign::END);
    FlushUITasks();
    ASSERT_EQ(CountMiddleItems(builder), 0);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->isBackToTopRunning_ = true;
    pattern_->StartBackToTopAnimation();
    DriveFastScroll();
    EXPECT_NEAR(pattern_->GetItemRectInGroup(0, 0).Top(), 0.0f, 0.1f);
    EXPECT_NEAR(pattern_->GetItemRectInGroup(0, 0).Height(), ITEM_MAIN_SIZE, 0.1f);
    EXPECT_EQ(CountMiddleItems(builder), GetParam() ? 800 : 0);
}

INSTANTIATE_TEST_SUITE_P(InnerSize, ListFastScrollGroupTestNg, testing::Bool());

class FastScrollStackGroupBuilder final : public ListItemMockLazy {
public:
    explicit FastScrollStackGroupBuilder(bool uneven)
        : ListItemMockLazy(INNER_ITEM_COUNT, ITEM_MAIN_SIZE), uneven_(uneven) {}

    float GetItemSize(int32_t index) const
    {
        return uneven_ && index >= INNER_ITEM_COUNT / HEIGHT_SECTION_COUNT ?
            HEIGHT_SECTION_COUNT * ITEM_MAIN_SIZE : ITEM_MAIN_SIZE;
    }

    int32_t CountMiddleItems() const
    {
        return std::count_if(createdIndices_.begin(), createdIndices_.end(), [](int32_t index) {
            return index >= MIDDLE_START_INDEX && index < SUFFIX_START_INDEX;
        });
    }

protected:
    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        createdIndices_.insert(index);
        ListItemModelNG model;
        model.Create();
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(GetItemSize(index)));
        return { std::to_string(index), ViewStackProcessor::GetInstance()->Finish() };
    }

private:
    bool uneven_ = false;
    std::set<int32_t> createdIndices_;
};

class ListFastScrollStackGroupTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<bool, bool>> {
protected:
    void CheckGroupExtraOffset(bool knownOuterSize);
    void VerifyGroupExtraOffsetGeometry(ScrollAlign align, float extraOffset);

    void CreateStackGroup(bool knownOuterSize = true, bool boxed = false)
    {
        const auto [known, uneven] = GetParam();
        auto model = CreateList();
        model.SetStackFromEnd(true);
        model.SetCachedCount(0);
        builder_ = AceType::MakeRefPtr<FastScrollStackGroupBuilder>(uneven);
        std::vector<float> itemSizes;
        float groupSize = headerSize + footerSize + (INNER_ITEM_COUNT - 1) * SPACE;
        for (int32_t index = 0; index < INNER_ITEM_COUNT; ++index) {
            itemSizes.emplace_back(builder_->GetItemSize(index));
            groupSize += itemSizes.back();
        }
        if (boxed) {
            groupSize += topMargin + bottomMargin + topPadding + bottomPadding;
        }
        if (knownOuterSize) {
            model.GetOrCreateListChildrenMainSize()->ChangeData(0, 1, { groupSize });
        }
        auto group = CreateListItemGroup();
        group.SetSpace(Dimension(SPACE));
        group.SetHeader(GetRowOrColBuilder(FILL_LENGTH, Dimension(headerSize)));
        group.SetFooter(GetRowOrColBuilder(FILL_LENGTH, Dimension(footerSize)));
        if (boxed) {
            SetGroupInsets();
        }
        if (known) {
            group.GetOrCreateListChildrenMainSize()->ChangeData(0, INNER_ITEM_COUNT, itemSizes);
        }
        auto stack = ViewStackProcessor::GetInstance();
        stack->StartGetAccessRecordingFor(GetElmtId());
        LazyForEachModelNG lazyModel;
        lazyModel.Create(RefPtr<LazyForEachBuilder>(builder_));
        stack->Pop();
        stack->StopGetAccessRecording();
        stack->Pop();
        stack->StopGetAccessRecording();
        CreateDone();
        MockAnimationManager::GetInstance().SetTicks(PREMEASURE_ANIMATION_TICKS);
    }

    void ScrollToGroupEdge(ScrollAlign align, float extraOffset = 0.0f)
    {
        pattern_->ScrollToIndexFastScroll(0, true, align, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
        DriveFastScroll();
        EXPECT_FALSE(pattern_->GetFastScrollRequest());
        EXPECT_EQ(builder_->CountMiddleItems(), 0);
    }

    void ExpectGroupEdge(ScrollAlign align, bool boxed = false)
    {
        const bool start = align == ScrollAlign::START;
        const int32_t index = start ? 0 : INNER_ITEM_COUNT - 1;
        const auto item = pattern_->GetItemRectInGroup(0, index);
        const float startInset = boxed ? topMargin + topPadding : 0.0f;
        const float endInset = boxed ? bottomMargin + bottomPadding : 0.0f;
        EXPECT_NEAR(item.Height(), builder_->GetItemSize(index), 0.1f);
        EXPECT_NEAR(start ? item.Top() : item.Bottom(),
            start ? headerSize + startInset : pattern_->contentMainSize_ - footerSize - endInset, 0.1f);
    }

    void SetGroupInsets()
    {
        auto property = ViewStackProcessor::GetInstance()->GetMainFrameNode()->GetLayoutProperty();
        MarginProperty margin;
        margin.top = CalcLength(topMargin);
        margin.bottom = CalcLength(bottomMargin);
        property->UpdateMargin(margin);
        PaddingProperty padding;
        padding.top = CalcLength(topPadding);
        padding.bottom = CalcLength(bottomPadding);
        property->UpdatePadding(padding);
    }

    void CheckBoxedGroup(bool knownOuterSize)
    {
        CreateStackGroup(knownOuterSize, true);
        ASSERT_EQ(builder_->CountMiddleItems(), 0);
        for (auto align : { ScrollAlign::START, ScrollAlign::END, ScrollAlign::START }) {
            SCOPED_TRACE(testing::Message() << "align=" << static_cast<int32_t>(align));
            ScrollToGroupEdge(align);
            ExpectGroupEdge(align, true);
        }
    }

    static constexpr float headerSize = 30.0f;
    static constexpr float footerSize = 70.0f;
    static constexpr float topMargin = 11.0f;
    static constexpr float bottomMargin = 17.0f;
    static constexpr float topPadding = 13.0f;
    static constexpr float bottomPadding = 19.0f;
    RefPtr<FastScrollStackGroupBuilder> builder_;
};

/**
 * @tc.name: FastScrollStackGroupRoundTrip001
 * @tc.desc: Stack-from-end skips both directions using physical item indices and unequal header/footer sizes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollStackGroupTestNg, FastScrollStackGroupRoundTrip001, TestSize.Level1)
{
    CreateStackGroup();
    ASSERT_EQ(builder_->CountMiddleItems(), 0);
    ScrollToGroupEdge(ScrollAlign::START);
    ExpectGroupEdge(ScrollAlign::START);
    ScrollToGroupEdge(ScrollAlign::END);
    ExpectGroupEdge(ScrollAlign::END);
    ScrollToGroupEdge(ScrollAlign::START);
    ExpectGroupEdge(ScrollAlign::START);
}

/**
 * @tc.name: FastScrollStackGroupRowBoundary001
 * @tc.desc: Mirrored row boundaries and spacing retain the actual first visible physical row.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollStackGroupTestNg, FastScrollStackGroupRowBoundary001, TestSize.Level1)
{
    CreateStackGroup();
    const float firstRowEnd = headerSize + builder_->GetItemSize(0);
    for (float gapOffset : { 0.0f, SPACE / 2, SPACE }) {
        SCOPED_TRACE(testing::Message() << "gapOffset=" << gapOffset);
        ScrollToGroupEdge(ScrollAlign::START, firstRowEnd + gapOffset);
        const auto second = pattern_->GetItemRectInGroup(0, 1);
        EXPECT_NEAR(second.Height(), builder_->GetItemSize(1), 0.1f);
        EXPECT_NEAR(second.Top(), SPACE - gapOffset, 0.1f);
        ScrollToGroupEdge(ScrollAlign::END);
        ExpectGroupEdge(ScrollAlign::END);
    }
}

/**
 * @tc.name: FastScrollStackGroupInsets001
 * @tc.desc: Supplied outer size includes asymmetric group margin, padding, header and footer.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollStackGroupTestNg, FastScrollStackGroupInsets001, TestSize.Level1)
{
    CheckBoxedGroup(true);
}

INSTANTIATE_TEST_SUITE_P(InnerSize, ListFastScrollStackGroupTestNg,
    testing::Values(std::make_tuple(true, false), std::make_tuple(true, true)));

class ListFastScrollUnknownOuterGroupTestNg : public ListFastScrollStackGroupTestNg {};

/**
 * @tc.name: FastScrollUnknownOuterStackGroupRoundTrip001
 * @tc.desc: Without outer supplied sizes, stack Group START/END retains actual geometry and skips the middle.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollUnknownOuterGroupTestNg, FastScrollUnknownOuterStackGroupRoundTrip001, TestSize.Level1)
{
    CreateStackGroup(false);
    ASSERT_EQ(pattern_->childrenSize_, nullptr);
    ASSERT_EQ(builder_->CountMiddleItems(), 0);
    for (auto align : { ScrollAlign::START, ScrollAlign::END, ScrollAlign::START }) {
        SCOPED_TRACE(testing::Message() << "align=" << static_cast<int32_t>(align));
        ScrollToGroupEdge(align);
        ExpectGroupEdge(align);
    }
}

/**
 * @tc.name: FastScrollUnknownOuterStackGroupInsets001
 * @tc.desc: Estimated outer size applies asymmetric group insets once in START and END coordinates.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollUnknownOuterGroupTestNg, FastScrollUnknownOuterStackGroupInsets001, TestSize.Level1)
{
    CheckBoxedGroup(false);
}

/**
 * @tc.name: FastScrollGroupExtraOffsetBoundary001
 * @tc.desc: An unknown outer group retains the real offset window with reverse layout and unequal padded items.
 * @tc.type: FUNC
 */
void ListFastScrollStackGroupTestNg::CheckGroupExtraOffset(bool knownOuterSize)
{
    CreateStackGroup(knownOuterSize, true);
    VerifyGroupExtraOffsetGeometry(ScrollAlign::START, 900.0f);
    VerifyGroupExtraOffsetGeometry(ScrollAlign::END, -900.0f);
}

void ListFastScrollStackGroupTestNg::VerifyGroupExtraOffsetGeometry(ScrollAlign align, float extraOffset)
{
    const float leading = topMargin + topPadding + headerSize;
    const float trailing = bottomMargin + bottomPadding + footerSize;
    float groupSize = leading + trailing + (INNER_ITEM_COUNT - 1) * SPACE;
    for (int32_t index = 0; index < INNER_ITEM_COUNT; ++index) {
        groupSize += builder_->GetItemSize(index);
    }
    const float windowStart = (align == ScrollAlign::START ? 0.0f : groupSize - HEIGHT) + extraOffset;
    pattern_->ScrollToIndexFastScroll(0, true, align, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    EXPECT_EQ(builder_->CountMiddleItems(), 0) << "Preparing a new target must retain the current group window";
    DriveFastScroll();
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    auto group = frameNode_->GetChildByIndex(0);
    ASSERT_NE(group, nullptr);
    auto groupPattern = group->GetHostNode()->GetPattern<ListItemGroupPattern>();
    ASSERT_NE(groupPattern, nullptr);
    ASSERT_FALSE(groupPattern->itemPosition_.empty());
    SCOPED_TRACE(testing::Message() << "align=" << static_cast<int32_t>(align) << ", window=" << windowStart
        << ", offset=" << pattern_->GetTotalOffset() << ", group=" << pattern_->GetItemRect(0).ToString()
        << ", header=" << groupPattern->headerMainSize_ << ", footer=" << groupPattern->footerMainSize_
        << ", first=" << groupPattern->itemPosition_.begin()->first << ":"
        << groupPattern->itemPosition_.begin()->second.startPos);
    float position = leading;
    for (int32_t index = 0; index < INNER_ITEM_COUNT; ++index) {
        const float top = position - windowStart;
        const float height = builder_->GetItemSize(index);
        if (top < HEIGHT && top + height > 0.0f) {
            const auto rect = pattern_->GetItemRectInGroup(0, index);
            EXPECT_NEAR(rect.Top(), top, 0.1f) << "inner item=" << index;
            EXPECT_NEAR(rect.Height(), height, 0.1f);
        }
        position += height + SPACE;
    }
    EXPECT_EQ(builder_->CountMiddleItems(), 0);
}

HWTEST_P(ListFastScrollUnknownOuterGroupTestNg, FastScrollGroupExtraOffsetBoundary001, TestSize.Level1)
{
    CheckGroupExtraOffset(false);
}

/**
 * @tc.name: FastScrollGroupExtraOffsetBeyondRetainedWindow001
 * @tc.desc: A large END offset reaches the real inner window beyond the retained animation screens.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollUnknownOuterGroupTestNg, FastScrollGroupExtraOffsetBeyondRetainedWindow001, TestSize.Level1)
{
    CreateStackGroup(false, true);
    VerifyGroupExtraOffsetGeometry(ScrollAlign::END, -2100.0f);
}

/**
 * @tc.name: FastScrollKnownGroupExtraOffsetBoundary001
 * @tc.desc: Supplied outer sizes retain the same real offset window as the measured-size path.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollStackGroupTestNg, FastScrollKnownGroupExtraOffsetBoundary001, TestSize.Level1)
{
    CheckGroupExtraOffset(true);
}

/**
 * @tc.name: FastScrollKnownGroupCenterUsesWindowLayout001
 * @tc.desc: Exact outer and inner sizes start immediately and only lay out the visible center window.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollStackGroupTestNg, FastScrollKnownGroupCenterUsesWindowLayout001, TestSize.Level1)
{
    CreateStackGroup();
    pattern_->ScrollToIndexFastScroll(0, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->AnimateRunning());
    EXPECT_EQ(builder_->CountMiddleItems(), 0);
    auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    DriveFastScroll();
    EXPECT_NEAR(pattern_->GetTotalOffset(),
        (pattern_->childrenSize_->GetChildSize(0, false) - HEIGHT) / 2.0f, 0.1f);
    EXPECT_GT(builder_->CountMiddleItems(), 0);
    EXPECT_LT(builder_->CountMiddleItems(), 40);
    algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm());
    ASSERT_NE(algorithm, nullptr);
    EXPECT_FALSE(algorithm->GetOrCreateFastScrollState().input.useKnownMainSize);
}

class ListFastScrollOffsetCrossGroupTestNg : public ListFastScrollLayoutTestNg,
    public testing::WithParamInterface<std::tuple<int32_t, ScrollAlign, float>> {};

/**
 * @tc.name: FastScrollExtraOffsetCrossesGroup001
 * @tc.desc: The final window can lie before, after or inside a group while the requested item is offscreen.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollOffsetCrossGroupTestNg, FastScrollExtraOffsetCrossesGroup001, TestSize.Level1)
{
    const auto [target, align, extraOffset] = GetParam();
    CreateList();
    CreateListItems(20);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 30);
    CreateListItems(80);
    CreateDone();
    const float targetStart = (target > 20 ? target + 29 : target) * ITEM_MAIN_SIZE;
    const float targetSize = (target == 20 ? 30 : 1) * ITEM_MAIN_SIZE;
    float destination = targetStart + extraOffset;
    if (align == ScrollAlign::END) {
        destination += targetSize - HEIGHT;
    } else if (align == ScrollAlign::CENTER) {
        destination += (targetSize - HEIGHT) / 2.0f;
    }
    destination = std::clamp(destination, 0.0f, 130 * ITEM_MAIN_SIZE - HEIGHT);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(target, true, align, extraOffset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    DriveFastScroll();
    ASSERT_FALSE(pattern_->GetFastScrollRequest());
    for (int32_t item = 0; item < 130; ++item) {
        const float top = item * ITEM_MAIN_SIZE - destination;
        if (top >= HEIGHT || top + ITEM_MAIN_SIZE <= 0.0f) {
            continue;
        }
        const auto rect = item >= 20 && item < 50 ? pattern_->GetItemRectInGroup(20, item - 20) :
            pattern_->GetItemRect(item < 20 ? item : item - 29);
        EXPECT_NEAR(rect.Top(), top, 0.1f) << "flat item=" << item;
        EXPECT_NEAR(rect.Height(), ITEM_MAIN_SIZE, 0.1f);
    }
}

INSTANTIATE_TEST_SUITE_P(CrossGroup, ListFastScrollOffsetCrossGroupTestNg,
    testing::Values(std::make_tuple(20, ScrollAlign::START, 3500.0f),
        std::make_tuple(20, ScrollAlign::START, -1500.0f), std::make_tuple(20, ScrollAlign::END, -3500.0f),
        std::make_tuple(20, ScrollAlign::CENTER, 3000.0f), std::make_tuple(21, ScrollAlign::START, -2000.0f),
        std::make_tuple(19, ScrollAlign::START, 2000.0f)));

INSTANTIATE_TEST_SUITE_P(OuterUnknown, ListFastScrollUnknownOuterGroupTestNg,
    testing::Values(std::make_tuple(false, false), std::make_tuple(true, false), std::make_tuple(true, true),
        std::make_tuple(false, true)));

/**
 * @tc.name: FastScrollNeighborSuffixSummaryAfterInvalidation001
 * @tc.desc: Repreparing a widened uneven suffix must not merge coordinates from different size estimates.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollNeighborSuffixSummaryAfterInvalidation001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false, true);
    auto group = GetChildFrameNode(frameNode_, 80);
    ASSERT_NE(group, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->SetFastScrollTarget(81, ScrollAlign::START);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 80, 2100.0f, false));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 6u);
    ASSERT_FLOAT_EQ(groupAlgorithm->totalMainSize_, 400000.0f);

    // The animation reserve exposes row 993 without changing the prepared total size.
    ASSERT_EQ(algorithm->GetFastScrollPreparedLayouts()->count(80), 1u);
    algorithm->SetListItemGroupParam(group, 80, -397400.0f, true, layoutProperty_, false);
    group->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    group->Measure(algorithm->childLayoutConstraint_);
    ASSERT_TRUE(groupAlgorithm->layoutedItemInfo_);
    ASSERT_EQ(groupAlgorithm->layoutedItemInfo_->startIndex, 993);
    ASSERT_EQ(groupAlgorithm->layoutedItemInfo_->endIndex, 999);
    ASSERT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->startPos, 397000.0f);
    ASSERT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->endPos, 400000.0f);

    // Discard request-local preparation, then verify that a later suffix preparation keeps its measured origin.
    algorithm->GetOrCreateFastScrollState().preparedLayouts.reset();
    for (int32_t attempt = 0; attempt < 3; ++attempt) {
        SCOPED_TRACE(attempt);
        group->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 80, 2100.0f, false));
        EXPECT_FLOAT_EQ(groupAlgorithm->totalMainSize_, 400000.0f);
        EXPECT_FLOAT_EQ(group->GetGeometryNode()->GetFrameSize().Height(), 400000.0f);
        EXPECT_FLOAT_EQ(groupAlgorithm->GetLayoutInfo().averageHeight, 400.0f);
        ASSERT_EQ(groupAlgorithm->GetItemPosition().count(995), 1u);
        EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().at(995).startPos, 397900.0f);
        EXPECT_FLOAT_EQ(groupAlgorithm->GetItemPosition().rbegin()->second.endPos, 400000.0f);
        EXPECT_EQ(builder->GetCreatedIndices().size(), 7u);
        EXPECT_EQ(*builder->GetCreatedIndices().begin(), 993);
    }
}

/**
 * @tc.name: FastScrollSuffixKeepsUnchangedSummary001
 * @tc.desc: A suffix remeasure keeps complete size knowledge when its coordinate origin has not changed.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollSuffixKeepsUnchangedSummary001, TestSize.Level1)
{
    CreateList();
    CreateListItems(8);
    CreateListItemGroups(1, V2::ListItemGroupStyle::NONE, 10);
    CreateDone();
    auto group = GetChildFrameNode(frameNode_, 8);
    ASSERT_NE(group, nullptr);
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    algorithm->SetFastScrollTarget(9, ScrollAlign::START);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 2000.0f, false));
    auto groupAlgorithm = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(
        group->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(groupAlgorithm, nullptr);
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 10u);
    group->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    ASSERT_TRUE(algorithm->PremeasureFastScrollGroup(group, 8, 50.0f, false));
    ASSERT_EQ(groupAlgorithm->GetItemPosition().size(), 1u);
    ASSERT_TRUE(groupAlgorithm->layoutedItemInfo_);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->startIndex, 0);
    EXPECT_EQ(groupAlgorithm->layoutedItemInfo_->endIndex, 9);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->startPos, 0.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->layoutedItemInfo_->endPos, 1000.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->GetLayoutInfo().averageHeight, 100.0f);
    EXPECT_FLOAT_EQ(groupAlgorithm->totalMainSize_, 1000.0f);
}

/**
 * @tc.name: FastScrollUnevenNeighborStartOffsetStaysInSuffix001
 * @tc.desc: A START offset into the previous Group's suffix keeps geometry and direction stable on every frame.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollUnevenNeighborStartOffsetStaysInSuffix001, TestSize.Level1)
{
    auto builder = CreateLargeLazyGroup(false, 80, false, true);
    MockAnimationManager::GetInstance().SetTicks(4);
    pattern_->ScrollToIndexFastScroll(81, true, ScrollAlign::START, -2100.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    int32_t previousStart = 0;
    bool finished = false;
    for (int32_t frame = 0; frame < 24; ++frame) {
        SCOPED_TRACE(frame);
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks();
        ASSERT_NO_FATAL_FAILURE(FlushFastScrollIdleTasks());
        ASSERT_FALSE(pattern_->itemPosition_.empty());
        EXPECT_GE(pattern_->GetStartIndexInItemPosition(), previousStart);
        EXPECT_LE(pattern_->GetEndIndexInItemPosition(), 81);
        previousStart = pattern_->GetStartIndexInItemPosition();
        auto group = pattern_->itemPosition_.find(80);
        if (group != pattern_->itemPosition_.end()) {
            const float estimatedSize = group->second.endPos - group->second.startPos;
            EXPECT_GE(estimatedSize, INNER_ITEM_COUNT * ITEM_MAIN_SIZE);
            EXPECT_LE(estimatedSize, INNER_ITEM_COUNT * 500.0f);
        }
        const auto& created = builder->GetCreatedIndices();
        EXPECT_EQ(std::count_if(created.begin(), created.end(), [](int32_t index) { return index < 990; }), 0);
        if (!pattern_->GetFastScrollRequest() && !pattern_->AnimateRunning() && !pattern_->scrollStop_) {
            finished = true;
            break;
        }
    }
    EXPECT_TRUE(finished);
    EXPECT_EQ(pattern_->GetStartIndexInItemPosition(), 80);
    EXPECT_EQ(pattern_->GetEndIndexInItemPosition(), 80);
    EXPECT_NEAR(pattern_->GetItemRectInGroup(80, 995).Top(), 0.0f, 0.1f);
    EXPECT_FLOAT_EQ(pattern_->GetItemRectInGroup(80, 995).Height(), 500.0f);
}
/**
 * @tc.name: FastScrollPremeasuredItemKeepsCleanupUntilLayout001
 * @tc.desc: Restarting preparation clears only pending Lazy adjustments, not measured-node ownership.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollPremeasuredItemKeepsCleanupUntilLayout001, TestSize.Level1)
{
    CreateList();
    CreateListItems(2);
    CreateDone();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    auto item = GetChildFrameNode(frameNode_, 0);
    auto child = GetChildFrameNode(frameNode_, 1);
    algorithm->TrackFastScrollPremeasuredItem(item, 0, true);
    auto& record = algorithm->GetOrCreateFastScrollState().premeasuredItems.at(0);
    record.measuredNodes.emplace_back(child);
    record.pendingLazyAdjustment = AdjustOffset { 25.0f, 50.0f };
    algorithm->PrepareFastScrollTargetWindow(AceType::RawPtr(frameNode_));
    ASSERT_EQ(algorithm->GetOrCreateFastScrollState().premeasuredItems.size(), 1);
    EXPECT_EQ(record.wrapper, item);
    EXPECT_TRUE(record.clearEntireTree);
    EXPECT_FALSE(record.pendingLazyAdjustment.has_value());
    ASSERT_EQ(record.measuredNodes.size(), 1);
    EXPECT_EQ(record.measuredNodes.front().Upgrade(), child);
    algorithm->ResetLayoutItem(AceType::RawPtr(frameNode_));
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().premeasuredItems.empty());
}

/**
 * @tc.name: FastScrollPremeasuredItemReplacesRebuiltNode001
 * @tc.desc: Re-registering one wrapper preserves cleanup; changing its host replaces frame-local state without an ID.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollPremeasuredItemReplacesRebuiltNode001, TestSize.Level1)
{
    CreateList();
    CreateListItems(2);
    CreateDone();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    auto first = GetChildFrameNode(frameNode_, 0);
    auto rebuilt = GetChildFrameNode(frameNode_, 1);
    algorithm->TrackFastScrollPremeasuredItem(first, 0, true);
    auto& record = algorithm->GetOrCreateFastScrollState().premeasuredItems.at(0);
    record.measuredNodes.emplace_back(first);
    record.pendingLazyAdjustment = AdjustOffset { 25.0f, 50.0f };
    algorithm->TrackFastScrollPremeasuredItem(first, 0, true);
    EXPECT_EQ(record.measuredNodes.size(), 1);
    EXPECT_TRUE(record.pendingLazyAdjustment.has_value());
    algorithm->TrackFastScrollPremeasuredItem(rebuilt, 0, false);
    EXPECT_EQ(record.wrapper, rebuilt);
    EXPECT_FALSE(record.clearEntireTree);
    EXPECT_TRUE(record.measuredNodes.empty());
    EXPECT_FALSE(record.pendingLazyAdjustment.has_value());
}
/**
 * @tc.name: FastScrollKnownLazySizeHintIsScoped001
 * @tc.desc: Normal Lazy layout receives no known size; fast layout consumes it even after clearing the target.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollKnownLazySizeHintIsScoped001, TestSize.Level1)
{
    CreateUnevenLazyGrid();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    ASSERT_NE(algorithm, nullptr);
    EXPECT_FALSE(algorithm->GetOrCreateFastScrollState().input.useKnownMainSize);
    float referencePos = 0.0f;
    algorithm->MeasureLazyChild(grid_, 0, referencePos, true);
    auto constraint = grid_->GetGeometryNode()->GetParentLayoutConstraint();
    ASSERT_TRUE(constraint);
    ASSERT_TRUE(constraint->viewPosRef);
    EXPECT_FALSE(constraint->viewPosRef->knownMainSize);

    algorithm->SetFastScrollTarget(0, ScrollAlign::END);
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().input.useKnownMainSize);
    algorithm->GetOrCreateFastScrollState().input.targetIndex.reset();
    EXPECT_TRUE(algorithm->GetOrCreateFastScrollState().input.useKnownMainSize);
    EXPECT_FALSE(algorithm->HasFastScrollPreparationRequest());
    algorithm->MeasureLazyChild(grid_, 0, referencePos, true);
    constraint = grid_->GetGeometryNode()->GetParentLayoutConstraint();
    ASSERT_TRUE(constraint);
    ASSERT_TRUE(constraint->viewPosRef);
    ASSERT_TRUE(constraint->viewPosRef->knownMainSize);
    const float totalSize = INNER_ITEM_COUNT / 2 * (ITEM_MAIN_SIZE + TALL_ITEM_SIZE);
    EXPECT_FLOAT_EQ(constraint->viewPosRef->knownMainSize.value(), totalSize);
    EXPECT_NEAR(grid_->GetGeometryNode()->GetFrameSize().Height(), totalSize, 0.1f);
    algorithm->SetFastScrollPreparationOffset(0.0f);
    EXPECT_TRUE(algorithm->HasFastScrollPreparationRequest());
    grid_->ClearSubtreeLayoutAlgorithm();
}

/**
 * @tc.name: FastScrollPreparationResetsUnsupportedResult001
 * @tc.desc: Starting another preparation pass clears the previous pass's unsupported result.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollPreparationResetsUnsupportedResult001, TestSize.Level1)
{
    CreateList();
    CreateListItems(2);
    CreateDone();
    auto algorithm = CreateOffsetPremeasureAlgorithm();
    algorithm->GetOrCreateFastScrollState().result.status = FastScrollMeasureStatus::UNSUPPORTED;
    algorithm->PrepareFastScrollTargetWindow(AceType::RawPtr(frameNode_));
    EXPECT_NE(algorithm->GetFastScrollMeasureResult()->status, FastScrollMeasureStatus::UNSUPPORTED);
}

/**
 * @tc.name: FastScrollSkipKeepsPreparedGroupPosition001
 * @tc.desc: An overestimated preceding item must not displace a prepared Group into an unrelated internal window.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLayoutTestNg, FastScrollSkipKeepsPreparedGroupPosition001, TestSize.Level1)
{
    CreateList();
    auto builder = AceType::MakeRefPtr<FastScrollInnerBuilder>();
    auto outer = AceType::MakeRefPtr<FastScrollOuterGroupBuilder>(builder);
    auto stack = ViewStackProcessor::GetInstance();
    stack->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazy;
    lazy.Create(RefPtr<LazyForEachBuilder>(outer));
    stack->Pop();
    stack->StopGetAccessRecording();
    CreateDone();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, 500.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    ASSERT_TRUE(Positive(pattern_->GetFastScrollRequest()->viewportSize));
    const auto target = pattern_->posMap_->GetPositionInfo(80);
    auto previous = pattern_->posMap_->GetPositionInfo(79);
    previous.mainSize = 5000.0f;
    pattern_->posMap_->UpdatePos(79, previous);
    ASSERT_GT(pattern_->posMap_->GetPositionInfo(79).mainSize, ITEM_MAIN_SIZE);
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - target.mainPos + 300.0f,
        SCROLL_FROM_ANIMATION_CONTROLLER));
    pattern_->fastScrollSkip_ = true;
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetItemRectInGroup(80, 0).Top(), 300.0f, 0.1f);
    EXPECT_EQ(std::count_if(builder->GetCreatedIndices().begin(), builder->GetCreatedIndices().end(),
        [](int32_t index) { return index >= 100 && index < 900; }), 0);
}

/**
 * @tc.name: FastScrollSkipKeepsPreparedLazyGridPosition001
 * @tc.desc: A skip entering a prepared LazyGrid keeps its global start while preceding estimates are measured.
 * @tc.type: FUNC
 */
HWTEST_F(ListFastScrollLazyGridTestNg, FastScrollSkipKeepsPreparedLazyGridPosition001, TestSize.Level1)
{
    auto outer = CreateOuterLazyGrid();
    pattern_->ScrollToIndexFastScroll(80, true, ScrollAlign::START, 500.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks();
    ASSERT_NO_FATAL_FAILURE(FlushFastScrollIdleTasks());
    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
    ASSERT_TRUE(Positive(pattern_->GetFastScrollRequest()->viewportSize));
    const auto target = pattern_->posMap_->GetPositionInfo(80);
    ASSERT_GT(pattern_->posMap_->GetPositionInfo(79).mainSize, ITEM_MAIN_SIZE);
    ASSERT_TRUE(pattern_->UpdateCurrentOffset(pattern_->GetTotalOffset() - target.mainPos + 300.0f,
        SCROLL_FROM_ANIMATION_CONTROLLER));
    pattern_->fastScrollSkip_ = true;
    FlushUITasks();
    EXPECT_NEAR(pattern_->GetItemRect(80).Top(), 300.0f, 0.1f);
    EXPECT_EQ(std::count_if(builder_->GetCreatedIndices().begin(), builder_->GetCreatedIndices().end(),
        [](int32_t index) { return index >= 100 && index < 2900; }), 0);
}

} // namespace OHOS::Ace::NG

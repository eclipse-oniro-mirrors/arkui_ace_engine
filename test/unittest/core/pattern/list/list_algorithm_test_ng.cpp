/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <optional>
#include <tuple>

#include "gtest/gtest.h"
#include "test/unittest/core/pattern/test_ng.h"
#include "test/unittest/core/syntax/mock_lazy_for_each_builder.h"

#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/list/list_item_group_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_item_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/list/list_properties.h"
#include "core/components_ng/syntax/if_else_node.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_node.h"
#include "core/common/text_field_manager_ng.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t FIRST_SEGMENT_GROUP_INDEX = 5;
constexpr int32_t SECOND_SEGMENT_GROUP_INDEX = 17;
constexpr int32_t TYPE_SCAN_LANES = 3;
constexpr int32_t TYPE_SCAN_ITEM_COUNT = 24;
constexpr int32_t SYNTAX_RANGE_ITEM_COUNT = 8;
constexpr int32_t SYNTAX_LIST_ITEM_COUNT = 13;
constexpr int32_t SYNTAX_ROW_END_DELTA = 2;
} // namespace

using namespace testing;
using namespace testing::ext;

class ListAlgorithmTestNg : public TestNG {
public:
};

/**
 * @tc.name: LayoutIndexUpdateUsesMinimumPlatformVersion001
 * @tc.desc: Index callbacks retain the minimum platform gate when it differs from the target API.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutIndexUpdateUsesMinimumPlatformVersion001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    auto container = Container::Current();
    ASSERT_NE(pipeline, nullptr);
    ASSERT_NE(container, nullptr);
    const auto oldMinimum = pipeline->GetMinPlatformVersion();
    const auto oldTarget = container->GetApiTargetVersion();
    for (const auto& versions : {
        std::make_pair(PlatformVersion::VERSION_NINE, PlatformVersion::VERSION_TWENTY_SIX),
        std::make_pair(PlatformVersion::VERSION_TEN, PlatformVersion::VERSION_NINE) }) {
        SCOPED_TRACE(static_cast<int32_t>(versions.first));
        pipeline->SetMinPlatformVersion(static_cast<int32_t>(versions.first));
        container->SetApiTargetVersion(static_cast<int32_t>(versions.second));
        auto pattern = AceType::MakeRefPtr<ListPattern>();
        auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), pattern);
        auto algorithm = AceType::DynamicCast<ListLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
        auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(list,
            list->GetGeometryNode(), list->GetLayoutProperty());
        wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algorithm));
        pattern->startIndex_ = 0;
        pattern->endIndex_ = 0;
        pattern->centerIndex_ = -1;
        pattern->targetIndex_ = 0;
        EXPECT_TRUE(pattern->OnDirtyLayoutWrapperSwap(wrapper, {}));
        const bool legacyUpdate = versions.first == PlatformVersion::VERSION_NINE;
        EXPECT_EQ(pattern->startIndex_, legacyUpdate ? -1 : 0);
        EXPECT_EQ(pattern->endIndex_, legacyUpdate ? -1 : 0);
    }
    pipeline->SetMinPlatformVersion(oldMinimum);
    container->SetApiTargetVersion(oldTarget);
}

/**
 * @tc.name: FastScrollTargetOffsetMatchingRemainsExact001
 * @tc.desc: Parameter matching must reject a different offset even when it lies within geometric tolerance.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollTargetOffsetMatchingRemainsExact001, TestSize.Level1)
{
    ListLayoutAlgorithm algorithm;
    algorithm.SetFastScrollTarget(10, ScrollAlign::START);
    algorithm.SetFastScrollPreparationOffset(1.0f);
    EXPECT_TRUE(algorithm.MatchesFastScrollTarget(10, ScrollAlign::START, 1.0f));
    EXPECT_FALSE(algorithm.MatchesFastScrollTarget(10, ScrollAlign::START, 1.0001f));
    EXPECT_FALSE(algorithm.MatchesFastScrollTarget(11, ScrollAlign::START, 1.0f));
    EXPECT_FALSE(algorithm.MatchesFastScrollTarget(10, ScrollAlign::END, 1.0f));
}

namespace {
enum class PredictScanOutcome { POSITION, PREDICT_AND_POSITION, WAIT_FOR_PARENT };

struct PredictScanState {
    const char* name;
    bool dirty;
    bool layoutComplete;
    bool constraintChanged;
    bool preMeasured;
    bool missing;
    PredictScanOutcome single;
    PredictScanOutcome multi;
};

using Outcome = PredictScanOutcome;
const PredictScanState PREDICT_SCAN_STATES[] = {
    { "Missing", false, false, false, false, true, Outcome::WAIT_FOR_PARENT, Outcome::WAIT_FOR_PARENT },
    { "DirtyUnpositioned", true, false, false, false, false, Outcome::WAIT_FOR_PARENT, Outcome::WAIT_FOR_PARENT },
    { "DirtyPositioned", true, true, false, false, false,
        Outcome::PREDICT_AND_POSITION, Outcome::PREDICT_AND_POSITION },
    { "CleanPositioned", false, true, false, false, false, Outcome::POSITION, Outcome::POSITION },
    { "CleanUnpositioned", false, false, false, false, false, Outcome::POSITION, Outcome::POSITION },
    { "ConstraintPositioned", false, true, true, false, false,
        Outcome::PREDICT_AND_POSITION, Outcome::PREDICT_AND_POSITION },
    { "ConstraintUnpositioned", false, false, true, false, false,
        Outcome::PREDICT_AND_POSITION, Outcome::PREDICT_AND_POSITION },
    { "PreMeasuredDirtyUnpositioned", true, false, false, true, false, Outcome::POSITION, Outcome::WAIT_FOR_PARENT },
    { "PreMeasuredDirtyPositioned", true, true, false, true, false, Outcome::POSITION, Outcome::PREDICT_AND_POSITION },
    { "PreMeasuredConstraint", false, true, true, true, false, Outcome::POSITION, Outcome::PREDICT_AND_POSITION },
};
using PredictScanParam = std::tuple<int32_t, bool, PredictScanState>;
constexpr float SCAN_START = 100.0f;
constexpr float SCAN_END = 200.0f;
constexpr float SCAN_SPACE = 7.0f;
constexpr int32_t SCAN_ITEM_COUNT = 3;

// LayoutWrapperNode intentionally has no GetChildByIndex implementation.
class PredictScanWrapper : public LayoutWrapperNode {
    DECLARE_ACE_TYPE(PredictScanWrapper, LayoutWrapperNode);
public:
    explicit PredictScanWrapper(const RefPtr<FrameNode>& node)
        : LayoutWrapperNode(node, node->GetGeometryNode(), node->GetLayoutProperty()) {}

    RefPtr<LayoutWrapper> GetChildByIndex(uint32_t index, bool isCache) override
    {
        return GetOrCreateChildByIndex(index, false, isCache);
    }
};

class LanesTypeScanItem : public LayoutWrapperNode {
    DECLARE_ACE_TYPE(LanesTypeScanItem, LayoutWrapperNode);
public:
    explicit LanesTypeScanItem(const RefPtr<FrameNode>& node)
        : LayoutWrapperNode(node, node->GetGeometryNode(), node->GetLayoutProperty()) {}

    void Measure(const std::optional<LayoutConstraintF>&) override
    {
        ++measureCount_;
    }

    void Layout() override
    {
        ++layoutCount_;
    }

    int32_t measureCount_ = 0;
    int32_t layoutCount_ = 0;
};

class LanesTypeScanWrapper : public LayoutWrapperNode {
    DECLARE_ACE_TYPE(LanesTypeScanWrapper, LayoutWrapperNode);
public:
    explicit LanesTypeScanWrapper(const RefPtr<FrameNode>& node)
        : LayoutWrapperNode(node, node->GetGeometryNode(), node->GetLayoutProperty()) {}

    RefPtr<LayoutWrapper> GetOrCreateChildByIndex(uint32_t index, bool addToRenderTree, bool isCache,
        bool allowPlaceholderSubstitution = false) override
    {
        calls_.emplace_back(index, addToRenderTree, isCache);
        if (index == missingIndex_) {
            return nullptr;
        }
        return LayoutWrapperNode::GetOrCreateChildByIndex(
            index, addToRenderTree, isCache, allowPlaceholderSubstitution);
    }

    std::vector<std::tuple<uint32_t, bool, bool>> calls_;
    uint32_t missingIndex_ = UINT32_MAX;
};

struct LanesTypeScanCase {
    const char* name;
    std::vector<bool> groups;
    int32_t lanes;
    int32_t target;
    int32_t firstScanned;
    int32_t lastScanned;
    ListPositionMap::EstimateRange targetRow;
};

class ListFastScrollSegmentOriginTestNg : public TestNG,
    public testing::WithParamInterface<std::tuple<bool, bool, bool, int32_t>> {
public:
    bool IsGroup(int32_t index) const
    {
        const int32_t physical = stack_ ? itemCount_ - index - 1 : index;
        return !homogeneous_ && (physical == FIRST_SEGMENT_GROUP_INDEX || physical == SECOND_SEGMENT_GROUP_INDEX);
    }

    int32_t SegmentStart(int32_t index) const
    {
        if (IsGroup(index)) {
            return index;
        }
        while (index > 0 && !IsGroup(index - 1)) {
            --index;
        }
        return index;
    }

    std::pair<int32_t, int32_t> Row(int32_t index, int32_t lanes) const
    {
        EXPECT_GT(lanes, 0);
        CHECK_NULL_RETURN(lanes > 0, (std::pair<int32_t, int32_t> { -1, -1 }));
        if (IsGroup(index)) {
            return { index, index };
        }
        const int32_t first = index - (index - SegmentStart(index)) % lanes;
        int32_t last = first;
        while (last + 1 < itemCount_ && last - first + 1 < lanes && !IsGroup(last + 1)) {
            ++last;
        }
        return { first, last };
    }

    void Prepare()
    {
        const auto [stack, backward, homogeneous, lanes] = GetParam();
        stack_ = stack;
        homogeneous_ = homogeneous;
        algorithm_.SetLanes(lanes);
        algorithm_.totalItemCount_ = itemCount_;
        algorithm_.isStackFromEnd_ = stack;
        algorithm_.SetFastScrollMode(homogeneous ? FastScrollMode::SKIP_CREATE_AND_MEASURE :
            FastScrollMode::SKIP_MEASURE);
        algorithm_.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
        auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
        nodes_.push_back(list);
        parent_ = AceType::MakeRefPtr<LanesTypeScanWrapper>(list);
        for (int32_t physical = 0; physical < itemCount_; ++physical) {
            const bool group = !homogeneous && (physical == 5 || physical == 17);
            RefPtr<Pattern> pattern = AceType::MakeRefPtr<ListItemPattern>(nullptr);
            if (group) {
                pattern = AceType::MakeRefPtr<ListItemGroupPattern>(nullptr, V2::ListItemGroupOptions {});
            }
            auto node = FrameNode::CreateFrameNode(group ? V2::LIST_ITEM_GROUP_ETS_TAG : V2::LIST_ITEM_ETS_TAG,
                GetElmtId(), pattern);
            nodes_.push_back(node);
            auto wrapper = AceType::MakeRefPtr<LanesTypeScanItem>(node);
            items_.push_back(wrapper);
            parent_->AppendChild(wrapper);
        }
        const auto [first, last] = Row(15, lanes);
        for (int32_t index = first; index <= last; ++index) {
            const int32_t physical = stack ? itemCount_ - index - 1 : index;
            algorithm_.itemPosition_[physical] = { physical, 0.0f, 100.0f, IsGroup(index) };
        }
    }

protected:
    static constexpr int32_t itemCount_ = 30;
    bool stack_ = false;
    bool homogeneous_ = false;
    ListLanesLayoutAlgorithm algorithm_;
    RefPtr<LanesTypeScanWrapper> parent_;
    std::vector<RefPtr<FrameNode>> nodes_;
    std::vector<RefPtr<LanesTypeScanItem>> items_;
};

/**
 * @tc.name: FastScrollSegmentsRemainValidAfterLaneChange
 * @tc.desc: Build-only preparation writes true segment origins that the original lane queries can reuse.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollSegmentOriginTestNg, FastScrollSegmentsRemainValidAfterLaneChange, TestSize.Level1)
{
    Prepare();
    const auto [stack, backward, homogeneous, lanes] = GetParam();
    const int32_t logicalTarget = backward ? 2 : 26;
    const int32_t target = stack ? itemCount_ - logicalTarget - 1 : logicalTarget;
    ASSERT_TRUE(algorithm_.BuildFastScrollLanesRange(AceType::RawPtr(parent_), target));
    for (const auto& [start, end] : algorithm_.lanesItemRange_) {
        EXPECT_EQ(start, SegmentStart(start));
        EXPECT_GE(end, start);
        for (int32_t index = start; index <= end; ++index) {
            EXPECT_EQ(SegmentStart(index), start);
        }
    }
    const auto ranges = algorithm_.lanesItemRange_;
    const int32_t newLanes = lanes == 2 ? 3 : 2;
    const auto property = parent_->GetHostNode()->GetLayoutProperty<ListLayoutProperty>();
    property->UpdateLanes(newLanes);
    LayoutConstraintF constraint;
    property->UpdateLayoutConstraint(constraint);
    property->UpdateContentConstraint();
    algorithm_.CalculateLanes(property, constraint, 360.0f, Axis::VERTICAL);
    EXPECT_EQ(algorithm_.lanesItemRange_, ranges);
    const auto expected = Row(logicalTarget, newLanes);
    EXPECT_EQ(algorithm_.GetLanesFloor(AceType::RawPtr(parent_), logicalTarget), expected.first);
    EXPECT_EQ(algorithm_.GetLanesCeil(AceType::RawPtr(parent_), logicalTarget), expected.second);
    for (const auto& item : items_) {
        EXPECT_EQ(item->measureCount_, 0);
        EXPECT_EQ(item->layoutCount_, 0);
    }
}

INSTANTIATE_TEST_SUITE_P(DirectionsAndModes, ListFastScrollSegmentOriginTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool(), testing::Values(2, 3)));

const LanesTypeScanCase LANES_TYPE_SCAN_CASES[] = {
    { "OrdinaryComplete", { false, false, false, false, false, false, false }, 3, 4, 0, 5, { 3, 5 } },
    { "GroupTarget", { false, true, false, false, false }, 3, 1, 1, 1, { 1, 1 } },
    { "GroupBreaksPartial", { false, true, false, false, false, true, false }, 3, 3, 1, 4, { 2, 4 } },
    { "GroupAfterTarget", { false, false, true, false }, 3, 1, 0, 2, { 0, 1 } },
    { "AdjacentGroups", { true, true, false, false, false, true }, 3, 3, 1, 4, { 2, 4 } },
    { "AllGroups", { true, true, true, true, true }, 2, 2, 2, 2, { 2, 2 } },
    { "PartialTail", { false, true, false, false }, 3, 3, 1, 3, { 2, 3 } },
    { "TargetZero", { false, false, false, false }, 2, 0, 0, 1, { 0, 1 } },
};
} // namespace

class ListFastScrollLanesTypeTestNg : public TestNG, public testing::WithParamInterface<LanesTypeScanCase> {
public:
    void PrepareTypes(int32_t itemStartIndex = 0, bool stackFromEnd = false);
    void CheckBuildOnly();

protected:
    RefPtr<ListLanesLayoutAlgorithm> algorithm_;
    RefPtr<LanesTypeScanWrapper> parent_;
    std::vector<RefPtr<FrameNode>> nodes_;
    std::vector<RefPtr<LanesTypeScanItem>> items_;
};

void ListFastScrollLanesTypeTestNg::PrepareTypes(int32_t itemStartIndex, bool stackFromEnd)
{
    const auto& testCase = GetParam();
    algorithm_ = AceType::MakeRefPtr<ListLanesLayoutAlgorithm>();
    algorithm_->SetLanes(testCase.lanes);
    algorithm_->totalItemCount_ = static_cast<int32_t>(testCase.groups.size());
    algorithm_->itemStartIndex_ = itemStartIndex;
    algorithm_->isStackFromEnd_ = stackFromEnd;
    algorithm_->SetFastScrollMode(FastScrollMode::SKIP_MEASURE);
    algorithm_->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm_->posMap_->UpdateTotalCount(algorithm_->totalItemCount_);
    auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
    nodes_.push_back(list);
    parent_ = AceType::MakeRefPtr<LanesTypeScanWrapper>(list);
    algorithm_->listLayoutProperty_ = AceType::DynamicCast<ListLayoutProperty>(list->GetLayoutProperty());
    for (int32_t index = 0; index < algorithm_->totalItemCount_ + itemStartIndex; ++index) {
        const int32_t physical = index - itemStartIndex;
        const int32_t logical = stackFromEnd ? algorithm_->totalItemCount_ - physical - 1 : physical;
        const bool isGroup = physical >= 0 && testCase.groups[logical];
        RefPtr<Pattern> pattern = AceType::MakeRefPtr<ListItemPattern>(nullptr);
        if (isGroup) {
            pattern = AceType::MakeRefPtr<ListItemGroupPattern>(nullptr, V2::ListItemGroupOptions {});
        }
        auto node = FrameNode::CreateFrameNode(isGroup ? V2::LIST_ITEM_GROUP_ETS_TAG : V2::LIST_ITEM_ETS_TAG,
            GetElmtId(), pattern);
        nodes_.push_back(node);
        auto item = AceType::MakeRefPtr<LanesTypeScanItem>(node);
        item->SetActive(false);
        item->GetGeometryNode()->SetFrameSize(SizeF(90.0f, 70.0f));
        items_.push_back(item);
        parent_->AppendChild(item);
    }
}

void ListFastScrollLanesTypeTestNg::CheckBuildOnly()
{
    const auto& testCase = GetParam();
    std::set<int32_t> visited;
    for (const auto& [child, render, cache] : parent_->calls_) {
        const int32_t physical = static_cast<int32_t>(child) - algorithm_->itemStartIndex_;
        ASSERT_GE(physical, 0);
        ASSERT_LT(physical, algorithm_->totalItemCount_);
        const int32_t logical = algorithm_->isStackFromEnd_ ? algorithm_->totalItemCount_ - physical - 1 : physical;
        visited.insert(logical);
        const bool visible = algorithm_->itemPosition_.count(physical) ||
            (algorithm_->listLayoutProperty_->GetShowCachedItemsValue(false) &&
                algorithm_->cachedItemPosition_.count(physical));
        EXPECT_EQ(render, visible);
        EXPECT_EQ(cache, !visible);
    }
    std::set<int32_t> expected;
    for (int32_t index = testCase.firstScanned; index <= testCase.lastScanned; ++index) {
        expected.insert(index);
    }
    EXPECT_EQ(visited, expected);
    for (const auto& item : items_) {
        EXPECT_EQ(item->measureCount_, 0);
        EXPECT_EQ(item->layoutCount_, 0);
        EXPECT_EQ(item->GetGeometryNode()->GetFrameSize(), SizeF(90.0f, 70.0f));
    }
    EXPECT_TRUE(algorithm_->itemPosition_.empty());
    EXPECT_TRUE(algorithm_->noLayoutedItems_.empty());
}

/**
 * @tc.name: FastScrollLanesBuildTypes
 * @tc.desc: Build-only preparation confirms mixed types and completes the target row without measuring.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLanesTypeTestNg, FastScrollLanesBuildTypes, TestSize.Level1)
{
    const auto& testCase = GetParam();
    PrepareTypes();
    ASSERT_TRUE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), testCase.target));
    const auto row = algorithm_->GetPhysicalLanesRow(testCase.target);
    ASSERT_TRUE(row);
    EXPECT_EQ(row->startIndex, testCase.targetRow.startIndex);
    EXPECT_EQ(row->endIndex, testCase.targetRow.endIndex);
    ASSERT_NO_FATAL_FAILURE(CheckBuildOnly());
    for (int32_t index = testCase.firstScanned; index <= testCase.lastScanned; ++index) {
        const auto info = algorithm_->posMap_->GetPositionInfo(index);
        EXPECT_EQ(info.mainPos, -1.0f);
        EXPECT_EQ(info.mainSize, -1.0f);
        EXPECT_EQ(info.isGroup, testCase.groups[index]);
        EXPECT_FALSE(items_[index]->IsActive());
    }
    const auto calls = parent_->calls_.size();
    for (int32_t index = row->startIndex; index <= row->endIndex; ++index) {
        EXPECT_EQ(algorithm_->GetLanesFloor(AceType::RawPtr(parent_), index), row->startIndex);
    }
    EXPECT_EQ(parent_->calls_.size(), calls);
}

/**
 * @tc.name: FastScrollLanesBuildTypesReversed
 * @tc.desc: Mirrored physical nodes produce the same logical type segments, excluding the List header.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLanesTypeTestNg, FastScrollLanesBuildTypesReversed, TestSize.Level1)
{
    constexpr int32_t itemStartIndex = 1;
    const auto& testCase = GetParam();
    PrepareTypes(itemStartIndex, true);
    const int32_t target = algorithm_->totalItemCount_ - testCase.target - 1;
    ASSERT_TRUE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), target));
    ASSERT_NO_FATAL_FAILURE(CheckBuildOnly());
    const auto row = algorithm_->GetFastScrollLanesRow(testCase.target);
    ASSERT_TRUE(row);
    EXPECT_EQ(row->startIndex, testCase.targetRow.startIndex);
    EXPECT_EQ(row->endIndex, testCase.targetRow.endIndex);
    const auto physical = algorithm_->GetPhysicalLanesRow(target);
    ASSERT_TRUE(physical);
    EXPECT_EQ(physical->startIndex, algorithm_->totalItemCount_ - row->endIndex - 1);
    EXPECT_EQ(physical->endIndex, algorithm_->totalItemCount_ - row->startIndex - 1);
    for (int32_t index = row->startIndex; index <= row->endIndex; ++index) {
        EXPECT_EQ(algorithm_->GetLanesFloor(AceType::RawPtr(parent_), index), row->startIndex);
    }
}

/**
 * @tc.name: FastScrollLanesBuildTypesKeepsVisibleIntent
 * @tc.desc: Visible and shown cached items stay active while offscreen items use cache acquisition.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLanesTypeTestNg, FastScrollLanesBuildTypesKeepsVisibleIntent, TestSize.Level1)
{
    const auto& testCase = GetParam();
    PrepareTypes();
    algorithm_->listLayoutProperty_->UpdateShowCachedItems(true);
    algorithm_->itemPosition_[0] = { -1, 0.0f, 70.0f, testCase.groups[0] };
    algorithm_->cachedItemPosition_[1] = { -1, 70.0f, 140.0f, testCase.groups[1] };
    items_[0]->SetActive(true);
    items_[1]->SetActive(true);
    algorithm_->posMap_->UpdatePos(0, { 120.0f, 70.0f, !testCase.groups[0], true });
    for (int32_t index = 0; index < algorithm_->totalItemCount_; ++index) {
        ASSERT_NE(algorithm_->GetFastScrollLanesItem(AceType::RawPtr(parent_), index), nullptr);
    }
    ASSERT_EQ(parent_->calls_.size(), static_cast<size_t>(algorithm_->totalItemCount_));
    for (int32_t index = 0; index < algorithm_->totalItemCount_; ++index) {
        const bool visible = index <= 1;
        EXPECT_EQ(parent_->calls_[index], std::make_tuple(static_cast<uint32_t>(index), visible, !visible));
        EXPECT_EQ(items_[index]->IsActive(), visible);
        EXPECT_EQ(items_[index]->measureCount_, 0);
        EXPECT_EQ(items_[index]->layoutCount_, 0);
    }
    const auto info = algorithm_->posMap_->GetPositionInfo(0);
    EXPECT_EQ(info.mainPos, 120.0f);
    EXPECT_EQ(info.mainSize, 70.0f);
    EXPECT_EQ(info.isGroup, !testCase.groups[0]);
    EXPECT_TRUE(info.isLazyChild);
    EXPECT_EQ(algorithm_->itemPosition_.size(), 1);
    EXPECT_EQ(algorithm_->cachedItemPosition_.size(), 1);
}

/**
 * @tc.name: FastScrollLanesBuildTypesHiddenCache
 * @tc.desc: Cached items that are not shown must not be promoted to the render tree by type preparation.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLanesTypeTestNg, FastScrollLanesBuildTypesHiddenCache, TestSize.Level1)
{
    const auto& testCase = GetParam();
    PrepareTypes();
    algorithm_->listLayoutProperty_->UpdateShowCachedItems(false);
    algorithm_->itemPosition_[0] = { -1, 0.0f, 70.0f, testCase.groups[0] };
    algorithm_->cachedItemPosition_[1] = { -1, 70.0f, 140.0f, testCase.groups[1] };
    items_[0]->SetActive(true);
    for (int32_t index = 0; index < algorithm_->totalItemCount_; ++index) {
        ASSERT_NE(algorithm_->GetFastScrollLanesItem(AceType::RawPtr(parent_), index), nullptr);
    }
    ASSERT_EQ(parent_->calls_.size(), static_cast<size_t>(algorithm_->totalItemCount_));
    for (int32_t index = 0; index < algorithm_->totalItemCount_; ++index) {
        const bool visible = index == 0;
        EXPECT_EQ(parent_->calls_[index], std::make_tuple(static_cast<uint32_t>(index), visible, !visible));
        EXPECT_EQ(items_[index]->IsActive(), visible);
        EXPECT_EQ(items_[index]->measureCount_, 0);
        EXPECT_EQ(items_[index]->layoutCount_, 0);
    }
    EXPECT_EQ(algorithm_->itemPosition_.size(), 1);
    EXPECT_EQ(algorithm_->cachedItemPosition_.size(), 1);
}

/**
 * @tc.name: FastScrollLanesBuildTypesFailureKeepsRanges
 * @tc.desc: A missing requested node cannot confirm a segment or replace previously measured geometry.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLanesTypeTestNg, FastScrollLanesBuildTypesFailureKeepsRanges, TestSize.Level1)
{
    const auto& testCase = GetParam();
    PrepareTypes();
    algorithm_->listLayoutProperty_->UpdateSupportLazyLoadingEmptyBranch(true);
    algorithm_->lanesItemRange_ = { { 0, 0 } };
    const auto ranges = algorithm_->lanesItemRange_;
    algorithm_->posMap_->UpdatePos(0, { 120.0f, 70.0f, testCase.groups[0], true });
    parent_->missingIndex_ = testCase.groups[testCase.target] ? testCase.target : 1;
    EXPECT_FALSE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), testCase.target));
    EXPECT_EQ(algorithm_->lanesItemRange_, ranges);
    EXPECT_EQ(algorithm_->posMap_->posMap_.size(), 1);
    EXPECT_EQ(algorithm_->posMap_->GetPositionInfo(0).mainPos, 120.0f);
    EXPECT_EQ(algorithm_->posMap_->GetPositionInfo(0).mainSize, 70.0f);
    EXPECT_EQ(algorithm_->posMap_->GetPositionInfo(0).isGroup, testCase.groups[0]);
    EXPECT_TRUE(algorithm_->posMap_->GetPositionInfo(0).isLazyChild);
    ASSERT_FALSE(parent_->calls_.empty());
    EXPECT_EQ(parent_->calls_.back(), std::make_tuple(parent_->missingIndex_, false, true));
    if (!testCase.groups[0]) {
        EXPECT_FALSE(algorithm_->GetFastScrollLanesRow(0));
    }
    for (const auto& item : items_) {
        EXPECT_EQ(item->measureCount_, 0);
        EXPECT_EQ(item->layoutCount_, 0);
        EXPECT_FALSE(item->IsActive());
        EXPECT_EQ(item->GetGeometryNode()->GetFrameSize(), SizeF(90.0f, 70.0f));
    }
    EXPECT_TRUE(algorithm_->itemPosition_.empty());
    EXPECT_TRUE(algorithm_->noLayoutedItems_.empty());
}

class ListFastScrollVisibleRowsTestNg : public TestNG,
    public testing::WithParamInterface<std::tuple<bool, bool, bool>> {
public:
    void CreateRows()
    {
        const auto [backward, homogeneous, stack] = GetParam();
        algorithm_.SetLanes(TYPE_SCAN_LANES);
        algorithm_.totalItemCount_ = TYPE_SCAN_ITEM_COUNT;
        algorithm_.itemStartIndex_ = 1;
        algorithm_.isStackFromEnd_ = stack;
        algorithm_.SetFastScrollMode(homogeneous ? FastScrollMode::SKIP_CREATE_AND_MEASURE :
            FastScrollMode::SKIP_MEASURE);
        algorithm_.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
        auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
        nodes_.push_back(list);
        parent_ = AceType::MakeRefPtr<LanesTypeScanWrapper>(list);
        for (int32_t child = 0; child <= TYPE_SCAN_ITEM_COUNT; ++child) {
            const int32_t physical = child - 1;
            const int32_t logical = stack ? 23 - physical : physical;
            const bool group = physical >= 0 && !homogeneous && (logical == 6 || logical == 19);
            RefPtr<Pattern> pattern = AceType::MakeRefPtr<ListItemPattern>(nullptr);
            if (group) {
                pattern = AceType::MakeRefPtr<ListItemGroupPattern>(nullptr, V2::ListItemGroupOptions {});
            }
            auto node = FrameNode::CreateFrameNode(group ? V2::LIST_ITEM_GROUP_ETS_TAG : V2::LIST_ITEM_ETS_TAG,
                GetElmtId(), pattern);
            nodes_.push_back(node);
            auto item = AceType::MakeRefPtr<LanesTypeScanItem>(node);
            items_.push_back(item);
            parent_->AppendChild(item);
        }
        const int32_t visibleStart = homogeneous ? 12 : 13;
        for (int32_t index = visibleStart; index < visibleStart + TYPE_SCAN_LANES; ++index) {
            const int32_t physical = stack ? 23 - index : index;
            algorithm_.itemPosition_[physical] = { physical, 0.0f, 100.0f, false };
            items_[physical + 1]->SetActive(true);
        }
    }

protected:
    ListLanesLayoutAlgorithm algorithm_;
    RefPtr<LanesTypeScanWrapper> parent_;
    std::vector<RefPtr<FrameNode>> nodes_;
    std::vector<RefPtr<LanesTypeScanItem>> items_;
};

/**
 * @tc.name: FastScrollBuildsFromVisibleRows
 * @tc.desc: Preparation may extend to the true segment head and remains usable by the original lane queries.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollVisibleRowsTestNg, FastScrollBuildsFromVisibleRows, TestSize.Level1)
{
    CreateRows();
    const auto [backward, homogeneous, stack] = GetParam();
    const int32_t logicalTarget = backward ? 5 : 20;
    const int32_t target = stack ? 23 - logicalTarget : logicalTarget;
    ASSERT_TRUE(algorithm_.BuildFastScrollLanesRange(AceType::RawPtr(parent_), target));
    const auto row = algorithm_.GetFastScrollLanesRow(logicalTarget);
    ASSERT_TRUE(row);
    EXPECT_EQ(row->startIndex, backward ? 3 : (homogeneous ? 18 : 20));
    EXPECT_EQ(row->endIndex, backward ? 5 : (homogeneous ? 20 : 22));
    if (homogeneous) {
        EXPECT_TRUE(parent_->calls_.empty());
    } else {
        std::set<int32_t> visited;
        for (const auto& [child, render, cache] : parent_->calls_) {
            ASSERT_GE(child, 1u);
            ASSERT_LE(child, 24u);
            const int32_t physical = static_cast<int32_t>(child) - 1;
            visited.insert(stack ? 23 - physical : physical);
            const bool visible = algorithm_.itemPosition_.count(physical) != 0;
            EXPECT_EQ(render, visible);
            EXPECT_EQ(cache, !visible);
        }
        std::set<int32_t> expected;
        for (int32_t index = backward ? 0 : 6; index <= (backward ? 15 : 22); ++index) {
            expected.insert(index);
        }
        EXPECT_EQ(visited, expected);
    }
    ListLanesLayoutAlgorithm next;
    next.posMap_ = algorithm_.posMap_;
    next.totalItemCount_ = 24;
    next.isStackFromEnd_ = stack;
    next.SetLanes(3);
    algorithm_.SwapLanesItemRange(next.lanesItemRange_);
    EXPECT_EQ(next.GetLanesFloor(AceType::RawPtr(parent_), logicalTarget), row->startIndex);
    EXPECT_EQ(next.GetLanesCeil(AceType::RawPtr(parent_), logicalTarget), row->endIndex);
    for (const auto& item : items_) {
        EXPECT_EQ(item->measureCount_, 0);
        EXPECT_EQ(item->layoutCount_, 0);
    }
}

INSTANTIATE_TEST_SUITE_P(DirectionsAndModes, ListFastScrollVisibleRowsTestNg,
    testing::Combine(testing::Bool(), testing::Bool(), testing::Bool()));

/**
 * @tc.name: FastScrollUsesSegmentCacheWithoutRowAllocation001
 * @tc.desc: Extending a true segment retains one origin, while consecutive Groups remain independent.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollUsesSegmentCacheWithoutRowAllocation001, TestSize.Level1)
{
    ListLanesLayoutAlgorithm algorithm;
    algorithm.SetLanes(3);
    algorithm.totalItemCount_ = 20;
    algorithm.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm.posMap_->UpdateTotalCount(20);
    for (int32_t index = 0; index < 20; ++index) {
        algorithm.RecordFastScrollItemType(index, false);
    }
    algorithm.RecordFastScrollLanesRange(0, 4);
    algorithm.RecordFastScrollLanesRange(0, 8);
    algorithm.RecordFastScrollLanesRange(0, 11);
    EXPECT_EQ(algorithm.lanesItemRange_, (std::map<int32_t, int32_t> { { 0, 11 } }));
    const auto first = algorithm.GetFastScrollLanesRow(2);
    const auto middle = algorithm.GetFastScrollLanesRow(7);
    ASSERT_TRUE(first);
    ASSERT_TRUE(middle);
    EXPECT_EQ(first->endIndex, 2);
    EXPECT_EQ(middle->startIndex, 6);
    algorithm.RecordFastScrollItemType(12, true);
    algorithm.RecordFastScrollLanesRange(12, 12);
    algorithm.RecordFastScrollItemType(13, true);
    algorithm.RecordFastScrollLanesRange(13, 13);
    algorithm.RecordFastScrollLanesRange(14, 19);
    EXPECT_EQ(algorithm.lanesItemRange_,
        (std::map<int32_t, int32_t> { { 0, 11 }, { 12, 12 }, { 13, 13 }, { 14, 19 } }));
    EXPECT_TRUE(algorithm.posMap_->GetPositionInfo(12).isGroup);
    EXPECT_TRUE(algorithm.posMap_->GetPositionInfo(13).isGroup);
}

/**
 * @tc.name: FastScrollSegmentExtensionKeepsUnclassifiedGroupBoundary001
 * @tc.desc: Reusing a true segment origin preserves cached Group boundaries absent from the position map.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollSegmentExtensionKeepsUnclassifiedGroupBoundary001, TestSize.Level1)
{
    ListLanesLayoutAlgorithm algorithm;
    algorithm.SetLanes(3);
    algorithm.totalItemCount_ = 12;
    algorithm.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm.posMap_->UpdateTotalCount(12);
    const std::map<int32_t, int32_t> ranges { { 0, 3 }, { 4, 4 }, { 5, 11 } };
    algorithm.lanesItemRange_ = ranges;
    EXPECT_EQ(algorithm.posMap_->posMap_.count(4), 0u);

    algorithm.RecordFastScrollLanesRange(0, 2);

    EXPECT_EQ(algorithm.lanesItemRange_, ranges);
    EXPECT_EQ(algorithm.posMap_->posMap_.count(4), 0u);
    EXPECT_EQ(algorithm.GetLanesFloor(nullptr, 4), 4);
    EXPECT_EQ(algorithm.GetLanesCeil(nullptr, 4), 4);
}

/**
 * @tc.name: FastScrollPartialRangeKeepsLegacyQuerySemantics001
 * @tc.desc: Fast preparation confirms a partial row without imposing its completeness rule on legacy queries.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollPartialRangeKeepsLegacyQuerySemantics001, TestSize.Level1)
{
    ListLanesLayoutAlgorithm algorithm;
    algorithm.SetLanes(3);
    algorithm.totalItemCount_ = 12;
    algorithm.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm.posMap_->UpdateTotalCount(12);
    algorithm.lanesItemRange_ = { { 0, 4 } };
    EXPECT_FALSE(algorithm.GetFastScrollLanesRow(4));
    EXPECT_EQ(algorithm.GetLanesFloor(nullptr, 4), 3);
    const auto complete = algorithm.GetFastScrollLanesRow(2);
    ASSERT_TRUE(complete);
    EXPECT_EQ(complete->endIndex, 2);
    algorithm.RecordFastScrollItemType(5, true);
    algorithm.RecordFastScrollLanesRange(5, 5);
    const auto partial = algorithm.GetFastScrollLanesRow(4);
    const auto group = algorithm.GetFastScrollLanesRow(5);
    ASSERT_TRUE(partial);
    ASSERT_TRUE(group);
    EXPECT_EQ(partial->startIndex, 3);
    EXPECT_EQ(partial->endIndex, 4);
    EXPECT_EQ(group->startIndex, 5);
    EXPECT_EQ(group->endIndex, 5);
    EXPECT_EQ(algorithm.GetLanesFloor(nullptr, 4), 3);
    EXPECT_EQ(algorithm.GetLanesCeil(nullptr, 4), 4);
}

/**
 * @tc.name: FastScrollKnownReverseWorksAfterTargetCleared001
 * @tc.desc: Final-frame coordinate bookkeeping leaves the public table canonical; scoped layout owns its mirror.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollKnownReverseWorksAfterTargetCleared001, TestSize.Level1)
{
    ListLayoutAlgorithm algorithm;
    algorithm.totalItemCount_ = 3;
    algorithm.isStackFromEnd_ = true;
    algorithm.childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>();
    algorithm.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    algorithm.posMap_->UpdateTotalCount(3);
    algorithm.posMap_->totalHeight_ = 300.0f;
    algorithm.posMap_->UpdatePos(0, { 0.0f, 50.0f });
    algorithm.posMap_->UpdatePos(1, { 60.0f, 200.0f, true });
    algorithm.posMap_->UpdatePos(2, { 270.0f, 30.0f });
    algorithm.SetFastScrollKnownMainSize(true);
    ASSERT_FALSE(algorithm.GetFastScrollState()->input.targetIndex);
    const auto shared = algorithm.posMap_;
    algorithm.ProcessStackFromEnd();
    EXPECT_EQ(algorithm.posMap_, shared);
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(1).mainPos, 60.0f);
    EXPECT_TRUE(algorithm.posMap_->GetPositionInfo(1).isGroup);
    algorithm.ProcessStackFromEnd();
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(1).mainPos, 60.0f);
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(2).mainPos, 270.0f);
    EXPECT_TRUE(algorithm.posMap_->GetPositionInfo(1).isGroup);
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetTotalHeight(), 300.0f);
}

/**
 * @tc.name: FastScrollLanesBuildTypesGuards001
 * @tc.desc: Invalid inputs do not acquire nodes or change existing cached geometry.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FastScrollLanesBuildTypesGuards001, TestSize.Level1)
{
    ListLanesLayoutAlgorithm algorithm;
    algorithm.SetLanes(2);
    algorithm.totalItemCount_ = 1;
    algorithm.posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
    auto parent = AceType::MakeRefPtr<LanesTypeScanWrapper>(list);
    auto node = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG, GetElmtId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr));
    auto item = AceType::MakeRefPtr<LanesTypeScanItem>(node);
    item->GetGeometryNode()->SetFrameSize(SizeF(90.0f, 70.0f));
    item->SetActive(false);
    parent->AppendChild(item);
    algorithm.lanesItemRange_ = { { 0, 0 } };
    const auto ranges = algorithm.lanesItemRange_;
    algorithm.posMap_->UpdatePos(0, { 120.0f, 70.0f, true });
    EXPECT_FALSE(algorithm.BuildFastScrollLanesRange(nullptr, 0));
    EXPECT_FALSE(algorithm.BuildFastScrollLanesRange(AceType::RawPtr(parent), -1));
    EXPECT_FALSE(algorithm.BuildFastScrollLanesRange(AceType::RawPtr(parent), algorithm.totalItemCount_));
    algorithm.SetLanes(1);
    EXPECT_FALSE(algorithm.BuildFastScrollLanesRange(AceType::RawPtr(parent), 0));
    ASSERT_TRUE(parent->calls_.empty());
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(0).mainPos, 120.0f);
    EXPECT_FLOAT_EQ(algorithm.posMap_->GetPositionInfo(0).mainSize, 70.0f);
    EXPECT_TRUE(algorithm.posMap_->GetPositionInfo(0).isGroup);
    algorithm.SetLanes(2);
    algorithm.posMap_.Reset();
    EXPECT_FALSE(algorithm.BuildFastScrollLanesRange(AceType::RawPtr(parent), 0));
    EXPECT_EQ(algorithm.lanesItemRange_, ranges);
    EXPECT_TRUE(parent->calls_.empty());
    EXPECT_EQ(item->measureCount_, 0);
    EXPECT_EQ(item->layoutCount_, 0);
    EXPECT_EQ(item->GetGeometryNode()->GetFrameSize(), SizeF(90.0f, 70.0f));
    EXPECT_FALSE(item->IsActive());
    EXPECT_TRUE(algorithm.itemPosition_.empty());
    EXPECT_TRUE(algorithm.noLayoutedItems_.empty());
}

INSTANTIATE_TEST_SUITE_P(MixedAndHomogeneous, ListFastScrollLanesTypeTestNg,
    testing::ValuesIn(LANES_TYPE_SCAN_CASES), [](const testing::TestParamInfo<LanesTypeScanCase>& info) {
        return info.param.name;
    });

namespace {
RefPtr<UINode> CreateFastScrollSyntaxItem(bool group, bool wrapped)
{
    RefPtr<Pattern> pattern = AceType::MakeRefPtr<ListItemPattern>(nullptr);
    if (group) {
        pattern = AceType::MakeRefPtr<ListItemGroupPattern>(nullptr, V2::ListItemGroupOptions {});
    }
    auto item = FrameNode::CreateFrameNode(group ? V2::LIST_ITEM_GROUP_ETS_TAG : V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    item->GetGeometryNode()->SetFrameSize(SizeF(90.0f, 70.0f));
    if (!wrapped) {
        return item;
    }
    auto branch = AceType::MakeRefPtr<IfElseNode>(ElementRegister::GetInstance()->MakeUniqueId());
    branch->AddChild(item);
    return branch;
}

class FastScrollSyntaxLazyBuilder final : public Framework::MockLazyForEachBuilder {
    DECLARE_ACE_TYPE(FastScrollSyntaxLazyBuilder, Framework::MockLazyForEachBuilder);
public:
    explicit FastScrollSyntaxLazyBuilder(bool mixed) : mixed_(mixed) {}

    std::vector<int32_t> createdIndices_;

protected:
    int32_t OnGetTotalCount() override
    {
        return SYNTAX_RANGE_ITEM_COUNT;
    }

    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>&) override
    {
        createdIndices_.push_back(index);
        return { std::to_string(index), CreateFastScrollSyntaxItem(mixed_ && (index == 2 || index == 6), mixed_) };
    }

private:
    bool mixed_;
};

class FastScrollSyntaxScanWrapper final : public LanesTypeScanWrapper {
    DECLARE_ACE_TYPE(FastScrollSyntaxScanWrapper, LanesTypeScanWrapper);
public:
    explicit FastScrollSyntaxScanWrapper(const RefPtr<FrameNode>& host) : LanesTypeScanWrapper(host) {}

    RefPtr<LayoutWrapper> GetOrCreateChildByIndex(uint32_t index, bool render, bool cache,
        bool allowPlaceholderSubstitution = false) override
    {
        calls_.emplace_back(index, render, cache);
        auto child = GetHostNode()->GetOrCreateChildByIndex(index, render, cache, allowPlaceholderSubstitution);
        CHECK_NULL_RETURN(child && child->GetHostNode(), nullptr);
        auto node = child->GetHostNode();
        auto found = items_.find(node->GetId());
        if (found == items_.end()) {
            found = items_.emplace(node->GetId(), AceType::MakeRefPtr<LanesTypeScanItem>(node)).first;
        }
        return found->second;
    }

    std::map<int32_t, RefPtr<LanesTypeScanItem>> items_;
};
} // namespace

class ListFastScrollSyntaxTestBase : public TestNG {
protected:
    void PrepareProvider(const RefPtr<UINode>& provider, bool stack)
    {
        list_ = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
        list_->AddChild(CreateFastScrollSyntaxItem(false, false)); // Excluded by itemStartIndex_.
        list_->AddChild(CreateFastScrollSyntaxItem(true, false));
        list_->AddChild(CreateFastScrollSyntaxItem(false, false));
        list_->AddChild(CreateFastScrollSyntaxItem(false, false));
        list_->AddChild(provider);
        list_->AddChild(CreateFastScrollSyntaxItem(false, false));
        list_->AddChild(CreateFastScrollSyntaxItem(true, false));
        parent_ = AceType::MakeRefPtr<FastScrollSyntaxScanWrapper>(list_);
        algorithm_ = AceType::MakeRefPtr<ListLanesLayoutAlgorithm>();
        algorithm_->SetLanes(TYPE_SCAN_LANES);
        algorithm_->totalItemCount_ = SYNTAX_LIST_ITEM_COUNT;
        algorithm_->itemStartIndex_ = 1;
        algorithm_->isStackFromEnd_ = stack;
        algorithm_->SetFastScrollMode(FastScrollMode::SKIP_MEASURE);
        algorithm_->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
        algorithm_->posMap_->UpdateTotalCount(SYNTAX_LIST_ITEM_COUNT);
        algorithm_->listLayoutProperty_ = list_->GetLayoutProperty<ListLayoutProperty>();
    }

    void CheckPreparedSyntaxRow(bool stack, int32_t segmentStart)
    {
        const int32_t target = stack ? 4 : 8;
        const int32_t rowStart = target - (target - segmentStart) % 3;
        const auto row = algorithm_->GetFastScrollLanesRow(target);
        ASSERT_TRUE(row);
        EXPECT_EQ(row->startIndex, rowStart);
        EXPECT_EQ(row->endIndex, rowStart + SYNTAX_ROW_END_DELTA);
        auto segment = algorithm_->lanesItemRange_.upper_bound(target);
        ASSERT_NE(segment, algorithm_->lanesItemRange_.begin());
        EXPECT_EQ(std::prev(segment)->first, segmentStart);
        EXPECT_EQ(algorithm_->GetLanesFloor(AceType::RawPtr(parent_), target), rowStart);
        EXPECT_TRUE(algorithm_->noLayoutedItems_.empty());
        for (const auto& [id, item] : parent_->items_) {
            EXPECT_EQ(item->measureCount_, 0) << "node=" << id;
            EXPECT_EQ(item->layoutCount_, 0) << "node=" << id;
        }
    }

    RefPtr<UINode> CreateRepeatProvider(bool secondVersion)
    {
        const auto create = [created = created_](uint32_t index) {
            created->push_back(index);
            ViewStackProcessor::GetInstance()->Push(CreateFastScrollSyntaxItem(false, true));
        };
        if (secondVersion) {
            auto getRid = [create](IndexType index, bool, bool) -> std::pair<RIDType, uint32_t> {
                create(index);
                return { static_cast<RIDType>(index + 1), OnGetRid4IndexResult::CREATED_NEW_NODE };
            };
            return RepeatVirtualScroll2Node::GetOrCreateRepeatNode(GetElmtId(), SYNTAX_RANGE_ITEM_COUNT,
                SYNTAX_RANGE_ITEM_COUNT, 0, getRid,
                [](IndexType, IndexType) {}, [](int32_t, int32_t, int32_t, int32_t, bool, bool) {},
                [](IndexType, IndexType) {}, []() {}, []() {}, []() {});
        }
        auto keys = [](uint32_t first, uint32_t last) {
            std::list<std::string> result;
            for (uint32_t index = first; index <= last; ++index) {
                result.emplace_back(std::to_string(index));
            }
            return result;
        };
        auto types = [](uint32_t first, uint32_t last) {
            return std::list<std::string>(last - first + 1, "item");
        };
        return RepeatVirtualScrollNode::GetOrCreateRepeatNode(GetElmtId(), SYNTAX_RANGE_ITEM_COUNT, { { "item",
            { true, SYNTAX_RANGE_ITEM_COUNT } } },
            create, [](const std::string&, uint32_t) {}, keys, types, [](int32_t, int32_t) {}, false);
    }

    RefPtr<FrameNode> list_;
    RefPtr<FastScrollSyntaxScanWrapper> parent_;
    RefPtr<ListLanesLayoutAlgorithm> algorithm_;
    std::shared_ptr<std::vector<int32_t>> created_ = std::make_shared<std::vector<int32_t>>();
};

class ListFastScrollLazySyntaxTestNg : public ListFastScrollSyntaxTestBase,
    public testing::WithParamInterface<bool> {};

/**
 * @tc.name: FastScrollDirectLazyUsesActiveAndExpiringIndices
 * @tc.desc: A real homogeneous Lazy resolves both cache states and extends through ordinary container prefixes.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLazySyntaxTestNg, FastScrollDirectLazyUsesActiveAndExpiringIndices, TestSize.Level1)
{
    const bool stack = GetParam();
    for (bool cache : { false, true }) {
        SCOPED_TRACE(cache);
        auto builder = AceType::MakeRefPtr<FastScrollSyntaxLazyBuilder>(false);
        auto lazy = LazyForEachNode::CreateLazyForEachNode(GetElmtId(), builder);
        PrepareProvider(lazy, stack);
        auto child = parent_->GetOrCreateChildByIndex(9, !cache, cache);
        ASSERT_NE(child, nullptr);
        auto node = child->GetHostNode();
        ASSERT_EQ(node->GetParent(), lazy);
        EXPECT_EQ(lazy->GetFrameNodeIndex(node), 5);
        EXPECT_EQ(lazy->GetIndexByUINode(node), cache ? -1 : 5);
        ASSERT_EQ(builder->GetAllChildren().count(5), 1u);
        EXPECT_EQ(builder->GetAllChildren().at(5).second == nullptr, cache);
        const auto range = algorithm_->GetFastScrollLazyRange(node, 8);
        ASSERT_TRUE(range);
        EXPECT_EQ(range->startIndex, 3);
        EXPECT_EQ(range->endIndex, 10);
        if (!cache) {
            algorithm_->itemPosition_[8] = { node->GetId(), 0.0f, 70.0f, false };
        }
        ASSERT_TRUE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), 8));
        ASSERT_NO_FATAL_FAILURE(CheckPreparedSyntaxRow(stack, 1));
        EXPECT_EQ(builder->createdIndices_, (std::vector<int32_t> { 5 }));
        EXPECT_GT(parent_->calls_.size(), builder->createdIndices_.size());
        const uint32_t prefix = stack ? 12 : 3;
        EXPECT_TRUE(std::any_of(parent_->calls_.begin(), parent_->calls_.end(), [prefix](const auto& call) {
            return std::get<0>(call) == prefix;
        }));
    }
}

/**
 * @tc.name: FastScrollWrappedLazyDoesNotInferMixedTypes
 * @tc.desc: An IfElse between Lazy and FrameNode requires actual mixed-node classification in either direction.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollLazySyntaxTestNg, FastScrollWrappedLazyDoesNotInferMixedTypes, TestSize.Level1)
{
    const bool stack = GetParam();
    auto builder = AceType::MakeRefPtr<FastScrollSyntaxLazyBuilder>(true);
    auto lazy = LazyForEachNode::CreateLazyForEachNode(GetElmtId(), builder);
    PrepareProvider(lazy, stack);
    auto child = parent_->GetOrCreateChildByIndex(9, false, true);
    ASSERT_NE(child, nullptr);
    ASSERT_TRUE(AceType::InstanceOf<IfElseNode>(child->GetHostNode()->GetParent()));
    EXPECT_EQ(lazy->GetFrameNodeIndex(child->GetHostNode()), 5);
    EXPECT_FALSE(algorithm_->GetFastScrollLazyRange(child->GetHostNode(), 8));
    ASSERT_TRUE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), 8));
    ASSERT_NO_FATAL_FAILURE(CheckPreparedSyntaxRow(stack, stack ? 4 : 6));
    const std::set<int32_t> created(builder->createdIndices_.begin(), builder->createdIndices_.end());
    EXPECT_EQ(created, (stack ? std::set<int32_t> { 3, 4, 5, 6 } : std::set<int32_t> { 2, 3, 4, 5 }));
    EXPECT_EQ(builder->createdIndices_.size(), created.size());
    EXPECT_TRUE(algorithm_->posMap_->GetPositionInfo(stack ? 9 : 5).isGroup);
    EXPECT_FALSE(algorithm_->posMap_->GetPositionInfo(8).isGroup);
}

INSTANTIATE_TEST_SUITE_P(NormalAndStack, ListFastScrollLazySyntaxTestNg, testing::Bool());

class ListFastScrollRepeatSyntaxTestNg : public ListFastScrollSyntaxTestBase,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

/**
 * @tc.name: FastScrollRepeatUsesRealVirtualCacheIndex
 * @tc.desc: Both Repeat versions resolve their real cache index through a non-Frame ancestor without prefix creation.
 * @tc.type: FUNC
 */
HWTEST_P(ListFastScrollRepeatSyntaxTestNg, FastScrollRepeatUsesRealVirtualCacheIndex, TestSize.Level1)
{
    const auto [secondVersion, stack] = GetParam();
    auto repeat = CreateRepeatProvider(secondVersion);
    PrepareProvider(repeat, stack);
    auto child = parent_->GetOrCreateChildByIndex(9, false, true);
    ASSERT_NE(child, nullptr);
    auto node = child->GetHostNode();
    ASSERT_TRUE(AceType::InstanceOf<IfElseNode>(node->GetParent()));
    EXPECT_EQ(repeat->GetFrameNodeIndex(node), 5);
    const auto range = algorithm_->GetFastScrollLazyRange(node, 8);
    ASSERT_TRUE(range);
    EXPECT_EQ(range->startIndex, 3);
    EXPECT_EQ(range->endIndex, 10);
    const auto types = algorithm_->GetFastScrollTypeRange(AceType::RawPtr(parent_), stack ? 4 : 8);
    ASSERT_TRUE(types);
    EXPECT_EQ(types->startIndex, stack ? 2 : 3);
    EXPECT_EQ(types->endIndex, stack ? 9 : 10);
    EXPECT_FALSE(types->isGroup);
    ASSERT_TRUE(algorithm_->BuildFastScrollLanesRange(AceType::RawPtr(parent_), 8));
    ASSERT_NO_FATAL_FAILURE(CheckPreparedSyntaxRow(stack, 1));
    EXPECT_EQ(*created_, (std::vector<int32_t> { 5 }));
    EXPECT_GT(parent_->calls_.size(), created_->size());
}

INSTANTIATE_TEST_SUITE_P(VersionsAndStack, ListFastScrollRepeatSyntaxTestNg,
    testing::Combine(testing::Bool(), testing::Bool()));

class ListPredictScanTestNg : public TestNG, public testing::WithParamInterface<PredictScanParam> {
public:
    void PrepareScan();
    void CheckScanResult(const std::list<PredictLayoutItem>& predictions, int32_t cachedCount, int32_t lastIndex);

protected:
    RefPtr<ListLayoutAlgorithm> algorithm_;
    RefPtr<PredictScanWrapper> parent_;
    std::vector<RefPtr<FrameNode>> nodes_;
    int32_t target_ = 0;
};

void ListPredictScanTestNg::PrepareScan()
{
    const auto& [lanes, forward, state] = GetParam();
    if (lanes == 1) {
        algorithm_ = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    } else {
        auto algorithm = AceType::MakeRefPtr<ListLanesLayoutAlgorithm>();
        algorithm->SetLanes(lanes);
        algorithm_ = algorithm;
    }
    auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
    nodes_.push_back(list);
    list->GetGeometryNode()->SetFrameSize(SizeF(240.0f, 400.0f));
    parent_ = AceType::MakeRefPtr<PredictScanWrapper>(list);
    algorithm_->totalItemCount_ = SCAN_ITEM_COUNT;
    algorithm_->contentMainSize_ = 400.0f;
    algorithm_->spaceWidth_ = SCAN_SPACE;
    algorithm_->itemPosition_[1] = { -1, SCAN_START, SCAN_END, false };
    target_ = forward ? SCAN_ITEM_COUNT - 1 : 0;
    for (int32_t index = 0; index < algorithm_->totalItemCount_; ++index) {
        auto node = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG, GetElmtId(),
            AceType::MakeRefPtr<ListItemPattern>(nullptr));
        nodes_.push_back(node);
        node->isLayoutComplete_ = state.layoutComplete;
        auto geometry = node->GetGeometryNode();
        geometry->SetFrameSize(SizeF(120.0f, 100.0f));
        auto constraint = algorithm_->childLayoutConstraint_;
        if (state.constraintChanged) {
            constraint.maxSize.SetWidth(123.0f);
        }
        geometry->SetParentLayoutConstraint(constraint);
        auto child = AceType::MakeRefPtr<LayoutWrapperNode>(node, geometry, node->GetLayoutProperty());
        child->needForceMeasureAndLayout_ = state.dirty;
        if (state.preMeasured) {
            child->SetHasPreMeasured();
        }
        parent_->AppendChild(child);
    }
    if (state.missing) {
        parent_->childrenMap_.erase(target_);
    }
}

void ListPredictScanTestNg::CheckScanResult(
    const std::list<PredictLayoutItem>& predictions, int32_t cachedCount, int32_t lastIndex)
{
    const auto& [lanes, forward, state] = GetParam();
    const auto outcome = lanes == 1 ? state.single : state.multi;
    const bool needPredict = outcome != Outcome::POSITION;
    const bool needParentLayout = outcome == Outcome::WAIT_FOR_PARENT;
    ASSERT_EQ(predictions.size(), needPredict ? 1 : 0);
    if (needPredict) {
        const auto& item = predictions.front();
        EXPECT_EQ(item.index, target_);
        EXPECT_EQ(item.needParentLayout, needParentLayout);
        EXPECT_EQ(item.forwardCacheCount, forward ? 0 : -1);
        EXPECT_EQ(item.backwardCacheCount, forward ? -1 : 0);
        EXPECT_TRUE(item.forceCache);
        EXPECT_FLOAT_EQ(item.referencePos, forward ? SCAN_END + SCAN_SPACE : SCAN_START - SCAN_SPACE);
    }
    EXPECT_EQ(cachedCount, needParentLayout ? 0 : 1);
    EXPECT_EQ(lastIndex, needParentLayout ? 1 : target_);
    EXPECT_EQ(algorithm_->cachedItemPosition_.count(target_), needParentLayout ? 0 : 1);
}

/**
 * @tc.name: ClassifiesParentLayoutRequirement
 * @tc.desc: Cache scans distinguish missing, dirty, positioned, constraint-changed and premeasured items.
 * @tc.type: FUNC
 */
HWTEST_P(ListPredictScanTestNg, ClassifiesParentLayoutRequirement, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(PrepareScan());
    std::list<PredictLayoutItem> predictions;
    int32_t cachedCount = 0;
    const bool forward = std::get<1>(GetParam());
    const int32_t lastIndex = forward ?
        algorithm_->LayoutCachedForward(AceType::RawPtr(parent_), 1, cachedCount, 1, predictions, false) :
        algorithm_->LayoutCachedBackward(AceType::RawPtr(parent_), 1, cachedCount, 1, predictions, false);
    ASSERT_NO_FATAL_FAILURE(CheckScanResult(predictions, cachedCount, lastIndex));
}

INSTANTIATE_TEST_SUITE_P(CacheScan, ListPredictScanTestNg,
    testing::Combine(testing::Values(1, 2), testing::Bool(), testing::ValuesIn(PREDICT_SCAN_STATES)),
    ([](const testing::TestParamInfo<PredictScanParam>& info) {
        const auto& [lanes, forward, state] = info.param;
        return std::string(lanes == 1 ? "Single" : "Multi") + (forward ? "Forward" : "Backward") + state.name;
    }));

/**
 * @tc.name: CachedGroupBoundaryDoesNotRequestParentLayout
 * @tc.desc: A Group after a partial multi-lane row ends the row without scheduling another prediction.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CachedGroupBoundaryDoesNotRequestParentLayout, TestSize.Level1)
{
    auto algorithm = AceType::MakeRefPtr<ListLanesLayoutAlgorithm>();
    auto node = FrameNode::CreateFrameNode(V2::LIST_ITEM_GROUP_ETS_TAG, GetElmtId(),
        AceType::MakeRefPtr<ListItemGroupPattern>(nullptr, V2::ListItemGroupOptions {}));
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        node, node->GetGeometryNode(), node->GetLayoutProperty());
    for (bool forward : { false, true }) {
        SCOPED_TRACE(forward);
        bool isGroup = false;
        const auto [needBreak, needPredict] = algorithm->CheckACachedItem(wrapper, 1, isGroup, 0.0f, forward);
        EXPECT_FALSE(isGroup);
        EXPECT_TRUE(needBreak);
        EXPECT_FALSE(needPredict);
    }
}

/**
 * @tc.name: UpdateListItemConstraint001
 * @tc.desc: Test ListLayoutAlgorithm UpdateListItemConstraint
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, UpdateListItemConstraint001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    std::optional<float> width = 5.0f;
    std::optional<float> height = std::nullopt;
    OptionalSizeF selfIdealSize(width, height);
    LayoutConstraintF contentConstraint;
    contentConstraint.maxSize.SetCrossSize(10.0f, Axis::HORIZONTAL);
    contentConstraint.percentReference.SetCrossSize(6.0f, Axis::HORIZONTAL);
    listLayoutAlgorithm->UpdateListItemConstraint(Axis::HORIZONTAL, selfIdealSize, contentConstraint);
    EXPECT_EQ(selfIdealSize.Height(), std::nullopt);
    EXPECT_EQ(selfIdealSize.Width(), 5.0f);
    EXPECT_EQ(contentConstraint.maxSize.Height(), 10.0f);
    EXPECT_EQ(contentConstraint.percentReference.Height(), 6.0f);
}

/**
 * @tc.name: ReviseSpace001
 * @tc.desc: Test ListLayoutAlgorithm ReviseSpace
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, ReviseSpace001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->isRoundingMode_ = true;
    listLayoutAlgorithm->contentMainSize_ = 10.0f;
    listLayoutAlgorithm->spaceWidth_ = 3.8f;
    listLayoutAlgorithm->chainInterval_ = 1.0f;
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listLayoutAlgorithm->ReviseSpace(listLayoutProperty);
    EXPECT_EQ(listLayoutAlgorithm->spaceWidth_, 5.0f);
}

/**
 * @tc.name: Measure001
 * @tc.desc: Test ListLayoutAlgorithm Measure
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, Measure001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    std::optional<float> width = 5.0f;
    std::optional<float> height = 8.0f;
    OptionalSizeF selfIdealSize(width, height);
    OptionalSizeF parentIdealSize(width, height);
    ViewPosReference viewPosRef = { 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, ReferenceEdge::END, Axis::FREE };
    LayoutConstraintF layoutConstraint;
    layoutConstraint.selfIdealSize = selfIdealSize;
    layoutConstraint.parentIdealSize = parentIdealSize;
    layoutConstraint.viewPosRef = viewPosRef;
    listLayoutProperty->layoutConstraint_ = layoutConstraint;
    listLayoutProperty->contentConstraint_ = layoutConstraint;
    listLayoutProperty->safeAreaExpandOpts_ = std::make_unique<SafeAreaExpandOpts>();
    listLayoutProperty->safeAreaExpandOpts_->type = SAFE_AREA_TYPE_SYSTEM;
    listLayoutProperty->safeAreaExpandOpts_->edges = SAFE_AREA_EDGE_BOTTOM;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    listPattern->frameNode_ = frameNode;
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    ListItemInfo listItemInfo { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->itemPosition_[0] = listItemInfo;
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    ListPositionInfo positionInfo { 2.0f, 4.0f, true };
    listPositionMap->posMap_[0] = positionInfo;
    listLayoutAlgorithm->posMap_ = listPositionMap;
    listLayoutAlgorithm->childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>();
    listLayoutAlgorithm->totalItemCount_ = 0;
    listLayoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_TRUE(listLayoutAlgorithm->itemPosition_.empty());
    EXPECT_TRUE(listLayoutAlgorithm->posMap_->posMap_.empty());
}

/**
 * @tc.name: GetStartPositionWithChainOffset001
 * @tc.desc: Test ListLayoutAlgorithm GetStartPositionWithChainOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, GetStartPositionWithChainOffset001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->itemPosition_.clear();
    auto result = listLayoutAlgorithm->GetStartPositionWithChainOffset();
    EXPECT_EQ(result, 0.0f);
}

/**
 * @tc.name: GetStartPositionWithChainOffset002
 * @tc.desc: Test ListLayoutAlgorithm GetStartPositionWithChainOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, GetStartPositionWithChainOffset002, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->itemPosition_.clear();
    ListItemInfo listItemInfo { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->itemPosition_[0] = listItemInfo;
    listLayoutAlgorithm->chainOffsetFunc_ = [](int32_t value) { return static_cast<float>(value) * 1.5f; };
    listLayoutAlgorithm->isStackFromEnd_ = true;
    auto result = listLayoutAlgorithm->GetStartPositionWithChainOffset();
    EXPECT_EQ(result, 3.5f);
}

/**
 * @tc.name: BeginLayoutForward001
 * @tc.desc: Test ListLayoutAlgorithm BeginLayoutForward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, BeginLayoutForward001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    listLayoutAlgorithm->BeginLayoutForward(2.0f, &layoutWrapper);
    EXPECT_FALSE(listLayoutAlgorithm->jumpIndex_.has_value());
}

/**
 * @tc.name: CheckNoNeedJumpListItemGroup001
 * @tc.desc: Test ListLayoutAlgorithm CheckNoNeedJumpListItemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckNoNeedJumpListItemGroup001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 2;
    auto result = listLayoutAlgorithm->CheckNoNeedJumpListItemGroup(&layoutWrapper, 2, 3, 5, 2.0f);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckNoNeedJumpListItemGroup002
 * @tc.desc: Test ListLayoutAlgorithm CheckNoNeedJumpListItemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckNoNeedJumpListItemGroup002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_COMPONENT_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 6;
    layoutWrapper.childrenMap_[5] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    auto result = listLayoutAlgorithm->CheckNoNeedJumpListItemGroup(&layoutWrapper, 2, 3, 5, 2.0f);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckNoNeedJumpListItemGroup003
 * @tc.desc: Test ListLayoutAlgorithm CheckNoNeedJumpListItemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckNoNeedJumpListItemGroup003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ITEM_GROUP_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 6;
    layoutWrapper.childrenMap_[5] = layoutWrapperNode;
    listLayoutAlgorithm->jumpIndexInGroup_ = std::nullopt;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    auto result = listLayoutAlgorithm->CheckNoNeedJumpListItemGroup(&layoutWrapper, 2, 3, 5, 2.0f);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckNoNeedJumpListItemGroup004
 * @tc.desc: Test ListLayoutAlgorithm CheckNoNeedJumpListItemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckNoNeedJumpListItemGroup004, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->jumpIndexInGroup_ = 3;
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ITEM_GROUP_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    RefPtr<ListItemGroupLayoutAlgorithm> listItemGroupLayoutAlgorithm =
        AceType::MakeRefPtr<ListItemGroupLayoutAlgorithm>(0, 0, 2);
    ListItemGroupInfo listItemGroupInfo { 2, 2.0f, 4.0f, true };
    listItemGroupLayoutAlgorithm->itemPosition_[2] = listItemGroupInfo;
    layoutAlgorithmWrapper->layoutAlgorithm_ = listItemGroupLayoutAlgorithm;
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    layoutWrapperNode->layoutProperty_ = listLayoutProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 6;
    layoutWrapper.childrenMap_[5] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    auto result = listLayoutAlgorithm->CheckNoNeedJumpListItemGroup(&layoutWrapper, 2, 6, 5, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndexInGroup_, 3);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckNoNeedJumpListItemGroup005
 * @tc.desc: Test ListLayoutAlgorithm CheckNoNeedJumpListItemGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckNoNeedJumpListItemGroup005, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->jumpIndexInGroup_ = 3;
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ITEM_GROUP_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    RefPtr<ListItemGroupLayoutAlgorithm> listItemGroupLayoutAlgorithm =
        AceType::MakeRefPtr<ListItemGroupLayoutAlgorithm>(0, 0, 2);
    layoutAlgorithmWrapper->layoutAlgorithm_ = listItemGroupLayoutAlgorithm;
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    layoutWrapperNode->layoutProperty_ = listLayoutProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 6;
    layoutWrapper.childrenMap_[5] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->jumpIndexInGroup_ = 0;
    auto result = listLayoutAlgorithm->CheckNoNeedJumpListItemGroup(&layoutWrapper, 2, 6, 5, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::NOT_CHANGE);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: NoNeedJump001
 * @tc.desc: Test ListLayoutAlgorithm NoNeedJump
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, NoNeedJump001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 2;
    auto result = listLayoutAlgorithm->NoNeedJump(&layoutWrapper, 2.0f, 3.0f, 1, 2, 3, 2.0f);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: JudgeInOfScreenScrollAutoType001
 * @tc.desc: Test ListLayoutAlgorithm JudgeInOfScreenScrollAutoType
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, JudgeInOfScreenScrollAutoType001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> layoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    layoutProperty->propStickyStyle_ = V2::StickyStyle::BOTH;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = layoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->hostNode_ = frameNode;
    listLayoutAlgorithm->startMainPos_ = 2.0f;
    listLayoutAlgorithm->endMainPos_ = 2.0f;
    auto result = listLayoutAlgorithm->JudgeInOfScreenScrollAutoType(layoutWrapper, layoutProperty, 2.0f, 6.0f);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::NOT_CHANGE);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: JudgeInOfScreenScrollAutoType002
 * @tc.desc: Test ListLayoutAlgorithm JudgeInOfScreenScrollAutoType
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, JudgeInOfScreenScrollAutoType002, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> layoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    layoutProperty->propStickyStyle_ = V2::StickyStyle::HEADER;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = layoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->hostNode_ = frameNode;
    listLayoutAlgorithm->startMainPos_ = 1.5f;
    listLayoutAlgorithm->endMainPos_ = 6.5f;
    listItemGroupPattern->headerMainSize_ = 4.0f;
    listItemGroupPattern->footerMainSize_ = 3.0f;
    auto result = listLayoutAlgorithm->JudgeInOfScreenScrollAutoType(layoutWrapper, layoutProperty, 2.0f, 6.0f);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::END);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: JudgeInOfScreenScrollAutoType003
 * @tc.desc: Test ListLayoutAlgorithm JudgeInOfScreenScrollAutoType
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, JudgeInOfScreenScrollAutoType003, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> layoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    layoutProperty->propStickyStyle_ = V2::StickyStyle::HEADER;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = layoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->hostNode_ = frameNode;
    listLayoutAlgorithm->startMainPos_ = 2.5f;
    listLayoutAlgorithm->endMainPos_ = 8.0f;
    listItemGroupPattern->headerMainSize_ = 4.0f;
    listItemGroupPattern->footerMainSize_ = 3.0f;
    auto result = listLayoutAlgorithm->JudgeInOfScreenScrollAutoType(layoutWrapper, layoutProperty, 2.0f, 6.0f);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::START);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: JudgeOutOfScreenScrollAutoType001
 * @tc.desc: Test ListLayoutAlgorithm JudgeOutOfScreenScrollAutoType
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, JudgeOutOfScreenScrollAutoType001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> layoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    layoutProperty->propStickyStyle_ = V2::StickyStyle::HEADER;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = layoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->hostNode_ = frameNode;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    RefPtr<ListItemGroupLayoutAlgorithm> listItemGroupLayoutAlgorithm =
        AceType::MakeRefPtr<ListItemGroupLayoutAlgorithm>(0, 0, 2);
    listItemGroupLayoutAlgorithm->itemPosition_[2] = ListItemGroupInfo { 2, 2.0f, 4.0f, true };
    layoutAlgorithmWrapper->layoutAlgorithm_ = listItemGroupLayoutAlgorithm;
    layoutWrapper->layoutAlgorithm_ = layoutAlgorithmWrapper;
    listLayoutAlgorithm->startMainPos_ = 2.5f;
    listLayoutAlgorithm->endMainPos_ = 8.0f;
    listItemGroupPattern->headerMainSize_ = 4.0f;
    listItemGroupPattern->footerMainSize_ = 3.0f;
    listLayoutAlgorithm->JudgeOutOfScreenScrollAutoType(layoutWrapper, 2, layoutProperty, 2, 1, 2, 4);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::END);
}

/**
 * @tc.name: JudgeOutOfScreenScrollAutoType002
 * @tc.desc: Test ListLayoutAlgorithm JudgeOutOfScreenScrollAutoType
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, JudgeOutOfScreenScrollAutoType002, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> layoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    layoutProperty->propStickyStyle_ = V2::StickyStyle::HEADER;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = layoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->hostNode_ = frameNode;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    RefPtr<ListItemGroupLayoutAlgorithm> listItemGroupLayoutAlgorithm =
        AceType::MakeRefPtr<ListItemGroupLayoutAlgorithm>(0, 0, 2);
    listItemGroupLayoutAlgorithm->itemPosition_[2] = ListItemGroupInfo { 2, 2.0f, 4.0f, true };
    layoutAlgorithmWrapper->layoutAlgorithm_ = listItemGroupLayoutAlgorithm;
    layoutWrapper->layoutAlgorithm_ = layoutAlgorithmWrapper;
    listLayoutAlgorithm->startMainPos_ = 2.5f;
    listLayoutAlgorithm->endMainPos_ = 8.0f;
    listItemGroupPattern->headerMainSize_ = 4.0f;
    listItemGroupPattern->footerMainSize_ = 3.0f;
    listLayoutAlgorithm->JudgeOutOfScreenScrollAutoType(layoutWrapper, 2, layoutProperty, 2, 3, 1, 2);
    EXPECT_EQ(listLayoutAlgorithm->scrollAutoType_, ScrollAutoType::START);
}

/**
 * @tc.name: CheckJumpToIndex001
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpToIndex001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->jumpIndex_ = std::nullopt;
    listLayoutAlgorithm->isNeedCheckOffset_ = true;
    listLayoutAlgorithm->childrenSize_ = nullptr;
    listLayoutAlgorithm->currentDelta_ = 5.0f;
    listLayoutAlgorithm->contentMainSize_ = 1.0f;
    listLayoutAlgorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    ListItemGroupLayoutInfo groupInfo = { true, true, 4.0f, 2.0f, 4.0f, 2.0f };
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true, false, true, 1.0f, 2.0f, groupInfo };
    listLayoutAlgorithm->itemPosition_[0] = listItemInfo;
    listLayoutAlgorithm->CheckJumpToIndex();
    EXPECT_EQ(listLayoutAlgorithm->groupItemAverageHeight_, 4.0f);
}

/**
 * @tc.name: CheckJumpToIndex002
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpToIndex002, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->jumpIndex_ = std::nullopt;
    listLayoutAlgorithm->isNeedCheckOffset_ = true;
    listLayoutAlgorithm->childrenSize_ = nullptr;
    listLayoutAlgorithm->currentDelta_ = 5.0f;
    listLayoutAlgorithm->contentMainSize_ = 1.0f;
    listLayoutAlgorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->itemPosition_[0] = listItemInfo;
    listLayoutAlgorithm->CheckJumpToIndex();
    EXPECT_EQ(listLayoutAlgorithm->groupItemAverageHeight_, 0.0f);
}

/**
 * @tc.name: CheckJumpToIndex003
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpToIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpToIndex003, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->jumpIndex_ = std::nullopt;
    listLayoutAlgorithm->isNeedCheckOffset_ = true;
    listLayoutAlgorithm->childrenSize_ = nullptr;
    listLayoutAlgorithm->currentDelta_ = -1.0f;
    listLayoutAlgorithm->contentMainSize_ = 0.0f;
    listLayoutAlgorithm->totalItemCount_ = 3;
    listLayoutAlgorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[2] = listItemInfo;
    listLayoutAlgorithm->CheckJumpToIndex();
    EXPECT_EQ(listLayoutAlgorithm->currentDelta_, -1.0f);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndex_.value(), 1.0f);
}

/**
 * @tc.name: CheckJumpValid001
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpValid
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpValid001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 2;
    listLayoutAlgorithm->jumpIndex_ = LAST_ITEM;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->jumpIndexInGroup_ = std::nullopt;
    auto result = listLayoutAlgorithm->CheckJumpValid(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndex_.value(), 3);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckJumpValid002
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpValid
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpValid002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->jumpIndex_ = LAST_ITEM;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->jumpIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    auto result = listLayoutAlgorithm->CheckJumpValid(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndex_.value(), 3);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckJumpValid003
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpValid
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpValid003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->currentChildCount_ = 10;
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_COMPONENT_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 10;
    listLayoutAlgorithm->jumpIndex_ = LAST_ITEM;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->jumpIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = true;
    layoutWrapper.childrenMap_[0] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    auto result = listLayoutAlgorithm->CheckJumpValid(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndexInGroup_.value(), 2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckJumpValid004
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpValid
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpValid004, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->pattern_ = listItemGroupPattern;
    frameNode->layoutProperty_ = listLayoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    layoutWrapperNode->currentChildCount_ = 10;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 10;
    listLayoutAlgorithm->jumpIndex_ = LAST_ITEM;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->jumpIndexInGroup_ = -1;
    listLayoutAlgorithm->isStackFromEnd_ = true;
    layoutWrapper.childrenMap_[0] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    auto result = listLayoutAlgorithm->CheckJumpValid(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndexInGroup_.value(), 7);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckJumpValid005
 * @tc.desc: Test ListLayoutAlgorithm CheckJumpValid
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckJumpValid005, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->pattern_ = listItemGroupPattern;
    frameNode->layoutProperty_ = listLayoutProperty;
    listItemGroupPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    layoutWrapperNode->currentChildCount_ = 10;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 10;
    listLayoutAlgorithm->jumpIndex_ = LAST_ITEM;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->jumpIndexInGroup_ = 10;
    listLayoutAlgorithm->isStackFromEnd_ = true;
    layoutWrapper.childrenMap_[0] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    auto result = listLayoutAlgorithm->CheckJumpValid(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->jumpIndexInGroup_.value(), 10);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckAndMeasureStartItem001
 * @tc.desc: Test ListLayoutAlgorithm CheckAndMeasureStartItem
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndMeasureStartItem001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 2;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->prevContentMainSize_ = 1;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    float startPos = 2.0f;
    listLayoutAlgorithm->CheckAndMeasureStartItem(&layoutWrapper, 2, startPos, true, false);
    EXPECT_EQ(startPos, 2.0f);
}

/**
 * @tc.name: CheckAndMeasureStartItem002
 * @tc.desc: Test ListLayoutAlgorithm CheckAndMeasureStartItem
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndMeasureStartItem002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->currentChildCount_ = 10;
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_COMPONENT_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 10;
    listLayoutAlgorithm->totalItemCount_ = 4;
    std::pair<int32_t, ListItemInfo> firstItemInfo = { 1, { 2, 0.0f, 10.0f, false } };
    listLayoutAlgorithm->firstItemInfo_ = firstItemInfo;
    layoutWrapper.childrenMap_[2] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->prevContentMainSize_ = 1;
    float startPos = 2.0f;
    listLayoutAlgorithm->CheckAndMeasureStartItem(&layoutWrapper, 2, startPos, true, false);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->firstItemInfo_.value().first, 1);
    EXPECT_EQ(listLayoutAlgorithm->firstItemInfo_.value().second.endPos, 10.0f);
}

/**
 * @tc.name: CheckAndMeasureStartItem003
 * @tc.desc: Test ListLayoutAlgorithm CheckAndMeasureStartItem
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndMeasureStartItem003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    RefPtr<LayoutWrapperNode> layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, listLayoutProperty);
    layoutWrapperNode->SetActive();
    layoutWrapperNode->currentChildCount_ = 10;
    layoutWrapperNode->geometryNode_ = geometryNode;
    layoutWrapperNode->hostNode_ = frameNode;
    layoutWrapperNode->hostNode_.Upgrade()->tag_ = V2::LIST_ITEM_GROUP_ETS_TAG;
    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithmWrapper =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(listLayoutAlgorithm);
    RefPtr<ListItemGroupLayoutAlgorithm> listItemGroupLayoutAlgorithm =
        AceType::MakeRefPtr<ListItemGroupLayoutAlgorithm>(0, 0, 2);
    listItemGroupLayoutAlgorithm->refPos_ = 6.0f;
    layoutAlgorithmWrapper->layoutAlgorithm_ = listItemGroupLayoutAlgorithm;
    layoutWrapperNode->layoutAlgorithm_ = layoutAlgorithmWrapper;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.currentChildCount_ = 10;
    listLayoutAlgorithm->totalItemCount_ = 4;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true };
    std::pair<int32_t, ListItemInfo> firstItemInfo = { 1, listItemInfo };
    listLayoutAlgorithm->firstItemInfo_ = firstItemInfo;
    layoutWrapper.childrenMap_[2] = layoutWrapperNode;
    listLayoutAlgorithm->itemStartIndex_ = 0;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->prevContentMainSize_ = 1;
    float startPos = 2.0f;
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>();
    listLayoutAlgorithm->childrenSize_ = listChildrenMainSize;
    listLayoutAlgorithm->CheckAndMeasureStartItem(&layoutWrapper, 2, startPos, true, false);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->firstItemInfo_.value().second.startPos, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->firstItemInfo_.value().second.endPos, 2.0f);
}

/**
 * @tc.name: LayoutDirectionForTargetIndex001
 * @tc.desc: Test ListLayoutAlgorithm LayoutDirectionForTargetIndex
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutDirectionForTargetIndex001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    auto result = listLayoutAlgorithm->LayoutDirectionForTargetIndex(&layoutWrapper, 2);
    EXPECT_EQ(result, LayoutDirection::NONE);
}

/**
 * @tc.name: LayoutALineForward001
 * @tc.desc: Test ListLayoutAlgorithm LayoutALineForward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutALineForward001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    int32_t currentIndex = 2;
    float endPos = 4.0f;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->firstItemInfo_ = std::nullopt;
    auto result = listLayoutAlgorithm->LayoutALineForward(&layoutWrapper, currentIndex, 2.0f, endPos);
    EXPECT_EQ(currentIndex, 2);
    EXPECT_EQ(endPos, 4.0f);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: LayoutALineBackward001
 * @tc.desc: Test ListLayoutAlgorithm LayoutALineBackward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutALineBackward001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    int32_t currentIndex = 2;
    float startPos = 2.0f;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    int32_t index = 1;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->firstItemInfo_ = std::make_pair(index, listItemInfo);
    auto result = listLayoutAlgorithm->LayoutALineBackward(&layoutWrapper, currentIndex, 4.0f, startPos);
    EXPECT_EQ(currentIndex, 1);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_FALSE(listLayoutAlgorithm->firstItemInfo_.has_value());
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: LayoutALineBackward002
 * @tc.desc: Test ListLayoutAlgorithm LayoutALineBackward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutALineBackward002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    int32_t currentIndex = 2;
    float startPos = 2.0f;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->firstItemInfo_ = std::nullopt;
    auto result = listLayoutAlgorithm->LayoutALineBackward(&layoutWrapper, currentIndex, 4.0f, startPos);
    EXPECT_EQ(currentIndex, 2);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: LayoutALineBackward003
 * @tc.desc: Test ListLayoutAlgorithm LayoutALineBackward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutALineBackward003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    int32_t currentIndex = 2;
    float startPos = 2.0f;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    int32_t index = 3;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->firstItemInfo_ = std::make_pair(index, listItemInfo);
    auto result = listLayoutAlgorithm->LayoutALineBackward(&layoutWrapper, currentIndex, 4.0f, startPos);
    EXPECT_EQ(currentIndex, 2);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: LayoutALineBackward004
 * @tc.desc: Test ListLayoutAlgorithm LayoutALineBackward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutALineBackward004, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    int32_t currentIndex = 2;
    float startPos = 2.0f;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    listLayoutAlgorithm->totalItemCount_ = 4;
    listLayoutAlgorithm->targetIndex_ = 2;
    listLayoutAlgorithm->targetIndexInGroup_ = 2;
    listLayoutAlgorithm->isStackFromEnd_ = false;
    int32_t index = 1;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, true };
    listLayoutAlgorithm->firstItemInfo_ = std::make_pair(index, listItemInfo);
    auto result = listLayoutAlgorithm->LayoutALineBackward(&layoutWrapper, currentIndex, 4.0f, startPos);
    EXPECT_EQ(startPos, 2.0f);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(currentIndex, 1);
    EXPECT_EQ(listLayoutAlgorithm->itemPosition_[1].startPos, 2.0f);
}

/**
 * @tc.name: LayoutBackward001
 * @tc.desc: Test ListLayoutAlgorithm LayoutBackward
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, LayoutBackward001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listLayoutProperty->propScrollSnapAlign_ = ScrollSnapAlign::CENTER;
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 1;
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    listLayoutAlgorithm->overScrollFeature_ = true;
    listLayoutAlgorithm->backwardFeature_ = false;
    listLayoutAlgorithm->prevContentMainSize_ = 2.0f;
    listLayoutAlgorithm->currentOffset_ = 3.0f;
    listLayoutAlgorithm->contentMainSize_ = 10.0f;
    listLayoutAlgorithm->contentStartOffset_ = 1.0f;
    listLayoutAlgorithm->contentEndOffset_ = 8.0f;
    listLayoutAlgorithm->startMainPos_ = 2.0f;
    listLayoutAlgorithm->totalItemCount_ = 2;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->LayoutBackward(&layoutWrapper, -1, 4.0f);
    EXPECT_EQ(listLayoutAlgorithm->currentOffset_, 11.0f);
    EXPECT_EQ(listLayoutAlgorithm->contentMainSize_, 5.0f);
}

/**
 * @tc.name: FixPredictSnapPos001
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapPos
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapPos001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapPos();
    EXPECT_EQ(listLayoutAlgorithm->scrollSnapAlign_, ScrollSnapAlign::NONE);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 2.0f);
}

/**
 * @tc.name: FixPredictSnapOffset001
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffset001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffset(listLayoutProperty);
    EXPECT_FALSE(listLayoutAlgorithm->predictSnapOffset_.has_value());
    EXPECT_FALSE(listLayoutAlgorithm->predictSnapEndPos_.has_value());
}

/**
 * @tc.name: FixPredictSnapOffset002
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffset002, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->itemPosition_[2] = { 2, 3.0f, 4.0f, false };
    listLayoutAlgorithm->FixPredictSnapOffset(listLayoutProperty);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, -2.0f);
}

/**
 * @tc.name: FixPredictSnapOffset003
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffset003, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listLayoutAlgorithm->scrollSnapVelocity_ = 1000.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 10.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    ListItemInfo listItemInfo = { 2, -100.0f, 100.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffset(listLayoutProperty);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 0.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 0.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignStart001
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignStart
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignStart001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignStart();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 2.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignCenter001
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignCenter
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignCenter001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignCenter();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 2.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignCenter002
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignCenter
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignCenter002, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->totalOffset_ = 2.0f;
    listLayoutAlgorithm->spaceWidth_ = 6.0f;
    listLayoutAlgorithm->contentMainSize_ = 1.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->isSpringEffect_ = true;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignCenter();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 2.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignCenter003
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignCenter
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignCenter003, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->totalOffset_ = 2.0f;
    listLayoutAlgorithm->spaceWidth_ = 6.0f;
    listLayoutAlgorithm->contentMainSize_ = 1.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->isSpringEffect_ = false;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignCenter();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 1.5f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 0.5f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignCenter004
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignCenter
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignCenter004, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->totalOffset_ = 4.0f;
    listLayoutAlgorithm->spaceWidth_ = 6.0f;
    listLayoutAlgorithm->contentMainSize_ = 8.0f;
    listLayoutAlgorithm->totalItemCount_ = 1;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->isSpringEffect_ = true;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignCenter();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 2.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 2.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignCenter005
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignCenter
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignCenter005, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->totalOffset_ = 4.0f;
    listLayoutAlgorithm->spaceWidth_ = 6.0f;
    listLayoutAlgorithm->contentMainSize_ = 8.0f;
    listLayoutAlgorithm->totalItemCount_ = 1;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::CENTER;
    listLayoutAlgorithm->isSpringEffect_ = false;
    ListItemInfo listItemInfo = { 2, 2.0f, 4.0f, false };
    listLayoutAlgorithm->itemPosition_[1] = listItemInfo;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignCenter();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 7.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, -3.0f);
}

/**
 * @tc.name: FixPredictSnapOffsetAlignEnd001
 * @tc.desc: Test ListLayoutAlgorithm FixPredictSnapOffsetAlignEnd
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, FixPredictSnapOffsetAlignEnd001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 5.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 6.0f;
    listLayoutAlgorithm->scrollSnapAlign_ = ScrollSnapAlign::END;
    listLayoutAlgorithm->FixPredictSnapOffsetAlignEnd();
    EXPECT_EQ(listLayoutAlgorithm->predictSnapOffset_, 6.0f);
    EXPECT_EQ(listLayoutAlgorithm->predictSnapEndPos_, 5.0f);
}

/**
 * @tc.name: CalculateLaneCrossOffset001
 * @tc.desc: Test ListLayoutAlgorithm CalculateLaneCrossOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CalculateLaneCrossOffset001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->predictSnapEndPos_ = 2.0f;
    listLayoutAlgorithm->predictSnapOffset_ = 2.0f;
    listLayoutAlgorithm->totalOffset_ = 6.0f;
    int32_t number = 6;
    listLayoutAlgorithm->listItemAlign_ = static_cast<OHOS::Ace::V2::ListItemAlign>(number);
    auto result = listLayoutAlgorithm->CalculateLaneCrossOffset(3.0f, 2.0f, true);
    EXPECT_EQ(result, 0.0f);
}

/**
 * @tc.name: OnSurfaceChanged001
 * @tc.desc: Test ListLayoutAlgorithm OnSurfaceChanged
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, OnSurfaceChanged001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<FocusHub> focusHub = AceType::MakeRefPtr<FocusHub>(node);
    focusHub->currentFocus_ = true;
    frameNode->focusHub_ = focusHub;
    listPattern->frameNode_ = frameNode;
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    layoutWrapper.hostNode_ = frameNode;
    listLayoutAlgorithm->contentMainSize_ = 3.0f;
    listLayoutAlgorithm->prevContentMainSize_ = 5.0f;
    listLayoutAlgorithm->currentDelta_ = 2.0f;
    listLayoutAlgorithm->OnSurfaceChanged(&layoutWrapper);
    EXPECT_EQ(listLayoutAlgorithm->currentDelta_, 2.0f);
}

/**
 * @tc.name: OnSurfaceChanged002
 * @tc.desc: Test ListLayoutAlgorithm OnSurfaceChanged updates currentOffset_ (not currentDelta_)
 *           when the focused TextField caret would be clipped after the list shrinks.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, OnSurfaceChanged002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextFieldPattern> textFieldPattern = AceType::MakeRefPtr<TextFieldPattern>();
    auto textFieldNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 3, textFieldPattern);
    ASSERT_NE(textFieldNode, nullptr);
    textFieldPattern->frameNode_ = textFieldNode;
    RefPtr<PipelineContext> pipe = AceType::MakeRefPtr<PipelineContext>();
    RefPtr<TextFieldManagerNG> manager = AceType::MakeRefPtr<TextFieldManagerNG>();
    manager->onFocusTextField_ = textFieldPattern;
    pipe->SetTextFieldManager(manager);
    auto context = AceType::RawPtr(pipe);
    frameNode->context_ = context;
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<FocusHub> focusHub = AceType::MakeRefPtr<FocusHub>(node);
    focusHub->currentFocus_ = true;
    frameNode->focusHub_ = focusHub;
    listPattern->frameNode_ = frameNode;
    frameNode->layoutProperty_ = listLayoutProperty;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    layoutWrapper.hostNode_ = frameNode;
    // list shrinks (3.0 < 5.0) so the caret auto-scroll branch is reachable.
    // In the host env caretPos/globalOffset are 0 and RESERVE_BOTTOM_HEIGHT(24_vp)->24px,
    // so offset = 3 + 0 - 0 - 24 = -21 (<=0), currentOffset_ -= -21 => +21.
    listLayoutAlgorithm->contentMainSize_ = 3.0f;
    listLayoutAlgorithm->prevContentMainSize_ = 5.0f;
    listLayoutAlgorithm->currentDelta_ = 5.0f;
    listLayoutAlgorithm->currentOffset_ = 0.0f;
    listLayoutAlgorithm->OnSurfaceChanged(&layoutWrapper);
    frameNode->context_ = nullptr;
    // Patch guarantee: OnSurfaceChanged no longer mutates currentDelta_.
    EXPECT_EQ(listLayoutAlgorithm->currentDelta_, 5.0f);
    // The auto-scroll adjustment now targets currentOffset_.
    EXPECT_EQ(listLayoutAlgorithm->currentOffset_, 21.0f);
}

/**
 * @tc.name: OnSurfaceChanged003
 * @tc.desc: Test ListLayoutAlgorithm OnSurfaceChanged keeps both currentDelta_ and currentOffset_
 *           unchanged when the list does not shrink, even with a focused TextField present.
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, OnSurfaceChanged003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextFieldPattern> textFieldPattern = AceType::MakeRefPtr<TextFieldPattern>();
    auto textFieldNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 3, textFieldPattern);
    ASSERT_NE(textFieldNode, nullptr);
    textFieldPattern->frameNode_ = textFieldNode;
    RefPtr<PipelineContext> pipe = AceType::MakeRefPtr<PipelineContext>();
    RefPtr<TextFieldManagerNG> manager = AceType::MakeRefPtr<TextFieldManagerNG>();
    manager->onFocusTextField_ = textFieldPattern;
    pipe->SetTextFieldManager(manager);
    auto context = AceType::RawPtr(pipe);
    frameNode->context_ = context;
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<FocusHub> focusHub = AceType::MakeRefPtr<FocusHub>(node);
    focusHub->currentFocus_ = true;
    frameNode->focusHub_ = focusHub;
    listPattern->frameNode_ = frameNode;
    frameNode->layoutProperty_ = listLayoutProperty;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    layoutWrapper.hostNode_ = frameNode;
    // contentMainSize_ >= prevContentMainSize_: list did not shrink, returns at the first guard.
    listLayoutAlgorithm->contentMainSize_ = 10.0f;
    listLayoutAlgorithm->prevContentMainSize_ = 5.0f;
    listLayoutAlgorithm->currentDelta_ = 7.0f;
    listLayoutAlgorithm->currentOffset_ = 4.0f;
    listLayoutAlgorithm->OnSurfaceChanged(&layoutWrapper);
    frameNode->context_ = nullptr;
    EXPECT_EQ(listLayoutAlgorithm->currentDelta_, 7.0f);
    EXPECT_EQ(listLayoutAlgorithm->currentOffset_, 4.0f);
}

/**
 * @tc.name: CheckAndUpdateCurOffset001
 * @tc.desc: Test ListItemLayoutAlgorithm CheckAndUpdateCurOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndUpdateCurOffset001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListItemLayoutAlgorithm> listItemLayoutAlgorithm = AceType::MakeRefPtr<ListItemLayoutAlgorithm>(2, 4, 3);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    RectT rect(20.0f, 20.0f, 80.0f, 80.0f);
    geometryProperty.rect_ = rect;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.hostNode_ = frameNode;
    listItemLayoutAlgorithm->isCurOffsetUpdated_ = false;
    listItemLayoutAlgorithm->canUpdateCurOffset_ = true;
    listItemLayoutAlgorithm->curOffset_ = 2.0f;
    listItemLayoutAlgorithm->CheckAndUpdateCurOffset(&layoutWrapper);
    EXPECT_TRUE(listItemLayoutAlgorithm->isCurOffsetUpdated_);
    EXPECT_EQ(listItemLayoutAlgorithm->curOffset_, 80.0f);
}

/**
 * @tc.name: CheckAndUpdateCurOffset002
 * @tc.desc: Test ListItemLayoutAlgorithm CheckAndUpdateCurOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndUpdateCurOffset002, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListItemLayoutAlgorithm> listItemLayoutAlgorithm = AceType::MakeRefPtr<ListItemLayoutAlgorithm>(-1, 4, 3);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    RectT rect(20.0f, 20.0f, 80.0f, 80.0f);
    geometryProperty.rect_ = rect;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.hostNode_ = frameNode;
    listItemLayoutAlgorithm->isCurOffsetUpdated_ = false;
    listItemLayoutAlgorithm->canUpdateCurOffset_ = true;
    listItemLayoutAlgorithm->curOffset_ = 2.0f;
    listItemLayoutAlgorithm->CheckAndUpdateCurOffset(&layoutWrapper);
    EXPECT_TRUE(listItemLayoutAlgorithm->isCurOffsetUpdated_);
    EXPECT_EQ(listItemLayoutAlgorithm->curOffset_, 2.0f);
}

/**
 * @tc.name: CheckAndUpdateCurOffset003
 * @tc.desc: Test ListItemLayoutAlgorithm CheckAndUpdateCurOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndUpdateCurOffset003, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListItemLayoutAlgorithm> listItemLayoutAlgorithm = AceType::MakeRefPtr<ListItemLayoutAlgorithm>(2, 4, 3);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    RectT rect(20.0f, 20.0f, 80.0f, 80.0f);
    geometryProperty.rect_ = rect;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.hostNode_ = frameNode;
    listItemLayoutAlgorithm->isCurOffsetUpdated_ = false;
    listItemLayoutAlgorithm->canUpdateCurOffset_ = true;
    listItemLayoutAlgorithm->curOffset_ = -2.0f;
    listItemLayoutAlgorithm->CheckAndUpdateCurOffset(&layoutWrapper);
    EXPECT_TRUE(listItemLayoutAlgorithm->isCurOffsetUpdated_);
    EXPECT_EQ(listItemLayoutAlgorithm->curOffset_, -80.0f);
}

/**
 * @tc.name: CheckAndUpdateCurOffset004
 * @tc.desc: Test ListItemLayoutAlgorithm CheckAndUpdateCurOffset
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CheckAndUpdateCurOffset004, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListItemLayoutAlgorithm> listItemLayoutAlgorithm = AceType::MakeRefPtr<ListItemLayoutAlgorithm>(-2, -4, 3);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    RectT rect(20.0f, 20.0f, 80.0f, 80.0f);
    geometryProperty.rect_ = rect;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.geometryNode_ = geometryNode;
    layoutWrapper.hostNode_ = frameNode;
    listItemLayoutAlgorithm->isCurOffsetUpdated_ = false;
    listItemLayoutAlgorithm->canUpdateCurOffset_ = true;
    listItemLayoutAlgorithm->curOffset_ = -2.0f;
    listItemLayoutAlgorithm->CheckAndUpdateCurOffset(&layoutWrapper);
    EXPECT_TRUE(listItemLayoutAlgorithm->isCurOffsetUpdated_);
    EXPECT_EQ(listItemLayoutAlgorithm->curOffset_, -2.0f);
}

/**
 * @tc.name: CalculateFixOffset001
 * @tc.desc: Test ListLayoutAlgorithm CalculateFixOffset when isStackFromEnd_ is false, fix offsets should not swap
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CalculateFixOffset001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    
    listLayoutAlgorithm->contentClipMode_ = ContentClipMode::BOUNDARY;
    listLayoutAlgorithm->contentMainSize_ = 100.0f;  // 100.0f: content main size
    listLayoutAlgorithm->paddingBeforeContent_ = 10.0f;  // 10.0f: padding before content
    listLayoutAlgorithm->paddingAfterContent_ = 15.0f;  // 15.0f: padding after content
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->startFixOffset_ = 0.0f;
    listLayoutAlgorithm->endFixOffset_ = 0.0f;
    
    ScaleProperty scaleProperty;
    listLayoutAlgorithm->CalculateFixOffset(scaleProperty);
    
    // When isStackFromEnd_ is false, startFixOffset should be paddingBefore (10.0f)
    // endFixOffset should be paddingAfter (15.0f)
    EXPECT_EQ(listLayoutAlgorithm->startFixOffset_, 10.0f);  // 10.0f: preset startFixOffset
    EXPECT_EQ(listLayoutAlgorithm->endFixOffset_, 15.0f);  // 15.0f: preset endFixOffset
}

/**
 * @tc.name: CalculateFixOffset002
 * @tc.desc: Test ListLayoutAlgorithm CalculateFixOffset when isStackFromEnd_ is true, fix offsets should swap
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CalculateFixOffset002, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    
    listLayoutAlgorithm->contentClipMode_ = ContentClipMode::BOUNDARY;
    listLayoutAlgorithm->contentMainSize_ = 100.0f;  // 100.0f: content main size
    listLayoutAlgorithm->paddingBeforeContent_ = 10.0f;  // 10.0f: padding before content
    listLayoutAlgorithm->paddingAfterContent_ = 15.0f;  // 15.0f: padding after content
    listLayoutAlgorithm->isStackFromEnd_ = true;
    listLayoutAlgorithm->startFixOffset_ = 0.0f;
    listLayoutAlgorithm->endFixOffset_ = 0.0f;
    
    ScaleProperty scaleProperty;
    listLayoutAlgorithm->CalculateFixOffset(scaleProperty);
    
    // When isStackFromEnd_ is true, fix offsets should swap
    // startFixOffset becomes paddingAfter (15.0f), endFixOffset becomes paddingBefore (10.0f)
    EXPECT_EQ(listLayoutAlgorithm->startFixOffset_, 15.0f);  // 15.0f: preset startFixOffset
    EXPECT_EQ(listLayoutAlgorithm->endFixOffset_, 10.0f);  // 10.0f: preset endFixOffset
}

/**
 * @tc.name: CalculateFixOffset003
 * @tc.desc: Test ListLayoutAlgorithm CalculateFixOffset when contentClipMode is CONTENT_ONLY, should return early
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, CalculateFixOffset003, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    
    listLayoutAlgorithm->contentClipMode_ = ContentClipMode::CONTENT_ONLY;
    listLayoutAlgorithm->contentMainSize_ = 100.0f;  // 100.0f: content main size
    listLayoutAlgorithm->paddingBeforeContent_ = 10.0f;  // 10.0f: padding before content
    listLayoutAlgorithm->paddingAfterContent_ = 15.0f;  // 15.0f: padding after content
    listLayoutAlgorithm->isStackFromEnd_ = false;
    listLayoutAlgorithm->startFixOffset_ = 10.0f;  // 10.0f: preset startFixOffset
    listLayoutAlgorithm->endFixOffset_ = 15.0f;  // 15.0f: preset endFixOffset
    
    ScaleProperty scaleProperty;
    listLayoutAlgorithm->CalculateFixOffset(scaleProperty);
    
    // When contentClipMode is CONTENT_ONLY, CalculateFixOffset should return early
    // startFixOffset and endFixOffset should remain unchanged
    EXPECT_EQ(listLayoutAlgorithm->startFixOffset_, 10.0f);  // 10.0f: preset startFixOffset
    EXPECT_EQ(listLayoutAlgorithm->endFixOffset_, 15.0f);  // 15.0f: preset endFixOffset
}

/**
 * @tc.name: ProcessCacheCount001
 * @tc.desc: Test ListLayoutAlgorithm ProcessCacheCount when itemPosition is empty
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, ProcessCacheCount001, TestSize.Level1)
{
    RefPtr<ListPattern> listPattern = AceType::MakeRefPtr<ListPattern>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, listPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = listLayoutProperty;
    listPattern->frameNode_ = frameNode;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryNode->frame_ = geometryProperty;
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.layoutProperty_ = listLayoutProperty;
    layoutWrapper.hostNode_ = frameNode;
    
    listLayoutAlgorithm->itemPosition_.clear();
    listLayoutAlgorithm->startFixOffset_ = 10.0f;  // 10.0f: preset startFixOffset
    listLayoutAlgorithm->endFixOffset_ = 15.0f;  // 15.0f: preset endFixOffset
    
    int32_t cacheCount = 2;
    bool show = false;
    listLayoutAlgorithm->ProcessCacheCount(&layoutWrapper, cacheCount, show);
    
    // ProcessCacheCount should not modify fix offsets when itemPosition is empty
    EXPECT_EQ(listLayoutAlgorithm->startFixOffset_, 10.0f);  // 10.0f: preset startFixOffset
    EXPECT_EQ(listLayoutAlgorithm->endFixOffset_, 15.0f);  // 15.0f: preset endFixOffset
}

/**
 * @tc.name: ListLayoutAlgorithmCanUseInfoInPosMapWithLazyChild001
 * @tc.desc: Test CanUseInfoInPosMap rejects oversized lazy child
 * @tc.type: FUNC
 */
HWTEST_F(ListAlgorithmTestNg, ListLayoutAlgorithmCanUseInfoInPosMapWithLazyChild001, TestSize.Level1)
{
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    listLayoutAlgorithm->contentMainSize_ = 100.0f;
    listLayoutAlgorithm->totalItemCount_ = 3;
    listLayoutAlgorithm->posMap_ = AceType::MakeRefPtr<ListPositionMap>();
    listLayoutAlgorithm->posMap_->UpdatePos(0, { 0.0f, 250.0f, false, false });
    listLayoutAlgorithm->posMap_->UpdatePos(1, { 260.0f, 250.0f, true, false });
    listLayoutAlgorithm->posMap_->UpdatePos(2, { 520.0f, 250.0f, false, true });

    EXPECT_TRUE(listLayoutAlgorithm->CanUseInfoInPosMap(0, 0.0f));
    EXPECT_FALSE(listLayoutAlgorithm->CanUseInfoInPosMap(1, 0.0f));
    EXPECT_FALSE(listLayoutAlgorithm->CanUseInfoInPosMap(2, 0.0f));
}

namespace {
constexpr float END_ADJUST_VIEWPORT = 400.0f;
constexpr int32_t END_ADJUST_ITEM_COUNT = 1;
// 2^23 px: from this content end on the float ULP is already 1.0px, so a sub-pixel contentEndOffset is
// rounded away when the end-of-content offset adjustment is evaluated in float instead of double.
constexpr float END_ADJUST_HUGE_END_POS = 8388608.0f;
constexpr float END_ADJUST_HUGE_END_OFFSET = 0.5f;
constexpr double END_ADJUST_HUGE_CURRENT_OFFSET = 8388700.0;
constexpr double END_ADJUST_HUGE_EXPECTED = 8388208.5;
// What the same expression yields once hugeEndPos + hugeEndOffset collapses back to hugeEndPos in float.
constexpr double END_ADJUST_HUGE_ROUNDED = 8388208.0;
constexpr float END_ADJUST_SMALL_END_POS = 1000.0f;
constexpr float END_ADJUST_SMALL_END_OFFSET = 50.0f;
constexpr double END_ADJUST_SMALL_CURRENT_OFFSET = 700.0;
constexpr double END_ADJUST_SMALL_EXPECTED = 650.0;
} // namespace

class ListForwardEndOffsetTestNg : public TestNG {
public:
    void PrepareForwardEndOffset(float contentEndPos, float contentEndOffset, double currentOffset,
        bool canOverScrollEnd, bool hasJumpIndex);
    void RunForwardEndOffsetAdjust();

protected:
    RefPtr<ListLayoutAlgorithm> algorithm_;
    RefPtr<LayoutWrapperNode> wrapper_;
    std::vector<RefPtr<FrameNode>> nodes_;
    float contentEndPos_ = 0.0f;
};

void ListForwardEndOffsetTestNg::PrepareForwardEndOffset(float contentEndPos, float contentEndOffset,
    double currentOffset, bool canOverScrollEnd, bool hasJumpIndex)
{
    contentEndPos_ = contentEndPos;
    algorithm_ = AceType::MakeRefPtr<ListLayoutAlgorithm>();
    auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ListPattern>());
    ASSERT_NE(list, nullptr);
    nodes_.push_back(list);
    wrapper_ = AceType::MakeRefPtr<LayoutWrapperNode>(list, list->GetGeometryNode(), list->GetLayoutProperty());
    wrapper_->currentChildCount_ = 0;
    algorithm_->totalItemCount_ = END_ADJUST_ITEM_COUNT;
    algorithm_->contentMainSize_ = END_ADJUST_VIEWPORT;
    algorithm_->contentEndOffset_ = contentEndOffset;
    algorithm_->contentStartOffset_ = 0.0f;
    algorithm_->canOverScrollEnd_ = canOverScrollEnd;
    algorithm_->overScrollFeature_ = canOverScrollEnd;
    algorithm_->prevItemPosCount_ = END_ADJUST_ITEM_COUNT;
    algorithm_->currentOffset_ = currentOffset;
    algorithm_->startMainPos_ = static_cast<float>(currentOffset);
    algorithm_->endMainPos_ = static_cast<float>(currentOffset) + END_ADJUST_VIEWPORT;
    if (hasJumpIndex) {
        algorithm_->jumpIndex_ = 0;
    }
    algorithm_->itemPosition_[0] = { 0, 0.0f, contentEndPos, false };
}

void ListForwardEndOffsetTestNg::RunForwardEndOffsetAdjust()
{
    ASSERT_NE(algorithm_, nullptr);
    ASSERT_NE(wrapper_, nullptr);
    // startIndex == totalItemCount_ makes LayoutALineForward lay out nothing, so the do-while exits with
    // currentEndPos still at the prepared content end and the tail "adjust offset" branch is reached.
    algorithm_->LayoutForward(AceType::RawPtr(wrapper_), END_ADJUST_ITEM_COUNT, contentEndPos_);
}

/**
 * @tc.name: LayoutForwardEndOffsetAdjustKeepsDoublePrecision001
 * @tc.desc: End-of-content offset adjustment keeps double precision for a huge content end
 * @tc.type: FUNC
 */
HWTEST_F(ListForwardEndOffsetTestNg, LayoutForwardEndOffsetAdjustKeepsDoublePrecision001, TestSize.Level1)
{
    PrepareForwardEndOffset(END_ADJUST_HUGE_END_POS, END_ADJUST_HUGE_END_OFFSET,
        END_ADJUST_HUGE_CURRENT_OFFSET, false, false);
    RunForwardEndOffsetAdjust();
    EXPECT_NE(algorithm_->currentOffset_, END_ADJUST_HUGE_ROUNDED);
    EXPECT_DOUBLE_EQ(algorithm_->currentOffset_, END_ADJUST_HUGE_EXPECTED);
}

/**
 * @tc.name: LayoutForwardEndOffsetAdjustKeepsDoublePrecision002
 * @tc.desc: End-of-content offset adjustment is unchanged for an ordinary content end
 * @tc.type: FUNC
 */
HWTEST_F(ListForwardEndOffsetTestNg, LayoutForwardEndOffsetAdjustKeepsDoublePrecision002, TestSize.Level1)
{
    PrepareForwardEndOffset(END_ADJUST_SMALL_END_POS, END_ADJUST_SMALL_END_OFFSET,
        END_ADJUST_SMALL_CURRENT_OFFSET, false, false);
    RunForwardEndOffsetAdjust();
    EXPECT_DOUBLE_EQ(algorithm_->currentOffset_, END_ADJUST_SMALL_EXPECTED);
}

/**
 * @tc.name: LayoutForwardEndOffsetAdjustKeepsDoublePrecision003
 * @tc.desc: End-of-content offset adjustment keeps double precision on a spring jump to the last item
 * @tc.type: FUNC
 */
HWTEST_F(ListForwardEndOffsetTestNg, LayoutForwardEndOffsetAdjustKeepsDoublePrecision003, TestSize.Level1)
{
    PrepareForwardEndOffset(END_ADJUST_HUGE_END_POS, END_ADJUST_HUGE_END_OFFSET,
        END_ADJUST_HUGE_CURRENT_OFFSET, true, true);
    RunForwardEndOffsetAdjust();
    EXPECT_NE(algorithm_->currentOffset_, END_ADJUST_HUGE_ROUNDED);
    EXPECT_DOUBLE_EQ(algorithm_->currentOffset_, END_ADJUST_HUGE_EXPECTED);
}
} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"

#include "base/log/event_report.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_node.h"

namespace OHOS::Ace::NG {

void ListLanesLayoutAlgorithm::UpdateLineItemPositions(
    int32_t startIndex, int32_t endIndex, float startPos, float endPos)
{
    for (int32_t index = startIndex; index <= endIndex; index++) {
        auto iter = itemPosition_.find(index);
        if (iter == itemPosition_.end()) {
            TAG_LOGW(AceLogTag::ACE_LIST,
                "Update line item position failed, index:%{public}d, start:%{public}d, end:%{public}d",
                index, startIndex, endIndex);
            continue;
        }
        iter->second.startPos = startPos;
        iter->second.endPos = endPos;
    }
}

bool ListLanesLayoutAlgorithm::TryUseFirstItemInfo(int32_t& currentIndex, float& linePos, bool forward)
{
    if (!firstItemInfo_) {
        return false;
    }
    int32_t expectedIndex = currentIndex + (forward ? 1 : -1);
    if (firstItemInfo_->first != expectedIndex) {
        firstItemInfo_.reset();
        return false;
    }
    currentIndex += forward ? 1 : -1;
    linePos = forward ? firstItemInfo_->second.endPos : firstItemInfo_->second.startPos;
    itemPosition_[currentIndex] = std::move(firstItemInfo_->second);
    firstItemInfo_.reset();
    return true;
}

void ListLanesLayoutAlgorithm::UpdateListItemConstraint(
    Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint)
{
    contentConstraint.parentIdealSize = selfIdealSize;
    contentConstraint.maxSize.SetMainSize(LayoutInfinity<float>(), axis);
    groupLayoutConstraint_ = contentConstraint;
    auto crossSizeOptional = selfIdealSize.CrossSize(axis);
    if (crossSizeOptional.has_value()) {
        float crossSize = crossSizeOptional.value();
        groupLayoutConstraint_.maxSize.SetCrossSize(crossSize, axis);
        if (lanes_ > 1) {
            float laneGutter = GetLaneGutter();
            crossSize = (crossSize + laneGutter) / lanes_ - laneGutter;
            crossSize = crossSize <= 0 ? 1 : crossSize;
        }
        if (maxLaneLength_.has_value() && maxLaneLength_.value() < crossSize) {
            crossSize = maxLaneLength_.value();
        }
        contentConstraint.percentReference.SetCrossSize(crossSize, axis);
        contentConstraint.parentIdealSize.SetCrossSize(crossSize, axis);
        contentConstraint.maxSize.SetCrossSize(crossSize, axis);
        if (minLaneLength_.has_value()) {
            contentConstraint.minSize.SetCrossSize(minLaneLength_.value(), axis);
        }
    }
}

float ListLanesLayoutAlgorithm::GetChildHeight(LayoutWrapper* layoutWrapper, int32_t childIndex)
{
    CHECK_NULL_RETURN(childrenSize_, 0.0f);
    float mainLen = 0.0f;
    int32_t laneCeil = GetLanesCeil(layoutWrapper, childIndex);
    for (int32_t index = GetLanesFloor(layoutWrapper, childIndex); index <= laneCeil; index++) {
        mainLen = std::max(mainLen, childrenSize_->GetChildSize(index, isStackFromEnd_));
    }
    return mainLen;
}

float ListLanesLayoutAlgorithm::MeasureAndGetChildHeight(LayoutWrapper* layoutWrapper, int32_t childIndex,
    bool groupLayoutAll)
{
    auto wrapper = GetListItem(layoutWrapper, childIndex);
    if (!wrapper) {
        ReportGetChildError("MeasureAndGetChildHeightLanes", childIndex);
        return 0.0f;
    }
    bool isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    float mainLen = 0.0f;
    if (isGroup) {
        auto listLayoutProperty =
            AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
        // true: layout forward, true: layout all group items.
        SetListItemGroupParam(wrapper, childIndex, 0.0f, true, listLayoutProperty, true);
        wrapper->Measure(groupLayoutConstraint_);
        mainLen = GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_);
        CheckGroupMeasureBreak(wrapper);
    } else {
        auto laneCeil = GetLanesCeil(layoutWrapper, childIndex);
        for (int32_t i = GetLanesFloor(layoutWrapper, childIndex); i <= laneCeil; i++) {
            auto wrapper = GetListItem(layoutWrapper, i);
            if (!wrapper) {
                ReportGetChildError("MeasureAndGetChildHeightLanesItem", i);
                continue;
            }
            wrapper->Measure(childLayoutConstraint_);
            mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
        }
    }
    return mainLen;
}

void ListLanesLayoutAlgorithm::MeasureGroup(LayoutWrapper* listWrapper, const RefPtr<LayoutWrapper>& groupWrapper,
    int32_t index, float& pos, bool forward)
{
    CHECK_NULL_VOID(groupWrapper);
    auto host = groupWrapper->GetHostNode();
    const char* direction = forward ? "Forward" : "Backward";
    if (host) {
        ACE_SCOPED_TRACE("[Measure%sListItemGroup:%d][self:%d][parent:%d]", direction, index, host->GetId(),
            host->GetParent() ? host->GetParent()->GetId() : 0);
    }
    auto listLayoutProperty = AceType::DynamicCast<ListLayoutProperty>(listWrapper->GetLayoutProperty());
    SetListItemGroupParam(groupWrapper, index, pos, forward, listLayoutProperty, false);
    groupWrapper->Measure(groupLayoutConstraint_);
    if (forward && (LessOrEqual(pos, 0.0f) || GetPrevMeasureBreak())) {
        AdjustStartPosition(groupWrapper, pos);
    }
    CheckGroupMeasureBreak(groupWrapper);
}

void ListLanesLayoutAlgorithm::MeasureItem(const RefPtr<LayoutWrapper>& itemWrapper, int32_t index, bool forward)
{
    CHECK_NULL_VOID(itemWrapper);
    auto host = itemWrapper->GetHostNode();
    const char* direction = forward ? "Forward" : "Backward";
    if (host) {
        ACE_SCOPED_TRACE("[Measure%sListItem:%d][self:%d][parent:%d]", direction, index, host->GetId(),
            host->GetParent() ? host->GetParent()->GetId() : 0);
    }
    UpdateListItemEditModeCheckBoxSpace(itemWrapper);
    itemWrapper->Measure(childLayoutConstraint_);
}

int32_t ListLanesLayoutAlgorithm::LayoutALineForward(LayoutWrapper* layoutWrapper,
    int32_t& currentIndex, float startPos, float& endPos)
{
    float mainLen = 0.0f;
    bool isGroup = false;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    if (TryUseFirstItemInfo(currentIndex, endPos, true)) {
        return 1;
    }
    for (int32_t i = 0; i < lanes && currentIndex + 1 <= GetMaxListItemIndex() && !isGroup; i++) {
        auto wrapper = GetListItem(layoutWrapper, currentIndex + 1);
        if (!wrapper) {
            ReportGetChildError("LayoutALineForwardLanes", currentIndex + 1);
            break;
        }
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            wrapper->SetActive(false);
            isGroup = false;
            break;
        }
        cnt++;
        ++currentIndex;
        mainLen = std::max(mainLen, MeasureLineItem(layoutWrapper, wrapper, currentIndex, startPos, true, isGroup));
    }
    if (cnt > 0) {
        endPos = startPos + mainLen;
        UpdateLineItemPositions(currentIndex - cnt + 1, currentIndex, startPos, endPos);
    }
    return cnt;
}

int32_t ListLanesLayoutAlgorithm::LayoutALineBackward(LayoutWrapper* layoutWrapper,
    int32_t& currentIndex, float endPos, float& startPos)
{
    float mainLen = 0.0f;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    if (TryUseFirstItemInfo(currentIndex, startPos, false)) {
        return 1;
    }
    for (int32_t i = 0; i < lanes && currentIndex - 1 >= 0; i++) {
        if (currentIndex > GetMaxListItemIndex() + 1) {
            --currentIndex;
            continue;
        }
        auto wrapper = GetListItem(layoutWrapper, currentIndex - 1);
        if (!wrapper) {
            ReportGetChildError("LayoutALineBackwardLanes", currentIndex - 1);
            break;
        }
        bool isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            wrapper->SetActive(false);
            isGroup = false;
            break;
        }
        --currentIndex;
        cnt++;
        mainLen = std::max(mainLen, MeasureLineItem(layoutWrapper, wrapper, currentIndex, endPos, false, isGroup));
        if (CheckCurRowMeasureFinished(layoutWrapper, currentIndex, isGroup)) {
            break;
        }
    }
    if (cnt > 0) {
        startPos = endPos - mainLen;
        UpdateLineItemPositions(currentIndex, currentIndex + cnt - 1, startPos, endPos);
    }
    return cnt;
}

bool ListLanesLayoutAlgorithm::CheckCurRowMeasureFinished(LayoutWrapper* layoutWrapper, int32_t curIndex, bool isGroup)
{
    if (childrenSize_) {
        return isGroup || posMap_->GetRowStartIndex(curIndex) == curIndex;
    }
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    return isGroup || (curIndex - FindLanesStartIndex(layoutWrapper, curIndex)) % lanes == 0;
}

void ListLanesLayoutAlgorithm::SetCacheCount(LayoutWrapper* layoutWrapper, int32_t cacheCount)
{
    bool hasGroup = false;
    auto& itemPosition = GetItemPosition();
    for (auto &pos : itemPosition) {
        if (pos.second.isGroup) {
            hasGroup = true;
            break;
        }
    }
    int32_t count = hasGroup ? cacheCount : cacheCount * lanes_;
    layoutWrapper->SetCacheCount(count);
}

int32_t ListLanesLayoutAlgorithm::CalculateLanesParam(std::optional<float>& minLaneLength,
    std::optional<float>& maxLaneLength, int32_t lanes, std::optional<float> crossSizeOptional, float laneGutter)
{
    if (lanes < 1) {
        return 1;
    }
    // Case 1: lane length constrain is not set
    //      1.1: use [lanes] set by user if [lanes] is set
    //      1.2: set [lanes] to 1 if [lanes] is not set
    if (!crossSizeOptional.has_value() || GreaterOrEqualToInfinity(crossSizeOptional.value()) ||
        !minLaneLength.has_value() || !maxLaneLength.has_value()) {
        maxLaneLength.reset();
        minLaneLength.reset();
        return lanes;
    }
    // Case 2: lane length constrain is set --> need to calculate [lanes_] according to contraint.
    // We agreed on such rules (assuming we have a vertical list here):
    // rule 1: [minLaneLength_] has a higher priority than [maxLaneLength_] when decide [lanes_], for e.g.,
    //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 120,
    //         the [lanes_] is 3 rather than 2.
    // rule 2: after [lanes_] is determined by rule 1, the width of lane will be as large as it can be, for
    // e.g.,
    //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 132, the [lanes_] is 3
    //         according to rule 1, then the width of lane will be 132 / 3 = 44 rather than 40,
    //         its [minLaneLength_].
    auto crossSize = crossSizeOptional.value();
    ModifyLaneLength(minLaneLength, maxLaneLength, crossSize);

    // if minLaneLength is 40, maxLaneLength is 60
    // when list's width is 120, lanes_ = 3
    // when list's width is 80, lanes_ = 2
    // when list's width is 70, lanes_ = 1
    float maxLanes = (crossSize + laneGutter) / (minLaneLength.value() + laneGutter);
    float minLanes = (crossSize + laneGutter) / (maxLaneLength.value() + laneGutter);
    // let's considerate scenarios when maxCrossSize > 0
    // now it's guaranteed that [minLaneLength_] <= [maxLaneLength_], i.e., maxLanes >= minLanes > 0
    // there are 3 scenarios:
    // 1. 1 > maxLanes >= minLanes > 0
    // 2. maxLanes >= 1 >= minLanes > 0
    // 3. maxLanes >= minLanes > 1
    // 1. 1 > maxLanes >= minLanes > 0 ---> maxCrossSize < minLaneLength_ =< maxLaneLength
    if (GreatNotEqual(1, maxLanes) && GreatOrEqual(maxLanes, minLanes)) {
        lanes = 1;
        minLaneLength = crossSize;
        maxLaneLength = crossSize;
        return lanes;
    }
    // 2. maxLanes >= 1 >= minLanes > 0 ---> minLaneLength_ = maxCrossSize < maxLaneLength
    if (GreatOrEqual(maxLanes, 1) && LessOrEqual(minLanes, 1)) {
        lanes = std::floor(maxLanes);
        maxLaneLength = crossSize;
        return lanes;
    }
    // 3. maxLanes >= minLanes > 1 ---> minLaneLength_ <= maxLaneLength < maxCrossSize
    if (GreatOrEqual(maxLanes, minLanes) && GreatNotEqual(minLanes, 1)) {
        lanes = std::floor(maxLanes);
        return lanes;
    }
    lanes = 1;
    return lanes;
}

void ListLanesLayoutAlgorithm::CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
    const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis)
{
    const auto& contentConstraintOps = layoutProperty->GetContentLayoutConstraint();
    CHECK_NULL_VOID(contentConstraintOps);
    auto contentConstraint = contentConstraintOps.value();
    auto mainPercentRefer = GetMainAxisSize(contentConstraint.percentReference, axis);
    int32_t lanes = layoutProperty->GetLanes().value_or(1);
    if (layoutProperty->GetItemFillPolicy().has_value()) {
        auto fillType = layoutProperty->GetItemFillPolicy();
        const auto& padding = listLayoutProperty_->CreatePaddingAndBorder();
        auto leftPadding = axis == Axis::HORIZONTAL ? padding.top.value_or(0) : padding.left.value_or(0);
        auto rightPadding = axis == Axis::HORIZONTAL ? padding.bottom.value_or(0) : padding.right.value_or(0);
        WidthBreakpoint point = GetCommonWidthBreakpoint(
            (crossSizeOptional.value_or(0.0) + leftPadding + rightPadding), contentConstraint.scaleProperty.vpScale);
        lanes = GetListLanesByFillType(fillType.value(), point);
    }
    lanes = lanes > 1 ? lanes : 1;
    if (layoutProperty->GetLaneMinLength().has_value()) {
        minLaneLength_ =
            ConvertToPx(layoutProperty->GetLaneMinLength().value(), layoutConstraint.scaleProperty, mainPercentRefer);
    }
    if (layoutProperty->GetLaneMaxLength().has_value()) {
        maxLaneLength_ =
            ConvertToPx(layoutProperty->GetLaneMaxLength().value(), layoutConstraint.scaleProperty, mainPercentRefer);
    }
    float laneGutter = 0.0f;
    if (layoutProperty->GetLaneGutter().has_value()) {
        laneGutter = ConvertToPx(layoutProperty->GetLaneGutter().value(),
            layoutConstraint.scaleProperty, crossSizeOptional.value_or(0.0)).value_or(0.0f);
        SetLaneGutter(laneGutter);
    }
    lanes_ = CalculateLanesParam(minLaneLength_, maxLaneLength_, lanes, crossSizeOptional, laneGutter);
}

void ListLanesLayoutAlgorithm::ModifyLaneLength(
    std::optional<float>& minLaneLength, std::optional<float>& maxLaneLength, float crossSize)
{
    if (GreatNotEqual(minLaneLength.value(), maxLaneLength.value())) {
        maxLaneLength = minLaneLength;
    }
}

float ListLanesLayoutAlgorithm::CalculateLaneCrossOffset(float crossSize, float childCrossSize, bool isGroup)
{
    if (lanes_ <= 0) {
        return 0.0f;
    }
    if (isGroup) {
        return ListLayoutAlgorithm::CalculateLaneCrossOffset(crossSize, childCrossSize, isGroup);
    }
    return ListLayoutAlgorithm::CalculateLaneCrossOffset((crossSize + GetLaneGutter()) / lanes_,
        childCrossSize / lanes_, isGroup);
}

float ListLanesLayoutAlgorithm::MeasureLineItem(LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& wrapper,
    int32_t index, float& referencePos, bool forward, bool isGroup)
{
    auto childNode = wrapper->GetHostNode();
    const bool isLazyChild = !isGroup && CanSupportNestedLazy(childNode, layoutWrapper->GetHostNode(), GetLanes());
    const int32_t id = childNode ? childNode->GetId() : -1;
    SetItemInfo(index, { id, 0.0f, 0.0f, isGroup, isLazyChild });
    if (isGroup) {
        MeasureGroup(layoutWrapper, wrapper, index, referencePos, forward);
    } else if (isLazyChild) {
        MeasureLazyChild(wrapper, index, referencePos, forward);
    } else if (CheckNeedMeasure(wrapper)) {
        MeasureItem(wrapper, index, forward);
    }
    return childrenSize_ ? childrenSize_->GetChildSize(index, isStackFromEnd_) :
        GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_);
}

RefPtr<LayoutWrapper> ListLanesLayoutAlgorithm::GetFastScrollLanesItem(
    LayoutWrapper* layoutWrapper, int32_t index) const
{
    const bool visible = itemPosition_.count(index) ||
        (listLayoutProperty_ && listLayoutProperty_->GetShowCachedItemsValue(false) &&
            cachedItemPosition_.count(index));
    const int32_t childIndex = index + itemStartIndex_;
    // An empty-branch placeholder cannot confirm the actual item type.
    return layoutWrapper->GetOrCreateChildByIndex(childIndex, visible, !visible);
}

bool ListLanesLayoutAlgorithm::BuildFastScrollLanesRange(LayoutWrapper* layoutWrapper, int32_t endIndex)
{
    if (!layoutWrapper || !posMap_ || lanes_ <= 1 || endIndex < 0 || endIndex >= totalItemCount_) {
        return false;
    }
    posMap_->UpdateTotalCount(totalItemCount_);
    const int32_t firstPhysical = itemPosition_.empty() ? endIndex :
        std::min(endIndex, itemPosition_.begin()->first);
    const int32_t lastPhysical = itemPosition_.empty() ? endIndex :
        std::max(endIndex, itemPosition_.rbegin()->first);
    const int32_t first = isStackFromEnd_ ? totalItemCount_ - lastPhysical - 1 : firstPhysical;
    const int32_t last = isStackFromEnd_ ? totalItemCount_ - firstPhysical - 1 : lastPhysical;
    auto types = GetFastScrollTypeRange(layoutWrapper, first);
    CHECK_NULL_RETURN(types, false);
    auto start = FindFastScrollSegmentStart(layoutWrapper, first, types.value());
    CHECK_NULL_RETURN(start, false);
    if (!ConfirmFastScrollLanesRange(layoutWrapper, first, last, start.value(), types.value())) {
        return false;
    }
    return GetPhysicalLanesRow(endIndex).has_value();
}

std::optional<ListPositionMap::EstimateRange> ListLanesLayoutAlgorithm::GetFastScrollLazyRange(
    const RefPtr<FrameNode>& node, int32_t index) const
{
    auto parent = node->GetParent();
    int32_t localIndex = -1;
    if (const auto lazy = AceType::DynamicCast<LazyForEachNode>(parent)) {
        localIndex = lazy->GetFrameNodeIndex(node);
    } else {
        while (parent && !AceType::InstanceOf<FrameNode>(parent)) {
            if (AceType::InstanceOf<RepeatVirtualScrollNode>(parent) ||
                AceType::InstanceOf<RepeatVirtualScroll2Node>(parent)) {
                localIndex = parent->GetFrameNodeIndex(node);
                break;
            }
            parent = parent->GetParent();
        }
    }
    if (localIndex < 0 || !parent) {
        return std::nullopt;
    }
    const int32_t count = parent->FrameCount();
    const int32_t start = index - localIndex;
    if (localIndex >= count || start < 0 || count > totalItemCount_ - start) {
        return std::nullopt;
    }
    return ListPositionMap::EstimateRange { start, start + count - 1 };
}

std::optional<ListLanesLayoutAlgorithm::FastScrollTypeRange> ListLanesLayoutAlgorithm::GetFastScrollTypeRange(
    LayoutWrapper* layoutWrapper, int32_t index) const
{
    const auto* state = GetFastScrollState();
    if (state && state->input.mode == FastScrollMode::SKIP_CREATE_AND_MEASURE && !itemPosition_.empty()) {
        return FastScrollTypeRange { 0, totalItemCount_ - 1, itemPosition_.begin()->second.isGroup };
    }
    const int32_t physical = isStackFromEnd_ ? totalItemCount_ - index - 1 : index;
    const auto wrapper = GetFastScrollLanesItem(layoutWrapper, physical);
    CHECK_NULL_RETURN(wrapper && wrapper->GetHostNode(), std::nullopt);
    FastScrollTypeRange types { index, index, wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG };
    if (const auto range = GetFastScrollLazyRange(wrapper->GetHostNode(), physical)) {
        types.startIndex = isStackFromEnd_ ? totalItemCount_ - range->endIndex - 1 : range->startIndex;
        types.endIndex = isStackFromEnd_ ? totalItemCount_ - range->startIndex - 1 : range->endIndex;
    }
    return types;
}

std::optional<int32_t> ListLanesLayoutAlgorithm::FindFastScrollSegmentStart(
    LayoutWrapper* layoutWrapper, int32_t index, const FastScrollTypeRange& types)
{
    auto current = types;
    while (index >= 0) {
        if (current.isGroup) {
            RecordFastScrollItemTypes(index, index, true);
            RecordFastScrollLanesRange(index, index);
            return types.isGroup ? index : index + 1;
        }
        auto cached = lanesItemRange_.upper_bound(index);
        if (cached != lanesItemRange_.begin() && (--cached)->second >= index) {
            return cached->first;
        }
        if (current.startIndex == 0) {
            return 0;
        }
        index = current.startIndex - 1;
        auto previous = GetFastScrollTypeRange(layoutWrapper, index);
        CHECK_NULL_RETURN(previous, std::nullopt);
        current = previous.value();
    }
    return std::nullopt;
}

bool ListLanesLayoutAlgorithm::ConfirmFastScrollLanesRange(LayoutWrapper* layoutWrapper,
    int32_t first, int32_t last, int32_t segmentStart, FastScrollTypeRange types)
{
    int32_t index = first;
    while (index < totalItemCount_) {
        if (types.isGroup) {
            const int32_t end = std::min(types.endIndex, std::max(index, last));
            RecordFastScrollItemTypes(index, end, true);
            for (; index <= end; ++index) {
                RecordFastScrollLanesRange(index, index);
            }
            segmentStart = index;
        } else {
            const int32_t rowStart = std::max(index, last) - (std::max(index, last) - segmentStart) % lanes_;
            const int32_t rowEnd = rowStart + std::min(lanes_ - 1, totalItemCount_ - rowStart - 1);
            const int32_t end = std::min(types.endIndex, rowEnd);
            RecordFastScrollItemTypes(index, end, false);
            RecordFastScrollLanesRange(segmentStart, end);
            index = end + 1;
        }
        if (index == totalItemCount_ || (index > last && (index - segmentStart) % lanes_ == 0)) {
            break;
        }
        auto next = GetFastScrollTypeRange(layoutWrapper, index);
        CHECK_NULL_RETURN(next, false);
        types = next.value();
    }
    return true;
}

void ListLanesLayoutAlgorithm::RecordFastScrollItemType(int32_t index, bool isGroup)
{
    auto info = posMap_->GetPositionInfo(index);
    info.isGroup = isGroup;
    posMap_->UpdatePos(index, info);
}

void ListLanesLayoutAlgorithm::RecordFastScrollItemTypes(int32_t start, int32_t end, bool isGroup)
{
    for (int32_t index = start; index <= end; ++index) {
        RecordFastScrollItemType(isStackFromEnd_ ? totalItemCount_ - index - 1 : index, isGroup);
    }
}

std::map<int32_t, int32_t>::iterator ListLanesLayoutAlgorithm::ReplaceFastScrollLanesRange(
    int32_t start, int32_t& end)
{
    auto suffix = lanesItemRange_.upper_bound(end);
    if (suffix != lanesItemRange_.begin()) {
        --suffix;
        if (suffix->second > end) {
            const int32_t physical = isStackFromEnd_ ? totalItemCount_ - start - 1 : start;
            if (!posMap_->GetPositionInfo(physical).isGroup) {
                end = suffix->second;
            } else {
                // A confirmed Group establishes the next ordinary segment's true origin.
                lanesItemRange_.emplace(end + 1, suffix->second);
            }
        }
    }
    auto first = lanesItemRange_.upper_bound(start);
    if (first != lanesItemRange_.begin()) {
        auto previous = std::prev(first);
        if (previous->second >= start) {
            if (previous->first < start) {
                previous->second = start - 1;
            } else {
                first = previous;
            }
        }
    }
    while (first != lanesItemRange_.end() && first->first <= end) {
        first = lanesItemRange_.erase(first);
    }
    return first;
}

void ListLanesLayoutAlgorithm::RecordFastScrollLanesRange(int32_t start, int32_t end)
{
    const int32_t physical = isStackFromEnd_ ? totalItemCount_ - start - 1 : start;
    auto segment = lanesItemRange_.find(start);
    if (segment != lanesItemRange_.end() && !posMap_->GetPositionInfo(physical).isGroup) {
        segment->second = std::max(segment->second, end);
        auto next = std::next(segment);
        while (next != lanesItemRange_.end() && next->first <= segment->second) {
            next = lanesItemRange_.erase(next);
        }
        return;
    }
    const auto next = ReplaceFastScrollLanesRange(start, end);
    lanesItemRange_.emplace_hint(next, start, end);
}

bool ListLanesLayoutAlgorithm::ExtendFastScrollLanesRange(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (GetPhysicalLanesRow(index)) {
        return true;
    }
    const int32_t logical = isStackFromEnd_ ? totalItemCount_ - index - 1 : index;
    auto types = GetFastScrollTypeRange(layoutWrapper, logical);
    CHECK_NULL_RETURN(types, false);
    auto start = FindFastScrollSegmentStart(layoutWrapper, logical, types.value());
    return start && ConfirmFastScrollLanesRange(layoutWrapper, logical, logical, start.value(), types.value()) &&
        GetPhysicalLanesRow(index).has_value();
}

bool ListLanesLayoutAlgorithm::PrepareFastScrollRows(LayoutWrapper* layoutWrapper)
{
    if (lanes_ == 1) {
        return true;
    }
    const auto* state = GetFastScrollState();
    return state && state->input.targetIndex && !itemPosition_.empty() &&
        BuildFastScrollLanesRange(layoutWrapper, state->input.targetIndex.value());
}

bool ListLanesLayoutAlgorithm::MeasureFastScrollRow(
    LayoutWrapper* layoutWrapper, int32_t index, float extent, bool forward, float& size)
{
    if (lanes_ == 1) {
        return MeasureFastScrollItem(layoutWrapper, index, extent, forward, size);
    }
    if (!ExtendFastScrollLanesRange(layoutWrapper, index)) {
        return false;
    }
    const auto row = GetPhysicalLanesRow(index).value();
    size = 0.0f;
    for (int32_t item = row.startIndex; item <= row.endIndex; ++item) {
        float itemSize = 0.0f;
        if (!MeasureFastScrollItem(layoutWrapper, item, extent, forward, itemSize)) {
            return false;
        }
        size = std::max(size, itemSize);
    }
    return true;
}

bool ListLanesLayoutAlgorithm::MeasureFastScrollNeighbors(LayoutWrapper* layoutWrapper, float size)
{
    if (lanes_ == 1) {
        return ListLayoutAlgorithm::MeasureFastScrollNeighbors(layoutWrapper, size);
    }
    const auto target = GetPhysicalLanesRow(GetFastScrollState()->input.targetIndex.value()).value();
    const auto [start, end] = GetFastScrollPreparationRange(size);
    if (!std::isfinite(start) || !std::isfinite(end)) {
        GetOrCreateFastScrollState().result.status = FastScrollMeasureStatus::INVALID;
        return false;
    }
    float position = size + spaceWidth_;
    int32_t index = target.endIndex + 1;
    while (index < totalItemCount_ && LessNotEqual(position, end)) {
        if (!MeasureFastScrollRow(layoutWrapper, index, end - position, true, size)) {
            return false;
        }
        position += size + spaceWidth_;
        index = GetPhysicalLanesRow(index)->endIndex + 1;
    }
    position = -spaceWidth_;
    index = target.startIndex - 1;
    while (index >= 0 && GreatNotEqual(position, start)) {
        if (!MeasureFastScrollRow(layoutWrapper, index, position - start, false, size)) {
            return false;
        }
        position -= size + spaceWidth_;
        index = GetPhysicalLanesRow(index)->startIndex - 1;
    }
    return true;
}

float ListLanesLayoutAlgorithm::GetFastScrollRowHeight(int32_t start, int32_t end) const
{
    const float sample = posMap_->GetAverageRowHeight(lanes_).value_or(0.0f);
    const auto* result = GetFastScrollMeasureResult();
    float size = 0.0f;
    for (int32_t index = start; index <= end; ++index) {
        const float previous = posMap_->GetPositionInfo(index).mainSize;
        float itemSize = Negative(previous) ? sample : previous;
        if (result) {
            const auto measured = result->measuredSizes.find(index);
            if (measured != result->measuredSizes.end()) {
                itemSize = measured->second.mainSize;
            }
        }
        size = std::max(size, itemSize);
    }
    return size;
}

bool ListLanesLayoutAlgorithm::IsFastScrollForward(float size) const
{
    if (lanes_ == 1 || childrenSize_ || itemPosition_.empty() || lanesItemRange_.empty()) {
        return ListLayoutAlgorithm::IsFastScrollForward(size);
    }
    const auto* state = GetFastScrollState();
    if (!state || !state->input.targetIndex) {
        return false;
    }
    const auto target = GetPhysicalLanesRow(state->input.targetIndex.value());
    if (!target) {
        return ListLayoutAlgorithm::IsFastScrollForward(size);
    }
    const int32_t visible = itemPosition_.begin()->first;
    float position = itemPosition_.begin()->second.startPos;
    const int32_t first = std::min(visible, target->startIndex);
    const int32_t last = std::max(visible, target->startIndex);
    for (int32_t index = first; index < last;) {
        const auto row = GetPhysicalLanesRow(index);
        CHECK_NULL_RETURN(row, ListLayoutAlgorithm::IsFastScrollForward(size));
        const float height = GetFastScrollRowHeight(row->startIndex, row->endIndex) + spaceWidth_;
        position += target->startIndex >= visible ? height : -height;
        index = row->endIndex + 1;
    }
    return NonNegative(position + GetFastScrollWindowStart(size));
}

std::optional<ListPositionMap::EstimateRange> ListLanesLayoutAlgorithm::GetFastScrollLanesRow(int32_t index) const
{
    if (!posMap_ || lanesItemRange_.empty()) {
        return std::nullopt;
    }
    const int32_t physical = isStackFromEnd_ ? totalItemCount_ - index - 1 : index;
    auto row = posMap_->GetLanesRow(physical, lanes_, lanesItemRange_, isStackFromEnd_);
    if (row && isStackFromEnd_) {
        row = ListPositionMap::EstimateRange {
            totalItemCount_ - row->endIndex - 1, totalItemCount_ - row->startIndex - 1 };
    }
    if (!row || posMap_->GetPositionInfo(physical).isGroup || row->endIndex == totalItemCount_ - 1 ||
        row->endIndex - row->startIndex + 1 == lanes_) {
        return row;
    }
    const int32_t next = isStackFromEnd_ ? totalItemCount_ - row->endIndex - 2 : row->endIndex + 1;
    if (posMap_->GetPositionInfo(next).isGroup) {
        return row;
    }
    return std::nullopt;
}

int32_t ListLanesLayoutAlgorithm::GetLazyForEachIndex(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_RETURN(host, -1);
    auto parent = host->GetParent();
    auto lazyForEach = AceType::DynamicCast<LazyForEachNode>(parent);
    if (lazyForEach) {
        return lazyForEach->GetIndexByUINode(host);
    }
    while (parent && !AceType::InstanceOf<FrameNode>(parent)) {
        if (AceType::InstanceOf<RepeatVirtualScrollNode>(parent)) {
            return parent->GetFrameNodeIndex(host);
        }
        if (AceType::InstanceOf<RepeatVirtualScroll2Node>(parent)) {
            return parent->GetFrameNodeIndex(host);
        }
        parent = parent->GetParent();
    }
    return -1;
}

int32_t ListLanesLayoutAlgorithm::FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t startIndex, int32_t index)
{
    auto wrapper = GetListItem(layoutWrapper, index);
    CHECK_NULL_RETURN(wrapper, index);
    if (wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
        return index;
    }
    auto lazyIndex = GetLazyForEachIndex(wrapper->GetHostNode());
    if (lazyIndex > 0) {
        index -= lazyIndex;
    }
    for (int32_t idx = index; idx > startIndex; idx--) {
        auto wrapper = GetListItem(layoutWrapper, idx - 1);
        CHECK_NULL_RETURN(wrapper, idx);
        if (wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
            return idx;
        }
    }
    if (startIndex == 0) {
        return 0;
    }
    return -1;
}

int32_t ListLanesLayoutAlgorithm::FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ == 1) {
        return 0;
    }
    auto it = lanesItemRange_.upper_bound(index);
    if (it == lanesItemRange_.begin()) {
        int32_t startIdx = FindLanesStartIndex(layoutWrapper, 0, index);
        lanesItemRange_[startIdx] = index;
        return startIdx;
    }
    it--;
    if (it->second >= index) {
        return it->first;
    }
    int32_t startIdx = FindLanesStartIndex(layoutWrapper, it->second, index);
    if (startIdx >= 0) {
        lanesItemRange_[startIdx] = index;
        return startIdx;
    }
    it->second = index;
    return it->first;
}

int32_t ListLanesLayoutAlgorithm::GetLanesFloor(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ > 1) {
        if (childrenSize_) {
            return posMap_->GetRowStartIndex(index);
        }
        int32_t startIndex = FindLanesStartIndex(layoutWrapper, index);
        return index - (index - startIndex) % lanes_;
    }
    return index;
}

int32_t ListLanesLayoutAlgorithm::GetLanesCeil(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ > 1) {
        if (childrenSize_) {
            return posMap_->GetRowEndIndex(index);
        }
        int32_t startIndex = GetLanesFloor(layoutWrapper, index);
        while (startIndex == GetLanesFloor(layoutWrapper, index + 1)) {
            index++;
        }
    }
    return index;
}

void ListLanesLayoutAlgorithm::LayoutCachedALine(LayoutWrapper* layoutWrapper,
    std::pair<const int, ListItemInfo>& pos, int32_t startIndex, float crossSize)
{
    auto wrapper = GetChildByIndex(layoutWrapper, pos.first, true);
    CHECK_NULL_VOID(wrapper);
    bool isDirty = wrapper->CheckNeedForceMeasureAndLayout() || !IsListLanesEqual(wrapper);
    LayoutItem(wrapper, pos.first, pos.second, startIndex, crossSize);
    SyncGeometry(wrapper, isDirty);
    wrapper->SetActive(false);
}

std::pair<bool, bool> ListLanesLayoutAlgorithm::CheckACachedItem(
    const RefPtr<LayoutWrapper>& wrapper, int32_t cnt, bool& isGroup, float ref, bool forward) const
{
    if (!wrapper) {
        return std::make_pair(true, true);
    }
    isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (isGroup && cnt > 0) {
        isGroup = false;
        return std::make_pair(true, false);
    }
    bool isDirty = wrapper->CheckNeedForceMeasureAndLayout() || !IsListLanesEqual(wrapper);
    if (!isGroup && (isDirty || CheckLayoutConstraintChanged(wrapper, ref, forward))) {
        if (isDirty && !wrapper->GetHostNode()->IsLayoutComplete()) {
            return std::make_pair(true, true);
        }
        return std::make_pair(false, true);
    }
    return std::make_pair(false, false);
}

int32_t ListLanesLayoutAlgorithm::LayoutCachedForward(LayoutWrapper* layoutWrapper,
    int32_t cacheCount, int32_t& cachedCount, int32_t curIndex, std::list<PredictLayoutItem>& predictList, bool show)
{
    auto prop = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(prop, curIndex);
    int32_t minCacheCount = prop->GetMinCacheCount();
    float crossSize = GetLayoutCrossAxisSize(layoutWrapper);
    RefPtr<LayoutWrapper> wrapper;
    curIndex = GetItemPosition().rbegin()->first + 1;
    auto startPos = GetItemPosition().rbegin()->second.endPos + GetSpaceWidth();
    while (cachedCount < cacheCount && curIndex <= GetMaxListItemIndex()) {
        ListLayoutAlgorithm::PositionMap posMap;
        float mainLen = 0.0f;
        bool isGroup = false;
        int32_t cnt = 0;
        bool forceCache = cachedCount <= minCacheCount;
        for (int32_t i = 0; i < lanes_ && curIndex + i <= GetMaxListItemIndex() && !isGroup; i++) {
            wrapper = GetChildByIndex(layoutWrapper, curIndex + i, !show);
            auto [needBreak, needPredict] = CheckACachedItem(wrapper, cnt, isGroup, startPos, true);
            if (needPredict) {
                predictList.emplace_back(
                    PredictLayoutItem { curIndex + i, cachedCount, -1, forceCache, needBreak, startPos });
            }
            if (needBreak) {
                break;
            }
            cnt++;
            mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
            auto childNode = wrapper->GetHostNode();
            posMap[curIndex + i] = { childNode ? childNode->GetId() : -1, startPos, startPos + mainLen, isGroup,
                !isGroup && CanSupportNestedLazy(childNode, layoutWrapper->GetHostNode(), GetLanes()) };
        }
        auto startIndex = curIndex;
        int32_t currCache = 1;
        if (isGroup) {
            auto res = GetLayoutGroupCachedCount(layoutWrapper, wrapper, cacheCount - cachedCount, -1, curIndex, true);
            if (res.forwardCachedCount < res.forwardCacheMax && res.forwardCachedCount < cacheCount - cachedCount) {
                LayoutItem(wrapper, posMap.begin()->first, posMap.begin()->second, startIndex, crossSize);
                PredictLayoutItem predictItem = {
                    posMap.begin()->first, cachedCount, -1, forceCache, false, startPos };
                predictList.emplace_back(predictItem);
                return res.forwardCachedCount > 0 ? curIndex : curIndex - 1;
            }
            currCache = std::max(res.forwardCacheMax, 1);
        } else if (cnt <= 0) {
            break;
        }
        for (auto& pos : posMap) {
            pos.second.endPos = startPos + mainLen;
            LayoutCachedALine(layoutWrapper, pos, startIndex, crossSize);
            if (NearEqual(pos.second.endPos, GetContentMainSize()) && NearZero(mainLen)) {
                SetItemInfo(pos.first, std::move(pos.second));
                currCache = 0;
            } else {
                SetCachedItemInfo(pos.first, std::move(pos.second));
            }
        }
        cachedCount += currCache;
        if (isStackFromEnd_) {
            SetLaneIdx4Divider(cnt - 1);
        }
        startPos = startPos + mainLen + GetSpaceWidth();
        curIndex += cnt;
    }
    return curIndex - 1;
}

int32_t ListLanesLayoutAlgorithm::LayoutCachedBackward(LayoutWrapper* layoutWrapper,
    int32_t cacheCount, int32_t& cachedCount, int32_t curIndex, std::list<PredictLayoutItem>& predictList, bool show)
{
    auto prop = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(prop, curIndex);
    int32_t minCacheCount = prop->GetMinCacheCount();
    float crossSize = GetLayoutCrossAxisSize(layoutWrapper);
    RefPtr<LayoutWrapper> wrapper;
    curIndex = GetItemPosition().begin()->first - 1;
    auto endPos = GetItemPosition().begin()->second.startPos - GetSpaceWidth();
    while (cachedCount < cacheCount && curIndex >= 0) {
        ListLayoutAlgorithm::PositionMap posMap;
        float mainLen = 0.0f;
        bool isGroup = false;
        int32_t cnt = 0;
        bool forceCache = cachedCount <= minCacheCount;
        for (int32_t i = 0; i < lanes_ && curIndex - i >= 0; i++) {
            auto idx = curIndex - i;
            wrapper = GetChildByIndex(layoutWrapper, idx, !show);
            auto [needBreak, needPredict] = CheckACachedItem(wrapper, cnt, isGroup, endPos, false);
            if (needPredict) {
                predictList.emplace_back(PredictLayoutItem { idx, -1, cachedCount, forceCache, needBreak, endPos });
            }
            if (needBreak) {
                break;
            }
            cnt++;
            mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
            auto childNode = wrapper->GetHostNode();
            posMap[idx] = { childNode ? childNode->GetId() : -1, endPos - mainLen, endPos, isGroup,
                !isGroup && CanSupportNestedLazy(childNode, layoutWrapper->GetHostNode(), GetLanes()) };
            if (CheckCurRowMeasureFinished(layoutWrapper, idx, isGroup)) {
                break;
            }
        }
        auto startIndex = GetLanesFloor(layoutWrapper, curIndex);
        int32_t currCache = 1;
        if (isGroup) {
            auto res = GetLayoutGroupCachedCount(layoutWrapper, wrapper, -1, cacheCount - cachedCount, curIndex, true);
            if (res.backwardCachedCount < res.backwardCacheMax && res.backwardCachedCount < cacheCount - cachedCount) {
                LayoutItem(wrapper, posMap.begin()->first, posMap.begin()->second, startIndex, crossSize);
                PredictLayoutItem predictItem = {
                    posMap.begin()->first, -1, cachedCount, forceCache, false, endPos };
                predictList.emplace_back(predictItem);
                return res.backwardCachedCount > 0 ? curIndex : curIndex + 1;
            }
            currCache = std::max(res.backwardCacheMax, 1);
        } else if (cnt <= 0) {
            break;
        }
        for (auto& pos: posMap) {
            pos.second.startPos = endPos - mainLen;
            LayoutCachedALine(layoutWrapper, pos, startIndex, crossSize);
            if (NearEqual(pos.second.startPos, 0.0f) && NearZero(mainLen)) {
                SetItemInfo(pos.first, std::move(pos.second));
                currCache = 0;
            } else {
                SetCachedItemInfo(pos.first, std::move(pos.second));
            }
        }
        cachedCount += currCache;
        if (!isStackFromEnd_) {
            SetLaneIdx4Divider(curIndex - startIndex + 1 - cnt);
        }
        endPos = endPos - mainLen - GetSpaceWidth();
        curIndex -= cnt;
    }
    return curIndex + 1;
}
} // namespace OHOS::Ace::NG

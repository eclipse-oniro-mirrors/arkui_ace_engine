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

#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/list/list_position_map.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
bool HasValidGeometry(const ListPositionInfo& info)
{
    return std::isfinite(info.mainPos) && NonNegative(info.mainPos) &&
        std::isfinite(info.mainSize) && NonNegative(info.mainSize);
}
} // namespace

void ListPositionMap::UpdatePosRange(int32_t startIndex, int32_t endIndex,
    ListPositionInfo posInfo, float space, int32_t lanes)
{
    for (int32_t i = startIndex; i < endIndex; i++) {
        auto& currPosInfo = posMap_[i];
        bool isLazyChild = currPosInfo.isLazyChild;
        currPosInfo = posInfo;
        currPosInfo.isLazyChild = isLazyChild;
        if (lanes >= 1 && (i % lanes == lanes - 1)) {
            posInfo.mainPos = posInfo.mainPos + posInfo.mainSize + space;
        }
    }
}

void ListPositionMap::UpdatePosWithCheck(int32_t index, ListPositionInfo posInfo)
{
    auto iter = posMap_.find(index);
    if (iter == posMap_.end()) {
        posMap_[index] = posInfo;
        return;
    }
    iter->second.isGroup = posInfo.isGroup;
    iter->second.isLazyChild = posInfo.isLazyChild;
    if (LessNotEqual(iter->second.mainSize, posInfo.mainSize)) {
        iter->second.mainSize = posInfo.mainSize;
    }
}

ListPosMapUpdate ListPositionMap::CheckPosMapUpdateRule()
{
    ListPosMapUpdate flag;
    if (dirty_ == LIST_NO_CHANGE) {
        flag = ListPosMapUpdate::NO_CHANGE;
    } else if (0 == (dirty_ & (LIST_UPDATE_CHILD_SIZE | LIST_UPDATE_LANES | LIST_GROUP_UPDATE_HEADER_FOOTER |
                                    LIST_UPDATE_HEADER_FOOTER))) {
        flag = ListPosMapUpdate::UPDATE_ALL_SIZE;
    } else {
        flag = ListPosMapUpdate::RE_CALCULATE;
    }
    return flag;
}

void ListPositionMap::UpdatePosMapStart(float delta, double& listCurrentPos, float space,
    int32_t startIndex, float startPos, bool groupAtStart)
{
    auto it = posMap_.find(startIndex);
    if (it == posMap_.begin() || it == posMap_.end()) {
        return;
    }
    startPos = startPos + listCurrentPos;
    it--;
    float prevPos = it->second.mainPos + it->second.mainSize + space;
    int32_t prevIndex = it->first;
    if (prevIndex + 1 >= startIndex && groupAtStart) {
        if (NearEqual(prevPos, startPos)) {
            return;
        }
    } else {
        if (LessNotEqual(prevPos, startPos)) {
            return;
        }
    }
    listCurrentPos += prevPos - startPos;
}

void ListPositionMap::UpdatePosMapEnd(int32_t prevEndIndex, float space, bool groupAtEnd)
{
    auto it = posMap_.find(prevEndIndex);
    if (it == posMap_.end()) {
        return;
    }
    float prevPos = it->second.mainPos + it->second.mainSize + space;
    it++;
    if (it == posMap_.end()) {
        return;
    }
    if (prevEndIndex + 1 >= it->first && groupAtEnd) {
        if (NearEqual(prevPos, it->second.mainPos)) {
            return;
        }
    } else {
        if (LessNotEqual(prevPos, it->second.mainPos)) {
            return;
        }
    }
    float delta = prevPos - it->second.mainPos;
    while (it != posMap_.end()) {
        it->second.mainPos += delta;
        it++;
    }
}

void ListPositionMap::CalculateUINode(RefPtr<UINode> node)
{
    CHECK_NULL_VOID(node);
    auto children = node->GetChildren();
    for (const auto& child : children) {
        if (AceType::InstanceOf<FrameNode>(child)) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            CalculateFrameNode(frameNode);
        } else if (AceType::InstanceOf<LazyForEachNode>(child)) {
            // Rules: only one type node(ListItem or ListItemGroup) can exist in LazyForEach.
            CalculateLazyForEachNode(child);
        } else if (AceType::InstanceOf<RepeatVirtualScrollNode>(child)) {
            CalculateLazyForEachNode(child);
        } else if (AceType::InstanceOf<RepeatVirtualScroll2Node>(child)) {
            CalculateLazyForEachNode(child);
        } else {
            CalculateUINode(child);
        }
    }
}

std::optional<bool> ListPositionMap::GetLazyForEachChildIsGroup(RefPtr<UINode> node)
{
    std::optional<bool> isGroup;
    auto children = node->GetChildren();
    if (children.size() > 0) {
        auto child = children.begin();
        while (child != children.end() && !((*child)->GetFrameChildByIndex(0, false))) {
            child++;
        }
        RefPtr<FrameNode> frameNode;
        if (child != children.end()) {
            frameNode = AceType::DynamicCast<FrameNode>((*child)->GetFrameChildByIndex(0, false));
        }
        if (frameNode) {
            isGroup = frameNode->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        }
    }
    if (!(isGroup.has_value())) {
        auto listNode = node->GetParentFrameNode();
        CHECK_NULL_RETURN(listNode, isGroup);
        auto wrapper = listNode->GetOrCreateChildByIndex(curIndex_);
        CHECK_NULL_RETURN(wrapper, isGroup);
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    }
    return isGroup;
}

void ListPositionMap::CalculateLazyForEachNode(RefPtr<UINode> node)
{
    int32_t count = node->FrameCount();
    if (count <= 0) {
        return;
    }
    std::optional<bool> isGroup = GetLazyForEachChildIsGroup(node);
    if (!(isGroup.has_value())) {
        TAG_LOGW(AceLogTag::ACE_LIST,
            "ListPositionMap Conflict: LazyForEach FrameCount > 0, but get child type failed.");
        isGroup = false;
    }
    while (count--) {
        isGroup.value() ? CalculateGroupNode() : CalculateListItemNode();
    }
    return;
}

void ListPositionMap::CalculateFrameNode(RefPtr<FrameNode> frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto listItemGroupPatten = frameNode->GetPattern<ListItemGroupPattern>();
    if (listItemGroupPatten) {
        CalculateGroupNode();
    } else {
        CalculateListItemNode();
    }
}

void ListPositionMap::CalculateListItemNode()
{
    curRowHeight_ = std::max(curRowHeight_, childrenSize_->GetChildSize(curIndex_));
    curLine_++;
    if (curLine_ == lanes_ || curIndex_ == totalItemCount_ - 1) {
        while (curLine_) {
            curLine_--;
            posMap_[curIndex_ - curLine_] = { totalHeight_, curRowHeight_ };
        }
        totalHeight_ += (curRowHeight_ + space_);
        curRowHeight_ = 0.0f;
    }
    curIndex_++;
}

void ListPositionMap::CalculateGroupNode()
{
    if (curLine_ > 0) {
        while (curLine_) {
            curLine_--;
            posMap_[curIndex_ - 1 - curLine_] = { totalHeight_, curRowHeight_ };
        }
        totalHeight_ += (curRowHeight_ + space_);
        curRowHeight_ = 0.0f;
    }
    curRowHeight_ = childrenSize_->GetChildSize(curIndex_);
    posMap_[curIndex_] = { totalHeight_, curRowHeight_ };
    totalHeight_ += (curRowHeight_ + space_);
    curLine_ = 0;
    curRowHeight_ = 0.0f;
    curIndex_++;
}

void ListPositionMap::PosMapRecalculate(LayoutWrapper* layoutWrapper)
{
    curIndex_ = 0;
    curLine_ = 0;
    totalHeight_ = 0.0f;
    curRowHeight_ = 0.0f;
    if (lanes_ == 1) {
        for (int32_t index = 0; index < totalItemCount_; index++) {
            curRowHeight_ = childrenSize_->GetChildSize(index);
            posMap_[index] = { totalHeight_, curRowHeight_ };
            totalHeight_ += (curRowHeight_ + space_);
        }
        totalHeight_ -= space_;
    } else {
        auto listNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(listNode);
        CalculateUINode(listNode);
        totalHeight_ -= space_;
    }
    TAG_LOGI(AceLogTag::ACE_LIST, "List PosMapRecalculate totalHeight:%{public}f prevTotalHeight:%{public}f, "
        "lanes:%{public}d, space:%{public}f, totalItemCount:%{public}d",
        totalHeight_, prevTotalHeight_, lanes_, space_, totalItemCount_);
}

void ListPositionMap::GroupPosMapRecalculate()
{
    curIndex_ = 0;
    totalHeight_ = headerSize_;
    curRowHeight_ = 0.0f;
    curLine_ = 0;

    for (int32_t index = 0; index < totalItemCount_;) {
        while (curLine_ < lanes_) {
            curRowHeight_ = std::max(curRowHeight_, childrenSize_->GetChildSize(index + curLine_));
            curLine_++;
        }
        curLine_ = 0;
        int32_t curRowEndIndex = std::min(index + lanes_ - 1, totalItemCount_ - 1);
        while (index <= curRowEndIndex) {
            posMap_[index++] = { totalHeight_, curRowHeight_ };
        }
        totalHeight_ += (curRowHeight_ + space_);
        curRowHeight_ = 0.0f;
    }
    totalHeight_ = totalHeight_ - space_ + footerSize_;
}

void ListPositionMap::UpdatePosMap(LayoutWrapper* layoutWrapper, int32_t lanes, float space,
    RefPtr<ListChildrenMainSize>& childrenSize)
{
    childrenSize_ = childrenSize;
    if (totalItemCount_ != layoutWrapper->GetTotalChildCount()) {
        dirty_ |= LIST_UPDATE_ITEM_COUNT;
        totalItemCount_ = layoutWrapper->GetTotalChildCount();
    }
    if (lanes != lanes_) {
        dirty_ |= LIST_UPDATE_LANES;
        lanes_ = lanes;
    }
    if (!NearEqual(space, space_)) {
        dirty_ |= LIST_UPDATE_SPACE;
        space_ = space;
    }
    switch (CheckPosMapUpdateRule()) {
        case ListPosMapUpdate::NO_CHANGE:
            break;
        case ListPosMapUpdate::UPDATE_ALL_SIZE:
            PosMapRecalculate(layoutWrapper);
            break;
        case ListPosMapUpdate::RE_CALCULATE:
        default:
            PosMapRecalculate(layoutWrapper);
    }
    ClearDirty();
}

void ListPositionMap::UpdateGroupPosMap(int32_t totalCount, int32_t lanes, float space,
    RefPtr<ListChildrenMainSize>& childrenSize, float headerSize, float footerSize)
{
    childrenSize_ = childrenSize;
    prevTotalHeight_ = totalHeight_;
    if (totalCount != totalItemCount_) {
        dirty_ |= LIST_UPDATE_ITEM_COUNT;
        totalItemCount_ = totalCount;
    }
    if (lanes != lanes_) {
        dirty_ |= LIST_UPDATE_LANES;
        lanes_ = lanes;
    }
    if (!NearEqual(space, space_)) {
        dirty_ |= LIST_UPDATE_SPACE;
        space_ = space;
    }
    if (!NearEqual(headerSize, headerSize_) || !NearEqual(footerSize, footerSize_)) {
        dirty_ |= LIST_GROUP_UPDATE_HEADER_FOOTER;
        headerSize_ = headerSize;
        footerSize_ = footerSize;
    }
    if (totalItemCount_ <= 0) {
        totalHeight_ = 0;
        ClearPosMap();
        ClearDirty();
        return;
    }
    switch (CheckPosMapUpdateRule()) {
        case ListPosMapUpdate::NO_CHANGE:
            break;
        case ListPosMapUpdate::UPDATE_ALL_SIZE:
            GroupPosMapRecalculate();
            break;
        case ListPosMapUpdate::RE_CALCULATE:
        default:
            GroupPosMapRecalculate();
    }
    ClearDirty();
}

void ListPositionMap::UpdateTotalCount(int32_t totalCount)
{
    totalItemCount_ = totalCount;
    auto iter = posMap_.lower_bound(totalCount);
    posMap_.erase(iter, posMap_.end());
}

ListPositionInfo ListPositionMap::GetPositionInfo(int32_t index) const
{
    auto it = posMap_.find(index);
    if (it == posMap_.end()) {
        return { -1.0f, -1.0f };
    }
    return it->second;
}

ListPositionMap::ItemGroupFlags ListPositionMap::GetItemGroupFlags() const
{
    ItemGroupFlags flags;
    flags.reserve(posMap_.size());
    for (const auto& [index, info] : posMap_) {
        flags.emplace_back(index, info.isGroup);
    }
    return flags;
}

void ListPositionMap::UpdateItemGroupFlags(const ItemGroupFlags& flags)
{
    for (const auto& [index, isGroup] : flags) {
        if (index < 0 || index >= totalItemCount_) {
            continue;
        }
        auto& info = posMap_.try_emplace(index, ListPositionInfo { -1.0f, -1.0f }).first->second;
        info.isGroup = isGroup;
    }
}

void ListPositionMap::UpdateFastScrollPosMapStart(float delta, double& listCurrentPos, float space,
    int32_t startIndex, float startPos, bool groupAtStart)
{
    auto it = posMap_.find(startIndex);
    if (it == posMap_.begin() || it == posMap_.end()) {
        return;
    }
    startPos = startPos + listCurrentPos;
    do {
        --it;
    } while (it != posMap_.begin() && !HasValidGeometry(it->second));
    if (!HasValidGeometry(it->second)) {
        return;
    }
    float prevPos = it->second.mainPos + it->second.mainSize + space;
    int32_t prevIndex = it->first;
    if (prevIndex + 1 >= startIndex && groupAtStart) {
        if (NearEqual(prevPos, startPos)) {
            return;
        }
    } else {
        if (LessNotEqual(prevPos, startPos)) {
            return;
        }
    }
    listCurrentPos += prevPos - startPos;
}

void ListPositionMap::UpdateFastScrollPosMapEnd(int32_t prevEndIndex, float space, bool groupAtEnd)
{
    auto it = posMap_.find(prevEndIndex);
    if (it == posMap_.end() || !HasValidGeometry(it->second)) {
        return;
    }
    // A Group occupies a complete outer row even when its internal tail is outside the viewport.
    const bool rowBoundary = groupAtEnd || it->second.isGroup;
    float prevPos = it->second.mainPos + it->second.mainSize + space;
    it++;
    while (it != posMap_.end() && !HasValidGeometry(it->second)) {
        ++it;
    }
    if (it == posMap_.end()) {
        return;
    }
    if (prevEndIndex + 1 >= it->first && rowBoundary) {
        if (NearEqual(prevPos, it->second.mainPos)) {
            return;
        }
    } else {
        if (LessNotEqual(prevPos, it->second.mainPos)) {
            return;
        }
    }
    float delta = prevPos - it->second.mainPos;
    while (it != posMap_.end()) {
        if (HasValidGeometry(it->second)) {
            it->second.mainPos += delta;
        }
        it++;
    }
}

void ListPositionMap::RemoveUnpositionedEntries()
{
    for (auto it = posMap_.begin(); it != posMap_.end();) {
        if (!HasValidGeometry(it->second)) {
            it = posMap_.erase(it);
        } else {
            ++it;
        }
    }
}

std::optional<float> ListPositionMap::GetAverageRowHeight(int32_t lanes) const
{
    if (lanes <= 0) {
        return std::nullopt;
    }
    double totalHeight = 0.0;
    int32_t rows = 0;
    auto iter = posMap_.begin();
    while (iter != posMap_.end()) {
        const auto row = iter;
        float rowHeight = -1.0f;
        int32_t previousIndex = iter->first;
        do {
            const float size = iter->second.mainSize;
            if (std::isfinite(size) && NonNegative(size)) {
                rowHeight = std::max(rowHeight, std::max(0.0f, size));
            }
            previousIndex = iter->first;
            ++iter;
        } while (iter != posMap_.end() && !row->second.isGroup && !iter->second.isGroup &&
            iter->first == previousIndex + 1 && iter->first - row->first < lanes &&
            std::isfinite(row->second.mainPos) && NonNegative(row->second.mainPos) &&
            std::isfinite(iter->second.mainPos) && NearEqual(row->second.mainPos, iter->second.mainPos));
        if (NonNegative(rowHeight)) {
            totalHeight += rowHeight;
            ++rows;
        }
    }
    return rows > 0 ? std::make_optional(static_cast<float>(totalHeight / rows)) : std::nullopt;
}

bool ListPositionMap::EstimatePositions(const EstimateRange& range, float estimatedSize, int32_t lanes, float space,
    const std::map<int32_t, int32_t>& itemRanges, bool stackFromEnd)
{
    if (childrenSize_ || range.startIndex < 0 || range.startIndex > range.endIndex ||
        range.endIndex >= totalItemCount_ || lanes <= 0 ||
        !std::isfinite(estimatedSize) || Negative(estimatedSize) || !std::isfinite(space) || Negative(space)) {
        return false;
    }
    if (!posMap_.empty() && (posMap_.begin()->first < 0 || posMap_.rbegin()->first >= totalItemCount_)) {
        return false;
    }
    if (lanes == 1) {
        return EstimateSingleLanePositions(range, estimatedSize, space);
    }
    return EstimateConfirmedRows(range, estimatedSize, space, { lanes, stackFromEnd, itemRanges });
}

std::optional<ListPositionMap::EstimateRange> ListPositionMap::GetLanesRow(int32_t index, int32_t lanes,
    const std::map<int32_t, int32_t>& itemRanges, bool stackFromEnd) const
{
    if (lanes <= 0 || index < 0 || index >= totalItemCount_) {
        return std::nullopt;
    }
    const int32_t layoutIndex = stackFromEnd ? totalItemCount_ - index - 1 : index;
    auto segment = itemRanges.upper_bound(layoutIndex);
    if (segment == itemRanges.begin() || (--segment)->second < layoutIndex ||
        segment->first < 0 || segment->second >= totalItemCount_) {
        return std::nullopt;
    }
    const int32_t start = segment->first + (layoutIndex - segment->first) / lanes * lanes;
    const int32_t end = start + std::min(lanes - 1, segment->second - start);
    return stackFromEnd ? EstimateRange { totalItemCount_ - end - 1, totalItemCount_ - start - 1 } :
        EstimateRange { start, end };
}

std::optional<double> ListPositionMap::CollectEstimatedRows(const EstimateRange& range, int32_t anchorIndex,
    float estimatedSize, float space, const LanesEstimate& layout,
    std::vector<std::pair<int32_t, float>>& heights) const
{
    double offset = 0.0;
    double length = 0.0;
    int32_t next = range.startIndex;
    auto iter = posMap_.lower_bound(next);
    while (next <= range.endIndex) {
        const auto row = GetLanesRow(next, layout.lanes, layout.itemRanges, layout.stackFromEnd);
        if (!row || row->startIndex != next || row->endIndex > range.endIndex) {
            return std::nullopt;
        }
        float height = 0.0f;
        for (int32_t index = next; index <= row->endIndex; ++index) {
            float size = estimatedSize;
            if (iter != posMap_.end() && iter->first == index) {
                size = iter->second.mainSize;
                ++iter;
            }
            if (!std::isfinite(size)) {
                return std::nullopt;
            }
            height = std::max(height, Negative(size) ? estimatedSize : size);
        }
        heights.emplace_back(row->endIndex, height);
        length += static_cast<double>(height) + space;
        if (row->endIndex < anchorIndex) {
            offset = length;
        }
        next = row->endIndex + 1;
    }
    return next == range.endIndex + 1 && std::isfinite(static_cast<float>(length)) ?
        std::make_optional(offset) : std::nullopt;
}

bool ListPositionMap::EstimateConfirmedRows(
    EstimateRange range, float estimatedSize, float space, const LanesEstimate& layout)
{
    const auto first = GetLanesRow(range.startIndex, layout.lanes, layout.itemRanges, layout.stackFromEnd);
    const auto last = GetLanesRow(range.endIndex, layout.lanes, layout.itemRanges, layout.stackFromEnd);
    if (!first || !last) {
        return false;
    }
    range = { first->startIndex, last->endIndex };
    const auto anchor = FindEstimatedAnchor(range);
    if (anchor != posMap_.end() && (anchor->first < range.startIndex || anchor->first > range.endIndex)) {
        return false;
    }
    std::vector<std::pair<int32_t, float>> heights;
    const auto offset = CollectEstimatedRows(range, anchor == posMap_.end() ? range.startIndex : anchor->first,
        estimatedSize, space, layout, heights);
    CHECK_NULL_RETURN(offset, false);
    double position = (anchor == posMap_.end() ? 0.0 : anchor->second.mainPos) - offset.value();
    double minimumPosition = 0.0;
    auto prefix = posMap_.lower_bound(range.startIndex);
    while (prefix != posMap_.begin()) {
        --prefix;
        if (HasValidGeometry(prefix->second)) {
            minimumPosition = static_cast<double>(prefix->second.mainPos) + prefix->second.mainSize + space;
            break;
        }
    }
    position = std::max(position, minimumPosition);
    double endPosition = position;
    for (const auto& [endIndex, height] : heights) {
        endPosition += static_cast<double>(height) + space;
    }
    if (!std::isfinite(static_cast<float>(endPosition))) {
        return false;
    }
    FillConfirmedRows(range.startIndex, heights, position, space);
    UpdateFastScrollPosMapEnd(range.endIndex, space, true);
    return true;
}

void ListPositionMap::FillConfirmedRows(int32_t startIndex,
    const std::vector<std::pair<int32_t, float>>& heights, double position, float space)
{
    auto iter = posMap_.lower_bound(startIndex);
    for (const auto& [endIndex, height] : heights) {
        for (; startIndex <= endIndex; ++startIndex) {
            iter = posMap_.try_emplace(iter, startIndex, ListPositionInfo { static_cast<float>(position), height });
            iter->second.mainPos = position;
            iter->second.mainSize = height;
            ++iter;
        }
        position += static_cast<double>(height) + space;
    }
}

ListPositionMap::PositionMap::iterator ListPositionMap::FindEstimatedAnchor(const EstimateRange& range)
{
    const auto first = posMap_.lower_bound(range.startIndex);
    auto right = first;
    while (right != posMap_.end() && !HasValidGeometry(right->second)) {
        ++right;
    }
    if (right != posMap_.end() && right->first <= range.endIndex) {
        return right;
    }
    auto left = posMap_.end();
    auto iter = first;
    while (iter != posMap_.begin()) {
        --iter;
        if (HasValidGeometry(iter->second)) {
            left = iter;
            break;
        }
    }
    if (right == posMap_.end()) {
        return left;
    }
    const int32_t rightDistance = std::max(0, right->first - range.endIndex);
    if (left != posMap_.end() && range.startIndex - left->first <= rightDistance) {
        return left;
    }
    return right;
}

std::optional<double> ListPositionMap::CheckEstimatedPositions(
    const EstimateRange& range, PositionMap::const_iterator anchor, float estimatedSize, float space) const
{
    double position = 0.0;
    double anchorOffset = 0.0;
    double minPosition = 0.0;
    double maxPosition = 0.0;
    const int32_t anchorIndex = range.startIndex == 0 ? 0 : anchor->first;
    auto iter = posMap_.lower_bound(range.startIndex);
    for (int32_t index = range.startIndex; index <= range.endIndex; ++index) {
        if (index == anchorIndex) {
            anchorOffset = position;
        }
        minPosition = std::min(minPosition, position);
        maxPosition = std::max(maxPosition, position);
        float size = estimatedSize;
        if (iter != posMap_.end() && iter->first == index) {
            size = Negative(iter->second.mainSize) ? estimatedSize : iter->second.mainSize;
            ++iter;
        }
        if (!std::isfinite(size)) {
            return std::nullopt;
        }
        position += static_cast<double>(size) + space;
    }
    const double startPos = range.startIndex == 0 ? 0.0 : anchor->second.mainPos - anchorOffset;
    const double offset = std::max(0.0, startPos);
    if (!std::isfinite(static_cast<float>(minPosition + offset)) ||
        !std::isfinite(static_cast<float>(maxPosition + offset))) {
        return std::nullopt;
    }
    return startPos;
}

bool ListPositionMap::EstimateSingleLanePositions(EstimateRange range, float estimatedSize, float space)
{
    const auto anchor = FindEstimatedAnchor(range);
    if (anchor == posMap_.end()) {
        range.startIndex = 0;
    } else {
        range.startIndex = std::min(range.startIndex, anchor->first);
        range.endIndex = std::max(range.endIndex, anchor->first);
    }
    auto startPos = CheckEstimatedPositions(range, anchor, estimatedSize, space);
    CHECK_NULL_RETURN(startPos, false);
    const auto origin = posMap_.begin();
    if (Negative(startPos.value()) && origin != posMap_.end() && origin->first == 0 &&
        HasValidGeometry(origin->second)) {
        range.startIndex = 0;
        startPos = CheckEstimatedPositions(range, anchor, estimatedSize, space);
        CHECK_NULL_RETURN(startPos, false);
    }
    const double anchorPos = range.startIndex == 0 ? 0.0 : anchor->second.mainPos;
    const double shift = std::max(0.0, -startPos.value());
    if (Positive(shift)) {
        for (const auto& [index, info] : posMap_) {
            if (HasValidGeometry(info) && !std::isfinite(static_cast<float>(info.mainPos + shift))) {
                return false;
            }
        }
        for (auto& [index, info] : posMap_) {
            if (HasValidGeometry(info)) {
                info.mainPos += shift;
            }
        }
    }
    FillEstimatedPositions(range, anchor, anchorPos + shift, estimatedSize, space);
    UpdateFastScrollPosMapEnd(range.endIndex, space, true);
    return true;
}

void ListPositionMap::FillEstimatedPositions(const EstimateRange& range, PositionMap::iterator anchor,
    double anchorPos, float estimatedSize, float space)
{
    const int32_t anchorIndex = range.startIndex == 0 ? 0 : anchor->first;
    double position = anchorPos;
    auto hint = anchor;
    for (int32_t index = anchorIndex - 1; index >= range.startIndex; --index) {
        const auto previous = hint == posMap_.begin() ? posMap_.end() : std::prev(hint);
        hint = previous != posMap_.end() && previous->first == index ? previous :
            posMap_.emplace_hint(hint, index, ListPositionInfo { 0.0f, estimatedSize });
        auto& info = hint->second;
        if (Negative(info.mainSize)) {
            info.mainSize = estimatedSize;
        }
        position -= static_cast<double>(info.mainSize) + space;
        info.mainPos = static_cast<float>(position);
    }
    position = anchorPos;
    hint = range.startIndex == 0 ? posMap_.begin() : anchor;
    for (int32_t index = anchorIndex; index <= range.endIndex; ++index) {
        auto iter = hint != posMap_.end() && hint->first == index ? hint :
            posMap_.emplace_hint(hint, index, ListPositionInfo { 0.0f, estimatedSize });
        iter->second.mainPos = static_cast<float>(position);
        if (Negative(iter->second.mainSize)) {
            iter->second.mainSize = estimatedSize;
        }
        position += static_cast<double>(iter->second.mainSize) + space;
        hint = std::next(iter);
    }
}

std::pair<int32_t, float> ListPositionMap::FindRowAtPosition(float position, int32_t lanes) const
{
    if (posMap_.empty() || !std::isfinite(position) || lanes <= 0) {
        return { -1, 0.0f };
    }
    auto iter = posMap_.begin();
    std::pair<int32_t, float> result = { -1, 0.0f };
    while (iter != posMap_.end()) {
        if (!HasValidGeometry(iter->second)) {
            ++iter;
            continue;
        }
        auto row = iter;
        float endPos = row->second.mainPos + row->second.mainSize;
        ++iter;
        while (!row->second.isGroup && iter != posMap_.end() && HasValidGeometry(iter->second) &&
            !iter->second.isGroup &&
            iter->first - row->first < lanes &&
            NearEqual(iter->second.mainPos, row->second.mainPos)) {
            endPos = std::max(endPos, iter->second.mainPos + iter->second.mainSize);
            ++iter;
        }
        result = { row->first, position - row->second.mainPos };
        if (LessNotEqual(position, endPos)) {
            return result;
        }
    }
    return result;
}

RefPtr<ListPositionMap> ListPositionMap::CreateFastScrollMirroredPosMap() const
{
    auto reversed = AceType::MakeRefPtr<ListPositionMap>();
    reversed->childrenSize_ = childrenSize_;
    reversed->totalItemCount_ = totalItemCount_;
    reversed->totalHeight_ = totalHeight_;
    reversed->prevTotalHeight_ = prevTotalHeight_;
    reversed->lanes_ = lanes_;
    reversed->space_ = space_;
    reversed->headerSize_ = headerSize_;
    reversed->footerSize_ = footerSize_;
    reversed->chainOffsetFunc_ = chainOffsetFunc_;
    for (auto iter = posMap_.rbegin(); iter != posMap_.rend(); ++iter) {
        auto mirrored = iter->second;
        mirrored.mainPos = totalHeight_ - mirrored.mainPos - mirrored.mainSize;
        reversed->posMap_.emplace_hint(reversed->posMap_.end(), totalItemCount_ - iter->first - 1, mirrored);
    }
    return reversed;
}

std::pair<int32_t, float> ListPositionMap::GetStartIndexAndPos() const
{
    if (posMap_.empty()) {
        return { -1, 0.0f };
    }
    const auto& start = posMap_.begin();
    return { start->first, start->second.mainPos };
}

std::pair<int32_t, float> ListPositionMap::GetEndIndexAndPos() const
{
    if (posMap_.empty()) {
        return { -1, 0.0f };
    }
    const auto& end = posMap_.rbegin();
    return { end->first, end->second.mainPos + end->second.mainSize };
}

int32_t ListPositionMap::GetEntryAtOrBeforeIndex(int32_t index) const
{
    if (posMap_.empty()) {
        return -1;
    }
    auto it = posMap_.lower_bound(index);
    if (it != posMap_.end() && it->first == index) {
        return it->first;
    }
    if (it == posMap_.begin()) {
        return -1;
    }
    --it;
    return it->first;
}

int32_t ListPositionMap::GetEntryAtOrAfterIndex(int32_t index) const
{
    if (posMap_.empty()) {
        return -1;
    }
    auto it = posMap_.lower_bound(index);
    if (it == posMap_.end()) {
        return -1;
    }
    return it->first;
}

void ListPositionMap::OptimizeBeforeMeasure(int32_t& beginIndex, float& beginPos,
    const float offset, const float contentSize)
{
    if (NearZero(offset) || GreatOrEqual(contentSize, totalHeight_) ||
        posMap_.find(beginIndex) == posMap_.end()) {
        return;
    }
    float chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
    if (Positive(offset)) {
        float criticalPos = offset;
        std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(beginIndex);
        while (beginIndex < totalItemCount_ && !NearEqual(posMap_[beginIndex].mainPos + rowInfo.second, totalHeight_) &&
            LessNotEqual(beginPos + rowInfo.second + chainOffset, criticalPos)) {
            beginIndex = rowInfo.first + 1;
            beginPos += (rowInfo.second + space_);
            rowInfo = GetRowEndIndexAndHeight(beginIndex);
            chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
        }
    } else {
        float criticalPos = offset + contentSize;
        std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(beginIndex);
        while (beginIndex < totalItemCount_ && Positive(posMap_[beginIndex].mainPos) &&
            GreatNotEqual(beginPos - rowInfo.second + chainOffset, criticalPos)) {
            beginIndex = GetRowStartIndex(beginIndex) - 1;
            beginPos -= (rowInfo.second + space_);
            rowInfo = GetRowEndIndexAndHeight(beginIndex);
            chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
        }
    }
}

int32_t ListPositionMap::GetRowStartIndex(const int32_t input)
{
    int32_t startIndex = input;
    while (startIndex > 0 && NearEqual(posMap_[startIndex].mainPos, posMap_[startIndex - 1].mainPos)) {
        startIndex--;
    }
    return startIndex;
}

int32_t ListPositionMap::GetRowEndIndex(const int32_t input)
{
    std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(input);
    return rowInfo.first;
}

float ListPositionMap::GetRowHeight(int32_t input)
{
    std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(input);
    return rowInfo.second;
}

std::pair<int32_t, float> ListPositionMap::GetRowEndIndexAndHeight(const int32_t input)
{
    int32_t endIndex = input;
    float rowHeight = 0.0f;
    while (endIndex < (totalItemCount_ - 1) &&
        NearEqual(posMap_[endIndex].mainPos, posMap_[endIndex + 1].mainPos)) {
        endIndex++;
    }
    if (endIndex == totalItemCount_ - 1) {
        rowHeight = totalHeight_ - posMap_[endIndex].mainPos - footerSize_;
    } else {
        rowHeight = posMap_[endIndex + 1].mainPos  - posMap_[endIndex].mainPos - space_;
    }
    return { endIndex, rowHeight };
}

void ListPositionMap::ReversePosMap()
{
    totalHeight_ = 0.0f;
    float curRowHeight = posMap_.begin()->second.mainSize;
    std::map<int32_t, ListPositionInfo> posMap;
    for (int32_t index = totalItemCount_ - 1; index >= 0; --index) {
        auto posIndex = totalItemCount_ - index - 1;
        posMap[posIndex] = {totalHeight_, posMap_[posIndex].mainSize};
        if (index > 0 && !NearEqual(posMap_[index].mainPos, posMap_[index - 1].mainPos)) {
            curRowHeight = std::max(posMap_[posIndex].mainSize, curRowHeight);
            totalHeight_ += (curRowHeight + space_);
            curRowHeight = 0.0f;
        } else {
            curRowHeight = std::max(posMap_[posIndex].mainSize, curRowHeight);
        }
    }
    posMap_ = std::move(posMap);
    totalHeight_ += curRowHeight;
}

} // namespace OHOS::Ace::NG

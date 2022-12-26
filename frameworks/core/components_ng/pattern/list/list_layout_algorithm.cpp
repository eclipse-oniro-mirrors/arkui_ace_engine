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

#include "core/components_ng/pattern/list/list_layout_algorithm.h"

#include <algorithm>
#include <unordered_set>

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/layout_param.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/list/list_item_group_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {

void ListLayoutAlgorithm::UpdateListItemConstraint(
    Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint)
{
    contentConstraint.parentIdealSize = selfIdealSize;
    if (axis == Axis::VERTICAL) {
        contentConstraint.maxSize.SetHeight(Infinity<float>());
        auto width = selfIdealSize.Width();
        if (width.has_value()) {
            contentConstraint.maxSize.SetWidth(width.value());
            contentConstraint.percentReference.SetWidth(width.value());
        }
        return;
    }
    contentConstraint.maxSize.SetWidth(Infinity<float>());
    auto height = selfIdealSize.Height();
    if (height.has_value()) {
        contentConstraint.maxSize.SetHeight(height.value());
        contentConstraint.percentReference.SetHeight(height.value());
    }
}

void ListLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    if (overScrollFeature_ && !layoutWrapper->CheckChildNeedForceMeasureAndLayout() && !HasItemGroup()) {
        LOGD("in over scroll case");
        return;
    }
    auto listLayoutProperty = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(listLayoutProperty);

    const auto& layoutConstraint = listLayoutProperty->GetLayoutConstraint().value();

    // calculate idealSize and set FrameSize
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);

    // calculate main size.
    auto contentConstraint = listLayoutProperty->GetContentLayoutConstraint().value();
    auto contentIdealSize = CreateIdealSize(
        contentConstraint, axis, listLayoutProperty->GetMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS));

    const auto& padding = listLayoutProperty->CreatePaddingAndBorder();
    paddingBeforeContent_ = axis == Axis::HORIZONTAL ? padding.left.value_or(0) : padding.top.value_or(0);
    paddingAfterContent_ = axis == Axis::HORIZONTAL ? padding.right.value_or(0) : padding.bottom.value_or(0);
    contentMainSize_ = 0.0f;
    totalItemCount_ = layoutWrapper->GetTotalChildCount();
    if (!GetMainAxisSize(contentIdealSize, axis)) {
        if (totalItemCount_ == 0) {
            contentMainSize_ = 0.0f;
        } else {
            // use parent max size first.
            auto parentMaxSize = contentConstraint.maxSize;
            contentMainSize_ = GetMainAxisSize(parentMaxSize, axis) - paddingBeforeContent_ - paddingAfterContent_;
            mainSizeIsDefined_ = false;
        }
    } else {
        contentMainSize_ = GetMainAxisSize(contentIdealSize.ConvertToSizeT(), axis);
        mainSizeIsDefined_ = true;
    }

    if (totalItemCount_ > 0) {
        currentOffset_ = currentDelta_;
        startMainPos_ = currentOffset_;
        endMainPos_ = currentOffset_ + contentMainSize_;
        stickyStyle_ = listLayoutProperty->GetStickyStyle().value_or(V2::StickyStyle::NONE);
        auto mainPercentRefer = GetMainAxisSize(contentConstraint.percentReference, axis);
        auto space = listLayoutProperty->GetSpace().value_or(Dimension(0));
        spaceWidth_ = ConvertToPx(space, layoutConstraint.scaleProperty, mainPercentRefer).value_or(0);
        if (listLayoutProperty->GetDivider().has_value()) {
            auto divider = listLayoutProperty->GetDivider().value();
            std::optional<float> dividerSpace =
                ConvertToPx(divider.strokeWidth, layoutConstraint.scaleProperty, mainPercentRefer);
            if (dividerSpace.has_value()) {
                spaceWidth_ = std::max(spaceWidth_, dividerSpace.value());
            }
        }

        CalculateLanes(listLayoutProperty, layoutConstraint, contentIdealSize.CrossSize(axis), axis);
        listItemAlign_ = listLayoutProperty->GetListItemAlign().value_or(V2::ListItemAlign::START);
        // calculate child layout constraint.
        auto childLayoutConstraint = listLayoutProperty->CreateChildConstraint();
        UpdateListItemConstraint(axis, contentIdealSize, childLayoutConstraint);
        MeasureList(layoutWrapper, childLayoutConstraint, axis);
    } else {
        LOGI("child size is empty");
    }

    if (axis == Axis::HORIZONTAL) {
        contentIdealSize.SetWidth(contentMainSize_);
    } else {
        contentIdealSize.SetHeight(contentMainSize_);
    }
    AddPaddingToSize(padding, contentIdealSize);
    layoutWrapper->GetGeometryNode()->SetFrameSize(contentIdealSize.ConvertToSizeT());

    // set list cache info.
    layoutWrapper->SetCacheCount(listLayoutProperty->GetCachedCountValue(1) * GetLanes());

    LOGD("new start index is %{public}d, new end index is %{public}d, offset is %{public}f, mainSize is %{public}f",
        GetStartIndex(), GetEndIndex(), currentOffset_, contentMainSize_);
}

void ListLayoutAlgorithm::CalculateEstimateOffset()
{
    if (itemPosition_.empty()) {
        estimateOffset_ = 0;
        return;
    }
    float itemsHeight = (itemPosition_.rbegin()->second.endPos - itemPosition_.begin()->second.startPos) + spaceWidth_;
    auto lines = static_cast<int32_t>(itemPosition_.size());
    if (GetLanes() > 1) {
        lines = (lines / GetLanes()) + (lines % GetLanes() > 0 ? 1 : 0);
    }
    if (lines > 0) {
        float averageHeight = itemsHeight / static_cast<float>(lines);
        estimateOffset_ = averageHeight * static_cast<float>(itemPosition_.begin()->first);
    } else {
        estimateOffset_ = 0;
    }
}

void ListLayoutAlgorithm::MeasureList(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis)
{
    int32_t startIndex = 0;
    float startPos = 0.0f;
    if (!itemPosition_.empty()) {
        startPos = itemPosition_.begin()->second.startPos;
        startIndex = std::min(GetStartIndex(), totalItemCount_ - 1);
        itemPosition_.clear();
        layoutWrapper->RemoveAllChildInRenderTree();
    }
    if (jumpIndex_ && (jumpIndex_.value() < 0 || jumpIndex_.value() >= totalItemCount_)) {
        LOGW("jump index is illegal, %{public}d, %{public}d", jumpIndex_.value(), totalItemCount_);
        jumpIndex_.reset();
    }
    if (jumpIndex_) {
        LOGD("Jump index: %{public}d, offset is %{public}f, startMainPos: %{public}f, endMainPos: %{public}f",
            jumpIndex_.value(), currentOffset_, startMainPos_, endMainPos_);
        jumpIndex_ = GetLanesFloor(layoutWrapper, jumpIndex_.value());
        if (scrollIndexAlignment_ == ScrollIndexAlignment::ALIGN_TOP) {
            LayoutForward(layoutWrapper, layoutConstraint, axis, jumpIndex_.value(), startMainPos_);
            if (jumpIndex_.value() > 0 && GreatNotEqual(GetStartPosition(), startMainPos_)) {
                LayoutBackward(layoutWrapper, layoutConstraint, axis, jumpIndex_.value() - 1, GetStartPosition());
            }
        } else if (scrollIndexAlignment_ == ScrollIndexAlignment::ALIGN_BUTTON) {
            LayoutBackward(layoutWrapper, layoutConstraint, axis, jumpIndex_.value(), endMainPos_);
            if (jumpIndex_.value() < totalItemCount_ - 1 && LessNotEqual(GetEndPosition(), endMainPos_)) {
                LayoutForward(layoutWrapper, layoutConstraint, axis, jumpIndex_.value() + 1, GetEndPosition());
            }
        }
        CalculateEstimateOffset();
    } else {
        LOGD("StartIndex index: %{public}d, offset is %{public}f, startMainPos: %{public}f, endMainPos: %{public}f",
            startIndex, currentOffset_, startMainPos_, endMainPos_);
        LayoutForward(layoutWrapper, layoutConstraint, axis, startIndex, startPos);
        if (GetStartIndex() > 0 && GreatNotEqual(GetStartPosition(), startMainPos_)) {
            LayoutBackward(layoutWrapper, layoutConstraint, axis, GetStartIndex() - 1, GetStartPosition());
        }
    }
    CreateItemGroupList(layoutWrapper);
}

int32_t ListLayoutAlgorithm::LayoutALineForward(LayoutWrapper* layoutWrapper,
    const LayoutConstraintF& layoutConstraint, Axis axis, int32_t& currentIndex, float startPos, float& endPos)
{
    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex + 1);
    CHECK_NULL_RETURN(wrapper, 0);
    ++currentIndex;
    bool isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (isGroup) {
        SetListItemGroupParam(wrapper);
    }
    {
        ACE_SCOPED_TRACE("ListLayoutAlgorithm::MeasureListItem:%d", currentIndex);
        wrapper->Measure(layoutConstraint);
    }
    float mainLen = GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis);
    endPos = startPos + mainLen;
    itemPosition_[currentIndex] = { startPos, endPos, isGroup };
    return 1;
}

int32_t ListLayoutAlgorithm::LayoutALineBackward(LayoutWrapper* layoutWrapper,
    const LayoutConstraintF& layoutConstraint, Axis axis, int32_t& currentIndex, float endPos, float& startPos)
{
    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex - 1);
    CHECK_NULL_RETURN(wrapper, 0);
    --currentIndex;
    bool isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (isGroup) {
        SetListItemGroupParam(wrapper);
    }
    {
        ACE_SCOPED_TRACE("ListLayoutAlgorithm::MeasureListItem:%d", currentIndex);
        wrapper->Measure(layoutConstraint);
    }
    float mainLen = GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis);
    startPos = endPos - mainLen;
    itemPosition_[currentIndex] = { startPos, endPos, isGroup };
    return 1;
}

void ListLayoutAlgorithm::LayoutForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
    Axis axis, int32_t startIndex, float startPos)
{
    float currentEndPos = startPos;
    float currentStartPos = 0.0f;
    float endMainPos = overScrollFeature_ ? std::max(startPos + contentMainSize_, endMainPos_) : endMainPos_;
    auto currentIndex = startIndex - 1;
    do {
        currentStartPos = currentEndPos;
        int32_t count = LayoutALineForward(layoutWrapper, layoutConstraint, axis, currentIndex,
            currentStartPos, currentEndPos);
        if (count == 0) {
            break;
        }
        if (currentIndex >= 0 && currentIndex < (totalItemCount_ - 1)) {
            currentEndPos += spaceWidth_;
        }
        LOGD("LayoutForward: %{public}d current start pos: %{public}f, current end pos: %{public}f", currentIndex,
            currentStartPos, currentEndPos);
    } while (LessNotEqual(currentEndPos, endMainPos + paddingAfterContent_));

    if (overScrollFeature_) {
        LOGD("during over scroll, just return in LayoutForward");
        return;
    }

    // adjust offset.
    if (LessNotEqual(currentEndPos, endMainPos_) && !itemPosition_.empty()) {
        auto firstItemTop = itemPosition_.begin()->second.startPos;
        auto itemTotalSize = currentEndPos - firstItemTop;
        if (LessOrEqual(itemTotalSize, contentMainSize_) && (itemPosition_.begin()->first == 0)) {
            // all items size is less than list.
            currentOffset_ = firstItemTop;
            startMainPos_ = currentOffset_;
            if (!mainSizeIsDefined_) {
                // adapt child size.
                LOGD("LayoutForward: adapt child total size");
                contentMainSize_ = itemTotalSize;
            }
        } else {
            // adjust offset. If edgeEffect is SPRING, jump adjust to allow list scroll through boundary
            if (!canOverScroll_ || jumpIndex_.has_value()) {
                currentOffset_ = currentEndPos - contentMainSize_;
                LOGD("LayoutForward: adjust offset to %{public}f", currentOffset_);
            }
            startMainPos_ = currentEndPos - contentMainSize_;
            endMainPos_ = currentEndPos;
        }
    }

    // Mark inactive in wrapper.
    for (auto pos = itemPosition_.begin(); pos != itemPosition_.end();) {
        if (GreatOrEqual(pos->second.endPos, startMainPos_ - paddingBeforeContent_)) {
            break;
        }
        LOGI("recycle item:%{public}d", pos->first);
        layoutWrapper->RemoveChildInRenderTree(pos->first);
        itemPosition_.erase(pos++);
    }
}

void ListLayoutAlgorithm::LayoutBackward(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis, int32_t endIndex, float endPos)
{
    float currentStartPos = endPos;
    float currentEndPos = 0.0f;
    float startMainPos = overScrollFeature_ ? std::min(endPos - contentMainSize_, startMainPos_) : startMainPos_;
    auto currentIndex = endIndex + 1;
    do {
        currentEndPos = currentStartPos;
        int32_t count = LayoutALineBackward(layoutWrapper, layoutConstraint, axis, currentIndex,
            currentEndPos, currentStartPos);
        if (count == 0) {
            break;
        }
        if (currentIndex > 0) {
            currentStartPos = currentStartPos - spaceWidth_;
        }
        LOGD("LayoutBackward: %{public}d current start pos: %{public}f, current end pos: %{public}f", currentIndex,
            currentStartPos, currentEndPos);
    } while (GreatNotEqual(currentStartPos, startMainPos - paddingBeforeContent_));

    if (overScrollFeature_) {
        LOGD("during over scroll, just return in LayoutBackward");
        return;
    }

    // adjust offset. If edgeEffect is SPRING, jump adjust to allow list scroll through boundary
    if (GreatNotEqual(currentStartPos, startMainPos_)) {
        if (!canOverScroll_ || jumpIndex_.has_value()) {
            currentOffset_ = currentStartPos;
        }
        endMainPos_ = currentStartPos + contentMainSize_;
        startMainPos_ = currentStartPos;
    }

    // Mark inactive in wrapper.
    std::list<int32_t> removeIndexes;
    for (auto pos = itemPosition_.rbegin(); pos != itemPosition_.rend(); ++pos) {
        if (LessOrEqual(pos->second.startPos, endMainPos_ + paddingAfterContent_)) {
            break;
        }
        layoutWrapper->RemoveChildInRenderTree(pos->first);
        removeIndexes.emplace_back(pos->first);
    }
    for (const auto& index : removeIndexes) {
        itemPosition_.erase(index);
    }
}

void ListLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto listLayoutProperty = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(listLayoutProperty);
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto paddingOffset = padding.Offset();
    float crossSize = GetCrossAxisSize(size, axis);
    totalItemCount_ = layoutWrapper->GetTotalChildCount();
    listItemAlign_ = listLayoutProperty->GetListItemAlign().value_or(V2::ListItemAlign::START);
    int32_t startIndex = GetStartIndex();

    // layout items.
    for (auto& pos : itemPosition_) {
        int32_t index = pos.first;
        auto offset = paddingOffset;
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!wrapper) {
            LOGI("wrapper is out of boundary");
            continue;
        }
        float childCrossSize = GetCrossAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis);
        float crossOffset = 0.0f;
        pos.second.startPos -= currentOffset_;
        pos.second.endPos -= currentOffset_;
        if (GetLanes() > 1) {
            int32_t laneIndex = 0;
            if (pos.second.isGroup) {
                startIndex = index + 1;
            } else {
                laneIndex = (index - startIndex) % GetLanes();
            }
            crossOffset = CalculateLaneCrossOffset(crossSize, childCrossSize * GetLanes());
            crossOffset += crossSize / GetLanes() * laneIndex;
        } else {
            crossOffset = CalculateLaneCrossOffset(crossSize, childCrossSize);
        }
        if (axis == Axis::VERTICAL) {
            offset = offset + OffsetF(crossOffset, pos.second.startPos);
        } else {
            offset = offset + OffsetF(pos.second.startPos, crossOffset);
        }
        wrapper->GetGeometryNode()->SetMarginFrameOffset(offset);
        if (!overScrollFeature_ || wrapper->CheckNeedForceMeasureAndLayout()) {
            wrapper->Layout();
        }
    }
}

float ListLayoutAlgorithm::CalculateLaneCrossOffset(float crossSize, float childCrossSize)
{
    float delta = crossSize - childCrossSize;
    if (LessOrEqual(delta, 0)) {
        return 0.0f;
    }
    switch (listItemAlign_) {
        case OHOS::Ace::V2::ListItemAlign::START:
            return 0.0f;
        case OHOS::Ace::V2::ListItemAlign::CENTER:
            return delta / 2.0f;
        case OHOS::Ace::V2::ListItemAlign::END:
            return delta;
        default:
            LOGW("Invalid ListItemAlign: %{public}d", listItemAlign_);
            return 0.0f;
    }
}

void ListLayoutAlgorithm::SetListItemGroupParam(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    auto layoutAlgorithmWrapper = layoutWrapper->GetLayoutAlgorithm();
    CHECK_NULL_VOID(layoutAlgorithmWrapper);
    auto itemGroup = AceType::DynamicCast<ListItemGroupLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_VOID(itemGroup);
    itemGroup->SetListMainSize(contentMainSize_);
}

void ListLayoutAlgorithm::CreateItemGroupList(LayoutWrapper* layoutWrapper)
{
    itemGroupList_.clear();
    if (stickyStyle_ != V2::StickyStyle::NONE) {
        for (const auto& item : itemPosition_) {
            if (item.second.isGroup) {
                auto wrapper = layoutWrapper->GetOrCreateChildByIndex(item.first);
                itemGroupList_.push_back(wrapper->GetWeakHostNode());
            }
        }
    }
}

bool ListLayoutAlgorithm::HasItemGroup()
{
    return std::any_of(itemPosition_.begin(), itemPosition_.end(),
        [](const auto& item) { return item.second.isGroup; });
}
} // namespace OHOS::Ace::NG

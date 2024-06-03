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

#include "core/components_ng/pattern/swiper/swiper_layout_algorithm.h"

#include <algorithm>
#include <cstdint>

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_utils.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_paint_property.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_utils.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

namespace {
constexpr Dimension INDICATOR_PADDING = 8.0_vp;
constexpr uint32_t INDICATOR_HAS_CHILD = 2;
constexpr uint32_t SWIPER_HAS_CHILD = 5;
} // namespace

bool SwiperLayoutAlgorithm::CheckIsSingleCase(const RefPtr<SwiperLayoutProperty>& property)
{
    bool hasMinSize = property->GetMinSize().has_value() && !LessOrEqual(property->GetMinSizeValue().Value(), 0);
    bool hasPrevMargin =
        property->GetPrevMargin().has_value() && !LessOrEqual(property->GetPrevMarginValue().ConvertToPx(), 0);
    bool hasNextMargin =
        property->GetNextMargin().has_value() && !LessOrEqual(property->GetNextMarginValue().ConvertToPx(), 0);

    return !hasMinSize && (!hasPrevMargin && !hasNextMargin) &&
           ((property->GetDisplayCount().has_value() && property->GetDisplayCountValue() == 1) ||
               (!property->GetDisplayCount().has_value() && SwiperUtils::IsStretch(property)));
}

void SwiperLayoutAlgorithm::IndicatorAndArrowMeasure(LayoutWrapper* layoutWrapper, const OptionalSizeF& parentIdealSize)
{
    auto property = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(property);

    // Measure swiper indicator
    if (property->GetShowIndicatorValue(true)) {
        auto hostNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(hostNode);
        auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
        CHECK_NULL_VOID(swiperPattern);
        auto indicatorWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_INDICATOR_ETS_TAG);
        if (indicatorWrapper) {
            auto indicatorLayoutConstraint = property->CreateChildConstraint();
            indicatorLayoutConstraint.parentIdealSize = parentIdealSize;
            indicatorWrapper->Measure(indicatorLayoutConstraint);
        }
    }

    if (property->GetDisplayArrowValue(false)) {
        auto hostNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(hostNode);
        auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
        CHECK_NULL_VOID(swiperPattern);

        if (swiperPattern->HasLeftButtonNode() && swiperPattern->HasRightButtonNode()) {
            auto leftArrowWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_ARROW_ETS_TAG);
            auto rightArrowWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_ARROW_ETS_TAG);
            CHECK_NULL_VOID(leftArrowWrapper);
            CHECK_NULL_VOID(rightArrowWrapper);
            if (leftArrowWrapper->GetHostTag() == V2::SWIPER_LEFT_ARROW_ETS_TAG &&
                rightArrowWrapper->GetHostTag() == V2::SWIPER_RIGHT_ARROW_ETS_TAG) {
                MeasureArrow(leftArrowWrapper, property);
                MeasureArrow(rightArrowWrapper, property);
            }
        }
    }
}

void SwiperLayoutAlgorithm::UpdateLayoutInfoBeforeMeasureSwiper(const RefPtr<SwiperLayoutProperty>& property)
{
    currentOffset_ = currentDelta_;
    startMainPos_ = currentOffset_;
    ACE_SCOPED_TRACE("measure swiper startMainPos_:%f", startMainPos_);
    if ((Positive(prevMargin_) && NonPositive(property->GetPrevMarginValue(0.0_px).ConvertToPx())) ||
        (Positive(nextMargin_) && NonPositive(property->GetNextMarginValue(0.0_px).ConvertToPx()))) {
        prevMargin_ = 0.0f;
        nextMargin_ = 0.0f;
        isNeedResetPrevMarginAndNextMargin_ = true;
    }
    if (!NearZero(prevMargin_)) {
        if (!NearZero(nextMargin_)) {
            endMainPos_ = currentOffset_ + contentMainSize_ - prevMargin_ - nextMargin_ - 2 * spaceWidth_;
        } else {
            endMainPos_ = currentOffset_ + contentMainSize_ - prevMargin_ - spaceWidth_;
        }
    } else {
        if (!NearZero(nextMargin_)) {
            endMainPos_ = currentOffset_ + contentMainSize_ - nextMargin_ - spaceWidth_;
        } else {
            endMainPos_ = currentOffset_ + contentMainSize_;
        }
    }
}

int32_t SwiperLayoutAlgorithm::GetLoopIndex(int32_t originalIndex) const
{
    auto loopIndex = originalIndex;
    while (loopIndex < 0) {
        loopIndex = loopIndex + totalItemCount_;
    }
    loopIndex %= totalItemCount_;
    return loopIndex;
}

void SwiperLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);

    if (swiperLayoutProperty->GetIsCustomAnimation().value_or(false)) {
        MeasureTabsCustomAnimation(layoutWrapper);
        return;
    }

    if (SwiperUtils::IsStretch(swiperLayoutProperty)) {
        prevMargin_ = static_cast<float>(swiperLayoutProperty->GetPrevMarginValue(0.0_px).ConvertToPx());
        nextMargin_ = static_cast<float>(swiperLayoutProperty->GetNextMarginValue(0.0_px).ConvertToPx());
    }

    auto axis = swiperLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
    // calculate main size.
    auto contentConstraint = swiperLayoutProperty->GetContentLayoutConstraint().value();
    auto isSingleCase = CheckIsSingleCase(swiperLayoutProperty);
    OptionalSizeF contentIdealSize;
    if (isSingleCase) {
        contentIdealSize = CreateIdealSizeByPercentRef(contentConstraint, axis, MeasureType::MATCH_CONTENT);
        if (mainSizeIsMeasured_) {
            if (layoutWrapper->IsContraintNoChanged()) {
                contentIdealSize.SetMainSize(contentMainSize_, axis);
            } else {
                mainSizeIsMeasured_ = false;
            }
        }
    } else {
        contentIdealSize = CreateIdealSizeByPercentRef(contentConstraint, axis, MeasureType::MATCH_PARENT_MAIN_AXIS);
        if (!layoutWrapper->IsContraintNoChanged()) {
            mainSizeIsMeasured_ = false;
            jumpIndex_ = currentIndex_;
        }
    }

    const auto& padding = swiperLayoutProperty->CreatePaddingAndBorder();
    paddingBeforeContent_ = axis == Axis::HORIZONTAL ? padding.left.value_or(0) : padding.top.value_or(0);
    paddingAfterContent_ = axis == Axis::HORIZONTAL ? padding.right.value_or(0) : padding.bottom.value_or(0);
    contentMainSize_ = 0.0f;
    if (!GetMainAxisSize(contentIdealSize, axis) && (!isSingleCase || !mainSizeIsMeasured_)) {
        if (totalItemCount_ == 0) {
            contentMainSize_ = 0.0f;
        } else {
            // use parent percentReference size first.
            auto parentPercentReference = contentConstraint.percentReference;
            contentMainSize_ =
                GetMainAxisSize(parentPercentReference, axis) - paddingBeforeContent_ - paddingAfterContent_;
            mainSizeIsDefined_ = false;
        }
    } else {
        contentMainSize_ = GetMainAxisSize(contentIdealSize.ConvertToSizeT(), axis);
        mainSizeIsDefined_ = true;
    }
    auto hostNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(hostNode);
    auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto getAutoFill = swiperPattern->IsAutoFill();

    // calculate child layout constraint.
    auto childLayoutConstraint =
        SwiperUtils::CreateChildConstraint(swiperLayoutProperty, contentIdealSize, getAutoFill);
    auto itemSpace = SwiperUtils::GetItemSpace(swiperLayoutProperty);
    spaceWidth_ = itemSpace > (contentMainSize_ + paddingBeforeContent_ + paddingAfterContent_) ? 0.0f : itemSpace;
    if (totalItemCount_ > 0) {
        UpdateLayoutInfoBeforeMeasureSwiper(swiperLayoutProperty);
        MeasureSwiper(layoutWrapper, childLayoutConstraint, axis);
    } else {
        itemPosition_.clear();
    }

    auto crossSize = contentIdealSize.CrossSize(axis);
    if ((crossSize.has_value() && GreaterOrEqualToInfinity(crossSize.value())) || !crossSize.has_value()) {
        contentCrossSize_ = GetChildMaxSize(layoutWrapper, axis, false) == 0.0f
                                ? contentCrossSize_
                                : GetChildMaxSize(layoutWrapper, axis, false);
        contentIdealSize.SetCrossSize(contentCrossSize_, axis);
        crossMatchChild_ = true;
    }

    if (!mainSizeIsDefined_ && isSingleCase) {
        auto childMaxMainSize = GetChildMaxSize(layoutWrapper, axis, true);
        if (childMaxMainSize != contentMainSize_) {
            contentMainSize_ = childMaxMainSize;
            // CheckInactive
            SetInactive(layoutWrapper, 0.0f, contentMainSize_, currentTargetIndex_);
        }
    }
    MeasureSwiperCustomAnimation(layoutWrapper, childLayoutConstraint);
    if (itemPosition_.empty()) {
        layoutWrapper->SetActiveChildRange(-1, -1);
    } else if (itemPositionInAnimation_.empty()) {
        int32_t startIndex = std::min(GetLoopIndex(GetStartIndex()), realTotalCount_ - 1);
        int32_t endIndex = std::min(GetLoopIndex(GetEndIndex()), realTotalCount_ - 1);
        layoutWrapper->SetActiveChildRange(startIndex, endIndex);
    } else {
        int32_t startIndex = std::min(GetLoopIndex(itemPositionInAnimation_.begin()->first), realTotalCount_ - 1);
        int32_t endIndex = std::min(GetLoopIndex(itemPositionInAnimation_.rbegin()->first), realTotalCount_ - 1);
        while (startIndex + 1 < realTotalCount_ &&
               itemPositionInAnimation_.find(startIndex + 1) != itemPositionInAnimation_.end()) {
            startIndex++;
        }
        while (endIndex - 1 >= 0 && itemPositionInAnimation_.find(endIndex - 1) != itemPositionInAnimation_.end()) {
            endIndex--;
        }
        layoutWrapper->SetActiveChildRange(endIndex, startIndex);
    }

    contentIdealSize.SetMainSize(contentMainSize_, axis);
    AddPaddingToSize(padding, contentIdealSize);
    layoutWrapper->GetGeometryNode()->SetFrameSize(contentIdealSize.ConvertToSizeT());
    if (!itemPosition_.empty()) {
        mainSizeIsMeasured_ = true;
    }

    // set swiper cache info.
    auto measuredItemCount = static_cast<int32_t>(measuredItems_.size());
    auto maxCachedCount = static_cast<int32_t>(std::ceil(static_cast<float>(realTotalCount_ - measuredItemCount) / 2));
    layoutWrapper->SetCacheCount(std::min(swiperPattern->GetCachedCount(), maxCachedCount), childLayoutConstraint);
    layoutWrapper->SetLongPredictTask();

    IndicatorAndArrowMeasure(layoutWrapper, contentIdealSize);
    CaptureMeasure(layoutWrapper, childLayoutConstraint);

    if (swiperLayoutProperty->GetFlexItemProperty()) {
        measured_ = true;
    }
}

void SwiperLayoutAlgorithm::CaptureMeasure(LayoutWrapper* layoutWrapper, LayoutConstraintF& childLayoutConstraint)
{
    if (!hasCachedCapture_ || itemPosition_.empty()) {
        return;
    }
    auto hostNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(hostNode);
    auto leftCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_CAPTURE_ETS_TAG);
    auto rightCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_CAPTURE_ETS_TAG);
    if (isCaptureReverse_) {
        leftCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_CAPTURE_ETS_TAG);
        rightCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_CAPTURE_ETS_TAG);
    }
    CHECK_NULL_VOID(leftCaptureWrapper);
    CHECK_NULL_VOID(rightCaptureWrapper);
    auto lastWrapper = layoutWrapper->GetOrCreateChildByIndex(GetLoopIndex(itemPosition_.rbegin()->first));
    CHECK_NULL_VOID(lastWrapper);
    auto lastNode = lastWrapper->GetGeometryNode();
    CHECK_NULL_VOID(lastNode);
    auto leftCaptureGeometryNode = leftCaptureWrapper->GetGeometryNode();
    CHECK_NULL_VOID(leftCaptureGeometryNode);
    auto leftOldSize = leftCaptureGeometryNode->GetFrameSize();
    childLayoutConstraint.UpdateSelfMarginSizeWithCheck(OptionalSizeF(lastNode->GetMarginFrameSize()));
    leftCaptureWrapper->Measure(childLayoutConstraint);

    auto firstWrapper = layoutWrapper->GetOrCreateChildByIndex(GetLoopIndex(itemPosition_.begin()->first));
    CHECK_NULL_VOID(firstWrapper);
    auto firstNode = firstWrapper->GetGeometryNode();
    CHECK_NULL_VOID(firstNode);
    auto rightCaptureGeometryNode = rightCaptureWrapper->GetGeometryNode();
    CHECK_NULL_VOID(rightCaptureGeometryNode);
    auto rightOldSize = rightCaptureGeometryNode->GetFrameSize();
    childLayoutConstraint.UpdateSelfMarginSizeWithCheck(OptionalSizeF(firstNode->GetMarginFrameSize()));
    rightCaptureWrapper->Measure(childLayoutConstraint);

    isNeedUpdateCapture_ = leftOldSize != lastNode->GetFrameSize() || rightOldSize != firstNode->GetFrameSize();
}

void SwiperLayoutAlgorithm::MeasureTabsCustomAnimation(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
    auto contentConstraint = layoutProperty->GetContentLayoutConstraint().value();
    auto contentIdealSize = CreateIdealSizeByPercentRef(contentConstraint, axis, MeasureType::MATCH_PARENT_MAIN_AXIS);
    auto childLayoutConstraint = SwiperUtils::CreateChildConstraint(layoutProperty, contentIdealSize, false);

    auto currentIndex = layoutProperty->GetIndex().value_or(0);
    auto currentIndexWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
    CHECK_NULL_VOID(currentIndexWrapper);
    currentIndexWrapper->Measure(childLayoutConstraint);

    if (customAnimationToIndex_) {
        auto toIndexWrapper = layoutWrapper->GetOrCreateChildByIndex(customAnimationToIndex_.value());
        CHECK_NULL_VOID(toIndexWrapper);
        toIndexWrapper->Measure(childLayoutConstraint);
    }

    layoutWrapper->GetGeometryNode()->SetFrameSize(contentIdealSize.ConvertToSizeT());

    std::set<int32_t> removeIndexs;
    for (const auto& index : needUnmountIndexs_) {
        if (indexsInAnimation_.find(index) != indexsInAnimation_.end()) {
            continue;
        }

        layoutWrapper->RemoveChildInRenderTree(index);
        removeIndexs.insert(index);
    }

    for (const auto& index : removeIndexs) {
        needUnmountIndexs_.erase(index);
    }
}

void SwiperLayoutAlgorithm::MeasureSwiperCustomAnimation(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint)
{
    std::set<int32_t> measureIndexSet;
    for (const auto& pos : itemPosition_) {
        measureIndexSet.insert(GetLoopIndex(pos.first));
    }
    for (const auto& pos : itemPositionInAnimation_) {
        if (measureIndexSet.find(pos.first) == measureIndexSet.end()) {
            auto currentIndexWrapper = layoutWrapper->GetOrCreateChildByIndex(pos.first);
            CHECK_NULL_VOID(currentIndexWrapper);
            currentIndexWrapper->Measure(layoutConstraint);
        }
    }
}

float SwiperLayoutAlgorithm::GetChildMaxSize(LayoutWrapper* layoutWrapper, Axis axis, bool isMainAxis) const
{
    if (itemPosition_.empty()) {
        return 0.0f;
    }
    float maxSize = 0.0f;
    float size = 0.0f;
    for (const auto& pos : itemPosition_) {
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetLoopIndex(pos.first), false);
        if (!wrapper) {
            continue;
        }
        auto geometryNode = wrapper->GetGeometryNode();
        if (!geometryNode) {
            continue;
        }
        size = isMainAxis ? geometryNode->GetMarginFrameSize().MainSize(axis)
                          : geometryNode->GetMarginFrameSize().CrossSize(axis);
        maxSize = std::max(size, maxSize);
    }
    return maxSize;
}

void SwiperLayoutAlgorithm::AdjustStartInfoOnSwipeByGroup(
    int32_t startIndex, const PositionMap& itemPosition, int32_t& startIndexInVisibleWindow, float& startPos)
{
    if (!swipeByGroup_ || isFrameAnimation_) {
        return;
    }

    startIndexInVisibleWindow = startIndex;
    auto iter = itemPosition.find(startIndex);
    if (iter != itemPosition.end()) {
        startPos = iter->second.startPos;
    }
}

void SwiperLayoutAlgorithm::MeasureSwiper(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis)
{
    if (layoutWrapper->GetLayoutProperty()->GetFlexItemProperty() && measured_) {
        // flex property causes Swiper to be measured twice, and itemPosition_ would
        // reset after the first measure. Restore to that on second measure.
        itemPosition_ = prevItemPosition_;
        // targetIndex_ has also been reset during the first measure.
        targetIndex_ = currentTargetIndex_;
    } else if (targetIndex_.has_value()) {
        currentTargetIndex_ = targetIndex_.value();
    }
    int32_t startIndex = 0;
    int32_t endIndex = 0;
    float startPos = 0.0f;
    float endPos = 0.0f;
    int32_t startIndexInVisibleWindow = 0;
    prevItemPosition_ = itemPosition_;
    if (!itemPosition_.empty()) {
        startPos = itemPosition_.begin()->second.startPos;
        endPos = itemPosition_.rbegin()->second.endPos;
        for (const auto& item : itemPosition_) {
            float endPos = Positive(prevMargin_) ? item.second.endPos + prevMargin_ + spaceWidth_ : item.second.endPos;
            if (Positive(endPos)) {
                startIndexInVisibleWindow = item.first;
                startPos = item.second.startPos;
                break;
            }
        }
        if (!isLoop_) {
            startIndex = std::min(GetLoopIndex(GetStartIndex()), totalItemCount_ - 1);
            endIndex = std::min(GetLoopIndex(GetEndIndex()), totalItemCount_ - 1);
            startIndexInVisibleWindow = std::min(GetLoopIndex(startIndexInVisibleWindow), totalItemCount_ - 1);
            if (targetIndex_.has_value()) {
                targetIndex_ = GetLoopIndex(targetIndex_.value());
            }
        } else {
            startIndex = GetStartIndex();
            endIndex = GetEndIndex();
        }

        itemPosition_.clear();
    }

    if (!placeItemWidth_ && !prevItemPosition_.empty()) {
        placeItemWidth_ = prevItemPosition_.begin()->second.endPos - prevItemPosition_.begin()->second.startPos;
    }

    if (jumpIndex_) {
        needUnmountIndexs_.clear();
        itemPositionInAnimation_.clear();
        startPos = (jumpIndex_.value() == 0) && Negative(startMainPos_) ? startMainPos_ : 0;
        LayoutForward(layoutWrapper, layoutConstraint, axis, jumpIndex_.value(), startPos);
        auto prevMarginMontage = Positive(prevMargin_) ? prevMargin_ + spaceWidth_ : 0.0f;
        if ((jumpIndex_.value() > 0 && GreatNotEqual(GetStartPosition(), startMainPos_ - prevMarginMontage)) ||
            (isLoop_ && Positive(prevMargin_))) {
            LayoutBackward(
                layoutWrapper, layoutConstraint, axis, jumpIndex_.value() - 1, GetStartPosition());
        }
        currentIndex_ = jumpIndex_.value();
    } else if (hasCachedCapture_) {
        if (targetIndex_.has_value()) {
            auto firstItemIndex = prevItemPosition_.begin()->first;
            // Swipe to the left, layout forward from (targetIndex - 1)
            if (targetIndex_.value() > firstItemIndex) {
                LayoutForward(layoutWrapper, layoutConstraint, axis, targetIndex_.value() - 1,
                    prevItemPosition_[targetIndex_.value() - 1].startPos);
                // Swipe to the right, layout backward from (endIndex - 1)
            } else if (targetIndex_.value() < firstItemIndex) {
                auto basicItem = ++prevItemPosition_.rbegin();
                LayoutBackward(layoutWrapper, layoutConstraint, axis, basicItem->first, basicItem->second.endPos);
                // captures need not to be updated
            } else {
                itemPosition_ = prevItemPosition_;
            }
            // captures need to be updated, swap capture to avoid flickering of disappearing real node's position
            if (itemPosition_.begin()->first != prevItemPosition_.begin()->first) {
                isCaptureReverse_ = !isCaptureReverse_;
            }
        } else {
            if (NonNegative(currentOffset_)) {
                // Swipe to the left
                LayoutBackward(layoutWrapper, layoutConstraint, axis, endIndex, endPos);
                LayoutForward(layoutWrapper, layoutConstraint, axis, GetEndIndex() + 1, GetEndPosition());
            } else {
                // Swipe to the right
                LayoutForward(layoutWrapper, layoutConstraint, axis, startIndexInVisibleWindow, startPos);
                LayoutBackward(layoutWrapper, layoutConstraint, axis, GetStartIndex() - 1, GetStartPosition());
            }
        }
    } else if (targetIndex_.has_value()) {
        // isMeasureOneMoreItem_ param is used to ensure item continuity when play property animation.
        if (LessNotEqual(startIndexInVisibleWindow, targetIndex_.value())) {
            if (isMeasureOneMoreItem_) {
                targetIndex_ = isLoop_ ? targetIndex_.value() + 1 :
                    std::clamp(targetIndex_.value() + 1, 0, realTotalCount_ - 1);
            }
            AdjustStartInfoOnSwipeByGroup(startIndex, prevItemPosition_, startIndexInVisibleWindow, startPos);
            LayoutForward(layoutWrapper, layoutConstraint, axis, startIndexInVisibleWindow, startPos);
            if (GreatNotEqualCustomPrecision(GetStartPosition(), startMainPos_, 0.01f)) {
                LayoutBackward(layoutWrapper, layoutConstraint, axis, GetStartIndex() - 1, GetStartPosition());
            }
        } else if (GreatNotEqual(startIndexInVisibleWindow, targetIndex_.value())) {
            if (isMeasureOneMoreItem_) {
                targetIndex_ = isLoop_ ? targetIndex_.value() - 1 :
                    std::clamp(targetIndex_.value() - 1, 0, realTotalCount_ - 1);
            }
            LayoutBackward(layoutWrapper, layoutConstraint, axis, endIndex, endPos);
            if (LessNotEqualCustomPrecision(GetEndPosition(), endMainPos_, -0.01f)) {
                LayoutForward(layoutWrapper, layoutConstraint, axis, GetEndIndex() + 1, GetEndPosition());
            }
        } else {
            targetIsSameWithStartFlag_ = true;
            AdjustStartInfoOnSwipeByGroup(startIndex, prevItemPosition_, startIndexInVisibleWindow, startPos);
            LayoutForward(layoutWrapper, layoutConstraint, axis, startIndexInVisibleWindow, startPos);
            if (isMeasureOneMoreItem_ || Positive(prevMargin_)) {
                float startPosition =
                    itemPosition_.empty() ? 0.0f : itemPosition_.begin()->second.startPos - spaceWidth_;
                LayoutBackward(layoutWrapper, layoutConstraint, axis, GetStartIndex() - 1, startPosition);
            }
        }
    } else {
        AdjustStartInfoOnSwipeByGroup(startIndex, prevItemPosition_, startIndexInVisibleWindow, startPos);
        bool overScrollTop = startIndexInVisibleWindow == 0 && GreatNotEqual(startPos, startMainPos_);
        if ((!overScrollFeature_ && NonNegative(currentOffset_)) || (overScrollFeature_ && overScrollTop)) {
            LayoutForward(layoutWrapper, layoutConstraint, axis, startIndexInVisibleWindow, startPos);
            if (GetStartIndex() > 0 && GreatNotEqual(GetStartPosition(), startMainPos_)) {
                LayoutBackward(layoutWrapper, layoutConstraint, axis, GetStartIndex() - 1, GetStartPosition());
            }
        } else {
            LayoutBackward(layoutWrapper, layoutConstraint, axis, endIndex, endPos);
            if (GetEndIndex() < (totalItemCount_ - 1) && LessNotEqual(GetEndPosition(), endMainPos_)) {
                LayoutForward(layoutWrapper, layoutConstraint, axis, GetEndIndex() + 1, GetEndPosition());
            }
        }
    }
}

bool SwiperLayoutAlgorithm::LayoutForwardItem(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
    Axis axis, int32_t& currentIndex, float startPos, float& endPos)
{
    if ((currentIndex + 1 >= totalItemCount_ && !isLoop_) ||
        (static_cast<int32_t>(itemPosition_.size()) >= totalItemCount_)) {
        return false;
    }

    auto measureIndex = GetLoopIndex(currentIndex + 1);
    if (isMeasureOneMoreItem_ && !itemPosition_.empty() &&
        measureIndex == GetLoopIndex(itemPosition_.begin()->first)) {
        return false;
    }

    if (swipeByGroup_ && measureIndex >= realTotalCount_) {
        ++currentIndex;
        endPos = startPos + placeItemWidth_.value_or(0.0f);
        itemPosition_[currentIndex] = { startPos, endPos, nullptr };
        return true;
    }

    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(measureIndex);
    if (!IsNormalItem(wrapper)) {
        return false;
    }
    ++currentIndex;
    {
        if (wrapper->GetHostNode()) {
            ACE_SCOPED_TRACE("[MeasureSwiperForwardItem:%d][self:%d][parent:%d][start:%f]", currentIndex,
                wrapper->GetHostNode()->GetId(),
                wrapper->GetHostNode()->GetParent() ? wrapper->GetHostNode()->GetParent()->GetId() : 0, startPos);
        }
        wrapper->Measure(layoutConstraint);
        measuredItems_.insert(measureIndex);
    }

    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(swiperLayoutProperty, false);

    float mainAxisSize = GetChildMainAxisSize(wrapper, swiperLayoutProperty, axis);
    endPos = startPos + mainAxisSize;
    itemPosition_[currentIndex] = { startPos, endPos, wrapper->GetHostNode() };
    return true;
}

bool SwiperLayoutAlgorithm::LayoutBackwardItem(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
    Axis axis, int32_t& currentIndex, float endPos, float& startPos)
{
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(swiperLayoutProperty, 0);
    int32_t displayCount =
        swiperLayoutProperty->GetDisplayCount().has_value() ? swiperLayoutProperty->GetDisplayCount().value() : 1;
    if ((currentIndex - 1 < 0 && !isLoop_) ||
        static_cast<int32_t>(itemPosition_.size()) >= totalItemCount_ + displayCount - 1) {
        return false;
    }
    if (hasCachedCapture_ && static_cast<int32_t>(itemPosition_.size()) >= totalItemCount_) {
        return false;
    }

    auto measureIndex = GetLoopIndex(currentIndex - 1);
    if (isMeasureOneMoreItem_ && !itemPosition_.empty() &&
        measureIndex == GetLoopIndex(itemPosition_.rbegin()->first)) {
        return false;
    }

    if (swipeByGroup_ && measureIndex >= realTotalCount_) {
        --currentIndex;
        startPos = endPos - placeItemWidth_.value_or(0.0f);
        itemPosition_[currentIndex] = { startPos, endPos, nullptr };
        return true;
    }

    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetLoopIndex(measureIndex));
    if (!IsNormalItem(wrapper)) {
        return false;
    }
    --currentIndex;
    {
        if (wrapper->GetHostNode()) {
            ACE_SCOPED_TRACE("[MeasureSwiperBackwardItem:%d][self:%d][parent:%d][endPos:%f]", currentIndex,
                wrapper->GetHostNode()->GetId(),
                wrapper->GetHostNode()->GetParent() ? wrapper->GetHostNode()->GetParent()->GetId() : 0, endPos);
        }
        wrapper->Measure(layoutConstraint);
        measuredItems_.insert(measureIndex);
    }

    float mainAxisSize = GetChildMainAxisSize(wrapper, swiperLayoutProperty, axis);
    startPos = endPos - mainAxisSize;
    itemPosition_[currentIndex] = { startPos, endPos, wrapper->GetHostNode() };
    return true;
}

void SwiperLayoutAlgorithm::SetInactiveOnForward(LayoutWrapper* layoutWrapper, Axis axis)
{
    auto displayCount = GetDisplayCount(layoutWrapper);
    for (auto pos = itemPosition_.begin(); pos != itemPosition_.end();) {
        auto endPos = pos->second.endPos;
        auto index = pos->first;
        if (swipeByGroup_) {
            auto endPageIndex = SwiperUtils::ComputePageEndIndex(index, displayCount);
            auto iter = itemPosition_.find(endPageIndex);
            if (iter != itemPosition_.end()) {
                endPos = iter->second.endPos;
            }
        }

        if (GreatNotEqual(endPos, prevMargin_ != 0.0f ? startMainPos_ - prevMargin_ - spaceWidth_ : startMainPos_)) {
            break;
        }

        ResetOffscreenItemPosition(layoutWrapper, GetLoopIndex(pos->first), true, axis);
        itemPosition_.erase(pos++);
    }
}

int32_t SwiperLayoutAlgorithm::GetDisplayCount(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, 1);
    return layoutProperty->GetDisplayCount().value_or(1);
}

float SwiperLayoutAlgorithm::GetChildMainAxisSize(
    const RefPtr<LayoutWrapper>& childWrapper, const RefPtr<SwiperLayoutProperty>& swiperProperty, Axis axis)
{
    CHECK_NULL_RETURN(childWrapper, 0.0f);

    float mainAxisSize = GetMainAxisSize(childWrapper->GetGeometryNode()->GetMarginFrameSize(), axis);
    if (!placeItemWidth_.has_value()) {
        placeItemWidth_ = mainAxisSize;
    }

    auto displayCount = swiperProperty->GetDisplayCountValue(1);
    if (!SwiperUtils::IsStretch(swiperProperty) || displayCount == 0) {
        return mainAxisSize;
    }

    auto childProperty = childWrapper->GetLayoutProperty();
    CHECK_NULL_RETURN(childProperty, mainAxisSize);
    auto visibilityValue = childProperty->GetVisibilityValue(VisibleType::VISIBLE);
    if (visibilityValue == VisibleType::INVISIBLE || visibilityValue == VisibleType::GONE) {
        mainAxisSize = (contentMainSize_ - (displayCount - 1) * spaceWidth_) / displayCount;
    }

    return mainAxisSize;
}

void SwiperLayoutAlgorithm::LayoutForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
    Axis axis, int32_t startIndex, float startPos)
{
    float currentEndPos = startPos;
    float currentStartPos = 0.0f;
    float endMainPos = overScrollFeature_ ? std::max(startPos + contentMainSize_, endMainPos_) : endMainPos_;
    if (targetIndex_) {
        endMainPos = Infinity<float>();
    }
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);

    auto currentIndex = startIndex - 1;
    do {
        currentStartPos = currentEndPos;
        auto result =
            LayoutForwardItem(layoutWrapper, layoutConstraint, axis, currentIndex, currentStartPos, currentEndPos);
        if (!result) {
            break;
        }
        bool hasMinSize = !LessOrEqual(swiperLayoutProperty->GetMinSizeValue(Dimension(0)).Value(), 0);
        bool hasPrevMargin = !LessOrEqual(swiperLayoutProperty->GetPrevMarginValue(Dimension(0)).ConvertToPx(), 0);
        bool hasNextMargin = !LessOrEqual(swiperLayoutProperty->GetNextMarginValue(Dimension(0)).ConvertToPx(), 0);
        auto isSingleCase =
            !hasMinSize && !hasPrevMargin && !hasNextMargin &&
            (swiperLayoutProperty->GetDisplayCountValue(0) == 1 ||
                (!swiperLayoutProperty->GetDisplayCount().has_value() && SwiperUtils::IsStretch(swiperLayoutProperty)));
        if (isSingleCase && !mainSizeIsDefined_) {
            endMainPos = itemPosition_.begin()->second.endPos - itemPosition_.begin()->second.startPos;
            if (measured_) {
                endMainPos += currentOffset_;
            }
            endMainPos_ = endMainPos;
        }
        if ((currentIndex >= 0 && currentIndex < (totalItemCount_ - 1)) || isLoop_) {
            currentEndPos += spaceWidth_;
        }
        // reach the valid target index
        if (targetIndex_ && currentIndex >= targetIndex_.value()) {
            endMainPos = targetIsSameWithStartFlag_ ? endMainPos_ : currentStartPos + contentMainSize_;
            targetIndex_.reset();
        }
        if (static_cast<int32_t>(itemPosition_.size()) >= totalItemCount_) {
            break;
        }
    } while (LessNotEqual(currentEndPos, nextMargin_ != 0.0f ? endMainPos + nextMargin_ + spaceWidth_ : endMainPos) ||
             (targetIndex_ && currentIndex < targetIndex_.value()));

    if (overScrollFeature_ && canOverScroll_) {
        return;
    }

    // adjust offset.
    if (LessNotEqual(currentEndPos, endMainPos_) && !itemPosition_.empty()) {
        auto firstItemTop = itemPosition_.begin()->second.startPos;
        auto itemTotalSize = currentEndPos - firstItemTop;
        if (LessOrEqual(itemTotalSize, contentMainSize_) && (itemPosition_.begin()->first == 0)) {
            // all items size is less than swiper.
            if (!canOverScroll_) {
                currentOffset_ = firstItemTop;
                startMainPos_ = currentOffset_;
            }
            if (!mainSizeIsDefined_) {
                // adapt child size.
                contentMainSize_ = itemTotalSize;
            }
        } else {
            // adjust offset. If edgeEffect is SPRING, jump adjust to allow swiper scroll through boundary
            if (!canOverScroll_ || jumpIndex_.has_value()) {
                auto prevMarginMontage = Positive(prevMargin_) ? prevMargin_ + spaceWidth_ : 0.0f;
                auto nextMarginMontage = Positive(nextMargin_) ? nextMargin_ + spaceWidth_ : 0.0f;
                currentOffset_ = currentEndPos - contentMainSize_ + prevMarginMontage + nextMarginMontage;
            }
            startMainPos_ = currentEndPos - contentMainSize_;
            endMainPos_ = currentEndPos;
        }
    }

    // Mark inactive in wrapper.
    SetInactiveOnForward(layoutWrapper, axis);
}

void SwiperLayoutAlgorithm::SetInactive(
    LayoutWrapper* layoutWrapper, float startMainPos, float endMainPos, std::optional<int32_t> targetIndex)
{
    if (measured_) {
        // Theoretically, offset should be added in all cases to get correct results. Only apply in flex for now.
        startMainPos += currentOffset_;
        endMainPos += currentOffset_;
    }
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);
    std::list<int32_t> removeIndexes;
    for (auto pos = itemPosition_.rbegin(); pos != itemPosition_.rend(); ++pos) {
        if (targetIndex.has_value() && targetIndex.value() == pos->first) {
            continue;
        }
        if (LessOrEqual(
                pos->second.endPos, prevMargin_ != 0.0f ? startMainPos - prevMargin_ - spaceWidth_ : startMainPos) ||
            GreatOrEqual(
                pos->second.startPos, nextMargin_ != 0.0f ? endMainPos + nextMargin_ + spaceWidth_ : endMainPos)) {
            removeIndexes.emplace_back(pos->first);
        }
    }
    for (const auto& index : removeIndexes) {
        itemPosition_.erase(index);
    }
}

void SwiperLayoutAlgorithm::SetInactiveOnBackward(LayoutWrapper* layoutWrapper, Axis axis)
{
    std::list<int32_t> removeIndexes;
    auto displayCount = GetDisplayCount(layoutWrapper);
    for (auto pos = itemPosition_.rbegin(); pos != itemPosition_.rend(); ++pos) {
        auto startPos = pos->second.startPos;
        auto index = pos->first;

        if (swipeByGroup_) {
            auto startPageIndex = SwiperUtils::ComputePageIndex(index, displayCount);
            auto iter = itemPosition_.find(startPageIndex);
            if (iter != itemPosition_.end()) {
                startPos = iter->second.startPos;
            }
        }

        if (LessNotEqual(startPos, nextMargin_ != 0.0f ? endMainPos_ + nextMargin_ + spaceWidth_ : endMainPos_)) {
            break;
        }

        ResetOffscreenItemPosition(layoutWrapper, GetLoopIndex(index), false, axis);
        removeIndexes.emplace_back(index);
    }

    for (const auto& index : removeIndexes) {
        itemPosition_.erase(index);
    }
}

void SwiperLayoutAlgorithm::LayoutBackward(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis, int32_t endIndex, float endPos)
{
    float currentStartPos = endPos;
    float currentEndPos = 0.0f;
    float startMainPos = overScrollFeature_ ? std::min(endPos - contentMainSize_, startMainPos_) : startMainPos_;
    if (targetIndex_) {
        startMainPos = -Infinity<float>();
    }
    auto currentIndex = endIndex + 1;

    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);
    int32_t displayCount =
        swiperLayoutProperty->GetDisplayCount().has_value() ? swiperLayoutProperty->GetDisplayCount().value() : 1;
    do {
        currentEndPos = currentStartPos;
        auto result =
            LayoutBackwardItem(layoutWrapper, layoutConstraint, axis, currentIndex, currentEndPos, currentStartPos);
        if (!result) {
            break;
        }
        if (currentIndex > 0 || isLoop_) {
            currentStartPos = currentStartPos - spaceWidth_;
        }
        // reach the valid target index
        if (targetIndex_ && LessOrEqual(currentIndex, targetIndex_.value())) {
            startMainPos = currentStartPos;
            targetIndex_.reset();
        }
        if (static_cast<int32_t>(itemPosition_.size()) >= totalItemCount_ + displayCount - 1) {
            break;
        }
    } while (
        GreatNotEqual(currentStartPos, prevMargin_ != 0.0f ? startMainPos - prevMargin_ - spaceWidth_ : startMainPos));

    // adjust offset. If edgeEffect is SPRING, jump adjust to allow swiper scroll through boundary
    if (GreatNotEqual(currentStartPos, startMainPos_)) {
        if (!canOverScroll_ || jumpIndex_.has_value()) {
            currentOffset_ = currentStartPos;
            if (!mainSizeIsDefined_ && GetEndIndex() == totalItemCount_ - 1) {
                auto itemTotalSize = GetEndPosition() - currentStartPos;
                contentMainSize_ = std::min(contentMainSize_, itemTotalSize);
            }
        }
        endMainPos_ = currentStartPos + contentMainSize_;
        startMainPos_ = currentStartPos;
    }

    if (overScrollFeature_) {
        return;
    }

    // Mark inactive in wrapper.
    SetInactiveOnBackward(layoutWrapper, axis);
}

void SwiperLayoutAlgorithm::LayoutCustomAnimation(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto paddingOffset = padding.Offset();

    if (customAnimationToIndex_) {
        auto toIndexWrapper = layoutWrapper->GetOrCreateChildByIndex(customAnimationToIndex_.value());
        CHECK_NULL_VOID(toIndexWrapper);

        toIndexWrapper->GetGeometryNode()->SetMarginFrameOffset(paddingOffset);
        toIndexWrapper->Layout();
    }

    auto currentIndex = layoutProperty->GetIndex().value_or(0);
    auto currentIndexWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
    CHECK_NULL_VOID(currentIndexWrapper);

    currentIndexWrapper->GetGeometryNode()->SetMarginFrameOffset(paddingOffset);
    currentIndexWrapper->Layout();
}

void SwiperLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);

    if (swiperLayoutProperty->GetIsCustomAnimation().value_or(false)) {
        LayoutCustomAnimation(layoutWrapper);
        return;
    }

    auto axis = swiperLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto paddingOffset = padding.Offset();

    // layout items.
    std::set<int32_t> layoutIndexSet;
    for (auto& pos : itemPosition_) {
        layoutIndexSet.insert(GetLoopIndex(pos.first));
        pos.second.startPos -= currentOffset_;
        pos.second.endPos -= currentOffset_;
        LayoutItem(layoutWrapper, axis, paddingOffset, pos);
    }
    for (auto& pos : itemPositionInAnimation_) {
        if (layoutIndexSet.find(pos.first) == layoutIndexSet.end()) {
            LayoutItem(layoutWrapper, axis, paddingOffset, pos);
        }
    }

    // Layout swiper indicator
    if (swiperLayoutProperty->GetShowIndicatorValue(true)) {
        auto hostNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(hostNode);
        auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
        CHECK_NULL_VOID(swiperPattern);
        auto indicatorWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_INDICATOR_ETS_TAG);
        if (indicatorWrapper) {
            if (swiperLayoutProperty->GetIndicatorTypeValue(SwiperIndicatorType::DOT) == SwiperIndicatorType::DIGIT) {
                PlaceDigitChild(indicatorWrapper, swiperLayoutProperty);
            }
            indicatorWrapper->Layout();
        }
    }

    if (swiperLayoutProperty->GetDisplayArrowValue(false)) {
        auto hostNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(hostNode);
        auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
        CHECK_NULL_VOID(swiperPattern);

        if (swiperPattern->HasLeftButtonNode() && swiperPattern->HasRightButtonNode()) {
            auto leftArrowWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_ARROW_ETS_TAG);
            auto rightArrowWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_ARROW_ETS_TAG);
            if (leftArrowWrapper && (leftArrowWrapper->GetHostTag() == V2::SWIPER_LEFT_ARROW_ETS_TAG)) {
                ArrowLayout(layoutWrapper, leftArrowWrapper, padding);
            }
            if (rightArrowWrapper && (rightArrowWrapper->GetHostTag() == V2::SWIPER_RIGHT_ARROW_ETS_TAG)) {
                ArrowLayout(layoutWrapper, rightArrowWrapper, padding);
            }
        }
    }

    CaptureLayout(layoutWrapper);
}

void SwiperLayoutAlgorithm::LayoutItem(
    LayoutWrapper* layoutWrapper, Axis axis, OffsetF offset, std::pair<int32_t, SwiperItemInfo> pos)
{
    auto layoutIndex = GetLoopIndex(pos.first);
    if (swipeByGroup_ && layoutIndex >= realTotalCount_) {
        return;
    }

    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(layoutIndex);
    if (!IsNormalItem(wrapper)) {
        return;
    }

    float crossOffset = 0.0f;
    if (axis == Axis::VERTICAL) {
        offset += OffsetF(crossOffset, pos.second.startPos);
        if (!NearZero(prevMargin_)) {
            offset += OffsetF(crossOffset, prevMargin_ + spaceWidth_);
        }
    } else {
        offset += OffsetF(pos.second.startPos, crossOffset);
        if (!NearZero(prevMargin_)) {
            offset += OffsetF(prevMargin_ + spaceWidth_, crossOffset);
        }
    }
    wrapper->GetGeometryNode()->SetMarginFrameOffset(offset);
    wrapper->Layout();
}

void SwiperLayoutAlgorithm::CaptureLayout(LayoutWrapper* layoutWrapper)
{
    if (!hasCachedCapture_ || itemPosition_.empty()) {
        return;
    }
    auto hostNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(hostNode);
    auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto leftCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_CAPTURE_ETS_TAG);
    auto rightCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_CAPTURE_ETS_TAG);
    if (isCaptureReverse_) {
        leftCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_RIGHT_CAPTURE_ETS_TAG);
        rightCaptureWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_LEFT_CAPTURE_ETS_TAG);
    }
    CHECK_NULL_VOID(leftCaptureWrapper);
    CHECK_NULL_VOID(rightCaptureWrapper);
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto axis = swiperLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
    auto geometryNode = leftCaptureWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto leftCaptureSize = axis == Axis::VERTICAL ? geometryNode->GetMarginFrameSize().Height()
                                                  : geometryNode->GetMarginFrameSize().Width();
    auto leftPosition = itemPosition_.begin()->second.startPos - spaceWidth_ - leftCaptureSize;
    auto rightPosition = itemPosition_.rbegin()->second.endPos + spaceWidth_;

    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto leftOffset = padding.Offset();
    auto rightOffset = padding.Offset();
    auto deltaOffset = 0.0f;
    if (!NearZero(prevMargin_)) {
        deltaOffset = prevMargin_ + spaceWidth_;
    }
    if (axis == Axis::VERTICAL) {
        leftOffset += OffsetF(0.0f, leftPosition + deltaOffset);
        rightOffset += OffsetF(0.0f, rightPosition + deltaOffset);
    } else {
        leftOffset += OffsetF(leftPosition + deltaOffset, 0.0f);
        rightOffset += OffsetF(rightPosition + deltaOffset, 0.0f);
    }
    leftCaptureWrapper->GetGeometryNode()->SetMarginFrameOffset(leftOffset);
    leftCaptureWrapper->Layout();
    rightCaptureWrapper->GetGeometryNode()->SetMarginFrameOffset(rightOffset);
    rightCaptureWrapper->Layout();
}

void SwiperLayoutAlgorithm::PlaceDigitChild(
    const RefPtr<LayoutWrapper>& indicatorWrapper, const RefPtr<LayoutProperty>& layoutProperty)
{
    if (indicatorWrapper->GetTotalChildCount() != INDICATOR_HAS_CHILD) {
        return;
    }
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutProperty);
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto indicatorGeometryNode = indicatorWrapper->GetGeometryNode();
    CHECK_NULL_VOID(indicatorGeometryNode);
    auto indicatorWidth = INDICATOR_PADDING.ConvertToPx() * 2.0;
    auto indicatorHeight = 0.0f;
    for (auto&& child : indicatorWrapper->GetAllChildrenWithBuild()) {
        auto textGeometryNode = child->GetGeometryNode();
        CHECK_NULL_VOID(textGeometryNode);
        auto textFrameSize = textGeometryNode->GetFrameSize();
        indicatorWidth += textFrameSize.Width();
        if (indicatorHeight < textFrameSize.Height()) {
            indicatorHeight = textFrameSize.Height();
        }
    }

    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(swiperIndicatorTheme);
    if (LessNotEqual(indicatorHeight, swiperIndicatorTheme->GetIndicatorDigitHeight().ConvertToPx())) {
        indicatorHeight = swiperIndicatorTheme->GetIndicatorDigitHeight().ConvertToPx();
    }

    auto frameNode = indicatorWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto indicatorlayoutProperty = frameNode->GetLayoutProperty<SwiperIndicatorLayoutProperty>();
    CHECK_NULL_VOID(indicatorlayoutProperty);

    auto currentOffset = SwiperIndicatorUtils::CalcIndicatrFrameOffSet(
        swiperLayoutProperty, indicatorlayoutProperty, indicatorWidth, indicatorHeight);

    if (swiperLayoutProperty->GetDisplayArrowValue(false) && !swiperLayoutProperty->GetIsSidebarMiddleValue(false) &&
        HasCustomIndicatorOffset(indicatorWrapper)) {
        useCustomIndicatorOffset = true;
        auto indicatorOffset = CalculateCustomOffset(indicatorWrapper, currentOffset);
        if (swiperLayoutProperty->GetDirectionValue(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
            currentOffset.SetX(indicatorOffset.GetX());
        } else {
            currentOffset.SetY(indicatorOffset.GetY());
        }
    }
    if (swiperLayoutProperty->GetDirectionValue(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
        auto top = indicatorlayoutProperty->GetTop();
        auto bottom = indicatorlayoutProperty->GetBottom();
        if ((!top.has_value() || NearZero(top.value().Value())) &&
            (!bottom.has_value() || NearZero(bottom.value().Value()))) {
            auto themeHeight = swiperIndicatorTheme->GetIndicatorDigitHeight().ConvertToPx();
            auto dightPadding = 0.0;
            if (themeHeight > indicatorHeight) {
                dightPadding = (themeHeight - indicatorHeight) / 2;
            }
            auto dightVerPadding = swiperIndicatorTheme->GetIndicatorDigitVerticalPadding().ConvertToPx();
            currentOffset.SetY(currentOffset.GetY() - dightVerPadding + dightPadding);
        }
    }

    indicatorGeometryNode->SetMarginFrameOffset(currentOffset);
}

RefPtr<LayoutWrapper> SwiperLayoutAlgorithm::GetNodeLayoutWrapperByTag(
    LayoutWrapper* layoutWrapper, const std::string& tagName) const
{
    CHECK_NULL_RETURN(layoutWrapper, nullptr);
    auto hostNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(hostNode, nullptr);
    auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, nullptr);
    RefPtr<LayoutWrapper> nodeWrapper = nullptr;
    int32_t totalChildCount = layoutWrapper->GetTotalChildCount();
    if (totalChildCount == 0) {
        return nullptr;
    }
    int32_t lastChildIndex = totalChildCount - 1;
    int32_t endLoopChildIndex = lastChildIndex - SWIPER_HAS_CHILD;
    for (int32_t index = lastChildIndex; index > endLoopChildIndex && index >= 0; index--) {
        nodeWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (nodeWrapper && (nodeWrapper->GetHostTag() == tagName)) {
            return nodeWrapper;
        }
    }

    return nullptr;
}

bool SwiperLayoutAlgorithm::HasCustomIndicatorOffset(const RefPtr<LayoutWrapper>& indicatorWrapper)
{
    auto frameNode = indicatorWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto indicatorLayoutProperty = frameNode->GetLayoutProperty<SwiperIndicatorLayoutProperty>();
    CHECK_NULL_RETURN(indicatorLayoutProperty, false);
    return indicatorLayoutProperty->GetLeft().has_value() || indicatorLayoutProperty->GetRight().has_value() ||
           indicatorLayoutProperty->GetTop().has_value() || indicatorLayoutProperty->GetBottom().has_value();
}

const OffsetF SwiperLayoutAlgorithm::CalculateCustomOffset(
    const RefPtr<LayoutWrapper>& indicatorWrapper, const OffsetF& currentOffset)
{
    OffsetF indicatorOffset(currentOffset.GetX(), currentOffset.GetY());
    auto indicatorGeometryNode = indicatorWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(indicatorGeometryNode, indicatorOffset);
    SizeF indicatorFrameSize = indicatorGeometryNode->GetFrameSize();
    auto indicatorLayoutProperty =
        AceType::DynamicCast<SwiperIndicatorLayoutProperty>(indicatorWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(indicatorLayoutProperty, indicatorOffset);

    auto swiperNode = DynamicCast<FrameNode>(indicatorWrapper->GetHostNode()->GetParent());
    CHECK_NULL_RETURN(swiperNode, indicatorOffset);
    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(swiperLayoutProperty, indicatorOffset);
    SizeF swiperFrameSize = swiperNode->GetGeometryNode()->GetFrameSize();
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, indicatorOffset);
    auto arrowNode = DynamicCast<FrameNode>(
        swiperNode->GetChildAtIndex(swiperNode->GetChildIndexById(swiperPattern->GetLeftButtonId())));
    CHECK_NULL_RETURN(arrowNode, indicatorOffset);
    SizeF arrowFrameSize = arrowNode->GetGeometryNode()->GetFrameSize();
    auto left = indicatorLayoutProperty->GetLeft();
    auto right = indicatorLayoutProperty->GetRight();
    auto top = indicatorLayoutProperty->GetTop();
    auto bottom = indicatorLayoutProperty->GetBottom();
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, indicatorOffset);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_RETURN(swiperIndicatorTheme, indicatorOffset);
    auto indicatorPadding = swiperIndicatorTheme->GetIndicatorDigitPadding().ConvertToPx();

    if (swiperLayoutProperty->GetDirectionValue(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
        auto horizonOffset = arrowFrameSize.Width() + indicatorPadding;
        auto offset = 0.0;
        if (left.has_value()) {
            offset = currentOffset.GetX() + horizonOffset;
            indicatorOffset.SetX(
                std::min(offset, swiperFrameSize.Width() - indicatorFrameSize.Width() - horizonOffset));
        } else if (right.has_value()) {
            offset = currentOffset.GetX() - horizonOffset;
            indicatorOffset.SetX(std::max(offset, horizonOffset));
        }
    } else {
        auto verticleOffset = arrowFrameSize.Height() + indicatorPadding;
        auto offset = 0.0;
        if (top.has_value()) {
            offset = currentOffset.GetY() + verticleOffset;
            indicatorOffset.SetY(
                std::min(offset, swiperFrameSize.Height() - indicatorFrameSize.Height() - verticleOffset));
        } else if (bottom.has_value()) {
            offset = currentOffset.GetY() - verticleOffset;
            indicatorOffset.SetY(std::max(offset, verticleOffset));
        }
    }
    return indicatorOffset;
}

void SwiperLayoutAlgorithm::MeasureArrow(
    const RefPtr<LayoutWrapper>& arrowWrapper, const RefPtr<LayoutProperty>& layoutProperty) const
{
    CHECK_NULL_VOID(arrowWrapper);
    CHECK_NULL_VOID(layoutProperty);
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutProperty);
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto arrowGeometryNode = arrowWrapper->GetGeometryNode();
    CHECK_NULL_VOID(arrowGeometryNode);

    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(swiperIndicatorTheme);

    arrowGeometryNode->SetFrameSize(
        SizeF { static_cast<float>(
                    swiperLayoutProperty->GetBackgroundSizeValue(swiperIndicatorTheme->GetBigArrowBackgroundSize())
                        .ConvertToPx()),
            static_cast<float>(
                swiperLayoutProperty->GetBackgroundSizeValue(swiperIndicatorTheme->GetBigArrowBackgroundSize())
                    .ConvertToPx()) });
    auto indicatorLayoutConstraint = swiperLayoutProperty->CreateChildConstraint();
    arrowWrapper->Measure(indicatorLayoutConstraint);
}

void SwiperLayoutAlgorithm::ArrowLayout(
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& arrowWrapper, const PaddingPropertyF padding) const
{
    CHECK_NULL_VOID(layoutWrapper);
    CHECK_NULL_VOID(arrowWrapper);
    auto swiperLayoutProperty = AceType::DynamicCast<SwiperLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto axis = swiperLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
    auto indicatorType = swiperLayoutProperty->GetIndicatorTypeValue(SwiperIndicatorType::DOT);
    auto arrowGeometryNode = arrowWrapper->GetGeometryNode();
    CHECK_NULL_VOID(arrowGeometryNode);
    auto arrowFrameSize = arrowGeometryNode->GetFrameSize();
    auto layoutGeometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(layoutGeometryNode);
    auto swiperFrameSize = layoutGeometryNode->GetFrameSize();
    auto isShowIndicatorArrow =
        (!swiperLayoutProperty->GetIsSidebarMiddleValue(false) && swiperLayoutProperty->GetShowIndicatorValue(true));
    SizeF indicatorFrameSize;
    RectF indicatorFrameRect;
    auto normalArrowMargin = 0.0f;
    if (isShowIndicatorArrow) {
        auto indicatorWrapper = GetNodeLayoutWrapperByTag(layoutWrapper, V2::SWIPER_INDICATOR_ETS_TAG);
        CHECK_NULL_VOID(indicatorWrapper);
        auto indicatorGeometry = indicatorWrapper->GetGeometryNode();
        CHECK_NULL_VOID(indicatorGeometry);
        indicatorFrameSize = indicatorGeometry->GetFrameSize();
        indicatorFrameRect = indicatorGeometry->GetFrameRect();
        if (indicatorType == SwiperIndicatorType::DOT) {
            auto hostNode = layoutWrapper->GetHostNode();
            CHECK_NULL_VOID(hostNode);
            auto swiperPattern = hostNode->GetPattern<SwiperPattern>();
            CHECK_NULL_VOID(swiperPattern);
            auto itemCount = swiperPattern->TotalCount();
            auto indicatorNode = indicatorWrapper->GetHostNode();
            CHECK_NULL_VOID(indicatorNode);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
            CHECK_NULL_VOID(theme);
            auto indicatorPaintProperty = indicatorNode->GetPaintProperty<DotIndicatorPaintProperty>();
            CHECK_NULL_VOID(indicatorPaintProperty);
            auto itemWidth =
                static_cast<float>(indicatorPaintProperty->GetItemWidthValue(theme->GetSize()).ConvertToPx());
            auto selectedItemWidth =
                static_cast<float>(indicatorPaintProperty->GetSelectedItemWidthValue(theme->GetSize()).ConvertToPx());
            auto indicatorPadding = static_cast<float>(theme->GetIndicatorDotPadding().ConvertToPx());
            auto allPointDiameterSum = itemWidth * static_cast<float>(itemCount + 1);
            if (indicatorPaintProperty->GetIsCustomSizeValue(false)) {
                allPointDiameterSum = itemWidth * static_cast<float>(itemCount - 1) + selectedItemWidth;
            }
            auto allPointSpaceSum =
                static_cast<float>(theme->GetIndicatorDotItemSpace().ConvertToPx()) * (itemCount - 1);
            auto indicatorWidth = indicatorPadding + allPointDiameterSum + allPointSpaceSum + indicatorPadding;
            normalArrowMargin = ((axis == Axis::HORIZONTAL ? indicatorFrameSize.Width() : indicatorFrameSize.Height()) -
                                    indicatorWidth) *
                                0.5f;
        }
    }
    auto isLeftArrow = arrowWrapper->GetHostTag() == V2::SWIPER_LEFT_ARROW_ETS_TAG;
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(swiperIndicatorTheme);
    OffsetF arrowOffset(0.0f, 0.0f);
    float startPoint = 0.0f;
    if (axis == Axis::HORIZONTAL && isShowIndicatorArrow) {
        auto indicatorPadding = indicatorType == SwiperIndicatorType::DIGIT
                                    ? swiperIndicatorTheme->GetIndicatorDigitPadding().ConvertToPx()
                                    : swiperIndicatorTheme->GetIndicatorDotPadding().ConvertToPx();
        if (useCustomIndicatorOffset && indicatorType == SwiperIndicatorType::DIGIT) {
            startPoint = isLeftArrow ? (indicatorFrameRect.Left() - arrowFrameSize.Width() - indicatorPadding)
                                     : (indicatorFrameRect.Right() + indicatorPadding);
        } else {
            startPoint =
                isLeftArrow
                    ? (indicatorFrameRect.Left() - arrowFrameSize.Width() -
                          swiperIndicatorTheme->GetArrowScale().ConvertToPx() + indicatorPadding + normalArrowMargin)
                    : (indicatorFrameRect.Right() + swiperIndicatorTheme->GetArrowScale().ConvertToPx() -
                          indicatorPadding - normalArrowMargin);
        }
        arrowOffset.SetX(startPoint);
        if (isLeftArrow && !NonNegative(arrowOffset.GetX() - padding.left.value_or(0.0f))) {
            arrowOffset.SetX(padding.left.value_or(0.0f));
        }
        if (GreatOrEqual(
                arrowOffset.GetX() + arrowFrameSize.Width(), swiperFrameSize.Width() - padding.right.value_or(0.0f))) {
            arrowOffset.SetX(swiperFrameSize.Width() - arrowFrameSize.Width() - padding.right.value_or(0.0f));
        }
        arrowOffset.SetY(indicatorFrameRect.Top() + (indicatorFrameSize.Height() - arrowFrameSize.Height()) * 0.5f);
    } else if (axis == Axis::HORIZONTAL && !isShowIndicatorArrow) {
        startPoint = isLeftArrow
                         ? swiperIndicatorTheme->GetArrowHorizontalMargin().ConvertToPx() + padding.left.value_or(0.0f)
                         : (swiperFrameSize.Width() - padding.right.value_or(0.0f) - arrowFrameSize.Width() -
                               swiperIndicatorTheme->GetArrowHorizontalMargin().ConvertToPx());
        arrowOffset.SetX(startPoint);
        arrowOffset.SetY((swiperFrameSize.Height() - padding.top.value_or(0.0f) - padding.bottom.value_or(0.0f) -
                             arrowFrameSize.Height()) *
                             0.5f +
                         padding.top.value_or(0.0f));
    } else if (axis != Axis::HORIZONTAL && isShowIndicatorArrow) {
        auto indicatorPadding = indicatorType == SwiperIndicatorType::DIGIT
                                    ? swiperIndicatorTheme->GetIndicatorDigitPadding().ConvertToPx()
                                    : swiperIndicatorTheme->GetIndicatorDotPadding().ConvertToPx();
        if (useCustomIndicatorOffset && indicatorType == SwiperIndicatorType::DIGIT) {
            startPoint = isLeftArrow ? (indicatorFrameRect.Top() - arrowFrameSize.Height() -
                                           padding.top.value_or(0.0f) - indicatorPadding)
                                     : (indicatorFrameRect.Bottom() + padding.bottom.value_or(0.0f) + indicatorPadding);
        } else {
            startPoint =
                isLeftArrow
                    ? (indicatorFrameRect.Top() - arrowFrameSize.Height() - padding.top.value_or(0.0f) -
                          swiperIndicatorTheme->GetArrowScale().ConvertToPx() + indicatorPadding + normalArrowMargin)
                    : (indicatorFrameRect.Bottom() + padding.bottom.value_or(0.0f) +
                          swiperIndicatorTheme->GetArrowScale().ConvertToPx() - indicatorPadding - normalArrowMargin);
        }
        arrowOffset.SetX(indicatorFrameRect.Left() + (indicatorFrameSize.Width() - arrowFrameSize.Width()) * 0.5f);
        arrowOffset.SetY(startPoint);
        if (isLeftArrow && !NonNegative(arrowOffset.GetY() - padding.top.value_or(0.0f))) {
            arrowOffset.SetY(padding.top.value_or(0.0f));
        }
        if (GreatOrEqual(arrowOffset.GetY() + arrowFrameSize.Height(),
                swiperFrameSize.Height() - padding.bottom.value_or(0.0f))) {
            arrowOffset.SetY(swiperFrameSize.Height() - arrowFrameSize.Height() - padding.bottom.value_or(0.0f));
        }
    } else {
        startPoint = isLeftArrow
                         ? swiperIndicatorTheme->GetArrowVerticalMargin().ConvertToPx() + padding.top.value_or(0.0f)
                         : (swiperFrameSize.Height() - arrowFrameSize.Width() - padding.bottom.value_or(0.0f) -
                               swiperIndicatorTheme->GetArrowVerticalMargin().ConvertToPx());
        arrowOffset.SetX(padding.left.value_or(0.0f) + (swiperFrameSize.Width() - padding.left.value_or(0.0f) -
                                                           padding.right.value_or(0.0f) - arrowFrameSize.Width()) *
                                                           0.5f);
        arrowOffset.SetY(startPoint);
    }
    arrowGeometryNode->SetMarginFrameOffset(arrowOffset);
    arrowWrapper->Layout();
}

void SwiperLayoutAlgorithm::ResetOffscreenItemPosition(
    LayoutWrapper* layoutWrapper, int32_t index, bool isForward, Axis axis) const
{
    auto swiperGeometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(swiperGeometryNode);
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(childWrapper);

    if (childWrapper->GetHostTag() == V2::SWIPER_INDICATOR_ETS_TAG ||
        childWrapper->GetHostTag() == V2::SWIPER_LEFT_ARROW_ETS_TAG ||
        childWrapper->GetHostTag() == V2::SWIPER_RIGHT_ARROW_ETS_TAG) {
        return;
    }

    auto childGeometryNode = childWrapper->GetGeometryNode();
    CHECK_NULL_VOID(childGeometryNode);
    auto swiperFrameRect = swiperGeometryNode->GetFrameRect();
    auto childFrameRect = childGeometryNode->GetFrameRect();

    OffsetF offset(0.0f, 0.0f);
    if (axis == Axis::HORIZONTAL) {
        offset.SetX(isForward ? -childFrameRect.Width() : swiperFrameRect.Width());
    } else {
        offset.SetY(isForward ? -childFrameRect.Height() : swiperFrameRect.Height());
    }

    childGeometryNode->SetMarginFrameOffset(offset);
    childWrapper->Layout();
}

bool SwiperLayoutAlgorithm::IsNormalItem(const RefPtr<LayoutWrapper>& wrapper) const
{
    CHECK_NULL_RETURN(wrapper, false);
    auto tag = wrapper->GetHostTag();
    if (tag == V2::SWIPER_INDICATOR_ETS_TAG || tag == V2::SWIPER_LEFT_ARROW_ETS_TAG ||
        tag == V2::SWIPER_RIGHT_ARROW_ETS_TAG || tag == V2::SWIPER_LEFT_CAPTURE_ETS_TAG ||
        tag == V2::SWIPER_RIGHT_CAPTURE_ETS_TAG) {
        return false;
    }
    return true;
}
} // namespace OHOS::Ace::NG

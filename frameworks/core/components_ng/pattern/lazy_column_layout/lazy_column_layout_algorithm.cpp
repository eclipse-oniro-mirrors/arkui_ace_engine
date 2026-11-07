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

#include "core/components_ng/pattern/lazy_column_layout/lazy_column_layout_algorithm.h"

#include <cstddef>
#include <cstdint>
#include <iterator>

#include "base/log/log_wrapper.h"
#include "base/utils/time_util.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_pattern.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/lazy_layout/prepared_layout_geometry.h"
#include "core/components_ng/pattern/lazy_layout/header_footer_utils.h"
#include "core/components_ng/pattern/lazy_column_layout/lazy_column_layout_pattern.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {

namespace {
float GetVerticalMargin(const RefPtr<LayoutWrapper>& wrapper, bool isStart)
{
    CHECK_NULL_RETURN(wrapper, 0.0f);
    auto geometryNode = wrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    const auto& margin = geometryNode->GetMargin();
    CHECK_NULL_RETURN(margin, 0.0f);
    return isStart ? margin->top.value_or(0.0f) : margin->bottom.value_or(0.0f);
}
} // namespace

std::optional<LazyPremeasureResult> LazyColumnLayoutAlgorithm::Premeasure(LayoutWrapper* layoutWrapper)
{
    LazyColumnLayoutAlgorithm measure(layoutInfo_);
    measure.header_ = header_;
    measure.footer_ = footer_;
    return measure.MeasureBoundaryRange(layoutWrapper);
}

std::optional<LazyPremeasureResult> LazyColumnLayoutAlgorithm::MeasureBoundaryRange(LayoutWrapper* layoutWrapper)
{
    OptionalSizeF ideal;
    CHECK_NULL_RETURN(PrepareBoundaryMeasure(layoutWrapper, ideal), std::nullopt);
    const float oldTotal = layoutInfo_->totalMainSize_;
    const float oldHeader = layoutInfo_->headerMainSize_;
    prevBodyMainSize_ = std::max(0.0f, oldTotal - oldHeader - layoutInfo_->footerMainSize_);
    const auto oldAdjust = layoutInfo_->adjustOffset_;
    const auto anchor = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    const std::optional<float> oldAnchor = anchor == layoutInfo_->posMap_.end()
        ? std::nullopt : std::optional<float>(anchor->second.startPos + oldHeader);
    MeasureHeader(layoutWrapper);
    MeasureFooter(layoutWrapper);
    layoutInfo_->SetSpace(space_);
    layoutInfo_->SetTotalItemCount(totalItemCount_);
    const auto& reference = layoutWrapper->GetLayoutProperty()->GetContentLayoutConstraint()->viewPosRef.value();
    const auto range = LazyLayoutUtils::CalculateViewRange(reference,
        prevBodyMainSize_ + layoutInfo_->headerMainSize_ + layoutInfo_->footerMainSize_);
    startPos_ = range.start - layoutInfo_->headerMainSize_;
    endPos_ = range.end - layoutInfo_->headerMainSize_;
    auto result = MeasureBoundaryItems(layoutWrapper, reference.referenceEdge == ReferenceEdge::START);
    const bool accepted = FinishBoundaryMeasure(result, oldTotal, oldHeader, oldAnchor);
    CapturePreparedGeometry(*result.geometry);
    result.unsupported = result.unsupported || !accepted;
    layoutInfo_->adjustOffset_ = oldAdjust;
    result.complete = result.complete && !result.unsupported;
    result.reachedOppositeEdge = result.reachedOppositeEdge && result.complete;
    SetFrameSize(layoutWrapper, ideal);
    result.marginMainSize = layoutWrapper->GetGeometryNode()->GetMarginFrameSize().Height();
    return result;
}

bool LazyColumnLayoutAlgorithm::PrepareBoundaryMeasure(LayoutWrapper* layoutWrapper, OptionalSizeF& ideal)
{
    CHECK_NULL_RETURN(layoutWrapper, false);
    auto property = AceType::DynamicCast<LazyColumnLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(property && layoutInfo_ && property->GetContentLayoutConstraint(), false);
    const auto& constraint = property->GetContentLayoutConstraint().value();
    CHECK_NULL_RETURN(constraint.viewPosRef, false);
    knownMainSize_ = constraint.viewPosRef->knownMainSize;
    if (constraint.selfIdealSize.Height() || Positive(constraint.minSize.Height()) ||
        !GreaterOrEqualToInfinity(constraint.maxSize.Height())) {
        return false;
    }
    UpdateHeaderFooterIndexes(layoutWrapper);
    totalItemCount_ = CalculateItemCount(layoutWrapper);
    ideal = ResolveContentIdealSize(constraint, property);
    UpdateAttribute(property, constraint);
    UpdateChildConstraint(property, ideal);
    return true;
}

LazyPremeasureResult LazyColumnLayoutAlgorithm::MeasureBoundaryItems(
    LayoutWrapper* layoutWrapper, bool forward)
{
    LazyPremeasureResult result;
    result.geometry = std::make_shared<PreparedLayoutGeometry>();
    result.geometry->fromEnd = !forward;
    const float limit = forward ? endPos_ : startPos_;
    float cursor = forward ? 0.0f : prevBodyMainSize_;
    int32_t index = forward ? 0 : totalItemCount_ - 1;
    bool crossedChildren = true;
    result.complete = true;
    while (index >= 0 && index < totalItemCount_ &&
        (forward ? LessNotEqual(cursor, limit) : GreatNotEqual(cursor, limit))) {
        const bool displayed = (index >= layoutInfo_->startIndex_ && index <= layoutInfo_->endIndex_) ||
            (index >= layoutInfo_->cachedStartIndex_ && index <= layoutInfo_->cachedEndIndex_);
        auto child = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(index), false, !displayed);
        if (!child) {
            result.complete = false;
            break;
        }
        auto item = MeasureBoundaryItem(child, cursor, forward);
        const bool nested = NeedLazyLayout(child);
        if (!item) {
            result.unsupported = true;
            result.complete = false;
            break;
        }
        result.unsupported = item->unsupported;
        if ((!item->complete && !nested) ||
            !std::isfinite(item->marginMainSize) || Negative(item->marginMainSize)) {
            result.complete = false;
            break;
        }
        // An incomplete nested pass still returns its committed children and their compensation.
        result.complete = item->complete && !result.unsupported;
        crossedChildren = crossedChildren && item->reachedOppositeEdge;
        if (index == layoutInfo_->startIndex_) {
            result.adjust.start = item->adjust.start;
        }
        cursor = CommitBoundaryItem(index, cursor, forward, item.value());
        RecordBoundaryItemGeometry(index, item.value(), result);
        index += forward ? 1 : -1;
        if (!result.complete) {
            break;
        }
    }
    result.reachedOppositeEdge = result.complete && crossedChildren && (index < 0 || index >= totalItemCount_);
    return result;
}

void LazyColumnLayoutAlgorithm::RecordBoundaryItemGeometry(
    int32_t index, const LazyPremeasureResult& item, LazyPremeasureResult& result)
{
    result.geometry->positions.try_emplace(index);
    if (item.geometry) {
        result.geometry->children[index] = item.geometry;
    }
}

float LazyColumnLayoutAlgorithm::CommitBoundaryItem(
    int32_t index, float cursor, bool forward, const LazyPremeasureResult& item)
{
    // Nested compensation belongs to the displayed anchor, not the requested measurement boundary.
    const float start = forward ? cursor : cursor - item.marginMainSize;
    layoutInfo_->SetPosMap(index, { start, start + item.marginMainSize });
    return forward ? start + item.marginMainSize + space_ : start - space_;
}

std::optional<LazyPremeasureResult> LazyColumnLayoutAlgorithm::MeasureBoundaryItem(
    const RefPtr<LayoutWrapper>& child, float cursor, bool forward)
{
    if (NeedLazyLayout(child)) {
        auto constraint = childLayoutConstraint_;
        constraint.viewPosRef = ViewPosReference {
            .viewPosStart = startPos_, .viewPosEnd = endPos_, .referencePos = cursor,
            .referenceEdge = forward ? ReferenceEdge::START : ReferenceEdge::END, .axis = Axis::VERTICAL,
        };
        if (totalItemCount_ == 1) {
            constraint.viewPosRef->knownMainSize = GetKnownBodySize();
        }
        return LazyLayoutUtils::Premeasure(child, constraint);
    }
    child->Measure(childLayoutConstraint_);
    CHECK_NULL_RETURN(LazyLayoutUtils::IsChildMeasureComplete(child), LazyPremeasureResult());
    return LazyPremeasureResult { true, true, child->GetGeometryNode()->GetMarginFrameSize().Height(), {} };
}

void LazyColumnLayoutAlgorithm::CapturePreparedGeometry(PreparedLayoutGeometry& geometry) const
{
    geometry.type = PreparedLayoutType::COLUMN;
    geometry.itemCount = totalItemCount_;
    geometry.bodyMainSize = layoutInfo_->totalMainSize_ - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_;
    geometry.headerMainSize = layoutInfo_->headerMainSize_;
    geometry.footerMainSize = layoutInfo_->footerMainSize_;
    geometry.space = space_;
    geometry.estimatedItemSize = layoutInfo_->estimateItemSize_;
    const float origin = geometry.fromEnd ? geometry.bodyMainSize : 0.0f;
    for (auto& [index, item] : geometry.positions) {
        const auto& position = layoutInfo_->posMap_.at(index);
        item = { position.startPos - origin, position.endPos - origin };
    }
}

bool LazyColumnLayoutAlgorithm::FinishBoundaryMeasure(LazyPremeasureResult& result, float oldTotal, float oldHeader,
    const std::optional<float>& oldAnchor)
{
    layoutInfo_->totalMainSize_ = prevBodyMainSize_;
    bool accepted = true;
    if (totalItemCount_ > 0 || knownMainSize_) {
        accepted = layoutInfo_->UpdatePosMap(prevBodyMainSize_, GetKnownBodySize());
    } else {
        layoutInfo_->totalMainSize_ = 0.0f;
    }
    totalMainSize_ = layoutInfo_->totalMainSize_ + layoutInfo_->headerMainSize_ + layoutInfo_->footerMainSize_;
    layoutInfo_->totalMainSize_ = totalMainSize_;
    layoutInfo_->premeasuredMainSize_ = totalMainSize_;
    const float nestedAnchorAdjust = result.adjust.start;
    result.adjust.start = 0.0f;
    const auto anchor = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    if (oldAnchor && anchor != layoutInfo_->posMap_.end()) {
        result.adjust.start = anchor->second.startPos + layoutInfo_->headerMainSize_ - oldAnchor.value() +
            nestedAnchorAdjust;
    }
    RebasePremeasureWindow(oldAnchor ? result.adjust.start - layoutInfo_->headerMainSize_ + oldHeader : 0.0f);
    result.adjust.end = totalMainSize_ - oldTotal - result.adjust.start;
    return accepted;
}

std::optional<float> LazyColumnLayoutAlgorithm::GetKnownBodySize() const
{
    if (!knownMainSize_) {
        return std::nullopt;
    }
    return knownMainSize_.value() - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_;
}

AdjustOffset LazyColumnLayoutAlgorithm::PrepareKnownBodyMeasure()
{
    const auto knownBody = GetKnownBodySize();
    CHECK_NULL_RETURN(knownBody, AdjustOffset());
    layoutInfo_->SetSpace(space_);
    layoutInfo_->SetTotalItemCount(totalItemCount_);
    CHECK_NULL_RETURN(layoutInfo_->ApplyKnownBodySize(prevBodyMainSize_, knownBody.value()), AdjustOffset());
    const auto adjust = layoutInfo_->adjustOffset_;
    prevBodyMainSize_ = layoutInfo_->totalMainSize_;
    totalMainSize_ = prevBodyMainSize_ + layoutInfo_->headerMainSize_ + layoutInfo_->footerMainSize_;
    RebasePremeasureWindow(adjust.start);
    layoutInfo_->adjustOffset_ = {};
    return hadMeasuredItems_ ? adjust : AdjustOffset();
}

void LazyColumnLayoutAlgorithm::RebasePremeasureWindow(float bodyAnchorAdjust)
{
    // Retain the same displayed/cached indexes; only their body-coordinate bookkeeping changes.
    const float bodyEnd = totalMainSize_ - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_;
    auto first = layoutInfo_->posMap_.find(layoutInfo_->layoutedStartIndex_);
    if (layoutInfo_->layoutedStartIndex_ >= totalItemCount_) {
        layoutInfo_->layoutedStart_ = bodyEnd;
    } else if (first != layoutInfo_->posMap_.end()) {
        layoutInfo_->layoutedStart_ = first->first == 0 ? 0.0f : first->second.startPos - space_;
    }
    auto last = layoutInfo_->posMap_.find(layoutInfo_->layoutedEndIndex_);
    if (layoutInfo_->layoutedEndIndex_ >= totalItemCount_ - 1) {
        layoutInfo_->layoutedEnd_ = bodyEnd;
    } else if (last != layoutInfo_->posMap_.end()) {
        layoutInfo_->layoutedEnd_ = last->second.endPos + space_;
    }
    layoutInfo_->cacheStartPos_ += bodyAnchorAdjust;
    layoutInfo_->cacheEndPos_ += bodyAnchorAdjust;
}

void LazyColumnLayoutAlgorithm::ShiftLayoutWindow(float delta)
{
    if (NearZero(delta)) {
        return;
    }
    startPos_ += delta;
    endPos_ += delta;
    cacheStartPos_ += delta;
    cacheEndPos_ += delta;
}

void LazyColumnLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper && layoutInfo_);
    auto layoutProperty = AceType::DynamicCast<LazyColumnLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto hostNode = layoutWrapper->GetHostNode();
    auto pattern = hostNode ? hostNode->GetPattern<LazyLayoutPattern>() : nullptr;
    if (LazyLayoutUtils::MeasurePreparedBoundary(AceType::RawPtr(pattern))) {
        return;
    }
    if (pattern) {
        pattern->NotifyParentOnStickyHeaderChange();
    }
    auto contentConstraint = layoutProperty->GetContentLayoutConstraint().value_or(LayoutConstraintF());
    knownMainSize_ = contentConstraint.viewPosRef ? contentConstraint.viewPosRef->knownMainSize : std::nullopt;
    auto contentIdealSize = ResolveContentIdealSize(contentConstraint, layoutProperty);
    UpdateHeaderFooterIndexes(layoutWrapper);
    totalItemCount_ = CalculateItemCount(layoutWrapper);
    CaptureFrameBaseline();
    UpdateAttribute(layoutProperty, contentConstraint);
    UpdateChildConstraint(layoutProperty, contentIdealSize);

    // Header first: UpdatePosReference needs this frame's header to convert the viewport to body coords.
    MeasureHeader(layoutWrapper);
    if (knownMainSize_) {
        MeasureFooter(layoutWrapper);
    }
    const auto knownAdjust = PrepareKnownBodyMeasure();
    UpdatePosReference(layoutWrapper, contentConstraint.viewPosRef);
    if (!knownMainSize_) {
        MeasureFooter(layoutWrapper);
    }

    ComposeChildStickyInsets(layoutWrapper);

    // Section extent was needed above for backward-anchor conversion; item-window math below is body-local.
    totalMainSize_ = prevBodyMainSize_;

    MeasureItemsByMode(layoutWrapper, layoutProperty, contentIdealSize);

    // Body-local: totalMainSize_ holds the body extent here; fold the edge slots into the section total.
    totalMainSize_ += layoutInfo_->headerMainSize_ + layoutInfo_->footerMainSize_;
    layoutInfo_->totalMainSize_ = totalMainSize_;
    layoutInfo_->adjustOffset_.start += knownAdjust.start;
    layoutInfo_->adjustOffset_.end += knownAdjust.end;

    SetFrameSize(layoutWrapper, contentIdealSize);
}

void LazyColumnLayoutAlgorithm::MeasureItemsByMode(LayoutWrapper* layoutWrapper,
    const RefPtr<LazyColumnLayoutProperty>& layoutProperty, const OptionalSizeF& contentIdealSize)
{
    const bool updateKnownSize = knownMainSize_ && (totalItemCount_ == 0 || isEstimatePass_ ||
        (!layoutInfo_->deadline_ && (needAllLayout_ || needSkipLayout_)));
    if (totalItemCount_ == 0) {
        layoutInfo_->SetTotalItemCount(0);
        totalMainSize_ = 0.0f;
    } else if (isEstimatePass_) {
        MeasureEstimateItems(layoutWrapper);
    } else if (layoutInfo_->deadline_) {
        MeasurePredictItems(layoutWrapper, layoutProperty, contentIdealSize);
    } else if (needAllLayout_) {
        MeasureAllItems(layoutWrapper);
    } else if (!needSkipLayout_) {
        MeasureItemsLazy(layoutWrapper);
    }
    if (updateKnownSize) {
        layoutInfo_->UpdatePosMap(prevBodyMainSize_, GetKnownBodySize());
        totalMainSize_ = layoutInfo_->totalMainSize_;
    }
}

OptionalSizeF LazyColumnLayoutAlgorithm::ResolveContentIdealSize(const LayoutConstraintF& contentConstraint,
    const RefPtr<LazyColumnLayoutProperty>& layoutProperty) const
{
    auto contentIdealSize = CreateIdealSize(contentConstraint, Axis::VERTICAL, layoutProperty->GetMeasureType());
    if (contentIdealSize.Width().has_value()) {
        return contentIdealSize;
    }
    auto maxSize = contentConstraint.maxSize.Width();
    if (GreaterOrEqualToInfinity(maxSize)) {
        maxSize = contentConstraint.percentReference.Width();
    }
    contentIdealSize.SetWidth(maxSize);
    return contentIdealSize;
}

void LazyColumnLayoutAlgorithm::CaptureFrameBaseline()
{
    // Preparation commits info without geometry. Consume that baseline before this frame changes it.
    premeasuredMainSize_ = layoutInfo_->premeasuredMainSize_;
    layoutInfo_->premeasuredMainSize_.reset();
    totalMainSize_ = layoutInfo_->totalMainSize_;
    // Edge sizes still hold last frame's values here.
    prevBodyMainSize_ = std::max(0.0f, totalMainSize_ - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_);
    // UpdatePosMap() treats layoutInfo_ as body-local; keep the published section extent (edges included) out of
    // it, or a no-item-update pass appends the edge slots twice.
    layoutInfo_->totalMainSize_ = prevBodyMainSize_;
    // Body known via prev extent or live items; edges alone must not count (empty -> non-empty would mis-clamp).
    hadMeasuredItems_ = Positive(prevBodyMainSize_) || !layoutInfo_->posMap_.empty();
    // Defensive: non-lazy paths (empty data / full layout) must not leak last frame's adjust to the parent.
    layoutInfo_->adjustOffset_ = {};
}

void LazyColumnLayoutAlgorithm::MeasurePredictItems(LayoutWrapper* layoutWrapper,
    const RefPtr<LazyColumnLayoutProperty>& layoutProperty, const OptionalSizeF& contentIdealSize)
{
    layoutInfo_->SetTotalItemCount(totalItemCount_);
    layoutInfo_->SetSpace(space_);
    auto crossSize = contentIdealSize.Width().value();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto paddingOffset = OffsetF(padding.left.value_or(0.0f), padding.top.value_or(0.0f));
    PredictLayoutForward(layoutWrapper, crossSize, paddingOffset);
    PredictLayoutBackward(layoutWrapper, crossSize, paddingOffset);
    SyncPredictLayoutInfo(layoutWrapper);
}

void LazyColumnLayoutAlgorithm::MeasureHeader(LayoutWrapper* layoutWrapper)
{
    const float headerMainSize = HeaderFooterUtils::MeasureEdgeMainSize(
        layoutWrapper, header_, headerIndex_, edgeLayoutConstraint_, Axis::VERTICAL);
    headerMainSizeDelta_ = headerMainSize - layoutInfo_->headerMainSize_;
    layoutInfo_->headerMainSize_ = headerMainSize;
}

void LazyColumnLayoutAlgorithm::UpdateHeaderAdjustOffset()
{
    // Backward windows anchor the section bottom; the first lazy frame has no parent anchor. No compensation.
    if (!forwardLayout_ || !hadMeasuredItems_ || NearZero(headerMainSizeDelta_)) {
        return;
    }
    headerAdjustOffset_ =
        HeaderFooterUtils::CalcHeaderResizeAdjust(headerMainSizeDelta_, startPos_, layoutInfo_->headerMainSize_);
}

void LazyColumnLayoutAlgorithm::MeasureFooter(LayoutWrapper* layoutWrapper)
{
    layoutInfo_->footerMainSize_ = HeaderFooterUtils::MeasureEdgeMainSize(
        layoutWrapper, footer_, footerIndex_, edgeLayoutConstraint_, Axis::VERTICAL);
}

void LazyColumnLayoutAlgorithm::ComposeChildStickyInsets(LayoutWrapper* layoutWrapper)
{
    // Header / footer are already measured this frame, so the composed insets reflect current edge sizes.
    const auto stickyStyle = ResolveStickyStyle(layoutWrapper);
    childStickyTopInset_ =
        stickyTopInset_ + (HeaderFooterUtils::IsHeaderSticky(stickyStyle) ? layoutInfo_->headerMainSize_ : 0.0f);
    childStickyBottomInset_ =
        stickyBottomInset_ + (HeaderFooterUtils::IsFooterSticky(stickyStyle) ? layoutInfo_->footerMainSize_ : 0.0f);
}

void LazyColumnLayoutAlgorithm::UpdatePosReference(LayoutWrapper* layoutWrapper,
    std::optional<ViewPosReference>& posRef)
{
    headerAdjustOffset_ = 0.0f;
    isEstimatePass_ = false;
    auto host = layoutWrapper->GetHostNode();
    // A fresh cached host either has no parent viewport yet or receives the parent's predictive deadline. Sample two
    // items to seed the existing total-height estimator instead of publishing a zero-height host.
    const auto measureMode = LazyLayoutUtils::ResolveMeasureMode(
        host, Axis::VERTICAL, posRef, totalItemCount_, 1, hadMeasuredItems_);
    if (ApplyMeasureMode(measureMode)) {
        return;
    }
    if (!posRef.has_value()) {
        posRef = LazyLayoutUtils::GetViewPosReference(host);
    }
    if (!posRef.has_value() || posRef.value().axis != Axis::VERTICAL) {
        needAllLayout_ = true;
        needSkipLayout_ = false;
        stickyTopInset_ = 0.0f;
        stickyBottomInset_ = 0.0f;
        return;
    }
    if (!UpdateViewRange(layoutWrapper, posRef.value())) {
        return;
    }
    UpdateHeaderAdjustOffset();
}

bool LazyColumnLayoutAlgorithm::ApplyMeasureMode(LazyLayoutMeasureMode measureMode)
{
    if (measureMode == LazyLayoutMeasureMode::NORMAL) {
        return false;
    }
    needAllLayout_ = false;
    needSkipLayout_ = measureMode == LazyLayoutMeasureMode::SKIP;
    isEstimatePass_ = measureMode == LazyLayoutMeasureMode::ESTIMATE;
    layoutInfo_->deadline_.reset();
    return true;
}

bool LazyColumnLayoutAlgorithm::UpdateViewRange(LayoutWrapper* layoutWrapper, const ViewPosReference& posRef)
{
    forwardLayout_ = posRef.referenceEdge == ReferenceEdge::START;
    // for self-triggered prediction, reuse the referencePos_ adjusted by adjustOffset from the previous frame.
    referencePos_ = layoutInfo_->deadline_.has_value() ? layoutInfo_->referencePos_ : posRef.referencePos;
    viewExtStart_ = posRef.viewExtStart;
    viewExtEnd_ = posRef.viewExtEnd;
    stickyTopInset_ = posRef.stickyInsetStart;
    stickyBottomInset_ = posRef.stickyInsetEnd;
    if (forwardLayout_) {
        startPos_ = posRef.viewPosStart - viewExtStart_ - referencePos_;
        endPos_ = posRef.viewPosEnd + viewExtEnd_ - referencePos_;
    } else {
        auto mainSize = premeasuredMainSize_;
        if (!mainSize) {
            auto geometryNode = layoutWrapper->GetGeometryNode();
            CHECK_NULL_RETURN(geometryNode, false);
            mainSize = geometryNode->GetPaddingSize().Height();
        }
        referencePos_ += totalMainSize_ - mainSize.value();
        startPos_ = posRef.viewPosStart - viewExtStart_ - (referencePos_ - totalMainSize_);
        endPos_ = posRef.viewPosEnd + viewExtEnd_ - (referencePos_ - totalMainSize_);
    }
    const float viewSize = posRef.viewPosEnd - posRef.viewPosStart;
    cacheStartPos_ = startPos_ - viewSize * cacheSize_;
    cacheEndPos_ = endPos_ + viewSize * cacheSize_;
    // Translate the section viewport to body coords once; all window math downstream is body-local.
    const auto headerMainSize = layoutInfo_->headerMainSize_;
    if (!NearZero(headerMainSize)) {
        startPos_ -= headerMainSize;
        endPos_ -= headerMainSize;
        cacheStartPos_ -= headerMainSize;
        cacheEndPos_ -= headerMainSize;
    }
    needAllLayout_ = false;
    needSkipLayout_ = false;
    // When not in own idle task but parent is doing predictive layout in idle,
    // inherit deadline and cache positions
    if (!layoutInfo_->deadline_.has_value() && posRef.deadline.has_value()) {
        layoutInfo_->deadline_ = posRef.deadline.value();
        layoutInfo_->cacheStartPos_ = cacheStartPos_;
        layoutInfo_->cacheEndPos_ = cacheEndPos_;
    }
    return true;
}

void LazyColumnLayoutAlgorithm::UpdateAttribute(const RefPtr<LazyColumnLayoutProperty>& layoutProperty,
    const LayoutConstraintF& contentConstraint)
{
    auto mainPercentRefer = contentConstraint.percentReference.Height();
    auto spaceDim = layoutProperty->GetSpace().value_or(Dimension(0));
    space_ = ConvertToPx(spaceDim, contentConstraint.scaleProperty, mainPercentRefer).value_or(0);
    space_ = std::max(space_, 0.0f);
    horizontalAlign_ = layoutProperty->GetHorizontalAlign().value_or(HorizontalAlign::CENTER);
    isRtl_ = layoutProperty->GetNonAutoLayoutDirection() == TextDirection::RTL;
}

void LazyColumnLayoutAlgorithm::UpdateChildConstraint(const RefPtr<LazyColumnLayoutProperty>& layoutProperty,
    const OptionalSizeF& contentIdealSize)
{
    auto width = contentIdealSize.Width().value();
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.parentIdealSize = contentIdealSize;
    childLayoutConstraint.maxSize.SetHeight(Infinity<float>());
    childLayoutConstraint.maxSize.SetWidth(width);
    childLayoutConstraint.percentReference.SetWidth(width);
    childLayoutConstraint.viewPosRef.reset();
    childLayoutConstraint_ = childLayoutConstraint;

    // Header/footer measure against the full cross size with infinite main size, like content items.
    edgeLayoutConstraint_ = childLayoutConstraint;
}

int32_t LazyColumnLayoutAlgorithm::CalculateItemCount(LayoutWrapper* layoutWrapper) const
{
    CHECK_NULL_RETURN(layoutWrapper, 0);
    return HeaderFooterUtils::CalcContentItemCount(layoutWrapper->GetTotalChildCount(), headerIndex_, footerIndex_);
}

void LazyColumnLayoutAlgorithm::UpdateHeaderFooterIndexes(LayoutWrapper* layoutWrapper)
{
    HeaderFooterUtils::ResolveEdgeIndexes<LazyColumnLayoutPattern>(
        layoutWrapper, header_, footer_, headerIndex_, footerIndex_);
}

int32_t LazyColumnLayoutAlgorithm::GetRawIndexForItem(int32_t itemIndex) const
{
    return itemIndex + (headerIndex_ >= 0 ? 1 : 0);
}

StickyStyle LazyColumnLayoutAlgorithm::ResolveStickyStyle(LayoutWrapper* layoutWrapper) const
{
    CHECK_NULL_RETURN(layoutWrapper, StickyStyle::NONE);
    auto layoutProperty = AceType::DynamicCast<LazyColumnLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, StickyStyle::NONE);
    return layoutProperty->GetStickyStyle().value_or(StickyStyle::NONE);
}

bool LazyColumnLayoutAlgorithm::CheckNeedMeasure(const RefPtr<LayoutWrapper>& layoutWrapper) const
{
    if (layoutWrapper->CheckNeedForceMeasureAndLayout()) {
        return true;
    }
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, true);
    auto constraint = geometryNode->GetParentLayoutConstraint();
    CHECK_NULL_RETURN(constraint, true);
    return constraint.value() != childLayoutConstraint_;
}

void LazyColumnLayoutAlgorithm::MeasureAllItems(LayoutWrapper* layoutWrapper)
{
    float totalSize = 0.0f;
    layoutInfo_->posMap_.clear();
    layoutInfo_->space_ = space_;
    int32_t curIndex = 0;
    while (curIndex < totalItemCount_) {
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(curIndex));
        if (!wrapper) {
            curIndex++;
            continue;
        }
        if (CheckNeedMeasure(wrapper)) {
            wrapper->Measure(childLayoutConstraint_);
        }
        UpdateSiblingHandoffGap(layoutWrapper, wrapper, curIndex);
        auto geometryNode = wrapper->GetGeometryNode();
        if (!geometryNode) {
            curIndex++;
            continue;
        }
        float mainSize = geometryNode->GetMarginFrameSize().Height();
        layoutInfo_->posMap_[curIndex] = { totalSize, totalSize + mainSize };
        totalSize += mainSize;
        if (curIndex < totalItemCount_ - 1) {
            totalSize += space_;
        }
        curIndex++;
    }
    layoutInfo_->startIndex_ = 0;
    layoutInfo_->endIndex_ = totalItemCount_ - 1;
    layoutInfo_->visibleStartIndex_ = 0;
    layoutInfo_->visibleEndIndex_ = totalItemCount_ - 1;
    layoutInfo_->totalMainSize_ = totalSize;
    layoutInfo_->totalItemCount_ = totalItemCount_;
    totalMainSize_ = totalSize;
}

void LazyColumnLayoutAlgorithm::MeasureEstimateItems(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    CHECK_NULL_VOID(layoutInfo_);
    const int32_t sampleCount = LazyLayoutUtils::CalculateEstimateSampleCount(totalItemCount_, 1);
    layoutInfo_->SetTotalItemCount(totalItemCount_);
    layoutInfo_->space_ = space_;
    float currentPos = 0.0f;
    float measuredMainSize = 0.0f;
    int32_t measuredCount = 0;
    for (int32_t index = 0; index < sampleCount; ++index) {
        float mainSize = 0.0f;
        if (!MeasureItem(layoutWrapper, index, currentPos, true, mainSize)) {
            continue;
        }
        layoutInfo_->posMap_[index] = { currentPos, currentPos + mainSize };
        currentPos += mainSize;
        measuredMainSize += mainSize;
        ++measuredCount;
        if (index < totalItemCount_ - 1) {
            currentPos += space_;
        }
    }
    if (measuredCount <= 0) {
        totalMainSize_ = prevBodyMainSize_;
        return;
    }
    const float averageMainSize = measuredMainSize / static_cast<float>(measuredCount);
    totalMainSize_ = LazyLayoutUtils::EstimateTotalMainSize(averageMainSize, totalItemCount_, space_);
    layoutInfo_->totalMainSize_ = totalMainSize_;
    layoutInfo_->adjustOffset_ = {};
}

void LazyColumnLayoutAlgorithm::GetStartIndexInfo(int32_t& index, float& pos)
{
    // Clamp against the body extent (skipped on the first layout: extent unknown, measure from item 0 instead).
    if (hadMeasuredItems_ && GreatNotEqual(startPos_, prevBodyMainSize_)) {
        index = layoutInfo_->totalItemCount_;
        pos = prevBodyMainSize_;
        return;
    } else if (LessNotEqual(endPos_, 0.0f)) {
        pos = 0.0f;
        index = -1;
        return;
    } else if (LessOrEqual(startPos_, 0.0f) || layoutInfo_->endIndex_ < 0) {
        pos = 0.0f;
        index = 0;
        return;
    }
    auto it = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    if (it == layoutInfo_->posMap_.end()) {
        pos = 0.0f;
        index = 0;
        return;
    }
    auto nextIt = it;
    ++nextIt;
    while (nextIt != layoutInfo_->posMap_.end() && LessNotEqual(it->second.endPos, startPos_)) {
        it = nextIt;
        ++nextIt;
    }
    if (LessOrEqual(it->second.startPos - space_, startPos_)) {
        if (it->first >= totalItemCount_) {
            index = totalItemCount_;
            pos = prevBodyMainSize_ + space_;
            return;
        }
        index = it->first;
        pos = it->second.startPos;
        return;
    }
    std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> rit(it);
    while (rit != layoutInfo_->posMap_.rend() && GreatNotEqual(rit->second.startPos - space_, startPos_)) {
        ++rit;
    }
    --rit;
    pos = rit->second.startPos;
    index = rit->first;
}

void LazyColumnLayoutAlgorithm::GetEndIndexInfo(int32_t& index, float& pos)
{
    if (LessNotEqual(endPos_, 0.0f)) {
        index = -1;
        pos = 0.0f;
        return;
    } else if (hadMeasuredItems_ && GreatNotEqual(startPos_, prevBodyMainSize_)) {
        index = totalItemCount_;
        pos = prevBodyMainSize_;
        return;
    } else if (GreatOrEqual(endPos_, prevBodyMainSize_) ||
               layoutInfo_->endIndex_ >= layoutInfo_->totalItemCount_) {
        pos = prevBodyMainSize_;
        index = totalItemCount_ - 1;
        return;
    }
    auto it = layoutInfo_->posMap_.find(layoutInfo_->endIndex_);
    if (it == layoutInfo_->posMap_.end()) {
        if (!layoutInfo_->premeasuredMainSize_ || layoutInfo_->posMap_.empty()) {
            pos = prevBodyMainSize_;
            index = layoutInfo_->totalItemCount_ - 1;
            return;
        }
        it = std::prev(layoutInfo_->posMap_.end());
    }
    std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> rit(++it);
    auto nextIt = rit;
    ++nextIt;
    while (nextIt != layoutInfo_->posMap_.rend() && GreatNotEqual(rit->second.startPos, endPos_)) {
        rit = nextIt;
        ++nextIt;
    }
    if (GreatOrEqual(rit->second.endPos + space_, endPos_)) {
        index = rit->first;
        pos = rit->second.endPos;
        return;
    }
    while (it != layoutInfo_->posMap_.end() && LessNotEqual(it->second.endPos + space_, endPos_)) {
        ++it;
    }
    --it;
    index = it->first;
    pos = it->second.endPos;
}

bool LazyColumnLayoutAlgorithm::NeedLazyLayout(const RefPtr<LayoutWrapper>& childWrapper)
{
    CHECK_NULL_RETURN(childWrapper, false);
    auto childLayoutProperty = childWrapper->GetLayoutProperty();
    CHECK_NULL_RETURN(childLayoutProperty, false);
    return childLayoutProperty->GetNeedLazyLayout();
}

LayoutConstraintF LazyColumnLayoutAlgorithm::GetLazyLayoutConstraint(float referencePos, bool forwardLayout)
{
    ViewPosReference ref {
        .viewPosStart = startPos_,
        .viewPosEnd = endPos_,
        .referencePos = referencePos,
        .referenceEdge = forwardLayout ? ReferenceEdge::START : ReferenceEdge::END,
        .axis = Axis::VERTICAL,
        .deadline = layoutInfo_->deadline_,
    };
    if (totalItemCount_ == 1) {
        ref.knownMainSize = GetKnownBodySize();
    }
    auto constraint = childLayoutConstraint_;
    constraint.viewPosRef = ref;
    LazyLayoutUtils::SetStickyInsets(constraint, childStickyTopInset_, childStickyBottomInset_);
    return constraint;
}

void LazyColumnLayoutAlgorithm::UpdateSiblingHandoffGap(
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& wrapper, int32_t itemIndex) const
{
    CHECK_NULL_VOID(layoutWrapper);
    CHECK_NULL_VOID(wrapper);
    auto currentPattern = LazyLayoutUtils::GetLazyLayoutPattern(wrapper->GetHostNode());
    // The last section has no next sibling: clear its own outbound gap.
    if (currentPattern && itemIndex >= totalItemCount_ - 1) {
        currentPattern->PublishNextStickyHeaderGap(std::nullopt);
    }
    if (itemIndex <= 0) {
        return;
    }
    // Cache-only lookup: a never-built previous item cannot hold stale state, so don't build one on this hot path.
    auto previousWrapper = layoutWrapper->GetChildByIndex(GetRawIndexForItem(itemIndex - 1), true);
    CHECK_NULL_VOID(previousWrapper);
    auto previousPattern = LazyLayoutUtils::GetLazyLayoutPattern(previousWrapper->GetHostNode());
    CHECK_NULL_VOID(previousPattern);
    const bool currentHasStickyHeader = currentPattern && currentPattern->HasStickyHeader();
    const bool previousHasStickyHeader = previousPattern->HasStickyHeader();
    std::optional<float> handoffGap;
    if (previousHasStickyHeader && currentHasStickyHeader) {
        // Item positions advance by margin-frame size, so the frame-to-frame distance includes both margins.
        handoffGap = space_ + GetVerticalMargin(previousWrapper, false) + GetVerticalMargin(wrapper, true);
    }
    previousPattern->PublishNextStickyHeaderGap(handoffGap);
}

AdjustOffset LazyColumnLayoutAlgorithm::GetAdjustOffset(const RefPtr<LayoutWrapper>& item)
{
    AdjustOffset offset {};
    RefPtr<UINode> child = AceType::DynamicCast<FrameNode>(item);
    do {
        CHECK_NULL_RETURN(child, offset);
        auto frameNode = AceType::DynamicCast<FrameNode>(child);
        if (!frameNode) {
            child = child->GetFirstChild();
            continue;
        }
        if (!frameNode->GetLayoutProperty()->GetNeedLazyLayout()) {
            return offset;
        }
        auto pattern = frameNode->GetPattern<LazyLayoutPattern>();
        if (pattern) {
            return pattern->GetAndResetAdjustOffset();
        }
        child = child->GetFirstChild();
    } while (child);
    return offset;
}

void LazyColumnLayoutAlgorithm::ApplyLazyNodeAdjustOffset(
    const RefPtr<LayoutWrapper>& wrapper, float& referencePos, bool forward)
{
    auto adjustOffset = GetAdjustOffset(wrapper);
    if (NearEqual(adjustOffset.start, 0.0f) && NearEqual(adjustOffset.end, 0.0f)) {
        return;
    }

    if (forward) {
        referencePos -= adjustOffset.start;
    } else {
        referencePos += adjustOffset.end;
    }
}

bool LazyColumnLayoutAlgorithm::MeasureItem(
    LayoutWrapper* layoutWrapper, int32_t curIndex, float currentPos, bool forward, float& mainSize)
{
    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(curIndex));
    if (!wrapper) {
        return false;
    }
    if (NeedLazyLayout(wrapper)) {
        auto host = layoutWrapper->GetHostNode();
        auto pattern = host ? host->GetPattern<LazyLayoutPattern>() : nullptr;
        auto geometry = pattern ? pattern->GetPreparedGeometry() : nullptr;
        if (geometry) {
            auto child = geometry->children.find(curIndex);
            auto lazy = LazyLayoutUtils::GetLazyLayoutPattern(wrapper->GetHostNode());
            if (child != geometry->children.end() && lazy) {
                lazy->RestorePreparedGeometry(child->second);
            }
        }
        wrapper->Measure(GetLazyLayoutConstraint(currentPos, forward));
        ApplyLazyNodeAdjustOffset(wrapper, currentPos, forward);
    } else if (CheckNeedMeasure(wrapper)) {
        wrapper->Measure(childLayoutConstraint_);
    }
    UpdateSiblingHandoffGap(layoutWrapper, wrapper, curIndex);
    auto geometryNode = wrapper->GetGeometryNode();
    if (!geometryNode) {
        return false;
    }
    mainSize = geometryNode->GetMarginFrameSize().Height();
    return true;
}

void LazyColumnLayoutAlgorithm::MeasureForward(LayoutWrapper* layoutWrapper, int32_t startIndex, float startPos)
{
    float currentPos = startPos;
    int32_t curIndex = std::max(startIndex, 0);
    auto estimateItemSize = layoutInfo_->GetEstimateItemSize();
    while (LessOrEqual(currentPos, endPos_) && curIndex < totalItemCount_) {
        auto mainSize = 0.0f;
        auto iter = layoutInfo_->posMap_.find(curIndex);
        if (iter != layoutInfo_->posMap_.end()) {
            mainSize = iter->second.endPos - iter->second.startPos;
        }
        if (!Positive(mainSize)) {
            // Use estimated size for items far outside viewport to avoid jank during large scroll jumps
            mainSize = estimateItemSize;
        }
        bool skipMeasure = Positive(mainSize) && LessNotEqual(currentPos + mainSize, cacheStartPos_);
        if (!skipMeasure && !MeasureItem(layoutWrapper, curIndex, currentPos, true, mainSize)) {
            curIndex++;
            continue;
        }
        layoutInfo_->SetPosMap(curIndex, { currentPos, currentPos + mainSize });
        currentPos += mainSize;
        layoutInfo_->endIndex_ = curIndex;
        if (curIndex < totalItemCount_ - 1) {
            currentPos += space_;
        }
        curIndex++;
    }
}

void LazyColumnLayoutAlgorithm::MeasureBackward(LayoutWrapper* layoutWrapper, int32_t endIndex, float endPos)
{
    float currentPos = endPos;
    int32_t curIndex = std::min(endIndex, totalItemCount_ - 1);
    auto estimateItemSize = layoutInfo_->GetEstimateItemSize();
    while (curIndex >= 0 && GreatOrEqual(currentPos, startPos_)) {
        auto mainSize = 0.0f;
        auto iter = layoutInfo_->posMap_.find(curIndex);
        if (iter != layoutInfo_->posMap_.end()) {
            mainSize = iter->second.endPos - iter->second.startPos;
        }
        if (!Positive(mainSize)) {
            // Use estimated size for items far outside viewport to avoid jank during large scroll jumps
            mainSize = estimateItemSize;
        }
        bool skipMeasure = Positive(mainSize) && GreatNotEqual(currentPos - mainSize, cacheEndPos_);
        if (!skipMeasure && !MeasureItem(layoutWrapper, curIndex, currentPos, false, mainSize)) {
            curIndex--;
            continue;
        }
        layoutInfo_->SetPosMap(curIndex, { currentPos - mainSize, currentPos });
        currentPos -= mainSize;
        layoutInfo_->startIndex_ = curIndex;
        if (curIndex > 0) {
            currentPos -= space_;
        }
        curIndex--;
    }
}

void LazyColumnLayoutAlgorithm::CheckRecycle()
{
    FixIndexRange(layoutInfo_->startIndex_, layoutInfo_->endIndex_);
    auto it = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    while (it != layoutInfo_->posMap_.end()) {
        if (LessNotEqual(it->second.endPos, startPos_)) {
            layoutInfo_->startIndex_ = it->first + 1;
            ++it;
        } else {
            break;
        }
    }
    it = layoutInfo_->posMap_.find(layoutInfo_->endIndex_);
    if (it == layoutInfo_->posMap_.end()) {
        return;
    }
    std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> rit(++it);
    while (rit != layoutInfo_->posMap_.rend()) {
        if (GreatNotEqual(rit->second.startPos, endPos_)) {
            layoutInfo_->endIndex_ = rit->first - 1;
            ++rit;
        } else {
            break;
        }
    }
}

void LazyColumnLayoutAlgorithm::CalculateVisibleStartIndex()
{
    layoutInfo_->visibleStartIndex_ = layoutInfo_->startIndex_;
    if (LessOrEqual(viewExtStart_, 0.0f)) {
        return;
    }
    auto iter = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    if (iter == layoutInfo_->posMap_.end()) {
        return;
    }
    float visibleStartPos = startPos_ + viewExtStart_;
    while (iter != layoutInfo_->posMap_.end()) {
        if (LessNotEqual(iter->second.endPos, visibleStartPos)) {
            layoutInfo_->visibleStartIndex_ = iter->first + 1;
            ++iter;
        } else {
            break;
        }
    }
}

void LazyColumnLayoutAlgorithm::CalculateVisibleEndIndex()
{
    layoutInfo_->visibleEndIndex_ = layoutInfo_->endIndex_;
    if (LessOrEqual(viewExtEnd_, 0.0f)) {
        return;
    }
    auto iter = layoutInfo_->posMap_.find(layoutInfo_->endIndex_);
    if (iter == layoutInfo_->posMap_.end()) {
        return;
    }
    float visibleEndPos = endPos_ - viewExtEnd_;
    std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> riter(++iter);
    while (riter != layoutInfo_->posMap_.rend()) {
        if (GreatNotEqual(riter->second.startPos, visibleEndPos)) {
            layoutInfo_->visibleEndIndex_ = riter->first - 1;
            ++riter;
        } else {
            break;
        }
    }
}

void LazyColumnLayoutAlgorithm::CheckCacheRecycle()
{
    auto it = layoutInfo_->posMap_.find(layoutInfo_->cachedStartIndex_);
    while (it != layoutInfo_->posMap_.end()) {
        if (LessNotEqual(it->second.endPos, layoutInfo_->cacheStartPos_)) {
            layoutedStartIndex_ = it->first + 1;
            layoutedStart_ = it->second.endPos;
            cachedStartIndex_ = layoutedStartIndex_;
            ++it;
        } else {
            break;
        }
    }
    if (layoutedStartIndex_ >= totalItemCount_) {
        layoutedEndIndex_ = totalItemCount_;
        layoutedEnd_ = prevBodyMainSize_; // body-local sentinel; totalMainSize_ is still the stale section here
        cachedEndIndex_ = layoutedEndIndex_;
        return;
    }
    it = layoutInfo_->posMap_.find(layoutInfo_->cachedEndIndex_);
    if (it == layoutInfo_->posMap_.end()) {
        return;
    }
    std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> rit(++it);
    while (rit != layoutInfo_->posMap_.rend()) {
        if (GreatNotEqual(rit->second.startPos, layoutInfo_->cacheEndPos_)) {
            layoutedEndIndex_ = rit->first - 1;
            layoutedEnd_ = rit->second.startPos;
            cachedEndIndex_ = layoutedEndIndex_;
            ++rit;
        } else {
            break;
        }
    }
    if (layoutedEndIndex_ < 0) {
        layoutedStartIndex_ = -1;
        layoutedStart_ = 0.0f;
        cachedStartIndex_ = layoutedStartIndex_;
        return;
    }
}

void LazyColumnLayoutAlgorithm::MeasureItemsLazy(LayoutWrapper* layoutWrapper)
{
    // Pre-shift: fill exactly the window the parent will show after consuming adjustOffset_.start.
    ShiftLayoutWindow(headerAdjustOffset_);
    if (forwardLayout_) {
        int32_t startIndex = 0;
        float startPos = 0.0f;
        GetStartIndexInfo(startIndex, startPos);
        layoutInfo_->SetTotalItemCount(totalItemCount_);
        layoutInfo_->SetSpace(space_);
        layoutInfo_->startIndex_ = startIndex;
        layoutInfo_->endIndex_ = startIndex;
        if (startIndex >= 0) {
            MeasureForward(layoutWrapper, startIndex, startPos);
            MeasureBackward(layoutWrapper, startIndex - 1, startPos - space_);
        }
    } else {
        int32_t endIndex = layoutInfo_->endIndex_;
        float endPos = totalMainSize_;
        GetEndIndexInfo(endIndex, endPos);
        layoutInfo_->SetTotalItemCount(totalItemCount_);
        layoutInfo_->SetSpace(space_);
        layoutInfo_->endIndex_ = endIndex;
        layoutInfo_->startIndex_ = endIndex;
        if (endIndex < totalItemCount_) {
            MeasureBackward(layoutWrapper, endIndex, endPos);
            MeasureForward(layoutWrapper, endIndex + 1, endPos + space_);
        }
    }
    CheckRecycle();
    layoutInfo_->UpdatePosMap(prevBodyMainSize_, GetKnownBodySize());
    ShiftLayoutWindow(layoutInfo_->adjustOffset_.start);
    if (!hadMeasuredItems_) {
        // First layout: no parent anchor to preserve.
        layoutInfo_->adjustOffset_ = {};
    }
    // Report the pre-shifted header compensation; never apply it to the window a second time.
    layoutInfo_->adjustOffset_.start += headerAdjustOffset_;
    referencePos_ = forwardLayout_ ? referencePos_ - layoutInfo_->adjustOffset_.start
                                   : referencePos_ + layoutInfo_->adjustOffset_.end;
    totalMainSize_ = layoutInfo_->totalMainSize_;
    CalculateVisibleStartIndex();
    CalculateVisibleEndIndex();
}

void LazyColumnLayoutAlgorithm::SetFrameSize(LayoutWrapper* layoutWrapper, OptionalSizeF& contentIdealSize)
{
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto property = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    const auto& layoutConstraint = property->GetLayoutConstraint();
    const auto& minSize = layoutConstraint->minSize;
    const auto& maxSize = layoutConstraint->maxSize;
    OptionalSizeF frameSize;
    frameSize.UpdateSizeWithCheck(layoutConstraint->selfIdealSize);
    if (frameSize.IsValid()) {
        geometryNode->SetFrameSize(frameSize.ConvertToSizeT());
        return;
    }

    ApplyMainAxisExtentByLayoutPolicy(
        contentIdealSize, layoutConstraint.value(),
            property->GetLayoutPolicyProperty(), Axis::VERTICAL, totalMainSize_);

    const auto& padding = property->CreatePaddingAndBorder();
    AddPaddingToSize(padding, contentIdealSize);
    frameSize.UpdateIllegalSizeWithCheck(contentIdealSize);
    if (layoutConstraint->selfIdealSize.Width()) {
        frameSize.ConstrainFloat(minSize, maxSize, false, true);
    } else if (layoutConstraint->selfIdealSize.Height()) {
        frameSize.ConstrainFloat(minSize, maxSize, true, true);
    } else {
        frameSize.Constrain(minSize, maxSize, true);
    }
    frameSize.UpdateIllegalSizeWithCheck(SizeF { 0.0f, 0.0f });
    geometryNode->SetFrameSize(frameSize.ConvertToSizeT());
}

void LazyColumnLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    if (isEstimatePass_) {
        return;
    }
    auto layoutProperty = AceType::DynamicCast<LazyColumnLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto size = geometryNode->GetFrameSize();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0.0f);
    auto top = padding.top.value_or(0.0f);
    auto crossSize = size.Width();
    auto paddingOffset = OffsetF(left, top);
    
    auto stickyStyle = ResolveStickyStyle(layoutWrapper);
    const auto nextStickyHeaderGap = HeaderFooterUtils::GetNextStickyHeaderGap(layoutWrapper);
    // Body-local: startPos_/endPos_ are body coords; add the header back so sticky math runs in section coords.
    const HeaderFooterStickyMetrics stickyMetrics { startPos_ + layoutInfo_->headerMainSize_,
        endPos_ + layoutInfo_->headerMainSize_, totalMainSize_,
        layoutInfo_->headerMainSize_, layoutInfo_->footerMainSize_, stickyTopInset_, stickyBottomInset_,
        nextStickyHeaderGap };
    const auto stickyHeaderPos = HeaderFooterUtils::CalcStickyHeaderPos(stickyMetrics);
    const auto stickyFooterPos = HeaderFooterUtils::CalcStickyFooterPos(stickyMetrics);

    if (layoutInfo_->deadline_) {
        layoutInfo_->deadline_.reset();
        // Re-lay the sticky header/footer on predictive frames too, else the edges freeze for a frame and flicker.
        LayoutHeader(layoutWrapper, crossSize, paddingOffset, stickyStyle, stickyHeaderPos);
        LayoutFooter(layoutWrapper, crossSize, paddingOffset, stickyStyle, stickyFooterPos);
        return;
    }

    // Layout order follows section semantics: header -> content items -> footer.
    LayoutHeader(layoutWrapper, crossSize, paddingOffset, stickyStyle, stickyHeaderPos);
    LayoutItems(layoutWrapper, crossSize, paddingOffset);
    LayoutCachedItems(layoutWrapper, crossSize, paddingOffset);
    LayoutFooter(layoutWrapper, crossSize, paddingOffset, stickyStyle, stickyFooterPos);
}

float LazyColumnLayoutAlgorithm::CalculateCrossOffset(float crossSize, float childCrossSize) const
{
    float delta = crossSize - childCrossSize;
    if (LessOrEqual(delta, 0)) {
        return 0.0f;
    }
    switch (horizontalAlign_) {
        case HorizontalAlign::START:
            return 0.0f;
        case HorizontalAlign::CENTER:
            return delta / 2.0f;
        case HorizontalAlign::END:
            return delta;
        default:
            return 0.0f;
    }
}

void LazyColumnLayoutAlgorithm::SetItemOffset(const RefPtr<LayoutWrapper>& wrapper, const ColumnItemMainPos& pos,
    float crossSize, const OffsetF& paddingOffset)
{
    auto geometryNode = wrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto offset = paddingOffset;
    auto childSize = geometryNode->GetMarginFrameSize();
    float crossOffset = CalculateCrossOffset(crossSize, childSize.Width());
    // Body-local: posMap_ stores body coords; shift items down by the header at layout time.
    const float mainOffset = pos.startPos + layoutInfo_->headerMainSize_;
    if (isRtl_) {
        offset = offset + OffsetF(crossSize - childSize.Width() - crossOffset, mainOffset);
    } else {
        offset = offset + OffsetF(crossOffset, mainOffset);
    }
    geometryNode->SetMarginFrameOffset(offset);
}

void LazyColumnLayoutAlgorithm::PredictLayoutForward(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    int32_t currIndex = layoutInfo_->layoutedEndIndex_;
    float currPos = layoutInfo_->layoutedEnd_;
    // Allow forward prediction when not yet laid out (currIndex<0) with forward layout,
    // or when already laid out (currIndex>=0) with item in posMap
    if ((currIndex < 0 && !forwardLayout_) ||
        (currIndex >= 0 && layoutInfo_->posMap_.find(currIndex) == layoutInfo_->posMap_.end())) {
        layoutedEndIndex_ = currIndex;
        cachedEndIndex_ = std::max(layoutInfo_->cachedEndIndex_, currIndex);
        layoutedEnd_ = currPos;
        return;
    }
    auto deadline = layoutInfo_->deadline_.value();
    while (currIndex < totalItemCount_ - 1 && LessNotEqual(currPos, layoutInfo_->cacheEndPos_)) {
        if (GetSysTimestamp() > deadline) {
            break;
        }
        int32_t index = currIndex + 1;
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(index));
        if (!wrapper || !wrapper->GetHostNode() || !wrapper->GetHostNode()->RenderCustomChild(deadline)) {
            break;
        }
        auto geometryNode = wrapper->GetGeometryNode();
        if (!geometryNode) {
            break;
        }
        if (NeedLazyLayout(wrapper)) {
            wrapper->Measure(GetLazyLayoutConstraint(currPos, true));
        } else {
            wrapper->Measure(childLayoutConstraint_);
        }
        UpdateSiblingHandoffGap(layoutWrapper, wrapper, index);
        auto size = geometryNode->GetMarginFrameSize().Height();
        ColumnItemMainPos pos { currPos, currPos + size };
        currPos = pos.endPos + space_;
        layoutInfo_->SetCachedPosMap(index, pos);
        SetItemOffset(wrapper, pos, crossSize, paddingOffset);
        wrapper->Layout();
        currIndex = index;
    }
    layoutedEndIndex_ = currIndex;
    cachedEndIndex_ = std::max(layoutInfo_->cachedEndIndex_, currIndex);
    layoutedEnd_ = currPos;
}

void LazyColumnLayoutAlgorithm::PredictLayoutBackward(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    int32_t currIndex = layoutInfo_->layoutedStartIndex_;
    float currPos = layoutInfo_->layoutedStart_;
    // Allow backward prediction when not yet laid out (currIndex<0) with backward layout,
    // or when already laid out (currIndex>=0) with item in posMap
    if ((currIndex < 0 && forwardLayout_) ||
        (currIndex >= 0 && layoutInfo_->posMap_.find(currIndex) == layoutInfo_->posMap_.end())) {
        layoutedStartIndex_ = currIndex;
        cachedStartIndex_ = layoutInfo_->cachedStartIndex_ >= 0
            ? std::min(layoutInfo_->cachedStartIndex_, currIndex)
            : currIndex;
        layoutedStart_ = currPos;
        return;
    }
    if (currIndex < 0 && !forwardLayout_) {
        currIndex = totalItemCount_;
    }
    auto deadline = layoutInfo_->deadline_.value();
    while (currIndex > 0 && (GreatNotEqual(currPos, layoutInfo_->cacheStartPos_))) {
        if (GetSysTimestamp() > deadline) {
            break;
        }
        int32_t index = currIndex - 1;
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(index));
        if (!wrapper || !wrapper->GetHostNode() || !wrapper->GetHostNode()->RenderCustomChild(deadline)) {
            break;
        }
        auto geometryNode = wrapper->GetGeometryNode();
        if (!geometryNode) {
            break;
        }
        if (NeedLazyLayout(wrapper)) {
            wrapper->Measure(GetLazyLayoutConstraint(currPos, false));
        } else {
            wrapper->Measure(childLayoutConstraint_);
        }
        UpdateSiblingHandoffGap(layoutWrapper, wrapper, index);
        float size = geometryNode->GetMarginFrameSize().Height();
        ColumnItemMainPos pos { currPos - size, currPos };
        currPos = pos.startPos - space_;
        layoutInfo_->SetCachedPosMap(index, pos);
        SetItemOffset(wrapper, pos, crossSize, paddingOffset);
        wrapper->Layout();
        currIndex = index;
    }
    layoutedStartIndex_ = currIndex;
    cachedStartIndex_ = layoutInfo_->cachedStartIndex_ >= 0
        ? std::min(layoutInfo_->cachedStartIndex_, currIndex)
        : currIndex;
    layoutedStart_ = currPos;
}

void LazyColumnLayoutAlgorithm::FixIndexRange(int32_t& startIndex, int32_t& endIndex)
{
    if (startIndex < 0 && endIndex >= 0 && endIndex < totalItemCount_) {
        startIndex = 0;
    }
    if ((endIndex < 0 || endIndex >= totalItemCount_) && startIndex >= 0 && startIndex < totalItemCount_) {
        endIndex = totalItemCount_ - 1;
    }
}

void LazyColumnLayoutAlgorithm::SyncPredictLayoutInfo(LayoutWrapper* layoutWrapper)
{
    // Recycle items outside visible and cache range
    CheckRecycle();
    CheckCacheRecycle();
    CalculateVisibleStartIndex();
    CalculateVisibleEndIndex();
    FixIndexRange(cachedStartIndex_, cachedEndIndex_);
    FixIndexRange(layoutedStartIndex_, layoutedEndIndex_);
    UpdatePredictActiveRange(layoutWrapper);
    // Sync prediction results back to layoutInfo
    layoutInfo_->layoutedStartIndex_ = layoutedStartIndex_;
    layoutInfo_->cachedStartIndex_ = cachedStartIndex_;
    layoutInfo_->layoutedStart_ = layoutedStart_;

    layoutInfo_->layoutedEndIndex_ = layoutedEndIndex_;
    layoutInfo_->cachedEndIndex_ = cachedEndIndex_;
    layoutInfo_->layoutedEnd_ = layoutedEnd_;

    // Skip posMap update if only self measure changed, as child positions remain valid.
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    if (!knownMainSize_ && layoutProperty->GetPropertyChangeFlag() == PROPERTY_UPDATE_MEASURE_SELF) {
        // Reuse last frame's body; old section minus CURRENT edges would lock the total across an edge resize.
        totalMainSize_ = prevBodyMainSize_;
        return;
    }
    layoutInfo_->UpdatePosMap(prevBodyMainSize_, GetKnownBodySize());
    totalMainSize_ = layoutInfo_->totalMainSize_;
}

void LazyColumnLayoutAlgorithm::UpdatePredictActiveRange(LayoutWrapper* layoutWrapper)
{
    if ((layoutInfo_->cachedStartIndex_ == cachedStartIndex_) && (layoutInfo_->cachedEndIndex_ == cachedEndIndex_)) {
        return;
    }
    if (headerIndex_ < 0 && footerIndex_ < 0) {
        layoutWrapper->SetActiveChildRange(cachedStartIndex_, cachedEndIndex_);
    } else if (cachedStartIndex_ < 0 || cachedEndIndex_ < cachedStartIndex_) {
        SetHeaderFooterActive(layoutWrapper);
    } else {
        ActiveChildSets activeChildSets;
        const auto rawStart = GetRawIndexForItem(cachedStartIndex_);
        const auto rawEnd = GetRawIndexForItem(cachedEndIndex_);
        for (auto i = rawStart; i <= rawEnd; ++i) {
            activeChildSets.activeItems.insert(i);
        }
        if (headerIndex_ >= 0) {
            activeChildSets.activeItems.insert(headerIndex_);
        }
        if (footerIndex_ >= 0) {
            activeChildSets.activeItems.insert(footerIndex_);
        }
        layoutWrapper->SetActiveChildRange(std::optional<ActiveChildSets>(activeChildSets), std::nullopt);
    }
}

void LazyColumnLayoutAlgorithm::SyncGeometry(const RefPtr<LayoutWrapper>& wrapper)
{
    CHECK_NULL_VOID(wrapper);
    auto host = wrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    host->ForceSyncGeometryNode();
    host->ResetLayoutAlgorithm();
}

void LazyColumnLayoutAlgorithm::LayoutItems(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    auto iter = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
    for (; iter != layoutInfo_->posMap_.end() && iter->first <= layoutInfo_->endIndex_; iter++) {
        // h/f/s: posMap_ keys are item indices; translate to raw indices for child fetches.
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(GetRawIndexForItem(iter->first));
        if (!wrapper) {
            continue;
        }
        SetItemOffset(wrapper, iter->second, crossSize, paddingOffset);
        if (wrapper->CheckNeedForceMeasureAndLayout()) {
            wrapper->Layout();
        } else {
            SyncGeometry(wrapper);
        }
    }
}

void LazyColumnLayoutAlgorithm::LayoutCachedItemsForward(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    auto iter = layoutInfo_->posMap_.begin();
    if (layoutInfo_->endIndex_ >= totalItemCount_ - 1) {
        layoutedEnd_ = totalMainSize_ - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_;
        cachedEndIndex_ = layoutInfo_->endIndex_;
    } else if (layoutInfo_->endIndex_ < 0) {
        cachedEndIndex_ = -1;
        layoutedEnd_ = 0.0f;
    } else {
        cachedEndIndex_ = layoutInfo_->endIndex_;
        iter = layoutInfo_->posMap_.find(layoutInfo_->endIndex_);
        if (iter == layoutInfo_->posMap_.end()) {
            TAG_LOGE(AceLogTag::ACE_LAZY_COLUMN, "find end index error:%{public}d", layoutInfo_->endIndex_);
            return;
        }
        layoutedEnd_ = iter->second.endPos + space_;
        iter++;
    }
    layoutedEndIndex_ = cachedEndIndex_;
    bool hadDirty = false;
    while (iter != layoutInfo_->posMap_.end() && LessNotEqual(iter->second.startPos, cacheEndPos_)) {
        if (cachedEndIndex_ + 1 != iter->first) {
            break;
        }
        // h/f/s: posMap_ keys are item indices; translate to raw indices for child fetches.
        auto wrapper = layoutWrapper->GetChildByIndex(GetRawIndexForItem(iter->first), true);
        if (!wrapper) {
            break;
        }
        SetItemOffset(wrapper, iter->second, crossSize, paddingOffset);
        SyncGeometry(wrapper);
        if (hadDirty || CheckNeedMeasure(wrapper)) {
            hadDirty = true;
        } else {
            layoutedEnd_ = iter->second.endPos + space_;
            layoutedEndIndex_ = iter->first;
        }
        cachedEndIndex_ = iter->first;
        iter++;
    }
}

void LazyColumnLayoutAlgorithm::LayoutCachedItemsBackward(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    auto rIter = layoutInfo_->posMap_.rbegin();
    if (layoutInfo_->startIndex_ == 0) {
        layoutedStart_ = 0.0f;
        cachedStartIndex_ = 0;
    } else if (layoutInfo_->startIndex_ >= totalItemCount_) {
        layoutedStart_ = totalMainSize_ - layoutInfo_->headerMainSize_ - layoutInfo_->footerMainSize_;
        cachedStartIndex_ = totalItemCount_;
    } else {
        cachedStartIndex_ = layoutInfo_->startIndex_;
        auto iter = layoutInfo_->posMap_.find(layoutInfo_->startIndex_);
        if (iter == layoutInfo_->posMap_.end()) {
            TAG_LOGE(AceLogTag::ACE_LAZY_COLUMN, "find start index error:%{public}d", layoutInfo_->endIndex_);
            return;
        }
        layoutedStart_ = iter->second.startPos - space_;
        std::reverse_iterator<std::map<int32_t, ColumnItemMainPos>::iterator> rit(iter);
        rIter = rit;
    }
    layoutedStartIndex_ = cachedStartIndex_;
    bool hadDirty = false;
    while (rIter != layoutInfo_->posMap_.rend() && GreatNotEqual(rIter->second.endPos, cacheStartPos_)) {
        if (cachedStartIndex_ - 1 != rIter->first) {
            break;
        }
        // h/f/s: posMap_ keys are item indices; translate to raw indices for child fetches.
        auto wrapper = layoutWrapper->GetChildByIndex(GetRawIndexForItem(rIter->first), true);
        if (!wrapper) {
            break;
        }
        SetItemOffset(wrapper, rIter->second, crossSize, paddingOffset);
        SyncGeometry(wrapper);
        if (hadDirty || CheckNeedMeasure(wrapper)) {
            hadDirty = true;
        } else {
            layoutedStart_ = rIter->second.startPos - space_;
            layoutedStartIndex_ = rIter->first;
        }
        cachedStartIndex_ = rIter->first;
        rIter++;
    }
}

void LazyColumnLayoutAlgorithm::LayoutCachedItems(
    LayoutWrapper* layoutWrapper, float crossSize, const OffsetF& paddingOffset)
{
    LayoutCachedItemsForward(layoutWrapper, crossSize, paddingOffset);
    LayoutCachedItemsBackward(layoutWrapper, crossSize, paddingOffset);
    FixIndexRange(cachedStartIndex_, cachedEndIndex_);
    // h/f/s: translate item indices to raw indices for SetActiveChildRange. When header / footer are mounted,
    // use ActiveChildSets so the edges stay active alongside the item cache window.
    if (headerIndex_ < 0 && footerIndex_ < 0) {
        layoutWrapper->SetActiveChildRange(cachedStartIndex_, cachedEndIndex_, 0, 0);
    } else if (cachedStartIndex_ < 0 || cachedEndIndex_ < cachedStartIndex_) {
        SetHeaderFooterActive(layoutWrapper);
    } else {
        ActiveChildSets activeChildSets;
        const auto rawStart = GetRawIndexForItem(cachedStartIndex_);
        const auto rawEnd = GetRawIndexForItem(cachedEndIndex_);
        for (auto i = rawStart; i <= rawEnd; ++i) {
            activeChildSets.activeItems.insert(i);
        }
        if (headerIndex_ >= 0) {
            activeChildSets.activeItems.insert(headerIndex_);
        }
        if (footerIndex_ >= 0) {
            activeChildSets.activeItems.insert(footerIndex_);
        }
        layoutWrapper->SetActiveChildRange(std::optional<ActiveChildSets>(activeChildSets), std::nullopt);
    }
    layoutInfo_->referencePos_ = referencePos_;
    layoutInfo_->layoutedStart_ = layoutedStart_;
    layoutInfo_->layoutedEnd_ = layoutedEnd_;
    layoutInfo_->layoutedStartIndex_ = layoutedStartIndex_;
    layoutInfo_->layoutedEndIndex_ = layoutedEndIndex_;
    layoutInfo_->cacheStartPos_ = cacheStartPos_;
    layoutInfo_->cacheEndPos_ = cacheEndPos_;
    layoutInfo_->cachedStartIndex_ = cachedStartIndex_;
    layoutInfo_->cachedEndIndex_ = cachedEndIndex_;
}

void LazyColumnLayoutAlgorithm::SetHeaderFooterActive(LayoutWrapper* layoutWrapper) const
{
    ActiveChildSets activeChildSets;
    if (headerIndex_ >= 0) {
        activeChildSets.activeItems.insert(headerIndex_);
    }
    if (footerIndex_ >= 0) {
        activeChildSets.activeItems.insert(footerIndex_);
    }
    layoutWrapper->SetActiveChildRange(std::optional<ActiveChildSets>(activeChildSets), std::nullopt);
}

void LazyColumnLayoutAlgorithm::LayoutHeader(LayoutWrapper* layoutWrapper, float crossSize,
    const OffsetF& paddingOffset, StickyStyle stickyStyle, float stickyHeaderPos) const
{
    auto isSticky = HeaderFooterUtils::IsHeaderSticky(stickyStyle);
    auto offset = paddingOffset + (isSticky ? OffsetF(0.0f, stickyHeaderPos) : OffsetF());
    HeaderFooterUtils::LayoutEdge(layoutWrapper, headerIndex_, offset, isSticky, isRtl_, crossSize,
        HeaderFooterUtils::STICKY_HEADER_Z_INDEX);
}

void LazyColumnLayoutAlgorithm::LayoutFooter(LayoutWrapper* layoutWrapper, float crossSize,
    const OffsetF& paddingOffset, StickyStyle stickyStyle, float stickyFooterPos) const
{
    auto isSticky = HeaderFooterUtils::IsFooterSticky(stickyStyle);
    auto footerPos = isSticky ? stickyFooterPos : totalMainSize_ - layoutInfo_->footerMainSize_;
    auto offset = OffsetF(paddingOffset.GetX(), paddingOffset.GetY() + footerPos);
    HeaderFooterUtils::LayoutEdge(layoutWrapper, footerIndex_, offset, isSticky, isRtl_, crossSize);
}
} // namespace OHOS::Ace::NG

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

#include "core/components_ng/pattern/lazy_layout/lazy_layout_pattern.h"

#include <string>

#include "base/log/dump_log.h"
#include "base/utils/utils.h"
#include "core/components_ng/animation/geometry_transition.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
class LazyPremeasureScope final {
public:
    LazyPremeasureScope(const RefPtr<FrameNode>& node, const RefPtr<LazyLayoutPattern>& pattern,
        LayoutWrapper* wrapper)
        : node_(node), pattern_(pattern), geometry_(node->GetGeometryNode()),
          previous_(pattern->SwapPremeasureWrapper(wrapper))
    {
        node_->SetGeometryNode(wrapper->GetGeometryNode());
    }

    ~LazyPremeasureScope()
    {
        node_->SetGeometryNode(geometry_);
        pattern_->SwapPremeasureWrapper(previous_);
        node_->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF);
    }

    ACE_DISALLOW_COPY_AND_MOVE(LazyPremeasureScope);

private:
    RefPtr<FrameNode> node_;
    RefPtr<LazyLayoutPattern> pattern_;
    RefPtr<GeometryNode> geometry_;
    LayoutWrapper* previous_ = nullptr;
};

// Only lazy containers use a temporary window. Ordinary descendants keep FrameNode's complete Measure lifecycle.
class LazyPremeasureWrapper final : public LayoutWrapperNode {
    DECLARE_ACE_TYPE(LazyPremeasureWrapper, LayoutWrapperNode);
public:
    explicit LazyPremeasureWrapper(const RefPtr<LayoutWrapper>& source)
        : LayoutWrapperNode(source->GetHostNode(), source->GetGeometryNode()->Clone(), source->GetLayoutProperty()),
          source_(source)
    {
        auto host = source->GetHostNode();
        RefPtr<LayoutAlgorithm> algorithm;
        if (AceType::InstanceOf<FrameNode>(source)) {
            algorithm = host->GetPattern()->CreateLayoutAlgorithm();
        } else {
            const auto& sourceAlgorithm = source->GetLayoutAlgorithm();
            algorithm = sourceAlgorithm ? sourceAlgorithm->GetLayoutAlgorithm() : nullptr;
        }
        if (algorithm) {
            SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(algorithm));
        }
    }

    int32_t GetTotalChildCount() const override { return source_->GetTotalChildCount(); }
    void Layout() override {}
    bool CheckNeedForceMeasureAndLayout() override { return true; }

    RefPtr<LayoutWrapper> GetOrCreateChildByIndex(
        uint32_t index, bool, bool isCache, bool = false) override
    {
        auto it = measuredChildren_.find(index);
        if (it != measuredChildren_.end()) {
            return it->second;
        }
        // Preserve the published window's cache flag without activating children or inferring it from sparse indexes.
        auto child = source_->GetOrCreateChildByIndex(index, false, isCache || !source_->IsActive());
        if (!child) {
            childrenComplete_ = false;
            return nullptr;
        }
        if (!child->GetLayoutProperty()->GetNeedLazyLayout()) {
            measuredChildren_.emplace(index, child);
            return child;
        }
        auto measured = MakeRefPtr<LazyPremeasureWrapper>(child);
        // Preserve the converted reference even for an intermediate which does not forward it itself.
        if (LazyLayoutUtils::IsAllowedIntermediateNode(GetHostNode())) {
            measured->inheritedReference_ = GetLayoutProperty()->GetContentLayoutConstraint()->viewPosRef;
        }
        measuredChildren_.emplace(index, measured);
        return measured;
    }

    RefPtr<LayoutWrapper> GetChildByIndex(uint32_t index, bool isCache) override
    {
        return GetOrCreateChildByIndex(index, false, isCache);
    }

    ChildrenListWithGuard GetAllChildrenWithBuild(bool) override
    {
        childList_.clear();
        for (int32_t index = 0; index < GetTotalChildCount(); ++index) {
            auto child = GetOrCreateChildByIndex(index, false, false);
            if (child) {
                childList_.push_back(child);
            }
        }
        return ChildrenListWithGuard(childList_, lock_);
    }

    void Measure(const std::optional<LayoutConstraintF>& constraint) override
    {
        result_.reset();
        childrenComplete_ = true;
        auto algorithmWrapper = GetLayoutAlgorithm();
        CHECK_NULL_VOID(algorithmWrapper);
        algorithmWrapper->SetHasMeasured(false);
        CHECK_NULL_VOID(IsSupported());
        if (GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::GONE) {
            MeasureGone();
            return;
        }
        auto applied = constraint.value_or(LayoutConstraintF());
        if (!applied.viewPosRef) {
            applied.viewPosRef = inheritedReference_;
        }
        CHECK_NULL_VOID(PrepareConstraint(applied));
        auto algorithm = algorithmWrapper->GetLayoutAlgorithm();
        CHECK_NULL_VOID(algorithm);
        auto lazy = GetHostNode()->GetPattern<LazyLayoutPattern>();
        if (lazy) {
            if (!applied.viewPosRef || applied.viewPosRef->deadline) {
                return;
            }
            MeasureLazyContainer(lazy, applied);
        } else {
            LayoutWrapperNode::Measure(applied);
            if (LazyLayoutUtils::IsAllowedIntermediateNode(GetHostNode()) && measuredChildren_.size() == 1) {
                auto nested = AceType::DynamicCast<LazyPremeasureWrapper>(measuredChildren_.begin()->second);
                result_ = nested ? nested->GetResult() : std::nullopt;
            }
            childrenComplete_ = childrenComplete_ && measuredChildren_.size() ==
                static_cast<size_t>(GetTotalChildCount());
        }
        FinishMeasure();
    }

    std::optional<LazyPremeasureResult> GetResult() const { return result_; }

    void MeasureBoundary(LazyLayoutPattern* pattern)
    {
        result_ = pattern->Premeasure(this);
    }

    void CollectMeasuredFrameNodes(std::vector<WeakPtr<FrameNode>>& nodes) const
    {
        if (measuredHost_) {
            nodes.emplace_back(measuredHost_);
        }
        for (const auto& [index, child] : measuredChildren_) {
            auto nested = AceType::DynamicCast<LazyPremeasureWrapper>(child);
            if (nested) {
                nested->CollectMeasuredFrameNodes(nodes);
                continue;
            }
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            if (frameNode) {
                nodes.emplace_back(frameNode);
            }
        }
    }

private:
    void MeasureLazyContainer(const RefPtr<LazyLayoutPattern>& pattern, const LayoutConstraintF& constraint)
    {
        auto node = AceType::DynamicCast<FrameNode>(source_);
        if (!node) {
            MeasureBoundary(AceType::RawPtr(pattern));
            return;
        }
        result_ = LazyPremeasureResult();
        measuredHost_ = node;
        LazyPremeasureScope scope(node, pattern, this);
        node->Measure(constraint);
        childrenComplete_ = childrenComplete_ && LazyLayoutUtils::IsChildMeasureComplete(node);
    }

    bool IsSupported() const
    {
        auto host = GetHostNode();
        auto property = GetLayoutProperty();
        // These paths require FrameNode callbacks or tree mutation, not isolated wrapper measurement.
        return !host->GetExtensionHandler() && !host->GetKitNode() && !host->GetOverlayNode() &&
            !AceType::InstanceOf<CustomMeasureLayoutNode>(host) && !property->GetLayoutRect() &&
            !property->GetGeometryTransition();
    }

    bool PrepareConstraint(const LayoutConstraintF& applied)
    {
        ApplyConstraintWithoutMeasure(applied);
        auto property = GetLayoutProperty();
        // A temporary constraint must never let the next real Measure reuse unpublished container geometry.
        property->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF);
        property->UpdateContentConstraint();
        auto geometry = GetGeometryNode();
        geometry->UpdateMargin(property->CreateMargin());
        geometry->UpdatePaddingWithBorder(property->CreatePaddingAndBorder());
        if (GetHostNode()->GetPattern<LazyLayoutPattern>() || !property->GetNeedLazyLayout()) {
            return true;
        }
        const auto& content = property->GetContentLayoutConstraint().value();
        return LazyLayoutUtils::IsAllowedIntermediateNode(GetHostNode()) && GetTotalChildCount() == 1 &&
            !content.selfIdealSize.Height() && !Positive(content.minSize.Height()) &&
            GreaterOrEqualToInfinity(content.maxSize.Height());
    }

    void MeasureGone()
    {
        const float oldSize = source_->GetGeometryNode()->GetMarginFrameSize().Height();
        GetGeometryNode()->SetFrameSize(SizeF());
        GetGeometryNode()->UpdateMargin(MarginPropertyF());
        GetLayoutAlgorithm()->SetHasMeasured(true);
        if (GetLayoutProperty()->GetNeedLazyLayout()) {
            result_ = LazyPremeasureResult { true, true, 0.0f, { 0.0f, -oldSize } };
        }
    }

    void FinishMeasure()
    {
        auto geometry = GetGeometryNode();
        auto property = GetLayoutProperty();
        auto pipeline = GetHostNode()->GetContext();
        if (!measuredHost_ && AceType::InstanceOf<FrameNode>(source_) && pipeline &&
            pipeline->GetPixelRoundMode() == PixelRoundMode::PIXEL_ROUND_AFTER_MEASURE) {
            const auto size = geometry->GetFrameSize();
            geometry->SetPreFrameSize(size);
            geometry->SetFrameSize(SizeF(std::round(size.Width()), std::round(size.Height())));
        }
        for (const auto& entry : measuredChildren_) {
            childrenComplete_ = childrenComplete_ && LazyLayoutUtils::IsChildMeasureComplete(entry.second);
        }
        childrenComplete_ = childrenComplete_ && !GetLayoutAlgorithm()->MeasureInNextFrame();
        if (result_) {
            result_->complete = result_->complete && childrenComplete_ && !result_->unsupported;
            result_->reachedOppositeEdge = result_->reachedOppositeEdge && result_->complete;
            const auto& oldPadding = source_->GetGeometryNode()->GetPadding();
            const float oldTop = oldPadding ? oldPadding->top.value_or(0.0f) : 0.0f;
            const float newTop = property->CreatePaddingAndBorder().top.value_or(0.0f);
            const auto& oldMargin = source_->GetGeometryNode()->GetMargin();
            const float oldMarginTop = oldMargin ? oldMargin->top.value_or(0.0f) : 0.0f;
            const float startInset = newTop - oldTop + property->CreateMargin().top.value_or(0.0f) - oldMarginTop;
            result_->startInsetAdjustment += startInset;
            result_->adjust.start += startInset;
            result_->adjust.end = geometry->GetMarginFrameSize().Height() -
                source_->GetGeometryNode()->GetMarginFrameSize().Height() - result_->adjust.start;
            result_->marginMainSize = geometry->GetMarginFrameSize().Height();
        }
        GetLayoutAlgorithm()->SetHasMeasured(childrenComplete_);
        // Publish container geometry only through its next normal Measure, retaining old-geometry callbacks.
    }

    RefPtr<LayoutWrapper> source_;
    RefPtr<FrameNode> measuredHost_;
    std::map<uint32_t, RefPtr<LayoutWrapper>> measuredChildren_;
    std::list<RefPtr<LayoutWrapper>> childList_;
    RecursiveLock lock_;
    std::optional<ViewPosReference> inheritedReference_;
    std::optional<LazyPremeasureResult> result_;
    bool childrenComplete_ = true;
};
} // namespace

bool LazyLayoutUtils::MeasurePreparedBoundary(LazyLayoutPattern* pattern)
{
    CHECK_NULL_RETURN(pattern, false);
    auto measured = AceType::DynamicCast<LazyPremeasureWrapper>(pattern->GetPremeasureWrapper());
    CHECK_NULL_RETURN(measured, false);
    measured->MeasureBoundary(pattern);
    return true;
}

bool LazyLayoutUtils::IsChildMeasureComplete(const RefPtr<LayoutWrapper>& wrapper)
{
    CHECK_NULL_RETURN(wrapper, false);
    const auto& algorithm = wrapper->GetLayoutAlgorithm();
    CHECK_NULL_RETURN(algorithm && !algorithm->MeasureInNextFrame(), false);
    if (algorithm->SkipMeasure()) {
        return !algorithm->SkipLayout() ||
            wrapper->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::GONE;
    }
    const auto& inner = algorithm->GetLayoutAlgorithm();
    if (algorithm->GetHasMeasured() || (inner && inner->GetHasMeasured())) {
        return true;
    }
    auto frame = AceType::DynamicCast<FrameNode>(wrapper);
    return frame && (frame->GetKitNode() || frame->GetExtensionHandler()) && !frame->IsLayoutDirtyMarked();
}

std::optional<LazyPremeasureResult> LazyLayoutUtils::Premeasure(
    const RefPtr<LayoutWrapper>& wrapper, const LayoutConstraintF& constraint,
    std::vector<WeakPtr<FrameNode>>* measuredNodes)
{
    CHECK_NULL_RETURN(wrapper, std::nullopt);
    if (!constraint.viewPosRef || constraint.viewPosRef->deadline ||
        constraint.viewPosRef->axis != Axis::VERTICAL) {
        return std::nullopt;
    }
    const auto& ref = constraint.viewPosRef.value();
    if (!std::isfinite(ref.viewPosStart) || !std::isfinite(ref.viewPosEnd) ||
        !std::isfinite(ref.referencePos) || !std::isfinite(ref.viewExtStart) ||
        !std::isfinite(ref.viewExtEnd) || GreatNotEqualCustomPrecision(ref.viewPosStart, ref.viewPosEnd, 0.0)) {
        return std::nullopt;
    }
    auto host = wrapper->GetHostNode();
    CHECK_NULL_RETURN(host, std::nullopt);
    if (!GetLazyLayoutPattern(host)) {
        return std::nullopt;
    }
    auto measured = AceType::DynamicCast<LazyPremeasureWrapper>(wrapper);
    if (!measured) {
        measured = AceType::MakeRefPtr<LazyPremeasureWrapper>(wrapper);
    }
    measured->Measure(constraint);
    if (measuredNodes) {
        measured->CollectMeasuredFrameNodes(*measuredNodes);
    }
    return measured->GetResult();
}

void LazyLayoutPattern::RestorePreparedGeometry(const std::shared_ptr<const PreparedLayoutGeometry>& geometry)
{
    if (!geometry || GetPreparedGeometry() == geometry || !ApplyPreparedGeometry(*geometry)) {
        return;
    }
    if (!preparedState_) {
        preparedState_ = std::make_unique<PreparedLayoutState>();
    }
    preparedState_->geometry = geometry;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF);
}

std::shared_ptr<const PreparedLayoutGeometry> LazyLayoutPattern::GetPreparedGeometry() const
{
    return preparedState_ ? preparedState_->geometry.lock() : nullptr;
}

LayoutWrapper* LazyLayoutPattern::SwapPremeasureWrapper(LayoutWrapper* wrapper)
{
    auto previous = GetPremeasureWrapper();
    if (!preparedState_) {
        if (!wrapper) {
            return previous;
        }
        preparedState_ = std::make_unique<PreparedLayoutState>();
    }
    preparedState_->premeasureWrapper = wrapper;
    if (!wrapper && preparedState_->geometry.expired()) {
        preparedState_.reset();
    }
    return previous;
}

void LazyLayoutPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    if (IsLazyLayoutEnabled()) {
        // needLazyLayout is a monotonic capability marker. Publish it before OnAttachToMainTree so a
        // scrollable parent can recognize cached LazyForEach hosts during its first prediction pass.
        layoutProperty->SetNeedLazyLayout(true);
    }
}

void LazyLayoutPattern::AfterMountToParent()
{
    if (!IsLazyLayoutEnabled()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto parent = host->GetParent();
    while (parent) {
        auto parentFrameNode = AceType::DynamicCast<FrameNode>(parent);
        if (!parentFrameNode) {
            parent = parent->GetParent();
            continue;
        }
        if (!LazyLayoutUtils::IsAllowedIntermediateNode(parentFrameNode)) {
            return;
        }
        auto layoutProperty = parentFrameNode->GetLayoutProperty();
        CHECK_NULL_VOID(layoutProperty);
        layoutProperty->SetNeedLazyLayout(true);
        parent = parent->GetParent();
    }
}

bool LazyLayoutPattern::SetNextStickyHeaderGap(const std::optional<float>& gap)
{
    const bool unchanged = nextStickyHeaderGap_.has_value() == gap.has_value() &&
                           (!gap.has_value() || NearEqual(nextStickyHeaderGap_.value(), gap.value()));
    if (unchanged) {
        return false;
    }
    nextStickyHeaderGap_ = gap;
    return true;
}

void LazyLayoutPattern::PublishNextStickyHeaderGap(const std::optional<float>& gap)
{
    if (!SetNextStickyHeaderGap(gap)) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = host->GetContext();
    if (pipeline && pipeline->IsLayouting()) {
        pipeline->AddAfterLayoutTask([weakHost = WeakPtr<FrameNode>(host)]() {
            auto node = weakHost.Upgrade();
            CHECK_NULL_VOID(node);
            node->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        });
        return;
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void LazyLayoutPattern::NotifyParentOnStickyHeaderChange()
{
    const bool hasStickyHeader = HasStickyHeader();
    if (!lastNotifiedHasStickyHeader_.has_value()) {
        lastNotifiedHasStickyHeader_ = hasStickyHeader;
        return;
    }
    if (lastNotifiedHasStickyHeader_.value() == hasStickyHeader) {
        return;
    }
    lastNotifiedHasStickyHeader_ = hasStickyHeader;

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto parent = host->GetAncestorNodeOfFrame(true);
    while (parent) {
        if (parent->GetPattern<LazyLayoutPattern>()) {
            parent->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
            return;
        }
        auto layoutProperty = parent->GetLayoutProperty();
        if (!layoutProperty || !layoutProperty->GetNeedLazyLayout()) {
            return;
        }
        parent = parent->GetAncestorNodeOfFrame(true);
    }
}

void LazyLayoutPattern::OnDetachFromMainTree()
{
    nextStickyHeaderGap_.reset();
    lastNotifiedHasStickyHeader_.reset();
    Pattern::OnDetachFromMainTree();
}

void LazyLayoutPattern::DumpStickyHeaderHandoffInfo() const
{
    const auto gap = nextStickyHeaderGap_.has_value() ? std::to_string(nextStickyHeaderGap_.value()) : "null";
    DumpLog::GetInstance().AddDesc("nextStickyHeaderGap:" + gap);
}

void LazyLayoutPattern::DumpStickyHeaderHandoffInfo(std::unique_ptr<JsonValue>& json) const
{
    CHECK_NULL_VOID(json);
    if (nextStickyHeaderGap_.has_value()) {
        json->Put("nextStickyHeaderGap", nextStickyHeaderGap_.value());
        return;
    }
    json->Put("nextStickyHeaderGap", "null");
}

} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.h"

#include "core/components_ng/pattern/scrollable/scrollable_paint_property.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "core/components_ng/pattern/waterflow/layout/water_flow_layout_utils.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
std::pair<float, float> GetMainStartEnd(const RectF& rect, Axis axis, bool isReverse, float frameMainSize)
{
    const float start = axis == Axis::VERTICAL ? rect.Top() : rect.Left();
    const float size = rect.GetSize().MainSize(axis);
    if (isReverse) {
        const float end = frameMainSize - start;
        return { end - size, end };
    }
    return { start, start + size };
}

RectF GetPaddingRectInFrame(const RefPtr<GeometryNode>& geo)
{
    CHECK_NULL_RETURN(geo, RectF());
    auto size = geo->GetFrameSize();
    OffsetF offset;
    const auto& padding = geo->GetPadding();
    if (padding) {
        MinusPaddingToSize(*padding, size);
        offset = OffsetF(padding->left.value_or(0.0f), padding->top.value_or(0.0f));
    }
    return RectF { offset, size };
}

std::optional<RectF> ResolveCustomClipRect(
    const RefPtr<ShapeRect>& shape, const ScaleProperty& scale, const SizeF& frameSize, Axis axis)
{
    CHECK_NULL_RETURN(shape, std::nullopt);
    const float mainSize = frameSize.MainSize(axis);
    if (GreaterOrEqualToInfinity(mainSize)) {
        return std::nullopt;
    }

    float x = ConvertToPx(shape->GetOffset().GetX(), scale, frameSize.Width()).value_or(0.0f);
    float y = ConvertToPx(shape->GetOffset().GetY(), scale, frameSize.Height()).value_or(0.0f);
    float width = ConvertToPx(shape->GetWidth(), scale, frameSize.Width()).value_or(0.0f);
    float height = ConvertToPx(shape->GetHeight(), scale, frameSize.Height()).value_or(0.0f);
    return RectF { x, y, width, height };
}

std::optional<RectF> ResolveContentClipRect(
    ContentClipMode mode, const RefPtr<GeometryNode>& geo, const std::optional<ExpandEdges>& safeAreaPad)
{
    CHECK_NULL_RETURN(geo, std::nullopt);
    switch (mode) {
        case ContentClipMode::CONTENT_ONLY: {
            return GetPaddingRectInFrame(geo);
        }
        case ContentClipMode::SAFE_AREA: {
            if (!safeAreaPad.has_value()) {
                return std::nullopt;
            }
            auto rect = GetPaddingRectInFrame(geo);
            auto size = rect.GetSize();
            AddPaddingToSize(safeAreaPad.value(), size);

            auto offset = rect.GetOffset();
            offset -= OffsetF(safeAreaPad->left.value_or(0.0f), safeAreaPad->top.value_or(0.0f));
            return RectF { offset, size };
        }
        case ContentClipMode::BOUNDARY: {
            return RectF { OffsetF(), geo->GetFrameSize() };
        }
        default:
            return std::nullopt;
    }
}

std::optional<RectF> ResolveClipRect(const RefPtr<WaterFlowLayoutProperty>& layoutProperty,
    const RefPtr<GeometryNode>& geometryNode, ContentClipMode mode, const RefPtr<ShapeRect>& clipShapeRect,
    const std::optional<ExpandEdges>& safeAreaPad)
{
    CHECK_NULL_RETURN(layoutProperty, std::nullopt);
    CHECK_NULL_RETURN(geometryNode, std::nullopt);
    if (mode == ContentClipMode::CUSTOM) {
        const auto& constraint = layoutProperty->GetLayoutConstraint();
        CHECK_NULL_RETURN(constraint, std::nullopt);
        return ResolveCustomClipRect(clipShapeRect, constraint->scaleProperty, geometryNode->GetFrameSize(),
            layoutProperty->GetAxis());
    }
    return ResolveContentClipRect(mode, geometryNode, safeAreaPad);
}

void ApplyContentClipFixOffset(const RectF& clipRect, const RefPtr<WaterFlowLayoutProperty>& layoutProperty,
    const RefPtr<GeometryNode>& geometryNode, const RefPtr<WaterFlowLayoutInfoBase>& info)
{
    auto contentRect = GetPaddingRectInFrame(geometryNode);
    const auto axis = layoutProperty->GetAxis();
    const float frameMainSize = geometryNode->GetFrameSize().MainSize(axis);
    auto [contentStart, contentEnd] =
        GetMainStartEnd(contentRect, axis, layoutProperty->IsReverse(), frameMainSize);
    auto [clipStart, clipEnd] = GetMainStartEnd(clipRect, axis, layoutProperty->IsReverse(), frameMainSize);

    if (GreatNotEqual(contentStart, clipStart)) {
        info->startFixOffset_ = contentStart - clipStart;
    }
    const float contentEndWithExpand = contentEnd + info->expandHeight_;
    if (GreatNotEqual(clipEnd, contentEndWithExpand)) {
        info->endFixOffset_ = clipEnd - contentEndWithExpand;
    }
}
} // namespace

void WaterFlowLayoutBase::PreloadItems(
    LayoutWrapper* host, const RefPtr<WaterFlowLayoutInfoBase>& info, int32_t cacheCount)
{
    if (cacheCount <= 0) {
        return;
    }
    auto frameNode = host->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<WaterFlowPattern>();
    CHECK_NULL_VOID(pattern);
    const bool taskRegistered = !pattern->PreloadListEmpty();
    pattern->SetPreloadList(GeneratePreloadList(info, host, cacheCount, false));
    if (pattern->PreloadListEmpty()) {
        return;
    }

    pattern->SetCacheLayoutAlgo(Claim(this));
    if (taskRegistered) {
        return;
    }
    PostIdleTask(frameNode);
}

void WaterFlowLayoutBase::SyncPreloadItems(
    LayoutWrapper* host, const RefPtr<WaterFlowLayoutInfoBase>& info, int32_t cacheCount)
{
    auto list = GeneratePreloadList(info, host, cacheCount, true);
    if (list.empty()) {
        return;
    }

    StartCacheLayout();
    for (auto&& item : list) {
        SyncPreloadItem(host, item);
    }
    EndCacheLayout();
}

std::list<int32_t> WaterFlowLayoutBase::GeneratePreloadList(
    const RefPtr<WaterFlowLayoutInfoBase>& info, LayoutWrapper* host, int32_t cacheCount, bool force)
{
    std::list<int32_t> preloadList;
    if (info->startIndex_ > info->endIndex_) {
        return preloadList;
    }
    const int32_t endBound = std::min(info->ItemCnt(info->GetChildrenCount()) - 1, info->endIndex_ + cacheCount);
    for (int32_t i = info->endIndex_ + 1; i <= endBound; ++i) {
        if (force || !host->GetChildByIndex(info->NodeIdx(i), true)) {
            preloadList.emplace_back(i);
        }
    }

    const int32_t startBound = std::max(0, info->startIndex_ - cacheCount);
    for (int32_t i = info->startIndex_ - 1; i >= startBound; --i) {
        if (force || !host->GetChildByIndex(info->NodeIdx(i), true)) {
            preloadList.emplace_back(i);
        }
    }
    return preloadList;
}

void WaterFlowLayoutBase::PostIdleTask(const RefPtr<FrameNode>& frameNode)
{
    ACE_UINODE_TRACE(frameNode);
    auto* context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    context->AddPredictTask([weak = WeakPtr(frameNode)](int64_t deadline, bool canUseLongPredictTask) {
        auto host = weak.Upgrade();
        CHECK_NULL_VOID(host);
        auto pattern = host->GetPattern<WaterFlowPattern>();
        CHECK_NULL_VOID(pattern);

        auto&& algo = pattern->GetCacheLayoutAlgo();
        CHECK_NULL_VOID(algo);
        algo->StartCacheLayout();

        bool needMarkDirty = false;
        auto items = pattern->MovePreloadList();
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (GetSysTimestamp() > deadline) {
                pattern->SetPreloadList(std::list<int32_t>(it, items.end()));
                PostIdleTask(host);
                break;
            }
            ACE_SCOPED_TRACE("Preload FlowItem %d", *it);
            ScopedLayout scope(host->GetContext());
            needMarkDirty |= algo->PreloadItem(RawPtr(host), *it, deadline);
        }
        if (needMarkDirty) {
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        }
        algo->EndCacheLayout();
    });
}

int32_t WaterFlowLayoutBase::GetUpdateIdx(LayoutWrapper* host, int32_t footerIdx)
{
    auto layoutHost = host->GetHostNode();
    CHECK_NULL_RETURN(layoutHost, 0);
    int32_t updateIdx = layoutHost->GetChildrenUpdated();
    auto pattern = layoutHost->GetPattern<WaterFlowPattern>();
    if (pattern && pattern->GetEdgeState().enabled && updateIdx >= 0) {
        return std::max(0, pattern->GetEdgeIndexMap().NodeToData(updateIdx));
    }
    if (updateIdx > 0 && footerIdx == 0) {
        --updateIdx;
    }
    return updateIdx;
}

void WaterFlowLayoutBase::UpdateOverlay(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto paintProperty = frameNode->GetPaintProperty<ScrollablePaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    if (!paintProperty->GetFadingEdge().value_or(false)) {
        return;
    }
    auto overlayNode = frameNode->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto overlayGeometryNode = overlayNode->GetGeometryNode();
    CHECK_NULL_VOID(overlayGeometryNode);
    overlayGeometryNode->SetFrameSize(geometryNode->GetFrameSize(true));
}

void WaterFlowLayoutBase::GetExpandArea(
    const RefPtr<WaterFlowLayoutProperty>& layoutProperty, const RefPtr<WaterFlowLayoutInfoBase>& info)
{
    auto&& safeAreaOpts = layoutProperty->GetSafeAreaExpandOpts();
    expandSafeArea_ = safeAreaOpts && safeAreaOpts->Expansive();
    info->expandHeight_ = ScrollableUtils::CheckHeightExpansion(layoutProperty, layoutProperty->GetAxis());
}

void WaterFlowLayoutBase::CalculateContentClipFixOffset(
    LayoutWrapper* layoutWrapper, const RefPtr<WaterFlowLayoutInfoBase>& info)
{
    CHECK_NULL_VOID(layoutWrapper);
    CHECK_NULL_VOID(info);
    info->startFixOffset_ = 0.0f;
    info->endFixOffset_ = 0.0f;
    if (contentClipMode_ == ContentClipMode::CONTENT_ONLY) {
        return;
    }

    PostClipContentSafeAreaBundle(layoutWrapper);
    if (contentClipMode_ == ContentClipMode::SAFE_AREA && !safeAreaPad_.has_value()) {
        return;
    }

    auto layoutProperty = AceType::DynamicCast<WaterFlowLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);

    auto clipRect = ResolveClipRect(layoutProperty, geometryNode, contentClipMode_, clipShapeRect_, safeAreaPad_);
    if (!clipRect.has_value()) {
        return;
    }
    ApplyContentClipFixOffset(clipRect.value(), layoutProperty, geometryNode, info);
}

void WaterFlowLayoutBase::PostClipContentSafeAreaBundle(LayoutWrapper* layoutWrapper) const
{
    if (contentClipMode_ != ContentClipMode::SAFE_AREA) {
        return;
    }
    CHECK_NULL_VOID(layoutWrapper);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    host->PostBundle({}, false, LayoutSafeAreaBundleType::CONTENT_CLIP_SAFE_AREA);
}

ViewPosReference WaterFlowLayoutBase::CreateLazyChildViewPosReference(const RefPtr<WaterFlowLayoutInfoBase>& info,
    float mainSize, float referencePos, ReferenceEdge referenceEdge, Axis axis, std::optional<int64_t> deadline,
    bool useInfinityWhenPositionCalc) const
{
    CHECK_NULL_RETURN(info, ViewPosReference {});
    return {
        .viewPosStart = 0,
        // Lazy layout adds viewExtEnd to viewPosEnd internally; keep viewPosEnd at the report bound.
        .viewPosEnd = useInfinityWhenPositionCalc && info->duringPositionCalc_ ? LayoutInfinity<float>()
                                                                                : info->GetViewEndBound(mainSize, true),
        .viewExtStart = info->startFixOffset_,
        .viewExtEnd = info->endFixOffset_,
        .referencePos = referencePos,
        .referenceEdge = referenceEdge,
        .axis = axis,
        .deadline = deadline,
    };
}

bool WaterFlowLayoutBase::ShouldReanchorLazyChildToStart(
    double previousPos, double currentPos, float startBoundary, bool isCacheLayout) const
{
    return !isCacheLayout && !canOverScrollStart_ && LessOrEqual(previousPos, startBoundary) &&
           GreatNotEqual(currentPos, startBoundary);
}

bool WaterFlowLayoutBase::AdjustLazyChildOffset(double& currentOffset, float contentStartOffset, float adjustStart,
    float adjustEnd, bool includeEndOffset, bool isCacheLayout) const
{
    const double previousOffset = currentOffset;
    currentOffset -= adjustStart;
    currentOffset -= includeEndOffset ? adjustEnd : 0.0f;
    const bool shouldReanchor =
        ShouldReanchorLazyChildToStart(previousOffset, currentOffset, contentStartOffset, isCacheLayout);
    if (shouldReanchor) {
        currentOffset = contentStartOffset;
    }
    return shouldReanchor;
}

void WaterFlowLayoutBase::ConsumeLazyChildReanchorOffset(const RefPtr<LayoutWrapper>& child, int32_t index) const
{
    auto adjustOffset = WaterFlowLayoutUtils::GetAdjustOffset(child);
    if (NearZero(adjustOffset.start) && NearZero(adjustOffset.end)) {
        return;
    }
    TAG_LOGW(AceLogTag::ACE_WATERFLOW,
        "Lazy child re-anchor did not converge. index:%{public}d, start:%{public}f, end:%{public}f", index,
        adjustOffset.start, adjustOffset.end);
}

void WaterFlowLayoutBase::InitUnlayoutedItems()
{
    if (isLayouted_) {
        return;
    }

    prevStartIndex_ = LayoutInfo()->StartIndex();
    prevEndIndex_ = LayoutInfo()->EndIndex();
}

void WaterFlowLayoutBase::ClearUnlayoutedItems(LayoutWrapper* layoutWrapper)
{
    if (prevStartIndex_ < 0 || prevEndIndex_ < 0) {
        return;
    }

    for (int32_t idx = prevStartIndex_; idx <= prevEndIndex_; ++idx) {
        // Skip if index is within current layout range
        if (idx >= measuredStartIndex_ && idx <= measuredEndIndex_) {
            continue;
        }

        // Get child wrapper by index
        auto wrapper = layoutWrapper->GetChildByIndex(LayoutInfo()->NodeIdx(idx));
        CHECK_NULL_CONTINUE(wrapper);

        // Clear layout algorithm for frame node
        auto frameNode = AceType::DynamicCast<FrameNode>(wrapper);
        CHECK_NULL_CONTINUE(frameNode);
        frameNode->ClearSubtreeLayoutAlgorithm();
    }
}

void WaterFlowLayoutBase::PrepareParentLayout(LayoutWrapper* wrapper,
    const RefPtr<WaterFlowLayoutInfoBase>& info, SizeF& size, bool matchChildren)
{
    auto property = wrapper->GetLayoutProperty();
    auto constraint = property->GetContentLayoutConstraint();
    auto reference = constraint ? constraint->viewPosRef : std::nullopt;
    const bool enabled = info->edgeSlots_ && matchChildren && property->GetNeedLazyLayout() &&
        reference.has_value() && reference->axis == Axis::VERTICAL &&
        !GreaterOrEqualToInfinity(reference->viewPosEnd - reference->viewPosStart);
    if (enabled != info->parentDriven_) {
        info->Reset();
        info->UpdateOffset(-info->Offset());
        info->parentAdjustOffset_ = {};
        if (enabled) {
            info->jumpIndex_ = WaterFlowLayoutInfoBase::EMPTY_JUMP_INDEX;
            info->startIndex_ = 0;
            info->endIndex_ = -1;
        }
    }
    info->parentDriven_ = enabled;
    info->parentDeadline_ = enabled ? reference->deadline : std::nullopt;
    if (!enabled) {
        return;
    }
    auto range = LazyLayoutUtils::CalculateViewRange(*reference, info->parentMainSize_);
    info->parentViewStart_ = range.start;
    info->parentViewEnd_ = range.end;
    // Even a new offscreen host needs a bounded sample to establish its estimated extent.
    size.SetHeight(std::max(range.end, 1.0f));
    info->parentViewEnd_ = size.Height();
    canOverScrollStart_ = true;
    canOverScrollEnd_ = true;
}

bool WaterFlowLayoutBase::FinishParentLayout(
    LayoutWrapper* wrapper, const RefPtr<WaterFlowLayoutInfoBase>& info, SizeF size)
{
    if (!info->parentDriven_) {
        return false;
    }
    const float extent = std::max(info->EstimateTotalHeight(), info->headerHeight_ + info->edgeFooterHeight_) +
        info->contentStartOffset_ + info->contentEndOffset_;
    info->parentMainSize_ = extent;
    size.SetHeight(extent);
    AddPaddingToSize(wrapper->GetLayoutProperty()->CreatePaddingAndBorder(), size);
    wrapper->GetGeometryNode()->SetFrameSize(size);
    return true;
}

void WaterFlowLayoutBase::MeasureEdges(
    LayoutWrapper* wrapper, const RefPtr<WaterFlowLayoutInfoBase>& info, const SizeF& size)
{
    if (!info->edgeSlots_) {
        return;
    }
    auto props = AceType::DynamicCast<WaterFlowLayoutProperty>(wrapper->GetLayoutProperty());
    CHECK_NULL_VOID(props);
    auto host = wrapper->GetHostNode();
    auto pattern = host ? host->GetPattern<WaterFlowPattern>() : nullptr;
    CHECK_NULL_VOID(pattern);
    const bool supported = props->GetWaterflowDirectionValue(FlexDirection::COLUMN) == FlexDirection::COLUMN;
    auto constraint = props->CreateChildConstraint();
    const auto axis = props->GetAxis();
    constraint.maxSize.SetMainSize(LayoutInfinity<float>(), axis);
    if (axis == Axis::VERTICAL) {
        constraint.maxSize.SetWidth(size.Width());
        constraint.percentReference.SetWidth(size.Width());
        constraint.selfIdealSize.SetWidth(size.Width());
    } else {
        constraint.maxSize.SetHeight(size.Height());
        constraint.percentReference.SetHeight(size.Height());
        constraint.selfIdealSize.SetHeight(size.Height());
    }
    const auto headerHeight = supported ? HeaderFooterUtils::MeasureHeaderFooter(
        wrapper, info->edgeIndexMap_.HeaderIndex(), constraint, axis, MeasureType::MATCH_CONTENT) : 0.0f;
    const bool showFooter = supported || !pattern->GetEdgeState().footerControlled;
    info->edgeFooterHeight_ = showFooter ? HeaderFooterUtils::MeasureHeaderFooter(
        wrapper, info->edgeIndexMap_.FooterIndex(), constraint, axis, MeasureType::MATCH_CONTENT) : 0.0f;
    info->footerHeight_ = info->edgeFooterHeight_;
    if (!NearEqual(headerHeight, info->headerHeight_)) {
        // Cached item positions are body-local and must be invalidated when the prefix changes.
        if (info->parentDriven_ && Positive(info->parentViewStart_)) {
            info->parentAdjustOffset_.start += HeaderFooterUtils::CalcHeaderResizeAdjust(
                headerHeight - info->headerHeight_, info->parentViewStart_ - info->headerHeight_, info->headerHeight_);
        }
        info->Reset();
        if (info->parentDriven_) {
            info->jumpIndex_ = WaterFlowLayoutInfoBase::EMPTY_JUMP_INDEX;
            info->startIndex_ = 0;
            info->endIndex_ = -1;
        }
        info->headerHeight_ = headerHeight;
    }
    if (pattern->GetSections() && info->segmentTails_.empty()) {
        info->InitSegments(pattern->GetSections()->GetSectionInfo(), 0);
    }
}

void WaterFlowLayoutBase::LayoutEdges(
    LayoutWrapper* wrapper, const RefPtr<WaterFlowLayoutInfoBase>& info, float bodyEnd)
{
    if (!info->edgeSlots_) {
        return;
    }
    auto host = wrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<WaterFlowPattern>();
    CHECK_NULL_VOID(pattern);
    auto props = AceType::DynamicCast<WaterFlowLayoutProperty>(wrapper->GetLayoutProperty());
    CHECK_NULL_VOID(props);
    auto padding = props->CreatePaddingAndBorder();
    if (props->GetWaterflowDirectionValue(FlexDirection::COLUMN) != FlexDirection::COLUMN) {
        auto header = info->edgeIndexMap_.hasHeader
            ? wrapper->GetChildByIndex(info->edgeIndexMap_.HeaderIndex()) : nullptr;
        if (header) {
            header->SetActive(false);
        }
        auto footer = info->edgeIndexMap_.hasFooter
            ? wrapper->GetChildByIndex(info->edgeIndexMap_.FooterIndex()) : nullptr;
        if (footer) {
            if (pattern->GetEdgeState().footerControlled) {
                footer->SetActive(false);
            } else {
                auto mainPos = bodyEnd + info->Offset();
                if (props->IsReverse()) {
                    mainPos = info->lastMainSize_ - info->edgeFooterHeight_ - mainPos;
                }
                const auto axis = props->GetAxis();
                auto offset = axis == Axis::VERTICAL ? OffsetF(0.0f, mainPos) : OffsetF(mainPos, 0.0f);
                offset += OffsetF(padding.left.value_or(0.0f), padding.top.value_or(0.0f));
                footer->GetGeometryNode()->SetMarginFrameOffset(offset);
                footer->SetActive(true);
                footer->Layout();
            }
        }
        return;
    }
    OffsetF origin(padding.left.value_or(0.0f), padding.top.value_or(0.0f) + info->Offset());
    const float crossSize = wrapper->GetGeometryNode()->GetPaddingSize().Width();
    HeaderFooterUtils::LayoutEdge(wrapper, info->edgeIndexMap_.HeaderIndex(), origin, false, false, crossSize);
    bodyEnd = std::max(bodyEnd, info->headerHeight_);
    if (info->parentDriven_ && info->EndIndex() < info->ItemCnt(info->GetChildrenCount()) - 1) {
        // The footer follows the estimated full body, not the tail of the currently measured window.
        bodyEnd = std::max(bodyEnd, info->EstimateTotalHeight() - info->edgeFooterHeight_);
    }
    HeaderFooterUtils::LayoutEdge(wrapper, info->edgeIndexMap_.FooterIndex(), origin + OffsetF(0.0f, bodyEnd),
        false, false, crossSize);
    pattern->SetEdgeGeometry(origin, bodyEnd + info->edgeFooterHeight_);
}

void WaterFlowLayoutBase::CalcContentOffset(
    LayoutWrapper* layoutWrapper, const RefPtr<WaterFlowLayoutInfoBase>& info, float mainSize)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto property = AceType::DynamicCast<WaterFlowLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(property);
    auto startOffset = property->GetContentStartOffset();
    if (!startOffset.has_value()) {
        info->contentStartOffset_ = 0.0f;
    }
    auto endOffset = property->GetContentEndOffset();
    if (!endOffset.has_value()) {
        info->contentEndOffset_ = 0.0f;
    }
    if (!endOffset && !startOffset) {
        return;
    }
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto pipeline = host->GetContext();
    CHECK_NULL_VOID(pipeline);
    if (startOffset) {
        info->contentStartOffset_ =
            std::max(pipeline->NormalizeToPx(Dimension(startOffset.value(), DimensionUnit::VP)), 0.0);
    }
    if (endOffset) {
        info->contentEndOffset_ =
            std::max(pipeline->NormalizeToPx(Dimension(endOffset.value(), DimensionUnit::VP)), 0.0);
    }
    if (GreatOrEqual(info->contentStartOffset_ + info->contentEndOffset_, mainSize)) {
        info->contentStartOffset_ = 0.0f;
        info->contentEndOffset_ = 0.0f;
    }
}
} // namespace OHOS::Ace::NG

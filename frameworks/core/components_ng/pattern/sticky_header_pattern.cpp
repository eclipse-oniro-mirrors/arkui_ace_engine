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

#include "core/components_ng/pattern/sticky_header_pattern.h"

#include <array>
#include <cmath>
#include <limits>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/scroll_viewport_coordinator.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_pattern.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scrollable/header_footer_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr char PARENT_LIST_ETS_TAG[] = "List";
constexpr char PARENT_GRID_ETS_TAG[] = "Grid";

bool AdvanceStickyTransform(const RefPtr<FrameNode>& node, Matrix4& contentToAncestor)
{
    auto render = node->GetRenderContext();
    CHECK_NULL_RETURN(render, false);
    const auto offset = render->GetPaintRectWithoutTransform().GetOffset();
    auto transform = render->GetMatrixWithTransformRotate();
    contentToAncestor = transform * Matrix4::CreateTranslate(offset.GetX(), offset.GetY(), 0.0f) * contentToAncestor;
    for (int32_t i = 0; i < contentToAncestor.Count(); ++i) {
        if (!std::isfinite(contentToAncestor[i])) {
            return false;
        }
    }
    const auto determinant = contentToAncestor.Determinant();
    // Matrix4::Invert returns identity for singular matrices, so validate before inversion.
    return std::isfinite(determinant) && !NearZero(determinant);
}

std::optional<std::pair<float, float>> ProjectStickyRange(const Matrix4& inverse, const RectF& rect)
{
    const std::array<OffsetF, 4> corners = { OffsetF(rect.Left(), rect.Top()), OffsetF(rect.Right(), rect.Top()),
        OffsetF(rect.Left(), rect.Bottom()), OffsetF(rect.Right(), rect.Bottom()) };
    float start = std::numeric_limits<float>::max();
    float end = -std::numeric_limits<float>::max();
    double firstW = 0.0;
    for (const auto& corner : corners) {
        const double source[] = { corner.GetX(), corner.GetY(), 0.0, 1.0 };
        double target[Matrix4::DIMENSION];
        inverse.MapScalars(source, target);
        const double w = target[3];
        if (!std::isfinite(w) || NearZero(w) || firstW * w < 0.0) {
            return std::nullopt;
        }
        firstW = w;
        const auto y = static_cast<float>(target[1] / w);
        if (!std::isfinite(y)) {
            return std::nullopt;
        }
        start = std::min(start, y);
        end = std::max(end, y);
    }
    return std::make_pair(start, end);
}

bool ResolveStickyViewport(const RefPtr<FrameNode>& host, const OffsetF& origin, HeaderFooterStickyMetrics& metrics)
{
    auto contentToAncestor = Matrix4::CreateTranslate(origin.GetX(), origin.GetY(), 0.0f);
    float viewStart = -std::numeric_limits<float>::max();
    float viewEnd = std::numeric_limits<float>::max();
    float pinStart = viewStart;
    float pinEnd = viewEnd;
    bool hasViewport = false;
    bool resolvedViewport = false;
    auto child = host;
    for (auto parent = child->GetParentFrameNode(); parent; parent = child->GetParentFrameNode()) {
        if (child->CheckTopWindowBoundary()) {
            break;
        }
        if (!AdvanceStickyTransform(child, contentToAncestor)) {
            return false;
        }
        const auto inverse = Matrix4::Invert(contentToAncestor);
        auto nested = parent->GetPattern<NestableScrollContainer>();
        auto scroll = AceType::DynamicCast<ScrollablePattern>(nested);
        const auto& tag = parent->GetTag();
        const bool boundary = nested && (tag != PARENT_GRID_ETS_TAG || (scroll && scroll->IsScrollable()));
        const bool eligible = boundary && scroll && scroll->GetAxis() == Axis::VERTICAL && !scroll->IsReverse() &&
            (tag == V2::SCROLL_ETS_TAG || tag == PARENT_LIST_ETS_TAG ||
             tag == PARENT_GRID_ETS_TAG || tag == V2::WATERFLOW_ETS_TAG);
        const auto size = parent->GetGeometryNode()->GetFrameSize();
        if (eligible && !resolvedViewport) {
            auto padding = parent->GetLayoutProperty()->CreatePaddingAndBorder();
            const float top = padding.top.value_or(0.0f);
            const float bottom = size.Height() - padding.bottom.value_or(0.0f);
            float insetStart = scroll->GetContentStartOffset();
            float insetEnd = scroll->GetContentEndOffset();
            const auto& constraint = host->GetLayoutProperty()->GetLayoutConstraint();
            if (constraint && constraint->viewPosRef && constraint->viewPosRef->axis == Axis::VERTICAL) {
                // The inherited values already include the scrolling parent's content offsets.
                insetStart = constraint->viewPosRef->stickyInsetStart;
                insetEnd = constraint->viewPosRef->stickyInsetEnd;
            }
            auto viewport = ProjectStickyRange(inverse, RectF(0.0f, top, size.Width(), bottom - top));
            auto pinTop = ProjectStickyRange(inverse, RectF(0.0f, top + insetStart, size.Width(), 0.0f));
            auto pinBottom = ProjectStickyRange(inverse, RectF(0.0f, bottom - insetEnd, size.Width(), 0.0f));
            CHECK_NULL_RETURN(viewport && pinTop && pinBottom, false);
            viewStart = std::max(viewStart, viewport->first);
            viewEnd = std::min(viewEnd, viewport->second);
            pinStart = pinTop->first;
            pinEnd = pinBottom->second;
            hasViewport = true;
        }
        resolvedViewport = resolvedViewport || boundary;
        auto render = parent->GetRenderContext();
        if (render && render->GetClipEdgeValue(false)) {
            auto clip = ProjectStickyRange(inverse, RectF(OffsetF(), size));
            CHECK_NULL_RETURN(clip, false);
            viewStart = std::max(viewStart, clip->first);
            viewEnd = std::min(viewEnd, clip->second);
        }
        child = parent;
    }
    metrics.viewStart = viewStart;
    metrics.viewEnd = viewEnd;
    metrics.stickyTopInset = std::max(0.0f, pinStart - viewStart);
    metrics.stickyBottomInset = std::max(0.0f, viewEnd - pinEnd);
    return hasViewport && viewEnd > viewStart;
}
} // namespace

StickyHeaderPattern::EdgeState::EdgeState() : sticky(StickyStyle::NONE) {}

RefPtr<FrameNode> StickyHeaderPattern::GetStickyHost() const
{
    auto pattern = AceType::DynamicCast<Pattern>(this);
    return pattern ? pattern->GetHost() : nullptr;
}

uint64_t StickyHeaderPattern::BeginEdgeUpdate(bool isHeader)
{
    (isHeader ? edgeState_.headerContent : edgeState_.footerContent).Reset();
    return edgeState_.BeginUpdate(isHeader);
}

bool StickyHeaderPattern::IsValidEdgeContent(bool isHeader, const RefPtr<UINode>& content) const
{
    CHECK_NULL_RETURN(content, true); // undefined removes the edge.
    auto host = GetStickyHost();
    CHECK_NULL_RETURN(host, false);
    // Reject cycles even when the candidate ancestor is currently detached.
    for (RefPtr<UINode> ancestor = host; ancestor; ancestor = ancestor->GetParent()) {
        if (ancestor == content) {
            return false;
        }
    }
    auto current = isHeader ? edgeState_.header.Upgrade() : GetStickyFooter();
    auto peer = isHeader ? GetStickyFooter() : edgeState_.header.Upgrade();
    if (content == peer || (peer && peer->GetFirstChild() == content)) {
        return false;
    }
    if (current && edgeState_.enabled && current->GetFirstChild() == content) {
        return true;
    }
    if (content->GetParent() || !HeaderFooterUtils::GetHeaderFooterFrameNode(content)) {
        return false;
    }
    return true;
}

void StickyHeaderPattern::PublishNextStickyHeaderGap(const std::optional<float>& gap)
{
    if (edgeState_.nextStickyHeaderGap == gap) {
        return;
    }
    edgeState_.nextStickyHeaderGap = gap;
    // The existing Pipeline callback consumes the geometry after parent layout; no extra measure request.
    SetEdgeGeometry(edgeState_.origin, edgeState_.totalSize, edgeState_.measuredTotalSize);
}

void StickyHeaderPattern::NotifyStickyHeaderChange()
{
    const bool hasStickyHeader = HasStickyHeader();
    if (!hasStickyHeader) {
        edgeState_.nextStickyHeaderGap.reset();
    }
    if (edgeState_.lastNotifiedHasStickyHeader == hasStickyHeader) {
        return;
    }
    edgeState_.lastNotifiedHasStickyHeader = hasStickyHeader;
    auto host = GetStickyHost();
    CHECK_NULL_VOID(host);
    auto parent = host->GetParentFrameNode();
    while (parent) {
        if (parent->GetPattern<LazyLayoutPattern>()) {
            parent->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
            return;
        }
        if (!LazyLayoutUtils::IsAllowedIntermediateNode(parent)) {
            return;
        }
        parent = parent->GetParentFrameNode();
    }
}

void StickyHeaderPattern::SetSticky(StickyStyle style)
{
    auto host = GetStickyHost();
    CHECK_NULL_VOID(host);
    // Attributes may arrive before an asynchronous builder; retain the style without opting in.
    edgeState_.sticky = style;
    NotifyStickyHeaderChange();
    if (!edgeState_.enabled) {
        return;
    }
    SetEdgeGeometry(edgeState_.origin, edgeState_.totalSize, edgeState_.measuredTotalSize);
    UpdateEdgePositions();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void StickyHeaderPattern::SetEdgeGeometry(const OffsetF& origin, float totalSize, std::optional<float> measuredTotalSize)
{
    if (!edgeState_.enabled) {
        return;
    }
    edgeState_.origin = origin;
    edgeState_.totalSize = totalSize;
    edgeState_.measuredTotalSize = measuredTotalSize.value_or(totalSize);
    auto host = GetStickyHost();
    CHECK_NULL_VOID(host);
    auto pipeline = host->GetContext();
    CHECK_NULL_VOID(pipeline);
    if (edgeState_.sticky == StickyStyle::NONE || (!edgeState_.header.Upgrade() && !GetStickyFooter())) {
        if (edgeState_.viewportRegistered) {
            pipeline->GetScrollViewportCoordinator()->Unregister(host->GetId());
        }
        edgeState_.viewportRegistered = false;
        return;
    }
    if (edgeState_.viewportRegistered) {
        return;
    }
    pipeline->GetScrollViewportCoordinator()->Register(host, [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->UpdateEdgePositions();
    });
    edgeState_.viewportRegistered = true;
}

void StickyHeaderPattern::DetachStickyEdges(FrameNode* frameNode)
{
    edgeState_.InvalidateGenerations();
    edgeState_.nextStickyHeaderGap.reset();
    edgeState_.lastNotifiedHasStickyHeader = false;
    auto pipeline = frameNode ? frameNode->GetContext() : nullptr;
    if (pipeline && edgeState_.viewportRegistered) {
        pipeline->GetScrollViewportCoordinator()->Unregister(frameNode->GetId());
    }
    edgeState_.viewportRegistered = false;
}

void StickyHeaderPattern::UpdateStickyEdgePositions(float headerHeight, float footerHeight)
{
    if (!edgeState_.enabled || (!edgeState_.header.Upgrade() && !GetStickyFooter())) {
        return;
    }
    auto host = GetStickyHost();
    CHECK_NULL_VOID(host);
    HeaderFooterStickyMetrics metrics;
    metrics.totalMainSize = edgeState_.measuredTotalSize;
    metrics.nextStickyHeaderGap = HasStickyHeader() ? edgeState_.nextStickyHeaderGap : std::nullopt;
    metrics.headerMainSize = headerHeight;
    metrics.footerMainSize = footerHeight;
    const bool canPin =
        edgeState_.sticky != StickyStyle::NONE && ResolveStickyViewport(host, edgeState_.origin, metrics);
    auto place = [this, canPin, &metrics](const RefPtr<UINode>& edge, bool header) {
        auto node = AceType::DynamicCast<FrameNode>(edge);
        CHECK_NULL_VOID(node);
        const bool sticky = canPin && (header ? HeaderFooterUtils::IsHeaderSticky(edgeState_.sticky)
                                             : HeaderFooterUtils::IsFooterSticky(edgeState_.sticky));
        float y = header ? 0.0f : std::max(metrics.headerMainSize,
            edgeState_.totalSize - metrics.footerMainSize);
        if (sticky) {
            y = header ? HeaderFooterUtils::CalcStickyHeaderPos(metrics)
                       : HeaderFooterUtils::CalcStickyFooterPos(metrics);
        }
        const auto offset = edgeState_.origin + OffsetF(0.0f, y);
        const bool moved = node->GetGeometryNode()->GetMarginFrameOffset() != offset;
        if (moved) {
            node->GetGeometryNode()->SetMarginFrameOffset(offset);
        }
        // The framework rank belongs to the wrapper; preserve an explicit child zIndex, including zero.
        auto* content = node->GetTotalChildCount() == 1 ? node->GetFrameNodeChildByIndexWithoutBuild(0) : nullptr;
        auto render = node->GetRenderContext();
        if (render) {
            auto userZ = content && content->GetRenderContext() ? content->GetRenderContext()->GetZIndex()
                                                              : std::nullopt;
            const int32_t rank = sticky ? (header ? HeaderFooterUtils::STICKY_HEADER_Z_INDEX
                                                 : HeaderFooterUtils::STICKY_FOOTER_Z_INDEX) : 0;
            render->UpdateZIndex(userZ.value_or(rank));
        }
        if (moved) {
            node->ForceSyncGeometryNode();
        }
    };
    place(edgeState_.header.Upgrade(), true);
    place(GetStickyFooter(), false);
}
} // namespace OHOS::Ace::NG

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

#include "core/components_ng/base/inspector_occlusion_collector.h"

#include <cmath>
#include <optional>
#include <string>
#include <unordered_set>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_occlusion_region.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr size_t MAX_SNAPSHOTS = 4096;
constexpr size_t MAX_DOMAINS = 16;
constexpr size_t MAX_DEPTH = 128;

bool IsFiniteOpacity(double opacity)
{
    return std::isfinite(opacity) && opacity >= 0.0 && opacity <= 1.0;
}

bool IsExternalOrScrollable(const std::string& tag)
{
    return tag == V2::WEB_ETS_TAG || tag == V2::XCOMPONENT_ETS_TAG || tag == V2::VIDEO_ETS_TAG ||
           tag == V2::FORM_ETS_TAG || tag == V2::PLUGIN_ETS_TAG || tag == V2::EMBEDDED_COMPONENT_ETS_TAG ||
           tag == V2::UI_EXTENSION_COMPONENT_TAG || tag == V2::UI_EXTENSION_SURFACE_TAG ||
           tag == V2::UI_EXTENSION_COMPONENT_ETS_TAG || tag == V2::SCROLL_ETS_TAG || tag == V2::LIST_ETS_TAG ||
           tag == V2::GRID_ETS_TAG || tag == V2::WATERFLOW_ETS_TAG;
}

bool HasUnsupportedGeometry(const RefPtr<RenderContext>& context)
{
    return !context || context->GetTransformMatrix().has_value() || context->GetTransform3DMatrix().has_value() ||
           context->GetTransformScale().has_value() || context->GetTransformRotate().has_value() ||
           context->GetTransformRotateAngle().has_value() || context->GetTransformTranslate().has_value() ||
           context->GetClipShape().has_value() || context->GetClipMask().has_value() ||
           context->GetBorderRadius().has_value() || context->GetProgressMask().has_value();
}

bool HasUnsupportedPaint(const RefPtr<RenderContext>& context)
{
    return context->GetAnimationsCount() != 0 || context->HasTransitionOutAnimation() ||
           context->IsGeometryTransitionAnimating() || context->GetForeground() || context->GetBackground() ||
           context->GetCustomBackground() || context->GetGraphics() || context->GetBorder() ||
           context->GetOuterBorder() || context->GetPreBorderWidth().has_value() ||
           context->GetPreBorderColor().has_value() || context->GetPreBackShadow().has_value() ||
           context->GetGradient() || context->GetOverlay() || context->GetMotion() || context->GetUnionEffect() ||
           context->GetEdgeLight() || context->GetPointLight() || context->GetTransitionAppearing() ||
           context->GetTransitionDisappearing();
}

bool CollectSnapshots(const RefPtr<FrameNode>& node, int32_t domainId, double parentOpacity,
    const std::optional<RectF>& parentClip, bool ancestorTrusted, size_t depth,
    std::vector<OcclusionPaintSnapshot>& snapshots)
{
    if (!node || depth >= MAX_DEPTH || snapshots.size() >= MAX_SNAPSHOTS || node->IsRenderTreeSyncPending() ||
        (depth > 0 && node->IsWindowBoundary())) {
        return false;
    }
    auto context = node->GetRenderContext();
    if (!context) {
        return false;
    }

    const auto ownOpacity = context->GetOpacity().value_or(1.0);
    if (!IsFiniteOpacity(ownOpacity) || !IsFiniteOpacity(parentOpacity)) {
        return false;
    }
    const auto inheritedOpacity = parentOpacity * ownOpacity;
    const bool simpleGeometry = !HasUnsupportedGeometry(context);
    const bool supportedPaint = !HasUnsupportedPaint(context);
    const bool supportedDomain = !IsExternalOrScrollable(node->GetTag());
    const bool trusted = ancestorTrusted && simpleGeometry && supportedDomain && !node->GetIsGeometryTransitionIn() &&
                         !node->GetIsUseTransitionAnimator();
    const auto frameRect = node->GetTransformRectRelativeToWindow();

    std::optional<RectF> effectiveClip = parentClip;
    if (node->IsWindowBoundary()) {
        effectiveClip = frameRect;
    }
    auto evidenceRect = frameRect;
    if (effectiveClip) {
        evidenceRect = evidenceRect.Constrain(*effectiveClip);
    }

    OcclusionPaintSnapshot snapshot;
    snapshot.nodeId = node->GetId();
    snapshot.domainId = domainId;
    snapshot.candidateEnvelope = frameRect;
    snapshot.opaqueEvidence = evidenceRect;
    snapshot.paintOrderTrusted = true;
    // A leaf has a bounded candidate if it is Blank or the render node clips
    // all of its paint to its frame bounds.
    snapshot.candidateTrusted = trusted && supportedPaint && node->GetFrameChildren().empty() &&
                                node->GetChildren().empty() &&
                                (node->GetTag() == V2::BLANK_ETS_TAG || context->GetClipEdge().value_or(false));
    if (auto background = context->GetBackgroundColor(); background && background->GetAlpha() > 0) {
        snapshot.finalOpacity = inheritedOpacity * static_cast<double>(background->GetAlpha()) / 255.0;
        snapshot.evidenceTrusted = trusted && supportedPaint && IsFiniteOpacity(snapshot.finalOpacity) &&
                                   snapshot.finalOpacity > 0.0 && !evidenceRect.IsEmpty();
    }
    snapshots.emplace_back(snapshot);

    if (context->GetClipEdge().value_or(false)) {
        effectiveClip = effectiveClip ? frameRect.Constrain(*effectiveClip) : frameRect;
    }
    for (const auto& weakChild : node->GetFrameChildren()) {
        auto child = weakChild.Upgrade();
        if (!CollectSnapshots(
                child, domainId, inheritedOpacity, effectiveClip, trusted && supportedPaint, depth + 1, snapshots)) {
            return false;
        }
    }
    return true;
}

bool CollectAncestorState(const RefPtr<FrameNode>& startNode, double& opacity, std::optional<RectF>& clip)
{
    if (startNode->IsWindowBoundary()) {
        return true;
    }
    auto ancestor = startNode->GetParentFrameNode();
    size_t depth = 0;
    while (ancestor) {
        if (++depth >= MAX_DEPTH || ancestor->IsRenderTreeSyncPending() || ancestor->GetIsGeometryTransitionIn() ||
            ancestor->GetIsUseTransitionAnimator() || IsExternalOrScrollable(ancestor->GetTag())) {
            return false;
        }
        auto context = ancestor->GetRenderContext();
        if (HasUnsupportedGeometry(context) || HasUnsupportedPaint(context)) {
            return false;
        }
        const auto ownOpacity = context->GetOpacity().value_or(1.0);
        if (!IsFiniteOpacity(ownOpacity)) {
            return false;
        }
        opacity *= ownOpacity;
        if (context->GetClipEdge().value_or(false) || ancestor->IsWindowBoundary() ||
            ancestor->GetTag() == V2::ROOT_ETS_TAG) {
            auto rect = ancestor->GetTransformRectRelativeToWindow();
            clip = clip ? rect.Constrain(*clip) : rect;
        }
        if (ancestor->IsWindowBoundary()) {
            break;
        }
        ancestor = ancestor->GetParentFrameNode();
    }
    return true;
}
} // namespace

InspectorOcclusionCollector::Decisions InspectorOcclusionCollector::Evaluate(
    const std::vector<OcclusionPaintSnapshot>& paintOrder, double threshold)
{
    Decisions decisions;
    if (!IsFiniteOpacity(threshold) || paintOrder.size() > MAX_SNAPSHOTS) {
        return decisions;
    }
    std::unordered_set<int32_t> unknownDomains;
    std::unordered_map<int32_t, int32_t> firstDomainByNode;
    for (const auto& snapshot : paintOrder) {
        if (!snapshot.paintOrderTrusted) {
            unknownDomains.insert(snapshot.domainId);
        }
        const auto [first, inserted] = firstDomainByNode.emplace(snapshot.nodeId, snapshot.domainId);
        if (!inserted) {
            unknownDomains.insert(first->second);
            unknownDomains.insert(snapshot.domainId);
        }
    }
    std::unordered_map<int32_t, OcclusionRegionSet> coveredByDomain;
    for (auto it = paintOrder.rbegin(); it != paintOrder.rend(); ++it) {
        const auto& snapshot = *it;
        if (unknownDomains.find(snapshot.domainId) != unknownDomains.end()) {
            decisions[snapshot.nodeId] = OcclusionDecision::UNKNOWN;
            continue;
        }
        auto domain = coveredByDomain.find(snapshot.domainId);
        if (domain == coveredByDomain.end()) {
            if (coveredByDomain.size() >= MAX_DOMAINS) {
                decisions[snapshot.nodeId] = OcclusionDecision::UNKNOWN;
                continue;
            }
            domain = coveredByDomain.emplace(snapshot.domainId, OcclusionRegionSet()).first;
        }
        auto& covered = domain->second;
        if (!snapshot.candidateTrusted || covered.IsLimitReached()) {
            decisions[snapshot.nodeId] = OcclusionDecision::UNKNOWN;
        } else {
            const bool covers = covered.Covers(snapshot.candidateEnvelope);
            decisions[snapshot.nodeId] = covered.IsLimitReached()
                                             ? OcclusionDecision::UNKNOWN
                                             : (covers ? OcclusionDecision::OCCLUDED : OcclusionDecision::VISIBLE);
        }
        if (snapshot.evidenceTrusted && IsFiniteOpacity(snapshot.finalOpacity) && snapshot.finalOpacity > 0.0 &&
            snapshot.finalOpacity >= threshold && !covered.IsLimitReached()) {
            covered.Add(snapshot.opaqueEvidence);
        }
    }
    return decisions;
}

InspectorOcclusionCollector::Decisions InspectorOcclusionCollector::Collect(
    const RefPtr<FrameNode>& startNode, double threshold, int32_t domainId)
{
    if (!startNode || !IsFiniteOpacity(threshold)) {
        return {};
    }
    double parentOpacity = 1.0;
    std::optional<RectF> parentClip;
    if (!CollectAncestorState(startNode, parentOpacity, parentClip)) {
        return {};
    }
    std::vector<OcclusionPaintSnapshot> snapshots;
    if (!CollectSnapshots(startNode, domainId, parentOpacity, parentClip, true, 0, snapshots)) {
        return {};
    }
    return Evaluate(snapshots, threshold);
}

} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/layout/overlay_safe_area_helper.h"

#include <algorithm>
#include <sstream>

#include "base/geometry/rect.h"
#include "base/log/log_wrapper.h"
#include "base/subwindow/subwindow.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/avoid_info/avoid_info_manager.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "core/components_ng/pattern/overlay/dialog_manager.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {

// Maps a window pixel interval onto the UEC interval [localStart, localEnd] (both expressed in
// window coordinates) and keeps only the part abutting the UEC leading edge (top/left): the guard
// passes only when overlapStart == localStart. The result keeps window coordinates on purpose:
// Inset is a pixel interval (Length() == end - start, safe_area_insets.h) and same-edge sources
// merged via Inset::Combine (keyboard inset, popup free-nav rects) are recorded in window
// coordinates, so the union hull stays meaningful. Length() carries the avoidance amount
// relative to the UEC leading edge. Returns a zero inset when there is no edge-abutting overlap.
SafeAreaInsets::Inset MapInsetToLeadingEdge(
    const SafeAreaInsets::Inset& hostInset, float localStart, float localEnd)
{
    float overlapStart = std::max(static_cast<float>(hostInset.start), localStart);
    float overlapEnd = std::min(static_cast<float>(hostInset.end), localEnd);
    if (overlapEnd <= overlapStart || overlapStart > localStart) {
        return SafeAreaInsets::Inset();
    }
    return SafeAreaInsets::Inset { static_cast<uint32_t>(overlapStart),
        static_cast<uint32_t>(overlapEnd) };
}

// Maps a window pixel interval onto the UEC interval [localStart, localEnd] and keeps only the
// part abutting the UEC trailing edge (bottom/right): the guard passes only when
// overlapEnd == localEnd. Window coordinates are kept for the same Combine-consistency reason as
// MapInsetToLeadingEdge; Length() == localEnd - overlapStart is the avoidance amount relative to
// the UEC trailing edge. Returns a zero inset when there is no edge-abutting overlap.
SafeAreaInsets::Inset MapInsetToTrailingEdge(
    const SafeAreaInsets::Inset& hostInset, float localStart, float localEnd)
{
    float overlapStart = std::max(static_cast<float>(hostInset.start), localStart);
    float overlapEnd = std::min(static_cast<float>(hostInset.end), localEnd);
    if (overlapEnd <= overlapStart || overlapEnd < localEnd) {
        return SafeAreaInsets::Inset();
    }
    return SafeAreaInsets::Inset { static_cast<uint32_t>(overlapStart),
        static_cast<uint32_t>(overlapEnd) };
}

} // namespace

std::string AvoidConfig::ToString() const
{
    std::stringstream ss;
    ss << "AvoidConfig{safeArea:" << avoidSafeArea << ", keyboard:" << avoidKeyboard
        << ", popupFreeNav:" << avoidPopupFreeNav << ", displayLimit:" << avoidDisplayLimit << "}";
    return ss.str();
}

RectF OverlaySafeAreaHelper::ComputeDisplayableRect(
    const RefPtr<FrameNode>& frameNode, const AvoidConfig& config, int32_t subwindowType)
{
    CHECK_NULL_RETURN(frameNode, RectF());
    auto context = DialogManager::GetMainPipelineContext(frameNode);
    CHECK_NULL_RETURN(context, RectF());

    // Step 1: base rect = rect of the window the overlay node belongs to.
    RectF displayableRect = context->GetRootRect();

    // Step 2~4: collect enabled sources and merge per edge by interval union (no stacking).
    SafeAreaInsets combined = CollectInsetsByEdge(frameNode, config);

    // Step 5: shrink; width/height never go negative.
    float left = static_cast<float>(combined.left_.Length());
    float top = static_cast<float>(combined.top_.Length());
    float right = static_cast<float>(combined.right_.Length());
    float bottom = static_cast<float>(combined.bottom_.Length());
    float width = std::max(0.0f, displayableRect.Width() - left - right);
    float height = std::max(0.0f, displayableRect.Height() - top - bottom);
    displayableRect = RectF(displayableRect.Left() + left, displayableRect.Top() + top, width, height);

    // Step 6: optional clip with the available display rect (dock bar / subwindow screen edge).
    // The subwindow type is caller-supplied: on the super-fold + sub-container branch it selects
    // the fold-expand subwindow, so each overlay must pass the type its legacy path uses.
    if (config.avoidDisplayLimit) {
        Rect availableRect = OverlayManager::GetDisplayAvailableRect(frameNode, subwindowType);
        if (availableRect.Width() > 0 && availableRect.Height() > 0) {
            RectF availableRectF(static_cast<float>(availableRect.Left()),
                static_cast<float>(availableRect.Top()), availableRect.Width(), availableRect.Height());
            displayableRect = displayableRect.IntersectRectT(availableRectF);
        }
    }

    if (displayableRect.Width() <= 0 || displayableRect.Height() <= 0) {
        TAG_LOGI(AceLogTag::ACE_OVERLAY, "ComputeDisplayableRect result empty, config: %{public}s",
            config.ToString().c_str());
    }
    return displayableRect;
}

AvoidConfig OverlaySafeAreaHelper::GetAvoidConfig(const RefPtr<FrameNode>& frameNode)
{
    AvoidConfig config;
    CHECK_NULL_RETURN(frameNode, config);
    if (IsBindPopupPath(frameNode)) {
        // bindPopup uses the non-switch free-navigation path.
        config.avoidPopupFreeNav = true;
    }
    return config;
}

SafeAreaInsets OverlaySafeAreaHelper::CollectInsetsByEdge(
    const RefPtr<FrameNode>& frameNode, const AvoidConfig& config)
{
    SafeAreaInsets combined;
    CHECK_NULL_RETURN(frameNode, combined);
    auto context = DialogManager::GetMainPipelineContext(frameNode);
    CHECK_NULL_RETURN(context, combined);

    if (config.avoidSafeArea) {
        // Aggregated result of the own window; composition and switch semantics live upstream.
        SafeAreaInsets safeArea = GetWindowSafeArea(frameNode);
        combined.left_ = combined.left_.Combine(safeArea.left_);
        combined.top_ = combined.top_.Combine(safeArea.top_);
        combined.right_ = combined.right_.Combine(safeArea.right_);
        combined.bottom_ = combined.bottom_.Combine(safeArea.bottom_);
    }
    if (config.avoidKeyboard) {
        auto safeAreaManager = context->GetSafeAreaManager();
        CHECK_NULL_RETURN(safeAreaManager, combined);
        // Keyboard inset is maintained separately and is not part of the aggregated result; the
        // interval union with the bottom safe area avoids double stacking.
        combined.bottom_ = combined.bottom_.Combine(safeAreaManager->GetKeyboardInsetWithoutProcess());
    }
    if (config.avoidPopupFreeNav) {
        CollectPopupFreeNavInsets(combined, frameNode);
    }
    return combined;
}

void OverlaySafeAreaHelper::CollectPopupFreeNavInsets(
    SafeAreaInsets& combined, const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto context = DialogManager::GetMainPipelineContext(frameNode);
    CHECK_NULL_VOID(context);
    auto avoidInfoManager = context->GetAvoidInfoManager();
    CHECK_NULL_VOID(avoidInfoManager);
    // Free-navigation avoidance only applies when the container modal asks for it.
    if (!avoidInfoManager->NeedAvoidContainerModal()) {
        return;
    }
    int32_t titleHeight = avoidInfoManager->GetContainerModalTitleHeight();
    if (titleHeight > 0) {
        combined.top_ = combined.top_.Combine(
            SafeAreaInsets::Inset { 0, static_cast<uint32_t>(titleHeight) });
    }
    RectF containerModal;
    RectF buttonsRect;
    // Control buttons abut the right edge of the window: only the right-edge shrink applies.
    if (!avoidInfoManager->GetContainerModalButtonsRect(containerModal, buttonsRect) ||
        buttonsRect.Width() <= 0 || buttonsRect.Height() <= 0) {
        return;
    }
    RectF rootRect = context->GetRootRect();
    if (buttonsRect.Right() < rootRect.Right()) {
        return;
    }
    // Clamp the start: buttons wider than the root rect (left edge outside the window) would
    // otherwise make the subtraction negative and the uint32_t conversion undefined behavior;
    // clamp to the full-width interval instead.
    combined.right_ = combined.right_.Combine(SafeAreaInsets::Inset {
        static_cast<uint32_t>(std::max(0.0f, rootRect.Width() - buttonsRect.Width())),
        static_cast<uint32_t>(rootRect.Width()) });
}

SafeAreaInsets OverlaySafeAreaHelper::GetWindowSafeArea(const RefPtr<FrameNode>& frameNode)
{
    SafeAreaInsets insets;
    CHECK_NULL_RETURN(frameNode, insets);
    auto context = DialogManager::GetMainPipelineContext(frameNode);
    CHECK_NULL_RETURN(context, insets);
    auto safeAreaManager = context->GetSafeAreaManager();
    CHECK_NULL_RETURN(safeAreaManager, insets);
    insets = safeAreaManager->GetSafeAreaWithoutProcess();
    auto uecAncestor = FindUecAncestor(frameNode);
    if (uecAncestor) {
        // In UEC mode the fetched safe area belongs to the host window and must be intersected
        // with the UEC extent (edge-abutting parts only) before use; interval positions stay in
        // window coordinates so that same-edge Combine partners (keyboard / popup free-nav,
        // ADR-3 union) share one coordinate system (ADR-7 v1.3).
        insets = ConvertUecSafeArea(uecAncestor, insets);
    }
    return insets;
}

RefPtr<FrameNode> OverlaySafeAreaHelper::FindUecAncestor(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto ancestor = frameNode->GetAncestorNodeOfFrame(false);
    while (ancestor) {
        if (ancestor->GetTag() == V2::UI_EXTENSION_COMPONENT_ETS_TAG) {
            return ancestor;
        }
        ancestor = ancestor->GetAncestorNodeOfFrame(false);
    }
    return nullptr;
}

SafeAreaInsets OverlaySafeAreaHelper::ConvertUecSafeArea(
    const RefPtr<FrameNode>& uecAncestor, const SafeAreaInsets& hostInsets)
{
    SafeAreaInsets localInsets;
    CHECK_NULL_RETURN(uecAncestor, hostInsets);
    auto geometryNode = uecAncestor->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, hostInsets);
    RectF uecRect = geometryNode->GetFrameRect();
    OffsetF globalOffset = uecAncestor->GetPaintRectOffsetNG();
    float localLeft = globalOffset.GetX();
    float localTop = globalOffset.GetY();
    float localRight = globalOffset.GetX() + uecRect.Width();
    float localBottom = globalOffset.GetY() + uecRect.Height();
    localInsets.left_ = MapInsetToLeadingEdge(hostInsets.left_, localLeft, localRight);
    localInsets.top_ = MapInsetToLeadingEdge(hostInsets.top_, localTop, localBottom);
    localInsets.right_ = MapInsetToTrailingEdge(hostInsets.right_, localLeft, localRight);
    localInsets.bottom_ = MapInsetToTrailingEdge(hostInsets.bottom_, localTop, localBottom);
    return localInsets;
}

bool OverlaySafeAreaHelper::IsBindPopupPath(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    return frameNode->GetTag() == V2::POPUP_ETS_TAG;
}

} // namespace OHOS::Ace::NG

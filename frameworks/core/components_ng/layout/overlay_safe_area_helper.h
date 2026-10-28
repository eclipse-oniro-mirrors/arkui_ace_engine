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

#ifndef FOUNDATION_ACE_FRAMEWORK_CORE_COMPONENTS_NG_LAYOUT_OVERLAY_SAFE_AREA_HELPER_H
#define FOUNDATION_ACE_FRAMEWORK_CORE_COMPONENTS_NG_LAYOUT_OVERLAY_SAFE_AREA_HELPER_H

#include <string>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/subwindow/subwindow.h"
#include "base/utils/macros.h"
#include "core/components_ng/property/safe_area_insets.h"

namespace OHOS::Ace::NG {

class FrameNode;

// Avoidance configuration for overlay components (Dialog/Popup/Menu/ActionSheet/Toast/Tips).
// Each field maps to exactly one data source and is independently observable (Func-04-02-01-Feat-06):
//  - avoidSafeArea:     safe area insets of the window the overlay node belongs to, aggregated by
//                       SafeAreaManager::GetSafeAreaWithoutProcess() (system/cutout/nav/floatNav are
//                       combined upstream and cannot be decomposed; switch semantics live upstream).
//  - avoidKeyboard:     keyboard inset (SafeAreaManager::GetKeyboardInsetWithoutProcess), which is
//                       NOT part of the aggregated result and must be fetched separately.
//  - avoidPopupFreeNav: bindPopup free-navigation-button path, sourced from AvoidInfoManager's
//                       ContainerModalAvoidInfo; independent from the global switch.
//  - avoidDisplayLimit: dock bar and subwindow screen edge, applied by clipping the result with
//                       OverlayManager::GetDisplayAvailableRect.
// Overlapping sources on the same edge are merged by interval union (no double stacking).
struct AvoidConfig {
    bool avoidSafeArea = false;
    bool avoidKeyboard = false;
    bool avoidPopupFreeNav = false;
    bool avoidDisplayLimit = false;

    // Out-of-line (defined in the .cpp) so the header does not propagate <sstream> to every
    // includer; ACE_FORCE_EXPORT keeps the symbol linkable from the pattern libraries, following
    // the project convention for struct ToString (e.g. BorderProperty::ToString).
    ACE_FORCE_EXPORT std::string ToString() const;
};

// Utility that computes the displayable rect for overlay components. Stateless: all sources are
// read live on each call, no member state, no switch polling (switch semantics are fully
// upstreamed into the safe area data). Combination use only; the LayoutAlgorithm inheritance
// chain is not changed.
class ACE_FORCE_EXPORT OverlaySafeAreaHelper {
public:
    // Computes the displayable rect: starts from the rect of the window the overlay node belongs
    // to, shrinks by the union of all enabled avoidance sources (per-edge interval union), then
    // optionally clips with the available display rect. Returns an empty rect for null node or
    // missing context (no crash).
    // subwindowType: the SubwindowType of this overlay, consumed only by the display-limit clip —
    // OverlayManager::GetDisplayAvailableRect selects the fold-expand subwindow by it on the
    // super-fold + sub-container branch. Callers pass the type their legacy code paths use
    // (dialog: TYPE_DIALOG, menu: TYPE_MENU); defaults to TYPE_POPUP (bubble's legacy type).
    static RectF ComputeDisplayableRect(const RefPtr<FrameNode>& frameNode, const AvoidConfig& config,
        int32_t subwindowType = static_cast<int32_t>(SubwindowType::TYPE_POPUP));

    // Returns the default AvoidConfig for the component type of the given overlay node
    // (bindPopup style node defaults avoidPopupFreeNav to true). Returns an all-false config for
    // null node.
    static AvoidConfig GetAvoidConfig(const RefPtr<FrameNode>& frameNode);

private:
    // Collects the per-edge avoidance insets of all enabled sources and merges them by interval
    // union.
    static SafeAreaInsets CollectInsetsByEdge(const RefPtr<FrameNode>& frameNode, const AvoidConfig& config);
    // Merges the bindPopup free-navigation-button insets (container modal title + control buttons)
    // into combined. Guard-clause style to keep CollectInsetsByEdge shallow.
    static void CollectPopupFreeNavInsets(SafeAreaInsets& combined, const RefPtr<FrameNode>& frameNode);

    // Safe area of the window the node belongs to (window attribution: frameNode -> own pipeline).
    // In UEC mode the host window safe area is converted to the UEC-local one first.
    static SafeAreaInsets GetWindowSafeArea(const RefPtr<FrameNode>& frameNode);

    // Returns the UI extension component (UEC) ancestor of the node, or null when the node does
    // not sit inside a UEC. Single walk: the caller reuses the found node directly instead of a
    // boolean probe followed by a second walk of the same chain.
    static RefPtr<FrameNode> FindUecAncestor(const RefPtr<FrameNode>& frameNode);

    // Converts host-window safe area insets into insets clipped to the UEC extent (derived from
    // the UEC global offset and geometry, following AvoidInfoManager::GetContainerModalAvoidInfo
    // ForUEC): keeps only the part of each window inset that abuts the corresponding UEC edge.
    // Interval positions stay in window coordinates (ADR-7 v1.3) so that same-edge sources merged
    // via Inset::Combine (keyboard, popup free-nav) share one coordinate system; Length() carries
    // the avoidance amount relative to the UEC edge.
    static SafeAreaInsets ConvertUecSafeArea(const RefPtr<FrameNode>& uecAncestor, const SafeAreaInsets& hostInsets);

    // Whether the node belongs to a bindPopup style overlay (non-switch free-navigation path).
    static bool IsBindPopupPath(const RefPtr<FrameNode>& frameNode);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORK_CORE_COMPONENTS_NG_LAYOUT_OVERLAY_SAFE_AREA_HELPER_H

/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_SNAP_STRATEGY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_SNAP_STRATEGY_H

#include <functional>

#include "core/components_ng/pattern/scrollable/scrollable_properties.h"

namespace OHOS::Ace {
/**
 * Feat-20260806915268: shared item scroll snap strategy contract for List/Grid/WaterFlow.
 *
 * Mirrors the public ArkTS `scrollSnapStrategy(strategy?: ScrollSnapAlign | ScrollSnapOffsetProvider)`
 * declaration (interface_sdk-js PR #35304):
 *  - built-in strategy: ScrollSnapAlign START/CENTER/END, item anchors align to the main-axis snap line;
 *  - custom provider: two-stage snap. `calculateApproachOffset(velocity, decayOffset)` decides the approach
 *    offset magnitude of a fling (negative/non-finite/exception falls back to natural friction decay);
 *    `calculateSnapOffset(velocity)` decides the absolute final snap target at handoff (non-finite/exception
 *    falls back to the natural snap target, result clamped to the legal scroll range).
 *
 * Stored as a layout property. `ScrollSnapAlign::NONE` without a provider disables the capability.
 *
 * Lives in a dedicated strategy header for the property classes that store it, instead of
 * the widely-included scrollable_properties.h: pattern headers forward-declare these types so their
 * definitions only fan out to item-snap consumers.
 */
using SnapApproachOffsetFunc = std::function<double(double velocity, double decayOffset)>;
using SnapOffsetFunc = std::function<double(double velocity)>;

struct ScrollSnapStrategy {
    ScrollSnapAlign align = ScrollSnapAlign::NONE;
    bool hasProvider = false;
    SnapApproachOffsetFunc calculateApproachOffset;
    SnapOffsetFunc calculateSnapOffset;

    bool IsEnabled() const
    {
        return hasProvider || align != ScrollSnapAlign::NONE;
    }

    // Callback identity is not comparable. Align-only strategies compare by align so redundant
    // writes dedupe; any provider-bearing strategy never compares equal, forcing the layout
    // property update path (NearEqual -> operator==) to always store the fresh callbacks —
    // otherwise replacing provider A with provider B would silently keep A's closures.
    bool operator==(const ScrollSnapStrategy& other) const
    {
        if (hasProvider || other.hasProvider) {
            return false;
        }
        return align == other.align;
    }

    bool operator!=(const ScrollSnapStrategy& other) const
    {
        return !(*this == other);
    }
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_SNAP_STRATEGY_H

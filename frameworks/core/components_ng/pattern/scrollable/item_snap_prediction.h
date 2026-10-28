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
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_ITEM_SNAP_PREDICTION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_ITEM_SNAP_PREDICTION_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

#include "base/utils/macros.h"

namespace OHOS::Ace::NG {

// The last vsync may predate a programmatic release after an idle interval.
// Do not initialize a replacement spring in that old frame or move a future frame backwards.
inline uint64_t ResolveItemSnapStartTime(uint64_t vsync, uint64_t now)
{
    return std::max(vsync, now);
}

// Same fixed curve as List slow snapping; only the takeover position changes.
inline constexpr float ITEM_SNAP_FIXED_RESPONSE = 0.7f;
inline constexpr float ITEM_SNAP_FIXED_DAMPING = 2.5f;

ACE_EXPORT std::optional<float> CalculateItemSnapTakeoverDistance(float naturalDistance, float speed, float beyond);

// Never apply the part of a delayed friction frame beyond its handoff point.
inline float ClampItemSnapFrictionFrame(float current, float position, float naturalEnd, float threshold)
{
    const float remaining = naturalEnd - current;
    if (!std::isfinite(remaining) || !std::isfinite(position) || !std::isfinite(threshold) ||
        threshold <= 0.0f || std::abs(remaining) <= threshold) {
        return position;
    }
    const float direction = remaining > 0.0f ? 1.0f : -1.0f;
    const float handoff = naturalEnd - direction * threshold;
    return direction * (position - handoff) > 0.0f ? handoff : position;
}

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_ITEM_SNAP_PREDICTION_H

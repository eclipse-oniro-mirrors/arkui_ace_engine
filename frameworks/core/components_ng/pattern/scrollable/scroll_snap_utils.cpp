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

#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "core/components_ng/pattern/scrollable/item_snap_prediction.h"

#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
namespace {
// Index of the first candidate whose offset is greater than [offset].
int32_t UpperBoundIndex(const std::vector<SnapCandidate>& candidates, float offset)
{
    return static_cast<int32_t>(std::upper_bound(candidates.begin(), candidates.end(), offset,
        [](float value, const SnapCandidate& candidate) { return value < candidate.offset; }) -
        candidates.begin());
}

SnapSelectResult MakeResult(const SnapCandidate& candidate)
{
    SnapSelectResult result;
    result.found = true;
    result.targetOffset = candidate.offset;
    result.targetIndex = candidate.index;
    return result;
}
} // namespace

void ScrollSnapUtils::NormalizeCandidates(std::vector<SnapCandidate>& candidates)
{
    // Plain (offset, index) ordering keeps the comparator a strict weak ordering; grouping
    // near-equal offsets with an epsilon inside the comparator breaks transitivity (UB in
    // std::sort). The near-equal merge happens in the dedup pass below instead.
    std::sort(candidates.begin(), candidates.end(), [](const SnapCandidate& lhs, const SnapCandidate& rhs) {
        if (lhs.offset != rhs.offset) {
            return lhs.offset < rhs.offset;
        }
        return lhs.index < rhs.index;
    });
    // Same snap offset can come from multiple items (Grid rows / equal-height WaterFlow lanes):
    // keep the first (smallest index) and drop the rest (R-6). Explicit single-pass compaction
    // against the last KEPT candidate — NearEqual is not transitive, so std::unique's
    // equivalence-relation precondition would be violated (UB per [alg.unique]).
    std::vector<SnapCandidate> compacted;
    compacted.reserve(candidates.size());
    for (const auto& candidate : candidates) {
        if (compacted.empty() ||
            !NearEqual(compacted.back().offset, candidate.offset, SNAP_OFFSET_EPSILON)) {
            compacted.push_back(candidate);
        }
    }
    candidates = std::move(compacted);
}

SnapSelectResult ScrollSnapUtils::SelectByThreshold(
    const std::vector<SnapCandidate>& candidates, float currentOffset, float threshold, bool forward)
{
    SnapSelectResult result;
    if (candidates.empty()) {
        return result;
    }
    // Clamp threshold into [0, 1]; invalid values fall back to the default (R-4 / R-11).
    float clampedThreshold = std::isfinite(threshold) ? std::clamp(threshold, 0.0f, 1.0f) : DEFAULT_SNAP_THRESHOLD;

    auto upper = UpperBoundIndex(candidates, currentOffset);
    int32_t currentIdx = -1;
    int32_t neighborIdx = -1;
    if (forward) {
        // current candidate: last one at or before the offset; neighbor: the next one.
        currentIdx = upper > 0 ? upper - 1 : 0;
        neighborIdx = upper < static_cast<int32_t>(candidates.size()) ? upper : -1;
    } else {
        currentIdx = upper < static_cast<int32_t>(candidates.size()) ? upper
                                                                      : static_cast<int32_t>(candidates.size()) - 1;
        neighborIdx = upper > 0 ? upper - 1 : -1;
    }
    if (neighborIdx < 0) {
        return MakeResult(candidates[currentIdx]);
    }
    const auto& current = candidates[currentIdx];
    const auto& neighbor = candidates[neighborIdx];
    float distance = std::abs(neighbor.offset - current.offset);
    if (NearZero(distance)) {
        return MakeResult(current);
    }
    float effective = std::abs(currentOffset - current.offset);
    // "< D * T returns to the current candidate; >= D * T enters the adjacent candidate" (R-4).
    return GreatOrEqual(effective, distance * clampedThreshold) ? MakeResult(neighbor) : MakeResult(current);
}

SnapSelectResult ScrollSnapUtils::SelectByPredictedLanding(
    const std::vector<SnapCandidate>& candidates, float currentOffset, float predictedOffset, bool forward,
    float threshold)
{
    if (candidates.empty()) {
        return SnapSelectResult();
    }
    // Restrict the search to candidates that do not move against the fling direction (R-5).
    // Forward keeps [current-eps, last]; backward keeps [first, current+eps] so the candidate
    // at the current offset itself stays selectable in both directions.
    auto upper = UpperBoundIndex(candidates, currentOffset - SNAP_OFFSET_EPSILON);
    int32_t begin = forward ? upper : 0;
    int32_t end = forward ? static_cast<int32_t>(candidates.size())
                          : UpperBoundIndex(candidates, currentOffset + SNAP_OFFSET_EPSILON);
    if (begin >= end) {
        // No candidate in the fling direction: keep the threshold result instead of reversing.
        return SelectByThreshold(candidates, currentOffset, threshold, forward);
    }
    int32_t best = begin;
    float bestDistance = std::abs(candidates[begin].offset - predictedOffset);
    // Index of the last candidate found equidistant (within epsilon) with the current best.
    // A boolean flag alone cannot tell WHICH pair was equidistant when several near-equal
    // pairs appear; the index resolves the tie deterministically in the fling direction (R-6).
    int32_t equidistantWith = -1;
    for (int32_t i = begin + 1; i < end; ++i) {
        float distance = std::abs(candidates[i].offset - predictedOffset);
        if (NearEqual(distance, bestDistance, SNAP_OFFSET_EPSILON)) {
            equidistantWith = i;
            continue;
        }
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
            equidistantWith = -1;
        }
    }
    if (equidistantWith >= 0) {
        // Equal distance: prefer the candidate in the fling direction (R-6).
        best = forward ? std::max(best, equidistantWith) : std::min(best, equidistantWith);
    }
    return MakeResult(candidates[best]);
}

SnapSelectResult ScrollSnapUtils::SelectByDirection(
    const std::vector<SnapCandidate>& candidates, float currentOffset, bool forward)
{
    if (candidates.empty()) {
        return SnapSelectResult();
    }
    auto upper = UpperBoundIndex(candidates, currentOffset + SNAP_OFFSET_EPSILON);
    if (forward) {
        if (upper < static_cast<int32_t>(candidates.size())) {
            return MakeResult(candidates[upper]);
        }
    } else {
        // Step strictly backwards: bound below the current offset so resting exactly on an
        // anchor still steps to the previous candidate (upper_bound(current - eps) already
        // points past the anchor), instead of returning the anchor itself and stalling.
        auto lower = UpperBoundIndex(candidates, currentOffset - SNAP_OFFSET_EPSILON);
        if (lower > 0) {
            return MakeResult(candidates[lower - 1]);
        }
    }
    // Already at the edge: stay on the nearest candidate in that direction.
    return SelectByThreshold(candidates, currentOffset, DEFAULT_SNAP_THRESHOLD, forward);
}

SnapSelectResult ScrollSnapUtils::SelectSnapTarget(const std::vector<SnapCandidate>& candidates,
    float currentOffset, float velocity, float predictDelta, float threshold, float velocityScale)
{
    if (candidates.empty()) {
        return SnapSelectResult();
    }
    // Non-finite inputs degrade deterministically: an invalid velocity/scale falls back to the
    // zero-velocity behavior (direction from predictDelta, threshold selection) instead of
    // landing in a random comparison branch (NaN compares false everywhere).
    const bool velocityValid = std::isfinite(velocity) && !NearZero(velocity);
    const bool deltaValid = std::isfinite(predictDelta) && !NearZero(predictDelta);
    if (!velocityValid && !deltaValid) {
        // With no effective direction, the threshold's current/neighbor ordering is
        // undefined. Select the nearest anchor, breaking equal distances by item index.
        auto best = candidates.begin();
        float bestDistance = std::abs(best->offset - currentOffset);
        for (auto candidate = best + 1; candidate != candidates.end(); ++candidate) {
            const float distance = std::abs(candidate->offset - currentOffset);
            const bool equal = NearEqual(distance, bestDistance, SNAP_OFFSET_EPSILON);
            if ((!equal && distance < bestDistance) || (equal && candidate->index < best->index)) {
                best = candidate;
                bestDistance = distance;
            }
        }
        return MakeResult(*best);
    }
    bool forward = velocityValid ? velocity > 0.0f : predictDelta > 0.0f;
    float safeScale = (std::isfinite(velocityScale) && GreatNotEqual(velocityScale, 0.0f)) ? velocityScale : 1.0f;
    float speedThreshold = SNAP_VELOCITY_THRESHOLD * safeScale;
    if (velocityValid && std::abs(velocity) >= speedThreshold && deltaValid) {
        float predictedOffset = currentOffset + predictDelta;
        return SelectByPredictedLanding(candidates, currentOffset, predictedOffset, forward, threshold);
    }
    return SelectByThreshold(candidates, currentOffset, threshold, forward);
}

float ScrollSnapUtils::ClampSnapOffset(float offset, float minOffset, float maxOffset)
{
    if (maxOffset < minOffset) {
        return minOffset;
    }
    return std::clamp(offset, minOffset, maxOffset);
}

// Prefer a handoff with monotonic deceleration, preserving the real target and fling velocity.
// If the friction-stop bound makes this impossible, choose the closest bounded plan; the
// spring sample guard still prevents overshoot, but this cannot guarantee zero acceleration.
// Safe interval for q=omega*D/V: [1/(zeta+sqrt(zeta*zeta-1)), 2*zeta].
std::optional<float> CalculateItemSnapTakeoverDistance(float naturalDistance, float speed, float beyond)
{
    constexpr float response = ITEM_SNAP_FIXED_RESPONSE;
    constexpr float damping = ITEM_SNAP_FIXED_DAMPING;
    if (!std::isfinite(naturalDistance) || !std::isfinite(speed) || !std::isfinite(beyond)) {
        return std::nullopt;
    }
    if (naturalDistance == 0.0f) {
        return std::nullopt;
    }
    if (naturalDistance <= 0.0f || speed <= 0.0f || naturalDistance + beyond <= 0.0f) {
        return std::nullopt;
    }
    const float rate = speed / naturalDistance;
    const float omega = 2.0f * 3.14159265f / response;
    const float earliest = naturalDistance - std::min(1.0f, (naturalDistance + beyond) * 0.5f);
    if (!std::isfinite(rate) || rate <= 0.0f || !std::isfinite(omega) || earliest <= 0.0f) {
        return std::nullopt;
    }
    // The real friction stops below 120px/s, before a mathematically late handoff.
    // Leave 5px/s of headroom; a fixed curve may become infeasible under this bound.
    const float latest = std::min(earliest, std::max(125.0f / rate, std::max(1.0f, -beyond + 1.0f)));
    const float safeLow = 1.01f / (damping + std::sqrt(damping * damping - 1.0f));
    const float safeHigh = 1.98f * damping;
    const float desired = 1.9f;
    float selected = earliest;
    float best = std::numeric_limits<float>::infinity();
    auto consider =
        [latest, earliest, omega, beyond, rate, safeLow, safeHigh, desired, &best, &selected](float remaining) {
        if (!std::isfinite(remaining) || remaining < latest || remaining > earliest) {
            return;
        }
        const float ratio = omega * (remaining + beyond) / (rate * remaining);
        const float penalty = std::max(safeLow - ratio, std::max(ratio - safeHigh, 0.0f));
        const float score = penalty * 1000.0f + std::abs(ratio - desired);
        if (std::isfinite(score) && score < best) {
            best = score;
            selected = remaining;
        }
    };
    consider(earliest);
    consider(latest);
    for (float ratio : {desired, safeLow, safeHigh}) {
        const float denominator = ratio * rate - omega;
        if (denominator != 0.0f) {
            consider(omega * beyond / denominator);
        }
    }
    return selected;
}

} // namespace OHOS::Ace::NG

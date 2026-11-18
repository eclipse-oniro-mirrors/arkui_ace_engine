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

#include "core/components_ng/base/inspector_occlusion_region.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace OHOS::Ace::NG {
namespace {
constexpr size_t MAX_RECTS = 256;
constexpr size_t MAX_OPERATIONS = 16384;
constexpr size_t MAX_BYTES = 32768;
} // namespace

OcclusionRegionSet::OcclusionRegionSet() : OcclusionRegionSet(Limits()) {}

OcclusionRegionSet::OcclusionRegionSet(Limits limits)
{
    // The retained set and the two scratch buffers may coexist during subtraction.
    const size_t memoryRectLimit = limits.maxBytes / (3 * sizeof(RegionRect));
    limits_.maxRects = std::min({ limits.maxRects, MAX_RECTS, memoryRectLimit });
    limits_.maxOperations = std::min(limits.maxOperations, MAX_OPERATIONS);
    limits_.maxBytes = std::min(limits.maxBytes, MAX_BYTES);
}

bool OcclusionRegionSet::ReadRect(const RectF& source, RegionRect& result)
{
    const double left = source.GetX();
    const double top = source.GetY();
    const double width = source.Width();
    const double height = source.Height();
    const double right = left + width;
    const double bottom = top + height;
    constexpr double maxFloat = std::numeric_limits<float>::max();
    if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(width) || !std::isfinite(height) ||
        !std::isfinite(right) || !std::isfinite(bottom) || width < 0.0 || height < 0.0 || right > maxFloat ||
        bottom > maxFloat) {
        return false;
    }
    result = { left, top, right, bottom };
    return true;
}

bool OcclusionRegionSet::Intersect(const RegionRect& first, const RegionRect& second, RegionRect& result)
{
    result = { std::max(first.left, second.left), std::max(first.top, second.top), std::min(first.right, second.right),
        std::min(first.bottom, second.bottom) };
    return result.left < result.right && result.top < result.bottom;
}

size_t OcclusionRegionSet::Subtract(const RegionRect& source, const RegionRect& blocker, RegionRect (&parts)[4])
{
    RegionRect overlap;
    if (!Intersect(source, blocker, overlap)) {
        parts[0] = source;
        return 1;
    }

    size_t count = 0;
    if (source.top < overlap.top) {
        parts[count++] = { source.left, source.top, source.right, overlap.top };
    }
    if (overlap.bottom < source.bottom) {
        parts[count++] = { source.left, overlap.bottom, source.right, source.bottom };
    }
    if (source.left < overlap.left) {
        parts[count++] = { source.left, overlap.top, overlap.left, overlap.bottom };
    }
    if (overlap.right < source.right) {
        parts[count++] = { overlap.right, overlap.top, source.right, overlap.bottom };
    }
    return count;
}

bool OcclusionRegionSet::AppendWithinLimit(
    std::vector<RegionRect>& destination, const RegionRect& rect, size_t retained)
{
    const size_t slotLimit = limits_.maxBytes / sizeof(RegionRect);
    if (destination.size() >= limits_.maxRects || retained >= slotLimit || destination.size() >= slotLimit - retained) {
        limitReached_ = true;
        return false;
    }
    destination.push_back(rect);
    return true;
}

bool OcclusionRegionSet::ConsumeOperation()
{
    if (operations_ >= limits_.maxOperations) {
        limitReached_ = true;
        return false;
    }
    ++operations_;
    return true;
}

OcclusionRegionSet::AddResult OcclusionRegionSet::Add(const RectF& rect)
{
    RegionRect region;
    if (!ReadRect(rect, region)) {
        return AddResult::INVALID;
    }
    if (region.left == region.right || region.top == region.bottom) {
        return AddResult::EMPTY;
    }
    return AddRegionRect(region);
}

OcclusionRegionSet::AddResult OcclusionRegionSet::AddClipped(const RectF& rect, const RectF& clip)
{
    RegionRect region;
    RegionRect clipRegion;
    if (!ReadRect(rect, region) || !ReadRect(clip, clipRegion)) {
        return AddResult::INVALID;
    }
    RegionRect clipped;
    if (!Intersect(region, clipRegion, clipped)) {
        return AddResult::EMPTY;
    }
    return AddRegionRect(clipped);
}

OcclusionRegionSet::AddResult OcclusionRegionSet::AddRegionRect(const RegionRect& rect)
{
    if (limitReached_ || limits_.maxOperations == 0 || limits_.maxRects == 0) {
        limitReached_ = true;
        return AddResult::LIMIT_REACHED;
    }
    if (rects_.capacity() < limits_.maxRects) {
        rects_.reserve(limits_.maxRects);
    }
    std::vector<RegionRect> pending;
    pending.reserve(limits_.maxRects);
    pending.push_back(rect);
    for (const auto& covered : rects_) {
        std::vector<RegionRect> next;
        next.reserve(limits_.maxRects);
        for (const auto& fragment : pending) {
            if (!ConsumeOperation()) {
                return AddResult::LIMIT_REACHED;
            }
            RegionRect parts[4];
            const auto count = Subtract(fragment, covered, parts);
            for (size_t i = 0; i < count; ++i) {
                if (!AppendWithinLimit(next, parts[i], rects_.size() + pending.size())) {
                    return AddResult::LIMIT_REACHED;
                }
            }
        }
        pending.swap(next);
        if (pending.empty()) {
            return AddResult::EMPTY;
        }
    }
    if (rects_.size() + pending.size() > limits_.maxRects ||
        (rects_.size() + pending.size()) * sizeof(RegionRect) > limits_.maxBytes) {
        limitReached_ = true;
        return AddResult::LIMIT_REACHED;
    }
    rects_.insert(rects_.end(), pending.begin(), pending.end());
    return AddResult::ADDED;
}

bool OcclusionRegionSet::Covers(const RectF& envelope)
{
    RegionRect region;
    if (limitReached_ || rects_.empty() || !ReadRect(envelope, region) || region.left == region.right ||
        region.top == region.bottom) {
        return false;
    }
    std::vector<RegionRect> pending;
    pending.reserve(limits_.maxRects);
    pending.push_back(region);
    for (const auto& covered : rects_) {
        std::vector<RegionRect> next;
        next.reserve(limits_.maxRects);
        for (const auto& fragment : pending) {
            if (!ConsumeOperation()) {
                return false;
            }
            RegionRect parts[4];
            const auto count = Subtract(fragment, covered, parts);
            for (size_t i = 0; i < count; ++i) {
                if (!AppendWithinLimit(next, parts[i], rects_.size() + pending.size())) {
                    return false;
                }
            }
        }
        pending.swap(next);
        if (pending.empty()) {
            return true;
        }
    }
    return false;
}

} // namespace OHOS::Ace::NG

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_REGION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_REGION_H

#include <cstddef>
#include <vector>

#include "base/geometry/ng/rect_t.h"

namespace OHOS::Ace::NG {

// Stores only verified, axis-aligned occlusion evidence. Every limit failure
// disables further positive coverage decisions for this request-local set.
class OcclusionRegionSet final {
public:
    struct Limits {
        size_t maxRects = 256;
        size_t maxOperations = 16384;
        size_t maxBytes = 32768;
    };

    enum class AddResult {
        ADDED,
        EMPTY,
        INVALID,
        LIMIT_REACHED,
    };

    OcclusionRegionSet();
    explicit OcclusionRegionSet(Limits limits);

    AddResult Add(const RectF& rect);
    AddResult AddClipped(const RectF& rect, const RectF& clip);
    bool Covers(const RectF& envelope);

    size_t GetRectCount() const
    {
        return rects_.size();
    }

    size_t GetOperationCount() const
    {
        return operations_;
    }

    bool IsLimitReached() const
    {
        return limitReached_;
    }

private:
    struct RegionRect {
        double left;
        double top;
        double right;
        double bottom;
    };

    static bool ReadRect(const RectF& source, RegionRect& result);
    static bool Intersect(const RegionRect& first, const RegionRect& second, RegionRect& result);
    static size_t Subtract(const RegionRect& source, const RegionRect& blocker, RegionRect (&parts)[4]);
    bool AppendWithinLimit(std::vector<RegionRect>& destination, const RegionRect& rect, size_t retained);
    bool ConsumeOperation();
    AddResult AddRegionRect(const RegionRect& rect);

    Limits limits_;
    std::vector<RegionRect> rects_;
    size_t operations_ = 0;
    bool limitReached_ = false;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_REGION_H

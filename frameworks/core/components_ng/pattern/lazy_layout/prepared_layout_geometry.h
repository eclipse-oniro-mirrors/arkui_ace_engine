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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LAZY_LAYOUT_PREPARED_LAYOUT_GEOMETRY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LAZY_LAYOUT_PREPARED_LAYOUT_GEOMETRY_H

#include <map>
#include <memory>

#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
enum class PreparedLayoutType : uint8_t { GROUP, GRID, COLUMN };

struct PreparedItemPosition {
    float startPos = 0.0f;
    float endPos = 0.0f;
    int32_t lane = 0;
};

// Request-owned values, never nodes. Tail positions are relative to the body end, not its estimated origin.
struct PreparedLayoutGeometry {
    PreparedLayoutType type = PreparedLayoutType::GROUP;
    int32_t itemCount = 0;
    int32_t lanes = 1;
    float bodyMainSize = 0.0f;
    float headerMainSize = 0.0f;
    float footerMainSize = 0.0f;
    float space = 0.0f;
    float estimatedItemSize = 0.0f;
    bool fromEnd = false;
    std::map<int32_t, PreparedItemPosition> positions;
    std::map<int32_t, std::shared_ptr<const PreparedLayoutGeometry>> children;

    bool FindAnchor(float start, float end, float bodySize, bool forward, int32_t& index, float& position) const
    {
        if (positions.empty()) {
            return false;
        }
        const float origin = fromEnd ? bodySize : 0.0f;
        if (LessNotEqual(end, positions.begin()->second.startPos + origin) ||
            GreatNotEqual(start, positions.rbegin()->second.endPos + origin)) {
            return false;
        }
        if (forward) {
            auto item = positions.begin();
            auto next = std::next(item);
            while (next != positions.end() && LessNotEqual(item->second.endPos + origin, start)) {
                item = next++;
            }
            index = item->first;
            position = item->second.startPos + origin;
        } else {
            auto item = positions.rbegin();
            auto next = std::next(item);
            while (next != positions.rend() && GreatNotEqual(item->second.startPos + origin, end)) {
                item = next++;
            }
            index = item->first;
            position = item->second.endPos + origin;
        }
        return true;
    }
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LAZY_LAYOUT_PREPARED_LAYOUT_GEOMETRY_H

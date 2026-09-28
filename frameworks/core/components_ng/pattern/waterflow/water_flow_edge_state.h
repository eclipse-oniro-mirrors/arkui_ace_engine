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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_EDGE_STATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_EDGE_STATE_H

#include <algorithm>
#include <cstdint>

#include "core/components_ng/pattern/lazy_layout/header_footer_utils.h"

namespace OHOS::Ace::NG {
// New attributes reserve at most two FrameNode slots ahead of the data children.
// The constructor footer retains its historical mapping until an edge attribute is used.
struct WaterFlowChildIndexMap {
    bool hasHeader = false;
    bool hasFooter = false;

    int32_t EdgeCount() const { return static_cast<int32_t>(hasHeader) + static_cast<int32_t>(hasFooter); }
    int32_t HeaderIndex() const { return hasHeader ? static_cast<int32_t>(hasFooter) : -1; }
    int32_t FooterIndex() const { return hasFooter ? 0 : -1; }
    int32_t DataToNode(int32_t index) const { return index + EdgeCount(); }
    int32_t NodeToData(int32_t index) const { return index < EdgeCount() ? -1 : index - EdgeCount(); }
    int32_t DataCount(int32_t rawCount) const { return std::max(0, rawCount - EdgeCount()); }
};

struct WaterFlowEdgeState {
    bool enabled = false;
    bool footerControlled = false;
    WeakPtr<UINode> header;
    uint64_t headerGeneration = 0;
    uint64_t footerGeneration = 0;
    StickyStyle sticky = StickyStyle::NONE;

    uint64_t BeginUpdate(bool isHeader)
    {
        enabled = true;
        if (!isHeader) {
            footerControlled = true;
        }
        return isHeader ? ++headerGeneration : ++footerGeneration;
    }

    bool IsCurrent(bool isHeader, uint64_t generation) const
    {
        return generation == (isHeader ? headerGeneration : footerGeneration);
    }
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_EDGE_STATE_H

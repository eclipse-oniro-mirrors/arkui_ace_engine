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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_PARENT_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_PARENT_CONTEXT_H

#include <vector>

#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/list/list_layout_property.h"

namespace OHOS::Ace::NG {
// Ownership of lazy requests is independent of the sticky viewport subscription.
class WaterFlowParentContext {
public:
    ~WaterFlowParentContext() { Reset(); }

    void Reset()
    {
        for (auto& weak : requestPath_) {
            auto property = weak.Upgrade();
            if (property) {
                property->SetLazyLayoutRequest(owner_, false);
            }
        }
        requestPath_.clear();
    }

    bool Update(const RefPtr<FrameNode>& host, bool enabled)
    {
        Reset();
        if (!host || !enabled) {
            return false;
        }
        owner_ = host->GetId();
        std::vector<RefPtr<FrameNode>> path { host };
        auto parent = host->GetParentFrameNode();
        while (parent && LazyLayoutUtils::IsAllowedIntermediateNode(parent)) {
            path.push_back(parent);
            parent = parent->GetParentFrameNode();
        }
        if (!LazyLayoutUtils::IsVerticalScrollableParent(parent)) {
            return false;
        }
        if (parent->GetTag() == V2::LIST_ETS_TAG) {
            auto property = parent->GetLayoutProperty<ListLayoutProperty>();
            if (!property || property->HasLanes() || property->HasLaneMinLength() || property->HasLaneMaxLength() ||
                property->HasChainAnimation() || property->HasScrollSnapAlign()) {
                return false;
            }
        }
        for (const auto& node : path) {
            auto property = node->GetLayoutProperty();
            property->SetLazyLayoutRequest(owner_, true);
            requestPath_.emplace_back(property);
        }
        return true;
    }

private:
    int32_t owner_ = -1;
    std::vector<WeakPtr<LayoutProperty>> requestPath_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_PARENT_CONTEXT_H

/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "layout_synchronizer.h"

#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/pattern/list/list_layout_property.h"
#include "frameworks/core/components_ng/pattern/list/list_pattern.h"
#include "frameworks/core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "frameworks/core/components_ng/pattern/swiper/swiper_pattern.h"
#include "frameworks/core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "frameworks/core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/layout/section/staggered_fill_algorithm.h"
namespace OHOS::Ace::NG {

void LayoutSynchronizer::Sync(const RefPtr<LayoutProperty>& props, const StaggeredFillAlgorithm& fillAlgo)
{
    if (auto p = AceType::DynamicCast<WaterFlowLayoutProperty>(props); p) {
        auto host = p->GetHost();
        auto pattern = host->GetPattern<WaterFlowPattern>();
        int32_t startIdx = pattern->GetBeginIndex();
        int32_t endIdx = pattern->GetEndIndex();
        float offset = pattern->GetStoredOffset();
        if (startIdx != fillAlgo.StartIdx()) {

        }
        if (endIdx != fillAlgo.EndIdx()) {

        }
        if (offset != fillAlgo.StartPos()) {
            
        }
    } else if (auto p = AceType::DynamicCast<ListLayoutProperty>(props); p) {
    } else if (auto p = AceType::DynamicCast<SwiperLayoutProperty>(props); p) {
    }
}
} // namespace OHOS::Ace::NG

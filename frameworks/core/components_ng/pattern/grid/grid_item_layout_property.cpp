/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/grid/grid_item_layout_property.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void GridItemLayoutProperty::ResetGridLayoutInfoAndMeasure() const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    auto uiNode = DynamicCast<UINode>(host);
    while (uiNode->GetTag() != V2::GRID_ETS_TAG) {
        uiNode = uiNode->GetParent();
        CHECK_NULL_VOID(uiNode);
    }
    auto grid = DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_VOID(grid);
    auto pattern = grid->GetPattern<GridPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->ResetGridLayoutInfo();
    grid->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void GridItemLayoutProperty::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    LayoutProperty::ToJsonValue(json);
    json->Put("rowStart", propRowStart_.value_or(0));
    json->Put("rowEnd", propRowEnd_.value_or(0));
    json->Put("columnStart", propColumnStart_.value_or(0));
    json->Put("columnEnd", propColumnEnd_.value_or(0));
}

} // namespace OHOS::Ace::NG

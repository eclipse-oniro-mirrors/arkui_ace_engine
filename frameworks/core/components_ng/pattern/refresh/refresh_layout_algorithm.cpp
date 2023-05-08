/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "frameworks/core/components_ng/pattern/refresh/refresh_layout_algorithm.h"

#include "frameworks/base/utils/utils.h"
#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_layout_property.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_pattern.h"
#include "frameworks/core/components_ng/property/measure_property.h"
#include "frameworks/core/components_ng/property/measure_utils.h"
#include "frameworks/core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

RefreshLayoutAlgorithm::RefreshLayoutAlgorithm() = default;

void RefreshLayoutAlgorithm::Layout(FrameNode* frameNode)
{
    PerformLayout(frameNode);
    auto layoutProperty = AceType::DynamicCast<NG::RefreshLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    for (auto&& child : frameNode->GetAllFrameNodeChildren()) {
        if (!child) {
            continue;
        }
        child->Layout();
    }
}
// Called to perform layout render node and child.
void RefreshLayoutAlgorithm::PerformLayout(FrameNode* frameNode)
{
    // update child position.
    auto size = frameNode->GetGeometryNode()->GetFrameSize();
    const auto& padding = frameNode->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::TOP_LEFT;
    if (frameNode->GetLayoutProperty()->GetPositionProperty()) {
        align = frameNode->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
    }
    auto layoutProperty = AceType::DynamicCast<NG::RefreshLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);


    auto pattern = frameNode->GetPattern<RefreshPattern>();
    CHECK_NULL_VOID(pattern);
    // Update child position.
    int32_t index = 0;
    for (const auto& child : frameNode->GetAllFrameNodeChildren()) {
        if (!child) {
            index++;
            continue;
        }
        auto paddingOffsetChild = paddingOffset;
        auto alignChild = align;
        if (!layoutProperty->GetIsCustomBuilderExistValue(false)) {
            if (index == frameNode->TotalChildCount() - 2) {
                paddingOffsetChild += layoutProperty->GetShowTimeOffsetValue();
                alignChild = Alignment::TOP_CENTER;
            } else if (index == frameNode->TotalChildCount() - 1) {
                alignChild = Alignment::TOP_CENTER;
            }
        } else {
            if (index == layoutProperty->GetCustomBuilderIndexValue(-1)) {
                alignChild = Alignment::TOP_CENTER;
                paddingOffsetChild += layoutProperty->GetCustomBuilderOffsetValue();
            } else {
                paddingOffsetChild += pattern->GetScrollOffsetValue();
            }
        }
        auto translate = Alignment::GetAlignPosition(size, child->GetGeometryNode()->GetMarginFrameSize(), alignChild) +
                         paddingOffsetChild;
        child->GetGeometryNode()->SetMarginFrameOffset(translate);
        index++;
    }
}

} // namespace OHOS::Ace::NG

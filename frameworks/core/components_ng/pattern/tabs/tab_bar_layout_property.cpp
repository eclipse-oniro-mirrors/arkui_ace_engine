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

#include "core/components_ng/pattern/tabs/tab_bar_layout_property.h"

#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

RectF TabBarLayoutProperty::GetIndicatorRect(int32_t index)
{
    auto node = GetHost();
    CHECK_NULL_RETURN(node, RectF());
    auto tabTheme = node->GetTheme<TabTheme>(true);
    CHECK_NULL_RETURN(tabTheme, RectF());
    auto childColumn = DynamicCast<FrameNode>(node->GetChildAtIndex(index));
    CHECK_NULL_RETURN(childColumn && !childColumn->GetChildren().empty(), RectF());
    auto grandChildren = TabBarPattern::GetTextNodeFromColumn(childColumn);
    CHECK_NULL_RETURN(grandChildren, RectF());
    auto geometryNode = grandChildren->GetGeometryNode();
    RectF indicator = geometryNode->GetFrameRect(true);

    /* Accumulate offset from textNode up to the tabBarNode (host).
     * When badge wraps the content, the path is:
     *   tabBarNode -> columnNode -> badgeNode -> contentChild -> textNode
     * We need to sum offsets from textNode's parent chain up to (but not including) childColumn,
     * then add childColumn's offset relative to tabBarNode.
     */
    float accumulatedX = 0.0f;
    float accumulatedY = 0.0f;
    auto current = grandChildren->GetParent();
    while (current && current != childColumn) {
        auto currentFrame = DynamicCast<FrameNode>(current);
        if (currentFrame) {
            auto currentGeo = currentFrame->GetGeometryNode();
            if (currentGeo) {
                auto currentRect = currentGeo->GetFrameRect(true);
                accumulatedX += currentRect.GetX();
                accumulatedY += currentRect.GetY();
            }
        }
        current = current->GetParent();
    }

    /* Set indicator at the bottom of columnNode's last child */
    auto childColumnRect = childColumn->GetGeometryNode()->GetFrameRect(true);
    indicator.SetLeft(indicator.GetX() + accumulatedX + childColumnRect.GetX());
    indicator.SetTop(indicator.Bottom() + accumulatedY + childColumnRect.GetY());
    return indicator;
}

Dimension TabBarLayoutProperty::GetBarHeight() const
{
    auto node = GetHost();
    CHECK_NULL_RETURN(node, 0.0_vp);
    auto geometryNode = node->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0_vp);
    auto frameSize = geometryNode->GetFrameSize();
    return Dimension(PipelineBase::Px2VpWithCurrentDensity(frameSize.Height()), DimensionUnit::VP);
}

} // namespace OHOS::Ace::NG

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

#include "safe_area_inset_ani_modifier.h"

#include <cmath>

#include "core/common/container.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/syntax/static/detached_free_root_proxy_node.h"

namespace OHOS::Ace::NG {
void SetSafeAreaInsetStatic(ArkUINodeHandle node, const ArkUIAniSafeAreaInsetEdge* edges, int32_t count)
{
    CHECK_NULL_VOID(edges);
    if (count != static_cast<int32_t>(SAFE_AREA_INSET_EDGE_COUNT)) {
        return;
    }
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    SafeAreaInsetOptions options;
    SafeAreaInsetContentArray contents;
    for (size_t index = 0; index < SAFE_AREA_INSET_EDGE_COUNT; ++index) {
        const auto& edge = edges[index];
        if (!edge.content) {
            continue;
        }
        auto root = AceType::Claim(reinterpret_cast<UINode*>(edge.content));
        auto proxy = AceType::MakeRefPtr<DetachedFreeRootProxyNode>(container->GetInstanceId());
        proxy->AddChild(root);
        contents[index] = proxy;
        SafeAreaInsetEdgeOptions config;
        config.spacing = CalcLength(Dimension(0.0, DimensionUnit::VP));
        // Check raw units before conversion; unknown units must not silently become VP.
        if (std::isfinite(edge.spacing) && edge.spacing >= 0.0 &&
            (edge.unit == static_cast<int32_t>(DimensionUnit::VP) ||
                edge.unit == static_cast<int32_t>(DimensionUnit::PX) ||
                edge.unit == static_cast<int32_t>(DimensionUnit::FP) ||
                edge.unit == static_cast<int32_t>(DimensionUnit::LPX))) {
            config.spacing = CalcLength(Dimension(edge.spacing, static_cast<DimensionUnit>(edge.unit)));
        }
        if (edge.alignment >= static_cast<int32_t>(SafeAreaInsetAlignment::START) &&
            edge.alignment <= static_cast<int32_t>(SafeAreaInsetAlignment::END)) {
            config.alignment = static_cast<SafeAreaInsetAlignment>(edge.alignment);
        }
        options.edges[index] = config;
    }
    // A null host disposes temporary detached roots if building the snapshot failed.
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetSafeAreaInset(frameNode, options, contents);
}

} // namespace OHOS::Ace::NG

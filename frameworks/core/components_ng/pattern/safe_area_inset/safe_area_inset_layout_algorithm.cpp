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

#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_layout_algorithm.h"

#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/property/measure_property.h"

namespace OHOS::Ace::NG {

void SafeAreaInsetLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    const auto& layoutProperty = layoutWrapper->GetLayoutProperty();
    const auto& geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(layoutProperty);
    CHECK_NULL_VOID(geometryNode);

    auto childConstraint = layoutProperty->GetContentLayoutConstraint().value_or(LayoutConstraintF());
    childConstraint.selfIdealSize.Reset();
    childConstraint.minSize.Reset();
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<SafeAreaInsetPattern>();
    CHECK_NULL_VOID(pattern);
    for (size_t index = 0; index < SAFE_AREA_INSET_EDGE_COUNT; ++index) {
        auto edge = static_cast<SafeAreaInsetEdge>(index);
        if (!pattern->GetEdgeContent(edge) || pattern->GetEdgeContent(edge)->FrameCount() == 0) {
            continue;
        }
        auto contentWrapper = layoutWrapper->GetOrCreateChildByIndex(pattern->GetContentFrameIndex(edge));
        CHECK_NULL_CONTINUE(contentWrapper);
        contentWrapper->Measure(childConstraint);
    }

    // The auxiliary container never contributes its own size to the host. Its
    // final ordinary-padding inner rect is assigned by the host during Layout.
    geometryNode->SetFrameSize(SizeF());
}

void SafeAreaInsetLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    for (int32_t index = 0; index < layoutWrapper->GetTotalChildCount(); ++index) {
        auto contentWrapper = layoutWrapper->GetChildByIndex(index);
        CHECK_NULL_CONTINUE(contentWrapper);
        contentWrapper->Layout();
    }
}

} // namespace OHOS::Ace::NG

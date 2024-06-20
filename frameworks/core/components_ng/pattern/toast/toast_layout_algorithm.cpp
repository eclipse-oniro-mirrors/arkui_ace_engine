/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/toast/toast_layout_algorithm.h"

#include "base/utils/utils.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components_ng/pattern/toast/toast_view.h"
#include "core/components_ng/pattern/toast/toast_pattern.h"

namespace OHOS::Ace::NG {
void UpdateToastAlign(int32_t& alignment)
{
    bool isRtl = AceApplicationInfo::GetInstance().IsRightToLeft();
    if (alignment == static_cast<int32_t>(ToastAlignment::TOP_START)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::TOP_END);
        }
    } else if (alignment == static_cast<int32_t>(ToastAlignment::TOP_END)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::TOP_START);
        }
    } else if (alignment == static_cast<int32_t>(ToastAlignment::CENTER_START)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::CENTER_END);
        }
    } else if (alignment == static_cast<int32_t>(ToastAlignment::CENTER_END)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::CENTER_START);
        }
    } else if (alignment == static_cast<int32_t>(ToastAlignment::BOTTOM_START)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::BOTTOM_END);
        }
    } else if (alignment == static_cast<int32_t>(ToastAlignment::BOTTOM_END)) {
        if (isRtl) {
            alignment = static_cast<int32_t>(ToastAlignment::BOTTOM_START);
        }
    }
}

void ToastLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto toastPattern = frameNode->GetPattern<ToastPattern>();
    CHECK_NULL_VOID(toastPattern);
    auto toastProperty = frameNode->GetLayoutProperty<ToastLayoutProperty>();
    CHECK_NULL_VOID(toastProperty);
    auto alignment = toastPattern->GetToastInfo().alignment;
    UpdateToastAlign(alignment);
    auto align = Alignment::ParseAlignment(alignment);
    if (align.has_value()) {
        toastProperty->UpdateToastAlignment(align.value());
    } else {
        toastProperty->ResetToastAlignment();
    }
    auto offset = toastPattern->GetToastInfo().offset;
    if (offset.has_value()) {
        bool isRtl = AceApplicationInfo::GetInstance().IsRightToLeft();
        double xValue = isRtl ? offset->GetX().Value() * (-1) : offset->GetX().Value();
        Dimension offsetX = Dimension(xValue);
        offset->SetX(offsetX);
        toastProperty->UpdateToastOffset(offset.value());
    } else {
        toastProperty->ResetToastOffset();
    }
    auto text = layoutWrapper->GetOrCreateChildByIndex(0);
    text->Layout();
}

void ToastLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    auto text = layoutWrapper->GetOrCreateChildByIndex(0);
    text->Measure(layoutConstraint);
    PerformMeasureSelf(layoutWrapper);
}
} // namespace OHOS::Ace::NG

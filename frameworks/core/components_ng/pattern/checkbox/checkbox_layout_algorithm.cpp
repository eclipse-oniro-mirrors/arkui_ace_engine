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

#include "core/components_ng/pattern/checkbox/checkbox_layout_algorithm.h"

#include <algorithm>

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/checkbox/checkbox_pattern.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

std::optional<SizeF> CheckBoxLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(host, std::nullopt);
    auto pattern = host->GetPattern<CheckBoxPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);
    if (pattern->UseContentModifier()) {
        return BoxLayoutAlgorithm::MeasureContent(contentConstraint, layoutWrapper);
    }
    InitializeParam();
    // Case 1: Width and height are set in the front end.
    if (contentConstraint.selfIdealSize.Width().has_value() && contentConstraint.selfIdealSize.Height().has_value() &&
        contentConstraint.selfIdealSize.IsNonNegative()) {
        auto height = contentConstraint.selfIdealSize.Height().value();
        auto width = contentConstraint.selfIdealSize.Width().value();
        auto length = std::min(width, height);
        return SizeF(length, length);
    }
    // Case 2: The front end only sets either width or height
    // 2.1 Width is set to a value
    if (contentConstraint.selfIdealSize.Width().has_value() &&
        NonNegative(contentConstraint.selfIdealSize.Width().value())) {
        auto width = contentConstraint.selfIdealSize.Width().value();
        return SizeF(width, width);
    }
    // 2.2 Height is set to a value
    if (contentConstraint.selfIdealSize.Height().has_value() &&
        NonNegative(contentConstraint.selfIdealSize.Height().value())) {
        auto height = contentConstraint.selfIdealSize.Height().value();
        return SizeF(height, height);
    }
    // Case 3: Width and height are not set in the front end, so return from the theme
    auto width = defaultWidth_ - 2 * horizontalPadding_;
    auto height = defaultHeight_ - 2 * verticalPadding_;
    auto size = SizeF(width, height);
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    size.Constrain(contentConstraint.minSize, contentConstraint.maxSize);
    if (!NearEqual(size.Width(), size.Height())) {
        auto length = std::min(size.Width(), size.Height());
        size.SetWidth(length);
        size.SetHeight(length);
    }
    return size;
}

void CheckBoxLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    if (layoutWrapper->GetHostTag() == V2::CHECKBOX_ETS_TAG) {
        // Checkbox does not have child nodes. If a child is added to a toggle, then hide the child.
        for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
            child->GetGeometryNode()->SetFrameSize(SizeF());
        }
        PerformMeasureSelf(layoutWrapper);
    } else {
        BoxLayoutAlgorithm::Measure(layoutWrapper);
    }
}

void CheckBoxLayoutAlgorithm::InitializeParam()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto checkBoxTheme = pipeline->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);
    defaultWidth_ = checkBoxTheme->GetDefaultWidth().ConvertToPx();
    defaultHeight_ = checkBoxTheme->GetDefaultHeight().ConvertToPx();
    horizontalPadding_ = checkBoxTheme->GetHotZoneHorizontalPadding().ConvertToPx();
    verticalPadding_ = checkBoxTheme->GetHotZoneVerticalPadding().ConvertToPx();
}

void CheckBoxLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    BoxLayoutAlgorithm::Layout(layoutWrapper);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(childWrapper);
    SizeF childSize = childWrapper->GetGeometryNode()->GetMarginFrameSize();
    NG::OffsetF child_offset;
    child_offset.SetX((1.0 + align.GetHorizontal()) * (size.Width() - childSize.Width()) / 2.0);
    child_offset.SetY((1.0 + align.GetVertical()) * (size.Height() - childSize.Height()) / 2.0);
    auto translate = child_offset + paddingOffset;
    childWrapper->GetGeometryNode()->SetMarginFrameOffset(translate);
    childWrapper->Layout();
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    if (content) {
        NG::OffsetF offset;
        offset.SetX((1.0 + align.GetHorizontal()) * (size.Width() - content->GetRect().GetSize().Width()) / 2.0);
        offset.SetY((1.0 + align.GetVertical()) * (size.Height() - content->GetRect().GetSize().Height()) / 2.0);
        auto translate = offset + paddingOffset;
        content->SetOffset(translate);
    }
}

} // namespace OHOS::Ace::NG

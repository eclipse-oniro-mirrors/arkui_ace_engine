/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scroll/scroll_layout_algorithm.h"

#include <algorithm>

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/pattern/scroll/scroll_layout_property.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {
namespace {

void UpdateChildConstraint(Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint)
{
    contentConstraint.parentIdealSize = selfIdealSize;
    if (axis == Axis::VERTICAL) {
        contentConstraint.maxSize.SetHeight(Infinity<float>());
    } else {
        contentConstraint.maxSize.SetWidth(Infinity<float>());
    }
}

} // namespace

void ScrollLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<ScrollLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::VERTICAL);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto idealSize = CreateIdealSize(constraint.value(), axis, MeasureType::MATCH_CONTENT);
    auto padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, idealSize);
    // Calculate child layout constraint.
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    UpdateChildConstraint(axis, idealSize, childLayoutConstraint);
    // Measure child.
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto childSize = SizeF(0.f, 0.f);
    if (childWrapper) {
        childWrapper->Measure(childLayoutConstraint);
        // Use child size when self idea size of scroll is not setted.
        childSize = childWrapper->GetGeometryNode()->GetMarginFrameSize();
        if (!idealSize.Width()) {
            idealSize.SetWidth(childSize.Width());
        }
        if (!idealSize.Height()) {
            idealSize.SetHeight(childSize.Height());
        }
    }
    AddPaddingToSize(padding, idealSize);
    auto selfSize = idealSize.ConvertToSizeT();
    selfSize.Constrain(constraint->minSize, constraint->maxSize);
    auto scrollNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(scrollNode);
    auto scrollPattern = scrollNode->GetPattern<ScrollPattern>();
    CHECK_NULL_VOID(scrollPattern);
    if (scrollPattern->IsSelectScroll() && scrollPattern->GetHasOptionWidth()) {
        auto selectScrollWidth = scrollPattern->GetSelectScrollWidth();
        selfSize.SetWidth(selectScrollWidth);
    }
    auto lastViewSize = scrollPattern->GetViewSize();
    auto lastViewPortExtent = scrollPattern->GetViewPortExtent();
    if (!layoutWrapper->IsConstraintNoChanged() || lastViewSize != selfSize || lastViewPortExtent != childSize) {
        scrollPattern->AddScrollMeasureInfo(constraint, childLayoutConstraint, selfSize, childSize);
    }
    layoutWrapper->GetGeometryNode()->SetFrameSize(selfSize);
    //set initial offset
    if (scrollPattern->NeedSetInitialOffset()) {
        auto initialOffset = scrollPattern->GetInitialOffset();
        if (axis == Axis::VERTICAL) {
            auto offset = initialOffset.GetY();
            currentOffset_ = offset.Unit() == DimensionUnit::PERCENT ?
                             -offset.Value() * selfSize.Height() : -offset.ConvertToPx();
        } else {
            auto offset = initialOffset.GetX();
            currentOffset_ = offset.Unit() == DimensionUnit::PERCENT ?
                             -offset.Value() * selfSize.Width() : -offset.ConvertToPx();
        }
    }
}

void ScrollLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = AceType::DynamicCast<ScrollLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::VERTICAL);
    auto scrollNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(scrollNode);
    auto scrollPattern = AceType::DynamicCast<ScrollPattern>(scrollNode->GetPattern());
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(childWrapper);
    auto childGeometryNode = childWrapper->GetGeometryNode();
    CHECK_NULL_VOID(childGeometryNode);
    auto size = geometryNode->GetFrameSize();

    auto layoutDirection = layoutWrapper->GetLayoutProperty()->GetNonAutoLayoutDirection();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    viewSize_ = size;
    MinusPaddingToSize(padding, size);
    viewPort_ = size;
    auto childSize = childGeometryNode->GetMarginFrameSize();
    auto contentEndOffset = layoutProperty->GetScrollContentEndOffsetValue(.0f);
    scrollableDistance_ = GetMainAxisSize(childSize, axis) - GetMainAxisSize(viewPort_, axis) + contentEndOffset;
    if (!scrollPattern->CanOverScroll(scrollPattern->GetScrollSource())) {
        if (scrollableDistance_ > 0.0f) {
            currentOffset_ = std::clamp(currentOffset_, -scrollableDistance_, 0.0f);
        } else {
            currentOffset_ = std::clamp(currentOffset_, 0.0f, -scrollableDistance_);
        }
    }
    viewPortExtent_ = childSize;
    viewPortLength_ = GetMainAxisSize(viewPort_, axis);
    auto currentOffset = axis == Axis::VERTICAL ? OffsetF(0.0f, currentOffset_) : OffsetF(currentOffset_, 0.0f);
    if (layoutDirection == TextDirection::RTL && axis == Axis::HORIZONTAL) {
        currentOffset = OffsetF(size.Width() - childSize.Width() - currentOffset_, 0.0f);
    }
    auto scrollAlignment = Alignment::CENTER;
    if (layoutProperty->GetPositionProperty() && layoutProperty->GetPositionProperty()->HasAlignment()) {
        scrollAlignment = layoutProperty->GetPositionProperty()->GetAlignment().value();
    }
    auto alignmentPosition = Alignment::GetAlignPosition(size, viewPortExtent_, scrollAlignment);
    childGeometryNode->SetMarginFrameOffset(padding.Offset() + currentOffset + alignmentPosition);
    childWrapper->Layout();
}

} // namespace OHOS::Ace::NG

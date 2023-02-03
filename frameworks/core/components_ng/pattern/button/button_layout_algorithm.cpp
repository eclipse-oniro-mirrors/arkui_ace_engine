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

#include "core/components_ng/pattern/button/button_layout_algorithm.h"

#include "core/components/button/button_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void ButtonLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    // If the ButtonType is CIRCLE, then omit text by the smaller edge.
    if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CIRCLE) {
        auto minLength = std::min(layoutConstraint.maxSize.Width(), layoutConstraint.maxSize.Height());
        layoutConstraint.maxSize = SizeF(minLength, minLength);
    }
    // If the button has label, according to whether the font size is set to do the corresponding expansion button, font
    // reduction, truncation and other operations.
    if (buttonLayoutProperty->HasLabel()) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
        CHECK_NULL_VOID(childWrapper);
        auto childConstraint = childWrapper->GetLayoutProperty()->GetContentLayoutConstraint();
        childWrapper->Measure(childConstraint);
        childSize_ = childWrapper->GetGeometryNode()->GetContentSize();
        if (buttonLayoutProperty->HasFontSize()) {
            // Fonsize is set. When the font height is larger than the button height, make the button fit the font
            // height.
            if (GreatOrEqual(childSize_.Height(), layoutConstraint.maxSize.Height())) {
                layoutConstraint.maxSize.SetHeight(childSize_.Height());
            }
        } else {
            // Fonsize is not set. When the font width is greater than the button width, dynamically change the font
            // size to no less than 9sp.
            if (GreatOrEqual(childSize_.Width(), layoutConstraint.maxSize.Width())) {
                auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
                auto textLayoutProperty = DynamicCast<TextLayoutProperty>(childWrapper->GetLayoutProperty());
                textLayoutProperty->UpdateAdaptMaxFontSize(buttonTheme->GetMaxFontSize());
                textLayoutProperty->UpdateAdaptMinFontSize(buttonTheme->GetMinFontSize());
                childWrapper->Measure(layoutConstraint);
            }
        }
    }
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->Measure(layoutConstraint);
    }
    PerformMeasureSelf(layoutWrapper);
}

// Called to perform measure current render node.
void ButtonLayoutAlgorithm::PerformMeasureSelf(LayoutWrapper* layoutWrapper)
{
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);
    auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    // Determine if the button needs to fit the font size.
    if (buttonLayoutProperty->HasFontSize()) {
        if (GreatOrEqual(childSize_.Height(), frameSize.Height())) {
            frameSize = SizeF(frameSize.Width(), childSize_.Height());
            layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize);
        }
    }
    Dimension radius;
    if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CIRCLE) {
        auto minSize = std::min(frameSize.Height(), frameSize.Width());
        radius.SetValue(minSize / 2.0);
        BorderRadiusProperty borderRadius { radius, radius, radius, radius };
        host->GetRenderContext()->UpdateBorderRadius(borderRadius);
        MeasureCircleButton(layoutWrapper);
    } else if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CAPSULE) {
        radius.SetValue(frameSize.Height() / 2.0);
    } else {
        radius = buttonLayoutProperty->GetBorderRadiusValue(Dimension());
    }
    BorderRadiusProperty borderRadius { radius, radius, radius, radius };
    host->GetRenderContext()->UpdateBorderRadius(borderRadius);
}

void ButtonLayoutAlgorithm::MeasureCircleButton(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    const auto& radius = frameNode->GetRenderContext()->GetBorderRadius();
    SizeF frameSize = { -1, -1 };

    if (radius.has_value()) {
        auto radiusTopMax = std::max(radius->radiusTopLeft, radius->radiusTopRight);
        auto radiusBottomMax = std::max(radius->radiusBottomLeft, radius->radiusBottomRight);
        auto radiusMax = std::max(radiusTopMax, radiusBottomMax);
        auto rrectRadius = radiusMax->ConvertToPx();
        frameSize.SetSizeT(SizeF { static_cast<float>(rrectRadius * 2), static_cast<float>(rrectRadius * 2) });
    }
    frameSize.UpdateIllegalSizeWithCheck(SizeF { 0.0f, 0.0f });
    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize);
}

} // namespace OHOS::Ace::NG

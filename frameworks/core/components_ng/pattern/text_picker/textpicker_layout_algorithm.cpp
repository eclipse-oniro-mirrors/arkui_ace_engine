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

#include "core/components_ng/pattern/text_picker/textpicker_layout_algorithm.h"

#include "core/components/picker/picker_theme.h"
#include "core/components_ng/pattern/text_picker/textpicker_layout_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
const int32_t DIVIDER_SIZE = 2;
const int32_t TEXT_PICKER_CHILD_SIZE = 5;
const float PICKER_HEIGHT_HALF = 2.5f;
const float ITEM_HEIGHT_HALF = 2.0f;
const int32_t TEXT_PICKER_GRADIENT_CHILD_SIZE = 4;
} // namespace
void TextPickerLayoutAlgorithm::Measure(FrameNode* frameNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    SizeF frameSize = { -1.0f, -1.0f };

    float pickerHeight = 0.0f;
    auto stackNode = DynamicCast<FrameNode>(frameNode->GetParent());
    CHECK_NULL_VOID(stackNode);
    auto pickerNode = DynamicCast<FrameNode>(stackNode->GetParent());
    CHECK_NULL_VOID(pickerNode);
    auto layoutProperty = pickerNode->GetLayoutProperty<TextPickerLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    isDefaultPickerItemHeight_ = layoutProperty->HasDefaultPickerItemHeight();
    if (isDefaultPickerItemHeight_) {
        auto defaultPickerItemHeightValue = layoutProperty->GetDefaultPickerItemHeightValue();
        if (LessOrEqual(defaultPickerItemHeightValue.Value(), 0.0)) {
            isDefaultPickerItemHeight_ = false;
        }
    }

    if (isDefaultPickerItemHeight_) {
        pickerHeight = static_cast<float>(defaultPickerItemHeight_ * TEXT_PICKER_CHILD_SIZE);
    } else {
        pickerHeight =
            static_cast<float>(pickerTheme->GetGradientHeight().ConvertToPx() * TEXT_PICKER_GRADIENT_CHILD_SIZE +
                               pickerTheme->GetDividerSpacing().ConvertToPx());
    }

    auto layoutConstraint = pickerNode->GetLayoutProperty()->GetLayoutConstraint();
    auto width = layoutConstraint->selfIdealSize.Width();
    auto height = layoutConstraint->selfIdealSize.Height();
    float pickerWidth = 0.0f;
    auto children = pickerNode->GetChildren();

    if (width.has_value()) {
        pickerWidth = width.value() / static_cast<float>(children.size());
    } else {
        pickerWidth = static_cast<float>((pickerTheme->GetDividerSpacing() * DIVIDER_SIZE).ConvertToPx());
    }

    if (height.has_value()) {
        pickerHeight = height.value();
    }
    pickerItemHeight_ = pickerHeight;
    frameSize.SetWidth(pickerWidth);
    frameSize.SetHeight(pickerHeight);
    frameNode->GetGeometryNode()->SetFrameSize(frameSize);
    auto layoutChildConstraint = frameNode->GetLayoutProperty()->CreateChildConstraint();
    for (auto&& child : frameNode->GetAllFrameNodeChildren()) {
        child->Measure(layoutChildConstraint);
    }
    MeasureText(frameNode, frameSize);
}

void TextPickerLayoutAlgorithm::MeasureText(FrameNode* frameNode, const SizeF& size)
{
    auto totalChild = frameNode->TotalChildCount();
    for (int32_t index = 0; index < totalChild; index++) {
        auto child = frameNode->GetFrameNodeByIndex(index);
        ChangeTextStyle(index, totalChild, size, child, frameNode);
    }
}

void TextPickerLayoutAlgorithm::ChangeTextStyle(uint32_t index, uint32_t showOptionCount, const SizeF& size,
    const RefPtr<FrameNode>& child, FrameNode* frameNode)
{
    SizeF frameSize = { -1.0f, -1.0f };
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    frameSize.SetWidth(size.Width());
    uint32_t selectedIndex = showOptionCount / 2; // the center option is selected.
    auto layoutChildConstraint = frameNode->GetLayoutProperty()->CreateChildConstraint();
    if (isDefaultPickerItemHeight_) {
        frameSize.SetHeight(static_cast<float>(defaultPickerItemHeight_));
    } else {
        if (index == selectedIndex) {
            frameSize.SetHeight(static_cast<float>(pickerTheme->GetDividerSpacing().ConvertToPx()));
        } else {
            frameSize.SetHeight(static_cast<float>(pickerTheme->GetGradientHeight().ConvertToPx()));
        }
    }
    layoutChildConstraint.selfIdealSize = { frameSize.Width(), frameSize.Height() };
    child->Measure(layoutChildConstraint);
}

void TextPickerLayoutAlgorithm::Layout(FrameNode* frameNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto layoutProperty = AceType::DynamicCast<LinearLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto size = geometryNode->GetFrameSize();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto children = frameNode->GetAllFrameNodeChildren();
    float childStartCoordinate = 0.0f;

    if (isDefaultPickerItemHeight_) {
        childStartCoordinate +=
            static_cast<float>(size.Height() / ITEM_HEIGHT_HALF - defaultPickerItemHeight_ * PICKER_HEIGHT_HALF);
    } else {
        childStartCoordinate += static_cast<float>(pickerItemHeight_ / ITEM_HEIGHT_HALF -
                                                   pickerTheme->GetGradientHeight().ConvertToPx() * ITEM_HEIGHT_HALF -
                                                   pickerTheme->GetDividerSpacing().ConvertToPx() / ITEM_HEIGHT_HALF);
    }

    for (const auto& child : children) {
        auto childGeometryNode = child->GetGeometryNode();
        auto childSize = childGeometryNode->GetMarginFrameSize();
        auto childOffset = OffsetF(0.0f, childStartCoordinate + currentOffset_ + padding.Offset().GetY());
        childGeometryNode->SetMarginFrameOffset(childOffset);
        child->Layout();
        childStartCoordinate += childSize.Height();
    }
}

} // namespace OHOS::Ace::NG

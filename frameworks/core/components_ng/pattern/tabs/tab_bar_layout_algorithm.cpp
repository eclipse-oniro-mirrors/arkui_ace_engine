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

#include "core/components_ng/pattern/tabs/tab_bar_layout_algorithm.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/grid_layout_info.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/tabs/tab_bar_paint_property.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int8_t MASK_COUNT = 2;
constexpr int8_t SM_COLUMN_NUM = 4;
constexpr int8_t MD_COLUMN_NUM = 8;
constexpr int8_t LG_COLUMN_NUM = 12;
} // namespace
void TabBarLayoutAlgorithm::UpdateChildConstraint(LayoutConstraintF& childConstraint,
    const RefPtr<TabBarLayoutProperty>& layoutProperty, const SizeF& ideaSize, int32_t childCount, Axis axis)
{
    childConstraint.parentIdealSize = OptionalSizeF(ideaSize);
    const auto& barMode = layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    if (barMode == TabBarMode::FIXED) {
        auto childIdeaSize = ideaSize;
        if (axis == Axis::HORIZONTAL) {
            childIdeaSize.SetWidth(GetContentWidth(layoutProperty, ideaSize) / childCount);
            childConstraint.maxSize.SetHeight(childConstraint.maxSize.Height());
            if (tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
                childConstraint.minSize.SetWidth(tabTheme->GetSubTabBarMinWidth().ConvertToPx());
            }
        } else if (axis == Axis::VERTICAL) {
            childIdeaSize.SetHeight(tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE
                                        ? ideaSize.Height() / (childCount * 2)
                                        : ideaSize.Height() / childCount);
        }
        childConstraint.selfIdealSize = OptionalSizeF(childIdeaSize);
    } else {
        if (axis == Axis::HORIZONTAL) {
            childConstraint.maxSize.SetWidth(Infinity<float>());
            childConstraint.maxSize.SetHeight(childConstraint.maxSize.Height());
            childConstraint.selfIdealSize.SetHeight(ideaSize.Height());
            if (tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
                childConstraint.minSize.SetWidth(tabTheme->GetSubTabBarMinWidth().ConvertToPx());
            }
        } else if (axis == Axis::VERTICAL) {
            childConstraint.maxSize.SetHeight(Infinity<float>());
            childConstraint.selfIdealSize.SetWidth(ideaSize.Width());
        }
    }
}

void TabBarLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    useItemWidth_ = true;
    previousChildrenMainSize_ = childrenMainSize_;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    axis_ = GetAxis(layoutWrapper);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto idealSize =
        CreateIdealSize(constraint.value(), axis_, layoutProperty->GetMeasureType(MeasureType::MATCH_PARENT));

    auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
    if (childCount <= 0) {
        return;
    }

    if (axis_ == Axis::VERTICAL && constraint->selfIdealSize.Width().has_value() &&
        constraint->selfIdealSize.Width().value() < constraint->parentIdealSize.Width().value_or(0.0f) &&
        constraint->selfIdealSize.Width().value() > tabTheme->GetHorizontalBottomTabMinWidth().ConvertToPx()) {
        // Vertical tab bar may apply LayoutMode.AUTO
        ApplyLayoutMode(layoutWrapper, constraint->selfIdealSize.Width().value(), childCount);
    }
    if (constraint->selfIdealSize.Width().has_value() &&
        constraint->selfIdealSize.Width().value() > constraint->parentIdealSize.Width().value_or(0.0f)) {
        idealSize.SetWidth(static_cast<float>(
            axis_ == Axis::HORIZONTAL                         ? constraint->parentIdealSize.Width().value_or(0.0f)
            : tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE ? tabTheme->GetTabBarDefaultWidth().ConvertToPx()
                                                             : tabTheme->GetTabBarDefaultHeight().ConvertToPx()));
    }
    if (constraint->selfIdealSize.Height().has_value() &&
        constraint->selfIdealSize.Height().value() > constraint->parentIdealSize.Height().value_or(0.0f)) {
        float height = axis_ == Axis::HORIZONTAL
                           ? (tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE &&
                                         Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)
                                     ? tabTheme->GetBottomTabBarDefaultWidth().ConvertToPx()
                                     : tabTheme->GetTabBarDefaultHeight().ConvertToPx())
                           : constraint->parentIdealSize.Height().value_or(0.0f);

        idealSize.SetHeight(static_cast<float>(height));
    }
    if (!constraint->selfIdealSize.Width().has_value() && axis_ == Axis::VERTICAL) {
        idealSize.SetWidth(static_cast<float>(tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE
                                                  ? tabTheme->GetTabBarDefaultWidth().ConvertToPx()
                                                  : tabTheme->GetTabBarDefaultHeight().ConvertToPx()));
    }

    auto frameSize = idealSize.ConvertToSizeT();

    if ((axis_ == Axis::VERTICAL && NearZero(idealSize.ConvertToSizeT().Width())) ||
        (axis_ == Axis::HORIZONTAL && NearZero(idealSize.ConvertToSizeT().Height()))) {
        layoutWrapper->SetActive(false);
        geometryNode->SetFrameSize(SizeF());
        return;
    } else {
        layoutWrapper->SetActive(true);
    }

    if (axis_ == Axis::HORIZONTAL) {
        ConfigHorizontal(layoutWrapper, frameSize, childCount);
    } else {
        UpdateHorizontalPadding(layoutWrapper, 0.0f);
    }

    if (!constraint->selfIdealSize.Height().has_value() && axis_ == Axis::HORIZONTAL) {
        float defaultHeight = (tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE &&
                                  Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE))
                                  ? static_cast<float>(tabTheme->GetBottomTabBarDefaultWidth().ConvertToPx())
                                  : static_cast<float>(tabTheme->GetTabBarDefaultHeight().ConvertToPx());
        idealSize.SetHeight(std::max(defaultHeight, maxHeight_));
    }

    geometryNode->SetFrameSize(idealSize.ConvertToSizeT());

    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    UpdateChildConstraint(childLayoutConstraint, layoutProperty, idealSize.ConvertToSizeT(), childCount, axis_);

    // Measure children.
    childrenMainSize_ = (axis_ == Axis::VERTICAL ? 0.0f : scrollMargin_ * 2);
    for (int32_t index = 0; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!childWrapper) {
            continue;
        }
        if (static_cast<int32_t>(itemWidths_.size()) == childCount && useItemWidth_) {
            auto childIdeaSize = idealSize.ConvertToSizeT();
            childIdeaSize.SetWidth(itemWidths_[index]);
            childLayoutConstraint.selfIdealSize = OptionalSizeF(childIdeaSize);
        }
        auto iconWrapper = childWrapper->GetOrCreateChildByIndex(0);
        if (iconWrapper && iconWrapper->GetHostNode() && iconWrapper->GetHostNode()->GetTag() == V2::SYMBOL_ETS_TAG) {
            childWrapper->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
        }
        childWrapper->Measure(childLayoutConstraint);
        if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::FIXED) {
            // In fixed mode, large paddings may overwhelm the constraint after measure
            AdjustFixedItem(childWrapper, childLayoutConstraint.selfIdealSize, axis_);
        }
        childrenMainSize_ += childWrapper->GetGeometryNode()->GetMarginFrameSize().MainSize(axis_);
        tabBarPattern->UpdateSymbolEffect(index);
    }
    MeasureMask(layoutWrapper, childCount);
}

void TabBarLayoutAlgorithm::CheckMarqueeForScrollable(LayoutWrapper* layoutWrapper, int32_t childCount) const
{
    for (int32_t index = 0; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(childWrapper);
        auto textWrapper = childWrapper->GetOrCreateChildByIndex(1);
        CHECK_NULL_VOID(textWrapper);
        auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(textLayoutProperty);
        if (textLayoutProperty->GetTextOverflow().value_or(TextOverflow::NONE) == TextOverflow::MARQUEE) {
            textLayoutProperty->UpdateTextOverflow(TextOverflow::NONE);
        }
    }
}

void TabBarLayoutAlgorithm::MeasureMask(LayoutWrapper* layoutWrapper, int32_t childCount) const
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto maskLayoutConstraint = layoutProperty->CreateChildConstraint();
    auto selectedMaskWrapper = layoutWrapper->GetOrCreateChildByIndex(childCount);
    CHECK_NULL_VOID(selectedMaskWrapper);
    maskLayoutConstraint.selfIdealSize = OptionalSizeF(selectedMaskWrapper->GetGeometryNode()->GetFrameSize());
    selectedMaskWrapper->Measure(maskLayoutConstraint);

    auto unselectedMaskWrapper = layoutWrapper->GetOrCreateChildByIndex(childCount + 1);
    CHECK_NULL_VOID(unselectedMaskWrapper);
    maskLayoutConstraint.selfIdealSize = OptionalSizeF(unselectedMaskWrapper->GetGeometryNode()->GetFrameSize());
    unselectedMaskWrapper->Measure(maskLayoutConstraint);
}

void TabBarLayoutAlgorithm::ConfigHorizontal(LayoutWrapper* layoutWrapper, const SizeF& frameSize, int32_t childCount)
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    // Apply grid column options to the tab bar
    auto horizontalPadding = ApplyBarGridAlign(layoutProperty, frameSize);

    UpdateHorizontalPadding(layoutWrapper, horizontalPadding);

    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::FIXED) {
        HandleFixedMode(layoutWrapper, frameSize, childCount);
    } else {
        // Handle scrollable mode
        auto layoutStyle = layoutProperty->GetScrollableBarModeOptions().value_or(ScrollableBarModeOptions());
        scrollMargin_ = layoutStyle.margin.ConvertToPx();
        CheckMarqueeForScrollable(layoutWrapper, childCount);
        MeasureItemWidths(layoutWrapper, childCount);

        if (layoutStyle.nonScrollableLayoutStyle == LayoutStyle::ALWAYS_AVERAGE_SPLIT) {
            HandleAlwaysAverageSplitLayoutStyle(layoutWrapper, frameSize, childCount);
        } else if (layoutStyle.nonScrollableLayoutStyle == LayoutStyle::SPACE_BETWEEN_OR_CENTER) {
            HandleSpaceBetweenOrCenterLayoutStyle(layoutWrapper, frameSize, childCount);
        } else if (layoutStyle.nonScrollableLayoutStyle == LayoutStyle::ALWAYS_CENTER) {
            if (LessOrEqual(childrenMainSize_, GetContentWidth(layoutProperty, frameSize))) {
                childrenMainSize_ -= scrollMargin_ * 2;
                scrollMargin_ = 0.0f;
            } else {
                useItemWidth_ = false;
            }
        }
    }
    if (layoutProperty->GetBarAdaptiveHeight().value_or(false)) {
        MeasureMaxHeight(layoutWrapper, childCount);
    }
}

void TabBarLayoutAlgorithm::HandleFixedMode(LayoutWrapper* layoutWrapper, const SizeF& frameSize, int32_t childCount)
{
    if (childCount <= 0) {
        return;
    }
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto allocatedWidth = GetContentWidth(layoutProperty, frameSize) / childCount;
    MeasureItemWidths(layoutWrapper, childCount);
    ApplyLayoutMode(layoutWrapper, allocatedWidth, childCount);
    ApplySymmetricExtensible(layoutWrapper, allocatedWidth, childCount);
}

void TabBarLayoutAlgorithm::MeasureItemWidths(LayoutWrapper* layoutWrapper, int32_t childCount)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);

    childrenMainSize_ = scrollMargin_ * 2;
    itemWidths_.clear();
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.maxSize.SetWidth(Infinity<float>());
    if (tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
        childLayoutConstraint.minSize.SetWidth(tabTheme->GetSubTabBarMinWidth().ConvertToPx());
    }
    for (int32_t index = 0; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID (childWrapper);
        if (tabBarPattern->GetTabBarStyle(index) == TabBarStyle::BOTTOMTABBATSTYLE) {
            auto iconWrapper = childWrapper->GetOrCreateChildByIndex(0);
            CHECK_NULL_VOID(iconWrapper);
            if (iconWrapper->GetHostNode()->GetTag() == V2::SYMBOL_ETS_TAG) {
                auto symbolLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(iconWrapper->GetLayoutProperty());
                CHECK_NULL_VOID(symbolLayoutProperty);
                symbolLayoutProperty->UpdateMargin({ CalcLength(0.0_vp), CalcLength(0.0_vp), {}, {} });
            } else {
                auto imageLayoutProperty = AceType::DynamicCast<ImageLayoutProperty>(iconWrapper->GetLayoutProperty());
                CHECK_NULL_VOID(imageLayoutProperty);
                imageLayoutProperty->UpdateMargin({ CalcLength(0.0_vp), CalcLength(0.0_vp), {}, {} });
            }
            auto textWrapper = childWrapper->GetOrCreateChildByIndex(1);
            CHECK_NULL_VOID(textWrapper);
            auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
            CHECK_NULL_VOID(textLayoutProperty);
            textLayoutProperty->UpdateMargin({ CalcLength(0.0_vp), CalcLength(0.0_vp), {}, {} });
        }

        childWrapper->Measure(childLayoutConstraint);
        auto geometryNode = childWrapper->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        itemWidths_.emplace_back(geometryNode->GetMarginFrameSize().MainSize(Axis::HORIZONTAL));
        childrenMainSize_ += itemWidths_.back();
    }
}

void TabBarLayoutAlgorithm::MeasureMaxHeight(LayoutWrapper* layoutWrapper, int32_t childCount)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);

    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.maxSize.SetWidth(Infinity<float>());
    if (tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
        childLayoutConstraint.minSize.SetWidth(tabTheme->GetSubTabBarMinWidth().ConvertToPx());
    }
    for (int32_t index = 0; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(childWrapper);
        if (static_cast<int32_t>(itemWidths_.size()) == childCount) {
            childLayoutConstraint.selfIdealSize.SetWidth(itemWidths_[index]);
        }
        childWrapper->Measure(childLayoutConstraint);
        auto geometryNode = childWrapper->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        maxHeight_ =
            std::max(maxHeight_, geometryNode->GetMarginFrameSize().MainSize(Axis::VERTICAL));
    }
}

void TabBarLayoutAlgorithm::HandleAlwaysAverageSplitLayoutStyle(
    LayoutWrapper* layoutWrapper, const SizeF& frameSize, int32_t childCount)
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    if (GreatNotEqual(childrenMainSize_, GetContentWidth(layoutProperty, frameSize)) || childCount <= 0 ||
        childCount != static_cast<int32_t>(itemWidths_.size())) {
        useItemWidth_ = false;
        return;
    }

    bool hasLongItem = false;
    int32_t remainingChildCount = childCount;
    childrenMainSize_ -= scrollMargin_ * 2;
    auto totalWidth = GetContentWidth(layoutProperty, frameSize) - scrollMargin_ * 2;
    scrollMargin_ = 0.0f;
    auto allocatedItemWidth = totalWidth / remainingChildCount;

    std::vector<float> originalWidth;
    for (int32_t index = 0; index < childCount; index++) {
        originalWidth.emplace_back(itemWidths_[index]);
        itemWidths_[index] = 0.0f;
    }

    /* Calculate the widths of long items. A long item refers to an item whose length is above the average,
        so remainingChildCount can't be zero */
    do {
        hasLongItem = false;
        for (int32_t index = 0; index < childCount; index++) {
            if (NearZero(itemWidths_[index]) && GreatNotEqual(originalWidth[index], allocatedItemWidth)) {
                itemWidths_[index] = originalWidth[index];
                hasLongItem = true;
                remainingChildCount--;
                totalWidth -= originalWidth[index];
            }
        }
        allocatedItemWidth = totalWidth / remainingChildCount;
    } while (hasLongItem);

    // Calculate the widths of other items
    for (int32_t index = 0; index < childCount; index++) {
        if (NearZero(itemWidths_[index])) {
            itemWidths_[index] = allocatedItemWidth;
        }
    }
}

void TabBarLayoutAlgorithm::HandleSpaceBetweenOrCenterLayoutStyle(
    LayoutWrapper* layoutWrapper, const SizeF& frameSize, int32_t childCount)
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    if (GreatNotEqual(childrenMainSize_, GetContentWidth(layoutProperty, frameSize)) || childCount <= 0 ||
        childCount != static_cast<int32_t>(itemWidths_.size())) {
        useItemWidth_ = false;
        return;
    }

    childrenMainSize_ -= scrollMargin_ * 2;
    scrollMargin_ = 0.0f;

    if (GreatNotEqual(childrenMainSize_, GetContentWidth(layoutProperty, frameSize) / 2)) {
        useItemWidth_ = false;
        return;
    }
    auto additionalWidth = (GetContentWidth(layoutProperty, frameSize) / 2 - childrenMainSize_) / childCount;

    for (int32_t index = 0; index < childCount; ++index) {
        itemWidths_[index] += additionalWidth;
    }
}

void TabBarLayoutAlgorithm::ApplyLayoutMode(LayoutWrapper* layoutWrapper, float allocatedWidth, int32_t childCount)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);

    bool isVertical = LessOrEqual(allocatedWidth, tabTheme->GetHorizontalBottomTabMinWidth().ConvertToPx());

    // Calculate the initial buffer and initial space request of each item.
    for (int32_t index = 0; index < childCount; ++index) {
        auto bottomTabBarStyle = tabBarPattern->GetBottomTabBarStyle(index);
        if (tabBarPattern->GetTabBarStyle(index) != TabBarStyle::BOTTOMTABBATSTYLE ||
            bottomTabBarStyle.layoutMode != LayoutMode::AUTO) {
            continue;
        }
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(childWrapper);
        auto linearLayoutProperty = AceType::DynamicCast<LinearLayoutProperty>(childWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(linearLayoutProperty);
        auto textWrapper = childWrapper->GetOrCreateChildByIndex(1);
        CHECK_NULL_VOID(textWrapper);
        auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(textLayoutProperty);
        if (isVertical) {
            linearLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
            linearLayoutProperty->UpdateSpace(tabTheme->GetBottomTabBarSpace());
            linearLayoutProperty->UpdateMainAxisAlign(bottomTabBarStyle.verticalAlign);
            linearLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
            linearLayoutProperty->SetIsVertical(true);
            textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
            if (!tabBarPattern->GetBottomTabLabelStyle(index).fontSize.has_value() &&
                Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                textLayoutProperty->UpdateFontSize(tabTheme->GetBottomTabTextSize());
            }
        } else {
            linearLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
            linearLayoutProperty->UpdateSpace(tabTheme->GetHorizontalBottomTabBarSpace());
            linearLayoutProperty->UpdateMainAxisAlign(FlexAlign::CENTER);
            linearLayoutProperty->UpdateCrossAxisAlign(bottomTabBarStyle.verticalAlign);
            linearLayoutProperty->SetIsVertical(false);
            textLayoutProperty->UpdateTextAlign(TextAlign::LEFT);
            if (!tabBarPattern->GetBottomTabLabelStyle(index).fontSize.has_value() &&
                Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                textLayoutProperty->UpdateFontSize(tabTheme->GetBottomTabHorizontalTextSize());
            }
        }
    }
}

void TabBarLayoutAlgorithm::ApplySymmetricExtensible(
    LayoutWrapper* layoutWrapper, float allocatedWidth, int32_t childCount)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);

    if (childCount <= 2 || static_cast<int32_t>(itemWidths_.size()) != childCount) {
        useItemWidth_ = false;
        for (int32_t index = 0; index < static_cast<int32_t>(itemWidths_.size()); ++index) {
            itemWidths_[index] = allocatedWidth;
        }
        return;
    }

    std::vector<float> leftBuffers(childCount);
    std::vector<float> rightBuffers(childCount);
    std::vector<float> spaceRequests(childCount);

    // Calculate the initial buffer and initial space request of each item.
    for (int32_t index = 0; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(childWrapper);
        auto linearLayoutProperty = AceType::DynamicCast<LinearLayoutProperty>(childWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(linearLayoutProperty);
        if (GreatNotEqual(itemWidths_[index], allocatedWidth)) {
            if (tabBarPattern->GetTabBarStyle(index) != TabBarStyle::BOTTOMTABBATSTYLE ||
                !tabBarPattern->GetBottomTabBarStyle(index).symmetricExtensible || index == 0 ||
                index == childCount - 1) {
                spaceRequests[index] = 0.0f;
            } else {
                spaceRequests[index] = (itemWidths_[index] - allocatedWidth) / 2;
            }
            leftBuffers[index] = 0.0f;
            rightBuffers[index] = 0.0f;
        } else {
            if (tabBarPattern->GetTabBarStyle(index) != TabBarStyle::BOTTOMTABBATSTYLE) {
                leftBuffers[index] = 0.0f;
                rightBuffers[index] = 0.0f;
            } else {
                leftBuffers[index] = (index == 0 ? 0.0f : (allocatedWidth - itemWidths_[index]) / 2);
                rightBuffers[index] = (index == childCount - 1 ? 0.0f : (allocatedWidth - itemWidths_[index]) / 2);
            }
            spaceRequests[index] = 0.0f;
        }
    }

    // Decide the used buffer and used space request of each item.
    for (int32_t index = 1; index < childCount - 1; ++index) {
        auto actualRequest = std::min(std::min(rightBuffers[index - 1], leftBuffers[index + 1]), spaceRequests[index]);
        spaceRequests[index] = actualRequest;
        rightBuffers[index - 1] = actualRequest;
        leftBuffers[index + 1] = actualRequest;
    }

    spaceRequests[0] = 0.0f;
    spaceRequests[childCount - 1] = 0.0f;

    leftBuffers[1] = 0.0f;
    rightBuffers[childCount - 2] = 0.0f;

    CalculateItemWidthsForSymmetricExtensible(
        layoutWrapper, childCount, spaceRequests, leftBuffers, rightBuffers, allocatedWidth);
}

void TabBarLayoutAlgorithm::CalculateItemWidthsForSymmetricExtensible(LayoutWrapper* layoutWrapper, int32_t childCount,
    const std::vector<float>& spaceRequests, const std::vector<float>& leftBuffers,
    const std::vector<float>& rightBuffers, float allocatedWidth)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);

    if ((static_cast<int32_t>(spaceRequests.size()) != childCount) ||
        (static_cast<int32_t>(leftBuffers.size()) != childCount) ||
        (static_cast<int32_t>(rightBuffers.size()) != childCount) ||
        (static_cast<int32_t>(itemWidths_.size()) != childCount)) {
        return;
    }

    for (int32_t index = 0; index < childCount; ++index) {
        if (tabBarPattern->GetTabBarStyle(index) != TabBarStyle::BOTTOMTABBATSTYLE) {
            itemWidths_[index] = allocatedWidth;
            continue;
        }
        if (!NearZero(spaceRequests[index])) {
            itemWidths_[index] = allocatedWidth + spaceRequests[index] * 2;
        } else if (!NearZero(leftBuffers[index]) || !NearZero(rightBuffers[index])) {
            itemWidths_[index] = allocatedWidth - leftBuffers[index] - rightBuffers[index];
            auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
            CHECK_NULL_VOID(childWrapper);
            // Adjust margin to keep the position of current item.
            auto leftMargin = rightBuffers[index];
            auto rightMargin = leftBuffers[index];
            if (GreatNotEqual(leftMargin, rightMargin)) {
                leftMargin -= rightMargin;
                rightMargin = 0.0f;
            } else {
                rightMargin -= leftMargin;
                leftMargin = 0.0f;
            }
            UpdateChildMarginProperty(rightMargin, leftMargin, childWrapper);
        } else {
            itemWidths_[index] = allocatedWidth;
        }
    }
}

void TabBarLayoutAlgorithm::UpdateChildMarginProperty(
    float rightMargin, float leftMargin, const RefPtr<LayoutWrapper>& childWrapper)
{
    auto linearLayoutProperty = AceType::DynamicCast<LinearLayoutProperty>(childWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(linearLayoutProperty);
    auto textWrapper = childWrapper->GetOrCreateChildByIndex(1);
    CHECK_NULL_VOID(textWrapper);
    auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateMargin(
        { CalcLength(Dimension(leftMargin)), CalcLength(Dimension(rightMargin)), {}, {} });
    auto iconWrapper = childWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(iconWrapper);
    if (iconWrapper->GetHostNode()->GetTag() == V2::SYMBOL_ETS_TAG) {
        auto symbolLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(iconWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(symbolLayoutProperty);
        symbolLayoutProperty->UpdateMargin(
            { CalcLength(Dimension(leftMargin)), CalcLength(Dimension(rightMargin)), {}, {} });
        if (linearLayoutProperty->GetFlexDirection().value_or(FlexDirection::COLUMN) == FlexDirection::ROW) {
            symbolLayoutProperty->UpdateMargin({ CalcLength(Dimension(leftMargin)), {}, {}, {} });
            textLayoutProperty->UpdateMargin({ {}, CalcLength(Dimension(rightMargin)), {}, {} });
        }
    } else {
        auto imageLayoutProperty = AceType::DynamicCast<ImageLayoutProperty>(iconWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(imageLayoutProperty);
        imageLayoutProperty->UpdateMargin(
            { CalcLength(Dimension(leftMargin)), CalcLength(Dimension(rightMargin)), {}, {} });
        if (linearLayoutProperty->GetFlexDirection().value_or(FlexDirection::COLUMN) == FlexDirection::ROW) {
            imageLayoutProperty->UpdateMargin({ CalcLength(Dimension(leftMargin)), {}, {}, {} });
            textLayoutProperty->UpdateMargin({ {}, CalcLength(Dimension(rightMargin)), {}, {} });
        }
    }
}

float TabBarLayoutAlgorithm::ApplyBarGridAlign(
    const RefPtr<TabBarLayoutProperty>& layoutProperty, const SizeF& frameSize) const
{
    if (!layoutProperty->GetBarGridAlign()) {
        return 0.0f;
    }
    auto option = layoutProperty->GetBarGridAlign().value();
    auto gridSizeType = GetGridSizeType(frameSize);
    int32_t columnNum = -1;
    if (gridSizeType == GridSizeType::SM) {
        columnNum = option.sm;
        if (columnNum > SM_COLUMN_NUM) {
            return 0.0f;
        }
    } else if (gridSizeType == GridSizeType::MD) {
        columnNum = option.md;
        if (columnNum > MD_COLUMN_NUM) {
            return 0.0f;
        }
    } else if (gridSizeType == GridSizeType::LG) {
        columnNum = option.lg;
        if (columnNum > LG_COLUMN_NUM) {
            return 0.0f;
        }
    } else {
        return 0.0f;
    }
    if (columnNum < 0 || columnNum % 2) {
        return 0.0f;
    }
    auto gridWidth = GetGridWidth(option, frameSize, columnNum);
    return (frameSize.Width() - gridWidth) / 2;
}

void TabBarLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    tabItemOffset_.clear();
    CHECK_NULL_VOID(layoutWrapper);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    axis_ = GetAxis(layoutWrapper);
    if ((axis_ == Axis::VERTICAL && NearZero(geometryNode->GetFrameSize().Width())) ||
        (axis_ == Axis::HORIZONTAL && NearZero(geometryNode->GetFrameSize().Height()))) {
        return;
    }

    auto frameSize = geometryNode->GetPaddingSize();
    auto tabsNode = AceType::DynamicCast<TabsNode>(layoutWrapper->GetHostNode()->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto tabLayoutProperty = AceType::DynamicCast<TabsLayoutProperty>(tabsNode->GetLayoutProperty());
    CHECK_NULL_VOID(tabLayoutProperty);
    isRTL_ = tabLayoutProperty->GetNonAutoLayoutDirection() == TextDirection::RTL;
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    int32_t indicator = layoutProperty->GetIndicatorValue(0);
    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::FIXED &&
        tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE && axis_ == Axis::VERTICAL) {
        indicator_ = indicator;
        auto space = frameSize.Height() / 4;
        OffsetF childOffset = OffsetF(0.0f, space);
        LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
        return;
    }
    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::FIXED &&
        tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
        indicator_ = indicator;
        currentOffset_ = 0.0f;
        OffsetF childOffset = OffsetF(0.0f, 0.0f);
        LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
        return;
    }
    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE &&
        childrenMainSize_ <= frameSize.MainSize(axis_)) {
        indicator_ = indicator;
        auto frontSpace = (frameSize.MainSize(axis_) - childrenMainSize_) / 2;
        OffsetF childOffset = (axis_ == Axis::HORIZONTAL ? OffsetF(frontSpace, 0.0f) : OffsetF(0.0f, frontSpace));
        LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
        return;
    }
    if ((indicator != indicator_ || (indicator == indicator_ && needSetCentered_)) &&
        layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        if (childrenMainSize_ > frameSize.MainSize(axis_) && tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE &&
            axis_ == Axis::HORIZONTAL) {
            OffsetF childOffset = OffsetF(currentOffset_, 0.0f);
            indicator_ = indicator;
            LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
            return;
        }
        indicator_ = indicator;
        auto space = GetSpace(layoutWrapper, indicator, frameSize, axis_);
        float frontChildrenMainSize = CalculateFrontChildrenMainSize(layoutWrapper, indicator, axis_);
        if (space < 0.0f) {
            OffsetF childOffset = (axis_ == Axis::HORIZONTAL ? OffsetF(-frontChildrenMainSize, 0.0f)
                                                            : OffsetF(0.0f, -frontChildrenMainSize));
            currentOffset_ = -frontChildrenMainSize;
            LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
            return;
        }
        if (frontChildrenMainSize < space) {
            OffsetF childOffset = OffsetF(0.0f, 0.0f);
            currentOffset_ = 0.0f;
            LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
            return;
        }
        float backChildrenMainSize = CalculateBackChildrenMainSize(layoutWrapper, indicator, axis_);
        if (backChildrenMainSize < space) {
            auto scrollableDistance = std::max(childrenMainSize_ - frameSize.MainSize(axis_), 0.0f);
            currentOffset_ = -scrollableDistance;
            OffsetF childOffset =
                (axis_ == Axis::HORIZONTAL ? OffsetF(-scrollableDistance, 0.0f) : OffsetF(0.0f, -scrollableDistance));
            LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
            return;
        }
        auto scrollableDistance = std::max(frontChildrenMainSize - space, 0.0f);
        currentOffset_ = -scrollableDistance;
        OffsetF childOffset =
            (axis_ == Axis::HORIZONTAL ? OffsetF(-scrollableDistance, 0.0f) : OffsetF(0.0f, -scrollableDistance));
        LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
        return;
    }
    if (previousChildrenMainSize_ != childrenMainSize_) {
        auto scrollableDistance = std::max(childrenMainSize_ - frameSize.MainSize(axis_), 0.0f);
        currentOffset_ = std::clamp(currentOffset_, -scrollableDistance, 0.0f);
    }
    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::FIXED) {
        currentOffset_ = 0.0f;
    }
    OffsetF childOffset = (axis_ == Axis::HORIZONTAL ? OffsetF(currentOffset_, 0.0f) : OffsetF(0.0f, currentOffset_));
    indicator_ = indicator;
    LayoutChildren(layoutWrapper, frameSize, axis_, childOffset);
}

Axis TabBarLayoutAlgorithm::GetAxis(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, Axis::HORIZONTAL);
    return layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
}

float TabBarLayoutAlgorithm::GetSpace(
    LayoutWrapper* layoutWrapper, int32_t indicator, const SizeF& frameSize, Axis axis)
{
    if (isRTL_ && axis_ == Axis::HORIZONTAL) {
        auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
        indicator = childCount - indicator - 1;
    }
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(indicator);
    if (!childWrapper) {
        return 0.0f;
    }
    auto childGeometryNode = childWrapper->GetGeometryNode();
    auto childFrameSize = childGeometryNode->GetMarginFrameSize();
    return (frameSize.MainSize(axis) - childFrameSize.MainSize(axis)) / 2;
}

float TabBarLayoutAlgorithm::CalculateFrontChildrenMainSize(LayoutWrapper* layoutWrapper, int32_t indicator, Axis axis)
{
    if (isRTL_ && axis_ == Axis::HORIZONTAL) {
        auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
        indicator = childCount - indicator - 1;
    }
    float frontChildrenMainSize = scrollMargin_;
    for (int32_t index = 0; index < indicator; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!childWrapper) {
            return 0.0f;
        }
        auto childGeometryNode = childWrapper->GetGeometryNode();
        auto childFrameSize = childGeometryNode->GetMarginFrameSize();
        frontChildrenMainSize += childFrameSize.MainSize(axis);
    }
    return indicator == 0 ? 0.0f : frontChildrenMainSize;
}

void TabBarLayoutAlgorithm::LayoutChildren(
    LayoutWrapper* layoutWrapper, const SizeF& frameSize, Axis axis, OffsetF& childOffset)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
    if (childCount < 0) {
        return;
    }
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    if (layoutProperty->GetPaddingProperty()) {
        childOffset += OffsetF(
            layoutProperty->GetPaddingProperty()->left.value_or(CalcLength(0.0_vp)).GetDimension().ConvertToPx(), 0.0f);
    }

    if (layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE &&
        childrenMainSize_ > frameSize.MainSize(axis) && axis == Axis::HORIZONTAL) {
        childOffset += OffsetF(scrollMargin_, 0);
    }

    std::vector<OffsetF> childOffsetDelta;
    for (int32_t index = 0; index < childCount; ++index) {
        auto pos = (isRTL_ && axis_ == Axis::HORIZONTAL)? (childCount - index -1):index;
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(pos);
        if (!childWrapper) {
            childOffsetDelta.emplace_back(OffsetF());
            continue;
        }
        auto childGeometryNode = childWrapper->GetGeometryNode();
        auto childFrameSize = childGeometryNode->GetMarginFrameSize();
        OffsetF centerOffset = (axis == Axis::HORIZONTAL)
                                   ? OffsetF(0, (frameSize.Height() - childFrameSize.Height()) / 2.0)
                                   : OffsetF((frameSize.Width() - childFrameSize.Width()) / 2.0, 0);
        childOffsetDelta.emplace_back(childOffset + centerOffset - childGeometryNode->GetMarginFrameOffset());
        childGeometryNode->SetMarginFrameOffset(childOffset + centerOffset);
        childWrapper->Layout();
        tabItemOffset_.emplace_back(childOffset);

        childOffset +=
            axis == Axis::HORIZONTAL ? OffsetF(childFrameSize.Width(), 0.0f) : OffsetF(0.0f, childFrameSize.Height());
    }
    tabItemOffset_.emplace_back(childOffset);
    LayoutMask(layoutWrapper, childOffsetDelta);
}

void TabBarLayoutAlgorithm::LayoutMask(LayoutWrapper* layoutWrapper, const std::vector<OffsetF>& childOffsetDelta)
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
    if (childCount < 0) {
        return;
    }
    auto selectedMaskWrapper = layoutWrapper->GetOrCreateChildByIndex(childCount);
    CHECK_NULL_VOID(selectedMaskWrapper);
    auto unselectedMaskWrapper = layoutWrapper->GetOrCreateChildByIndex(childCount + 1);
    CHECK_NULL_VOID(unselectedMaskWrapper);
    for (int32_t i = 0; i < MASK_COUNT; i++) {
        auto currentWrapper = (i == 0 ? selectedMaskWrapper : unselectedMaskWrapper);
        auto currentMask = (i == 0 ? layoutProperty->GetSelectedMask().value_or(-1)
                                   : layoutProperty->GetUnselectedMask().value_or(-1));
        if (currentMask < 0) {
            currentWrapper->GetGeometryNode()->SetFrameSize(SizeF());
            currentWrapper->Layout();
            currentWrapper->SetActive(false);
        } else {
            auto offset = currentWrapper->GetGeometryNode()->GetMarginFrameOffset();
            currentWrapper->GetGeometryNode()->SetMarginFrameOffset(offset + childOffsetDelta[currentMask]);
            auto imageWrapper = currentWrapper->GetOrCreateChildByIndex(0);
            CHECK_NULL_VOID(imageWrapper);
            auto imageNode = imageWrapper->GetHostNode();
            CHECK_NULL_VOID(imageNode);
            auto imageRenderContext = imageNode->GetRenderContext();
            CHECK_NULL_VOID(imageRenderContext);
            imageRenderContext->SetVisible(true);
            currentWrapper->Layout();
            currentWrapper->SetActive(true);
        }
    }
}

float TabBarLayoutAlgorithm::CalculateBackChildrenMainSize(LayoutWrapper* layoutWrapper, int32_t indicator, Axis axis)
{
    if (isRTL_ && axis_ == Axis::HORIZONTAL) {
        auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
        indicator = childCount - indicator - 1;
    }
    float backChildrenMainSize = scrollMargin_;
    auto childCount = layoutWrapper->GetTotalChildCount() - MASK_COUNT;
    for (int32_t index = indicator + 1; index < childCount; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!childWrapper) {
            return 0.0f;
        }
        auto childGeometryNode = childWrapper->GetGeometryNode();
        auto childFrameSize = childGeometryNode->GetMarginFrameSize();
        backChildrenMainSize += childFrameSize.MainSize(axis);
    }
    return indicator == childCount - 1 ? 0.0f : backChildrenMainSize;
}

GridSizeType TabBarLayoutAlgorithm::GetGridSizeType(const SizeF& frameSize) const
{
    auto gridColumnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::TAB_BAR);
    CHECK_NULL_RETURN(gridColumnInfo, GridSizeType::UNDEFINED);
    auto parent = gridColumnInfo->GetParent();
    CHECK_NULL_RETURN(parent, GridSizeType::UNDEFINED);
    parent->BuildColumnWidth(frameSize.Width());
    return parent->GetSizeType();
}

float TabBarLayoutAlgorithm::GetGridWidth(
    const BarGridColumnOptions& option, const SizeF& frameSize, int32_t columns) const
{
    auto gridColumnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::TAB_BAR);
    CHECK_NULL_RETURN(gridColumnInfo, 0.0f);
    auto parent = gridColumnInfo->GetParent();
    CHECK_NULL_RETURN(parent, 0.0f);
    parent->SetGutterWidth(option.gutter);
    parent->SetMarginLeft(option.margin);
    parent->SetMarginRight(option.margin);
    parent->BuildColumnWidth(frameSize.Width());
    if (columns < 0) {
        return gridColumnInfo->GetMaxWidth();
    }
    return gridColumnInfo->GetWidth(columns);
}

float TabBarLayoutAlgorithm::GetContentWidth(
    const RefPtr<TabBarLayoutProperty>& layoutProperty, const SizeF& frameSize) const
{
    const auto& padding = layoutProperty->GetPaddingProperty();
    CHECK_NULL_RETURN(padding, frameSize.Width());
    auto left = padding->left.value_or(CalcLength(0.0_vp)).GetDimension().ConvertToPx();
    auto right = padding->right.value_or(CalcLength(0.0_vp)).GetDimension().ConvertToPx();
    return Positive(frameSize.Width() - left - right) ? frameSize.Width() - left - right : 0.0f;
}

void TabBarLayoutAlgorithm::UpdateHorizontalPadding(LayoutWrapper* layoutWrapper, float horizontalPadding) const
{
    auto layoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    layoutProperty->UpdatePadding(
        { CalcLength(Dimension(horizontalPadding)), CalcLength(Dimension(horizontalPadding)), {}, {} });
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto hostLayoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(hostLayoutProperty);
    hostLayoutProperty->UpdatePadding(
        { CalcLength(Dimension(horizontalPadding)), CalcLength(Dimension(horizontalPadding)), {}, {} });
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    geometryNode->UpdatePaddingWithBorder({ horizontalPadding, horizontalPadding, 0.0f, 0.0f });
}

void TabBarLayoutAlgorithm::AdjustFixedItem(
    const RefPtr<LayoutWrapper>& childWrapper, const OptionalSizeF& frameSize, Axis axis) const
{
    auto geometryNode = childWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    if (axis == Axis::HORIZONTAL &&
        GreatNotEqual(geometryNode->GetMarginFrameSize().MainSize(axis), frameSize.Width().value_or(0.0f))) {
        geometryNode->SetFrameSize(
            SizeF(frameSize.Width().value_or(0.0f), geometryNode->GetMarginFrameSize().MainSize(Axis::VERTICAL)));
    } else if (axis == Axis::VERTICAL &&
               GreatNotEqual(geometryNode->GetMarginFrameSize().MainSize(axis), frameSize.Height().value_or(0.0f))) {
        geometryNode->SetFrameSize(
            SizeF(geometryNode->GetMarginFrameSize().MainSize(Axis::HORIZONTAL), frameSize.Height().value_or(0.0f)));
    }
}
} // namespace OHOS::Ace::NG

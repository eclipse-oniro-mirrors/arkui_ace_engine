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

#include "core/components_ng/pattern/navigation/navigation_layout_algorithm.h"

#include <cmath>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

constexpr Dimension WINDOW_WIDTH = 520.0_vp;

namespace {

void MeasureDivider(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, const SizeF& dividerSize)
{
    auto dividerNode = hostNode->GetDividerNode();
    CHECK_NULL_VOID(dividerNode);
    auto index = hostNode->GetChildIndexById(dividerNode->GetId());
    auto divider = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_VOID(divider);
    auto constraint = navigationLayoutProperty->CreateChildConstraint();
    constraint.selfIdealSize = OptionalSizeF(dividerSize.Width(), dividerSize.Height());
    divider->Measure(constraint);
}

void MeasureNavBar(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, const SizeF& navBarSize)
{
    auto navBarNode = hostNode->GetNavBarNode();
    CHECK_NULL_VOID(navBarNode);
    auto index = hostNode->GetChildIndexById(navBarNode->GetId());
    auto navBar = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_VOID(navBar);
    auto constraint = navigationLayoutProperty->CreateChildConstraint();
    constraint.selfIdealSize = OptionalSizeF(navBarSize.Width(), navBarSize.Height());
    navBar->Measure(constraint);
}

void MeasureContentChild(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, const SizeF& contentSize)
{
    auto contentNode = hostNode->GetContentNode();
    CHECK_NULL_VOID(contentNode);
    auto index = hostNode->GetChildIndexById(contentNode->GetId());
    auto content = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_VOID(content);
    auto constraint = navigationLayoutProperty->CreateChildConstraint();
    constraint.selfIdealSize = OptionalSizeF(contentSize.Width(), contentSize.Height());
    content->Measure(constraint);
}

float LayoutNavBar(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, const NavBarPosition& position)
{
    if (navigationLayoutProperty->GetHideNavBar().value_or(false) &&
        navigationLayoutProperty->GetNavigationModeValue() == NavigationMode::SPLIT) {
        return 0.0f;
    }

    auto contentNode = hostNode->GetContentNode();
    CHECK_NULL_RETURN(contentNode, 0.0f);
    auto navBarNode = hostNode->GetNavBarNode();
    CHECK_NULL_RETURN(navBarNode, 0.0f);
    auto index = hostNode->GetChildIndexById(navBarNode->GetId());
    auto navBar = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_RETURN(navBar, 0.0f);
    auto geometryNode = navBar->GetGeometryNode();
    auto navigationGeometryNode = frameNode->GetGeometryNode();
    if (position == NavBarPosition::END) {
        auto navBarOffset =
            OffsetT<float>(navigationGeometryNode->GetFrameSize().Width() - geometryNode->GetFrameSize().Width(),
                geometryNode->GetFrameOffset().GetY());
        geometryNode->SetMarginFrameOffset(navBarOffset);
        navBar->Layout();
        return geometryNode->GetFrameSize().Width();
    }
    if ((navigationLayoutProperty->GetDestinationChange().value_or(false) ||
            contentNode->FindChildNodeOfClass<NavDestinationGroupNode>()) &&
        navigationLayoutProperty->GetNavigationMode() == NavigationMode::STACK) {
        auto contentOffset = OffsetT<float>(-geometryNode->GetFrameSize().Width(), 0.0f);
        geometryNode->SetMarginFrameOffset(contentOffset);
        navBar->Layout();
        return geometryNode->GetFrameSize().Width();
    }
    auto navBarOffset = OffsetT<float>(0.0f, 0.0f);
    geometryNode->SetMarginFrameOffset(navBarOffset);
    navBar->Layout();
    return geometryNode->GetFrameSize().Width();
}

float LayoutDivider(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, float navBarWidth, const NavBarPosition& position)
{
    auto dividerNode = hostNode->GetDividerNode();
    CHECK_NULL_RETURN(dividerNode, 0.0f);
    auto index = hostNode->GetChildIndexById(dividerNode->GetId());
    auto divider = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_RETURN(divider, 0.0f);
    auto geometryNode = divider->GetGeometryNode();
    auto navigationGeometryNode = frameNode->GetGeometryNode();
    OffsetT<float> dividerOffset;
    if (position == NavBarPosition::END) {
        dividerOffset = OffsetT<float>(
            navigationGeometryNode->GetFrameSize().Width() - geometryNode->GetFrameSize().Width() - navBarWidth,
            geometryNode->GetFrameOffset().GetY());
    } else {
        dividerOffset = OffsetT<float>(navBarWidth, geometryNode->GetFrameOffset().GetY());
    }
    geometryNode->SetMarginFrameOffset(dividerOffset);
    divider->Layout();
    return geometryNode->GetFrameSize().Width();
}

void LayoutContent(FrameNode* frameNode, const RefPtr<NavigationGroupNode>& hostNode,
    const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, float navBarWidth, float dividerWidth,
    const NavBarPosition& position)
{
    auto contentNode = hostNode->GetContentNode();
    CHECK_NULL_VOID(contentNode);
    auto index = hostNode->GetChildIndexById(contentNode->GetId());
    auto content = frameNode->GetFrameNodeByIndex(index);
    CHECK_NULL_VOID(content);
    auto geometryNode = content->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    const auto& navigationGeometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(navigationGeometryNode);

    OffsetT<float> contentOffset;
    if ((navigationLayoutProperty->GetDestinationChange().value_or(false) ||
            contentNode->FindChildNodeOfClass<NavDestinationGroupNode>()) &&
        navigationLayoutProperty->GetNavigationMode() == NavigationMode::STACK) {
        contentOffset = OffsetT<float>(0.0f, 0.0f);
        geometryNode->SetMarginFrameOffset(contentOffset);
        content->Layout();
        return;
    }
    if (position == NavBarPosition::END) {
        navigationLayoutProperty->GetNavigationMode() == NavigationMode::STACK
            ? contentOffset = OffsetT<float>(-geometryNode->GetFrameSize().Width(), 0.0f)
            : contentOffset = OffsetT<float>(0.0f, 0.0f);
        geometryNode->SetMarginFrameOffset(contentOffset);
        content->Layout();
        return;
    }
    navigationLayoutProperty->GetNavigationMode() == NavigationMode::STACK
        ? contentOffset =
              OffsetT<float>(navigationGeometryNode->GetFrameSize().Width(), geometryNode->GetFrameOffset().GetY())
        : contentOffset = OffsetT<float>(navBarWidth + dividerWidth, geometryNode->GetFrameOffset().GetY());
    geometryNode->SetMarginFrameOffset(contentOffset);
    content->Layout();
}

void FitScrollFullWindow(SizeF& frameSize)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID_NOLOG(pipeline);
    if (frameSize.Width() == Infinity<float>()) {
        frameSize.SetWidth(pipeline->GetRootWidth());
    }
    if (frameSize.Height() == Infinity<float>()) {
        frameSize.SetHeight(pipeline->GetRootHeight());
    }
}

} // namespace

void NavigationLayoutAlgorithm::Measure(FrameNode* frameNode)
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(Claim(frameNode));
    CHECK_NULL_VOID(hostNode);
    auto contentNode = hostNode->GetContentNode();
    CHECK_NULL_VOID(contentNode);
    auto navigationLayoutProperty = AceType::DynamicCast<NavigationLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(navigationLayoutProperty);
    const auto& constraint = navigationLayoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(constraint);
    auto pattern = AceType::DynamicCast<NavigationPattern>(hostNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    auto geometryNode = frameNode->GetGeometryNode();
    auto size = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT, true);
    FitScrollFullWindow(size);
    const auto& padding = frameNode->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);

    usrNavigationMode_ = navigationLayoutProperty->GetUsrNavigationModeValue(NavigationMode::AUTO);
    navigationMode_ = usrNavigationMode_;
    navigationLayoutProperty->UpdateNavigationMode(navigationMode_);
    if (navigationMode_ == NavigationMode::AUTO) {
        if (size.Width() >= static_cast<float>(WINDOW_WIDTH.ConvertToPx())) {
            navigationMode_ = NavigationMode::SPLIT;
            navigationLayoutProperty->UpdateNavigationMode(navigationMode_);
        } else {
            navigationMode_ = NavigationMode::STACK;
            navigationLayoutProperty->UpdateNavigationMode(navigationMode_);
        }
    } else {
        pattern->SetNavigationMode(navigationMode_);
    }

    auto navBarSize = size;
    auto contentSize = size;
    auto dividerSize = SizeF(0.0f, 0.0f);
    float navBarWidth = 0.0f;
    float contentWidth = 0.0f;
    float dividerWidth = 0.0f;
    auto navBarWidthValue = navigationLayoutProperty->GetNavBarWidthValue(DEFAULT_NAV_BAR_WIDTH);
    bool isPercentSize = (navBarWidthValue.Unit() == DimensionUnit::PERCENT);
    if (isPercentSize) {
        navBarWidth = std::floor(static_cast<float>(navBarWidthValue.Value() * size.Width()));
    } else {
        navBarWidth = std::floor(
            static_cast<float>(navigationLayoutProperty->GetNavBarWidthValue(DEFAULT_NAV_BAR_WIDTH).ConvertToPx()));
    }

    if (navigationLayoutProperty->GetNavigationModeValue(navigationMode_) == NavigationMode::SPLIT) {
        if (navigationLayoutProperty->GetHideNavBar().value_or(false)) {
            contentWidth = size.Width();
            navBarSize.SetWidth(0.0f);
            navBarSize.SetHeight(0.0f);
            contentSize.SetWidth(contentWidth);
            MeasureNavBar(frameNode, hostNode, navigationLayoutProperty, navBarSize);
            MeasureContentChild(frameNode, hostNode, navigationLayoutProperty, contentSize);
            MeasureDivider(frameNode, hostNode, navigationLayoutProperty, dividerSize);
            frameNode->GetGeometryNode()->SetFrameSize(size);
            return;
        }
        contentWidth = std::floor(size.Width() - static_cast<float>(DIVIDER_WIDTH.ConvertToPx()) - navBarWidth);
        dividerWidth = size.Width() - navBarWidth - contentWidth;
        navBarSize.SetWidth(navBarWidth);
        contentSize.SetWidth(contentWidth);
        dividerSize.SetWidth(dividerWidth);
        dividerSize.SetHeight(size.Height());
    } else {
        if (navigationLayoutProperty->GetDestinationChange().value_or(false) ||
            contentNode->FindChildNodeOfClass<NavDestinationGroupNode>()) {
            navigationLayoutProperty->GetNavBarPositionValue(NavBarPosition::START) == NavBarPosition::END
                ? navBarSize.SetWidth(0.0f)
                : navBarSize.SetWidth(navBarWidth);
        } else {
            if (navigationLayoutProperty->GetNavBarPositionValue(NavBarPosition::START) == NavBarPosition::END) {
                contentWidth = std::floor(size.Width() - static_cast<float>(DIVIDER_WIDTH.ConvertToPx()) - navBarWidth);
            }
            contentSize.SetWidth(contentWidth);
        }
    }
    MeasureNavBar(frameNode, hostNode, navigationLayoutProperty, navBarSize);
    MeasureContentChild(frameNode, hostNode, navigationLayoutProperty, contentSize);
    MeasureDivider(frameNode, hostNode, navigationLayoutProperty, dividerSize);
    frameNode->GetGeometryNode()->SetFrameSize(size);
}

void NavigationLayoutAlgorithm::Layout(FrameNode* frameNode)
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(Claim(frameNode));
    CHECK_NULL_VOID(hostNode);
    auto navigationLayoutProperty = AceType::DynamicCast<NavigationLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(navigationLayoutProperty);
    auto navBarPosition = navigationLayoutProperty->GetNavBarPositionValue(NavBarPosition::START);
    float navBarWidth = LayoutNavBar(frameNode, hostNode, navigationLayoutProperty, navBarPosition);
    float dividerWidth = LayoutDivider(frameNode, hostNode, navigationLayoutProperty, navBarWidth, navBarPosition);
    LayoutContent(frameNode, hostNode, navigationLayoutProperty, navBarWidth, dividerWidth, navBarPosition);
}

} // namespace OHOS::Ace::NG

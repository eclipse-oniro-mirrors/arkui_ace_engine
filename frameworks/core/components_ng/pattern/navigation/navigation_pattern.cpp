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

#include "core/components_ng/pattern/navigation/navigation_pattern.h"

#include "base/mousestyle/mouse_style.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_event_hub.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_model_data.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_event_hub.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/navrouter/navrouter_group_node.h"
#include "core/gestures/gesture_info.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

constexpr int32_t NAVIMODE_CHANGE_ANIMATION_DURATION = 250;
constexpr int32_t OPACITY_ANIMATION_DURATION_APPEAR = 150;
constexpr int32_t OPACITY_ANIMATION_DURATION_DISAPPEAR = 250;
constexpr Dimension DEFAULT_DRAG_REGION = 20.0_vp;
constexpr float DEFAULT_HALF = 2.0f;

namespace {

constexpr static int32_t PLATFORM_VERSION_TEN = 10;

void MountNavBar(const RefPtr<NavigationGroupNode>& hostNode)
{
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    navBarNode->MarkModifyDone();
}

} // namespace

RefPtr<RenderContext> NavigationPattern::GetTitleBarRenderContext()
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, nullptr);
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    auto contentNode = AceType::DynamicCast<FrameNode>(hostNode->GetContentNode());
    CHECK_NULL_RETURN(contentNode, nullptr);
    if (layoutProperty->GetDestinationChange().value_or(false) ||
        contentNode->FindChildNodeOfClass<NavDestinationGroupNode>()) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
        CHECK_NULL_RETURN(navBarNode, nullptr);
        auto renderContext = navBarNode->GetRenderContext();
        return renderContext;
    } else {
        auto renderContext = contentNode->GetRenderContext();
        return renderContext;
    }
}

void NavigationPattern::DoAnimation(NavigationMode currentMode)
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto usrNavigationMode = layoutProperty->GetUsrNavigationModeValue(NavigationMode::AUTO);

    auto lastMode = navigationMode_;
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    layoutProperty->UpdateUsrNavigationMode(lastMode);
    hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    AnimationOption option = AnimationOption();
    option.SetDuration(NAVIMODE_CHANGE_ANIMATION_DURATION);
    option.SetCurve(Curves::FRICTION);
    option.SetFillMode(FillMode::FORWARDS);
    AnimationOption optionAlpha = AnimationOption();
    optionAlpha.SetCurve(Curves::SHARP);
    optionAlpha.SetFillMode(FillMode::FORWARDS);
    auto renderContext = GetTitleBarRenderContext();
    CHECK_NULL_VOID(renderContext);

    std::function<void()> finishCallback = [optionAlpha, renderContext, hostNode]() {
        renderContext->OpacityAnimation(optionAlpha, 0, 1);
        hostNode->SetIsModeChange(false);
        hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    };

    context->OpenImplicitAnimation(option, option.GetCurve(), finishCallback);
    layoutProperty->UpdateUsrNavigationMode(currentMode);
    hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    context->FlushUITasks();
    if (currentMode == NavigationMode::STACK || lastMode == NavigationMode::SPLIT) {
        optionAlpha.SetDuration(OPACITY_ANIMATION_DURATION_DISAPPEAR);
        renderContext->OpacityAnimation(optionAlpha, 1, 0);
    } else if (currentMode == NavigationMode::SPLIT || lastMode == NavigationMode::STACK) {
        optionAlpha.SetDuration(OPACITY_ANIMATION_DURATION_APPEAR);
        renderContext->OpacityAnimation(optionAlpha, 0, 1);
    }
    context->CloseImplicitAnimation();
    layoutProperty->UpdateUsrNavigationMode(usrNavigationMode);
    layoutProperty->UpdateNavigationMode(usrNavigationMode);
}

void NavigationPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddWindowStateChangedCallback(host->GetId());
}

void NavigationPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID_NOLOG(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
}

void NavigationPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    MountNavBar(hostNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    navBarNode->MarkModifyDone();
    auto contentNode = hostNode->GetContentNode();
    auto preTopNavPath = GetTopNavPath();
    auto prePathListSize = navPathList_.size();
    auto pathNames = navigationStack_->GetAllPathName();

    if (!navPathList_.empty()) {
        navPathList_.clear();
    }
    for (size_t i = 0; i < pathNames.size(); ++i) {
        auto pathName = pathNames[i];
        RefPtr<UINode> uiNode = navigationStack_->Get(pathName);
        navigationStack_->RemoveInNavPathList(pathName, uiNode);
        if (uiNode) {
            navPathList_.emplace_back(std::make_pair(pathName, uiNode));
            continue;
        }
        if (CheckExistPreStack(pathName)) {
            uiNode = GetNodeAndRemoveByName(pathName);
        } else {
            uiNode = GenerateUINodeByIndex(static_cast<int32_t>(i));
        }

        navPathList_.emplace_back(std::make_pair(pathName, uiNode));
    }

    navigationStack_->SetNavPathList(navPathList_);
    auto newTopNavPath = GetTopNavPath();
    if (preTopNavPath != newTopNavPath) {
        // fire onHidden and lostFocus event
        if (preTopNavPath.second != nullptr) {
            auto preTop = preTopNavPath.second;
            auto preTopNavDestination =
                AceType::DynamicCast<NavDestinationGroupNode>(NavigationGroupNode::GetNavDestinationNode(preTop));
            auto navDestinationPattern =
                AceType::DynamicCast<NavDestinationPattern>(preTopNavDestination->GetPattern());
            CHECK_NULL_VOID(navDestinationPattern);
            if (navDestinationPattern->GetIsOnShow()) {
                auto eventHub = preTopNavDestination->GetEventHub<NavDestinationEventHub>();
                CHECK_NULL_VOID(eventHub);
                eventHub->FireOnHiddenEvent();
                navDestinationPattern->SetIsOnShow(false);
            }
            auto focusHub = AceType::DynamicCast<FrameNode>(preTopNavDestination)->GetFocusHub();
            CHECK_NULL_VOID(focusHub);
            focusHub->SetParentFocusable(false);
            focusHub->LostFocus();
        }

        // fire onShown and requestFocus event
        if (newTopNavPath.second != nullptr) {
            auto newTop = newTopNavPath.second;
            auto newTopNavDestination =
                AceType::DynamicCast<NavDestinationGroupNode>(NavigationGroupNode::GetNavDestinationNode(newTop));
            auto navDestinationPattern =
                AceType::DynamicCast<NavDestinationPattern>(newTopNavDestination->GetPattern());
            CHECK_NULL_VOID(navDestinationPattern);
            if (!navDestinationPattern->GetIsOnShow()) {
                auto eventHub = newTopNavDestination->GetEventHub<NavDestinationEventHub>();
                CHECK_NULL_VOID(eventHub);
                eventHub->FireOnShownEvent();
                navDestinationPattern->SetIsOnShow(true);
            }
            auto focusHub = AceType::DynamicCast<FrameNode>(newTopNavDestination)->GetFocusHub();
            CHECK_NULL_VOID(focusHub);
            focusHub->SetParentFocusable(true);
            focusHub->RequestFocus();
        }
        auto navigationLayoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
        CHECK_NULL_VOID(navigationLayoutProperty);
        if (navigationLayoutProperty->GetNavigationModeValue(NavigationMode::AUTO) == NavigationMode::STACK) {
            DoNavigationTransitionAnimation(
                preTopNavPath.second, newTopNavPath.second, prePathListSize, pathNames.size());
        } else {
            contentNode->Clean();
            hostNode->AddNavDestinationToNavigation();
        }
        hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }

    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto currentMode = layoutProperty->GetUsrNavigationModeValue(NavigationMode::AUTO);
    if (navigationMode_ != currentMode) {
        hostNode->SetIsModeChange(true);
        DoAnimation(currentMode);
    }
    preNavPathList_ = navPathList_;

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto currentPlatformVersion = pipeline->GetMinPlatformVersion();

    if (currentPlatformVersion >= PLATFORM_VERSION_TEN) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_VOID(hub);
        auto gestureHub = hub->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        InitDragEvent(gestureHub);
        auto inputHub = hub->GetOrCreateInputEventHub();
        CHECK_NULL_VOID(inputHub);
        InitDividerMouseEvent(inputHub);
    }
}

void NavigationPattern::OnNavBarStateChange()
{
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto visibilityValue = layoutProperty->GetVisibilityValue(VisibleType::VISIBLE);
    if (visibilityValue != VisibleType::VISIBLE) {
        return;
    }

    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto eventHub = hostNode->GetEventHub<NavigationEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto currentNavigationMode = GetNavigationMode();

    if (GetNavModeChange() && (!layoutProperty->GetHideNavBarValue(false))) {
        if (currentNavigationMode == NavigationMode::SPLIT) {
            eventHub->FireNavBarStateChangeEvent(true);
        } else {
            eventHub->FireNavBarStateChangeEvent(false);
        }
        SetNavModeChange(false);
        SetNavBarVisibilityChange(false);
        return;
    }

    if (GetNavBarVisibilityChange() && (currentNavigationMode == NavigationMode::SPLIT)) {
        if (!layoutProperty->GetHideNavBarValue(false)) {
            eventHub->FireNavBarStateChangeEvent(true);
        } else {
            eventHub->FireNavBarStateChangeEvent(false);
        }
        SetNavBarVisibilityChange(false);
    }
}

void NavigationPattern::DoNavigationTransitionAnimation(const RefPtr<UINode>& preTopNavDestination,
    const RefPtr<UINode>& newTopNavDestination, int preStackSize, int newStackSize)
{
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(navigationNode);
    auto contentNode = navigationNode->GetContentNode();
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    if (newTopNavDestination != nullptr && preTopNavDestination != nullptr && preStackSize != 0 &&
        newStackSize > preStackSize) {
        contentNode->Clean();
        navigationNode->AddNavDestinationToNavigation();
        auto preNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(preTopNavDestination));
        auto newNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(newTopNavDestination));
        auto curNavTitleBarNode = AceType::DynamicCast<TitleBarNode>(preNavDestination->GetTitleBarNode());
        auto destinationTitleBarNode = AceType::DynamicCast<TitleBarNode>(newNavDestination->GetTitleBarNode());
        auto backButtonNode = AceType::DynamicCast<FrameNode>(destinationTitleBarNode->GetBackButton());
        if (curNavTitleBarNode || destinationTitleBarNode) {
            navigationNode->TitleTransitionInAnimation(curNavTitleBarNode, destinationTitleBarNode);
        }
        if (backButtonNode) {
            navigationNode->BackButtonAnimation(backButtonNode, true);
        }
        navigationNode->NavTransitionInAnimation(preNavDestination, newNavDestination);
    } else if (newTopNavDestination != nullptr && preStackSize == 0 && newStackSize > preStackSize) {
        contentNode->Clean();
        navigationNode->AddNavDestinationToNavigation();
        auto newNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(newTopNavDestination));
        auto curNavTitleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
        auto destinationTitleBarNode = AceType::DynamicCast<TitleBarNode>(newNavDestination->GetTitleBarNode());
        auto backButtonNode = AceType::DynamicCast<FrameNode>(destinationTitleBarNode->GetBackButton());
        if (curNavTitleBarNode || destinationTitleBarNode) {
            navigationNode->TitleTransitionInAnimation(curNavTitleBarNode, destinationTitleBarNode);
        }
        if (backButtonNode) {
            navigationNode->BackButtonAnimation(backButtonNode, true);
        }
        navigationNode->NavTransitionInAnimation(navBarNode, newNavDestination);
    } else if (newTopNavDestination != nullptr && newStackSize <= preStackSize && newStackSize != 0) {
        auto newNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(newTopNavDestination));
        auto preNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(preTopNavDestination));
        navigationNode->BackToPreNavDestination(newNavDestination, preNavDestination);
    } else if (newTopNavDestination == nullptr && newStackSize < preStackSize && newStackSize == 0) {
        auto preNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(preTopNavDestination));
        navigationNode->BackToNavBar(preNavDestination);
    }
}

bool NavigationPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }

    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, false);
    auto navigationLayoutProperty = AceType::DynamicCast<NavigationLayoutProperty>(hostNode->GetLayoutProperty());
    CHECK_NULL_RETURN(navigationLayoutProperty, false);

    if (ifNeedInit_) {
        RangeCalculation(hostNode, navigationLayoutProperty);
    }

    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto navigationLayoutAlgorithm =
        DynamicCast<NavigationLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(navigationLayoutAlgorithm, false);

    if (config.frameSizeChange) {
        if (navigationLayoutProperty->GetUsrNavigationModeValue(NavigationMode::AUTO) == NavigationMode::AUTO) {
            auto currentMode = navigationLayoutAlgorithm->GetNavigationMode();
            if (navigationMode_ != NavigationMode::AUTO && navigationMode_ != currentMode) {
                hostNode->SetIsModeChange(true);
                DoAnimation(currentMode);
            }
            navigationMode_ = currentMode;
        }
    }
    OnNavBarStateChange();
    UpdateTitleModeChangeEventHub(hostNode);
    UpdateResponseRegion(navigationLayoutAlgorithm->GetRealDividerWidth(),
        navigationLayoutAlgorithm->GetRealNavBarWidth(), navigationLayoutAlgorithm->GetRealNavBarHeight(),
        navigationLayoutAlgorithm->GetNavBarOffset());

    AddDividerHotZoneRect(navigationLayoutAlgorithm);
    ifNeedInit_ = false;
    return false;
}

bool NavigationPattern::UpdateTitleModeChangeEventHub(const RefPtr<NavigationGroupNode>& hostNode)
{
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
    CHECK_NULL_RETURN(navBarNode, false);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleBarNode, false);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_RETURN(titleBarLayoutProperty, false);
    auto eventHub = hostNode->GetEventHub<NavigationEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) {
        auto titleBarPattern = AceType::DynamicCast<TitleBarPattern>(titleBarNode->GetPattern());
        CHECK_NULL_RETURN(titleBarPattern, false);
        NavigationTitleMode titleMode = titleBarPattern->GetNavigationTitleMode();
        if (titleMode != NavigationTitleMode::FREE && titleMode_ != titleMode) {
            NavigationTitleModeChangeEvent navigationTitleModeChange(titleMode == NavigationTitleMode::MINI);
            eventHub->FireChangeEvent(&navigationTitleModeChange);
            titleMode_ = titleMode;
        }
    }
    return true;
}

bool NavigationPattern::CheckExistPreStack(const std::string& name)
{
    return std::any_of(preNavPathList_.begin(), preNavPathList_.end(),
        [&](const std::pair<std::string, RefPtr<UINode>>& preNamePair) { return preNamePair.first == name; });
}

RefPtr<UINode> NavigationPattern::GetNodeAndRemoveByName(const std::string& name)
{
    RefPtr<UINode> uiNode;
    // match from bottom to top
    for (auto iter = preNavPathList_.begin(); iter != preNavPathList_.end(); iter++) {
        if (iter->first == name) {
            uiNode = iter->second;
            preNavPathList_.erase(iter);
            break;
        }
    }

    return uiNode;
}

RefPtr<UINode> NavigationPattern::GenerateUINodeByIndex(int32_t index)
{
    return navigationStack_->CreateNodeByIndex(index);
}

void NavigationPattern::InitDividerMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    CHECK_NULL_VOID(inputHub);
    CHECK_NULL_VOID_NOLOG(!hoverEvent_);

    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->OnHover(isHover);
        }
    };
    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent_);
}

void NavigationPattern::HandleDragStart()
{
    preNavBarWidth_ = realNavBarWidth_;
}

void NavigationPattern::HandleDragUpdate(float xOffset)
{
    auto navigationLayoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(navigationLayoutProperty);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto frameSize = geometryNode->GetFrameSize();
    auto frameWidth = frameSize.Width();
    auto constraint = navigationLayoutProperty->GetLayoutConstraint();
    auto parentSize = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT);

    float minNavBarWidthPx = minNavBarWidthValue_.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
    float maxNavBarWidthPx = maxNavBarWidthValue_.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
    float minContentWidthPx = minContentWidthValue_.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
    auto dividerWidth = static_cast<float>(DIVIDER_WIDTH.ConvertToPx());

    auto navigationPosition = navigationLayoutProperty->GetNavBarPosition().value_or(NavBarPosition::START);
    bool isNavBarStart = navigationPosition == NavBarPosition::START;
    float navBarLine = preNavBarWidth_ + (isNavBarStart ? xOffset : -xOffset);
    float currentNavBarWidth = realNavBarWidth_;

    if (maxNavBarWidthPx + dividerWidth + minContentWidthPx > frameWidth) {
        maxNavBarWidthPx = frameWidth - minContentWidthPx - dividerWidth;
    }
    navBarLine = std::min(navBarLine, maxNavBarWidthPx);

    if (userSetMinContentFlag_ && !userSetNavBarRangeFlag_) {
        if (minContentWidthPx >= frameWidth) {
            realNavBarWidth_ = 0.0f;
        } else if (navBarLine + dividerWidth + minContentWidthPx <= frameWidth) {
            realNavBarWidth_ = navBarLine;

        } else {
            realNavBarWidth_ = frameWidth - minContentWidthPx - dividerWidth;
        }
    } else {
        realDividerWidth_ = dividerWidth;
        float remainingSpace = frameWidth - navBarLine - dividerWidth;
        if (remainingSpace >= minContentWidthPx) {
            realNavBarWidth_ = navBarLine;
        } else if (remainingSpace < minContentWidthPx && navBarLine > minNavBarWidthPx) {
            realNavBarWidth_ = frameWidth - minContentWidthPx - dividerWidth;
        } else {
            realNavBarWidth_ = minNavBarWidthPx;
        }
    }

    realNavBarWidth_ = std::min(realNavBarWidth_, frameWidth);
    realNavBarWidth_ = std::min(realNavBarWidth_, maxNavBarWidthPx);
    realNavBarWidth_ = std::max(realNavBarWidth_, minNavBarWidthPx);

    // MEASURE
    if (realNavBarWidth_ != currentNavBarWidth) {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    }
}

void NavigationPattern::HandleDragEnd()
{
    preNavBarWidth_ = realNavBarWidth_;
}

void NavigationPattern::InitDragEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!dragEvent_);
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragStart();
    };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragUpdate(static_cast<float>(info.GetOffsetX()));
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragEnd();
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragEnd();
    };
    dragEvent_ = MakeRefPtr<DragEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    PanDirection panDirection = { .type = PanDirection::HORIZONTAL };
    gestureHub->SetDragEvent(dragEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void NavigationPattern::OnHover(bool isHover)
{
    MouseFormat format = isHover ? MouseFormat::RESIZE_LEFT_RIGHT : MouseFormat::DEFAULT;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto windowId = pipeline->GetWindowId();
    auto mouseStyle = MouseStyle::CreateMouseStyle();
    int32_t currentPointerStyle = 0;
    mouseStyle->GetPointerStyle(windowId, currentPointerStyle);
    if (currentPointerStyle != static_cast<int32_t>(format)) {
        mouseStyle->SetPointerStyle(windowId, format);
    }
}

void NavigationPattern::UpdateResponseRegion(
    float realDividerWidth, float realNavBarWidth, float dragRegionHeight, OffsetF dragRectOffset)
{
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto scaleProperty = constraint->scaleProperty;
    auto halfDragRegionWidth = ConvertToPx(DEFAULT_DRAG_REGION, scaleProperty).value_or(0);
    auto dragRegionWidth = halfDragRegionWidth * 2;
    realDividerWidth_ = realDividerWidth;
    auto halfRealDividerWidth = 0.0f;
    if (realDividerWidth_ > 0.0f) {
        halfRealDividerWidth = realDividerWidth_ / DEFAULT_HALF;
    }
    halfDragRegionWidth += halfRealDividerWidth;
    dragRegionWidth += realDividerWidth_;
    realNavBarWidth_ = realNavBarWidth;

    auto navBarPosition = layoutProperty->GetNavBarPosition().value_or(NavBarPosition::START);
    if (navBarPosition == NavBarPosition::START) {
        dragRectOffset.SetX(dragRectOffset.GetX() + halfRealDividerWidth + realNavBarWidth_ - halfDragRegionWidth);
    } else {
        dragRectOffset.SetX(dragRectOffset.GetX() - halfDragRegionWidth);
    }

    dragRect_.SetOffset(dragRectOffset);
    dragRect_.SetSize(SizeF(dragRegionWidth, dragRegionHeight));

    auto eventHub = GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    gestureEventHub->MarkResponseRegion(true);
    std::vector<DimensionRect> responseRegion;
    DimensionOffset responseOffset(dragRectOffset);
    DimensionRect responseRect(Dimension(dragRect_.Width(), DimensionUnit::PX),
        Dimension(dragRect_.Height(), DimensionUnit::PX), responseOffset);
    responseRegion.emplace_back(responseRect);
    gestureEventHub->SetResponseRegion(responseRegion);
}

void NavigationPattern::AddDividerHotZoneRect(const RefPtr<NavigationLayoutAlgorithm>& layoutAlgorithm)
{
    CHECK_NULL_VOID(layoutAlgorithm);
    if (realDividerWidth_ <= 0.0f) {
        return;
    }
    OffsetF hotZoneOffset;
    hotZoneOffset.SetX(
        layoutAlgorithm->GetRealNavBarWidth() - DEFAULT_DIVIDER_HOT_ZONE_HORIZONTAL_PADDING.ConvertToPx());
    hotZoneOffset.SetY(DEFAULT_DIVIDER_START_MARGIN.ConvertToPx());
    SizeF hotZoneSize;
    hotZoneSize.SetWidth(realDividerWidth_ + DIVIDER_HOT_ZONE_HORIZONTAL_PADDING_NUM *
                                                 DEFAULT_DIVIDER_HOT_ZONE_HORIZONTAL_PADDING.ConvertToPx());
    hotZoneSize.SetHeight(layoutAlgorithm->GetRealNavBarHeight());
    DimensionRect hotZoneRegion;
    hotZoneRegion.SetSize(DimensionSize(Dimension(hotZoneSize.Width()), Dimension(hotZoneSize.Height())));
    hotZoneRegion.SetOffset(DimensionOffset(Dimension(hotZoneOffset.GetX()), Dimension(hotZoneOffset.GetY())));
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    for (auto begin = children.begin(); begin != children.end(); begin++) {
        auto dividerNode = *begin;
        if (dividerNode->GetTag() == V2::DIVIDER_ETS_TAG) {
            auto dividerFrameNode = AceType::DynamicCast<FrameNode>(dividerNode);
            CHECK_NULL_VOID(dividerFrameNode);
            dividerFrameNode->AddHotZoneRect(hotZoneRegion);
            break;
        }
    }
}

void NavigationPattern::OnWindowHide()
{
    auto curTopNavPath = GetTopNavPath();
    CHECK_NULL_VOID(curTopNavPath.second);
    auto curTopNavDestination =
        AceType::DynamicCast<NavDestinationGroupNode>(NavigationGroupNode::GetNavDestinationNode(curTopNavPath.second));
    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(curTopNavDestination->GetPattern());
    CHECK_NULL_VOID(navDestinationPattern);
    CHECK_NULL_VOID_NOLOG(navDestinationPattern->GetIsOnShow());
    auto eventHub = curTopNavDestination->GetEventHub<NavDestinationEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireOnHiddenEvent();
    navDestinationPattern->SetIsOnShow(false);
}

void NavigationPattern::OnWindowShow()
{
    auto curTopNavPath = GetTopNavPath();
    CHECK_NULL_VOID(curTopNavPath.second);
    auto curTopNavDestination =
        AceType::DynamicCast<NavDestinationGroupNode>(NavigationGroupNode::GetNavDestinationNode(curTopNavPath.second));
    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(curTopNavDestination->GetPattern());
    CHECK_NULL_VOID(navDestinationPattern);
    CHECK_NULL_VOID_NOLOG(!(navDestinationPattern->GetIsOnShow()));
    auto eventHub = curTopNavDestination->GetEventHub<NavDestinationEventHub>();
    eventHub->FireOnShownEvent();
    navDestinationPattern->SetIsOnShow(true);
}

void NavigationPattern::RangeCalculation(
    const RefPtr<NavigationGroupNode>& hostNode, const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty)
{
    const auto& constraint = navigationLayoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(constraint);
    auto parentSize = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT);
    auto frameSize = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT, true);
    float frameSizeWidth = frameSize.Width();
    Dimension defaultValue = Dimension(-1.0);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto currentPlatformVersion = pipeline->GetMinPlatformVersion();

    minContentWidthValue_ = navigationLayoutProperty->GetMinContentWidthValue(defaultValue);
    if (minContentWidthValue_ == defaultValue) {
        userSetMinContentFlag_ = false;
        minContentWidthValue_ = DEFAULT_MIN_CONTENT_WIDTH;
    } else {
        userSetMinContentFlag_ = true;
    }

    minNavBarWidthValue_ = navigationLayoutProperty->GetMinNavBarWidthValue(DEFAULT_MIN_NAV_BAR_WIDTH);
    auto userSetMaxNavBarWidthValue = navigationLayoutProperty->GetMaxNavBarWidthValue(defaultValue);

    float minNavBarWidth = std::min(static_cast<float>(minNavBarWidthValue_.ConvertToPx()), frameSizeWidth);
    float maxNavBarWidth = 0.0f;

    if (userSetMaxNavBarWidthValue == defaultValue) {
        userSetNavBarRangeFlag_ = false;
        maxNavBarWidth = std::min(
            static_cast<float>(DEFAULT_MAX_NAV_BAR_WIDTH.ConvertToPx()), frameSizeWidth * MAX_NAV_BAR_WIDTH_SCALE);
    } else {
        userSetNavBarRangeFlag_ = true;
        maxNavBarWidth = static_cast<float>(userSetMaxNavBarWidthValue.ConvertToPx());
    }

    maxNavBarWidthValue_ = Dimension(std::max(maxNavBarWidth, minNavBarWidth), DimensionUnit::PX);

    auto navBarWidthValue = navigationLayoutProperty->GetNavBarWidthValue(DEFAULT_NAV_BAR_WIDTH);
    auto navBarWidth = navBarWidthValue.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
    realNavBarWidth_ = navBarWidth;
    if (currentPlatformVersion >= PLATFORM_VERSION_TEN) {
        auto minNavBarWidth = minNavBarWidthValue_.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
        auto maxNavBarWidth = maxNavBarWidthValue_.ConvertToPxWithSize(parentSize.Width().value_or(0.0f));
        realNavBarWidth_ = std::max(realNavBarWidth_, static_cast<float>(minNavBarWidth));
        realNavBarWidth_ = std::min(realNavBarWidth_, static_cast<float>(maxNavBarWidth));
    }
}

} // namespace OHOS::Ace::NG

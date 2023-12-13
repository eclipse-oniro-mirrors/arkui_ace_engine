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

#include "base/log/log_wrapper.h"
#include "base/memory/referenced.h"
#include "base/mousestyle/mouse_style.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/recorder/event_recorder.h"
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
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

constexpr int32_t NAVIMODE_CHANGE_ANIMATION_DURATION = 250;
constexpr int32_t OPACITY_ANIMATION_DURATION_APPEAR = 150;
constexpr int32_t OPACITY_ANIMATION_DURATION_DISAPPEAR = 250;
constexpr int32_t EMPTY_DESTINATION_CHILD_SIZE = 1;
constexpr Dimension DEFAULT_DRAG_REGION = 12.0_vp;
constexpr float DEFAULT_HALF = 2.0f;

namespace {

constexpr static int32_t PLATFORM_VERSION_TEN = 10;

} // namespace

RefPtr<RenderContext> NavigationPattern::GetTitleBarRenderContext()
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, nullptr);
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    auto contentNode = AceType::DynamicCast<FrameNode>(hostNode->GetContentNode());
    CHECK_NULL_RETURN(contentNode, nullptr);
    if (contentNode->FindChildNodeOfClass<NavDestinationGroupNode>()) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
        CHECK_NULL_RETURN(navBarNode, nullptr);
        auto renderContext = navBarNode->GetRenderContext();
        return renderContext;
    } else {
        auto renderContext = contentNode->GetRenderContext();
        return renderContext;
    }
}

void NavigationPattern::DoAnimation(NavigationMode usrNavigationMode)
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    layoutProperty->UpdateNavigationMode(navigationMode_);
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
        hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    };

    context->OpenImplicitAnimation(option, option.GetCurve(), finishCallback);
    layoutProperty->UpdateNavigationMode(usrNavigationMode);
    hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    context->FlushUITasks();
    if (usrNavigationMode == NavigationMode::STACK || navigationMode_ == NavigationMode::SPLIT) {
        optionAlpha.SetDuration(OPACITY_ANIMATION_DURATION_DISAPPEAR);
        renderContext->OpacityAnimation(optionAlpha, 1, 0);
    } else if (usrNavigationMode == NavigationMode::SPLIT || navigationMode_ == NavigationMode::STACK) {
        optionAlpha.SetDuration(OPACITY_ANIMATION_DURATION_APPEAR);
        renderContext->OpacityAnimation(optionAlpha, 0, 1);
    }
    context->CloseImplicitAnimation();
    navigationMode_ = usrNavigationMode;
}

void NavigationPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddWindowStateChangedCallback(host->GetId());
    auto theme = NavigationGetTheme();
    if (theme && theme->GetNavBarUnfocusEffectEnable()) {
        pipelineContext->AddWindowFocusChangedCallback(host->GetId());
    }
    SafeAreaExpandOpts opts = {.edges = SAFE_AREA_EDGE_BOTTOM, .type = SAFE_AREA_TYPE_SYSTEM };
    host->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
}

void NavigationPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
}

void NavigationPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    navBarNode->MarkModifyDone();
    CHECK_NULL_VOID(navigationStack_);
    auto preTopNavPath = navigationStack_->GetPreTopNavPath();
    auto pathNames = navigationStack_->GetAllPathName();
    auto preSize = navigationStack_->PreSize();
    NavPathList navPathList;
    auto replaceValue = navigationStack_->GetReplaceValue();
    for (size_t i = 0; i < pathNames.size(); ++i) {
        auto pathName = pathNames[i];
        RefPtr<UINode> uiNode = navigationStack_->Get(pathName);
        auto isSameWithLast = preTopNavPath && (replaceValue == 1) && (uiNode == preTopNavPath->second)
            && (preTopNavPath->first == pathName);
        if (uiNode) {
            navigationStack_->RemoveInNavPathList(pathName, uiNode);
            navigationStack_->RemoveInPreNavPathList(pathName, uiNode);
            if (isSameWithLast) {
                uiNode = GenerateUINodeByIndex(static_cast<int32_t>(i));
            }
            navPathList.emplace_back(std::make_pair(pathName, uiNode));
            continue;
        }
        uiNode = navigationStack_->GetFromPreBackup(pathName);
        if (uiNode) {
            navigationStack_->RemoveInPreNavPathList(pathName, uiNode);
            if (isSameWithLast) {
                uiNode = GenerateUINodeByIndex(static_cast<int32_t>(i));
            }
            navPathList.emplace_back(std::make_pair(pathName, uiNode));
            continue;
        }
        uiNode = GenerateUINodeByIndex(static_cast<int32_t>(i));
        navPathList.emplace_back(std::make_pair(pathName, uiNode));
    }

    navigationStack_->SetNavPathList(navPathList);
    hostNode->UpdateNavDestinationNodeWithoutMarkDirty(
        preTopNavPath.has_value() ? preTopNavPath->second : nullptr, navigationModeChange_);
    auto newTopNavPath = navigationStack_->GetTopNavPath();
    auto size = navigationStack_->Size();
    CheckTopNavPathChange(preTopNavPath, newTopNavPath, preSize > size);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto currentPlatformVersion = pipeline->GetMinPlatformVersion();

    if (currentPlatformVersion >= PLATFORM_VERSION_TEN) {
        auto dividerNode = GetDividerNode();
        CHECK_NULL_VOID(dividerNode);
        auto gestureHub = dividerNode->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        InitDragEvent(gestureHub);
        auto inputHub = dividerNode->GetOrCreateInputEventHub();
        CHECK_NULL_VOID(inputHub);
        InitDividerMouseEvent(inputHub);
    }
}

void NavigationPattern::CheckTopNavPathChange(
    const std::optional<std::pair<std::string, RefPtr<UINode>>>& preTopNavPath,
    const std::optional<std::pair<std::string, RefPtr<UINode>>>& newTopNavPath, bool isPopPage)
{
    auto replaceValue = navigationStack_->GetReplaceValue();
    if (preTopNavPath == newTopNavPath && replaceValue != 1) {
        return;
    }
    isChanged_ = true;
    if (replaceValue == 1) {
        const int32_t replaceAnimation = 2;
        navigationStack_->UpdateReplaceValue(replaceAnimation);
    }
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto contentNode = hostNode->GetContentNode();
    CHECK_NULL_VOID(contentNode);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    // fire onHidden and lostFocus event
    RefPtr<NavDestinationGroupNode> preTopNavDestination;
    if (preTopNavPath.has_value()) {
        // pre page is not in the current stack
        isPopPage |= navigationStack_->FindIndex(preTopNavPath->first, preTopNavPath->second, true) == -1;
        preTopNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(preTopNavPath->second));
        if (preTopNavDestination) {
            auto navDestinationPattern =
                AceType::DynamicCast<NavDestinationPattern>(preTopNavDestination->GetPattern());
            CHECK_NULL_VOID(navDestinationPattern);
            if (navDestinationPattern->GetIsOnShow()) {
                auto eventHub = preTopNavDestination->GetEventHub<NavDestinationEventHub>();
                CHECK_NULL_VOID(eventHub);
                NotifyPageHide(preTopNavPath->first);
                eventHub->FireOnHiddenEvent(navDestinationPattern->GetName());
                navDestinationPattern->SetIsOnShow(false);
                // The navigations in NavDestination should be fired the hidden event
                NavigationPattern::FireNavigationStateChange(preTopNavDestination, false);
            }
            auto focusHub = preTopNavDestination->GetOrCreateFocusHub();
            focusHub->SetParentFocusable(false);
            focusHub->LostFocus();
        }
    } else {
        // navBar to new top page case
        auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
        CHECK_NULL_VOID(navBarNode);
        auto focusHub = navBarNode->GetOrCreateFocusHub();
        focusHub->SetParentFocusable(false);
        focusHub->LostFocus();
    }
    RefPtr<NavDestinationGroupNode> newTopNavDestination;
    // fire onShown and requestFocus Event
    if (newTopNavPath.has_value()) {
        newTopNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(newTopNavPath->second));
        if (newTopNavDestination) {
            auto navDestinationPattern =
                AceType::DynamicCast<NavDestinationPattern>(newTopNavDestination->GetPattern());
            CHECK_NULL_VOID(navDestinationPattern);
            if (!navDestinationPattern->GetIsOnShow()) {
                auto eventHub = newTopNavDestination->GetEventHub<NavDestinationEventHub>();
                CHECK_NULL_VOID(eventHub);
                NotifyPageShow(newTopNavPath->first);
                navDestinationPattern->SetIsOnShow(true);
            }
            auto focusHub = newTopNavDestination->GetOrCreateFocusHub();
            context->AddAfterLayoutTask([focusHub]() {
                focusHub->SetParentFocusable(true);
                focusHub->RequestFocus();
            });
        }
    } else {
        // back to navBar case
        auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
        CHECK_NULL_VOID(navBarNode);
        navBarNode->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
        auto stageManager = context->GetStageManager();
        if (stageManager != nullptr) {
            RefPtr<FrameNode> pageNode = stageManager->GetLastPage();
            CHECK_NULL_VOID(pageNode);
            auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
            if (pagePattern != nullptr) {
                auto pageInfo = pagePattern->GetPageInfo();
                NotifyPageShow(pageInfo->GetPageUrl());
            }
        }
        navBarNode->GetEventHub<EventHub>()->SetEnabledInternal(true);
        auto focusHub = navBarNode->GetOrCreateFocusHub();
        focusHub->SetParentFocusable(true);
        focusHub->RequestFocus();
    }

    bool disableAllAnimation = navigationStack_->GetDisableAnimation();
    bool animated = navigationStack_->GetAnimatedValue();
    TAG_LOGI(AceLogTag::ACE_NAVIGATION,
        "transition start, disableAllAnimation: %{public}d, animated: %{public}d, isPopPage: %{public}d",
        disableAllAnimation, animated, isPopPage);
    if (disableAllAnimation) {
        TransitionWithOutAnimation(preTopNavDestination, newTopNavDestination, isPopPage);
        return;
    }
    if (animated) {
        // animation need to run after layout task
        context->AddAfterLayoutTask(
            [preTopNavDestination, newTopNavDestination, isPopPage, weakNavigationPattern = WeakClaim(this)]() {
                auto navigationPattern = weakNavigationPattern.Upgrade();
                CHECK_NULL_VOID(navigationPattern);
                navigationPattern->TransitionWithAnimation(preTopNavDestination, newTopNavDestination, isPopPage);
            });
    } else {
        TransitionWithOutAnimation(preTopNavDestination, newTopNavDestination, isPopPage);
        navigationStack_->UpdateAnimatedValue(true);
    }
    hostNode->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
}

RefPtr<UINode> NavigationPattern::FireNavDestinationStateChange(bool show)
{
    // Only need to check top NavDestination every time.
    auto topNavPath = navigationStack_->GetTopNavPath();
    if (!topNavPath.has_value()) {
        return nullptr;
    }

    auto navDestinationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(
        NavigationGroupNode::GetNavDestinationNode(topNavPath->second));
    CHECK_NULL_RETURN(navDestinationGroupNode, nullptr);

    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(navDestinationGroupNode->GetPattern());
    CHECK_NULL_RETURN(navDestinationPattern, nullptr);

    // Same state, no need to fire event
    if (navDestinationPattern->GetIsOnShow() == show) {
        return navDestinationGroupNode;
    }

    auto eventHub = navDestinationGroupNode->GetEventHub<NavDestinationEventHub>();
    CHECK_NULL_RETURN(eventHub, nullptr);

    auto id = GetHost()->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_RETURN(pipeline, nullptr);

    if (show) {
        NotifyPageShow(topNavPath->first);
        auto param = Recorder::EventRecorder::Get().IsPageRecordEnable() ? navigationStack_->GetRouteParam() : "";
        eventHub->FireOnShownEvent(navDestinationPattern->GetName(), param);
        navDestinationPattern->SetIsOnShow(true);
        // The change from hiding to showing of top page means the navigation return to screen,
        // so add window state callback again.
        pipeline->AddWindowStateChangedCallback(id);
    } else {
        NotifyPageHide(topNavPath->first);
        eventHub->FireOnHiddenEvent(navDestinationPattern->GetName());
        navDestinationPattern->SetIsOnShow(false);
        // The change from showing to hiding of top page means the navigation leaves from screen,
        // so remove window state callback.
        pipeline->RemoveWindowStateChangedCallback(id);
    }

    return navDestinationGroupNode;
}

void NavigationPattern::FireNavigationStateChange(const RefPtr<UINode>& node, bool show)
{
    const auto& children = node->GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;

        auto navigation = AceType::DynamicCast<NavigationGroupNode>(child);
        if (navigation) {
            auto navigationPattern = AceType::DynamicCast<NavigationPattern>(navigation->GetPattern());
            CHECK_NULL_VOID(navigationPattern);
            auto changedNode = navigationPattern->FireNavDestinationStateChange(show);
            if (changedNode) {
                // Ignore node from navigation to navdestination in node tree, start from navdestination node directly.
                NavigationPattern::FireNavigationStateChange(changedNode, show);
                continue;
            }
        }
        NavigationPattern::FireNavigationStateChange(child, show);
    }
}

void NavigationPattern::NotifyPageHide(const std::string& pageName)
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pageUrlChecker = container->GetPageUrlChecker();
    CHECK_NULL_VOID(pageUrlChecker);
    pageUrlChecker->NotifyPageHide(pageName);
}

void NavigationPattern::NotifyPageShow(const std::string& pageName)
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pageUrlChecker = container->GetPageUrlChecker();
    CHECK_NULL_VOID(pageUrlChecker);
    pageUrlChecker->NotifyPageShow(pageName);
}

void NavigationPattern::TransitionWithOutAnimation(const RefPtr<NavDestinationGroupNode>& preTopNavDestination,
    const RefPtr<NavDestinationGroupNode>& newTopNavDestination, bool isPopPage)
{
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(navigationNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    // replace
    auto replaceVal = navigationStack_->GetReplaceValue();
    if (replaceVal != 0) {
        if (newTopNavDestination && preTopNavDestination) {
            navigationNode->DealNavigationExit(preTopNavDestination, false);
        } else if (newTopNavDestination && navigationMode_ == NavigationMode::STACK) {
            navigationNode->DealNavigationExit(navBarNode, true);
        }
        navigationNode->OnAccessibilityEvent(AccessibilityEventType::PAGE_CHANGE);
        navigationStack_->UpdateReplaceValue(0);
        return;
    }

    // navDestination push/pop navDestination
    if (newTopNavDestination && preTopNavDestination) {
        if (isPopPage) {
            auto parent = preTopNavDestination->GetParent();
            CHECK_NULL_VOID(parent);
            parent->RemoveChild(preTopNavDestination);
            parent->RebuildRenderContextTree();
            pipeline->RequestFrame();
        } else {
            auto layoutProperty = preTopNavDestination->GetLayoutProperty();
            CHECK_NULL_VOID(layoutProperty);
            layoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
        }
        navigationNode->OnAccessibilityEvent(AccessibilityEventType::PAGE_CHANGE);
        return;
    }

    // navBar push navDestination
    if (newTopNavDestination) {
        auto layoutProperty = navBarNode->GetLayoutProperty();
        CHECK_NULL_VOID(layoutProperty);
        layoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
    }

    // navDestination pop to navBar
    if (preTopNavDestination) {
        auto parent = preTopNavDestination->GetParent();
        CHECK_NULL_VOID(parent);
        parent->RemoveChild(preTopNavDestination);
        parent->RebuildRenderContextTree();
        pipeline->RequestFrame();
    }
    navigationNode->OnAccessibilityEvent(AccessibilityEventType::PAGE_CHANGE);
}

void NavigationPattern::TransitionWithAnimation(const RefPtr<NavDestinationGroupNode>& preTopNavDestination,
    const RefPtr<NavDestinationGroupNode>& newTopNavDestination, bool isPopPage)
{
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(navigationNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);

    // replace
    auto replaceValue = navigationStack_->GetReplaceValue();
    if (replaceValue != 0) {
        if (newTopNavDestination && preTopNavDestination) {
            navigationNode->TransitionWithReplace(preTopNavDestination, newTopNavDestination, false);
        } else if (newTopNavDestination && navigationMode_ == NavigationMode::STACK) {
            navigationNode->TransitionWithReplace(navBarNode, newTopNavDestination, true);
        }
        navigationStack_->UpdateReplaceValue(0);
        return;
    }

    // navDestination push/pop navDestination
    if (newTopNavDestination && preTopNavDestination) {
        if (isPopPage) {
            navigationNode->ExitTransitionWithPop(preTopNavDestination);
            navigationNode->EnterTransitionWithPop(newTopNavDestination);
        } else {
            navigationNode->ExitTransitionWithPush(preTopNavDestination);
            navigationNode->EnterTransitionWithPush(newTopNavDestination);
        }
        return;
    }

    auto layoutProperty = navigationNode->GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetHideNavBarValue(false)) {
        return;
    }

    // navBar push navDestination
    if (newTopNavDestination && navigationMode_ == NavigationMode::STACK) {
        navigationNode->ExitTransitionWithPush(navBarNode, true);
        navigationNode->EnterTransitionWithPush(newTopNavDestination);
        return;
    }

    // navDestination pop to navBar
    if (preTopNavDestination) {
        navigationNode->ExitTransitionWithPop(preTopNavDestination);
        if (navigationMode_ == NavigationMode::STACK) {
            navigationNode->EnterTransitionWithPop(navBarNode, true);
        }
    }
}

void NavigationPattern::OnVisibleChange(bool isVisible)
{
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto eventHub = hostNode->GetEventHub<NavigationEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireNavBarStateChangeEvent(isVisible);
}

void NavigationPattern::OnNavBarStateChange(bool modeChange)
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

    if (modeChange) {
        if (currentNavigationMode == NavigationMode::SPLIT) {
            if (layoutProperty->GetHideNavBarValue(false)) {
                eventHub->FireNavBarStateChangeEvent(false);
            } else {
                eventHub->FireNavBarStateChangeEvent(true);
            }
        } else {
            if (navigationStack_->Empty() && !layoutProperty->GetHideNavBarValue(false)) {
                eventHub->FireNavBarStateChangeEvent(true);
            } else {
                eventHub->FireNavBarStateChangeEvent(false);
            }
        }
        SetNavBarVisibilityChange(false);
        return;
    }

    if (GetNavBarVisibilityChange()) {
        if (!layoutProperty->GetHideNavBarValue(false)) {
            eventHub->FireNavBarStateChangeEvent(true);
        } else {
            eventHub->FireNavBarStateChangeEvent(false);
        }
        SetNavBarVisibilityChange(false);
        return;
    }

    // STACK mode, check navigationStack
    if (navigationStack_->Empty()) {
        eventHub->FireNavBarStateChangeEvent(true);
    } else {
        eventHub->FireNavBarStateChangeEvent(false);
    }
}

void NavigationPattern::OnNavigationModeChange(bool modeChange)
{
    if (!modeChange) {
        TAG_LOGD(AceLogTag::ACE_NAVIGATION, "navigation mode doesn't change");
        return;
    }
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto eventHub = hostNode->GetEventHub<NavigationEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireNavigationModeChangeEvent(navigationMode_);
}

bool NavigationPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto navigationLayoutAlgorithm =
        DynamicCast<NavigationLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(navigationLayoutAlgorithm, false);
    auto hostNode = AceType::DynamicCast<NavigationGroupNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, false);
    auto oldMode = navigationMode_;
    navigationMode_ = navigationLayoutAlgorithm->GetNavigationMode();
    OnNavBarStateChange(oldMode != navigationMode_);
    OnNavigationModeChange(oldMode != navigationMode_);
    auto context = PipelineContext::GetCurrentContext();
    if (context) {
        context->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), navigationStackWeak = WeakPtr<NavigationStack>(navigationStack_),
                navigationWeak = WeakPtr<NavigationGroupNode>(hostNode)] {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                auto navigationGroupNode = navigationWeak.Upgrade();
                CHECK_NULL_VOID(navigationGroupNode);
                auto navigationLayoutProperty =
                    AceType::DynamicCast<NavigationLayoutProperty>(navigationGroupNode->GetLayoutProperty());
                CHECK_NULL_VOID(navigationLayoutProperty);
                auto navigationStack = navigationStackWeak.Upgrade();
                CHECK_NULL_VOID(navigationStack);
                auto navigationContentNode = AceType::DynamicCast<FrameNode>(navigationGroupNode->GetContentNode());
                CHECK_NULL_VOID(navigationContentNode);
                auto navDestinationNode =
                    AceType::DynamicCast<NavDestinationGroupNode>(navigationContentNode->GetLastChild());
                CHECK_NULL_VOID(navDestinationNode);
                auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
                auto curTopNavPath = navigationStack->GetTopNavPath();
                if (curTopNavPath.has_value()) {
                    // considering backButton visibility
                    auto curTopNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
                        NavigationGroupNode::GetNavDestinationNode(curTopNavPath->second));
                    pattern->UpdateContextRect(curTopNavDestination, navigationGroupNode);
                    if (pattern->isChanged_ && curTopNavDestination) {
                        auto eventHub = curTopNavDestination->GetEventHub<NavDestinationEventHub>();
                        if (eventHub) {
                            auto param = Recorder::EventRecorder::Get().IsPageRecordEnable()
                                            ? navigationStack->GetRouteParam()
                                            : "";
                            eventHub->FireOnShownEvent(navDestinationPattern->GetName(), param);
                            NavigationPattern::FireNavigationStateChange(curTopNavDestination, true);
                        }
                        pattern->isChanged_ = false;
                    }
                }
                // considering navBar visibility
                auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
                CHECK_NULL_VOID(navBarNode);
                auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
                CHECK_NULL_VOID(navBarLayoutProperty);
                bool isSetInvisible =
                    (navigationGroupNode->GetNeedSetInvisible() && navigationStack->Size() != 0) ? true : false;
                if (navigationLayoutProperty->GetHideNavBar().value_or(false) ||
                    (pattern->GetNavigationMode() == NavigationMode::STACK && isSetInvisible)) {
                    navBarLayoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
                } else {
                    navBarNode->GetRenderContext()->UpdateOpacity(1.0f);
                    navBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
                }
                if (navDestinationNode->GetChildren().size() <= EMPTY_DESTINATION_CHILD_SIZE &&
                    navDestinationPattern->GetBackButtonState()) {
                    auto focusHub = navDestinationNode->GetOrCreateFocusHub();
                    focusHub->SetFocusable(true);
                    focusHub->SetParentFocusable(true);
                    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
                    CHECK_NULL_VOID(titleBarNode);
                    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
                    backButtonNode->GetOrCreateFocusHub()->SetIsDefaultFocus(true);
                    focusHub->RequestFocusWithDefaultFocusFirstly();
                }
            },
            TaskExecutor::TaskType::UI);
    }
    auto navigationLayoutProperty = AceType::DynamicCast<NavigationLayoutProperty>(hostNode->GetLayoutProperty());
    CHECK_NULL_RETURN(navigationLayoutProperty, false);

    UpdateTitleModeChangeEventHub(hostNode);
    AddDividerHotZoneRect(navigationLayoutAlgorithm);
    ifNeedInit_ = false;
    return false;
}

void NavigationPattern::UpdateContextRect(
    const RefPtr<NavDestinationGroupNode>& curDestination, const RefPtr<NavigationGroupNode>& hostNode)
{
    CHECK_NULL_VOID(curDestination);
    CHECK_NULL_VOID(hostNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(hostNode->GetPattern());
    CHECK_NULL_VOID(navigationPattern);

    if (navigationPattern->GetNavigationMode() == NavigationMode::STACK) {
        curDestination->GetRenderContext()->SetActualForegroundColor(DEFAULT_MASK_COLOR);
        navBarNode->GetEventHub<EventHub>()->SetEnabledInternal(false);
        return;
    }
    auto navigationLayoutProperty = hostNode->GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(navigationLayoutProperty);
    auto navBarProperty = navBarNode->GetLayoutProperty();
    navBarProperty->UpdateVisibility(navigationLayoutProperty->GetVisibilityValue(VisibleType::VISIBLE));
    curDestination->GetRenderContext()->UpdateTranslateInXY(OffsetF { 0.0f, 0.0f });
    curDestination->GetRenderContext()->SetActualForegroundColor(DEFAULT_MASK_COLOR);
    navBarNode->GetEventHub<EventHub>()->SetEnabledInternal(true);
    auto titleNode = AceType::DynamicCast<FrameNode>(navBarNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    titleNode->GetRenderContext()->UpdateTranslateInXY(OffsetF { 0.0f, 0.0f });
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

RefPtr<UINode> NavigationPattern::GenerateUINodeByIndex(int32_t index)
{
    return navigationStack_->CreateNodeByIndex(index);
}

void NavigationPattern::InitDividerMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    CHECK_NULL_VOID(inputHub);
    CHECK_NULL_VOID(!hoverEvent_);

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
    auto navBarLine = preNavBarWidth_ + (isNavBarStart ? xOffset : -xOffset);
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
    CHECK_NULL_VOID(!dragEvent_);
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragStart();
    };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragUpdate(static_cast<float>(info.GetOffsetX()));
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragEnd();
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
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

RefPtr<FrameNode> NavigationPattern::GetDividerNode() const
{
    RefPtr<FrameNode> dividerFrameNode;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto children = host->GetChildren();
    for (auto begin = children.begin(); begin != children.end(); begin++) {
        auto dividerNode = *begin;
        if (dividerNode->GetTag() == V2::DIVIDER_ETS_TAG) {
            dividerFrameNode = AceType::DynamicCast<FrameNode>(dividerNode);
            CHECK_NULL_RETURN(dividerFrameNode, nullptr);
            break;
        }
    }
    return dividerFrameNode;
}

void NavigationPattern::AddDividerHotZoneRect(const RefPtr<NavigationLayoutAlgorithm>& layoutAlgorithm)
{
    CHECK_NULL_VOID(layoutAlgorithm);
    if (realDividerWidth_ <= 0.0f) {
        return;
    }
    OffsetF hotZoneOffset;
    hotZoneOffset.SetX(-DEFAULT_DIVIDER_HOT_ZONE_HORIZONTAL_PADDING.ConvertToPx());
    hotZoneOffset.SetY(DEFAULT_DIVIDER_START_MARGIN.ConvertToPx());
    SizeF hotZoneSize;
    hotZoneSize.SetWidth(realDividerWidth_ + DIVIDER_HOT_ZONE_HORIZONTAL_PADDING_NUM *
                                                 DEFAULT_DIVIDER_HOT_ZONE_HORIZONTAL_PADDING.ConvertToPx());
    hotZoneSize.SetHeight(layoutAlgorithm->GetRealNavBarHeight());
    DimensionRect hotZoneRegion;
    if (navigationMode_ == NavigationMode::STACK) {
        hotZoneRegion.SetSize(DimensionSize(Dimension(0.0f), Dimension(0.0f)));
    } else {
        hotZoneRegion.SetSize(DimensionSize(Dimension(hotZoneSize.Width()), Dimension(hotZoneSize.Height())));
    }
    hotZoneRegion.SetOffset(DimensionOffset(Dimension(hotZoneOffset.GetX()), Dimension(hotZoneOffset.GetY())));

    std::vector<DimensionRect> mouseRegion;
    mouseRegion.emplace_back(hotZoneRegion);

    auto dividerFrameNode = GetDividerNode();
    CHECK_NULL_VOID(dividerFrameNode);
    auto dividerGestureHub = dividerFrameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(dividerGestureHub);
    dividerGestureHub->SetMouseResponseRegion(mouseRegion);

    auto dragRectOffset = layoutAlgorithm->GetNavBarOffset();
    dragRectOffset.SetX(-DEFAULT_DRAG_REGION.ConvertToPx());
    dragRect_.SetOffset(dragRectOffset);
    if (navigationMode_ == NavigationMode::STACK) {
        dragRect_.SetSize(SizeF(0.0f, 0.0f));
    } else {
        dragRect_.SetSize(SizeF(DEFAULT_DRAG_REGION.ConvertToPx() * DEFAULT_HALF + realDividerWidth_,
            layoutAlgorithm->GetRealNavBarHeight()));
    }

    std::vector<DimensionRect> responseRegion;
    DimensionOffset responseOffset(dragRectOffset);
    DimensionRect responseRect(Dimension(dragRect_.Width(), DimensionUnit::PX),
        Dimension(dragRect_.Height(), DimensionUnit::PX), responseOffset);
    responseRegion.emplace_back(responseRect);
    dividerGestureHub->MarkResponseRegion(true);
    dividerGestureHub->SetResponseRegion(responseRegion);
}

void NavigationPattern::OnWindowHide()
{
    auto curTopNavPath = navigationStack_->GetTopNavPath();
    CHECK_NULL_VOID(curTopNavPath.has_value());
    CHECK_NULL_VOID(curTopNavPath->second);
    auto curTopNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
        NavigationGroupNode::GetNavDestinationNode(curTopNavPath->second));
    CHECK_NULL_VOID(curTopNavDestination);
    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(curTopNavDestination->GetPattern());
    CHECK_NULL_VOID(navDestinationPattern);
    CHECK_NULL_VOID(navDestinationPattern->GetIsOnShow());
    auto eventHub = curTopNavDestination->GetEventHub<NavDestinationEventHub>();
    CHECK_NULL_VOID(eventHub);
    NotifyPageHide(curTopNavPath->first);
    eventHub->FireOnHiddenEvent(navDestinationPattern->GetName());
    navDestinationPattern->SetIsOnShow(false);
}

void NavigationPattern::OnWindowShow()
{
    auto curTopNavPath = navigationStack_->GetTopNavPath();
    CHECK_NULL_VOID(curTopNavPath.has_value());
    CHECK_NULL_VOID(curTopNavPath->second);
    auto curTopNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(
        NavigationGroupNode::GetNavDestinationNode(curTopNavPath->second));
    CHECK_NULL_VOID(curTopNavDestination);
    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(curTopNavDestination->GetPattern());
    CHECK_NULL_VOID(navDestinationPattern);
    CHECK_NULL_VOID(!(navDestinationPattern->GetIsOnShow()));
    auto eventHub = curTopNavDestination->GetEventHub<NavDestinationEventHub>();
    NotifyPageShow(curTopNavPath->first);
    auto param = Recorder::EventRecorder::Get().IsPageRecordEnable() ? navigationStack_->GetRouteParam() : "";
    eventHub->FireOnShownEvent(navDestinationPattern->GetName(), param);
    navDestinationPattern->SetIsOnShow(true);
}
} // namespace OHOS::Ace::NG

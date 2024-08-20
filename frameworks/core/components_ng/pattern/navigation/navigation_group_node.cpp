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

#include "core/components_ng/pattern/navigation/navigation_group_node.h"

#if !defined(PREVIEW) && !defined(ACE_UNITTEST) && defined(OHOS_PLATFORM)
#include "interfaces/inner_api/ui_session/ui_session_manager.h"
#endif
#include "base/log/ace_checker.h"
#include "base/log/ace_performance_check.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/perfmonitor/perf_constants.h"
#include "base/perfmonitor/perf_monitor.h"
#include "base/utils/utils.h"
#include "core/animation/page_transition_common.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components/theme/app_theme.h"

#if !defined(ACE_UNITTEST)
#include "core/components_ng/base/transparent_node_detector.h"
#endif

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_event_hub.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/navrouter/navrouter_event_hub.h"
#include "core/components_ng/pattern/navrouter/navrouter_group_node.h"
#include "core/components_ng/pattern/stack/stack_layout_property.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double HALF = 0.5;
constexpr double PARENT_PAGE_OFFSET = 0.2;
constexpr double PARENT_TITLE_OFFSET = 0.02;
constexpr float REMOVE_CLIP_SIZE = 10000.0f;
constexpr int32_t MASK_DURATION = 350;
constexpr int32_t OPACITY_TITLE_OUT_DELAY = 17;
constexpr int32_t OPACITY_TITLE_IN_DELAY = 33;
constexpr int32_t OPACITY_TITLE_DURATION = 150;
constexpr int32_t OPACITY_BACKBUTTON_IN_DELAY = 150;
constexpr int32_t OPACITY_BACKBUTTON_IN_DURATION = 200;
constexpr int32_t OPACITY_BACKBUTTON_OUT_DURATION = 67;
constexpr int32_t DEFAULT_ANIMATION_DURATION = 450;
constexpr int32_t DEFAULT_REPLACE_DURATION = 150;
const Color MASK_COLOR = Color::FromARGB(25, 0, 0, 0);
const RefPtr<InterpolatingSpring> springCurve = AceType::MakeRefPtr<InterpolatingSpring>(0.0f, 1.0f, 342.0f, 37.0f);
const RefPtr<CubicCurve> replaceCurve = AceType::MakeRefPtr<CubicCurve>(0.33, 0.0, 0.67, 1.0);
} // namespace
class InspectorFilter;

RefPtr<NavigationGroupNode> NavigationGroupNode::GetOrCreateGroupNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto frameNode = GetFrameNode(tag, nodeId);
    CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<NavigationGroupNode>(frameNode));
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    auto navigationGroupNode = AceType::MakeRefPtr<NavigationGroupNode>(tag, nodeId, pattern);
    navigationGroupNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(navigationGroupNode);
    return navigationGroupNode;
}

NavigationGroupNode::~NavigationGroupNode()
{
    auto navigationPattern = GetPattern<NavigationPattern>();
    const auto& navDestinationNodes = navigationPattern->GetAllNavDestinationNodes();
    for (auto iter : navDestinationNodes) {
        auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(GetNavDestinationNode(iter.second));
        if (navDestinationNode) {
            navDestinationNode->GetPattern<NavDestinationPattern>()->SetCustomNode(nullptr);
        }
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto stageManager = context->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    RefPtr<FrameNode> pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pagePattern = pageNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    CHECK_NULL_VOID(pagePattern->GetPageInfo());
    int32_t pageId = pagePattern->GetPageInfo()->GetPageId();
    context->RemoveNavigationNode(pageId, GetId());
    context->DeleteNavigationNode(curId_);
}

void NavigationGroupNode::AddChildToGroup(const RefPtr<UINode>& child, int32_t slot)
{
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_VOID(pattern);
    auto navBar = AceType::DynamicCast<NavBarNode>(GetNavBarNode());
    CHECK_NULL_VOID(navBar);
    auto contentNode = navBar->GetNavBarContentNode();
    if (!contentNode) {
        auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
        contentNode = FrameNode::GetOrCreateFrameNode(
            V2::NAVBAR_CONTENT_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        navBar->SetNavBarContentNode(contentNode);
        navBar->AddChild(contentNode);

        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            auto navBarContentNode = AceType::DynamicCast<FrameNode>(contentNode);
            SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
                .edges = SAFE_AREA_EDGE_ALL };
            navBarContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
        }
    }
    contentNode->AddChild(child);
}

void NavigationGroupNode::UpdateNavDestinationNodeWithoutMarkDirty(const RefPtr<UINode>& remainChild, bool modeChange)
{
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_VOID(pattern);
    const auto& navDestinationNodes = pattern->GetAllNavDestinationNodes();

    auto navigationContentNode = AceType::DynamicCast<FrameNode>(GetContentNode());
    CHECK_NULL_VOID(navigationContentNode);
    bool hasChanged = false;
    int32_t slot = 0;
    int32_t beforeLastStandardIndex = lastStandardIndex_;
    auto preLastStandardNode = navigationContentNode->GetChildAtIndex(beforeLastStandardIndex);
    UpdateLastStandardIndex();
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "last standard page index is %{public}d", lastStandardIndex_);
    if (!ReorderNavDestination(navDestinationNodes, navigationContentNode, slot, hasChanged)) {
        return;
    }

    for (uint32_t index = 0; index < navDestinationNodes.size(); index++) {
        const auto& childNode = navDestinationNodes[index];
        const auto& uiNode = childNode.second;
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(GetNavDestinationNode(uiNode));
        hasChanged = (UpdateNavDestinationVisibility(navDestination, remainChild, index,
            navDestinationNodes.size(), preLastStandardNode) || hasChanged);
    }

    RemoveRedundantNavDestination(
        navigationContentNode, remainChild, static_cast<int32_t>(slot), hasChanged, beforeLastStandardIndex);
    if (modeChange) {
        navigationContentNode->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    } else if (hasChanged) {
        navigationContentNode->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
}

bool NavigationGroupNode::ReorderNavDestination(
    const std::vector<std::pair<std::string, RefPtr<UINode>>>& navDestinationNodes,
    RefPtr<FrameNode>& navigationContentNode, int32_t& slot, bool& hasChanged)
{
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_RETURN(pattern, false);
    for (uint32_t i = 0; i != navDestinationNodes.size(); ++i) {
        const auto& childNode = navDestinationNodes[i];
        const auto& uiNode = childNode.second;
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(GetNavDestinationNode(uiNode));
        if (navDestination == nullptr) {
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "get destination node failed");
            return false;
        }
        auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
        CHECK_NULL_RETURN(navDestinationPattern, false);
        navDestinationPattern->SetName(childNode.first);
        navDestinationPattern->SetCustomNode(uiNode);
        navDestinationPattern->SetIndex(static_cast<int32_t>(i));
        SetBackButtonEvent(navDestination);
        navDestination->SetIndex(i);
        auto eventHub = navDestination->GetEventHub<NavDestinationEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        if (!eventHub->GetOnStateChange()) {
            auto onStateChangeMap = pattern->GetOnStateChangeMap();
            auto iter = onStateChangeMap.find(uiNode->GetId());
            if (iter != onStateChangeMap.end()) {
                eventHub->SetOnStateChange(iter->second);
                pattern->DeleteOnStateChangeItem(iter->first);
            }
        }
        int32_t childIndex = navigationContentNode->GetChildIndex(navDestination);
        if (childIndex < 0) {
            navigationContentNode->AddChild(navDestination, slot);
            hasChanged = true;
        } else if (childIndex != slot) {
            navDestination->MovePosition(slot);
            hasChanged = true;
        }
        slot++;
    }
    return true;
}

void NavigationGroupNode::RemoveRedundantNavDestination(RefPtr<FrameNode>& navigationContentNode,
    const RefPtr<UINode>& remainChild, int32_t slot, bool& hasChanged, int32_t beforeLastStandardIndex)
{
    auto pattern = GetPattern<NavigationPattern>();
    RefPtr<UINode> maxAnimatingDestination = nullptr;
    RefPtr<UINode> remainDestination = GetNavDestinationNode(remainChild);
    RefPtr<UINode> curTopDestination = navigationContentNode->GetChildAtIndex(slot - 1);
    // record remove destination size
    int32_t removeSize = 0;
    // record animating destination size
    int32_t animatingSize = 0;
    while (slot + removeSize + animatingSize < static_cast<int32_t>(navigationContentNode->GetChildren().size())) {
        // delete useless nodes that are not at the top
        int32_t candidateIndex = static_cast<int32_t>(navigationContentNode->GetChildren().size()) - 1 - animatingSize;
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(
            navigationContentNode->GetChildAtIndex(candidateIndex));
        if (!navDestination) {
            navigationContentNode->RemoveChildAtIndex(candidateIndex);
            hasChanged = true;
            continue;
        }
        auto eventHub = navDestination->GetEventHub<NavDestinationEventHub>();
        if (eventHub) {
            eventHub->FireChangeEvent(false);
        }
        if (navDestination == remainDestination) {
            if (pattern->IsCurTopNewInstance()) {
                // remain the last child for push animation, and this child
                // will be remove in push's animation finish callback
                navDestination->SetNeedRemoveInPush(true);
                navDestination->MovePosition(slot - 1);
            } else {
                // remain the last child for pop animation
                navDestination->MovePosition(slot);
            }
            ++slot;
            continue;
        }
        // The NavDestination in the EXIT animation needs to be cleaned in the animation onfinish callback.
        if (navDestination->IsOnAnimation()) {
            if (navDestination->GetTransitionType() == PageTransitionType::EXIT_POP) {
                ++animatingSize;
                continue;
            }
            if (navDestination->NeedRemoveInPush()) {
                if (maxAnimatingDestination == nullptr) {
                    maxAnimatingDestination = navDestination;
                }
                ++animatingSize;
                continue;
            }
        }
        // remove content child
        auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "remove child: %{public}s", navDestinationPattern->GetName().c_str());
        if (navDestination->GetIndex() >= beforeLastStandardIndex) {
            hideNodes_.emplace_back(std::make_pair(navDestination, true));
            navDestination->SetCanReused(false);
            removeSize++;
            auto index = slot + removeSize - 1;
            // move current destination position to navigation stack size + remove navDestination nodes
            if (index > 0) {
                navDestination->MovePosition(index);
            }
            continue;
        }
        DealRemoveDestination(navDestination);
        hasChanged = true;
    }
    if (pattern->IsCurTopNewInstance()) {
        ReorderAnimatingDestination(
            navigationContentNode, maxAnimatingDestination, remainDestination, curTopDestination);
    }
}

void NavigationGroupNode::ReorderAnimatingDestination(RefPtr<FrameNode>& navigationContentNode,
    RefPtr<UINode>& maxAnimatingDestination, RefPtr<UINode>& remainDestination, RefPtr<UINode>& curTopDestination)
{
    auto maxAnimatingIndex = navigationContentNode->GetChildIndex(maxAnimatingDestination);
    if (maxAnimatingIndex != -1 && remainDestination) {
        remainDestination->MovePosition(maxAnimatingIndex + 1);
    }
    auto remainIndex = navigationContentNode->GetChildIndex(remainDestination);
    if (remainIndex != -1 && curTopDestination) {
        curTopDestination->MovePosition(remainIndex + 1);
    }
}

void NavigationGroupNode::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    FrameNode::ToJsonValue(json, filter);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    navBarNode->ToJsonValue(json, filter);
}

RefPtr<UINode> NavigationGroupNode::GetNavDestinationNode(RefPtr<UINode> uiNode)
{
    if (!uiNode) {
        return nullptr;
    }
    // create NavDestinationNode from uiNode stored in NavigationStack
    while (uiNode) {
        if (uiNode->GetTag() == V2::NAVDESTINATION_VIEW_ETS_TAG) {
            // this is a navDestination node
            return uiNode;
        }
        if (AceType::DynamicCast<UINode>(uiNode)) {
            // this is an UINode, go deep further for navDestination node
            auto children = uiNode->GetChildren();
            uiNode = children.front();
            continue;
        }
    }
    CHECK_NULL_RETURN(uiNode, nullptr);
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "get navDestination node failed: id: %{public}d, %{public}s",
        uiNode->GetId(), uiNode->GetTag().c_str());
    return nullptr;
}

void NavigationGroupNode::SetBackButtonEvent(const RefPtr<NavDestinationGroupNode>& navDestination)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestination->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
    CHECK_NULL_VOID(backButtonNode);
    auto backButtonEventHub = backButtonNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(backButtonEventHub);
    auto onBackButtonEvent = [navDestinationWeak = WeakPtr<NavDestinationGroupNode>(navDestination),
                                 navigationWeak = WeakClaim(this)](GestureEvent& /*info*/) -> bool {
        auto navDestination = navDestinationWeak.Upgrade();
        TAG_LOGD(AceLogTag::ACE_NAVIGATION, "click navigation back button");
        CHECK_NULL_RETURN(navDestination, false);
        auto eventHub = navDestination->GetEventHub<NavDestinationEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        eventHub->SetState(NavDestinationState::ON_BACKPRESS);
        auto navdestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
        UIObserverHandler::GetInstance().NotifyNavigationStateChange(
            navdestinationPattern, NavDestinationState::ON_BACKPRESS);
        auto isOverride = eventHub->GetOnBackPressedEvent();
        auto result = false;
        if (isOverride) {
            result = eventHub->FireOnBackPressedEvent();
        }
        if (result) {
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation user onBackPress return true");
            return true;
        }
        auto navigation = navigationWeak.Upgrade();
        CHECK_NULL_RETURN(navigation, false);
        // if set hideNavBar and stack size is one, return false
        auto navigationLayoutProperty = AceType::DynamicCast<NavigationLayoutProperty>(navigation->GetLayoutProperty());
        auto pattern = AceType::DynamicCast<NavigationPattern>(navigation->GetPattern());
        auto stack = pattern->GetNavigationStack();
        CHECK_NULL_RETURN(stack, false);
        if (navigationLayoutProperty->GetHideNavBarValue(false) && stack->Size() <= 1) {
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "set hideNavBar and stack size is no more than one");
            return false;
        }
        auto isLastChild = stack->Size() == 1;
        if (isOverride) {
            result = navigation->HandleBack(navDestination, isLastChild, true);
        } else {
            result = navigation->HandleBack(navDestination, isLastChild, false);
        }
        // when js navigationStack is provided, modifyDone will be called by state manager.
        auto navigationPattern = navigation->GetPattern<NavigationPattern>();
        CHECK_NULL_RETURN(navigationPattern, false);
        if (!navigationPattern->GetNavigationStackProvided()) {
            navigation->MarkModifyDone();
            navigation->MarkDirtyNode();
        }

        return result;
    }; // backButton event

    navDestination->SetNavDestinationBackButtonEvent(onBackButtonEvent);
    backButtonEventHub->GetOrCreateGestureEventHub()->SetUserOnClick(onBackButtonEvent);
}

RefPtr<FrameNode> NavigationGroupNode::GetTopDestination()
{
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_RETURN(pattern, nullptr);
    auto navigationStack = pattern->GetNavigationStack();
    CHECK_NULL_RETURN(navigationStack, nullptr);
    auto topNavdestination = AceType::DynamicCast<FrameNode>(GetNavDestinationNode(navigationStack->Get()));
    return topNavdestination;
}

bool NavigationGroupNode::CheckCanHandleBack()
{
    auto navigation = AceType::WeakClaim(this).Upgrade();
    CHECK_NULL_RETURN(navigation, false);
    auto navigationPattern = GetPattern<NavigationPattern>();
    CHECK_NULL_RETURN(navigationPattern, false);

    auto navigationStack = navigationPattern->GetNavigationStack();
    CHECK_NULL_RETURN(navigationStack, false);
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(
        NavigationGroupNode::GetNavDestinationNode(navigationStack->Get()));
    if (!navDestination) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "can't find destination node to process back press");
        return false;
    }
    if (!navigationPattern->IsFinishInteractiveAnimation()) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "can't handle back press during interactive animation");
        return true;
    }
    auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(navDestination->GetPattern());
    if (navDestinationPattern->OverlayOnBackPressed()) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navDestination's ovelay consume backPressed event: %{public}s",
            navDestinationPattern->GetName().c_str());
        return true;
    }
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navDestination consume back button event: %{public}s",
        navDestinationPattern->GetName().c_str());
    GestureEvent gestureEvent;
    return navDestination->GetNavDestinationBackButtonEvent()(gestureEvent);
}

bool NavigationGroupNode::HandleBack(const RefPtr<FrameNode>& node, bool isLastChild, bool isOverride)
{
    auto navigationPattern = GetPattern<NavigationPattern>();
    if (!isOverride && !isLastChild) {
        navigationPattern->RemoveNavDestination();
        return true;
    }
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(node);
    CHECK_NULL_RETURN(navDestination, false);

    auto mode = navigationPattern->GetNavigationMode();
    auto layoutProperty = GetLayoutProperty<NavigationLayoutProperty>();
    if (isLastChild && (mode == NavigationMode::SPLIT ||
                           (mode == NavigationMode::STACK && layoutProperty->GetHideNavBar().value_or(false)))) {
        return false;
    }

    navigationPattern->RemoveNavDestination();
    return true;
}

void NavigationGroupNode::CreateAnimationWithPop(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
    const AnimationFinishCallback finishCallback, bool isNavBar)
{
    CHECK_NULL_VOID(preNode);
    auto preNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
    CHECK_NULL_VOID(preNavDestination);
    auto preTitleNode = AceType::DynamicCast<TitleBarNode>(preNavDestination->GetTitleBarNode());
    CHECK_NULL_VOID(preTitleNode);
    auto preBackIcon = AceType::DynamicCast<FrameNode>(preTitleNode->GetBackButton());
    CHECK_NULL_VOID(preBackIcon);
    RefPtr<TitleBarNode> curTitleBarNode;
    if (!GetCurTitleBarNode(curTitleBarNode, curNode, isNavBar)) {
        return;
    }
    auto preFrameSize = preNode->GetGeometryNode()->GetFrameSize();
    /* set initial status of animation */
    preNode->GetEventHub<EventHub>()->SetEnabledInternal(false);
    preNode->GetRenderContext()->ClipWithRRect(RectF(0.0f, 0.0f, preFrameSize.Width(), REMOVE_CLIP_SIZE),
        RadiusF(EdgeF(0.0f, 0.0f)));
    preNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    preTitleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });

    if (curNode) {
        auto curFrameSize = curNode->GetGeometryNode()->GetFrameSize();
        curNode->GetRenderContext()->ClipWithRRect(
            RectF(0.0f, 0.0f, REMOVE_CLIP_SIZE, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
        float flag = CheckLanguageDirection();
        curNode->GetRenderContext()->UpdateTranslateInXY({ -curFrameSize.Width() * PARENT_PAGE_OFFSET * flag, 0.0f });
        curTitleBarNode->GetRenderContext()->UpdateTranslateInXY(
            { curFrameSize.Width() * PARENT_TITLE_OFFSET * flag, 0.0f });
    }

    /* start transition animation */
    AnimationOption option = CreateAnimationOption(springCurve, FillMode::FORWARDS, DEFAULT_ANIMATION_DURATION,
        finishCallback);
    auto newPopAnimation = AnimationUtils::StartAnimation(option, [
        this, preNode, preTitleNode, preFrameSize, curNode, curTitleBarNode]() {
            PerfMonitor::GetPerfMonitor()->Start(PerfConstants::ABILITY_OR_PAGE_SWITCH, PerfActionType::LAST_UP, "");
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation pop animation start");
            /* preNode */
            float flag = CheckLanguageDirection();
            if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
                preNode->GetRenderContext()->ClipWithRRect(
                    RectF(0.0f, 0.0f, preFrameSize.Width() * HALF, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
            } else {
                preNode->GetRenderContext()->ClipWithRRect(
                    RectF(preFrameSize.Width() * HALF, 0.0f, preFrameSize.Width(), REMOVE_CLIP_SIZE),
                    RadiusF(EdgeF(0.0f, 0.0f)));
            }
            preNode->GetRenderContext()->UpdateTranslateInXY({ preFrameSize.Width() * HALF * flag, 0.0f });
            preTitleNode->GetRenderContext()->UpdateTranslateInXY({ preFrameSize.Width() * HALF * flag, 0.0f });

            /* curNode */
            if (curNode) {
                curNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
                curTitleBarNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
            }
    }, option.GetOnFinishEvent());
    if (newPopAnimation) {
        popAnimations_.emplace_back(newPopAnimation);
    }

    /* opacity for title and backIcon */
    auto titleOpacityAnimation = TitleOpacityAnimation(preTitleNode, true);
    if (titleOpacityAnimation) {
        popAnimations_.emplace_back(titleOpacityAnimation);
    }
    auto backButtonAnimation = BackButtonAnimation(preBackIcon, false);
    if (backButtonAnimation) {
        popAnimations_.emplace_back(backButtonAnimation);
    }

    /* mask animation */
    std::shared_ptr<AnimationUtils::Animation> maskAnimation;
    if (curNode) {
        AnimationOption maskOption = CreateAnimationOption(Curves::FRICTION, FillMode::FORWARDS, MASK_DURATION,
            nullptr);
        curNode->GetRenderContext()->SetActualForegroundColor(MASK_COLOR);
        maskAnimation = AnimationUtils::StartAnimation(
            maskOption, [curNode]() { curNode->GetRenderContext()->SetActualForegroundColor(Color::TRANSPARENT); });
    }
    if (maskAnimation) {
        popAnimations_.emplace_back(maskAnimation);
    }
}

void NavigationGroupNode::TransitionWithPop(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
    bool isNavBar)
{
    CHECK_NULL_VOID(preNode);

    auto preNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
    CHECK_NULL_VOID(preNavDestination);
    preNavDestination->SetTransitionType(PageTransitionType::EXIT_POP);

    /* obtain preTitle, preBackIcon and preFrameSize of preNavDestination */
    auto preTitleNode = AceType::DynamicCast<TitleBarNode>(preNavDestination->GetTitleBarNode());
    CHECK_NULL_VOID(preTitleNode);
    auto preBackIcon = AceType::DynamicCast<FrameNode>(preTitleNode->GetBackButton());
    CHECK_NULL_VOID(preBackIcon);

    RefPtr<TitleBarNode> curTitleBarNode;
    if (!GetCurTitleBarNode(curTitleBarNode, curNode, isNavBar)) {
        return;
    }

    /* create animation finish callback */
    CleanPopAnimations();
    AnimationFinishCallback callback = [weakPreNode = WeakPtr<NavDestinationGroupNode>(preNavDestination),
        weakPreTitle = WeakPtr<TitleBarNode>(preTitleNode),
        weakPreBackIcon = WeakPtr<FrameNode>(preBackIcon),
        weakNavigation = WeakClaim(this)] {
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation pop animation end");
            PerfMonitor::GetPerfMonitor()->End(PerfConstants::ABILITY_OR_PAGE_SWITCH, true);
            auto navigation = weakNavigation.Upgrade();
            auto backIcon = weakPreBackIcon.Upgrade();
            if (navigation) {
                navigation->isOnAnimation_ = false;
                auto id = navigation->GetTopDestination() ? navigation->GetTopDestination()->GetAccessibilityId() : -1;
                navigation->OnAccessibilityEvent(
                    AccessibilityEventType::PAGE_CHANGE, id, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_INVALID);
                navigation->CleanPopAnimations();
            }
            auto preNavDesNode = weakPreNode.Upgrade();
            CHECK_NULL_VOID(preNavDesNode);
            if (preNavDesNode->GetTransitionType() != PageTransitionType::EXIT_POP) {
                // has another transition, just return
                TAG_LOGW(AceLogTag::ACE_NAVIGATION, "preNavDesNode has another transition");
                return;
            }
            auto preNavDesPattern = preNavDesNode->GetPattern<NavDestinationPattern>();
            CHECK_NULL_VOID(preNavDesPattern);
            // NavRouter will restore the preNavDesNode and needs to set the initial state after the animation ends.
            auto shallowBuilder = preNavDesPattern->GetShallowBuilder();
            if (shallowBuilder) {
                shallowBuilder->MarkIsExecuteDeepRenderDone(false);
            }
            preNavDesNode->SetIsOnAnimation(false);
            preNavDesNode->GetEventHub<EventHub>()->SetEnabledInternal(true);
            preNavDesNode->GetRenderContext()->ClipWithRRect(RectF(0.0f, 0.0f, REMOVE_CLIP_SIZE, REMOVE_CLIP_SIZE),
                RadiusF(EdgeF(0.0f, 0.0f)));
            preNavDesNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
            auto preTitleNode = weakPreTitle.Upgrade();
            if (preTitleNode) {
                preTitleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
                preTitleNode->GetRenderContext()->SetOpacity(1.0);
            }
            if (!preNavDesNode->IsCacheNode() && preNavDesNode->GetContentNode()) {
                preNavDesNode->GetContentNode()->Clean();
            }
            if (backIcon)  {
                backIcon->GetRenderContext()->SetOpacity(1.0);
            }
            auto parent = preNavDesNode->GetParent();
            CHECK_NULL_VOID(parent);
            parent->RemoveChild(preNavDesNode);
            parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            auto context = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(context);
            context->MarkNeedFlushMouseEvent();
        };

    preNavDestination->SetIsOnAnimation(true);
    CreateAnimationWithPop(preNode, curNode, callback, isNavBar);

    // clear this flag for navBar layout only
    if (isNavBar) {
        SetNeedSetInvisible(false);
    }
    isOnAnimation_ = true;
#if !defined(PREVIEW) && !defined(ACE_UNITTEST) && defined(OHOS_PLATFORM)
    UiSessionManager::GetInstance().OnRouterChange(navigationPathInfo_, "navigationPopPage");
#endif
}

void NavigationGroupNode::CreateAnimationWithPush(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
    const AnimationFinishCallback finishCallback, bool isNavBar)
{
    CHECK_NULL_VOID(preNode);
    CHECK_NULL_VOID(curNode);
    RefPtr<TitleBarNode> preTitleNode;
    if (isNavBar) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(preNode);
        preTitleNode = navBarNode ? AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode()) : nullptr;
    } else {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
        preTitleNode = navDestination ? AceType::DynamicCast<TitleBarNode>(navDestination->GetTitleBarNode()) : nullptr;
    }
    CHECK_NULL_VOID(preTitleNode);
    auto curNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(curNode);
    CHECK_NULL_VOID(curNavDestination);
    auto curTitleNode = AceType::DynamicCast<TitleBarNode>(curNavDestination->GetTitleBarNode());
    CHECK_NULL_VOID(curTitleNode);
    auto preFrameSize = preNode->GetGeometryNode()->GetFrameSize();
    auto curFrameSize = curNode->GetGeometryNode()->GetFrameSize();
    auto mode = GetNavigationMode();
    /* set initial status of animation */
    /* preNode */
    preNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    preTitleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    /* curNode */
    float flag = CheckLanguageDirection();
    if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
        curNode->GetRenderContext()->ClipWithRRect(
            RectF(0.0f, 0.0f, curFrameSize.Width() * HALF, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
    } else {
        curNode->GetRenderContext()->ClipWithRRect(
            RectF(curFrameSize.Width() * HALF, 0.0f, curFrameSize.Width(), REMOVE_CLIP_SIZE),
            RadiusF(EdgeF(0.0f, 0.0f)));
    }
    curNode->GetRenderContext()->UpdateTranslateInXY({ curFrameSize.Width() * HALF * flag, 0.0f });
    curTitleNode->GetRenderContext()->UpdateTranslateInXY({ curFrameSize.Width() * HALF * flag, 0.0f });

    /* start transition animation */
    AnimationOption option = CreateAnimationOption(springCurve, FillMode::FORWARDS, DEFAULT_ANIMATION_DURATION,
        finishCallback);
    auto newPushAnimation = AnimationUtils::StartAnimation(option, [
        this, preNode, preTitleNode, curNode, curTitleNode, preFrameSize, curFrameSize, mode]() {
            PerfMonitor::GetPerfMonitor()->Start(PerfConstants::ABILITY_OR_PAGE_SWITCH, PerfActionType::LAST_UP, "");
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation push animation start");
            float flag = CheckLanguageDirection();
            // preNode
            preNode->GetRenderContext()->UpdateTranslateInXY(
                { -preFrameSize.Width() * PARENT_PAGE_OFFSET * flag, 0.0f });
            preTitleNode->GetRenderContext()->UpdateTranslateInXY(
                { preFrameSize.Width() * PARENT_TITLE_OFFSET * flag, 0.0f });
            // curNode
            curNode->GetRenderContext()->ClipWithRRect(
                RectF(0.0f, 0.0f, curFrameSize.Width(), REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
            curNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
            curTitleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    }, option.GetOnFinishEvent());
    if (newPushAnimation) {
        pushAnimations_.emplace_back(newPushAnimation);
    }
    auto maskAnimation = MaskAnimation(preNode->GetRenderContext());
    if (maskAnimation) {
        pushAnimations_.emplace_back(maskAnimation);
    }

    // title opacity
    auto titleOpacityAnimation = TitleOpacityAnimation(curTitleNode, false);
    if (titleOpacityAnimation) {
        pushAnimations_.emplace_back(titleOpacityAnimation);
    }
    // backIcon opacity
    auto backIcon = AceType::DynamicCast<FrameNode>(curTitleNode->GetBackButton());
    auto backButtonAnimation = BackButtonAnimation(backIcon, true);
    if (backButtonAnimation) {
        pushAnimations_.emplace_back(backButtonAnimation);
    }
}
void NavigationGroupNode::TransitionWithPush(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
    bool isNavBar)
{
    CHECK_NULL_VOID(preNode);
    CHECK_NULL_VOID(curNode);

    RefPtr<TitleBarNode> preTitleNode;
    if (isNavBar) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(preNode);
        navBarNode->SetTransitionType(PageTransitionType::EXIT_PUSH);
        preTitleNode = navBarNode ? AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode()) : nullptr;
    } else {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
        navDestination->SetTransitionType(PageTransitionType::EXIT_PUSH);
        preTitleNode = navDestination ? AceType::DynamicCast<TitleBarNode>(navDestination->GetTitleBarNode()) : nullptr;
    }
    CHECK_NULL_VOID(preTitleNode);

    auto curNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(curNode);
    CHECK_NULL_VOID(curNavDestination);
    curNavDestination->SetTransitionType(PageTransitionType::ENTER_PUSH);
    auto curTitleNode = AceType::DynamicCast<TitleBarNode>(curNavDestination->GetTitleBarNode());
    CHECK_NULL_VOID(curTitleNode);

    /* Create animation callback */
    CleanPushAnimations();
    AnimationFinishCallback callback = [weakPreNode = WeakPtr<FrameNode>(preNode),
        weakPreTitle = WeakPtr<FrameNode>(preTitleNode),
        weakNavigation = WeakClaim(this),
        weakCurNode = WeakPtr<FrameNode>(curNode),
        isNavBar] {
            PerfMonitor::GetPerfMonitor()->End(PerfConstants::ABILITY_OR_PAGE_SWITCH, true);
            TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation push animation end");
            auto navigation = weakNavigation.Upgrade();
            CHECK_NULL_VOID(navigation);
            auto preNode = weakPreNode.Upgrade();
            while (preNode) {
                auto preDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
                if (preDestination && preDestination->NeedRemoveInPush()) {
                    navigation->hideNodes_.emplace_back(std::make_pair(preDestination, true));
                    break;
                }
                auto preTitle = weakPreTitle.Upgrade();
                if (preTitle) {
                    preTitle->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
                }
                preNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
                preNode->GetRenderContext()->SetActualForegroundColor(Color::TRANSPARENT);
                bool needSetInvisible = false;
                if (isNavBar) {
                    needSetInvisible = AceType::DynamicCast<NavBarNode>(preNode)->GetTransitionType() ==
                        PageTransitionType::EXIT_PUSH;
                    // store this flag for navBar layout only
                    navigation->SetNeedSetInvisible(needSetInvisible);
                } else {
                    preDestination->SetIsOnAnimation(false);
                    needSetInvisible = preDestination->GetTransitionType() == PageTransitionType::EXIT_PUSH;
                }
                // for the case, the navBar form EXIT_PUSH to push during animation
                if (needSetInvisible) {
                    if (!isNavBar) {
                        preNode->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
                        preNode->SetJSViewActive(false);
                    } else {
                        // navigation mode could be transformed to split mode in the process of animation and
                        // navBar will be invisible only under the stack mode
                        if (navigation->GetNavigationMode() == NavigationMode::STACK) {
                            preNode->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
                            preNode->SetJSViewActive(false);
                            navigation->NotifyPageHide();
                        }
                    }
                }
                break;
            }
            navigation->RemoveDialogDestination();
            auto id = navigation->GetTopDestination() ? navigation->GetTopDestination()->GetAccessibilityId() : -1;
            navigation->OnAccessibilityEvent(
                AccessibilityEventType::PAGE_CHANGE, id, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_INVALID);
            auto curNode = weakCurNode.Upgrade();
            if (curNode) {
                auto curNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(curNode);
                CHECK_NULL_VOID(curNavDestination);
                if (curNavDestination->GetTransitionType() != PageTransitionType::ENTER_PUSH) {
                    TAG_LOGW(AceLogTag::ACE_NAVIGATION, "curNode has another transition");
                    return;
                }
                curNavDestination->SetIsOnAnimation(false);
                curNode->GetRenderContext()->ClipWithRRect(
                    RectF(0.0f, 0.0f, REMOVE_CLIP_SIZE, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
            }
            navigation->isOnAnimation_ = false;
            navigation->CleanPushAnimations();
        };

    if (!isNavBar) {
        auto preDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
        preDestination->SetIsOnAnimation(true);
        if (preDestination->NeedRemoveInPush()) {
            preNode->GetEventHub<EventHub>()->SetEnabledInternal(false);
        }
    }
    curNavDestination->SetIsOnAnimation(true);
    CreateAnimationWithPush(preNode, curNode, callback, isNavBar);

    isOnAnimation_ = true;
    if (AceChecker::IsPerformanceCheckEnabled()) {
        int64_t startTime = GetSysTimestamp();
        auto pipeline = AceType::DynamicCast<NG::PipelineContext>(PipelineContext::GetCurrentContext());
        // After completing layout tasks at all nodes on the page, perform performance testing and management
        pipeline->AddAfterLayoutTask([weakNav = WeakClaim(this), weakNode = WeakPtr<FrameNode>(curNode), startTime,
                                         path = curNavDestination->GetNavDestinationPathInfo()]() {
            auto navigation = weakNav.Upgrade();
            CHECK_NULL_VOID(navigation);
            auto curNode = weakNode.Upgrade();
            int64_t endTime = GetSysTimestamp();
            CHECK_NULL_VOID(curNode);
            PerformanceCheckNodeMap nodeMap;
            curNode->GetPerformanceCheckData(nodeMap);
            AceScopedPerformanceCheck::RecordPerformanceCheckData(nodeMap, endTime - startTime, path);
        });
    }
#if !defined(PREVIEW) && !defined(ACE_UNITTEST) && defined(OHOS_PLATFORM)
    UiSessionManager::GetInstance().OnRouterChange(navigationPathInfo_, "navigationPushPage");
#endif
#if !defined(ACE_UNITTEST)
    TransparentNodeDetector::GetInstance().PostCheckNodeTransparentTask(curNode);
#endif
}

std::shared_ptr<AnimationUtils::Animation> NavigationGroupNode::BackButtonAnimation(
    const RefPtr<FrameNode>& backButtonNode, bool isTransitionIn)
{
    CHECK_NULL_RETURN(backButtonNode, nullptr);
    AnimationOption transitionOption;
    transitionOption.SetCurve(Curves::SHARP);
    auto backButtonNodeContext = backButtonNode->GetRenderContext();
    CHECK_NULL_RETURN(backButtonNodeContext, nullptr);
    if (isTransitionIn) {
        transitionOption.SetDelay(OPACITY_BACKBUTTON_IN_DELAY);
        transitionOption.SetDuration(OPACITY_BACKBUTTON_IN_DURATION);
        backButtonNodeContext->SetOpacity(0.0f);
        return AnimationUtils::StartAnimation(transitionOption, [backButtonNodeContext]() {
            CHECK_NULL_VOID(backButtonNodeContext);
            backButtonNodeContext->SetOpacity(1.0f);
        });
    }
    transitionOption.SetDuration(OPACITY_BACKBUTTON_OUT_DURATION);
    // recover after transition animation.
    backButtonNodeContext->SetOpacity(1.0f);
    return AnimationUtils::StartAnimation(transitionOption, [backButtonNodeContext]() {
        CHECK_NULL_VOID(backButtonNodeContext);
        backButtonNodeContext->SetOpacity(0.0f);
    });
}

std::shared_ptr<AnimationUtils::Animation> NavigationGroupNode::MaskAnimation(
    const RefPtr<RenderContext>& transitionOutNodeContext)
{
    AnimationOption maskOption;
    maskOption.SetCurve(Curves::FRICTION);
    maskOption.SetDuration(MASK_DURATION);
    maskOption.SetFillMode(FillMode::FORWARDS);
    transitionOutNodeContext->SetActualForegroundColor(Color::TRANSPARENT);
    return AnimationUtils::StartAnimation(
        maskOption, [transitionOutNodeContext]() { transitionOutNodeContext->SetActualForegroundColor(MASK_COLOR); },
        maskOption.GetOnFinishEvent());
}

std::shared_ptr<AnimationUtils::Animation> NavigationGroupNode::TitleOpacityAnimation(
    const RefPtr<FrameNode>& node, bool isTransitionOut)
{
    auto titleNode = AceType::DynamicCast<TitleBarNode>(node);
    CHECK_NULL_RETURN(titleNode, nullptr);
    auto titleRenderContext = titleNode->GetRenderContext();
    CHECK_NULL_RETURN(titleRenderContext, nullptr);
    AnimationOption opacityOption;
    opacityOption.SetCurve(Curves::SHARP);
    opacityOption.SetDuration(OPACITY_TITLE_DURATION);
    if (isTransitionOut) {
        opacityOption.SetDelay(OPACITY_TITLE_OUT_DELAY);
        // recover after transition animation.
        titleRenderContext->SetOpacity(1.0f);
        return AnimationUtils::StartAnimation(opacityOption, [titleRenderContext]() {
            CHECK_NULL_VOID(titleRenderContext);
            titleRenderContext->SetOpacity(0.0f);
        });
    }
    opacityOption.SetDelay(OPACITY_TITLE_IN_DELAY);
    titleRenderContext->SetOpacity(0.0f);
    return AnimationUtils::StartAnimation(opacityOption, [titleRenderContext]() {
        CHECK_NULL_VOID(titleRenderContext);
        titleRenderContext->SetOpacity(1.0f);
    });
}

void NavigationGroupNode::TransitionWithReplace(
    const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode, bool isNavBar)
{
    CHECK_NULL_VOID(preNode);
    CHECK_NULL_VOID(curNode);
    AnimationOption option;
    option.SetCurve(replaceCurve);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetDuration(DEFAULT_REPLACE_DURATION);
    option.SetOnFinishEvent([weakPreNode = WeakPtr<FrameNode>(preNode), weakNavigation = WeakClaim(this),
                                isNavBar]() {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navigation replace animation end");
        PerfMonitor::GetPerfMonitor()->End(PerfConstants::ABILITY_OR_PAGE_SWITCH, true);
        auto preNode = weakPreNode.Upgrade();
        CHECK_NULL_VOID(preNode);
        auto navigationNode = weakNavigation.Upgrade();
        CHECK_NULL_VOID(navigationNode);
        navigationNode->isOnAnimation_ = false;
        auto id = navigationNode->GetTopDestination() ? navigationNode->GetTopDestination()->GetAccessibilityId() : -1;
        navigationNode->OnAccessibilityEvent(
            AccessibilityEventType::PAGE_CHANGE, id, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_INVALID);
        navigationNode->DealNavigationExit(preNode, isNavBar);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->MarkNeedFlushMouseEvent();
    });
    preNode->GetEventHub<EventHub>()->SetEnabledInternal(false);
    curNode->GetRenderContext()->UpdateOpacity(0.0f);
    if (!isNavBar) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
        if (navDestination) {
            navDestination->SetIsOnAnimation(true);
        }
    }
    AnimationUtils::Animate(
        option,
        [curNode]() {
            PerfMonitor::GetPerfMonitor()->Start(PerfConstants::ABILITY_OR_PAGE_SWITCH, PerfActionType::LAST_UP, "");
            curNode->GetRenderContext()->UpdateOpacity(1.0f);
        },
        option.GetOnFinishEvent());
    isOnAnimation_ = true;
}

void NavigationGroupNode::OnInspectorIdUpdate(const std::string& id)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddOrReplaceNavigationNode(id, WeakClaim(this));
    curId_ = id;
}

void NavigationGroupNode::DealNavigationExit(const RefPtr<FrameNode>& preNode, bool isNavBar, bool isAnimated)
{
    CHECK_NULL_VOID(preNode);
    if (preNode->GetEventHub<EventHub>()) {
        preNode->GetEventHub<EventHub>()->SetEnabledInternal(true);
    }
    if (isNavBar && isAnimated) {
        SetNeedSetInvisible(true);
        return;
    }
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(preNode);
    CHECK_NULL_VOID(navDestinationNode);
    navDestinationNode->SetIsOnAnimation(false);
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    auto shallowBuilder = navDestinationPattern->GetShallowBuilder();
    if (shallowBuilder) {
        shallowBuilder->MarkIsExecuteDeepRenderDone(false);
    }
    // remove old navdestination node
    if (navDestinationNode->GetContentNode()) {
        navDestinationNode->GetContentNode()->Clean();
    }
    auto parent = AceType::DynamicCast<FrameNode>(preNode->GetParent());
    CHECK_NULL_VOID(parent);
    parent->RemoveChild(preNode);
    RemoveDialogDestination();
    parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void NavigationGroupNode::NotifyPageHide()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto stageManager = context->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pageUrlChecker = container->GetPageUrlChecker();
    CHECK_NULL_VOID(pageUrlChecker);
    RefPtr<FrameNode> pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_VOID(pageInfo);
    pageUrlChecker->NotifyPageHide(pageInfo->GetPageUrl());
}

void NavigationGroupNode::UpdateLastStandardIndex()
{
    // remove the impact of last standard index
    lastStandardIndex_ = -1;
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_VOID(navigationPattern);
    auto navigationStack = navigationPattern->GetNavigationStack();
    CHECK_NULL_VOID(navigationStack);
    const auto& navDestinationNodes = navigationStack->GetAllNavDestinationNodes();
    if (navDestinationNodes.size() == 0) {
        return;
    }
    for (int32_t index = static_cast<int32_t>(navDestinationNodes.size()) - 1; index >= 0; index--) {
        const auto& curPath = navDestinationNodes[index];
        const auto& uiNode = curPath.second;
        if (!uiNode) {
            continue;
        }
        auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(GetNavDestinationNode(uiNode));
        if (navDestinationNode && navDestinationNode->GetNavDestinationMode() == NavDestinationMode::STANDARD) {
            lastStandardIndex_ = index;
            return;
        }
    }
}

bool NavigationGroupNode::UpdateNavDestinationVisibility(const RefPtr<NavDestinationGroupNode>& navDestination,
    const RefPtr<UINode>& remainChild, int32_t index, size_t destinationSize, const RefPtr<UINode>& preLastStandardNode)
{
    auto eventHub = navDestination->GetEventHub<NavDestinationEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (index == static_cast<int32_t>(destinationSize) - 1) {
        // process shallow builder
        navDestination->ProcessShallowBuilder();
        navDestination->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE, true);
        navDestination->SetJSViewActive(true);
        navDestination->GetEventHub<EventHub>()->SetEnabledInternal(true);
        // for the navDestination at the top, FireChangeEvent
        eventHub->FireChangeEvent(true);
        bool hasChanged = CheckNeedMeasure(navDestination->GetLayoutProperty()->GetPropertyChangeFlag());
        if (!hasChanged && NavigationLayoutAlgorithm::IsAutoHeight(GetLayoutProperty<NavigationLayoutProperty>())) {
            hasChanged = true;
        }
        return hasChanged;
    }
    if (index < lastStandardIndex_) {
        auto pattern = AceType::DynamicCast<NavDestinationPattern>(navDestination->GetPattern());
        if (navDestination->IsOnAnimation()) {
            return false;
        }
        if (!pattern || !pattern->GetIsOnShow()) {
            return false;
        }
        eventHub->FireChangeEvent(false);
        if (pattern->GetCustomNode() != remainChild) {
            if (navDestination->GetNavDestinationMode() == NavDestinationMode::DIALOG ||
                navDestination == AceType::DynamicCast<NavDestinationGroupNode>(preLastStandardNode)) {
                hideNodes_.insert(hideNodes_.begin(), std::pair(navDestination, false));
            } else {
                navDestination->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
                auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
                pattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_WILL_HIDE, true);
                pattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_HIDE, true);
            }
        }
        return false;
    }
    auto pattern = AceType::DynamicCast<NavDestinationPattern>(navDestination->GetPattern());
    if (navDestination->GetPattern<NavDestinationPattern>()->GetCustomNode() != remainChild &&
        !navDestination->IsOnAnimation()) {
        navDestination->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
        navDestination->SetJSViewActive(true);
    }
    return false;
}

AnimationOption NavigationGroupNode::CreateAnimationOption(const RefPtr<Curve>& curve, FillMode mode,
    int32_t duration, const AnimationFinishCallback& callback)
{
    AnimationOption option;
    option.SetCurve(curve);
    option.SetFillMode(mode);
    option.SetDuration(duration);
    if (callback != nullptr) {
        option.SetOnFinishEvent(callback);
    }
    return option;
}

NavigationMode NavigationGroupNode::GetNavigationMode()
{
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    CHECK_NULL_RETURN(navigationPattern, NavigationMode::AUTO);
    return navigationPattern->GetNavigationMode();
}

void NavigationGroupNode::OnDetachFromMainTree(bool recursive, PipelineContext* context)
{
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    if (pattern) {
        pattern->DetachNavigationStackFromParent();
        pattern->RemoveFromDumpManager();
    }

    GroupNode::OnDetachFromMainTree(recursive, context);
}

bool NavigationGroupNode::FindNavigationParent(const std::string& parentName)
{
    auto parent = GetParent();
    while (parent) {
        if (parent->GetTag() == parentName) {
            return true;
            break;
        }
        parent = parent->GetParent();
    }
    return parent != nullptr;
}

void NavigationGroupNode::OnAttachToMainTree(bool recursive)
{
    GroupNode::OnAttachToMainTree(recursive);

    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    if (pattern) {
        pattern->AttachNavigationStackToParent();
        pattern->AddToDumpManager();
    }
    auto parent = GetParent();
    while (parent) {
        if (parent->GetTag() == V2::JS_VIEW_ETS_TAG) {
            break;
        }
        parent = parent->GetParent();
    }
    if (parent) {
        pattern->SetParentCustomNode(parent);
    } else {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "parent custom node is nullptr");
    }
    bool findNavdestination = FindNavigationParent(V2::NAVDESTINATION_VIEW_ETS_TAG);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    RefPtr<FrameNode> pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pagePattern = pageNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    CHECK_NULL_VOID(pagePattern->GetPageInfo());
    int32_t pageId = pagePattern->GetPageInfo()->GetPageId();
    if (!findNavdestination) {
        pipelineContext->AddNavigationNode(pageId, WeakClaim(this));
    }
}

void NavigationGroupNode::FireHideNodeChange(NavDestinationLifecycle lifecycle)
{
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    for (auto iter = hideNodes_.begin(); iter != hideNodes_.end(); ++iter) {
        auto navDestination = iter->first;
        if (!navDestination) {
            continue;
        }
        if (lifecycle == NavDestinationLifecycle::ON_WILL_DISAPPEAR) {
            if (iter->second) {
                navigationPattern->NotifyDestinationLifecycle(
                    navDestination, NavDestinationLifecycle::ON_WILL_DISAPPEAR, true);
            }
            continue;
        }
        auto pattern = AceType::DynamicCast<NavDestinationPattern>(navDestination->GetPattern());
        if (!pattern->GetIsOnShow()) {
            continue;
        }
        if (lifecycle == NavDestinationLifecycle::ON_WILL_HIDE) {
            navigationPattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_WILL_HIDE, true);
            continue;
        }
        if (lifecycle == NavDestinationLifecycle::ON_HIDE) {
            navigationPattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_HIDE, true);
        }
    }
}

float NavigationGroupNode::CheckLanguageDirection()
{
    bool isRightToLeft = AceApplicationInfo::GetInstance().IsRightToLeft();
    if (isRightToLeft) {
        return -1.0f;
    } else {
        return 1.0f;
    }
}
bool NavigationGroupNode::GetCurTitleBarNode(
    RefPtr<TitleBarNode>& curTitleBarNode, const RefPtr<FrameNode>& curNode, bool isNavBar)
{
    if (curNode) {
        if (isNavBar) {
            auto navBarNode = AceType::DynamicCast<NavBarNode>(curNode);
            CHECK_NULL_RETURN(navBarNode, false);
            navBarNode->SetTransitionType(PageTransitionType::ENTER_POP);
            curTitleBarNode = navBarNode ? AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode()) : nullptr;
        } else {
            auto curNavDestination = AceType::DynamicCast<NavDestinationGroupNode>(curNode);
            CHECK_NULL_RETURN(curNavDestination, false);
            curNavDestination->SetTransitionType(PageTransitionType::ENTER_POP);
            curTitleBarNode =
                curNavDestination ? AceType::DynamicCast<TitleBarNode>(curNavDestination->GetTitleBarNode()) : nullptr;
        }
        CHECK_NULL_RETURN(curTitleBarNode, false);
    }
    return true;
}

void NavigationGroupNode::RemoveDialogDestination()
{
    for (auto iter = hideNodes_.begin(); iter != hideNodes_.end(); iter++) {
        auto navDestination = iter->first;
        if (!navDestination) {
            continue;
        }
        if (!iter->second) {
            // navDestination node don't need to remove, update visibility invisible
            navDestination->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
            navDestination->SetJSViewActive(false);
            continue;
        }
        auto parent = navDestination->GetParent();
        if (!parent) {
            continue;
        }
        auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(navDestination->GetPattern());
        if (!navDestinationPattern) {
            continue;
        }
        auto shallowBuilder = navDestinationPattern->GetShallowBuilder();
        if (shallowBuilder) {
            shallowBuilder->MarkIsExecuteDeepRenderDone(false);
        }
        if (navDestination->GetContentNode()) {
            navDestination->GetContentNode()->Clean();
        }
        parent->RemoveChild(navDestination);
    }
    hideNodes_.clear();
}

void NavigationGroupNode::DealRemoveDestination(const RefPtr<NavDestinationGroupNode>& navDestination)
{
    // remove content child
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    auto pattern = AceType::DynamicCast<NavigationPattern>(GetPattern());
    if (navDestinationPattern->GetIsOnShow()) {
        pattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_WILL_HIDE, true);
        pattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_HIDE, true);
        navDestinationPattern->SetIsOnShow(false);
    }
    pattern->NotifyDestinationLifecycle(navDestination, NavDestinationLifecycle::ON_WILL_DISAPPEAR, true);
    auto shallowBuilder = navDestinationPattern->GetShallowBuilder();
    if (shallowBuilder) {
        shallowBuilder->MarkIsExecuteDeepRenderDone(false);
    }
    if (navDestination->GetContentNode()) {
        navDestination->GetContentNode()->Clean();
    }
    contentNode_->RemoveChild(navDestination, true);
}
} // namespace OHOS::Ace::NG

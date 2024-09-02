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

#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"

#include "core/components_ng/pattern/navigation/navigation_title_util.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"

namespace OHOS::Ace::NG {
constexpr double HALF = 0.5;
constexpr float CONTENT_OFFSET_PERCENT = 0.2f;
constexpr float TITLE_OFFSET_PERCENT = 0.02f;
constexpr float REMOVE_CLIP_SIZE = 10000.0f;

NavDestinationGroupNode::~NavDestinationGroupNode()
{
    if (contentNode_) {
        contentNode_->Clean();
    }
}

RefPtr<NavDestinationGroupNode> NavDestinationGroupNode::GetOrCreateGroupNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto frameNode = GetFrameNode(tag, nodeId);
    CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<NavDestinationGroupNode>(frameNode));
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    auto navDestinationNode = AceType::MakeRefPtr<NavDestinationGroupNode>(tag, nodeId, pattern);
    navDestinationNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(navDestinationNode);
    return navDestinationNode;
}

void NavDestinationGroupNode::AddChildToGroup(const RefPtr<UINode>& child, int32_t slot)
{
    auto pattern = AceType::DynamicCast<Pattern>(GetPattern());
    CHECK_NULL_VOID(pattern);
    auto contentNode = GetContentNode();
    if (!contentNode) {
        auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVDESTINATION_CONTENT_ETS_TAG, nodeId);
        contentNode = FrameNode::GetOrCreateFrameNode(V2::NAVDESTINATION_CONTENT_ETS_TAG, nodeId,
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        SetContentNode(contentNode);
        AddChild(contentNode);

        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            auto navdestinationContentNode = AceType::DynamicCast<FrameNode>(contentNode);
            SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
                .edges = SAFE_AREA_EDGE_ALL };
            navdestinationContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
        }
    }
    contentNode->AddChild(child, slot);
}

void NavDestinationGroupNode::DeleteChildFromGroup(int32_t slot)
{
    auto navDestination = GetContentNode();
    CHECK_NULL_VOID(navDestination);
    navDestination->RemoveChildAtIndex(slot);
}

void NavDestinationGroupNode::OnAttachToMainTree(bool recursive)
{
    if (!UseOffscreenProcess()) {
        ProcessShallowBuilder();
    }
    FrameNode::OnAttachToMainTree(recursive);
}

void NavDestinationGroupNode::OnOffscreenProcess(bool recursive)
{
    ProcessShallowBuilder();
    FrameNode::OnOffscreenProcess(recursive);
}

void NavDestinationGroupNode::ProcessShallowBuilder()
{
    if (isCacheNode_) {
        return;
    }

    TAG_LOGD(AceLogTag::ACE_NAVIGATION, "render navDestination content");
    auto navDestinationPattern = GetPattern<NavDestinationPattern>();
    CHECK_NULL_VOID(navDestinationPattern);
    auto shallowBuilder = navDestinationPattern->GetShallowBuilder();
    if (shallowBuilder && !shallowBuilder->IsExecuteDeepRenderDone()) {
        auto eventHub = GetEventHub<NavDestinationEventHub>();
        if (eventHub) {
            auto ctx = navDestinationPattern->GetNavDestinationContext();
            eventHub->FireOnReady(ctx);
        }
        shallowBuilder->ExecuteDeepRender();
        GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
        AceType::DynamicCast<FrameNode>(contentNode_)
            ->GetLayoutProperty()
            ->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
}

RefPtr<CustomNodeBase> NavDestinationGroupNode::GetNavDestinationCustomNode()
{
    return customNode_.Upgrade();
}

void NavDestinationGroupNode::SetNavDestinationMode(NavDestinationMode mode)
{
    mode_ = mode;
    auto pattern = GetPattern<NavDestinationPattern>();
    CHECK_NULL_VOID(pattern);
    auto context = pattern->GetNavDestinationContext();
    CHECK_NULL_VOID(context);
    context->SetMode(mode);
}

void NavDestinationGroupNode::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    FrameNode::ToJsonValue(json, filter);
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }
    auto titleBarNode = DynamicCast<TitleBarNode>(titleBarNode_);
    if (titleBarNode) {
        std::string title = NavigationTitleUtil::GetTitleString(titleBarNode, GetPrevTitleIsCustomValue(false));
        std::string subtitle = NavigationTitleUtil::GetSubtitleString(titleBarNode);
        json->PutExtAttr("title", title.c_str(), filter);
        json->PutExtAttr("subtitle", subtitle.c_str(), filter);
    }
    json->PutExtAttr("mode", mode_ == NavDestinationMode::DIALOG
        ? "NavDestinationMode::DIALOG"
        : "NavDestinationMode::STANDARD", filter);
}

void NavDestinationGroupNode::InitSystemTransitionPush(bool transitionIn)
{
    auto titleBarNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    float isRTL = GetLanguageDirection();
    if (transitionIn) {
        SetIsOnAnimation(true);
        SetTransitionType(PageTransitionType::ENTER_PUSH);
        auto frameSize = GetGeometryNode()->GetFrameSize();
        if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
            GetRenderContext()->ClipWithRRect(
                RectF(0.0f, 0.0f, frameSize.Width() * HALF, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
        } else {
            GetRenderContext()->ClipWithRRect(
                RectF(frameSize.Width() * HALF, 0.0f, frameSize.Width(), REMOVE_CLIP_SIZE),
                RadiusF(EdgeF(0.0f, 0.0f)));
        }
        GetRenderContext()->UpdateTranslateInXY({ frameSize.Width() * HALF * isRTL, 0.0f });
        CHECK_NULL_VOID(titleBarNode);
        titleBarNode->GetRenderContext()->UpdateTranslateInXY({ frameSize.Width() * HALF * isRTL, 0.0f });
        return;
    }
    SetTransitionType(PageTransitionType::EXIT_PUSH);
    SetIsOnAnimation(true);
    GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    GetEventHub<EventHub>()->SetEnabledInternal(false);
    CHECK_NULL_VOID(titleBarNode);
    titleBarNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    if (NeedRemoveInPush()) {
        GetEventHub<EventHub>()->SetEnabledInternal(false);
    }
}

void NavDestinationGroupNode::StartSystemTransitionPush(bool transitionIn)
{
    auto titleBarNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    auto frameSize = GetGeometryNode()->GetFrameSize();
    float isRTL = GetLanguageDirection();
    if (transitionIn) {
        GetRenderContext()->ClipWithRRect(
            RectF(0.0f, 0.0f, frameSize.Width(), REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
        GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
        CHECK_NULL_VOID(titleBarNode);
        titleBarNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
        return;
    }
    GetRenderContext()->UpdateTranslateInXY(
        { -frameSize.Width() * CONTENT_OFFSET_PERCENT * isRTL, 0.0f });
    CHECK_NULL_VOID(titleBarNode);
    titleBarNode->GetRenderContext()->UpdateTranslateInXY(
        { frameSize.Width() * TITLE_OFFSET_PERCENT  * isRTL, 0.0f });
}

void NavDestinationGroupNode::SystemTransitionPushCallback(bool transitionIn)
{
    if (transitionIn) {
        if (GetTransitionType() != PageTransitionType::ENTER_PUSH) {
            TAG_LOGW(AceLogTag::ACE_NAVIGATION, "curNode has another transition");
            return;
        }
        SetIsOnAnimation(false);
        GetRenderContext()->RemoveClipWithRRect();
        return;
    }
    GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    GetRenderContext()->SetActualForegroundColor(Color::TRANSPARENT);
    SetIsOnAnimation(false);
    if (GetTransitionType() == PageTransitionType::EXIT_PUSH) {
        GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    }
    auto titleNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    titleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
}

void NavDestinationGroupNode::InitSystemTransitionPop(bool isTransitionIn)
{
    auto frameSize = GetGeometryNode()->GetFrameSize();
    auto titleBarNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    float isRTL = GetLanguageDirection();
    if (isTransitionIn) {
        SetTransitionType(PageTransitionType::ENTER_POP);
        GetRenderContext()->RemoveClipWithRRect();
        GetRenderContext()->UpdateTranslateInXY({ -frameSize.Width() * CONTENT_OFFSET_PERCENT * isRTL, 0.0f });
        CHECK_NULL_VOID(titleBarNode);
        titleBarNode->GetRenderContext()->UpdateTranslateInXY(
            { frameSize.Width() * TITLE_OFFSET_PERCENT * isRTL, 0.0f });
        return;
    }
    SetIsOnAnimation(true);
    SetTransitionType(PageTransitionType::EXIT_POP);
    GetEventHub<EventHub>()->SetEnabledInternal(false);
    GetRenderContext()->ClipWithRRect(RectF(0.0f, 0.0f, frameSize.Width(), REMOVE_CLIP_SIZE),
        RadiusF(EdgeF(0.0f, 0.0f)));
    GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    CHECK_NULL_VOID(titleBarNode);
    titleBarNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
}

void NavDestinationGroupNode::StartSystemTransitionPop(bool transitionIn)
{
    auto titleBarNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    if (transitionIn) {
        GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
        CHECK_NULL_VOID(titleBarNode);
        titleBarNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
        return;
    }
    auto frameSize = GetGeometryNode()->GetFrameSize();
    if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
        GetRenderContext()->ClipWithRRect(
            RectF(0.0f, 0.0f, frameSize.Width() * HALF, REMOVE_CLIP_SIZE), RadiusF(EdgeF(0.0f, 0.0f)));
    } else {
        GetRenderContext()->ClipWithRRect(
            RectF(frameSize.Width() * HALF, 0.0f, frameSize.Width(), REMOVE_CLIP_SIZE),
            RadiusF(EdgeF(0.0f, 0.0f)));
    }
    float isRTL = GetLanguageDirection();
    GetRenderContext()->UpdateTranslateInXY({ frameSize.Width() * HALF * isRTL, 0.0f });
    CHECK_NULL_VOID(titleBarNode);
    titleBarNode->GetRenderContext()->UpdateTranslateInXY({ frameSize.Width() * HALF * isRTL, 0.0f });
}

bool NavDestinationGroupNode::SystemTransitionPopCallback(bool transitionIn)
{
    if (GetTransitionType() != PageTransitionType::EXIT_POP || transitionIn) {
        // has another transition, just return
        TAG_LOGW(AceLogTag::ACE_NAVIGATION, "preNavDesNode has another transition");
        return false;
    }
    auto preNavDesPattern = GetPattern<NavDestinationPattern>();
    CHECK_NULL_RETURN(preNavDesPattern, false);

    // NavRouter will restore the preNavDesNode and needs to set the initial state after the animation ends.
    auto shallowBuilder = preNavDesPattern->GetShallowBuilder();
    if (shallowBuilder) {
        shallowBuilder->MarkIsExecuteDeepRenderDone(false);
    }
    if (!IsCacheNode() && GetContentNode()) {
        GetContentNode()->Clean();
    }
    SetIsOnAnimation(false);
    GetEventHub<EventHub>()->SetEnabledInternal(true);
    GetRenderContext()->RemoveClipWithRRect();
    GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
    auto preTitleNode = AceType::DynamicCast<FrameNode>(GetTitleBarNode());
    if (preTitleNode) {
        preTitleNode->GetRenderContext()->UpdateTranslateInXY({ 0.0f, 0.0f });
        preTitleNode->GetRenderContext()->SetOpacity(1.0);
        auto titleBarNode = AceType::DynamicCast<TitleBarNode>(preTitleNode);
        CHECK_NULL_RETURN(titleBarNode, true);
        auto preBackIcon = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
        if (preBackIcon)  {
            preBackIcon->GetRenderContext()->SetOpacity(1.0);
        }
    }
    return true;
}

void NavDestinationGroupNode::InitDialogTransition(bool isZeroY)
{
    auto contentNode = AceType::DynamicCast<FrameNode>(GetContentNode());
    CHECK_NULL_VOID(contentNode);
    auto context = contentNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (isZeroY) {
        context->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });
        return;
    }
    context->UpdateTransformTranslate({ 0.0f,
        contentNode->GetGeometryNode()->GetFrameSize().Height(), 0.0f });
}
} // namespace OHOS::Ace::NG

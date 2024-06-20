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

#include "core/common/container.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_context.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {

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
    auto pattern = GetPattern<NavDestinationPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    auto navDestinationNode = pattern->GetCustomNode();
    CHECK_NULL_RETURN(navDestinationNode, nullptr);

    auto child = navDestinationNode->GetFirstChild();
    while (child) {
        if (AceType::InstanceOf<NavDestinationGroupNode>(child)) {
            break;
        }

        if (AceType::InstanceOf<CustomNodeBase>(child)) {
            auto customNode = DynamicCast<CustomNodeBase>(child);
            return customNode;
        }
        child = child->GetFirstChild();
    }
    return nullptr;
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
} // namespace OHOS::Ace::NG

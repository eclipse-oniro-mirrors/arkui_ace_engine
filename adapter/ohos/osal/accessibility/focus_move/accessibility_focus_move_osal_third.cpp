/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal_third.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation_utils.h"

#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "adapter/ohos/osal/accessibility/third_accessibility_traversal.h"
#include "frameworks/core/accessibility/accessibility_provider.h"

using namespace OHOS::Accessibility;
using namespace OHOS::AccessibilityConfig;

namespace OHOS::Ace::Framework {
namespace {
// Native action descriptions are borrowed pointers. A request snapshot must own
// them before another provider callback can replace their backing storage.
struct ThirdNodeSnapshot : ArkUI_AccessibilityElementInfo {
    explicit ThirdNodeSnapshot(const ArkUI_AccessibilityElementInfo& source)
        : ArkUI_AccessibilityElementInfo(source)
    {
        const auto& actions = source.GetOperationActions();
        descriptions.reserve(actions.size());
        for (const auto& action : actions) {
            descriptions.emplace_back(action.description ? action.description : "");
        }
        ClearOperationActions();
        for (size_t i = 0; i < actions.size(); ++i) {
            auto action = actions[i];
            action.description = descriptions[i].c_str();
            AddOperationAction(action);
        }
    }
    std::vector<std::string> descriptions;
};

struct ThirdTreeBuildContext {
    const WeakPtr<AccessibilityProvider>& provider;
    ThirdRulesCheckNode::ParentMap& parents;
    ThirdRulesCheckNode::NodeMap& nodes;
    ThirdAccessibilityTraversal& traversal;
    std::vector<std::pair<std::shared_ptr<ArkUI_AccessibilityElementInfo>, size_t>> pending;
};

void CollectThirdChildren(const std::shared_ptr<ArkUI_AccessibilityElementInfo>& parent,
    size_t depth, ThirdTreeBuildContext& context)
{
    for (auto childId : parent->GetChildNodeIds()) {
        if (context.traversal.Exhausted()) {
            break;
        }
        if (!context.traversal.Visit(childId, depth + 1)) {
            continue;
        }
        auto child = std::make_shared<ArkUI_AccessibilityElementInfo>();
        if (!JsThirdProviderInteractionOperation::FindNativeInfoById(
            context.provider, childId, child, &context.traversal)) {
            continue;
        }
        child = std::make_shared<ThirdNodeSnapshot>(*child);
        auto reportedParent = child->GetParentId();
        // Missing parent IDs are a supported provider convention, not a warning.
        if (reportedParent >= 0 && reportedParent != parent->GetElementId()) {
            context.traversal.Record(ThirdAccessibilityTraversal::WRONG_PARENT, childId);
        }
        context.parents.emplace(childId, parent->GetElementId());
        context.nodes.emplace(childId, child);
        context.pending.emplace_back(child, depth + 1);
    }
}

void SanitizeThirdNode(const std::shared_ptr<ArkUI_AccessibilityElementInfo>& node,
    const ThirdRulesCheckNode::ParentMap& parents)
{
    const auto id = node->GetElementId();
    const auto children = node->GetChildNodeIds();
    node->ClearChildNodeIds();
    std::unordered_set<int64_t> emitted;
    for (auto childId : children) {
        auto parent = parents.find(childId);
        if (parent != parents.end() && parent->second == id && emitted.insert(childId).second) {
            node->AddChildNodeId(childId);
        }
    }
    node->SetParentId(parents.at(id));
}
} // namespace

void FocusStrategyOsalThird::InvailidElementInfo(Accessibility::AccessibilityElementInfo& info)
{
    info.SetValidElement(false);
}

bool FocusStrategyOsalThird::UpdateOriginNodeInfo(int64_t elementId)
{
    nodeInfo_.reset();
    if (!BuildParentMap()) {
        return false;
    }
    auto origin = nodeMap_->find(elementId == -1 ? rootInfo_->GetElementId() : elementId);
    if (origin == nodeMap_->end()) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "Third accessibility focus: unreachable origin %{public}lld", static_cast<long long>(elementId));
        return false;
    }
    nodeInfo_ = origin->second;
    return true;
}

std::shared_ptr<FocusRulesCheckNode> FocusStrategyOsalThird::GetCurrentCheckNode()
{
    CHECK_NULL_RETURN(nodeInfo_, nullptr);
    return std::make_shared<ThirdRulesCheckNode>(
        nodeInfo_, nodeInfo_->GetElementId(), accessibilityProvider_, parentMap_, nodeMap_);
}

void FocusStrategyOsalThird::UpdateNextFocus(std::shared_ptr<FocusRulesCheckNode>& checkNode)
{
}

std::shared_ptr<FocusRulesCheckNode> FocusStrategyOsalThird::ChangeToRoot()
{
    CHECK_NULL_RETURN(rootInfo_, nullptr);
    return std::make_shared<ThirdRulesCheckNode>(
        rootInfo_, rootInfo_->GetElementId(), accessibilityProvider_, parentMap_, nodeMap_);
}

bool FocusStrategyOsalThird::ChangeToEmbed(
    const std::shared_ptr<FocusRulesCheckNode>& resultNode, Accessibility::AccessibilityElementInfo& info)
{
    return false;
}

bool FocusStrategyOsalThird::UpdateElementInfo(
    const std::shared_ptr<FocusRulesCheckNode>& resultNode, Accessibility::AccessibilityElementInfo& info)
{
    CHECK_NULL_RETURN(resultNode, false);
    auto checkTarget = std::static_pointer_cast<ThirdRulesCheckNode>(resultNode);
    CHECK_NULL_RETURN(checkTarget, false);
    auto nativeInfo = checkTarget->GetNodeInfo();
    CHECK_NULL_RETURN(nativeInfo, false);
    AccessibilityThirdProviderUtils::TransformAccessibilityElementInfo(*nativeInfo, info);
    if (parentMap_) {
        auto parent = parentMap_->find(nativeInfo->GetElementId());
        if (parent != parentMap_->end()) {
            info.SetParent(parent->second);
        }
    }
    JsThirdProviderInteractionOperation::FillNodeConfig(config_, info);
    info.SetValidElement(true);
    return true;
}

bool FocusStrategyOsalThird::CanSendHoverWithTargetIdByReadableRules(
    int64_t currentId, int64_t& targetId)
{
    targetId = currentId;
    UpdateOriginNodeInfo(currentId);
    auto checkNode = GetCurrentCheckNode();
    CHECK_NULL_RETURN(checkNode, false);
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    auto changeResult = NeedChangeToReadableNodeThroughAncestor(checkNode, targetNode, FocusRuleType::DEFAULT);
    CHECK_NE_RETURN(changeResult, true, true); // changeResult false means not change, send targetId with currentId
    CHECK_NULL_RETURN(targetNode, false); // changeResult true but no targetNode means not find any readable
    targetId = targetNode->GetAccessibilityId();
    HILOG_INFO_FOCUS("third CanSendHoverWithTargetId %{public}" PRId64
        ", currentId %{public}" PRId64, targetId, currentId);
    return true;
}
bool FocusStrategyOsalThird::BuildParentMap()
{
    parentMap_ = std::make_shared<ThirdRulesCheckNode::ParentMap>();
    nodeMap_ = std::make_shared<ThirdRulesCheckNode::NodeMap>();
    rootInfo_.reset();
    ThirdAccessibilityTraversal traversal("focus");
    auto root = std::make_shared<ArkUI_AccessibilityElementInfo>();
    if (!JsThirdProviderInteractionOperation::FindNativeInfoById(accessibilityProvider_, -1, root, &traversal) ||
        !traversal.Visit(root->GetElementId(), 0)) {
        return false;
    }
    root = std::make_shared<ThirdNodeSnapshot>(*root);
    parentMap_->emplace(root->GetElementId(), AccessibilityElementInfo::ROOT_PARENT_ID);
    nodeMap_->emplace(root->GetElementId(), root);
    ThirdTreeBuildContext context { accessibilityProvider_, *parentMap_, *nodeMap_, traversal, { { root, 0 } } };
    while (!context.pending.empty() && !traversal.Exhausted()) {
        auto [parent, depth] = context.pending.back();
        context.pending.pop_back();
        CollectThirdChildren(parent, depth, context);
    }
    if (traversal.Exhausted()) {
        parentMap_->clear();
        nodeMap_->clear();
        return false; // Do not report a match from an incomplete tree.
    }
    // Publish only the accepted edges, including in element info returned to the service.
    for (const auto& [id, node] : *nodeMap_) {
        SanitizeThirdNode(node, *parentMap_);
    }
    rootInfo_ = root;
    return true;
}

} // OHOS::Ace::Framework

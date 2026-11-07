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

#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation.h"
#include "frameworks/core/accessibility/accessibility_manager.h"
#include "frameworks/core/accessibility/accessibility_provider.h"
#include "frameworks/core/components_ng/property/accessibility_property.h"

namespace OHOS::Ace::Framework {

ThirdRulesCheckNode::ThirdRulesCheckNode(std::shared_ptr<ArkUI_AccessibilityElementInfo> nodeInfo,
    int64_t accessibilityId,
    const WeakPtr<AccessibilityProvider>& accessibilityProvider,
    const std::shared_ptr<ParentMap>& parentMap, const std::shared_ptr<NodeMap>& nodeMap)
    : FocusRulesCheckNode(accessibilityId), nodeInfo_(std::move(nodeInfo)),
    accessibilityProvider_(accessibilityProvider), parentMap_(parentMap), nodeMap_(nodeMap)
{
}

RefPtr<NG::CustomAccessibilityProperty> ThirdRulesCheckNode::GetCustomProperty()
{
    auto provider = accessibilityProvider_.Upgrade();
    CHECK_NULL_RETURN(provider, nullptr);
    auto thirdManager = provider->GetThirdAccessibilityManager().lock();
    CHECK_NULL_RETURN(thirdManager, nullptr);
    return thirdManager->GetThirdCustomProperty(GetAccessibilityId());
}

bool ThirdRulesCheckNode::IsChildTreeContainer()
{
    // Mirror the main-tree host flow (FrameNodeRulesCheckNode::IsChildTreeContainer): a node with a child tree bound.
    auto provider = accessibilityProvider_.Upgrade();
    CHECK_NULL_RETURN(provider, false);
    auto thirdManager = provider->GetThirdAccessibilityManager().lock();
    CHECK_NULL_RETURN(thirdManager, false);
    return thirdManager->HasChildTree(GetAccessibilityId());
}

bool ThirdRulesCheckNode::GetPropText(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetContents();
    return true;
}

bool ThirdRulesCheckNode::GetPropHintText(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetHintText();
    return true;
}

bool ThirdRulesCheckNode::GetPropDesc(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetAccessibilityDescription();
    return true;
}

bool ThirdRulesCheckNode::GetPropAccessibilityText(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto customProperty = GetCustomProperty();
    if (customProperty && !customProperty->GetAccessibilityText().empty()) {
        value.valueType = Accessibility::ValueType::STRING;
        value.valueStr = customProperty->GetAccessibilityText();
        return true;
    }
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetAccessibilityText();
    return true;
}

bool ThirdRulesCheckNode::GetPropType(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto customProperty = GetCustomProperty();
    if (customProperty && !customProperty->GetRole().empty()) {
        value.valueType = Accessibility::ValueType::STRING;
        value.valueStr = customProperty->GetRole();
        return true;
    }
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetComponentType();
    return true;
}

bool ThirdRulesCheckNode::GetPropAccessibilityLevel(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto customProperty = GetCustomProperty();
    if (customProperty && !customProperty->GetAccessibilityLevel().empty()) {
        value.valueType = Accessibility::ValueType::STRING;
        value.valueStr = customProperty->GetAccessibilityLevel();
        return true;
    }
    value.valueType = Accessibility::ValueType::STRING;
    value.valueStr = nodeInfo_->GetAccessibilityLevel();
    return true;
}

bool ThirdRulesCheckNode::GetPropAccessibilityGroup(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto customProperty = GetCustomProperty();
    if (customProperty) {
        value.valueType = Accessibility::ValueType::BOOL;
        value.valueBool = customProperty->GetAccessibilityGroup();
        return true;
    }
    value.valueType = Accessibility::ValueType::BOOL;
    value.valueBool = nodeInfo_->GetAccessibilityGroup();
    return true;
}

bool ThirdRulesCheckNode::GetPropIsEnable(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto customProperty = GetCustomProperty();
    if (customProperty) {
        value.valueType = Accessibility::ValueType::BOOL;
        value.valueBool = customProperty->GetEnabled();
        return true;
    }
    value.valueType = Accessibility::ValueType::BOOL;
    value.valueBool = nodeInfo_->IsEnabled();
    return true;
}

bool ThirdRulesCheckNode::GetPropChildrenCount(Accessibility::PropValue& value)
{
    CHECK_NULL_RETURN(nodeInfo_, false);
    value.valueType = Accessibility::ValueType::NUMBER;
    auto children = nodeInfo_->GetChildNodeIds();
    value.valueNum = static_cast<int32_t>(children.size());
    return true;
}

bool ThirdRulesCheckNode::GetPropActionNames(Accessibility::PropValue& value)
{
    value.valueType = Accessibility::ValueType::ARRAY;
    CHECK_NULL_RETURN(nodeInfo_, false);
    auto supportAceActions = nodeInfo_->GetOperationActions();
    for (auto& action: supportAceActions) {
        auto findResult = AccessibilityFocusStrategy::actionToFocusActionName.find(
            static_cast<OHOS::Accessibility::ActionType>(action.actionType));
        if (findResult != AccessibilityFocusStrategy::actionToFocusActionName.end()) {
            value.valueArray.insert(findResult->second);
        }
    }
    return true;
}

template<typename T>
std::vector<std::shared_ptr<T>> ThirdRulesCheckNode::GetChildrenTemplate()
{
    std::vector<std::shared_ptr<T>> checkNodeChildren;
    CHECK_NULL_RETURN(nodeInfo_, checkNodeChildren);
    const auto& childrenIds = nodeInfo_->GetChildNodeIds();
    std::unordered_set<int64_t> visited;
    for (auto& childId : childrenIds) {
        if (!visited.insert(childId).second) {
            continue;
        }
        if (parentMap_) {
            auto parent = parentMap_->find(childId);
            if (parent == parentMap_->end() || parent->second != GetAccessibilityId()) {
                continue;
            }
        }
        if (childId == -1) {
            continue;
        }
        if (childId == Accessibility::AccessibilityElementInfo::ROOT_PARENT_ID) {
            continue;
        }
        if (nodeMap_) {
            auto child = nodeMap_->find(childId);
            if (child != nodeMap_->end()) {
                checkNodeChildren.push_back(std::make_shared<ThirdRulesCheckNode>(
                    child->second, childId, accessibilityProvider_, parentMap_, nodeMap_));
            }
            continue;
        }
        auto child = std::make_shared<ArkUI_AccessibilityElementInfo>();
        if (JsThirdProviderInteractionOperation::FindNativeInfoById(accessibilityProvider_, childId, child) &&
            child->GetElementId() == childId) {
            auto childCheckNode = std::make_shared<ThirdRulesCheckNode>(
                child, child->GetElementId(), accessibilityProvider_, parentMap_);
            checkNodeChildren.push_back(childCheckNode);
        }
    }
    return checkNodeChildren;
}

std::vector<std::shared_ptr<FocusRulesCheckNode>> ThirdRulesCheckNode::GetAceChildren()
{
    return GetChildrenTemplate<FocusRulesCheckNode>();
}

std::vector<std::shared_ptr<Accessibility::ReadableRulesNode>> ThirdRulesCheckNode::GetChildren()
{
    return GetChildrenTemplate<Accessibility::ReadableRulesNode>();
}

std::shared_ptr<FocusRulesCheckNode> ThirdRulesCheckNode::GetAceParent()
{
    CHECK_NULL_RETURN(nodeInfo_, nullptr);
    auto parentId = nodeInfo_->GetParentId();
    if (parentMap_) {
        auto parent = parentMap_->find(GetAccessibilityId());
        CHECK_EQUAL_RETURN(parent == parentMap_->end(), true, nullptr);
        parentId = parent->second;
    }
    if ((parentId == -1) || (parentId == Accessibility::AccessibilityElementInfo::ROOT_PARENT_ID)) {
        return nullptr;
    }
    if (nodeMap_) {
        auto parent = nodeMap_->find(parentId);
        if (parent == nodeMap_->end()) {
            return nullptr;
        }
        return std::make_shared<ThirdRulesCheckNode>(
            parent->second, parentId, accessibilityProvider_, parentMap_, nodeMap_);
    }
    auto parent = std::make_shared<ArkUI_AccessibilityElementInfo>();
    if (JsThirdProviderInteractionOperation::FindNativeInfoById(accessibilityProvider_, parentId, parent) &&
        parent->GetElementId() == parentId) {
        auto parentCheckNode = std::make_shared<ThirdRulesCheckNode>(
            parent, parent->GetElementId(), accessibilityProvider_, parentMap_);
        return parentCheckNode;
    }
    return nullptr;
}

std::shared_ptr<Accessibility::ReadableRulesNode> ThirdRulesCheckNode::GetParent()
{
    return GetAceParent();
}
} // OHOS::Ace::Framework

/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/node/frame_node_modifier.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace::NG {
ArkUI_Bool IsModifiable(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, false);
    auto* frameNode = AceType::DynamicCast<UINode>(currentNode);
    CHECK_NULL_RETURN(frameNode, false);
    return frameNode->GetTag() == "FrameNode";
}
void AppendChildInFrameNode(ArkUINodeHandle node, ArkUINodeHandle child)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_VOID(currentNode);
    auto* childNode = reinterpret_cast<UINode*>(child);
    auto childRef = Referenced::Claim<UINode>(childNode);
    currentNode->AddChild(childRef);
    currentNode->MarkNeedFrameFlushDirty(NG::PROPERTY_UPDATE_MEASURE);
}

void InsertChildAfterInFrameNode(ArkUINodeHandle node, ArkUINodeHandle child, ArkUINodeHandle sibling)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_VOID(currentNode);
    auto* childNode = reinterpret_cast<UINode*>(child);
    auto index = -1;
    auto* siblingNode = reinterpret_cast<UINode*>(sibling);
    index = currentNode->GetChildIndex(Referenced::Claim<UINode>(siblingNode));
    currentNode->AddChild(Referenced::Claim<UINode>(childNode), index + 1);
    currentNode->MarkNeedFrameFlushDirty(NG::PROPERTY_UPDATE_MEASURE);
}

void RemoveChildInFrameNode(ArkUINodeHandle node, ArkUINodeHandle child)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_VOID(currentNode);
    auto* childNode = reinterpret_cast<UINode*>(child);
    currentNode->RemoveChild(Referenced::Claim<UINode>(childNode));
    currentNode->MarkNeedFrameFlushDirty(NG::PROPERTY_UPDATE_MEASURE);
}

void ClearChildrenInFrameNode(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_VOID(currentNode);
    currentNode->Clean();
    currentNode->MarkNeedFrameFlushDirty(NG::PROPERTY_UPDATE_MEASURE);
}

ArkUI_Int32 GetChildrenNumber(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, -1);
    return currentNode->GetChildren().size();
}

ArkUINodeHandle GetChild(ArkUINodeHandle node, ArkUI_Int32 index)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, nullptr);
    auto child = currentNode->GetChildAtIndex(index);
    return reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(child));
}

ArkUINodeHandle GetFirst(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, nullptr);
    auto child = currentNode->GetFirstChild();
    return reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(child));
}

ArkUINodeHandle GetNextSibling(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, nullptr);
    auto parent = currentNode->GetParent();
    CHECK_NULL_RETURN(parent, nullptr);
    auto index = parent->GetChildIndex(Referenced::Claim<UINode>(currentNode));
    auto sibling = parent->GetChildAtIndex(index + 1);
    return reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(sibling));
}

ArkUINodeHandle GetPreviousSibling(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, nullptr);
    auto parent = currentNode->GetParent();
    CHECK_NULL_RETURN(parent, nullptr);
    auto index = parent->GetChildIndex(Referenced::Claim<UINode>(currentNode));
    auto sibling = parent->GetChildAtIndex(index - 1);
    return reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(sibling));
}

ArkUINodeHandle GetParent(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, nullptr);
    auto parent = currentNode->GetParent();
    return reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(parent));
}

ArkUI_Bool GetIdByNodePtr(ArkUINodeHandle node)
{
    auto* currentNode = reinterpret_cast<UINode*>(node);
    CHECK_NULL_RETURN(currentNode, -1);
    auto nodeId = currentNode->GetId();
    return nodeId;
}

namespace NodeModifier {
const ArkUIFrameNodeModifier* GetFrameNodeModifier()
{
    static const ArkUIFrameNodeModifier modifier = { IsModifiable, AppendChildInFrameNode, InsertChildAfterInFrameNode,
        RemoveChildInFrameNode, ClearChildrenInFrameNode, GetChildrenNumber, GetChild, GetFirst, GetNextSibling,
        GetPreviousSibling, GetParent, GetIdByNodePtr };

    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG

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

#include "core/components_ng/syntax/content_slot_node.h"

#include "core/common/builder_util.h"
#include "core/pipeline/base/element_register.h"
namespace OHOS::Ace::NG {

ContentSlotNode::~ContentSlotNode()
{
    std::list<RefPtr<NG::UINode>> nodes;
    for (const auto& child : GetChildren()) {
        BuilderUtils::GetBuilderNodes(child, nodes);
    }
    BuilderUtils::ClearChildInBuilderContainer(GetId(), nodes);
}
RefPtr<ContentSlotNode> ContentSlotNode::GetOrCreateContentSlot(int32_t nodeId)
{
    auto node = ElementRegister::GetInstance()->GetSpecificItemById<ContentSlotNode>(nodeId);
    if (node) {
        return node;
    }
    node = MakeRefPtr<ContentSlotNode>(nodeId);
    ElementRegister::GetInstance()->AddUINode(node);
    return node;
}

} // namespace OHOS::Ace::NG
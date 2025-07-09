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

#include "core/components_ng/syntax/arkoala_lazy_node.h"
namespace OHOS::Ace::NG {
void ArkoalaLazyNode::DoSetActiveChildRange(
    int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool showCache)
{
    struct Range {
        int32_t start;
        int32_t end;
    };
    const Range liveRange { start - cacheStart, end + cacheEnd };
    if (updateRange_) {
        updateRange_(liveRange.start, liveRange.end);
    }

    const auto activeRange = showCache ? liveRange : Range { start, end };
    std::list<RefPtr<UINode>> toRemove;
    for (const auto& [index, nodeWeak] : items_) {
        auto node = nodeWeak.Upgrade();
        if (index < liveRange.start || index > liveRange.end) {
            RemoveChild(node);
        } else {
            node->SetActive(index >= activeRange.start && index <= activeRange.end);
        }
    }

    items_.RemoveIf([liveRange](const uint32_t& k, const auto& _) {
        const auto idx = static_cast<int32_t>(k);
        return idx < liveRange.start || idx > liveRange.end;
    });
}

// void ArkoalaLazyNode::OnDataChange(int32_t changeIndex)
// {
//     items_.RemoveIf([idx = static_cast<int32_t>(changeIndex)](const uint32_t& k, const auto& _) { return k >= idx;
//     });
// }
RefPtr<UINode> ArkoalaLazyNode::GetFrameChildByIndex(uint32_t index, bool needBuild, bool isCache, bool addToRenderTree)
{
    const auto indexCasted = static_cast<int32_t>(index);
    auto item = GetFrameNode(indexCasted);
    if (item || !needBuild) {
        return item;
    }
    auto newItem = createItem_ ? createItem_(indexCasted) : nullptr;
    items_.Put(indexCasted, newItem);
    AddChild(newItem);

    if (isCache) {
        newItem->SetJSViewActive(false, true);
    } else if (addToRenderTree) {
        newItem->SetActive(true);
    }

    return newItem;
}
RefPtr<FrameNode> ArkoalaLazyNode::GetFrameNode(int32_t index)
{
    auto item = items_.Get(index);
    return item ? item->Upgrade() : nullptr;
}
} // namespace OHOS::Ace::NG
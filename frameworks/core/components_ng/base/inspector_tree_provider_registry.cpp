/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/inspector_tree_provider_registry.h"

#include "base/log/log_wrapper.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr char XCOMPONENT_TEXTURE_ETS_TAG[] = "XComponentTexture";
} // namespace

struct InspectorGetterEntry {
    InspectorTreeProviderRegistry::InspectorTreeGetter getter = nullptr;
    void* userData = nullptr;
    InspectorGetterEntry* next = nullptr;
};

namespace {
// Deletes a whole getter entry chain.
void DeleteGetterList(InspectorGetterEntry* head)
{
    while (head != nullptr) {
        auto* next = head->next;
        delete head;
        head = next;
    }
}
} // namespace

int32_t InspectorTreeProviderRegistry::Register(UINode* anchorNode, InspectorTreeGetter getter, void* userData)
{
    if (anchorNode == nullptr || getter == nullptr) {
        return RESULT_INVALID_PARAM;
    }
    if (!anchorNode->IsInspectorSubtreeHost() && anchorNode->GetTag() != V2::CUSTOM_ETS_TAG &&
        anchorNode->GetTag() != V2::XCOMPONENT_ETS_TAG && anchorNode->GetTag() != XCOMPONENT_TEXTURE_ETS_TAG) {
        return RESULT_UNSUPPORTED_HOST;
    }

    InspectorGetterEntry* tail = nullptr;
    auto* current = anchorNode->GetInspectorGetterEntry();
    while (current != nullptr) {
        if (current->getter == getter) {
            current->userData = userData;
            TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR,
                "Inspector tree getter re-registered on node %{private}d, userData replaced", anchorNode->GetId());
            return RESULT_OK;
        }
        tail = current;
        current = current->next;
    }
    auto* entry = new InspectorGetterEntry { getter, userData, nullptr };
    if (tail == nullptr) {
        anchorNode->SetInspectorGetterEntry(entry);
    } else {
        tail->next = entry;
    }
    TAG_LOGD(
        AceLogTag::ACE_LAYOUT_INSPECTOR, "Inspector tree getter registered on node %{private}d", anchorNode->GetId());
    return RESULT_OK;
}

int32_t InspectorTreeProviderRegistry::Unregister(UINode* anchorNode, InspectorTreeGetter getter)
{
    if (anchorNode == nullptr) {
        return RESULT_INVALID_PARAM;
    }
    // nullptr removes all registered getters of the node.
    if (getter == nullptr) {
        auto* head = anchorNode->GetInspectorGetterEntry();
        anchorNode->SetInspectorGetterEntry(nullptr);
        DeleteGetterList(head);
        if (head != nullptr) {
            TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR, "All inspector tree getters unregistered from node %{private}d",
                anchorNode->GetId());
        }
        return RESULT_OK;
    }
    InspectorGetterEntry* previous = nullptr;
    auto* current = anchorNode->GetInspectorGetterEntry();
    while (current != nullptr) {
        if (current->getter == getter) {
            if (previous == nullptr) {
                anchorNode->SetInspectorGetterEntry(current->next);
            } else {
                previous->next = current->next;
            }
            current->next = nullptr;
            delete current;
            TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR, "Inspector tree getter unregistered from node %{private}d",
                anchorNode->GetId());
            return RESULT_OK;
        }
        previous = current;
        current = current->next;
    }
    // The getter is not registered on the node.
    TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR,
        "Inspector tree getter not registered on node %{private}d, unregister rejected", anchorNode->GetId());
    return RESULT_NOT_FOUND;
}

void InspectorTreeProviderRegistry::GetInspectorGettersForNode(
    const RefPtr<UINode>& node, std::vector<GetterEntry>& getters)
{
    getters.clear();
    if (!node) {
        return;
    }
    auto* current = node->GetInspectorGetterEntry();
    while (current != nullptr) {
        if (current->getter != nullptr) {
            getters.push_back(GetterEntry { current->getter, current->userData });
        }
        current = current->next;
    }
}

void InspectorTreeProviderRegistry::ReleaseNodeEntry(UINode* node)
{
    CHECK_NULL_VOID(node);
    // UI-thread confined (see Register); unregistered fast path skips work.
    auto* head = node->GetInspectorGetterEntry();
    if (head == nullptr) {
        return;
    }
    node->SetInspectorGetterEntry(nullptr);
    DeleteGetterList(head);
    TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR,
        "Inspector tree getter entries released on node destruction, node %{private}d", node->GetId());
}
} // namespace OHOS::Ace::NG

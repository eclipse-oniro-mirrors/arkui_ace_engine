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

#include "core/components_ng/base/cmp_accessibility_registry.h"

#include <mutex>
#include <vector>

#include "base/log/log_wrapper.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/accessibility/node_utils/accessibility_frame_node_utils.h"
#include "core/common/frontend.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::CmpAccessibility {
namespace {
// Per-binding subAM registry, identity-anchored by the exported subtree root NODE POINTER.
struct SubAMRegistryEntry {
    WeakPtr<NG::FrameNode> root;
    WeakPtr<AceType> subAM;
};
std::vector<SubAMRegistryEntry>& GetSubAMRegistry()
{
    static std::vector<SubAMRegistryEntry> registry;
    return registry;
}
std::mutex& GetSubAMRegistryMutex()
{
    static std::mutex mutex;
    return mutex;
}
} // namespace

void CmpAccessibilityRegistry::RegisterSubAM(NG::FrameNode* exportedRoot, const RefPtr<AceType>& subAM)
{
    CHECK_NULL_VOID(exportedRoot);
    CHECK_NULL_VOID(subAM);
    // Single registry entry per exported subtree root: nodes inside the subtree reach the subAM by parent-chain walk-up (pointer identity).
    const auto rootWeak = Referenced::WeakClaim(exportedRoot);
    const auto subAMWeak = Referenced::WeakClaim(subAM.GetRawPtr());
    {
        std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
        auto& registry = GetSubAMRegistry();
        for (auto it = registry.begin(); it != registry.end();) {
            auto root = it->root.Upgrade();
            if (!root || root.GetRawPtr() == exportedRoot) {
                it = registry.erase(it); // stale entry, or idempotent re-bind for this root
            } else {
                ++it;
            }
        }
        registry.push_back({ rootWeak, subAMWeak });
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "RegisterSubAM: exportedRootId=%{public}d", exportedRoot->GetId());
}

void CmpAccessibilityRegistry::UnregisterSubAM(NG::FrameNode* exportedRoot)
{
    CHECK_NULL_VOID(exportedRoot);
    {
        std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
        auto& registry = GetSubAMRegistry();
        for (auto it = registry.begin(); it != registry.end();) {
            auto root = it->root.Upgrade();
            if (root && root.GetRawPtr() == exportedRoot) {
                it = registry.erase(it);
            } else {
                ++it;
            }
        }
    }
    // Pure registry-index erase (binding teardown path only).
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "UnregisterSubAM: exportedRootId=%{public}d", exportedRoot->GetId());
}

bool CmpAccessibilityRegistry::HasAnySubAM()
{
    std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
    return !GetSubAMRegistry().empty();
}

RefPtr<AceType> CmpAccessibilityRegistry::GetSubAM(const NG::FrameNode* node)
{
    CHECK_NULL_RETURN(node, nullptr);
    const NG::UINode* current = static_cast<const NG::UINode*>(node);
    while (current) {
        {
            std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
            for (const auto& entry : GetSubAMRegistry()) {
                auto root = entry.root.Upgrade();
                if (root && static_cast<const NG::UINode*>(root.GetRawPtr()) == current) {
                    return entry.subAM.Upgrade();
                }
            }
        }
        auto parent = current->GetParent(); // RefPtr<UINode> keeps parent alive during traversal
        current = parent ? static_cast<const NG::UINode*>(parent.GetRawPtr()) : nullptr;
    }
    return nullptr;
}

bool CmpAccessibilityRegistry::IsRegisteredExportedRoot(const NG::FrameNode* node)
{
    CHECK_NULL_RETURN(node, false);
    std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
    for (const auto& entry : GetSubAMRegistry()) {
        auto root = entry.root.Upgrade();
        if (root && root.GetRawPtr() == node) {
            return true;
        }
    }
    return false;
}

RefPtr<AceType> CmpAccessibilityRegistry::GetMainAccessibilityManager(NG::FrameNode* node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto context = node->GetContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto frontend = context->GetFrontend();
    CHECK_NULL_RETURN(frontend, nullptr);
    auto mainAM = frontend->GetAccessibilityManager();
    return AceType::DynamicCast<AceType>(mainAM);
}

bool CmpAccessibilityRegistry::HasSubAM(const NG::FrameNode* node)
{
    return GetSubAM(node) != nullptr;
}

RefPtr<NG::FrameNode> CmpAccessibilityRegistry::FindNodeInExportedSubtrees(int64_t accessibilityId)
{
    RefPtr<NG::FrameNode> node;
    FindSubAMAndNodeInExportedSubtrees(accessibilityId, node);
    return node;
}

RefPtr<AceType> CmpAccessibilityRegistry::FindSubAMAndNodeInExportedSubtrees(
    int64_t accessibilityId, RefPtr<NG::FrameNode>& node)
{
    node = nullptr;
    std::vector<SubAMRegistryEntry> entries;
    {
        std::lock_guard<std::mutex> lock(GetSubAMRegistryMutex());
        for (const auto& entry : GetSubAMRegistry()) {
            if (!entry.subAM.Upgrade() || !entry.root.Upgrade()) {
                continue; // stale entry — binding torn down without unregister / root died
            }
            entries.push_back(entry);
        }
    }
    for (const auto& entry : entries) {
        auto exportedRoot = entry.root.Upgrade();
        if (!exportedRoot) {
            continue;
        }
        auto found = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, accessibilityId);
        if (found) {
            node = found;
            return entry.subAM.Upgrade();
        }
    }
    return nullptr;
}
} // namespace OHOS::Ace::CmpAccessibility

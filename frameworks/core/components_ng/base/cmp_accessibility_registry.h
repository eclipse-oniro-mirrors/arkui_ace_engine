/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CMP_ACCESSIBILITY_REGISTRY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CMP_ACCESSIBILITY_REGISTRY_H

#include "base/memory/referenced.h"

namespace OHOS::Ace {
class AceType;
}

namespace OHOS::Ace::NG {
class FrameNode;
}

namespace OHOS::Ace::CmpAccessibility {

/**
 * @brief Lightweight registry for CMP mixed-rendering sub accessibility manager (subAM).
 *
 * Lifecycle: BindExportTree creates the subAM entry (per binding), UnbindExportTree or
 * adoption termination (RemoveAdoptedChild) destroys it. The subAM takes over the AM role
 * for Web/XComponent/Custom/UEC nodes within the exported subtree (they retrieve it by
 * tree walk-up via GetSubAM / GetAccessibilityManagerForNode).
 *
 * This header is declaration-only (core layer); the concrete implementation
 * lives in the adapter layer (js_accessibility_manager.cpp).
 */
class CmpAccessibilityRegistry final {
public:
    // Called from BindExportTree.
    static void RegisterSubAM(NG::FrameNode* exportedRoot, const RefPtr<AceType>& subAM);
    // Called from binding teardown (Unbind/DetachCmpBinding).
    static void UnregisterSubAM(NG::FrameNode* exportedRoot);
    // Fast path for hot paths.
    static bool HasAnySubAM();
    // Get the subAM from any node within an exported subtree (walk up to find registry entry).
    static RefPtr<AceType> GetSubAM(const NG::FrameNode* node);
    // Resolve a node by accessibility id inside any registered exported subtree.
    static RefPtr<NG::FrameNode> FindNodeInExportedSubtrees(int64_t accessibilityId);
    // Resolve (owning subAM, node) by accessibility id in one registry-anchored pass.
    static RefPtr<AceType> FindSubAMAndNodeInExportedSubtrees(
        int64_t accessibilityId, RefPtr<NG::FrameNode>& node);
    // True when the node is a registered exported-subtree root (pointer identity).
    static bool IsRegisteredExportedRoot(const NG::FrameNode* node);
    // Get the main accessibility manager from a node's pipeline context.
    static RefPtr<AceType> GetMainAccessibilityManager(NG::FrameNode* node);
    // Check whether the node is inside a bound exported subtree.
    static bool HasSubAM(const NG::FrameNode* node);
    // Bind/Unbind export tree via the main AM (dispatched to the concrete JsAccessibilityManager).
    static bool BindExportTree(NG::FrameNode* owner, int64_t bindId, NG::FrameNode* exportedRoot);
    static bool UnbindExportTree(NG::FrameNode* owner, int64_t bindId);

private:
    CmpAccessibilityRegistry() = delete;
    ~CmpAccessibilityRegistry() = delete;
};
} // namespace OHOS::Ace::CmpAccessibility

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CMP_ACCESSIBILITY_REGISTRY_H

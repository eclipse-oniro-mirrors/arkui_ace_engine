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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_ACCESSIBILITY_FOCUS_MOVE_THIRD_PARTY_FOCUS_TREE_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_ACCESSIBILITY_FOCUS_MOVE_THIRD_PARTY_FOCUS_TREE_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/accessibility/third_accessibility_traversal.h"
#include "base/memory/ace_type.h"
#include "frameworks/core/accessibility/accessibility_provider.h"
#include "frameworks/core/accessibility/native_interface_accessibility_impl.h"

namespace OHOS::Ace::Framework {

// Request-local focus tree assembled from a third-party provider answer. The focus search
// algorithm (AccessibilityFocusStrategy) walks ordered siblings (GetAceChildren) and parent
// chains (GetAceParent), so the flat ArkUI_AccessibilityElementInfoList returned by the
// provider must first be resolved into an explicit tree; this assembler is that step.
// Anomaly scenarios, budgets and repair rules follow the per-station comments in
// accessibility_focus_move_osal_third.cpp.
//
// The acquisition forms and their provider callback counts.
//   A. complete recursive answer  -> 1 callback;
//   B. recursive answer with holes -> 1 + H per-node hole-fill callbacks;
//   C. single record (the provider ignores the recursive mode, or the tree is a root only)
//      -> 1 + N per-node discovery callbacks, behavior kept compatible with the legacy path
//         (the record returned for the -1 query is the root, no root marker required).
// All three forms feed the same assembly pipeline (implementations in
// accessibility_focus_move_osal_third.cpp).
//
// Ownership: every record snapshot is a std::shared_ptr owned by the NodeMap handed to the
// caller; the assembler itself holds no raw ownership and releases all intermediate state
// when the request finishes. The provider is only referenced through a WeakPtr that is
// re-upgraded (and null-checked) before every callback.
class ThirdFocusTree {
public:
    using ParentMap = ThirdRulesCheckNode::ParentMap;
    using NodeMap = ThirdRulesCheckNode::NodeMap;

    struct Outcome {
        // Published resolution: id -> parent id (the root maps to the root marker).
        std::shared_ptr<ParentMap> parents = std::make_shared<ParentMap>();
        // Published snapshots, payload-sanitized (only dirty nodes were rewritten).
        std::shared_ptr<NodeMap> nodes = std::make_shared<NodeMap>();
        std::shared_ptr<ArkUI_AccessibilityElementInfo> root;
    };

    // Builds the tree for one focus request. Returns false when the answer cannot define a
    // usable tree (root query failed, empty answer, no root marker in a list answer, or a
    // budget was exceeded); every failure except the empty answer reports an anomaly through
    // the traversal. On failure the outcome maps stay empty.
    static bool Assemble(const WeakPtr<AccessibilityProvider>& provider, int32_t recursiveMode,
        ThirdAccessibilityTraversal& traversal, Outcome& outcome);

private:
    // How the record's own parentId declaration (pClaim) is categorized during ScanEdges.
    enum class ParentKind {
        // Missing (unfilled / -1 / 0-with-no-node-0 / pruned dangling / invalid).
        NONE,
        // parentId == ROOT_PARENT_ID: the record claims to be the root.
        MARKER,
        // A usable declaration pointing at another indexed record.
        VALID,
    };

    struct ThirdRecord {
        std::shared_ptr<ArkUI_AccessibilityElementInfo> snapshot;
        int64_t id = -1;
        ParentKind parentKind = ParentKind::NONE;
        // Meaningful only for ParentKind::VALID.
        int64_t declaredParent = -1;
        // Appended at the parent's children tail in receive order.
        bool adopted = false;
        // Excluded before publication.
        bool dropped = false;
    };

    ThirdFocusTree(const WeakPtr<AccessibilityProvider>& provider, int32_t recursiveMode,
        ThirdAccessibilityTraversal& traversal);
    ~ThirdFocusTree() = default;
    ThirdFocusTree(const ThirdFocusTree&) = delete;
    ThirdFocusTree& operator=(const ThirdFocusTree&) = delete;
    ThirdFocusTree(ThirdFocusTree&&) = delete;
    ThirdFocusTree& operator=(ThirdFocusTree&&) = delete;

    // Pipeline stations.
    // Total control.
    bool Run(Outcome& outcome);
    // The recursive root query; an empty answer fails silently. A provider that rejects
    // the prefetch mode gets one legacy retry without it.
    bool AcquireAnswer(std::vector<ArkUI_AccessibilityElementInfo>& answer);
    // Form C discovery from the record returned for the -1 query.
    bool AcquirePerNode(const ArkUI_AccessibilityElementInfo& rootRecord);
    // Per-child id and budget guard for the per-node walk.
    bool PushChild(int64_t childId, size_t depth, std::deque<std::pair<int64_t, size_t>>& pending);
    // Hole fill for records referenced by the answer but missing from it.
    bool FillThirdListHoles();
    void CollectListHoles(std::deque<int64_t>& pending);
    void CollectRecordHoles(const ArkUI_AccessibilityElementInfo& record,
        std::deque<int64_t>& pending);
    // Id, duplicate and budget guard for one record.
    bool IndexThirdRecord(const ArkUI_AccessibilityElementInfo& record);
    // One per-node provider query.
    std::shared_ptr<ArkUI_AccessibilityElementInfo> QueryOne(int64_t id);
    // Payload byte budget.
    bool ConsumeNodePayload(const ArkUI_AccessibilityElementInfo& node, int64_t id);
    // Claims, declarations and the edge budget.
    bool ScanEdges(bool listPath);
    // Root marker selection.
    bool ResolveRoot(bool listPath);
    // Per-node parent resolution.
    void RepairEdges();
    // Cycle pre-check and children build.
    void AttachNodes();
    // Depth budget and reachability.
    bool ComputeReachable();
    // Unreachable drop.
    void ReportUnreachable();
    // Dirty rewrite and publication.
    void SanitizeAndPublish(Outcome& outcome);

    // Helpers.
    bool BudgetBlown() const;
    // Budget message selection for a failed Visit on the per-node path.
    void ReportTraversalBudget(int64_t id);
    std::shared_ptr<ArkUI_AccessibilityElementInfo> SnapshotOf(int64_t id);
    // Cycle pre-check.
    bool WouldCloseCycle(int64_t nodeId, int64_t startParent) const;
    // Chain classification.
    bool IsChainCyclic(int64_t nodeId) const;

    const WeakPtr<AccessibilityProvider>& provider_;
    int32_t recursiveMode_;
    ThirdAccessibilityTraversal& traversal_;

    // Records in receive order.
    std::vector<ThirdRecord> records_;
    // id -> records_ slot.
    std::unordered_map<int64_t, size_t> index_;
    // child id -> claimers in discovery order.
    std::unordered_map<int64_t, std::vector<int64_t>> claims_;
    // Resolved parent.
    std::unordered_map<int64_t, int64_t> parentOf_;
    // Resolved, ordered children.
    std::unordered_map<int64_t, std::vector<int64_t>> childrenOf_;
    std::unordered_set<int64_t> reachable_;
    // Root markers in receive order.
    std::vector<int64_t> rootMarkers_;
    int64_t rootId_ = -1;
    // A parent was repaired from a claimer.
    bool sawMissingParentRepair_ = false;
    // A node was adopted by its declaration.
    bool sawUnclaimedAdoption_ = false;
};
} // namespace OHOS::Ace::Framework
#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_ACCESSIBILITY_FOCUS_MOVE_THIRD_PARTY_FOCUS_TREE_H

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
#include <cstring>
#include <limits>

#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal_third.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/accessibility/focus_move/third_party_focus_tree.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation_utils.h"

#include "accessibility_system_ability_client.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "adapter/ohos/osal/accessibility/third_accessibility_traversal.h"
#include "frameworks/core/accessibility/accessibility_provider.h"

using namespace OHOS::Accessibility;
using namespace OHOS::AccessibilityConfig;

namespace OHOS::Ace::Framework {

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
    CHECK_NULL_RETURN(rootInfo_, false);
    CHECK_NULL_RETURN(nodeMap_, false);
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
    // A failed or rejected assembly publishes nothing: the maps stay empty and rootInfo_ is
    // null, so the strategy reports an unusable tree for this request only.
    parentMap_ = std::make_shared<ThirdRulesCheckNode::ParentMap>();
    nodeMap_ = std::make_shared<ThirdRulesCheckNode::NodeMap>();
    rootInfo_.reset();
    ThirdAccessibilityTraversal traversal("focus");
    ThirdFocusTree::Outcome outcome;
    const int32_t recursiveMode = static_cast<int32_t>(PREFETCH_RECURSIVE_CHILDREN);
    if (!ThirdFocusTree::Assemble(accessibilityProvider_, recursiveMode, traversal, outcome)) {
        return false;
    }
    parentMap_ = std::move(outcome.parents);
    nodeMap_ = std::move(outcome.nodes);
    rootInfo_ = std::move(outcome.root);
    return rootInfo_ != nullptr;
}

// ThirdFocusTree implementation (declaration and contract in third_party_focus_tree.h).
// Lives in this translation unit, not a separate third_party_focus_tree.cpp, so every
// target that already compiles this file links the assembler without an extra source entry.
namespace {
// Valid element ids fit the SDK contract: non-negative int32.
bool IsValidElementId(int64_t id)
{
    return id >= 0 && id <= std::numeric_limits<int32_t>::max();
}

// The convention marking a record as the tree root.
int64_t RootMarker()
{
    return Accessibility::AccessibilityElementInfo::ROOT_PARENT_ID;
}

// How one record's own parentId declaration is classified.
struct DeclarationInfo {
    // parentId == ROOT_PARENT_ID: the record claims to be the root.
    bool isMarker = false;
    // A usable declaration pointing at another indexed record.
    bool declaredValid = false;
    // Meaningful only when declaredValid.
    int64_t declaredParent = -1;
};

// Whether the declared parent is among the claimers.
bool ContainsClaimer(const std::vector<int64_t>& claimers, int64_t declaredParent)
{
    for (int64_t claimer : claimers) {
        if (claimer == declaredParent) {
            return true;
        }
    }
    return false;
}

// Classifies one record's parentId declaration. Unfilled conventions (-1, or 0 when node 0
// is absent) and pruned dangling references resolve to no declaration at all; self and
// out-of-domain declarations are reported and ignored.
DeclarationInfo ClassifyParentDeclaration(int64_t id, int64_t parentId,
    const std::unordered_map<int64_t, size_t>& index, ThirdAccessibilityTraversal& traversal)
{
    DeclarationInfo info;
    if (parentId == RootMarker()) {
        info.isMarker = true;
    } else if (parentId == id) {
        traversal.ReportAnomaly(ThirdAccessibilityTraversal::SELF_PARENT, id,
            "node declares itself as its parent, declaration ignored");
    } else if (parentId == -1 || parentId == 0) {
        // Unfilled conventions. parentId=0 additionally resolves as a legal edge when
        // node 0 exists (both interpretations then produce the same edge).
        if (parentId == 0 && index.count(0) != 0) {
            info.declaredValid = true;
            info.declaredParent = 0;
        }
    } else if (!IsValidElementId(parentId)) {
        traversal.ReportAnomaly(ThirdAccessibilityTraversal::INVALID_ID, id,
            "node declares an invalid parent id, declaration ignored");
    } else if (index.count(parentId) != 0) {
        info.declaredValid = true;
        info.declaredParent = parentId;
    }
    // Otherwise the declaration dangles: the failed hole chase already reported it.
    return info;
}

// The claim-walk state shared by the records of one answer: what the per-record claim
// collection reads and writes.
struct ClaimWalkContext {
    // Whether this is a list answer; the edge budget and the per-reference anomaly logs
    // are list-only (the per-node walk already enforced and reported them).
    bool listPath = false;
    // id -> records_ slot, to skip references the answer never delivered.
    const std::unordered_map<int64_t, size_t>& index;
    // child id -> claimers in discovery order.
    std::unordered_map<int64_t, std::vector<int64_t>>& claims;
    // Edge budget for list answers.
    size_t edgeCount = 0;
};

// Collects the claims one record lays on its childNodeIds, in declaration order. Enforces
// the edge budget for list answers; for per-node acquisition Visit already counted and
// enforced the same edges (and reported invalid/repeated references), so those logs are
// list-only. Returns false when the edge budget rejects the tree.
bool CollectDeclaredClaims(int64_t parentId, const ArkUI_AccessibilityElementInfo& snapshot,
    ClaimWalkContext& context, ThirdAccessibilityTraversal& traversal)
{
    std::unordered_set<int64_t> emitted;
    for (int64_t childId : snapshot.GetChildNodeIds()) {
        if (context.listPath && ++context.edgeCount > ThirdAccessibilityTraversal::MAX_EDGES) {
            traversal.ReportAnomaly(ThirdAccessibilityTraversal::LIMIT_EXCEEDED, childId,
                "child edge budget exceeded, tree rejected");
            return false;
        }
        if (!IsValidElementId(childId)) {
            if (context.listPath) {
                traversal.ReportAnomaly(ThirdAccessibilityTraversal::INVALID_ID, childId,
                    "invalid child reference in childNodeIds, edge dropped");
            }
            continue;
        }
        if (context.index.count(childId) == 0) {
            // The hole chase already reported the missing record.
            continue;
        }
        if (!emitted.insert(childId).second) {
            if (context.listPath) {
                traversal.ReportAnomaly(ThirdAccessibilityTraversal::DUPLICATE_CHILD, childId,
                    "duplicate child edge in one answer, first kept");
            }
            continue;
        }
        // Claim discovery order: parent receive order.
        context.claims[childId].push_back(parentId);
    }
    return true;
}

// The parent resolution for one record.
struct ParentResolution {
    // parent is meaningful only when resolved.
    bool resolved = false;
    int64_t parent = -1;
    // The node was adopted by its declaration (appended at the children tail).
    bool adopted = false;
    // The node has no relationship at all and is dropped.
    bool dropped = false;
    // The missing declaration was repaired from the unique claimer.
    bool repairedFromClaimer = false;
    // The declared parent was adopted because nobody claimed the node.
    bool adoptedByDeclaration = false;
};

// Appends the children one record claims in childNodeIds declaration order, keeping only
// the entries that resolved to this record.
void AppendDeclaredChildren(int64_t parentId, const ArkUI_AccessibilityElementInfo& snapshot,
    const std::unordered_map<int64_t, int64_t>& parentOf,
    std::unordered_map<int64_t, std::vector<int64_t>>& children,
    const std::unordered_set<int64_t>& dropped)
{
    std::unordered_set<int64_t> emitted;
    for (int64_t childId : snapshot.GetChildNodeIds()) {
        if (!emitted.insert(childId).second) {
            // Repeated entry: the first position wins.
            continue;
        }
        const auto childIt = parentOf.find(childId);
        if (childIt == parentOf.end() || childIt->second != parentId || dropped.count(childId) != 0) {
            // Resolved elsewhere, not indexed, or the child was dropped.
            continue;
        }
        children[parentId].push_back(childId);
    }
}

// Resolves the parent from the claimers when the declaration is not confirmed: a single
// claimer wins outright (a conflicting declaration is reported and dropped, a missing one
// is repaired), several claimers keep the first in receive order.
ParentResolution ResolveFromClaimers(const std::vector<int64_t>& claimers, bool declaredValid,
    int64_t id, ThirdAccessibilityTraversal& traversal)
{
    ParentResolution result;
    if (claimers.size() == 1) {
        if (declaredValid) {
            traversal.ReportAnomaly(ThirdAccessibilityTraversal::PARENT_CONFLICT, id,
                "parent declaration conflicts with childNodeIds, child side kept");
        } else {
            traversal.ReportAnomaly(ThirdAccessibilityTraversal::MISSING_PARENT, id,
                "missing parentId, repaired from the unique claimer");
            result.repairedFromClaimer = true;
        }
        result.resolved = true;
        result.parent = claimers.front();
        return result;
    }
    // Multiple claimers without a confirmed declaration.
    if (declaredValid) {
        traversal.ReportAnomaly(ThirdAccessibilityTraversal::PARENT_CONFLICT, id,
            "parent declaration conflicts with childNodeIds, child side kept");
    }
    traversal.ReportAnomaly(ThirdAccessibilityTraversal::MULTI_PARENT_FIRST, id,
        "node claimed by multiple parents, first claimer kept");
    result.resolved = true;
    result.parent = claimers.front();
    return result;
}

// Resolves one parent for one non-root, non-marker record from the record's own parentId
// declaration and the childNodeIds claims of other records.
//   both sides agree                    -> declared parent, no log
//   missing declaration, unique claimer -> claimer
//   declared, unclaimed                 -> declared parent, adopted in receive order
//   declared != claimer                 -> claimer side (childNodeIds carries order)
//   multiple claimers, self-picked      -> declared parent
//   multiple claimers, no pick          -> first claimer in receive order
//   neither side declares anything      -> dropped
ParentResolution ResolveParentForRecord(int64_t id, bool declaredValid, int64_t declaredParent,
    const std::vector<int64_t>* claimers, ThirdAccessibilityTraversal& traversal)
{
    ParentResolution result;
    const bool hasClaimers = claimers != nullptr;
    const bool declaredClaimed = declaredValid && hasClaimers && ContainsClaimer(*claimers, declaredParent);
    // The declaration is confirmed by a claim.
    if (declaredValid && declaredClaimed) {
        if (claimers->size() > 1) {
            traversal.ReportAnomaly(ThirdAccessibilityTraversal::MULTI_PARENT_SELF, id,
                "node claimed by multiple parents, self-declared parent kept");
        }
        result.resolved = true;
        result.parent = declaredParent;
        return result;
    }
    // Declared parent, nobody claims the node.
    if (declaredValid && !hasClaimers) {
        traversal.ReportAnomaly(ThirdAccessibilityTraversal::UNCLAIMED_CHILD, id,
            "declared parent unclaimed, node adopted in receive order");
        result.resolved = true;
        result.parent = declaredParent;
        result.adopted = true;
        result.adoptedByDeclaration = true;
        return result;
    }
    // No claim at all.
    if (!hasClaimers) {
        traversal.ReportAnomaly(ThirdAccessibilityTraversal::ORPHAN_NODE, id,
            "isolated node without any relationship dropped");
        result.dropped = true;
        return result;
    }
    return ResolveFromClaimers(*claimers, declaredValid, id, traversal);
}
} // namespace

bool ThirdFocusTree::Assemble(const WeakPtr<AccessibilityProvider>& provider, int32_t recursiveMode,
    ThirdAccessibilityTraversal& traversal, Outcome& outcome)
{
    ThirdFocusTree tree(provider, recursiveMode, traversal);
    return tree.Run(outcome);
}

ThirdFocusTree::ThirdFocusTree(const WeakPtr<AccessibilityProvider>& provider, int32_t recursiveMode,
    ThirdAccessibilityTraversal& traversal)
    : provider_(provider), recursiveMode_(recursiveMode), traversal_(traversal) {}

bool ThirdFocusTree::Run(Outcome& outcome)
{
    std::vector<ArkUI_AccessibilityElementInfo> answer;
    if (!AcquireAnswer(answer)) {
        // Root query failed (logged) or an empty answer (silent, e.g. an empty page).
        return false;
    }
    // Form detection: a single record means the provider ignored the recursive prefetch
    // mode, or the tree is a root only. Every single-record answer keeps the legacy root
    // contract (the record returned for the -1 query is the root), so no root marker is
    // required; walking an empty child list costs no extra query.
    const bool perNodeMode = answer.size() == 1;
    if (perNodeMode) {
        if (!AcquirePerNode(answer.front())) {
            return false;
        }
    } else {
        for (const auto& record : answer) {
            if (!IndexThirdRecord(record)) {
                // Budget exceeded.
                return false;
            }
        }
        if (!FillThirdListHoles()) {
            // Budget exceeded.
            return false;
        }
    }
    if (!ScanEdges(!perNodeMode)) {
        // Edge budget (list path only; Visit guards the per-node path).
        return false;
    }
    if (!ResolveRoot(!perNodeMode)) {
        // No root marker.
        return false;
    }
    RepairEdges();
    AttachNodes();
    if (!ComputeReachable()) {
        // Depth budget.
        return false;
    }
    ReportUnreachable();
    SanitizeAndPublish(outcome);
    // Defensive only: a successful run always publishes the root.
    return outcome.root != nullptr && !outcome.nodes->empty();
}

// The single recursive root query FindAccessibilityNodeInfosById(-1, RECURSIVE).
bool ThirdFocusTree::AcquireAnswer(std::vector<ArkUI_AccessibilityElementInfo>& answer)
{
    auto provider = provider_.Upgrade();
    if (!provider) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::QUERY_FAILED, -1,
            "provider root query failed");
        return false;
    }
    int32_t code = provider->FindAccessibilityNodeInfosById(-1, recursiveMode_, 0, answer);
    if (code != 0 && recursiveMode_ != 0) {
        // A provider that rejects the recursive prefetch mode keeps the legacy contract:
        // one retry without a prefetch mode before the search fails. A legacy provider is
        // supported, so a successful retry reports nothing.
        answer.clear();
        code = provider->FindAccessibilityNodeInfosById(-1, 0, 0, answer);
    }
    if (code != 0) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::QUERY_FAILED, -1,
            "provider root query failed");
        return false;
    }
    if (answer.empty()) {
        // An empty answer is not a provider anomaly (for example an empty page); the
        // focus search fails silently.
        return false;
    }
    return true;
}

// Form C acquisition: per-node discovery from the record returned for the -1 query.
bool ThirdFocusTree::AcquirePerNode(const ArkUI_AccessibilityElementInfo& rootRecord)
{
    const int64_t rootId = rootRecord.GetElementId();
    if (!IsValidElementId(rootId)) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::INVALID_ID, rootId,
            "invalid element id rejected");
        return false;
    }
    if (!IndexThirdRecord(rootRecord)) {
        // Payload budget.
        return false;
    }
    // Legacy contract: the record returned for the -1 query is the root; no marker required.
    rootId_ = rootId;
    if (!traversal_.Visit(rootId, 0)) {
        ReportTraversalBudget(rootId);
        return false;
    }
    std::deque<std::pair<int64_t, size_t>> pending;
    pending.emplace_back(rootId, 0);
    while (!pending.empty()) {
        const auto [id, depth] = pending.front();
        pending.pop_front();
        // The root snapshot is already indexed from the root answer; every other node is
        // fetched per-node.
        std::shared_ptr<ArkUI_AccessibilityElementInfo> snapshot =
            (id == rootId_) ? SnapshotOf(id) : QueryOne(id);
        if (snapshot == nullptr) {
            // A per-node query failure is already reported; a null root snapshot is defensive.
            if (id != rootId_ && BudgetBlown()) {
                return false;
            }
            continue;
        }
        if (id != rootId_ && !IndexThirdRecord(*snapshot)) {
            // Budget exceeded.
            return false;
        }
        for (int64_t childId : snapshot->GetChildNodeIds()) {
            if (!PushChild(childId, depth, pending)) {
                // Budget exceeded.
                return false;
            }
        }
    }
    return true;
}

// One guarded child push for the per-node walk: repeated or invalid references consume an
// edge and are skipped, budget failures abort the whole tree.
bool ThirdFocusTree::PushChild(int64_t childId, size_t depth,
    std::deque<std::pair<int64_t, size_t>>& pending)
{
    const bool invalid = !IsValidElementId(childId);
    if (!traversal_.Visit(childId, depth + 1)) {
        if (BudgetBlown()) {
            ReportTraversalBudget(childId);
            return false;
        }
        // Visit already counted the issue; log the line without a second count.
        if (invalid) {
            traversal_.LogAnomaly(ThirdAccessibilityTraversal::INVALID_ID, childId,
                "invalid element id rejected");
        } else {
            // A non-budget failure with a valid id has one remaining cause: a repeated id
            // (duplicate edge or cycle).
            traversal_.LogAnomaly(ThirdAccessibilityTraversal::REPEATED_ID, childId,
                "repeated id (duplicate edge or cycle) rejected");
        }
        return true;
    }
    pending.emplace_back(childId, depth + 1);
    return true;
}

// Fetch per-node every record referenced by the answer but missing from it.
// Only parent references greater than 0 are chased: parentId=0 is the struct default and
// chasing it would trigger a per-node query for every record that never set a parent.
bool ThirdFocusTree::FillThirdListHoles()
{
    std::deque<int64_t> pending;
    CollectListHoles(pending);
    std::unordered_set<int64_t> chased;
    // One chase is one synchronous provider callback on the calling thread. A chase
    // fetches one record, so it is charged against the node budget, not the wider edge
    // budget; the bound then matches the per-node discovery callback count.
    size_t holeChases = 0;
    while (!pending.empty()) {
        const int64_t id = pending.front();
        pending.pop_front();
        if (!IsValidElementId(id) || chased.count(id) != 0 || index_.count(id) != 0) {
            // Invalid references are reported by ScanEdges; already fetched.
            continue;
        }
        if (++holeChases > ThirdAccessibilityTraversal::MAX_NODES) {
            traversal_.ReportAnomaly(ThirdAccessibilityTraversal::LIMIT_EXCEEDED, id,
                "hole chase budget exceeded, tree rejected");
            return false;
        }
        chased.insert(id);
        // A hole chase is the supported acquisition form's normal cost, not a provider
        // error: count it for the summary line only, so repeated focus requests from a
        // provider with sparse answers stay silent.
        traversal_.Record(ThirdAccessibilityTraversal::LIST_HOLE, id);
        auto snapshot = QueryOne(id);
        if (!snapshot) {
            if (BudgetBlown()) {
                // Budget exceeded.
                return false;
            }
            // Query failure already reported; referencing edges are pruned later.
            continue;
        }
        if (!IndexThirdRecord(*snapshot)) {
            // Budget exceeded.
            return false;
        }
        CollectRecordHoles(*snapshot, pending);
    }
    return true;
}

void ThirdFocusTree::CollectListHoles(std::deque<int64_t>& pending)
{
    for (const auto& entry : records_) {
        if (!entry.snapshot) {
            continue;
        }
        CollectRecordHoles(*entry.snapshot, pending);
    }
}

void ThirdFocusTree::CollectRecordHoles(const ArkUI_AccessibilityElementInfo& record,
    std::deque<int64_t>& pending)
{
    for (int64_t childId : record.GetChildNodeIds()) {
        if (IsValidElementId(childId) && index_.count(childId) == 0) {
            pending.push_back(childId);
        }
    }
    const int64_t parentId = record.GetParentId();
    if (parentId > 0 && IsValidElementId(parentId) && parentId != record.GetElementId() &&
        index_.count(parentId) == 0) {
        pending.push_back(parentId);
    }
}

// IndexThirdRecord: invalid element id, duplicate id (first record wins), node and payload
// budgets.
bool ThirdFocusTree::IndexThirdRecord(const ArkUI_AccessibilityElementInfo& record)
{
    const int64_t id = record.GetElementId();
    if (!IsValidElementId(id)) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::INVALID_ID, id,
            "record with an invalid element id ignored");
        // Drop this record, keep assembling.
        return true;
    }
    if (index_.count(id) != 0) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::DUPLICATE_ID, id,
            "duplicate element id in one answer, first record kept");
        // First record wins.
        return true;
    }
    if (records_.size() >= ThirdAccessibilityTraversal::MAX_NODES) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::LIMIT_EXCEEDED, id,
            "node budget exceeded, tree rejected");
        return false;
    }
    if (!ConsumeNodePayload(record, id)) {
        // Payload budget (reported inside).
        return false;
    }
    ThirdRecord entry;
    entry.snapshot = AccessibilityThirdProviderUtils::MakeNodeSnapshot(record);
    entry.id = id;
    if (!entry.snapshot) {
        // Defensive: MakeNodeSnapshot allocates and cannot return null.
        return false;
    }
    records_.push_back(std::move(entry));
    index_.emplace(id, records_.size() - 1);
    return true;
}

// One per-node provider query. Duplicates in a per-node answer keep the first record;
// an id mismatch or a failed callback rejects the record. The payload budget is charged
// where the caller indexes the snapshot (IndexThirdRecord), never here: both call sites
// always index, so every record is charged exactly once in every acquisition form.
std::shared_ptr<ArkUI_AccessibilityElementInfo> ThirdFocusTree::QueryOne(int64_t id)
{
    auto provider = provider_.Upgrade();
    if (!provider) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::QUERY_FAILED, id,
            "provider per-node query failed");
        return nullptr;
    }
    std::vector<ArkUI_AccessibilityElementInfo> nativeInfos;
    const int32_t code = provider->FindAccessibilityNodeInfosById(id, 0, 0, nativeInfos);
    if (code != 0 || nativeInfos.empty()) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::QUERY_FAILED, id,
            "provider per-node query failed");
        return nullptr;
    }
    // Deliberate divergence from the shared single-node response validator, which rejects
    // a multi-record answer outright: here the first record must carry the queried id
    // (checked below), which the provider contract ranks first, and dropping the record
    // would cut the node out of the tree. The first record wins; the extras only count.
    if (nativeInfos.size() > 1) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::DUPLICATE_ID, id,
            "provider returned duplicate records for one query, first kept");
    }
    const auto& first = nativeInfos.front();
    if (first.GetElementId() != id) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::ID_MISMATCH, id,
            "provider returned a record with an unexpected id, rejected");
        return nullptr;
    }
    return AccessibilityThirdProviderUtils::MakeNodeSnapshot(first);
}

// Payload byte budget, mirroring the shared response validator's accounting.
bool ThirdFocusTree::ConsumeNodePayload(const ArkUI_AccessibilityElementInfo& node, int64_t id)
{
    size_t payloadBytes = sizeof(node);
    payloadBytes += node.GetChildNodeIds().size() * sizeof(int64_t);
    const auto& actions = node.GetOperationActions();
    if (actions.size() > ThirdAccessibilityTraversal::MAX_BYTES / sizeof(ArkUI_AccessibleAction)) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::PAYLOAD_LIMIT, id,
            "payload byte budget exceeded");
        return false;
    }
    payloadBytes += actions.size() * sizeof(ArkUI_AccessibleAction);
    const auto strings = { &ArkUI_AccessibilityElementInfo::GetComponentType,
        &ArkUI_AccessibilityElementInfo::GetContents, &ArkUI_AccessibilityElementInfo::GetHintText,
        &ArkUI_AccessibilityElementInfo::GetAccessibilityText,
        &ArkUI_AccessibilityElementInfo::GetAccessibilityDescription,
        &ArkUI_AccessibilityElementInfo::GetAccessibilityLevel,
        &ArkUI_AccessibilityElementInfo::GetBackgroundColor,
        &ArkUI_AccessibilityElementInfo::GetBackgroundImage,
        &ArkUI_AccessibilityElementInfo::GetBlur,
        &ArkUI_AccessibilityElementInfo::GetHitTestBehavior,
        &ArkUI_AccessibilityElementInfo::GetComponentIdentifier };
    for (auto getter : strings) {
        payloadBytes += (node.*getter)().size();
    }
    for (const auto& action : actions) {
        if (action.description != nullptr) {
            payloadBytes += strnlen(action.description, ThirdAccessibilityTraversal::MAX_BYTES + 1);
        }
    }
    if (!traversal_.ConsumeBytes(payloadBytes, id)) {
        // ConsumeBytes already counted the issue; log the line without a second count.
        traversal_.LogAnomaly(ThirdAccessibilityTraversal::PAYLOAD_LIMIT, id,
            "payload byte budget exceeded");
        return false;
    }
    return true;
}

// ScanEdges: classify every pClaim, collect claims in declaration order, and enforce the
// edge budget for list answers. For per-node acquisition Visit already counted and enforced
// the same edges (and reported invalid/repeated references), so those logs are list-only.
bool ThirdFocusTree::ScanEdges(bool listPath)
{
    ClaimWalkContext context { listPath, index_, claims_, 0 };
    for (auto& entry : records_) {
        if (!entry.snapshot) {
            continue;
        }
        const DeclarationInfo declaration =
            ClassifyParentDeclaration(entry.id, entry.snapshot->GetParentId(), index_, traversal_);
        if (declaration.isMarker) {
            entry.parentKind = ParentKind::MARKER;
            rootMarkers_.push_back(entry.id);
        } else if (declaration.declaredValid) {
            entry.parentKind = ParentKind::VALID;
            entry.declaredParent = declaration.declaredParent;
        }
        if (!CollectDeclaredClaims(entry.id, *entry.snapshot, context, traversal_)) {
            return false;
        }
    }
    return true;
}

// SelectRoot: extra markers (first in receive order wins); a non-empty list answer without
// any marker fails the search. The marker is the hard contract for a list answer: guessing
// a markerless record into the root would move focus to wrong nodes, so the search fails
// instead of repairing. Providers that cannot set the marker keep the legacy root contract
// by answering the -1 query with a single record (the per-node path).
bool ThirdFocusTree::ResolveRoot(bool listPath)
{
    if (listPath) {
        if (rootMarkers_.empty()) {
            traversal_.ReportAnomaly(ThirdAccessibilityTraversal::NO_ROOT, -1,
                "no root marker in a non-empty answer, focus search failed");
            return false;
        }
        rootId_ = rootMarkers_.front();
    }
    for (int64_t markerId : rootMarkers_) {
        if (markerId != rootId_) {
            traversal_.ReportAnomaly(ThirdAccessibilityTraversal::MULTIPLE_ROOTS, markerId,
                "extra root marker, only the first root is used");
        }
    }
    return true;
}

// RepairEdges: resolve one parent per node from the two independent declaration sources,
// then merge the mixed missing-declaration forms (childIds order first, adopted tail).
void ThirdFocusTree::RepairEdges()
{
    for (auto& entry : records_) {
        if (!entry.snapshot) {
            continue;
        }
        if (entry.id == rootId_ || entry.parentKind == ParentKind::MARKER) {
            // The selected root and detached pseudo-roots attach to nothing, even when a
            // childNodeIds claim on them exists: the marker outranks a claim, so an extra
            // marker and its subtree stay unreachable from the root and drop.
            parentOf_[entry.id] = RootMarker();
            continue;
        }
        const auto claimIt = claims_.find(entry.id);
        const std::vector<int64_t>* claimers =
            (claimIt == claims_.end() || claimIt->second.empty()) ? nullptr : &claimIt->second;
        const ParentResolution resolution = ResolveParentForRecord(entry.id,
            entry.parentKind == ParentKind::VALID, entry.declaredParent, claimers, traversal_);
        if (resolution.dropped) {
            entry.dropped = true;
        } else if (resolution.resolved) {
            parentOf_[entry.id] = resolution.parent;
        }
        entry.adopted = entry.adopted || resolution.adopted;
        sawMissingParentRepair_ = sawMissingParentRepair_ || resolution.repairedFromClaimer;
        sawUnclaimedAdoption_ = sawUnclaimedAdoption_ || resolution.adoptedByDeclaration;
    }
    if (sawMissingParentRepair_ && sawUnclaimedAdoption_) {
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::MIXED_SOURCES, -1,
            "mixed missing declarations, merged from both sides");
    }
}

// Tree-attach pass: cycle pre-check before every attach, then the ordered children
// lists (childIds declaration order first, adopted nodes appended in receive order).
void ThirdFocusTree::AttachNodes()
{
    for (auto& entry : records_) {
        if (!entry.snapshot || entry.dropped) {
            continue;
        }
        const auto parentIt = parentOf_.find(entry.id);
        if (parentIt == parentOf_.end()) {
            // Not resolved: pseudo-roots and dropped orphans.
            continue;
        }
        if (parentIt->second == RootMarker()) {
            // The selected root and detached pseudo-roots attach to nothing.
            continue;
        }
        if (WouldCloseCycle(entry.id, parentIt->second)) {
            traversal_.ReportAnomaly(ThirdAccessibilityTraversal::WOULD_CYCLE, entry.id,
                "node would close a cycle, dropped");
            entry.dropped = true;
        }
    }
    // Claimed children in declaration order.
    std::unordered_set<int64_t> dropped;
    for (const auto& entry : records_) {
        if (entry.dropped) {
            dropped.insert(entry.id);
        }
    }
    for (auto& entry : records_) {
        if (entry.snapshot) {
            AppendDeclaredChildren(entry.id, *entry.snapshot, parentOf_, childrenOf_, dropped);
        }
    }
    // Adopted children appended in receive order, after the declared ones.
    for (auto& entry : records_) {
        if (!entry.snapshot || entry.dropped || !entry.adopted) {
            continue;
        }
        const auto parentIt = parentOf_.find(entry.id);
        if (parentIt == parentOf_.end()) {
            continue;
        }
        childrenOf_[parentIt->second].push_back(entry.id);
    }
}

// ComputeReachable: BFS from the root; a tree deeper than the depth budget is rejected.
bool ThirdFocusTree::ComputeReachable()
{
    std::deque<std::pair<int64_t, size_t>> pending;
    reachable_.insert(rootId_);
    pending.emplace_back(rootId_, 0);
    while (!pending.empty()) {
        const auto [id, depth] = pending.front();
        pending.pop_front();
        const auto childrenIt = childrenOf_.find(id);
        if (childrenIt == childrenOf_.end()) {
            continue;
        }
        for (int64_t childId : childrenIt->second) {
            if (depth + 1 > ThirdAccessibilityTraversal::MAX_DEPTH) {
                traversal_.ReportAnomaly(ThirdAccessibilityTraversal::DEPTH_LIMIT, childId,
                    "tree deeper than the depth budget, tree rejected");
                return false;
            }
            if (reachable_.insert(childId).second) {
                pending.emplace_back(childId, depth + 1);
            }
        }
    }
    return true;
}

// ReportUnreachable: drop nodes unreachable from the root, classified CYCLE or ORPHAN by
// walking the parent chain.
void ThirdFocusTree::ReportUnreachable()
{
    for (const auto& entry : records_) {
        if (!entry.snapshot || entry.dropped || reachable_.count(entry.id) != 0) {
            continue;
        }
        const bool cyclic = IsChainCyclic(entry.id);
        traversal_.ReportAnomaly(ThirdAccessibilityTraversal::UNREACHABLE, entry.id,
            cyclic ? "node unreachable from the root (cycle), dropped" :
                     "node unreachable from the root (orphan), dropped");
    }
}

// Payload sanitization and publication: rewrite only dirty nodes (where the resolution
// differs from the report), then fill the outcome maps with the reachable records.
void ThirdFocusTree::SanitizeAndPublish(Outcome& outcome)
{
    for (auto& entry : records_) {
        if (!entry.snapshot || entry.dropped || reachable_.count(entry.id) == 0) {
            continue;
        }
        const auto parentIt = parentOf_.find(entry.id);
        if (parentIt == parentOf_.end()) {
            continue;
        }
        const int64_t parentId = parentIt->second;
        static const std::vector<int64_t> emptyChildren;
        const auto childrenIt = childrenOf_.find(entry.id);
        const std::vector<int64_t>& children =
            (childrenIt == childrenOf_.end()) ? emptyChildren : childrenIt->second;
        if (entry.snapshot->GetParentId() != parentId ||
            entry.snapshot->GetChildNodeIds() != children) {
            entry.snapshot->SetParentId(parentId);
            entry.snapshot->ClearChildNodeIds();
            for (int64_t childId : children) {
                entry.snapshot->AddChildNodeId(childId);
            }
        }
        outcome.parents->emplace(entry.id, parentId);
        outcome.nodes->emplace(entry.id, entry.snapshot);
        if (entry.id == rootId_) {
            outcome.root = entry.snapshot;
        }
    }
}

bool ThirdFocusTree::BudgetBlown() const
{
    return traversal_.HasIssue(ThirdAccessibilityTraversal::LIMIT_EXCEEDED) ||
        traversal_.HasIssue(ThirdAccessibilityTraversal::DEPTH_LIMIT) ||
        traversal_.HasIssue(ThirdAccessibilityTraversal::PAYLOAD_LIMIT);
}

// Budget message selection for a failed Visit on the per-node path. Visit already counted
// the issue, so the selected line is logged without a second count.
void ThirdFocusTree::ReportTraversalBudget(int64_t id)
{
    if (traversal_.HasIssue(ThirdAccessibilityTraversal::DEPTH_LIMIT)) {
        traversal_.LogAnomaly(ThirdAccessibilityTraversal::DEPTH_LIMIT, id,
            "tree deeper than the depth budget, tree rejected");
    } else if (traversal_.EdgeBudgetBlown()) {
        traversal_.LogAnomaly(ThirdAccessibilityTraversal::LIMIT_EXCEEDED, id,
            "traversal edge/depth budget exceeded");
    } else {
        traversal_.LogAnomaly(ThirdAccessibilityTraversal::LIMIT_EXCEEDED, id,
            "traversal node budget exceeded");
    }
}

std::shared_ptr<ArkUI_AccessibilityElementInfo> ThirdFocusTree::SnapshotOf(int64_t id)
{
    const auto it = index_.find(id);
    if (it == index_.end() || it->second >= records_.size()) {
        return nullptr;
    }
    return records_[it->second].snapshot;
}

// Pre-check: would attaching nodeId under startParent close a cycle? Walks the already
// resolved parent chain, bounded by the depth budget.
bool ThirdFocusTree::WouldCloseCycle(int64_t nodeId, int64_t startParent) const
{
    int64_t cursor = startParent;
    for (size_t steps = 0; steps <= ThirdAccessibilityTraversal::MAX_DEPTH; ++steps) {
        if (cursor == nodeId) {
            return true;
        }
        if (cursor == RootMarker()) {
            return false;
        }
        const auto it = parentOf_.find(cursor);
        if (it == parentOf_.end()) {
            // The chain ends at a dropped or unresolved node.
            return false;
        }
        cursor = it->second;
    }
    // Deeper than the depth budget; the reachability pass handles it.
    return false;
}

// Classification helper: does the parent chain of nodeId loop?
bool ThirdFocusTree::IsChainCyclic(int64_t nodeId) const
{
    std::unordered_set<int64_t> walked;
    int64_t cursor = nodeId;
    for (size_t steps = 0; steps <= ThirdAccessibilityTraversal::MAX_DEPTH; ++steps) {
        if (!walked.insert(cursor).second) {
            return true;
        }
        const auto it = parentOf_.find(cursor);
        if (it == parentOf_.end() || it->second == RootMarker()) {
            return false;
        }
        cursor = it->second;
    }
    return false;
}

} // OHOS::Ace::Framework

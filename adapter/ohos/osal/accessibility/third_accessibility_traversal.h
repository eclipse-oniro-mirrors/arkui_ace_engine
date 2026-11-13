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
#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_ACCESSIBILITY_THIRD_ACCESSIBILITY_TRAVERSAL_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_ACCESSIBILITY_THIRD_ACCESSIBILITY_TRAVERSAL_H

#include <cstdint>
#include <limits>
#include <list>
#include <map>
#include <utility>
#include <string>
#include <unordered_set>

#include "base/log/log_wrapper.h"

namespace OHOS::Accessibility {
class AccessibilityElementInfo;
}

namespace OHOS::Ace::Framework {
class JsThirdProviderInteractionOperation;
// Request-local limits; a repeated edge also consumes work. No normal-path logging.
class ThirdAccessibilityTraversal {
public:
    static constexpr size_t MAX_NODES = 10000;
    static constexpr size_t MAX_DEPTH = 64;
    static constexpr size_t MAX_EDGES = 40000;
    static constexpr size_t MAX_BYTES = 10 * 1024 * 1024;
    enum Issue : uint32_t {
        REPEATED_ID = 1 << 0,
        INVALID_ID = 1 << 1,
        QUERY_FAILED = 1 << 2,
        ID_MISMATCH = 1 << 3,
        MULTIPLE_ROOTS = 1 << 4,
        WRONG_PARENT = 1 << 5,
        LIMIT_EXCEEDED = 1 << 6,
        EXTRA_RESULTS = 1 << 7,
        DEPTH_LIMIT = 1 << 8,
        PAYLOAD_LIMIT = 1 << 9,
        DUPLICATE_ID = 1 << 10,
        CYCLE = 1 << 11,
        DUPLICATE_CHILD = 1 << 12,
        SHARED_CHILD = 1 << 13,
        // Focus-tree assembly findings (reported by the ThirdFocusTree assembler).
        // parentId missing, repaired from the unique claimer.
        MISSING_PARENT = 1 << 14,
        // Declared parent unclaimed, adopted in receive order.
        UNCLAIMED_CHILD = 1 << 15,
        // Both missing-declaration forms merged in one answer.
        MIXED_SOURCES = 1 << 16,
        // parentId disagrees with childNodeIds, child side kept.
        PARENT_CONFLICT = 1 << 17,
        // Multiple claimers, self-declared parent kept.
        MULTI_PARENT_SELF = 1 << 18,
        // Multiple claimers, first claimer kept.
        MULTI_PARENT_FIRST = 1 << 19,
        // Self/invalid parent declaration ignored.
        SELF_PARENT = 1 << 20,
        // Isolated node dropped.
        ORPHAN_NODE = 1 << 21,
        // Referenced record missing, fetched per node.
        LIST_HOLE = 1 << 22,
        // No root marker in a non-empty answer.
        NO_ROOT = 1 << 23,
        // Node would close a cycle, dropped.
        WOULD_CYCLE = 1 << 24,
        // Node unreachable from the root, dropped.
        UNREACHABLE = 1 << 25,
    };

    // Each issue type logs at most this many immediate ERROR lines; further occurrences
    // only feed the per-issue counters reused by the destructor summary line.
    static constexpr size_t MAX_ANOMALY_LOGS_PER_ISSUE = 5;

    explicit ThirdAccessibilityTraversal(const char* operation) : operation_(operation) {}
    ~ThirdAccessibilityTraversal()
    {
        if (!issues_) {
            return;
        }
        const std::pair<Issue, const char*> names[] = {
            { REPEATED_ID, "repeated-id/cycle/duplicate-child/shared-child" }, { INVALID_ID, "invalid-id" },
            { QUERY_FAILED, "query-failed" }, { ID_MISMATCH, "id-mismatch" },
            { MULTIPLE_ROOTS, "multiple-roots" }, { WRONG_PARENT, "wrong-parent" },
            { LIMIT_EXCEEDED, "limit-exceeded" }, { EXTRA_RESULTS, "extra-results" },
            { DEPTH_LIMIT, "depth-limit" }, { PAYLOAD_LIMIT, "payload-limit" },
            { DUPLICATE_ID, "duplicate-id" }, { CYCLE, "cycle" },
            { DUPLICATE_CHILD, "duplicate-child" }, { SHARED_CHILD, "shared-child" },
            { MISSING_PARENT, "missing-parent" }, { UNCLAIMED_CHILD, "unclaimed-child" },
            { MIXED_SOURCES, "mixed-sources" }, { PARENT_CONFLICT, "parent-conflict" },
            { MULTI_PARENT_SELF, "multi-parent-self" }, { MULTI_PARENT_FIRST, "multi-parent-first" },
            { SELF_PARENT, "self-parent" }, { ORPHAN_NODE, "orphan-node" },
            { LIST_HOLE, "list-hole" }, { NO_ROOT, "no-root" },
            { WOULD_CYCLE, "would-cycle" }, { UNREACHABLE, "unreachable" },
        };
        std::string reasons;
        for (const auto& [issue, name] : names) {
            if (issues_ & issue) {
                const auto count = issueCounts_.find(issue);
                reasons += std::string(name) + "=" +
                    std::to_string(count == issueCounts_.end() ? 0 : count->second) + " ";
            }
        }
        if (issues_ & MULTIPLE_ROOTS) {
            TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
                "Third accessibility %{public}s: %{public}s firstBadId=%{public}lld nodes=%{public}zu "
                "edges=%{public}zu; only the first root is used", operation_, reasons.c_str(),
                static_cast<long long>(firstBadId_), visited_.size(), edges_);
        } else {
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "Third accessibility %{public}s: %{public}s firstBadId=%{public}lld nodes=%{public}zu "
                "edges=%{public}zu", operation_, reasons.c_str(), static_cast<long long>(firstBadId_),
                visited_.size(), edges_);
        }
    }

    void Record(Issue issue, int64_t id)
    {
        if (!issues_) {
            firstBadId_ = id;
        }
        issues_ |= issue;
        ++issueCounts_[issue];
    }

    // Reports an assembly anomaly at the point of detection: one immediate rate-limited ERROR
    // line plus the per-issue counter reused by the destructor summary. Normal paths never call
    // this, so a healthy answer still produces zero logs.
    void ReportAnomaly(Issue issue, int64_t id, const char* scenario)
    {
        Record(issue, id);
        size_t& logged = anomalyLogs_[issue];
        if (logged < MAX_ANOMALY_LOGS_PER_ISSUE) {
            ++logged;
            TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
                "Third accessibility %{public}s: %{public}s, id=%{public}lld", operation_,
                scenario != nullptr ? scenario : "", static_cast<long long>(id));
        }
    }

    // Immediate rate-limited ERROR line for an issue that Visit or ConsumeBytes already
    // counted: logs the detection point without a second count, so the summary line keeps
    // one count per occurrence.
    void LogAnomaly(Issue issue, int64_t id, const char* scenario)
    {
        size_t& logged = anomalyLogs_[issue];
        if (logged < MAX_ANOMALY_LOGS_PER_ISSUE) {
            ++logged;
            TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
                "Third-party %{public}s: %{public}s, id=%{public}lld", operation_,
                scenario != nullptr ? scenario : "", static_cast<long long>(id));
        }
    }

    bool Visit(int64_t id, size_t depth)
    {
        if (Exhausted()) {
            return false;
        }
        if (++edges_ > MAX_EDGES) {
            Record(LIMIT_EXCEEDED, id);
            return false;
        }
        if (depth > MAX_DEPTH) {
            Record(DEPTH_LIMIT, id);
            return false;
        }
        if (id < 0 || id > std::numeric_limits<int32_t>::max()) {
            Record(INVALID_ID, id);
            return false;
        }
        if (visited_.count(id)) {
            Record(REPEATED_ID, id);
            return false;
        }
        if (visited_.size() >= MAX_NODES) {
            Record(LIMIT_EXCEEDED, id);
            return false;
        }
        visited_.insert(id);
        return true;
    }

    bool ConsumeBytes(size_t bytes, int64_t id)
    {
        if (bytes > MAX_BYTES - bytes_) {
            Record(PAYLOAD_LIMIT, id);
            return false;
        }
        bytes_ += bytes;
        return true;
    }

    bool FindNodeInfos(JsThirdProviderInteractionOperation& operation, int64_t elementId,
        std::list<Accessibility::AccessibilityElementInfo>& infos, bool ignoreHostOffset = false);

    size_t RemainingBytes() const { return MAX_BYTES - bytes_; }

    bool Exhausted() const { return (issues_ & LIMIT_EXCEEDED) != 0; }
    bool HasIssues() const { return issues_ != 0; }
    bool HasIssue(Issue issue) const { return (issues_ & issue) != 0; }
    bool EdgeBudgetBlown() const { return edges_ > MAX_EDGES; }

private:
    const char* operation_;
    std::unordered_set<int64_t> visited_;
    size_t edges_ = 0;
    size_t bytes_ = 0;
    uint32_t issues_ = 0;
    int64_t firstBadId_ = -1;
    std::map<uint32_t, size_t> issueCounts_;
    std::map<uint32_t, size_t> anomalyLogs_;
};
} // namespace OHOS::Ace::Framework
#endif

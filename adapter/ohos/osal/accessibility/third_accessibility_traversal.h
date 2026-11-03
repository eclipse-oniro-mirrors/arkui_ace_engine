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
    };

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
        };
        std::string reasons;
        for (const auto& [issue, name] : names) {
            if (issues_ & issue) {
                reasons += std::string(name) + " ";
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

private:
    const char* operation_;
    std::unordered_set<int64_t> visited_;
    size_t edges_ = 0;
    size_t bytes_ = 0;
    uint32_t issues_ = 0;
    int64_t firstBadId_ = -1;
};
} // namespace OHOS::Ace::Framework
#endif

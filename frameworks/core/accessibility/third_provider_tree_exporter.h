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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_THIRD_PROVIDER_TREE_EXPORTER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_THIRD_PROVIDER_TREE_EXPORTER_H

#include <cstddef>
#include <memory>

#include "base/json/json_util.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"

struct ArkUI_AccessibilityProvider;

namespace OHOS::Ace::NG {
class FrameNode;
} // namespace OHOS::Ace::NG

// JSON key under which the third-party provider subtree is attached to the host node.
inline constexpr char THIRD_PROVIDER_JSON_CHILDREN[] = "$children-third";

enum class ThirdProviderExportStatus {
    SUCCESS,
    NOT_REGISTERED,
    QUERY_FAILED,
    INVALID_DATA,
    NODE_LIMIT,
    SIZE_LIMIT,
    COLLECTION_FAILED,
};

struct ThirdProviderTreeExportBudget {
    size_t nodes = 0;
    size_t bytes = 0;
};

struct ThirdProviderTreeExportResult {
    ThirdProviderExportStatus status = ThirdProviderExportStatus::COLLECTION_FAILED;
    std::shared_ptr<OHOS::Ace::JsonValue> nodes;
};

// Queries a registered ArkUI_AccessibilityProvider for the full accessibility
// subtree (PREFETCH_RECURSIVE_CHILDREN) and converts the flat element list into
// a hierarchical JsonValue tree.
//
// Returns nullptr when the provider is null, unregistered, returns an error,
// yields no elements, or exceeds the 10000-node / 10 MiB serialized-tree limits.
// Oversized trees are rejected as a whole before the caller attaches them.
// The caller owns the returned JsonValue.
class ACE_FORCE_EXPORT ThirdProviderTreeExporter {
public:
    static std::shared_ptr<OHOS::Ace::JsonValue> ExportToInspectorJson(
        ArkUI_AccessibilityProvider* provider,
        const OHOS::Ace::RefPtr<OHOS::Ace::NG::FrameNode>& host);

    // Independent withCustomContent channel consumed by InspectorTreeRequestContext.
    static ThirdProviderTreeExportResult ExportFlatToInspectorJson(ArkUI_AccessibilityProvider* provider,
        const OHOS::Ace::RefPtr<OHOS::Ace::NG::FrameNode>& host, ThirdProviderTreeExportBudget& budget);
    static ThirdProviderExportStatus AccumulateInspectorTree(
        const OHOS::Ace::JsonValue& tree, ThirdProviderTreeExportBudget& budget);
};

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_THIRD_PROVIDER_TREE_EXPORTER_H

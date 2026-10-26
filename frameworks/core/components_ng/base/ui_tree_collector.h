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
#ifndef FOUNDATION_ACE_UI_TREE_COLLECTOR_H
#define FOUNDATION_ACE_UI_TREE_COLLECTOR_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "interfaces/inner_api/ui_session/param_config.h"

#include "base/json/json_util.h"
#include "base/memory/ace_type.h"

struct ArkUITreeOptions;
struct ArkUITreeCompletion;

namespace OHOS::Ace {
namespace NG {
class UINode;
}

struct TreeNodeVisibility {
    bool pruned = false;
    std::optional<double> opacity;
};

struct TreeCollectionState {
    int32_t error = 0;
    uint32_t nodes = 0;
    size_t compactBytes = 0;
    // Each standalone traversal retains its root, even when pure-layout descendants are promoted.
    const NG::UINode* traversalRoot = nullptr;
    // Only populated by VISIBLE collection; nodes remain alive during the read-only request.
    std::unordered_map<const NG::UINode*, TreeNodeVisibility> visibility;
    void Fail(int32_t e)
    {
        if (error == 0) {
            error = e;
        }
    }
    int32_t GetError() const
    {
        return error;
    }
};

namespace NG {
class PipelineContext;
class UINode;

struct UITreeRequestState {
    bool pending = false;
    bool closing = false;
    uint64_t sequence = 0;
    uint64_t requestId = 0;
    int32_t treeType = 0;
    int32_t format = 0;
    bool filterPureLayout = false;
    bool interaction = false;
    bool accessibility = false;
    bool visual = false;
    bool functional = false;
    std::function<void(uint64_t requestId, int32_t code, const char* data, uint32_t size)> deliver;
};

bool ShouldPruneNode(
    const RefPtr<UINode>& node, bool visible, const ParamConfig& config, const TreeNodeVisibility* parent = nullptr);
// Public collection retains boundary hosts without expanding their external subtrees.
bool IsHostOnlyNode(const UINode& node, const ParamConfig& config);
bool IsPureLayoutNode(const RefPtr<UINode>& node, const ParamConfig& config);
void RecordTreeNode(const RefPtr<UINode>& node, const ParamConfig& config);
bool AppendTreeChild(
    JsonValue& array, std::shared_ptr<JsonValue>&& child, const RefPtr<UINode>& node, const ParamConfig& config);
void TreeCheck(bool success, TreeCollectionState* state);
void RecordTreeNodeBytes(const JsonValue& value, const ParamConfig& config, bool countNode = false);
void RecordTreeChildInsertion(const JsonValue& parent, const ParamConfig& config);

ACE_EXPORT int32_t CollectUITree(int32_t instanceId, const ArkUITreeOptions* options, char** data, uint32_t* size);
ACE_EXPORT int32_t SubmitUITree(
    int32_t instanceId, const ArkUITreeOptions* options, const ArkUITreeCompletion* completion, uint64_t* requestId);
ACE_EXPORT void CloseUITreeRequests(PipelineContext& pipeline);
int32_t SerializeUITree(const std::shared_ptr<JsonValue>& tree, bool pretty, char** data, uint32_t* size);
} // namespace NG
} // namespace OHOS::Ace
#endif

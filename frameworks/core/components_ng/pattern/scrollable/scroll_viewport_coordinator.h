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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_VIEWPORT_COORDINATOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_VIEWPORT_COORDINATOR_H

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace::NG {
// Pipeline-owned registry: one persistent task, weak targets, no callback per frame or ancestor.
class ScrollViewportCoordinator : public AceType {
    DECLARE_ACE_TYPE(ScrollViewportCoordinator, AceType);
public:
    void Register(const RefPtr<FrameNode>& node, std::function<void()>&& update)
    {
        CHECK_NULL_VOID(node);
        CHECK_NULL_VOID(update);
        targets_.insert_or_assign(node->GetId(), Target { node, std::move(update), ++generation_ });
    }

    void Unregister(int32_t id)
    {
        targets_.erase(id);
    }

    void Flush(PipelineContext* pipeline)
    {
        if (flushing_) {
            return;
        }
        flushing_ = true;
        // Callbacks may update the registry; finish collecting the snapshot before invoking them.
        std::vector<std::pair<int32_t, Target>> updates;
        for (auto iter = targets_.begin(); iter != targets_.end();) {
            auto node = iter->second.node.Upgrade();
            if (!node || node->GetContext() != pipeline) {
                iter = targets_.erase(iter);
                continue;
            }
            updates.emplace_back(node->GetDepth(), iter->second);
            ++iter;
        }
        // An ancestor can move an edge containing another target. Refresh it before descendants
        // consume its geometry, regardless of creation order. AddChild updates depth on reparenting.
        std::stable_sort(updates.begin(), updates.end(), [](const auto& left, const auto& right) {
            return left.first < right.first;
        });
        for (const auto& update : updates) {
            const auto& target = update.second;
            auto node = target.node.Upgrade();
            if (!node || node->GetContext() != pipeline) {
                continue;
            }
            auto current = targets_.find(node->GetId());
            if (current == targets_.end() || current->second.generation != target.generation) {
                continue;
            }
            target.update();
        }
        flushing_ = false;
    }

private:
    struct Target {
        WeakPtr<FrameNode> node;
        std::function<void()> update;
        uint64_t generation;
    };
    std::map<int32_t, Target> targets_;
    uint64_t generation_ = 0;
    bool flushing_ = false;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_SCROLL_VIEWPORT_COORDINATOR_H

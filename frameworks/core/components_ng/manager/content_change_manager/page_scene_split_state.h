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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_PAGE_SCENE_SPLIT_STATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_PAGE_SCENE_SPLIT_STATE_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "content_change_manager.h"

namespace OHOS::Ace::NG {
class UINode;
// Lazily allocated UISession split-tracking state, owned by ContentChangeManager.
// Created on split configuration and released on unregister, so the disabled
// path keeps a null state and does no tree/geometry work. Pure data: safe on
// every platform; the detector is only installed where UISession is wired.
struct ContentChangeManager::SplitTrackingState {
    SplitTrackingState();
    ~SplitTrackingState();
    // Internal operations require a live tracking state; manager entry points perform optional-state guards.
    void AddPageSceneSplitNode(const RefPtr<FrameNode>& node);
    void IndexPageSceneSplitNode(const RefPtr<FrameNode>& node);
    void UpdatePageSceneSplitTags(const std::set<std::string>& tags);
    void RefreshPageSceneSplitRoots(const std::vector<RefPtr<FrameNode>>& roots, std::set<int32_t> ids);
    void MarkPageSceneSplitNavigationDirty(const RefPtr<FrameNode>& node);
    void HandlePageSceneSplitNodeChanged(const RefPtr<FrameNode>& node, bool attached);
    void RemovePageSceneSplitAncestors(int32_t nodeId);
    void SeedPageSceneSplitSubtree(const RefPtr<UINode>& node, const std::set<std::string>& tags, bool root = false);
    void PrunePageSceneSplitRoots();
    void ErasePageSceneSplitNode(int32_t nodeId);
    void ErasePageSceneSplitNodes(const std::vector<int32_t>& nodeIds);
    bool HasRelevantPageSceneSplitNode(int32_t nodeId) const;
    bool IsTrackedSplitNavigation(const RefPtr<UINode>& node) const;
    void PrunePageSceneSplitTags(const std::set<std::string>& tags);
    void AppendPageSceneSplitBucket(const std::string& tag, std::vector<RefPtr<FrameNode>>& nodes);
    void MarkPageSceneSplitNodeDirty(const RefPtr<FrameNode>& node);
    void AttachPageSceneSplitNode(const RefPtr<FrameNode>& node);
    bool IsPageSceneSplitNavigationDependency(const FrameNode* node) const;
    bool IsInsidePageSceneSplitRoots(const RefPtr<UINode>& node) const;
    std::set<std::string> pendingTags;
    bool dirty = false;
    bool detecting = false;
    uint64_t seedVisits = 0;
    std::set<std::string> tags;
    std::unordered_set<std::string> trackedTags;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> roots;
    std::set<int32_t> rootIds;
    std::unordered_map<int32_t, uint32_t> ancestorIds;
    std::unordered_map<int32_t, std::vector<int32_t>> nodeAncestors;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> nodes;
    std::unordered_map<std::string, std::unordered_map<int32_t, WeakPtr<FrameNode>>> tagBuckets;
    std::unordered_map<int32_t, std::string> nodeTags;
    std::function<void()> detector;
};

struct DeferredPageSceneQuery {
    std::function<bool()> run;
    std::function<void()> cancel;
    ~DeferredPageSceneQuery()
    {
        if (cancel) {
            cancel();
        }
    }
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_PAGE_SCENE_SPLIT_STATE_H

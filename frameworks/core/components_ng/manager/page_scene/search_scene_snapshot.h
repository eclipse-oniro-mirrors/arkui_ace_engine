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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_SNAPSHOT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_SNAPSHOT_H

#include <string>
#include <vector>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {
class UINode;
class FrameNode;

struct SearchKeywordSource {
    std::string name;
    std::string value;
};

// Light-weight per-node feature extracted at the stable point (design.md
// "快照与匹配层"). Rect is in main-window coordinates after subwindow mapping.
struct SearchSceneNodeFeature {
    int32_t nodeId = -1;
    std::string inspectorId;
    std::string nodeType;
    RectF visibleRect;
    std::vector<SearchKeywordSource> keywordSources;
    bool swiperAutoPlay = false;
    // True when collected from an ArkUI subwindow; its event PageName becomes
    // `<mainPageName>+Dialog` or `<mainPageName>+Sheet`.
    bool isSubwindow = false;
};

// Light-weight snapshot built once at a ContentChange stable point and reused
// by all candidate rules (ADR-1).
struct SearchSceneSnapshot {
    // Event PageName: primary top page name (subwindow uses `<main>+Dialog` or `<main>+Sheet`).
    std::string pageName;
    // Rule page matching values, fetched by pageType (ADR-2).
    std::string navDestinationName;
    std::string pageUrl;
    std::vector<SearchSceneNodeFeature> nodes;
};

// One window's traversal root plus the offset to map its local window
// coordinates into main-window coordinates (0 for the main window).
struct SearchWindowContext {
    RefPtr<FrameNode> root;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool isSubwindow = false;
};

// Builds the light-weight snapshot once per stable point from main window and
// ArkUI subwindow roots (ADR-1). BuildLocal (max 10 layers) belongs to the
// click fallback path (TASK-4).
class ACE_FORCE_EXPORT SearchSceneSnapshotBuilder final {
public:
    SearchSceneSnapshot Build(const std::string& pageName, const std::string& navDestinationName,
        const std::string& pageUrl, const std::vector<SearchWindowContext>& contexts) const;
    // Local snapshot from the response control downward, at most maxDepth levels
    // (click fallback path, AC-7.4/R-14).
    SearchSceneSnapshot BuildLocal(const RefPtr<FrameNode>& responseNode, int32_t maxDepth) const;

private:
    std::string CollectNode(const SearchWindowContext& window, const RefPtr<UINode>& node,
        SearchSceneSnapshot& snapshot, int32_t remainingDepth = -1) const;
    bool BuildNodeFeature(const SearchWindowContext& window, const RefPtr<FrameNode>& frameNode,
        std::vector<SearchSceneNodeFeature>& nodes) const;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_SNAPSHOT_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SPLIT_SCENE_RECOGNIZER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SPLIT_SCENE_RECOGNIZER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry/ng/rect_t.h"

namespace OHOS::Ace::NG {

// Custom geometry is validated at the PageScene request boundary before recognition.
struct SplitSceneRuleSpec {
    std::vector<std::string> compNames;
    bool allowNavigation = true;
    double heightRatioGt = 0.8;
    double widthRatioLte = 0.5;
    double totalWidthRatioGt = 0.8;
    bool nonOverlapping = true;
};

struct SplitSceneCandidate {
    int32_t nodeId = -1;
    std::string compName;
    RectF rect;
    bool visible = true;
    bool active = true;
    bool focusable = false;
    bool navigation = false;
    bool navigationSplit = false;
    bool navBarVisible = true;
    bool fullyCovered = false;
};

struct SplitSceneMatchState {
    bool matched = false;
    std::string splitType;
    std::vector<int32_t> matchedNodeIds;
    uint32_t visitedCandidates = 0;
    uint32_t filteredCandidates = 0;
    uint32_t earlyExitStage = 0;
};

struct SplitSceneBatchStats {
    uint32_t rectClips = 0;
    uint32_t geometrySorts = 0;
    uint32_t recognitions = 0;
    uint32_t cacheHits = 0;
};

// Borrows an immutable snapshot for one UI detection/query batch only.
// The collector guarantees unique node IDs, including merged Navigation/custom candidates.
// Registration, deduplication and reporting state remain owned by the caller.
class SplitSceneBatch final {
public:
    SplitSceneBatch(const std::vector<SplitSceneCandidate>& candidates, const RectF& windowRect);
    ~SplitSceneBatch();
    SplitSceneMatchState Recognize(const SplitSceneRuleSpec& rule);
    const std::vector<SplitSceneCandidate>& GetCandidates() const;
    const RectF& GetClippedRect(size_t index);
    const SplitSceneBatchStats& GetStats() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class SplitSceneRecognizer final {
public:
    // Requires validated rules and a snapshot with unique node IDs, as for SplitSceneBatch.
    static SplitSceneMatchState Recognize(
        const SplitSceneRuleSpec& rule, const std::vector<SplitSceneCandidate>& candidates, const RectF& windowRect);

    // Shared by the UI collector so Navigation can stop before reading custom geometry.
    static bool IsValidWindow(const RectF& windowRect);
    static bool IsNavigationMatch(
        const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate, const RectF& windowRect);
    static bool IsCustomCandidate(const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate);
    static RectF ClipToWindow(const RectF& rect, const RectF& windowRect);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SPLIT_SCENE_RECOGNIZER_H

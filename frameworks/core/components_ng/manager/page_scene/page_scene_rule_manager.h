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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_PAGE_SCENE_RULE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_PAGE_SCENE_RULE_MANAGER_H

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/referenced.h"
#include "core/components_ng/dump_utils/dump_util.h"
#include "core/components_ng/manager/page_scene/search_scene_click_matcher.h"
#include "core/components_ng/manager/page_scene/search_scene_matcher.h"
#include "core/components_ng/manager/page_scene/search_scene_snapshot.h"

namespace OHOS::Ace {
class JsonValue;
}

namespace OHOS::Ace::NG {
class FrameNode;
class UINode;

struct PageSceneRectInfo {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct PageSceneNodeInfo {
    int32_t nodeId = -1;
    std::string nodeType;
    std::string text;
    std::string placeholder;
    std::string contentType;
    std::string inputType;
    std::optional<bool> enableAutoFill;
    bool focusable = false;
    PageSceneRectInfo rect;
};

struct PageSceneRule {
    std::string ruleId;
    std::string sceneType;
    bool enabled = true;
    bool onlyVisible = true;
    bool rectCulling = false;
    bool includeWeb = false;
    bool includeUIExtension = false;
    bool includeNodeIds = true;
    bool includeNodeTypes = true;
    bool includeRect = true;
    bool includeFocusable = true;
    bool includeText = false;
    bool reportOnRegister = true;
    bool deduplicate = true;
    int32_t minReportIntervalMs = 500;
    int32_t threshold = 2;
    std::set<std::string> nodeTypes;
};

struct PageSceneRuleSet {
    int32_t version = 1;
    std::string ruleSetId;
    std::string rawJson;
    std::string webRulesRaw;
    bool includeUnfocusableTextInput = false;
    bool arkuiEnabled = true;
    bool webEnabled = false;
    bool uiExtensionEnabled = false;
    std::vector<PageSceneRule> rules;
};

// Search scene (UISession PageScene 搜索功能识别) rule model. Field spellings
// and validation follow the frozen schema in spec.md "数据与事件契约".
struct SearchComponentFeature {
    std::string type;
    std::optional<float> maxWidth;
    std::optional<float> maxHeight;
    std::optional<float> maxRatio;
    std::optional<bool> autoPlay;
};

struct SearchSceneRule {
    std::string ruleId;
    std::string pageType;
    std::string pageName;
    std::vector<PageSceneRectInfo> regions;
    std::vector<std::string> keywords;
    std::vector<SearchComponentFeature> components;
    std::vector<std::string> inspectorIds;
};

struct SearchRuleSet {
    std::string ruleSetId;
    std::string rawJson;
    std::string registrationGeneration;
    std::vector<SearchSceneRule> rules;
};

struct PageSceneMatchResult {
    std::string ruleSetId;
    std::string ruleId;
    std::string sceneType;
    std::string eventName;
    std::string pageName;
    bool matched = false;
    int32_t matchedCount = 0;
    int32_t minReportIntervalMs = 500;
    bool deduplicate = true;
    std::vector<int32_t> nodeIds;
    std::string sceneJson;
};

struct PageSceneReportState {
    std::vector<int32_t> lastReportedNodeIds;
    std::chrono::steady_clock::time_point lastReportTime;
};

class PageSceneInputCountTracker {
public:
    // startNodes carries Inspector-style page and overlay traversal roots
    // resolved by DumpUtil. AtomicService structure markers are not
    // traversal roots.
    void Initialize(const PageSceneRuleSet& ruleSet, const PageSceneRule& rule,
        const DumpStartNodeSet& startNodes);
    void Reset();
    const std::vector<PageSceneNodeInfo>& GetVisibleInputNodes() const;

private:
    void CollectInputNodes(const PageSceneRuleSet& ruleSet, const PageSceneRule& rule, const RefPtr<UINode>& node);
    std::optional<PageSceneNodeInfo> BuildNodeInfo(
        const PageSceneRuleSet& ruleSet, const PageSceneRule& rule, const RefPtr<FrameNode>& node) const;

    std::vector<PageSceneNodeInfo> visibleInputNodes_;
    RectF pageViewportRect_;

    bool IsOpacityVisible(const RefPtr<FrameNode>& node) const;
};

class PageSceneRuleManager {
public:
    ~PageSceneRuleManager();

    int32_t RegisterRuleSet(int32_t processId, const std::string& ruleJson);
    int32_t UnregisterRuleSet(int32_t processId, const std::string& ruleSetId);
    int32_t BeginGetPageScene(int32_t processId, const std::string& ruleJsonOrRuleSetId, std::string& ruleJson);
    void CompleteGetPageScene(int32_t processId);
    // Legacy rule removal must preserve search rules registered by the same process.
    void ClearProcess(int32_t processId, bool clearSearchRules = true);

    std::vector<std::pair<int32_t, std::string>> GetActiveRuleJsons() const;
    std::optional<PageSceneMatchResult> MatchPageScene(int32_t processId, const std::string& ruleJson,
        const DumpStartNodeSet& startNodes, const std::string& pageName, bool forceReportUnmatched);
    bool ShouldReport(int32_t processId, const PageSceneMatchResult& result);
    bool IsTextInputNodeType(const std::string& nodeType) const;

    // Search scene: per-process dual ruleSet storage (pid -> ruleSetId -> ruleSet).
    int32_t RegisterSearchRuleSet(int32_t processId, const std::string& ruleJson);
    int32_t UnregisterSearchRuleSet(
        int32_t processId, const std::string& ruleSetId, const std::string& generation = "");
    std::optional<SearchRuleSet> GetSearchRuleSet(int32_t processId, const std::string& ruleSetId) const;
    bool HasRegisteredSearchRule(int32_t processId) const;

    // Matches a stable snapshot against all registered search ruleSets of a
    // process and updates the presence state machine (TASK-2).
    std::vector<SearchMatchEvidence> MatchSearchScene(
        int32_t processId, const SearchSceneSnapshot& snapshot) const;
    std::vector<SearchPresenceStateMachine::PresenceEvent> UpdateSearchPresence(int32_t processId,
        const SearchSceneSnapshot& snapshot, const std::vector<SearchMatchEvidence>& hits);
    std::optional<SearchPresenceStateMachine::PresenceEvent> ExitSearchPresence(
        int32_t processId, const std::string& pageName);
    // Emits false for every PRESENT page of the process except the current one
    // (route transition ordering, AC-6.4).
    std::vector<SearchPresenceStateMachine::PresenceEvent> ExitOtherSearchPresence(
        int32_t processId, const std::string& currentPageName);
    // Emits false for every PRESENT page across all registered processes
    // (app background, AC-6.3). Returns (processId, event) pairs for reporting.
    std::vector<std::pair<int32_t, SearchPresenceStateMachine::PresenceEvent>> ExitAllSearchPresence();

    // Search click chain (TASK-4): subscribe the in-repo observation points and
    // buffer candidates (deduped by operationId) until a stable consume.
    uint32_t RegisterSearchClickObserver(const std::weak_ptr<PageSceneRuleManager>& weakManager);
    void UnregisterSearchClickObserver(uint32_t handlerId);
    void AddPendingClickCandidate(const SearchClickCandidate& candidate);
    std::vector<SearchClickMatchResult> ConsumeSearchClickCandidates(const SearchClickPageContext& page);
    // Trigger independent consumption on stable-page pure clicks. Checks the
    // stability gate (via IsContentStableFunction) first; only calls
    // ConsumeSearchClickCandidates when stable.
    using IsContentStableFunction = std::function<bool()>;
    void SetIsContentStableFunction(IsContentStableFunction&& func);
    std::vector<SearchClickMatchResult> FlushPendingSearchClicksIfStable(const SearchClickPageContext& page);

private:
    void StopSearchClickObservationIfIdleLocked();
    void PrunePendingClickCandidatesLocked(int64_t nowMs);
    std::optional<PageSceneRuleSet> ParseRuleSet(const std::string& ruleJson) const;
    std::optional<PageSceneRule> ParseRule(const std::unique_ptr<JsonValue>& ruleJson) const;
    std::optional<SearchRuleSet> ParseSearchRuleSet(const std::string& ruleJson) const;
    std::optional<SearchSceneRule> ParseSearchRule(
        const std::unique_ptr<JsonValue>& ruleJson, std::set<std::string>& ruleIds) const;
    std::string BuildSceneJson(const PageSceneRuleSet& ruleSet, const PageSceneRule& rule,
        const std::string& pageName, const std::vector<PageSceneNodeInfo>& nodes, bool matched,
        const std::string& eventName) const;

    mutable std::mutex mutex_;
    std::unordered_map<int32_t, PageSceneRuleSet> registeredRuleSets_;
    std::unordered_map<int32_t, std::unordered_map<std::string, SearchRuleSet>> searchRuleSets_;
    SearchPresenceStateMachine presenceStateMachine_;
    mutable std::mutex presenceStateMutex_;
    uint32_t searchClickObserverId_ = 0;
    std::unordered_map<std::string, SearchClickCandidate> pendingClickCandidates_;
    // Keep a bounded history across consumes so DOWN and click report only once.
    std::unordered_map<std::string, int64_t> recentClickOperations_;
    std::set<int32_t> pendingGetProcesses_;
    std::unordered_map<std::string, PageSceneReportState> reportStates_;
    // Stability gate callback (injected by UIContentImpl, reusing
    // ContentChangeManager's IsScrolling/IsTransitioning/IsSwiperScrolling).
    IsContentStableFunction isContentStableFunc_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_PAGE_SCENE_RULE_MANAGER_H

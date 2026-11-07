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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CLICK_MATCHER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CLICK_MATCHER_H

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "core/components_ng/manager/page_scene/search_scene_snapshot.h"

namespace OHOS::Ace::NG {
class FrameNode;
struct SearchRuleSet;
struct SearchSceneRule;

// Produced at the in-repo onClick/onTouch observation points
// (ClickRecognizer::OnAccepted, TouchEventActuator::TriggerCallBacks) when a
// developer callback actually executed (AC-7.2: no callback -> no candidate).
// Decoupled from any external observer (2026-09-29).
struct SearchClickCandidate {
    int32_t nodeId = -1;
    std::string inspectorId;
    std::string nodeType;
    // Operation token for touch/click aggregation and dedup (AC-7.11/7.12).
    std::string operationId;
    // Monotonic hook-site timestamp used to expire retained candidates.
    int64_t operationTimestampMs = 0;
    // Weak reference to the response control, upgraded at stable consume time
    // for the <=10 layer fallback; the snapshot never holds it strongly.
    WeakPtr<FrameNode> responseNode;
};

using SearchClickHandler = std::function<void(const SearchClickCandidate&)>;

// Singleton fan-out publisher mounted at the two observation points. Multiple
// PageSceneRuleManager instances (per UIContent) register handlers; Publish
// notifies all, and each handler routes by its own registered ruleSets.
class ACE_FORCE_EXPORT SearchClickObserver final {
public:
    SearchClickObserver() = default;
    static SearchClickObserver& GetInstance();
    SearchClickObserver(const SearchClickObserver&) = delete;
    SearchClickObserver& operator=(const SearchClickObserver&) = delete;
    static std::string BuildOperationId(int64_t deviceId, int32_t pointerId, int64_t pressedTimeNs, int32_t nodeId);
    uint32_t AddHandler(const SearchClickHandler& handler);
    void RemoveHandler(uint32_t handlerId);
    bool HasHandlers() const;
    void Publish(const SearchClickCandidate& candidate) const;

private:
    mutable std::mutex mutex_;
    uint32_t nextHandlerId_ = 0;
    std::unordered_map<uint32_t, SearchClickHandler> handlers_;
};

// Rule page matching context, fetched independently by pageType (ADR-2).
struct SearchClickPageContext {
    std::string navDestinationName;
    std::string pageUrl;
};

// Per-rule click match. matchType is "inspectorId" | "region" | "keyword".
struct SearchClickMatchResult {
    std::string ruleSetId;
    std::string ruleId;
    std::string matchType;
    std::string matchedKeyword;
    std::string inspectorId;
    int32_t nodeId = -1;
    std::string nodeType;
    RectF rect;
    int32_t processId = -1;
    std::string operationId;
};

// Click matching: Inspector ID case-sensitive full exact match first; only when
// inspectorIds are unconfigured/empty or not matched, fall back from the
// response control downward at most maxDepth levels using "region OR keyword"
// (AC-7.3~7.10, R-13/R-14). ID hits bypass region/keyword but never bypass
// pageType/pageName, actual callback, dedup, stable gate or event merge.
class ACE_FORCE_EXPORT SearchClickMatcher final {
public:
    std::vector<SearchClickMatchResult> Match(const SearchClickCandidate& candidate,
        const SearchClickPageContext& page, const SearchRuleSet& ruleSet,
        const RefPtr<FrameNode>& responseNode, int32_t maxDepth = 10) const;

private:
    static bool MatchInspectorIdExact(const std::string& id, const std::vector<std::string>& inspectorIds);
    static bool IsRulePageMatched(const SearchClickPageContext& page, const SearchSceneRule& rule);
    SearchClickMatchResult MatchInspectorId(const SearchClickCandidate& candidate,
        const SearchRuleSet& ruleSet, const SearchSceneSnapshot& localSnapshot) const;
    std::optional<SearchClickMatchResult> MatchFallback(const SearchClickCandidate& candidate,
        const SearchRuleSet& ruleSet, const SearchSceneRule& rule, const SearchSceneSnapshot& localSnapshot) const;
    std::optional<SearchClickMatchResult> MatchKeywordInNode(
        const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const;
    std::optional<SearchClickMatchResult> MatchRegionInNode(
        const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CLICK_MATCHER_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_MATCHER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_MATCHER_H

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/geometry/ng/rect_t.h"
#include "core/components_ng/manager/page_scene/search_scene_snapshot.h"

namespace OHOS::Ace::NG {
struct PageSceneRectInfo;
struct SearchComponentFeature;
struct SearchRuleSet;
struct SearchSceneRule;

// Subwindow pageName suffixes. Shared so that the matcher, the presence
// state machine, and the event JSON builder reference the same constants
// instead of duplicating magic string literals.
// A subwindow rule's pageName is "<mainPageName>+Dialog" or "<mainPageName>+Sheet".
inline constexpr char SUBWINDOW_DIALOG_SUFFIX[] = "+Dialog";
inline constexpr char SUBWINDOW_SHEET_SUFFIX[] = "+Sheet";

inline bool HasSearchPageSuffix(const std::string& pageName, const std::string& suffix)
{
    return pageName.size() >= suffix.size() &&
        pageName.compare(pageName.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string BuildSearchSubwindowPageName(const std::string& pageName, const std::string& suffix)
{
    return pageName.empty() || HasSearchPageSuffix(pageName, suffix) ? pageName : pageName + suffix;
}

// Per-rule match evidence. evidenceType is "keyword" or "component"; for a
// keyword hit, source carries the keyword source name and matchedKeyword the
// matched keyword; for a component hit, source carries the component type.
struct SearchMatchEvidence {
    std::string ruleSetId;
    std::string ruleId;
    std::string evidenceType;
    std::string source;
    std::string matchedKeyword;
    int32_t nodeId = -1;
    std::string nodeType;
    RectF rect;
    // True when the evidence node belongs to an ArkUI subwindow (AC-4.5).
    bool isSubwindow = false;
    // Subwindow suffix extracted from the matching rule's pageName ("+Dialog"
    // or "+Sheet"); empty for main-window hits. Carried through the evidence
    // chain so the presence state machine and event JSON use the correct suffix
    // instead of a hardcoded one.
    std::string subwindowSuffix;
};

// Role that matches a single search snapshot against all rules of one ruleSet.
// Keyword condition and component features are OR; keywords scan stops at the
// first hit node (AC-2.9); same-type constraints are AND, different types OR.
class ACE_FORCE_EXPORT SearchSnapshotMatcher final {
public:
    std::vector<SearchMatchEvidence> Match(
        const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet) const;

private:
    static bool IsRulePageMatched(const SearchSceneSnapshot& snapshot, const SearchSceneRule& rule);
    static bool IsPointInRegion(const PageSceneRectInfo& region, const RectF& rect);
    static bool ContainsIgnoreCase(const std::string& text, const std::string& keyword);
    std::optional<SearchMatchEvidence> FindKeywordHit(
        const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const;
    std::optional<SearchMatchEvidence> FindKeywordHitInNode(
        const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule,
        const std::string& suffix) const;
    std::optional<SearchMatchEvidence> FindComponentHit(
        const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const;
    static bool IsComponentFeatureMatched(
        const SearchComponentFeature& feature, const SearchSceneNodeFeature& node);
};

// Presence state machine: first PRESENT reports one true; evidence changes with
// state unchanged do not repeat; feature vanish / route leave / background report
// one false (idempotent); re-appearance allows another true (AC-5, AC-6).
class ACE_FORCE_EXPORT SearchPresenceStateMachine final {
public:
    enum class PageState : uint8_t {
        UNKNOWN = 0,
        ABSENT,
        PRESENT,
    };

    struct PresenceEvent {
        bool isTrue = false;
        std::string pageName;
        // ruleSetId/ruleId pairs involved in the transition (exits on false).
        std::vector<std::pair<std::string, std::string>> ruleIds;
        // Full match evidence retained for the true/click event contract
        // (AC-8.1: evidenceType/source/matchedKeyword/nodeId/type/rect).
        std::vector<SearchMatchEvidence> hits;
        // True when any evidence node belongs to an ArkUI subwindow; event
        // PageName then becomes `<mainPageName>+Dialog` or `<mainPageName>+Sheet`
        // (AC-4.5). subwindowSuffix carries the actual suffix from the hits.
        bool anySubwindow = false;
        std::string subwindowSuffix;
    };

    // Returns an event only on a state transition; otherwise std::nullopt.
    std::optional<PresenceEvent> Update(const std::string& pageKey, const std::string& pageName,
        bool hasSearch, const std::vector<SearchMatchEvidence>& hits);
    // Route leave / background: emits one false if the page was PRESENT (AC-6.2/6.3).
    std::optional<PresenceEvent> Exit(const std::string& pageKey, const std::string& pageName);
    // Emits one false for every PRESENT page whose key starts with prefix but is
    // not keepKey; used for route transition ordering (A false before B true).
    std::vector<PresenceEvent> ExitAllExcept(const std::string& prefix, const std::string& keepKey);
    // Preserve the main page and its exact Dialog/Sheet states on same-page scans.
    std::vector<PresenceEvent> ExitOtherPages(const std::string& prefix, const std::string& pageName);
    // Clears states whose key starts with prefix (e.g. "<pid>:" on SA death).
    void ClearPrefix(const std::string& prefix);
    void ClearRuleSet(const std::string& processId, const std::string& ruleSetId);
    void Clear();
    PageState GetState(const std::string& pageKey) const;

private:
    static std::string ExtractPageName(const std::string& pageKey);
    static PresenceEvent MakeEvent(const std::string& pageKey, bool isTrue,
        const std::vector<SearchMatchEvidence>& hits);

    std::unordered_map<std::string, PageState> pageStates_;
    std::unordered_map<std::string, std::vector<SearchMatchEvidence>> pageActiveHits_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_MATCHER_H

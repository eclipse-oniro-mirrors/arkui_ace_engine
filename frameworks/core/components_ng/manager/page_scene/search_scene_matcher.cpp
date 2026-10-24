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

#include "core/components_ng/manager/page_scene/search_scene_matcher.h"

#include <algorithm>
#include <cctype>

#include "base/log/log_wrapper.h"
#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float ZERO_RECT_SIZE = 0.0f;
const char KEYWORD_EVIDENCE[] = "keyword";
const char COMPONENT_EVIDENCE[] = "component";
const char INSPECTOR_ID_SOURCE[] = "inspectorId";

// Matches when rule.pageName equals base or base+"+Dialog"/"+Sheet"
bool IsPageTargetMatched(const std::string& basePageName, const std::string& rulePageName)
{
    if (basePageName.empty() || rulePageName.empty()) {
        return false;
    }
    return rulePageName == basePageName ||
        rulePageName == basePageName + SUBWINDOW_DIALOG_SUFFIX ||
        rulePageName == basePageName + SUBWINDOW_SHEET_SUFFIX;
}

// Resolve the suffix relative to the actual page identity. A main page whose
// name contains (or ends in) +Dialog/+Sheet is still a main-window target.
std::string ExtractSubwindowSuffix(const SearchSceneSnapshot& snapshot, const SearchSceneRule& rule)
{
    const auto& base = rule.pageType == "Navigation" ? snapshot.navDestinationName : snapshot.pageUrl;
    if (rule.pageName == base + SUBWINDOW_DIALOG_SUFFIX) {
        return SUBWINDOW_DIALOG_SUFFIX;
    }
    if (rule.pageName == base + SUBWINDOW_SHEET_SUFFIX) {
        return SUBWINDOW_SHEET_SUFFIX;
    }
    return "";
}
} // namespace

bool SearchSnapshotMatcher::IsPointInRegion(const PageSceneRectInfo& region, const RectF& rect)
{
    float centerX = rect.GetX() + rect.Width() / 2.0f;
    float centerY = rect.GetY() + rect.Height() / 2.0f;
    // Region containment includes the boundary (AC-3.10).
    return centerX >= region.x && centerX <= region.x + region.width &&
           centerY >= region.y && centerY <= region.y + region.height;
}

bool SearchSnapshotMatcher::ContainsIgnoreCase(const std::string& text, const std::string& keyword)
{
    if (keyword.empty() || text.size() < keyword.size()) {
        return false;
    }
    auto toLower = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return value;
    };
    return toLower(text).find(toLower(keyword)) != std::string::npos;
}

bool SearchSnapshotMatcher::IsRulePageMatched(const SearchSceneSnapshot& snapshot, const SearchSceneRule& rule)
{
    // Support +Dialog/+Sheet subwindow pageName matching
    if (rule.pageType == "Navigation") {
        return !snapshot.navDestinationName.empty() &&
            IsPageTargetMatched(snapshot.navDestinationName, rule.pageName);
    }
    if (rule.pageType == "Router") {
        return !snapshot.pageUrl.empty() &&
            IsPageTargetMatched(snapshot.pageUrl, rule.pageName);
    }
    return false;
}

bool SearchSnapshotMatcher::IsComponentFeatureMatched(
    const SearchComponentFeature& feature, const SearchSceneNodeFeature& node)
{
    if (feature.type != node.nodeType) {
        return false;
    }
    // A component with zero final visible height never matches (AC-3.8).
    if (node.visibleRect.Height() <= ZERO_RECT_SIZE) {
        return false;
    }
    if (feature.maxWidth.has_value() && node.visibleRect.Width() > feature.maxWidth.value()) {
        return false;
    }
    if (feature.maxHeight.has_value() && node.visibleRect.Height() > feature.maxHeight.value()) {
        return false;
    }
    if (feature.maxRatio.has_value()) {
        float ratio = node.visibleRect.Width() / node.visibleRect.Height();
        // maxRatio includes equality (AC-3.9).
        if (ratio > feature.maxRatio.value()) {
            return false;
        }
    }
    if (feature.autoPlay.has_value() && feature.autoPlay.value() && !node.swiperAutoPlay) {
        return false;
    }
    return true;
}

std::optional<SearchMatchEvidence> SearchSnapshotMatcher::FindKeywordHit(
    const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const
{
    if (rule.keywords.empty()) {
        return std::nullopt;
    }
    // Subwindow rules only match isSubwindow nodes; normal rules only match main-window nodes
    const auto suffix = ExtractSubwindowSuffix(snapshot, rule);
    bool requireSubwindow = !suffix.empty();
    for (const auto& node : snapshot.nodes) {
        if (requireSubwindow != node.isSubwindow) {
            continue;
        }
        if (node.visibleRect.Height() <= ZERO_RECT_SIZE || node.visibleRect.Width() <= ZERO_RECT_SIZE) {
            continue;
        }
        bool inAnyRegion = std::any_of(rule.regions.begin(), rule.regions.end(),
            [&node](const auto& region) { return IsPointInRegion(region, node.visibleRect); });
        if (!inAnyRegion) {
            continue;
        }
        auto evidence = FindKeywordHitInNode(node, ruleSet, rule, suffix);
        if (evidence.has_value()) {
            return evidence;
        }
    }
    return std::nullopt;
}

std::optional<SearchMatchEvidence> SearchSnapshotMatcher::FindKeywordHitInNode(
    const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule,
    const std::string& suffix) const
{
    // inspectorId participates as a keyword source via substring fuzzy match.
    for (const auto& keyword : rule.keywords) {
        for (const auto& source : node.keywordSources) {
            if (ContainsIgnoreCase(source.value, keyword)) {
                SearchMatchEvidence evidence;
                evidence.ruleSetId = ruleSet.ruleSetId;
                evidence.ruleId = rule.ruleId;
                evidence.evidenceType = KEYWORD_EVIDENCE;
                evidence.source = source.name;
                evidence.matchedKeyword = keyword;
                evidence.nodeId = node.nodeId;
                evidence.nodeType = node.nodeType;
                evidence.rect = node.visibleRect;
                evidence.isSubwindow = node.isSubwindow;
                evidence.subwindowSuffix = suffix;
                return evidence;
            }
        }
        if (!node.inspectorId.empty() && ContainsIgnoreCase(node.inspectorId, keyword)) {
            SearchMatchEvidence evidence;
            evidence.ruleSetId = ruleSet.ruleSetId;
            evidence.ruleId = rule.ruleId;
            evidence.evidenceType = KEYWORD_EVIDENCE;
            evidence.source = INSPECTOR_ID_SOURCE;
            evidence.matchedKeyword = keyword;
            evidence.nodeId = node.nodeId;
            evidence.nodeType = node.nodeType;
            evidence.rect = node.visibleRect;
            evidence.isSubwindow = node.isSubwindow;
            evidence.subwindowSuffix = suffix;
            return evidence;
        }
    }
    return std::nullopt;
}

std::optional<SearchMatchEvidence> SearchSnapshotMatcher::FindComponentHit(
    const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const
{
    if (rule.components.empty()) {
        return std::nullopt;
    }
    // Subwindow rules only match isSubwindow nodes; normal rules only match main-window nodes
    const auto suffix = ExtractSubwindowSuffix(snapshot, rule);
    bool requireSubwindow = !suffix.empty();
    for (const auto& node : snapshot.nodes) {
        if (requireSubwindow != node.isSubwindow) {
            continue;
        }
        if (node.visibleRect.Height() <= ZERO_RECT_SIZE || node.visibleRect.Width() <= ZERO_RECT_SIZE) {
            continue;
        }
        bool inAnyRegion = std::any_of(rule.regions.begin(), rule.regions.end(),
            [&node](const auto& region) { return IsPointInRegion(region, node.visibleRect); });
        if (!inAnyRegion) {
            continue;
        }
        for (const auto& feature : rule.components) {
            if (IsComponentFeatureMatched(feature, node)) {
                SearchMatchEvidence evidence;
                evidence.ruleSetId = ruleSet.ruleSetId;
                evidence.ruleId = rule.ruleId;
                evidence.evidenceType = COMPONENT_EVIDENCE;
                evidence.source = feature.type;
                evidence.nodeId = node.nodeId;
                evidence.nodeType = node.nodeType;
                evidence.rect = node.visibleRect;
                evidence.isSubwindow = node.isSubwindow;
                evidence.subwindowSuffix = suffix;
                return evidence;
            }
        }
    }
    return std::nullopt;
}

std::vector<SearchMatchEvidence> SearchSnapshotMatcher::Match(
    const SearchSceneSnapshot& snapshot, const SearchRuleSet& ruleSet) const
{
    std::vector<SearchMatchEvidence> hits;
    for (const auto& rule : ruleSet.rules) {
        if (!IsRulePageMatched(snapshot, rule)) {
            continue;
        }
        // Keyword OR component; keyword early-stops at the first hit node.
        auto keywordHit = FindKeywordHit(snapshot, ruleSet, rule);
        if (keywordHit.has_value()) {
            hits.emplace_back(keywordHit.value());
            continue;
        }
        auto componentHit = FindComponentHit(snapshot, ruleSet, rule);
        if (componentHit.has_value()) {
            hits.emplace_back(componentHit.value());
        }
    }
    return hits;
}

std::string SearchPresenceStateMachine::ExtractPageName(const std::string& pageKey)
{
    auto pos = pageKey.find(':');
    return pos == std::string::npos ? pageKey : pageKey.substr(pos + 1);
}

SearchPresenceStateMachine::PresenceEvent SearchPresenceStateMachine::MakeEvent(
    const std::string& pageKey, bool isTrue, const std::vector<SearchMatchEvidence>& hits)
{
    PresenceEvent event;
    event.isTrue = isTrue;
    event.pageName = ExtractPageName(pageKey);
    event.hits = hits;
    for (const auto& hit : hits) {
        event.ruleIds.emplace_back(hit.ruleSetId, hit.ruleId);
        if (hit.isSubwindow) {
            event.anySubwindow = true;
            if (event.subwindowSuffix.empty()) {
                event.subwindowSuffix = hit.subwindowSuffix;
            }
        }
    }
    return event;
}

std::optional<SearchPresenceStateMachine::PresenceEvent> SearchPresenceStateMachine::Update(
    const std::string& pageKey, const std::string& pageName, bool hasSearch,
    const std::vector<SearchMatchEvidence>& hits)
{
    if (pageKey.empty() || pageName.empty()) {
        return std::nullopt;
    }
    auto iter = pageStates_.find(pageKey);
    PageState current = (iter != pageStates_.end()) ? iter->second : PageState::UNKNOWN;
    if (hasSearch) {
        if (current != PageState::PRESENT) {
            pageStates_[pageKey] = PageState::PRESENT;
            pageActiveHits_[pageKey] = hits;
            auto event = MakeEvent(pageKey, true, hits);
            event.pageName = pageName;
            return event;
        }
        // Evidence may change while the state stays PRESENT: no repeat (AC-5.2).
        return std::nullopt;
    }
    // hasSearch == false.
    pageStates_[pageKey] = PageState::ABSENT;
    if (current == PageState::PRESENT) {
        auto event = MakeEvent(pageKey, false, pageActiveHits_[pageKey]);
        event.pageName = pageName;
        pageActiveHits_.erase(pageKey);
        return event;
    }
    return std::nullopt;
}

std::optional<SearchPresenceStateMachine::PresenceEvent> SearchPresenceStateMachine::Exit(
    const std::string& pageKey, const std::string& pageName)
{
    if (pageKey.empty()) {
        return std::nullopt;
    }
    auto iter = pageStates_.find(pageKey);
    if (iter == pageStates_.end() || iter->second != PageState::PRESENT) {
        return std::nullopt;
    }
    iter->second = PageState::ABSENT;
    auto event = MakeEvent(pageKey, false, pageActiveHits_[pageKey]);
    event.pageName = pageName;
    pageActiveHits_.erase(pageKey);
    return event;
}

std::vector<SearchPresenceStateMachine::PresenceEvent> SearchPresenceStateMachine::ExitAllExcept(
    const std::string& prefix, const std::string& keepKey)
{
    std::vector<PresenceEvent> events;
    if (prefix.empty()) {
        return events;
    }
    for (auto iter = pageStates_.begin(); iter != pageStates_.end();) {
        if (iter->first.compare(0, prefix.size(), prefix) == 0 &&
            iter->first != keepKey && iter->second == PageState::PRESENT) {
            iter->second = PageState::ABSENT;
            auto event = MakeEvent(iter->first, false, pageActiveHits_[iter->first]);
            pageActiveHits_.erase(iter->first);
            events.emplace_back(std::move(event));
            ++iter;
        } else {
            ++iter;
        }
    }
    return events;
}

std::vector<SearchPresenceStateMachine::PresenceEvent> SearchPresenceStateMachine::ExitOtherPages(
    const std::string& prefix, const std::string& pageName)
{
    std::vector<PresenceEvent> events;
    if (prefix.empty() || pageName.empty()) {
        return events;
    }
    const auto mainKey = prefix + pageName;
    const auto dialogKey = prefix + BuildSearchSubwindowPageName(pageName, SUBWINDOW_DIALOG_SUFFIX);
    const auto sheetKey = prefix + BuildSearchSubwindowPageName(pageName, SUBWINDOW_SHEET_SUFFIX);
    for (const auto& [key, state] : pageStates_) {
        if (key.compare(0, prefix.size(), prefix) != 0 || key == mainKey || key == dialogKey || key == sheetKey) {
            continue;
        }
        auto event = Exit(key, ExtractPageName(key));
        if (event) {
            events.emplace_back(std::move(*event));
        }
    }
    return events;
}

void SearchPresenceStateMachine::ClearPrefix(const std::string& prefix)
{
    if (prefix.empty()) {
        return;
    }
    for (auto iter = pageStates_.begin(); iter != pageStates_.end();) {
        if (iter->first.compare(0, prefix.size(), prefix) == 0) {
            pageActiveHits_.erase(iter->first);
            iter = pageStates_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void SearchPresenceStateMachine::ClearRuleSet(const std::string& processId, const std::string& ruleSetId)
{
    const auto prefix = processId + ":";
    for (auto stateIter = pageStates_.begin(); stateIter != pageStates_.end();) {
        if (stateIter->first.compare(0, prefix.size(), prefix) != 0) {
            ++stateIter;
            continue;
        }
        auto hitsIter = pageActiveHits_.find(stateIter->first);
        if (hitsIter != pageActiveHits_.end()) {
            auto& hits = hitsIter->second;
            hits.erase(std::remove_if(hits.begin(), hits.end(),
                [&ruleSetId](const auto& hit) { return hit.ruleSetId == ruleSetId; }), hits.end());
            if (!hits.empty()) {
                ++stateIter;
                continue;
            }
            pageActiveHits_.erase(hitsIter);
        }
        stateIter = pageStates_.erase(stateIter);
    }
}

void SearchPresenceStateMachine::Clear()
{
    pageStates_.clear();
    pageActiveHits_.clear();
}

SearchPresenceStateMachine::PageState SearchPresenceStateMachine::GetState(
    const std::string& pageKey) const
{
    auto iter = pageStates_.find(pageKey);
    return (iter != pageStates_.end()) ? iter->second : PageState::UNKNOWN;
}

} // namespace OHOS::Ace::NG

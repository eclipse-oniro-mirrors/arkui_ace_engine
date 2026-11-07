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

#include "core/components_ng/manager/page_scene/search_scene_click_matcher.h"

#include <algorithm>
#include <cctype>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DEFAULT_CLICK_FALLBACK_MAX_DEPTH = 10;
constexpr float ZERO_RECT_SIZE = 0.0f;
const char CLICK_MATCH_INSPECTOR_ID[] = "inspectorId";
const char CLICK_MATCH_REGION[] = "region";
const char CLICK_MATCH_KEYWORD[] = "keyword";
} // namespace

SearchClickObserver& SearchClickObserver::GetInstance()
{
    static SearchClickObserver instance;
    return instance;
}

uint32_t SearchClickObserver::AddHandler(const SearchClickHandler& handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t handlerId = ++nextHandlerId_;
    handlers_[handlerId] = handler;
    return handlerId;
}

std::string SearchClickObserver::BuildOperationId(
    int64_t deviceId, int32_t pointerId, int64_t pressedTimeNs, int32_t nodeId)
{
    // DOWN and its accepted click share the input's press time. Two separate
    // presses on the same control must remain distinct, even when very close.
    return std::to_string(deviceId) + ":" + std::to_string(pointerId) + ":" +
        std::to_string(pressedTimeNs) + ":" + std::to_string(nodeId);
}

void SearchClickObserver::RemoveHandler(uint32_t handlerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.erase(handlerId);
}

bool SearchClickObserver::HasHandlers() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !handlers_.empty();
}

void SearchClickObserver::Publish(const SearchClickCandidate& candidate) const
{
    std::unordered_map<uint32_t, SearchClickHandler> snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = handlers_;
    }
    for (const auto& [handlerId, handler] : snapshot) {
        if (handler) {
            handler(candidate);
        }
    }
}

namespace {
bool ContainsIgnoreCase(const std::string& text, const std::string& keyword)
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

bool IsPointInRegion(const PageSceneRectInfo& region, const RectF& rect)
{
    float centerX = rect.GetX() + rect.Width() / 2.0f;
    float centerY = rect.GetY() + rect.Height() / 2.0f;
    return centerX >= region.x && centerX <= region.x + region.width &&
           centerY >= region.y && centerY <= region.y + region.height;
}
} // namespace

bool SearchClickMatcher::MatchInspectorIdExact(
    const std::string& id, const std::vector<std::string>& inspectorIds)
{
    if (id.empty() || inspectorIds.empty()) {
        return false;
    }
    return std::find(inspectorIds.begin(), inspectorIds.end(), id) != inspectorIds.end();
}

bool SearchClickMatcher::IsRulePageMatched(const SearchClickPageContext& page, const SearchSceneRule& rule)
{
    if (rule.pageType == "Navigation") {
        return !page.navDestinationName.empty() && rule.pageName == page.navDestinationName;
    }
    if (rule.pageType == "Router") {
        return !page.pageUrl.empty() && rule.pageName == page.pageUrl;
    }
    return false;
}

std::optional<SearchClickMatchResult> SearchClickMatcher::MatchFallback(const SearchClickCandidate& candidate,
    const SearchRuleSet& ruleSet, const SearchSceneRule& rule, const SearchSceneSnapshot& localSnapshot) const
{
    if (localSnapshot.nodes.empty()) {
        return std::nullopt;
    }
    // Keyword first (AC-7.4 "命中即上报"); case-insensitive substring against
    // node keyword sources. Inspector ID fuzzy is not used here: click ID path
    // is exact-only.
    for (const auto& node : localSnapshot.nodes) {
        auto keywordResult = MatchKeywordInNode(node, ruleSet, rule);
        if (keywordResult.has_value()) {
            return keywordResult;
        }
    }
    // Region fallback: node center inside any rule region.
    for (const auto& node : localSnapshot.nodes) {
        if (node.visibleRect.Height() <= ZERO_RECT_SIZE || node.visibleRect.Width() <= ZERO_RECT_SIZE) {
            continue;
        }
        auto regionResult = MatchRegionInNode(node, ruleSet, rule);
        if (regionResult.has_value()) {
            return regionResult;
        }
    }
    return std::nullopt;
}

std::optional<SearchClickMatchResult> SearchClickMatcher::MatchKeywordInNode(
    const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const
{
    for (const auto& keyword : rule.keywords) {
        for (const auto& source : node.keywordSources) {
            if (ContainsIgnoreCase(source.value, keyword)) {
                SearchClickMatchResult result;
                result.ruleSetId = ruleSet.ruleSetId;
                result.ruleId = rule.ruleId;
                result.matchType = CLICK_MATCH_KEYWORD;
                result.matchedKeyword = keyword;
                result.inspectorId = node.inspectorId;
                result.nodeId = node.nodeId;
                result.nodeType = node.nodeType;
                result.rect = node.visibleRect;
                return result;
            }
        }
    }
    return std::nullopt;
}

std::optional<SearchClickMatchResult> SearchClickMatcher::MatchRegionInNode(
    const SearchSceneNodeFeature& node, const SearchRuleSet& ruleSet, const SearchSceneRule& rule) const
{
    for (const auto& region : rule.regions) {
        if (IsPointInRegion(region, node.visibleRect)) {
            SearchClickMatchResult result;
            result.ruleSetId = ruleSet.ruleSetId;
            result.ruleId = rule.ruleId;
            result.matchType = CLICK_MATCH_REGION;
            result.inspectorId = node.inspectorId;
            result.nodeId = node.nodeId;
            result.nodeType = node.nodeType;
            result.rect = node.visibleRect;
            return result;
        }
    }
    return std::nullopt;
}

SearchClickMatchResult SearchClickMatcher::MatchInspectorId(
    const SearchClickCandidate& candidate, const SearchRuleSet& ruleSet,
    const SearchSceneSnapshot& localSnapshot) const
{
    SearchClickMatchResult result;
    result.ruleSetId = ruleSet.ruleSetId;
    result.matchType = CLICK_MATCH_INSPECTOR_ID;
    result.inspectorId = candidate.inspectorId;
    result.nodeId = candidate.nodeId;
    result.nodeType = candidate.nodeType;
    for (const auto& node : localSnapshot.nodes) {
        if (node.nodeId == candidate.nodeId) {
            result.rect = node.visibleRect;
            break;
        }
    }
    return result;
}

std::vector<SearchClickMatchResult> SearchClickMatcher::Match(
    const SearchClickCandidate& candidate, const SearchClickPageContext& page,
    const SearchRuleSet& ruleSet, const RefPtr<FrameNode>& responseNode, int32_t maxDepth) const
{
    std::vector<SearchClickMatchResult> results;
    if (maxDepth <= 0) {
        maxDepth = DEFAULT_CLICK_FALLBACK_MAX_DEPTH;
    }
    SearchSceneSnapshotBuilder builder;
    std::optional<SearchSceneSnapshot> localSnapshot;
    for (const auto& rule : ruleSet.rules) {
        if (!IsRulePageMatched(page, rule)) {
            continue;
        }
        // Inspector ID exact first: response control's own ID, case-sensitive
        // full string equality (AC-7.3). Not nodeId/prefix/substring/ancestors.
        if (!candidate.inspectorId.empty() &&
            MatchInspectorIdExact(candidate.inspectorId, rule.inspectorIds)) {
            if (!localSnapshot.has_value()) {
                localSnapshot = builder.BuildLocal(responseNode, maxDepth);
            }
            auto result = MatchInspectorId(candidate, ruleSet, localSnapshot.value());
            result.ruleId = rule.ruleId;
            results.emplace_back(result);
            continue;
        }
        // Fallback only when inspectorIds unconfigured/empty or not matched
        // (AC-7.4); local snapshot is built lazily once (max 10 layers).
        if (!localSnapshot.has_value()) {
            localSnapshot = builder.BuildLocal(responseNode, maxDepth);
        }
        auto fallback = MatchFallback(candidate, ruleSet, rule, localSnapshot.value());
        if (fallback.has_value()) {
            results.emplace_back(fallback.value());
        }
    }
    return results;
}

} // namespace OHOS::Ace::NG

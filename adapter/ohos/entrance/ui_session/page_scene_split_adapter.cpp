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

#include "adapter/ohos/entrance/ui_session/page_scene_split_adapter.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "adapter/ohos/entrance/ui_session/ui_session_manager_ohos.h"
#include "base/json/json_util.h"
#include "base/log/ace_trace.h"
#include "base/thread/task_executor.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::PageSceneSplitAdapter {
namespace {
constexpr uint32_t MIN_SPLIT_CHECK_NODE_COUNT = 2;
constexpr double SPLIT_OPAQUE_VALUE = 1.0;

bool IsSplitFrameVisible(const RefPtr<NG::FrameNode>& frame)
{
    if (!frame->IsOnMainTree() || !frame->IsVisibleAndActive()) {
        return false;
    }
    auto render = frame->GetRenderContext();
    return !render || !NearZero(render->GetOpacityValue(SPLIT_OPAQUE_VALUE));
}

bool IsSplitNodeVisible(const RefPtr<NG::FrameNode>& node, bool& covered)
{
    RefPtr<NG::UINode> current = node;
    while (current) {
        auto frame = AceType::DynamicCast<NG::FrameNode>(current);
        if (frame) {
            if (!IsSplitFrameVisible(frame)) {
                return false;
            }
            auto page = frame->GetPattern<NG::PagePattern>();
            covered = covered || (page && page->GetIsModalCovered());
        }
        current = current->GetParent();
    }
    return true;
}

RefPtr<NG::FrameNode> GetSplitNavBar(const RefPtr<NG::FrameNode>& node)
{
    auto property = node->GetLayoutProperty<NG::NavigationLayoutProperty>();
    if (!property || property->GetHideNavBarValue(false)) {
        return nullptr;
    }
    auto navigation = AceType::DynamicCast<NG::NavigationGroupNode>(node);
    if (!navigation) {
        return nullptr;
    }
    return AceType::DynamicCast<NG::FrameNode>(navigation->GetNavBarOrHomeDestinationNode());
}

bool IsSplitNavBarVisible(const RefPtr<NG::FrameNode>& node, const NG::RectF& window)
{
    auto bar = GetSplitNavBar(node);
    if (!bar) {
        return false;
    }
    bool covered = false;
    if (!IsSplitNodeVisible(bar, covered) || covered) {
        return false;
    }
    const auto rect = bar->GetTransformRectRelativeToWindowOnlyVisible(true);
    return NG::SplitSceneRecognizer::IsValidWindow(NG::SplitSceneRecognizer::ClipToWindow(rect, window));
}

NG::SplitSceneCandidate BuildSplitSceneCandidate(const RefPtr<NG::FrameNode>& node, const NG::RectF& window,
    bool navigationOnly, std::unordered_set<int32_t>& geometryNodes)
{
    NG::SplitSceneCandidate result;
    result.nodeId = node->GetId();
    result.compName = node->GetTag();
    result.active = node->IsActive();
    auto navigation = node->GetPattern<NG::NavigationPattern>();
    result.navigation = navigation != nullptr;
    result.navigationSplit = navigation && navigation->GetNavigationMode() == NG::NavigationMode::SPLIT;
    if (navigationOnly && !result.navigationSplit) {
        return result;
    }
    result.visible = IsSplitNodeVisible(node, result.fullyCovered);
    if (!result.visible || result.fullyCovered) {
        return result;
    }
    result.navBarVisible = result.navigationSplit && IsSplitNavBarVisible(node, window);
    if (navigationOnly && !result.navBarVisible) {
        return result;
    }
    result.rect = node->GetTransformRectRelativeToWindowOnlyVisible(true);
    geometryNodes.insert(result.nodeId);
    return result;
}

void PostSplitSceneEvents(int32_t pid, const std::vector<NG::PageSceneMatchResult>& changes,
    const std::shared_ptr<NG::PageSceneRuleManager>& matcher, const RefPtr<TaskExecutor>& executor)
{
    if (changes.empty() || !executor) {
        return;
    }
    auto root = JsonUtil::Create(true);
    auto events = JsonUtil::CreateArray(true);
    for (const auto& change : changes) {
        events->Put(matcher->BuildSplitSceneJson(change));
    }
    root->Put("events", events);
    auto json = root->ToString();
    // Keep the detection snapshot and UI runner order, regardless of later registration changes.
    executor->PostTask([pid, json]() { UiSessionManager::GetInstance()->ReportPageSceneEvent(pid, json, false); },
        TaskExecutor::TaskType::UI, "UiSessionSplitPageSceneReport", PriorityType::IDLE);
}

bool CollectSplitNavigation(const RefPtr<NG::ContentChangeManager>& content, const NG::RectF& window,
    std::vector<NG::SplitSceneCandidate>& candidates, std::unordered_set<int32_t>& geometryNodes)
{
    NG::SplitSceneRuleSpec navigationRule;
    const auto nodes = content->GetPageSceneSplitNodes({ V2::NAVIGATION_VIEW_ETS_TAG });
    for (const auto& node : nodes) {
        candidates.emplace_back(BuildSplitSceneCandidate(node, window, true, geometryNodes));
        if (NG::SplitSceneRecognizer::IsNavigationMatch(navigationRule, candidates.back(), window)) {
            return true;
        }
    }
    return false;
}

void CollectCustomSplitCandidates(const RefPtr<NG::ContentChangeManager>& content, const std::set<std::string>& tags,
    const NG::RectF& window, std::vector<NG::SplitSceneCandidate>& candidates,
    std::unordered_set<int32_t>& geometryNodes)
{
    const auto nodes = content->GetPageSceneSplitNodes(tags);
    if (nodes.size() < MIN_SPLIT_CHECK_NODE_COUNT) {
        return;
    }
    std::unordered_map<int32_t, size_t> collected;
    for (size_t index = 0; index < candidates.size(); ++index) {
        collected.emplace(candidates[index].nodeId, index);
    }
    for (const auto& node : nodes) {
        if (geometryNodes.count(node->GetId()) != 0) {
            continue;
        }
        auto candidate = BuildSplitSceneCandidate(node, window, false, geometryNodes);
        auto found = collected.find(node->GetId());
        if (found == collected.end()) {
            candidates.emplace_back(std::move(candidate));
        } else {
            // A STACK/hidden-NavBar node can still participate in a custom rule.
            candidates[found->second] = std::move(candidate);
        }
    }
}

std::vector<NG::SplitSceneCandidate> CollectSplitCandidates(
    const RefPtr<NG::ContentChangeManager>& content, const NG::RectF& window, const NG::CompiledSplitRuleSet& compiled)
{
    std::vector<NG::SplitSceneCandidate> candidates;
    if (!NG::SplitSceneRecognizer::IsValidWindow(window)) {
        return candidates;
    }
    bool navigationMatched = false;
    std::unordered_set<int32_t> geometryNodes;
    if (compiled.trackNavigation) {
        navigationMatched = CollectSplitNavigation(content, window, candidates, geometryNodes);
    }
    const auto& tags = navigationMatched ? compiled.navigationDisabledTags : compiled.customTags;
    CollectCustomSplitCandidates(content, tags, window, candidates, geometryNodes);
    return candidates;
}

std::vector<RefPtr<NG::FrameNode>> GetSplitPageRoots(const RefPtr<NG::PipelineContext>& pipeline)
{
    auto stage = pipeline->GetStageManager();
    auto roots =
        NG::PageSceneRuleManager::CollectSplitStartNodes(pipeline->GetRootElement(), pipeline->GetContainerModalNode(),
            stage ? stage->GetLastPage() : pipeline->GetRootElement(), pipeline->GetOverlayManager());
    auto seedRoots = std::move(roots.pageStartNodes);
    seedRoots.insert(seedRoots.end(), roots.overlayNodes.begin(), roots.overlayNodes.end());
    return seedRoots;
}

void SeedSplitPage(const RefPtr<NG::ContentChangeManager>& content, const RefPtr<NG::PipelineContext>& pipeline)
{
    if (content->IsPageSceneSplitTracking()) {
        content->SeedPageSceneSplitNodes(GetSplitPageRoots(pipeline));
    }
}

bool NeedsSplitQueryDetection(const NG::CompiledSplitRuleSet& rules, const RefPtr<NG::PipelineContext>& pipeline)
{
    const auto hasSplit = rules.trackNavigation || !rules.customTags.empty();
    return hasSplit && NG::ContentChangeManager::IsPageSceneSplitWindowSupported(AceType::RawPtr(pipeline));
}

std::vector<NG::SplitSceneCandidate> CollectQuerySplitCandidates(
    const std::shared_ptr<const NG::CompiledSplitRuleSet>& compiled, const RefPtr<NG::PipelineContext>& pipeline)
{
    if (!NeedsSplitQueryDetection(*compiled, pipeline)) {
        return {};
    }
    auto content = AceType::MakeRefPtr<NG::ContentChangeManager>();
    content->ConfigurePageSceneSplitTracking(compiled->customTags, compiled->trackNavigation);
    SeedSplitPage(content, pipeline);
    return CollectSplitCandidates(content, pipeline->GetRootRect(), *compiled);
}

struct SplitQueryRequest {
    int32_t processId;
    std::string generation;
    std::shared_ptr<const NG::CompiledSplitRuleSet> rules;
    std::weak_ptr<NG::PageSceneRuleManager> matcher;
    WeakPtr<NG::PipelineContext> pipeline;
};

void CancelSplitQuery(const SplitQueryRequest& request)
{
    auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
    if (session) {
        session->CancelSplitPageSceneQuery(request.processId, request.generation);
    }
}

bool IsSplitQueryLayoutStable(const RefPtr<NG::PipelineContext>& pipeline)
{
    return !pipeline->IsLayouting() && pipeline->IsDirtyNodesEmpty() && pipeline->IsDirtyLayoutNodesEmpty() &&
           pipeline->IsDirtyPropertyNodesEmpty();
}

bool IsSplitQueryGeometryReady(const RefPtr<NG::PipelineContext>& pipeline)
{
    return IsSplitQueryLayoutStable(pipeline) && NG::SplitSceneRecognizer::IsValidWindow(pipeline->GetRootRect());
}

bool IsReadySplitQueryWindow(const RefPtr<NG::PipelineContext>& pipeline)
{
    const auto& content = pipeline->GetContentChangeManager();
    return pipeline->GetOnShow() && IsSplitQueryGeometryReady(pipeline) && (!content || !content->IsContentChanging());
}

void SendSplitQueryResult(
    const SplitQueryRequest& request, const std::string& json, const RefPtr<TaskExecutor>& executor)
{
    if (!executor) {
        CancelSplitQuery(request);
        return;
    }
    const auto posted = executor->PostTask(
        [pid = request.processId, generation = request.generation, json]() {
            auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
            if (session) {
                session->ReportSplitPageSceneQueryResult(pid, json, generation);
            }
        },
        TaskExecutor::TaskType::BACKGROUND, "UiSessionSplitPageSceneQuery");
    if (!posted) {
        CancelSplitQuery(request);
    }
}

bool TryRunReadySplitQuery(const SplitQueryRequest& request, const std::shared_ptr<NG::PageSceneRuleManager>& matcher,
    const RefPtr<NG::PipelineContext>& pipeline)
{
    // Existing untracked motion is outside the query stability guarantee; do not rescan for it.
    if (NeedsSplitQueryDetection(*request.rules, pipeline) && !IsReadySplitQueryWindow(pipeline)) {
        return false;
    }
    const auto includeSplit = NG::ContentChangeManager::IsPageSceneSplitWindowSupported(AceType::RawPtr(pipeline));
    auto candidates = CollectQuerySplitCandidates(request.rules, pipeline);
    auto json = matcher->QueryCompiledSplitPageScenes(
        *request.rules, candidates, pipeline->GetRootRect(), pipeline->GetCurrentPageName(), includeSplit);
    SendSplitQueryResult(request, json, pipeline->GetTaskExecutor());
    return true;
}

bool RunSplitQuery(const SplitQueryRequest& request)
{
    auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
    if (!session->IsSplitPageSceneQueryCurrent(request.processId, request.generation)) {
        return true;
    }
    auto matcher = request.matcher.lock();
    auto pipeline = request.pipeline.Upgrade();
    if (!matcher || !pipeline) {
        CancelSplitQuery(request);
        return true;
    }
    return TryRunReadySplitQuery(request, matcher, pipeline);
}

void DetectSplitRegistration(const std::shared_ptr<NG::PageSceneRuleManager>& matcher,
    const RefPtr<NG::PipelineContext>& pipeline, const NG::CompiledSplitRegistration& registration)
{
    auto content = pipeline->GetContentChangeManager();
    SeedSplitPage(content, pipeline);
    const auto window = pipeline->GetRootRect();
    const auto pageName = pipeline->GetCurrentPageName();
    auto candidates = CollectSplitCandidates(content, window, *registration.rules);
    NG::SplitSceneBatch snapshot(candidates, window);
    auto changes = matcher->DetectCompiledSplitChanges(registration, snapshot, pageName);
    PostSplitSceneEvents(registration.processId, changes, matcher, pipeline->GetTaskExecutor());
}

void DetectStableSplitScenes(
    const std::shared_ptr<NG::PageSceneRuleManager>& matcher, const RefPtr<NG::PipelineContext>& pipeline)
{
    ACE_SCOPED_TRACE("PageSceneSplitDetect");
    if (!NG::ContentChangeManager::IsPageSceneSplitWindowSupported(AceType::RawPtr(pipeline))) {
        return;
    }
    auto content = pipeline->GetContentChangeManager();
    if (!content || !pipeline->GetOnShow()) {
        return;
    }
    auto registration = matcher->GetCompiledSplitRegistration();
    if (!registration) {
        content->ConfigurePageSceneSplitTracking({}, false);
        return;
    }
    if (!IsSplitQueryGeometryReady(pipeline)) {
        content->MarkPageSceneSplitDirty();
        return;
    }
    DetectSplitRegistration(matcher, pipeline, *registration);
}

bool ApplyCurrentSplitConfiguration(int32_t processId, const std::shared_ptr<NG::PageSceneRuleManager>& matcher)
{
    // Resolve ownership when the UI task runs, so queued configurations cannot
    // restore an unregistered rule or replace a newer registration.
    auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
    const auto rules = session->GetSplitPageSceneRulesForDetection(processId);
    return matcher->UpdateSplitRegistration(processId, rules.empty() ? "" : rules.front());
}

void ConfigureReceivedSplitBatch(
    const std::shared_ptr<NG::PageSceneRuleManager>& matcher, const RefPtr<NG::PipelineContext>& pipeline)
{
    if (!NG::ContentChangeManager::IsPageSceneSplitWindowSupported(AceType::RawPtr(pipeline))) {
        return;
    }
    auto content = pipeline->GetContentChangeManager();
    if (!content) {
        return;
    }
    const auto registration = matcher->GetCompiledSplitRegistration();
    if (!registration) {
        content->ConfigurePageSceneSplitTracking({}, false);
        return;
    }
    content->ConfigurePageSceneSplitTracking(registration->rules->customTags, registration->rules->trackNavigation);
    if (!content->IsPageSceneSplitTracking()) {
        return;
    }
    content->SetPageSceneSplitDetector([weakMatcher = std::weak_ptr<NG::PageSceneRuleManager>(matcher),
                                           weakPipeline = WeakPtr<NG::PipelineContext>(pipeline)]() {
        auto matcher = weakMatcher.lock();
        auto pipeline = weakPipeline.Upgrade();
        if (matcher && pipeline) {
            DetectStableSplitScenes(matcher, pipeline);
        }
    });
    content->MarkPageSceneSplitDirty();
    content->FlushPageSceneSplitDetection();
}
} // namespace

void QuerySplitPageScenes(int32_t pid, const std::string& ruleJson, const std::string& generation,
    const std::shared_ptr<NG::PageSceneRuleManager>& matcher, const RefPtr<NG::PipelineContext>& pipeline)
{
    auto compiled = matcher->CompileSplitRuleSet(ruleJson);
    if (!compiled) {
        auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
        session->CancelSplitPageSceneQuery(pid, generation);
        return;
    }
    SplitQueryRequest request { pid, generation, compiled, matcher, pipeline };
    if (RunSplitQuery(request)) {
        return;
    }
    auto content = pipeline->GetContentChangeManager();
    if (!content) {
        CancelSplitQuery(request);
        return;
    }
    content->DeferPageSceneQuery(
        pid, [request]() { return RunSplitQuery(request); }, [request]() { CancelSplitQuery(request); });
}

bool HandleConfigurationChanged(
    int32_t processId, const std::unique_ptr<JsonValue>& root, const std::shared_ptr<NG::PageSceneRuleManager>& matcher)
{
    if (!root || !root->IsObject() || !root->GetBool("_splitBatch", false) || root->Contains("version") ||
        root->Contains("ruleSetId") || root->Contains("rules")) {
        return false;
    }
    auto pipeline = NG::PipelineContext::GetCurrentContextSafely();
    if (pipeline && ApplyCurrentSplitConfiguration(processId, matcher)) {
        ConfigureReceivedSplitBatch(matcher, pipeline);
    }
    return true;
}

void CompletePageSceneQuery(int32_t processId, const std::string& ruleJson)
{
    auto root = JsonUtil::ParseJsonString(ruleJson);
    auto generation = root->GetString("_splitQueryGeneration");
    if (generation.empty()) {
        UiSessionManager::GetInstance()->CompleteGetPageScene(processId);
        return;
    }
    auto session = static_cast<UiSessionManagerOhos*>(UiSessionManager::GetInstance());
    session->CancelSplitPageSceneQuery(processId, generation);
}

} // namespace OHOS::Ace::PageSceneSplitAdapter

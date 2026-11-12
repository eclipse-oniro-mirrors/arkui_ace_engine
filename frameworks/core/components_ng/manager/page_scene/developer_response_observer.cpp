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

#include "core/components_ng/manager/page_scene/developer_response_observer.h"

#include <map>

#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "base/thread/task_executor.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/stage/page_info.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "interfaces/inner_api/ui_session/ui_session_manager.h"

namespace OHOS::Ace::NG {
namespace {
constexpr char RESPONSE_EVENT_TYPE[] = "RESPONSE_EVENT";
constexpr char RESPONSE_SCENE_TYPE[] = "RESPONSE";
constexpr char RESPONSE_EXIT_EVENT[] = "RESPONSE_EXIT";
constexpr char PAGE_SCENE_RULE_SETS[] = "ruleSets";
constexpr char PAGE_SCENE_RULE_SET_ID[] = "ruleSetId";
constexpr char PAGE_SCENE_SCENE[] = "scene";
} // namespace

PageSceneResponseObserver* PageSceneResponseObserver::GetInstance()
{
    static PageSceneResponseObserver instance;
    return &instance;
}

void PageSceneResponseObserver::SetActive(bool active)
{
    auto publisher = DeveloperResponsePublisher::GetInstance();
    std::lock_guard<std::mutex> lock(mutex_);
    active_ = active;
    if (!active) {
        ops_.clear();
        pendingOps_.clear();
        nodeOpIndex_.clear();
    }
    publisher->SetObserver(active ? this : nullptr);
}

void PageSceneResponseObserver::OnDeveloperResponse(const DeveloperResponseCandidate& candidate)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!active_) {
        return;
    }
    if (candidate.callbackType == DeveloperResponseCallbackType::TOUCH &&
        candidate.touchType == TouchType::DOWN) {
        ops_[candidate.operationToken] = candidate;
        return;
    }
    // UP/CANCEL/CLICK settles the operation. A click is a completed operation even
    // though no DOWN candidate was recorded for it.
    ops_.erase(candidate.operationToken);
    AppendSettledCandidate(candidate);
}

void PageSceneResponseObserver::AppendSettledCandidate(const DeveloperResponseCandidate& candidate)
{
    for (const auto& pending : pendingOps_) {
        if (pending.operationToken == candidate.operationToken) {
            return;
        }
    }
    auto node = candidate.node.Upgrade();
    CHECK_NULL_VOID(node);
    auto nodeId = node->GetId();
    if (nodeOpIndex_.find(nodeId) != nodeOpIndex_.end()) {
        return;
    }
    nodeOpIndex_[nodeId] = pendingOps_.size();
    pendingOps_.emplace_back(candidate);
}

void PageSceneResponseObserver::OnContentStable()
{
    std::vector<DeveloperResponseCandidate> candidates;
    // The stable gate also runs when no candidate settled: a scene that was reported
    // earlier may have left the screen and still owes one exit event.
    if (CollectSettledCandidates(candidates) && !candidates.empty()) {
        auto ruleJsons = UiSessionManager::GetInstance()->GetActivePageSceneRuleJsons();
        if (!ruleJsons.empty()) {
            std::map<int32_t, std::vector<std::pair<std::string, std::string>>> payloadsByPid;
            std::map<int32_t, RefPtr<TaskExecutor>> executorByPid;
            MatchSettledCandidates(candidates, ruleJsons, payloadsByPid, executorByPid);
            DispatchResponseReports(payloadsByPid, executorByPid);
        }
    }
    DispatchResponseExits();
}

bool PageSceneResponseObserver::CollectSettledCandidates(std::vector<DeveloperResponseCandidate>& candidates)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!active_) {
        return false;
    }
    candidates.swap(pendingOps_);
    nodeOpIndex_.clear();
    return !candidates.empty();
}

void PageSceneResponseObserver::MatchSettledCandidates(
    const std::vector<DeveloperResponseCandidate>& candidates,
    const std::vector<std::pair<int32_t, std::string>>& ruleJsons,
    std::map<int32_t, std::vector<std::pair<std::string, std::string>>>& payloadsByPid,
    std::map<int32_t, RefPtr<TaskExecutor>>& executorByPid)
{
    for (const auto& candidate : candidates) {
        auto node = candidate.node.Upgrade();
        CHECK_NULL_CONTINUE(node);
        auto pipeline = node->GetContextRefPtr();
        CHECK_NULL_CONTINUE(pipeline);
        auto startNodes = pipeline->GetDumpStartNodes();
        std::string routerPageName;
        std::string navDestinationName;
        auto pageInfo = pipeline->GetLastPageInfo();
        if (pageInfo) {
            routerPageName = pageInfo->GetPageUrl();
            navDestinationName = pipeline->GetNavDestinationPageName(pageInfo);
        }
        for (const auto& [pid, ruleJson] : ruleJsons) {
            auto match = ruleManager_.MatchPageSceneResponse(
                pid, ruleJson, node, startNodes, routerPageName, navDestinationName);
            if (!match.has_value()) {
                continue;
            }
            if (executorByPid.find(pid) == executorByPid.end()) {
                executorByPid[pid] = pipeline->GetTaskExecutor();
            }
            payloadsByPid[pid].emplace_back(match->ruleSetId, match->sceneJson);
        }
    }
}

void PageSceneResponseObserver::DispatchResponseReports(
    const std::map<int32_t, std::vector<std::pair<std::string, std::string>>>& payloadsByPid,
    const std::map<int32_t, RefPtr<TaskExecutor>>& executorByPid)
{
    for (const auto& entry : payloadsByPid) {
        auto pid = entry.first;
        const auto& payloads = entry.second;
        auto executorIter = executorByPid.find(pid);
        if (executorIter == executorByPid.end() || !executorIter->second) {
            continue;
        }
        auto envelope = JsonUtil::Create(true);
        envelope->Put("eventType", RESPONSE_EVENT_TYPE);
        auto ruleSetsJson = JsonUtil::CreateArray(true);
        for (const auto& [ruleSetId, sceneJson] : payloads) {
            auto ruleSetJson = JsonUtil::Create(true);
            ruleSetJson->Put(PAGE_SCENE_RULE_SET_ID, ruleSetId.c_str());
            ruleSetJson->Put(PAGE_SCENE_SCENE, sceneJson.c_str());
            ruleSetsJson->Put(ruleSetJson);
        }
        envelope->Put(PAGE_SCENE_RULE_SETS, ruleSetsJson);
        auto payload = envelope->ToString();
        executorIter->second->PostTask(
            [pid, payload]() {
                UiSessionManager::GetInstance()->ReportPageSceneEvent(pid, payload, false);
            },
            TaskExecutor::TaskType::BACKGROUND, "UiSessionPageSceneReport");
    }
}

namespace {
// One RESPONSE_EVENT envelope carrying the exit scene of every ruleSet that left.
std::string BuildResponseExitPayload(const std::map<std::string, std::vector<std::string>>& ruleSets)
{
    auto envelope = JsonUtil::Create(true);
    envelope->Put("eventType", RESPONSE_EVENT_TYPE);
    auto ruleSetsJson = JsonUtil::CreateArray(true);
    for (const auto& [ruleSetId, ruleIds] : ruleSets) {
        auto sceneJson = JsonUtil::Create(true);
        sceneJson->Put("ruleSetId", ruleSetId.c_str());
        sceneJson->Put("sceneType", RESPONSE_SCENE_TYPE);
        sceneJson->Put("eventName", RESPONSE_EXIT_EVENT);
        sceneJson->Put("matched", false);
        auto exitsJson = JsonUtil::CreateArray(true);
        for (const auto& ruleId : ruleIds) {
            auto exitJson = JsonUtil::Create(true);
            exitJson->Put("ruleId", ruleId.c_str());
            exitsJson->Put(exitJson);
        }
        // "exits" (not "hits") keeps an exit unambiguous for consumers that count hits.
        sceneJson->Put("exits", exitsJson);
        auto ruleSetJson = JsonUtil::Create(true);
        ruleSetJson->Put(PAGE_SCENE_RULE_SET_ID, ruleSetId.c_str());
        ruleSetJson->Put(PAGE_SCENE_SCENE, sceneJson->ToString().c_str());
        ruleSetsJson->Put(ruleSetJson);
    }
    envelope->Put(PAGE_SCENE_RULE_SETS, ruleSetsJson);
    return envelope->ToString();
}
} // namespace

void PageSceneResponseObserver::DispatchResponseExits()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!active_) {
            return;
        }
    }
    auto exits = ruleManager_.CollectResponseExits();
    if (exits.empty()) {
        return;
    }
    // pid -> ruleSetId -> ruleIds; keep the owning pipeline to post on its executor.
    std::map<int32_t, std::map<std::string, std::vector<std::string>>> ruleIdsByPid;
    std::map<int32_t, RefPtr<TaskExecutor>> executorByPid;
    for (const auto& exit : exits) {
        ruleIdsByPid[exit.processId][exit.ruleSetId].emplace_back(exit.ruleId);
        if (executorByPid.find(exit.processId) != executorByPid.end()) {
            continue;
        }
        auto pipeline = exit.pipeline.Upgrade();
        if (pipeline) {
            executorByPid[exit.processId] = pipeline->GetTaskExecutor();
        }
    }
    for (const auto& entry : ruleIdsByPid) {
        const int32_t pid = entry.first;
        const auto& ruleSets = entry.second;
        auto executorIter = executorByPid.find(pid);
        if (executorIter == executorByPid.end() || !executorIter->second) {
            continue;
        }
        auto payload = BuildResponseExitPayload(ruleSets);
        executorIter->second->PostTask(
            [pid, payload]() {
                UiSessionManager::GetInstance()->ReportPageSceneEvent(pid, payload, false);
            },
            TaskExecutor::TaskType::BACKGROUND, "UiSessionPageSceneExitReport");
    }
}

int32_t PageSceneResponseObserver::RegisterRuleSet(int32_t processId, const std::string& ruleJson)
{
    return ruleManager_.RegisterResponseRuleSet(processId, ruleJson);
}

int32_t PageSceneResponseObserver::UnregisterRuleSet(int32_t processId, const std::string& ruleSetId)
{
    return ruleManager_.UnregisterResponseRuleSet(processId, ruleSetId);
}

void PageSceneResponseObserver::ClearProcess(int32_t processId)
{
    ruleManager_.ClearProcess(processId);
}

std::vector<std::pair<int32_t, std::string>> PageSceneResponseObserver::GetActiveRuleJsons() const
{
    return ruleManager_.GetActiveResponseRuleJsons();
}

bool PageSceneResponseObserver::HasRegisteredRules() const
{
    return ruleManager_.HasRegisteredResponseRules();
}

} // namespace OHOS::Ace::NG

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
    if (!CollectSettledCandidates(candidates)) {
        return;
    }
    auto ruleJsons = UiSessionManager::GetInstance()->GetActivePageSceneRuleJsons();
    if (ruleJsons.empty()) {
        return;
    }
    std::map<int32_t, std::vector<std::pair<std::string, std::string>>> payloadsByPid;
    std::map<int32_t, RefPtr<TaskExecutor>> executorByPid;
    MatchSettledCandidates(candidates, ruleJsons, payloadsByPid, executorByPid);
    DispatchResponseReports(payloadsByPid, executorByPid);
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

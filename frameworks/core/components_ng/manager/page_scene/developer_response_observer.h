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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_DEVELOPER_RESPONSE_OBSERVER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_DEVELOPER_RESPONSE_OBSERVER_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/event/developer_response_publisher.h"
#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"

namespace OHOS::Ace {
class TaskExecutor;
}

namespace OHOS::Ace::NG {

// PageScene RESPONSE observer singleton: consumes, deduplicates and stable-gates candidates.
class PageSceneResponseObserver : public DeveloperResponseObserver {
    DECLARE_ACE_TYPE(PageSceneResponseObserver, DeveloperResponseObserver);
public:
    static PageSceneResponseObserver* GetInstance();

    // Toggles the whole RESPONSE path. When active, this instance is wired to the
    // developer response publisher; otherwise candidates are dropped on the fast path.
    void SetActive(bool active);

    void OnDeveloperResponse(const DeveloperResponseCandidate& candidate) override;

    // Called by UiSessionManagerOhos::FlushPageSceneNodeChanged at a stable point.
    void OnContentStable();

    int32_t RegisterRuleSet(int32_t processId, const std::string& ruleJson);
    int32_t UnregisterRuleSet(int32_t processId, const std::string& ruleSetId);
    void ClearProcess(int32_t processId);
    std::vector<std::pair<int32_t, std::string>> GetActiveRuleJsons() const;
    bool HasRegisteredRules() const;

private:
    PageSceneResponseObserver() = default;
    ~PageSceneResponseObserver() override = default;

    void AppendSettledCandidate(const DeveloperResponseCandidate& candidate);
    bool CollectSettledCandidates(std::vector<DeveloperResponseCandidate>& candidates);
    void MatchSettledCandidates(const std::vector<DeveloperResponseCandidate>& candidates,
        const std::vector<std::pair<int32_t, std::string>>& ruleJsons,
        std::map<int32_t, std::vector<std::pair<std::string, std::string>>>& payloadsByPid,
        std::map<int32_t, RefPtr<TaskExecutor>>& executorByPid);
    void DispatchResponseReports(
        const std::map<int32_t, std::vector<std::pair<std::string, std::string>>>& payloadsByPid,
        const std::map<int32_t, RefPtr<TaskExecutor>>& executorByPid);

    bool active_ = false;
    std::mutex mutex_;
    PageSceneRuleManager ruleManager_;
    // DOWN-phase operations keyed by operation token.
    std::unordered_map<std::string, DeveloperResponseCandidate> ops_;
    // Settled operations waiting for the stable gate.
    std::vector<DeveloperResponseCandidate> pendingOps_;
    // node id -> index in pendingOps_; coalesces the touch and click results of one
    // operation when their tokens differ.
    std::unordered_map<int32_t, size_t> nodeOpIndex_;

    ACE_DISALLOW_COPY_AND_MOVE(PageSceneResponseObserver);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_DEVELOPER_RESPONSE_OBSERVER_H

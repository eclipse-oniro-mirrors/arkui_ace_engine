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

#include "base/input_manager/input_manager.h"

#include "saf_gui_fence.h"

#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "core/common/container.h"
#include "core/components_ng/base/node_sensitive_input.h"

namespace OHOS::Ace {

void InputManager::ReportAgentInputDecision(const NG::AgentInputResult& decision)
{
    OHOS::Security::SAF::ComponentClickResult result;
    result.compId = decision.compId;
    result.allowed = decision.allowed;
    // SAF forwards reason unchanged; carry the component metadata as JSON.
    auto metadata = JsonUtil::Create(true);
    // Keep the opaque uint64 session ID exact in JSON.
    metadata->Put("sessionId", std::to_string(decision.sessionId).c_str());
    metadata->Put("compId", decision.compId.c_str());
    metadata->Put("tag", decision.tag.c_str());
    result.reason = metadata->ToString();
    auto container = Container::GetContainer(decision.instanceId);
    CHECK_NULL_VOID(container);
    auto executor = container->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    const auto callingUid = decision.callingUid;
    const bool posted = executor->PostTask(
        [callingUid, result]() {
            // SAF names the routing argument sessionId; pass the injector UID as agreed.
            auto ret = OHOS::Security::SAF::SafGuiFence::ReportComponentClickDecision(callingUid, result);
            if (ret != 0) {
                TAG_LOGW(AceLogTag::ACE_INPUTTRACKING,
                    "report agent input decision failed, error=%{public}d", ret);
            }
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIReportAgentInputDecision");
    if (!posted) {
        TAG_LOGW(AceLogTag::ACE_INPUTTRACKING, "post agent input decision task failed");
    }
}

} // namespace OHOS::Ace

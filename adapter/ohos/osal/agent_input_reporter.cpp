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

#include "application_context.h"
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
    // SAF routes the fence notification per app-clone; 0 when not in a clone.
    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    const int32_t appCloneIndex = context ? context->GetCurrentAppCloneIndex() : 0;
    // SAF forwards reason unchanged; carry the component metadata as JSON.
    auto metadata = JsonUtil::Create(true);
    metadata->Put("nodeId", std::to_string(decision.nodeId).c_str());
    metadata->Put("tag", decision.tag.c_str());
    metadata->Put("bundleName", decision.bundleName.c_str());
    metadata->Put("index", std::to_string(appCloneIndex).c_str());
    result.reason = metadata->ToString();
    auto container = Container::Current();
    if (!container) {
        // Off-UI-thread callers (e.g. batch dispatch on the IPC thread) have no
        // thread-local scope; fall back to the foreground instance.
        container = Container::GetActive();
    }
    CHECK_NULL_VOID(container);
    auto executor = container->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    const auto sessionId = decision.sessionId;
    const bool posted = executor->PostTask(
        [sessionId, result]() {
            // SAF routes the fence notification by the caller-relayed sessionId (tokenId).
            TAG_LOGI(AceLogTag::ACE_INPUTTRACKING, "SendCommand notify security fence.");
            auto ret = OHOS::Security::SAF::SafGuiFence::ReportComponentClickDecision(sessionId, result);
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

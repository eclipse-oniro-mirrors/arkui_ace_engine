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

#include "core/components_ng/pattern/command/ui_command_security_gate.h"

#include <string>

#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/components_ng/relaxed_interaction/utils/focused_target_security_gate.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

thread_local CommandSecurityContext UiCommandSecurityGate::context_;

bool UiCommandSecurityGate::IsCommandDenied(const RefPtr<FrameNode>& target, bool needSecurityAuth)
{
    if (!needSecurityAuth) {
        return false;
    }
    auto node = target;
    if (!node || node->GetAgentControlMode() != AgentControlMode::DENY) {
        return false;
    }
    return InputManager::HasControlComponentPermission();
}

void UiCommandSecurityGate::ReportDeniedCommand(
    const RefPtr<FrameNode>& target, const CommandSecurityContext& security)
{
    auto node = target;
    if (!node) {
        return;
    }
    AgentInputResult result;
    result.sessionId = security.tokenId;
    result.nodeId = node->GetId();
    auto compId = node->GetInspectorId().value_or("");
    if (compId.empty()) {
        compId = std::to_string(node->GetId());
    }
    result.compId = std::move(compId);
    result.tag = node->GetTag();
    result.allowed = false;
    result.bundleName = AceApplicationInfo::GetInstance().GetPackageName();
    InputManager::ReportAgentInputDecision(result);
}

bool IsFocusedTargetDenied(const WeakPtr<PipelineContext>& contextWeak, const std::string& executorType)
{
    const auto& security = UiCommandSecurityGate::GetContext();
    if (!security.needSecurityAuth) {
        return false;
    }
    auto context = contextWeak.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto focusManager = context->GetFocusManager();
    auto focusedHub = focusManager ? focusManager->GetCurrentFocus() : nullptr;
    auto focusedNode = focusedHub ? focusedHub->GetFrameNode() : nullptr;
    if (!UiCommandSecurityGate::IsCommandDenied(focusedNode, security.needSecurityAuth)) {
        return false;
    }
    UiCommandSecurityGate::ReportDeniedCommand(focusedNode, security);
    TAG_LOGW(AceLogTag::ACE_UIEVENT, "Executor %{public}s refused by security gate at focused node",
        executorType.c_str());
    return true;
}

} // namespace OHOS::Ace::NG

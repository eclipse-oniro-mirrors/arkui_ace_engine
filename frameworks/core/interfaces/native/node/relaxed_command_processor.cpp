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

#include "relaxed_command_processor.h"

#include <string>

#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "base/thread/task_executor.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/relaxed_interaction/executor_generator.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"
#include "core/interfaces/native/utility/error_message_macros.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "interfaces/native/ui_event_injection.h"

namespace OHOS::Ace::NG {
namespace {

OH_ArkUI_NativeModule_UIEventInjection_ResultCode MapExecutionResult(ExecutionState state)
{
    switch (state) {
        case ExecutionState::SUCCESS:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS;
        case ExecutionState::TARGET_NOT_FOUND:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND;
        case ExecutionState::INTERRUPTED_BY_USER:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_INTERRUPTED_BY_USER;
        default:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED;
    }
}

bool ValidateCommand(const std::string& command, std::string& type)
{
    const auto nullPosition = command.find('\0');
    if (nullPosition != std::string::npos && nullPosition + 1 < command.size()) {
        return false;
    }
    auto json = JsonUtil::ParseJsonString(command);
    if (!json || !json->IsValid() || !json->IsObject() || json->Contains(FALLBACK_CMD_KEY)) {
        return false;
    }
    auto version = json->GetValue("schemaVersion");
    auto cmd = json->GetValue(CMD_KEY);
    if (!version || !version->IsNumber() || version->GetDouble() != 1.0 || !cmd || !cmd->IsObject()) {
        return false;
    }
    type = cmd->GetString(TYPE_KEY);
    if (type != CLICK && type != CONTENT_SWITCH && type != SCROLL && type != BACKPRESS && type != KEY) {
        return false;
    }
#if defined(RELAXED_INTERACTION_SUPPORT) || defined(ACE_UNITTEST)
    return ExecutorGenerator::ValidateCommand(json);
#else
    return false;
#endif
}

struct Completion {
    int32_t instanceId;
    OH_ArkUI_NativeModule_UIEventInjectionCallback callback;
    void* userData;
    std::string type;

    void Finish(ExecutionState state) const
    {
        const auto result = MapExecutionResult(state);
        TAG_LOGI(AceLogTag::ACE_UIEVENT, "Relaxed injection completed: type=%{public}s, result=%{public}d",
            type.c_str(), static_cast<int32_t>(result));
        if (callback) {
            ContainerScope scope(instanceId);
            callback(result, userData);
        }
    }
};

void ExecuteRelaxedCommandOnUI(const WeakPtr<Container>& weakOwner,
    const Completion& completion, const std::string& command)
{
    ContainerScope scope(completion.instanceId);
    auto originalOwner = weakOwner.Upgrade();
    if (!originalOwner || Container::GetContainer(completion.instanceId) != originalOwner) {
        completion.Finish(ExecutionState::TARGET_NOT_FOUND);
        return;
    }
    auto pipeline = AceType::DynamicCast<PipelineContext>(originalOwner->GetPipelineContext());
    if (!pipeline || pipeline->IsDestroyed()) {
        completion.Finish(ExecutionState::TARGET_NOT_FOUND);
        return;
    }
    auto result = pipeline->ProcessCommand(command, [completion](ExecutionState state) {
        completion.Finish(state);
    });
    if (result != ProcessResult::SUCCESS) {
        completion.Finish(ExecutionState::FAILED);
    }
}

} // namespace

int32_t InjectRelaxedCommandImpl(const ArkUIInjectRelaxedCommandParams* params)
{
    if (!params || !params->json || params->jsonSize == 0) {
        TAG_LOGW(AceLogTag::ACE_UIEVENT, "Relaxed injection rejected: empty command");
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "command must not be empty");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    std::string command(params->json, params->jsonSize);
    std::string type;
    if (!ValidateCommand(command, type)) {
        TAG_LOGW(AceLogTag::ACE_UIEVENT, "Relaxed injection rejected: invalid command");
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "invalid relaxed command json");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }

    auto owner = Container::GetContainer(params->instanceId);
    auto taskExecutor = owner ? owner->GetTaskExecutor() : nullptr;
    if (!owner || !taskExecutor) {
        TAG_LOGW(AceLogTag::ACE_UIEVENT, "Relaxed injection rejected: unavailable instance");
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID, "UI instance unavailable");
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    const Completion completion { params->instanceId,
        reinterpret_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(params->callback), params->userData, type };
    auto execute = [weakOwner = WeakPtr<Container>(owner), completion, command = std::move(command)]() {
        ExecuteRelaxedCommandOnUI(weakOwner, completion, command);
    };
    if (!taskExecutor->PostTask(std::move(execute), TaskExecutor::TaskType::UI, "ArkUIInjectRelaxedCommand")) {
        TAG_LOGW(AceLogTag::ACE_UIEVENT, "Relaxed injection rejected: submission failed");
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "command submission failed");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    TAG_LOGI(AceLogTag::ACE_UIEVENT, "Relaxed injection accepted: type=%{public}s, result=0", type.c_str());
    return ARKUI_ERROR_CODE_NO_ERROR;
}

} // namespace OHOS::Ace::NG

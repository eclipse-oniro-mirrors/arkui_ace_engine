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

#include "interfaces/inner_api/ui_session/ui_session_manager.h"

#include <cmath>
#include <limits>

#include "adapter/ohos/entrance/ui_session/ui_session_batch_command.h"
#include "adapter/ohos/entrance/ui_session/ui_session_command_result.h"
#include "base/thread/task_executor.h"
#include "core/components_ng/base/frame_node.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace {
BatchResolvedTarget ResolveBatchTextTarget(int32_t nodeId);

namespace {
BatchResolvedTarget ResolveBatchTextTargetWithResult(int32_t nodeId)
{
    auto target = ResolveBatchTextTarget(nodeId);
    if (target.status == BatchTargetStatus::READY) {
        auto node = AceType::DynamicCast<NG::FrameNode>(ElementRegister::GetInstance()->GetUINodeById(nodeId));
        if (!node) {
            return {
                .status = BatchTargetStatus::NOT_FOUND
            };
        }
        target.execute = [node](const std::string& command) {
            return node->OnRecvCommandWithResult(command).success;
        };
    }
    return target;
}

bool IsKeyCodeNumber(const std::unique_ptr<JsonValue>& json)
{
    auto value = json->GetValue("cmd");
    if (!value || !value->IsNumber() || !std::isfinite(value->GetDouble())) {
        return false;
    }
    auto number = value->GetDouble();
    return number == std::trunc(number) && number >= std::numeric_limits<int32_t>::min() &&
        number <= std::numeric_limits<int32_t>::max();
}

NG::CommandResult ExecuteKeyCodeWithResult(
    const std::unique_ptr<JsonValue>& json, const RefPtr<NG::PipelineContext>& pipeline)
{
    if (!IsKeyCodeNumber(json)) {
        return NG::CommandResult::Error(NG::CommandErrorType::INVALID_PARAM);
    }
    KeyEvent keyEvent;
    keyEvent.action = KeyAction::DOWN;
    keyEvent.code = static_cast<KeyCode>(json->GetInt("cmd"));
    keyEvent.pressedCodes = { keyEvent.code };
    return pipeline->OnNonPointerEvent(keyEvent)
        ? NG::CommandResult::Success() : NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL);
}

void ExecuteGlobalCommandWithResult(
    const std::string& command, const UiSessionManager::CommandResultCallback& finishCallback)
{
    auto json = JsonUtil::ParseJsonString(command);
    if (!json || !json->IsValid() || !json->IsObject()) {
        finishCallback(SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::INVALID_PARAM)));
        return;
    }
    auto batch = UiSessionBatchCommand::Parse(command);
    if (batch.status != BatchParseStatus::NOT_BATCH) {
        finishCallback(UiSessionBatchCommand::ExecuteWithResult(batch, ResolveBatchTextTargetWithResult));
        return;
    }
    auto pipeline = NG::PipelineContext::GetCurrentContextSafely();
    if (!pipeline) {
        finishCallback(SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL)));
        return;
    }
    auto value = json->GetValue("cmd");
    if (value && value->IsNumber()) {
        finishCallback(SerializeCommandResult(ExecuteKeyCodeWithResult(json, pipeline)));
        return;
    }
    pipeline->ProcessCommandWithResult(command, [finishCallback](const NG::CommandResult& result) {
        finishCallback(SerializeCommandResult(result));
    });
}
} // namespace

void UiSessionManager::RegisterSendCommandWithResultCallback(const WeakPtr<TaskExecutor>& taskExecutor)
{
    auto callback = [weakTaskExecutor = taskExecutor](const std::string& command,
        const UiSessionManager::CommandResultCallback& finishCallback) {
        auto taskExecutor = weakTaskExecutor.Upgrade();
        if (!taskExecutor) {
            finishCallback(SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL)));
            return;
        }
        bool posted = taskExecutor->PostTask(
            [command, finishCallback]() { ExecuteGlobalCommandWithResult(command, finishCallback); },
            TaskExecutor::TaskType::UI, "UiSessionSendCommandWithResult");
        if (!posted) {
            finishCallback(SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL)));
        }
    };
    SaveSendCommandWithResultFunction(std::move(callback));
}

void UiSessionManager::RegisterNodeCommandWithResultCallback(const WeakPtr<TaskExecutor>& taskExecutor)
{
    auto sendCommandWithResult =
        [weakTaskExecutor = taskExecutor](int32_t id, const std::string& command,
            const UiSessionManager::CommandResultCallback& finishCallback) {
            auto taskExecutor = weakTaskExecutor.Upgrade();
            if (!taskExecutor) {
                finishCallback(SerializeCommandResult(
                    NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL)));
                return;
            }
            bool posted = taskExecutor->PostTask(
                [id, command, finishCallback]() {
                    auto node = AceType::DynamicCast<NG::FrameNode>(
                        ElementRegister::GetInstance()->GetUINodeById(id));
                    if (!node) {
                        finishCallback(SerializeCommandResult(
                            NG::CommandResult::Error(NG::CommandErrorType::NODE_NOT_FOUND)));
                        return;
                    }
                    finishCallback(SerializeCommandResult(node->OnRecvCommandWithResult(command)));
                },
                TaskExecutor::TaskType::UI, "UiSessionSendCommandPatternWithResult");
            if (!posted) {
                finishCallback(SerializeCommandResult(
                    NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL)));
            }
        };
    SaveForSendCommandWithResultFunction(std::move(sendCommandWithResult));
}
} // namespace OHOS::Ace

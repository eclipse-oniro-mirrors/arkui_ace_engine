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

#include "core/components_ng/relaxed_interaction/relaxed_interaction_manager.h"

#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/components_ng/relaxed_interaction/base_command_parser.h"
#include "core/components_ng/relaxed_interaction/executor_choreographer.h"
#include "core/components_ng/relaxed_interaction/executor_generator.h"
#include "core/components_ng/relaxed_interaction/executors/backpress_executor.h"
#include "core/components_ng/relaxed_interaction/utils/workflow_dumper.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

RelaxedInteractionManager::RelaxedInteractionManager(WeakPtr<PipelineContext> context) : context_(context)
{
    executorGenerator_ = std::make_unique<ExecutorGenerator>(context);
    choreographer_ = std::make_unique<ExecutorChoreographer>();
}

ProcessResult RelaxedInteractionManager::ProcessCommand(const std::string& jsonStr)
{
    return ProcessCommandInternal(jsonStr, nullptr, true);
}

ProcessResult RelaxedInteractionManager::ProcessCommand(
    const std::string& jsonStr, const std::function<void(ExecutionState)>& finishCallback)
{
    return ProcessCommandInternal(jsonStr, finishCallback, false);
}

ProcessResult RelaxedInteractionManager::ProcessCommandInternal(const std::string& jsonStr,
    const std::function<void(ExecutionState)>& finishCallback, bool preserveLegacySemantics)
{
    WorkflowDumper::GetInstance().AddLog("Processing command: " + jsonStr);
    auto json = JsonUtil::ParseJsonString(jsonStr);
    if (!json || !json->IsValid() || !json->IsObject()) {
        WorkflowDumper::GetInstance().AddLog("Failed to parse command because of invalid JSON format");
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Failed to parse command because of invalid JSON format");
        return ProcessResult::PARSE_ERROR;
    }

    if (!preserveLegacySemantics && !json->Contains(FALLBACK_CMD_KEY)) {
        auto cmd = json->GetValue(CMD_KEY);
        if (!cmd || !cmd->IsObject()) {
            return ProcessResult::VALIDATION_ERROR;
        }
        auto type = cmd->GetValue(TYPE_KEY);
        if (!type || !type->IsString()) {
            return ProcessResult::VALIDATION_ERROR;
        }
        auto name = type->GetString();
        if (name != BACKPRESS && name != CLICK && name != CONTENT_SWITCH && name != SCROLL &&
            name != TAP && name != SLIDE) {
            return ProcessResult::NO_VALID_CMD;
        }
    }

    std::vector<std::unique_ptr<BaseExecutor>> executors = executorGenerator_->ParseCommand(json);
    if (executors.empty()) {
        WorkflowDumper::GetInstance().AddLog("No valid executor parsed from command");
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "No valid executor parsed from command");
        return ProcessResult::VALIDATION_ERROR;
    }

    if (SystemProperties::GetDebugEnabled()) {
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "executors size: %{public}zu, types: %{public}s", executors.size(),
            ExecutorsToString(executors).c_str());
    }

    WorkflowDumper::GetInstance().AddLog("Parsed executors: " + ExecutorsToString(executors));
    size_t executorCount = executors.size();
    choreographer_->Enqueue(std::move(executors));
    workflowStates_.push_back({ executorCount, finishCallback, preserveLegacySemantics });
    return ProcessResult::SUCCESS;
}

ExecutionState RelaxedInteractionManager::ExecuteNextStep()
{
    if (!choreographer_->HasCurrent() && !choreographer_->HasNext()) {
        return ExecutionState::FAILED;
    }

    std::function<void(ExecutionState)> finishCallback;
    ExecutionState state = DoExecuteNextStep(finishCallback);
    WorkflowDumper::GetInstance().AddLog("Current workflow state changed to: " + ExecutionStateToString(state));
    if (finishCallback) {
        finishCallback(state);
    }
    return state;
}

bool RelaxedInteractionManager::HasPendingWorkflow() const
{
    return choreographer_ && (choreographer_->HasCurrent() || choreographer_->HasNext());
}

ExecutionState RelaxedInteractionManager::DoExecuteNextStep(
    std::function<void(ExecutionState)>& finishCallback)
{
    ExecutorResult result = choreographer_->ExecuteNext();

    if (workflowStates_.empty()) {
        switch (result) {
            case ExecutorResult::PENDING:
                return ExecutionState::RUNNING;
            case ExecutorResult::SUCCESS:
                choreographer_->Clear();
                return ExecutionState::SUCCESS;
            case ExecutorResult::FAILED:
                return choreographer_->HasNext() ? ExecutionState::RUNNING : ExecutionState::FAILED;
        }
        return ExecutionState::FAILED;
    }

    auto& workflow = workflowStates_.front();

    if (workflow.preserveLegacySemantics) {
        return ExecuteLegacyWorkflow(result);
    }

    switch (result) {
        case ExecutorResult::PENDING:
            return ExecutionState::RUNNING;

        case ExecutorResult::SUCCESS:
            if (workflow.remainingExecutors > 1) {
                choreographer_->DiscardFront(workflow.remainingExecutors - 1);
            }
            finishCallback = std::move(workflow.finishCallback);
            workflowStates_.pop_front();
            return ExecutionState::SUCCESS;

        case ExecutorResult::FAILED:
            if (workflow.remainingExecutors > 0) {
                --workflow.remainingExecutors;
            }
            if (workflow.remainingExecutors > 0) {
                return ExecutionState::RUNNING;
            } else {
                finishCallback = std::move(workflow.finishCallback);
                workflowStates_.pop_front();
                return ExecutionState::FAILED;
            }
    }

    return ExecutionState::FAILED;
}

ExecutionState RelaxedInteractionManager::ExecuteLegacyWorkflow(ExecutorResult result)
{
    auto& workflow = workflowStates_.front();
    if (result == ExecutorResult::PENDING) {
        return ExecutionState::RUNNING;
    }
    if (result == ExecutorResult::SUCCESS) {
        if (workflow.remainingExecutors > 1) {
            choreographer_->DiscardFront(workflow.remainingExecutors - 1);
        }
        workflowStates_.pop_front();
        while (!workflowStates_.empty() && workflowStates_.front().preserveLegacySemantics) {
            choreographer_->DiscardFront(workflowStates_.front().remainingExecutors);
            workflowStates_.pop_front();
        }
        return ExecutionState::SUCCESS;
    }
    if (workflow.remainingExecutors > 0) {
        --workflow.remainingExecutors;
    }
    if (workflow.remainingExecutors == 0) {
        workflowStates_.pop_front();
    }
    return choreographer_->HasNext() ? ExecutionState::RUNNING : ExecutionState::FAILED;
}

void RelaxedInteractionManager::Clear()
{
    if (choreographer_) {
        choreographer_->Clear();
    }
    workflowStates_.clear();
}

std::string RelaxedInteractionManager::ExecutorsToString(
    const std::vector<std::unique_ptr<BaseExecutor>>& executors)
{
    std::ostringstream buffer;
    for (size_t i = 0; i < executors.size(); ++i) {
        if (i != 0) {
            buffer << ",";
        }
        buffer << executors[i]->GetDescription();
    }
    return buffer.str();
}

std::string RelaxedInteractionManager::ExecutionStateToString(ExecutionState state)
{
    switch (state) {
        case ExecutionState::NONE:
            return "NONE";
        case ExecutionState::RUNNING:
            return "RUNNING";
        case ExecutionState::SUCCESS:
            return "SUCCESS";
        case ExecutionState::FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

} // namespace OHOS::Ace::NG

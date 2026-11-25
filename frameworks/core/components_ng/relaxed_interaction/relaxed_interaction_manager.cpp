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

#include <algorithm>

#include "base/json/json_util.h"
#include "base/log/log.h"
#include "core/components_ng/pattern/command/ui_command_security_gate.h"
#include "core/components_ng/relaxed_interaction/base_command_parser.h"
#include "core/components_ng/relaxed_interaction/executor_choreographer.h"
#include "core/components_ng/relaxed_interaction/executor_generator.h"
#include "core/components_ng/relaxed_interaction/executors/backpress_executor.h"
#include "core/components_ng/relaxed_interaction/utils/workflow_dumper.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
bool IsSupportedCommandType(const std::string& type)
{
    return type == BACKPRESS || type == CLICK || type == CONTENT_SWITCH || type == SCROLL ||
        type == TAP || type == SLIDE || type == PRESS_KEY || type == WHEEL_SCROLL ||
        type == PRESS_AND_MOVE;
}

ProcessResult ValidateCommandType(const std::unique_ptr<JsonValue>& json, bool preserveLegacySemantics,
    bool interruptibleByUser)
{
    if (preserveLegacySemantics || json->Contains(FALLBACK_CMD_KEY)) {
        return ProcessResult::SUCCESS;
    }
    auto cmd = json->GetValue(CMD_KEY);
    if (!cmd || !cmd->IsObject()) {
        return ProcessResult::VALIDATION_ERROR;
    }
    auto type = cmd->GetValue(TYPE_KEY);
    if (!type || !type->IsString()) {
        return ProcessResult::VALIDATION_ERROR;
    }
    auto name = type->GetString();
    if (!IsSupportedCommandType(name) && !(interruptibleByUser && name == KEY)) {
        return ProcessResult::NO_VALID_CMD;
    }
    return ProcessResult::SUCCESS;
}

// P2: identity carried next to cmd in the command JSON.
RelaxedCommandIdentity ParseCommandIdentity(const std::unique_ptr<JsonValue>& json)
{
    RelaxedCommandIdentity identity;
    auto identityJson = json->GetValue("identity");
    if (identityJson && identityJson->IsObject()) {
        if (identityJson->Contains("source")) {
            identity.source = identityJson->GetString("source");
        }
        if (identityJson->Contains("token")) {
            identity.token = identityJson->GetString("token");
        }
        if (identityJson->Contains("label")) {
            identity.label = identityJson->GetString("label");
        }
    }
    if (identity.source.empty() && json->Contains("source")) {
        identity.source = json->GetString("source");
    }
    if (identity.token.empty() && json->Contains("token")) {
        identity.token = json->GetString("token");
    }
    if (identity.label.empty() && json->Contains("label")) {
        identity.label = json->GetString("label");
    }
    return identity;
}
}

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
    const std::function<void(ExecutionState)>& finishCallback, bool preserveLegacySemantics, bool interruptibleByUser,
    bool isPcCommand)
{
    WorkflowDumper::GetInstance().AddLog("Processing relaxed interaction command");
    auto json = JsonUtil::ParseJsonString(jsonStr);
    if (!json || !json->IsValid() || !json->IsObject()) {
        WorkflowDumper::GetInstance().AddLog("Failed to parse command because of invalid JSON format");
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Failed to parse command because of invalid JSON format");
        return ProcessResult::PARSE_ERROR;
    }

    auto validation = ValidateCommandType(json, preserveLegacySemantics, interruptibleByUser);
    if (validation != ProcessResult::SUCCESS) {
        return validation;
    }

    return ParseAndEnqueueCommand(json, finishCallback, preserveLegacySemantics, interruptibleByUser, isPcCommand);
}

ProcessResult RelaxedInteractionManager::ParseAndEnqueueCommand(const std::unique_ptr<JsonValue>& json,
    const std::function<void(ExecutionState)>& finishCallback, bool preserveLegacySemantics, bool interruptibleByUser,
    bool isPcCommand)
{
    // Drop any stale special-component hit payload left by a previous workflow.
    BaseExecutor::ClearSpecialComponentHit();

    // P2: stamp the security identity carried by this command before parsing.
    executorGenerator_->SetIdentity(ParseCommandIdentity(json));

    auto executors = interruptibleByUser ? executorGenerator_->ParseInjectionCommand(json)
                                       : executorGenerator_->ParseCommand(json);
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
    auto securityContext = UiCommandSecurityGate::GetContext();
    std::string commandType;
    if (!preserveLegacySemantics && !json->Contains(FALLBACK_CMD_KEY)) {
        auto cmd = json->GetValue(CMD_KEY);
        if (cmd && cmd->IsObject() && cmd->Contains(TYPE_KEY)) {
            commandType = cmd->GetString(TYPE_KEY);
        }
    }
    // A declared hold legitimately extends the workflow timeout window (see HandleWorkflowTimeout).
    int32_t holdDurationMs = 0;
    for (const auto& executor : executors) {
        holdDurationMs = std::max(holdDurationMs, executor->GetHoldMs());
    }
    choreographer_->Enqueue(std::move(executors));
    // startTime stays zero: a workflow's timeout window opens when it becomes the running one.
    workflowStates_.push_back({ executorCount, finishCallback, preserveLegacySemantics,
        interruptibleByUser ? InterruptionPolicy::INTERRUPTIBLE : InterruptionPolicy::IGNORE_USER_INPUT,
        securityContext.needSecurityAuth, securityContext.agentTokenId, holdDurationMs,
        commandType, std::chrono::steady_clock::time_point {}, isPcCommand });
    return ProcessResult::SUCCESS;
}

ProcessResult RelaxedInteractionManager::ProcessCommand(const std::string& jsonStr,
    const std::function<void(ExecutionState)>& finishCallback, bool interruptibleByUser, bool isPcCommand)
{
    return ProcessCommandInternal(jsonStr, finishCallback, false, interruptibleByUser, isPcCommand);
}

ExecutionState RelaxedInteractionManager::ExecuteNextStep()
{
    std::deque<std::function<void(ExecutionState)>> completions;
    completions.swap(pendingUserCompletions_);
    for (auto& completion : completions) {
        completion(ExecutionState::INTERRUPTED_BY_USER);
    }
    if (!choreographer_->HasCurrent() && !choreographer_->HasNext()) {
        UiCommandSecurityGate::ClearContext();
        return ExecutionState::FAILED;
    }

    if (!workflowStates_.empty()) {
        const auto& workflow = workflowStates_.front();
        UiCommandSecurityGate::SetContext(
            CommandSecurityContext { workflow.needSecurityAuth, workflow.agentTokenId });
    }

    if (!workflowStates_.empty() && workflowStates_.front().isPcCommand) {
        auto context = context_.Upgrade();
        if (!context || context->IsDestroyed()) {
            std::function<void(ExecutionState)> finishCallback;
            CompleteCurrentWorkflow(finishCallback, false);
            if (finishCallback) {
                finishCallback(ExecutionState::CONTEXT_UNAVAILABLE);
            }
            return ExecutionState::CONTEXT_UNAVAILABLE;
        }
    }

    // P1 per-command timeout: close out the running workflow when it exceeds the limit.
    std::function<void(ExecutionState)> timeoutCallback;
    if (HandleWorkflowTimeout(timeoutCallback)) {
        if (timeoutCallback) {
            timeoutCallback(ExecutionState::TIMEOUT);
        }
        return ExecutionState::TIMEOUT;
    }

    std::function<void(ExecutionState)> finishCallback;
    ExecutionState state = DoExecuteNextStep(finishCallback);
    WorkflowDumper::GetInstance().AddLog("Current workflow state changed to: " + ExecutionStateToString(state));
    if (finishCallback) {
        finishCallback(state);
    }
    if (state != ExecutionState::RUNNING && workflowStates_.empty() && !HasPendingWorkflow()) {
        UiCommandSecurityGate::ClearContext();
    }
    return state;
}

bool RelaxedInteractionManager::HandleWorkflowTimeout(std::function<void(ExecutionState)>& timeoutCallback)
{
    if (executionPhase_ != ExecutionPhase::IDLE) {
        return false;
    }
    if (workflowStates_.empty()) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    auto& workflow = workflowStates_.front();
    // The timeout window opens when the workflow becomes the running one.
    if (workflow.startTime == std::chrono::steady_clock::time_point {}) {
        workflow.startTime = now;
        return false;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - workflow.startTime).count();
    // A declared hold extends the effective timeout: hold_ms can legitimately reach 5000 ms,
    // equal to the default window, so judge TIMEOUT only past hold + scheduling margin.
    // Workflows without a hold keep the configured window unchanged.
    const auto effectiveTimeout = workflow.holdDurationMs > 0
        ? std::max(commandTimeoutMs_, workflow.holdDurationMs + WORKFLOW_TIMEOUT_MARGIN_MS)
        : commandTimeoutMs_;
    if (elapsed < static_cast<int64_t>(effectiveTimeout)) {
        return false;
    }
    TAG_LOGW(AceLogTag::ACE_UIEVENT, "Workflow exceeded command timeout %{public}d ms", effectiveTimeout);
    WorkflowDumper::GetInstance().AddLog("Workflow exceeded command timeout, completing with TIMEOUT");
    auto cancelledExecutors = choreographer_->ExtractExecutors(std::vector<bool>(workflow.remainingExecutors, true));
    timeoutCallback = std::move(workflow.finishCallback);
    workflowStates_.pop_front();
    // Detach the workflow before compensation can reenter the manager.
    for (auto& executor : cancelledExecutors) {
        executor->Cancel();
    }
    return true;
}

void RelaxedInteractionManager::SetDenyCallback(RelaxedDenyCallback callback)
{
    if (executorGenerator_) {
        executorGenerator_->SetDenyCallback(std::move(callback));
    }
}

bool RelaxedInteractionManager::IsPointerClassInjectionRunning() const
{
    return !workflowStates_.empty() && IsPointerClassCommand(workflowStates_.front().commandType);
}

bool RelaxedInteractionManager::IsKeyClassInjectionRunning() const
{
    return !workflowStates_.empty() && IsKeyClassCommand(workflowStates_.front().commandType);
}

bool RelaxedInteractionManager::IsContextAvailable() const
{
    auto context = context_.Upgrade();
    return context && !context->IsDestroyed();
}

bool RelaxedInteractionManager::HasPendingWorkflow() const
{
    return !pendingUserCompletions_.empty() ||
        (choreographer_ && (choreographer_->HasCurrent() || choreographer_->HasNext()));
}

// Completes a step that finished while no workflow is queued (legacy late completion).
ExecutionState RelaxedInteractionManager::ExecuteStepWithoutWorkflow(ExecutorResult result)
{
    switch (result) {
        case ExecutorResult::PENDING:
            return ExecutionState::RUNNING;
        case ExecutorResult::SUCCESS:
            choreographer_->Clear();
            return ExecutionState::SUCCESS;
        case ExecutorResult::FAILED:
            return choreographer_->HasNext() ? ExecutionState::RUNNING : ExecutionState::FAILED;
        default:
            return ExecutionState::FAILED;
    }
}

ExecutionState RelaxedInteractionManager::DoExecuteNextStep(
    std::function<void(ExecutionState)>& finishCallback)
{
    executionPhase_ = ExecutionPhase::EXECUTING;
    ExecutorResult result = choreographer_->ExecuteNext();
    ExecutionState clearedState;
    if (HandleDeferredClear(result, clearedState)) {
        return clearedState;
    }
    executionPhase_ = ExecutionPhase::IDLE;

    if (workflowStates_.empty()) {
        return ExecuteStepWithoutWorkflow(result);
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
            if (choreographer_->GetFailureState() == ExecutionState::CONTROL_DENIED ||
                (workflow.isPcCommand && choreographer_->GetFailureState() == ExecutionState::CONTEXT_UNAVAILABLE)) {
                auto failure = choreographer_->GetFailureState();
                CompleteCurrentWorkflow(finishCallback, true);
                return failure;
            }
            if (workflow.isPcCommand && choreographer_->CurrentExecutorHasProducedInput()) {
                // The failed executor already delivered input (partial multi-click): a fallback
                // would replay the full count, so finish the workflow as failed without fallback.
                CompleteCurrentWorkflow(finishCallback, true);
                return choreographer_->GetFailureState();
            }
            if (workflow.remainingExecutors > 0) {
                --workflow.remainingExecutors;
            }
            if (workflow.remainingExecutors > 0) {
                return ExecutionState::RUNNING;
            } else {
                finishCallback = std::move(workflow.finishCallback);
                workflowStates_.pop_front();
                return choreographer_->GetFailureState();
            }
    }

    return ExecutionState::FAILED;
}

void RelaxedInteractionManager::CompleteCurrentWorkflow(
    std::function<void(ExecutionState)>& finishCallback, bool stepFinished)
{
    auto count = workflowStates_.front().remainingExecutors;
    if (stepFinished && count > 0) {
        --count;
    }
    finishCallback = std::move(workflowStates_.front().finishCallback);
    workflowStates_.pop_front();
    auto cancelledExecutors = choreographer_->ExtractExecutors(std::vector<bool>(count, true));
    for (auto& executor : cancelledExecutors) {
        executor->Cancel();
    }
}

void RelaxedInteractionManager::InterruptForUserInput(UserInput input, bool interruptLegacy)
{
    bool interrupted = false;
    for (auto& workflow : workflowStates_) {
        if (workflow.interruptionPolicy == InterruptionPolicy::IGNORE_USER_INPUT) {
            continue;
        }
        const bool conflicts = workflow.isPcCommand
            ? ((input == UserInput::KEY && IsKeyClassCommand(workflow.commandType)) ||
                (input == UserInput::POINTER && IsPointerClassCommand(workflow.commandType)))
            : interruptLegacy;
        if (conflicts) {
            workflow.interruptionPolicy = InterruptionPolicy::INTERRUPTED;
            interrupted = true;
        }
    }
    if (!interrupted) {
        return;
    }
    if (executionPhase_ != ExecutionPhase::IDLE) {
        if (executionPhase_ == ExecutionPhase::EXECUTING) {
            clearReason_ = ExecutionState::INTERRUPTED_BY_USER;
        }
        executionPhase_ = ExecutionPhase::CLEAR_PENDING;
        return;
    }
    ClearInterruptedWorkflows();
}

bool RelaxedInteractionManager::HandleDeferredClear(ExecutorResult result, ExecutionState& state)
{
    if (executionPhase_ != ExecutionPhase::CLEAR_PENDING) {
        return false;
    }
    auto reason = clearReason_;
    executionPhase_ = ExecutionPhase::IDLE;
    if (reason == ExecutionState::INTERRUPTED_BY_USER) {
        bool currentInterrupted = !workflowStates_.empty() &&
            workflowStates_.front().interruptionPolicy == InterruptionPolicy::INTERRUPTED;
        ClearInterruptedWorkflows(result != ExecutorResult::PENDING);
        if (!currentInterrupted && !workflowStates_.empty()) {
            return false;
        }
    } else {
        Clear(reason);
    }
    state = HasPendingWorkflow() ? ExecutionState::RUNNING : reason;
    return true;
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
    pendingUserCompletions_.clear();
    for (auto& workflow : workflowStates_) {
        workflow.finishCallback = nullptr;
    }
    Clear(ExecutionState::FAILED);
}

void RelaxedInteractionManager::Clear(ExecutionState reason)
{
    if (reason == ExecutionState::INTERRUPTED_BY_USER) {
        bool hasInterrupted = false;
        for (auto& workflow : workflowStates_) {
            if (workflow.interruptionPolicy != InterruptionPolicy::IGNORE_USER_INPUT) {
                workflow.interruptionPolicy = InterruptionPolicy::INTERRUPTED;
                hasInterrupted = true;
            }
        }
        if (!hasInterrupted) {
            return;
        }
    }
    if (executionPhase_ != ExecutionPhase::IDLE) {
        if (executionPhase_ == ExecutionPhase::EXECUTING ||
            (clearReason_ != ExecutionState::TARGET_NOT_FOUND &&
                (reason != ExecutionState::INTERRUPTED_BY_USER || clearReason_ == reason))) {
            clearReason_ = reason;
        }
        executionPhase_ = ExecutionPhase::CLEAR_PENDING;
        return;
    }
    if (reason == ExecutionState::INTERRUPTED_BY_USER) {
        ClearInterruptedWorkflows();
        return;
    }
    std::deque<WorkflowState> cancelledWorkflows;
    cancelledWorkflows.swap(workflowStates_);
    std::deque<std::function<void(ExecutionState)>> completions;
    completions.swap(pendingUserCompletions_);
    if (choreographer_) {
        choreographer_->Clear();
    }
    UiCommandSecurityGate::ClearContext();
    for (auto& completion : completions) {
        completion(ExecutionState::INTERRUPTED_BY_USER);
    }
    for (auto& workflow : cancelledWorkflows) {
        if (workflow.finishCallback && workflow.interruptionPolicy != InterruptionPolicy::IGNORE_USER_INPUT) {
            auto state = reason;
            auto context = context_.Upgrade();
            if (workflow.isPcCommand && state == ExecutionState::TARGET_NOT_FOUND &&
                (!context || context->IsDestroyed())) {
                state = ExecutionState::CONTEXT_UNAVAILABLE;
            }
            workflow.finishCallback(state);
        }
    }
}

void RelaxedInteractionManager::ClearInterruptedWorkflows(bool currentStepFinished)
{
    std::vector<bool> removeExecutors;
    std::deque<WorkflowState> retainedWorkflows;
    std::deque<std::function<void(ExecutionState)>> synchronousCompletions;
    for (auto& workflow : workflowStates_) {
        size_t count = workflow.remainingExecutors;
        if (currentStepFinished && count > 0) {
            --count;
        }
        currentStepFinished = false;
        bool interrupted = workflow.interruptionPolicy == InterruptionPolicy::INTERRUPTED;
        removeExecutors.insert(removeExecutors.end(), count, interrupted);
        if (!interrupted) {
            retainedWorkflows.push_back(std::move(workflow));
        } else if (workflow.finishCallback) {
            auto& completions = workflow.isPcCommand ? synchronousCompletions : pendingUserCompletions_;
            completions.push_back(std::move(workflow.finishCallback));
        }
    }
    workflowStates_.swap(retainedWorkflows);
    auto cancelledExecutors = choreographer_->ExtractExecutors(removeExecutors);
    for (auto& executor : cancelledExecutors) {
        executor->Cancel();
    }
    if (workflowStates_.empty()) {
        UiCommandSecurityGate::ClearContext();
    }
    for (auto& completion : synchronousCompletions) {
        completion(ExecutionState::INTERRUPTED_BY_USER);
    }
    auto context = context_.Upgrade();
    if (!pendingUserCompletions_.empty() && context && !context->IsDestroyed()) {
        context->RequestFrame();
    }
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
        case ExecutionState::TARGET_NOT_FOUND:
            return "TARGET_NOT_FOUND";
        case ExecutionState::INTERRUPTED_BY_USER:
            return "INTERRUPTED_BY_USER";
        case ExecutionState::TIMEOUT:
            return "TIMEOUT";
        case ExecutionState::CONTROL_DENIED:
            return "CONTROL_DENIED";
        case ExecutionState::CONTEXT_UNAVAILABLE:
            return "CONTEXT_UNAVAILABLE";
        default:
            return "UNKNOWN";
    }
}

} // namespace OHOS::Ace::NG

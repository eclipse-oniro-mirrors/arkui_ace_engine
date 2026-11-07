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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_MANAGER_H

#include <deque>
#include <functional>
#include <memory>
#include <string>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"

namespace OHOS::Ace::NG {

class BaseExecutor;
class ExecutorGenerator;
class ExecutorChoreographer;
class PipelineContext;

class RelaxedInteractionManager : public AceType {
    DECLARE_ACE_TYPE(RelaxedInteractionManager, AceType);

public:
    explicit RelaxedInteractionManager(WeakPtr<PipelineContext> context);
    ~RelaxedInteractionManager() = default;

    ProcessResult ProcessCommand(const std::string& jsonStr);
    ProcessResult ProcessCommand(
        const std::string& jsonStr, const std::function<void(ExecutionState)>& finishCallback);
    ProcessResult ProcessCommand(const std::string& jsonStr,
        const std::function<void(ExecutionState)>& finishCallback, bool interruptibleByUser);

    ExecutionState ExecuteNextStep();

    bool HasPendingWorkflow() const;

    void Clear();
    void Clear(ExecutionState reason);

private:
    ProcessResult ProcessCommandInternal(const std::string& jsonStr,
        const std::function<void(ExecutionState)>& finishCallback, bool preserveLegacySemantics,
        bool interruptibleByUser = false);
    void ClearInterruptedWorkflows(bool currentStepFinished = false);
    bool HandleDeferredClear(ExecutorResult result, ExecutionState& state);
    ExecutionState DoExecuteNextStep(std::function<void(ExecutionState)>& finishCallback);
    ExecutionState ExecuteLegacyWorkflow(ExecutorResult result);
    std::string ExecutionStateToString(ExecutionState state);
    std::string ExecutorsToString(const std::vector<std::unique_ptr<BaseExecutor>>& executors);

    WeakPtr<PipelineContext> context_;
    std::unique_ptr<ExecutorGenerator> executorGenerator_;
    std::unique_ptr<ExecutorChoreographer> choreographer_;
    enum class InterruptionPolicy {
        IGNORE_USER_INPUT,
        INTERRUPTIBLE,
        INTERRUPTED,
    };
    struct WorkflowState {
        size_t remainingExecutors = 0;
        std::function<void(ExecutionState)> finishCallback;
        bool preserveLegacySemantics = false;
        InterruptionPolicy interruptionPolicy = InterruptionPolicy::IGNORE_USER_INPUT;
    };
    enum class ExecutionPhase {
        IDLE,
        EXECUTING,
        CLEAR_PENDING,
    };
    std::deque<WorkflowState> workflowStates_;
    std::deque<std::function<void(ExecutionState)>> pendingUserCompletions_;
    ExecutionPhase executionPhase_ = ExecutionPhase::IDLE;
    ExecutionState clearReason_ = ExecutionState::FAILED;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_MANAGER_H

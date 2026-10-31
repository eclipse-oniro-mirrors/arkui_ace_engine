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

#include "core/components_ng/relaxed_interaction/executor_choreographer.h"
#include "core/components_ng/relaxed_interaction/utils/workflow_dumper.h"

namespace OHOS::Ace::NG {

void ExecutorChoreographer::Enqueue(std::vector<std::unique_ptr<BaseExecutor>>&& executors)
{
    for (auto& executor : executors) {
        executorQueue_.push_back(std::move(executor));
    }
}

ExecutorResult ExecutorChoreographer::ExecuteNext()
{
    // If no current executor, try to move to the next one.
    if (!currentExecutor_) {
        if (!MoveToNext() || !currentExecutor_) {
            return ExecutorResult::FAILED;
        }
    }

    ExecutorResult result = currentExecutor_->ExecuteStep();
    WorkflowDumper::GetInstance().AddLog("Executor '" + currentExecutor_->GetDescription() + "' execution result: " +
        ExecutorResultToString(result));
    failureState_ = currentExecutor_->GetFailureState();
    if (result != ExecutorResult::PENDING) {
        currentExecutor_.reset();
    }

    return result;
}

void ExecutorChoreographer::Clear()
{
    auto current = std::move(currentExecutor_);
    std::deque<std::unique_ptr<BaseExecutor>> cancelledExecutors;
    cancelledExecutors.swap(executorQueue_);
    if (current) {
        current->Cancel();
    }
    for (auto& executor : cancelledExecutors) {
        executor->Cancel();
    }
}

void ExecutorChoreographer::DiscardFront(size_t count)
{
    while (count > 0 && !executorQueue_.empty()) {
        executorQueue_.pop_front();
        --count;
    }
}

std::vector<std::unique_ptr<BaseExecutor>> ExecutorChoreographer::ExtractExecutors(
    const std::vector<bool>& removeExecutors)
{
    std::vector<std::unique_ptr<BaseExecutor>> removed;
    size_t index = 0;
    if (currentExecutor_) {
        if (index < removeExecutors.size() && removeExecutors[index]) {
            removed.push_back(std::move(currentExecutor_));
        }
        ++index;
    }
    std::deque<std::unique_ptr<BaseExecutor>> retained;
    for (auto& executor : executorQueue_) {
        if (index < removeExecutors.size() && removeExecutors[index]) {
            removed.push_back(std::move(executor));
        } else {
            retained.push_back(std::move(executor));
        }
        ++index;
    }
    executorQueue_.swap(retained);
    return removed;
}

bool ExecutorChoreographer::MoveToNext()
{
    if (executorQueue_.empty()) {
        return false;
    }

    currentExecutor_ = std::move(executorQueue_.front());
    executorQueue_.pop_front();
    return true;
}

std::string ExecutorChoreographer::ExecutorResultToString(ExecutorResult result)
{
    switch (result) {
        case ExecutorResult::SUCCESS:
            return "SUCCESS";
        case ExecutorResult::FAILED:
            return "FAILED";
        case ExecutorResult::PENDING:
            return "PENDING";
        default:
            return "UNKNOWN";
    }
}

} // namespace OHOS::Ace::NG

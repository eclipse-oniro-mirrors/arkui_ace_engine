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

#include "gtest/gtest.h"

#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "core/common/container_scope.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"
#include "core/interfaces/native/node/relaxed_command_processor.h"
#include "interfaces/native/ui_event_injection.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
using Result = OH_ArkUI_NativeModule_UIEventInjection_ResultCode;
const std::string COMMAND = R"({"schemaVersion":1,"cmd":{"type":"backpress"}})";
std::map<int32_t, std::vector<std::function<void(ExecutionState)>>> g_completions;
ProcessResult g_processResult = ProcessResult::SUCCESS;
std::string g_command;
int32_t g_processScope = -1;
std::thread::id g_processThread;

class QueuedExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType, VsyncBarrierOption barrierOption) const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (reject_) {
            return false;
        }
        tasks_.push(std::move(task));
        return true;
    }

    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return std::this_thread::get_id() == uiThread_;
    }

    Task PopTask()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            return {};
        }
        auto task = std::move(tasks_.front());
        tasks_.pop();
        return task;
    }

    void Drain()
    {
        while (auto task = PopTask()) {
            task();
        }
    }

    bool reject_ = false;

private:
    std::thread::id uiThread_ = std::this_thread::get_id();
    mutable std::mutex mutex_;
    mutable std::queue<Task> tasks_;
};

struct CompletionState {
    Result result = OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS;
    int32_t count = 0;
    int32_t scope = -1;
    std::thread::id thread;
};

void OnCompletion(Result result, void* userData)
{
    auto* state = static_cast<CompletionState*>(userData);
    state->result = result;
    state->scope = ContainerScope::CurrentId();
    state->thread = std::this_thread::get_id();
    ++state->count;
}

int32_t Submit(const std::string& command, CompletionState* state, int32_t instanceId = 31)
{
    ArkUIInjectRelaxedCommandParams params = {};
    params.instanceId = instanceId;
    params.json = command.data();
    params.jsonSize = static_cast<uint32_t>(command.size());
    params.callback = state ? reinterpret_cast<void (*)(int32_t, void*)>(OnCompletion) : nullptr;
    params.userData = state;
    return InjectRelaxedCommandImpl(&params);
}
} // namespace

// Compile the real backend and parsers, with the pipeline boundary captured for this target.
ProcessResult PipelineContext::ProcessCommand(const std::string& command,
    const std::function<void(ExecutionState)>& completion)
{
    g_command = command;
    g_processScope = ContainerScope::CurrentId();
    g_processThread = std::this_thread::get_id();
    if (g_processResult == ProcessResult::SUCCESS) {
        g_completions[g_processScope].push_back(completion);
    }
    return g_processResult;
}

class RelaxedCommandProcessorTest : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp(MockPipelineContext::GetCurrent());
        executor_ = AceType::MakeRefPtr<QueuedExecutor>();
        MockContainer::Current()->SetTaskExecutor(executor_);
        g_completions.clear();
        g_processResult = ProcessResult::SUCCESS;
        g_command.clear();
        g_processScope = -1;
    }

    void TearDown() override
    {
        executor_->Drain();
        g_completions.clear();
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }

    RefPtr<QueuedExecutor> executor_;
};

HWTEST_F(RelaxedCommandProcessorTest, Completion_RestoresTypedCallbackAndMapsResult, TestSize.Level1)
{
    for (const auto state : { ExecutionState::SUCCESS, ExecutionState::TARGET_NOT_FOUND,
             ExecutionState::FAILED, ExecutionState::INTERRUPTED_BY_USER }) {
        CompletionState completionState;
        const auto previousScope = ContainerScope::CurrentId();
        ASSERT_EQ(Submit(COMMAND, &completionState), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(completionState.count, 0);
        EXPECT_TRUE(g_command.empty());
        executor_->Drain();
        EXPECT_EQ(g_command, COMMAND);
        EXPECT_EQ(g_processScope, 31);
        EXPECT_EQ(ContainerScope::CurrentId(), previousScope);
        auto completion = g_completions[31].back();
        completion(state);
        EXPECT_EQ(completionState.count, 1);
        EXPECT_EQ(completionState.scope, 31);
        const std::map<ExecutionState, Result> expected = {
            { ExecutionState::SUCCESS, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS },
            { ExecutionState::TARGET_NOT_FOUND, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND },
            { ExecutionState::FAILED, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED },
            { ExecutionState::INTERRUPTED_BY_USER,
                OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_INTERRUPTED_BY_USER },
        };
        EXPECT_EQ(completionState.result, expected.at(state));
        g_command.clear();
    }
}

HWTEST_F(RelaxedCommandProcessorTest, Rejection_DoesNotRetainCallback, TestSize.Level1)
{
    for (const auto* command : {
             "{}", "invalid", R"({"schemaVersion":2,"cmd":{"type":"backpress"}})",
             R"({"schemaVersion":1,"cmd":{"type":"unknown"}})",
             R"({"schemaVersion":1,"cmd":{"type":"key","action_info":{"keyCode":2049}}})",
             R"({"schemaVersion":1,"cmd":{"type":"key","action_info":{"code":999}}})",
             R"({"schemaVersion":1,"cmd":{"type":"click"}})",
             R"({"schemaVersion":1,"cmd":{"type":"content_switch","mode":"target",)"
             R"("action_info":{"count":0,"coordinates":{"pos":{"x":1,"y":1}}}}})" }) {
        CompletionState state;
        EXPECT_EQ(Submit(command, &state), ARKUI_ERROR_CODE_PARAM_INVALID) << command;
        executor_->Drain();
        EXPECT_TRUE(g_completions.empty());
        EXPECT_TRUE(g_command.empty());
        EXPECT_EQ(state.count, 0);
    }
    std::string hidden = COMMAND;
    hidden.push_back('\0');
    hidden += "hidden";
    EXPECT_EQ(Submit(hidden, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(RelaxedCommandProcessorTest, NullCallback_IsAccepted, TestSize.Level1)
{
    ASSERT_EQ(Submit(COMMAND, nullptr), ARKUI_ERROR_CODE_NO_ERROR);
    executor_->Drain();
    ASSERT_EQ(g_completions[31].size(), 1u);
    g_completions[31].back()(ExecutionState::SUCCESS);
}

HWTEST_F(RelaxedCommandProcessorTest, InvalidContextAndTaskExecutor_AreRejected, TestSize.Level1)
{
    EXPECT_EQ(InjectRelaxedCommandImpl(nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    ArkUIInjectRelaxedCommandParams params = {};
    EXPECT_EQ(InjectRelaxedCommandImpl(&params), ARKUI_ERROR_CODE_PARAM_INVALID);
    MockContainer::Current()->SetTaskExecutor(nullptr);
    EXPECT_EQ(Submit(COMMAND, nullptr), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    MockContainer::TearDown();
    EXPECT_EQ(Submit(COMMAND, nullptr), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_TRUE(g_completions.empty());
}

HWTEST_F(RelaxedCommandProcessorTest, WorkerSubmission_CopiesInputAndRunsOnUI, TestSize.Level1)
{
    CompletionState state;
    int32_t result = -1;
    std::thread worker([&result, &state]() {
        auto command = COMMAND;
        result = Submit(command, &state);
        command.assign("destroyed caller buffer");
    });
    worker.join();
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(g_command.empty());
    EXPECT_EQ(state.count, 0);
    executor_->Drain();
    EXPECT_EQ(g_command, COMMAND);
    EXPECT_EQ(g_processThread, std::this_thread::get_id());
    ASSERT_EQ(g_completions[31].size(), 1u);
    g_completions[31].back()(ExecutionState::SUCCESS);
    EXPECT_EQ(state.thread, std::this_thread::get_id());
    EXPECT_EQ(state.count, 1);
}

HWTEST_F(RelaxedCommandProcessorTest, PostFailure_RejectsWithoutCallback, TestSize.Level1)
{
    CompletionState state;
    executor_->reject_ = true;
    EXPECT_EQ(Submit(COMMAND, &state), ARKUI_ERROR_CODE_PARAM_INVALID);
    executor_->Drain();
    EXPECT_EQ(state.count, 0);
    EXPECT_TRUE(g_completions.empty());
}

HWTEST_F(RelaxedCommandProcessorTest, AcceptedButUnsupported_ReportsExecutionFailure, TestSize.Level1)
{
    for (const auto result : { ProcessResult::PARSE_ERROR, ProcessResult::VALIDATION_ERROR,
             ProcessResult::NO_VALID_CMD }) {
        CompletionState state;
        g_processResult = result;
        ASSERT_EQ(Submit(COMMAND, &state), ARKUI_ERROR_CODE_NO_ERROR);
        executor_->Drain();
        EXPECT_EQ(state.count, 1);
        EXPECT_EQ(state.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED);
    }
}

HWTEST_F(RelaxedCommandProcessorTest, UnregisteredOrReplacedInstance_CompletesWithTargetNotFound, TestSize.Level1)
{
    CompletionState first;
    ASSERT_EQ(Submit(COMMAND, &first), ARKUI_ERROR_CODE_NO_ERROR);
    MockContainer::SetGetContainerCallback([](int32_t) -> RefPtr<Container> { return nullptr; });
    executor_->Drain();
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(first.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND);
    MockContainer::SetGetContainerCallback(nullptr);
    CompletionState second;
    ASSERT_EQ(Submit(COMMAND, &second), ARKUI_ERROR_CODE_NO_ERROR);
    auto replacement = AceType::MakeRefPtr<MockContainer>(MockPipelineContext::GetCurrent());
    MockContainer::SetGetContainerCallback([replacement](int32_t) -> RefPtr<Container> { return replacement; });
    executor_->Drain();
    EXPECT_EQ(second.count, 1);
    EXPECT_EQ(second.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND);
    EXPECT_TRUE(g_command.empty());
}

HWTEST_F(RelaxedCommandProcessorTest, ConcurrentInstances_DoNotShareCallbacksOrBusyGate, TestSize.Level1)
{
    auto firstOwner = MockContainer::Current();
    auto secondOwner = AceType::MakeRefPtr<MockContainer>(MockPipelineContext::GetCurrent());
    secondOwner->SetTaskExecutor(executor_);
    MockContainer::SetGetContainerCallback([firstOwner, secondOwner](int32_t id) -> RefPtr<Container> {
        return id == 31 ? firstOwner : (id == 32 ? secondOwner : nullptr);
    });
    std::vector<CompletionState> states(8);
    std::vector<int32_t> results(8, -1);
    std::vector<std::thread> workers;
    for (size_t i = 0; i < states.size(); ++i) {
        workers.emplace_back([&results, &states, i]() { results[i] = Submit(COMMAND, &states[i], 31 + i % 2); });
    }
    for (auto& worker : workers) {
        worker.join();
    }
    executor_->Drain();
    ASSERT_EQ(g_completions[31].size(), 4u);
    ASSERT_EQ(g_completions[32].size(), 4u);
    for (auto& completion : g_completions[31]) {
        completion(ExecutionState::SUCCESS);
    }
    for (auto& completion : g_completions[32]) {
        completion(ExecutionState::INTERRUPTED_BY_USER);
    }
    for (size_t i = 0; i < states.size(); ++i) {
        EXPECT_EQ(results[i], ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(states[i].count, 1);
        EXPECT_EQ(states[i].scope, 31 + i % 2);
        EXPECT_EQ(states[i].result, i % 2 == 0 ? OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS
            : OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_INTERRUPTED_BY_USER);
    }
}

HWTEST_F(RelaxedCommandProcessorTest, ValidOptionalScrollDefaults_AreAcceptedFromWorker, TestSize.Level1)
{
    const std::string command = R"({"schemaVersion":1,"cmd":{"type":"scroll","mode":"simulating",)"
                                R"("action_info":{"coordinates":{"x1":10,"y1":10}}}})";
    int32_t result = -1;
    std::thread worker([&result, &command]() { result = Submit(command, nullptr); });
    worker.join();
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
    executor_->Drain();
    EXPECT_EQ(g_command, command);
}
} // namespace OHOS::Ace::NG

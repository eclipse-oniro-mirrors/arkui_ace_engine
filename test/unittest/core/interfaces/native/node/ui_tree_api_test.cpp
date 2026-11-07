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
#include <cstdlib>
#include <deque>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/common/container_scope.h"
#include "core/components_ng/base/ui_tree_collector.h"
#include "core/interfaces/native/node/ui_tree_api.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEST_INSTANCE_ID = 17;
constexpr uint64_t REJECTED_REQUEST_ID = 42;
constexpr uint64_t BUSY_REQUEST_ID = 999;
constexpr uint64_t CLOSED_REQUEST_ID = 33;
constexpr int32_t INVALID_OPTION = 99;
constexpr int32_t UNDELIVERED_RESULT = -1;
constexpr uintptr_t OUTPUT_POINTER_SENTINEL = 1;
constexpr uint32_t OUTPUT_SIZE_SENTINEL = 99;
class TreeExecutor : public MockTaskExecutor {
public:
    bool uiThread = true;
    bool reject = false;
    mutable std::deque<Task> tasks;
    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return uiThread && type == TaskType::UI;
    }
    bool OnPostTask(Task&& task, TaskType, uint32_t, const std::string&, PriorityType = PriorityType::LOW,
        VsyncBarrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (reject) {
            return false;
        }
        tasks.push_back(std::move(task));
        return true;
    }
    void Drain()
    {
        while (!tasks.empty()) {
            auto task = std::move(tasks.front());
            tasks.pop_front();
            task();
        }
    }
};
struct Result {
    int count = 0;
    int32_t code = UNDELIVERED_RESULT;
    uint64_t id = 0;
    std::string json;
    std::function<void()> callback;
};
void UnusedPublicCallback(ArkUI_ContextHandle, uint64_t, ArkUI_ErrorCode, OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{}
void Deliver(const ArkUITreeCompletion& completion, uint64_t id, int32_t code, const char* data, uint32_t size)
{
    auto& result = *static_cast<Result*>(completion.userData);
    ++result.count;
    result.id = id;
    result.code = code;
    result.json = data ? std::string(data, size) : "";
    EXPECT_EQ(data == nullptr, code != ARKUI_ERROR_CODE_NO_ERROR);
    if (result.callback) {
        result.callback();
    }
}
ArkUITreeCompletion Completion(Result& result)
{
    return { nullptr, UnusedPublicCallback, &result, Deliver };
}
} // namespace

class UITreeApiTest : public Test {
protected:
    RefPtr<MockPipelineContext> pipeline;
    RefPtr<TreeExecutor> executor;
    int32_t instanceId = TEST_INSTANCE_ID;
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        pipeline = MockPipelineContext::GetCurrent();
        pipeline->SetThisInstanceId(instanceId);
        MockContainer::SetUp(pipeline);
        ON_CALL(*MockContainer::Current(), GetInstanceId()).WillByDefault(Return(instanceId));
        executor = AceType::MakeRefPtr<TreeExecutor>();
        pipeline->SetTaskExecutor(executor);
        MockContainer::Current()->SetTaskExecutor(executor);
    }
    void TearDown() override
    {
        pipeline->GetUITreeRequestState().pending = false;
        pipeline->GetUITreeRequestState().deliver = {};
        CloseUITreeRequests(*pipeline);
        executor->Drain();
        MockContainer::SetGetContainerCallback({});
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
        pipeline.Reset();
    }
};

HWTEST_F(UITreeApiTest, RejectedRequestDoesNotCallbackOrChangeId, TestSize.Level1)
{
    ArkUITreeOptions options;
    Result result;
    auto completion = Completion(result);
    uint64_t id = REJECTED_REQUEST_ID;
    executor->uiThread = false;
    EXPECT_EQ(SubmitUITree(instanceId, &options, &completion, &id), ARKUI_ERROR_CODE_NODE_ON_INVALID_THREAD);
    EXPECT_EQ(id, REJECTED_REQUEST_ID);
    executor->uiThread = true;
    options.treeType = INVALID_OPTION;
    EXPECT_EQ(SubmitUITree(instanceId, &options, &completion, &id), ARKUI_ERROR_CODE_PARAM_INVALID);
    options.treeType = OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL;
    EXPECT_EQ(SubmitUITree(instanceId + 1, &options, &completion, &id), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(result.count, 0);
    EXPECT_TRUE(executor->tasks.empty());
}

HWTEST_F(UITreeApiTest, AsyncSnapshotBusyAndExactlyOneCompletion, TestSize.Level1)
{
    ArkUITreeOptions options;
    options.visual = true;
    Result result;
    auto completion = Completion(result);
    uint64_t id = 0;
    ASSERT_EQ(SubmitUITree(instanceId, &options, &completion, &id), ARKUI_ERROR_CODE_NO_ERROR);
    options.visual = false;
    options.format = INVALID_OPTION;
    EXPECT_TRUE(pipeline->GetUITreeRequestState().visual);
    EXPECT_EQ(pipeline->GetUITreeRequestState().format, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT);
    ArkUITreeOptions next;
    uint64_t rejected = BUSY_REQUEST_ID;
    EXPECT_EQ(SubmitUITree(instanceId, &next, &completion, &rejected), ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    EXPECT_EQ(rejected, BUSY_REQUEST_ID);
    EXPECT_EQ(result.count, 0);
    executor->Drain();
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.code, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.id, id);
    EXPECT_NE(result.json.find("advanced"), std::string::npos);
    EXPECT_FALSE(pipeline->GetUITreeRequestState().pending);
    executor->Drain();
    EXPECT_EQ(result.count, 1);
}

HWTEST_F(UITreeApiTest, CompletionCanReenterSameContext, TestSize.Level1)
{
    ArkUITreeOptions options;
    Result first;
    Result second;
    auto firstCompletion = Completion(first);
    auto secondCompletion = Completion(second);
    uint64_t firstId = 0;
    uint64_t secondId = 0;
    first.callback = [this, &options, &secondCompletion, &secondId] {
        EXPECT_FALSE(pipeline->GetUITreeRequestState().pending);
        EXPECT_EQ(SubmitUITree(instanceId, &options, &secondCompletion, &secondId), ARKUI_ERROR_CODE_NO_ERROR);
    };
    ASSERT_EQ(SubmitUITree(instanceId, &options, &firstCompletion, &firstId), ARKUI_ERROR_CODE_NO_ERROR);
    executor->Drain();
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(second.count, 1);
    EXPECT_NE(firstId, secondId);
}

HWTEST_F(UITreeApiTest, CloseCompletesOnceAndStaleTaskDoesNotRepeat, TestSize.Level1)
{
    ArkUITreeOptions options;
    Result result;
    auto completion = Completion(result);
    uint64_t id = 0;
    ASSERT_EQ(SubmitUITree(instanceId, &options, &completion, &id), ARKUI_ERROR_CODE_NO_ERROR);
    result.callback = [this, &options, &completion] {
        uint64_t rejected = CLOSED_REQUEST_ID;
        EXPECT_EQ(SubmitUITree(instanceId, &options, &completion, &rejected), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
        EXPECT_EQ(rejected, CLOSED_REQUEST_ID);
    };
    CloseUITreeRequests(*pipeline);
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.code, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_TRUE(result.json.empty());
    executor->Drain();
    CloseUITreeRequests(*pipeline);
    EXPECT_EQ(result.count, 1);
}

HWTEST_F(UITreeApiTest, PostFailureIsAcceptedAndCompletesOnCallingUiThread, TestSize.Level1)
{
    executor->reject = true;
    ArkUITreeOptions options;
    Result result;
    auto completion = Completion(result);
    uint64_t id = 0;
    result.callback = [this, &result, &id] {
        EXPECT_EQ(result.id, id); // The accepted ID must be published before inline failure delivery.
        EXPECT_TRUE(executor->WillRunOnCurrentThread(TaskExecutor::TaskType::UI));
    };
    EXPECT_EQ(SubmitUITree(instanceId, &options, &completion, &id), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.code, ARKUI_ERROR_CODE_INTERNAL_ERROR);
    EXPECT_FALSE(pipeline->GetUITreeRequestState().pending);
}

HWTEST_F(UITreeApiTest, PendingIsPerPipelineAndOwnerReplacementFailsOldRequest, TestSize.Level1)
{
    auto otherPipeline = AceType::MakeRefPtr<MockPipelineContext>();
    otherPipeline->SetThisInstanceId(instanceId + 1);
    otherPipeline->SetTaskExecutor(executor);
    RefPtr<MockContainer> otherContainer = AceType::MakeRefPtr<NiceMock<MockContainer>>(otherPipeline);
    otherContainer->SetTaskExecutor(executor);
    ON_CALL(*otherContainer, GetInstanceId()).WillByDefault(Return(instanceId + 1));
    auto originalContainer = MockContainer::Current();
    MockContainer::SetGetContainerCallback([this, originalContainer, otherContainer](int32_t id) -> RefPtr<Container> {
        return id == instanceId ? originalContainer : otherContainer;
    });
    ArkUITreeOptions options;
    Result first;
    Result second;
    auto one = Completion(first);
    auto two = Completion(second);
    uint64_t id1 = 0;
    uint64_t id2 = 0;
    ASSERT_EQ(SubmitUITree(instanceId, &options, &one, &id1), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(SubmitUITree(instanceId + 1, &options, &two, &id2), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_NE(id1, id2);
    // Replace the original owner before its task runs, while keeping its state alive for completion.
    MockContainer::SetGetContainerCallback([otherContainer](int32_t) -> RefPtr<Container> { return otherContainer; });
    executor->Drain();
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(first.code, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(second.count, 1);
    EXPECT_EQ(second.code, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(second.json, R"({"schemaVersion":1,"treeInfo":null})");
    CloseUITreeRequests(*otherPipeline);
    MockContainer::SetGetContainerCallback({});
}

HWTEST_F(UITreeApiTest, SyncInvalidInputsClearOutputs, TestSize.Level1)
{
    ArkUITreeOptions options;
    char* data = reinterpret_cast<char*>(OUTPUT_POINTER_SENTINEL);
    uint32_t size = OUTPUT_SIZE_SENTINEL;
    EXPECT_EQ(CollectUITree(instanceId, nullptr, &data, &size), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(size, 0u);
    executor->uiThread = false;
    EXPECT_EQ(CollectUITree(instanceId, &options, &data, &size), ARKUI_ERROR_CODE_NODE_ON_INVALID_THREAD);
    EXPECT_EQ(data, nullptr);
    executor->uiThread = true;
    EXPECT_EQ(CollectUITree(instanceId + 1, &options, &data, &size), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(data, nullptr);
}
} // namespace OHOS::Ace::NG

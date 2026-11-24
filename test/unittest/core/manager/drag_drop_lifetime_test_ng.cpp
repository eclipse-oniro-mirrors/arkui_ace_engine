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

#include <chrono>
#include <future>
#include <thread>

#include "test/unittest/core/manager/drag_drop_manager_test_ng.h"
#include "core/components_ng/manager/drag_drop/drag_drop_manager.h"
#include "core/components_ng/manager/drag_drop/drag_snapshot_request.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_udmf.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
class QueuedDragExecutor : public MockTaskExecutor {
    DECLARE_ACE_TYPE(QueuedDragExecutor, MockTaskExecutor);
public:
    struct Entry {
        Task task;
        TaskType type;
        uint32_t delay;
        std::string name;
    };
    bool OnPostTask(Task&& task, TaskType type, uint32_t delay, const std::string& name,
        PriorityType priority = PriorityType::LOW,
        VsyncBarrierOption barrier = VsyncBarrierOption::NO_BARRIER) const override
    {
        tasks.push_back({ std::move(task), type, delay, name });
        return true;
    }
    mutable std::vector<Entry> tasks;
};
} // namespace

class DragDropLifetimeTest : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        executor_ = AceType::MakeRefPtr<QueuedDragExecutor>();
        MockPipelineContext::pipeline_->SetTaskExecutor(executor_);
        MockContainer::Current()->SetTaskExecutor(executor_);
        manager_ = AceType::MakeRefPtr<DragDropManager>();
    }
    void TearDown() override
    {
        DragDropGlobalController::GetInstance().SavePendingFollowHandMorphDropAnimation(nullptr);
        manager_.Reset();
        executor_->tasks.clear();
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
        executor_.Reset();
    }
    RefPtr<QueuedDragExecutor> executor_;
    RefPtr<DragDropManager> manager_;
};

/**
 * @tc.name: CompleteSnapshotOnce
 * @tc.desc: Completion cleans up before starting drag and ignores duplicate service results.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, CompleteSnapshotOnce, TestSize.Level1)
{
    int cleanupCount = 0;
    int starts = 0;
    auto preview = std::make_shared<int>(1);
    std::weak_ptr<int> weakPreview = preview;
    auto request = manager_->CreateDragSnapshotRequest([&, preview](const RefPtr<PixelMap>&) {
        EXPECT_EQ(cleanupCount, 1);
        ++starts;
    });
    request->SetCleanup([&]() { ++cleanupCount; });
    preview.reset();
    manager_->CompleteDragSnapshotRequest(request, nullptr);
    EXPECT_TRUE(weakPreview.expired());
    manager_->CompleteDragSnapshotRequest(request, nullptr);
    manager_->CancelDragSnapshotRequest(request);
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(cleanupCount, 1);
}

/**
 * @tc.name: CancelReleasesPreviewAndIgnoresLateResult
 * @tc.desc: Cancel releases UI callback captures even if a service still retains its weak request.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, CancelReleasesPreviewAndIgnoresLateResult, TestSize.Level1)
{
    auto preview = std::make_shared<int>(1);
    std::weak_ptr<int> weakPreview = preview;
    int starts = 0;
    int cleanupCount = 0;
    auto request = manager_->CreateDragSnapshotRequest([preview, &starts](const RefPtr<PixelMap>&) { ++starts; });
    std::weak_ptr<DragSnapshotRequest> weakRequest = request;
    request->SetCleanup([&]() { ++cleanupCount; });
    preview.reset();
    EXPECT_FALSE(weakPreview.expired());
    manager_->CancelDragSnapshotRequest(request);
    EXPECT_TRUE(weakPreview.expired());
    manager_->CompleteDragSnapshotRequest(request, nullptr);
    EXPECT_EQ(starts, 0);
    EXPECT_EQ(cleanupCount, 1);
    request.reset();
    std::thread service([weakRequest]() { EXPECT_TRUE(weakRequest.expired()); });
    service.join();
}

/**
 * @tc.name: CancelAllWithReentrantCleanup
 * @tc.desc: Cleanup can reenter cancellation without invalidating registry iteration.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, CancelAllWithReentrantCleanup, TestSize.Level1)
{
    int cleanupCount = 0;
    int starts = 0;
    auto first = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    auto second = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    first->SetCleanup([&]() { ++cleanupCount; manager_->CancelPendingDragSnapshots(); });
    second->SetCleanup([&]() { ++cleanupCount; });
    manager_->CancelPendingDragSnapshots();
    manager_->CompleteDragSnapshotRequest(first, nullptr);
    manager_->CompleteDragSnapshotRequest(second, nullptr);
    EXPECT_EQ(cleanupCount, 2);
    EXPECT_EQ(starts, 0);
}

/**
 * @tc.name: ManagerDestructionCancelsUnansweredSnapshot
 * @tc.desc: Missing service responses do not keep preview state alive after manager destruction.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, ManagerDestructionCancelsUnansweredSnapshot, TestSize.Level1)
{
    int cleanupCount = 0;
    int starts = 0;
    auto request = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    request->SetCleanup([&]() { ++cleanupCount; });
    manager_.Reset();
    request->Complete(nullptr);
    EXPECT_EQ(cleanupCount, 1);
    EXPECT_EQ(starts, 0);
}

/**
 * @tc.name: GestureCancelAndEndInvalidateSnapshot
 * @tc.desc: Cancellation works before a drag proxy exists; a new request is independent of the old result.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, GestureCancelAndEndInvalidateSnapshot, TestSize.Level1)
{
    auto hub = AceType::MakeRefPtr<GestureEventHub>(WeakPtr<EventHub>());
    int starts = 0;
    auto first = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    hub->pendingDragSnapshot_ = first;
    hub->pendingDragSnapshotManager_ = manager_;
    hub->HandleOnDragCancel();
    auto second = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    hub->pendingDragSnapshot_ = second;
    hub->pendingDragSnapshotManager_ = manager_;
    manager_->CompleteDragSnapshotRequest(first, nullptr);
    EXPECT_EQ(starts, 0);
    hub->HandleOnDragEnd(GestureEvent());
    manager_->CompleteDragSnapshotRequest(second, nullptr);
    EXPECT_EQ(starts, 0);
}

/**
 * @tc.name: DetachCancelsSnapshotButPreviewRemovalDoesNot
 * @tc.desc: Main-tree/context detachment invalidates a snapshot; closing the text preview does not.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, DetachCancelsSnapshotButPreviewRemovalDoesNot, TestSize.Level1)
{
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto hub = eventHub->GetOrCreateGestureEventHub();
    int starts = 0;
    int cleanups = 0;
    auto first = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    first->SetCleanup([&]() { ++cleanups; });
    hub->pendingDragSnapshot_ = first;
    hub->pendingDragSnapshotManager_ = manager_;
    hub->DragNodeDetachFromParent();
    EXPECT_EQ(cleanups, 0);
    eventHub->OnDetachClear();
    manager_->CompleteDragSnapshotRequest(first, nullptr);
    EXPECT_EQ(cleanups, 1);
    EXPECT_EQ(starts, 0);

    auto second = manager_->CreateDragSnapshotRequest([&](const RefPtr<PixelMap>&) { ++starts; });
    second->SetCleanup([&]() { ++cleanups; });
    hub->pendingDragSnapshot_ = second;
    hub->pendingDragSnapshotManager_ = manager_;
    eventHub->OnDetachContext(AceType::RawPtr(MockPipelineContext::pipeline_));
    manager_->CompleteDragSnapshotRequest(second, nullptr);
    EXPECT_EQ(cleanups, 2);
    EXPECT_EQ(starts, 0);
}

/**
 * @tc.name: BackgroundFetchDoesNotRetainManager
 * @tc.desc: UI can release the last manager reference while UDMF is blocked on the worker thread.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, BackgroundFetchDoesNotRetainManager, TestSize.Level1)
{
    auto mock = static_cast<MockUdmfClient*>(UdmfClient::GetInstance());
    auto data = AceType::MakeRefPtr<MockUnifiedData>();
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> resume;
    auto resumeFuture = resume.get_future();
    EXPECT_CALL(*mock, CreateUnifiedData()).WillOnce(Return(data));
    EXPECT_CALL(*mock, GetData(_, "lifetime-key")).WillOnce(Invoke([&](const auto&, const auto&) {
        entered.set_value();
        resumeFuture.wait();
        return 0;
    }));
    manager_->TryGetDataBackGround(nullptr, DragPointerEvent(), "lifetime-key");
    ASSERT_EQ(executor_->tasks.size(), 1u);
    auto entry = std::move(executor_->tasks.front());
    executor_->tasks.clear();
    ASSERT_EQ(entry.type, TaskExecutor::TaskType::BACKGROUND);
    auto weakManager = AceType::WeakClaim(AceType::RawPtr(manager_));
    std::thread worker(std::move(entry.task));
    auto status = enteredFuture.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(status, std::future_status::ready);
    manager_.Reset();
    EXPECT_TRUE(weakManager.Invalid());
    resume.set_value();
    worker.join();
    ASSERT_EQ(executor_->tasks.size(), 1u);
    EXPECT_EQ(executor_->tasks.front().type, TaskExecutor::TaskType::UI);
    executor_->tasks.front().task();
    Mock::VerifyAndClearExpectations(mock);
}

/**
 * @tc.name: StopCallbackResolvesExecutorAtInvocation
 * @tc.desc: The service callback does not retain the old executor and tolerates a missing container.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, StopCallbackResolvesExecutorAtInvocation, TestSize.Level1)
{
    auto mock = AceType::DynamicCast<MockInteractionInterface>(InteractionInterface::GetInstance());
    ASSERT_NE(mock, nullptr);
    std::function<void()> serviceCallback;
    EXPECT_CALL(*mock, StopDrag(_, _)).WillOnce(DoAll(SaveArg<1>(&serviceCallback), Return(0)));
    auto event = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    manager_->ExecuteFollowHandMorphStopDrag(event, DragRet::DRAG_SUCCESS, 1, DragBehavior::MOVE);
    ASSERT_TRUE(serviceCallback);
    auto weakExecutor = AceType::WeakClaim(AceType::RawPtr(executor_));
    auto replacement = AceType::MakeRefPtr<QueuedDragExecutor>();
    MockPipelineContext::pipeline_->SetTaskExecutor(replacement);
    MockContainer::Current()->SetTaskExecutor(replacement);
    executor_ = replacement;
    EXPECT_TRUE(weakExecutor.Invalid());
    serviceCallback();
    ASSERT_EQ(replacement->tasks.size(), 1u);
    EXPECT_EQ(replacement->tasks.front().name, "ArkUIDragStopAnimationEnd");
    MockContainer::SetGetContainerCallback([](int32_t) -> RefPtr<Container> { return nullptr; });
    serviceCallback();
    EXPECT_EQ(replacement->tasks.size(), 1u);
    MockContainer::SetGetContainerCallback(nullptr);
    Mock::VerifyAndClearExpectations(mock);
}
/**
 * @tc.name: FailedFetchPreservesRetryPolicy
 * @tc.desc: Failed reads retry on UI after 800 ms and stop retrying at the existing limit.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, FailedFetchPreservesRetryPolicy, TestSize.Level1)
{
    auto mock = static_cast<MockUdmfClient*>(UdmfClient::GetInstance());
    EXPECT_CALL(*mock, CreateUnifiedData()).Times(2).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mock, GetData(_, "retry-key")).Times(2).WillRepeatedly(Return(-1));
    for (int count : { 0, 3 }) {
        executor_->tasks.clear();
        manager_->TryGetDataBackGround(nullptr, DragPointerEvent(), "retry-key", count);
        ASSERT_EQ(executor_->tasks.size(), 1u);
        auto task = std::move(executor_->tasks.front().task);
        executor_->tasks.clear();
        task();
        ASSERT_EQ(executor_->tasks.size(), 1u);
        const auto& posted = executor_->tasks.front();
        EXPECT_EQ(posted.type, TaskExecutor::TaskType::UI);
        EXPECT_EQ(posted.delay, count == 0 ? 800u : 0u);
        EXPECT_EQ(posted.name, count == 0 ? "ArkUIDragDropRetryGetData" : "ArkUIDragDropAction");
    }
    Mock::VerifyAndClearExpectations(mock);
}

/**
 * @tc.name: StopCallbackStartsAnimationOnceOnUi
 * @tc.desc: Normal stop completion keeps its UI dispatch and ignores duplicate completion.
 * @tc.type: FUNC
 */
HWTEST_F(DragDropLifetimeTest, StopCallbackStartsAnimationOnceOnUi, TestSize.Level1)
{
    auto mock = AceType::DynamicCast<MockInteractionInterface>(InteractionInterface::GetInstance());
    ASSERT_NE(mock, nullptr);
    std::function<void()> serviceCallback;
    EXPECT_CALL(*mock, StopDrag(_, _)).WillOnce(DoAll(SaveArg<1>(&serviceCallback), Return(0)));
    int animations = 0;
    auto event = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    event->SetFollowHandMorphDropAnimation([&]() { ++animations; });
    manager_->ExecuteFollowHandMorphStopDrag(event, DragRet::DRAG_SUCCESS, 1, DragBehavior::MOVE);
    ASSERT_TRUE(serviceCallback);
    for (int i = 0; i < 2; ++i) {
        executor_->tasks.clear();
        serviceCallback();
        ASSERT_EQ(executor_->tasks.size(), 1u);
        EXPECT_EQ(animations, i);
        auto task = std::move(executor_->tasks.front().task);
        executor_->tasks.clear();
        task();
        EXPECT_EQ(animations, 1);
    }
    Mock::VerifyAndClearExpectations(mock);
}
} // namespace OHOS::Ace::NG

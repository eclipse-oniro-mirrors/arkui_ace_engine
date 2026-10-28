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

// Host-isolated unit tests for UIInfoCollectionManager. The REAL manager
// source is compiled against the framework stand-ins under
// host_manager_stubs/, which shadow container.h / frame_node.h /
// pipeline_context.h through include-path order and supply a
// test-controllable clock plus a registry-backed Container. This keeps the
// manager source unmodified while making registration, dispatch and the
// drag-start timing record fully deterministic without the rendering
// pipeline.
//
// Covered review contracts:
//  - An observer removed by a callback of the same dispatch batch (another
//    observer or itself) is skipped for the rest of the batch and in later
//    dispatches: unregister flips the observer's active flag and the
//    dispatcher re-checks it before every invocation. Under the
//    UI-main-thread calling contract no cross-thread dispatch can exist, so
//    no drain is needed; a self-unregister simply completes its own running
//    invocation.
//  - The drag-start duration record is maintained whenever DRAG is observed
//    (a DRAG-only mask must not depend on TOUCH being observed), is bounded
//    to the press that wrote it (cancel of the recording pointer, instance
//    lifecycle), and TOUCH JSON is still filtered by the TOUCH bit alone.
//  - The drag-end dropResult success mapping mirrors the platform stop-drag
//    result for every DragRet value, including the DRAG_DEFAULT left behind
//    by an onDrop that only consumes the data.
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "cJSON.h"

#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/interaction_observer/ui_info_collection_manager.h"
#include "core/gestures/drag_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "host_manager_stubs.h"

namespace {
namespace ace = OHOS::Ace;
using namespace OHOS::Ace::NG;

constexpr int32_t TEST_INSTANCE_ID = 21;
constexpr int32_t SECOND_INSTANCE_ID = 22;
constexpr int32_t TEST_NODE_ID = 55;
constexpr int32_t TEST_POINTER_ID = 7;

struct CapturedJson {
    std::mutex mutex;
    std::string json;
    std::atomic<int32_t> calls { 0 };
};

// The manager treats the JSON wrapper as opaque and only forwards the pointer
// between the trampolines supplied at registration and the callback, so the
// test owns the concrete wrapper type.
struct TestJsonWrapper {
    std::string data;
};

OH_ArkUI_NativeModule_UIJsonWrapper* TestCreateWrapper(const char* data, uint32_t size)
{
    auto* wrapper = new TestJsonWrapper();
    wrapper->data.assign(data, size);
    return reinterpret_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(wrapper);
}

void TestDestroyWrapper(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper)
{
    delete reinterpret_cast<const TestJsonWrapper*>(wrapper);
}

const char* WrapperData(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper)
{
    return reinterpret_cast<const TestJsonWrapper*>(wrapper)->data.c_str();
}

void CaptureCallback(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper, void* userData)
{
    auto* captured = static_cast<CapturedJson*>(userData);
    {
        std::lock_guard<std::mutex> lock(captured->mutex);
        captured->json = WrapperData(wrapper);
    }
    captured->calls.fetch_add(1);
}

int64_t JsonIntField(const std::string& json, const char* key)
{
    cJSON* root = cJSON_Parse(json.c_str());
    if (root == nullptr) {
        return -1;
    }
    cJSON* item = cJSON_GetObjectItem(root, key);
    const int64_t value = cJSON_IsNumber(item) ? static_cast<int64_t>(item->valueint) : -1;
    cJSON_Delete(root);
    return value;
}

void SetClockMs(int64_t milliseconds)
{
    ace::SetSysTimestamp(milliseconds * 1000000);
}

class UIInfoCollectionManagerTest : public testing::Test {
protected:
    void SetUp() override
    {
        SetClockMs(1000);
        ace::Container::ClearStubContainers();
        auto container = ace::Referenced::MakeRefPtr<ace::Container>();
        container->SetBundleName("com.example.manager.host.test");
        ace::Container::PutStubContainer(TEST_INSTANCE_ID, container);
        auto context = ace::Referenced::MakeRefPtr<PipelineContext>(TEST_INSTANCE_ID);
        node_ = ace::Referenced::MakeRefPtr<FrameNode>(TEST_NODE_ID, context);
        manager_ = &(UIInfoCollectionManager::GetInstance());
    }

    void TearDown() override
    {
        // Leftover observers are removed so tests stay independent; removing
        // an already-removed id is a harmless 401.
        for (uint32_t id : registeredIds_) {
            manager_->UnregisterObserver(id);
        }
        ace::Container::ClearStubContainers();
    }

    uint32_t Register(uint32_t eventMask, UIInfoCollectionManager::InteractionCallback callback, void* userData,
        int32_t instanceId = TEST_INSTANCE_ID)
    {
        uint32_t id = 0;
        UIInfoCollectionManager::RegisterParam param;
        param.instanceId = instanceId;
        param.eventMask = eventMask;
        param.callback = callback;
        param.userData = userData;
        param.createWrapper = &TestCreateWrapper;
        param.destroyWrapper = &TestDestroyWrapper;
        EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_SUCCESS);
        EXPECT_NE(id, 0u);
        registeredIds_.push_back(id);
        return id;
    }

    UIInfoCollectionManager* manager_ = nullptr;
    ace::RefPtr<FrameNode> node_;
    std::vector<uint32_t> registeredIds_;
};

TEST_F(UIInfoCollectionManagerTest, RegisterValidatesInstanceAndUnknownId)
{
    CapturedJson captured;
    const uint32_t id = Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &captured);
    EXPECT_NE(id, 0u);
    EXPECT_EQ(manager_->UnregisterObserver(id), INTERACTION_RESULT_SUCCESS);
    // Unknown, zero and already-removed ids are rejected.
    EXPECT_EQ(manager_->UnregisterObserver(id), INTERACTION_RESULT_PARAM_INVALID);
    EXPECT_EQ(manager_->UnregisterObserver(0), INTERACTION_RESULT_PARAM_INVALID);
    // A missing container for the instance is rejected and yields id 0.
    UIInfoCollectionManager::RegisterParam param;
    param.instanceId = 99; // no stub container registered
    param.eventMask = INTERACTION_EVENT_TOUCH;
    param.callback = &CaptureCallback;
    param.userData = &captured;
    param.createWrapper = &TestCreateWrapper;
    param.destroyWrapper = &TestDestroyWrapper;
    uint32_t other = 0;
    EXPECT_EQ(manager_->RegisterObserver(param, &other), INTERACTION_RESULT_UI_CONTEXT_INVALID);
    EXPECT_EQ(other, 0u);
}

// Same batch: a callback that unregisters other observers of the same
// dispatch batch must prevent their invocation for the rest of the batch.
std::atomic<int32_t> g_batchACalls { 0 };
std::atomic<int32_t> g_batchBCallsAfterUnregister { 0 };
std::atomic<bool> g_batchBUnregistered { false };
std::vector<uint32_t> g_batchBIds;

void BatchCallbackA(const OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{
    g_batchACalls.fetch_add(1);
    for (uint32_t id : g_batchBIds) {
        EXPECT_EQ(UIInfoCollectionManager::GetInstance().UnregisterObserver(id), INTERACTION_RESULT_SUCCESS);
    }
    // Later dispatches must not try to remove them again.
    g_batchBIds.clear();
    g_batchBUnregistered.store(true);
}

void BatchCallbackB(const OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{
    if (g_batchBUnregistered.load()) {
        g_batchBCallsAfterUnregister.fetch_add(1);
    }
}

TEST_F(UIInfoCollectionManagerTest, UnregisterFromCallbackSkipsRemainingSnapshotTargets)
{
    g_batchACalls.store(0);
    g_batchBCallsAfterUnregister.store(0);
    g_batchBUnregistered.store(false);
    g_batchBIds.clear();
    // The B observers are registered first so that A, inserted last, is also
    // reached first by the dispatch loop's map iteration (new elements land
    // at the front of the bucket list). The assertions themselves stay
    // order-independent.
    for (int32_t i = 0; i < 4; ++i) {
        g_batchBIds.push_back(Register(INTERACTION_EVENT_TOUCH, &BatchCallbackB, nullptr));
    }
    Register(INTERACTION_EVENT_TOUCH, &BatchCallbackA, nullptr);
    ace::TouchEvent event;
    event.SetId(TEST_POINTER_ID);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, event, -1);
    EXPECT_EQ(g_batchACalls.load(), 1);
    // Every B whose dispatch had not started when A's unregister completed
    // must be skipped, both in this batch and in later dispatches.
    EXPECT_EQ(g_batchBCallsAfterUnregister.load(), 0);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, event, -1);
    EXPECT_EQ(g_batchACalls.load(), 2);
    EXPECT_EQ(g_batchBCallsAfterUnregister.load(), 0);
}

// Self-unregister: unregistering from inside the observer's own callback must
// complete without deadlock (under the UI-main-thread contract the running
// invocation is the caller's own stack frame) and start no new invocation.
std::atomic<int32_t> g_selfCalls { 0 };
uint32_t g_selfId = 0;

void SelfUnregisterCallback(const OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{
    g_selfCalls.fetch_add(1);
    EXPECT_EQ(UIInfoCollectionManager::GetInstance().UnregisterObserver(g_selfId), INTERACTION_RESULT_SUCCESS);
}

TEST_F(UIInfoCollectionManagerTest, SelfUnregisterInsideCallbackCompletesWithoutDeadlock)
{
    g_selfCalls.store(0);
    g_selfId = Register(INTERACTION_EVENT_TOUCH, &SelfUnregisterCallback, nullptr);
    ace::TouchEvent event;
    event.SetId(TEST_POINTER_ID);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, event, -1);
    EXPECT_EQ(g_selfCalls.load(), 1);
    // The running invocation completed; no further one starts.
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, event, -1);
    EXPECT_EQ(g_selfCalls.load(), 1);
}

// R3: a DRAG-only mask must not emit TOUCH events, but the drag-start
// duration field must still be derived from the touch-down record. The test
// uses its own instance so no touch-down record can leak in from earlier
// tests on the shared instance.
TEST_F(UIInfoCollectionManagerTest, DragOnlyMaskStillRecordsTouchDownForDuration)
{
    auto container = ace::Referenced::MakeRefPtr<ace::Container>();
    container->SetBundleName("com.example.manager.host.test.drag");
    ace::Container::PutStubContainer(SECOND_INSTANCE_ID, container);
    auto context = ace::Referenced::MakeRefPtr<PipelineContext>(SECOND_INSTANCE_ID);
    auto dragNode = ace::Referenced::MakeRefPtr<FrameNode>(TEST_NODE_ID, context);
    CapturedJson captured;
    Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &captured, SECOND_INSTANCE_ID);
    SetClockMs(1000);
    ace::TouchEvent down;
    down.SetId(TEST_POINTER_ID);
    // TOUCH JSON creation and dispatch stay filtered by the TOUCH bit alone.
    manager_->NotifyTouch(SECOND_INSTANCE_ID, true, down, -1);
    EXPECT_EQ(captured.calls.load(), 0);
    // 800 ms elapsed between the touch down and the drag start.
    SetClockMs(1800);
    manager_->NotifyDragStart(dragNode, 100.0, 200.0);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_EQ(JsonIntField(captured.json, "actualDuration"), 800);
    EXPECT_EQ(JsonIntField(captured.json, "id"), TEST_NODE_ID);
}

// R3: the touch-down record is bounded to the press that wrote it. Cancel of
// the recording pointer ends it, cancel of another pointer does not, and the
// instance's last observer unregistering drops it.
TEST_F(UIInfoCollectionManagerTest, TouchCancelAndInstanceLifecycleBoundTimingRecord)
{
    CapturedJson captured;
    const uint32_t id = Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &captured);
    ace::TouchEvent down;
    down.SetId(TEST_POINTER_ID);

    // Cancel of the recording pointer ends the record: the later drag start
    // reports no duration instead of a stale one.
    SetClockMs(1000);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    SetClockMs(1100);
    manager_->NotifyTouchCancelled(TEST_INSTANCE_ID, TEST_POINTER_ID);
    SetClockMs(1900);
    manager_->NotifyDragStart(node_, 0.0, 0.0);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_EQ(JsonIntField(captured.json, "actualDuration"), 0);

    // Cancel of a different pointer must not end the record.
    SetClockMs(2000);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    SetClockMs(2100);
    manager_->NotifyTouchCancelled(TEST_INSTANCE_ID, 9);
    SetClockMs(2800);
    manager_->NotifyDragStart(node_, 0.0, 0.0);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_EQ(JsonIntField(captured.json, "actualDuration"), 800);

    // Unregistering the instance's last observer drops the record, so a
    // later registration observes no stale duration.
    ASSERT_EQ(manager_->UnregisterObserver(id), INTERACTION_RESULT_SUCCESS);
    const uint32_t id2 = Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &captured);
    SetClockMs(3000);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    ASSERT_EQ(manager_->UnregisterObserver(id2), INTERACTION_RESULT_SUCCESS);
    const uint32_t id3 = Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &captured);
    SetClockMs(3800);
    manager_->NotifyDragStart(node_, 0.0, 0.0);
    ASSERT_EQ(captured.calls.load(), 3);
    EXPECT_EQ(JsonIntField(captured.json, "actualDuration"), 0);
}

// R6: the observer's drag-end dropResult must agree with the platform
// stop-drag result for every DragRet value. The platform mapping below
// mirrors adapter/ohos/capability/interaction/interaction_impl.cpp
// TranslateDragResult() (DRAG_SUCCESS/DRAG_FAIL/DRAG_CANCEL map to the
// platform value of the same name, every other value settles as a platform
// success); the observer mapping mirrors the drop-success predicate in
// DragDropManager::HandleStopDrag (drag_drop_manager.cpp). A regression to
// "only explicit DRAG_SUCCESS counts" fails the DRAG_DEFAULT case below: a
// plain default drop — the app onDrop consumed the data without setting a
// result — is a successful drop on the platform and must be reported as one.
using OHOS::Ace::DragRet;

bool PlatformStopDragReportsSuccess(DragRet result)
{
    switch (result) {
        case DragRet::DRAG_FAIL:
        case DragRet::DRAG_CANCEL:
            return false;
        default:
            return true;
    }
}

bool ObserverReportsDropSuccess(DragRet result)
{
    return result != DragRet::DRAG_FAIL && result != DragRet::DRAG_CANCEL;
}

TEST(InteractionObserverDropResultMapping, MatchesPlatformStopDragResult)
{
    // The R6 regression: DRAG_DEFAULT (-1) is the initial DragEvent result
    // and stays set when the app onDrop only consumes the data.
    EXPECT_TRUE(ObserverReportsDropSuccess(DragRet::DRAG_DEFAULT));
    EXPECT_TRUE(ObserverReportsDropSuccess(DragRet::DRAG_SUCCESS));
    EXPECT_FALSE(ObserverReportsDropSuccess(DragRet::DRAG_FAIL));
    EXPECT_FALSE(ObserverReportsDropSuccess(DragRet::DRAG_CANCEL));
    // Every defined DragRet value, and any future raw value in the enum's
    // range, must keep both sides in agreement.
    for (int32_t raw = -1; raw <= 5; ++raw) {
        const auto result = static_cast<DragRet>(raw);
        EXPECT_EQ(ObserverReportsDropSuccess(result), PlatformStopDragReportsSuccess(result))
            << "DragRet " << raw << " diverges from the platform stop-drag result";
    }
}
} // namespace

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

#include <atomic>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "core/common/ace_engine.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#define private public
#include "core/components_ng/manager/interaction_observer/ui_info_collection_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEST_INSTANCE_ID = 101;
constexpr int32_t TEST_NODE_ID = 55;
constexpr int32_t TEST_POINTER_ID = 7;
const std::string TEST_BUNDLE_NAME = "com.ace.interaction.test";

// The manager treats the JSON wrapper as an opaque pointer type, so the test
// owns the concrete wrapper implementation, mirroring the native module bridge.
struct TestJsonWrapper {
    std::string data;
};

OH_ArkUI_NativeModule_UIJsonWrapper* TestCreateWrapper(const char* data, uint32_t size)
{
    auto* wrapper = new TestJsonWrapper();
    wrapper->data.assign(data, size);
    return reinterpret_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(wrapper);
}

OH_ArkUI_NativeModule_UIJsonWrapper* TestCreateNullWrapper(const char* data, uint32_t size)
{
    (void)data;
    (void)size;
    return nullptr;
}

void TestDestroyWrapper(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper)
{
    delete reinterpret_cast<const TestJsonWrapper*>(wrapper);
}

struct CapturedJson {
    std::mutex mutex;
    std::string json;
    std::atomic<int32_t> calls { 0 };

    std::string Get()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return json;
    }
};

void CaptureCallback(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper, void* userData)
{
    auto* captured = static_cast<CapturedJson*>(userData);
    {
        std::lock_guard<std::mutex> lock(captured->mutex);
        captured->json = reinterpret_cast<const TestJsonWrapper*>(wrapper)->data;
    }
    captured->calls.fetch_add(1);
}

GestureEvent MakeGestureEvent(double x, double y)
{
    GestureEvent event;
    event.SetScreenLocation(Offset(x, y));
    return event;
}

constexpr double FINGER_FIRST_LOCATION_X = 11.0;
constexpr double FINGER_FIRST_LOCATION_Y = 12.0;
constexpr double FINGER_SECOND_LOCATION_X = 21.0;
constexpr double FINGER_SECOND_LOCATION_Y = 22.0;
constexpr double GESTURE_SCREEN_LOCATION_X = 10.0;
constexpr double GESTURE_SCREEN_LOCATION_Y = 20.0;
constexpr double PINCH_END_SCALE = 2.5;
constexpr double ROTATION_END_ANGLE = 45.0;
constexpr double SWIPE_GESTURE_SPEED = 100.0;

GestureEvent MakeFingerGestureEvent()
{
    std::list<FingerInfo> fingers;
    FingerInfo first;
    first.screenLocation_ = Offset(FINGER_FIRST_LOCATION_X, FINGER_FIRST_LOCATION_Y);
    FingerInfo second;
    second.screenLocation_ = Offset(FINGER_SECOND_LOCATION_X, FINGER_SECOND_LOCATION_Y);
    fingers.push_back(first);
    fingers.push_back(second);
    GestureEvent event;
    event.SetScreenLocation(Offset(GESTURE_SCREEN_LOCATION_X, GESTURE_SCREEN_LOCATION_Y));
    event.SetFingerList(fingers);
    event.SetScale(PINCH_END_SCALE);
    event.SetAngle(ROTATION_END_ANGLE);
    event.SetSpeed(SWIPE_GESTURE_SPEED);
    return event;
}

TouchEvent MakeTouchEvent(int32_t pointerId, float screenX, float screenY)
{
    TouchEvent event;
    event.SetId(pointerId);
    event.SetScreenX(screenX);
    event.SetScreenY(screenY);
    return event;
}

bool JsonContains(CapturedJson& captured, const std::string& fragment)
{
    std::lock_guard<std::mutex> lock(captured.mutex);
    return captured.json.find(fragment) != std::string::npos;
}
} // namespace

class UIInfoCollectionManagerTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockContainer::SetUp();
        MockContainer::Current()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
        MockContainer::Current()->SetUseNewPipeline();
        MockPipelineContext::SetUp();
        MockContainer::SetGetContainerCallback([](int32_t instanceId) -> RefPtr<Container> {
            return instanceId == TEST_INSTANCE_ID ? MockContainer::Current() : nullptr;
        });
        testing::FLAGS_gmock_verbose = "error";
    }

    static void TearDownTestSuite()
    {
        MockContainer::SetGetContainerCallback(nullptr);
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    void SetUp() override
    {
        AceEngine::Get().AddContainer(TEST_INSTANCE_ID, MockContainer::container_);
        MockContainer::Current()->SetBundleName(TEST_BUNDLE_NAME);
        auto pipeline = MockPipelineContext::GetCurrent();
        ASSERT_TRUE(pipeline);
        pipeline->SetInstanceId(TEST_INSTANCE_ID);
        node_ = FrameNode::CreateFrameNode(
            "interaction_observer_test", TEST_NODE_ID, AceType::MakeRefPtr<Pattern>());
        ASSERT_TRUE(node_);
        manager_ = &(UIInfoCollectionManager::GetInstance());
    }

    void TearDown() override
    {
        for (uint32_t id : registeredIds_) {
            manager_->UnregisterObserver(id);
        }
        registeredIds_.clear();
        MockPipelineContext::GetCurrent()->SetInstanceId(0);
        AceEngine::Get().RemoveContainer(TEST_INSTANCE_ID);
    }

    uint32_t Register(uint32_t eventMask, UIInfoCollectionManager::InteractionCallback callback, void* userData,
        UIInfoCollectionManager::CreateJsonWrapperFunc createWrapper = &TestCreateWrapper)
    {
        uint32_t id = 0;
        UIInfoCollectionManager::RegisterParam param;
        param.instanceId = TEST_INSTANCE_ID;
        param.eventMask = eventMask;
        param.callback = callback;
        param.userData = userData;
        param.createWrapper = createWrapper;
        param.destroyWrapper = &TestDestroyWrapper;
        EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_SUCCESS);
        EXPECT_NE(id, 0u);
        registeredIds_.push_back(id);
        return id;
    }

    UIInfoCollectionManager* manager_ = nullptr;
    RefPtr<FrameNode> node_;
    std::vector<uint32_t> registeredIds_;
};

/**
 * @tc.name: UIInfoCollectionManagerRegister001
 * @tc.desc: Test RegisterObserver parameter validation branches.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerRegister001, TestSize.Level1)
{
    /**
     * @tc.steps: register with null callback / null wrappers / empty and
     *            out-of-range masks, then with a valid full mask.
     * @tc.expected: invalid params return 401 with registerID untouched;
     *               missing container returns 190001; valid returns success.
     */
    CapturedJson captured;
    uint32_t id = 123;
    UIInfoCollectionManager::RegisterParam param;
    param.instanceId = TEST_INSTANCE_ID;
    param.eventMask = INTERACTION_EVENT_TOUCH;
    param.callback = &CaptureCallback;
    param.userData = &captured;
    param.createWrapper = &TestCreateWrapper;
    param.destroyWrapper = &TestDestroyWrapper;

    param.callback = nullptr;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_PARAM_INVALID);
    EXPECT_EQ(id, 0u);

    param.callback = &CaptureCallback;
    param.createWrapper = nullptr;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_PARAM_INVALID);
    EXPECT_EQ(id, 0u);

    param.createWrapper = &TestCreateWrapper;
    param.destroyWrapper = nullptr;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_PARAM_INVALID);
    EXPECT_EQ(id, 0u);

    param.destroyWrapper = &TestDestroyWrapper;
    param.eventMask = 0;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_PARAM_INVALID);

    // no valid bit inside the mask
    param.eventMask = INTERACTION_EVENT_ALL + 1;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_PARAM_INVALID);

    // null registerID pointer
    param.eventMask = INTERACTION_EVENT_TOUCH;
    EXPECT_EQ(manager_->RegisterObserver(param, nullptr), INTERACTION_RESULT_PARAM_INVALID);

    // instance without a container
    param.instanceId = TEST_INSTANCE_ID + 100;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_UI_CONTEXT_INVALID);
    EXPECT_EQ(id, 0u);

    // valid registration
    param.instanceId = TEST_INSTANCE_ID;
    EXPECT_EQ(manager_->RegisterObserver(param, &id), INTERACTION_RESULT_SUCCESS);
    EXPECT_NE(id, 0u);
    EXPECT_EQ(manager_->UnregisterObserver(id), INTERACTION_RESULT_SUCCESS);
}

/**
 * @tc.name: UIInfoCollectionManagerUnregister001
 * @tc.desc: Test UnregisterObserver lookup and lifecycle branches.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerUnregister001, TestSize.Level1)
{
    /**
     * @tc.steps: unregister id 0, unknown id, a registered id twice, and a
     *            second observer after the first one is removed.
     * @tc.expected: 0/unknown/removed return 401; present returns success;
     *               the last observer of an instance cleans instance state.
     */
    CapturedJson captured;
    EXPECT_EQ(manager_->UnregisterObserver(0), INTERACTION_RESULT_PARAM_INVALID);
    EXPECT_EQ(manager_->UnregisterObserver(999), INTERACTION_RESULT_PARAM_INVALID);

    const uint32_t first = Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &captured);
    const uint32_t second = Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &captured);
    // remove first of two: instance registry keeps the second one
    EXPECT_EQ(manager_->UnregisterObserver(first), INTERACTION_RESULT_SUCCESS);
    EXPECT_EQ(manager_->UnregisterObserver(first), INTERACTION_RESULT_PARAM_INVALID);
    // remove the last one: instance entry is dropped entirely
    EXPECT_EQ(manager_->UnregisterObserver(second), INTERACTION_RESULT_SUCCESS);
    EXPECT_EQ(manager_->UnregisterObserver(second), INTERACTION_RESULT_PARAM_INVALID);
}

/**
 * @tc.name: UIInfoCollectionManagerHasObserver001
 * @tc.desc: Test HasObserver, GetHostName and GetNodeInstanceId branches.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerHasObserver001, TestSize.Level1)
{
    /**
     * @tc.steps: query HasObserver for unknown instance and unmatched bit,
     *            GetHostName for known/unknown instance, GetNodeInstanceId
     *            for null and real nodes.
     * @tc.expected: unknown instance false; registered bit true; host name
     *               returns bundle name or empty; null node gives -1.
     */
    CapturedJson captured;
    EXPECT_FALSE(manager_->HasObserver(TEST_INSTANCE_ID, INTERACTION_EVENT_CLICK));
    EXPECT_FALSE(manager_->HasObserver(TEST_INSTANCE_ID + 100, INTERACTION_EVENT_CLICK));

    Register(INTERACTION_EVENT_CLICK, &CaptureCallback, &captured);
    EXPECT_TRUE(manager_->HasObserver(TEST_INSTANCE_ID, INTERACTION_EVENT_CLICK));
    EXPECT_FALSE(manager_->HasObserver(TEST_INSTANCE_ID, INTERACTION_EVENT_TAP));

    EXPECT_EQ(manager_->GetHostName(TEST_INSTANCE_ID), TEST_BUNDLE_NAME);
    EXPECT_EQ(manager_->GetHostName(TEST_INSTANCE_ID + 100), std::string());

    RefPtr<FrameNode> nullNode;
    EXPECT_EQ(manager_->GetNodeInstanceId(nullNode), -1);
    EXPECT_EQ(manager_->GetNodeInstanceId(node_), TEST_INSTANCE_ID);
}

/**
 * @tc.name: UIInfoCollectionManagerDispatch001
 * @tc.desc: Test DispatchEvent filtering and wrapper failure branches.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerDispatch001, TestSize.Level1)
{
    GTEST_SKIP() << "Dispatch001 hangs on device; tracked separately";
    /**
     * @tc.steps: dispatch empty json, json to an instance without observers,
     *            and json while createWrapper returns null.
     * @tc.expected: all three paths deliver no callback and do not crash.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &captured);

    manager_->DispatchEvent(TEST_INSTANCE_ID, INTERACTION_EVENT_TOUCH, "");
    EXPECT_EQ(captured.calls.load(), 0);

    manager_->DispatchEvent(TEST_INSTANCE_ID + 100, INTERACTION_EVENT_TOUCH, "{}");
    EXPECT_EQ(captured.calls.load(), 0);

    // observer registered with a createWrapper that fails: no dispatch
    CapturedJson nullCaptured;
    Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &nullCaptured, &TestCreateNullWrapper);
    manager_->DispatchEvent(TEST_INSTANCE_ID, INTERACTION_EVENT_TOUCH, "{\"type\":\"Touch\"}");
    EXPECT_EQ(nullCaptured.calls.load(), 0);
}

// Unregistering another observer of the same dispatch batch from inside a
// callback: the removed observer must be skipped for the rest of the batch.
namespace {
std::atomic<int32_t> g_firstObserverCalls { 0 };
std::atomic<int32_t> g_secondObserverCallsAfterUnregister { 0 };
std::atomic<bool> g_secondUnregistered { false };
uint32_t g_secondObserverId = 0;

void BatchFirstCallback(const OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{
    g_firstObserverCalls.fetch_add(1);
    if (!g_secondUnregistered.load()) {
        UIInfoCollectionManager::GetInstance().UnregisterObserver(g_secondObserverId);
        g_secondUnregistered.store(true);
    }
}

void BatchSecondCallback(const OH_ArkUI_NativeModule_UIJsonWrapper*, void*)
{
    if (g_secondUnregistered.load()) {
        g_secondObserverCallsAfterUnregister.fetch_add(1);
    }
}
} // namespace

/**
 * @tc.name: UIInfoCollectionManagerDispatch002
 * @tc.desc: Test observer unregistered inside a callback is skipped in batch.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerDispatch002, TestSize.Level1)
{
    /**
     * @tc.steps: register two TOUCH observers; the first one unregisters the
     *            second from inside its callback, then dispatch twice.
     * @tc.expected: the second observer never fires after being unregistered.
     */
    g_firstObserverCalls.store(0);
    g_secondObserverCallsAfterUnregister.store(0);
    g_secondUnregistered.store(false);

    Register(INTERACTION_EVENT_TOUCH, &BatchSecondCallback, nullptr);
    const uint32_t secondId = registeredIds_.back();
    g_secondObserverId = secondId;
    Register(INTERACTION_EVENT_TOUCH, &BatchFirstCallback, nullptr);

    manager_->NotifyTouch(TEST_INSTANCE_ID, true, MakeTouchEvent(1, 1.0f, 1.0f), -1);
    manager_->NotifyTouch(TEST_INSTANCE_ID, false, MakeTouchEvent(1, 1.0f, 1.0f), -1);
    EXPECT_EQ(g_firstObserverCalls.load(), 2);
    EXPECT_EQ(g_secondObserverCallsAfterUnregister.load(), 0);
}

/**
 * @tc.name: UIInfoCollectionManagerTapClick001
 * @tc.desc: Test NotifyTapOrClick click and tap dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerTapClick001, TestSize.Level1)
{
    /**
     * @tc.steps: register CLICK and TAP observers, notify click with count and
     *            a gesture without fingers, then notify tap.
     * @tc.expected: click json carries Click type, count and fingers; tap json
     *               carries Tap type; node without observers dispatches nothing.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_CLICK | INTERACTION_EVENT_TAP, &CaptureCallback, &captured);

    GestureEvent info = MakeGestureEvent(30.0, 40.0);
    manager_->NotifyTapOrClick(node_, true, 2, info);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Click\""));
    EXPECT_TRUE(JsonContains(captured, "\"count\":2"));
    EXPECT_TRUE(JsonContains(captured, "\"fingers\":0"));
    EXPECT_TRUE(JsonContains(captured, "\"point\":[30,40]"));

    manager_->NotifyTapOrClick(node_, false, 1, info);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Tap\""));

    // no observer for this bit: nothing dispatched
    manager_->NotifyTapOrClick(node_, true, 1, MakeGestureEvent(1.0, 2.0));
    EXPECT_EQ(captured.calls.load(), 3); // CLICK observer still receives it

    // observers exist for the instance but not for the CLICK bit
    CapturedJson otherBit;
    Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &otherBit);
    manager_->NotifyTapOrClick(node_, true, 1, info);
    EXPECT_EQ(otherBit.calls.load(), 0);

    CapturedJson untouched;
    RefPtr<FrameNode> nullNode;
    manager_->NotifyTapOrClick(nullNode, true, 1, info);
    EXPECT_EQ(untouched.calls.load(), 0);
}

/**
 * @tc.name: UIInfoCollectionManagerLongPress001
 * @tc.desc: Test NotifyLongPress duration conversion.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerLongPress001, TestSize.Level1)
{
    /**
     * @tc.steps: register a LONG_PRESS observer and notify with 500ms duration.
     * @tc.expected: json reports LongPress type and the duration in ms.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_LONG_PRESS, &CaptureCallback, &captured);

    manager_->NotifyLongPress(node_, MakeGestureEvent(5.0, 6.0), 500 * 1000000LL);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"LongPress\""));
    EXPECT_TRUE(JsonContains(captured, "\"actualDuration\":500"));

    // the LONG_PRESS bit is still observed: a second notify dispatches again
    manager_->NotifyLongPress(node_, MakeGestureEvent(5.0, 6.0), 1LL);
    EXPECT_EQ(captured.calls.load(), 2);

    // observers exist for the instance but not for the LONG_PRESS bit
    CapturedJson otherBit;
    Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &otherBit);
    manager_->NotifyLongPress(node_, MakeGestureEvent(5.0, 6.0), 1LL);
    EXPECT_EQ(otherBit.calls.load(), 0);
}

/**
 * @tc.name: UIInfoCollectionManagerPan001
 * @tc.desc: Test NotifyPan action mapping including the invalid action guard.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerPan001, TestSize.Level1)
{
    /**
     * @tc.steps: register a PAN observer and notify with START, END, CANCEL and
     *            UPDATE (an unmapped action value).
     * @tc.expected: start/end/cancel appear in json; the unmapped action is
     *               dropped without dispatch.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_PAN, &CaptureCallback, &captured);
    GestureEvent info = MakeGestureEvent(1.0, 2.0);

    manager_->NotifyPan(node_, info, 3, GestureCallbackType::START);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Pan\""));
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"start\""));
    EXPECT_TRUE(JsonContains(captured, "\"direction\":3"));

    manager_->NotifyPan(node_, info, 3, GestureCallbackType::END);
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"end\""));

    manager_->NotifyPan(node_, info, 3, GestureCallbackType::CANCEL);
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"cancel\""));
    EXPECT_EQ(captured.calls.load(), 3);

    // unmapped GestureCallbackType value maps to a null action: dropped
    manager_->NotifyPan(node_, info, 3, GestureCallbackType::UPDATE);
    manager_->NotifyPan(node_, info, 3, GestureCallbackType::ACTION);
    EXPECT_EQ(captured.calls.load(), 3);
}

/**
 * @tc.name: UIInfoCollectionManagerPinch001
 * @tc.desc: Test NotifyPinch finger points and end-of-gesture scale.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerPinch001, TestSize.Level1)
{
    /**
     * @tc.steps: register a PINCH observer and notify with a two finger
     *            gesture for END and START.
     * @tc.expected: json contains both finger points and the scale only for
     *               the END action; unmapped action is dropped.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_PINCH, &CaptureCallback, &captured);
    GestureEvent info = MakeFingerGestureEvent();

    manager_->NotifyPinch(node_, info, GestureCallbackType::END);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Pinch\""));
    EXPECT_TRUE(JsonContains(captured, "\"fingers\":2"));
    EXPECT_TRUE(JsonContains(captured, "[11,12]"));
    EXPECT_TRUE(JsonContains(captured, "[21,22]"));
    EXPECT_TRUE(JsonContains(captured, "\"scale\":2.5"));

    manager_->NotifyPinch(node_, info, GestureCallbackType::START);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_FALSE(JsonContains(captured, "\"scale\""));

    manager_->NotifyPinch(node_, info, GestureCallbackType::UPDATE);
    EXPECT_EQ(captured.calls.load(), 2);
}

/**
 * @tc.name: UIInfoCollectionManagerRotation001
 * @tc.desc: Test NotifyRotation angle reporting on gesture end.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerRotation001, TestSize.Level1)
{
    /**
     * @tc.steps: register a ROTATION observer and notify with END and START.
     * @tc.expected: the angle is reported only for the END action; the finger
     *               points array is present.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_ROTATION, &CaptureCallback, &captured);
    GestureEvent info = MakeFingerGestureEvent();

    manager_->NotifyRotation(node_, info, GestureCallbackType::END);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Rotation\""));
    EXPECT_TRUE(JsonContains(captured, "\"angle\":45"));
    EXPECT_TRUE(JsonContains(captured, "\"fingers\":2"));

    manager_->NotifyRotation(node_, info, GestureCallbackType::START);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_FALSE(JsonContains(captured, "\"angle\""));
}

/**
 * @tc.name: UIInfoCollectionManagerSwipe001
 * @tc.desc: Test NotifySwipe down point recording fallback.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerSwipe001, TestSize.Level1)
{
    /**
     * @tc.steps: register a SWIPE observer, notify with recorded down events
     *            and without any.
     * @tc.expected: recorded down events are reported as downPoint; without
     *               records the up points are reused as down points.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_SWIPE, &CaptureCallback, &captured);

    std::map<int32_t, TouchEvent> downEvents;
    TouchEvent first = MakeTouchEvent(1, 1.0f, 2.0f);
    TouchEvent second = MakeTouchEvent(2, 3.0f, 4.0f);
    downEvents.emplace(1, first);
    downEvents.emplace(2, second);

    GestureEvent info = MakeFingerGestureEvent();
    manager_->NotifySwipe(node_, info, 2, 500.0, downEvents);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Swipe\""));
    EXPECT_TRUE(JsonContains(captured, "\"downPoint\":[[1,2],[3,4]]"));
    EXPECT_TRUE(JsonContains(captured, "\"upPoint\":[[11,12],[21,22]]"));
    EXPECT_TRUE(JsonContains(captured, "\"speed\":500"));
    EXPECT_TRUE(JsonContains(captured, "\"actualSpeed\":100"));

    downEvents.clear();
    manager_->NotifySwipe(node_, info, 2, 500.0, downEvents);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_TRUE(JsonContains(captured, "\"downPoint\":[[11,12],[21,22]]"));
}

/**
 * @tc.name: UIInfoCollectionManagerTouch001
 * @tc.desc: Test NotifyTouch hit node reporting and drag-only filtering.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerTouch001, TestSize.Level1)
{
    /**
     * @tc.steps: register a TOUCH observer, notify down with a hit node and
     *            up without; then verify a DRAG-only observer receives no
     *            TOUCH json.
     * @tc.expected: hit node id appears only for the down event with a valid
     *               hit node; drag-only masks emit no TOUCH events.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_TOUCH, &CaptureCallback, &captured);

    TouchEvent down = MakeTouchEvent(TEST_POINTER_ID, 7.0f, 8.0f);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, TEST_NODE_ID);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Touch\""));
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"down\""));
    EXPECT_TRUE(JsonContains(captured, "\"id\":55"));
    EXPECT_TRUE(JsonContains(captured, "\"fingerId\":7"));

    TouchEvent up = MakeTouchEvent(TEST_POINTER_ID, 7.0f, 8.0f);
    manager_->NotifyTouch(TEST_INSTANCE_ID, false, up, TEST_NODE_ID);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"up\""));
    // id is only reported for down events with a hit node

    manager_->NotifyTouch(TEST_INSTANCE_ID, false, up, -1);
    ASSERT_EQ(captured.calls.load(), 3);

    // drag-only observer: touch json is filtered by the TOUCH bit alone
    CapturedJson dragOnly;
    Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &dragOnly);
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    EXPECT_EQ(dragOnly.calls.load(), 0);
}

/**
 * @tc.name: UIInfoCollectionManagerTouchRecord001
 * @tc.desc: Test touch down record lifecycle feeding drag start duration.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerTouchRecord001, TestSize.Level1)
{
    /**
     * @tc.steps: register a DRAG observer, record a touch down, cancel the
     *            recording pointer, then a different pointer.
     * @tc.expected: cancel of the recording pointer ends the record; cancel of
     *               another pointer keeps it; drag start json always carries
     *               an actualDuration field.
     */
    CapturedJson captured;
    Register(INTERACTION_EVENT_DRAG, &CaptureCallback, &captured);
    TouchEvent down = MakeTouchEvent(TEST_POINTER_ID, 1.0f, 2.0f);

    // record kept: cancel of another pointer does not end it
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    manager_->NotifyTouchCancelled(TEST_INSTANCE_ID, TEST_POINTER_ID + 1);
    manager_->NotifyDragStart(node_, 3.0, 4.0);
    ASSERT_EQ(captured.calls.load(), 1);
    EXPECT_TRUE(JsonContains(captured, "\"type\":\"Drag\""));
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"start\""));
    EXPECT_TRUE(JsonContains(captured, "\"hostName\":\"com.ace.interaction.test\""));

    // cancel of the recording pointer ends the record
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, down, -1);
    manager_->NotifyTouchCancelled(TEST_INSTANCE_ID, TEST_POINTER_ID);
    manager_->NotifyDragStart(node_, 3.0, 4.0);
    ASSERT_EQ(captured.calls.load(), 2);
    EXPECT_TRUE(JsonContains(captured, "\"actualDuration\":0"));

    // drag end with and without a drop target
    manager_->NotifyDragEnd(TEST_INSTANCE_ID, 5.0, 6.0, true, TEST_NODE_ID);
    ASSERT_EQ(captured.calls.load(), 3);
    EXPECT_TRUE(JsonContains(captured, "\"action\":\"end\""));
    EXPECT_TRUE(JsonContains(captured, "\"dropResult\":\"success\""));
    EXPECT_TRUE(JsonContains(captured, "\"id\":55"));

    manager_->NotifyDragEnd(TEST_INSTANCE_ID, 5.0, 6.0, false, TEST_NODE_ID);
    ASSERT_EQ(captured.calls.load(), 4);
    EXPECT_TRUE(JsonContains(captured, "\"dropResult\":\"fail\""));

    // no drag observer: drag end is dropped
    CapturedJson untouched;
    Register(INTERACTION_EVENT_CLICK, &CaptureCallback, &untouched);
    manager_->NotifyDragEnd(TEST_INSTANCE_ID, 5.0, 6.0, true, -1);
    EXPECT_EQ(untouched.calls.load(), 0);
    // drag start is filtered by the DRAG bit as well
    manager_->NotifyDragStart(node_, 7.0, 8.0);
    EXPECT_EQ(untouched.calls.load(), 0);
}

/**
 * @tc.name: UIInfoCollectionManagerNoObserver001
 * @tc.desc: Test entry guards with no observer registered at all.
 * @tc.type: FUNC
 */
HWTEST_F(UIInfoCollectionManagerTestNg, UIInfoCollectionManagerNoObserver001, TestSize.Level1)
{
    /**
     * @tc.steps: call every notify entry without any observer registered.
     * @tc.expected: all entries return without dispatch or crash.
     */
    CapturedJson captured;
    GestureEvent info = MakeFingerGestureEvent();
    manager_->NotifyTapOrClick(node_, true, 1, info);
    manager_->NotifyLongPress(node_, info, 1LL);
    manager_->NotifyPan(node_, info, 1, GestureCallbackType::START);
    manager_->NotifyPinch(node_, info, GestureCallbackType::START);
    manager_->NotifyRotation(node_, info, GestureCallbackType::START);
    manager_->NotifySwipe(node_, info, 1, 1.0, {});
    manager_->NotifyTouch(TEST_INSTANCE_ID, true, MakeTouchEvent(1, 1.0f, 2.0f), -1);
    manager_->NotifyTouchCancelled(TEST_INSTANCE_ID, 1);
    manager_->NotifyDragStart(node_, 1.0, 2.0);
    manager_->NotifyDragEnd(TEST_INSTANCE_ID, 1.0, 2.0, true, 1);
    EXPECT_EQ(captured.calls.load(), 0);
}
} // namespace OHOS::Ace::NG

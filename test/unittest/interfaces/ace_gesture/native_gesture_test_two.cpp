/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "native_gesture.h"
#include "native_interface.h"
#include "native_node.h"
#include "native_type.h"

#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/gestures/recognizers/pinch_recognizer.h"
#include "core/components_ng/gestures/recognizers/rotation_recognizer.h"
#include "core/gestures/gesture_info.h"
#include "frameworks/core/components_ng/gestures/recognizers/click_recognizer.h"
#include "frameworks/core/components_ng/gestures/recognizers/long_press_recognizer.h"
#include "frameworks/core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "frameworks/core/components_ng/gestures/recognizers/swipe_recognizer.h"
#include "frameworks/core/interfaces/arkoala/arkoala_api.h"


namespace OHOS::Ace::NodeModel {
extern void (*g_eventReceiver)(ArkUI_NodeEvent* event);
extern void (*g_customEventReceiver)(ArkUI_NodeCustomEvent* event);
} // namespace OHOS::Ace::NodeModel

using namespace testing;
using namespace testing::ext;
class NativeGestureTestTwo : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: NativeGestureTest001
 * @tc.desc: Test createTapGesture function.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, NativeGestureTest001, TestSize.Level1)
{
    auto gestureAPI = reinterpret_cast<ArkUI_NativeGestureAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_GESTURE, "ArkUI_NativeGestureAPI_1"));
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto gestureNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    auto group = gestureAPI->createGroupGesture(EXCLUSIVE_GROUP);
    auto tapGesture = gestureAPI->createTapGesture(1, 1);
    auto tapGesture1 = gestureAPI->createTapGesture(0, 11);
    auto longPressGesture = gestureAPI->createLongPressGesture(1, true, 500);
    auto panGesture = gestureAPI->createPanGesture(1, GESTURE_DIRECTION_DOWN, 5);
    auto swipeGesture = gestureAPI->createSwipeGesture(1, 1, 5);
    auto pinchGesture = gestureAPI->createPinchGesture(2, 20);
    auto rotateGesture = gestureAPI->createRotationGesture(2, 90);
    gestureAPI->addChildGesture(group, tapGesture);
    gestureAPI->addChildGesture(group, tapGesture1);
    gestureAPI->addChildGesture(group, longPressGesture);
    auto onActionCallBack = [](ArkUI_GestureEvent *event, void *extraParam) {};
    gestureAPI->setGestureEventTarget(
        longPressGesture, GESTURE_EVENT_ACTION_ACCEPT,
        gestureNode, onActionCallBack);
    auto onInterruptCallback = [](ArkUI_GestureInterruptInfo* info) -> ArkUI_GestureInterruptResult {
        return GESTURE_INTERRUPT_RESULT_REJECT;
    };
    gestureAPI->setGestureInterrupterToNode(gestureNode, onInterruptCallback);
    auto ret = gestureAPI->addGestureToNode(gestureNode, group, PRIORITY, NORMAL_GESTURE_MASK);
    EXPECT_EQ(ret, 0);
    gestureAPI->removeGestureFromNode(gestureNode, group);
    gestureAPI->removeChildGesture(group, tapGesture);
    gestureAPI->removeChildGesture(group, tapGesture1);
    gestureAPI->removeChildGesture(group, longPressGesture);
    gestureAPI->dispose(tapGesture);
    gestureAPI->dispose(longPressGesture);
    gestureAPI->dispose(panGesture);
    gestureAPI->dispose(swipeGesture);
    gestureAPI->dispose(pinchGesture);
    gestureAPI->dispose(rotateGesture);
}

/**
 * @tc.name: NativeGestureTest002
 * @tc.desc: Test createTapGesture function.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, NativeGestureTest002, TestSize.Level1)
{
    auto gestureAPI = reinterpret_cast<ArkUI_NativeGestureAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_GESTURE, "ArkUI_NativeGestureAPI_1"));
    auto pinchGesture = gestureAPI->createPinchGesture(2, 0.0f);
    EXPECT_NE(pinchGesture, nullptr);
    auto swipeGesture = gestureAPI->createSwipeGesture(1, 1, 0.0f);
    EXPECT_NE(swipeGesture, nullptr);
    auto panGesture = gestureAPI->createPanGesture(0, GESTURE_DIRECTION_DOWN, 5);
    EXPECT_NE(panGesture, nullptr);
    gestureAPI->dispose(pinchGesture);
    gestureAPI->dispose(swipeGesture);
    gestureAPI->dispose(panGesture);
    EXPECT_NE(gestureAPI, nullptr);
}

/**
 * @tc.name: NativeGestureTest003
 * @tc.desc: Test the API2 setGestureInterrupterToNode function.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, NativeGestureTest003, TestSize.Level1)
{
    auto gestureAPI = reinterpret_cast<ArkUI_NativeGestureAPI_2*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_GESTURE, "ArkUI_NativeGestureAPI_2"));
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto gestureNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    auto group = gestureAPI->gestureApi1->createGroupGesture(EXCLUSIVE_GROUP);
    auto tapGesture = gestureAPI->gestureApi1->createTapGesture(1, 1);
    auto tapGesture1 = gestureAPI->gestureApi1->createTapGesture(0, 11);
    auto longPressGesture = gestureAPI->gestureApi1->createLongPressGesture(1, true, 500);
    auto panGesture = gestureAPI->gestureApi1->createPanGesture(1, GESTURE_DIRECTION_DOWN, 5);
    auto swipeGesture = gestureAPI->gestureApi1->createSwipeGesture(1, 1, 5);
    auto pinchGesture = gestureAPI->gestureApi1->createPinchGesture(2, 20);
    auto rotateGesture = gestureAPI->gestureApi1->createRotationGesture(2, 90);
    gestureAPI->gestureApi1->addChildGesture(group, tapGesture);
    gestureAPI->gestureApi1->addChildGesture(group, tapGesture1);
    gestureAPI->gestureApi1->addChildGesture(group, longPressGesture);
    auto onActionCallBack = [](ArkUI_GestureEvent *event, void *extraParam) {};
    gestureAPI->gestureApi1->setGestureEventTarget(
        longPressGesture, GESTURE_EVENT_ACTION_ACCEPT,
        gestureNode, onActionCallBack);
    auto onInterruptCallback = [](ArkUI_GestureInterruptInfo* info) -> ArkUI_GestureInterruptResult {
        return GESTURE_INTERRUPT_RESULT_REJECT;
    };
    int userData = 1;
    
    gestureAPI->setGestureInterrupterToNode(gestureNode, &userData, onInterruptCallback);
    auto ret = gestureAPI->gestureApi1->addGestureToNode(gestureNode, group, PRIORITY, NORMAL_GESTURE_MASK);
    EXPECT_EQ(ret, 0);
    gestureAPI->gestureApi1->removeGestureFromNode(gestureNode, group);
    gestureAPI->gestureApi1->removeChildGesture(group, tapGesture);
    gestureAPI->gestureApi1->removeChildGesture(group, tapGesture1);
    gestureAPI->gestureApi1->removeChildGesture(group, longPressGesture);
    gestureAPI->gestureApi1->dispose(tapGesture);
    gestureAPI->gestureApi1->dispose(longPressGesture);
    gestureAPI->gestureApi1->dispose(panGesture);
    gestureAPI->gestureApi1->dispose(swipeGesture);
    gestureAPI->gestureApi1->dispose(pinchGesture);
    gestureAPI->gestureApi1->dispose(rotateGesture);
}

/**
 * @tc.name: GestureApi3ParallelRejectsNullNode
 * @tc.desc: Call the API3 parallel gesture entry with a null node.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, GestureApi3ParallelRejectsNullNode, TestSize.Level1)
{
    auto* api = static_cast<ArkUI_NativeGestureAPI_3*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_GESTURE, "ArkUI_NativeGestureAPI_3"));
    ASSERT_NE(api, nullptr);
    ASSERT_NE(api->setGestureParallelTo, nullptr);
    int32_t userData = 1;
    auto parallelGesture = [](ArkUI_ParallelGestureEvent*) -> ArkUI_GestureRecognizer* { return nullptr; };
    EXPECT_EQ(api->setGestureParallelTo(nullptr, &userData, parallelGesture), ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeEventReceiverRegistrationEntries
 * @tc.desc: Call node event receiver registration and unregistration entries on the test thread.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, NodeEventReceiverRegistrationEntries, TestSize.Level1)
{
    auto* api = static_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(api, nullptr);
    ASSERT_NE(api->registerNodeEventReceiver, nullptr);
    ASSERT_NE(api->unregisterNodeEventReceiver, nullptr);
    auto receiver = +[](ArkUI_NodeEvent*) {};
    api->registerNodeEventReceiver(receiver);
    EXPECT_EQ(OHOS::Ace::NodeModel::g_eventReceiver, receiver);
    api->unregisterNodeEventReceiver();
    EXPECT_EQ(OHOS::Ace::NodeModel::g_eventReceiver, nullptr);
}

/**
 * @tc.name: NodeCustomEventReceiverRegistrationEntries
 * @tc.desc: Call custom node event receiver registration and unregistration entries on the test thread.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, NodeCustomEventReceiverRegistrationEntries, TestSize.Level1)
{
    auto* api = static_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(api, nullptr);
    ASSERT_NE(api->registerNodeCustomEventReceiver, nullptr);
    ASSERT_NE(api->unregisterNodeCustomEventReceiver, nullptr);
    auto receiver = +[](ArkUI_NodeCustomEvent*) {};
    api->registerNodeCustomEventReceiver(receiver);
    EXPECT_EQ(OHOS::Ace::NodeModel::g_customEventReceiver, receiver);
    api->unregisterNodeCustomEventReceiver();
    EXPECT_EQ(OHOS::Ace::NodeModel::g_customEventReceiver, nullptr);
}

/**
 * @tc.name: GestureApi1InnerParallelRejectsNullNode
 * @tc.desc: Call the inner parallel gesture entry with a null node.
 * @tc.type: FUNC
 */
HWTEST_F(NativeGestureTestTwo, GestureApi1InnerParallelRejectsNullNode, TestSize.Level1)
{
    auto* api = static_cast<ArkUI_NativeGestureAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_GESTURE, "ArkUI_NativeGestureAPI_1"));
    ASSERT_NE(api, nullptr);
    ASSERT_NE(api->setInnerGestureParallelTo, nullptr);
    int32_t userData = 1;
    auto parallelGesture = [](ArkUI_ParallelInnerGestureEvent*) -> ArkUI_GestureRecognizer* { return nullptr; };
    EXPECT_EQ(api->setInnerGestureParallelTo(nullptr, &userData, parallelGesture), ARKUI_ERROR_CODE_PARAM_INVALID);
}

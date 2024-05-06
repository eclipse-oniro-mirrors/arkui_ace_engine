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

#include <cstdint>

#include "native_gesture.h"
#include "native_node.h"
#include "native_type.h"
#include "node_model.h"
#include "gesture_impl.h"

#include "core/gestures/gesture_event.h"
#include "frameworks/core/interfaces/arkoala/arkoala_api.h"
#include "interfaces/native/event/ui_input_event_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ArkUI_GestureRecognizer {
    int32_t type = -1;
    ArkUIGesture* gesture = nullptr;
    void* extraData = nullptr;
};
#ifdef __cplusplus
};

// the ArkUI_GestureEvent struct actually same as ArkUIAPIEventGestureAsyncEvent;
struct ArkUI_GestureEvent {
    ArkUIAPIEventGestureAsyncEvent eventData;
};

#endif


ArkUI_GestureEventActionType OH_ArkUI_GestureEvent_GetActionType(const ArkUI_GestureEvent* event)
{
    ArkUI_GestureEventActionType ret;
    switch (event->eventData.subKind) {
        case ON_ACTION:
        case ON_ACTION_START:
            ret = GESTURE_EVENT_ACTION_ACCEPT;
            break;
        case ON_ACTION_UPDATE:
            ret = GESTURE_EVENT_ACTION_UPDATE;
            break;
        case ON_ACTION_END:
            ret = GESTURE_EVENT_ACTION_END;
            break;
        case ON_ACTION_CANCEL:
            ret = GESTURE_EVENT_ACTION_CANCEL;
            break;
        default:
            ret = GESTURE_EVENT_ACTION_ACCEPT;
            break;
    }
    return ret;
}

const ArkUI_UIInputEvent* OH_ArkUI_GestureEvent_GetRawInputEvent(const ArkUI_GestureEvent* event)
{
    if (!event) {
        return nullptr;
    }
    return reinterpret_cast<ArkUI_UIInputEvent*>(event->eventData.rawPointerEvent);
}

int32_t OH_ArkUI_LongPress_GetRepeatCount(const ArkUI_GestureEvent* event)
{
    return event->eventData.repeat;
}

float OH_ArkUI_PanGesture_GetVelocity(const ArkUI_GestureEvent* event)
{
    return event->eventData.velocity;
}

float OH_ArkUI_PanGesture_GetVelocityX(const ArkUI_GestureEvent* event)
{
    return event->eventData.velocityX;
}

float OH_ArkUI_PanGesture_GetVelocityY(const ArkUI_GestureEvent* event)
{
    return event->eventData.velocityY;
}

float OH_ArkUI_PanGesture_GetOffsetX(const ArkUI_GestureEvent* event)
{
    return event->eventData.x;
}

float OH_ArkUI_PanGesture_GetOffsetY(const ArkUI_GestureEvent* event)
{
    return event->eventData.y;
}

float OH_ArkUI_SwipeGesture_GetAngle(const ArkUI_GestureEvent* event)
{
    return event->eventData.angle;
}

float OH_ArkUI_SwipeGesture_GetVelocity(const ArkUI_GestureEvent* event)
{
    return event->eventData.velocity;
}

float OH_ArkUI_RotationGesture_GetAngle(const ArkUI_GestureEvent* event)
{
    return event->eventData.angle;
}

float OH_ArkUI_PinchGesture_GetScale(const ArkUI_GestureEvent* event)
{
    return event->eventData.scale;
}

float OH_ArkUI_PinchGesture_GetCenterX(const ArkUI_GestureEvent* event)
{
    return event->eventData.pinchCenterX;
}

float OH_ArkUI_PinchGesture_GetCenterY(const ArkUI_GestureEvent* event)
{
    return event->eventData.pinchCenterY;
}

namespace OHOS::Ace::GestureModel {

constexpr int32_t DEFAULT_PAN_FINGERS = 1;
constexpr int32_t MAX_PAN_FINGERS = 10;

struct GestureInnerData {
    void (*targetReceiver)(ArkUI_GestureEvent* event, void* extraParam);
    void* extraParam;
};

ArkUI_GestureRecognizer* CreateTapGesture(int32_t count, int32_t fingers)
{
    auto* gesture = OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createTapGesture(
        count, fingers);
    return new ArkUI_GestureRecognizer{ TAP_GESTURE, gesture, nullptr };
}

ArkUI_GestureRecognizer* CreateLongPressGesture(int32_t fingers, bool repeatResult, int32_t duration)
{
    auto* gesture =
        OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createLongPressGesture(fingers,
        repeatResult, duration);
    return new ArkUI_GestureRecognizer{ LONG_PRESS_GESTURE, gesture, nullptr };
}

ArkUI_GestureRecognizer* CreatePinchGesture(int32_t fingers, double distance)
{
    auto* gesture =
        OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createPinchGesture(fingers,
        distance);
    return new ArkUI_GestureRecognizer{ PINCH_GESTURE, gesture, nullptr };
}

ArkUI_GestureRecognizer* CreateRotationGesture(int32_t fingers, double angle)
{
    auto* gesture =
        OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createRotationGesture(fingers,
        angle);
    return new ArkUI_GestureRecognizer{ ROTATION_GESTURE, gesture, nullptr };
}

ArkUI_GestureRecognizer* CreateSwipeGesture(int32_t fingers, ArkUI_GestureDirectionMask directions, double speed)
{
    auto* gesture =
        OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createSwipeGesture(fingers,
        directions, speed);
    return new ArkUI_GestureRecognizer{ SWIPE_GESTURE, gesture, nullptr };
}

ArkUI_GestureRecognizer* CreatePanGesture(int32_t fingersNum, ArkUI_GestureDirectionMask mask, double distanceNum)
{
    int32_t fingers = DEFAULT_PAN_FINGERS;
    if (fingersNum < DEFAULT_PAN_FINGERS || fingersNum > MAX_PAN_FINGERS) {
        fingers = DEFAULT_PAN_FINGERS;
    } else {
        fingers = fingersNum;
    }
    auto* gesture = OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->createPanGesture(
        fingers, mask, distanceNum);
    return new ArkUI_GestureRecognizer{ PAN_GESTURE, gesture, nullptr };
}

void DisposeGesture(ArkUI_GestureRecognizer* recognizer)
{
    OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->dispose(recognizer->gesture);
    delete reinterpret_cast<GestureInnerData*>(recognizer->extraData);
    delete recognizer;
}

int32_t SetGestureEventTarget(ArkUI_GestureRecognizer* recognizer, ArkUI_GestureEventActionTypeMask mask,
    void* extraParam, void (*targetReceiver)(ArkUI_GestureEvent* event, void* extraParam))
{
    // 手势事件通过通用的异步事件接口进行处理。(HandleGestureEvent)
    // 把回调函数和上下文都封装到内部结构体中。
    if (recognizer->extraData) {
        delete reinterpret_cast<GestureInnerData*>(recognizer->extraData);
    }
    recognizer->extraData = new GestureInnerData { targetReceiver, extraParam };
    OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->registerGestureEvent(
        recognizer->gesture, mask, recognizer->extraData);
    return 0;
}

int32_t AddGestureToNode(ArkUI_NodeHandle node, ArkUI_GestureRecognizer* recognizer, ArkUI_GesturePriority priorityNum,
    ArkUI_GestureMask mask)
{
    OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->addGestureToNode(
        node->uiNodeHandle, recognizer->gesture, priorityNum, mask);
    return 0;
}

int32_t RemoveGestureFromNode(ArkUI_NodeHandle node, ArkUI_GestureRecognizer* recognizer)
{
    OHOS::Ace::NodeModel::GetFullImpl()->getNodeModifiers()->getGestureModifier()->removeGestureFromNode(
        node->uiNodeHandle, recognizer->gesture);
    return 0;
}

ArkUI_GestureRecognizerType GetGestureType(ArkUI_GestureRecognizer* recognizer)
{
    return static_cast<ArkUI_GestureRecognizerType>(recognizer->type);
}

void HandleGestureEvent(ArkUINodeEvent* event)
{
    auto* extraData = reinterpret_cast<GestureInnerData*>(event->extraParam);
    ArkUI_GestureEvent* gestureEvent = reinterpret_cast<ArkUI_GestureEvent *>(&event->gestureAsyncEvent);
    ArkUI_UIInputEvent uiEvent;
    uiEvent.inputType = ARKUI_UIINPUTEVENT_TYPE_TOUCH;
    uiEvent.eventTypeId = C_TOUCH_EVENT_ID;
    uiEvent.inputEvent = gestureEvent->eventData.rawPointerEvent;
    gestureEvent->eventData.rawPointerEvent = &uiEvent;
    extraData->targetReceiver(gestureEvent, extraData->extraParam);
    delete event;
}

}; // namespace OHOS::Ace::GestureModel
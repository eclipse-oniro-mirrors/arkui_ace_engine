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

#include "core/interfaces/native/node/node_gesture_modifier.h"

#include "core/components_ng/gestures/long_press_gesture.h"
#include "core/components_ng/pattern/gesture/gesture_model_ng.h"
#include "core/components_ng/gestures/pan_gesture.h"
#include "core/components_ng/gestures/pinch_gesture.h"
#include "core/components_ng/gestures/rotation_gesture.h"
#include "core/components_ng/gestures/swipe_gesture.h"
#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/node/touch_event_convertor.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/interfaces/arkoala/arkoala_api.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr int32_t MAX_POINTS = 10;
}
ArkUIGesture* createPanGesture(ArkUI_Int32 fingers, ArkUI_Int32 direction, ArkUI_Float64 distance, void* userData)
{
    PanDirection panDirection;
    switch (direction) {
        case ArkUI_GESTURE_DIRECTION_ALL:
            panDirection.type = panDirection.ALL;
            break;
        case ArkUI_GESTURE_DIRECTION_NONE:
            panDirection.type = panDirection.NONE;
            break;
        case ArkUI_GESTURE_DIRECTION_LEFT:
            panDirection.type = panDirection.LEFT;
            break;
        case ArkUI_GESTURE_DIRECTION_RIGHT:
            panDirection.type = panDirection.RIGHT;
            break;
        case ArkUI_GESTURE_DIRECTION_HORIZONTAL:
            panDirection.type = panDirection.HORIZONTAL;
            break;
        case ArkUI_GESTURE_DIRECTION_UP:
            panDirection.type = panDirection.UP;
            break;
        case  ArkUI_GESTURE_DIRECTION_DOWN:
            panDirection.type = panDirection.DOWN;
            break;
        case ArkUI_GESTURE_DIRECTION_VERTICAL:
            panDirection.type = panDirection.VERTICAL;
            break;
        default:
            panDirection.type = panDirection.NONE;
            break;
    }
    auto panGestureObject = AceType::MakeRefPtr<PanGesture>(fingers, panDirection, distance);
    panGestureObject->SetUserData(userData);
    panGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(panGestureObject));
}

ArkUIGesture* createTapGesture(ArkUI_Int32 count, ArkUI_Int32 fingers, void* userData)
{
    auto tapGestureObject = AceType::MakeRefPtr<TapGesture>(count, fingers);
    tapGestureObject->SetUserData(userData);
    tapGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(tapGestureObject));
}

ArkUIGesture* createLongPressGesture(ArkUI_Int32 fingers, bool repeat, ArkUI_Int32 duration, void* userData)
{
    auto longPressGestureObject = AceType::MakeRefPtr<LongPressGesture>(fingers, repeat, duration);
    longPressGestureObject->SetUserData(userData);
    longPressGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(longPressGestureObject));
}

ArkUIGesture* createPinchGesture(ArkUI_Int32 fingers, ArkUI_Float64 distance, void* userData)
{
    auto pinchGestureObject = AceType::MakeRefPtr<PinchGesture>(fingers, distance);
    pinchGestureObject->SetUserData(userData);
    pinchGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(pinchGestureObject));
}

ArkUIGesture* createRotationGesture(ArkUI_Int32 fingers, ArkUI_Float64 angle, void* userData)
{
    auto rotationGestureObject = AceType::MakeRefPtr<RotationGesture>(fingers, angle);
    rotationGestureObject->SetUserData(userData);
    rotationGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(rotationGestureObject));
}

ArkUIGesture* createSwipeGesture(ArkUI_Int32 fingers, ArkUI_Int32 directions, ArkUI_Float64 speed, void* userData)
{
    SwipeDirection swipeDirection{SwipeDirection::NONE};
    if (static_cast<uint32_t>(directions) & ArkUI_GESTURE_DIRECTION_HORIZONTAL) {
        swipeDirection.type = SwipeDirection::HORIZONTAL;
    }
    if (static_cast<uint32_t>(directions) & ArkUI_GESTURE_DIRECTION_VERTICAL) {
        swipeDirection.type += SwipeDirection::VERTICAL;
    }
    auto swipeGestureObject = AceType::MakeRefPtr<SwipeGesture>(fingers, swipeDirection, speed);
    swipeGestureObject->SetUserData(userData);
    swipeGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(swipeGestureObject));
}

ArkUIGesture* createSwipeGestureByModifier(ArkUI_Int32 fingers, ArkUI_Int32 direction, ArkUI_Float64 speed)
{
    SwipeDirection swipeDirection{ SwipeDirection::NONE};
    switch (direction) {
        case ArkUI_SWIPE_GESTURE_DIRECTION_ALL:
            swipeDirection.type = SwipeDirection::ALL;
            break;
        case ArkUI_SWIPE_GESTURE_DIRECTION_NONE:
            swipeDirection.type = SwipeDirection::NONE;
            break;
        case ArkUI_SWIPE_GESTURE_DIRECTION_HORIZONTAL:
            swipeDirection.type = SwipeDirection::HORIZONTAL;
            break;
        case ArkUI_SWIPE_GESTURE_DIRECTION_VERTICAL:
            swipeDirection.type = SwipeDirection::VERTICAL;
            break;
        default:
            swipeDirection.type = SwipeDirection::NONE;
            break;
    }
    auto swipeGestureObject = AceType::MakeRefPtr<SwipeGesture>(fingers, swipeDirection, speed);
    swipeGestureObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(swipeGestureObject));
}

ArkUIGesture* createGestureGroup(ArkUI_Int32 mode)
{
    auto gestureMode = static_cast<GestureMode>(mode);
    auto gestureGroupObject = AceType::MakeRefPtr<GestureGroup>(gestureMode);
    gestureGroupObject->IncRefCount();
    return reinterpret_cast<ArkUIGesture*>(AceType::RawPtr(gestureGroupObject));
}

void addGestureToGestureGroup(ArkUIGesture* group, ArkUIGesture* child)
{
    auto* gestureGroup = reinterpret_cast<GestureGroup*>(group);
    auto* childGesture = reinterpret_cast<Gesture*>(child);
    gestureGroup->AddGesture(AceType::Claim(childGesture));
}

void removeGestureFromGestureGroup(ArkUIGesture* group, ArkUIGesture* child)
{
    auto* gestureGroup = reinterpret_cast<GestureGroup*>(group);
    auto* childGesture = reinterpret_cast<Gesture*>(child);
    gestureGroup->RemoveGesture(AceType::Claim(childGesture));
}

void dispose(ArkUIGesture* recognizer)
{
    Gesture* gestureRef = reinterpret_cast<Gesture*>(recognizer);
    gestureRef->SetDisposeTag(true);
    gestureRef->DecRefCount();
}

void ConvertTouchPointsToPoints(GestureEvent& info, std::vector<TouchPoint>& touchPointes,
    std::array<ArkUITouchPoint, MAX_POINTS>& points)
{
    if (touchPointes.empty()) {
        return;
    }
    size_t i = 0;
    auto fingureIterator = std::begin(info.GetFingerList());
    auto fingureEnd = std::end(info.GetFingerList());
    for (auto& touchPoint : touchPointes) {
        if (i >= MAX_POINTS) {
            break;
        }
        points[i].id = touchPoint.id;
        points[i].nodeX = fingureIterator == fingureEnd ? 0.0f : fingureIterator->localLocation_.GetX();
        points[i].nodeY = fingureIterator == fingureEnd ? 0.0f : fingureIterator->localLocation_.GetY();
        points[i].windowX = fingureIterator == fingureEnd ? 0.0f : fingureIterator->globalLocation_.GetX();
        points[i].windowY = fingureIterator == fingureEnd ? 0.0f : fingureIterator->globalLocation_.GetY();
        points[i].screenX = touchPoint.screenX;
        points[i].screenY = touchPoint.screenY;
        points[i].contactAreaWidth = touchPoint.size;
        points[i].contactAreaHeight = touchPoint.size;
        points[i].pressure = touchPoint.force;
        points[i].tiltX = touchPoint.tiltX.value_or(0.0f);
        points[i].tiltY = touchPoint.tiltY.value_or(0.0f);
        points[i].pressedTime = touchPoint.downTime.time_since_epoch().count();
        points[i].toolType = static_cast<int32_t>(touchPoint.sourceTool);
        i++;
        fingureIterator++;
    }
}

void ConvertIMMEventToTouchEvent(GestureEvent& info, ArkUITouchEvent& touchEvent,
    std::array<ArkUITouchPoint, MAX_POINTS>& points)
{
    CHECK_NULL_VOID(info.GetPointerEvent());
    auto tempTouchEvent = NG::ConvertToTouchEvent(info.GetPointerEvent());
    touchEvent.action = static_cast<int32_t>(tempTouchEvent.type);
    touchEvent.sourceType = static_cast<int32_t>(tempTouchEvent.sourceType);
    touchEvent.timeStamp = tempTouchEvent.time.time_since_epoch().count();
    touchEvent.actionTouchPoint.pressure = tempTouchEvent.force;
    ConvertTouchPointsToPoints(info, tempTouchEvent.pointers, points);
    if (tempTouchEvent.pointers.size() > 0) {
        touchEvent.touchPointes = &(points[0]);
        touchEvent.actionTouchPoint.nodeX = touchEvent.touchPointes[0].nodeX;
        touchEvent.actionTouchPoint.nodeY = touchEvent.touchPointes[0].nodeY;
        touchEvent.actionTouchPoint.windowX = touchEvent.touchPointes[0].windowX;
        touchEvent.actionTouchPoint.windowY = touchEvent.touchPointes[0].windowY;
        touchEvent.actionTouchPoint.screenX = touchEvent.touchPointes[0].screenX;
        touchEvent.actionTouchPoint.screenY = touchEvent.touchPointes[0].screenY;
        touchEvent.actionTouchPoint.toolType = touchEvent.touchPointes[0].toolType;
    }
    touchEvent.touchPointSize = tempTouchEvent.pointers.size() < MAX_POINTS ?
    tempTouchEvent.pointers.size() : MAX_POINTS;
}

void GetGestureEvent(ArkUIAPIEventGestureAsyncEvent& ret, GestureEvent& info)
{
    ret.repeat = info.GetRepeat();
    ret.velocityX = info.GetVelocity().GetVelocityX();
    ret.velocityY = info.GetVelocity().GetVelocityY();
    ret.velocity = info.GetVelocity().GetVelocityValue();
    ret.x = info.GetOffsetX();
    ret.y = info.GetOffsetY();
    ret.angle = info.GetAngle();
    ret.scale = info.GetScale();
    ret.pinchCenterX = info.GetPinchCenter().GetX();
    ret.pinchCenterY = info.GetPinchCenter().GetY();
}

void GetBaseGestureEvent(ArkUIAPIEventGestureAsyncEvent* ret, ArkUITouchEvent& rawInputEvent,
    const std::shared_ptr<BaseGestureEvent>& info)
{
    rawInputEvent.sourceType = static_cast<ArkUI_Int32>(info->GetSourceDevice());
    rawInputEvent.timeStamp = info->GetTimeStamp().time_since_epoch().count();
    rawInputEvent.actionTouchPoint.tiltX = info->GetTiltX().value_or(0.0f);
    rawInputEvent.actionTouchPoint.tiltY = info->GetTiltY().value_or(0.0f);
    rawInputEvent.actionTouchPoint.toolType = static_cast<ArkUI_Int32>(info->GetSourceTool());
    rawInputEvent.actionTouchPoint.pressure = info->GetForce();
    std::array<ArkUITouchPoint, MAX_POINTS> points;
    auto fingerList = info->GetFingerList();
    auto fingureIterator = std::begin(fingerList);
    for (size_t i = 0; i < fingerList.size(); i++) {
        points[i].id = fingureIterator->fingerId_;
        points[i].windowX = fingureIterator->globalLocation_.GetX();
        points[i].windowY = fingureIterator->globalLocation_.GetY();
        points[i].nodeX = fingureIterator->localLocation_.GetX();
        points[i].nodeY = fingureIterator->localLocation_.GetY();
        points[i].tiltX = rawInputEvent.actionTouchPoint.tiltX;
        points[i].tiltY = rawInputEvent.actionTouchPoint.tiltY;
        fingureIterator++;
    }
    rawInputEvent.touchPointes = &(points[0]);
    rawInputEvent.touchPointSize = fingerList.size();
    ret->rawPointerEvent = &rawInputEvent;
}

void setCancelActionFunc(Gesture* gestureRef, void* extraParam)
{
    auto onActionCancel = [extraParam]() {
        ArkUINodeEvent eventData;
        eventData.kind = GESTURE_ASYNC_EVENT;
        eventData.nodeId = 0;
        eventData.extraParam = reinterpret_cast<ArkUI_Int64>(extraParam);
        eventData.gestureAsyncEvent.subKind = ON_ACTION_CANCEL;
        SendArkUIAsyncEvent(&eventData);
    };
    gestureRef->SetOnActionCancelId(onActionCancel);
}

void registerGestureEvent(ArkUIGesture* gesture, ArkUI_Uint32 actionTypeMask, void* extraParam)
{
    Gesture* gestureRef = reinterpret_cast<Gesture*>(gesture);
    if (actionTypeMask & ARKUI_GESTURE_EVENT_ACTION_ACCEPT) {
        auto onActionAccept = [extraParam](GestureEvent& info) {
            ArkUINodeEvent eventData;
            eventData.kind = GESTURE_ASYNC_EVENT;
            eventData.nodeId = 0;
            eventData.extraParam = reinterpret_cast<ArkUI_Int64>(extraParam);
            eventData.gestureAsyncEvent.subKind = ON_ACTION_START;
            GetGestureEvent(eventData.gestureAsyncEvent, info);
            ArkUITouchEvent rawInputEvent;
            std::array<ArkUITouchPoint, MAX_POINTS> points;
            ConvertIMMEventToTouchEvent(info, rawInputEvent, points);
            eventData.gestureAsyncEvent.rawPointerEvent = &rawInputEvent;
            SendArkUIAsyncEvent(&eventData);
        };
        gestureRef->SetOnActionId(onActionAccept);
        gestureRef->SetOnActionStartId(onActionAccept);
    }
    if (actionTypeMask & ARKUI_GESTURE_EVENT_ACTION_UPDATE) {
        auto onActionUpdate = [extraParam](GestureEvent& info) {
            ArkUINodeEvent eventData;
            eventData.kind = GESTURE_ASYNC_EVENT;
            eventData.nodeId = 0;
            eventData.extraParam = reinterpret_cast<ArkUI_Int64>(extraParam);
            eventData.gestureAsyncEvent.subKind = ON_ACTION_UPDATE;
            GetGestureEvent(eventData.gestureAsyncEvent, info);
            ArkUITouchEvent rawInputEvent;
            std::array<ArkUITouchPoint, MAX_POINTS> points;
            ConvertIMMEventToTouchEvent(info, rawInputEvent, points);
            eventData.gestureAsyncEvent.rawPointerEvent = &rawInputEvent;
            SendArkUIAsyncEvent(&eventData);
        };
        gestureRef->SetOnActionUpdateId(onActionUpdate);
    }
    if (actionTypeMask & ARKUI_GESTURE_EVENT_ACTION_END) {
        auto onActionEnd = [extraParam](GestureEvent& info) {
            ArkUINodeEvent eventData;
            eventData.kind = GESTURE_ASYNC_EVENT;
            eventData.nodeId = 0;
            eventData.extraParam = reinterpret_cast<ArkUI_Int64>(extraParam);
            eventData.gestureAsyncEvent.subKind = ON_ACTION_END;
            GetGestureEvent(eventData.gestureAsyncEvent, info);
            ArkUITouchEvent rawInputEvent;
            std::array<ArkUITouchPoint, MAX_POINTS> points;
            ConvertIMMEventToTouchEvent(info, rawInputEvent, points);
            eventData.gestureAsyncEvent.rawPointerEvent = &rawInputEvent;
            SendArkUIAsyncEvent(&eventData);
        };
        gestureRef->SetOnActionEndId(onActionEnd);
    }
    if (actionTypeMask & ARKUI_GESTURE_EVENT_ACTION_CANCEL) {
        setCancelActionFunc(gestureRef, extraParam);
    }
}

void addGestureToNode(ArkUINodeHandle node, ArkUIGesture* gesture, ArkUI_Int32 priorityNum, ArkUI_Int32 mask)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    auto gesturePtr = Referenced::Claim(reinterpret_cast<Gesture*>(gesture));

    GesturePriority priority = GesturePriority::Low;
    if (priorityNum > static_cast<int32_t>(GesturePriority::Begin) &&
            priorityNum < static_cast<int32_t>(GesturePriority::End)) {
        priority = static_cast<GesturePriority>(priorityNum);
    }
    gesturePtr->SetPriority(priority);

    GestureMask gestureMask = GestureMask::Normal;
    if (mask > static_cast<int32_t>(GestureMask::Begin) &&
        mask < static_cast<int32_t>(GestureMask::End)) {
        gestureMask = static_cast<GestureMask>(mask);
    }
    gesturePtr->SetGestureMask(gestureMask);
    gestureHub->AttachGesture(gesturePtr);
}


void removeGestureFromNode(ArkUINodeHandle node, ArkUIGesture* gesture)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    auto gesturePtr = Referenced::Claim(reinterpret_cast<Gesture*>(gesture));
    gestureHub->RemoveGesture(gesturePtr);
}

void removeGestureFromNodeByTag(ArkUINodeHandle node, ArkUI_CharPtr gestureTag)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    std::string tag(gestureTag);
    gestureHub->RemoveGesturesByTag(tag);
}

void clearGestures(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    gestureHub->ClearModifierGesture();
}

void setGestureInterrupterToNode(
    ArkUINodeHandle node, ArkUI_Int32 (*interrupter)(ArkUIGestureInterruptInfo* interrupterInfo))
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    auto onGestureJudgeBegin = [interrupter](
        const RefPtr<NG::GestureInfo>& gestureInfo,
        const std::shared_ptr<BaseGestureEvent>& info)-> GestureJudgeResult {
        ArkUIAPIEventGestureAsyncEvent* gestureEvent = new ArkUIAPIEventGestureAsyncEvent();
        ArkUITouchEvent rawInputEvent;
        GetBaseGestureEvent(gestureEvent, rawInputEvent, info);
        ArkUIGestureInterruptInfo interruptInfo;
        interruptInfo.isSystemGesture = gestureInfo->IsSystemGesture();
        interruptInfo.systemRecognizerType = static_cast<ArkUI_Int32>(gestureInfo->GetType());
        interruptInfo.event = gestureEvent;
        interruptInfo.userData = gestureInfo->GetUserData();
        auto result = interrupter(&interruptInfo);
        delete gestureEvent;
        return static_cast<GestureJudgeResult>(result);
    };
    ViewAbstract::SetOnGestureJudgeBegin(frameNode, std::move(onGestureJudgeBegin));
}

namespace NodeModifier {
const ArkUIGestureModifier* GetGestureModifier()
{
    static const ArkUIGestureModifier modifier = {
        createTapGesture,
        createLongPressGesture,
        createPanGesture,
        createPinchGesture,
        createRotationGesture,
        createSwipeGesture,
        createSwipeGestureByModifier,
        createGestureGroup,
        addGestureToGestureGroup,
        removeGestureFromGestureGroup,
        dispose,
        registerGestureEvent,
        addGestureToNode,
        removeGestureFromNode,
        removeGestureFromNodeByTag,
        clearGestures,
        setGestureInterrupterToNode,
        };
    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG

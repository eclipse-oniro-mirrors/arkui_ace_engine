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

#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"

#include "base/log/log_wrapper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"
#include "core/event/key_event_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
// Touch event constants to avoid magic numbers
constexpr float HALF = 2.0f;

// Bitmap order of MouseButton as consumed by applications via pressedButtonsArray.
constexpr MouseButton MOUSE_BUTTONS[] = { MouseButton::LEFT_BUTTON, MouseButton::RIGHT_BUTTON,
    MouseButton::MIDDLE_BUTTON, MouseButton::BACK_BUTTON, MouseButton::FORWARD_BUTTON };

// Minimal code-to-character mapping for injected keys; mirrors what a real keyboard
// reports for the alphanumeric block. Modifiers are not applied (base layer).
uint32_t ResolveUnicode(KeyCode code)
{
    if (code >= KeyCode::KEY_A && code <= KeyCode::KEY_Z) {
        return static_cast<uint32_t>('a') + static_cast<uint32_t>(code) - static_cast<uint32_t>(KeyCode::KEY_A);
    }
    if (code >= KeyCode::KEY_0 && code <= KeyCode::KEY_9) {
        return static_cast<uint32_t>('0') + static_cast<uint32_t>(code) - static_cast<uint32_t>(KeyCode::KEY_0);
    }
    if (code >= KeyCode::KEY_NUMPAD_0 && code <= KeyCode::KEY_NUMPAD_9) {
        return static_cast<uint32_t>('0') + static_cast<uint32_t>(code) - static_cast<uint32_t>(KeyCode::KEY_NUMPAD_0);
    }
    switch (code) {
        case KeyCode::KEY_SPACE:
            return static_cast<uint32_t>(' ');
        case KeyCode::KEY_ENTER:
        case KeyCode::KEY_NUMPAD_ENTER:
            return static_cast<uint32_t>('\n');
        case KeyCode::KEY_TAB:
            return static_cast<uint32_t>('\t');
        default:
            return 0;
    }
}

TouchPoint CreateTouchPoint(const PointF& coordinates, TouchType touchType)
{
    TouchPoint touchPoint;
    touchPoint.id = 0;
    touchPoint.downTime = std::chrono::high_resolution_clock::now();
    touchPoint.x = coordinates.GetX();
    touchPoint.y = coordinates.GetY();
    touchPoint.screenX = coordinates.GetX();
    touchPoint.screenY = coordinates.GetY();
    touchPoint.globalDisplayX = coordinates.GetX();
    touchPoint.globalDisplayY = coordinates.GetY();
    touchPoint.isPressed = (touchType == TouchType::DOWN) || (touchType == TouchType::MOVE);
    touchPoint.force = 0;
    touchPoint.sourceTool = SourceTool::FINGER;
    touchPoint.originalId = 0;
    touchPoint.size = 0;
    touchPoint.width = 0;
    touchPoint.height = 0;
    touchPoint.tiltX = 0.0f;
    touchPoint.tiltY = 0.0f;
    touchPoint.rollAngle = 0.0f;
    touchPoint.operatingHand = 0;
    touchPoint.xReverse = 0;
    touchPoint.yReverse = 0;
    return touchPoint;
}

TouchEvent BuildTouchEvent(const TouchPoint& touchPoint, TouchType touchType)
{
    TouchEvent event;
    event.SetId(touchPoint.id)
        .SetX(touchPoint.x)
        .SetY(touchPoint.y)
        .SetScreenX(touchPoint.screenX)
        .SetScreenY(touchPoint.screenY)
        .SetGlobalDisplayX(touchPoint.globalDisplayX)
        .SetGlobalDisplayY(touchPoint.globalDisplayY)
        .SetType(touchType)
        .SetPullType(TouchType::UNKNOWN)
        .SetSize(touchPoint.size)
        .SetForce(touchPoint.force)
        .SetTiltX(touchPoint.tiltX)
        .SetTiltY(touchPoint.tiltY)
        .SetRollAngle(touchPoint.rollAngle)
        .SetSourceType(SourceType::TOUCH)
        .SetSourceTool(touchPoint.sourceTool)
        .SetOriginalId(touchPoint.originalId)
        .SetPressedTime(touchPoint.downTime)
        .SetWidth(touchPoint.width)
        .SetHeight(touchPoint.height)
        .SetOperatingHand(touchPoint.operatingHand)
        .SetXReverse(touchPoint.xReverse)
        .SetYReverse(touchPoint.yReverse)
        .SetTime(touchPoint.downTime)
        .SetIsInjected(true);
    event.SetDeviceId(0);
    return event;
}

Offset CalculateCenter(const RectF& rect)
{
    return Offset((rect.Right() - rect.Left()) / HALF, (rect.Bottom() - rect.Top()) / HALF);
}

Offset CalculateWindowCenter(const RectF& rect)
{
    return Offset((rect.Left() + rect.Right()) / HALF, (rect.Top() + rect.Bottom()) / HALF);
}
} // namespace

TouchEvent RelaxedEventFactory::CreateTouchEvent(const PointF& coordinates, TouchType touchType)
{
    auto touchPoint = CreateTouchPoint(coordinates, touchType);
    auto event = BuildTouchEvent(touchPoint, touchType);
    event.pointers.emplace_back(touchPoint);
    return event;
}

MouseEvent RelaxedEventFactory::CreateMouseEvent(const PointF& coordinates, MouseAction action,
    MouseButton button, int32_t pressedButtons, const std::vector<KeyCode>& pressedKeyCodes)
{
    MouseEvent event;
    auto now = std::chrono::high_resolution_clock::now();
    event.x = coordinates.GetX();
    event.y = coordinates.GetY();
    event.screenX = coordinates.GetX();
    event.screenY = coordinates.GetY();
    event.globalDisplayX = coordinates.GetX();
    event.globalDisplayY = coordinates.GetY();
    event.time = now;
    event.pressedTime = now;
    event.action = action;
    event.button = button;
    event.pressedButtons = pressedButtons;
    for (const auto& mouseButton : MOUSE_BUTTONS) {
        if (pressedButtons & static_cast<int32_t>(mouseButton)) {
            event.pressedButtonsArray.emplace_back(mouseButton);
        }
    }
    event.pressedKeyCodes_ = pressedKeyCodes;
    event.sourceType = SourceType::MOUSE;
    event.sourceTool = SourceTool::MOUSE;
    event.isInjected = true;
    event.deviceId = 0;
    event.originalId = 0;
    return event;
}

AxisEvent RelaxedEventFactory::CreateAxisEvent(const PointF& coordinates, double delta,
    const std::vector<KeyCode>& pressedCodes)
{
    return CreateAxisEvent(coordinates, Offset(), delta, pressedCodes, WheelScrollDirection::VERTICAL);
}

AxisEvent RelaxedEventFactory::CreateAxisEvent(const PointF& coordinates, const Offset& windowOffset, double delta,
    const std::vector<KeyCode>& pressedCodes, WheelScrollDirection direction, AxisAction action)
{
    AxisEvent event;
    auto now = std::chrono::high_resolution_clock::now();
    event.x = coordinates.GetX();
    event.y = coordinates.GetY();
    event.screenX = coordinates.GetX() + windowOffset.GetX();
    event.screenY = coordinates.GetY() + windowOffset.GetY();
    event.globalDisplayX = coordinates.GetX() + windowOffset.GetX();
    event.globalDisplayY = coordinates.GetY() + windowOffset.GetY();
    event.time = now;
    if (direction == WheelScrollDirection::HORIZONTAL) {
        event.horizontalAxis = delta;
        event.axes = 1U << static_cast<uint32_t>(AxisType::HORIZONTAL_AXIS);
    } else {
        event.verticalAxis = delta;
        event.axes = 1U << static_cast<uint32_t>(AxisType::VERTICAL_AXIS);
    }
    event.action = action;
    event.sourceType = SourceType::MOUSE;
    event.sourceTool = SourceTool::MOUSE;
    event.pressedCodes = pressedCodes;
    event.isInjected = true;
    event.deviceId = 0;
    return event;
}

KeyEvent RelaxedEventFactory::CreateKeyEvent(KeyCode code, KeyAction action,
    const std::vector<KeyCode>& pressedCodes, int32_t repeatTime)
{
    KeyEvent event(code, action, pressedCodes, repeatTime, std::chrono::high_resolution_clock::now(), 0, 0,
        SourceType::KEYBOARD, {});
    event.isInjected = true;
    // Keep injected key events consistent with the real keyboard pipeline: `msg` must stay
    // empty because KeyEvent::msg doubles as the IME paste payload (text_input_client inserts
    // it verbatim on ctrl+v), and the injection marker is `isInjected`.
    event.keyIntention = keyItemsTransKeyIntention(event.pressedCodes);
    event.key = KeyCodeToString(static_cast<int32_t>(code));
    event.unicode = ResolveUnicode(code);
    return event;
}

GestureEvent RelaxedEventFactory::CreateClickGestureEvent(const RefPtr<FrameNode>& frameNode)
{
    GestureEvent info;
    auto geometryNode = frameNode->GetGeometryNode();
    if (!geometryNode) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "GeometryNode is null");
        return info;
    }
    info.SetTimeStamp(std::chrono::high_resolution_clock::now());
    auto rect = geometryNode->GetFrameRect();
    auto centerToNode = CalculateCenter(rect);
    auto centerToWindow = CalculateWindowCenter(rect);

    info.SetLocalLocation(centerToNode);
    info.SetSourceDevice(SourceType::TOUCH);
    info.SetInputEventType(InputEventType::TOUCH_SCREEN);
    info.SetSourceTool(SourceTool::FINGER);
    info.SetPatternName(frameNode->GetTag().c_str());
    info.SetGlobalLocation(centerToWindow);
    info.SetDeviceId(0);

    auto pipelineContext = frameNode->GetContextRefPtr();
    if (pipelineContext) {
        auto windowOffset = pipelineContext->GetCurrentWindowRect().GetOffset() + centerToWindow;
        auto globalWindowOffset = pipelineContext->GetGlobalDisplayWindowRect().GetOffset() + centerToWindow;
        info.SetScreenLocation(windowOffset).SetGlobalDisplayLocation(globalWindowOffset);
    }

    auto eventHub = frameNode->GetEventHub<EventHub>();
    if (eventHub) {
        auto targetImpl = eventHub->CreateGetEventTargetImpl();
        info.SetTarget(targetImpl().value_or(EventTarget()));
    }
    return info;
}

} // namespace OHOS::Ace::NG

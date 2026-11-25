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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_UTILS_RELAXED_EVENT_FACTORY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_UTILS_RELAXED_EVENT_FACTORY_H

#include <chrono>

#include "base/geometry/ng/point_t.h"
#include "base/geometry/offset.h"
#include "core/event/axis_event.h"
#include "core/event/key_event.h"
#include "core/event/mouse_event.h"
#include "ui/event/touch_event.h"
#include "ui/gestures/gesture_event.h"

namespace OHOS::Ace::NG {

class FrameNode;
enum class WheelScrollDirection;

class RelaxedEventFactory {
public:
    RelaxedEventFactory() = delete;
    ~RelaxedEventFactory() = delete;

    static TouchEvent CreateTouchEvent(const PointF& coordinates, TouchType touchType);

    static MouseEvent CreateMouseEvent(const PointF& coordinates, MouseAction action, MouseButton button,
        int32_t pressedButtons, const std::vector<KeyCode>& pressedKeyCodes,
        const TimeStamp& pressedTime = std::chrono::high_resolution_clock::now());

    static AxisEvent CreateAxisEvent(const PointF& coordinates, double delta,
        const std::vector<KeyCode>& pressedCodes);

    static AxisEvent CreateAxisEvent(const PointF& coordinates, const Offset& windowOffset, double delta,
        const std::vector<KeyCode>& pressedCodes, WheelScrollDirection direction,
        AxisAction action = AxisAction::UPDATE);

    static KeyEvent CreateKeyEvent(KeyCode code, KeyAction action, const std::vector<KeyCode>& pressedCodes,
        int32_t repeatTime = 0);

    static GestureEvent CreateClickGestureEvent(const RefPtr<FrameNode>& frameNode);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_UTILS_RELAXED_EVENT_FACTORY_H

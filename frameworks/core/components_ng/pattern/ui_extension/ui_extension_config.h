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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_CONFIG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_CONFIG_H

#include "core/common/window_animation_config.h"

namespace OHOS::Ace::NG {
enum class RSSubsystemId : uint8_t { WM_UIEXT = 0, ARKUI_UIEXT, ABILITY_UIEXT, INVALID };

enum class PlaceholderType : int32_t {
    NONE = 0,
    UNDEFINED = 1,
    ROTATION = 2,
    FOLD_TO_EXPAND = 3,
    INITIAL = 4,
};

enum class UIExtCallbackEventId : uint32_t {
    ON_AREA_CHANGED = 0,
    ON_UEA_ACCESSIBILITY_READY = 6,
    ON_DRAW_FIRST = 7,
};

struct UIExtCallbackEvent {
    UIExtCallbackEventId eventId;
    bool repeat = false;
    UIExtCallbackEvent(UIExtCallbackEventId id, bool re = false) : eventId(id), repeat(re) {}
    bool operator < (const UIExtCallbackEvent& other) const
    {
        return eventId < other.eventId;
    }
};

enum class BusinessDataSendType : uint32_t {
    ASYNC = 0,
    SYNC,
};

enum class UIContentBusinessCode : uint32_t {
    UNDEFINED = 0,
    // uiextension session business begin
    EVENT_PROXY = 1001,
    SEND_PAGE_MODE_REQUEST = 1002,
    SEND_PAGE_MODE_TO_UEA = 1003,
    // accessibility business begin
    TRANSFORM_PARAM = 2001,
    // avoid info business begin
    NOTIFY_AVOID_INFO_CHANGE = 3001,
    GET_AVOID_INFO = 3002,
    // extension window code from 10000 to 20000
    WINDOW_CODE_BEGIN = 10000,
    WINDOW_CODE_END = 20000,
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_CONFIG_H

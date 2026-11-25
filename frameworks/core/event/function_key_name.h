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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_FUNCTION_KEY_NAME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_FUNCTION_KEY_NAME_H

#include <string>

#include "core/event/function_key.h"

namespace OHOS::Ace {

// Narrow header: only include where GetFunctionKeyName is actually called,
// to avoid expanding the widely-included function_key.h.
inline std::string GetFunctionKeyName(FunctionKey functionkey)
{
    switch (functionkey) {
        case FunctionKey::FK_ESC: return "ESC";
        case FunctionKey::FK_F1: return "F1";
        case FunctionKey::FK_F2: return "F2";
        case FunctionKey::FK_F3: return "F3";
        case FunctionKey::FK_F4: return "F4";
        case FunctionKey::FK_F5: return "F5";
        case FunctionKey::FK_F6: return "F6";
        case FunctionKey::FK_F7: return "F7";
        case FunctionKey::FK_F8: return "F8";
        case FunctionKey::FK_F9: return "F9";
        case FunctionKey::FK_F10: return "F10";
        case FunctionKey::FK_F11: return "F11";
        case FunctionKey::FK_F12: return "F12";
        case FunctionKey::FK_TAB: return "TAB";
        case FunctionKey::FK_DPAD_UP: return "DPAD_UP";
        case FunctionKey::FK_DPAD_DOWN: return "DPAD_DOWN";
        case FunctionKey::FK_DPAD_LEFT: return "DPAD_LEFT";
        case FunctionKey::FK_DPAD_RIGHT: return "DPAD_RIGHT";
        case FunctionKey::FK_CAPS_LOCK: return "CAPS_LOCK";
        case FunctionKey::FK_SCROLL_LOCK: return "SCROLL_LOCK";
        case FunctionKey::FK_PAUSE_BREAK: return "PAUSE_BREAK";
        case FunctionKey::FK_NUM_LOCK: return "NUM_LOCK";
        case FunctionKey::FK_INSERT: return "INSERT";
        case FunctionKey::FK_DELETE: return "DELETE";
        case FunctionKey::FK_HOME: return "HOME";
        case FunctionKey::FK_END: return "END";
        case FunctionKey::FK_PAGE_UP: return "PAGE_UP";
        case FunctionKey::FK_PAGE_DOWN: return "PAGE_DOWN";
        case FunctionKey::FK_ENTER: return "ENTER";
        case FunctionKey::FK_BACKSPACE: return "BACKSPACE";
        default: return std::string();
    }
}

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_FUNCTION_KEY_NAME_H

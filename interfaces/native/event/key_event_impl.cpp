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

#include <cstring>

#include "securec.h"

#include "interfaces/native/native_key_event.h"
#include "interfaces/native/node/event_converter.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/event/ui_input_event_impl.h"
#include "frameworks/core/common/ace_application_info.h"
#include "frameworks/core/interfaces/arkoala/arkoala_api.h"
#include "interfaces/native/native_error_message_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

ArkUI_KeyEventType OH_ArkUI_KeyEvent_GetType(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetType");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetType", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyEventType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetType", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyEventType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto result = static_cast<ArkUI_KeyEventType>(keyEvent->type);
            RETURN_RET_WITH_STATUS_CHECK(result, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyEventType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
}

int32_t OH_ArkUI_KeyEvent_GetKeyCode(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetKeyCode");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        RETURN_RET_WITH_STATUS_CHECK(-1, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                RETURN_RET_WITH_STATUS_CHECK(-1, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto result = static_cast<int32_t>(keyEvent->keyCode);
            RETURN_RET_WITH_STATUS_CHECK(result, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(-1, ARKUI_ERROR_CODE_PARAM_INVALID);
}

const char* OH_ArkUI_KeyEvent_GetKeyText(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetKeyText");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetKeyText", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(nullptr, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetKeyText", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(nullptr, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            RETURN_RET_WITH_STATUS_CHECK(keyEvent->keyText, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(nullptr, ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_KeySourceType OH_ArkUI_KeyEvent_GetKeySource(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetKeySource");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetKeySource", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeySourceType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetKeySource", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeySourceType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto result = static_cast<ArkUI_KeySourceType>(keyEvent->keySource);
            RETURN_RET_WITH_STATUS_CHECK(result, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeySourceType>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
}

void OH_ArkUI_KeyEvent_StopPropagation(const ArkUI_UIInputEvent *event, bool stopPropagation)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_StopPropagation");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_StopPropagation", "event is null");
        RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
                    "OH_ArkUI_KeyEvent_StopPropagation", "keyEvent is null");
                RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            keyEvent->stopPropagation = stopPropagation;
            RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_KeyIntension OH_ArkUI_KeyEvent_GetKeyIntensionCode(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetKeyIntensionCode");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_GetKeyIntensionCode", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyIntension>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
                    "OH_ArkUI_KeyEvent_GetKeyIntensionCode", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyIntension>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto result = static_cast<ArkUI_KeyIntension>(keyEvent->intentionCode);
            RETURN_RET_WITH_STATUS_CHECK(result, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(static_cast<ArkUI_KeyIntension>(-1), ARKUI_ERROR_CODE_PARAM_INVALID);
}

uint32_t OH_ArkUI_KeyEvent_GetUnicode(const ArkUI_UIInputEvent *event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_GetUnicode");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        RETURN_RET_WITH_STATUS_CHECK(0, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                RETURN_RET_WITH_STATUS_CHECK(0, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto result = static_cast<uint32_t>(keyEvent->unicode);
            RETURN_RET_WITH_STATUS_CHECK(result, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(0, ARKUI_ERROR_CODE_PARAM_INVALID);
}

void OH_ArkUI_KeyEvent_SetConsumed(const ArkUI_UIInputEvent *event, bool isConsumed)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_SetConsumed");
    CheckSupportedScenarioAndResetEventStatus(S_NODE_ON_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetConsumed", "event is null");
        RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetConsumed", "keyEvent is null");
                RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            keyEvent->isConsumed = isConsumed;
            RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
}

void OH_ArkUI_KeyEvent_Dispatch(ArkUI_NodeHandle node, const ArkUI_UIInputEvent* event)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_Dispatch");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!node) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Dispatch", "node is null");
        RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Dispatch", "event is null");
        RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Dispatch", "keyEvent is null");
                RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            auto fullImpl = OHOS::Ace::NodeModel::GetFullImpl();
            fullImpl->getNodeModifiers()->getCommonModifier()->dispatchKeyEvent(node->uiNodeHandle, keyEvent);
            RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsNumLockOn(const ArkUI_UIInputEvent* event, bool* state)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_IsNumLockOn");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsNumLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!state) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsNumLockOn", "state is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsNumLockOn", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            *state = keyEvent->isNumLockOn;
            RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsCapsLockOn(const ArkUI_UIInputEvent* event, bool* state)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_IsCapsLockOn");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsCapsLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!state) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsCapsLockOn", "state is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsCapsLockOn", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            *state = keyEvent->isCapsLockOn;
            RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsScrollLockOn(const ArkUI_UIInputEvent* event, bool* state)
{
    CHECK_UI_THREAD("OH_ArkUI_KeyEvent_IsScrollLockOn");
    CheckSupportedScenarioAndResetEventStatus(S_ALL_C_KEY_EVENT, event);
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsScrollLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!state) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_IsScrollLockOn", "state is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    switch (event->eventTypeId) {
        case C_KEY_EVENT_ID: {
            const auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
            if (!keyEvent) {
                SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
                    "OH_ArkUI_KeyEvent_IsScrollLockOn", "keyEvent is null");
                RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
            }
            *state = keyEvent->isScrollLockOn;
            RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
        }
        default: {
            break;
        }
    }
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_Create(ArkUI_UIInputEvent** event)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Create", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = new ArkUIKeyEvent();
    keyEvent->keyCode = static_cast<int32_t>(ArkUI_KeyCode::ARKUI_KEYCODE_UNKNOWN);
    keyEvent->type = static_cast<ArkUI_Int32>(ArkUI_KeyEventType::ARKUI_KEY_EVENT_UNKNOWN);
    memset_s(keyEvent->keyText, sizeof(keyEvent->keyText), 0, sizeof(keyEvent->keyText));
    keyEvent->pressedKeyCodes = nullptr;
    keyEvent->keyCodesLength = 0;
    keyEvent->isNumLockOn = false;
    keyEvent->isCapsLockOn = false;
    keyEvent->isScrollLockOn = false;
    auto* uiInputEvent = new ArkUI_UIInputEvent();
    uiInputEvent->inputEvent = keyEvent;
    uiInputEvent->eventTypeId = C_KEY_EVENT_ID;
    uiInputEvent->isCreatedByUser = true;
    uiInputEvent->inputType = ARKUI_UIINPUTEVENT_TYPE_KEY;
    uiInputEvent->apiVersion = static_cast<int32_t>(OHOS::Ace::PlatformVersion::VERSION_TWENTY_SIX);
    *event = uiInputEvent;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_Destroy(ArkUI_UIInputEvent** event)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Destroy", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!(*event)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Destroy", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* uiInputEvent = *event;
    if (uiInputEvent->eventTypeId != C_KEY_EVENT_ID || !uiInputEvent->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_Destroy",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(uiInputEvent->inputEvent);
    if (keyEvent) {
        if (keyEvent->pressedKeyCodes) {
            delete[] keyEvent->pressedKeyCodes;
            keyEvent->pressedKeyCodes = nullptr;
        }
        delete keyEvent;
    }
    delete uiInputEvent;
    *event = nullptr;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetType(ArkUI_UIInputEvent* event, ArkUI_KeyEventType type)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetType", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetType",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetType", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    int32_t typeVal = static_cast<int32_t>(type);
    if (typeVal < static_cast<int32_t>(ArkUI_KeyEventType::ARKUI_KEY_EVENT_UNKNOWN) ||
        typeVal > static_cast<int32_t>(ArkUI_KeyEventType::ARKUI_KEY_EVENT_CLICK)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetType", "type is out of range");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->type = static_cast<ArkUI_Int32>(type);
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyCode(ArkUI_UIInputEvent* event, int32_t keyCode)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyCode", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyCode",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyCode", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (keyCode < static_cast<int32_t>(ArkUI_KeyCode::ARKUI_KEYCODE_UNKNOWN) ||
        keyCode > static_cast<int32_t>(ArkUI_KeyCode::ARKUI_KEYCODE_BUTTON_THUMBR)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyCode", "keyCode is out of range");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->keyCode = keyCode;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyText(ArkUI_UIInputEvent* event, const char* keyText)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyText", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyText",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyText", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (keyText) {
        strncpy_s(keyEvent->keyText, sizeof(keyEvent->keyText), keyText, sizeof(keyEvent->keyText) - 1);
    } else {
        keyEvent->keyText[0] = '\0';
    }
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeySource(ArkUI_UIInputEvent* event, ArkUI_KeySourceType keySource)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeySource", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeySource",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeySource", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    int32_t keySourceVal = static_cast<int32_t>(keySource);
    if (keySourceVal < static_cast<int32_t>(ArkUI_KeySourceType::ARKUI_KEY_SOURCE_TYPE_MOUSE) ||
        keySourceVal > static_cast<int32_t>(ArkUI_KeySourceType::ARKUI_KEY_SOURCE_TYPE_JOYSTICK)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeySource",
            "keySource is out of range");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->keySource = keySource;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetDeviceId(ArkUI_UIInputEvent* event, int32_t deviceId)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetDeviceId", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetDeviceId",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetDeviceId", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->deviceId = static_cast<ArkUI_Int64>(deviceId);
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetUnicode(ArkUI_UIInputEvent* event, uint32_t unicode)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetUnicode", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetUnicode",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetUnicode", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->unicode = unicode;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetEventTime(ArkUI_UIInputEvent* event, int64_t timestamp)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetEventTime", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetEventTime",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetEventTime", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->timestamp = static_cast<ArkUI_Float64>(timestamp);
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetPressedKeys(
    ArkUI_UIInputEvent* event, const int32_t* pressedKeyCodes, int32_t length)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (length < 0) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys", "length is negative");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (length > 0 && !pressedKeyCodes) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys",
            "pressedKeyCodes is null when length is positive");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (length == 0 && pressedKeyCodes) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetPressedKeys",
            "pressedKeyCodes is not null when length is 0");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (keyEvent->pressedKeyCodes) {
        delete[] keyEvent->pressedKeyCodes;
        keyEvent->pressedKeyCodes = nullptr;
        keyEvent->keyCodesLength = 0;
    }
    if (pressedKeyCodes && length > 0) {
        keyEvent->pressedKeyCodes = new int32_t[length];
        memcpy_s(keyEvent->pressedKeyCodes, static_cast<size_t>(length) * sizeof(int32_t),
            pressedKeyCodes, static_cast<size_t>(length) * sizeof(int32_t));
        keyEvent->keyCodesLength = length;
    }
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyIntentionCode(
    ArkUI_UIInputEvent* event, OH_ArkUI_KeyIntention intentionCode)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyIntentionCode", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyIntentionCode",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyIntentionCode", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    int32_t intentionVal = static_cast<int32_t>(intentionCode);
    if (intentionVal < static_cast<int32_t>(ArkUI_KeyIntension::ARKUI_KEY_INTENSION_UNKNOWN) ||
        intentionVal > static_cast<int32_t>(ArkUI_KeyIntension::ARKUI_KEY_INTENTION_CAMERA)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetKeyIntentionCode",
            "intentionCode is out of range");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->intentionCode = intentionCode;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetNumLockOn(ArkUI_UIInputEvent* event, bool isOn)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetNumLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetNumLockOn",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetNumLockOn", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->isNumLockOn = isOn;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetCapsLockOn(ArkUI_UIInputEvent* event, bool isOn)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetCapsLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetCapsLockOn",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetCapsLockOn", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->isCapsLockOn = isOn;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetScrollLockOn(ArkUI_UIInputEvent* event, bool isOn)
{
    if (!event) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetScrollLockOn", "event is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (event->eventTypeId != C_KEY_EVENT_ID || !event->isCreatedByUser) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetScrollLockOn",
            "event was not created by OH_ArkUI_KeyEvent_Create");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto* keyEvent = reinterpret_cast<ArkUIKeyEvent*>(event->inputEvent);
    if (!keyEvent) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, "OH_ArkUI_KeyEvent_SetScrollLockOn", "keyEvent is null");
        RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_PARAM_INVALID, ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    keyEvent->isScrollLockOn = isOn;
    RETURN_RET_WITH_STATUS_CHECK(ARKUI_ERROR_CODE_NO_ERROR, ARKUI_ERROR_CODE_NO_ERROR);
}
#ifdef __cplusplus
};
#endif
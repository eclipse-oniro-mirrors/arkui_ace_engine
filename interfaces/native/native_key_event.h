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

/**
 * @addtogroup ArkUI_NativeModule
 * @{
 *
 * @brief Provides the general key event APIs of ArkUI on the native side.
 *
 * @since 14
 */

/**
 * @file native_key_event.h
 *
 * @brief Declares the APIs related to native key events.
 *
 * @library libace_ndk.z.so
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @kit ArkUI
 * @since 14
 */

#ifndef ARKUI_NATIVE_KEY_EVENT_H
#define ARKUI_NATIVE_KEY_EVENT_H

#include <stdint.h>

#include "native_type.h"
#include "ui_input_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines an enum for the key codes in key events.
 *
 * @since 14
 */
typedef enum {
    /** Unknown (or unrecognized) key **/
    ARKUI_KEYCODE_UNKNOWN = -1,
    /** Function (Fn) key **/
    ARKUI_KEYCODE_FN = 0,
    /** Volume Up key **/
    ARKUI_KEYCODE_VOLUME_UP = 16,
    /** Volume Down key **/
    ARKUI_KEYCODE_VOLUME_DOWN = 17,
    /** Power key **/
    ARKUI_KEYCODE_POWER = 18,
    /** Shutter key **/
    ARKUI_KEYCODE_CAMERA = 19,
    /** Speaker Mute key **/
    ARKUI_KEYCODE_VOLUME_MUTE = 22,
    /** Mute key **/
    ARKUI_KEYCODE_MUTE = 23,
    /** Brightness Up key **/
    ARKUI_KEYCODE_BRIGHTNESS_UP = 40,
    /** Brightness Down key **/
    ARKUI_KEYCODE_BRIGHTNESS_DOWN = 41,
    /** Key 0 **/
    ARKUI_KEYCODE_0 = 2000,
    /** Key 1 **/
    ARKUI_KEYCODE_1 = 2001,
    /** Key 2 **/
    ARKUI_KEYCODE_2 = 2002,
    /** Key 3 **/
    ARKUI_KEYCODE_3 = 2003,
    /** Key 4 **/
    ARKUI_KEYCODE_4 = 2004,
    /** Key 5 **/
    ARKUI_KEYCODE_5 = 2005,
    /** Key 6 **/
    ARKUI_KEYCODE_6 = 2006,
    /** Key 7 **/
    ARKUI_KEYCODE_7 = 2007,
    /** Key 8 **/
    ARKUI_KEYCODE_8 = 2008,
    /** Key 9 **/
    ARKUI_KEYCODE_9 = 2009,
    /** Key + **/
    ARKUI_KEYCODE_STAR = 2010,
    /** Key # **/
    ARKUI_KEYCODE_POUND = 2011,
     /** Up key on D-pad **/
    ARKUI_KEYCODE_DPAD_UP = 2012,
    /** Down key on D-pad **/
    ARKUI_KEYCODE_DPAD_DOWN = 2013,
    /** Left key on D-pad **/
    ARKUI_KEYCODE_DPAD_LEFT = 2014,
    /** Right key on D-pad **/
    ARKUI_KEYCODE_DPAD_RIGHT = 2015,
    /** OK key on D-pad **/
    ARKUI_KEYCODE_DPAD_CENTER = 2016,
    /** Key A **/
    ARKUI_KEYCODE_A = 2017,
    /** Key B **/
    ARKUI_KEYCODE_B = 2018,
    /** Key C **/
    ARKUI_KEYCODE_C = 2019,
    /** Key D **/
    ARKUI_KEYCODE_D = 2020,
    /** Key E **/
    ARKUI_KEYCODE_E = 2021,
    /** Key F **/
    ARKUI_KEYCODE_F = 2022,
    /** Key G **/
    ARKUI_KEYCODE_G = 2023,
    /** Key H **/
    ARKUI_KEYCODE_H = 2024,
    /** Key I **/
    ARKUI_KEYCODE_I = 2025,
    /** Key J **/
    ARKUI_KEYCODE_J = 2026,
    /** Key K **/
    ARKUI_KEYCODE_K = 2027,
    /** Key L **/
    ARKUI_KEYCODE_L = 2028,
    /** Key M **/
    ARKUI_KEYCODE_M = 2029,
    /** Key N **/
    ARKUI_KEYCODE_N = 2030,
    /** Key O **/
    ARKUI_KEYCODE_O = 2031,
    /** Key P **/
    ARKUI_KEYCODE_P = 2032,
    /** Key R **/
    ARKUI_KEYCODE_Q = 2033,
    /** Key R **/
    ARKUI_KEYCODE_R = 2034,
    /** Key S **/
    ARKUI_KEYCODE_S = 2035,
    /** Key T **/
    ARKUI_KEYCODE_T = 2036,
    /** Key U **/
    ARKUI_KEYCODE_U = 2037,
    /** Key V **/
    ARKUI_KEYCODE_V = 2038,
    /** Key W **/
    ARKUI_KEYCODE_W = 2039,
    /** Key X **/
    ARKUI_KEYCODE_X = 2040,
    /** Key Y **/
    ARKUI_KEYCODE_Y = 2041,
    /** Key Z **/
    ARKUI_KEYCODE_Z = 2042,
    /** Key # **/
    ARKUI_KEYCODE_COMMA = 2043,
    /** Key # **/
    ARKUI_KEYCODE_PERIOD = 2044,
    /** Left Alt key **/
    ARKUI_KEYCODE_ALT_LEFT = 2045,
    /** Right Alt key **/
    ARKUI_KEYCODE_ALT_RIGHT = 2046,
    /** Left Shift key **/
    ARKUI_KEYCODE_SHIFT_LEFT = 2047,
    /** Right Shift key **/
    ARKUI_KEYCODE_SHIFT_RIGHT = 2048,
    /** Tab key **/
    ARKUI_KEYCODE_TAB = 2049,
    /** Space key **/
    ARKUI_KEYCODE_SPACE = 2050,
    /** Symbol key **/
    ARKUI_KEYCODE_SYM = 2051,
    /** Explorer key, used to start the explorer application **/
    ARKUI_KEYCODE_EXPLORER = 2052,
    /** Email key, used to start the email application **/
    ARKUI_KEYCODE_ENVELOPE = 2053,
    /** Enter key **/
    ARKUI_KEYCODE_ENTER = 2054,
    /** Backspace key **/
    ARKUI_KEYCODE_DEL = 2055,
    /** Key ` **/
    ARKUI_KEYCODE_GRAVE = 2056,
    /** Key - **/
    ARKUI_KEYCODE_MINUS = 2057,
    /** Key = **/
    ARKUI_KEYCODE_EQUALS = 2058,
    /** Key [ **/
    ARKUI_KEYCODE_LEFT_BRACKET = 2059,
    /** Key ]**/
    ARKUI_KEYCODE_RIGHT_BRACKET = 2060,
    /** Key \\ **/
    ARKUI_KEYCODE_BACKSLASH = 2061,
    /** Key ; **/
    ARKUI_KEYCODE_SEMICOLON = 2062,
    /** Key ' **/
    ARKUI_KEYCODE_APOSTROPHE = 2063,
    /** Key / **/
    ARKUI_KEYCODE_SLASH = 2064,
    /** Key @ **/
    ARKUI_KEYCODE_AT = 2065,
    /** Key + **/
    ARKUI_KEYCODE_PLUS = 2066,
    /** Menu key **/
    ARKUI_KEYCODE_MENU = 2067,
    /** Page Up key **/
    ARKUI_KEYCODE_PAGE_UP = 2068,
    /** Page Down key **/
    ARKUI_KEYCODE_PAGE_DOWN = 2069,
    /** ESC key **/
    ARKUI_KEYCODE_ESCAPE = 2070,
    /** Delete key **/
    ARKUI_KEYCODE_FORWARD_DEL = 2071,
    /** Left Ctrl key **/
    ARKUI_KEYCODE_CTRL_LEFT = 2072,
    /** Right Ctrl key **/
    ARKUI_KEYCODE_CTRL_RIGHT = 2073,
    /** Caps Lock key **/
    ARKUI_KEYCODE_CAPS_LOCK = 2074,
    /** Scroll Lock key **/
    ARKUI_KEYCODE_SCROLL_LOCK = 2075,
    /** Left Meta key **/
    ARKUI_KEYCODE_META_LEFT = 2076,
    /** Right Meta key **/
    ARKUI_KEYCODE_META_RIGHT = 2077,
    /** Function key **/
    ARKUI_KEYCODE_FUNCTION = 2078,
    /** System Request/Print Screen key **/
    ARKUI_KEYCODE_SYSRQ = 2079,
    /** Break/Pause key **/
    ARKUI_KEYCODE_BREAK = 2080,
    /** Move to Home key **/
    ARKUI_KEYCODE_MOVE_HOME = 2081,
    /** Move to End key **/
    ARKUI_KEYCODE_MOVE_END = 2082,
    /** Insert key **/
    ARKUI_KEYCODE_INSERT = 2083,
    /** Forward key **/
    ARKUI_KEYCODE_FORWARD = 2084,
    /** Play key **/
    ARKUI_KEYCODE_MEDIA_PLAY = 2085,
    /** Pause key **/
    ARKUI_KEYCODE_MEDIA_PAUSE = 2086,
    /** Close key **/
    ARKUI_KEYCODE_MEDIA_CLOSE = 2087,
    /** Eject key **/
    ARKUI_KEYCODE_MEDIA_EJECT = 2088,
    /** Record key **/
    ARKUI_KEYCODE_MEDIA_RECORD = 2089,
    /** F1 key **/
    ARKUI_KEYCODE_F1 = 2090,
    /** F2 key **/
    ARKUI_KEYCODE_F2 = 2091,
    /** F3 key **/
    ARKUI_KEYCODE_F3 = 2092,
    /** F4 key **/
    ARKUI_KEYCODE_F4 = 2093,
    /** F5 key **/
    ARKUI_KEYCODE_F5 = 2094,
    /** F6 key **/
    ARKUI_KEYCODE_F6 = 2095,
    /** F7 key **/
    ARKUI_KEYCODE_F7 = 2096,
    /** F8 key **/
    ARKUI_KEYCODE_F8 = 2097,
    /** F9 key **/
    ARKUI_KEYCODE_F9 = 2098,
    /** F10 key **/
    ARKUI_KEYCODE_F10 = 2099,
    /** F11 key **/
    ARKUI_KEYCODE_F11 = 2100,
    /** F12 key **/
    ARKUI_KEYCODE_F12 = 2101,
    /** Number Lock key on numeric keypad **/
    ARKUI_KEYCODE_NUM_LOCK = 2102,
    /** Key 0 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_0 = 2103,
    /** Key 1 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_1 = 2104,
    /** Key 2 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_2 = 2105,
    /** Key 3 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_3 = 2106,
    /** Key 4 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_4 = 2107,
    /** Key 5 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_5 = 2108,
    /** Key 6 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_6 = 2109,
    /** Key 7 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_7 = 2110,
    /** Key 8 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_8 = 2111,
    /** Key 9 on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_9 = 2112,
    /** Key / on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_DIVIDE = 2113,
    /** Key ) on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_MULTIPLY = 2114,
    /** Key - on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_SUBTRACT = 2115,
    /** Key + on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_ADD = 2116,
    /** Key . on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_DOT = 2117,
    /** Key , on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_COMMA = 2118,
    /** Enter key on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_ENTER = 2119,
    /** Key = on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_EQUALS = 2120,
    /** Key ( on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_LEFT_PAREN = 2121,
    /** Key ) on numeric keypad **/
    ARKUI_KEYCODE_NUMPAD_RIGHT_PAREN = 2122,
    /**
     * Joystick key A
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_A = 2301,
    /**
     * Joystick key B
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_B = 2302,
    /**
     * Joystick key X
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_X = 2304,
    /**
     * Joystick key Y
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_Y = 2305,
    /**
     * Joystick key L1
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_L1 = 2307,
    /**
     * Joystick key R1
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_R1 = 2308,
    /**
     * Joystick key L2
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_L2 = 2309,
    /**
     * Joystick key R2
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_R2 = 2310,
    /**
     * Joystick key Select
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_SELECT = 2311,
    /**
     * Joystick key Start
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_START = 2312,
    /**
     * Joystick key Mode
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_MODE = 2313,
    /**
     * Joystick key THUMBL
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_THUMBL = 2314,
    /**
     * Joystick key THUMBR
     * @since 15
    */
    ARKUI_KEYCODE_BUTTON_THUMBR = 2315,
} ArkUI_KeyCode;

/**
 * @brief Defines an enum for the key event types.
 *
 * @since 14
 */
typedef enum {
    /** Unknown type **/
    ARKUI_KEY_EVENT_UNKNOWN = -1,
    /** Pressing of a key **/
    ARKUI_KEY_EVENT_DOWN = 0,
    /** Release of a key **/
    ARKUI_KEY_EVENT_UP = 1,
    /** Long press of a key **/
    ARKUI_KEY_EVENT_LONG_PRESS = 2,
    /** Click of a key **/
    ARKUI_KEY_EVENT_CLICK = 3,
} ArkUI_KeyEventType;

/**
 * @brief Defines an enum for the types of devices that trigger a key event.
 *
 * @since 14
 */
typedef enum {
    /** Unknown type **/
    ARKUI_KEY_SOURCE_UNKNOWN = 0,
    /** Mouse **/
    ARKUI_KEY_SOURCE_TYPE_MOUSE = 1,
    /** Keyboard **/
    ARKUI_KEY_SOURCE_TYPE_KEYBOARD = 4,
    /**
    * @brief Joystick.
    *
    * @since 15
    */
    ARKUI_KEY_SOURCE_TYPE_JOYSTICK = 5,
} ArkUI_KeySourceType;

/**
 * @brief Defines an enum for key intentions.
 *
 * @since 14
 */
typedef enum {
    /** Unknown intention **/
    ARKUI_KEY_INTENSION_UNKNOWN = -1,
    /**Upward**/
    ARKUI_KEY_INTENSION_UP = 1,
    /** Downward **/
    ARKUI_KEY_INTENSION_DOWN = 2,
    /** Leftward **/
    ARKUI_KEY_INTENSION_LEFT = 3,
    /** Rightward **/
    ARKUI_KEY_INTENSION_RIGHT = 4,
    /** Select **/
    ARKUI_KEY_INTENSION_SELECT = 5,
    /** Escape **/
    ARKUI_KEY_INTENSION_ESCAPE = 6,
    /** Back**/
    ARKUI_KEY_INTENSION_BACK = 7,
    /** Forward **/
    ARKUI_KEY_INTENSION_FORWARD = 8,
    /** Menu **/
    ARKUI_KEY_INTENSION_MENU = 9,
    /** Home **/
    ARKUI_KEY_INTENSION_HOME = 10,
    /** Page up **/
    ARKUI_KEY_INTENSION_PAGE_UP = 11,
    /** Page down **/
    ARKUI_KEY_INTENSION_PAGE_DOWN = 12,
    /** Zoom out **/
    ARKUI_KEY_INTENSION_ZOOM_OUT = 13,
    /** Zoom in **/
    ARKUI_KEY_INTENSION_ZOOM_IN = 14,

    /** Play or pause **/
    ARKUI_KEY_INTENTION_MEDIA_PLAY_PAUSE = 100,
    /** Fast-forward **/
    ARKUI_KEY_INTENTION_MEDIA_FAST_FORWARD = 101,
    /** Fast playback **/
    ARKUI_KEY_INTENTION_MEDIA_FAST_PLAYBACK = 103,
    /** Play next **/
    ARKUI_KEY_INTENTION_MEDIA_NEXT = 104,
    /** Play previous **/
    ARKUI_KEY_INTENTION_MEDIA_PREVIOUS = 105,
    /** Mute **/
    ARKUI_KEY_INTENTION_MEDIA_MUTE = 106,
    /** Volume up **/
    ARKUI_KEY_INTENTION_VOLUME_UP = 107,
    /** Volume down **/
    ARKUI_KEY_INTENTION_VOLUME_DOWN = 108,

    /** Answer a call **/
    ARKUI_KEY_INTENTION_CALL = 200,
    /** Camera **/
    ARKUI_KEY_INTENTION_CAMERA = 300,
} ArkUI_KeyIntension;

/**
 * @brief Obtains the type of a key event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the key event type.
 * @since 14
 */
ArkUI_KeyEventType OH_ArkUI_KeyEvent_GetType(const ArkUI_UIInputEvent* event);

/**
 * @brief Obtains the key code from a key event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the key code.
 * @since 14
 */
int32_t OH_ArkUI_KeyEvent_GetKeyCode(const ArkUI_UIInputEvent* event);

/**
 * @brief Obtains the key value from a key event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the key value.
 * @since 14
 */
const char *OH_ArkUI_KeyEvent_GetKeyText(const ArkUI_UIInputEvent* event);

/**
 * @brief Obtains the type of device that triggers a key event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the device type.
 * @since 14
 */
ArkUI_KeySourceType OH_ArkUI_KeyEvent_GetKeySource(const ArkUI_UIInputEvent* event);

/**
 * @brief Prevents a key event from bubbling up.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @param stopPropagation Whether to stop event propagation.
 * @since 14
 */
void OH_ArkUI_KeyEvent_StopPropagation(const ArkUI_UIInputEvent* event, bool stopPropagation);

/**
 * @brief Obtains the intention code associated with a key event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the intention code associated with the key event.
 * @since 14
 */
ArkUI_KeyIntension OH_ArkUI_KeyEvent_GetKeyIntensionCode(const ArkUI_UIInputEvent* event);

/**
 * @brief Obtains the Unicode value associated with a key event.
 * Non-space basic Latin characters in the 0x0021-0x007E range are supported. Characters with a value of 0 are not
 * supported. In the case of key combination, this API returns the Unicode value of the key corresponding to the key
 * event.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @return Returns the Unicode value.
 * @since 14
 */
uint32_t OH_ArkUI_KeyEvent_GetUnicode(const ArkUI_UIInputEvent* event);

/**
 * @brief Sets whether a key event is consumed in the key event callback.
 *
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @param isConsumed Whether the event is consumed.
 * @since 14
 */
void OH_ArkUI_KeyEvent_SetConsumed(const ArkUI_UIInputEvent* event, bool isConsumed);

/**
 * @brief Dispatch key event to a specific component node.
 *
 * @param node Indicates the pointer to a component node.
 * @param event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @since 15
 */
void OH_ArkUI_KeyEvent_Dispatch(ArkUI_NodeHandle node, const ArkUI_UIInputEvent* event);

/**
 * @brief Get the Num lock state of the key event.
 *
 * @param event event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @param state Return whether the Num Lock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 19
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsNumLockOn(const ArkUI_UIInputEvent* event, bool* state);

/**
 * @brief Get the Caps lock state of the key event.
 *
 * @param event event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @param state Return whether the Caps Lock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 19
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsCapsLockOn(const ArkUI_UIInputEvent* event, bool* state);

/**
 * @brief Get the Scroll lock state of the key event.
 *
 * @param event event Pointer to an <b>ArkUI_UIInputEvent</b> object.
 * @param state Return whether the Scroll Lock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 19
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_IsScrollLockOn(const ArkUI_UIInputEvent* event, bool* state);

/**
 * @brief Defines the correctly spelled alias for the key event intention type.
 *
 * The original type name {@link ArkUI_KeyIntension} is retained for compatibility.
 *
 * @since 26.2.0
 */
typedef ArkUI_KeyIntension OH_ArkUI_KeyIntention;

/**
 * @brief Creates a key event that can be dispatched by {@link OH_ArkUI_KeyEvent_Dispatch}.
 *
 * @param event [out] Double pointer to the created ArkUI_UIInputEvent object.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if event is null.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_Create(ArkUI_UIInputEvent** event);

/**
 * @brief Destroys a key event created by {@link OH_ArkUI_KeyEvent_Create} and sets the event pointer to null.
 *
 * @param event [in,out] Double pointer to the ArkUI_UIInputEvent object to destroy.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if event is null or the input event was not created by
 *         {@link OH_ArkUI_KeyEvent_Create}.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_Destroy(ArkUI_UIInputEvent** event);

/**
 * @brief Sets the type of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param type Key event type.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetType(ArkUI_UIInputEvent* event, ArkUI_KeyEventType type);

/**
 * @brief Sets the key code of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param keyCode Key code. The value is defined by {@link ArkUI_KeyCode}.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyCode(ArkUI_UIInputEvent* event, int32_t keyCode);

/**
 * @brief Sets the key text of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * This function copies keyText. The caller can release or reuse the original string after this function returns.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param keyText Null-terminated key text.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyText(ArkUI_UIInputEvent* event, const char* keyText);

/**
 * @brief Sets the input device type of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param keySource Input device type.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeySource(ArkUI_UIInputEvent* event, ArkUI_KeySourceType keySource);

/**
 * @brief Sets the device ID of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param deviceId Device ID.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetDeviceId(ArkUI_UIInputEvent* event, int32_t deviceId);

/**
 * @brief Sets the Unicode value of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param unicode Unicode value.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetUnicode(ArkUI_UIInputEvent* event, uint32_t unicode);

/**
 * @brief Sets the occurrence time of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param timestamp Event occurrence time, in ns.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetEventTime(ArkUI_UIInputEvent* event, int64_t timestamp);

/**
 * @brief Sets all pressed keys of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * This function copies pressedKeyCodes. The caller can release or reuse the original array after this function
 * returns. Pass null for pressedKeyCodes and 0 for length to clear the pressed keys.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param pressedKeyCodes Array of pressed key codes.
 * @param length Length of the pressed key codes array.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetPressedKeys(
    ArkUI_UIInputEvent* event, const int32_t* pressedKeyCodes, int32_t length);

/**
 * @brief Sets the key intention code of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param intentionCode Key intention code.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetKeyIntentionCode(
    ArkUI_UIInputEvent* event, OH_ArkUI_KeyIntention intentionCode);

/**
 * @brief Sets the NumLock state of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param isOn Whether the NumLock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetNumLockOn(ArkUI_UIInputEvent* event, bool isOn);

/**
 * @brief Sets the CapsLock state of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param isOn Whether the CapsLock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetCapsLockOn(ArkUI_UIInputEvent* event, bool isOn);

/**
 * @brief Sets the ScrollLock state of a key event created by {@link OH_ArkUI_KeyEvent_Create}.
 *
 * @param event Pointer to the target ArkUI_UIInputEvent object.
 * @param isOn Whether the ScrollLock is on.
 * @return Returns the result code.
 *         Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter error occurs.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_KeyEvent_SetScrollLockOn(ArkUI_UIInputEvent* event, bool isOn);
/**
 * @brief Enumerates the modifier keys supported by keyboard shortcuts.
 *
 * The values are bit flags and can be combined by using the bitwise OR operator.
 *
 * @since 26.2.0
 */
typedef enum {
    /**
     * @brief No modifier key. This value can be used only with a function key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_MODIFIER_NONE = 0,
    /**
     * @brief Ctrl key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_MODIFIER_CTRL = 1,
    /**
     * @brief Shift key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_MODIFIER_SHIFT = 2,
    /**
     * @brief Alt key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_MODIFIER_ALT = 4,
} OH_ArkUI_KeyboardShortcutModifierKey;

/**
 * @brief Enumerates the function keys supported by keyboard shortcuts.
 *
 * @since 26.2.0
 */
typedef enum {
    /**
     * @brief Esc key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_ESC = 0,
    /**
     * @brief F1 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F1 = 1,
    /**
     * @brief F2 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F2 = 2,
    /**
     * @brief F3 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F3 = 3,
    /**
     * @brief F4 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F4 = 4,
    /**
     * @brief F5 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F5 = 5,
    /**
     * @brief F6 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F6 = 6,
    /**
     * @brief F7 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F7 = 7,
    /**
     * @brief F8 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F8 = 8,
    /**
     * @brief F9 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F9 = 9,
    /**
     * @brief F10 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F10 = 10,
    /**
     * @brief F11 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F11 = 11,
    /**
     * @brief F12 key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_F12 = 12,
    /**
     * @brief Tab key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_TAB = 13,
    /**
     * @brief Up arrow key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_DPAD_UP = 14,
    /**
     * @brief Down arrow key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_DPAD_DOWN = 15,
    /**
     * @brief Left arrow key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_DPAD_LEFT = 16,
    /**
     * @brief Right arrow key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_DPAD_RIGHT = 17,
    /**
     * @brief CapsLock key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_CAPS_LOCK = 18,
    /**
     * @brief PrintScreen key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_PRINT_SCREEN = 19,
    /**
     * @brief ScrollLock key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_SCROLL_LOCK = 20,
    /**
     * @brief Pause/Break key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_PAUSE_BREAK = 21,
    /**
     * @brief NumLock key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_NUM_LOCK = 22,
    /**
     * @brief Insert key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_INSERT = 23,
    /**
     * @brief Delete key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_DELETE = 24,
    /**
     * @brief Home key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_HOME = 25,
    /**
     * @brief End key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_END = 26,
    /**
     * @brief Page Up key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_PAGE_UP = 27,
    /**
     * @brief Page Down key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_PAGE_DOWN = 28,
    /**
     * @brief Enter key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_ENTER = 29,
    /**
     * @brief Backspace key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_BACKSPACE = 30,
    /**
     * @brief Space key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_SPACE = 31,
} OH_ArkUI_KeyboardShortcutFunctionKey;

/**
 * @brief Enumerates the types of keys supported by keyboard shortcuts.
 *
 * @since 26.2.0
 */
typedef enum {
    /**
     * @brief Character key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_KEY_TYPE_CHARACTER = 0,
    /**
     * @brief Function key.
     *
     * @since 26.2.0
     */
    OH_ARKUI_KEYBOARD_SHORTCUT_KEY_TYPE_FUNCTION_KEY = 1,
} OH_ArkUI_KeyboardShortcutKeyType;

/**
 * @brief Defines the keyboard shortcut key struct, which describes the shortcut key design logic such as the key
 * type and key value. Applications can register and manage custom shortcut keys.
 * @see OH_ArkUI_KeyboardShortcutKey_Create
 * @see OH_ArkUI_KeyboardShortcutKey_Destroy
 *
 * @since 26.2.0
 */
typedef struct OH_ArkUI_KeyboardShortcutKey OH_ArkUI_KeyboardShortcutKey;

/**
 * @brief Creates a keyboard shortcut key object. You can call {@link OH_ArkUI_KeyboardShortcutKey_Destroy} to destroy
 * a keyboard shortcut key object.
 *
 * @return Keyboard shortcut key object.
 *     <ul>
 *     <li>On success, returns a caller-owned {@link OH_ArkUI_KeyboardShortcutKey} object that the caller must
 *     release with {@link OH_ArkUI_KeyboardShortcutKey_Destroy}.</li>
 *     <li>Returns <b>nullptr</b> if the operation fails, for example, due to a memory allocation failure.</li>
 *     </ul>
 * @release arkui/OH_ArkUI_KeyboardShortcutKey_Destroy {return}
 * @since 26.2.0
 */
OH_ArkUI_KeyboardShortcutKey* OH_ArkUI_KeyboardShortcutKey_Create(void);

/**
 * @brief Destroys a keyboard shortcut key object.
 *
 * @param key [in] Keyboard shortcut key object.
 * @since 26.2.0
 */
void OH_ArkUI_KeyboardShortcutKey_Destroy(OH_ArkUI_KeyboardShortcutKey** key);

/**
 * @brief Sets the type of a keyboard shortcut key.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @param type [in] Key type.
 * @since 26.2.0
 */
void OH_ArkUI_KeyboardShortcutKey_SetType(OH_ArkUI_KeyboardShortcutKey* key, OH_ArkUI_KeyboardShortcutKeyType type);

/**
 * @brief Obtains the type of a keyboard shortcut key.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @return Key type.
 *     <ul>
 *     <li>Key type, if the operation is successful.</li>
 *     <li>{@link OH_ARKUI_KEYBOARD_SHORTCUT_KEY_TYPE_CHARACTER}, if <b>key</b> is <b>nullptr</b>.</li>
 *     </ul>
 * @since 26.2.0
 */
OH_ArkUI_KeyboardShortcutKeyType OH_ArkUI_KeyboardShortcutKey_GetType(const OH_ArkUI_KeyboardShortcutKey* key);

/**
 * @brief Sets the character for a character keyboard shortcut key. The key type must be set to
 * {@link OH_ARKUI_KEYBOARD_SHORTCUT_KEY_TYPE_CHARACTER} before calling this function.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @param character [in] Single character that can be entered through the keyboard.
 * @since 26.2.0
 */
void OH_ArkUI_KeyboardShortcutKey_SetCharacter(OH_ArkUI_KeyboardShortcutKey* key, char character);

/**
 * @brief Obtains the character of a character keyboard shortcut key.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @return Character.
 *     <ul>
 *     <li>Character, if the operation is successful.</li>
 *     <li><b>'\\0'</b>, if <b>key</b> is <b>nullptr</b>.</li>
 *     </ul>
 * @since 26.2.0
 */
char OH_ArkUI_KeyboardShortcutKey_GetCharacter(const OH_ArkUI_KeyboardShortcutKey* key);

/**
 * @brief Sets the function key for a function keyboard shortcut key. The key type must be set to
 * {@link OH_ARKUI_KEYBOARD_SHORTCUT_KEY_TYPE_FUNCTION_KEY} before calling this function.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @param functionKey [in] Function key.
 * @since 26.2.0
 */
void OH_ArkUI_KeyboardShortcutKey_SetFunctionKey(
    OH_ArkUI_KeyboardShortcutKey* key, OH_ArkUI_KeyboardShortcutFunctionKey functionKey);

/**
 * @brief Obtains the function key of a function keyboard shortcut key.
 *
 * @param key [in] Pointer to the keyboard shortcut key object.
 * @return Function key.
 *     <ul>
 *     <li>Function key, if the operation is successful.</li>
 *     <li>{@link OH_ARKUI_KEYBOARD_SHORTCUT_FUNCTION_KEY_ESC}, if <b>key</b> is <b>nullptr</b>.</li>
 *     </ul>
 * @since 26.2.0
 */
OH_ArkUI_KeyboardShortcutFunctionKey OH_ArkUI_KeyboardShortcutKey_GetFunctionKey(
    const OH_ArkUI_KeyboardShortcutKey* key);

/**
 * @brief Defines the callback information for an application-level keyboard shortcut.
 *
 * @since 26.2.0
 */
typedef struct OH_ArkUI_ApplicationShortcutCallbackInfo OH_ArkUI_ApplicationShortcutCallbackInfo;

/**
 * @brief Defines the callback invoked when an application-level keyboard shortcut is triggered. The callback is
 * invoked on the UI thread of the UI instance where the shortcut is triggered.
 *
 * @param info [out] Pointer to the callback information object for the UI instance where the shortcut is triggered. The
 *     value must not be <b>nullptr</b>. Use
 *     {@link OH_ArkUI_ApplicationShortcutCallbackInfo_GetUIContext} to obtain its UI context. The object is valid only
 *     during the callback and must not be retained.
 * @param userData [out] User-defined data passed when the shortcut is registered. The value is not copied and
 *     must remain valid until the shortcut is unregistered.
 * @since 26.2.0
 */
typedef void (*OH_ArkUI_ApplicationShortcutCallback)(const OH_ArkUI_ApplicationShortcutCallbackInfo* info, void* userData);

/**
 * @brief Obtains the UI context of the UI instance where an application-level keyboard shortcut is triggered.
 *
 * @param info [in] Pointer to the callback information object received by {@link OH_ArkUI_ApplicationShortcutCallback}.
 *     The value must not be <b>nullptr</b>.
 * @return UI context.
 *     <ul>
 *     <li>The UI context of the UI instance where the shortcut is triggered, if the operation is successful.</li>
 *     <li><b>nullptr</b>, if <b>info</b> is invalid.</li>
 *     </ul>
 * @since 26.2.0
 */
ArkUI_ContextHandle OH_ArkUI_ApplicationShortcutCallbackInfo_GetUIContext(
    const OH_ArkUI_ApplicationShortcutCallbackInfo* info);

/**
 * @brief Registers an application-level keyboard shortcut. The shortcut takes effect globally in the application.
 * A matching component-level shortcut is triggered before the application-level shortcut.
 *
 * Each registration generates a positive integer registration ID unique within the application, even when the
 * shortcut and callback are identical. Multiple registrations of the same shortcut are retained. All matching registrations
 * are invoked in increasing registration ID order using a snapshot collected for the event. Registration IDs are not reused during the
 * application lifetime.
 *
 * @param key [in] Pointer to a keyboard shortcut key object created by {@link OH_ArkUI_KeyboardShortcutKey_Create}.
 *     The value must not be <b>nullptr</b>. For a character key, the character set through
 *     {@link OH_ArkUI_KeyboardShortcutKey_SetCharacter} must be a single keyboard-input character.
 * @param modifierKeys [in] Modifier key bitmask. Use a bitwise OR combination of values in
 *     {@link OH_ArkUI_KeyboardShortcutModifierKey}. For a function key, use
 *     {@link OH_ARKUI_KEYBOARD_SHORTCUT_MODIFIER_NONE} to register the shortcut without a modifier key. At least one
 *     modifier key must be specified for a character key. Only the bits defined in
 *     {@link OH_ArkUI_KeyboardShortcutModifierKey} are accepted; if any other bit is set,
 *     {@link ARKUI_ERROR_CODE_PARAM_INVALID} is returned.
 * @param userData [in] User-defined data passed to the callback. The value can be <b>nullptr</b>. The value is
 *     not copied and must remain valid until the shortcut is unregistered.
 * @param callback [in] Callback invoked when the shortcut is triggered. The value must not be <b>nullptr</b>.
 * @param registrationId [out] Output pointer that receives the registration ID of this shortcut binding. The value must
 *     not be <b>nullptr</b>. The ID is written only on success; the output is unchanged on failure.
 * @return Result code.
 *     <ul>
 *     <li>Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *     <li>Returns {@link ARKUI_ERROR_CODE_PARAM_INVALID} if a parameter is invalid.</li>
 *     </ul>
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_ApplicationShortcut_Register(const OH_ArkUI_KeyboardShortcutKey* key, uint32_t modifierKeys,
    void* userData, OH_ArkUI_ApplicationShortcutCallback callback, int32_t* registrationId);

/**
 * @brief Unregisters the application-level keyboard shortcut binding identified by the registration ID.
 *
 * Only the specified binding is removed. Other bindings remain active, including bindings of the same shortcut.
 * Callbacks already collected for the current event can still run. If the ID does not exist or has already
 * been unregistered, this method does nothing and still succeeds.
 *
 * @param registrationId [in] Registration ID obtained when registering the shortcut.
 * @return Result code.
 *     <br>Returns {@link ARKUI_ERROR_CODE_NO_ERROR} if the operation succeeds or the registration ID does not exist.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_ApplicationShortcut_Unregister(int32_t registrationId);

#ifdef __cplusplus
};
#endif

#endif // ARKUI_NATIVE_KEY_EVENT_H
/** @} */
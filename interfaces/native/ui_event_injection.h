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

/**
 * @addtogroup ArkUI_NativeModule
 * @{
 *
 * @brief Provides relaxed UI control capabilities of ArkUI on the native side for in-app intelligent
 * agents and UI automation, enabling the injection of non-precise interaction commands without
 * targeting a specific component.
 *
 * @since 26.2.0
 */

/**
 * @file ui_event_injection.h
 *
 * @brief Declares the APIs used by in-app intelligent agents and UI automation to inject relaxed
 * non-precise control actions into the UI.
 *
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @include <arkui/ui_event_injection.h>
 * @library libace_ndk.z.so
 * @kit ArkUI
 * @since 26.2.0
 */

#ifndef ARKUI_NATIVE_UI_EVENT_INJECTION_H
#define ARKUI_NATIVE_UI_EVENT_INJECTION_H

#include "native_type.h"
#include "ui_json_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the result codes of UI event injection.
 *
 * This enumeration is shared by all injection APIs, including relaxed interaction injection
 * (non-precise) and precise component-targeted injection. A nonzero result code indicates an
 * execution failure, and its specific value conveys the failure reason.
 *
 * @since 26.2.0
 */
typedef enum {
    /**
     * @brief The injected command was executed successfully.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS = 0,
    /**
     * @brief No component or target could be found to respond to the injected command.
     *
     * For example, the click coordinates do not hit any component, or the target required by the
     * execution mode cannot be located.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND = 1,
    /**
     * @brief The command cannot be executed in the current UI context.
     *
     * The command is structurally valid, but its type or execution mode is not supported by the
     * current UI state.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED = 2,
    /**
     * @brief The injected command was interrupted by a user operation.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_INTERRUPTED_BY_USER = 3,
} OH_ArkUI_NativeModule_UIEventInjection_ResultCode;

/**
 * @brief Callback for injection completion.
 *
 * The callback is invoked once when the injected command finishes executing, whether successfully
 * or not, except when a later injection reclaims a command whose original UI instance
 * has been unregistered or replaced. In that case, a pending command or unclaimed completion
 * callback is cancelled. An executing command retains the process slot until it returns;
 * a callback already claimed for execution completes normally.
 *
 * @param result [in] Result code of the injection execution. For details, see
 *     {@link OH_ArkUI_NativeModule_UIEventInjection_ResultCode}.
 * @param userData [in] Custom user data pointer passed during injection.It can be NULL if pass NULL when injection.
 *        The type, ownership, and lifetime of the pointed-to data are entirely defined by the
 *        caller. The framework does not dereference or retain this pointer; it is
 *        only passed back to this callback. Must remain valid until the callback
 *        finishes. Cancellation has no separate notification. Instance unregistration alone
 *        does not prove that a claimed callback has finished. If no callback is received,
 *        the caller must synchronize execution-environment shutdown and callback exit
 *        before releasing the data.
 * @since 26.2.0
 */
typedef void (*OH_ArkUI_NativeModule_UIEventInjectionCallback)(
    OH_ArkUI_NativeModule_UIEventInjection_ResultCode result, void *userData);

/**
 * @brief Injects a control command into a target UI node asynchronously.
 *
 * The target node is identified by its unique ID (the same ID used by
 * OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId). The framework resolves the
 * unique ID to the corresponding node within the UI instance specified by
 * uiContext.
 *
 * The command is carried by an OH_ArkUI_NativeModule_UIJsonWrapper object created with
 * OH_ArkUI_NativeModule_UIJsonWrapper_Create. The command payload's top-level structure is:
 * {"schemaVersion": 1, "cmd": {"type": "<command type>", "action_info": { ... }}}
 *
 * Supported command types and their action_info parameters please refernce to developer guide.
 *
 * The uniqueId must correspond to a node that belongs to the same UI instance
 * as uiContext. If the node belongs to a different instance, the framework fails
 * to resolve it within uiContext and returns ARKUI_ERROR_CODE_NODE_NOT_FOUND.
 *
 * This function must be called on the UI thread of the UI instance specified by
 * uiContext. If called on a non-UI thread, the framework triggers a fatal log
 * output and aborts the process. This is a deliberate fail-fast design to
 * prevent undefined behavior from cross-thread UI access.
 *
 * @param uiContext [in] Pointer to a UI instance. Must not be NULL. The caller
 *        must be on the UI thread of this instance; calling from any other thread
 *        triggers a fatal abort. The uniqueId must resolve to a node within this instance.
 * @param uniqueId [in] Unique ID of the target node. This is the same ID used
 *        by OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId. The framework resolves
 *        this ID to the corresponding node within the uiContext instance.
 * @param command [in] Pointer to a configured JSON command object. Must not be NULL.
 *        The JSON data is internally copied before the function returns; the caller
 *        retains ownership and may destroy the object immediately after return.
 * @param callback [in] Completion callback. Must not be NULL. For an accepted command,
 *        invoked once on the UI thread when command processing completes, unless cancelled
 *        by the instance-unregistration recovery described above.
 * @param userData [in] Custom user data pointer passed to the callback. Can be NULL.
 *        If non-NULL, must remain valid until callback completion or synchronized shutdown
 *        as described by OH_ArkUI_NativeModule_UIEventInjectionCallback. The framework
 *        does not dereference or own this pointer.
 * @return <ul>
 *     <li>{@link ARKUI_ERROR_CODE_NO_ERROR} if the command is queued successfully.</li>
 *     <li>{@link ARKUI_ERROR_CODE_PARAM_INVALID} if parameter invalid, including NULL callback, required fields are
 *     missing, or the JSON conversion fails </li>
 *     <li>{@link ARKUI_ERROR_CODE_NODE_NOT_FOUND} if uniqueId does not resolve to a node within the uiContext
 *     instance.</li>
 *     <li>{@link ARKUI_ERROR_CODE_UI_CONTEXT_INVALID} if the UI context is invalid.</li>
 *     <li>{@link ARKUI_ERROR_CODE_COMMAND_UNFINISHED} if another composite command is still in progress in the
 *     current process; the new command is not queued and no callback is invoked.</li>
 *     </ul>
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
    ArkUI_ContextHandle uiContext,
    uint32_t uniqueId,
    const OH_ArkUI_NativeModule_UIJsonWrapper *command,
    OH_ArkUI_NativeModule_UIEventInjectionCallback callback,
    void *userData);

#ifdef __cplusplus
};
#endif

#endif // ARKUI_NATIVE_UI_EVENT_INJECTION_H
/** @} */

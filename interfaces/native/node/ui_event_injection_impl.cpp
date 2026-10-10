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

#include <cstdint>

#include "ui_event_injection.h"
#include "ui_json_wrapper.h"

#include "interfaces/native/native_error_message_macros.h"
#include "interfaces/native/node/node_model.h"

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
    ArkUI_ContextHandle uiContext,
    uint32_t uniqueId,
    const OH_ArkUI_NativeModule_UIJsonWrapper *command,
    OH_ArkUI_NativeModule_UIEventInjectionCallback callback,
    void *userData)
{
    if (uiContext == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID, __FUNCTION__, "UI context is null");
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    if (command == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "Command parameter is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    const char* jsonData = OH_ArkUI_NativeModule_UIJsonWrapperGetData(command);
    uint32_t jsonSize = OH_ArkUI_NativeModule_UIJsonWrapperGetSize(command);
    if (jsonData == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "Command JSON data is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }

    int32_t instanceId = reinterpret_cast<ArkUI_Context*>(uiContext)->id;

    const auto* impl = OHOS::Ace::NodeModel::GetOrCreateFullImpl();
    if (impl == nullptr || impl->getNodeModifiers == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, __FUNCTION__, "Native module not initialized");
        return ARKUI_ERROR_CODE_CAPI_INIT_ERROR;
    }
    auto* nodeModifiers = impl->getNodeModifiers();
    if (nodeModifiers == nullptr || nodeModifiers->getFrameNodeModifier == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, __FUNCTION__, "Node modifiers not initialized");
        return ARKUI_ERROR_CODE_CAPI_INIT_ERROR;
    }
    auto* frameNodeModifier = nodeModifiers->getFrameNodeModifier();
    if (frameNodeModifier == nullptr || frameNodeModifier->injectCompositeCommand == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, __FUNCTION__, "Frame node modifier not initialized");
        return ARKUI_ERROR_CODE_CAPI_INIT_ERROR;
    }

    if (callback == nullptr) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "callback must not be null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }

    ArkUIInjectCommandParams params = {};
    params.instanceId = instanceId;
    params.uniqueId = uniqueId;
    params.json = jsonData;
    params.jsonSize = jsonSize;
    params.callback = reinterpret_cast<void (*)(ArkUI_Int32, void*)>(callback);
    params.userData = userData;

    return static_cast<ArkUI_ErrorCode>(frameNodeModifier->injectCompositeCommand(&params));
}

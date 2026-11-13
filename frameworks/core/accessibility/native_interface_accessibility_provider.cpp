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

#include "core/accessibility/native_interface_accessibility_provider.h"

#include "base/log/log.h"
#include "base/log/log_wrapper.h"

using namespace OHOS::Ace;
namespace {
constexpr int32_t SEND_EVENT_FAILED = -1;
constexpr int32_t SEND_EVENT_SUCCESS = 0;

bool CheckProviderCallback(ArkUI_AccessibilityProviderCallbacks* callbacks)
{
    if (callbacks == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "callbacks is null");
        return false;
    }

    bool result = true;
    if (callbacks->findAccessibilityNodeInfosById == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosById is null");
        result = false;
    }

    if (callbacks->findAccessibilityNodeInfosByText == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosByText is null");
        result = false;
    }

    if (callbacks->findFocusedAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findFocusedAccessibilityNode is null");
        result = false;
    }

    if (callbacks->findNextFocusAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findNextFocusAccessibilityNode is null");
        result = false;
    }

    if (callbacks->executeAccessibilityAction == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "executeAccessibilityAction is null");
        result = false;
    }

    if (callbacks->clearFocusedFocusAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "clearFocusedFocusAccessibilityNode is null");
        result = false;
    }

    if (callbacks->getAccessibilityNodeCursorPosition == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "getAccessibilityNodeCursorPosition is null");
        result = false;
    }

    return result;
}
bool CheckProviderCallbackWithInstance(ArkUI_AccessibilityProviderCallbacksWithInstance* callbacks)
{
    if (callbacks == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "callbacks is null");
        return false;
    }

    if (callbacks->findAccessibilityNodeInfosById == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosById is null");
        return false;
    }

    if (callbacks->findAccessibilityNodeInfosByText == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosByText is null");
        return false;
    }

    if (callbacks->findFocusedAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findFocusedAccessibilityNode is null");
        return false;
    }

    if (callbacks->findNextFocusAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findNextFocusAccessibilityNode is null");
        return false;
    }

    if (callbacks->executeAccessibilityAction == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "executeAccessibilityAction is null");
        return false;
    }

    if (callbacks->clearFocusedFocusAccessibilityNode == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "clearFocusedFocusAccessibilityNode is null");
        return false;
    }

    if (callbacks->getAccessibilityNodeCursorPosition == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "getAccessibilityNodeCursorPosition is null");
        return false;
    }

    return true;
}
}
int32_t ArkUI_AccessibilityProvider::AccessibilityProviderRegisterCallback(
    ArkUI_AccessibilityProviderCallbacks* callbacks)
{
    if (!CheckProviderCallback(callbacks)) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "CheckProviderCallback failed.");
        NotifyRegisterCallback(false);
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    }

    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        callbacks_.callbacks = *callbacks;
    }
    callbacksRegistered_.store(true, std::memory_order_release);
    NotifyRegisterCallback(true);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "AccessibilityProviderRegisterCallback success");
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t ArkUI_AccessibilityProvider::AccessibilityProviderRegisterCallbackWithInstance(const char* instanceId,
    ArkUI_AccessibilityProviderCallbacksWithInstance* callbacks)
{
    if (!CheckProviderCallbackWithInstance(callbacks)) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "CheckProviderCallbackWithInstance failed.");
        NotifyRegisterCallback(false);
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    }
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        callbacks_.callbacksWithInstance = *callbacks;
        callbacks_.instanceId = instanceId;
    }
    callbacksRegistered_.store(true, std::memory_order_release);
    NotifyRegisterCallback(true);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "AccessibilityProviderRegisterCallbackWithInstance success");
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

ArkUI_AccessibilityProvider::CallbackSnapshot ArkUI_AccessibilityProvider::GetCallbackSnapshot()
{
    std::lock_guard<std::mutex> lock(callbacksMutex_);
    return callbacks_;
}

int32_t ArkUI_AccessibilityProvider::FindAccessibilityNodeInfosById(
    const int64_t elementId, const int32_t mode, const int32_t requestId,
    std::vector<ArkUI_AccessibilityElementInfo>& infos)
{
    const auto callbacks = GetCallbackSnapshot();
    auto accessibilityElementInfoList = std::make_unique<ArkUI_AccessibilityElementInfoList>();
    int32_t ret = 0;
    if (callbacks.callbacksWithInstance.findAccessibilityNodeInfosById) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.findAccessibilityNodeInfosById);
        ret = callbacks.callbacksWithInstance.findAccessibilityNodeInfosById(callbacks.instanceId.c_str(),
            elementId, static_cast<ArkUI_AccessibilitySearchMode>(mode),
            requestId, accessibilityElementInfoList.get());
    } else if (callbacks.callbacks.findAccessibilityNodeInfosById) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.findAccessibilityNodeInfosById);
        ret = callbacks.callbacks.findAccessibilityNodeInfosById(
            elementId, static_cast<ArkUI_AccessibilitySearchMode>(mode),
            requestId, accessibilityElementInfoList.get());
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosById is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }

    if (!accessibilityElementInfoList->CopyAccessibilityElementInfo(infos)) {
        ret = AccessibilityProviderOperatorErrorCode::COPY_FAILED;
    }
    return ret;
}

int32_t ArkUI_AccessibilityProvider::FindAccessibilityNodeInfosByText(
    const int64_t elementId, std::string text, const int32_t requestId,
    std::vector<ArkUI_AccessibilityElementInfo>& infos)
{
    const auto callbacks = GetCallbackSnapshot();
    auto accessibilityElementInfoList = std::make_unique<ArkUI_AccessibilityElementInfoList>();
    int32_t ret = 0;
    if (callbacks.callbacksWithInstance.findAccessibilityNodeInfosByText) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.findAccessibilityNodeInfosByText);
        ret = callbacks.callbacksWithInstance.findAccessibilityNodeInfosByText(callbacks.instanceId.c_str(),
            elementId, text.c_str(), requestId, accessibilityElementInfoList.get());
    } else if (callbacks.callbacks.findAccessibilityNodeInfosByText) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.findAccessibilityNodeInfosByText);
        ret = callbacks.callbacks.findAccessibilityNodeInfosByText(
            elementId, text.c_str(), requestId, accessibilityElementInfoList.get());
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findAccessibilityNodeInfosByText is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }

    if (!accessibilityElementInfoList->CopyAccessibilityElementInfo(infos)) {
        ret = AccessibilityProviderOperatorErrorCode::COPY_FAILED;
    }
    return ret;
}

int32_t ArkUI_AccessibilityProvider::FindFocusedAccessibilityNode(
    const int64_t elementId, int32_t focusType, const int32_t requestId,
    ArkUI_AccessibilityElementInfo& elementInfo)
{
    const auto callbacks = GetCallbackSnapshot();
    if (callbacks.callbacksWithInstance.findFocusedAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.findFocusedAccessibilityNode);
        return callbacks.callbacksWithInstance.findFocusedAccessibilityNode(callbacks.instanceId.c_str(),
            elementId, static_cast<ArkUI_AccessibilityFocusType>(focusType), requestId, &elementInfo);
    } else if (callbacks.callbacks.findFocusedAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.findFocusedAccessibilityNode);
        return callbacks.callbacks.findFocusedAccessibilityNode(elementId,
            static_cast<ArkUI_AccessibilityFocusType>(focusType), requestId, &elementInfo);
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findFocusedAccessibilityNode is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }
}

int32_t ArkUI_AccessibilityProvider::FindNextFocusAccessibilityNode(
    const int64_t elementId, int32_t direction, const int32_t requestId,
    ArkUI_AccessibilityElementInfo& elementInfo)
{
    const auto callbacks = GetCallbackSnapshot();
    if (callbacks.callbacksWithInstance.findNextFocusAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.findNextFocusAccessibilityNode);
        return callbacks.callbacksWithInstance.findNextFocusAccessibilityNode(callbacks.instanceId.c_str(),
            elementId, static_cast<ArkUI_AccessibilityFocusMoveDirection>(direction), requestId, &elementInfo);
    } else if (callbacks.callbacks.findNextFocusAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.findNextFocusAccessibilityNode);
        return callbacks.callbacks.findNextFocusAccessibilityNode(elementId,
            static_cast<ArkUI_AccessibilityFocusMoveDirection>(direction), requestId, &elementInfo);
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "findNextFocusAccessibilityNode is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }
}

int32_t ArkUI_AccessibilityProvider::ExecuteAccessibilityAction(
    const int64_t elementId, int32_t action, const int32_t requestId,
    const std::map<std::string, std::string>& actionArguments)
{
    const auto callbacks = GetCallbackSnapshot();
    auto actionArgumentsUnique = std::make_unique<ArkUI_AccessibilityActionArguments>(actionArguments);
    if (callbacks.callbacksWithInstance.executeAccessibilityAction) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.executeAccessibilityAction);
        return callbacks.callbacksWithInstance.executeAccessibilityAction(callbacks.instanceId.c_str(), elementId,
            static_cast<ArkUI_Accessibility_ActionType>(action),
            actionArgumentsUnique.get(),
            requestId);
    } else if (callbacks.callbacks.executeAccessibilityAction) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.executeAccessibilityAction);
        return callbacks.callbacks.executeAccessibilityAction(
            elementId, static_cast<ArkUI_Accessibility_ActionType>(action),
            actionArgumentsUnique.get(), requestId);
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "executeAccessibilityAction is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }
}

int32_t ArkUI_AccessibilityProvider::ClearFocusedAccessibilityNode()
{
    const auto callbacks = GetCallbackSnapshot();
    if (callbacks.callbacksWithInstance.clearFocusedFocusAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.clearFocusedFocusAccessibilityNode);
        return callbacks.callbacksWithInstance.clearFocusedFocusAccessibilityNode(callbacks.instanceId.c_str());
    } else if (callbacks.callbacks.clearFocusedFocusAccessibilityNode) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.clearFocusedFocusAccessibilityNode);
        return callbacks.callbacks.clearFocusedFocusAccessibilityNode();
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "clearFocusedFocusAccessibilityNode is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }
}

int32_t ArkUI_AccessibilityProvider::GetAccessibilityNodeCursorPosition(
    const int64_t elementId, const int32_t requestId, int32_t &cursorPosition)
{
    const auto callbacks = GetCallbackSnapshot();
    if (callbacks.callbacksWithInstance.getAccessibilityNodeCursorPosition) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallbackWithInstance",
            callbacks.callbacksWithInstance.getAccessibilityNodeCursorPosition);
        return callbacks.callbacksWithInstance.getAccessibilityNodeCursorPosition(callbacks.instanceId.c_str(),
            elementId, requestId, &cursorPosition);
    } else if (callbacks.callbacks.getAccessibilityNodeCursorPosition) {
        LOG_CALLBACK(
            "OH_ArkUI_AccessibilityProviderRegisterCallback",
            callbacks.callbacks.getAccessibilityNodeCursorPosition);
        return callbacks.callbacks.getAccessibilityNodeCursorPosition(
            elementId, requestId, &cursorPosition);
    } else {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "getAccessibilityNodeCursorPosition is null");
        return AccessibilityProviderOperatorErrorCode::NOT_REGISTERED;
    }
}

int32_t ArkUI_AccessibilityProvider::SendAccessibilityAsyncEvent(
    ArkUI_AccessibilityEventInfo* accessibilityEvent,
    void (*callback)(int32_t errorCode))
{
    if (accessibilityEvent == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "accessibilityEvent is null");
        if (callback) {
            LOG_CALLBACK("OH_ArkUI_SendAccessibilityAsyncEvent", callback);
            callback(SEND_EVENT_FAILED);
        }
        return SEND_EVENT_FAILED;
    }

    OHOS::Ace::RefPtr<OHOS::Ace::AccessibilityProvider> accessibilityProvider;
    {
        std::lock_guard<std::mutex> lock(accessibilityProviderMutex_);
        accessibilityProvider = accessibilityProvider_.Upgrade();
    }
    if (accessibilityProvider == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "accessibilityProvider is null");
        if (callback) {
            LOG_CALLBACK("OH_ArkUI_SendAccessibilityAsyncEvent", callback);
            callback(SEND_EVENT_FAILED);
        }
        return SEND_EVENT_FAILED;
    }

    accessibilityProvider->SendAccessibilityAsyncEvent(*accessibilityEvent, callback);
    return SEND_EVENT_SUCCESS;
}

void ArkUI_AccessibilityProvider::SetInnerAccessibilityProvider(
    const OHOS::Ace::WeakPtr<OHOS::Ace::AccessibilityProvider>& accessibilityProvider)
{
    std::lock_guard<std::mutex> lock(accessibilityProviderMutex_);
    accessibilityProvider_ = accessibilityProvider;
}

bool ArkUI_AccessibilityProvider::IsRegister()
{
    return callbacksRegistered_.load(std::memory_order_acquire);
}

void ArkUI_AccessibilityProvider::SetRegisterCallback(RegisterCallback callback)
{
    std::lock_guard<std::mutex> lock(registerCallbackMutex_);
    registerCallback_.swap(callback);
}

void ArkUI_AccessibilityProvider::NotifyRegisterCallback(bool isRegister)
{
    RegisterCallback callback;
    {
        std::lock_guard<std::mutex> lock(registerCallbackMutex_);
        callback = registerCallback_;
    }
    if (callback) {
        callback(isRegister);
    }
}

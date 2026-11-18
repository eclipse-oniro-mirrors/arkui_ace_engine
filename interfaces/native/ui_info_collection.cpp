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
#include "ui_info_collection.h"

#include <cmath>
#include <new>

#include "core/interfaces/native/node/content_change_api.h"
#include "core/interfaces/native/node/ui_tree_api.h"
#include "core/interfaces/native/node/interaction_observer_api.h"
#include "core/interfaces/native/node/page_text_api.h"
#include "interfaces/inner_api/ace/node_module_inner.h"
#include "interfaces/native/native_error_message_wrapper.h"
#include "interfaces/native/native_error_message_macros.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/node/node_model.h"

struct OH_ArkUI_NativeModule_UIContentChangeEvent {
    OH_ArkUI_NativeModule_UIContentChangeEventType type;
    uint64_t timestamp;
    ArkUI_ContextHandle context;
    OH_ArkUI_NativeModule_UIJsonWrapper* json;
};

namespace {
// Trampolines so that the JSON wrappers handed to the application callback are
// created and destroyed by the module that defines the wrapper type; the
// framework-side manager only sees the opaque pointers.
OH_ArkUI_NativeModule_UIJsonWrapper* CreateInteractionJsonWrapper(const char* data, uint32_t size)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* wrapper = nullptr;
    if (OH_ArkUI_NativeModule_UIJsonWrapperCreate(data, size, &wrapper) != ARKUI_ERROR_CODE_NO_ERROR) {
        return nullptr;
    }
    return wrapper;
}

void DestroyInteractionJsonWrapper(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper)
{
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(
        const_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(wrapper));
}

struct ContentChangeCookie {
    ArkUI_ContextHandle context;
    void* userData;
    OH_ArkUI_NativeModule_UIContentChangeEventCallback callback;
};

void OnContentChangeEvent(uint32_t type, uint64_t timestamp, const char* data, uint32_t size, void* opaque)
{
    auto* cookie = static_cast<ContentChangeCookie*>(opaque);
    OH_ArkUI_NativeModule_UIJsonWrapper* json = nullptr;
    if (OH_ArkUI_NativeModule_UIJsonWrapperCreate(data, size, &json) != ARKUI_ERROR_CODE_NO_ERROR) {
        return;
    }
    OH_ArkUI_NativeModule_UIContentChangeEvent event {
        static_cast<OH_ArkUI_NativeModule_UIContentChangeEventType>(type), timestamp, cookie->context, json
    };
    cookie->callback(&event, cookie->userData);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(json);
}

} // namespace

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetPageText(
    ArkUI_ContextHandle uiContext, OH_ArkUI_NativeModule_UIJsonWrapper** pageText)
{
    auto fail = [](ArkUI_ErrorCode code, const char* reason) {
        OHOS::Ace::SetErrorMessageByModifier(code, "OH_ArkUI_NativeModule_UIAgentGetPageText", reason);
        return code;
    };
    if (!pageText) {
        return fail(ARKUI_ERROR_CODE_PARAM_INVALID, "Page text output slot is null.");
    }
    *pageText = nullptr;
    if (!uiContext) {
        return fail(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID, "UI context is null.");
    }
    const auto* api = OHOS::Ace::NodeModel::GetPageTextAPI();
    if (!api || !api->collect || !api->release) {
        return fail(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, "Page text native support is unavailable.");
    }
    char* data = nullptr;
    uint32_t size = 0;
    const char* reason = "Page text collection failed.";
    auto code = static_cast<ArkUI_ErrorCode>(api->collect(uiContext->id, &data, &size, &reason));
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        api->release(data);
        return fail(code, reason);
    }
    code = OH_ArkUI_NativeModule_UIJsonWrapperCreate(data, size, pageText);
    api->release(data);
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        return fail(ARKUI_ERROR_CODE_INTERNAL_ERROR, "Page text snapshot allocation failed.");
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

struct OH_ArkUI_NativeModule_UIAgentTreeRequest {
    ArkUITreeOptions options;
};

namespace {
bool IsTreeFormatValid(OH_ArkUI_NativeModule_UIJsonFormat format)
{
    return format == OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT || format == OH_ARKUI_NATIVE_MODULE_UI_JSON_PRETTY;
}

void DeliverTreeResult(
    const ArkUITreeCompletion& completion, uint64_t requestId, int32_t result, const char* data, uint32_t size)
{
    auto code = static_cast<ArkUI_ErrorCode>(result);
    OH_ArkUI_NativeModule_UIJsonWrapper* json = nullptr;
    if (code == ARKUI_ERROR_CODE_NO_ERROR) {
        code = OH_ArkUI_NativeModule_UIJsonWrapperCreate(data, size, &json);
    }
    // No request, context or completion storage is accessed after the application callback.
    completion.callback(completion.context, requestId, code, json, completion.userData);
}
} // namespace

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(
    OH_ArkUI_NativeModule_UIAgentTreeType treeType, OH_ArkUI_NativeModule_UIAgentTreeRequest** request)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    *request = nullptr;
    if (treeType != OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL &&
        treeType != OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_VISIBLE) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto* value = new (std::nothrow) OH_ArkUI_NativeModule_UIAgentTreeRequest;
    if (!value) {
        return ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
    }
    value->options.treeType = treeType;
    *request = value;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

void OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(OH_ArkUI_NativeModule_UIAgentTreeRequest* request)
{
    delete request;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetFilterPureLayoutNodes(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    request->options.filterPureLayout = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetFilterOccludedNodes(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (request->options.treeType == OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL) {
        return enabled ? ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED : ARKUI_ERROR_CODE_NO_ERROR;
    }
    request->options.filterOccludedNodes = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetOcclusionOpacityThreshold(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, float threshold)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    // Validate the value before the tree type so that an out-of-range threshold is always reported as such.
    if (!std::isfinite(threshold) || threshold < 0.0f || threshold > 1.0f) {
        return ARKUI_ERROR_CODE_PARAM_OUT_OF_RANGE;
    }
    if (request->options.treeType == OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL) {
        return ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED;
    }
    request->options.occlusionOpacityThreshold = threshold;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetInteractionInfo(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    request->options.interaction = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetAccessibilityInfo(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    request->options.accessibility = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectVisualProperties(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    request->options.visual = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectFunctionalProperties(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled)
{
    if (!request) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    request->options.functional = enabled;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetTreeJson(ArkUI_ContextHandle context,
    const OH_ArkUI_NativeModule_UIAgentTreeRequest* request, OH_ArkUI_NativeModule_UIJsonFormat format,
    OH_ArkUI_NativeModule_UIJsonWrapper** json)
{
    if (!json) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    *json = nullptr;
    if (!request || !IsTreeFormatValid(format)) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (!context) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto* api = OHOS::Ace::NodeModel::GetTreeAPI();
    if (!api || !api->collect || !api->release) {
        return ARKUI_ERROR_CODE_INTERNAL_ERROR;
    }
    auto options = request->options;
    options.format = format;
    char* data = nullptr;
    uint32_t size = 0;
    auto code = static_cast<ArkUI_ErrorCode>(api->collect(context->id, &options, &data, &size));
    if (code == ARKUI_ERROR_CODE_NO_ERROR) {
        code = OH_ArkUI_NativeModule_UIJsonWrapperCreate(data, size, json);
    }
    api->release(data);
    return code;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetTreeJsonAsync(ArkUI_ContextHandle context,
    const OH_ArkUI_NativeModule_UIAgentTreeRequest* request, OH_ArkUI_NativeModule_UIJsonFormat format,
    OH_ArkUI_NativeModule_UIAgentJsonCallback callback, void* userData, uint64_t* requestId)
{
    if (!request || !callback || !requestId || !IsTreeFormatValid(format)) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (!context) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto* api = OHOS::Ace::NodeModel::GetTreeAPI();
    if (!api || !api->submit) {
        // No collection context can be resolved by an engine without the versioned backend.
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto options = request->options;
    options.format = format;
    ArkUITreeCompletion completion { context, callback, userData, DeliverTreeResult };
    return static_cast<ArkUI_ErrorCode>(api->submit(context->id, &options, &completion, requestId));
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
    ArkUI_ContextHandle uiContext, uint32_t eventMask, uint32_t* registerID,
    OH_ArkUI_NativeModule_UIInfoCollectionInteractionJsonCallback callback, void* userData)
{
    CHECK_UI_THREAD("OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver");
    auto fail = [](ArkUI_ErrorCode code, const char* reason) {
        OHOS::Ace::SetErrorMessageByModifier(
            code, "OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver", reason);
        return code;
    };
    if (!registerID) {
        return fail(ARKUI_ERROR_CODE_PARAM_INVALID, "Registration ID output slot is null.");
    }
    *registerID = 0;
    if (!callback) {
        return fail(ARKUI_ERROR_CODE_PARAM_INVALID, "Interaction callback is null.");
    }
    const uint32_t supportedMask =
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_LONG_PRESS |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_PAN |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_PINCH |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_ROTATION |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_SWIPE |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_DRAG |
        OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH;
    if ((eventMask & supportedMask) == 0) {
        return fail(ARKUI_ERROR_CODE_PARAM_INVALID, "Event mask contains no supported event bit.");
    }
    if (!uiContext) {
        return fail(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID, "UI context is null.");
    }
    const auto* api = OHOS::Ace::NodeModel::GetInteractionObserverAPI();
    if (!api || !api->registerObserver || !api->unregisterObserver) {
        return fail(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, "Interaction observer native support is unavailable.");
    }
    auto code = static_cast<ArkUI_ErrorCode>(api->registerObserver(
        reinterpret_cast<const ArkUI_Context*>(uiContext)->id, eventMask, callback, userData,
        &CreateInteractionJsonWrapper, &DestroyInteractionJsonWrapper, registerID));
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        *registerID = 0;
        return fail(code, "Interaction observer registration failed.");
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(uint32_t registerID)
{
    CHECK_UI_THREAD("OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver");
    auto fail = [](ArkUI_ErrorCode code, const char* reason) {
        OHOS::Ace::SetErrorMessageByModifier(
            code, "OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver", reason);
        return code;
    };
    if (registerID == 0) {
        return fail(ARKUI_ERROR_CODE_PARAM_INVALID, "Registration ID is invalid.");
    }
    const auto* api = OHOS::Ace::NodeModel::GetInteractionObserverAPI();
    if (!api || !api->unregisterObserver) {
        return fail(ARKUI_ERROR_CODE_CAPI_INIT_ERROR, "Interaction observer native support is unavailable.");
    }
    auto code = static_cast<ArkUI_ErrorCode>(api->unregisterObserver(registerID));
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        return fail(code, "Interaction observer unregistration failed.");
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
extern "C" {
#endif

ArkUI_ErrorCode OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
    ArkUI_ContextHandle uiContext, bool withStart, uint32_t eventMask, uint32_t ignoreMask, void* userData,
    OH_ArkUI_NativeModule_UIContentChangeEventCallback callback, uint64_t* subscriptionId)
{
    CHECK_UI_THREAD("OH_ArkUI_NativeModule_RegisterUIContentChangeEvent");
    CHECK_NULL_RETURN_WITH_MESSAGE(uiContext, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID,
        __FUNCTION__, "UI context parameter is null");
    CHECK_NULL_RETURN_WITH_MESSAGE(callback, ARKUI_ERROR_CODE_CALLBACK_INVALID,
        __FUNCTION__, "Callback parameter is null");
    if (eventMask == 0) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "Event mask is zero");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    constexpr uint32_t validEventMask = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_PAGE |
        OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_SCROLL | OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_OVERLAY;
    constexpr uint32_t validIgnoreMask = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLBY |
        OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLTO | OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_TOAST;
    if ((eventMask != OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_ALL && (eventMask & ~validEventMask)) ||
        (ignoreMask & ~validIgnoreMask)) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "Event or ignore mask contains unknown bits");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    CHECK_NULL_RETURN_WITH_MESSAGE(subscriptionId, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Subscription ID output parameter is null");
    const auto* api = OHOS::Ace::NodeModel::GetContentChangeAPI();
    CHECK_NULL_RETURN_WITH_MESSAGE(api, ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Content change native support is unavailable");
    CHECK_NULL_RETURN_WITH_MESSAGE(api->registerEvent && api->unregisterEvent, ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Content change subscription is not initialized");
    auto* cookie = new (std::nothrow) ContentChangeCookie { uiContext, userData, callback };
    CHECK_NULL_RETURN_WITH_MESSAGE(cookie, ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Subscription allocation failed");
    uint64_t id = 0;
    auto code = static_cast<ArkUI_ErrorCode>(api->registerEvent(
        uiContext->id, withStart, eventMask, ignoreMask,
        cookie, OnContentChangeEvent, [](void* opaque) { delete static_cast<ContentChangeCookie*>(opaque); },
        &id));
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        delete cookie;
        SET_ERROR_FUNCTION_NAME(__FUNCTION__);
        return code;
    }
    *subscriptionId = id;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(
    ArkUI_ContextHandle uiContext, uint64_t subscriptionId)
{
    CHECK_UI_THREAD("OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent");
    CHECK_NULL_RETURN_WITH_MESSAGE(uiContext, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID,
        __FUNCTION__, "UI context parameter is null");
    if (subscriptionId == 0) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "Subscription ID is zero");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    const auto* api = OHOS::Ace::NodeModel::GetContentChangeAPI();
    CHECK_NULL_RETURN_WITH_MESSAGE(api, ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Content change native support is unavailable");
    CHECK_NULL_RETURN_WITH_MESSAGE(api->unregisterEvent, ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Content change subscription is not initialized");
    auto code = static_cast<ArkUI_ErrorCode>(api->unregisterEvent(uiContext->id, subscriptionId));
    if (code != ARKUI_ERROR_CODE_NO_ERROR) {
        SET_ERROR_FUNCTION_NAME(__FUNCTION__);
    }
    return code;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetType(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, OH_ArkUI_NativeModule_UIContentChangeEventType* type)
{
    CHECK_NULL_RETURN_WITH_MESSAGE(event, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Event parameter is null");
    CHECK_NULL_RETURN_WITH_MESSAGE(type, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Event type output parameter is null");
    *type = event->type;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetTimestamp(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, uint64_t* nanoTimestamp)
{
    CHECK_NULL_RETURN_WITH_MESSAGE(event, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Event parameter is null");
    CHECK_NULL_RETURN_WITH_MESSAGE(nanoTimestamp, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Timestamp output parameter is null");
    *nanoTimestamp = event->timestamp;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetContext(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, ArkUI_ContextHandle* uiContext)
{
    CHECK_NULL_RETURN_WITH_MESSAGE(uiContext, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "UI context output parameter is null");
    *uiContext = nullptr;
    CHECK_NULL_RETURN_WITH_MESSAGE(event, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Event parameter is null");
    *uiContext = event->context;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetContentChangeEventJson(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, const OH_ArkUI_NativeModule_UIJsonWrapper** json)
{
    CHECK_NULL_RETURN_WITH_MESSAGE(json, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "JSON output parameter is null");
    *json = nullptr;
    CHECK_NULL_RETURN_WITH_MESSAGE(event, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Event parameter is null");
    *json = event->json;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}
#endif

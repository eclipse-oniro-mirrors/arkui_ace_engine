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

#include <new>

#include "core/interfaces/native/node/ui_tree_api.h"

#include "core/interfaces/native/node/page_text_api.h"
#include "interfaces/native/native_error_message_wrapper.h"
#include "interfaces/native/node/node_model.h"

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

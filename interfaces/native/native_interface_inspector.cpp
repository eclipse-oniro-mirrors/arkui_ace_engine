/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Implementation of the inspector tree getter C API declared in
// interfaces/native/native_node.h:
//   OH_ArkUI_NativeModule_RegisterInspectorTreeGetter
//   OH_ArkUI_NativeModule_UnregisterInspectorTreeGetter

#include "native_error_message_macros.h"
#include "native_node.h"
#include "node/config_manager.h"
#include "node/node_model.h"

#include "base/log/log_wrapper.h"
#include "core/components_ng/base/inspector_tree_provider_registry.h"
#include "core/components_ng/base/ui_node.h"


namespace {
ArkUI_ErrorCode ReturnInspectorParamError(const char* functionName, const char* errorMessage)
{
    TAG_LOGE(OHOS::Ace::AceLogTag::ACE_LAYOUT_INSPECTOR, "%{public}s: %{public}s", functionName, errorMessage);
    return ARKUI_ERROR_CODE_PARAM_INVALID;
}

OHOS::Ace::NG::UINode* ResolveNodeAnchor(ArkUI_NodeHandle node)
{
    // ArkUINodeHandle is the raw UINode pointer (same reinterpret semantics as
    // frameworks/core/interfaces/native/node/frame_node_modifier.cpp).
    return reinterpret_cast<OHOS::Ace::NG::UINode*>(node->uiNodeHandle);
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

ArkUI_ErrorCode OH_ArkUI_NativeModule_RegisterInspectorTreeGetter(
    ArkUI_NodeHandle node, void* userData, void (*getInspectorTree)(void* userData, char** inspectorTree))
{
    CHECK_UI_THREAD(__FUNCTION__);
    CHECK_NULL_RETURN_WITH_MESSAGE(OHOS::Ace::NodeModel::InitialFullImpl(), ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Failed to initialize native module");
    if (node == nullptr || getInspectorTree == nullptr) {
        return ReturnInspectorParamError(__FUNCTION__, "node or getInspectorTree is null");
    }
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (node->magic != ARKUI_NODE_MAGIC_VALID) {
        return ReturnInspectorParamError(__FUNCTION__, "node handle is invalid");
    }
    auto* anchorNode = ResolveNodeAnchor(node);
    if (anchorNode == nullptr) {
        return ReturnInspectorParamError(__FUNCTION__, "node anchor is null");
    }
    int32_t result = OHOS::Ace::NG::InspectorTreeProviderRegistry::Register(anchorNode,
        reinterpret_cast<OHOS::Ace::NG::InspectorTreeProviderRegistry::InspectorTreeGetter>(getInspectorTree),
        userData);
    if (result == OHOS::Ace::NG::InspectorTreeProviderRegistry::RESULT_OK) {
        return ARKUI_ERROR_CODE_NO_ERROR;
    }
    if (result == OHOS::Ace::NG::InspectorTreeProviderRegistry::RESULT_UNSUPPORTED_HOST) {
        return ReturnInspectorParamError(__FUNCTION__, "node is not a CustomNode or XComponent host");
    }
    return ReturnInspectorParamError(__FUNCTION__, "invalid anchor or getter");
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_UnregisterInspectorTreeGetter(
    ArkUI_NodeHandle node, void (*getInspectorTree)(void* userData, char** inspectorTree))
{
    CHECK_UI_THREAD(__FUNCTION__);
    CHECK_NULL_RETURN_WITH_MESSAGE(OHOS::Ace::NodeModel::InitialFullImpl(), ARKUI_ERROR_CODE_CAPI_INIT_ERROR,
        __FUNCTION__, "Failed to initialize native module");
    if (node == nullptr) {
        return ReturnInspectorParamError(__FUNCTION__, "node is null");
    }
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (node->magic != ARKUI_NODE_MAGIC_VALID) {
        return ReturnInspectorParamError(__FUNCTION__, "node handle is invalid");
    }
    auto* anchorNode = ResolveNodeAnchor(node);
    if (anchorNode == nullptr) {
        return ReturnInspectorParamError(__FUNCTION__, "node anchor is null");
    }
    // A null getter removes all registrations of the node; a non-matching
    // getter has no effect (see the API definition).
    int32_t result = OHOS::Ace::NG::InspectorTreeProviderRegistry::Unregister(anchorNode,
        reinterpret_cast<OHOS::Ace::NG::InspectorTreeProviderRegistry::InspectorTreeGetter>(getInspectorTree));
    if (result == OHOS::Ace::NG::InspectorTreeProviderRegistry::RESULT_NOT_FOUND) {
        return ReturnInspectorParamError(__FUNCTION__, "getter is not registered on the node");
    }
    if (result != OHOS::Ace::NG::InspectorTreeProviderRegistry::RESULT_OK) {
        return ReturnInspectorParamError(__FUNCTION__, "unregister failed");
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
};
#endif

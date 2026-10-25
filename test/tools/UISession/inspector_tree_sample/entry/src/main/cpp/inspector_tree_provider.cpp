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

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <algorithm>
#include <arkui/native_interface_accessibility.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <napi/native_api.h>
#include <string>
#include <thread>
#include <vector>

namespace {
std::mutex configMutex;
std::string scenario = "provider_normal";
int32_t nodeCount = 2;
int32_t textBytes = 0;
int32_t delayMs = 0;
std::atomic<int32_t> calls { 0 };
constexpr int32_t ROOT_ID = 1234;
constexpr int32_t DEFAULT_PROVIDER_DELAY_MS = 1100;
constexpr int32_t MAX_PROVIDER_NODE_COUNT = 10002;
constexpr int32_t MAX_PROVIDER_TEXT_BYTES = 1024 * 1024;
constexpr int32_t MAX_PROVIDER_DELAY_MS = 10000;

int32_t Query(int64_t id, ArkUI_AccessibilitySearchMode mode, int32_t request, ArkUI_AccessibilityElementInfoList* list)
{
    ++calls;
    std::string current;
    int32_t count;
    int32_t bytes;
    int32_t delay;
    {
        std::lock_guard<std::mutex> lock(configMutex);
        current = scenario;
        count = nodeCount;
        bytes = textBytes;
        delay = delayMs;
    }
    if (current == "provider_empty") {
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
    }
    const bool tree = mode == ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN;
    if (tree && current == "provider_delay") {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay > 0 ? delay : DEFAULT_PROVIDER_DELAY_MS));
    }
    const int32_t size = tree ? count : 1;
    for (int32_t i = 0; i < size; ++i) {
        auto node = OH_ArkUI_AddAndGetAccessibilityElementInfo(list);
        if (!node) {
            return ARKUI_ACCESSIBILITY_NATIVE_RESULT_OUT_OF_MEMORY;
        }
        int32_t elementId = ROOT_ID + i;
        if (tree && current == "provider_invalid" && i == 1) {
            elementId = ROOT_ID;
        }
        OH_ArkUI_AccessibilityElementInfoSetElementId(node, elementId);
        OH_ArkUI_AccessibilityElementInfoSetParentId(node, i == 0 ? -1 : ROOT_ID);
        OH_ArkUI_AccessibilityElementInfoSetComponentType(node, i == 0 ? "FixtureRoot" : "FixtureChild");
        std::string content = bytes > 0 ? std::string(bytes, 'x') : "provider-" + std::to_string(i);
        OH_ArkUI_AccessibilityElementInfoSetContents(node, content.c_str());
        ArkUI_AccessibleRect rect { i * 2, i * 2, i * 2 + 20, i * 2 + 10 };
        OH_ArkUI_AccessibilityElementInfoSetScreenRect(node, &rect);
        OH_ArkUI_AccessibilityElementInfoSetVisible(node, true);
        if (i == 0 && count > 1) {
            std::vector<int64_t> children;
            for (int32_t child = 1; child < count; ++child) {
                children.push_back(ROOT_ID + child);
            }
            OH_ArkUI_AccessibilityElementInfoSetChildNodeIds(node, children.size(), children.data());
        }
    }
    return tree && current == "provider_failure" ? ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED
                                                 : ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

napi_value Configure(napi_env env, napi_callback_info info)
{
    constexpr size_t CONFIGURE_ARG_COUNT = 4;
    constexpr size_t TEXT_BYTES_ARG_INDEX = 2;
    constexpr size_t DELAY_MS_ARG_INDEX = 3;
    napi_value args[CONFIGURE_ARG_COUNT];
    size_t argc = CONFIGURE_ARG_COUNT;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc != CONFIGURE_ARG_COUNT) {
        return nullptr;
    }
    char buffer[80] = {};
    size_t length = 0;
    napi_get_value_string_utf8(env, args[0], buffer, sizeof(buffer), &length);
    std::lock_guard<std::mutex> lock(configMutex);
    scenario.assign(buffer, length);
    napi_get_value_int32(env, args[1], &nodeCount);
    napi_get_value_int32(env, args[TEXT_BYTES_ARG_INDEX], &textBytes);
    napi_get_value_int32(env, args[DELAY_MS_ARG_INDEX], &delayMs);
    nodeCount = std::clamp(nodeCount, 0, MAX_PROVIDER_NODE_COUNT);
    textBytes = std::clamp(textBytes, 0, MAX_PROVIDER_TEXT_BYTES);
    delayMs = std::clamp(delayMs, 0, MAX_PROVIDER_DELAY_MS);
    calls = 0;
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value QueryCount(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_create_int32(env, calls.load(), &result);
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor methods[] = { { "configure", nullptr, Configure, nullptr, nullptr, nullptr, napi_default,
                                               nullptr },
        { "queryCount", nullptr, QueryCount, nullptr, nullptr, nullptr, napi_default, nullptr } };
    constexpr size_t METHOD_COUNT = sizeof(methods) / sizeof(methods[0]);
    napi_define_properties(env, exports, METHOD_COUNT, methods);
    napi_value nativeObject;
    if (napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &nativeObject) == napi_ok) {
        OH_NativeXComponent* component = nullptr;
        if (napi_unwrap(env, nativeObject, reinterpret_cast<void**>(&component)) == napi_ok && component) {
            ArkUI_AccessibilityProvider* provider = nullptr;
            if (OH_NativeXComponent_GetNativeAccessibilityProvider(component, &provider) == 0 && provider) {
                static ArkUI_AccessibilityProviderCallbacks callbacks {};
                callbacks.findAccessibilityNodeInfosById = Query;
                callbacks.findAccessibilityNodeInfosByText = [](int64_t, const char*, int32_t,
                                                                 ArkUI_AccessibilityElementInfoList*) -> int32_t {
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED;
                };
                callbacks.findFocusedAccessibilityNode = [](int64_t, ArkUI_AccessibilityFocusType, int32_t,
                                                             ArkUI_AccessibilityElementInfo*) -> int32_t {
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED;
                };
                callbacks.findNextFocusAccessibilityNode = [](int64_t, ArkUI_AccessibilityFocusMoveDirection, int32_t,
                                                               ArkUI_AccessibilityElementInfo*) -> int32_t {
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED;
                };
                callbacks.executeAccessibilityAction = [](int64_t, ArkUI_Accessibility_ActionType,
                                                           ArkUI_AccessibilityActionArguments*, int32_t) -> int32_t {
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED;
                };
                callbacks.clearFocusedFocusAccessibilityNode = []() -> int32_t {
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
                };
                callbacks.getAccessibilityNodeCursorPosition = [](int64_t, int32_t, int32_t* cursor) -> int32_t {
                    if (cursor) {
                        *cursor = -1;
                    }
                    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_FAILED;
                };
                OH_ArkUI_AccessibilityProviderRegisterCallback(provider, &callbacks);
            }
        }
    }
    return exports;
}

napi_module module { 1, 0, nullptr, Init, "inspector_tree_provider", nullptr, { 0 } };
__attribute__((constructor)) void Register()
{
    napi_module_register(&module);
}
} // namespace

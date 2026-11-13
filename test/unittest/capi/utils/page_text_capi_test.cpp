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
#include <cstdlib>
#include <cstring>
#include <thread>
#include "gtest/gtest.h"
#include "securec.h"
#include "interfaces/native/ui_info_collection.h"
#include "interfaces/native/native_interface.h"
#include "interfaces/native/native_error_message_wrapper.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/node/node_model.h"
#include "core/interfaces/native/node/content_change_api.h"
#include "core/interfaces/native/node/interaction_observer_api.h"
#include "core/interfaces/native/node/page_text_api.h"
#include "core/interfaces/native/node/ui_tree_api.h"
#include "core/interfaces/native/utility/error_message_manager.h"
#include "core/components_ng/base/page_text_json.h"

namespace {
constexpr int32_t CONTENT_CHANGE_INSTANCE_ID = 17;
constexpr uint64_t CONTENT_CHANGE_SUBSCRIPTION_ID = 7;
constexpr uint64_t UNCHANGED_OUTPUT_SENTINEL = 42;
constexpr uint64_t CONTENT_CHANGE_TIMESTAMP = 123;
constexpr char CONTENT_CHANGE_JSON[] = "{\"a\":1}";
constexpr uint32_t PAGE_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_PAGE;
constexpr uint32_t NO_IGNORED_EVENTS = 0;
constexpr uint32_t EMPTY_EVENT_MASK = 0;
constexpr uint64_t INVALID_SUBSCRIPTION_ID = 0;
constexpr uint64_t NONZERO_SUBSCRIPTION_ID = 1;
constexpr uintptr_t NON_NULL_POINTER_SENTINEL = 1;

thread_local bool support = true;
thread_local bool errorSupport = true;
thread_local int32_t collectCode = 0;
thread_local int32_t lastInstance = -1;
bool contentSupport = false;
bool contentRegisterSupport = true;
bool contentUnregisterSupport = true;
int32_t contentRegisterCode = ARKUI_ERROR_CODE_NO_ERROR;
int32_t threadCheckCount = 0;
std::string lastThreadCheckApi;
bool lastWithStart = false;
uint32_t lastEventMask = 0;
uint32_t lastIgnoreMask = 0;
void* contentCookie = nullptr;
void (*contentSink)(uint32_t, uint64_t, const char*, uint32_t, void*) = nullptr;
void (*contentRelease)(void*) = nullptr;

int32_t Collect(int32_t id, char** data, uint32_t* size, const char** reason)
{
    lastInstance = id;
    *data = nullptr;
    *reason = "Injected collection failure.";
    if (collectCode) {
        return collectCode;
    }
    const char payload[] = "{\"schemaVersion\":1,\"texts\":[]}";
    *size = sizeof(payload) - 1;
    *data = static_cast<char*>(std::malloc(sizeof(payload)));
    if (!*data) {
        *reason = "Page text allocation failed.";
        // 100001 is ARKUI_ERROR_CODE_INTERNAL_ERROR for an allocation failure.
        return 100001;
    }
    EXPECT_EQ(memcpy_s(*data, sizeof(payload), payload, sizeof(payload)), EOK);
    return 0;
}
void SetCode(int32_t code, const char* reason)
{
    OHOS::Ace::ErrorMessageManager::GetInstance().SetErrorCodeAndMessage(code, reason);
}
void SetName(const char* name)
{
    OHOS::Ace::ErrorMessageManager::GetInstance().SetFunctionName(name);
}
const char* GetMessage()
{
    return OHOS::Ace::ErrorMessageManager::GetInstance().GetErrorMessage();
}
const ArkUIBasicAPI* Basic()
{
    static ArkUIBasicAPI api {};
    api.setErrorCodeAndMessage = SetCode;
    api.setErrorFunctionName = SetName;
    api.getErrorMessage = GetMessage;
    return &api;
}
}

namespace OHOS::Ace::NodeModel {
const ArkUIInteractionObserverAPI* GetInteractionObserverAPI()
{
    return nullptr;
}

const ArkUITreeAPI* GetTreeAPI()
{
    return nullptr;
}

const ArkUIPageTextAPI* GetPageTextAPI()
{
    static const ArkUIPageTextAPI api {
        Collect,
        [](char* data) {
            std::free(data);
        }
    };
    return support ? &api : nullptr;
}

const ArkUIContentChangeAPI* GetContentChangeAPI()
{
    static ArkUIContentChangeAPI api {};
    api.registerEvent =
        [](int32_t instanceId, bool withStart, uint32_t eventMask, uint32_t ignoreMask, void* cookie,
            void (*sink)(uint32_t, uint64_t, const char*, uint32_t, void*),
            void (*release)(void*), uint64_t* subscriptionId) -> int32_t {
            EXPECT_EQ(instanceId, CONTENT_CHANGE_INSTANCE_ID);
            lastWithStart = withStart;
            lastEventMask = eventMask;
            lastIgnoreMask = ignoreMask;
            if (contentRegisterCode != ARKUI_ERROR_CODE_NO_ERROR) {
                SetCode(contentRegisterCode, "Injected registration failure.");
                *subscriptionId = 0;
                return contentRegisterCode;
            }
            contentCookie = cookie;
            contentSink = sink;
            contentRelease = release;
            *subscriptionId = CONTENT_CHANGE_SUBSCRIPTION_ID;
            return ARKUI_ERROR_CODE_NO_ERROR;
        };
    api.unregisterEvent =
        [](int32_t instanceId, uint64_t subscriptionId) -> int32_t {
            EXPECT_EQ(instanceId, CONTENT_CHANGE_INSTANCE_ID);
            if (subscriptionId != CONTENT_CHANGE_SUBSCRIPTION_ID || !contentCookie) {
                SetCode(ARKUI_ERROR_CODE_PARAM_INVALID, "Unknown subscription.");
                return ARKUI_ERROR_CODE_PARAM_INVALID;
            }
            contentRelease(contentCookie);
            contentCookie = nullptr;
            contentSink = nullptr;
            contentRelease = nullptr;
            return ARKUI_ERROR_CODE_NO_ERROR;
        };
    if (!contentRegisterSupport) {
        api.registerEvent = nullptr;
    }
    if (!contentUnregisterSupport) {
        api.unregisterEvent = nullptr;
    }
    return contentSupport ? &api : nullptr;
}

ArkUIFullNodeAPI* GetFullImplForErrorMessage()
{
    static ArkUIFullNodeAPI api {};
    api.getBasicAPI = Basic;
    return errorSupport ? &api : nullptr;
}
}

namespace OHOS::Ace::NodeModel::ConfigManager {
// This suite exercises the public boundary with mocked backend dispatch.
void CheckUIThread(const char* apiName)
{
    ++threadCheckCount;
    lastThreadCheckApi = apiName;
}
}

class PageTextCapiTest : public testing::Test {
public:
    void SetUp() override
    {
        errorSupport = true;
        support = true;
        collectCode = 0;
        contentSupport = false;
        contentRegisterSupport = true;
        contentUnregisterSupport = true;
        contentRegisterCode = ARKUI_ERROR_CODE_NO_ERROR;
        threadCheckCount = 0;
        lastThreadCheckApi.clear();
        lastWithStart = false;
        lastEventMask = 0;
        lastIgnoreMask = 0;
    }

    void TearDown() override
    {
        if (contentCookie && contentRelease) {
            contentRelease(contentCookie);
        }
        contentCookie = nullptr;
        contentSink = nullptr;
        contentRelease = nullptr;
    }
};

TEST_F(PageTextCapiTest, validationOrderAndSharedDiagnostics)
{
    ArkUI_Context context { 17 };
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    // 401 is ARKUI_ERROR_CODE_PARAM_INVALID for the null output slot.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(nullptr, nullptr), 401);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("output slot"), std::string::npos);
    // 190001 is ARKUI_ERROR_CODE_UI_CONTEXT_INVALID for the null UI context.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(nullptr, &result), 190001);
    support = false;
    // 500 is ARKUI_ERROR_CODE_CAPI_INIT_ERROR while the query implementation is unavailable.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &result), 500);
    auto message = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    EXPECT_NE(message.find("500"), std::string::npos);
    EXPECT_NE(message.find("OH_ArkUI_NativeModule_UIAgentGetPageText"), std::string::npos);
    EXPECT_NE(message.find("unavailable"), std::string::npos);
    EXPECT_EQ(result, nullptr);
}

TEST_F(PageTextCapiTest, contentChangeValidationAndFailedAccessorOutputs)
{
    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    uint64_t subscriptionId = UNCHANGED_OUTPUT_SENTINEL;
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent*, void*) {};
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        nullptr, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, nullptr, &subscriptionId),
        ARKUI_ERROR_CODE_CALLBACK_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, EMPTY_EVENT_MASK, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, nullptr),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("Subscription ID output parameter is null"),
        std::string::npos);
    EXPECT_EQ(contentCookie, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
        ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
    EXPECT_EQ(subscriptionId, UNCHANGED_OUTPUT_SENTINEL);
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, INVALID_SUBSCRIPTION_ID),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, NONZERO_SUBSCRIPTION_ID),
        ARKUI_ERROR_CODE_CAPI_INIT_ERROR);

    auto type = OH_ARKUI_NATIVEMODULE_EVENT_TABS_END;
    uint64_t timestamp = UNCHANGED_OUTPUT_SENTINEL;
    ArkUI_ContextHandle outputContext = &context;
    const OH_ArkUI_NativeModule_UIJsonWrapper* json =
        reinterpret_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(NON_NULL_POINTER_SENTINEL);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetType(nullptr, &type), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetTimestamp(nullptr, &timestamp),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContext(nullptr, &outputContext),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContentChangeEventJson(nullptr, &json),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(type, OH_ARKUI_NATIVEMODULE_EVENT_TABS_END);
    EXPECT_EQ(timestamp, UNCHANGED_OUTPUT_SENTINEL);
    EXPECT_EQ(outputContext, nullptr);
    EXPECT_EQ(json, nullptr);
}

TEST_F(PageTextCapiTest, contentChangeCallbackBorrowsStableSnapshot)
{
    struct CallbackResult {
        ArkUI_ContextHandle expected = nullptr;
        int32_t calls = 0;
        std::string json;
    } result;
    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    result.expected = &context;
    contentSupport = true;
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent* event, void* userData) {
        auto* result = static_cast<CallbackResult*>(userData);
        ++result->calls;
        OH_ArkUI_NativeModule_UIContentChangeEventType type;
        uint64_t timestamp = 0;
        ArkUI_ContextHandle outputContext = nullptr;
        const OH_ArkUI_NativeModule_UIJsonWrapper* json = nullptr;
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetType(event, &type), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetTimestamp(event, &timestamp), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContext(event, &outputContext),
            ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContentChangeEventJson(event, &json),
            ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetType(event, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("UIContentChangeEventGetType"),
            std::string::npos);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetTimestamp(event, nullptr),
            ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("UIContentChangeEventGetTimestamp"),
            std::string::npos);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContext(event, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("UIContentChangeEventGetContext"),
            std::string::npos);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIContentChangeEventGetContentChangeEventJson(event, nullptr),
            ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("GetContentChangeEventJson"),
            std::string::npos);
        EXPECT_EQ(type, OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END);
        EXPECT_EQ(timestamp, CONTENT_CHANGE_TIMESTAMP);
        EXPECT_EQ(outputContext, result->expected);
        result->json.assign(OH_ArkUI_NativeModule_UIJsonWrapperGetData(json),
            OH_ArkUI_NativeModule_UIJsonWrapperGetSize(json));
    };
    uint64_t subscriptionId = 0;
    constexpr uint32_t ignoreMask = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLBY |
        OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_TOAST;
    ASSERT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(&context, true,
        OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_ALL, ignoreMask, &result, callback, &subscriptionId),
        ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(subscriptionId, CONTENT_CHANGE_SUBSCRIPTION_ID);
    EXPECT_TRUE(lastWithStart);
    EXPECT_EQ(lastEventMask, OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_ALL);
    EXPECT_EQ(lastIgnoreMask, ignoreMask);
    ASSERT_NE(contentSink, nullptr);
    contentSink(OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END, CONTENT_CHANGE_TIMESTAMP, CONTENT_CHANGE_JSON,
        sizeof(CONTENT_CHANGE_JSON) - 1, contentCookie);
    EXPECT_EQ(result.calls, 1);
    EXPECT_EQ(result.json, CONTENT_CHANGE_JSON);
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, subscriptionId),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(contentCookie, nullptr);
}

TEST_F(PageTextCapiTest, contentChangeMaskValidationPreservesOutput)
{
    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent*, void*) {};
    contentSupport = true;
    constexpr uint32_t unknownBit = 0x8;
    constexpr uint32_t knownBits = 0x7;
    constexpr uint32_t allBits = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_ALL;
    const uint32_t invalidMasks[][2] = {
        { 0, 0 }, { unknownBit, 0 }, { PAGE_CATEGORY | unknownBit, 0 },
        { PAGE_CATEGORY, unknownBit }, { PAGE_CATEGORY, PAGE_CATEGORY | unknownBit }, { PAGE_CATEGORY, allBits }
    };
    for (const auto& masks : invalidMasks) {
        uint64_t id = UNCHANGED_OUTPUT_SENTINEL;
        EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
            &context, true, masks[0], masks[1], nullptr, callback, &id), ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(id, UNCHANGED_OUTPUT_SENTINEL);
        EXPECT_EQ(contentCookie, nullptr);
        EXPECT_EQ(lastEventMask, 0U);
    }
    for (uint32_t eventMask : { PAGE_CATEGORY, knownBits, allBits }) {
        uint64_t id = UNCHANGED_OUTPUT_SENTINEL;
        ASSERT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
            &context, true, eventMask, knownBits, nullptr, callback, &id), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_NE(id, UNCHANGED_OUTPUT_SENTINEL);
        EXPECT_EQ(lastEventMask, eventMask);
        EXPECT_EQ(lastIgnoreMask, knownBits);
        EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, id), ARKUI_ERROR_CODE_NO_ERROR);
    }
}

TEST_F(PageTextCapiTest, contentChangeBridgeInitializationFailures)
{
    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    uint64_t subscriptionId = UNCHANGED_OUTPUT_SENTINEL;
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent*, void*) {};
    contentSupport = true;
    bool* registrationFlags[] = { &contentSupport, &contentRegisterSupport, &contentUnregisterSupport };
    for (auto* flag : registrationFlags) {
        *flag = false;
        EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
            &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
            ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
        EXPECT_EQ(subscriptionId, UNCHANGED_OUTPUT_SENTINEL);
        EXPECT_EQ(contentCookie, nullptr);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("RegisterUIContentChangeEvent"),
            std::string::npos);
        *flag = true;
    }
    bool* unregistrationFlags[] = { &contentSupport, &contentUnregisterSupport };
    for (auto* flag : unregistrationFlags) {
        *flag = false;
        EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, subscriptionId),
            ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("UnRegisterUIContentChangeEvent"),
            std::string::npos);
        *flag = true;
    }
}

TEST_F(PageTextCapiTest, contentChangeChecksThreadBeforeValidation)
{
    int32_t expectedThreadChecks = 0;
    uint64_t subscriptionId = UNCHANGED_OUTPUT_SENTINEL;
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        nullptr, false, EMPTY_EVENT_MASK, NO_IGNORED_EVENTS, nullptr, nullptr, &subscriptionId),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    ++expectedThreadChecks;
    EXPECT_EQ(threadCheckCount, expectedThreadChecks);
    EXPECT_EQ(lastThreadCheckApi, "OH_ArkUI_NativeModule_RegisterUIContentChangeEvent");
    EXPECT_EQ(subscriptionId, UNCHANGED_OUTPUT_SENTINEL);
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(nullptr, INVALID_SUBSCRIPTION_ID),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    ++expectedThreadChecks;
    EXPECT_EQ(threadCheckCount, expectedThreadChecks);
    EXPECT_EQ(lastThreadCheckApi, "OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent");

    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    contentSupport = true;
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent*, void*) {};
    ASSERT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
        ARKUI_ERROR_CODE_NO_ERROR);
    ++expectedThreadChecks;
    EXPECT_EQ(threadCheckCount, expectedThreadChecks);
    EXPECT_EQ(lastThreadCheckApi, "OH_ArkUI_NativeModule_RegisterUIContentChangeEvent");
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, subscriptionId),
        ARKUI_ERROR_CODE_NO_ERROR);
    ++expectedThreadChecks;
    EXPECT_EQ(threadCheckCount, expectedThreadChecks);
    EXPECT_EQ(lastThreadCheckApi, "OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent");
}

TEST_F(PageTextCapiTest, contentChangeBackendFailurePreservesOutputAndDiagnostics)
{
    ArkUI_Context context { CONTENT_CHANGE_INSTANCE_ID };
    uint64_t subscriptionId = UNCHANGED_OUTPUT_SENTINEL;
    auto callback = [](const OH_ArkUI_NativeModule_UIContentChangeEvent*, void*) {};
    contentSupport = true;
    contentRegisterCode = ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    EXPECT_EQ(OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
        &context, false, PAGE_CATEGORY, NO_IGNORED_EVENTS, nullptr, callback, &subscriptionId),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(subscriptionId, UNCHANGED_OUTPUT_SENTINEL);
    EXPECT_EQ(contentCookie, nullptr);
    auto message = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    EXPECT_NE(message.find("RegisterUIContentChangeEvent"), std::string::npos);
    EXPECT_NE(message.find("Injected registration failure"), std::string::npos);
    EXPECT_EQ(OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(&context, subscriptionId),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    message = OH_ArkUI_NativeModule_GetErrorMessage();
    EXPECT_NE(message.find("UnRegisterUIContentChangeEvent"), std::string::npos);
    EXPECT_NE(message.find("Unknown subscription"), std::string::npos);
}

TEST_F(PageTextCapiTest, failureRecoveryAndIndependentSnapshots)
{
    // Instance ID 17 is a non-default mock instance used to verify forwarding through the public API.
    ArkUI_Context context { 17 };
    OH_ArkUI_NativeModule_UIJsonWrapper* old = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &old), 0);
    // The collector must receive the exact mock instance ID 17 supplied by the caller.
    EXPECT_EQ(lastInstance, 17);
    OH_ArkUI_NativeModule_UIJsonWrapper* result = old;
    // Inject 100001 (ARKUI_ERROR_CODE_INTERNAL_ERROR) to test failure followed by recovery.
    collectCode = 100001;
    // 100001 is the injected ARKUI_ERROR_CODE_INTERNAL_ERROR and must reach the caller unchanged.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &result), 100001);
    EXPECT_EQ(result, nullptr);
    auto error = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    EXPECT_NE(error.find("Injected collection failure"), std::string::npos);
    collectCode = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &result), 0);
    EXPECT_EQ(OH_ArkUI_NativeModule_GetErrorMessage(), error);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(old), "{\"schemaVersion\":1,\"texts\":[]}");
    EXPECT_NE(old, result);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(old);
}

TEST_F(PageTextCapiTest, originalErrorChannelAvailabilityAndThreadIsolation)
{
    errorSupport = false;
    support = false;
    ArkUI_Context context { 1 };
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    // 500 is ARKUI_ERROR_CODE_CAPI_INIT_ERROR while the query implementation is unavailable.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &result), 500);
    EXPECT_EQ(result, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_GetErrorMessage(), "");
    OH_ArkUI_NativeModule_UIAgentGetPageText(nullptr, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_GetErrorMessage(), "");

    errorSupport = true;
    // 500 is ARKUI_ERROR_CODE_CAPI_INIT_ERROR while the query implementation is unavailable.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetPageText(&context, &result), 500);
    auto message = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    // The diagnostic must contain code 500, matching the unavailable query implementation.
    EXPECT_NE(message.find("500"), std::string::npos);
    std::thread worker([] {
        OH_ArkUI_NativeModule_UIAgentGetPageText(nullptr, nullptr);
        // Code 401 identifies the worker thread null-output error and must not replace the caller diagnostic.
        EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("401"), std::string::npos);
    });
    worker.join();
    EXPECT_EQ(OH_ArkUI_NativeModule_GetErrorMessage(), message);
    // Record 401 (ARKUI_ERROR_CODE_PARAM_INVALID) to verify replacement of the preceding diagnostic.
    OHOS::Ace::SetErrorMessageByModifier(401, "anotherAPI", "new error");
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("anotherAPI"), std::string::npos);
}

TEST(UIJsonWrapperTest, createRejectsNullDataAndZeroSize)
{
    const char source[] = "{}";
    OH_ArkUI_NativeModule_UIJsonWrapper* original = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(source, 2, &original), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(original, nullptr);

    auto* result = original;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(nullptr, 2, &result), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(result, nullptr);

    result = original;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(source, 0, &result), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(result, nullptr);

    result = original;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(nullptr, 0, &result), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(source, 2, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);

    // Rejecting a request must not destroy the object previously stored in the output slot.
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(original), source);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(original);
}

TEST(UIJsonWrapperTest, createCopiesNonTerminatedBufferBySize)
{
    const char source[] = { '{', '}', 'x' };
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(source, 2, &result), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(result), "{}");
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(result), 2u);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
}

TEST(UIJsonWrapperTest, immutableCopyMetadataNullAndConcurrentReaders)
{
    char source[] = "{\"texts\":[]}";
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(source, 12, &result), 0);
    source[0] = '!';
    auto read = [result] {
        EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(result), "{\"texts\":[]}");
        EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(result), 12u);
    };
    std::thread worker(read);
    read();
    worker.join();
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(nullptr), nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(nullptr), 0u);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(nullptr);
}

TEST(PageTextJsonTest, utf8GetterPayloadAndEmbeddedNul)
{
    OHOS::Ace::NG::PageTextJson json;
    std::string text = "中文e\u0301אב\"\\\n";
    text.push_back(0);
    text += "�";
    ASSERT_TRUE(json.String(text));
    uint32_t size = 0;
    char* data = json.Release(size);
    EXPECT_EQ(std::string(data, size), "\"中文e\u0301אב\\\"\\\\\\u000a\\u0000�\"");
    EXPECT_EQ(std::strlen(data), size);
    std::free(data);
}

TEST_F(PageTextCapiTest, sharedErrorChannelPreservesLongLegacyMessages)
{
    // A 1024-byte reason verifies that long legacy diagnostics are preserved without truncation.
    std::string longReason(1024, 'r');
    // A 256-byte function name verifies preservation of long names in the shared diagnostic channel.
    std::string longName(256, 'n');
    // Use 401 (ARKUI_ERROR_CODE_PARAM_INVALID) as the error paired with the long diagnostic strings.
    OHOS::Ace::SetErrorMessageByModifier(401, longName.c_str(), longReason.c_str());
    auto message = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    EXPECT_NE(message.find(longName), std::string::npos);
    EXPECT_NE(message.find(longReason), std::string::npos);
    // 500 (ARKUI_ERROR_CODE_CAPI_INIT_ERROR) replaces the code while retaining the existing function name.
    OHOS::Ace::SetErrorCodeAndMessageByModifier(500, "short reason");
    message = OH_ArkUI_NativeModule_GetErrorMessage();
    EXPECT_NE(message.find(longName), std::string::npos);
    EXPECT_NE(message.find("short reason"), std::string::npos);
    OHOS::Ace::SetErrorFunctionNameByModifier("short name");
    EXPECT_STREQ(OH_ArkUI_NativeModule_GetErrorMessage(),
        "errorCode: 500, functionName: short name, errorMessage: short reason");
}

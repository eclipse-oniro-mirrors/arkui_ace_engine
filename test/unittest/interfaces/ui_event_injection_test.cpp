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
#include <cstring>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "interfaces/native/ui_event_injection.h"
#include "interfaces/native/ui_json_wrapper.h"
#include "interfaces/native/native_type.h"
#include "interfaces/native/native_interface.h"
#include "interfaces/native/node/node_model.h"
#include "frameworks/core/interfaces/arkoala/arkoala_api.h"

using namespace testing;
using namespace testing::ext;

namespace {
struct CallbackState {
    int32_t count = 0;
    OH_ArkUI_NativeModule_UIEventInjection_ResultCode result =
        OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS;
};

struct MockJsonWrapper {
    std::string data;
};

ArkUI_ErrorCode g_mockResult = ARKUI_ERROR_CODE_NO_ERROR;
bool g_slotCalled = false;
bool g_returnNullImpl = false;
bool g_implInitialized = true;
int32_t g_initializationAttempts = 0;
bool g_returnNullNodeModifiers = false;
bool g_returnNullFrameNodeModifier = false;
bool g_returnNullSlot = false;
bool g_returnNullJsonData = false;
ArkUI_Int32 g_capturedInstanceId = -1;
ArkUI_Uint32 g_capturedUniqueId = 0;
std::string g_capturedJson;
void* g_capturedUserData = nullptr;
void (*g_capturedCallback)(ArkUI_Int32, void*) = nullptr;
bool g_completeInline = false;
ArkUI_Int32 g_mockErrorCode = 0;
std::string g_mockErrorMsg;
std::string g_mockFuncName;
std::string g_mockFormattedError;

void CompleteCapturedCommand(ArkUI_Int32 result = OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS)
{
    auto callback = g_capturedCallback;
    auto* userData = g_capturedUserData;
    g_capturedCallback = nullptr;
    g_capturedUserData = nullptr;
    ASSERT_NE(callback, nullptr);
    auto typedCallback = reinterpret_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(callback);
    typedCallback(static_cast<OH_ArkUI_NativeModule_UIEventInjection_ResultCode>(result), userData);
}
} // namespace

extern "C" {
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIJsonWrapperCreate(const char* data, uint32_t size,
    OH_ArkUI_NativeModule_UIJsonWrapper** json)
{
    if (data == nullptr || json == nullptr) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto* w = new MockJsonWrapper{ std::string(data, size) };
    *json = reinterpret_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(w);
    return ARKUI_ERROR_CODE_NO_ERROR;
}

const char* OH_ArkUI_NativeModule_UIJsonWrapperGetData(const OH_ArkUI_NativeModule_UIJsonWrapper* json)
{
    if (json == nullptr || g_returnNullJsonData) {
        return nullptr;
    }
    return reinterpret_cast<const MockJsonWrapper*>(json)->data.c_str();
}

uint32_t OH_ArkUI_NativeModule_UIJsonWrapperGetSize(const OH_ArkUI_NativeModule_UIJsonWrapper* json)
{
    if (json == nullptr) {
        return 0;
    }
    return static_cast<uint32_t>(reinterpret_cast<const MockJsonWrapper*>(json)->data.size());
}

void OH_ArkUI_NativeModule_UIJsonWrapperDestroy(OH_ArkUI_NativeModule_UIJsonWrapper* json)
{
    delete reinterpret_cast<MockJsonWrapper*>(json);
}
}

namespace OHOS::Ace::NodeModel {
namespace {
ArkUI_Int32 MockInjectCompositeCommand(const ArkUIInjectCommandParams* params)
{
    const auto result = g_mockResult;
    g_capturedCallback = result == ARKUI_ERROR_CODE_NO_ERROR ? params->callback : nullptr;
    g_slotCalled = true;
    g_capturedInstanceId = params->instanceId;
    g_capturedUniqueId = params->uniqueId;
    if (params->json != nullptr && params->jsonSize > 0) {
        g_capturedJson = std::string(params->json, params->jsonSize);
    }
    g_capturedUserData = result == ARKUI_ERROR_CODE_NO_ERROR ? params->userData : nullptr;
    if (result == ARKUI_ERROR_CODE_NO_ERROR && g_completeInline) {
        CompleteCapturedCommand();
    }
    return static_cast<ArkUI_Int32>(result);
}

const ArkUIFrameNodeModifier g_mockFrameNodeModifier = {
    .injectCompositeCommand = MockInjectCompositeCommand,
};

const ArkUIFrameNodeModifier g_mockFrameNodeModifierNullSlot = {};

const ArkUIFrameNodeModifier* MockGetFrameNodeModifier()
{
    return g_returnNullSlot ? &g_mockFrameNodeModifierNullSlot : &g_mockFrameNodeModifier;
}

const ArkUINodeModifiers g_mockNodeModifiers = {
    .getFrameNodeModifier = MockGetFrameNodeModifier,
};

const ArkUINodeModifiers* MockGetNodeModifiers()
{
    return g_returnNullNodeModifiers ? nullptr : &g_mockNodeModifiers;
}

void MockRefreshFormattedError()
{
    g_mockFormattedError = "errorCode: " + std::to_string(g_mockErrorCode);
    if (!g_mockFuncName.empty()) {
        g_mockFormattedError += ", functionName: " + g_mockFuncName;
    }
    if (!g_mockErrorMsg.empty()) {
        g_mockFormattedError += ", errorMessage: " + g_mockErrorMsg;
    }
}

void MockSetErrorCodeAndMessage(ArkUI_Int32 errorCode, ArkUI_CharPtr errorMessage)
{
    g_mockErrorCode = errorCode;
    g_mockErrorMsg = (errorMessage != nullptr) ? errorMessage : "";
    MockRefreshFormattedError();
}

void MockSetErrorFunctionName(ArkUI_CharPtr functionName)
{
    g_mockFuncName = (functionName != nullptr) ? functionName : "";
    MockRefreshFormattedError();
}

const char* MockGetErrorMessage()
{
    return g_mockFormattedError.c_str();
}

const ArkUIBasicAPI g_mockBasicAPI = {
    .setErrorCodeAndMessage = MockSetErrorCodeAndMessage,
    .setErrorFunctionName = MockSetErrorFunctionName,
    .getErrorMessage = MockGetErrorMessage,
};

const ArkUIBasicAPI* MockGetBasicAPI()
{
    return &g_mockBasicAPI;
}

ArkUIFullNodeAPI g_mockImpl = {
    .getNodeModifiers = MockGetNodeModifiers,
    .getBasicAPI = MockGetBasicAPI,
};
} // namespace

bool InitialFullImpl()
{
    ++g_initializationAttempts;
    if (g_returnNullImpl) {
        return false;
    }
    g_implInitialized = true;
    return true;
}

ArkUIFullNodeAPI* GetFullImpl()
{
    if (g_returnNullImpl || !g_implInitialized) {
        return nullptr;
    }
    return &g_mockImpl;
}

ArkUIFullNodeAPI* GetOrCreateFullImpl()
{
    if (!GetFullImpl() && !InitialFullImpl()) {
        return nullptr;
    }
    return GetFullImpl();
}

ArkUIFullNodeAPI* GetFullImplForErrorMessage()
{
    return &g_mockImpl;
}
} // namespace OHOS::Ace::NodeModel

extern "C" {
const char* OH_ArkUI_NativeModule_GetErrorMessage()
{
    const auto* impl = OHOS::Ace::NodeModel::GetFullImplForErrorMessage();
    if (impl == nullptr || impl->getBasicAPI == nullptr) {
        return "";
    }
    return impl->getBasicAPI()->getErrorMessage();
}
}

class UIEventInjectionTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    }

    void SetUp() override
    {
        g_mockResult = ARKUI_ERROR_CODE_NO_ERROR;
        g_slotCalled = false;
        g_returnNullImpl = false;
        g_implInitialized = true;
        g_initializationAttempts = 0;
        g_returnNullNodeModifiers = false;
        g_returnNullFrameNodeModifier = false;
        g_returnNullSlot = false;
        g_returnNullJsonData = false;
        g_capturedInstanceId = -1;
        g_capturedUniqueId = 0;
        g_capturedJson.clear();
        g_capturedUserData = nullptr;
        g_capturedCallback = nullptr;
        g_completeInline = false;
        g_mockErrorCode = 0;
        g_mockErrorMsg.clear();
        g_mockFuncName.clear();
        g_mockFormattedError.clear();
        testCtx_.context.id = 0;
        uiContext_ = reinterpret_cast<ArkUI_ContextHandle>(&testCtx_);
    }

    void TearDown() override
    {
        EXPECT_EQ(g_capturedCallback, nullptr); // Every accepted mock command must complete in its test.
        for (auto* w : wrappers_) {
            OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
        }
        wrappers_.clear();
    }

protected:
    OH_ArkUI_NativeModule_UIJsonWrapper* MakeCommand(const char* data)
    {
        OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
        uint32_t size = data != nullptr ? static_cast<uint32_t>(strlen(data)) : 0;
        OH_ArkUI_NativeModule_UIJsonWrapperCreate(data != nullptr ? data : "", size, &w);
        if (w != nullptr) {
            wrappers_.push_back(w);
        }
        return w;
    }

    struct {
        ArkUI_Context context{ .id = 0 };
    } testCtx_;
    ArkUI_ContextHandle uiContext_ = nullptr;
    std::vector<OH_ArkUI_NativeModule_UIJsonWrapper*> wrappers_;
};

static void Callback(OH_ArkUI_NativeModule_UIEventInjection_ResultCode result, void* userData)
{
    if (userData != nullptr) {
        auto& state = *static_cast<CallbackState*>(userData);
        state.result = result;
        ++state.count;
    }
}

// Merged: InjectNullUiContext001 + InjectNullCommand001 + InjectNullCallbackRejected001
// Covers impl L31 (uiContext==nullptr→190001) + L35 (command==nullptr→401) + L64 (callback==nullptr→401).
HWTEST_F(UIEventInjectionTest, InjectNullParamRejections001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);

    // NULL uiContext → 190001
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        nullptr, 1, cmd, nullptr, nullptr), ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_FALSE(g_slotCalled);

    // NULL command → 401
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, nullptr, nullptr, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_FALSE(g_slotCalled);

    // NULL callback → 401
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, nullptr, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_FALSE(g_slotCalled);
}

HWTEST_F(UIEventInjectionTest, InjectTableNull001, TestSize.Level1)
{
    g_implInitialized = false;
    g_returnNullImpl = true;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState callbackResult;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &callbackResult), ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
    EXPECT_EQ(g_initializationAttempts, 1);
    EXPECT_FALSE(g_implInitialized);
    EXPECT_EQ(callbackResult.count, 0);
    EXPECT_FALSE(g_slotCalled);
}

HWTEST_F(UIEventInjectionTest, InjectInitializesOnDemand001, TestSize.Level1)
{
    g_implInitialized = false;
    auto* cmd = MakeCommand("{\"schemaVersion\":1,\"cmd\":{\"type\":\"copy\",\"action_info\":{}}}");
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, nullptr), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(g_initializationAttempts, 1);
    EXPECT_TRUE(g_implInitialized);
    EXPECT_TRUE(g_slotCalled);
    CompleteCapturedCommand();
}

// Merged: InjectNodeModifiersNull001 + InjectSlotNull001
// Covers impl L54 (nodeModifiers==nullptr→500) + L59 (frameNodeModifier/slot==nullptr→500).
HWTEST_F(UIEventInjectionTest, InjectInitFailureChain001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);

    // nodeModifiers == nullptr → 500
    g_returnNullNodeModifiers = true;
    {
        CallbackState callbackResult;
        EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
            uiContext_, 1, cmd, Callback, &callbackResult), ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
        EXPECT_EQ(callbackResult.count, 0);
        EXPECT_FALSE(g_slotCalled);
    }
    g_returnNullNodeModifiers = false;

    // frameNodeModifier/injectCompositeCommand == nullptr → 500
    g_returnNullSlot = true;
    {
        CallbackState callbackResult;
        EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
            uiContext_, 1, cmd, Callback, &callbackResult), ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
        EXPECT_EQ(callbackResult.count, 0);
        EXPECT_FALSE(g_slotCalled);
    }
    g_returnNullSlot = false;
}

HWTEST_F(UIEventInjectionTest, InjectDelegatesToSlot001, TestSize.Level1)
{
    const char* json = "{\"schemaVersion\":1,\"cmd\":{\"type\":\"setText\",\"action_info\":{\"value\":\"hi\"}}}";
    auto* cmd = MakeCommand(json);
    ASSERT_NE(cmd, nullptr);
    g_mockResult = ARKUI_ERROR_CODE_NO_ERROR;

    CallbackState slot;
    auto ret = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 4242u, cmd,
        static_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(Callback), &slot);

    EXPECT_EQ(ret, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(g_initializationAttempts, 0);
    EXPECT_TRUE(g_slotCalled);
    EXPECT_EQ(g_capturedUniqueId, 4242u);
    EXPECT_EQ(g_capturedInstanceId, testCtx_.context.id);
    EXPECT_EQ(g_capturedJson, std::string(json));
    EXPECT_EQ(g_capturedUserData, &slot);
    EXPECT_EQ(reinterpret_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(g_capturedCallback), Callback);
    EXPECT_EQ(slot.count, 0);
    CompleteCapturedCommand();
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS);
}

HWTEST_F(UIEventInjectionTest, InjectReturnCodePropagated001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);

    g_mockResult = ARKUI_ERROR_CODE_COMMAND_UNFINISHED;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd,
        static_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(Callback), nullptr),
        ARKUI_ERROR_CODE_COMMAND_UNFINISHED);

    g_mockResult = ARKUI_ERROR_CODE_PARAM_INVALID;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd,
        static_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(Callback), nullptr),
        ARKUI_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(UIEventInjectionTest, CallbackResultAndUserDataPreserved001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"copy\"}");
    ASSERT_NE(cmd, nullptr);
    for (auto result : { OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS,
             OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND,
             OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED,
             OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_INTERRUPTED_BY_USER }) {
        CallbackState state;
        EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
            uiContext_, 1, cmd, Callback, &state), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.count, 0);
        CompleteCapturedCommand(static_cast<ArkUI_Int32>(result));
        EXPECT_EQ(state.count, 1);
        EXPECT_EQ(state.result, result);
    }
}

HWTEST_F(UIEventInjectionTest, InlineCompletionPreservesCallback001, TestSize.Level1)
{
    g_completeInline = true;
    auto* cmd = MakeCommand("{\"cmd\":\"copy\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState state;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &state), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS);
    EXPECT_EQ(g_capturedCallback, nullptr);
}

HWTEST_F(UIEventInjectionTest, CallbackReentryPreservesUserData001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"copy\"}");
    ASSERT_NE(cmd, nullptr);
    struct ReentryState {
        ArkUI_ContextHandle uiContext;
        const OH_ArkUI_NativeModule_UIJsonWrapper* command;
        int32_t count = 0;
        CallbackState next;
    } state { uiContext_, cmd };
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(uiContext_, 1, cmd,
        [](OH_ArkUI_NativeModule_UIEventInjection_ResultCode result, void* userData) {
            auto& state = *static_cast<ReentryState*>(userData);
            ++state.count;
            EXPECT_EQ(result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS);
            EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
                state.uiContext, 2, state.command, Callback, &state.next), ARKUI_ERROR_CODE_NO_ERROR);
        }, &state), ARKUI_ERROR_CODE_NO_ERROR);
    CompleteCapturedCommand();
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.next.count, 0);
    CompleteCapturedCommand(OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND);
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.next.count, 1);
    EXPECT_EQ(state.next.result, OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND);
}

HWTEST_F(UIEventInjectionTest, InjectInstanceIdDerived001, TestSize.Level1)
{
    testCtx_.context.id = 777;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(uiContext_, 1, cmd,
        static_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(Callback), nullptr);
    EXPECT_EQ(g_capturedInstanceId, 777);
    CompleteCapturedCommand();
}

HWTEST_F(UIEventInjectionTest, InjectNullCallbackRejected001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, nullptr, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_FALSE(g_slotCalled);
}

// Covers ui_event_injection_impl.cpp L41: jsonData == nullptr (defensive branch).
HWTEST_F(UIEventInjectionTest, InjectNullJsonDataFromWrapper001, TestSize.Level1)
{
    g_returnNullJsonData = true;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState callbackResult;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &callbackResult), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(callbackResult.count, 0);
    EXPECT_FALSE(g_slotCalled);
    const char* msg = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(msg, nullptr);
    EXPECT_NE(std::string(msg).find("Command JSON data is null"), std::string::npos);
    g_returnNullJsonData = false;
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterNullUiContext001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        nullptr, 1, cmd, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: UI context is null"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterNullCommand001, TestSize.Level1)
{
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, nullptr, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: Command parameter is null"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterNullCallback001, TestSize.Level1)
{
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: callback must not be null"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterInitError001, TestSize.Level1)
{
    g_returnNullImpl = true;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState callbackResult;
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &callbackResult);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
    EXPECT_EQ(callbackResult.count, 0);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: Native module not initialized"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterNodeModifiersNull001, TestSize.Level1)
{
    g_returnNullNodeModifiers = true;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState callbackResult;
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &callbackResult);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
    EXPECT_EQ(callbackResult.count, 0);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: Node modifiers not initialized"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageAfterSlotNull001, TestSize.Level1)
{
    g_returnNullSlot = true;
    auto* cmd = MakeCommand("{\"cmd\":\"x\"}");
    ASSERT_NE(cmd, nullptr);
    CallbackState callbackResult;
    auto result = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 1, cmd, Callback, &callbackResult);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_CAPI_INIT_ERROR);
    EXPECT_EQ(callbackResult.count, 0);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_NE(msgStr.find("errorCode: " + std::to_string(result)), std::string::npos);
    EXPECT_NE(msgStr.find("functionName: OH_ArkUI_NativeModule_UIEventInjectCompositeCommand"), std::string::npos);
    EXPECT_NE(msgStr.find("errorMessage: Frame node modifier not initialized"), std::string::npos);
}

HWTEST_F(UIEventInjectionTest, GetErrorMessageNoErrorAfterSuccess001, TestSize.Level1)
{
    const char* json = "{\"schemaVersion\":1,\"cmd\":{\"type\":\"setText\",\"action_info\":{\"value\":\"hi\"}}}";
    auto* cmd = MakeCommand(json);
    ASSERT_NE(cmd, nullptr);
    g_mockResult = ARKUI_ERROR_CODE_NO_ERROR;
    CallbackState slot;
    auto ret = OH_ArkUI_NativeModule_UIEventInjectCompositeCommand(
        uiContext_, 4242u, cmd,
        static_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(Callback), &slot);
    EXPECT_EQ(ret, ARKUI_ERROR_CODE_NO_ERROR);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string msgStr(errorMessage);
    EXPECT_EQ(msgStr.find("errorMessage: UI context is null"), std::string::npos);
    EXPECT_EQ(msgStr.find("errorMessage: Command parameter is null"), std::string::npos);
    EXPECT_EQ(msgStr.find("errorMessage: callback must not be null"), std::string::npos);
    CompleteCapturedCommand();
    EXPECT_EQ(slot.count, 1);
}

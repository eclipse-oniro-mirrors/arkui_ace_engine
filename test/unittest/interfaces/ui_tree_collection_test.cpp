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
#include <array>
#include <cstdlib>
#include <cstring>
#include <string>

#include "gtest/gtest.h"
#include "interfaces/inner_api/ace/node_module_inner.h"
#include "interfaces/native/ui_info_collection.h"
#include "securec.h"

#include "core/interfaces/native/node/page_text_api.h"
#include "core/interfaces/native/node/ui_tree_api.h"

using namespace testing;
using namespace testing::ext;
namespace {
ArkUITreeOptions captured;
ArkUITreeCompletion accepted;
uint64_t acceptedId = 0;
int32_t collectionResult = ARKUI_ERROR_CODE_NO_ERROR;
constexpr char PAYLOAD[] = R"({"schemaVersion":1,"treeInfo":null})";
constexpr int32_t TEST_CONTEXT_ID = 1;
constexpr uintptr_t OUTPUT_POINTER_SENTINEL = 1;
constexpr uint64_t UNCHANGED_REQUEST_ID = 99;
constexpr auto INVALID_TREE_TYPE = static_cast<OH_ArkUI_NativeModule_UIAgentTreeType>(2);
constexpr auto INVALID_JSON_FORMAT = static_cast<OH_ArkUI_NativeModule_UIJsonFormat>(2);
int callbacks = 0;
ArkUI_ErrorCode callbackCode = ARKUI_ERROR_CODE_NO_ERROR;
ArkUI_ContextHandle callbackContext = nullptr;
void* callbackUserData = nullptr;
uint64_t callbackId = 0;
std::string callbackPayload;
int releases = 0;

void Callback(ArkUI_ContextHandle context, uint64_t id, ArkUI_ErrorCode code,
    OH_ArkUI_NativeModule_UIJsonWrapper* result, void* userData)
{
    ++callbacks;
    callbackCode = code;
    callbackContext = context;
    callbackId = id;
    callbackUserData = userData;
    if (result) {
        callbackPayload.assign(
            OH_ArkUI_NativeModule_UIJsonWrapperGetData(result), OH_ArkUI_NativeModule_UIJsonWrapperGetSize(result));
    } else {
        callbackPayload.clear();
    }
    EXPECT_EQ(result == nullptr, code != ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
}

// Only the shared-library boundary is substituted. Requests, setters, validation,
// JSON wrappers and callback delivery use production code.
int32_t Collect(int32_t, const ArkUITreeOptions* options, char** data, uint32_t* size)
{
    captured = *options;
    *data = nullptr;
    *size = 0;
    if (collectionResult) {
        return collectionResult;
    }
    *size = std::strlen(PAYLOAD);
    constexpr size_t capacity = sizeof(PAYLOAD);
    *data = static_cast<char*>(std::malloc(capacity));
    if (!*data) {
        return ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
    }
    if (memcpy_s(*data, capacity, PAYLOAD, capacity) != EOK) {
        std::free(*data);
        *data = nullptr;
        *size = 0;
        return ARKUI_ERROR_CODE_INTERNAL_ERROR;
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}
int32_t Submit(int32_t, const ArkUITreeOptions* options, const ArkUITreeCompletion* completion, uint64_t* id)
{
    captured = *options;
    accepted = *completion;
    *id = ++acceptedId;
    return ARKUI_ERROR_CODE_NO_ERROR;
}
void Release(char* data)
{
    ++releases;
    std::free(data);
}
const ArkUITreeAPI api { Collect, Submit, Release };
using Setter = ArkUI_ErrorCode (*)(OH_ArkUI_NativeModule_UIAgentTreeRequest*, bool);
const Setter setters[] = {
    OH_ArkUI_NativeModule_UIAgentTreeRequestSetFilterPureLayoutNodes,
    OH_ArkUI_NativeModule_UIAgentTreeRequestSetInteractionInfo,
    OH_ArkUI_NativeModule_UIAgentTreeRequestSetAccessibilityInfo,
    OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectVisualProperties,
    OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectFunctionalProperties,
};
constexpr std::array TREE_TYPES { OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL,
    OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_VISIBLE };
constexpr std::array JSON_FORMATS { OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, OH_ARKUI_NATIVE_MODULE_UI_JSON_PRETTY };
constexpr unsigned OPTION_MASK_COUNT = 1U << std::size(setters);
constexpr size_t CONFIGURATION_COUNT = TREE_TYPES.size() * JSON_FORMATS.size() * OPTION_MASK_COUNT;
enum class TreeOption { FILTER_LAYOUT, INTERACTION, ACCESSIBILITY, VISUAL, FUNCTIONAL };

bool IsEnabled(unsigned mask, TreeOption option)
{
    return (mask & (1U << static_cast<unsigned>(option))) != 0;
}

void CheckCapturedOptions(const ArkUITreeOptions& options, unsigned mask)
{
    EXPECT_EQ(options.filterPureLayout, IsEnabled(mask, TreeOption::FILTER_LAYOUT));
    EXPECT_EQ(options.interaction, IsEnabled(mask, TreeOption::INTERACTION));
    EXPECT_EQ(options.accessibility, IsEnabled(mask, TreeOption::ACCESSIBILITY));
    EXPECT_EQ(options.visual, IsEnabled(mask, TreeOption::VISUAL));
    EXPECT_EQ(options.functional, IsEnabled(mask, TreeOption::FUNCTIONAL));
}

void ConfigureRequest(OH_ArkUI_NativeModule_UIAgentTreeRequest* request, unsigned mask)
{
    for (unsigned bit = 0; bit < std::size(setters); ++bit) {
        // Prove true -> false resets independently of creation defaults.
        EXPECT_EQ(setters[bit](request, true), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(setters[bit](request, (mask & (1U << bit)) != 0), ARKUI_ERROR_CODE_NO_ERROR);
    }
}

void CheckConfiguration(ArkUI_Context& context, size_t index)
{
    auto tree = TREE_TYPES[index / (JSON_FORMATS.size() * OPTION_MASK_COUNT)];
    auto format = JSON_FORMATS[(index / OPTION_MASK_COUNT) % JSON_FORMATS.size()];
    auto mask = static_cast<unsigned>(index % OPTION_MASK_COUNT);
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(tree, &request), ARKUI_ERROR_CODE_NO_ERROR);
    ConfigureRequest(request, mask);
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJson(&context, request, format, &result), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(result), PAYLOAD);
    EXPECT_EQ(captured.treeType, tree);
    EXPECT_EQ(captured.format, format);
    CheckCapturedOptions(captured, mask);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
    uint64_t id = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJsonAsync(&context, request, format, Callback, &context, &id),
        ARKUI_ERROR_CODE_NO_ERROR);
    auto snapshot = captured;
    OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(request);
    accepted.deliver(accepted, id, ARKUI_ERROR_CODE_NO_ERROR, PAYLOAD, std::strlen(PAYLOAD));
    EXPECT_EQ(callbackId, id);
    EXPECT_EQ(callbackContext, &context);
    EXPECT_EQ(callbackUserData, &context);
    EXPECT_EQ(callbackPayload, PAYLOAD);
    CheckCapturedOptions(snapshot, mask);
}

} // namespace
namespace OHOS::Ace::NodeModel {
const ArkUITreeAPI* GetTreeAPI()
{
    return &api;
}
const ArkUIPageTextAPI* GetPageTextAPI()
{
    return nullptr;
}
} // namespace OHOS::Ace::NodeModel

class UITreeCollectionTest : public testing::Test {
protected:
    void SetUp() override
    {
        captured = {};
        accepted = {};
        collectionResult = ARKUI_ERROR_CODE_NO_ERROR;
        callbacks = 0;
        releases = 0;
        callbackPayload.clear();
    }
};

HWTEST_F(UITreeCollectionTest, ParameterValidationAndOutputCleanup, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request = reinterpret_cast<decltype(request)>(OUTPUT_POINTER_SENTINEL);
    EXPECT_EQ(
        OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(INVALID_TREE_TYPE, &request), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(request, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL, nullptr),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    for (auto setter : setters) {
        EXPECT_EQ(setter(nullptr, true), ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL, &request),
        ARKUI_ERROR_CODE_NO_ERROR);
    auto* result = reinterpret_cast<OH_ArkUI_NativeModule_UIJsonWrapper*>(OUTPUT_POINTER_SENTINEL);
    EXPECT_EQ(
        OH_ArkUI_NativeModule_UIAgentGetTreeJson(nullptr, request, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, &result),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(result, nullptr);
    ArkUI_Context context { TEST_CONTEXT_ID };
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJson(&context, request, INVALID_JSON_FORMAT, &result),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    uint64_t id = UNCHANGED_REQUEST_ID;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJsonAsync(
                  &context, request, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, nullptr, nullptr, &id),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(id, UNCHANGED_REQUEST_ID);
    EXPECT_EQ(callbacks, 0);
    OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(request);
    OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(nullptr);
}

HWTEST_F(UITreeCollectionTest, All128ConfigurationsSyncAndAsync, TestSize.Level1)
{
    ArkUI_Context context { TEST_CONTEXT_ID };
    for (size_t index = 0; index < CONFIGURATION_COUNT; ++index) {
        SCOPED_TRACE(index);
        CheckConfiguration(context, index);
        ASSERT_FALSE(HasFatalFailure());
    }
    EXPECT_EQ(callbacks, CONFIGURATION_COUNT);
    EXPECT_EQ(releases, CONFIGURATION_COUNT);
}

HWTEST_F(UITreeCollectionTest, DefaultsAndFailuresDoNotReturnPartialWrapper, TestSize.Level1)
{
    ArkUI_Context context { TEST_CONTEXT_ID };
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request = nullptr;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL, &request),
        ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_UIJsonWrapper* result = nullptr;
    ASSERT_EQ(
        OH_ArkUI_NativeModule_UIAgentGetTreeJson(&context, request, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, &result),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(captured.filterPureLayout || captured.interaction || captured.accessibility || captured.visual ||
                 captured.functional);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(result);
    for (auto code :
        { ARKUI_ERROR_CODE_RESULT_TOO_LARGE, ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED, ARKUI_ERROR_CODE_INTERNAL_ERROR,
            ARKUI_ERROR_CODE_UI_CONTEXT_INVALID, ARKUI_ERROR_CODE_NODE_ON_INVALID_THREAD }) {
        collectionResult = code;
        result = reinterpret_cast<decltype(result)>(OUTPUT_POINTER_SENTINEL);
        EXPECT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJson(
                      &context, request, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, &result),
            code);
        EXPECT_EQ(result, nullptr);
        uint64_t id;
        ASSERT_EQ(OH_ArkUI_NativeModule_UIAgentGetTreeJsonAsync(
                      &context, request, OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT, Callback, nullptr, &id),
            ARKUI_ERROR_CODE_NO_ERROR);
        accepted.deliver(accepted, id, code, nullptr, 0);
        EXPECT_EQ(callbackCode, code);
        EXPECT_TRUE(callbackPayload.empty());
    }
    OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(request);
}

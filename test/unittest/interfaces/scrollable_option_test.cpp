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

#include "gtest/gtest.h"
#define private public
#define protected public
#include "native_interface.h"
#include "native_node.h"
#include "native_type.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include <thread>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components/common/layout/constants.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "frameworks/base/error/error_code.h"
#include "interfaces/native/native_error_message_wrapper.h"
#include "core/common/ace_application_info.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/node/node_model.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace;
using namespace OHOS::Ace::NG;

ArkUI_Int32 MockListItemExpand(ArkUINodeHandle node, ArkUI_Int32 direction)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    if (frameNode == nullptr) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_PARAM_INVALID, "frame node is null");
        return OHOS::Ace::ERROR_CODE_PARAM_INVALID;
    }
    if (frameNode->GetTag() != V2::LIST_ITEM_ETS_TAG) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_PARAM_ERROR, "node type is not ListItem");
        return OHOS::Ace::ERROR_CODE_PARAM_ERROR;
    }
    if (static_cast<int32_t>(ListItemSwipeActionDirection::START) > direction ||
        static_cast<int32_t>(ListItemSwipeActionDirection::END) < direction) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_PARAM_INVALID, "direction is invalid");
        return OHOS::Ace::ERROR_CODE_PARAM_INVALID;
    }
    if (!frameNode->IsOnMainTree()) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_NATIVE_IMPL_NODE_NOT_ON_MAIN_TREE, "node is not on main tree");
        return OHOS::Ace::ERROR_CODE_NATIVE_IMPL_NODE_NOT_ON_MAIN_TREE;
    }
    return OHOS::Ace::ERROR_CODE_NO_ERROR;
}

ArkUI_Int32 MockListItemCollapse(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    if (frameNode == nullptr) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_PARAM_INVALID, "frame node is null");
        return OHOS::Ace::ERROR_CODE_PARAM_INVALID;
    }
    if (frameNode->GetTag() != V2::LIST_ITEM_ETS_TAG) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_PARAM_ERROR, "node type is not ListItem");
        return OHOS::Ace::ERROR_CODE_PARAM_ERROR;
    }
    if (!frameNode->IsOnMainTree()) {
        OHOS::Ace::SetErrorCodeAndMessageByModifier(
            OHOS::Ace::ERROR_CODE_NATIVE_IMPL_NODE_NOT_ON_MAIN_TREE, "node is not on main tree");
        return OHOS::Ace::ERROR_CODE_NATIVE_IMPL_NODE_NOT_ON_MAIN_TREE;
    }
    return OHOS::Ace::ERROR_CODE_NO_ERROR;
}

void MockSetListCloseAllSwipeActions(ArkUINodeHandle node, void* userData, void (*onFinish)(void* userData))
{
    // no-op – the test only verifies the C-API wrapper path.
}

static ArkUIListItemModifier g_mockListItemModifier = {
    .expand = MockListItemExpand,
    .collapse = MockListItemCollapse,
};

static ArkUIListModifier g_mockListModifier = {
    .setListCloseAllSwipeActions = MockSetListCloseAllSwipeActions,
};

// Wrapper functions that return the mock modifier structs.
// getListItemModifier / getListModifier in ArkUINodeModifiers are
// function pointers:  const ArkUIXxxModifier* (*)()
const ArkUIListItemModifier* GetMockListItemModifier() { return &g_mockListItemModifier; }
const ArkUIListModifier* GetMockListModifier() { return &g_mockListModifier; }

// Mock BasicAPI for thread-safety checks inside RuntimeCheckManager.
// SetRuntimeCheckMode calls IsCurrentThreadSafe() which dereferences
// getBasicAPI()->isCurrentThreadSafe.  Without a valid mock the call
// crashes on a NULL pointer (Signal 11 at address 0x4).
static const ArkUIBasicAPI* g_mockBasicApi = nullptr;
static ArkUIBasicAPI g_basicApiCopy;

class ScrollableOptionErrorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        NG::MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->pipelineContext_ = NG::MockPipelineContext::GetCurrent();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        PipelineBase::GetCurrentContext()->SetThemeManager(themeManager);
    }
    static void TearDownTestCase()
    {
        NG::MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    void SetUp() override
    {
        // Ensure the full node-API impl is initialised.
        ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
        fullImpl_ = OHOS::Ace::NodeModel::GetFullImpl();
        ASSERT_NE(fullImpl_, nullptr);

        // Patch getBasicAPI with a mock that provides valid
        // isCurrentThreadSafe / isDebugForParallel so that
        // SetRuntimeCheckMode's internal IsCurrentThreadSafe()
        // check does not crash on a NULL pointer dereference.
        const auto* basicApi = fullImpl_->getBasicAPI();
        ASSERT_NE(basicApi, nullptr);
        g_basicApiCopy = *basicApi;
        g_basicApiCopy.isCurrentThreadSafe = []() -> ArkUI_Bool { return 1; };
        g_basicApiCopy.isDebugForParallel = []() -> ArkUI_Bool { return 0; };
        g_basicApiCopy.isDebugForParallelSet = []() -> ArkUI_Bool { return 1; };
        previousBasicApiGetter_ = fullImpl_->getBasicAPI;
        g_mockBasicApi = &g_basicApiCopy;
        fullImpl_->getBasicAPI = []() -> const ArkUIBasicAPI* { return g_mockBasicApi; };

        // Now safe to reset check mode to DISABLED (IsCurrentThreadSafe
        // will use the mock above and return true = safe).
        NodeModel::ConfigManager::SetRuntimeCheckMode(
            static_cast<int32_t>(OHOS::Ace::NodeModel::CheckType::UI_THREAD),
            static_cast<int32_t>(OHOS::Ace::NodeModel::CheckMode::DISABLED));

        // Patch the ListItem / List modifier function pointers with the
        // test mocks above so that Expand / Collapse / CloseAllSwipeActions
        // can be exercised even when the on-device library lacks them.
        auto* modifiers = const_cast<ArkUINodeModifiers*>(fullImpl_->getNodeModifiers());
        if (modifiers != nullptr) {
            originalListItemModifier_ = modifiers->getListItemModifier;
            originalListModifier_ = modifiers->getListModifier;
            modifiers->getListItemModifier = GetMockListItemModifier;
            modifiers->getListModifier = GetMockListModifier;
        }
    }
    void TearDown() override
    {
        // Restore the check mode to DISABLED first (still using the mock
        // basic API so IsCurrentThreadSafe is safe).
        NodeModel::ConfigManager::SetRuntimeCheckMode(
            static_cast<int32_t>(OHOS::Ace::NodeModel::CheckType::UI_THREAD),
            static_cast<int32_t>(OHOS::Ace::NodeModel::CheckMode::DISABLED));

        // Restore original modifier pointers so other test suites in the
        // same binary are not affected.
        if (fullImpl_ != nullptr) {
            auto* modifiers = const_cast<ArkUINodeModifiers*>(fullImpl_->getNodeModifiers());
            if (modifiers != nullptr) {
                modifiers->getListItemModifier = originalListItemModifier_;
                modifiers->getListModifier = originalListModifier_;
            }
            // Restore the original getBasicAPI getter.
            if (previousBasicApiGetter_ != nullptr) {
                fullImpl_->getBasicAPI = previousBasicApiGetter_;
                g_mockBasicApi = nullptr;
            }
        }
    }

private:
    ArkUIFullNodeAPI* fullImpl_ = nullptr;
    const ArkUIBasicAPI* (*previousBasicApiGetter_)() = nullptr;
    const ArkUIListItemModifier* (*originalListItemModifier_)() = nullptr;
    const ArkUIListModifier* (*originalListModifier_)() = nullptr;
};

/**
 * @tc.name: GridSetIrregularIndexes_NullOption_001
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_SetIrregularIndexes with null option.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridSetIrregularIndexes_NullOption_001, TestSize.Level1)
{
    uint32_t indexes[] = { 1, 2, 3 };
    auto result = OH_ArkUI_GridLayoutOptions_SetIrregularIndexes(nullptr, indexes, 3);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_SetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: option is null"), std::string::npos);
}

/**
 * @tc.name: GridSetIrregularIndexes_NullIndexes_002
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_SetIrregularIndexes with null irregularIndexes.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridSetIrregularIndexes_NullIndexes_002, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);
    auto result = OH_ArkUI_GridLayoutOptions_SetIrregularIndexes(option, nullptr, 3);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_SetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: irregularIndexes is null"), std::string::npos);
    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: GridSetIrregularIndexes_NegativeSize_003
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_SetIrregularIndexes with negative size.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridSetIrregularIndexes_NegativeSize_003, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);
    uint32_t indexes[] = { 1, 2, 3 };
    auto result = OH_ArkUI_GridLayoutOptions_SetIrregularIndexes(option, indexes, -1);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_SetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: size is less than 0"), std::string::npos);
    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: GridGetIrregularIndexes_NullOption_001
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_GetIrregularIndexes with null option.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridGetIrregularIndexes_NullOption_001, TestSize.Level1)
{
    uint32_t outIndexes[8] = { 0 };
    int32_t outSize = 8;
    auto result = OH_ArkUI_GridLayoutOptions_GetIrregularIndexes(nullptr, outIndexes, &outSize);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_GetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: option is null"), std::string::npos);
}

/**
 * @tc.name: GridGetIrregularIndexes_NullIndexes_002
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_GetIrregularIndexes with null irregularIndexes.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridGetIrregularIndexes_NullIndexes_002, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);
    int32_t outSize = 8;
    auto result = OH_ArkUI_GridLayoutOptions_GetIrregularIndexes(option, nullptr, &outSize);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_GetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: irregularIndexes is null"), std::string::npos);
    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: GridGetIrregularIndexes_NullSize_003
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_GetIrregularIndexes with null size.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridGetIrregularIndexes_NullSize_003, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);
    uint32_t outIndexes[8] = { 0 };
    auto result = OH_ArkUI_GridLayoutOptions_GetIrregularIndexes(option, outIndexes, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_GetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: size is null"), std::string::npos);
    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: GridGetIrregularIndexes_BufferTooSmall_004
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_GetIrregularIndexes with buffer size too small.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridGetIrregularIndexes_BufferTooSmall_004, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);
    uint32_t indexes[] = { 1, 2, 3 };
    OH_ArkUI_GridLayoutOptions_SetIrregularIndexes(option, indexes, 3);
    uint32_t outIndexes[2] = { 0 };
    int32_t outSize = 2;
    auto result = OH_ArkUI_GridLayoutOptions_GetIrregularIndexes(option, outIndexes, &outSize);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_BUFFER_SIZE_ERROR);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_GridLayoutOptions_GetIrregularIndexes"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: the provided buffer size is insufficient"), std::string::npos);
    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: GridSetAndGetIrregularIndexes_Success_001
 * @tc.desc: Test OH_ArkUI_GridLayoutOptions_SetIrregularIndexes and GetIrregularIndexes success path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, GridSetAndGetIrregularIndexes_Success_001, TestSize.Level1)
{
    ArkUI_GridLayoutOptions* option = OH_ArkUI_GridLayoutOptions_Create();
    ASSERT_NE(option, nullptr);

    uint32_t indexes[] = { 1, 3, 5 };
    auto setResult = OH_ArkUI_GridLayoutOptions_SetIrregularIndexes(option, indexes, 3);
    EXPECT_EQ(setResult, ARKUI_ERROR_CODE_NO_ERROR);

    uint32_t outIndexes[8] = { 0 };
    int32_t outSize = 8;
    auto getResult = OH_ArkUI_GridLayoutOptions_GetIrregularIndexes(option, outIndexes, &outSize);
    EXPECT_EQ(getResult, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outSize, 3);

    OH_ArkUI_GridLayoutOptions_Dispose(option);
}

/**
 * @tc.name: ListSetDefaultMainSize_NullOption_001
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize with null option.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSetDefaultMainSize_NullOption_001, TestSize.Level1)
{
    auto result = OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize(nullptr, 100.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize"),
        std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: option is null"), std::string::npos);
}

/**
 * @tc.name: ListSetDefaultMainSize_NegativeSize_002
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize with negative value.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSetDefaultMainSize_NegativeSize_002, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize(option, -1.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize"),
        std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: defaultMainSize is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSetDefaultMainSize_Success_003
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize with valid value.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSetDefaultMainSize_Success_003, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);

    auto result = OH_ArkUI_ListChildrenMainSizeOption_SetDefaultMainSize(option, 100.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetDefaultMainSize(option), 100.0f);

    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSplice_NullOption_001
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with null option.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_NullOption_001, TestSize.Level1)
{
    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(nullptr, 0, 0, 1);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_Splice"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: option is null"), std::string::npos);
}

/**
 * @tc.name: ListSplice_NegativeIndex_002
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with negative index.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_NegativeIndex_002, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(option, -1, 0, 1);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_Splice"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: index is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSplice_NegativeDeleteCount_003
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with negative deleteCount.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_NegativeDeleteCount_003, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 5);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(option, 0, -1, 0);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_Splice"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: deleteCount is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSplice_NegativeAddCount_004
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with negative addCount.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_NegativeAddCount_004, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 5);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(option, 0, 0, -1);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_Splice"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: addCount is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSplice_IndexOutOfRange_005
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with index exceeding mainSize array.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_IndexOutOfRange_005, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 3);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(option, 10, 0, 1);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_Splice"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: index exceeds the size of ChildrenMainSize"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListSplice_Success_006
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_Splice with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListSplice_Success_006, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 5);
    OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 0, 10.0f);
    OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 1, 20.0f);
    OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 2, 30.0f);

    auto result = OH_ArkUI_ListChildrenMainSizeOption_Splice(option, 1, 1, 2);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);

    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetMainSize(option, 0), 10.0f);
    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetMainSize(option, 1), -1.0f);
    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetMainSize(option, 2), -1.0f);
    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetMainSize(option, 3), 30.0f);

    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListUpdateSize_NullOption_001
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_UpdateSize with null option.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListUpdateSize_NullOption_001, TestSize.Level1)
{
    auto result = OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(nullptr, 0, 50.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_UpdateSize"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: option is null"), std::string::npos);
}

/**
 * @tc.name: ListUpdateSize_NegativeIndex_002
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_UpdateSize with negative index.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListUpdateSize_NegativeIndex_002, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, -1, 50.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_UpdateSize"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: index is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListUpdateSize_NegativeMainSize_003
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_UpdateSize with negative mainSize.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListUpdateSize_NegativeMainSize_003, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 5);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 0, -1.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_UpdateSize"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: mainSize is less than 0"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListUpdateSize_IndexOutOfRange_004
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_UpdateSize with index exceeding mainSize array.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListUpdateSize_IndexOutOfRange_004, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 3);
    auto result = OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 10, 50.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListChildrenMainSizeOption_UpdateSize"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: index exceeds the size of ChildrenMainSize"), std::string::npos);
    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: ListUpdateSize_Success_005
 * @tc.desc: Test OH_ArkUI_ListChildrenMainSizeOption_UpdateSize with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, ListUpdateSize_Success_005, TestSize.Level1)
{
    ArkUI_ListChildrenMainSize* option = OH_ArkUI_ListChildrenMainSizeOption_Create();
    ASSERT_NE(option, nullptr);
    OH_ArkUI_ListChildrenMainSizeOption_Resize(option, 3);

    auto result = OH_ArkUI_ListChildrenMainSizeOption_UpdateSize(option, 1, 50.0f);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(OH_ArkUI_ListChildrenMainSizeOption_GetMainSize(option, 1), 50.0f);

    OH_ArkUI_ListChildrenMainSizeOption_Dispose(option);
}

/**
 * @tc.name: Expand_NullNode_001
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Expand with null node.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Expand_NullNode_001, TestSize.Level1)
{
    auto result = OH_ArkUI_ListItemSwipeAction_Expand(nullptr, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Expand"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is null"), std::string::npos);
}

/**
 * @tc.name: Expand_WrongNodeType_002
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Expand with non-ListItem node type.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Expand_WrongNodeType_002, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto columnNode = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ASSERT_NE(columnNode, nullptr);

    auto result = OH_ArkUI_ListItemSwipeAction_Expand(columnNode, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_ERROR);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Expand"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node type is not ListItem"), std::string::npos);

    nodeAPI->disposeNode(columnNode);
}

/**
 * @tc.name: Expand_InvalidDirection_003
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Expand with invalid direction.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Expand_InvalidDirection_003, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listItemNode = nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    ASSERT_NE(listItemNode, nullptr);

    auto result = OH_ArkUI_ListItemSwipeAction_Expand(
        listItemNode, static_cast<ArkUI_ListItemSwipeActionDirection>(99));
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Expand"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: direction is invalid"), std::string::npos);

    nodeAPI->disposeNode(listItemNode);
}

/**
 * @tc.name: Expand_NodeNotOnMainTree_004
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Expand with node not on main tree.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Expand_NodeNotOnMainTree_004, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listItemNode = nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    ASSERT_NE(listItemNode, nullptr);

    auto result = OH_ArkUI_ListItemSwipeAction_Expand(
        listItemNode, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NODE_NOT_ON_MAIN_TREE);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Expand"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is not on main tree"), std::string::npos);

    nodeAPI->disposeNode(listItemNode);
}

/**
 * @tc.name: Expand_Success_005
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Expand with valid ListItem on main tree.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Expand_Success_005, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listItemNode = nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    ASSERT_NE(listItemNode, nullptr);

    auto* frameNode = reinterpret_cast<OHOS::Ace::NG::FrameNode*>(listItemNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    frameNode->onMainTree_ = true;

    auto result = OH_ArkUI_ListItemSwipeAction_Expand(
        listItemNode, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);

    nodeAPI->disposeNode(listItemNode);
}

/**
 * @tc.name: Collapse_NullNode_001
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Collapse with null node.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Collapse_NullNode_001, TestSize.Level1)
{
    auto result = OH_ArkUI_ListItemSwipeAction_Collapse(nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Collapse"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is null"), std::string::npos);
}

/**
 * @tc.name: Collapse_WrongNodeType_002
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Collapse with non-ListItem node type.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Collapse_WrongNodeType_002, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto columnNode = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ASSERT_NE(columnNode, nullptr);

    auto result = OH_ArkUI_ListItemSwipeAction_Collapse(columnNode);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_ERROR);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Collapse"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node type is not ListItem"), std::string::npos);

    nodeAPI->disposeNode(columnNode);
}

/**
 * @tc.name: Collapse_NodeNotOnMainTree_003
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Collapse with node not on main tree.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Collapse_NodeNotOnMainTree_003, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listItemNode = nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    ASSERT_NE(listItemNode, nullptr);

    auto result = OH_ArkUI_ListItemSwipeAction_Collapse(listItemNode);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NODE_NOT_ON_MAIN_TREE);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Collapse"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is not on main tree"), std::string::npos);

    nodeAPI->disposeNode(listItemNode);
}

/**
 * @tc.name: Collapse_Success_004
 * @tc.desc: Test OH_ArkUI_ListItemSwipeAction_Collapse with valid ListItem on main tree.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, Collapse_Success_004, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listItemNode = nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    ASSERT_NE(listItemNode, nullptr);

    auto* frameNode = reinterpret_cast<OHOS::Ace::NG::FrameNode*>(listItemNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    frameNode->onMainTree_ = true;

    auto result = OH_ArkUI_ListItemSwipeAction_Collapse(listItemNode);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);

    nodeAPI->disposeNode(listItemNode);
}

/**
 * @tc.name: CloseAllSwipeActions_NullNode_001
 * @tc.desc: Test OH_ArkUI_List_CloseAllSwipeActions with null node.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, CloseAllSwipeActions_NullNode_001, TestSize.Level1)
{
    auto result = OH_ArkUI_List_CloseAllSwipeActions(nullptr, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_List_CloseAllSwipeActions"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is null"), std::string::npos);
}

/**
 * @tc.name: CloseAllSwipeActions_WrongType_002
 * @tc.desc: Test OH_ArkUI_List_CloseAllSwipeActions with non-List node type.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, CloseAllSwipeActions_WrongType_002, TestSize.Level1)
{
    ArkUI_Node fakeNode;
    fakeNode.type = ARKUI_NODE_COLUMN;
    fakeNode.uiNodeHandle = nullptr;
    fakeNode.magic = ARKUI_NODE_MAGIC_VALID;
    auto result = OH_ArkUI_List_CloseAllSwipeActions(&fakeNode, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(result)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_List_CloseAllSwipeActions"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node type is not List"), std::string::npos);
}

/**
 * @tc.name: CloseAllSwipeActions_Success_003
 * @tc.desc: Test OH_ArkUI_List_CloseAllSwipeActions with valid List node.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, CloseAllSwipeActions_Success_003, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    auto listNode = nodeAPI->createNode(ARKUI_NODE_LIST);
    ASSERT_NE(listNode, nullptr);

    auto result = OH_ArkUI_List_CloseAllSwipeActions(listNode, nullptr, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);

    nodeAPI->disposeNode(listNode);
}

/**
 * @tc.name: NonUiThreadCall_Expand_001
 * @tc.desc: OH_ArkUI_ListItemSwipeAction_Expand from a non-UI thread with
 *           UI_THREAD check DISABLED does not crash and keeps validating params.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, NonUiThreadCall_Expand_001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));

    int32_t result = ARKUI_ERROR_CODE_NO_ERROR;
    std::thread worker([&result]() {
        result = OH_ArkUI_ListItemSwipeAction_Expand(
            nullptr, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    });
    worker.join();

    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    // Restore the process-level RuntimeCheckManager to DISABLED to avoid
    // leaking global state into sibling test cases.
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));
}

/**
 * @tc.name: NonUiThreadCall_Collapse_002
 * @tc.desc: OH_ArkUI_ListItemSwipeAction_Collapse from a non-UI thread with
 *           UI_THREAD check DISABLED does not crash and keeps validating params.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, NonUiThreadCall_Collapse_002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));

    int32_t result = ARKUI_ERROR_CODE_NO_ERROR;
    std::thread worker([&result]() {
        result = OH_ArkUI_ListItemSwipeAction_Collapse(nullptr);
    });
    worker.join();

    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));
}

/**
 * @tc.name: NonUiThreadCall_CloseAll_003
 * @tc.desc: OH_ArkUI_List_CloseAllSwipeActions from a non-UI thread with
 *           UI_THREAD check DISABLED does not crash and keeps validating params.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, NonUiThreadCall_CloseAll_003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));

    int32_t result = ARKUI_ERROR_CODE_NO_ERROR;
    std::thread worker([&result]() {
        result = OH_ArkUI_List_CloseAllSwipeActions(nullptr, nullptr, nullptr);
    });
    worker.join();

    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::DISABLED)));
}

/**
 * @tc.name: UiThreadCall_Expand_NoMisreport_004
 * @tc.desc: OH_ArkUI_ListItemSwipeAction_Expand on the UI thread is not
 *           affected by the new UI-thread guard (no false positive).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableOptionErrorTest, UiThreadCall_Expand_NoMisreport_004, TestSize.Level1)
{
    // Explicitly set the UI_THREAD check mode to LOG so the guard is active.
    // This avoids relying on state left by previous test cases and verifies
    // that a UI-thread call does not trigger a false-positive report.
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(NodeModel::CheckType::UI_THREAD),
        static_cast<int32_t>(NodeModel::CheckMode::LOG)));

    auto result = OH_ArkUI_ListItemSwipeAction_Expand(
        nullptr, ARKUI_LIST_ITEM_SWIPE_ACTION_DIRECTION_START);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_ListItemSwipeAction_Expand"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: node is null"), std::string::npos);
}

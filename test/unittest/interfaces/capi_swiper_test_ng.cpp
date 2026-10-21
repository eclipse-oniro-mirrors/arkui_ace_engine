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
#include <string>

#include "gtest/gtest.h"

#include "base/error/error_code.h"
#include "interfaces/native/native_node.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/node/config_manager.h"
#include "native_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class CapiSwiperOptionTestNg : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: SwiperFinishAnimationTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_FinishAnimation with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperFinishAnimationTestNullNode, TestSize.Level1)
{
    auto ret = OH_ArkUI_Swiper_FinishAnimation(nullptr);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperFinishAnimationTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_FinishAnimation with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperFinishAnimationTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    auto ret = OH_ArkUI_Swiper_FinishAnimation(&node);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperStartFakeDragTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_StartFakeDrag with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperStartFakeDragTestNullNode, TestSize.Level1)
{
    bool isSuccessful = false;
    auto ret = OH_ArkUI_Swiper_StartFakeDrag(nullptr, &isSuccessful);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperStartFakeDragTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_StartFakeDrag with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperStartFakeDragTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    bool isSuccessful = false;
    auto ret = OH_ArkUI_Swiper_StartFakeDrag(&node, &isSuccessful);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperFakeDragByTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_FakeDragBy with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperFakeDragByTestNullNode, TestSize.Level1)
{
    bool isConsumedOffset = false;
    auto ret = OH_ArkUI_Swiper_FakeDragBy(nullptr, 10.0f, &isConsumedOffset);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperFakeDragByTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_FakeDragBy with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperFakeDragByTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    bool isConsumedOffset = false;
    auto ret = OH_ArkUI_Swiper_FakeDragBy(&node, 10.0f, &isConsumedOffset);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperStopFakeDragTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_StopFakeDrag with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperStopFakeDragTestNullNode, TestSize.Level1)
{
    bool isSuccessful = false;
    auto ret = OH_ArkUI_Swiper_StopFakeDrag(nullptr, &isSuccessful);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperStopFakeDragTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_StopFakeDrag with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperStopFakeDragTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    bool isSuccessful = false;
    auto ret = OH_ArkUI_Swiper_StopFakeDrag(&node, &isSuccessful);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperIsFakeDraggingTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_IsFakeDragging with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperIsFakeDraggingTestNullNode, TestSize.Level1)
{
    bool isFakeDragging = false;
    auto ret = OH_ArkUI_Swiper_IsFakeDragging(nullptr, &isFakeDragging);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(ret)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_Swiper_IsFakeDragging"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: current node is null"), std::string::npos);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperIsFakeDraggingTestZeroNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_IsFakeDragging with zero node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperIsFakeDraggingTestZeroNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = 0;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    bool isFakeDragging = false;
    auto ret = OH_ArkUI_Swiper_IsFakeDragging(&node, &isFakeDragging);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find(std::string("errorCode: ") + std::to_string(ret)), std::string::npos);
    EXPECT_NE(errorMessageStr.find("functionName: OH_ArkUI_Swiper_IsFakeDragging"), std::string::npos);
    EXPECT_NE(errorMessageStr.find("errorMessage: Node type is not ARKUI_NODE_SWIPER"), std::string::npos);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperShowPreviousTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_ShowPrevious with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperShowPreviousTestNullNode, TestSize.Level1)
{
    auto ret = OH_ArkUI_Swiper_ShowPrevious(nullptr);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperShowPreviousTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_ShowPrevious with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperShowPreviousTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    auto ret = OH_ArkUI_Swiper_ShowPrevious(&node);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperShowNextTestNullNode
 * @tc.desc: Test OH_ArkUI_Swiper_ShowNext with null node
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperShowNextTestNullNode, TestSize.Level1)
{
    auto ret = OH_ArkUI_Swiper_ShowNext(nullptr);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: SwiperShowNextTestWrongNodeType
 * @tc.desc: Test OH_ArkUI_Swiper_ShowNext with wrong node type
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperShowNextTestWrongNodeType, TestSize.Level1)
{
    ArkUI_Node node;
    node.type = ARKUI_NODE_TEXT;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;
    auto ret = OH_ArkUI_Swiper_ShowNext(&node);
    EXPECT_EQ(ret, ERROR_CODE_PARAM_INVALID);
}

// ===== Swiper C API NodeHandle disposed-state (UAF) detection tests =====

namespace {
constexpr int32_t RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE = 1;

void RestoreRuntimeCheckMode()
{
    EXPECT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE,
        static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED)));
}

enum SwiperApiSlot : size_t {
    SLOT_SWIPER_FINISH_ANIMATION = 0,
    SLOT_SWIPER_START_FAKE_DRAG,
    SLOT_SWIPER_FAKE_DRAG_BY,
    SLOT_SWIPER_STOP_FAKE_DRAG,
    SLOT_SWIPER_IS_FAKE_DRAGGING,
    SLOT_SWIPER_SHOW_PREVIOUS,
    SLOT_SWIPER_SHOW_NEXT,
    SLOT_ARC_SWIPER_SHOW_NEXT,
    SLOT_ARC_SWIPER_SHOW_PREVIOUS,
    SLOT_ARC_SWIPER_FINISH_ANIMATION,
    SWIPER_SLOT_COUNT,
};

// Calls all 10 listed swiper/arcSwiper C APIs and records their return codes.
std::array<int32_t, SWIPER_SLOT_COUNT> CallAllSwiperApis(ArkUI_NodeHandle node)
{
    std::array<int32_t, SWIPER_SLOT_COUNT> results {};
    bool isSuccessful = false;
    bool isConsumedOffset = false;
    bool isFakeDragging = false;
    results[SLOT_SWIPER_FINISH_ANIMATION] = OH_ArkUI_Swiper_FinishAnimation(node);
    results[SLOT_SWIPER_START_FAKE_DRAG] = OH_ArkUI_Swiper_StartFakeDrag(node, &isSuccessful);
    results[SLOT_SWIPER_FAKE_DRAG_BY] = OH_ArkUI_Swiper_FakeDragBy(node, 10.0f, &isConsumedOffset);
    results[SLOT_SWIPER_STOP_FAKE_DRAG] = OH_ArkUI_Swiper_StopFakeDrag(node, &isSuccessful);
    results[SLOT_SWIPER_IS_FAKE_DRAGGING] = OH_ArkUI_Swiper_IsFakeDragging(node, &isFakeDragging);
    results[SLOT_SWIPER_SHOW_PREVIOUS] = OH_ArkUI_Swiper_ShowPrevious(node);
    results[SLOT_SWIPER_SHOW_NEXT] = OH_ArkUI_Swiper_ShowNext(node);
    results[SLOT_ARC_SWIPER_SHOW_NEXT] = OH_ArkUI_ArcSwiper_ShowNext(node);
    results[SLOT_ARC_SWIPER_SHOW_PREVIOUS] = OH_ArkUI_ArcSwiper_ShowPrevious(node);
    results[SLOT_ARC_SWIPER_FINISH_ANIMATION] = OH_ArkUI_ArcSwiper_FinishAnimation(node);
    return results;
}
} // namespace

/**
 * @tc.name: SwiperUafGuard001
 * @tc.desc: Controlled disposed samples hit the entry guard on all 10 listed
 *           swiper/arcSwiper APIs; LOG mode keeps the same return values as
 *           DISABLED and never pollutes the error message channel.
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperUafGuard001, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    ArkUI_Node node;
    node.type = ARKUI_NODE_SWIPER;
    node.cNode = true;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_INVALID;

    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED)));
    const auto disabledResults = CallAllSwiperApis(&node);

    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG)));
    const auto logResults = CallAllSwiperApis(&node);
    for (size_t i = 0; i < logResults.size(); i++) {
        EXPECT_EQ(logResults[i], disabledResults[i]) << "slot " << i;
    }
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    if (errorMessage != nullptr) {
        EXPECT_EQ(std::string(errorMessage).find("has been disposed"), std::string::npos);
    }
    RestoreRuntimeCheckMode();
}

/**
 * @tc.name: SwiperUafGuard002
 * @tc.desc: Valid magic never triggers the guard: LOG/DISABLED/CRASH modes keep
 *           identical return values on all 10 listed APIs for a valid handle.
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperUafGuard002, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    ArkUI_Node node;
    node.type = ARKUI_NODE_SWIPER;
    node.cNode = true;
    node.uiNodeHandle = nullptr;
    node.magic = ARKUI_NODE_MAGIC_VALID;

    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG)));
    const auto logResults = CallAllSwiperApis(&node);

    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED)));
    const auto disabledResults = CallAllSwiperApis(&node);

    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_CRASH)));
    const auto crashResults = CallAllSwiperApis(&node);

    for (size_t i = 0; i < logResults.size(); i++) {
        EXPECT_EQ(disabledResults[i], logResults[i]) << "slot " << i;
        EXPECT_EQ(crashResults[i], logResults[i]) << "slot " << i;
    }
    RestoreRuntimeCheckMode();
}

/**
 * @tc.name: SwiperUafGuard003
 * @tc.desc: Null inputs keep the existing parameter-error contracts on all 10
 *           listed swiper/arcSwiper APIs; the entry guard is null-safe and
 *           stays silent.
 * @tc.type: FUNC
 */
HWTEST_F(CapiSwiperOptionTestNg, SwiperUafGuard003, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    ASSERT_TRUE(NodeModel::ConfigManager::SetRuntimeCheckMode(
        RUNTIME_CHECK_TYPE_NODE_DISPOSED_VALUE, static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG)));
    const auto nullResults = CallAllSwiperApis(nullptr);
    for (size_t i = 0; i < nullResults.size(); i++) {
        EXPECT_EQ(nullResults[i], ERROR_CODE_PARAM_INVALID) << "slot " << i;
    }
    RestoreRuntimeCheckMode();
}
}

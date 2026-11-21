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

#include <csignal>
#include <functional>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "gtest/gtest.h"
#include "error_code.h"
#include "interfaces/native/node/config_manager.h"
#include "native_interface.h"
#include "node_extened.h"
#include "node_model.h"
#include "test/mock/frameworks/base/log/mock_log_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::NodeModel;

namespace {
constexpr int32_t FONT_STYLE_CHECK_TYPE_NODE_DISPOSED =
    static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_TYPE_NODE_DISPOSED);

// Switches the process-wide disposed-node check to the given mode; the fixture
// TearDown restores DISABLED so later tests in this binary are unaffected.
void EnableFontStyleCheckMode(int32_t mode)
{
    ASSERT_TRUE(ConfigManager::SetRuntimeCheckMode(FONT_STYLE_CHECK_TYPE_NODE_DISPOSED, mode));
}

void RestoreFontStyleCheckMode()
{
    EXPECT_TRUE(ConfigManager::SetRuntimeCheckMode(FONT_STYLE_CHECK_TYPE_NODE_DISPOSED,
        static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED)));
}

// Asserts one captured diagnostic log carries the three stable fields: the check name
// NODE_DISPOSED, the given API name and the fixed reason text.
void AssertFontStyleDiagnosticLog(const DiagnosticLog* diag, const char* apiName)
{
    ASSERT_NE(diag, nullptr);
    ASSERT_NE(diag->checkName, nullptr);
    ASSERT_NE(diag->apiName, nullptr);
    ASSERT_NE(diag->reason, nullptr);
    EXPECT_NE(std::string(diag->checkName).find("NODE_DISPOSED"), std::string::npos);
    EXPECT_NE(std::string(diag->apiName).find(apiName), std::string::npos);
    EXPECT_NE(std::string(diag->reason).find("Node has been disposed"), std::string::npos);
}

// Manually forks to verify CRASH mode aborts; EXPECT_DEATH relies on GoogleTest's
// CapturedStream which creates temp files in /tmp, a directory that may not exist
// on device environments (e.g. rk3568), causing the test process itself to abort.
void ExpectAbort(const std::function<void()>& fn)
{
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid < 0) {
        ADD_FAILURE() << "fork() failed";
        return;
    }
    if (pid == 0) {
        fn();
        _exit(0); // Should never reach here if CRASH mode works correctly
    }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) {
        ADD_FAILURE() << "waitpid() failed";
        return;
    }
    EXPECT_TRUE(WIFSIGNALED(status)) << "expected signal termination, got exit code: " << WEXITSTATUS(status);
    EXPECT_EQ(WTERMSIG(status), SIGABRT) << "expected SIGABRT, got signal: " << WTERMSIG(status);
}
} // namespace

class NodeUtilsErrorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        InitialFullImpl();
    }
    static void TearDownTestCase() {}
    void TearDown() override
    {
        RestoreFontStyleCheckMode();
    }
};

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetLayoutSize_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetLayoutSize with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetLayoutSize_NullNode, TestSize.Level1)
{
    ArkUI_IntSize size;
    int32_t result = OH_ArkUI_NodeUtils_GetLayoutSize(nullptr, &size);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetLayoutPosition_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetLayoutPosition with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetLayoutPosition_NullNode, TestSize.Level1)
{
    ArkUI_IntOffset localOffset;
    int32_t result = OH_ArkUI_NodeUtils_GetLayoutPosition(nullptr, &localOffset);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetLayoutPositionInWindow_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetLayoutPositionInWindow with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetLayoutPositionInWindow_NullNode, TestSize.Level1)
{
    ArkUI_IntOffset globalOffset;
    int32_t result = OH_ArkUI_NodeUtils_GetLayoutPositionInWindow(nullptr, &globalOffset);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetLayoutPositionInScreen_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetLayoutPositionInScreen with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetLayoutPositionInScreen_NullNode, TestSize.Level1)
{
    ArkUI_IntOffset screenOffset;
    int32_t result = OH_ArkUI_NodeUtils_GetLayoutPositionInScreen(nullptr, &screenOffset);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetPositionWithTranslateInWindow_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetPositionWithTranslateInWindow with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetPositionWithTranslateInWindow_NullNode, TestSize.Level1)
{
    ArkUI_IntOffset translateOffset;
    int32_t result = OH_ArkUI_NodeUtils_GetPositionWithTranslateInWindow(nullptr, &translateOffset);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetPositionWithTranslateInScreen_NullNode
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetPositionWithTranslateInScreen with null node parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetPositionWithTranslateInScreen_NullNode, TestSize.Level1)
{
    ArkUI_IntOffset translateOffset;
    int32_t result = OH_ArkUI_NodeUtils_GetPositionWithTranslateInScreen(nullptr, &translateOffset);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_NodeUtils_GetPositionWithTranslateInScreen_NullTranslateOffset
 * @tc.desc: Test OH_ArkUI_NodeUtils_GetPositionWithTranslateInScreen with null translateOffset parameter.
 *           Verifies SET_ERROR_MESSAGE triggers and returns ERROR_CODE_PARAM_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, NodeUtils_GetPositionWithTranslateInScreen_NullTranslateOffset, TestSize.Level1)
{
    ArkUI_Node node = {};
    int32_t result = OH_ArkUI_NodeUtils_GetPositionWithTranslateInScreen(&node, nullptr);
    EXPECT_EQ(result, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: NodeUtilsErrorTest_SystemFontStyleChange_DisposedCheck001
 * @tc.desc: A disposed-style node hits the NODE_DISPOSED guard on both system font style
 *           event entries in LOG mode: the diagnostic log carries NODE_DISPOSED, the API
 *           name and the reason; return values and side effects stay identical to
 *           DISABLED mode; the error message query channel stays free of disposed content.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, SystemFontStyleChange_DisposedCheck001, TestSize.Level1)
{
    // Stack-controlled sample: only the magic field drives the disposed-state check.
    ArkUI_Node disposedNode;
    disposedNode.uiNodeHandle = nullptr;
    disposedNode.magic = ARKUI_NODE_MAGIC_INVALID;

    // DISABLED baseline: no diagnosis, Register keeps its success return value.
    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED));
    ResetDiagnosticLog();
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(&disposedNode, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_NO_ERROR);
    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&disposedNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);

    // LOG mode: each entry diagnoses once and never short-circuits.
    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(&disposedNode, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetDiagnosticLogCount(), 1);
    AssertFontStyleDiagnosticLog(GetLastDiagnosticLog(), "OH_ArkUI_RegisterSystemFontStyleChangeEvent");

    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&disposedNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 2);
    AssertFontStyleDiagnosticLog(GetLastDiagnosticLog(), "OH_ArkUI_UnregisterSystemFontStyleChangeEvent");

    // The error message query channel does not carry disposed content from this check.
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    if (errorMessage != nullptr) {
        EXPECT_EQ(std::string(errorMessage).find("has been disposed"), std::string::npos);
    }
}

/**
 * @tc.name: NodeUtilsErrorTest_SystemFontStyleChange_DisposedCheck002
 * @tc.desc: Valid magic never triggers the guard: LOG/DISABLED/CRASH modes keep identical
 *           return values on both system font style event entries for a valid handle and
 *           CRASH does not terminate the process.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, SystemFontStyleChange_DisposedCheck002, TestSize.Level1)
{
    ArkUI_Node validNode;
    validNode.uiNodeHandle = nullptr;
    validNode.magic = ARKUI_NODE_MAGIC_VALID;

    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(&validNode, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_NO_ERROR);
    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&validNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);

    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED));
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(&validNode, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_NO_ERROR);
    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&validNode);

    // A valid handle does not trigger the check in CRASH mode either (no termination).
    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_CRASH));
    ResetDiagnosticLog();
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(&validNode, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_NO_ERROR);
    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&validNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);
}

/**
 * @tc.name: NodeUtilsErrorTest_SystemFontStyleChange_DisposedCheck003
 * @tc.desc: Null inputs keep the existing parameter contracts on both system font style
 *           event entries with the check enabled: Register returns 401, Unregister
 *           silently returns; the entry guard is null-safe and stays silent.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, SystemFontStyleChange_DisposedCheck003, TestSize.Level1)
{
    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();
    // Null node keeps Register's existing parameter-error contract (401).
    EXPECT_EQ(OH_ArkUI_RegisterSystemFontStyleChangeEvent(nullptr, nullptr, nullptr),
        OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    // Null node keeps Unregister's existing silent-return contract (void, no error code).
    OH_ArkUI_UnregisterSystemFontStyleChangeEvent(nullptr);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    if (errorMessage != nullptr) {
        EXPECT_EQ(std::string(errorMessage).find("has been disposed"), std::string::npos);
    }
}

/**
 * @tc.name: NodeUtilsErrorTest_SystemFontStyleChange_DisposedCheck004
 * @tc.desc: CRASH mode diagnoses and aborts on a disposed-style node before the callback
 *           registration path runs on either entry. Uses manual fork instead of
 *           EXPECT_DEATH to avoid GoogleTest's CapturedStream which requires /tmp.
 * @tc.type: FUNC
 */
HWTEST_F(NodeUtilsErrorTest, SystemFontStyleChange_DisposedCheck004, TestSize.Level1)
{
    ArkUI_Node disposedNode;
    disposedNode.uiNodeHandle = nullptr;
    disposedNode.magic = ARKUI_NODE_MAGIC_INVALID;

    EnableFontStyleCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_CRASH));
    ResetDiagnosticLog();
    // CRASH diagnoses and aborts before the registration path runs.
    ExpectAbort([&disposedNode]() {
        OH_ArkUI_RegisterSystemFontStyleChangeEvent(&disposedNode, nullptr, nullptr);
    });
    // CRASH diagnoses and aborts before the unregistration path runs.
    ExpectAbort([&disposedNode]() { OH_ArkUI_UnregisterSystemFontStyleChangeEvent(&disposedNode); });

    RestoreFontStyleCheckMode();
}

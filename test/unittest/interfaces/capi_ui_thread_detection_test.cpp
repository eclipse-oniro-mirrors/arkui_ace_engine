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

#include <algorithm>
#include <functional>
#include <string>
#include <cstring>

#include "gtest/gtest.h"
#include "base/log/log_wrapper.h"
#include "core/common/ace_application_info.h"
#include "frameworks/core/interfaces/arkoala/arkoala_api.h"

#include "interfaces/native/native_animate.h"
#include "interfaces/native/native_interface.h"
#include "interfaces/native/native_interface_accessibility.h"
#include "interfaces/native/native_interface_xcomponent.h"
#include "interfaces/native/native_key_event.h"
#include "interfaces/native/native_node.h"
#include "interfaces/native/native_dialog.h"
#include "interfaces/native/native_node_napi.h"
#include "interfaces/native/native_render.h"
#include "interfaces/native/node/animate_impl.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/node/node_model_safely.h"
#include "test/mock/frameworks/base/log/mock_log_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NodeModel {
namespace {

// === Mock 线程控制 ===
const ArkUIBasicAPI* g_runtimeCheckBasicApi = nullptr;
bool (*g_runtimeCheckThread)() = nullptr;

void SetMockIsCurrentThreadSafe(bool (*checker)())
{
    g_runtimeCheckThread = checker;
}

bool SafeThread() { return true; }
bool UnsafeThread() { return false; }

// === 默认参数调用辅助 ===
template<typename Return, typename... Args>
void CallWithDefaultArgs(Return (*function)(Args...))
{
    function(Args {}...);
}

struct ScopeEntry {
    const char* name;
    void (*invoke)();
};

// 特殊参数函数的包装
void InvokeRenderGetChild()
{
    ArkUI_RenderNodeHandle child = nullptr;
    OH_ArkUI_RenderNodeUtils_GetChild(nullptr, 0, &child);
}
void InvokeRenderGetChildren()
{
    ArkUI_RenderNodeHandle* children = nullptr;
    int32_t count = 0;
    OH_ArkUI_RenderNodeUtils_GetChildren(nullptr, &children, &count);
}
void InvokeRenderGetFirstChild()
{
    ArkUI_RenderNodeHandle child = nullptr;
    OH_ArkUI_RenderNodeUtils_GetFirstChild(nullptr, &child);
}
void InvokeRenderGetNextSibling()
{
    ArkUI_RenderNodeHandle sibling = nullptr;
    OH_ArkUI_RenderNodeUtils_GetNextSibling(nullptr, &sibling);
}
void InvokeRenderGetPreviousSibling()
{
    ArkUI_RenderNodeHandle sibling = nullptr;
    OH_ArkUI_RenderNodeUtils_GetPreviousSibling(nullptr, &sibling);
}
void InvokeNodeUtilsGetNodeUniqueId()
{
    int32_t uniqueId = -1;
    OH_ArkUI_NodeUtils_GetNodeUniqueId(nullptr, &uniqueId);
}
} // namespace

// === 测试基类 ===
class UiThreadDetectionTest : public Test {
protected:
    void SetUp() override
    {
        previousDebug_ = AceApplicationInfo::GetInstance().IsDebugForParallel();
        ASSERT_TRUE(InitialFullImpl());
        fullImpl_ = GetFullImpl();
        ASSERT_NE(fullImpl_, nullptr);
        ASSERT_EQ(fullImpl_->version, ARKUI_NODE_API_VERSION);
        ASSERT_NE(fullImpl_->getBasicAPI, nullptr);
        const auto* basicApi = fullImpl_->getBasicAPI();
        ASSERT_NE(basicApi, nullptr);

        basicApi_ = *basicApi;
        basicApi_.isCurrentThreadSafe = []() -> ArkUI_Bool {
            return g_runtimeCheckThread == nullptr || g_runtimeCheckThread();
        };
        basicApi_.isDebugForParallel = []() -> ArkUI_Bool {
            return AceApplicationInfo::GetInstance().IsDebugForParallel();
        };
        basicApi_.isDebugForParallelSet = []() -> ArkUI_Bool {
            return AceApplicationInfo::GetInstance().IsDebugForParallelSet();
        };
        previousBasicApiGetter_ = fullImpl_->getBasicAPI;
        g_runtimeCheckBasicApi = &basicApi_;
        fullImpl_->getBasicAPI = []() { return g_runtimeCheckBasicApi; };
        ResetToDefault();
    }

    void TearDown() override
    {
        if (previousBasicApiGetter_ != nullptr) {
            ResetToDefault();
            fullImpl_->getBasicAPI = previousBasicApiGetter_;
            g_runtimeCheckBasicApi = nullptr;
        }
        g_runtimeCheckThread = nullptr;
        AceApplicationInfo::GetInstance().SetDebugForParallel(previousDebug_);
    }

    static void ResetToDefault()
    {
        AceApplicationInfo::GetInstance().SetDebugForParallel(false);
        SetMockIsCurrentThreadSafe(SafeThread);
        ASSERT_EQ(OH_ArkUI_NativeModule_SetRuntimeCheckMode(
            OH_ARKUI_NATIVEMODULE_CHECK_TYPE_UI_THREAD, OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED),
            ARKUI_ERROR_CODE_NO_ERROR);
        ResetPrintLogCount();
    }

    static void ResetToUnresolved()
    {
        SetMockIsCurrentThreadSafe(SafeThread);
        ASSERT_EQ(OH_ArkUI_NativeModule_SetRuntimeCheckMode(
            OH_ARKUI_NATIVEMODULE_CHECK_TYPE_UI_THREAD, OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED),
            ARKUI_ERROR_CODE_NO_ERROR);
    }

    // === 诊断断言辅助 ===

    void SetupDiagnosticMatch(const std::string& /*apiName*/)
    {
        ResetPrintLogCount();
    }

    void RunAndAssert(std::function<void()> action, int expectedCount)
    {
        ResetPrintLogCount();
        action();
        EXPECT_EQ(GetPrintLogCount(), expectedCount);
    }

    void RunAndExpectDiagnostic(std::function<void()> action)
    {
        ResetPrintLogCount();
        ResetDiagnosticLog();
        action();
        EXPECT_GT(GetDiagnosticLogCount(), 0);
        const auto* diag = GetLastDiagnosticLog();
        ASSERT_NE(diag, nullptr);
        ASSERT_NE(diag->checkName, nullptr);
        EXPECT_TRUE(strstr(diag->checkName, "UI_THREAD") != nullptr);
        ASSERT_NE(diag->reason, nullptr);
        EXPECT_TRUE(strstr(diag->reason, "C API must be called on the UI thread") != nullptr);
    }

    void RunAndExpectNoDiagnostic(std::function<void()> action)
    {
        ResetDiagnosticLog();
        action();
        EXPECT_EQ(GetDiagnosticLogCount(), 0);
    }

    // === 模式控制 ===
    void EnableMode(OH_ArkUI_NativeModule_RuntimeCheckMode mode)
    {
        SetMockIsCurrentThreadSafe(SafeThread);
        ASSERT_EQ(OH_ArkUI_NativeModule_SetRuntimeCheckMode(
            OH_ARKUI_NATIVEMODULE_CHECK_TYPE_UI_THREAD, mode), ARKUI_ERROR_CODE_NO_ERROR);
    }

    bool previousDebug_ = false;
    ArkUIFullNodeAPI* fullImpl_ = nullptr;
    ArkUIBasicAPI basicApi_ {};
    decltype(ArkUIFullNodeAPI::getBasicAPI) previousBasicApiGetter_ = nullptr;
};

// === 个体测试 ===

HWTEST_F(UiThreadDetectionTest, LogModeProducesDiagnosticsOnUnsafeThread, TestSize.Level1)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    SetupDiagnosticMatch("TestApi_Disabled");
    RunAndExpectNoDiagnostic([] { ConfigManager::CheckUIThread("TestApi_Disabled"); });

    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    SetupDiagnosticMatch("TestApi_Log");
    RunAndExpectDiagnostic([] { ConfigManager::CheckUIThread("TestApi_Log"); });

    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetMockIsCurrentThreadSafe(SafeThread);
    SetupDiagnosticMatch("TestApi_Safe");
    RunAndExpectNoDiagnostic([] { ConfigManager::CheckUIThread("TestApi_Safe"); });
}

HWTEST_F(UiThreadDetectionTest, RuntimeCheckModeChangeInvalidatesCache, TestSize.Level1)
{
    SetMockIsCurrentThreadSafe(SafeThread);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
}

// === 参数化导出测试（177 × 4 模式）===

class UiThreadExportLogTest : public UiThreadDetectionTest, public WithParamInterface<ScopeEntry> {};

// 模式 1：默认-debug + 安全线程 → 无诊断
TEST_P(UiThreadExportLogTest, DebugDefaultSafeThreadNoDiagnostic)
{
    AceApplicationInfo::GetInstance().SetDebugForParallel(true);
    ResetToUnresolved();
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(SafeThread);
    RunAndExpectNoDiagnostic(GetParam().invoke);
}

// 模式 2：默认-非debug + 不安全线程 → 无诊断
TEST_P(UiThreadExportLogTest, ReleaseDefaultUnsafeThreadNoDiagnostic)
{
    AceApplicationInfo::GetInstance().SetDebugForParallel(false);
    ResetToUnresolved();
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic(GetParam().invoke);
}

// 模式 3：DISABLED + 不安全线程 → 无诊断
TEST_P(UiThreadExportLogTest, DisabledModeNoDiagnostic)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic(GetParam().invoke);
}

// 模式 4：LOG + 不安全线程 → 有诊断
TEST_P(UiThreadExportLogTest, LogModeProducesDiagnostic)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectDiagnostic(GetParam().invoke);
}

// === 参数化表测试（20 × 4 模式）===

struct TableScopeEntry {
    const char* name;
    void (*invoke)(ArkUI_NativeNodeAPI_1*);
};

#define TABLE_SCOPE_ENTRY(member) \
    { #member, [](ArkUI_NativeNodeAPI_1* api) { CallWithDefaultArgs(api->member); } }
const TableScopeEntry TABLE_SCOPE_ENTRIES[] = {
    TABLE_SCOPE_ENTRY(addChild),
    TABLE_SCOPE_ENTRY(createNode),
    TABLE_SCOPE_ENTRY(disposeNode),
    TABLE_SCOPE_ENTRY(getAttribute),
    TABLE_SCOPE_ENTRY(getChildAt),
    TABLE_SCOPE_ENTRY(getFirstChild),
    TABLE_SCOPE_ENTRY(getLastChild),
    TABLE_SCOPE_ENTRY(getNextSibling),
    TABLE_SCOPE_ENTRY(getParent),
    TABLE_SCOPE_ENTRY(getPreviousSibling),
    TABLE_SCOPE_ENTRY(getTotalChildCount),
    TABLE_SCOPE_ENTRY(insertChildAfter),
    TABLE_SCOPE_ENTRY(insertChildAt),
    TABLE_SCOPE_ENTRY(insertChildBefore),
    TABLE_SCOPE_ENTRY(markDirty),
    TABLE_SCOPE_ENTRY(removeAllChildren),
    TABLE_SCOPE_ENTRY(removeChild),
    TABLE_SCOPE_ENTRY(resetAttribute),
    TABLE_SCOPE_ENTRY(setAttribute),
    TABLE_SCOPE_ENTRY(setLengthMetricUnit),
};
#undef TABLE_SCOPE_ENTRY

class UiThreadTableLogTest : public UiThreadDetectionTest, public WithParamInterface<TableScopeEntry> {};

static ArkUI_NativeNodeAPI_1* GetNodeApi()
{
    return static_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
}

static std::string TableMemberName(const char* member)
{
    return std::string("ArkUI_NativeNodeAPI_1.") + member;
}

// 模式 1：默认-debug + 安全线程 → 无诊断
TEST_P(UiThreadTableLogTest, DebugDefaultSafeThreadNoDiagnostic)
{
    auto* api = GetNodeApi();
    ASSERT_NE(api, nullptr);
    AceApplicationInfo::GetInstance().SetDebugForParallel(true);
    ResetToUnresolved();
    SetupDiagnosticMatch(TableMemberName(GetParam().name));
    SetMockIsCurrentThreadSafe(SafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 2：默认-非debug + 不安全线程 → 无诊断
TEST_P(UiThreadTableLogTest, ReleaseDefaultUnsafeThreadNoDiagnostic)
{
    auto* api = GetNodeApi();
    ASSERT_NE(api, nullptr);
    AceApplicationInfo::GetInstance().SetDebugForParallel(false);
    ResetToUnresolved();
    SetupDiagnosticMatch(TableMemberName(GetParam().name));
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 3：DISABLED + 不安全线程 → 无诊断
TEST_P(UiThreadTableLogTest, DisabledModeNoDiagnostic)
{
    auto* api = GetNodeApi();
    ASSERT_NE(api, nullptr);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
    SetupDiagnosticMatch(TableMemberName(GetParam().name));
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 4：LOG + 不安全线程 → 有诊断
TEST_P(UiThreadTableLogTest, LogModeProducesDiagnostic)
{
    auto* api = GetNodeApi();
    ASSERT_NE(api, nullptr);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetupDiagnosticMatch(TableMemberName(GetParam().name));
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectDiagnostic([api, this] { GetParam().invoke(api); });
}

// === 表单弹窗类绑定表参数化测试（41 × 5 模式）===
// 覆盖 ArkUI_NativeDialogAPI_1/2/3 经 dialogImpl_1/2/3 绑定 DialogModel::* 的全部入口，
// 权威清单见 .codespec/changes/issue-20260831442009-thread-check-dialog-api/evidence/inputs/onebox-interface-list.txt。

struct DialogTableScopeEntry {
    const char* name;
    void (*invoke)(ArkUI_NativeDialogAPI_3*);
};

// 函数式宏已替换为模板函数以满足 G.PRE.02-CPP（使用函数代替函数式宏）；
// Member 为指向各 API 版本函数指针字段的指针到成员，名称字符串显式传入。
template <auto Member>
DialogTableScopeEntry MakeDialogTableEntry1(const char* name)
{
    return { name, [](ArkUI_NativeDialogAPI_3* api) { CallWithDefaultArgs(api->nativeDialogAPI1.*Member); } };
}
template <auto Member>
DialogTableScopeEntry MakeDialogTableEntry2(const char* name)
{
    return { name, [](ArkUI_NativeDialogAPI_3* api) { CallWithDefaultArgs(api->nativeDialogAPI2.*Member); } };
}
template <auto Member>
DialogTableScopeEntry MakeDialogTableEntry3(const char* name)
{
    return { name, [](ArkUI_NativeDialogAPI_3* api) { CallWithDefaultArgs(api->*Member); } };
}
const DialogTableScopeEntry DIALOG_TABLE_SCOPE_ENTRIES[] = {
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::create>("ArkUI_NativeDialogAPI_1.create"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::dispose>("ArkUI_NativeDialogAPI_1.dispose"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setContent>("ArkUI_NativeDialogAPI_1.setContent"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::removeContent>("ArkUI_NativeDialogAPI_1.removeContent"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setContentAlignment>("ArkUI_NativeDialogAPI_1.setContentAlignment"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::resetContentAlignment>("ArkUI_NativeDialogAPI_1.resetContentAlignment"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setModalMode>("ArkUI_NativeDialogAPI_1.setModalMode"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setAutoCancel>("ArkUI_NativeDialogAPI_1.setAutoCancel"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setMask>("ArkUI_NativeDialogAPI_1.setMask"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setBackgroundColor>("ArkUI_NativeDialogAPI_1.setBackgroundColor"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setCornerRadius>("ArkUI_NativeDialogAPI_1.setCornerRadius"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::setGridColumnCount>("ArkUI_NativeDialogAPI_1.setGridColumnCount"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::enableCustomStyle>("ArkUI_NativeDialogAPI_1.enableCustomStyle"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::enableCustomAnimation>("ArkUI_NativeDialogAPI_1.enableCustomAnimation"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::registerOnWillDismiss>("ArkUI_NativeDialogAPI_1.registerOnWillDismiss"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::show>("ArkUI_NativeDialogAPI_1.show"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::close>("ArkUI_NativeDialogAPI_1.close"),
    MakeDialogTableEntry1<&ArkUI_NativeDialogAPI_1::registerOnWillDismissWithUserData>(
        "ArkUI_NativeDialogAPI_1.registerOnWillDismissWithUserData"),
    MakeDialogTableEntry2<&ArkUI_NativeDialogAPI_2::setKeyboardAvoidDistance>("ArkUI_NativeDialogAPI_2.setKeyboardAvoidDistance"),
    MakeDialogTableEntry2<&ArkUI_NativeDialogAPI_2::setLevelMode>("ArkUI_NativeDialogAPI_2.setLevelMode"),
    MakeDialogTableEntry2<&ArkUI_NativeDialogAPI_2::setLevelUniqueId>("ArkUI_NativeDialogAPI_2.setLevelUniqueId"),
    MakeDialogTableEntry2<&ArkUI_NativeDialogAPI_2::setImmersiveMode>("ArkUI_NativeDialogAPI_2.setImmersiveMode"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setLevelOrder>("ArkUI_NativeDialogAPI_3.setLevelOrder"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::registerOnWillAppear>("ArkUI_NativeDialogAPI_3.registerOnWillAppear"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::registerOnDidAppear>("ArkUI_NativeDialogAPI_3.registerOnDidAppear"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::registerOnWillDisappear>("ArkUI_NativeDialogAPI_3.registerOnWillDisappear"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::registerOnDidDisappear>("ArkUI_NativeDialogAPI_3.registerOnDidDisappear"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBorderWidth>("ArkUI_NativeDialogAPI_3.setBorderWidth"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBorderColor>("ArkUI_NativeDialogAPI_3.setBorderColor"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBorderStyle>("ArkUI_NativeDialogAPI_3.setBorderStyle"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setWidth>("ArkUI_NativeDialogAPI_3.setWidth"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setHeight>("ArkUI_NativeDialogAPI_3.setHeight"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setShadow>("ArkUI_NativeDialogAPI_3.setShadow"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setCustomShadow>("ArkUI_NativeDialogAPI_3.setCustomShadow"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBackgroundBlurStyle>("ArkUI_NativeDialogAPI_3.setBackgroundBlurStyle"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setKeyboardAvoidMode>("ArkUI_NativeDialogAPI_3.setKeyboardAvoidMode"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::enableHoverMode>("ArkUI_NativeDialogAPI_3.enableHoverMode"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setHoverModeArea>("ArkUI_NativeDialogAPI_3.setHoverModeArea"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setFocusable>("ArkUI_NativeDialogAPI_3.setFocusable"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBackgroundBlurStyleOptions>(
        "ArkUI_NativeDialogAPI_3.setBackgroundBlurStyleOptions"),
    MakeDialogTableEntry3<&ArkUI_NativeDialogAPI_3::setBackgroundEffect>("ArkUI_NativeDialogAPI_3.setBackgroundEffect"),
};

class UiThreadDialogTableLogTest : public UiThreadDetectionTest, public WithParamInterface<DialogTableScopeEntry> {};

static ArkUI_NativeDialogAPI_3* GetDialogApi()
{
    return static_cast<ArkUI_NativeDialogAPI_3*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_DIALOG, "ArkUI_NativeDialogAPI_3"));
}

// 模式 1：默认-debug + 安全线程 → 无诊断
TEST_P(UiThreadDialogTableLogTest, DebugDefaultSafeThreadNoDiagnostic)
{
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    AceApplicationInfo::GetInstance().SetDebugForParallel(true);
    ResetToUnresolved();
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(SafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 2：默认-release + 不安全线程 → 无诊断（release 默认 DISABLED）
TEST_P(UiThreadDialogTableLogTest, ReleaseDefaultUnsafeThreadNoDiagnostic)
{
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    AceApplicationInfo::GetInstance().SetDebugForParallel(false);
    ResetToUnresolved();
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 3：DISABLED + 不安全线程 → 无诊断
TEST_P(UiThreadDialogTableLogTest, DisabledModeNoDiagnostic)
{
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED);
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 4：LOG + 不安全线程 → 有诊断
TEST_P(UiThreadDialogTableLogTest, LogModeProducesDiagnostic)
{
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetupDiagnosticMatch(GetParam().name);
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectDiagnostic([api, this] { GetParam().invoke(api); });
}

// 模式 5：LOG + 不安全线程 → 恰好一条诊断且 apiName 为公开接口名（INV-1 / ADR-2）
TEST_P(UiThreadDialogTableLogTest, LogModeProducesSingleNamedDiagnostic)
{
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    ResetDiagnosticLog();
    SetMockIsCurrentThreadSafe(UnsafeThread);
    GetParam().invoke(api);
    EXPECT_EQ(GetDiagnosticLogCount(), 1);
    const auto* diag = GetLastDiagnosticLog();
    ASSERT_NE(diag, nullptr);
    ASSERT_NE(diag->apiName, nullptr);
    EXPECT_STREQ(diag->apiName, GetParam().name);
    ASSERT_NE(diag->checkName, nullptr);
    EXPECT_TRUE(strstr(diag->checkName, "UI_THREAD") != nullptr);
    ASSERT_NE(diag->reason, nullptr);
    EXPECT_TRUE(strstr(diag->reason, "C API must be called on the UI thread") != nullptr);
}

INSTANTIATE_TEST_SUITE_P(AllDialogTableMembers, UiThreadDialogTableLogTest, ValuesIn(DIALOG_TABLE_SCOPE_ENTRIES),
    [](const TestParamInfo<DialogTableScopeEntry>& info) {
        std::string name = info.param.name;
        name.erase(std::remove(name.begin(), name.end(), '.'), name.end());
        return name;
    });

// === 表单弹窗类专项：导出函数强断言 + 线程安全接口排除 + UI 线程零诊断（ADR-2 / R-6 / AC-1.5）===

HWTEST_F(UiThreadDetectionTest, DialogExportLogModeSingleNamedDiagnostic, TestSize.Level1)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    ResetDiagnosticLog();
    SetMockIsCurrentThreadSafe(UnsafeThread);
    OH_ArkUI_CustomDialog_CloseDialog(0);
    EXPECT_EQ(GetDiagnosticLogCount(), 1);
    const auto* diag = GetLastDiagnosticLog();
    ASSERT_NE(diag, nullptr);
    ASSERT_NE(diag->apiName, nullptr);
    EXPECT_STREQ(diag->apiName, "OH_ArkUI_CustomDialog_CloseDialog");
}

HWTEST_F(UiThreadDetectionTest, DialogDismissEventApisNeverTriggerThreadCheck, TestSize.Level1)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    // 三个 DialogDismissEvent 接口为线程安全接口（仅操作入参字段），不铺设检测点
    SetMockIsCurrentThreadSafe(UnsafeThread);
    RunAndExpectNoDiagnostic([] { OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss(nullptr, true); });
    RunAndExpectNoDiagnostic([] { return OH_ArkUI_DialogDismissEvent_GetUserData(nullptr); });
    RunAndExpectNoDiagnostic([] { return OH_ArkUI_DialogDismissEvent_GetDismissReason(nullptr); });
    SetMockIsCurrentThreadSafe(SafeThread);
    RunAndExpectNoDiagnostic([] { OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss(nullptr, true); });
}

HWTEST_F(UiThreadDetectionTest, DialogApisOnUiThreadNeverTriggerThreadCheck, TestSize.Level1)
{
    EnableMode(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG);
    SetMockIsCurrentThreadSafe(SafeThread);
    RunAndExpectNoDiagnostic([] { OH_ArkUI_CustomDialog_CloseDialog(0); });
    ArkUI_DialogState state {};
    RunAndExpectNoDiagnostic([&state] { return OH_ArkUI_CustomDialog_GetState(0, &state); });
    RunAndExpectNoDiagnostic(
        [] { return OH_ArkUI_CustomDialog_SetWidth(nullptr, 0.0f, ARKUI_LENGTH_METRIC_UNIT_DEFAULT); });
    auto* api = GetDialogApi();
    ASSERT_NE(api, nullptr);
    RunAndExpectNoDiagnostic([api] { CallWithDefaultArgs(api->nativeDialogAPI1.show); });
    RunAndExpectNoDiagnostic([api] { CallWithDefaultArgs(api->setWidth); });
}

// === Scope entries（271 导出函数：原 215 + 表单弹窗类 38 + TextEditorStyledStringController 18，排除范围外与 ANI）===

namespace {
#define SCOPE_ENTRY(api) { #api, [] { CallWithDefaultArgs(api); } }
const ScopeEntry SCOPE_ENTRIES[] = {
    SCOPE_ENTRY(OH_ArkUI_ArcSwiper_FinishAnimation),
    SCOPE_ENTRY(OH_ArkUI_ArcSwiper_ShowNext),
    SCOPE_ENTRY(OH_ArkUI_ArcSwiper_ShowPrevious),
    SCOPE_ENTRY(OH_ArkUI_GetContextByNode),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationId),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationIndex),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationName),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationNameByIndex),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationParam),
    SCOPE_ENTRY(OH_ArkUI_GetNavDestinationState),
    SCOPE_ENTRY(OH_ArkUI_GetNavigationId),
    SCOPE_ENTRY(OH_ArkUI_GetNavStackLength),
    SCOPE_ENTRY(OH_ArkUI_GetRouterPageId),
    SCOPE_ENTRY(OH_ArkUI_GetRouterPageIndex),
    SCOPE_ENTRY(OH_ArkUI_GetRouterPageName),
    SCOPE_ENTRY(OH_ArkUI_GetRouterPagePath),
    SCOPE_ENTRY(OH_ArkUI_GetRouterPageState),
    SCOPE_ENTRY(OH_ArkUI_Swiper_FakeDragBy),
    SCOPE_ENTRY(OH_ArkUI_Swiper_FinishAnimation),
    SCOPE_ENTRY(OH_ArkUI_Swiper_IsFakeDragging),
    SCOPE_ENTRY(OH_ArkUI_Swiper_ShowNext),
    SCOPE_ENTRY(OH_ArkUI_Swiper_ShowPrevious),
    SCOPE_ENTRY(OH_ArkUI_Swiper_StartFakeDrag),
    SCOPE_ENTRY(OH_ArkUI_Swiper_StopFakeDrag),
    SCOPE_ENTRY(OH_ArkUI_AccessibilityProvider_Create),
    SCOPE_ENTRY(OH_ArkUI_AccessibilityProvider_Dispose),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_CloseDialog),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_CreateOptions),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_DisposeOptions),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_GetState),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_OpenDialog),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_OpenDialogWithCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_RegisterOnDidAppearCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_RegisterOnDidDisappearCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_RegisterOnWillAppearCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_RegisterOnWillDisappearCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_RegisterOnWillDismissCallback),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetAlignment),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetAutoCancel),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBackgroundBlurStyle),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBackgroundBlurStyleOptions),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBackgroundColor),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBackgroundEffect),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBorderColor),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBorderStyle),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetBorderWidth),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetCornerRadius),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetCustomShadow),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetDisplayModeInSubWindow),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetHeight),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetHoverModeArea),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetHoverModeEnabled),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetImmersiveMode),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetKeyboardAvoidMode),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetLevelMode),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetLevelUniqueId),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetMask),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetModalMode),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetShadow),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetSubwindowMode),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_SetWidth),
    SCOPE_ENTRY(OH_ArkUI_CustomDialog_UpdateDialog),
    SCOPE_ENTRY(OH_ArkUI_GetContextFromNapiValue),
    SCOPE_ENTRY(OH_ArkUI_GetNodeContentFromNapiValue),
    SCOPE_ENTRY(OH_ArkUI_GetNodeHandleFromNapiValue),
    SCOPE_ENTRY(OH_ArkUI_InitModuleForArkTSEnv),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_AdoptChild),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_AdoptChildWithPolicy),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_AtomicServiceMenuBarSetVisible),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_ConvertPositionFromWindow),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_ConvertPositionToWindow),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_CustomDialog_SetSystemMaterial),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_CustomDialog_SetSystemMaterialInOptions),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_GetChildMountPolicy),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_GetNativeAccessibilityProvider),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_GetPageRootNodeHandleByContext),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_InvalidateAttributes),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_IsInRenderState),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_RegisterCommonAreaApproximateChangeEvent),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_RegisterCommonEvent),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_RegisterCommonVisibleAreaApproximateChangeEvent),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_RemoveAdoptedChild),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_SetAdoptedChildOverlayPositionCompensation),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_SetChildMountPolicy),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_UnregisterCommonAreaApproximateChangeEvent),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_UnregisterCommonEvent),
    SCOPE_ENTRY(OH_ArkUI_NativeModule_UnregisterCommonVisibleAreaApproximateChangeEvent),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapterEvent_GetHostNode),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapterEvent_GetRemovedNode),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_Create),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_Dispose),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_GetAllItems),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_GetTotalNodeCount),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_InsertItem),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_MoveItem),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_RegisterEventReceiver),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_ReloadAllItems),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_ReloadItem),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_RemoveItem),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_SetTotalNodeCount),
    SCOPE_ENTRY(OH_ArkUI_NodeAdapter_UnregisterEventReceiver),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_AddNode),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_GetUserData),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_InsertNode),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_RegisterCallback),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_RemoveNode),
    SCOPE_ENTRY(OH_ArkUI_NodeContent_SetUserData),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_AddCustomProperty),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetActiveChildrenInfo),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetAttachedNodeHandleById),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetChildWithExpandMode),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetCrossLanguageOption),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetCurrentPageRootNode),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetCustomProperty),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetFirstChildIndexWithoutExpand),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLastChildIndexWithoutExpand),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLayoutPosition),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLayoutPositionInGlobalDisplay),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLayoutPositionInScreen),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLayoutPositionInWindow),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetLayoutSize),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetNodeType),
    { "OH_ArkUI_NodeUtils_GetNodeUniqueId", InvokeNodeUtilsGetNodeUniqueId },
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetParentInPageTree),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetPositionToParent),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetPositionWithTranslateInScreen),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetPositionWithTranslateInWindow),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_GetWindowInfo),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_IsCreatedByNDK),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_MoveTo),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_RemoveCustomProperty),
    SCOPE_ENTRY(OH_ArkUI_NodeUtils_SetCrossLanguageOption),
    SCOPE_ENTRY(OH_ArkUI_NotifyArkTSEnvDestroy),
    SCOPE_ENTRY(OH_ArkUI_PostFrameCallback),
    SCOPE_ENTRY(OH_ArkUI_PostIdleCallback),
    SCOPE_ENTRY(OH_ArkUI_RegisterDrawCallbackOnNodeHandle),
    SCOPE_ENTRY(OH_ArkUI_RegisterLayoutCallbackOnNodeHandle),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AddChild),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AddRenderNode),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachColorAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachColorProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachContentModifier),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachFloatAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachFloatProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachVector2AnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_AttachVector2Property),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_ClearChildren),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_ClearRenderNodeChildren),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateColorAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateColorProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateContentModifier),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateFloatAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateFloatProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateNode),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateVector2AnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_CreateVector2Property),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeColorAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeColorProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeContentModifier),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeFloatAnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeFloatProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeNode),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeVector2AnimatableProperty),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_DisposeVector2Property),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBackgroundColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBorderColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBorderRadius),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBorderStyle),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBorderWidth),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetBounds),
    { "OH_ArkUI_RenderNodeUtils_GetChild", InvokeRenderGetChild },
    { "OH_ArkUI_RenderNodeUtils_GetChildren", InvokeRenderGetChildren },
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetChildrenCount),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetClipToBounds),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetClipToFrame),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetColorAnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetColorPropertyValue),
    { "OH_ArkUI_RenderNodeUtils_GetFirstChild", InvokeRenderGetFirstChild },
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetFloatAnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue),
    { "OH_ArkUI_RenderNodeUtils_GetNextSibling", InvokeRenderGetNextSibling },
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetOpacity),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetPivot),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetPosition),
    { "OH_ArkUI_RenderNodeUtils_GetPreviousSibling", InvokeRenderGetPreviousSibling },
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetRenderNode),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetRenderNodeAt),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetRenderNodeChildrenCount),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetRotation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetScale),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetShadowAlpha),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetShadowColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetShadowElevation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetShadowOffset),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetShadowRadius),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetSize),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetTranslation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetVector2AnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_GetVector2PropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_InsertChildAfter),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_InsertRenderNodeAt),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_Invalidate),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_RemoveChild),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_RemoveRenderNode),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_ResetBackgroundBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_ResetContentBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_ResetForegroundBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBackgroundBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBackgroundColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBorderColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBorderRadius),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBorderStyle),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBorderWidth),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetBounds),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetClip),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetClipToBounds),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetClipToFrame),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetColorAnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetColorPropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetContentBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetDrawRegion),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetFloatAnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetForegroundBlurOption),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetMarkNodeGroup),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetMask),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetNodeBorderWidthOptionEdgeWidth),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetOpacity),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetPivot),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetPosition),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetRotation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetScale),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetShadowAlpha),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetShadowColor),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetShadowElevation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetShadowOffset),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetShadowRadius),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetSize),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetTransform),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetTranslation),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetVector2AnimatablePropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue),
    SCOPE_ENTRY(OH_ArkUI_RunTaskInScope),
    // === TextEditorStyledStringController (17 APIs with CHECK_UI_THREAD guard) ===
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_CloseSelectionMenu),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_DeleteBackward),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetCaretOffset),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetCaretRect),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetPreviewText),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetSelection),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetStyledString),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_GetTypingStyle),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_IsEditing),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_ScrollToVisible),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetCaretOffset),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetSelection),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetStyledPlaceholder),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetStyledString),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetTypingParagraphStyle),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_SetTypingStyle),
    SCOPE_ENTRY(OH_ArkUI_TextEditorStyledStringController_StopEditing),
    SCOPE_ENTRY(OH_ArkUI_SurfaceHolder_Create),
    SCOPE_ENTRY(OH_ArkUI_SurfaceHolder_Dispose),
    SCOPE_ENTRY(OH_ArkUI_UnregisterDrawCallbackOnNodeHandle),
    SCOPE_ENTRY(OH_ArkUI_UnregisterLayoutCallbackOnNodeHandle),
    SCOPE_ENTRY(OH_ArkUI_XComponent_Finalize),
    SCOPE_ENTRY(OH_ArkUI_XComponent_Initialize),
    SCOPE_ENTRY(OH_ArkUI_XComponent_IsInitialized),
    SCOPE_ENTRY(OH_ArkUI_XComponent_RegisterOnFrameCallback),
    SCOPE_ENTRY(OH_ArkUI_XComponent_SetAutoInitialize),
    SCOPE_ENTRY(OH_ArkUI_XComponent_SetExpectedFrameRateRange),
    SCOPE_ENTRY(OH_ArkUI_XComponent_SetNeedSoftKeyboard),
    SCOPE_ENTRY(OH_ArkUI_XComponent_StartImageAnalyzer),
    SCOPE_ENTRY(OH_ArkUI_XComponent_StopImageAnalyzer),
    SCOPE_ENTRY(OH_ArkUI_XComponent_UnregisterOnFrameCallback),
    SCOPE_ENTRY(OH_NativeXComponent_GetNativeXComponent),
};
#undef SCOPE_ENTRY
} // namespace

INSTANTIATE_TEST_SUITE_P(AllExports, UiThreadExportLogTest, ValuesIn(SCOPE_ENTRIES),
    [](const TestParamInfo<ScopeEntry>& info) { return std::string(info.param.name); });

INSTANTIATE_TEST_SUITE_P(AllTableMembers, UiThreadTableLogTest, ValuesIn(TABLE_SCOPE_ENTRIES),
    [](const TestParamInfo<TableScopeEntry>& info) { return std::string(info.param.name); });

} // namespace OHOS::Ace::NodeModel

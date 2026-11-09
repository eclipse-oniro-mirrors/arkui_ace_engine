/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <limits>
#include <thread>

#include "gtest/gtest.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "frameworks/core/accessibility/accessibility_node.h"
#include "frameworks/core/accessibility/accessibility_utils.h"
#include "frameworks/core/accessibility/native_interface_accessibility_provider.h"
#include "frameworks/core/accessibility/third_provider_tree_exporter.h"
#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/pattern/pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
class MockOhAccessibilityProvider : public AccessibilityProvider {
    DECLARE_ACE_TYPE(MockOhAccessibilityProvider, AccessibilityProvider);
public:
    MockOhAccessibilityProvider() = default;
    int32_t FindAccessibilityNodeInfosById(
        const int64_t elementId, const int32_t mode, const int32_t requestId,
        std::vector<ArkUI_AccessibilityElementInfo>& infos) override
    {
        return 0;
    }
    int32_t FindAccessibilityNodeInfosByText(
        const int64_t elementId, std::string text, const int32_t requestId,
        std::vector<ArkUI_AccessibilityElementInfo>& infos) override
    {
        return 0;
    }
    int32_t FindFocusedAccessibilityNode(
        const int64_t elementId, int32_t focusType, const int32_t requestId,
        ArkUI_AccessibilityElementInfo& elementInfo) override
    {
        return 0;
    }
    int32_t FindNextFocusAccessibilityNode(
        const int64_t elementId, int32_t direction, const int32_t requestId,
        ArkUI_AccessibilityElementInfo& elementInfo) override
    {
        return 0;
    }
    int32_t ExecuteAccessibilityAction(
        const int64_t elementId, int32_t action, const int32_t requestId,
        const std::map<std::string, std::string>& actionArguments) override
    {
        return 0;
    }
    int32_t ClearFocusedAccessibilityNode() override
    {
        return 0;
    }
    int32_t GetAccessibilityNodeCursorPosition(
        const int64_t elementId,
        const int32_t requestId,
        int32_t &cursorPosition) override
    {
        return 0;
    }
    int32_t SendAccessibilityAsyncEvent(
        const ArkUI_AccessibilityEventInfo& accessibilityEvent,
        void (*callback)(int32_t errorCode)) override
    {
        GTEST_LOG_(INFO) << "SendAccessibilityAsyncEvent";
        return 0;
    }
    void SendThirdAccessibilityProvider(
        const std::weak_ptr<ThirdAccessibilityManager>& thirdAccessibilityManager)
        override {}
};

class EventAccessibilityProvider : public MockOhAccessibilityProvider {
    DECLARE_ACE_TYPE(EventAccessibilityProvider, MockOhAccessibilityProvider);

public:
    std::function<void()> onSend;

    int32_t SendAccessibilityAsyncEvent(const ArkUI_AccessibilityEventInfo& event, void (*callback)(int32_t)) override
    {
        if (onSend) {
            onSend();
        }
        return 0;
    }
};
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
namespace {
std::string TEST_CONTENT_STR = "testForFindByText";
constexpr const char* INSTANCE_ID = "fake_instance_id";
constexpr int32_t MAX_TEST_PARAMETER = 20;
constexpr int32_t MAX_TEST_ELEMENT_COUNT = 10;
constexpr int32_t SEND_EVENT_FAILED = -1;
constexpr int32_t SEND_EVENT_SUCCESS = 0;
constexpr int32_t TEST_PARAMETER_OFFSET1 = 1;
constexpr int32_t TEST_PARAMETER_OFFSET2 = 2;
constexpr int32_t TEST_PARAMETER_OFFSET3 = 3;
constexpr size_t EXPORT_NODE_LIMIT = 10000;
constexpr size_t EXPORT_BYTE_LIMIT = 10 * 1024 * 1024;

class ProviderMockResult {
    public:
        ProviderMockResult()
        {
            for (int32_t i = 0; i < MAX_TEST_ELEMENT_COUNT; i++) {
                ArkUI_AccessibilityElementInfo tempElementInfo;
                tempElementInfo.SetElementId(i);
                tempElementInfo.SetContents(TEST_CONTENT_STR);
                elementInfosList_.push_back(tempElementInfo);
            }

            focusElementInfo_.SetElementId(
                MAX_TEST_ELEMENT_COUNT + TEST_PARAMETER_OFFSET1);
            nextFocusElementInfo_.SetElementId(
                MAX_TEST_ELEMENT_COUNT + TEST_PARAMETER_OFFSET2);

            originActionArguments_.emplace("action0", "1");
            originActionArguments_.emplace("action1", "2");
            cursorPosition_ = MAX_TEST_ELEMENT_COUNT + TEST_PARAMETER_OFFSET3;
        }
        ~ProviderMockResult() = default;

        void Reset()
        {
            elementId_ = 0;
            mode_ = ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT;
            requestId_ = 0;
            focusType_ = ARKUI_ACCESSIBILITY_NATIVE_FOCUS_TYPE_INVALID;
            direction_ = ARKUI_ACCESSIBILITY_NATIVE_DIRECTION_INVALID;
            action_ = ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_INVALID;
            receiveClear_ = false;
            errorCode_ = 0;
            instanceId_ = "";
        }

    public:
        int64_t elementId_;
        ArkUI_AccessibilitySearchMode mode_;
        int32_t requestId_;
        std::vector<ArkUI_AccessibilityElementInfo> elementInfosList_;
        ArkUI_AccessibilityFocusType focusType_;
        ArkUI_AccessibilityElementInfo focusElementInfo_;
        ArkUI_AccessibilityElementInfo nextFocusElementInfo_;
        ArkUI_AccessibilityFocusMoveDirection direction_;
        ArkUI_Accessibility_ActionType action_;
        std::map<std::string, std::string> originActionArguments_;
        std::map<std::string, std::string> resultActionArguments_;
        int32_t cursorPosition_;
        bool receiveClear_;

        bool registerResult_;

        int32_t errorCode_;
        std::string instanceId_;
};

ProviderMockResult g_providerMockResult;

int32_t MockFindAccessibilityNodeInfosById(
    int64_t elementId,
    ArkUI_AccessibilitySearchMode mode,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.mode_ = mode;
    g_providerMockResult.requestId_ = requestId;

    ArkUI_AccessibilityElementInfo* tempElementInfo;
    for (const auto &elementInfo: g_providerMockResult.elementInfosList_) {
        tempElementInfo = elementList->AddAndGetElementInfo();
        tempElementInfo->SetElementId(elementInfo.GetElementId());
    }

    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindAccessibilityNodeInfosByIdWithInstance(
    const char* instanceId,
    int64_t elementId,
    ArkUI_AccessibilitySearchMode mode,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockFindAccessibilityNodeInfosById(elementId, mode, requestId, elementList);
}

int32_t MockFindAccessibilityNodeInfosByIdEmpty(
    int64_t elementId,
    ArkUI_AccessibilitySearchMode mode,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.mode_ = mode;
    g_providerMockResult.requestId_ = requestId;
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindAccessibilityNodeInfosByText(
    int64_t elementId,
    const char* text,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.requestId_ = requestId;

    ArkUI_AccessibilityElementInfo* tempElementInfo;
    std::string textStr(text);
    for (const auto &elementInfo: g_providerMockResult.elementInfosList_) {
        if (elementInfo.GetContents() == textStr) {
            tempElementInfo = elementList->AddAndGetElementInfo();
            tempElementInfo->SetElementId(elementInfo.GetElementId());
        }
    }
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindAccessibilityNodeInfosByTextWithInstanceId(
    const char* instanceId,
    int64_t elementId,
    const char* text,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockFindAccessibilityNodeInfosByText(elementId, text, requestId, elementList);
}

int32_t MockFindAccessibilityNodeInfosByTextEmpty(
    int64_t elementId,
    const char* text,
    int32_t requestId,
    ArkUI_AccessibilityElementInfoList* elementList)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.requestId_ = requestId;
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindFocusedAccessibilityNode(
    int64_t elementId,
    ArkUI_AccessibilityFocusType focusType,
    int32_t requestId,
    ArkUI_AccessibilityElementInfo* elementInfo)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.focusType_ = focusType;
    g_providerMockResult.requestId_ = requestId;
    elementInfo->SetElementId(
        g_providerMockResult.focusElementInfo_.GetElementId());
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindFocusedAccessibilityNodeWithInstance(
    const char* instanceId,
    int64_t elementId,
    ArkUI_AccessibilityFocusType focusType,
    int32_t requestId,
    ArkUI_AccessibilityElementInfo* elementInfo)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockFindFocusedAccessibilityNode(elementId, focusType, requestId, elementInfo);
}

int32_t MockFindNextFocusAccessibilityNode(
    int64_t elementId,
    ArkUI_AccessibilityFocusMoveDirection direction,
    int32_t requestId,
    ArkUI_AccessibilityElementInfo* elementInfo)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.direction_ = direction;
    g_providerMockResult.requestId_ = requestId;
    elementInfo->SetElementId(
        g_providerMockResult.nextFocusElementInfo_.GetElementId());
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockFindNextFocusAccessibilityNodeWithInstance(
    const char* instanceId,
    int64_t elementId,
    ArkUI_AccessibilityFocusMoveDirection direction,
    int32_t requestId,
    ArkUI_AccessibilityElementInfo* elementInfo)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockFindNextFocusAccessibilityNode(elementId, direction, requestId, elementInfo);
}

int32_t MockExecuteAccessibilityAction(
    int64_t elementId,
    ArkUI_Accessibility_ActionType action,
    ArkUI_AccessibilityActionArguments *actionArguments,
    int32_t requestId)
{
    g_providerMockResult.action_ = action;
    g_providerMockResult.requestId_ = requestId;

    g_providerMockResult.resultActionArguments_.clear();
    for (auto &pair: g_providerMockResult.originActionArguments_) {
        auto value = actionArguments->FindValueByKey(pair.first.c_str());
        if (value) {
            std::string textStr(value);
            g_providerMockResult.resultActionArguments_.emplace(
                pair.first, textStr);
        }
    }

    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockExecuteAccessibilityActionWithInstance(
    const char* instanceId,
    int64_t elementId,
    ArkUI_Accessibility_ActionType action,
    ArkUI_AccessibilityActionArguments *actionArguments,
    int32_t requestId)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockExecuteAccessibilityAction(elementId, action, actionArguments, requestId);
}

int32_t MockClearFocusedFocusAccessibilityNode()
{
    g_providerMockResult.receiveClear_ = true;
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockClearFocusedFocusAccessibilityNodeWithInstance(const char* instanceId)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockClearFocusedFocusAccessibilityNode();
}

int32_t MockGetAccessibilityNodeCursorPosition(
    int64_t elementId,
    int32_t requestId,
    int32_t* index)
{
    g_providerMockResult.elementId_ = elementId;
    g_providerMockResult.requestId_ = requestId;
    *index = g_providerMockResult.cursorPosition_;
    return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
}

int32_t MockGetAccessibilityNodeCursorPositionWithInstance(
    const char* instanceId,
    int64_t elementId,
    int32_t requestId,
    int32_t* index)
{
    g_providerMockResult.instanceId_ = instanceId;
    return MockGetAccessibilityNodeCursorPosition(elementId, requestId, index);
}

void MockRegisterCallback(bool isRegistered)
{
    g_providerMockResult.registerResult_ = isRegistered;
}

void MockEventCallBack(int32_t errorCode)
{
    g_providerMockResult.errorCode_ = errorCode;
}

void BuildMockCallBack(ArkUI_AccessibilityProviderCallbacks &callBacks)
{
    // register success
    callBacks.findAccessibilityNodeInfosById =
        MockFindAccessibilityNodeInfosById;
    callBacks.findAccessibilityNodeInfosByText =
        MockFindAccessibilityNodeInfosByText;
    callBacks.findFocusedAccessibilityNode =
        MockFindFocusedAccessibilityNode;
    callBacks.findNextFocusAccessibilityNode =
        MockFindNextFocusAccessibilityNode;
    callBacks.executeAccessibilityAction =
        MockExecuteAccessibilityAction;
    callBacks.clearFocusedFocusAccessibilityNode =
        MockClearFocusedFocusAccessibilityNode;
    callBacks.getAccessibilityNodeCursorPosition =
        MockGetAccessibilityNodeCursorPosition;
}

void BuildMockCallBackWithInstance(ArkUI_AccessibilityProviderCallbacksWithInstance &callBacks)
{
    // register success
    callBacks.findAccessibilityNodeInfosById = MockFindAccessibilityNodeInfosByIdWithInstance;
    callBacks.findAccessibilityNodeInfosByText = MockFindAccessibilityNodeInfosByTextWithInstanceId;
    callBacks.findFocusedAccessibilityNode = MockFindFocusedAccessibilityNodeWithInstance;
    callBacks.findNextFocusAccessibilityNode = MockFindNextFocusAccessibilityNodeWithInstance;
    callBacks.executeAccessibilityAction = MockExecuteAccessibilityActionWithInstance;
    callBacks.clearFocusedFocusAccessibilityNode = MockClearFocusedFocusAccessibilityNodeWithInstance;
    callBacks.getAccessibilityNodeCursorPosition = MockGetAccessibilityNodeCursorPositionWithInstance;
}
} // namespace

class AccessibilityProviderTestNg : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_providerMockResult.Reset();
    }
    static void TearDownTestCase() {};
};


/**
 * @tc.name: accessibilityTest001
 * @tc.desc: AccessibilityProviderRegisterCallback abnormal
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest001, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks {};
    // callback null
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(nullptr);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findAccessibilityNodeInfosById null
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findAccessibilityNodeInfosByText null
    callBacks.findAccessibilityNodeInfosById = MockFindAccessibilityNodeInfosById;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findFocusedAccessibilityNode null
    callBacks.findAccessibilityNodeInfosByText =
        MockFindAccessibilityNodeInfosByText;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findNextFocusAccessibilityNode null
    callBacks.findFocusedAccessibilityNode = MockFindFocusedAccessibilityNode;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);

    // executeAccessibilityAction null
    callBacks.findNextFocusAccessibilityNode = MockFindNextFocusAccessibilityNode;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // clearFocusedFocusAccessibilityNode null
    callBacks.executeAccessibilityAction = MockExecuteAccessibilityAction;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // getAccessibilityNodeCursorPosition null
    callBacks.clearFocusedFocusAccessibilityNode =
        MockClearFocusedFocusAccessibilityNode;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    callBacks.getAccessibilityNodeCursorPosition =
        MockGetAccessibilityNodeCursorPosition;
}

/**
 * @tc.name: accessibilityTest002
 * @tc.desc: AccessibilityProviderRegisterCallback abnormal
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest002, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);
    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.registerResult_, true);
    EXPECT_EQ(accessibilityProvider.IsRegister(), true);

    // then register fail, call registercallback false
    callBacks.findAccessibilityNodeInfosById = nullptr;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(g_providerMockResult.registerResult_, false);
    EXPECT_TRUE(accessibilityProvider.IsRegister());
}

/**
 * @tc.name: accessibilityTest003
 * @tc.desc: FindAccessibilityNodeInfosById
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest003, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t mode = testParameter + TEST_PARAMETER_OFFSET1;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET2;
    std::vector<ArkUI_AccessibilityElementInfo> infos;

    // register fail
    ret = accessibilityProvider.FindAccessibilityNodeInfosById(
        elementId, mode, requestId, infos);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.FindAccessibilityNodeInfosById(
        elementId, mode, requestId, infos);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.elementId_, elementId);
    EXPECT_EQ(g_providerMockResult.mode_, mode);
    EXPECT_EQ(g_providerMockResult.requestId_, requestId);
    // id equals to prepared ids
    for (int64_t i = 0; i < g_providerMockResult.elementInfosList_.size(); i++) {
        EXPECT_EQ(g_providerMockResult.elementInfosList_[i].GetElementId(),
            infos[i].GetElementId());
    }

    // Empty provider results are successful.
    ArkUI_AccessibilityProvider accessibilityProvider2;
    accessibilityProvider2.SetRegisterCallback(MockRegisterCallback);
    callBacks.findAccessibilityNodeInfosById =
        MockFindAccessibilityNodeInfosByIdEmpty;
    ret = accessibilityProvider2.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    infos.clear();
    ret = accessibilityProvider2.FindAccessibilityNodeInfosById(
        elementId, mode, requestId, infos);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::OPERATOR_SUCCESS);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.name: ProviderEmptyQuery
 * @tc.desc: Empty provider results succeed and preserve callback errors.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ProviderEmptyQuery, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    constexpr int64_t rootId = -1;
    constexpr int32_t requestId = 0;
    constexpr auto mode = ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN;
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(rootId, mode, requestId, infos), NOT_REGISTERED);

    ArkUI_AccessibilityProviderCallbacks callbacks {};
    BuildMockCallBack(callbacks);
    callbacks.findAccessibilityNodeInfosById = MockFindAccessibilityNodeInfosByIdEmpty;
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(rootId, mode, requestId, infos), OPERATOR_SUCCESS);
    EXPECT_TRUE(infos.empty());

    callbacks.findAccessibilityNodeInfosById = [](int64_t, ArkUI_AccessibilitySearchMode, int32_t,
                                                  ArkUI_AccessibilityElementInfoList*) -> int32_t {
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(rootId, mode, requestId, infos),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_TRUE(infos.empty());

    callbacks.findAccessibilityNodeInfosById = MockFindAccessibilityNodeInfosById;
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(rootId, mode, requestId, infos), OPERATOR_SUCCESS);
    EXPECT_EQ(infos.size(), g_providerMockResult.elementInfosList_.size());
}

/**
 * @tc.name: ProviderEmptyQueryWithInstance
 * @tc.desc: Instance callbacks follow the same empty-success and error contract.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ProviderEmptyQueryWithInstance, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    ArkUI_AccessibilityProviderCallbacksWithInstance callbacks {};
    BuildMockCallBackWithInstance(callbacks);
    callbacks.findAccessibilityNodeInfosById = [](const char* instance, int64_t id, ArkUI_AccessibilitySearchMode mode,
                                                  int32_t request, ArkUI_AccessibilityElementInfoList*) -> int32_t {
        EXPECT_STREQ(instance, INSTANCE_ID);
        EXPECT_EQ(id, -1);
        EXPECT_EQ(mode, ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN);
        EXPECT_EQ(request, 0);
        return OPERATOR_SUCCESS;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callbacks), OPERATOR_SUCCESS);
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    constexpr auto mode = ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN;
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(-1, mode, 0, infos), OPERATOR_SUCCESS);
    EXPECT_TRUE(infos.empty());

    callbacks.findAccessibilityNodeInfosById = [](const char*, int64_t, ArkUI_AccessibilitySearchMode, int32_t,
                                                  ArkUI_AccessibilityElementInfoList*) -> int32_t {
        return COPY_FAILED;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callbacks), OPERATOR_SUCCESS);
    EXPECT_EQ(provider.FindAccessibilityNodeInfosById(-1, mode, 0, infos), COPY_FAILED);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.name: accessibilityTest004
 * @tc.desc: FindAccessibilityNodeInfosByText
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest004, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET1;
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    std::string text = TEST_CONTENT_STR;

    // register fail
    EXPECT_EQ(accessibilityProvider.FindAccessibilityNodeInfosByText(elementId, text, requestId, infos),
        AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    ASSERT_EQ(accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(accessibilityProvider.FindAccessibilityNodeInfosByText(elementId, text, requestId, infos),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.elementId_, elementId);
    EXPECT_EQ(g_providerMockResult.requestId_, requestId);
    ASSERT_EQ(infos.size(), g_providerMockResult.elementInfosList_.size());
    // all elementInfo's text matched
    for (size_t i = 0; i < infos.size(); i++) {
        EXPECT_EQ(g_providerMockResult.elementInfosList_[i].GetElementId(), infos[i].GetElementId());
    }

    // register with instance
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    BuildMockCallBackWithInstance(callBacksWithInstance);
    ASSERT_EQ(accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(
        INSTANCE_ID, &callBacksWithInstance),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    infos.clear();
    EXPECT_EQ(accessibilityProvider.FindAccessibilityNodeInfosByText(elementId, text, requestId, infos),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
}

/**
 * @tc.name: ProviderEmptyTextQuery
 * @tc.desc: Empty text query results succeed and preserve callback errors.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ProviderEmptyTextQuery, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    ArkUI_AccessibilityProviderCallbacks callbacks {};
    BuildMockCallBack(callbacks);
    callbacks.findAccessibilityNodeInfosByText = MockFindAccessibilityNodeInfosByTextEmpty;
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    EXPECT_EQ(provider.FindAccessibilityNodeInfosByText(-1, TEST_CONTENT_STR, 0, infos), OPERATOR_SUCCESS);
    EXPECT_TRUE(infos.empty());

    callbacks.findAccessibilityNodeInfosByText = [](int64_t, const char*, int32_t,
                                                   ArkUI_AccessibilityElementInfoList*) -> int32_t {
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    EXPECT_EQ(provider.FindAccessibilityNodeInfosByText(-1, TEST_CONTENT_STR, 0, infos),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
}

/**
 * @tc.name: accessibilityTest005
 * @tc.desc: FindFocusedAccessibilityNode and FindNextFocusAccessibilityNode
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest005, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t focusType = testParameter + TEST_PARAMETER_OFFSET1;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET2;
    int32_t direction = testParameter + TEST_PARAMETER_OFFSET3;
    ArkUI_AccessibilityElementInfo elementInfo;

    // register fail
    ret = accessibilityProvider.FindFocusedAccessibilityNode(
        elementId, focusType, requestId, elementInfo);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);
    ret = accessibilityProvider.FindNextFocusAccessibilityNode(
        elementId, direction, requestId, elementInfo);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);


    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.FindFocusedAccessibilityNode(
        elementId, focusType, requestId, elementInfo);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.elementId_, elementId);
    EXPECT_EQ(g_providerMockResult.focusType_, focusType);
    EXPECT_EQ(g_providerMockResult.requestId_, requestId);
    EXPECT_EQ(g_providerMockResult.focusElementInfo_.GetElementId(),
        elementInfo.GetElementId());

    ret = accessibilityProvider.FindNextFocusAccessibilityNode(
        elementId, direction, requestId, elementInfo);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.elementId_, elementId);
    EXPECT_EQ(g_providerMockResult.direction_, direction);
    EXPECT_EQ(g_providerMockResult.requestId_, requestId);
    EXPECT_EQ(g_providerMockResult.nextFocusElementInfo_.GetElementId(),
        elementInfo.GetElementId());

    // register with instance
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    BuildMockCallBackWithInstance(callBacksWithInstance);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacksWithInstance);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    ret = accessibilityProvider.FindFocusedAccessibilityNode(
        elementId, focusType, requestId, elementInfo);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    ret = accessibilityProvider.FindNextFocusAccessibilityNode(
        elementId, direction, requestId, elementInfo);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
}

/**
 * @tc.name: accessibilityTest006
 * @tc.desc: ExecuteAccessibilityAction
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest006, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t action = testParameter + TEST_PARAMETER_OFFSET1;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET2;
    std::map<std::string, std::string> actionArguments =
        g_providerMockResult.originActionArguments_;
    // register fail
    ret = accessibilityProvider.ExecuteAccessibilityAction(
        elementId, action, requestId, actionArguments);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.ExecuteAccessibilityAction(
        elementId, action, requestId, actionArguments);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(actionArguments, g_providerMockResult.resultActionArguments_);

    // register with instance
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    BuildMockCallBackWithInstance(callBacksWithInstance);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacksWithInstance);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    ret = accessibilityProvider.ExecuteAccessibilityAction(
        elementId, action, requestId, actionArguments);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
}

/**
 * @tc.name: accessibilityTest007
 * @tc.desc: ClearFocusedAccessibilityNode
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest007, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    // register fail
    ret = accessibilityProvider.ClearFocusedAccessibilityNode();
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.ClearFocusedAccessibilityNode();
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.receiveClear_, true);

    // register with instance
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    BuildMockCallBackWithInstance(callBacksWithInstance);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacksWithInstance);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    accessibilityProvider.ClearFocusedAccessibilityNode();
}

/**
 * @tc.name: accessibilityTest008
 * @tc.desc: GetAccessibilityNodeCursorPosition
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest008, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET1;
    int32_t cursorPosition;
    std::map<std::string, std::string> actionArguments =
        g_providerMockResult.originActionArguments_;
    // register fail
    ret = accessibilityProvider.GetAccessibilityNodeCursorPosition(
        elementId, requestId, cursorPosition);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.GetAccessibilityNodeCursorPosition(
        elementId, requestId, cursorPosition);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(cursorPosition, g_providerMockResult.cursorPosition_);

    // register with instance
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    BuildMockCallBackWithInstance(callBacksWithInstance);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacksWithInstance);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    ret = accessibilityProvider.GetAccessibilityNodeCursorPosition(
        elementId, requestId, cursorPosition);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
}

/**
 * @tc.name: accessibilityTest009
 * @tc.desc: SendAccessibilityAsyncEvent
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest009, TestSize.Level1)
{
    g_providerMockResult.Reset();
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;

    // 1.1 event null and callback null
    ret = accessibilityProvider.SendAccessibilityAsyncEvent(nullptr, nullptr);
    EXPECT_EQ(ret, SEND_EVENT_FAILED);

    // 1.2 event null and callback not null
    ret = accessibilityProvider.SendAccessibilityAsyncEvent(
        nullptr, MockEventCallBack);
    EXPECT_EQ(ret, SEND_EVENT_FAILED);
    EXPECT_EQ(g_providerMockResult.errorCode_, SEND_EVENT_FAILED);

    // 2.1 event not null and OH accessibilityProvider null and callback null
    ArkUI_AccessibilityEventInfo accessibilityEvent;
    ret = accessibilityProvider.SendAccessibilityAsyncEvent(
        &accessibilityEvent, nullptr);
    EXPECT_EQ(ret, SEND_EVENT_FAILED);
    // 2.2 event not null and  accessibilityProvider null and callback not null
    ret = accessibilityProvider.SendAccessibilityAsyncEvent(
        &accessibilityEvent, MockEventCallBack);
    EXPECT_EQ(ret, SEND_EVENT_FAILED);
    EXPECT_EQ(g_providerMockResult.errorCode_, SEND_EVENT_FAILED);


    // 3 event not null and  OH accessibilityProvider not null
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    accessibilityProvider.SetInnerAccessibilityProvider(ohAccessibilityProvider);
    ret = accessibilityProvider.SendAccessibilityAsyncEvent(
        &accessibilityEvent, MockEventCallBack);
    EXPECT_EQ(ret, SEND_EVENT_SUCCESS);
}

/**
 * @tc.name: ConcurrentInnerAccessibilityProvider
 * @tc.desc: Replace and release the inner provider concurrently with event sending.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ConcurrentInnerAccessibilityProvider, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    std::atomic<bool> start { false };
    std::atomic<int32_t> calls { 0 };
    constexpr int32_t ITERATIONS = 2000;
    std::thread writer([&] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < ITERATIONS; ++i) {
            auto inner = AceType::MakeRefPtr<EventAccessibilityProvider>();
            inner->onSend = [&] { ++calls; };
            provider.SetInnerAccessibilityProvider(inner);
            provider.SetInnerAccessibilityProvider(nullptr);
        }
    });
    ArkUI_AccessibilityEventInfo event;
    int32_t successful = 0;
    start.store(true);
    for (int32_t i = 0; i < ITERATIONS; ++i) {
        auto result = provider.SendAccessibilityAsyncEvent(&event, nullptr);
        EXPECT_TRUE(result == SEND_EVENT_SUCCESS || result == SEND_EVENT_FAILED);
        successful += result == SEND_EVENT_SUCCESS;
    }
    writer.join();
    EXPECT_EQ(calls.load(), successful);
    EXPECT_EQ(provider.SendAccessibilityAsyncEvent(&event, nullptr), SEND_EVENT_FAILED);
}

/**
 * @tc.name: InnerAccessibilityProviderReentrantSend
 * @tc.desc: Event dispatch retains its provider and allows reentrant deregistration.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, InnerAccessibilityProviderReentrantSend, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    auto inner = AceType::MakeRefPtr<EventAccessibilityProvider>();
    WeakPtr<EventAccessibilityProvider> weakInner = inner;
    inner->onSend = [&] {
        provider.SetInnerAccessibilityProvider(nullptr);
        inner.Reset();
        EXPECT_FALSE(weakInner.Invalid());
    };
    provider.SetInnerAccessibilityProvider(inner);
    ArkUI_AccessibilityEventInfo event;
    EXPECT_EQ(provider.SendAccessibilityAsyncEvent(&event, nullptr), SEND_EVENT_SUCCESS);
    EXPECT_TRUE(weakInner.Invalid());
    EXPECT_EQ(provider.SendAccessibilityAsyncEvent(&event, nullptr), SEND_EVENT_FAILED);

    inner = AceType::MakeRefPtr<EventAccessibilityProvider>();
    provider.SetInnerAccessibilityProvider(inner);
    inner.Reset();
    EXPECT_EQ(provider.SendAccessibilityAsyncEvent(&event, nullptr), SEND_EVENT_FAILED);
}

/**
 * @tc.name: ConcurrentRegistrationNotification
 * @tc.desc: Registration notifications use a stable callback while UI replaces it.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ConcurrentRegistrationNotification, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    std::atomic<int32_t> calls { 0 };
    std::atomic<bool> start { false };
    constexpr int32_t ITERATIONS = 2000;
    provider.SetRegisterCallback([&](bool) { ++calls; });
    std::thread writer([&] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < ITERATIONS; ++i) {
            provider.SetRegisterCallback([&](bool) { ++calls; });
        }
    });
    ArkUI_AccessibilityProviderCallbacks callbacks {};
    start.store(true);
    for (int32_t i = 0; i < ITERATIONS; ++i) {
        EXPECT_EQ(provider.AccessibilityProviderRegisterCallback(&callbacks),
            ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    }
    writer.join();
    EXPECT_EQ(calls.load(), ITERATIONS);
}

/**
 * @tc.name: ConcurrentRegistrationReadiness
 * @tc.desc: UI can inspect registration readiness while NDK publishes callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ConcurrentRegistrationReadiness, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    ArkUI_AccessibilityProviderCallbacks callbacks {};
    BuildMockCallBack(callbacks);
    std::atomic<bool> start { false };
    constexpr int32_t ITERATIONS = 2000;
    std::thread writer([&] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < ITERATIONS; ++i) {
            provider.AccessibilityProviderRegisterCallback(&callbacks);
        }
    });
    EXPECT_FALSE(provider.IsRegister());
    start.store(true);
    bool observedRegistration = false;
    for (int32_t i = 0; i < ITERATIONS; ++i) {
        bool registered = provider.IsRegister();
        EXPECT_FALSE(observedRegistration && !registered);
        observedRegistration = registered;
    }
    writer.join();
    EXPECT_TRUE(provider.IsRegister());
}

/**
 * @tc.name: RegistrationNotificationReentrantCallback
 * @tc.desc: A notification can replace its own callback without holding the registration mutex.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, RegistrationNotificationReentrantCallback, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    int32_t calls = 0;
    provider.SetRegisterCallback([&](bool isRegister) {
        EXPECT_FALSE(isRegister);
        ++calls;
        provider.SetRegisterCallback(nullptr);
    });
    ArkUI_AccessibilityProviderCallbacks callbacks {};
    EXPECT_EQ(
        provider.AccessibilityProviderRegisterCallback(&callbacks), ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(
        provider.AccessibilityProviderRegisterCallback(&callbacks), ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(calls, 1);
}

/**
 * @tc.name: accessibilityTest010
 * @tc.desc: AccessibilityProviderRegisterCallbackWithInstance test
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest010, TestSize.Level1)
{
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret = 0;
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacks {};
    // callback null
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(nullptr);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findAccessibilityNodeInfosById null
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findAccessibilityNodeInfosByText null
    callBacks.findAccessibilityNodeInfosById = MockFindAccessibilityNodeInfosByIdWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findFocusedAccessibilityNode null
    callBacks.findAccessibilityNodeInfosByText =MockFindAccessibilityNodeInfosByTextWithInstanceId;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // findNextFocusAccessibilityNode null
    callBacks.findFocusedAccessibilityNode = MockFindFocusedAccessibilityNodeWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);

    // executeAccessibilityAction null
    callBacks.findNextFocusAccessibilityNode = MockFindNextFocusAccessibilityNodeWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // clearFocusedFocusAccessibilityNode null
    callBacks.executeAccessibilityAction = MockExecuteAccessibilityActionWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    // getAccessibilityNodeCursorPosition null
    callBacks.clearFocusedFocusAccessibilityNode = MockClearFocusedFocusAccessibilityNodeWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER);
    EXPECT_EQ(accessibilityProvider.IsRegister(), false);
    callBacks.getAccessibilityNodeCursorPosition = MockGetAccessibilityNodeCursorPositionWithInstance;
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(accessibilityProvider.IsRegister(), true);
    EXPECT_EQ(g_providerMockResult.registerResult_, true);
}

/**
 * @tc.name: accessibilityTest011
 * @tc.desc: FindAccessibilityNodeInfosByIdWithInstance
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, AccessibilityProviderTest011, TestSize.Level1)
{
    ArkUI_AccessibilityProvider accessibilityProvider;
    int32_t ret;
    ArkUI_AccessibilityProviderCallbacks callBacks;
    ArkUI_AccessibilityProviderCallbacksWithInstance callBacksWithInstance;
    accessibilityProvider.SetRegisterCallback(MockRegisterCallback);

    auto now = std::chrono::system_clock::now();
    int32_t testParameter =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count();
    testParameter = testParameter % MAX_TEST_PARAMETER;
    int64_t elementId = testParameter;
    int32_t mode = testParameter + TEST_PARAMETER_OFFSET1;
    int32_t requestId = testParameter + TEST_PARAMETER_OFFSET2;
    std::vector<ArkUI_AccessibilityElementInfo> infos;

    // register fail
    ret = accessibilityProvider.FindAccessibilityNodeInfosById(
        elementId, mode, requestId, infos);
    EXPECT_EQ(ret, AccessibilityProviderOperatorErrorCode::NOT_REGISTERED);

    // register success
    BuildMockCallBack(callBacks);
    BuildMockCallBackWithInstance(callBacksWithInstance);
    
    ret = accessibilityProvider.AccessibilityProviderRegisterCallback(&callBacks);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    ret = accessibilityProvider.AccessibilityProviderRegisterCallbackWithInstance(INSTANCE_ID, &callBacksWithInstance);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);

    ret = accessibilityProvider.FindAccessibilityNodeInfosById(
        elementId, mode, requestId, infos);
    EXPECT_EQ(ret, ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(g_providerMockResult.elementId_, elementId);
    EXPECT_EQ(g_providerMockResult.mode_, mode);
    EXPECT_EQ(g_providerMockResult.requestId_, requestId);
    EXPECT_EQ(g_providerMockResult.instanceId_, INSTANCE_ID);
}

/**
 * @tc.name: CallbackSnapshotSurvivesReregistration
 * @tc.desc: A callback can re-register without changing its in-flight instance id or deadlocking.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, CallbackSnapshotSurvivesReregistration, TestSize.Level1)
{
    static ArkUI_AccessibilityProvider* currentProvider = nullptr;
    ArkUI_AccessibilityProvider provider;
    currentProvider = &provider;
    ArkUI_AccessibilityProviderCallbacksWithInstance callbacks {};
    BuildMockCallBackWithInstance(callbacks);
    callbacks.clearFocusedFocusAccessibilityNode = [](const char* instanceId) -> int32_t {
        EXPECT_STREQ(instanceId, "first-instance");
        ArkUI_AccessibilityProviderCallbacksWithInstance replacement {};
        BuildMockCallBackWithInstance(replacement);
        replacement.clearFocusedFocusAccessibilityNode = [](const char* id) -> int32_t {
            return std::strcmp(id, "second-instance") == 0 ? 0 : -1;
        };
        EXPECT_EQ(currentProvider->AccessibilityProviderRegisterCallbackWithInstance("second-instance", &replacement),
            ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
        // The original string must remain alive and unchanged until this callback returns.
        return std::strcmp(instanceId, "first-instance") == 0 ? 0 : -1;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallbackWithInstance("first-instance", &callbacks),
        ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    EXPECT_EQ(provider.ClearFocusedAccessibilityNode(), 0);
    EXPECT_EQ(provider.ClearFocusedAccessibilityNode(), 0);
    currentProvider = nullptr;
}

/**
 * @tc.name: ConcurrentCallbackRegistration
 * @tc.desc: Queries always observe a handler and instance id from the same registration.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityProviderTestNg, ConcurrentCallbackRegistration, TestSize.Level1)
{
    ArkUI_AccessibilityProvider provider;
    ArkUI_AccessibilityProviderCallbacksWithInstance first {};
    BuildMockCallBackWithInstance(first);
    auto second = first;
    first.clearFocusedFocusAccessibilityNode = [](const char* id) -> int32_t {
        return std::strcmp(id, "first-instance") == 0 ? 0 : -1;
    };
    second.clearFocusedFocusAccessibilityNode = [](const char* id) -> int32_t {
        return std::strcmp(id, "second-instance-with-a-longer-name") == 0 ? 0 : -1;
    };
    ASSERT_EQ(provider.AccessibilityProviderRegisterCallbackWithInstance("first-instance", &first), 0);
    std::atomic<bool> start { false };
    std::atomic<int32_t> failures { 0 };
    std::thread writer([&] {
        while (!start.load()) {}
        for (int32_t i = 0; i < 10000; ++i) {
            provider.AccessibilityProviderRegisterCallbackWithInstance("second-instance-with-a-longer-name", &second);
            provider.AccessibilityProviderRegisterCallbackWithInstance("first-instance", &first);
        }
    });
    auto query = [&] {
        while (!start.load()) {}
        for (int32_t i = 0; i < 20000; ++i) {
            if (provider.ClearFocusedAccessibilityNode() != 0) {
                ++failures;
            }
        }
    };
    std::thread firstReader(query);
    std::thread secondReader(query);
    start.store(true);
    writer.join();
    firstReader.join();
    secondReader.join();
    EXPECT_EQ(failures.load(), 0);
}

class ThirdProviderPasswordExportTest : public testing::TestWithParam<bool> {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        host_ = FrameNode::CreateFrameNode("ProviderHost", 1, AceType::MakeRefPtr<Pattern>());
        ArkUI_AccessibilityProviderCallbacks callbacks {};
        BuildMockCallBack(callbacks);
        callbacks.findAccessibilityNodeInfosById = [](int64_t, ArkUI_AccessibilitySearchMode, int32_t,
                                                      ArkUI_AccessibilityElementInfoList* list) -> int32_t {
            auto* info = list->AddAndGetElementInfo();
            if (info == nullptr) {
                return ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
            }
            info->SetElementId(1);
            info->SetParentId(-1);
            info->SetContents("provider-test-content");
            info->SetPassword(GetParam());
            return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
        };
        ASSERT_EQ(provider_.AccessibilityProviderRegisterCallback(&callbacks),
            ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL);
    }

    void TearDown() override
    {
        host_.Reset();
        MockPipelineContext::TearDown();
    }

protected:
    ArkUI_AccessibilityProvider provider_;
    RefPtr<FrameNode> host_;
};

/**
 * @tc.name: TreeContents
 * @tc.desc: Tree export hides password contents and preserves ordinary text.
 * @tc.type: FUNC
 */
TEST_P(ThirdProviderPasswordExportTest, TreeContents)
{
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetString("contents"), GetParam() ? "" : "provider-test-content");
    EXPECT_EQ(tree->GetBool("password"), GetParam());
    if (GetParam()) {
        EXPECT_EQ(tree->ToString().find("provider-test-content"), std::string::npos);
    }
}

/**
 * @tc.name: FlatContents
 * @tc.desc: Flat export hides password contents and preserves ordinary text and budget accounting.
 * @tc.type: FUNC
 */
TEST_P(ThirdProviderPasswordExportTest, FlatContents)
{
    ThirdProviderTreeExportBudget budget;
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    ASSERT_NE(result.nodes, nullptr);
    ASSERT_EQ(result.nodes->GetArraySize(), 1);
    auto node = result.nodes->GetArrayItem(0);
    EXPECT_EQ(node->GetString("contents"), GetParam() ? "" : "provider-test-content");
    EXPECT_EQ(node->GetBool("password"), GetParam());
    const auto json = result.nodes->ToString();
    EXPECT_EQ(budget.nodes, 1u);
    EXPECT_EQ(budget.bytes, json.size());
    if (GetParam()) {
        EXPECT_EQ(json.find("provider-test-content"), std::string::npos);
    }
}

/**
 * @tc.name: BudgetCountsOnlyExportedContents
 * @tc.desc: Hidden password contents do not exhaust the JSON budget; ordinary contents still do.
 * @tc.type: FUNC
 */
TEST_P(ThirdProviderPasswordExportTest, BudgetCountsOnlyExportedContents)
{
    ArkUI_AccessibilityProviderCallbacks callbacks {};
    BuildMockCallBack(callbacks);
    callbacks.findAccessibilityNodeInfosById = [](int64_t, ArkUI_AccessibilitySearchMode, int32_t,
                                                  ArkUI_AccessibilityElementInfoList* list) -> int32_t {
        auto* info = list->AddAndGetElementInfo();
        info->SetElementId(1);
        info->SetParentId(-1);
        info->SetContents(std::string(EXPORT_BYTE_LIMIT + 1, 'x'));
        info->SetPassword(GetParam());
        return ARKUI_ACCESSIBILITY_NATIVE_RESULT_SUCCESSFUL;
    };
    ASSERT_EQ(provider_.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    ThirdProviderTreeExportBudget budget;
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    if (GetParam()) {
        ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
        ASSERT_NE(result.nodes, nullptr);
        EXPECT_EQ(result.nodes->GetArrayItem(0)->GetString("contents"), "");
        EXPECT_EQ(budget.bytes, result.nodes->ToString().size());
        ASSERT_NE(tree, nullptr);
        EXPECT_FALSE(tree->Contains("contents"));
    } else {
        EXPECT_EQ(result.status, ThirdProviderExportStatus::SIZE_LIMIT);
        EXPECT_EQ(result.nodes, nullptr);
        EXPECT_EQ(tree, nullptr);
        EXPECT_EQ(budget.nodes, 0u);
        EXPECT_EQ(budget.bytes, 0u);
    }
}

INSTANTIATE_TEST_SUITE_P(PasswordAndPlainText, ThirdProviderPasswordExportTest, testing::Bool());

namespace {
ArkUI_AccessibilityElementInfo MakeExportNode(
    int32_t id, int64_t parent, std::initializer_list<int64_t> children = {})
{
    ArkUI_AccessibilityElementInfo info;
    info.SetElementId(id);
    info.SetParentId(parent);
    for (auto child : children) {
        info.AddChildNodeId(child);
    }
    return info;
}

struct ProviderExportMockResult {
    std::vector<ArkUI_AccessibilityElementInfo> nodes { MakeExportNode(1, -1) };
    std::vector<ArkUI_AccessibilityElementInfo> roots { MakeExportNode(1, -1) };
    std::vector<ArkUI_AccessibilitySearchMode> queries;
    int32_t recursiveResult = OPERATOR_SUCCESS;
    int32_t rootResult = OPERATOR_SUCCESS;
};

ProviderExportMockResult g_exportResult;

int32_t MockQueryExportNodes(int64_t id, ArkUI_AccessibilitySearchMode mode, int32_t request,
    ArkUI_AccessibilityElementInfoList* list)
{
    EXPECT_EQ(id, -1);
    EXPECT_EQ(request, 0);
    EXPECT_TRUE(mode == ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT ||
        mode == ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN);
    g_exportResult.queries.push_back(mode);
    const bool current = mode == ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT;
    const auto result = current ? g_exportResult.rootResult : g_exportResult.recursiveResult;
    if (result != OPERATOR_SUCCESS) {
        return result;
    }
    for (const auto& info : current ? g_exportResult.roots : g_exportResult.nodes) {
        auto* output = list->AddAndGetElementInfo();
        if (!output) {
            return ARKUI_ACCESSIBILITY_NATIVE_RESULT_OUT_OF_MEMORY;
        }
        *output = info;
    }
    return OPERATOR_SUCCESS;
}
} // namespace

class ThirdProviderTreeExportTest : public testing::Test {
public:
    void SetUp() override
    {
        g_exportResult = ProviderExportMockResult {};
        MockPipelineContext::SetUp();
        host_ = FrameNode::CreateFrameNode("ProviderHost", 1, AceType::MakeRefPtr<Pattern>());
        ArkUI_AccessibilityProviderCallbacks callbacks {};
        BuildMockCallBack(callbacks);
        callbacks.findAccessibilityNodeInfosById = MockQueryExportNodes;
        ASSERT_EQ(provider_.AccessibilityProviderRegisterCallback(&callbacks), OPERATOR_SUCCESS);
    }

    void TearDown() override
    {
        host_.Reset();
        MockPipelineContext::TearDown();
        g_exportResult = ProviderExportMockResult {};
    }

protected:
    void ExpectFlatFailure(ThirdProviderExportStatus status,
        ThirdProviderTreeExportBudget budget = { 7, 123 })
    {
        const auto before = budget;
        auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
        EXPECT_EQ(result.status, status);
        EXPECT_EQ(result.nodes, nullptr);
        EXPECT_EQ(budget.nodes, before.nodes);
        EXPECT_EQ(budget.bytes, before.bytes);
    }

    ArkUI_AccessibilityProvider provider_;
    RefPtr<FrameNode> host_;
};

/**
 * @tc.name: EmptyExportSkipsRootQuery
 * @tc.desc: An empty successful query exports [] and charges only its brackets.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, EmptyExportSkipsRootQuery, TestSize.Level1)
{
    g_exportResult.nodes.clear();
    g_exportResult.rootResult = ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    ThirdProviderTreeExportBudget budget { 7, 123 };
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    ASSERT_NE(result.nodes, nullptr);
    EXPECT_EQ(result.nodes->ToString(), "[]");
    EXPECT_EQ(budget.nodes, 7u);
    EXPECT_EQ(budget.bytes, 125u);
    const std::vector<ArkUI_AccessibilitySearchMode> expectedQueries {
        ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN };
    EXPECT_EQ(g_exportResult.queries, expectedQueries);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
}

/**
 * @tc.name: MissingProviderAndHost
 * @tc.desc: Missing providers and hosts return distinct statuses without charging the budget.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, MissingProviderAndHost, TestSize.Level1)
{
    ArkUI_AccessibilityProvider unregistered;
    for (auto* provider : { static_cast<ArkUI_AccessibilityProvider*>(nullptr), &unregistered }) {
        ThirdProviderTreeExportBudget budget { 7, 123 };
        auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(provider, host_, budget);
        EXPECT_EQ(result.status, ThirdProviderExportStatus::NOT_REGISTERED);
        EXPECT_EQ(result.nodes, nullptr);
        EXPECT_EQ(budget.nodes, 7u);
        EXPECT_EQ(budget.bytes, 123u);
        EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(provider, host_), nullptr);
    }
    host_.Reset();
    ExpectFlatFailure(ThirdProviderExportStatus::COLLECTION_FAILED);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
    EXPECT_TRUE(g_exportResult.queries.empty());
}

/**
 * @tc.name: QueryFailuresPreserveBudget
 * @tc.desc: Recursive and root callback errors remain query failures.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, QueryFailuresPreserveBudget, TestSize.Level1)
{
    g_exportResult.recursiveResult = ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    ExpectFlatFailure(ThirdProviderExportStatus::QUERY_FAILED);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
    g_exportResult.recursiveResult = OPERATOR_SUCCESS;
    g_exportResult.rootResult = ARKUI_ACCESSIBILITY_NATIVE_RESULT_BAD_PARAMETER;
    ExpectFlatFailure(ThirdProviderExportStatus::QUERY_FAILED);
    // Hierarchical export needs only the recursive query.
    EXPECT_NE(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
}

/**
 * @tc.name: RootQueryRequiresExactlyOneKnownRoot
 * @tc.desc: Empty, multiple and absent root IDs are invalid even after a successful callback.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, RootQueryRequiresExactlyOneKnownRoot, TestSize.Level1)
{
    g_exportResult.roots.clear();
    ExpectFlatFailure(ThirdProviderExportStatus::INVALID_DATA);
    g_exportResult.roots = { MakeExportNode(1, -1), MakeExportNode(2, -1) };
    ExpectFlatFailure(ThirdProviderExportStatus::INVALID_DATA);
    g_exportResult.roots = { MakeExportNode(9, -1) };
    ExpectFlatFailure(ThirdProviderExportStatus::INVALID_DATA);
}

/**
 * @tc.name: FlatExportPreservesOrderAndEscapedText
 * @tc.desc: Out-of-order nodes and external root parents are valid; JSON bytes are charged exactly.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, FlatExportPreservesOrderAndEscapedText, TestSize.Level1)
{
    g_exportResult.nodes = { MakeExportNode(2, 1), MakeExportNode(1, 999, { 2 }) };
    const std::string contents = "quote\" slash\\ newline\n control\x01";
    g_exportResult.nodes[0].SetContents(contents);
    ThirdProviderTreeExportBudget budget { 7, 123 };
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    ASSERT_NE(result.nodes, nullptr);
    ASSERT_EQ(result.nodes->GetArraySize(), 2);
    auto child = result.nodes->GetArrayItem(0);
    EXPECT_EQ(child->GetString("elementId"), "2");
    EXPECT_EQ(child->GetString("parentId"), "1");
    EXPECT_EQ(child->GetString("contents"), contents);
    auto root = result.nodes->GetArrayItem(1);
    EXPECT_EQ(root->GetString("parentId"), "999");
    EXPECT_EQ(root->GetValue("childNodeIds")->ToString(), "[\"2\"]");
    EXPECT_EQ(budget.nodes, 9u);
    EXPECT_EQ(budget.bytes, 123u + result.nodes->ToString().size());
    const std::vector<ArkUI_AccessibilitySearchMode> expectedQueries {
        ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN,
        ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT };
    EXPECT_EQ(g_exportResult.queries, expectedQueries);
}

struct InvalidExportTree {
    const char* name;
    std::vector<ArkUI_AccessibilityElementInfo> nodes;
};

class ThirdProviderInvalidTreeTest : public ThirdProviderTreeExportTest,
                                     public testing::WithParamInterface<InvalidExportTree> {};

/**
 * @tc.name: RejectsInvalidGraph
 * @tc.desc: Diagnostics for malformed graphs preserve INVALID_DATA and the caller's budget.
 * @tc.type: FUNC
 */
TEST_P(ThirdProviderInvalidTreeTest, RejectsInvalidGraph)
{
    g_exportResult.nodes = GetParam().nodes;
    ExpectFlatFailure(ThirdProviderExportStatus::INVALID_DATA);
}

INSTANTIATE_TEST_SUITE_P(GraphValidation, ThirdProviderInvalidTreeTest, testing::Values(
    InvalidExportTree { "DuplicateId", { MakeExportNode(1, -1), MakeExportNode(1, -1) } },
    InvalidExportTree { "MultipleRoots", { MakeExportNode(1, -1), MakeExportNode(2, -1) } },
    InvalidExportTree { "Orphan", { MakeExportNode(1, -1), MakeExportNode(2, 99) } },
    InvalidExportTree { "InternalRootParent", { MakeExportNode(1, 2, { 2 }), MakeExportNode(2, 1) } },
    InvalidExportTree { "MissingChild", { MakeExportNode(1, -1, { 99 }) } },
    InvalidExportTree { "SelfChild", { MakeExportNode(1, -1, { 1 }) } },
    InvalidExportTree { "RootAsChild", { MakeExportNode(1, -1, { 2 }), MakeExportNode(2, 1, { 1 }) } },
    InvalidExportTree { "RepeatedChild", { MakeExportNode(1, -1, { 2, 2 }), MakeExportNode(2, 1) } },
    InvalidExportTree { "SharedChild", {
        MakeExportNode(1, -1, { 2, 3 }), MakeExportNode(2, 1, { 3 }), MakeExportNode(3, 1) } },
    InvalidExportTree { "SharedChildReordered", {
        MakeExportNode(2, 1, { 3 }), MakeExportNode(3, 1), MakeExportNode(1, -1, { 2, 3 }) } },
    InvalidExportTree { "ParentMismatch", {
        MakeExportNode(1, -1, { 2 }), MakeExportNode(2, 1, { 3 }), MakeExportNode(3, 1) } },
    InvalidExportTree { "UnreferencedChild", { MakeExportNode(1, -1), MakeExportNode(2, 1) } },
    InvalidExportTree { "DisconnectedCycle", {
        MakeExportNode(1, -1), MakeExportNode(2, 3, { 3 }), MakeExportNode(3, 2, { 2 }) } }),
    [](const testing::TestParamInfo<InvalidExportTree>& info) { return info.param.name; });

/**
 * @tc.name: TreeKeepsFirstDuplicatePayloadAndParent
 * @tc.desc: Duplicate IDs cannot replace the first payload or move it to another parent.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, TreeKeepsFirstDuplicatePayloadAndParent, TestSize.Level1)
{
    g_exportResult.nodes = { MakeExportNode(1, -1), MakeExportNode(2, 1), MakeExportNode(2, 3),
        MakeExportNode(3, 1), MakeExportNode(4, 2) };
    g_exportResult.nodes[1].SetContents("first");
    g_exportResult.nodes[2].SetContents("duplicate");
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    auto children = tree->GetValue("$children");
    ASSERT_EQ(children->GetArraySize(), 2);
    auto first = children->GetArrayItem(0);
    EXPECT_EQ(first->GetInt("$ID"), 2);
    EXPECT_EQ(first->GetString("contents"), "first");
    EXPECT_EQ(first->GetValue("$children")->GetArrayItem(0)->GetInt("$ID"), 4);
    EXPECT_FALSE(children->GetArrayItem(1)->Contains("$children"));
    EXPECT_EQ(tree->ToString().find("duplicate"), std::string::npos);
}

/**
 * @tc.name: TreeSerializesGeometryAndAttributes
 * @tc.desc: Provider type, host-local rectangle, states and extended attributes survive tree export.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, TreeSerializesGeometryAndAttributes, TestSize.Level1)
{
    auto& info = g_exportResult.nodes[0];
    info.SetComponentType("Slider");
    info.SetRect({ 10, 20, 30, 40 });
    info.SetHintText("hint");
    info.SetAccessibilityText("accessible");
    info.SetEnabled(true);
    info.SetZIndex(3);
    info.SetOpacity(0.5f);
    info.SetRangeInfo({ 0, 100, 25 });
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetInt("$ID"), 1);
    EXPECT_EQ(tree->GetInt("$parentID"), -1);
    EXPECT_EQ(tree->GetString("$type"), "Slider");
    EXPECT_EQ(tree->GetString("type"), "third");
    EXPECT_EQ(tree->GetString("$rect"), "[10.00, 20.00],[30.00,40.00]");
    EXPECT_EQ(tree->GetString("hintText"), "hint");
    EXPECT_EQ(tree->GetString("accessibilityText"), "accessible");
    EXPECT_TRUE(tree->GetBool("enabled"));
    EXPECT_EQ(tree->GetInt("zIndex"), 3);
    EXPECT_DOUBLE_EQ(tree->GetDouble("opacity"), 0.5);
    auto range = tree->GetValue("$attrs")->GetValue("range");
    EXPECT_DOUBLE_EQ(range->GetDouble("min"), 0);
    EXPECT_DOUBLE_EQ(range->GetDouble("max"), 100);
    EXPECT_DOUBLE_EQ(range->GetDouble("current"), 25);
}

/**
 * @tc.name: TreeRootSelectionAndCyclePruning
 * @tc.desc: Prefer the first explicit root, then an orphan, then prune cycles from the fallback root.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, TreeRootSelectionAndCyclePruning, TestSize.Level1)
{
    g_exportResult.nodes = { MakeExportNode(9, 9), MakeExportNode(8, 99),
        MakeExportNode(1, -1), MakeExportNode(2, -1) };
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetInt("$ID"), 1);
    EXPECT_FALSE(tree->Contains("$children"));

    g_exportResult.nodes = { MakeExportNode(9, 9), MakeExportNode(8, 99), MakeExportNode(2, 88) };
    tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetInt("$ID"), 8);

    g_exportResult.nodes = { MakeExportNode(1, 2), MakeExportNode(2, 1) };
    tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetInt("$ID"), 1);
    auto children = tree->GetValue("$children");
    ASSERT_EQ(children->GetArraySize(), 1);
    EXPECT_EQ(children->GetArrayItem(0)->GetInt("$ID"), 2);
    EXPECT_FALSE(children->GetArrayItem(0)->Contains("$children"));

    g_exportResult.nodes = { MakeExportNode(9, 9) };
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
}

/**
 * @tc.name: TreePrunesOnlyBeyondDepthLimit
 * @tc.desc: Keep all 64 permitted levels, prune level 65, and still export a sibling.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, TreePrunesOnlyBeyondDepthLimit, TestSize.Level1)
{
    g_exportResult.nodes.clear();
    for (int32_t id = 1; id <= 65; ++id) {
        g_exportResult.nodes.push_back(MakeExportNode(id, id == 1 ? -1 : id - 1));
    }
    g_exportResult.nodes.push_back(MakeExportNode(100, 1));
    auto tree = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_);
    ASSERT_NE(tree, nullptr);
    auto children = tree->GetValue("$children");
    ASSERT_EQ(children->GetArraySize(), 2);
    EXPECT_EQ(children->GetArrayItem(1)->GetInt("$ID"), 100);
    auto node = children->GetArrayItem(0);
    for (int32_t id = 2; id <= 64; ++id) {
        ASSERT_EQ(node->GetInt("$ID"), id);
        if (id == 64) {
            EXPECT_FALSE(node->Contains("$children"));
            break;
        }
        auto next = node->GetValue("$children");
        ASSERT_EQ(next->GetArraySize(), 1);
        node = next->GetArrayItem(0);
    }
}

/**
 * @tc.name: NodeLimitsPreserveBudget
 * @tc.desc: Reject oversized query results and accept exactly the remaining shared node allowance.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, NodeLimitsPreserveBudget, TestSize.Level1)
{
    g_exportResult.nodes.resize(EXPORT_NODE_LIMIT + 1);
    ExpectFlatFailure(ThirdProviderExportStatus::NODE_LIMIT);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
    g_exportResult.nodes = { MakeExportNode(1, -1) };
    ThirdProviderTreeExportBudget budget { EXPORT_NODE_LIMIT - 1, 0 };
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.nodes, EXPORT_NODE_LIMIT);
    ExpectFlatFailure(ThirdProviderExportStatus::NODE_LIMIT, budget);
    ExpectFlatFailure(ThirdProviderExportStatus::NODE_LIMIT, { EXPORT_NODE_LIMIT + 1, 0 });
}

/**
 * @tc.name: ByteLimitAcceptsExactFit
 * @tc.desc: Brackets, escaped strings and separators count once; a one-byte overflow is atomic.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, ByteLimitAcceptsExactFit, TestSize.Level1)
{
    g_exportResult.nodes = { MakeExportNode(1, -1, { 2 }), MakeExportNode(2, 1) };
    g_exportResult.nodes[1].SetContents("\"\\\n\x01");
    ThirdProviderTreeExportBudget measured;
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, measured);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    ASSERT_NE(result.nodes, nullptr);
    ASSERT_EQ(measured.bytes, result.nodes->ToString().size());
    ThirdProviderTreeExportBudget exact { 7, EXPORT_BYTE_LIMIT - measured.bytes };
    result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, exact);
    EXPECT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(exact.nodes, 9u);
    EXPECT_EQ(exact.bytes, EXPORT_BYTE_LIMIT);
    ExpectFlatFailure(ThirdProviderExportStatus::SIZE_LIMIT, { 7, EXPORT_BYTE_LIMIT - measured.bytes + 1 });
    ExpectFlatFailure(ThirdProviderExportStatus::SIZE_LIMIT, { 7, EXPORT_BYTE_LIMIT + 1 });
}

/**
 * @tc.name: EmptyArrayBudgetBoundary
 * @tc.desc: Empty exports need two bytes but no nodes, including at the node limit.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, EmptyArrayBudgetBoundary, TestSize.Level1)
{
    g_exportResult.nodes.clear();
    ThirdProviderTreeExportBudget budget { EXPORT_NODE_LIMIT, EXPORT_BYTE_LIMIT - 2 };
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host_, budget);
    EXPECT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.nodes, EXPORT_NODE_LIMIT);
    EXPECT_EQ(budget.bytes, EXPORT_BYTE_LIMIT);
    ExpectFlatFailure(ThirdProviderExportStatus::SIZE_LIMIT, { EXPORT_NODE_LIMIT, EXPORT_BYTE_LIMIT - 1 });
}

/**
 * @tc.name: InvalidSecondNodeRollsBackBudget
 * @tc.desc: A nonfinite value in a later node rejects the whole collection after the first node was counted.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, InvalidSecondNodeRollsBackBudget, TestSize.Level1)
{
    g_exportResult.nodes = { MakeExportNode(1, -1, { 2 }), MakeExportNode(2, 1) };
    g_exportResult.nodes[1].SetRangeInfo({ 0, 1, std::numeric_limits<double>::quiet_NaN() });
    ExpectFlatFailure(ThirdProviderExportStatus::COLLECTION_FAILED);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
}

/**
 * @tc.name: PasswordStillChargesVisibleMetadata
 * @tc.desc: Password filtering exempts contents only, not exported hints or other metadata.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, PasswordStillChargesVisibleMetadata, TestSize.Level1)
{
    g_exportResult.nodes[0].SetPassword(true);
    g_exportResult.nodes[0].SetContents("secret");
    g_exportResult.nodes[0].SetHintText(std::string(EXPORT_BYTE_LIMIT + 1, 'x'));
    ExpectFlatFailure(ThirdProviderExportStatus::SIZE_LIMIT);
    EXPECT_EQ(ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host_), nullptr);
}

/**
 * @tc.name: AccumulateOnlyAccessibilityArrays
 * @tc.desc: Nested extension arrays share the budget; ordinary inspector fields do not consume it.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, AccumulateOnlyAccessibilityArrays, TestSize.Level1)
{
    auto tree = JsonUtil::ParseJsonString(
        R"({"ignored":[1,2,3],"$children":[{"accessibilityTree":[{"contents":"a\n\"b"}]},)"
        R"({"wrapper":{"accessibilityTree":[{},{}]}}]})");
    ASSERT_TRUE(tree->IsValid());
    auto children = tree->GetValue("$children");
    const auto firstBytes = children->GetArrayItem(0)->GetValue("accessibilityTree")->ToString().size();
    const auto secondBytes = children->GetArrayItem(1)->GetValue("wrapper")
        ->GetValue("accessibilityTree")->ToString().size();
    ThirdProviderTreeExportBudget budget { 7, 123 };
    EXPECT_EQ(ThirdProviderTreeExporter::AccumulateInspectorTree(*tree, budget), ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.nodes, 10u);
    EXPECT_EQ(budget.bytes, 123u + firstBytes + secondBytes);
}

/**
 * @tc.name: AccumulateFailureRollsBackEarlierArrays
 * @tc.desc: Malformed or overflowing later arrays cannot partially charge an extension.
 * @tc.type: FUNC
 */
HWTEST_F(ThirdProviderTreeExportTest, AccumulateFailureRollsBackEarlierArrays, TestSize.Level1)
{
    auto invalid = JsonUtil::ParseJsonString(
        R"({"$children":[{"accessibilityTree":{}},{"accessibilityTree":[{}]}]})");
    auto valid = JsonUtil::ParseJsonString(
        R"({"$children":[{"accessibilityTree":[{}]},{"accessibilityTree":[{}]}]})");
    ThirdProviderTreeExportBudget budget { 7, 123 };
    EXPECT_EQ(ThirdProviderTreeExporter::AccumulateInspectorTree(*invalid, budget),
        ThirdProviderExportStatus::INVALID_DATA);
    EXPECT_EQ(budget.nodes, 7u);
    EXPECT_EQ(budget.bytes, 123u);
    budget = { EXPORT_NODE_LIMIT - 1, 123 };
    EXPECT_EQ(ThirdProviderTreeExporter::AccumulateInspectorTree(*valid, budget),
        ThirdProviderExportStatus::NODE_LIMIT);
    EXPECT_EQ(budget.nodes, EXPORT_NODE_LIMIT - 1);
    EXPECT_EQ(budget.bytes, 123u);
    budget = { 7, EXPORT_BYTE_LIMIT - 4 };
    EXPECT_EQ(ThirdProviderTreeExporter::AccumulateInspectorTree(*valid, budget),
        ThirdProviderExportStatus::SIZE_LIMIT);
    EXPECT_EQ(budget.nodes, 7u);
    EXPECT_EQ(budget.bytes, EXPORT_BYTE_LIMIT - 4);
}
} // namespace OHOS::Ace::NG

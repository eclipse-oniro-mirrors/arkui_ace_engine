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

#include "gtest/gtest.h"
#include <deque>
#include <limits>
#include <thread>

#include "core/pipeline/base/element_register.h"
#include "core/accessibility/native_interface_accessibility_impl.h"

#define private public
#define protected public

#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "accessibility_system_ability_client.h"
#include "frameworks/core/accessibility/accessibility_manager.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation.h"

#include "base/log/dump_log.h"
#include "frameworks/core/accessibility/accessibility_node.h"
#include "frameworks/core/accessibility/accessibility_utils.h"
#include "frameworks/core/components_ng/property/accessibility_property.h"
#include "frameworks/core/accessibility/native_interface_accessibility_provider.h"
#include "frameworks/core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#include "js_third_provider_interaction_operation_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
namespace {
    const int32_t IGNORE_POSITION_TRANSITION_SWITCH = -990;
} // namespace

class JsThirdAccessibilityHoverNgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void JsThirdAccessibilityHoverNgTest::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    auto context = NG::PipelineContext::GetCurrentContext();
    context->instanceId_ = IGNORE_POSITION_TRANSITION_SWITCH;
    std::unique_ptr<std::ostream> ostream = std::make_unique<std::ostringstream>();
    ASSERT_NE(ostream, nullptr);
    DumpLog::GetInstance().SetDumpFile(std::move(ostream));
}

void JsThirdAccessibilityHoverNgTest::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest001
 * @tc.desc: ClearFocusFromProvider
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest001, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);

    bool ret;

    int64_t elementId = 0;
    Accessibility::AccessibilityElementInfo info;
    int64_t hostElementId = 0;
    // 1 jsThirdProviderOperator not registered
    ret = jsAccessibilityManager->GetElementInfoForThird(elementId, info, hostElementId);
    EXPECT_EQ(ret, false);

    jsAccessibilityManager->RegisterJsThirdProviderInteractionOperation(hostElementId, jsInteractionOperation);
    // 2 FindAccessibilityNodeInfosByIdFromProvider not ok
    ohAccessibilityProvider->SetInjectResult(-1);
    ret = jsAccessibilityManager->GetElementInfoForThird(elementId, info, hostElementId);
    EXPECT_EQ(ret, false);

    // 3 FindAccessibilityNodeInfosByIdFromProvider ok, info equals to mock element info
    ohAccessibilityProvider->providerMockResult_.Reset();
    ret = jsAccessibilityManager->GetElementInfoForThird(elementId, info, hostElementId);
    EXPECT_EQ(ret, true);

    EXPECT_EQ(info.GetAccessibilityId(),
        ohAccessibilityProvider->providerMockResult_.elementInfosList_[elementId].GetElementId());
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest001
 * @tc.desc: ClearFocusFromProvider
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest002, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);

    std::string hitTestMode = "HitTestMode.Block";
    bool shouldSearchSelf = true;
    bool shouldSearchChildren = true;
    // 1 HitTestMode.Block
    jsAccessibilityManager->UpdateSearchStrategyByHitTestModeStr(hitTestMode, shouldSearchSelf, shouldSearchChildren);
    EXPECT_EQ(shouldSearchSelf, true);
    EXPECT_EQ(shouldSearchChildren, false);

    // 2 HitTestMode.None
    hitTestMode = "HitTestMode.None";
    shouldSearchSelf = true;
    shouldSearchChildren = true;
    jsAccessibilityManager->UpdateSearchStrategyByHitTestModeStr(hitTestMode, shouldSearchSelf, shouldSearchChildren);
    EXPECT_EQ(shouldSearchSelf, false);
    EXPECT_EQ(shouldSearchChildren, true);

    // 3 HitTestMode.Default
    hitTestMode = "HitTestMode.Default";
    shouldSearchSelf = true;
    shouldSearchChildren = true;
    jsAccessibilityManager->UpdateSearchStrategyByHitTestModeStr(hitTestMode, shouldSearchSelf, shouldSearchChildren);
    EXPECT_EQ(shouldSearchSelf, true);
    EXPECT_EQ(shouldSearchChildren, true);
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest003
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest003, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);

    bool ret;
    Accessibility::AccessibilityElementInfo nodeInfo;

    // level yes
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::YES_STR);
    ret = jsAccessibilityManager->IsAccessibilityFocusable(nodeInfo);
    EXPECT_EQ(ret, true);

    // level no
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::NO_STR);
    ret = jsAccessibilityManager->IsAccessibilityFocusable(nodeInfo);
    EXPECT_EQ(ret, false);
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::AUTO);

    // group true
    nodeInfo.SetAccessibilityGroup(true);
    ret = jsAccessibilityManager->IsAccessibilityFocusable(nodeInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest004
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest004, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);

    bool ret;
    Accessibility::AccessibilityElementInfo nodeInfo;

    bool shouldSearchSelf;
    bool shouldSearchChildren;
    std::pair<bool, bool> result;
    // level yes
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::YES_STR);
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, true);
    EXPECT_EQ(shouldSearchChildren, true);

    // level NO_HIDE_DESCENDANTS
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS);
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, false);
    EXPECT_EQ(shouldSearchChildren, false);

    // level NO  with AccessibilityText
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::NO_STR);
    nodeInfo.SetAccessibilityText("test");
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, false);
    EXPECT_EQ(shouldSearchChildren, true);

    // level NO
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::NO_STR);
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, false);
    EXPECT_EQ(shouldSearchChildren, true);
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest004
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest0041, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);

    bool ret;
    Accessibility::AccessibilityElementInfo nodeInfo;

    bool shouldSearchSelf;
    bool shouldSearchChildren;
    std::pair<bool, bool> result;

    // level auto and not accessibility text and hit test mode default
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::AUTO);
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, true);
    EXPECT_EQ(shouldSearchChildren, true);

    // level auto and has accessibility text and hit test mode none
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::AUTO);
    nodeInfo.SetAccessibilityText("test");
    nodeInfo.SetHitTestBehavior("HitTestMode.None");
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, true);
    EXPECT_EQ(shouldSearchChildren, true);


    // level auto and has accessibility text and hit test mode default
    nodeInfo.SetAccessibilityLevel(NG::AccessibilityProperty::Level::AUTO);
    nodeInfo.SetAccessibilityText("");
    nodeInfo.SetHitTestBehavior("HitTestMode.None");
    result = jsAccessibilityManager->GetSearchStrategyForThird(nodeInfo);
    shouldSearchSelf = result.first;
    shouldSearchChildren = result.second;
    EXPECT_EQ(shouldSearchSelf, false);
    EXPECT_EQ(shouldSearchChildren, true);
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest004
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest005, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);
    int64_t hostElementId = 0;
    jsAccessibilityManager->RegisterJsThirdProviderInteractionOperation(hostElementId, jsInteractionOperation);

    NG::PointF point(1, 1);
    auto elementInfoListSize = ohAccessibilityProvider->providerMockResult_.elementInfosList_.size();
    for (int32_t i = 0; i < elementInfoListSize; i++) {
        auto leftTopX = 0;
        auto leftTopY = 0;
        auto rightBottomX = 100;
        auto rightBottomY = 100;
        ArkUI_AccessibleRect screenRect {leftTopX, leftTopY, rightBottomX, rightBottomY};
        ohAccessibilityProvider->providerMockResult_.elementInfosList_[i].SetRect(screenRect);
    }
    Accessibility::AccessibilityElementInfo rootInfo;

    auto ret = jsAccessibilityManager->GetElementInfoForThird(-1, rootInfo, hostElementId);
    EXPECT_EQ(ret, true);
    auto path = jsAccessibilityManager->HoverPathForThird(hostElementId, point, rootInfo, {});
    EXPECT_EQ(path.size(), elementInfoListSize);
}

/**
 * @tc.name: JsThirdAccessibilityHoverNgTest004
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest006, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);
    int64_t hostElementId = 0;
    jsAccessibilityManager->RegisterJsThirdProviderInteractionOperation(hostElementId, jsInteractionOperation);

    bool ret;

    auto elementInfoListSize = ohAccessibilityProvider->providerMockResult_.elementInfosList_.size();
    for (int32_t i = 0; i < elementInfoListSize; i++) {
        auto leftTopX = 0;
        auto leftTopY = 0;
        auto rightBottomX = 100;
        auto rightBottomY = 100;
        ArkUI_AccessibleRect screenRect {leftTopX, leftTopY, rightBottomX, rightBottomY};
        ohAccessibilityProvider->providerMockResult_.elementInfosList_[i].SetRect(screenRect);
    }

    NG::PointF point(0, 0);
    SourceType sourceType = SourceType::NONE;
    NG::AccessibilityHoverEventType eventType = NG::AccessibilityHoverEventType::ENTER;
    TimeStamp time = std::chrono::high_resolution_clock::now();
    Framework::AccessibilityHoverForThirdConfig config
        {hostElementId, point, sourceType, eventType, time, context};

    jsAccessibilityManager->HandleAccessibilityHoverForThird(config);
    // tbm  mock client to check event;
    EXPECT_NE(frameNode, nullptr);
}


/**
 * @tc.name: JsThirdAccessibilityHoverNgTest007
 * @tc.desc: IsAccessibilityFocusable
 * @tc.type: FUNC
 */
HWTEST_F(JsThirdAccessibilityHoverNgTest, JsThirdAccessibilityHoverNgTest007, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto frameNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<Pattern>(), true);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    auto context = NG::PipelineContext::GetCurrentContext();
    jsAccessibilityManager->SetPipelineContext(context);
    jsAccessibilityManager->Register(true);

    auto jsInteractionOperation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(
        ohAccessibilityProvider, jsAccessibilityManager, frameNode);
    jsInteractionOperation->SetBelongTreeId(0);
    int64_t hostElementId = 0;
    jsAccessibilityManager->RegisterJsThirdProviderInteractionOperation(hostElementId, jsInteractionOperation);

    std::vector<std::string> info;
    // 1. dump property
    std::vector<std::string> params;
    params.push_back("-inspector");
    params.push_back("1");

    jsAccessibilityManager->OnDumpChildInfoForThirdRecursive(hostElementId, params, info, jsAccessibilityManager);
    // tbm  make sure check method;
    EXPECT_NE(frameNode, nullptr);
    // 2. dump tree
    params.clear();
    params.push_back("-inspector");
    params.push_back("--root");
    params.push_back("1");
    params.push_back("2");

    jsAccessibilityManager->OnDumpChildInfoForThirdRecursive(hostElementId, params, info, jsAccessibilityManager);
    // tbm  make sure check method;
    EXPECT_NE(frameNode, nullptr);
}

namespace {
class NativeHoverTaskExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&& task, TaskType, uint32_t, const std::string& name,
        PriorityType = PriorityType::LOW,
        VsyncBarrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (name == "ArkUIHandleAccessibilityHoverForThird") {
            hoverTasks.emplace_back(std::move(task));
        } else if (name == "SetSearchElementInfoByTextResult") {
            ++eventCount;
        }
        return true;
    }
    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return type == TaskType::UI && onUiThread;
    }
    void DrainHoverTasks()
    {
        while (!hoverTasks.empty()) {
            auto task = std::move(hoverTasks.front());
            hoverTasks.pop_front();
            task();
        }
    }
    bool onUiThread = true;
    mutable size_t eventCount = 0;
    mutable std::deque<Task> hoverTasks;
};

struct NativeHoverCallbackData {
    int64_t id = 100;
    int32_t calls = 0;
    int32_t x = -1;
    int32_t y = -1;
    bool fill = true;
    RefPtr<FrameNode> unregisterHost;
    std::thread::id callbackThread;
};

void FillNativeHoverResult(void* userData, int32_t x, int32_t y, int32_t requestId,
    ArkUI_AccessibilityElementInfo* info)
{
    auto& data = *static_cast<NativeHoverCallbackData*>(userData);
    ++data.calls;
    data.x = x;
    data.y = y;
    data.callbackThread = std::this_thread::get_id();
    EXPECT_GE(requestId, 0);
    if (data.fill) {
        info->SetElementId(data.id);
        info->SetComponentType("Button");
        info->SetContents("native result");
    }
    if (data.unregisterHost) {
        NativeAccessibilityHoverRegistry::GetInstance().Unregister(data.unregisterHost);
    }
}

struct NativeHoverScenario {
    NativeHoverScenario()
    {
        context = PipelineContext::GetCurrentContext();
        previousExecutor = context->GetTaskExecutor();
        executor = AceType::MakeRefPtr<NativeHoverTaskExecutor>();
        context->taskExecutor_ = executor;
        manager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
        manager->SetPipelineContext(context);
        host = FrameNode::CreateFrameNode("nativeHost", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>(), true);
        provider = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
        operation = std::make_shared<Framework::JsThirdProviderInteractionOperation>(provider, manager, host);
        operation->SetBelongTreeId(7);
        manager->RegisterJsThirdProviderInteractionOperation(host->GetAccessibilityId(), operation);
        EXPECT_EQ(NativeAccessibilityHoverRegistry::GetInstance().Register(host, &data, FillNativeHoverResult), 0);
    }
    ~NativeHoverScenario()
    {
        NativeAccessibilityHoverRegistry::GetInstance().Unregister(host);
        context->taskExecutor_ = previousExecutor;
    }
    void Transfer(AccessibilityHoverEventType type)
    {
        manager->TransferThirdProviderHoverEvent(host, PointF(25, 24), SourceType::TOUCH, type, TimeStamp());
    }
    void FailLegacyQuery()
    {
        provider->providerMockResult_.elementId_ = 999;
        provider->SetInjectResult(-1);
    }
    RefPtr<PipelineContext> context;
    RefPtr<TaskExecutor> previousExecutor;
    RefPtr<NativeHoverTaskExecutor> executor;
    RefPtr<Framework::JsAccessibilityManager> manager;
    RefPtr<FrameNode> host;
    RefPtr<MockOhAccessibilityProvider> provider;
    std::shared_ptr<Framework::JsThirdProviderInteractionOperation> operation;
    NativeHoverCallbackData data;
};
} // namespace

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeCallbackRunsBeforeBackgroundTransfer, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.provider->providerMockResult_.elementId_ = 999;
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    EXPECT_EQ(scenario.data.calls, 1);
    EXPECT_EQ(scenario.data.callbackThread, std::this_thread::get_id());
    EXPECT_EQ(scenario.data.x, 25);
    EXPECT_EQ(scenario.data.y, 24);
    ASSERT_EQ(scenario.executor->hoverTasks.size(), 1);
    EXPECT_TRUE(scenario.manager->hoverForThirdState_.nodesHovering.empty());
    scenario.data.id = 200; // An already queued result is an owned snapshot.
    scenario.executor->DrainHoverTasks();
    ASSERT_TRUE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.manager->hoverForThirdState_.nodesHovering, std::vector<int64_t>({100}));
    EXPECT_EQ(scenario.provider->providerMockResult_.elementId_, 999); // No provider tree query.
    EXPECT_EQ(scenario.executor->eventCount, 1);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeTargetSwitchAndExitUseExistingThirdState, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    scenario.Transfer(AccessibilityHoverEventType::MOVE);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.executor->eventCount, 1);
    scenario.data.id = 200;
    scenario.Transfer(AccessibilityHoverEventType::MOVE);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.executor->eventCount, 3); // EXIT old, ENTER new.
    EXPECT_EQ(scenario.manager->hoverForThirdState_.nodesHovering, std::vector<int64_t>({200}));
    scenario.Transfer(AccessibilityHoverEventType::EXIT);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.executor->eventCount, 4);
    EXPECT_TRUE(scenario.manager->hoverForThirdState_.nodesHovering.empty());
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.data.calls, 3); // EXIT never invokes the hit-test callback.
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeEmptyResultFallsBackToProviderTree, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.data.fill = false;
    scenario.FailLegacyQuery();
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.data.calls, 1);
    EXPECT_EQ(scenario.provider->providerMockResult_.elementId_, -1);
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.executor->eventCount, 0);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeReentrantUnregisterDiscardsResult, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.data.unregisterHost = scenario.host;
    scenario.FailLegacyQuery();
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.data.calls, 1);
    EXPECT_FALSE(NativeAccessibilityHoverRegistry::GetInstance().HasHandler(scenario.host));
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.executor->eventCount, 0);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeQueuedResultIsRejectedAfterUnregister, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.FailLegacyQuery();
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    NativeAccessibilityHoverRegistry::GetInstance().Unregister(scenario.host);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.provider->providerMockResult_.elementId_, -1);
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.executor->eventCount, 0);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeSameIdReplacementStartsNewTarget, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    auto generation = scenario.manager->hoverForThirdState_.nativeHoverGeneration;
    NativeAccessibilityHoverRegistry::GetInstance().Register(scenario.host, &scenario.data, FillNativeHoverResult);
    scenario.Transfer(AccessibilityHoverEventType::MOVE);
    scenario.executor->DrainHoverTasks();
    EXPECT_NE(scenario.manager->hoverForThirdState_.nativeHoverGeneration, generation);
    EXPECT_EQ(scenario.executor->eventCount, 3);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeUnregisterRestoresProviderTraversal, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    NativeAccessibilityHoverRegistry::GetInstance().Unregister(scenario.host);
    scenario.FailLegacyQuery();
    scenario.Transfer(AccessibilityHoverEventType::MOVE);
    scenario.executor->DrainHoverTasks();
    EXPECT_EQ(scenario.data.calls, 1);
    EXPECT_EQ(scenario.provider->providerMockResult_.elementId_, -1);
    EXPECT_EQ(scenario.executor->eventCount, 2); // Includes EXIT from the saved native snapshot.
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeMissingProviderDoesNotKeepTarget, TestSize.Level1)
{
    NativeHoverScenario scenario;
    scenario.manager->DeregisterJsThirdProviderInteractionOperation(scenario.host->GetAccessibilityId());
    scenario.Transfer(AccessibilityHoverEventType::ENTER);
    scenario.executor->DrainHoverTasks();
    EXPECT_TRUE(scenario.manager->hoverForThirdState_.nodesHovering.empty());
    EXPECT_FALSE(scenario.manager->hoverForThirdState_.nativeHoverInfo);
    EXPECT_EQ(scenario.executor->eventCount, 0);
}

HWTEST_F(JsThirdAccessibilityHoverNgTest, NativeInvalidCoordinatesAndBackgroundNeverCallHandler, TestSize.Level1)
{
    NativeHoverScenario scenario;
    Framework::AccessibilityHoverForThirdConfig config;
    config.nativeHoverHost = scenario.host;
    config.context = scenario.context;
    for (float x : { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
        static_cast<float>(std::numeric_limits<int32_t>::max()) }) {
        config.point = PointF(x, 1);
        scenario.manager->PrepareNativeHoverResult(config);
        EXPECT_FALSE(config.nativeHoverInfo);
    }
    config.point = PointF(1, 2);
    scenario.executor->onUiThread = false;
    scenario.manager->PrepareNativeHoverResult(config);
    EXPECT_FALSE(config.nativeHoverInfo);
    EXPECT_EQ(scenario.data.calls, 0);
}

} // namespace OHOS::Ace::NG
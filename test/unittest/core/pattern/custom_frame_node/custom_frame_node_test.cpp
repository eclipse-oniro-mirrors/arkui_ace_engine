/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

#include "core/components_ng/pattern/pattern.h"

#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/accessibility/native_interface_accessibility_provider.h"
#include "core/components_ng/pattern/custom_frame_node/custom_accessibility_child_tree_callback.h"
#include "core/components_ng/pattern/custom_frame_node/custom_accessibility_provider.h"
#include "core/components_ng/pattern/custom_frame_node/custom_frame_node.h"
#include "core/components_ng/pattern/custom_frame_node/custom_frame_node_layout_algorithm.h"
#include "core/components_ng/pattern/custom_frame_node/custom_frame_node_pattern.h"
#include "core/components_ng/pattern/custom_frame_node/custom_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string CHILD_NODE = "ChildNode";

class QueuedAccessibilityTaskExecutor : public MockTaskExecutor {
    DECLARE_ACE_TYPE(QueuedAccessibilityTaskExecutor, MockTaskExecutor);

public:
    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return type == TaskType::UI && std::this_thread::get_id() == uiThread_;
    }

    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name, PriorityType priorityType,
        VsyncBarrierOption barrierOption) const override
    {
        EXPECT_EQ(type, TaskType::UI);
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push_back(std::move(task));
        return true;
    }

    void RunTasks()
    {
        std::vector<Task> tasks;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks.swap(tasks_);
        }
        for (auto& task : tasks) {
            task();
        }
    }

private:
    const std::thread::id uiThread_ = std::this_thread::get_id();
    mutable std::mutex mutex_;
    mutable std::vector<Task> tasks_;
};

class RecordingAccessibilityChildTreeCallback : public CustomAccessibilityChildTreeCallback {
public:
    explicit RecordingAccessibilityChildTreeCallback(const WeakPtr<CustomPattern>& pattern)
        : CustomAccessibilityChildTreeCallback(pattern, 1)
    {}

    bool OnDeregister() override
    {
        ++calls;
        calledOn = std::this_thread::get_id();
        return true;
    }

    int32_t calls = 0;
    std::thread::id calledOn;
};

class EventThirdAccessibilityManager : public ThirdAccessibilityManager {
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
} // namespace

class CustomFrameNodeTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    static RefPtr<FrameNode> CreateNode();
};

void CustomFrameNodeTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void CustomFrameNodeTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

RefPtr<FrameNode> CustomFrameNodeTestNg::CreateNode()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    return CustomFrameNode::GetOrCreateCustomFrameNode(nodeId);
}

/**
 * @tc.name: CustomFrameNodeFrameNodeCreator001
 * @tc.desc: Test the create of CustomFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, CustomFrameNodeFrameNodeCreator001, TestSize.Level1)
{
    RefPtr<FrameNode> customFrameNode = CreateNode();
    ASSERT_NE(customFrameNode, nullptr);
}

/**
 * @tc.name: CustomFrameNodeRenderNodeModifierCreate001
 * @tc.desc: Test creation of renderNodeModifier_ in CustomFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, CustomFrameNodeRenderNodeModifierCreate001, TestSize.Level1)
{
    RefPtr<FrameNode> customFrameNode = CreateNode();
    ASSERT_NE(customFrameNode, nullptr);
    RefPtr<FrameNode> childNode_0 = FrameNode::CreateFrameNode(CHILD_NODE, 0, AceType::MakeRefPtr<Pattern>());
    auto pattern = customFrameNode->GetPattern<CustomFrameNodePattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1.Create RenderNodeModifier by CreateNodePaintMethod.
     * @tc.expected: renderNodeModifier_ in pattern is nullptr, it created by fire CreateNodePaintMethod function.
     */
    ASSERT_EQ(pattern->renderNodeModifier_, nullptr);
    pattern->CreateNodePaintMethod();
    ASSERT_NE(pattern->renderNodeModifier_, nullptr);

    /**
     * @tc.steps: step2. Create RenderNodeModifier by SetDrawCallback.
     * @tc.expected: renderNodeModifier_ in pattern is nullptr.
     */
    pattern->renderNodeModifier_ = nullptr;
    ASSERT_EQ(pattern->renderNodeModifier_, nullptr);
    pattern->SetDrawCallback(nullptr);
    ASSERT_NE(pattern->renderNodeModifier_, nullptr);
}

/**
 * @tc.name: CustomFrameNodeRenderNodeInvalidate001
 * @tc.desc: Test Invalidate function in CustomFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, CustomFrameNodeRenderNodeInvalidate001, TestSize.Level1)
{
    RefPtr<FrameNode> customFrameNode = CreateNode();
    ASSERT_NE(customFrameNode, nullptr);
    RefPtr<FrameNode> childNode_0 = FrameNode::CreateFrameNode(CHILD_NODE, 0, AceType::MakeRefPtr<Pattern>());
    auto pattern = customFrameNode->GetPattern<CustomFrameNodePattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. Call Invalidate without renderNodeModifier_.
     * @tc.expected: Process success without crash.
     */
    ASSERT_EQ(pattern->renderNodeModifier_, nullptr);
    pattern->Invalidate();

    /**
     * @tc.steps: step2.Create RenderNodeModifier by CreateNodePaintMethod.
     * @tc.expected: renderNodeModifier_ in pattern is nullptr, it created by fire CreateNodePaintMethod function.
     */
    pattern->CreateNodePaintMethod();
    ASSERT_NE(pattern->renderNodeModifier_, nullptr);

    /**
     * @tc.steps: step3. Create RenderNodeModifier by SetDrawCallback.
     * @tc.expected: renderNodeModifier_ in pattern is nullptr.
     */
    ASSERT_EQ(pattern->renderNodeModifier_->renderNodeFlag_->Get(), 0);
    pattern->Invalidate();
    ASSERT_EQ(pattern->renderNodeModifier_->renderNodeFlag_->Get(), 1);
}

/**
 * @tc.name: OnRegisterTest
 * @tc.desc: Test OnRegister Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnRegisterTest001, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityChildTreeCallback
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_NE(pattern, nullptr);
    int accessibilityId = 2;
    auto customAccessibilityChildTreeCallback =
        std::make_shared<CustomAccessibilityChildTreeCallback>(pattern, accessibilityId);
    ASSERT_TRUE(customAccessibilityChildTreeCallback);
    /**
     * @tc.step2: Call OnRegister Func when nativeProvider->IsRegister() is false
     * @tc.expected: ret = false and xComponentAccessibilityChildTreeCallback->isReg_ = false
     */
    uint32_t windowId = 1;
    int32_t treeId = 2;
    bool ret = customAccessibilityChildTreeCallback->OnRegister(windowId, treeId);
    EXPECT_FALSE(ret);
    EXPECT_FALSE(customAccessibilityChildTreeCallback->isReg_);
}

/**
 * @tc.name: OnDeregisterTest
 * @tc.desc: Test OnDeregister Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnDeregisterTest002, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityChildTreeCallback
     * @tc.expected: Create Custom and CustomAccessibilityChildTreeCallback Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_NE(pattern, nullptr);
    int accessibilityId = 2;
    auto customAccessibilityChildTreeCallback =
        std::make_shared<CustomAccessibilityChildTreeCallback>(pattern, accessibilityId);
    ASSERT_TRUE(customAccessibilityChildTreeCallback);
    /**
     * @tc.step2: Call OnDeregister Func
     * @tc.expected: ret = false and customAccessibilityChildTreeCallback->isReg_ = false
     */
    bool ret = customAccessibilityChildTreeCallback->OnDeregister();
    EXPECT_TRUE(ret);
    EXPECT_FALSE(customAccessibilityChildTreeCallback->isReg_);
}

/**
 * @tc.name: OnSetChildTreeTest
 * @tc.desc: Test OnSetChildTree Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnSetChildTreeTest001, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityChildTreeCallback
     * @tc.expected: Create Custom and CustomAccessibilityChildTreeCallback Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    int accessibilityId = 2;
    auto customAccessibilityChildTreeCallback =
        std::make_shared<CustomAccessibilityChildTreeCallback>(pattern, accessibilityId);
    ASSERT_TRUE(customAccessibilityChildTreeCallback);
    /**
     * @tc.step2: Call OnDeregister Func
     * @tc.expected: ret = true
     */
    int32_t childWindowId = 1;
    int32_t childTreeId = 2;
    bool ret = customAccessibilityChildTreeCallback->OnSetChildTree(childWindowId, childTreeId);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnClearRegisterFlagTest
 * @tc.desc: Test OnClearRegisterFlag Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnClearRegisterFlagTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityChildTreeCallback
     * @tc.expected: Create Custom and CustomAccessibilityChildTreeCallback Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    int accessibilityId = 2;
    auto customAccessibilityChildTreeCallback =
        std::make_shared<CustomAccessibilityChildTreeCallback>(pattern, accessibilityId);
    ASSERT_TRUE(customAccessibilityChildTreeCallback);
    /**
     * @tc.step2: Call OnClearRegisterFlag Func
     * @tc.expected: isReg_ = false;
     */
    customAccessibilityChildTreeCallback->OnClearRegisterFlag();
    EXPECT_FALSE(customAccessibilityChildTreeCallback->isReg_);
}

/**
 * @tc.name: OnAccessibilityChildTreeDeregisterTest001
 * @tc.desc: Test InitNativeNodeCallbacks Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnAccessibilityChildTreeDeregisterTest001, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    /**
     * @tc.steps2: GetNativeProvider is null ptr
     * @tc.expected: OnAccessibilityChildTreeDeregister return false
     */
    auto result = pattern->OnAccessibilityChildTreeDeregister(AceType::RawPtr(testNode));
    ASSERT_FALSE(result);
    result = pattern->OnAccessibilityChildTreeDeregister();
    ASSERT_FALSE(result);
}

/**
 * @tc.name: FindAccessibilityNodeInfosByIdTest
 * @tc.desc: Test FindAccessibilityNodeInfosById Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, FindAccessibilityNodeInfosByIdTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call FindAccessibilityNodeInfosById Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    int32_t mode = 2;
    int32_t requestId = 3;
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    int32_t ret = customAccessibilityProvider->FindAccessibilityNodeInfosById(elementId, mode, requestId, infos);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: FindAccessibilityNodeInfosByTextTest
 * @tc.desc: Test FindAccessibilityNodeInfosByText Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, FindAccessibilityNodeInfosByTextTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call FindAccessibilityNodeInfosByText Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    string text = "2";
    int32_t requestId = 3;
    std::vector<ArkUI_AccessibilityElementInfo> infos;
    int32_t ret = customAccessibilityProvider->FindAccessibilityNodeInfosByText(elementId, text, requestId, infos);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: FindFocusedAccessibilityNodeTest
 * @tc.desc: Test FindFocusedAccessibilityNode Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, FindFocusedAccessibilityNodeTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call FindFocusedAccessibilityNode Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    int32_t focusType = 2;
    int32_t requestId = 3;
    ArkUI_AccessibilityElementInfo info;
    int32_t ret = customAccessibilityProvider->FindFocusedAccessibilityNode(elementId, focusType, requestId, info);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: FindNextFocusAccessibilityNodeTest
 * @tc.desc: Test FindNextFocusAccessibilityNode Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, FindNextFocusAccessibilityNodeTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call FindNextFocusAccessibilityNode Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    int32_t direction = 2;
    int32_t requestId = 3;
    ArkUI_AccessibilityElementInfo info;
    int32_t ret =
        customAccessibilityProvider->FindNextFocusAccessibilityNode(elementId, direction, requestId, info);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ExecuteAccessibilityActionTest
 * @tc.desc: Test ExecuteAccessibilityAction Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, ExecuteAccessibilityActionTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call ExecuteAccessibilityAction Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    int32_t action = 2;
    int32_t requestId = 3;
    std::map<std::string, std::string> actionArguments;
    int32_t ret =
        customAccessibilityProvider->ExecuteAccessibilityAction(elementId, action, requestId, actionArguments);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ClearFocusedAccessibilityNodeTest
 * @tc.desc: Test ClearFocusedAccessibilityNode Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, ClearFocusedAccessibilityNodeTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call ClearFocusedAccessibilityNode Func
     * @tc.expected: ret = 0
     */
    int32_t ret = customAccessibilityProvider->ClearFocusedAccessibilityNode();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetAccessibilityNodeCursorPositionTest
 * @tc.desc: Test GetAccessibilityNodeCursorPosition Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, GetAccessibilityNodeCursorPositionTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call GetAccessibilityNodeCursorPosition Func
     * @tc.expected: ret = 0
     */
    int64_t elementId = 1;
    int32_t requestId = 2;
    int32_t cursorPosition = 3;
    int32_t ret =
        customAccessibilityProvider->GetAccessibilityNodeCursorPosition(elementId, requestId, cursorPosition);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: SendAccessibilityAsyncEventTest1
 * @tc.desc: Test SendAccessibilityAsyncEvent Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, SendAccessibilityAsyncEventTest1, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call SendAccessibilityAsyncEvent Func when callback is not null and thirdAccessibilityManager_ is not
     * null
     * @tc.expected: ret = 0
     */
    ArkUI_AccessibilityEventInfo accessibilityEvent;
    auto thirdAccessibilityManager = std::make_shared<ThirdAccessibilityManager>();
    customAccessibilityProvider->thirdAccessibilityManager_ = thirdAccessibilityManager;
    auto callback = [](int32_t errorCode) {};
    int32_t ret = customAccessibilityProvider->SendAccessibilityAsyncEvent(accessibilityEvent, callback);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: SendAccessibilityAsyncEventTest2
 * @tc.desc: Test SendAccessibilityAsyncEvent Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, SendAccessibilityAsyncEventTest2, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call SendAccessibilityAsyncEvent Func when callback is null and thirdAccessibilityManager_ is not null
     * @tc.expected: ret = 0
     */
    ArkUI_AccessibilityEventInfo accessibilityEvent;
    auto thirdAccessibilityManager = std::make_shared<ThirdAccessibilityManager>();
    customAccessibilityProvider->thirdAccessibilityManager_ = thirdAccessibilityManager;
    int32_t ret = customAccessibilityProvider->SendAccessibilityAsyncEvent(accessibilityEvent, nullptr);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: SendAccessibilityAsyncEventTest3
 * @tc.desc: Test SendAccessibilityAsyncEvent Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, SendAccessibilityAsyncEventTest3, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);

    /**
     * @tc.step2: Call SendAccessibilityAsyncEvent Func when callback is not null and thirdAccessibilityManager_ is null
     * @tc.expected: ret = -1
     */
    ArkUI_AccessibilityEventInfo accessibilityEvent;
    customAccessibilityProvider->thirdAccessibilityManager_ = std::shared_ptr<ThirdAccessibilityManager>();
    auto callback = [](int32_t errorCode) {};
    int32_t ret = customAccessibilityProvider->SendAccessibilityAsyncEvent(accessibilityEvent, callback);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: OnDetachContextTest
 * @tc.desc: Test OnDetachContext Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnDetachContextTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom
     * @tc.expected: Create Custom Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    PipelineContext* context = testNode->GetContext();
    ASSERT_TRUE(context);
    /**
     * @tc.step2: Call OnDetachContext Func
     * @tc.expected: onWindowStateChangedCallbacks_ can not find nodeId by the frameNode
     */
    pattern->OnDetachContext(context);
    EXPECT_EQ(context->onWindowStateChangedCallbacks_.find(testNode->GetId()),
        context->onWindowStateChangedCallbacks_.end());
}

/**
 * @tc.name: InitializeAccessibility
 * @tc.desc: Test InitializeAccessibility Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, InitializeAccessibilityTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom
     * @tc.expected: Create Custom Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    PipelineContext* context = testNode->GetContext();
    ASSERT_TRUE(context);
    /**
     * @tc.step2: Call InitializeAccessibility Func
     */
    pattern->InitializeAccessibility(context);
    auto nprovider = pattern->GetNativeAccessibilityProvider();
    ASSERT_TRUE(nprovider);
}

/**
 * @tc.name: AccessibilityRegistrationOnUiThread
 * @tc.desc: Worker notifications run on UI; UI notifications retain synchronous behavior.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, AccessibilityRegistrationOnUiThread, TestSize.Level1)
{
    auto context = MockPipelineContext::GetCurrent();
    auto previousExecutor = context->GetTaskExecutor();
    auto executor = AceType::MakeRefPtr<QueuedAccessibilityTaskExecutor>();
    context->SetTaskExecutor(executor);
    auto node = FrameNode::CreateFrameNode("custom", 1, AceType::MakeRefPtr<CustomPattern>());
    auto pattern = node->GetPattern<CustomPattern>();
    auto callback = std::make_shared<RecordingAccessibilityChildTreeCallback>(pattern);
    pattern->accessibilityChildTreeCallback_ = callback;
    pattern->InitializeAccessibilityCallback();
    auto nativeProvider = pattern->GetNativeAccessibilityProvider();
    ArkUI_AccessibilityProviderCallbacks callbacks {};

    std::thread worker([&] { nativeProvider->AccessibilityProviderRegisterCallback(&callbacks); });
    worker.join();
    EXPECT_EQ(callback->calls, 0);
    executor->RunTasks();
    EXPECT_EQ(callback->calls, 1);
    EXPECT_EQ(callback->calledOn, std::this_thread::get_id());

    nativeProvider->AccessibilityProviderRegisterCallback(&callbacks);
    EXPECT_EQ(callback->calls, 2);
    executor->RunTasks();
    EXPECT_EQ(callback->calls, 2);
    context->SetTaskExecutor(previousExecutor);
}

/**
 * @tc.name: AccessibilityRegistrationAfterDetach
 * @tc.desc: Notifications queued before detach do not call the detached callback.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, AccessibilityRegistrationAfterDetach, TestSize.Level1)
{
    auto context = MockPipelineContext::GetCurrent();
    auto previousExecutor = context->GetTaskExecutor();
    auto executor = AceType::MakeRefPtr<QueuedAccessibilityTaskExecutor>();
    context->SetTaskExecutor(executor);
    auto node = FrameNode::CreateFrameNode("custom", 1, AceType::MakeRefPtr<CustomPattern>());
    auto pattern = node->GetPattern<CustomPattern>();
    auto callback = std::make_shared<RecordingAccessibilityChildTreeCallback>(pattern);
    pattern->accessibilityChildTreeCallback_ = callback;
    pattern->InitializeAccessibilityCallback();
    auto nativeProvider = pattern->GetNativeAccessibilityProvider();
    ArkUI_AccessibilityProviderCallbacks callbacks {};

    std::thread worker([&] { nativeProvider->AccessibilityProviderRegisterCallback(&callbacks); });
    worker.join();
    pattern->UninitializeAccessibility(node.GetRawPtr(), context.GetRawPtr());
    EXPECT_EQ(pattern->accessibilityChildTreeCallback_, nullptr);
    executor->RunTasks();
    EXPECT_EQ(callback->calls, 0);
    context->SetTaskExecutor(previousExecutor);
}

/**
 * @tc.name: AccessibilityRegistrationAfterReattach
 * @tc.desc: Old notifications cannot register or deregister the replacement attachment.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, AccessibilityRegistrationAfterReattach, TestSize.Level1)
{
    auto context = MockPipelineContext::GetCurrent();
    auto previousExecutor = context->GetTaskExecutor();
    auto executor = AceType::MakeRefPtr<QueuedAccessibilityTaskExecutor>();
    context->SetTaskExecutor(executor);
    auto node = FrameNode::CreateFrameNode("custom", 1, AceType::MakeRefPtr<CustomPattern>());
    auto pattern = node->GetPattern<CustomPattern>();
    auto oldCallback = std::make_shared<RecordingAccessibilityChildTreeCallback>(pattern);
    pattern->accessibilityChildTreeCallback_ = oldCallback;
    pattern->InitializeAccessibilityCallback();
    auto nativeProvider = pattern->GetNativeAccessibilityProvider();
    ArkUI_AccessibilityProviderCallbacks callbacks {};

    std::thread worker([&] { nativeProvider->AccessibilityProviderRegisterCallback(&callbacks); });
    worker.join();
    pattern->UninitializeAccessibility(node.GetRawPtr(), context.GetRawPtr());
    auto newCallback = std::make_shared<RecordingAccessibilityChildTreeCallback>(pattern);
    pattern->accessibilityChildTreeCallback_ = newCallback;
    pattern->InitializeAccessibilityCallback();
    executor->RunTasks();
    EXPECT_EQ(oldCallback->calls, 0);
    EXPECT_EQ(newCallback->calls, 0);

    std::thread newWorker([&] { nativeProvider->AccessibilityProviderRegisterCallback(&callbacks); });
    newWorker.join();
    executor->RunTasks();
    EXPECT_EQ(newCallback->calls, 1);
    context->SetTaskExecutor(previousExecutor);
}

/**
 * @tc.name: OnDumpChildInfoTest
 * @tc.desc: Test OnDumpChildInfo Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnDumpChildInfoTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityChildTreeCallback
     * @tc.expected: Create Custom and CustomAccessibilityChildTreeCallback Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    int accessibilityId = 2;
    auto customAccessibilityChildTreeCallback =
        std::make_shared<CustomAccessibilityChildTreeCallback>(pattern, accessibilityId);
    ASSERT_TRUE(customAccessibilityChildTreeCallback);
    /**
     * @tc.step2: Call OnDumpChildInfo Func
     * @tc.expected: ret = false
     */
    std::vector<std::string> g_params;
    std::vector<std::string> info;
    bool ret = customAccessibilityChildTreeCallback->OnDumpChildInfo(g_params, info);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SendThirdAccessibilityProviderTest
 * @tc.desc: Test SendThirdAccessibilityProvider Func
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, SendThirdAccessibilityProviderTest, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom and CustomAccessibilityProvider
     * @tc.expected: Create Custom and CustomAccessibilityProvider Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    auto customAccessibilityProvider = std::make_shared<CustomAccessibilityProvider>(pattern);
    ASSERT_TRUE(customAccessibilityProvider);
    /**
     * @tc.step2: Call SendThirdAccessibilityProvider Func
     * @tc.expected: customAccessibilityProvider->thirdAccessibilityManager_ is not null
     */
    auto thirdAccessibilityManager = std::make_shared<ThirdAccessibilityManager>();
    customAccessibilityProvider->SendThirdAccessibilityProvider(thirdAccessibilityManager);
    EXPECT_EQ(customAccessibilityProvider->GetThirdAccessibilityManager().lock(), thirdAccessibilityManager);
}

/**
 * @tc.name: ConcurrentThirdAccessibilityManager
 * @tc.desc: Register and release managers concurrently with event sending and manager snapshots.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, ConcurrentThirdAccessibilityManager, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<CustomAccessibilityProvider>(WeakPtr<CustomPattern>());
    std::atomic<bool> start { false };
    std::atomic<int32_t> calls { 0 };
    constexpr int32_t ITERATIONS = 2000;
    std::thread writer([&] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < ITERATIONS; ++i) {
            auto manager = std::make_shared<EventThirdAccessibilityManager>();
            manager->onSend = [&] { ++calls; };
            provider->SendThirdAccessibilityProvider(manager);
            provider->SendThirdAccessibilityProvider({});
        }
    });
    ArkUI_AccessibilityEventInfo event;
    int32_t successful = 0;
    start.store(true);
    for (int32_t i = 0; i < ITERATIONS; ++i) {
        auto manager = provider->GetThirdAccessibilityManager().lock();
        if (manager) {
            manager->SendAccessibilityAsyncEvent(event, nullptr);
            ++successful;
        }
        auto result = provider->SendAccessibilityAsyncEvent(event, nullptr);
        EXPECT_TRUE(result == 0 || result == -1);
        successful += result == 0;
    }
    writer.join();
    EXPECT_EQ(calls.load(), successful);
    EXPECT_TRUE(provider->GetThirdAccessibilityManager().expired());
}

/**
 * @tc.name: ThirdAccessibilityManagerReentrantSend
 * @tc.desc: Event dispatch holds a strong snapshot without holding the registration mutex.
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, ThirdAccessibilityManagerReentrantSend, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<CustomAccessibilityProvider>(WeakPtr<CustomPattern>());
    auto manager = std::make_shared<EventThirdAccessibilityManager>();
    std::weak_ptr<ThirdAccessibilityManager> weakManager = manager;
    manager->onSend = [&] {
        provider->SendThirdAccessibilityProvider({});
        manager.reset();
        EXPECT_FALSE(weakManager.expired());
    };
    provider->SendThirdAccessibilityProvider(manager);
    ArkUI_AccessibilityEventInfo event;
    EXPECT_EQ(provider->SendAccessibilityAsyncEvent(event, nullptr), 0);
    EXPECT_TRUE(weakManager.expired());
    EXPECT_EQ(provider->SendAccessibilityAsyncEvent(event, nullptr), -1);

    manager = std::make_shared<EventThirdAccessibilityManager>();
    provider->SendThirdAccessibilityProvider(manager);
    manager.reset();
    EXPECT_TRUE(provider->GetThirdAccessibilityManager().expired());
}

/**
 * @tc.name: OnAccessibilityChildTreeRegisterTest001
 * @tc.desc: Test OnAccessibilityChildTreeRegister
 * @tc.type: FUNC
 */
HWTEST_F(CustomFrameNodeTestNg, OnAccessibilityChildTreeRegisterTest001, TestSize.Level1)
{
    /**
     * @tc.step1: Create Custom
     * @tc.expected: Create Custom Successfully
     */
    auto testNode = FrameNode::CreateFrameNode("framenode", 1, AceType::MakeRefPtr<CustomPattern>(), true);
    ASSERT_NE(testNode, nullptr);
    auto pattern = testNode->GetPattern<CustomPattern>();
    ASSERT_TRUE(pattern);
    PipelineContext* context = testNode->GetContext();
    ASSERT_TRUE(context);
    context->SetIsSubPipeline(true);
    EXPECT_TRUE(context->IsSubPipeline());
    EXPECT_FALSE(pattern->OnAccessibilityChildTreeRegister(1, 0));
}
} // namespace OHOS::Ace::NG

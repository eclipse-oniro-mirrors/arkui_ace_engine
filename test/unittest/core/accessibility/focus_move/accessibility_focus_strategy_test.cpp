/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "accessibility_system_ability_client.h"
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "base/log/dump_log.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "frameworks/core/accessibility/accessibility_node.h"
#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_frame_node_utils.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_strategy.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace OHOS::Accessibility;
using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
namespace {
constexpr float TEST_FRAME_NODE_WIDTH = 10.0f;
constexpr float TEST_FRAME_NODE_HEIGHT = 10.0f;
class MockRenderContextTest : public NG::RenderContext {
public:
    NG::RectF GetPaintRectWithoutTransform() override
    {
        return *retf;
    }
    std::shared_ptr<NG::RectF> retf;
};

RefPtr<NG::FrameNode> CreatFrameNode()
{
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->SetActive(true, false);
    auto renderContext = AceType::MakeRefPtr<MockRenderContextTest>();
    CHECK_NULL_RETURN(renderContext, nullptr);
    auto rect = std::make_shared<NG::RectF>(0.0, 0.0, TEST_FRAME_NODE_WIDTH, TEST_FRAME_NODE_HEIGHT);
    renderContext->retf = rect;
    frameNode->renderContext_ = renderContext;
    return frameNode;
}

} // namespace

class AccessibilityFocusStrategyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AccessibilityFocusStrategyTest::SetUpTestCase()
{
    NG::MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = NG::MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;

    std::unique_ptr<std::ostream> ostream = std::make_unique<std::ostringstream>();
    ASSERT_NE(ostream, nullptr);
    DumpLog::GetInstance().SetDumpFile(std::move(ostream));
}

void AccessibilityFocusStrategyTest::TearDownTestCase()
{
    NG::MockPipelineContext::TearDown();
    MockContainer::TearDown();
}


/**
 * @tc.name: FindLastNodeWithoutCheck001
 * @tc.desc: UpdateAccessibilityElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, FindLastNodeWithoutCheck001, TestSize.Level1)
{
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);
    frameNode->SetActive(true, false);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    AccessibilityFocusStrategy focusStrategy;
    // 1. get self when has no children
    focusStrategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode->GetAccessibilityId(), targetNode->GetAccessibilityId());
    // 2. get child when has  children
    auto frameNode1 = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode1, nullptr);
    frameNode1->SetActive(true, false);
    frameNode->AddChild(frameNode1);
    targetNode = nullptr;
    auto result = focusStrategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode1->GetAccessibilityId(), targetNode->GetAccessibilityId());
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    // 3. childe is childtree container
    auto accessibilityProperty1 = frameNode1->GetAccessibilityProperty<NG::AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty1, nullptr);
    accessibilityProperty1->SetChildTreeId(1);
    result = focusStrategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_CHILDTREE);
}

/**
 * @tc.name: FindForwardScrollAncestor001
 * @tc.desc: UpdateAccessibilityElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, FindForwardScrollAncestor001, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);
    AccessibilityFocusStrategy focusStrategy;
    std::list<std::shared_ptr<FocusRulesCheckNode>> targetNodes;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());
    // 1. no fowardScrollAncestor
    auto result = focusStrategy.FindForwardScrollAncestor(condition, checkNode, targetNodes);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    EXPECT_EQ(targetNodes.size(), 0);
    // 2. parent is fowardScrollAncestor
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    accessibilityProperty->SetSpecificSupportActionCallback(
        [accessibilityPtr = AceType::WeakClaim(AceType::RawPtr(accessibilityProperty))]() {
        const auto& accessibilityProperty = accessibilityPtr.Upgrade();
        CHECK_NULL_VOID(accessibilityProperty);
        accessibilityProperty->AddSupportAction(AceAction::ACTION_SCROLL_FORWARD);
    });
    
    result = focusStrategy.FindForwardScrollAncestor(condition, checkNode, targetNodes);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    ASSERT_EQ(targetNodes.size(), 1);
    auto targetNode = targetNodes.front();
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode->GetAccessibilityId(), targetNode->GetAccessibilityId());
}

/**
 * @tc.name: FindForwardScrollAncestor001
 * @tc.desc: UpdateAccessibilityElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, FindForwardScrollAncestor002, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);
    AccessibilityFocusStrategy focusStrategy;
    std::list<std::shared_ptr<FocusRulesCheckNode>> targetNodes;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());
    // 1. parent is fowardScrollAncestor but not visiable
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    accessibilityProperty->SetSpecificSupportActionCallback(
        [accessibilityPtr = AceType::WeakClaim(AceType::RawPtr(accessibilityProperty))]() {
        const auto& accessibilityProperty = accessibilityPtr.Upgrade();
        CHECK_NULL_VOID(accessibilityProperty);
        accessibilityProperty->AddSupportAction(AceAction::ACTION_SCROLL_FORWARD);
    });
    auto renderContext = AceType::MakeRefPtr<MockRenderContextTest>();
    ASSERT_NE(renderContext, nullptr);
    auto rect = std::make_shared<NG::RectF>(0.0, 0.0, 0, 0);
    renderContext->retf = rect;
    frameNode->renderContext_ = renderContext;
    auto result = focusStrategy.FindForwardScrollAncestor(condition, checkNode, targetNodes);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    EXPECT_EQ(targetNodes.size(), 0);
}

/**
 * @tc.name: FindForwardScrollAncestor001
 * @tc.desc: UpdateAccessibilityElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, FindForwardScrollAncestor003, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);
    AccessibilityFocusStrategy focusStrategy;
    std::list<std::shared_ptr<FocusRulesCheckNode>> targetNodes;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());
    // 1. parent can not GetPropActionName
    frameNode->accessibilityProperty_ = nullptr;
    auto result = focusStrategy.FindForwardScrollAncestor(condition, checkNode, targetNodes);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    EXPECT_EQ(targetNodes.size(), 0);
}

/**
 * @tc.name: FindForwardScrollAncestor001
 * @tc.desc: UpdateAccessibilityElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, CanAccessibilityFocus001, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);

    AccessibilityFocusStrategy focusStrategy;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    // 1. node is visible
    auto result = focusStrategy.CanAccessibilityFocus(checkNode);
    EXPECT_EQ(result, false);
    // 2. node is not visible
    auto renderContext = AceType::MakeRefPtr<MockRenderContextTest>();
    ASSERT_NE(renderContext, nullptr);
    auto rect = std::make_shared<NG::RectF>(0.0, 0.0, 0, 0);
    renderContext->retf = rect;
    frameNode->renderContext_ = renderContext;
    result = focusStrategy.CanAccessibilityFocus(checkNode);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetPropActionNames001
 * @tc.desc: GetPropActionNames click info
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, GetPropActionNames001, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    // 1. focushub but not focusable
    auto focusHub = frameNode->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    Accessibility::PropValueStub value;
    auto result = checkNode->GetPropActionNames(value);
    EXPECT_EQ(result, true);
    auto findRet = value.valueArray.find("clearFocus");
    EXPECT_TRUE(findRet == value.valueArray.end());
    findRet = value.valueArray.find("focus");
    EXPECT_TRUE(findRet == value.valueArray.end());
    // 2. focushub , focusable , current focus is false
    focusHub->SetParentFocusable(true);
    focusHub->SetFocusable(true);
    focusHub->SetFocusType(NG::FocusType::NODE);
    focusHub->SetCurrentFocus(false);
    ASSERT_EQ(focusHub->IsFocusable(), true);
    ASSERT_EQ(focusHub->IsCurrentFocus(), false);
    Accessibility::PropValueStub value1;
    result = checkNode->GetPropActionNames(value1);
    EXPECT_EQ(result, true);
    findRet = value1.valueArray.find("clearFocus");
    EXPECT_TRUE(findRet == value1.valueArray.end());
    findRet = value1.valueArray.find("focus");
    EXPECT_TRUE(findRet != value1.valueArray.end());

    // 3. focushub , focusable , current focus is true
    focusHub->SetCurrentFocus(true);
    ASSERT_EQ(focusHub->IsCurrentFocus(), true);
    Accessibility::PropValueStub value2;
    result = checkNode->GetPropActionNames(value2);
    EXPECT_EQ(result, true);
    findRet = value2.valueArray.find("clearFocus");
    EXPECT_TRUE(findRet != value2.valueArray.end());
    findRet = value2.valueArray.find("focus");
    EXPECT_TRUE(findRet == value2.valueArray.end());
}

/**
 * @tc.name: GetPropActionNames001
 * @tc.desc: GetPropActionNames click info
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, GetPropActionNames002, TestSize.Level1)
{
    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    // 1. eventhub but not clickable not longclick able
    auto gestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub, nullptr);
    Accessibility::PropValueStub value;
    auto result = checkNode->GetPropActionNames(value);
    EXPECT_EQ(result, true);
    auto findRet = value.valueArray.find("click");
    EXPECT_TRUE(findRet == value.valueArray.end());
    findRet = value.valueArray.find("longClick");
    EXPECT_TRUE(findRet == value.valueArray.end());
    // 2. eventhub but  clickable and  longclick able
    auto clickCallback = [](GestureEvent& info) {
    };
    auto clickEvent = AceType::MakeRefPtr<NG::ClickEvent>(std::move(clickCallback));
    gestureEventHub->AddClickEvent(clickEvent);
    auto longPressTask = [](GestureEvent& info) {
    };
    auto longPressEventTest = AceType::MakeRefPtr<NG::LongPressEvent>(std::move(longPressTask));
    gestureEventHub->SetLongPressEvent(longPressEventTest);
    ASSERT_EQ(gestureEventHub->IsAccessibilityClickable(), true);
    ASSERT_EQ(gestureEventHub->IsAccessibilityLongClickable(), true);
    Accessibility::PropValueStub value1;
    result = checkNode->GetPropActionNames(value1);
    EXPECT_EQ(result, true);
    findRet = value1.valueArray.find("click");
    EXPECT_TRUE(findRet != value1.valueArray.end());
    findRet = value1.valueArray.find("longClick");
    EXPECT_TRUE(findRet != value1.valueArray.end());
}

/**
 * @tc.name: GetAceParent001
 * @tc.desc: get parent
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusStrategyTest, GetAceParent001, TestSize.Level1)
{
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    auto stage = NG::FrameNode::CreateFrameNode("stage", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(stage, nullptr);
    auto page = NG::FrameNode::CreateFrameNode("page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(page, nullptr);
    auto root = NG::FrameNode::CreateFrameNode("root", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(root, nullptr);

    // bypass page and stage
    page->AddChild(frameNode);
    frameNode->MountToParent(page);
    stage->AddChild(page);
    page->MountToParent(stage);
    root->AddChild(stage);
    stage->MountToParent(root);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    auto targetNode = checkNode->GetAceParent();
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), targetNode->GetAccessibilityId());
}
} // namespace OHOS::Ace::NG
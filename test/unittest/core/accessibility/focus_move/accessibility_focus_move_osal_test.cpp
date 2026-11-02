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

#include <atomic>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "accessibility_system_ability_client.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "ui/properties/ui_material.h"

#include "base/log/dump_log.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "frameworks/core/accessibility/accessibility_node.h"
#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_accessibility_node_utils.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_frame_node_utils.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal_ng.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal_third.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_strategy.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation.h"
#include "adapter/ohos/osal/accessibility/third_accessibility_traversal.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/core/components_ng/property/accessibility_property.h"

#include "test/unittest/core/accessibility/js_accessibility_manager_test.h"
#include "test/unittest/core/accessibility/js_third_provider_interaction_operation_test.h"

using namespace OHOS::Accessibility;
using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
namespace {
constexpr float TEST_FRAME_NODE_WIDTH = 10.0f;
constexpr float TEST_FRAME_NODE_HEIGHT = 10.0f;

class MockThirdAccessibilityManagerForCheckNode : public ThirdAccessibilityManager {
public:
    RefPtr<NG::CustomAccessibilityProperty> GetThirdCustomProperty(int64_t elementId) override
    {
        if (mockProperty) {
            return mockProperty;
        }
        return nullptr;
    }
    bool HasChildTree(int64_t elementId) override
    {
        return hasChildTree_ && (elementId == childTreeElementId_);
    }
    RefPtr<NG::CustomAccessibilityProperty> mockProperty;
    bool hasChildTree_ = false;
    int64_t childTreeElementId_ = -1;
};

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

void CreatNextPrevFrameNode(
    RefPtr<Framework::JsAccessibilityManager>& jsAccessibilityManager,
    int32_t instanceId,
    RefPtr<NG::FrameNode>& currentFrameNode,
    RefPtr<NG::FrameNode>& nextFrameNode,
    RefPtr<NG::FrameNode>& prevFrameNode)
{
    ASSERT_NE(jsAccessibilityManager, nullptr);
    currentFrameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(currentFrameNode, nullptr);

    nextFrameNode = NG::FrameNode::CreateFrameNode("nextFrameNode",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(nextFrameNode, nullptr);
    std::string nextKey = "next_key";
    nextFrameNode->UpdateInspectorId(nextKey);
    prevFrameNode = NG::FrameNode::CreateFrameNode("prevFrameNode",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(prevFrameNode, nullptr);
    std::string currentKey = "current_key";
    currentFrameNode->UpdateInspectorId(currentKey);
    auto accessibilityProperty = currentFrameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    accessibilityProperty->SetAccessibilityNextFocusInspectorKey(nextKey);
    jsAccessibilityManager->UpdateAccessibilityNextFocusIdMap(
        instanceId, currentKey, prevFrameNode->GetAccessibilityId());
}

} // namespace

class AccessibilityFocusMoveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AccessibilityFocusMoveTest::SetUpTestCase()
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

void AccessibilityFocusMoveTest::TearDownTestCase()
{
    NG::MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

class MockFocusStrategyOsal : public FocusStrategyOsal {
    DECLARE_ACE_TYPE(MockFocusStrategyOsal, AceType);
public:
    explicit MockFocusStrategyOsal(const RefPtr<JsAccessibilityManager>& jsAccessibilityManager)
        : FocusStrategyOsal(jsAccessibilityManager) {}
    ~MockFocusStrategyOsal() override = default;

    void InvailidElementInfo(Accessibility::AccessibilityElementInfo& info) override {}
    bool UpdateOriginNodeInfo(int64_t elementId) override { return updateOriginNodeInfoResult_; }
    std::shared_ptr<FocusRulesCheckNode> GetCurrentCheckNode() override { return currentCheckNode_; }
    void UpdateNextFocus(std::shared_ptr<FocusRulesCheckNode>& checkNode) override {}
    std::shared_ptr<FocusRulesCheckNode> ChangeToRoot() override { return rootCheckNode_; }
    bool ChangeToEmbed(
        const std::shared_ptr<FocusRulesCheckNode>& resultNode, Accessibility::AccessibilityElementInfo& info) override
    {
        return changeToEmbedResult_;
    }
    bool UpdateElementInfo(
        const std::shared_ptr<FocusRulesCheckNode>& resultNode, Accessibility::AccessibilityElementInfo& info) override
    {
        return updateElementInfoResult_;
    }

    bool CheckIsRootType(const std::shared_ptr<FocusRulesCheckNode>& checkNode)
    {
        CHECK_NULL_RETURN(checkNode, false);
        return checkNode->GetAccessibilityId() == isRootTypeId_;
    }

    bool CheckIsReadable(
        const std::shared_ptr<FocusRulesCheckNode>& checkNode, Accessibility::FocusRuleType focusRuleType) override
    {
        lastFocusRuleType_ = focusRuleType;
        checkIsReadableCount_++;
        CHECK_NULL_RETURN(checkNode, false);
        return checkNode->GetAccessibilityId() == isReadableId_;
    }

    bool CheckIsReadableRulesEnable() override
    {
        return readableRulesEnable_;
    }
public:
    bool updateOriginNodeInfoResult_ = true;
    std::shared_ptr<FocusRulesCheckNode> currentCheckNode_;
    bool updateNextFocusResult_ = true;
    std::shared_ptr<FocusRulesCheckNode> rootCheckNode_;
    bool changeToEmbedResult_ = true;
    bool updateElementInfoResult_ = true;
    int64_t isRootTypeId_ = -1;
    int64_t isReadableId_ = -1;
    Accessibility::FocusRuleType lastFocusRuleType_ = Accessibility::FocusRuleType::DEFAULT;
    int32_t checkIsReadableCount_ = 0;
    bool readableRulesEnable_ = true;
};

/**
 * @tc.name: AccessibilityNodeUtilsTest001
 * @tc.desc: GetPropText
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, AccessibilityNodeUtilsTest001, TestSize.Level1)
{
    int32_t nodeId = 1;
    auto accessibilityNode = AceType::MakeRefPtr<AccessibilityNode>(nodeId, "test");
    ASSERT_NE(accessibilityNode, nullptr);
    std::string testStr = "hello";
    std::string strStar(testStr.size(), '*');
    accessibilityNode->SetText(testStr);
    auto checkNode = std::make_shared<AccessibilityNodeRulesCheckNode>(
        accessibilityNode, static_cast<int64_t>(accessibilityNode->GetNodeId()));
    ASSERT_NE(checkNode, nullptr);
    Accessibility::PropValue value;
    auto result = checkNode->GetPropText(value);
    ASSERT_TRUE(result);
    ASSERT_EQ(value.valueStr, testStr);
    accessibilityNode->SetIsPassword(true);
    result = checkNode->GetPropText(value);
    ASSERT_TRUE(result);
    ASSERT_EQ(value.valueStr, strStar);
}

/**
 * @tc.name: AccessibilityNodeUtilsTest002
 * @tc.desc: GetPropActionNames
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, AccessibilityNodeUtilsTest002, TestSize.Level1)
{
    int32_t nodeId = 1;
    auto accessibilityNode = AceType::MakeRefPtr<AccessibilityNode>(nodeId, "test");
    ASSERT_NE(accessibilityNode, nullptr);
    auto checkNode = std::make_shared<AccessibilityNodeRulesCheckNode>(
        accessibilityNode, static_cast<int64_t>(accessibilityNode->GetNodeId()));
    ASSERT_NE(checkNode, nullptr);
    accessibilityNode->AddSupportAction(AceAction::ACTION_FOCUS);
    Accessibility::PropValue value;
    auto result = checkNode->GetPropActionNames(value);
    EXPECT_EQ(result, true);
    auto findRet = value.valueArray.find("clearFocus");
    EXPECT_TRUE(findRet == value.valueArray.end());
    findRet = value.valueArray.find("focus");
    EXPECT_TRUE(findRet != value.valueArray.end());
}

/**
 * @tc.name: AccessibilityNodeUtilsTest003
 * @tc.desc: GetPropHintText
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, AccessibilityNodeUtilsTest003, TestSize.Level1)
{
    int32_t nodeId = 1;
    auto accessibilityNode = AceType::MakeRefPtr<AccessibilityNode>(nodeId, "test");
    ASSERT_NE(accessibilityNode, nullptr);
    std::string testStr = "hello";
    accessibilityNode->SetHintText(testStr);
    auto checkNode = std::make_shared<AccessibilityNodeRulesCheckNode>(
        accessibilityNode, static_cast<int64_t>(accessibilityNode->GetNodeId()));
    ASSERT_NE(checkNode, nullptr);
    Accessibility::PropValue value;
    auto result = checkNode->GetPropHintText(value);
    ASSERT_TRUE(result);
    ASSERT_EQ(value.valueType, Accessibility::ValueType::STRING);
    ASSERT_EQ(value.valueStr, testStr);
}

/**
 * @tc.name: FocusStrategyOsalNGTest001
 * @tc.desc: SetNextFocusIdToElementInfo  SetPrevFocusIdToElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGTest001, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    ASSERT_NE(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    auto instanceId = context->GetInstanceId();
    ASSERT_NE(rootNode, nullptr);

    RefPtr<NG::FrameNode> currentFrameNode;
    RefPtr<NG::FrameNode> nextFrameNode;
    RefPtr<NG::FrameNode> prevFrameNode;
    CreatNextPrevFrameNode(jsAccessibilityManager, instanceId, currentFrameNode, nextFrameNode, prevFrameNode);
    ASSERT_NE(currentFrameNode, nullptr);
    ASSERT_NE(nextFrameNode, nullptr);
    ASSERT_NE(prevFrameNode, nullptr);
    rootNode->AddChild(currentFrameNode);
    rootNode->AddChild(nextFrameNode);
    rootNode->AddChild(prevFrameNode);

    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    strategy.UpdateOriginNodeInfo(currentFrameNode->GetAccessibilityId());
    auto checkNode = strategy.GetCurrentCheckNode();
    strategy.UpdateNextFocus(checkNode);
    Accessibility::AccessibilityElementInfo nodeInfo;
    strategy.UpdateElementInfo(checkNode, nodeInfo);
    ASSERT_NE(strategy.rootNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.baseNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.baseNextNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.basePrevNode_.Upgrade(), nullptr);
    EXPECT_EQ(nodeInfo.GetAccessibilityNextFocusId(), nextFrameNode->GetAccessibilityId());
    EXPECT_EQ(nodeInfo.GetAccessibilityPreviousFocusId(), prevFrameNode->GetAccessibilityId());
    rootNode->RemoveChild(currentFrameNode);
    rootNode->RemoveChild(nextFrameNode);
    rootNode->RemoveChild(prevFrameNode);
}
/**
 * @tc.name: FocusStrategyOsalNGTest002
 * @tc.desc: SetNextFocusIdToElementInfo  SetPrevFocusIdToElementInfo with tree Id
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGTest002, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    ASSERT_NE(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    auto instanceId = context->GetInstanceId();
    ASSERT_NE(rootNode, nullptr);

    RefPtr<NG::FrameNode> currentFrameNode;
    RefPtr<NG::FrameNode> nextFrameNode;
    RefPtr<NG::FrameNode> prevFrameNode;
    CreatNextPrevFrameNode(jsAccessibilityManager, instanceId, currentFrameNode, nextFrameNode, prevFrameNode);
    ASSERT_NE(currentFrameNode, nullptr);
    ASSERT_NE(nextFrameNode, nullptr);
    ASSERT_NE(prevFrameNode, nullptr);
    rootNode->AddChild(currentFrameNode);
    rootNode->AddChild(nextFrameNode);
    rootNode->AddChild(prevFrameNode);
    int32_t treeId = 1;
    jsAccessibilityManager->treeId_ = treeId;

    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    strategy.UpdateOriginNodeInfo(currentFrameNode->GetAccessibilityId());
    auto checkNode = strategy.GetCurrentCheckNode();
    strategy.UpdateNextFocus(checkNode);
    Accessibility::AccessibilityElementInfo nodeInfo;
    strategy.UpdateElementInfo(checkNode, nodeInfo);
    ASSERT_NE(strategy.rootNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.baseNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.baseNextNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.basePrevNode_.Upgrade(), nullptr);
    auto nextId = nextFrameNode->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, nextId);
    auto prevId = prevFrameNode->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, prevId);
    EXPECT_EQ(nodeInfo.GetAccessibilityNextFocusId(), nextId);
    EXPECT_EQ(nodeInfo.GetAccessibilityPreviousFocusId(), prevId);
    rootNode->RemoveChild(currentFrameNode);
    rootNode->RemoveChild(nextFrameNode);
    rootNode->RemoveChild(prevFrameNode);
}

/**
 * @tc.name: FocusStrategyOsalNGTest003
 * @tc.desc: SetNextFocusIdToElementInfo  SetPrevFocusIdToElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGTest003, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    ASSERT_NE(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    auto instanceId = context->GetInstanceId();
    ASSERT_NE(rootNode, nullptr);

    RefPtr<NG::FrameNode> finalFrameNode;
    RefPtr<NG::FrameNode> nextFrameNode;
    RefPtr<NG::FrameNode> prevFrameNode;
    CreatNextPrevFrameNode(jsAccessibilityManager, instanceId, finalFrameNode, nextFrameNode, prevFrameNode);
    ASSERT_NE(finalFrameNode, nullptr);
    ASSERT_NE(nextFrameNode, nullptr);
    ASSERT_NE(prevFrameNode, nullptr);
    rootNode->AddChild(finalFrameNode);
    rootNode->AddChild(nextFrameNode);
    rootNode->AddChild(prevFrameNode);

    auto currentFrameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(currentFrameNode, nullptr);
    rootNode->AddChild(currentFrameNode);
    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    strategy.UpdateOriginNodeInfo(currentFrameNode->GetAccessibilityId());
    auto checkNode = strategy.GetCurrentCheckNode();
    strategy.UpdateNextFocus(checkNode);
    ASSERT_NE(strategy.rootNode_.Upgrade(), nullptr);
    ASSERT_NE(strategy.baseNode_.Upgrade(), nullptr);
    ASSERT_EQ(strategy.baseNextNode_.Upgrade(), nullptr);
    ASSERT_EQ(strategy.basePrevNode_.Upgrade(), nullptr);
    Accessibility::AccessibilityElementInfo nodeInfo;
    auto finalNode = std::make_shared<FrameNodeRulesCheckNode>(finalFrameNode, finalFrameNode->GetAccessibilityId());
    strategy.UpdateElementInfo(finalNode, nodeInfo);
    EXPECT_EQ(nodeInfo.GetAccessibilityNextFocusId(), nextFrameNode->GetAccessibilityId());
    EXPECT_EQ(nodeInfo.GetAccessibilityPreviousFocusId(), prevFrameNode->GetAccessibilityId());
    rootNode->RemoveChild(currentFrameNode);
    rootNode->RemoveChild(finalFrameNode);
    rootNode->RemoveChild(nextFrameNode);
    rootNode->RemoveChild(prevFrameNode);
}

/**
 * @tc.name: FocusStrategyOsalNGTest004
 * @tc.desc: ChangeToEmbed
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGTest004, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    auto currentFrameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(currentFrameNode, nullptr);
    Accessibility::AccessibilityElementInfo nodeInfo;
    auto checkNode =
        std::make_shared<FrameNodeRulesCheckNode>(currentFrameNode, currentFrameNode->GetAccessibilityId());
    auto result = strategy.ChangeToEmbed(checkNode, nodeInfo);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckAndGetReadableInfoToRootTest001
 * @tc.desc: test CheckAndGetReadableInfoToRoot when framenode is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, CheckAndGetReadableInfoToRootTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct strategy
     */
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);

    /**
     * @tc.steps: step2. test CheckAndGetReadableInfoToRoot when framenode is nullptr
     */
    std::list<Accessibility::AccessibilityElementInfo> targetInfos;
    int32_t windowId = 0;
    auto result = strategy.CheckAndGetReadableInfoToRoot(nullptr, targetInfos, windowId);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL);
}

/**
 * @tc.name: FocusStrategyOsalThirdTest001
 * @tc.desc: ChangeToRoot
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalThirdTest001, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    NodeConfig config;
    FocusStrategyOsalThird strategy(jsAccessibilityManager, ohAccessibilityProvider, config);
    EXPECT_EQ(strategy.ChangeToRoot(), nullptr);
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(-1));
    auto rootNode = strategy.ChangeToRoot();
    EXPECT_NE(rootNode, nullptr);
}

/**
 * @tc.name: FocusStrategyOsalThirdTest002
 * @tc.desc: ChangeToRoot
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalThirdTest002, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    ohAccessibilityProvider->SetInjectResult(-1);
    NodeConfig config;
    FocusStrategyOsalThird strategy(jsAccessibilityManager, ohAccessibilityProvider, config);
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(-1));
    auto rootNode = strategy.ChangeToRoot();
    EXPECT_EQ(rootNode, nullptr);
}

/**
 * @tc.name: ThirdRulesCheckNodeIsChildTreeContainerTest001
 * @tc.desc: IsChildTreeContainer
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, ThirdRulesCheckNodeIsChildTreeContainerTest001, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    // std::make_shared (not AceType::MakeRefPtr): SendThirdAccessibilityProvider takes
    // std::weak_ptr<ThirdAccessibilityManager>, RefPtr cannot convert to it implicitly.
    auto thirdManager = std::make_shared<MockThirdAccessibilityManagerForCheckNode>();
    ASSERT_NE(thirdManager, nullptr);
    int64_t bindId = MAX_TEST_ELEMENT_COUNT - 1;
    thirdManager->hasChildTree_ = true;
    thirdManager->childTreeElementId_ = bindId;

    // 1 no manager registered on provider
    auto nodeInfo = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo->SetElementId(bindId);
    auto checkNode = std::make_shared<ThirdRulesCheckNode>(nodeInfo, bindId, ohAccessibilityProvider);
    ASSERT_NE(checkNode, nullptr);
    EXPECT_FALSE(checkNode->IsChildTreeContainer());

    // 2 manager registered and node id is the bindId mount node
    ohAccessibilityProvider->SendThirdAccessibilityProvider(thirdManager);
    EXPECT_TRUE(checkNode->IsChildTreeContainer());

    // 3 node id is not the bindId mount node
    auto nodeInfo2 = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo2->SetElementId(bindId - 1);
    auto checkNode2 = std::make_shared<ThirdRulesCheckNode>(nodeInfo2, bindId - 1, ohAccessibilityProvider);
    ASSERT_NE(checkNode2, nullptr);
    EXPECT_FALSE(checkNode2->IsChildTreeContainer());

    // 4 manager registered but no child tree bound
    thirdManager->hasChildTree_ = false;
    EXPECT_FALSE(checkNode->IsChildTreeContainer());
}

/**
 * @tc.name: ThirdFocusMoveChildTreeContainerTest001
 * @tc.desc: shared strategy emits FIND_CHILDTREE when last node search hits the bindId mount node
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, ThirdFocusMoveChildTreeContainerTest001, TestSize.Level1)
{
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    auto thirdManager = std::make_shared<MockThirdAccessibilityManagerForCheckNode>();
    ASSERT_NE(thirdManager, nullptr);
    thirdManager->hasChildTree_ = true;
    thirdManager->childTreeElementId_ = MAX_TEST_ELEMENT_COUNT - 1;
    ohAccessibilityProvider->SendThirdAccessibilityProvider(thirdManager);

    // Start from the provider's LINKED chain element (the mock links 0→1→…→9 via
    // child/parent ids): a bare info with only SetElementId(0) carries no child ids,
    // so the last-child descent would stop at the start node itself. Fetch element 0
    // through the same native-info channel ThirdRulesCheckNode::GetAceChildren uses.
    auto nodeInfo = std::make_shared<ArkUI_AccessibilityElementInfo>();
    ASSERT_TRUE(JsThirdProviderInteractionOperation::FindNativeInfoById(
        ohAccessibilityProvider, 0, nodeInfo));
    auto checkNode = std::make_shared<ThirdRulesCheckNode>(nodeInfo, 0, ohAccessibilityProvider);
    ASSERT_NE(checkNode, nullptr);

    AccessibilityFocusStrategy strategy;
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    std::shared_ptr<FocusRulesCheckNode> targetNode;

    // 1 last node of the provider chain is the bindId mount node
    auto result = strategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_CHILDTREE);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(targetNode->GetAccessibilityId(), MAX_TEST_ELEMENT_COUNT - 1);

    // 2 no child tree bound, last node is found directly
    thirdManager->hasChildTree_ = false;
    targetNode = nullptr;
    result = strategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    EXPECT_EQ(result, AceFocusMoveResult::FIND_SUCCESS);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(targetNode->GetAccessibilityId(), MAX_TEST_ELEMENT_COUNT - 1);
}

/**
 * @tc.name: ThirdRulesCheckNodeTest001
 * @tc.desc: GetPropActionNames
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, ThirdRulesCheckNodeTest001, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    auto nodeInfo = std::make_shared<ArkUI_AccessibilityElementInfo>();
    int32_t elementId = 1;
    nodeInfo->SetElementId(elementId);
    ArkUI_AccessibleAction tempAction { ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_GAIN_ACCESSIBILITY_FOCUS, "test"};
    nodeInfo->AddOperationAction(tempAction);
    auto checkNode = std::make_shared<ThirdRulesCheckNode>(nodeInfo, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode, nullptr);
    
    Accessibility::PropValue value;
    auto result = checkNode->GetPropActionNames(value);
    EXPECT_EQ(result, true);
    auto findRet = value.valueArray.find("clearAccessibilityFocus");
    EXPECT_TRUE(findRet == value.valueArray.end());
    findRet = value.valueArray.find("accessibilityFocus");
    EXPECT_TRUE(findRet != value.valueArray.end());
}

/**
 * @tc.name: ThirdRulesCheckNodeTest002
 * @tc.desc: GetAceChildren
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, ThirdRulesCheckNodeTest002, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    auto nodeInfo = std::make_shared<ArkUI_AccessibilityElementInfo>();
    int32_t elementId = 1;
    nodeInfo->SetElementId(elementId);
    nodeInfo->AddChildNodeId(-1);
    nodeInfo->AddChildNodeId(Accessibility::AccessibilityElementInfo::ROOT_PARENT_ID);
    nodeInfo->AddChildNodeId(MAX_TEST_ELEMENT_COUNT);
    nodeInfo->AddChildNodeId(0);
    auto checkNode = std::make_shared<ThirdRulesCheckNode>(nodeInfo, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode, nullptr);

    auto children = checkNode->GetAceChildren();
    EXPECT_EQ(children.size(), 1);
}

/**
 * @tc.name: ThirdRulesCheckNodeTest003
 * @tc.desc: GetAceParent
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, ThirdRulesCheckNodeTest003, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto ohAccessibilityProvider
        = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    ASSERT_NE(ohAccessibilityProvider, nullptr);
    int32_t elementId = 1;
    auto nodeInfo1 = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo1->SetElementId(elementId);
    nodeInfo1->SetParentId(-1);
    auto checkNode1 = std::make_shared<ThirdRulesCheckNode>(nodeInfo1, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode1, nullptr);
    auto parent = checkNode1->GetAceParent();
    EXPECT_EQ(parent, nullptr);

    auto nodeInfo2 = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo2->SetElementId(elementId);
    nodeInfo2->SetParentId(Accessibility::AccessibilityElementInfo::ROOT_PARENT_ID);
    auto checkNode2 = std::make_shared<ThirdRulesCheckNode>(nodeInfo2, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode2, nullptr);
    parent = checkNode2->GetAceParent();
    EXPECT_EQ(parent, nullptr);

    auto nodeInfo3 = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo3->SetElementId(elementId);
    nodeInfo3->SetParentId(MAX_TEST_ELEMENT_COUNT);
    auto checkNode3 = std::make_shared<ThirdRulesCheckNode>(nodeInfo3, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode3, nullptr);
    parent = checkNode3->GetAceParent();
    EXPECT_EQ(parent, nullptr);

    auto nodeInfo4 = std::make_shared<ArkUI_AccessibilityElementInfo>();
    nodeInfo4->SetElementId(elementId);
    nodeInfo4->SetParentId(0);
    auto checkNode4 = std::make_shared<ThirdRulesCheckNode>(nodeInfo4, elementId, ohAccessibilityProvider);
    ASSERT_NE(checkNode4, nullptr);
    parent = checkNode4->GetAceParent();
    EXPECT_NE(parent, nullptr);
}

/**
 * @tc.name: FocusStrategyOsalTest001
 * @tc.desc: IsProcessGetScrollAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalTest001, TestSize.Level1)
{
    std::map<FocusMoveDirection, bool> IsProcessGetScrollAncestorTest = {
        { FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR, true },
        { FocusMoveDirection::GET_BACKWARD_SCROLL_ANCESTOR, true },
        { FocusMoveDirection::GET_SCROLLABLE_ANCESTOR, true },
        { FocusMoveDirection::DIRECTION_INVALID, false },
    };
    Accessibility::AccessibilityFocusMoveParam param;
    for (auto& testMap : IsProcessGetScrollAncestorTest) {
        param.direction = testMap.first;
        auto result = FocusStrategyOsal::IsProcessGetScrollAncestor(param);
        ASSERT_EQ(result, testMap.second);
    }
}

/**
 * @tc.name: FocusStrategyOsalTest002
 * @tc.desc: ProcessGetScrollAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalTest002, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    bool result = false;
    Accessibility::AccessibilityFocusMoveParam param;

    int64_t elementId = 0;
    std::list<Accessibility::AccessibilityElementInfo> infos;
    param.direction = FocusMoveDirection::GET_BACKWARD_SCROLL_ANCESTOR;
    strategy.ProcessGetScrollAncestor(elementId, param, infos);
    ASSERT_EQ(infos.empty(), true);
    param.direction = FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR;
    strategy.ProcessGetScrollAncestor(elementId, param, infos);
    ASSERT_EQ(infos.empty(), true);
    param.direction = FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR;
    strategy.ProcessGetScrollAncestor(elementId, param, infos);
    ASSERT_EQ(infos.empty(), true);
}

/**
 * @tc.name: FocusStrategyOsalTest003
 * @tc.desc: GetAceCondition
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalTest003, TestSize.Level1)
{
    std::map<DetailCondition, AceFocusMoveDetailCondition> GetAceConditionTest = {
        { DetailCondition::BYPASS_SELF, {.bypassSelf = true, .bypassDescendants = false} },
        { DetailCondition::BYPASS_SELF_DESCENDANTS, {.bypassSelf = true, .bypassDescendants = true} },
        { DetailCondition::CHECK_SELF, {.bypassSelf = false, .bypassDescendants = false} },
        { DetailCondition::CHECK_SELF_BYPASS_DESCENDANTS, {.bypassSelf = false, .bypassDescendants = true} },
    };

    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;

    for (auto& testMap : GetAceConditionTest) {
        param.condition = testMap.first;
        auto result = strategy.GetAceCondition(param);
        ASSERT_EQ(result.bypassSelf, testMap.second.bypassSelf);
        ASSERT_EQ(result.bypassDescendants, testMap.second.bypassDescendants);
    }
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest001
 * @tc.desc: HandleFocusMoveSearchResult
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest001, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    // aceResult FIND_SUCCESS
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_SUCCESS;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_SUCCESS);
    // aceResult FIND_FAIL
    aceResult = AceFocusMoveResult::FIND_FAIL;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL);

    // aceResult FIND_SUCCESS, UpdateElementInfo fail
    aceResult = AceFocusMoveResult::FIND_SUCCESS;
    strategy.updateElementInfoResult_ = false;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL);

    // aceResult FIND_FAIL, finalNode nullptr
    std::shared_ptr<FocusRulesCheckNode> targetNode1;
    aceResult = AceFocusMoveResult::FIND_FAIL;
    strategy.updateElementInfoResult_ = false;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode1, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL);

    // aceResult FIND_FAIL_LOST_NODE, finalNode nullptr
    std::shared_ptr<FocusRulesCheckNode> targetNode2;
    aceResult = AceFocusMoveResult::FIND_FAIL_LOST_NODE;
    strategy.updateElementInfoResult_ = false;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode2, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);

    // aceResult FIND_FAIL, finalNode nullptr
    std::shared_ptr<FocusRulesCheckNode> targetNode3;
    aceResult = AceFocusMoveResult::FIND_FAIL_IN_ROOT_TYPE;
    strategy.updateElementInfoResult_ = false;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode3, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL);
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest002
 * @tc.desc: HandleFocusMoveSearchResult
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest002, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    // aceResult FIND_CHILDTREE
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_CHILDTREE;
    param.direction = FocusMoveDirection::BACKWARD;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST);
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest003
 * @tc.desc: HandleFocusMoveSearchResult
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest003, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    // aceResult FIND_FAIL_IN_CHILDTREE, direction  BACKWARD
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_FAIL_IN_CHILDTREE;
    param.direction = FocusMoveDirection::BACKWARD;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE);
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest004
 * @tc.desc: HandleFocusMoveSearchResult
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest004, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    // aceResult FIND_EMBED_TARGET, ChangeToEmbed successs
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_EMBED_TARGET;
    strategy.changeToEmbedResult_ = true;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_NEXT);

    // aceResult FIND_EMBED_TARGET, ChangeToEmbed fail
    aceResult = AceFocusMoveResult::FIND_EMBED_TARGET;
    strategy.changeToEmbedResult_ = false;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL);
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest005
 * @tc.desc: HandleFocusMoveSearchResult
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest005, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    // aceResult FIND_FAIL_IN_SCROLL
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_FAIL_IN_SCROLL;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL_IN_SCROLL);
}

/**
 * @tc.name: FocusStrategyOsalHandleFocusMoveSearchResultTest006
 * @tc.desc: HandleFocusMoveSearchResult FIND_FAIL_IN_SCROLL with FOCUS_BY_TITLE/FOCUS_BY_LINK
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalHandleFocusMoveSearchResultTest006, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    auto targetNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    AceFocusMoveResult aceResult = AceFocusMoveResult::FIND_FAIL_IN_SCROLL;
    // aceResult FIND_FAIL_IN_SCROLL, type FOCUS_BY_TITLE
    param.type = Accessibility::FocusRuleType::FOCUS_BY_TITLE;
    auto finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL_IN_SCROLL);

    // aceResult FIND_FAIL_IN_SCROLL, type FOCUS_BY_LINK
    param.type = Accessibility::FocusRuleType::FOCUS_BY_LINK;
    finalResult = strategy.HandleFocusMoveSearchResult(param, targetNode, aceResult, info);
    ASSERT_EQ(finalResult, FocusMoveResultType::SEARCH_FAIL_IN_SCROLL);
}

/**
 * @tc.name: FocusStrategyOsalTest004
 * @tc.desc: ExecuteFocusMoveSearch
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalTest004, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    auto frameNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
    AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
        int64_t elementId = 0;
    Accessibility::AccessibilityFocusMoveParam param;
    Accessibility::AccessibilityElementInfo info;
    param.direction = FocusMoveDirection::BACKWARD;
    auto finalResult = strategy.ExecuteFocusMoveSearch(elementId, param, info);
    param.direction = FocusMoveDirection::FORWARD;
    finalResult = strategy.ExecuteFocusMoveSearch(elementId, param, info);
    param.direction = FocusMoveDirection::FIND_LAST;
    finalResult = strategy.ExecuteFocusMoveSearch(elementId, param, info);
    param.direction = FocusMoveDirection::DIRECTION_INVALID;
    finalResult = strategy.ExecuteFocusMoveSearch(elementId, param, info);
    ASSERT_EQ(finalResult.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
}

/**
 * @tc.name: DetectParentRulesCheckNodeTest001
 * @tc.desc: ExecuteFocusMoveSearch
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, DetectParentRulesCheckNodeTest001, TestSize.Level1)
{
    auto baseNode = NG::FrameNode::CreateFrameNode("framenode", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::Pattern>(), false);
    ASSERT_NE(baseNode, nullptr);
    AccessibilityElementInfo nodeInfo;
    AccessibleAction action(ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, "ace");
    AccessibleAction action1(ActionType::ACCESSIBILITY_ACTION_INVALID, "ace");
    nodeInfo.AddAction(action);
    nodeInfo.AddAction(action1);
    auto checkNode = std::make_shared<DetectParentRulesCheckNode>(nodeInfo, baseNode);

    Accessibility::PropValue value;
    auto result = checkNode->GetPropActionNames(value);
    EXPECT_EQ(result, true);
    auto findRet = value.valueArray.find("clearAccessibilityFocus");
    EXPECT_TRUE(findRet == value.valueArray.end());
    findRet = value.valueArray.find("accessibilityFocus");
    EXPECT_TRUE(findRet != value.valueArray.end());
}

/**
 * @tc.name: FocusMoveSearchWithCondition001
 * @tc.desc: ExecuteFocusMoveSearch
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusMoveSearchWithCondition001, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);
    MockAccessibilityElementOperatorCallback operatorCallback;

    //  GetPipelineByWindowId fail
    auto windowId = context->GetWindowId();
    auto elementId = 0;
    Accessibility::AccessibilityFocusMoveParam param;
    int32_t requestId = 1;
    Accessibility::AccessibilityElementInfo info;
    info.SetAccessibilityId(elementId);
    jsAccessibilityManager->FocusMoveSearchWithCondition(info, param, requestId, operatorCallback, windowId + 1);
    EXPECT_EQ(operatorCallback.mockResult_.resultType, FocusMoveResultType::SEARCH_FAIL);
}

/**
 * @tc.name: FocusMoveSearchWithCondition002
 * @tc.desc: IsProcessGetScrollAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusMoveSearchWithCondition002, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);
    MockAccessibilityElementOperatorCallback operatorCallback;

    //  ProcessGetScrollAncestor no infos
    auto windowId = context->GetWindowId();
    jsAccessibilityManager->SetWindowId(windowId);
    auto elementId = 0;
    Accessibility::AccessibilityElementInfo info;
    info.SetAccessibilityId(elementId);
    Accessibility::AccessibilityFocusMoveParam param;
    int32_t requestId = 1;
    param.direction = FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR;
    jsAccessibilityManager->FocusMoveSearchWithCondition(info, param, requestId, operatorCallback, windowId);
    EXPECT_EQ(operatorCallback.mockResult_.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
}

/**
 * @tc.name: FocusMoveSearchWithCondition003
 * @tc.desc: IsProcessGetScrollAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusMoveSearchWithCondition003, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    ASSERT_NE(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    ASSERT_NE(rootNode, nullptr);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);
    rootNode->AddChild(frameNode);
    frameNode->MountToParent(rootNode);
    // parent is fowardScrollAncestor
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    accessibilityProperty->SetSpecificSupportActionCallback(
        [accessibilityPtr = AceType::WeakClaim(AceType::RawPtr(accessibilityProperty))]() {
        const auto& accessibilityProperty = accessibilityPtr.Upgrade();
        CHECK_NULL_VOID(accessibilityProperty);
        accessibilityProperty->AddSupportAction(AceAction::ACTION_SCROLL_FORWARD);
    });

    //  ProcessGetScrollAncestor have infos
    MockAccessibilityElementOperatorCallback operatorCallback;
    auto windowId = context->GetWindowId();
    jsAccessibilityManager->SetWindowId(windowId);
    auto elementId = frameNode1->GetAccessibilityId();
    Accessibility::AccessibilityElementInfo info;
    info.SetAccessibilityId(elementId);
    Accessibility::AccessibilityFocusMoveParam param;
    int32_t requestId = 1;
    param.direction = FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR;
    jsAccessibilityManager->FocusMoveSearchWithCondition(info, param, requestId, operatorCallback, windowId);
    EXPECT_EQ(operatorCallback.mockResult_.resultType, FocusMoveResultType::SEARCH_SUCCESS);
    rootNode->RemoveChild(frameNode);
}

/**
 * @tc.name: FocusMoveSearchWithCondition004
 * @tc.desc: IsProcessGetScrollAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusMoveSearchWithCondition004, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);
    MockAccessibilityElementOperatorCallback operatorCallback;

    //  ExecuteFocusMoveSearch no infos
    auto windowId = context->GetWindowId();
    jsAccessibilityManager->SetWindowId(windowId);
    auto elementId = 0;
    Accessibility::AccessibilityFocusMoveParam param;
    int32_t requestId = 1;
    param.direction = FocusMoveDirection::FORWARD;
    Accessibility::AccessibilityElementInfo info;
    info.SetAccessibilityId(elementId);
    jsAccessibilityManager->FocusMoveSearchWithCondition(info, param, requestId, operatorCallback, windowId);
    EXPECT_EQ(operatorCallback.mockResult_.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
}

/**
 * @tc.name: NeedChangeToReadableNodeThroughAncestorTest001
 * @tc.desc: NeedChangeToReadableNodeThroughAncestor
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, NeedChangeToReadableNodeThroughAncestorTest001, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());
    bool result;
    // frameNode -> frameNode1
    // 1 frameNode frameNode1 not readable not rootType, return true;
    result = strategy.NeedChangeToReadableNodeThroughAncestor(checkNode, targetNode);
    EXPECT_TRUE(result);
    // 2 frameNode1 not readable, frameNode rootType, return true;
    strategy.isRootTypeId_ = frameNode->GetAccessibilityId();
    result = strategy.NeedChangeToReadableNodeThroughAncestor(checkNode, targetNode);
    EXPECT_TRUE(result);
    // 3 frameNode1 not readable, frameNode readable, return true;
    strategy.isRootTypeId_ = -1;
    strategy.isReadableId_ = frameNode->GetAccessibilityId();
    result = strategy.NeedChangeToReadableNodeThroughAncestor(checkNode, targetNode);
    EXPECT_TRUE(result);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode->GetAccessibilityId(), targetNode->GetAccessibilityId());
}

/**
 * @tc.name: NeedChangeToReadableNodeThroughAncestorTest002
 * @tc.desc: NeedChangeToReadableNodeThroughAncestor passes FocusRuleType to readable check.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, NeedChangeToReadableNodeThroughAncestorTest002, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    strategy.isReadableId_ = frameNode->GetAccessibilityId();
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());

    auto result = strategy.NeedChangeToReadableNodeThroughAncestor(
        checkNode, targetNode, Accessibility::FocusRuleType::FOCUS_BY_TITLE);

    EXPECT_TRUE(result);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode->GetAccessibilityId(), targetNode->GetAccessibilityId());
    EXPECT_EQ(strategy.lastFocusRuleType_, Accessibility::FocusRuleType::FOCUS_BY_TITLE);
    EXPECT_EQ(strategy.checkIsReadableCount_, 2);
}

/**
 * @tc.name: NeedChangeToReadableNodeThroughAncestorTest003
 * @tc.desc: NeedChangeToReadableNodeThroughAncestor uses DEFAULT when FocusRuleType is omitted.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, NeedChangeToReadableNodeThroughAncestorTest003, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto frameNode1 = CreatFrameNode();
    ASSERT_NE(frameNode1, nullptr);
    frameNode->AddChild(frameNode1);
    frameNode1->MountToParent(frameNode);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    strategy.isReadableId_ = frameNode1->GetAccessibilityId();
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode1, frameNode1->GetAccessibilityId());

    auto result = strategy.NeedChangeToReadableNodeThroughAncestor(checkNode, targetNode);

    EXPECT_TRUE(result);
    ASSERT_NE(targetNode, nullptr);
    EXPECT_EQ(frameNode1->GetAccessibilityId(), targetNode->GetAccessibilityId());
    EXPECT_EQ(strategy.lastFocusRuleType_, Accessibility::FocusRuleType::DEFAULT);
    EXPECT_EQ(strategy.checkIsReadableCount_, 1);
}

/**
 * @tc.name: NeedChangeToReadableNodeThroughAncestorTest004
 * @tc.desc: NeedChangeToReadableNodeThroughAncestor returns false when readable rules are disabled.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, NeedChangeToReadableNodeThroughAncestorTest004, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    ASSERT_NE(checkNode, nullptr);

    MockFocusStrategyOsal strategy(jsAccessibilityManager);
    strategy.readableRulesEnable_ = false;
    std::shared_ptr<FocusRulesCheckNode> targetNode;

    auto result = strategy.NeedChangeToReadableNodeThroughAncestor(
        checkNode, targetNode, Accessibility::FocusRuleType::FOCUS_BY_LINK);

    EXPECT_FALSE(result);
    EXPECT_EQ(targetNode, nullptr);
    EXPECT_EQ(strategy.checkIsReadableCount_, 0);
}

/**
 * @tc.name: FocusStrategyOsalCheckIsReadable001
 * @tc.desc: CheckIsReadable returns false when focus type does not match.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalCheckIsReadable001, TestSize.Level1)
{
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    FocusStrategyOsal strategy(jsAccessibilityManager);

    EXPECT_FALSE(strategy.CheckIsReadable(nullptr, Accessibility::FocusRuleType::FOCUS_BY_TITLE));

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(frameNode, frameNode->GetAccessibilityId());
    ASSERT_NE(checkNode, nullptr);

    EXPECT_FALSE(strategy.CheckIsReadable(checkNode, Accessibility::FocusRuleType::FOCUS_BY_LINK));
}

/**
 * @tc.name: FocusStrategyOsalNGDetectFocusRuleType001
 * @tc.desc: DetectElementInfoFocusableThroughAncestor fails when root node or base node is missing.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGDetectFocusRuleType001, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    Accessibility::AccessibilityElementInfo info;
    Accessibility::AccessibilityFocusMoveParam param;
    param.direction = FocusMoveDirection::DETECT_FOCUSABLE_IN_HOVER;
    param.type = Accessibility::FocusRuleType::FOCUS_BY_TITLE;
    std::list<Accessibility::AccessibilityElementInfo> targetInfos;

    FocusStrategyOsalNG nullContextStrategy(jsAccessibilityManager, nullptr, context);
    auto result = nullContextStrategy.DetectElementInfoFocusableThroughAncestor(info, param, targetInfos, 1);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL);
    EXPECT_TRUE(targetInfos.empty());

    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    param.parentId = -1;
    result = strategy.DetectElementInfoFocusableThroughAncestor(info, param, targetInfos, 1);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL);
    EXPECT_TRUE(targetInfos.empty());
}

/**
 * @tc.name: FocusStrategyOsalNGDetectFocusRuleType002
 * @tc.desc: DetectElementInfoFocusableThroughAncestor carries FocusRuleType into readable validation path.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGDetectFocusRuleType002, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    ASSERT_NE(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    ASSERT_NE(rootNode, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    rootNode->AddChild(frameNode);
    frameNode->MountToParent(rootNode);

    Accessibility::AccessibilityElementInfo info;
    info.SetAccessibilityId(frameNode->GetAccessibilityId());
    Accessibility::AccessibilityFocusMoveParam param;
    param.direction = FocusMoveDirection::DETECT_FOCUSABLE_IN_FOCUS_MOVE;
    param.parentId = frameNode->GetAccessibilityId();
    param.type = Accessibility::FocusRuleType::FOCUS_BY_LINK;
    std::list<Accessibility::AccessibilityElementInfo> targetInfos;

    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    auto result = strategy.DetectElementInfoFocusableThroughAncestor(info, param, targetInfos, context->GetWindowId());

    EXPECT_NE(result.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
    EXPECT_FALSE(targetInfos.empty());
    rootNode->RemoveChild(frameNode);
}

/**
 * @tc.name: FocusStrategyOsalNGCheckReadableToRoot001
 * @tc.desc: CheckAndGetReadableInfoToRoot handles null node and unmatched FocusRuleType.
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityFocusMoveTest, FocusStrategyOsalNGCheckReadableToRoot001, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto jsAccessibilityManager = AceType::MakeRefPtr<Framework::JsAccessibilityManager>();
    ASSERT_NE(jsAccessibilityManager, nullptr);
    jsAccessibilityManager->SetPipelineContext(context);
    FocusStrategyOsalNG strategy(jsAccessibilityManager, context, context);
    std::list<Accessibility::AccessibilityElementInfo> targetInfos;

    auto result = strategy.CheckAndGetReadableInfoToRoot(
        nullptr, targetInfos, context->GetWindowId(), Accessibility::FocusRuleType::FOCUS_BY_TITLE);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL);
    EXPECT_TRUE(targetInfos.empty());

    auto frameNode = CreatFrameNode();
    ASSERT_NE(frameNode, nullptr);
    result = strategy.CheckAndGetReadableInfoToRoot(
        frameNode, targetInfos, context->GetWindowId(), Accessibility::FocusRuleType::FOCUS_BY_LINK);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL);
    EXPECT_TRUE(targetInfos.empty());
}

namespace {
constexpr int64_t FOCUS_DUMP_HOST_ID = 100;

class FocusDumpChildCallback : public AccessibilityChildTreeCallback {
public:
    FocusDumpChildCallback() : AccessibilityChildTreeCallback(FOCUS_DUMP_HOST_ID) {}
    bool OnRegister(uint32_t, int32_t) override { return true; }
    bool OnDeregister() override { return true; }
    bool OnSetChildTree(int32_t, int32_t) override { return true; }
    void OnClearRegisterFlag() override {}
    bool OnDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {
        received = params;
        info.emplace_back("third focus dispatch");
        return true;
    }
    std::vector<std::string> received;
};

class FocusTreeProvider : public MockOhAccessibilityProvider {
    DECLARE_ACE_TYPE(FocusTreeProvider, MockOhAccessibilityProvider);
public:
    std::map<int64_t, ArkUI_AccessibilityElementInfo> nodes;
    std::vector<int64_t> extraRoots;
    std::map<int64_t, size_t> queries;
    std::set<int64_t> emptyIds;
    std::set<int64_t> duplicateIds;
    std::map<int64_t, int64_t> wrongIds;
    int32_t FindAccessibilityNodeInfosById(int64_t id, int32_t mode, int32_t requestId,
        std::vector<ArkUI_AccessibilityElementInfo>& infos) override
    {
        ++queries[id];
        if (emptyIds.count(id)) {
            return 0;
        }
        auto it = nodes.find(id == -1 ? 100 : id);
        if (it == nodes.end()) {
            return -1;
        }
        infos.push_back(it->second);
        if (wrongIds.count(id)) {
            infos.back().SetElementId(wrongIds.at(id));
        }
        if (duplicateIds.count(id)) {
            infos.push_back(it->second);
            infos.back().SetContents("conflicting payload");
        }
        if (id == -1) {
            for (auto rootId : extraRoots) {
                infos.push_back(nodes.at(rootId));
            }
        }
        return 0;
    }
    void Add(int64_t id, int64_t parent, std::initializer_list<int64_t> children)
    {
        auto& node = nodes[id];
        node.SetElementId(id);
        node.SetParentId(parent);
        node.SetVisible(true);
        for (auto child : children) {
            node.AddChildNodeId(child);
        }
    }
};
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdParentMapRepairsIds, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -2100000, {101, 102});
    provider->Add(101, -1, {});
    provider->Add(102, 999, {103});
    provider->Add(103, 103, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(103));
    auto node = strategy.GetCurrentCheckNode();
    AccessibilityElementInfo output;
    output.SetValidElement(false);
    ASSERT_TRUE(strategy.UpdateElementInfo(node, output));
    EXPECT_TRUE(output.IsValidElement());
    EXPECT_EQ(output.GetParentNodeId(), 102);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), 102);
    auto root = parent->GetAceParent();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), 100);
    EXPECT_EQ(root->GetAceParent(), nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    EXPECT_EQ(children[0]->GetAceParent()->GetAccessibilityId(), 100);
    EXPECT_EQ(children[1]->GetAccessibilityId(), 102);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdParentMapPrunesCyclesAndDuplicates, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, 101, {100, 101, 101, 999});
    provider->Add(101, 100, {100, 101});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(-1));
    auto root = strategy.GetCurrentCheckNode();
    EXPECT_EQ(root->GetAceParent(), nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 1);
    EXPECT_TRUE(children.front()->GetAceChildren().empty());
    AccessibilityFocusStrategy search;
    std::shared_ptr<FocusRulesCheckNode> target;
    EXPECT_EQ(search.FindLastNodeWithoutCheck({}, root, target), AceFocusMoveResult::FIND_SUCCESS);
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->GetAccessibilityId(), 101);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdFailedLookupClearsOrigin, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -2100000, {101});
    provider->Add(101, 100, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(999));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
    AccessibilityFocusMoveParam param {};
    param.direction = FocusMoveDirection::FORWARD;
    param.condition = DetailCondition::BYPASS_SELF;
    param.type = FocusRuleType::FOCUS_BY_LINK;
    AccessibilityElementInfo output;
    auto result = strategy.ExecuteFocusMoveSearch(999, param, output);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
    EXPECT_TRUE(result.needTerminate);
    EXPECT_FALSE(output.IsValidElement());
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    provider->nodes.erase(100);
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(101));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdParentMapRefreshesAfterMutation, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -2100000, {101});
    provider->Add(101, 100, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    provider->nodes.clear();
    provider->Add(100, -2100000, {102});
    provider->Add(102, -1, {});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(101));
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(102));
    EXPECT_EQ(strategy.GetCurrentCheckNode()->GetAceParent()->GetAccessibilityId(), 100);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdVisibilityAndRootHandoff, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -2100000, {101});
    provider->Add(101, 100, {});
    provider->nodes[101].SetVisible(false);
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    NodeConfig config;
    config.belongTreeId = 7;
    FocusStrategyOsalThird strategy(manager, provider, config);
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    EXPECT_FALSE(strategy.GetCurrentCheckNode()->IsAccessibiltyVisible());
    AccessibilityFocusStrategy search(FocusRuleType::FOCUS_BY_TITLE);
    EXPECT_FALSE(search.CanAccessibilityFocus(strategy.GetCurrentCheckNode()));
    AccessibilityFocusMoveParam param {};
    param.direction = FocusMoveDirection::FORWARD;
    param.type = FocusRuleType::FOCUS_BY_TITLE;
    AccessibilityElementInfo output;
    EXPECT_EQ(strategy.HandleFocusMoveSearchResult(param, nullptr,
        AceFocusMoveResult::FIND_FAIL_IN_CHILDTREE, output), FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE);
    EXPECT_EQ(output.GetBelongTreeId(), 7);
    int64_t id = -1;
    int32_t tree = -1;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(output.GetAccessibilityId(), id, tree);
    EXPECT_EQ(id, 100);
    EXPECT_EQ(tree, 7);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdMultipleRootsKeepFirstAndRejectDetachedOrigin, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, AccessibilityElementInfo::ROOT_PARENT_ID, {101});
    provider->Add(101, 100, {});
    provider->Add(200, AccessibilityElementInfo::ROOT_PARENT_ID, {201});
    provider->Add(201, 200, {});
    provider->extraRoots = {200};
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(-1));
    auto root = strategy.GetCurrentCheckNode();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), 100);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 1);
    EXPECT_EQ(children.front()->GetAccessibilityId(), 101);

    AccessibilityFocusMoveParam param {};
    param.direction = FocusMoveDirection::FORWARD;
    param.condition = DetailCondition::BYPASS_SELF;
    param.type = FocusRuleType::FOCUS_BY_TITLE;
    for (auto id : {200, 201}) {
        AccessibilityElementInfo output;
        auto result = strategy.ExecuteFocusMoveSearch(id, param, output);
        EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
        EXPECT_TRUE(result.needTerminate);
        EXPECT_FALSE(output.IsValidElement());
    }
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdFocusDumpParameters, TestSize.Level1)
{
    DumpInfoArgument argument;
    EXPECT_TRUE(AccessibilityHidumper::ParseThirdFocusSearch(
        {"-accessibility", "--third-focus-search", "123", "16", "3", "1"}, argument));
    EXPECT_EQ(argument.nodeId, 123);
    EXPECT_EQ(argument.focusMoveRule, 3);
    EXPECT_FALSE(AccessibilityHidumper::ParseThirdFocusSearch(
        {"-accessibility", "--third-focus-search", "123", "1024", "3", "1"}, argument));
    EXPECT_FALSE(AccessibilityHidumper::ParseThirdFocusSearch(
        {"-accessibility", "--third-focus-search", "123x", "16", "3", "1"}, argument));
}

HWTEST_F(AccessibilityFocusMoveTest, FocusDumpNativeAndMainTreeIds, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->SetPipelineContext(context);
    manager->SetWindowId(context->GetWindowId());
    manager->treeId_ = 7;
    auto root = CreatFrameNode();
    auto child = CreatFrameNode();
    root->AddChild(child);
    auto previousRoot = context->rootNode_;
    context->rootNode_ = root;
    int64_t splicedId = root->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(7, splicedId);
    int64_t expectedId = child->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(7, expectedId);
    for (auto sourceId : {root->GetAccessibilityId(), splicedId}) {
        auto stream = std::make_unique<std::ostringstream>();
        auto* output = stream.get();
        DumpLog::GetInstance().SetDumpFile(std::move(stream));
        // FIND_LAST exercises native traversal without requiring screen-reader rule installation.
        manager->OnDumpInfoNG({"-accessibility", "--third-focus-search", std::to_string(sourceId), "64", "3", "1"},
            context->GetWindowId());
        EXPECT_NE(output->str().find("FocusMove result=0"), std::string::npos);
        EXPECT_NE(output->str().find("id=" + std::to_string(expectedId)), std::string::npos);
        EXPECT_EQ(output->str().find("Error:"), std::string::npos);
    }
    context->rootNode_ = previousRoot;
}

HWTEST_F(AccessibilityFocusMoveTest, FocusDumpChildTreeDispatchAndUnknownTree, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->treeId_ = 7;
    auto childCallback = std::make_shared<FocusDumpChildCallback>();
    childCallback->SetChildTreeId(8);
    manager->childTreeCallbackMap_[FOCUS_DUMP_HOST_ID] = childCallback;
    int64_t childId = 101;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(8, childId);
    std::vector<std::string> params {
        "-accessibility", "--third-focus-search", std::to_string(childId), "16", "2", "1"};
    auto stream = std::make_unique<std::ostringstream>();
    auto* output = stream.get();
    DumpLog::GetInstance().SetDumpFile(std::move(stream));
    manager->OnDumpInfoNG(params, 0);
    EXPECT_EQ(childCallback->received, params);
    EXPECT_NE(output->str().find("third focus dispatch"), std::string::npos);
    manager->childTreeCallbackMap_.clear();
    output->str("");
    manager->OnDumpInfoNG(params, 0);
    EXPECT_NE(output->str().find("child tree is not registered: 8"), std::string::npos);
    EXPECT_EQ(output->str().find("FocusMove result="), std::string::npos);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningSnapshotAndSanitizedEdges, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, 101, {100, 101, 101, 102});
    provider->Add(101, -1, {100, 102});
    provider->Add(102, 999, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    const auto queries = provider->queries;
    EXPECT_EQ(queries.at(-1), 1);
    EXPECT_EQ(queries.at(101), 1);
    EXPECT_EQ(queries.at(102), 1);
    provider->nodes.clear(); // No callbacks are allowed to replace the in-flight snapshot.
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), 100);
    EXPECT_EQ(root->GetAceChildren().size(), 2);
    EXPECT_TRUE(strategy.GetCurrentCheckNode()->GetAceChildren().empty());
    AccessibilityElementInfo output;
    ASSERT_TRUE(strategy.UpdateElementInfo(root, output));
    EXPECT_EQ(output.GetChildIds(), (std::vector<int64_t> {101, 102}));
    EXPECT_EQ(provider->queries, queries);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningDepthBoundaryAndFailure, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    const int64_t end = 100 + ThirdAccessibilityTraversal::MAX_DEPTH;
    for (int64_t id = 100; id <= end; ++id) {
        provider->Add(id, id - 1, {});
        if (id < end) {
            provider->nodes[id].AddChildNodeId(id + 1);
        }
    }
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(end));
    provider->nodes[end].AddChildNodeId(end + 1);
    provider->Add(end + 1, end, {});
    AccessibilityFocusMoveParam param {};
    param.direction = FocusMoveDirection::FIND_LAST;
    AccessibilityElementInfo output;
    auto result = strategy.ExecuteFocusMoveSearch(-1, param, output);
    EXPECT_EQ(result.resultType, FocusMoveResultType::SEARCH_FAIL_LOST_NODE);
    EXPECT_TRUE(result.needTerminate);
    EXPECT_FALSE(output.IsValidElement());
    EXPECT_EQ(strategy.ChangeToRoot(), nullptr);
    EXPECT_EQ(provider->queries.count(end + 1), 0);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningWideTreeAndNodeLimit, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {});
    for (size_t i = 1; i < ThirdAccessibilityTraversal::MAX_NODES; ++i) {
        const auto id = static_cast<int64_t>(100 + i);
        provider->nodes[100].AddChildNodeId(id);
        provider->Add(id, 100, {});
        provider->nodes[id].SetVisible(false);
    }
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(-1));
    auto children = strategy.GetCurrentCheckNode()->GetAceChildren();
    ASSERT_EQ(children.size(), ThirdAccessibilityTraversal::MAX_NODES - 1);
    AccessibilityFocusStrategy search;
    std::shared_ptr<FocusRulesCheckNode> target;
    AceFocusMoveDetailCondition condition { .bypassSelf = false, .bypassDescendants = false };
    EXPECT_EQ(search.FindNextReadableNodeBySelfAndSameLevel(
        condition, children.front(), children, target, nullptr), AceFocusMoveResult::FIND_FAIL);
    EXPECT_EQ(target, nullptr);
    provider->nodes[100].AddChildNodeId(20000);
    provider->Add(20000, 100, {});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(-1));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningRepeatedEdgesConsumeBudget, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101, 102});
    provider->Add(101, 100, {});
    provider->Add(102, 100, {});
    for (size_t i = 0; i < ThirdAccessibilityTraversal::MAX_EDGES / 2; ++i) {
        provider->nodes[101].AddChildNodeId(100);
        provider->nodes[102].AddChildNodeId(100);
    }
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(-1));
    EXPECT_EQ(provider->queries.size(), 3);
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningHoverSkipsCycleAndMissingSibling, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101, 999, 100});
    provider->Add(101, 100, {});
    for (auto& [id, node] : provider->nodes) {
        node.SetRect({0, 0, 100, 100});
        node.SetContents("readable");
        node.SetAccessibilityLevel("yes");
    }
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->SetPipelineContext(NG::PipelineContext::GetCurrentContext());
    auto host = NG::FrameNode::CreateFrameNode("XComponent", 200, AceType::MakeRefPtr<NG::Pattern>());
    auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, host);
    operation->SetBelongTreeId(7);
    manager->RegisterJsThirdProviderInteractionOperation(200, operation);
    AccessibilityElementInfo root;
    ASSERT_TRUE(manager->GetElementInfoForThird(-1, root, 200));
    auto path = manager->HoverPathForThird(200, NG::PointF(1, 1), root, {});
    ASSERT_FALSE(path.empty());
    int64_t expected = 101;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(7, expected);
    EXPECT_EQ(path.back(), expected);
    EXPECT_EQ(provider->queries[999], 1);
    EXPECT_EQ(provider->queries.count(100), 0);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningDumpTerminatesAndReportsMalformedTree, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101, 101, 100, 999});
    provider->Add(101, 100, {100});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->SetPipelineContext(NG::PipelineContext::GetCurrentContext());
    auto host = NG::FrameNode::CreateFrameNode("XComponent", 200, AceType::MakeRefPtr<NG::Pattern>());
    auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, host);
    operation->SetBelongTreeId(7);
    manager->RegisterJsThirdProviderInteractionOperation(200, operation);
    int64_t rootId = 100;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(7, rootId);
    auto stream = std::make_unique<std::ostringstream>();
    auto* output = stream.get();
    DumpLog::GetInstance().SetDumpFile(std::move(stream));
    std::vector<std::string> info;
    EXPECT_TRUE(manager->OnDumpChildInfoForThirdRecursive(200,
        {"-accessibility", "--root", std::to_string(rootId)}, info, manager));
    EXPECT_NE(output->str().find("dump is incomplete"), std::string::npos);
    EXPECT_EQ(provider->queries[100], 1);
    EXPECT_EQ(provider->queries[101], 1);
    EXPECT_EQ(provider->queries[999], 1);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningFailedHoverAndUnroutableSync, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101});
    provider->Add(101, 100, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->SetPipelineContext(NG::PipelineContext::GetCurrentContext());
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    int64_t target = -1;
    EXPECT_FALSE(strategy.CanSendHoverWithTargetIdByReadableRules(999, target));
    auto host = NG::FrameNode::CreateFrameNode("XComponent", 200, AceType::MakeRefPtr<NG::Pattern>());
    auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, host);
    operation->SetBelongTreeId(0);
    AccessibilityFocusMoveParam param {};
    param.direction = FocusMoveDirection::FIND_LAST;
    AccessibilityElementInfo output;
    auto result = operation->SearchFocusMoveWithConditionSync(-1, param, output);
    EXPECT_EQ(result.resultType, FocusMoveResultType::NOT_SUPPORT);
    EXPECT_TRUE(result.needTerminate);
    EXPECT_FALSE(output.IsValidElement());
    operation->SetBelongTreeId(7);
    AccessibilityElementInfo source;
    int64_t foreignId = 101;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(8, foreignId);
    source.SetAccessibilityId(foreignId);
    MockAccessibilityElementOperatorCallback callback;
    provider->queries.clear();
    operation->FocusMoveSearchWithCondition(source, param, 42, callback);
    EXPECT_EQ(callback.mockResult_.resultType, FocusMoveResultType::NOT_SUPPORT);
    EXPECT_TRUE(callback.mockResult_.needTerminate);
    EXPECT_EQ(callback.mockRequestId, 42);
    EXPECT_TRUE(provider->queries.empty());
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningPayloadLimitAndOwnedActions, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101});
    provider->Add(101, 100, {});
    std::string description = "original action";
    provider->nodes[101].AddOperationAction({
        ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_CLICK, description.c_str()});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(101));
    auto node = std::static_pointer_cast<ThirdRulesCheckNode>(strategy.GetCurrentCheckNode());
    description[0] = 'X';
    ASSERT_EQ(node->GetNodeInfo()->GetOperationActions().size(), 1);
    EXPECT_STREQ(node->GetNodeInfo()->GetOperationActions()[0].description, "original action");
    provider->nodes[101].SetContents(std::string(ThirdAccessibilityTraversal::MAX_BYTES, 'x'));
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(-1));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningConcurrentPropertyUpdates, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->Add(100, -1, {101});
    provider->Add(101, 100, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    manager->SetPipelineContext(NG::PipelineContext::GetCurrentContext());
    auto host = NG::FrameNode::CreateFrameNode("XComponent", 200, AceType::MakeRefPtr<NG::Pattern>());
    auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, host);
    operation->SetBelongTreeId(7);
    auto property = AceType::MakeRefPtr<NG::CustomAccessibilityProperty>();
    property->SetAccessibilityText("override");
    std::atomic<bool> start { false };
    std::thread writer([&start, operation, property] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (size_t i = 0; i < 2000; ++i) {
            operation->SetThirdCustomProperty(101, property);
            operation->RemoveThirdCustomProperty(101);
            operation->SetThirdCustomProperty(101, property);
            operation->ClearAllThirdCustomProperties();
        }
    });
    start.store(true);
    for (size_t i = 0; i < 2000; ++i) {
        auto current = operation->GetThirdCustomProperty(101);
        if (current) {
            EXPECT_EQ(current->GetAccessibilityText(), "override");
        }
        std::list<AccessibilityElementInfo> infos;
        EXPECT_TRUE(operation->FindAccessibilityNodeInfosByIdFromProvider(101, 0, 0, infos));
        if (!infos.empty()) {
            auto text = infos.front().GetAccessibilityText();
            EXPECT_TRUE(text.empty() || text == "override");
        }
    }
    writer.join();
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningConcurrentProviderRegistration, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto host = NG::FrameNode::CreateFrameNode("XComponent", 200, AceType::MakeRefPtr<NG::Pattern>());
    auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, host);
    std::atomic<bool> start { false };
    std::thread writer([&start, manager, operation] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (size_t i = 0; i < 2000; ++i) {
            manager->RegisterJsThirdProviderInteractionOperation(200, operation);
            manager->DeregisterJsThirdProviderInteractionOperation(200);
        }
    });
    start.store(true);
    for (size_t i = 0; i < 2000; ++i) {
        auto current = manager->GetJsThirdProviderInteractionOperation(200).lock();
        EXPECT_TRUE(!current || current == operation);
        EXPECT_TRUE(manager->GetJsThirdProviderInteractionOperation(1000 + i).expired());
    }
    writer.join();
    EXPECT_TRUE(manager->jsThirdProviderOperator_.empty());
}

namespace {
class QueuedThirdHoverExecutor : public MockTaskExecutor {
public:
    mutable std::vector<Task> queued;
    bool accept = true;
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW,
        VsyncBarrierOption barrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (!accept) {
            return false;
        }
        queued.emplace_back(std::move(task));
        return true;
    }
};
}

HWTEST_F(AccessibilityFocusMoveTest, ThirdHardeningHoverWorkerReservation, TestSize.Level1)
{
    auto context = NG::MockPipelineContext::GetCurrentContext();
    auto previousExecutor = context->taskExecutor_;
    auto executor = AceType::MakeRefPtr<QueuedThirdHoverExecutor>();
    context->taskExecutor_ = executor;
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    AccessibilityHoverForThirdConfig config;
    config.context = context;
    config.hostElementId = 200;
    manager->HandleAccessibilityHoverForThird(config);
    EXPECT_FALSE(manager->hoverForThirdState_.thirdOperationIdle.load());
    manager->HandleAccessibilityHoverForThird(config);
    EXPECT_EQ(executor->queued.size(), 1);
    if (!executor->queued.empty()) {
        auto task = std::move(executor->queued.front());
        executor->queued.clear();
        task(); // Missing provider exercises the early failure path.
    }
    EXPECT_TRUE(manager->hoverForThirdState_.thirdOperationIdle.load());
    executor->accept = false;
    manager->HandleAccessibilityHoverForThird(config);
    EXPECT_TRUE(manager->hoverForThirdState_.thirdOperationIdle.load());
    context->taskExecutor_ = previousExecutor;
}

} // namespace OHOS::Ace::NG

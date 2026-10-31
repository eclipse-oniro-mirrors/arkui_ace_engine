/*
 * Copyright (c) 2023-2024 iSoftStone Information Technology (Group) Co.,Ltd.
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
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "securec.h"

#define protected public
#define private public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/rect.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/inspector_tree_provider_registry.h"
#include "core/components_ng/base/simplified_inspector.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "base/json/json_util.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
std::string key = "key";
const std::string TEXT_NODE_TYPE = "Text";
const std::string TEST_TEXT = "SomeText";
constexpr char INSPECTOR_MSG_NEED_FREE_NODES_TRUE[] =
    "{\"method\":\"ArkUI.tree\", \"params\":{\"windowId\":\"10\", \"isNeedFreeNodes\":true}}";
constexpr char INSPECTOR_MSG_NEED_FREE_NODES_FALSE[] =
    "{\"method\":\"ArkUI.tree\", \"params\":{\"windowId\":\"10\", \"isNeedFreeNodes\":false}}";
constexpr char INSPECTOR_MSG_NO_NEED_FREE_NODES_PARAM[] =
    "{\"method\":\"ArkUI.tree\", \"params\":{\"windowId\":\"10\"}}";
constexpr char INSPECTOR_MSG_INVALID[] = "invalid message";
constexpr char INSPECTOR_MSG_INVALID_PARAMS[] =
    "{\"method\":\"ArkUI.tree\", \"paramss\":{\"windowId\":\"10\", \"isNeedFreeNodes\":true}}";
const char INSPECTOR_TYPE[] = "$type";
const char INSPECTOR_ID[] = "$ID";
const char INSPECTOR_DEBUGLINE[] = "$debugLine";
const char INSPECTOR_ATTRS[] = "$attrs";
const char INSPECTOR_CHILDREN[] = "$children";
const char INSPECTOR_OTHER_CONTENTS[] = "other_contents";
const char STAGE_NODE_TAG[] = "stage";
const char PAGE_NODE_TAG[] = "page";
const char CHILD_NODE_TAG[] = "child";
const char FREE_NODE_TAG[] = "free_node";
const char MAIN_TREE_NODE_TAG[] = "main_tree_node";
constexpr int32_t EMPTY_OTHER_CONTENTS_SIZE = 0;

class InspectorTestNode : public UINode {
    DECLARE_ACE_TYPE(InspectorTestNode, UINode);

public:
    static RefPtr<InspectorTestNode> CreateTestNode(int32_t nodeId)
    {
        auto node = MakeRefPtr<InspectorTestNode>(nodeId);
        return node;
    }

    bool IsAtomicNode() const override
    {
        return true;
    }

    explicit InspectorTestNode(int32_t nodeId) : UINode("InspectorTestNode", nodeId) {}
    ~InspectorTestNode() override = default;
};
}; // namespace

class MockStageManager : public StageManager {
public:
    explicit MockStageManager(const RefPtr<FrameNode>& stage)
        : StageManager(stage)
    {}

    ~MockStageManager() override = default;

    MOCK_METHOD(RefPtr<FrameNode>, GetLastPage, (), (const));
};

class MockOverlayManager : public OverlayManager {
public:
    explicit MockOverlayManager(const RefPtr<FrameNode>& rootNode)
        : OverlayManager(rootNode)
    {}

    ~MockOverlayManager() override = default;

    MOCK_METHOD(const WeakPtr<UINode>, GetRootNode, (), (const));
};

class InspectorTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: InspectorTestNg001
 * @tc.desc: Test the operation of Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg001, TestSize.Level1)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> TWO = FrameNode::CreateFrameNode("two", id2, AceType::MakeRefPtr<Pattern>());
    /**
     * @tc.steps: step1. callback GetFrameNodeByKey
     * @tc.expected: expect the function is run ok
     */
    auto nodePtr = Inspector::GetFrameNodeByKey(key);
    EXPECT_EQ(nodePtr, nullptr);

    /**
     * @tc.steps: step2. callback SetUp
     * @tc.expected: expect context1 is not null
     */
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    auto nodePtr1 = Inspector::GetFrameNodeByKey(key);
    EXPECT_EQ(nodePtr1, nullptr);

    /**
     * @tc.steps: step3. callback GetFrameNodeByKey
     * @tc.expected: expect nodePtr2 not null
     */
    context1->rootNode_ = ONE;
    context1->rootNode_->AddChild(TWO);
    auto rootNode = context1->GetRootElement();
    ASSERT_NE(rootNode, nullptr);
    auto nodePtr2 = Inspector::GetFrameNodeByKey(key);
    EXPECT_EQ(nodePtr2, nullptr);

    auto nodePtr3 = Inspector::GetFrameNodeByKey("");
    ASSERT_NE(nodePtr3, nullptr);
}

/**
 * @tc.name: InspectorTestNg002
 * @tc.desc: Test the operation of Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. callback GetInspectorNodeByKey
     * @tc.expected: expect the function is run ok
     */
    auto test = Inspector::GetInspectorNodeByKey(key);
    EXPECT_EQ(test, "");

    /**
     * @tc.steps: step2. callback SetUp
     * @tc.expected: expect test1 is "".
     */
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    auto test1 = Inspector::GetInspectorNodeByKey(key);
    EXPECT_EQ(test1, "");

    /**
     * @tc.steps: step1. callback rootNode_
     * @tc.expected: expect the function GetInspectorNodeByKey is return null
     */
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    context1->rootNode_ = ONE;
    ASSERT_NE(context1, nullptr);
    auto test2 = Inspector::GetInspectorNodeByKey(key);
    EXPECT_EQ(test2, "");
    /**
     * @tc.steps: step3. callback GetInspectorNodeByKey
     * @tc.expected: expect nodePtr2 not null
     */
    auto test3 = Inspector::GetInspectorNodeByKey("");
}

/**
 * @tc.name: InspectorTestNg003
 * @tc.desc: Test the operation of Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. callback GetInspector
     * @tc.expected: expect the function is run ok
     */
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(ONE);
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> TWO = FrameNode::CreateFrameNode("stage", id2, AceType::MakeRefPtr<Pattern>());
    ONE->AddChild(TWO);
    auto id3 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> THREE = FrameNode::CreateFrameNode("three", id3, AceType::MakeRefPtr<Pattern>());
    THREE->AddChild(ONE);

    /**
     * @tc.steps: step2. call GetInspector
     * @tc.expected: the return value is empty string
     */
    auto test1 = Inspector::GetInspector(false);
    EXPECT_NE(test1, "");
    auto test3 = Inspector::GetInspector(true);
    EXPECT_NE(test3, "");
    auto test4 = Inspector::GetInspectorOfNode(TWO);
    EXPECT_NE(test4, "");

    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    auto jsonRoot = JsonUtil::Create(true);
    const char inspectorType[] = "$type";
    const char inspectorRoot[] = "root";
    jsonRoot->Put(inspectorType, inspectorRoot);
    auto test5 = Inspector::GetInspector(false);
    EXPECT_EQ(test5, jsonRoot->ToString());
    MockPipelineContext::pipeline_ = pipeline_bak;

    InspectorFilter filter;
    std::string testId = "test";
    filter.SetFilterID(testId);
    bool needThrow = false;
    auto test6 = Inspector::GetInspector(false, filter, needThrow);
    auto rootNode = context1->GetStageManager()->GetLastPage();
    if (rootNode == nullptr) {
        EXPECT_EQ(test6, jsonRoot->ToString());
    } else {
        EXPECT_NE(test6, "");
    }
    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg004
 * @tc.desc: Test the operation of View Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set rootNode and taskExecutor call SendEventByKey
     * @tc.expected: expect return true
     */
    auto context = NG::PipelineContext::GetCurrentContext();
    context->rootNode_ = FrameNode::CreateFrameNode("frameNode", 60, AceType::MakeRefPtr<Pattern>(), true);
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto test3 = Inspector::SendEventByKey("", static_cast<int>(AceAction::ACTION_CLICK), "params");
    EXPECT_EQ(test3, true);
    test3 = Inspector::SendEventByKey("", static_cast<int>(AceAction::ACTION_LONG_CLICK), "params");
    EXPECT_EQ(test3, true);
    test3 = Inspector::SendEventByKey("", 31, "params");
    EXPECT_EQ(test3, true);
}

/**
 * @tc.name: InspectorTestNg005
 * @tc.desc: Test the operation of View Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. callback HideAllMenus
     * @tc.expected: expect the function is run ok
     */
    Inspector::HideAllMenus();
    auto context = PipelineContext::GetCurrentContext();

    /**
     * @tc.steps: step2. callback SetUp
     * @tc.expected: step2. expect context1 is return null.
     */
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    Inspector::HideAllMenus();
}

/**
 * @tc.name: InspectorTestNg006
 * @tc.desc: Test the operation of Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. callback GetInspector
     * @tc.expected: expect the function is run ok
     */
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(ONE);

    /**
     * @tc.steps: step2 add child to two and create a temp node with tag "temp"
                call GetInspector
     * @tc.expected: the return value is empty string
     */
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> TWO = FrameNode::CreateFrameNode("two", id2, AceType::MakeRefPtr<Pattern>());
    ONE->children_.clear();
    ONE->AddChild(TWO);
    auto stageParent = FrameNode::CreateFrameNode("stageParent", 5, AceType::MakeRefPtr<Pattern>(), true);
    stageParent->AddChild(ONE);

    const RefPtr<FrameNode> THREE = FrameNode::CreateFrameNode(
        "three", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    const RefPtr<FrameNode> STAGE = FrameNode::CreateFrameNode(
        "stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    const RefPtr<FrameNode> PAGE = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    THREE->isInternal_ = true;
    TWO->AddChild(PAGE);
    TWO->AddChild(THREE);
    TWO->AddChild(STAGE);
    auto temp = FrameNode::CreateFrameNode("temp", 5, AceType::MakeRefPtr<Pattern>(), true);
    STAGE->AddChild(temp);
    ONE->tag_ = "stage";
    auto test1 = Inspector::GetInspector(false);
    EXPECT_NE(test1, "");

    PAGE->hostPageId_ = TWO->GetPageId();
    auto test2 = Inspector::GetInspector(false);
    EXPECT_NE(test2, "");

    PAGE->hostPageId_ = TWO->GetPageId() + 1;
    test2 = Inspector::GetInspector(false);
    EXPECT_NE(test2, "");

    TWO->children_.clear();
    auto test3 = Inspector::GetInspector(false);
    EXPECT_NE(test3, "");

    /**
     *  ONE(stageNode)--TWO(GetLastPage())--PAGE
     *                   |--THREE
     *                   |--STAGE--temp
     *                   |--frameNode
     *                   |--frameNode2--frameNode3
     */
    /**
     * @tc.steps: step3 add child to two and create a frame node tree
            call GetInspector
     * @tc.expected: the return value is not empty string
     */
    auto spanNode = SpanNode::GetOrCreateSpanNode(int32_t(191));
    TWO->AddChild(spanNode);

    auto frameNode = FrameNode::CreateFrameNode("frameNode", 6, AceType::MakeRefPtr<Pattern>(), true);
    TWO->AddChild(frameNode);
    frameNode->isActive_ = true;
    auto frameNode2 = FrameNode::CreateFrameNode("frameNode2", 62, AceType::MakeRefPtr<Pattern>(), true);
    TWO->AddChild(frameNode2);
    frameNode2->isActive_ = false;
    auto frameNode3 = FrameNode::CreateFrameNode("frameNode3", 63, AceType::MakeRefPtr<Pattern>(), true);
    frameNode2->AddChild(frameNode3);
    test3 = Inspector::GetInspector(false);
    EXPECT_NE(test3, "");
    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg007
 * @tc.desc: Test the operation of Inspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. callback GetInspector
     * @tc.expected: expect the function is run ok
     */
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(ONE);

    /**
     * @tc.steps: step2. callback pop_back
     * @tc.expected: expect clean children and pageRootNode is noll
     */
    auto pageRootNode = context1->GetStageManager()->GetLastPage();
    auto test = Inspector::GetInspector(false);
    auto str = "{\"$type\":\"root\",\"width\":\"720.000000\",\"height\":\"1280.000000\",\"$resolution\":\"1.000000\"}";
    EXPECT_EQ(test, str);
    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg008
 * @tc.desc: Test the GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat frameNode
     * @tc.expected: expect the function is run ok
     */
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    /**
     * @tc.steps: step2. assign value to rootNode_
     * @tc.expected: rootNode_ pass non-null check.
     */
    context->rootNode_ = ONE;
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());

    /**
     * @tc.steps: step3. construct assignments and call GetRectangleById.
     * @tc.expected: expect the GetRectangleById is run ok and result is expected.
     */
    OHOS::Ace::NG::Rectangle rect;
    string key = "";
    Inspector::GetRectangleById(key, rect);
    float zResult = 1.0f;
    EXPECT_EQ(rect.scale.z, zResult);
    context->rootNode_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg009
 * @tc.desc: Test the GetFrameNodeByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg009, TestSize.Level1)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> TWO = FrameNode::CreateFrameNode("two", id2, AceType::MakeRefPtr<Pattern>());
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto nodePtr = Inspector::GetFrameNodeByKey(key);
    EXPECT_EQ(nodePtr, nullptr);

    Inspector::AddOffscreenNode(ONE);
    nodePtr = Inspector::GetFrameNodeByKey("one");
    EXPECT_EQ(nodePtr, 0);

    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    context = PipelineContext::GetCurrentContext();
    EXPECT_EQ(context, nullptr);
    nodePtr = Inspector::GetFrameNodeByKey("two");
    EXPECT_EQ(nodePtr, nullptr);
    MockPipelineContext::pipeline_ = pipeline_bak;
}

/**
 * @tc.name: InspectorTestNg010
 * @tc.desc: Test the GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg010, TestSize.Level1)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->rootNode_ = ONE;
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());
    std::string key = "key";
    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById(key, rect);
    EXPECT_EQ(rect.size.Width(), 0.0f);

    auto frameNode = Inspector::GetFrameNodeByKey("one");
    RefPtr<RenderContext> renderContextBak = ONE->renderContext_;
    ONE->renderContext_ = nullptr;
    Inspector::GetRectangleById("one", rect);
    EXPECT_EQ(rect.screenRect.Width(), 0.0f);
    ONE->renderContext_ = renderContextBak;
    EXPECT_EQ(rect.size.Width(), 0.0f);
}

/**
 * @tc.name: InspectorTestNg011
 * @tc.desc: Test the operation of GetSubWindowInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg011, TestSize.Level1)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(ONE);
    auto pageRootNode = context->GetStageManager()->GetLastPage();
    auto test = Inspector::GetSubWindowInspector(false);
    auto str = "";
    EXPECT_NE(test, str);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg013
 * @tc.desc: Test the function of InspectorFilter
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg013, TestSize.Level1)
{
    const char* hello = "hi";
    InspectorFilter testFilter;
    EXPECT_EQ(testFilter.CheckFilterAttr(FixedAttrBit::FIXED_ATTR_CONTENT, hello), true);
    testFilter.SetFilterDepth(1);
    std::string id = "id";
    testFilter.SetFilterID(id);
    testFilter.filterExt.emplace_back("abc");
    testFilter.AddFilterAttr("focusable");
    testFilter.AddFilterAttr("abc");
    testFilter.AddFilterAttr("def");
    EXPECT_EQ(testFilter.CheckFixedAttr(FixedAttrBit::FIXED_ATTR_CONTENT), false);
    EXPECT_EQ(testFilter.CheckExtAttr("zzz"), false);
    EXPECT_EQ(testFilter.CheckFilterAttr(FixedAttrBit::FIXED_ATTR_CONTENT, hello), false);
    EXPECT_EQ(testFilter.IsFastFilter(), false);
}

/**
 * @tc.name: InspectorTestNg014
 * @tc.desc: Test the function of InspectorFilter
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg014, TestSize.Level1)
{
    const char* hello = "hi";
    InspectorFilter testFilter;
    testFilter.AddFilterAttr("focusable");
    EXPECT_EQ(testFilter.CheckFilterAttr(FixedAttrBit::FIXED_ATTR_FOCUSABLE, hello), true);
}

/**
 * @tc.name: InspectorTestNg015
 * @tc.desc: Test the function of InspectorFilter
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg015, TestSize.Level1)
{
    const char* hello = "abc";
    InspectorFilter testFilter;
    testFilter.filterExt.emplace_back("abc");
    testFilter.AddFilterAttr("focusable");
    EXPECT_EQ(testFilter.CheckFilterAttr(FixedAttrBit::FIXED_ATTR_CONTENT, hello), true);
}

/**
 * @tc.name: InspectorTestNg016
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg016, TestSize.Level1)
{
    // tc.steps: step1. callback GetInspectorTree
    // tc.expected: expect the function is run ok
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode("stage", id, AceType::MakeRefPtr<Pattern>(), true);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);

    // tc.steps: step2 add lastPage and create a frame node tree to lastPage
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> lastPage = FrameNode::CreateFrameNode("two", id2, AceType::MakeRefPtr<Pattern>());
    stageNode->children_.clear();
    stageNode->AddChild(lastPage);
    auto stageParent = FrameNode::CreateFrameNode("stageParent", 5, AceType::MakeRefPtr<Pattern>(), true);
    stageParent->AddChild(stageNode);
    auto frameNode = FrameNode::CreateFrameNode("frameNode", 6, AceType::MakeRefPtr<Pattern>(), true);
    lastPage->AddChild(frameNode);
    frameNode->isActive_ = true;
    auto frameNode2 = FrameNode::CreateFrameNode("frameNode2", 62, AceType::MakeRefPtr<Pattern>(), true);
    lastPage->AddChild(frameNode2);
    frameNode2->isActive_ = false;
    auto frameNode3 = FrameNode::CreateFrameNode("frameNode3", 63, AceType::MakeRefPtr<Pattern>(), true);
    frameNode2->AddChild(frameNode3);
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    EXPECT_TRUE(!treesInfos.empty());
    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg017
 * @tc.desc: Test the operation of GetFrameNodeByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg017, TestSize.Level1)
{
    // tc.steps: step1. add frameNode to arkui tree with inspector id text1
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    std::string inspectorId = "text1";
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    frameNode->UpdateInspectorId(inspectorId);
    auto context = PipelineContext::GetCurrentContext();
    context->rootNode_ = rootNode;
    rootNode->AddChild(frameNode);
    auto searchedNode = Inspector::GetFrameNodeByKey(inspectorId);
    EXPECT_EQ(frameNode, searchedNode);

    // tc.steps:    add offScreencreenNode to off screencreen node with inspector id text1
    // tc.expected: expect the function GetFrameNodeByKey get the offScreencreenNode
    auto offScreencreenNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    offScreencreenNode->UpdateInspectorId(inspectorId);
    Inspector::AddOffscreenNode(offScreencreenNode);
    searchedNode = Inspector::GetFrameNodeByKey(inspectorId);
    EXPECT_EQ(offScreencreenNode, searchedNode);

    // tc.steps:    execute GetFrameNodeByKey when skipoffscreenNodes set true
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    searchedNode = Inspector::GetFrameNodeByKey(inspectorId, false, true);
    EXPECT_EQ(frameNode, searchedNode);

    // tc.steps:    execute GetFrameNodeByKey when remove offScreencreenNode
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    Inspector::RemoveOffscreenNode(offScreencreenNode);
    searchedNode = Inspector::GetFrameNodeByKey(inspectorId);
    EXPECT_EQ(frameNode, searchedNode);
}

/**
 * @tc.name: InspectorTestNg018
 * @tc.desc: Test the operation of GetInspectorNodeByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg018, TestSize.Level1)
{
    // tc.steps: step1. add frameNode to arkui tree with inspector id text1
    // tc.expected: expect the function GetInspectorNodeByKey get the frameNode
    std::string inspectorId = "text1";
    int32_t textNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        textNodeId, AceType::MakeRefPtr<Pattern>());
    frameNode->UpdateInspectorId(inspectorId);
    auto context = PipelineContext::GetCurrentContext();
    context->rootNode_ = rootNode;
    rootNode->AddChild(frameNode);
    std::string searchedNodeStr = Inspector::GetInspectorNodeByKey(inspectorId);
    auto searchedNode = JsonUtil::ParseJsonString(searchedNodeStr);
    EXPECT_TRUE(searchedNode->IsValid());

    std::string nodeType = searchedNode->GetString(INSPECTOR_TYPE);
    EXPECT_EQ(nodeType, TEXT_NODE_TYPE);

    int32_t nodeId = searchedNode->GetInt(INSPECTOR_ID);
    EXPECT_EQ(textNodeId, nodeId);

    std::string debugLine = searchedNode->GetString(INSPECTOR_DEBUGLINE);
    EXPECT_EQ(debugLine, "");

    auto attrJson = searchedNode->GetObject(INSPECTOR_ATTRS);
    std::string accessibilityTextAttr = attrJson->GetString("accessibilityText");
    EXPECT_EQ(accessibilityTextAttr, "");

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<AccessibilityProperty>();
    accessibilityProperty->SetAccessibilityText(TEST_TEXT);
    searchedNodeStr = Inspector::GetInspectorNodeByKey(inspectorId);
    searchedNode = JsonUtil::ParseJsonString(searchedNodeStr);
    EXPECT_TRUE(searchedNode->IsValid());
    attrJson = searchedNode->GetObject(INSPECTOR_ATTRS);
    accessibilityTextAttr = attrJson->GetString("accessibilityText");
    EXPECT_EQ(accessibilityTextAttr, TEST_TEXT);
}

/**
 * @tc.name: InspectorTestNg020
 * @tc.desc: Test the operation of GetInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg020, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode(
        "stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    const RefPtr<FrameNode> PAGE = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto customNode = CustomNode::CreateCustomNode(1, "custom");
    stageNode->AddChild(PAGE);
    PAGE->AddChild(customNode);

    // tc.steps:    execute GetInspector when param isLayoutInspector is false
    // tc.expected: expect the function GetInspector result without customNode
    std::string searchedNodesStr = Inspector::GetInspector();
    auto searchedNodes = JsonUtil::ParseJsonString(searchedNodesStr);
    EXPECT_TRUE(searchedNodes->IsValid());
    auto attrJson = searchedNodes->GetObject(INSPECTOR_CHILDREN);
    EXPECT_TRUE(attrJson->IsNull());
}

/**
 * @tc.name: InspectorTestNg021
 * @tc.desc: Test the operation of ParseWindowIdFromMsg
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg021, TestSize.Level1)
{
    std::string inspectorMsg = "";
    auto result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, -1);

    inspectorMsg = "invalid message";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, -1);

    inspectorMsg = "{\"method\":\"ArkUI.xxx\", \"params\":{\"windowId\":\"10\"}}";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, -1);

    inspectorMsg = "{\"method\":\"ArkUI.tree\", \"params\":{\"windowIds\":\"10\"}}";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, 0);

    inspectorMsg = "{\"method\":\"ArkUI.tree\", \"paramss\":{\"windowId\":\"10\"}}";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, 0);

    inspectorMsg = "{\"method\":\"ArkUI.tree\", \"params\":{\"windowId\":\"10\"}}";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 10);
    EXPECT_EQ(result.second, 0);

    inspectorMsg = "{\"method\":\"ArkUI.tree.3D\", \"params\":{\"windowId\":\"10\"}}";
    result = Inspector::ParseWindowIdFromMsg(inspectorMsg);
    EXPECT_EQ(result.first, 10);
    EXPECT_EQ(result.second, 1);
}

/**
 * @tc.name: InspectorTestNg029
 * @tc.desc: Test the operation of ParseNeedFreeNodes with valid params
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg029, TestSize.Level1)
{
    std::string inspectorMsg = INSPECTOR_MSG_NEED_FREE_NODES_TRUE;
    auto result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, true);

    inspectorMsg = INSPECTOR_MSG_NEED_FREE_NODES_FALSE;
    result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, false);

    inspectorMsg = INSPECTOR_MSG_NO_NEED_FREE_NODES_PARAM;
    result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: InspectorTestNg030
 * @tc.desc: Test the operation of ParseNeedFreeNodes with invalid message
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg030, TestSize.Level1)
{
    std::string inspectorMsg = "";
    auto result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, false);

    inspectorMsg = INSPECTOR_MSG_INVALID;
    result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, false);

    inspectorMsg = INSPECTOR_MSG_INVALID_PARAMS;
    result = Inspector::ParseNeedFreeNodes(inspectorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: InspectorTestNg031
 * @tc.desc: Test the operation of GetFreeNodesInspector in normal context
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg031, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        "stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(pageNode);
    auto childNode = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(childNode);

    bool needThrow = false;
    InspectorFilter filter;
    filter.EnableFreeNodes();
    auto expected = Inspector::GetInspector(true, filter, needThrow);
    EXPECT_EQ(needThrow, false);

    auto result = Inspector::GetFreeNodesInspector();
    EXPECT_EQ(result, expected);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;

    auto jsonValue = JsonUtil::ParseJsonString(result);
    EXPECT_NE(jsonValue, nullptr);
    if (jsonValue != nullptr) {
        auto typeValue = jsonValue->GetString("type");
        EXPECT_EQ(typeValue, "root");
    }
}

/**
 * @tc.name: InspectorTestNg032
 * @tc.desc: Test the operation of GetFreeNodesInspector when context is null
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg032, TestSize.Level1)
{
    RefPtr<MockPipelineContext> pipelineBak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;

    auto result = Inspector::GetFreeNodesInspector();
    MockPipelineContext::pipeline_ = pipelineBak;

    auto jsonValue = JsonUtil::ParseJsonString(result);
    EXPECT_NE(jsonValue, nullptr);
    if (jsonValue != nullptr) {
        auto typeValue = jsonValue->GetValue(INSPECTOR_TYPE);
        ASSERT_NE(typeValue, nullptr);
        EXPECT_STREQ(typeValue->GetString().c_str(), "root");
    }
}

/**
 * @tc.name: InspectorTestNg033
 * @tc.desc: Test that main-tree nodes are filtered in free-nodes inspector.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg033, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    context->rootNode_ = stageNode;

    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(pageNode);
    auto childNode = FrameNode::CreateFrameNode(
        CHILD_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(childNode);

    auto freeNode = FrameNode::CreateFrameNode(
        FREE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    freeNode->onMainTree_ = false;
    auto freeNodeId = freeNode->GetId();

    auto mainTreeNode = FrameNode::CreateFrameNode(
        MAIN_TREE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    mainTreeNode->onMainTree_ = true;
    auto mainTreeNodeId = mainTreeNode->GetId();

    auto result = Inspector::GetFreeNodesInspector();
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);

    auto otherContents = jsonValue->GetValue(INSPECTOR_OTHER_CONTENTS);
    ASSERT_NE(otherContents, nullptr);
    ASSERT_TRUE(otherContents->IsArray());

    bool foundFreeNode = false;
    bool foundMainTreeNode = false;
    for (int32_t i = 0; i < otherContents->GetArraySize(); ++i) {
        auto node = otherContents->GetArrayItem(i);
        auto idValue = node->GetValue(INSPECTOR_ID);
        ASSERT_NE(idValue, nullptr);
        if (idValue->GetInt() == freeNodeId) {
            foundFreeNode = true;
        }
        if (idValue->GetInt() == mainTreeNodeId) {
            foundMainTreeNode = true;
        }
    }

    EXPECT_TRUE(foundFreeNode);
    EXPECT_FALSE(foundMainTreeNode);
    context->stageManager_ = nullptr;
    context->rootNode_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg034
 * @tc.desc: Test that querying subtree with free nodes enabled still returns empty other_contents.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg034, TestSize.Level1)
{
    ElementRegister::GetInstance()->Clear();
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto targetNode = FrameNode::CreateFrameNode(
        CHILD_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    std::string id = "query_target_node";
    targetNode->UpdateInspectorId(id);
    stageNode->AddChild(pageNode);
    pageNode->AddChild(targetNode);

    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    context->rootNode_ = stageNode;

    bool needThrow = false;
    InspectorFilter filter;
    filter.SetFilterID(id);
    filter.EnableFreeNodes();
    auto result = Inspector::GetInspector(true, filter, needThrow);

    EXPECT_FALSE(needThrow);
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_TRUE(jsonValue->Contains(INSPECTOR_OTHER_CONTENTS));
    auto otherContents = jsonValue->GetValue(INSPECTOR_OTHER_CONTENTS);
    ASSERT_NE(otherContents, nullptr);
    EXPECT_TRUE(otherContents->IsArray());
    EXPECT_EQ(otherContents->GetArraySize(), EMPTY_OTHER_CONTENTS_SIZE);

    context->stageManager_ = nullptr;
    context->rootNode_ = nullptr;
}


/**
 * @tc.name: AddOffscreenNode_001
 * @tc.desc: Test the operation of AddOffscreenNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, AddOffscreenNode_001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto offscreenNodesMgr = context->GetInspectorOffscreenNodesMgr();
    ASSERT_NE(offscreenNodesMgr, nullptr);
    int32_t num = offscreenNodesMgr->GetOffscreenNodesSize();
    RefPtr<FrameNode> one = nullptr;
    Inspector::AddOffscreenNode(one);
    EXPECT_EQ(offscreenNodesMgr->GetOffscreenNodesSize(), num);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    one = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);

    context->stageManager_ = AceType::MakeRefPtr<StageManager>(one);
    num = offscreenNodesMgr->GetOffscreenNodesSize();
    Inspector::AddOffscreenNode(one);
    EXPECT_EQ(offscreenNodesMgr->GetOffscreenNodesSize(), num + 1);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: RemoveOffscreenNode_001
 * @tc.desc: Test the operation of RemoveOffscreenNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, RemoveOffscreenNode_001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto offscreenNodesMgr = context->GetInspectorOffscreenNodesMgr();
    ASSERT_NE(offscreenNodesMgr, nullptr);
    int32_t num = offscreenNodesMgr->GetOffscreenNodesSize();
    RefPtr<FrameNode> one = nullptr;
    Inspector::RemoveOffscreenNode(one);
    EXPECT_EQ(offscreenNodesMgr->GetOffscreenNodesSize(), num);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    one = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(one);
    Inspector::AddOffscreenNode(one);
    num = offscreenNodesMgr->GetOffscreenNodesSize();

    Inspector::RemoveOffscreenNode(one);
    EXPECT_EQ(offscreenNodesMgr->GetOffscreenNodesSize(), num - 1);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetOffScreenTreeNodes_001
 * @tc.desc: Test the operation of GetOffScreenTreeNodes
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOffScreenTreeNodes_001, TestSize.Level1)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> one = FrameNode::CreateFrameNode("one", id, AceType::MakeRefPtr<Pattern>(), true);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto offscreenNodesMgr = context->GetInspectorOffscreenNodesMgr();
    ASSERT_NE(offscreenNodesMgr, nullptr);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(one);
    Inspector::AddOffscreenNode(one);
    int32_t num = offscreenNodesMgr->GetOffscreenNodesSize();
    NG::InspectorTreeMap offNodes;
    Inspector::GetOffScreenTreeNodes(offNodes);
    EXPECT_EQ(offNodes.size(), num);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetRecordAllPagesNodes_001
 * @tc.desc: Test the operation of GetRecordAllPagesNodes
 * (stageNode)--PageA--PageB--PageC
 *                              |--frameNode
 *                              |--frameNode1--frameNode3
 *                              |--frameNode2
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetRecordAllPagesNodes_001, TestSize.Level1)
{
    // tc.steps: step1. call GetRecordAllPagesNodes
    // tc.expected: expect the function is run ok
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode("sageNode", id, AceType::MakeRefPtr<Pattern>(), true);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageNode->children_.clear();

    // tc.steps: step2 add lastPage and create a frame node tree to lastPage
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageA = FrameNode::CreateFrameNode("PageA", id2,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);
    auto id3 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageB = FrameNode::CreateFrameNode("PageB", id3,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageB);
    auto id4 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageC = FrameNode::CreateFrameNode("PageC", id4,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageC);

    auto frameNode = FrameNode::CreateFrameNode("frameNode0", 5, AceType::MakeRefPtr<Pattern>(), true);
    pageC->AddChild(frameNode);
    auto frameNode1 = FrameNode::CreateFrameNode("frameNode1", 6, AceType::MakeRefPtr<Pattern>(), true);
    pageC->AddChild(frameNode1);
    auto frameNode2 = FrameNode::CreateFrameNode("frameNode2", 62, AceType::MakeRefPtr<Pattern>(), true);
    pageC->AddChild(frameNode2);
    frameNode2->isActive_ = false;
    auto frameNode3 = FrameNode::CreateFrameNode("frameNode3", 63, AceType::MakeRefPtr<Pattern>(), true);
    frameNode1->AddChild(frameNode3);
    frameNode3->isActive_ = true;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetRecordAllPagesNodes(treesInfos);
    EXPECT_TRUE(!treesInfos.empty());
    context1->stageManager_ = nullptr;
    auto node3 = treesInfos[63];
    EXPECT_TRUE(node3 != nullptr);
    EXPECT_EQ(node3->GetParentId(), 6);
}

HWTEST_F(InspectorTestNg, GetRecordAllPagesNodes_002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    // STEP1:Create stageNode->pageA->frameNode
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode("stageNode", 1, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageNode->children_.clear();
    const RefPtr<FrameNode> pageA = FrameNode::CreateFrameNode("pageA", 2,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);
    auto frameNode = FrameNode::CreateFrameNode("frameNode0", 5, AceType::MakeRefPtr<Pattern>(), true);
    pageA->AddChild(frameNode);
    // STEP2:parent is CustomNode,stageNode->pageA->frameNode->customNode->customNodeChild
    auto customNode = CustomNode::CreateCustomNode(100, "custom");
    auto customNodeChild = FrameNode::CreateFrameNode("customNodeChild", 103, AceType::MakeRefPtr<Pattern>(), true);
    customNodeChild->isActive_ = true;
    customNode->AddChild(customNodeChild);
    frameNode->AddChild(customNode);
    // STEP3:parent is SpanNode,stageNode->pageA->frameNode->spanNode->spanNodeChild
    auto spanNode = SpanNode::GetOrCreateSpanNode(int32_t(101));
    auto spanNodeChild = FrameNode::CreateFrameNode("spanNodeChild", 104, AceType::MakeRefPtr<Pattern>(), true);
    spanNodeChild->isActive_ = true;
    spanNode->AddChild(spanNodeChild);
    frameNode->AddChild(spanNode);
    // STEP4:parent is TestNode,stageNode->pageA->frameNode->testNode->testNodeChild
    auto testNode = InspectorTestNode::CreateTestNode(102);
    auto testNodeChild = FrameNode::CreateFrameNode("testNodeChild", 105, AceType::MakeRefPtr<Pattern>(), true);
    testNodeChild->isActive_ = true;
    testNode->AddChild(testNodeChild);
    frameNode->AddChild(testNode);
    NG::InspectorTreeMap treesInfos;
    Inspector::GetRecordAllPagesNodes(treesInfos);
    auto treeNode = treesInfos[102];
    EXPECT_TRUE(treeNode == nullptr);
    treeNode = treesInfos[103];
    EXPECT_TRUE(treeNode != nullptr);
    EXPECT_EQ(treeNode->GetParentId(), 100);
    treeNode = treesInfos[104];
    EXPECT_TRUE(treeNode != nullptr);
    EXPECT_EQ(treeNode->GetParentId(), 101);
    treeNode = treesInfos[105];
    EXPECT_TRUE(treeNode != nullptr);
    EXPECT_EQ(treeNode->GetParentId(), 5);
}

/**
 * @tc.name: InspectorTestNg022
 * @tc.desc: Test the method GetInspectorChildrenInfo
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test GetInspectorChildrenInfo
     */
    auto spanNode = SpanNode::GetOrCreateSpanNode(int32_t(191));
    NG::InspectorTreeMap treesInfos;
    EXPECT_TRUE(AceType::InstanceOf<SpanNode>(spanNode));
    Inspector::GetInspectorChildrenInfo(spanNode, treesInfos, 1, 1);

    auto customNode = CustomNode::CreateCustomNode(1, "custom");
    EXPECT_TRUE(AceType::InstanceOf<CustomNode>(customNode));
    Inspector::GetInspectorChildrenInfo(spanNode, treesInfos, 1, 1);

    Inspector::GetInspectorChildrenInfo(spanNode, treesInfos, 1, 0);
}

/**
 * @tc.name: InspectorTestNg023
 * @tc.desc: Test the method GetCustomNodeInfo
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize context and create stage/page nodes
     * @tc.expected: Context should be valid, stage/page nodes created successfully
     */
    auto context = PipelineContext::GetCurrentContext();
    EXPECT_NE(context, nullptr);
    auto stageId = ElementRegister::GetInstance()->MakeUniqueId();
    auto stageNode = FrameNode::CreateFrameNode("stage", stageId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode("page", 100, AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(pageNode);

    /**
     * @tc.steps: step2. Build test node hierarchy
     */
    auto customNode = CustomNode::CreateCustomNode(101, V2::JS_VIEW_ETS_TAG);
    customNode->UpdateInspectorId("custom_node_id");
    customNode->extraInfo_ = { "pages", 15, 20 };
    pageNode->AddChild(customNode);
    auto frameNode = FrameNode::CreateFrameNode("button", 102, AceType::MakeRefPtr<Pattern>(), true);
    frameNode->tag_ = V2::JS_VIEW_ETS_TAG;
    customNode->AddChild(frameNode);
    auto customNode2 = CustomNode::CreateCustomNode(103, V2::JS_VIEW_ETS_TAG);
    frameNode->AddChild(customNode2);

    /**
     * @tc.steps: step3. Execute inspection with ID filter
     * @tc.expected: Should return valid JSON without error flag
     */
    bool needThrow = false;
    InspectorFilter filter;
    std::string id = "custom_node_id";
    filter.SetFilterID(id);
    std::string result = Inspector::GetInspector(true, filter, needThrow);
    EXPECT_FALSE(needThrow);

    /**
     * @tc.steps: step4. Verify frame node inspection data
     * @tc.expected: Should return non-empty valid JSON
     */
    auto resultFrameNode = Inspector::GetInspectorOfNode(frameNode);
    auto jsonFrameNode = JsonUtil::ParseJsonString(resultFrameNode);
    ASSERT_NE(resultFrameNode, "");
    EXPECT_STREQ(jsonFrameNode->GetValue(INSPECTOR_DEBUGLINE)->GetString().c_str(), "");

    /**
     * @tc.steps: step5. Verify CustomNode specific attributes
     * @tc.expected: CustomNode should show debug info, FrameNode as build-in type
     */
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_STREQ(jsonValue->GetValue("type")->GetString().c_str(), "root");
    auto content = jsonValue->GetValue("content");
    ASSERT_FALSE(content->IsNull());
    auto childrenArray = content->GetValue(INSPECTOR_CHILDREN);
    ASSERT_TRUE(childrenArray->IsArray());
    bool foundCustomNode = false;
    for (int32_t i = 0; i < childrenArray->GetArraySize(); ++i) {
        auto node = childrenArray->GetArrayItem(i);
        if (node->GetValue(INSPECTOR_ID)->GetInt() == 101) {
            auto debugLine = JsonUtil::ParseJsonString(node->GetValue(INSPECTOR_DEBUGLINE)->GetString());
            EXPECT_STREQ("pages(15:20)", debugLine->GetValue("$line")->GetString().c_str());
            foundCustomNode = true;
        } else if (node->GetValue(INSPECTOR_ID)->GetInt() == 102) {
            EXPECT_STREQ("build-in", node->GetValue("type")->GetString().c_str());
        }
    }
    EXPECT_TRUE(foundCustomNode);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorTestNg024
 * @tc.desc: Test the GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg024, TestSize.Level1)
{
    // tc.steps: step1. add frameNode to arkui tree with inspector id test
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    std::string inspectorId = "test";
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    frameNode->UpdateInspectorId(inspectorId);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->rootNode_ = rootNode;
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());
    rootNode->AddChild(frameNode);
    auto searchedNode = Inspector::GetFrameNodeByKey(inspectorId, true);
    EXPECT_NE(searchedNode, nullptr);

    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.size.Width(), 0.0f);
    auto renderContext = searchedNode->GetRenderContext();
    EXPECT_NE(renderContext, nullptr);

    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    auto container = Container::Current();
    ASSERT_NE(container, nullptr);
    container->isDynamicRender_ = false;
    auto pipeline = searchedNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    Offset offfset(1.0, 1.0);
    pipeline->SetHostParentOffsetToWindow(Offset(offfset.GetX(), offfset.GetY()));

    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.windowOffset, searchedNode->GetOffsetRelativeToWindow());

    container->isDynamicRender_ = true;
    container->uIContentType_ = UIContentType::UNDEFINED;
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.windowOffset, searchedNode->GetOffsetRelativeToWindow());

    container->uIContentType_ = UIContentType::DYNAMIC_COMPONENT;
    pipeline->GetHostParentOffsetToWindow();
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_NE(rect.windowOffset, searchedNode->GetOffsetRelativeToWindow());
}

/**
 * @tc.name: InspectorTestNg025
 * @tc.desc: Test the GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg025, TestSize.Level1)
{
    // tc.steps: step1. add frameNode to arkui tree with inspector id test
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    std::string inspectorId = "test";
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    frameNode->UpdateInspectorId(inspectorId);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->rootNode_ = rootNode;
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());
    rootNode->AddChild(frameNode);
    auto searchedNode = Inspector::GetFrameNodeByKey(inspectorId, true);
    EXPECT_NE(searchedNode, nullptr);

    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.size.Width(), 0.0f);
    auto renderContext = AceType::DynamicCast<MockRenderContext>(searchedNode->GetRenderContext());
    EXPECT_NE(renderContext, nullptr);

    const double halfDimension = 200.0;
    renderContext->paintRect_ = { 2.0f, 2.0f, 3.0f, 3.0f };
    DimensionOffset offset = DimensionOffset(
        Dimension(halfDimension, DimensionUnit::PX), Dimension(halfDimension, DimensionUnit::PX));
    renderContext->UpdateTransformCenter(offset);
    EXPECT_TRUE(renderContext->GetTransformCenter().has_value());
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.scale.centerX, offset.GetX().ConvertToVp());
    EXPECT_EQ(rect.scale.centerY, offset.GetY().ConvertToVp());

    offset = DimensionOffset(
        Dimension(halfDimension, DimensionUnit::PX), Dimension(halfDimension, DimensionUnit::PERCENT));
    renderContext->UpdateTransformCenter(offset);
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.scale.centerX, offset.GetX().ConvertToVp());
    EXPECT_NE(rect.scale.centerY, offset.GetY().ConvertToVp());

    offset = DimensionOffset(
        Dimension(halfDimension, DimensionUnit::PERCENT), Dimension(halfDimension, DimensionUnit::PERCENT));
    renderContext->UpdateTransformCenter(offset);
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_NE(rect.scale.centerX, offset.GetX().ConvertToVp());
    EXPECT_NE(rect.scale.centerY, offset.GetY().ConvertToVp());
}

/**
 * @tc.name: InspectorTestNg026
 * @tc.desc: Test the GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg026, TestSize.Level1)
{
    // tc.steps: step1. add frameNode to arkui tree with inspector id test
    // tc.expected: expect the function GetFrameNodeByKey get the frameNode
    std::string inspectorId = "test";
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    frameNode->UpdateInspectorId(inspectorId);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->rootNode_ = rootNode;
    context->rootNode_->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());
    rootNode->AddChild(frameNode);
    auto searchedNode = Inspector::GetFrameNodeByKey(inspectorId, true);
    EXPECT_NE(searchedNode, nullptr);

    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.size.Width(), 0.0f);
    auto renderContext = AceType::DynamicCast<MockRenderContext>(searchedNode->GetRenderContext());
    EXPECT_NE(renderContext, nullptr);

    CalcDimension x(30, DimensionUnit::VP);
    CalcDimension y(20, DimensionUnit::VP);
    CalcDimension z(20, DimensionUnit::VP);
    renderContext->UpdateTransformTranslate(TranslateOptions(x, y, z));
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_EQ(rect.translate.x, x.ConvertToVp());
    EXPECT_EQ(rect.translate.y, y.ConvertToVp());

    CalcDimension x2(30, DimensionUnit::PERCENT);
    CalcDimension y2(20, DimensionUnit::PERCENT);
    CalcDimension z2(20, DimensionUnit::PERCENT);
    renderContext->UpdateTransformTranslate(TranslateOptions(x2, y2, z2));
    Inspector::GetRectangleById(inspectorId, rect);
    EXPECT_NE(rect.translate.x, x.ConvertToVp());
    EXPECT_NE(rect.translate.y, y.ConvertToVp());
}

/**
 * @tc.name: InspectorInvalid001
 * @tc.desc: Test the SendEventByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid001, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    bool result = Inspector::SendEventByKey("testKey", 0, "");
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InspectorInvalid002
 * @tc.desc: Test the SendEventByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    auto rootNode = context->rootNode_;
    context->rootNode_ = nullptr;
    bool result = Inspector::SendEventByKey("testKey", 0, "");
    context->rootNode_ = rootNode;
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InspectorInvalid003
 * @tc.desc: Test the operation of GetInspectorNodeByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid003, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    std::string result = Inspector::GetInspectorNodeByKey("testKey");
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: InspectorInvalid004
 * @tc.desc: Test the operation of GetInspectorNodeByKey
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid004, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    auto rootNode = context->rootNode_;
    context->rootNode_ = nullptr;
    std::string result = Inspector::GetInspectorNodeByKey("testKey");
    context->rootNode_ = rootNode;
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: InspectorInvalid005
 * @tc.desc: Test the operation of GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid005, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById("testKey", rect);
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_EQ(rect.screenRect.height_, 0.0);
    EXPECT_EQ(rect.screenRect.width_, 0.0);
    EXPECT_EQ(rect.screenRect.x_, 0.0);
    EXPECT_EQ(rect.screenRect.y_, 0.0);
}

/**
 * @tc.name: InspectorInvalid006
 * @tc.desc: Test the operation of GetRectangleById
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid006, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    auto rootNode = context->rootNode_;
    context->rootNode_ = nullptr;
    OHOS::Ace::NG::Rectangle rect;
    Inspector::GetRectangleById("testKey", rect);
    context->rootNode_ = rootNode;
    EXPECT_EQ(rect.screenRect.height_, 0.0);
    EXPECT_EQ(rect.screenRect.width_, 0.0);
    EXPECT_EQ(rect.screenRect.x_, 0.0);
    EXPECT_EQ(rect.screenRect.y_, 0.0);
}

/**
 * @tc.name: InspectorInvalid007
 * @tc.desc: Test the operation of GetInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid007, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    auto result = Inspector::GetInspector(false);
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_EQ(result, "{\"$type\":\"root\"}");
}

/**
 * @tc.name: InspectorInvalid008
 * @tc.desc: Test the operation of GetInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid008, TestSize.Level1)
{
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockStageManager> stageManager = AceType::MakeRefPtr<MockStageManager>(stageNode);
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = stageManager;
    RefPtr<FrameNode> nullNode = nullptr;
    EXPECT_CALL(*stageManager, GetLastPage()).Times(1).WillOnce(Return(nullNode));
    auto result = Inspector::GetInspector(false);
    context->stageManager_ = stageManagerBak;
    EXPECT_EQ(result, "{\"$type\":\"root\",\"width\":\"720.000000\",\"height\":\"1280.000000\",\"$resolution\":\"1.000000\"}");
}

/**
 * @tc.name: InspectorInvalid009
 * @tc.desc: Test the operation of GetInspectorOfNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid009, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    auto frameNode = FrameNode::CreateFrameNode("button", 102, AceType::MakeRefPtr<Pattern>(), true);
    auto result = Inspector::GetInspectorOfNode(frameNode);
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_EQ(result, "{}");
}

/**
 * @tc.name: InspectorInvalid010
 * @tc.desc: Test the operation of GetInspectorOfNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid010, TestSize.Level1)
{
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockStageManager> stageManager = AceType::MakeRefPtr<MockStageManager>(stageNode);
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = stageManager;
    RefPtr<FrameNode> nullNode = nullptr;
    EXPECT_CALL(*stageManager, GetLastPage()).Times(1).WillOnce(Return(nullNode));
    auto frameNode = FrameNode::CreateFrameNode("button", 102, AceType::MakeRefPtr<Pattern>(), true);
    auto result = Inspector::GetInspectorOfNode(frameNode);
    context->stageManager_ = stageManagerBak;
    EXPECT_EQ(result, "{\"width\":\"720.000000\",\"height\":\"1280.000000\",\"$resolution\":\"1.000000\"}");
}

/**
 * @tc.name: InspectorInvalid011
 * @tc.desc: Test the operation of GetInspectorOfNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid011, TestSize.Level1)
{
    auto result = Inspector::GetInspectorOfNode(nullptr);
    EXPECT_EQ(result, "{\"width\":\"720.000000\",\"height\":\"1280.000000\",\"$resolution\":\"1.000000\"}");
}

/**
 * @tc.name: InspectorInvalid012
 * @tc.desc: Test the operation of GetSubWindowInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid012, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    auto result = Inspector::GetSubWindowInspector();
    MockPipelineContext::pipeline_ = pipeline_bak;
    EXPECT_EQ(result, "{\"$type\":\"root\"}");
}

/**
 * @tc.name: InspectorInvalid013
 * @tc.desc: Test the operation of GetSubWindowInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid013, TestSize.Level1)
{
    // auto rootNode = FrameNode::CreateFrameNode(
    //     V2::OVERLAY_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockOverlayManager> overlayManager = AceType::MakeRefPtr<MockOverlayManager>(nullptr);
    overlayManager->rootNodeWeak_ = nullptr;
    auto overlayManagerBak = context->overlayManager_;
    context->overlayManager_ = overlayManager;
    auto result = Inspector::GetSubWindowInspector();
    context->overlayManager_ = overlayManagerBak;
    EXPECT_EQ(result, "{\"$type\":\"root\",\"width\":\"720.000000\",\"height\":\"1280.000000\",\"$resolution\":\"1.000000\"}");
}

/**
 * @tc.name: InspectorInvalid014
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid014, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    MockPipelineContext::pipeline_ = pipeline_bak;
}

/**
 * @tc.name: InspectorInvalid015
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid015, TestSize.Level1)
{
    // 设置StageManager返回空指针
    auto context = MockPipelineContext::GetCurrentContext();
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = nullptr;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    context->stageManager_ = stageManagerBak;
}

/**
 * @tc.name: InspectorInvalid016
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid016, TestSize.Level1)
{
    // 设置StageManager GetLastPage方法返回空指针
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockStageManager> stageManager = AceType::MakeRefPtr<MockStageManager>(stageNode);
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = stageManager;
    RefPtr<FrameNode> nullNode = nullptr;
    EXPECT_CALL(*stageManager, GetLastPage()).Times(1).WillOnce(Return(nullNode));
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    context->stageManager_ = stageManagerBak;
}

/**
 * @tc.name: InspectorInvalid017
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid017, TestSize.Level1)
{
    // 设置PipelineContext返回空指针
    RefPtr<MockPipelineContext> pipeline_bak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetRecordAllPagesNodes(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    MockPipelineContext::pipeline_ = pipeline_bak;
}

/**
 * @tc.name: InspectorInvalid018
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid018, TestSize.Level1)
{
    // 设置StageManager返回空指针
    auto context = MockPipelineContext::GetCurrentContext();
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = nullptr;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetRecordAllPagesNodes(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    context->stageManager_ = stageManagerBak;
}

/**
 * @tc.name: InspectorInvalid019
 * @tc.desc: Test the operation of GetInspectorTree
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorInvalid019, TestSize.Level1)
{
    // 设置StageManager GetStageNode方法返回空指针
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockStageManager> stageManager = AceType::MakeRefPtr<MockStageManager>(nullptr);
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = stageManager;
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    EXPECT_TRUE(treesInfos.empty());
    context->stageManager_ = stageManagerBak;
}

/**
 * @tc.name: InspectorFilterTest001
 * @tc.desc: Test the operation of FilterEmptyInitially
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorFilterTest001, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    EXPECT_TRUE(filter->FilterEmpty());
}

/**
* @tc.name: InspectorFilterTest002
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest002, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    EXPECT_TRUE(filter->CheckFixedAttr(FIXED_ATTR_ID));
}

/**
* @tc.name: InspectorFilterTest003
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest003, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    EXPECT_TRUE(filter->CheckExtAttr("some_attr"));
}

/**
* @tc.name: InspectorFilterTest004
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest004, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("id");
    EXPECT_TRUE(filter->CheckFixedAttr(FIXED_ATTR_ID));
    EXPECT_FALSE(filter->CheckFixedAttr(FIXED_ATTR_CONTENT));
}

/**
* @tc.name: InspectorFilterTest005
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest005, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("id");
    filter->AddFilterAttr("content");

    EXPECT_TRUE(filter->CheckFixedAttr(FIXED_ATTR_ID));
    EXPECT_TRUE(filter->CheckFixedAttr(FIXED_ATTR_CONTENT));
    EXPECT_FALSE(filter->CheckFixedAttr(FIXED_ATTR_SRC));
}

/**
* @tc.name: InspectorFilterTest006
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest006, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("custom_attr");

    EXPECT_TRUE(filter->CheckExtAttr("custom_attr"));
    EXPECT_FALSE(filter->CheckExtAttr("other_attr"));
}

/**
* @tc.name: InspectorFilterTest007
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest007, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("id");

    EXPECT_TRUE(filter->CheckFilterAttr(FIXED_ATTR_ID, nullptr));
    EXPECT_FALSE(filter->CheckFilterAttr(FIXED_ATTR_CONTENT, nullptr));
}

/**
* @tc.name: InspectorFilterTest008
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest008, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("custom_attr");

    EXPECT_TRUE(filter->CheckFilterAttr(FIXED_ATTR_ID, "custom_attr"));
    EXPECT_FALSE(filter->CheckFilterAttr(FIXED_ATTR_ID, "other_attr"));
}

/**
* @tc.name: InspectorFilterTest009
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest009, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("id");
    filter->AddFilterAttr("content");

   EXPECT_TRUE(filter->IsFastFilter());
}

/**
* @tc.name: InspectorFilterTest010
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest010, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    filter->AddFilterAttr("custom_attr");

    EXPECT_FALSE(filter->IsFastFilter());
}

/**
* @tc.name: InspectorFilterTest011
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest011, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    std::string id = "test_id";
    filter->SetFilterID(id);

    EXPECT_EQ(id, filter->GetFilterID());
}

/**
* @tc.name: InspectorFilterTest012
* @tc.desc: Test the operation of FilterEmptyInitially
* @tc.type: FUNC
*/
HWTEST_F(InspectorTestNg, InspectorFilterTest012, TestSize.Level1)
{
    auto filter = std::make_unique<InspectorFilter>();
    size_t depth = 5;
    filter->SetFilterDepth(depth);
    EXPECT_EQ(depth, filter-> GetFilterDepth());
}

bool HasInternalIds(const std::unique_ptr<JsonValue>& children)
{
    auto hasInternalIds = false;
    CHECK_NULL_RETURN(children, false);
    for (size_t i = 0; i < children->GetArraySize(); i++) {
        auto child = children->GetArrayItem(i);
        if (child->GetValue("$type")->GetString() == "button") {
            auto internal = child->Contains("$INTERNAL_IDS");
            if (internal) {
                hasInternalIds = true;
                break;
            }
        }
    }
    return hasInternalIds;
}

/**
 * @tc.name: InspectorTestNg027
 * @tc.desc: Test the operation of GetInspector
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg027, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode(
        "sageNode", id, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageNode->children_.clear();

    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageA = FrameNode::CreateFrameNode("PageA", id2,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);
    auto id3 = ElementRegister::GetInstance()->MakeUniqueId();

    auto button = FrameNode::CreateFrameNode("button", id3, AceType::MakeRefPtr<Pattern>(), true);
    pageA->AddChild(button);

    auto id4 = ElementRegister::GetInstance()->MakeUniqueId();
    auto text = FrameNode::CreateFrameNode("text", id4, AceType::MakeRefPtr<Pattern>(), true);
    button->AddChild(text);
    button->isActive_ = true;

    std::string tree = Inspector::GetInspector();
    auto jsonRoot  = JsonUtil::ParseJsonString(tree);
    ASSERT_NE(jsonRoot, nullptr);

    auto children = jsonRoot->GetValue("$children");
    ASSERT_TRUE(children->IsArray());
    auto hasInternalIds = HasInternalIds(children);
    ASSERT_FALSE(hasInternalIds);

    text->SetInternal();
    tree = Inspector::GetInspector();
    jsonRoot  = JsonUtil::ParseJsonString(tree);
    ASSERT_NE(jsonRoot, nullptr);

    children = jsonRoot->GetValue("$children");
    ASSERT_TRUE(children->IsArray());
    hasInternalIds = HasInternalIds(children);
    ASSERT_TRUE(hasInternalIds);
}

/**
 * @tc.name: InspectorTestNg028
 * @tc.desc: Test GetInspectorOfNode when nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorTestNg028, TestSize.Level1)
{
    auto resultFrameNode = Inspector::GetInspectorOfNode(nullptr);
    std::string emptyString = "";
    EXPECT_NE(resultFrameNode, emptyString);
}

/**
 * @tc.name: GetOverlayNode_001
 * @tc.desc: Test the operation of GetOverlayNode in stage  overlay
 * column
 *    |--stage
 *       |--PageA
 *         |--frameNode
 *    |--overlay
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOverlayNode_001, TestSize.Level1)
{
    // tc.steps: step1. build tree
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);
    auto id0 = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> columnNode = FrameNode::CreateFrameNode("Column", id0, AceType::MakeRefPtr<Pattern>(), false);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode("stage", id, AceType::MakeRefPtr<Pattern>(), false);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageNode->children_.clear();
    columnNode->AddChild(stageNode);

    auto overlayNode = FrameNode::CreateFrameNode(
        "overlayNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    columnNode->AddChild(overlayNode);
    auto testNode = FrameNode::CreateFrameNode(
        "testNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    overlayNode->AddChild(testNode);

    // tc.steps: step2 GetOverlayNode in containermodal
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageA = FrameNode::CreateFrameNode("PageA", id2,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);

    auto frameNode = FrameNode::CreateFrameNode(
        "frameNode0", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    pageA->AddChild(frameNode);
    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    auto it = treesInfos.find(testNode->GetId());
    EXPECT_TRUE(it != treesInfos.end());

    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: GetOverlayNode_001
 * @tc.desc: Test the operation of GetOverlayNode in containermodal  overlay
 * column
 *    |--ContainerModal
 *       |--stage
 *          |--PageA
 *             |--frameNode
 *    |--overlay
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOverlayNode_002, TestSize.Level1)
{
    // tc.steps: step1. build tree
    auto context1 = PipelineContext::GetCurrentContext();
    ASSERT_NE(context1, nullptr);

    auto columnNode = FrameNode::CreateFrameNode(
        "Column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);

    auto id0 = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> contianerNode
        = FrameNode::CreateFrameNode(V2::CONTAINER_MODAL_ETS_TAG, id0, AceType::MakeRefPtr<Pattern>(), false);
    columnNode->AddChild(contianerNode);

    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> stageNode = FrameNode::CreateFrameNode("stage", id, AceType::MakeRefPtr<Pattern>(), false);
    context1->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageNode->children_.clear();
    contianerNode->AddChild(stageNode);

    auto overlayNode = FrameNode::CreateFrameNode(
        "overlayNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    columnNode->AddChild(overlayNode);

    auto testNode = FrameNode::CreateFrameNode(
        "testNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    overlayNode->AddChild(testNode);

    // tc.steps: step2 GetOverlayNode in containermodal
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> pageA = FrameNode::CreateFrameNode("PageA", id2,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);

    auto frameNode = FrameNode::CreateFrameNode(
        "frameNode0", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    pageA->AddChild(frameNode);

    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);

    auto it = treesInfos.find(testNode->GetId());
    EXPECT_TRUE(it != treesInfos.end());

    context1->stageManager_ = nullptr;
}

/**
 * @tc.name: GetElementRegisterNodes_001
 * @tc.desc: Test the operation of GetElementRegisterNodes
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetElementRegisterNodes_001, TestSize.Level1)
{
    auto id1 = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> ONE = FrameNode::CreateFrameNode("one", id1, AceType::MakeRefPtr<Pattern>(), true);
    auto id2 = ElementRegister::GetInstance()->MakeUniqueId();
    const RefPtr<FrameNode> TWO = FrameNode::CreateFrameNode("two", id2, AceType::MakeRefPtr<Pattern>());
    NG::InspectorTreeMap treesInfos;
    Inspector::GetElementRegisterNodes(treesInfos);
    auto it = treesInfos.find(id1);
    EXPECT_TRUE(it != treesInfos.end());
    it = treesInfos.find(id2);
    EXPECT_TRUE(it != treesInfos.end());
}

/**
 * @tc.name: InspectorOffscreenNodesMgrTest001
 * @tc.desc: Test InspectorOffscreenNodesMgr ClearOffscreenNodes/GetOffscreenNodes/GetOffscreenNodesSize
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorOffscreenNodesMgrTest001, TestSize.Level1)
{
    auto mgr = AceType::MakeRefPtr<InspectorOffscreenNodesMgr>();
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->GetOffscreenNodesSize(), 0);
    auto node1 = FrameNode::CreateFrameNode("one", 1001, AceType::MakeRefPtr<Pattern>(), true);
    auto node2 = FrameNode::CreateFrameNode("two", 1002, AceType::MakeRefPtr<Pattern>(), true);
    mgr->AddOffscreenNode(node1);
    mgr->AddOffscreenNode(node2);
    EXPECT_EQ(mgr->GetOffscreenNodesSize(), 2);
    auto nodes = mgr->GetOffscreenNodes();
    EXPECT_EQ(nodes.size(), 2);
    mgr->ClearOffscreenNodes();
    EXPECT_EQ(mgr->GetOffscreenNodesSize(), 0);
    auto emptyNodes = mgr->GetOffscreenNodes();
    EXPECT_TRUE(emptyNodes.empty());
}

/**
 * @tc.name: GetOffScreenTreeNodes_002
 * @tc.desc: Test GetOffScreenTreeNodes with null context and null offscreenNodesMgr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOffScreenTreeNodes_002, TestSize.Level1)
{
    RefPtr<MockPipelineContext> pipelineBak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    NG::InspectorTreeMap nodes;
    Inspector::GetOffScreenTreeNodes(nodes);
    EXPECT_TRUE(nodes.empty());
    MockPipelineContext::pipeline_ = pipelineBak;
}

/**
 * @tc.name: GetOffScreenTreeNodes_003
 * @tc.desc: Test GetOffScreenTreeNodes with null offscreenNodesMgr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOffScreenTreeNodes_003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto mgrBak = context->inspectorOffscreenNodesMgr_;
    context->inspectorOffscreenNodesMgr_ = nullptr;
    NG::InspectorTreeMap nodes;
    Inspector::GetOffScreenTreeNodes(nodes);
    EXPECT_TRUE(nodes.empty());
    context->inspectorOffscreenNodesMgr_ = mgrBak;
}

/**
 * @tc.name: AddOffscreenNode_002
 * @tc.desc: Test AddOffscreenNode with null pipeline and null offscreenNodesMgr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, AddOffscreenNode_002, TestSize.Level1)
{
    auto node = FrameNode::CreateFrameNode("one", 2001, AceType::MakeRefPtr<Pattern>(), true);
    RefPtr<MockPipelineContext> pipelineBak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    Inspector::AddOffscreenNode(node);
    EXPECT_EQ(node->context_, nullptr);
    MockPipelineContext::pipeline_ = pipelineBak;

    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto mgrBak = context->inspectorOffscreenNodesMgr_;
    context->inspectorOffscreenNodesMgr_ = nullptr;
    Inspector::AddOffscreenNode(node);
    context->inspectorOffscreenNodesMgr_ = mgrBak;
}

/**
 * @tc.name: RemoveOffscreenNode_002
 * @tc.desc: Test RemoveOffscreenNode with null pipeline and null offscreenNodesMgr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, RemoveOffscreenNode_002, TestSize.Level1)
{
    auto node = FrameNode::CreateFrameNode("one", 2002, AceType::MakeRefPtr<Pattern>(), true);
    RefPtr<MockPipelineContext> pipelineBak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    Inspector::RemoveOffscreenNode(node);
    MockPipelineContext::pipeline_ = pipelineBak;

    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto mgrBak = context->inspectorOffscreenNodesMgr_;
    context->inspectorOffscreenNodesMgr_ = nullptr;
    Inspector::RemoveOffscreenNode(node);
    context->inspectorOffscreenNodesMgr_ = mgrBak;
}

/**
 * @tc.name: ParseWindowIdFromMsg_002
 * @tc.desc: Test ParseWindowIdFromMsg with json not object and paramObj not object
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, ParseWindowIdFromMsg_002, TestSize.Level1)
{
    std::string msg = "[1, 2, 3]";
    auto result = Inspector::ParseWindowIdFromMsg(msg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, -1);

    msg = "{\"method\":\"ArkUI.tree\", \"params\":[1,2]}";
    result = Inspector::ParseWindowIdFromMsg(msg);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, 0);

    msg = "{\"method\":\"ArkUI.queryAbilities\", \"params\":{\"windowId\":\"20\"}}";
    result = Inspector::ParseWindowIdFromMsg(msg);
    EXPECT_EQ(result.first, 20);
    EXPECT_EQ(result.second, 2);
}

/**
 * @tc.name: ParseNeedFreeNodes_002
 * @tc.desc: Test ParseNeedFreeNodes with json not object and paramObj not object
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, ParseNeedFreeNodes_002, TestSize.Level1)
{
    std::string msg = "[1, 2, 3]";
    auto result = Inspector::ParseNeedFreeNodes(msg);
    EXPECT_FALSE(result);

    msg = "{\"method\":\"ArkUI.tree\", \"params\":[1,2]}";
    result = Inspector::ParseNeedFreeNodes(msg);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetFrameNodeByKey_003
 * @tc.desc: Test GetFrameNodeByKey with null offscreenNodesMgr
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetFrameNodeByKey_003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto mgrBak = context->inspectorOffscreenNodesMgr_;
    context->inspectorOffscreenNodesMgr_ = nullptr;
    auto nodePtr = Inspector::GetFrameNodeByKey("nonexistent_key");
    EXPECT_EQ(nodePtr, nullptr);
    context->inspectorOffscreenNodesMgr_ = mgrBak;
}

/**
 * @tc.name: GetInspectorByKey_001
 * @tc.desc: Test GetInspectorByKey traversal with id match and keyIsNull
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetInspectorByKey_001, TestSize.Level1)
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, 3001, AceType::MakeRefPtr<Pattern>(), true);
    auto child1 = FrameNode::CreateFrameNode("child1", 3002, AceType::MakeRefPtr<Pattern>());
    child1->UpdateInspectorId("myKey");
    auto child2 = FrameNode::CreateFrameNode("child2", 3003, AceType::MakeRefPtr<Pattern>());
    auto grandChild = FrameNode::CreateFrameNode("grand", 3004, AceType::MakeRefPtr<Pattern>());
    grandChild->UpdateInspectorId("deepKey");
    rootNode->AddChild(child1);
    rootNode->AddChild(child2);
    child2->AddChild(grandChild);

    auto found = Inspector::GetInspectorByKey(rootNode, "myKey");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetId(), 3002);

    found = Inspector::GetInspectorByKey(rootNode, "deepKey");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetId(), 3004);

    found = Inspector::GetInspectorByKey(rootNode, "nonexistent");
    EXPECT_EQ(found, nullptr);

    found = Inspector::GetInspectorByKey(rootNode, "");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetId(), 3001);
}

/**
 * @tc.name: NavDestinationInspector_001
 * @tc.desc: Test NavDestination handling in GetInspectorChildren with custom node
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, NavDestinationInspector_001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto parentNode = FrameNode::CreateFrameNode(
        "column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(parentNode);

    auto navDestPattern = AceType::MakeRefPtr<NavDestinationPattern>();
    auto navDestNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        [&navDestPattern]() { return navDestPattern; });
    ASSERT_NE(navDestNode, nullptr);
    parentNode->AddChild(navDestNode);

    auto navCustomNode = CustomNode::CreateCustomNode(
        ElementRegister::GetInstance()->MakeUniqueId(), "navCustom");
    navDestPattern->SetCustomNode(navCustomNode);
    navCustomNode->extraInfo_ = { "navPage", 10, 5 };

    auto childNode = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    navDestNode->AddChild(childNode);

    bool needThrow = false;
    auto result = Inspector::GetInspector(true, InspectorFilter(), needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");

    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_STREQ(jsonValue->GetValue("type")->GetString().c_str(), "root");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: NavDestinationInspector_002
 * @tc.desc: Test NavDestination handling when custom node is null
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, NavDestinationInspector_002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto parentNode = FrameNode::CreateFrameNode(
        "column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(parentNode);

    auto navDestPattern = AceType::MakeRefPtr<NavDestinationPattern>();
    auto navDestNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        [&navDestPattern]() { return navDestPattern; });
    ASSERT_NE(navDestNode, nullptr);
    parentNode->AddChild(navDestNode);

    auto childNode = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    navDestNode->AddChild(childNode);

    bool needThrow = false;
    auto result = Inspector::GetInspector(true, InspectorFilter(), needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: NavDestinationInspector_003
 * @tc.desc: Test NavDestination handling when custom node is not CustomNode type
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, NavDestinationInspector_003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto parentNode = FrameNode::CreateFrameNode(
        "column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(parentNode);

    auto navDestPattern = AceType::MakeRefPtr<NavDestinationPattern>();
    auto navDestNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        [&navDestPattern]() { return navDestPattern; });
    ASSERT_NE(navDestNode, nullptr);
    parentNode->AddChild(navDestNode);

    auto nonCustomNode = FrameNode::CreateFrameNode(
        "nonCustom", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    navDestPattern->SetCustomNode(nonCustomNode);

    auto childNode = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    navDestNode->AddChild(childNode);

    bool needThrow = false;
    auto result = Inspector::GetInspector(true, InspectorFilter(), needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: NavDestinationInspector_004
 * @tc.desc: Test NavDestination handling in non-layout inspector mode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, NavDestinationInspector_004, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto parentNode = FrameNode::CreateFrameNode(
        "column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(parentNode);

    auto navDestPattern = AceType::MakeRefPtr<NavDestinationPattern>();
    auto navDestNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        [&navDestPattern]() { return navDestPattern; });
    ASSERT_NE(navDestNode, nullptr);
    parentNode->AddChild(navDestNode);

    auto navCustomNode = CustomNode::CreateCustomNode(
        ElementRegister::GetInstance()->MakeUniqueId(), "navCustom");
    navDestPattern->SetCustomNode(navCustomNode);

    auto childNode = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    navDestNode->AddChild(childNode);

    bool needThrow = false;
    auto result = Inspector::GetInspector(false, InspectorFilter(), needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: AddInspectorTreeNode_001
 * @tc.desc: Test AddInspectorTreeNode with null node and non-processable type
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, AddInspectorTreeNode_001, TestSize.Level1)
{
    NG::InspectorTreeMap treesInfos;

    RefPtr<NG::UINode> nullNode = nullptr;
    auto recNode = Inspector::AddInspectorTreeNode(nullNode, treesInfos);
    EXPECT_EQ(recNode, nullptr);

    auto testNode = InspectorTestNode::CreateTestNode(4001);
    recNode = Inspector::AddInspectorTreeNode(testNode, treesInfos);
    EXPECT_EQ(recNode, nullptr);
}

/**
 * @tc.name: AddInspectorTreeNode_002
 * @tc.desc: Test AddInspectorTreeNode with null renderContext and custom node tag
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, AddInspectorTreeNode_002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode("frame", 4002, AceType::MakeRefPtr<Pattern>(), true);
    auto renderContextBak = frameNode->renderContext_;
    frameNode->renderContext_ = nullptr;
    NG::InspectorTreeMap treesInfos;
    auto recNode = Inspector::AddInspectorTreeNode(frameNode, treesInfos);
    ASSERT_NE(recNode, nullptr);
    frameNode->renderContext_ = renderContextBak;

    auto customNode = CustomNode::CreateCustomNode(4003, "myCustomTag");
    customNode->SetJSViewName("myCustomTag");
    recNode = Inspector::AddInspectorTreeNode(customNode, treesInfos);
    ASSERT_NE(recNode, nullptr);
    EXPECT_EQ(recNode->GetName(), "myCustomTag");
}

/**
 * @tc.name: GetInspectorChildrenInfo_002
 * @tc.desc: Test GetInspectorChildrenInfo with overlayNode and depth 0
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetInspectorChildrenInfo_002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode("parent", 4004, AceType::MakeRefPtr<Pattern>(), true);
    auto childNode = FrameNode::CreateFrameNode("child", 4005, AceType::MakeRefPtr<Pattern>(), true);
    frameNode->AddChild(childNode);

    auto overlayChild = FrameNode::CreateFrameNode("overlayChild", 4006, AceType::MakeRefPtr<Pattern>(), true);
    frameNode->SetOverlayNode(overlayChild);

    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorChildrenInfo(frameNode, treesInfos, 1, 0);
    EXPECT_TRUE(treesInfos.empty());

    treesInfos.clear();
    Inspector::GetInspectorChildrenInfo(frameNode, treesInfos, 1, 1);
    EXPECT_FALSE(treesInfos.empty());
}

/**
 * @tc.name: GetRecordAllPagesNodes_003
 * @tc.desc: Test GetRecordAllPagesNodes with non-PagePattern child
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetRecordAllPagesNodes_003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        "stageNode", 4007, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);

    auto nonPageNode = FrameNode::CreateFrameNode("nonPage", 4008, AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(nonPageNode);

    auto pageNode = FrameNode::CreateFrameNode("page", 4009,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);

    NG::InspectorTreeMap treesInfos;
    Inspector::GetRecordAllPagesNodes(treesInfos);
    auto it = treesInfos.find(4008);
    EXPECT_TRUE(it == treesInfos.end());
    it = treesInfos.find(4009);
    EXPECT_TRUE(it != treesInfos.end());
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetCustomNodeInfo_002
 * @tc.desc: Test GetCustomNodeInfo with non-empty debugLine
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetCustomNodeInfo_002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        "stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto customNode = CustomNode::CreateCustomNode(
        ElementRegister::GetInstance()->MakeUniqueId(), V2::JS_VIEW_ETS_TAG);
    customNode->extraInfo_ = { "pages/test", 30, 40 };
    pageNode->AddChild(customNode);
    customNode->debugLine_ = "pages/test(30:40)";

    bool needThrow = false;
    auto result = Inspector::GetInspector(true, InspectorFilter(), needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetOverlayNode_003
 * @tc.desc: Test GetOverlayNode when overlay tag is stage and container modal not found
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOverlayNode_003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto stageParent = FrameNode::CreateFrameNode(
        "stageParent", 4010, AceType::MakeRefPtr<Pattern>(), true);
    auto stageNode = FrameNode::CreateFrameNode(
        "stage", 4011, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    stageParent->AddChild(stageNode);

    auto pageNode = FrameNode::CreateFrameNode(
        "page", 4012, AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto frameNode = FrameNode::CreateFrameNode("frameNode", 4013, AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(frameNode);

    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    auto it = treesInfos.find(frameNode->GetId());
    EXPECT_TRUE(it != treesInfos.end());
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetOverlayNode_004
 * @tc.desc: Test GetOverlayNode with container modal where overlay is container modal
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetOverlayNode_004, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto columnNode = FrameNode::CreateFrameNode(
        "Column", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    auto containerNode = FrameNode::CreateFrameNode(
        V2::CONTAINER_MODAL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>(), false);
    columnNode->AddChild(containerNode);
    auto stageNode = FrameNode::CreateFrameNode(
        "stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    containerNode->AddChild(stageNode);

    auto containerModal2 = FrameNode::CreateFrameNode(
        V2::CONTAINER_MODAL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>(), false);
    columnNode->AddChild(containerModal2);

    auto pageNode = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto frameNode = FrameNode::CreateFrameNode(
        "frameNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), false);
    pageNode->AddChild(frameNode);

    NG::InspectorTreeMap treesInfos;
    Inspector::GetInspectorTree(treesInfos);
    auto it = treesInfos.find(frameNode->GetId());
    EXPECT_TRUE(it != treesInfos.end());
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetFreeNodesRoot_001
 * @tc.desc: Test GetRoot with onMainTree and no parent (root)
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetFreeNodesRoot_001, TestSize.Level1)
{
    ElementRegister::GetInstance()->Clear();
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    context->rootNode_ = stageNode;
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(pageNode);

    auto freeNode = FrameNode::CreateFrameNode(
        FREE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    freeNode->onMainTree_ = false;

    auto mainTreeNode = FrameNode::CreateFrameNode(
        MAIN_TREE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    mainTreeNode->onMainTree_ = true;

    auto result = Inspector::GetFreeNodesInspector();
    EXPECT_NE(result, "");
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_STREQ(jsonValue->GetString("type").c_str(), "root");

    context->stageManager_ = nullptr;
    context->rootNode_ = nullptr;
}

/**
 * @tc.name: GetInspectorInfo_001
 * @tc.desc: Test GetInspectorInfo with null pipeline in isLayoutInspector mode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetInspectorInfo_001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    stageNode->AddChild(pageNode);

    RefPtr<MockPipelineContext> pipelineBak = MockPipelineContext::pipeline_;
    MockPipelineContext::pipeline_ = nullptr;
    auto result = Inspector::GetInspector(true);
    EXPECT_NE(result, "");
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_FALSE(jsonValue->Contains("VsyncID"));
    MockPipelineContext::pipeline_ = pipelineBak;
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetSubWindowInspector_002
 * @tc.desc: Test GetSubWindowInspector with valid overlay node and children
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetSubWindowInspector_002, TestSize.Level1)
{
    auto context = MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto overlayRoot = FrameNode::CreateFrameNode(
        "overlayRoot", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto overlayChild = FrameNode::CreateFrameNode(
        "overlayChild", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    overlayRoot->AddChild(overlayChild);
    overlayChild->isActive_ = true;

    RefPtr<MockOverlayManager> overlayManager = AceType::MakeRefPtr<MockOverlayManager>(overlayRoot);
    auto overlayManagerBak = context->overlayManager_;
    context->overlayManager_ = overlayManager;

    auto result = Inspector::GetSubWindowInspector(true);
    EXPECT_NE(result, "");
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_STREQ(jsonValue->GetValue("type")->GetString().c_str(), "root");

    result = Inspector::GetSubWindowInspector(false);
    EXPECT_NE(result, "");
    context->overlayManager_ = overlayManagerBak;
}

/**
 * @tc.name: GetAllPageNodes_002
 * @tc.desc: Test GetAllPageNodes with multiple pages where lastPage is skipped
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetAllPageNodes_002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    context->rootNode_ = stageNode;

    auto pageA = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    auto pageB = FrameNode::CreateFrameNode(
        "page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageA);
    stageNode->AddChild(pageB);

    auto childA = FrameNode::CreateFrameNode(
        "childA", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageA->AddChild(childA);
    auto childB = FrameNode::CreateFrameNode(
        "childB", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    pageB->AddChild(childB);

    InspectorFilter filter;
    filter.EnableFreeNodes();
    bool needThrow = false;
    auto result = Inspector::GetInspector(true, filter, needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_NE(result, "");

    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);
    EXPECT_TRUE(jsonValue->Contains(INSPECTOR_OTHER_CONTENTS));

    context->stageManager_ = nullptr;
    context->rootNode_ = nullptr;
}

/**
 * @tc.name: GetFrameNodeChildren_001
 * @tc.desc: Test GetFrameNodeChildren with custom node in isLayoutInspector mode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetFrameNodeChildren_001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    auto customNode = CustomNode::CreateCustomNode(
        ElementRegister::GetInstance()->MakeUniqueId(), "customNode");
    pageNode->AddChild(customNode);

    auto result = Inspector::GetInspector(true);
    EXPECT_NE(result, "");
    auto jsonValue = JsonUtil::ParseJsonString(result);
    ASSERT_NE(jsonValue, nullptr);

    result = Inspector::GetInspector(false);
    EXPECT_NE(result, "");
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: GetInspector_003
 * @tc.desc: Test GetInspector with non-empty key where GetLastPage returns null
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetInspector_003, TestSize.Level1)
{
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto context = MockPipelineContext::GetCurrentContext();
    RefPtr<MockStageManager> stageManager = AceType::MakeRefPtr<MockStageManager>(stageNode);
    auto stageManagerBak = context->stageManager_;
    context->stageManager_ = stageManager;
    RefPtr<FrameNode> nullNode = nullptr;
    EXPECT_CALL(*stageManager, GetLastPage()).Times(1).WillOnce(Return(nullNode));

    InspectorFilter filter;
    std::string filterId = "nonexistent";
    filter.SetFilterID(filterId);
    bool needThrow = false;
    auto result = Inspector::GetInspector(false, filter, needThrow);
    EXPECT_TRUE(needThrow);
    context->stageManager_ = stageManagerBak;
}

/**
 * @tc.name: GetInspector_004
 * @tc.desc: Test GetInspector with non-empty key where GetInspectorByKey returns null
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, GetInspector_004, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stageNode = FrameNode::CreateFrameNode(
        STAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stageNode);
    auto pageNode = FrameNode::CreateFrameNode(
        PAGE_NODE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    stageNode->AddChild(pageNode);
    pageNode->hostPageId_ = pageNode->GetPageId();

    InspectorFilter filter;
    std::string filterId = "nonexistent_key";
    filter.SetFilterID(filterId);
    bool needThrow = false;
    auto result = Inspector::GetInspector(false, filter, needThrow);
    EXPECT_TRUE(needThrow);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: RecordOnePageNodes_001
 * @tc.desc: Test RecordOnePageNodes with null pageNode
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, RecordOnePageNodes_001, TestSize.Level1)
{
    NG::InspectorTreeMap treesInfos;
    Inspector::RecordOnePageNodes(nullptr, treesInfos);
    EXPECT_TRUE(treesInfos.empty());
}

/**
 * @tc.name: RecordOnePageNodes_002
 * @tc.desc: Test RecordOnePageNodes with valid page node
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, RecordOnePageNodes_002, TestSize.Level1)
{
    auto pageNode = FrameNode::CreateFrameNode("page", 4020,
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    auto childNode = FrameNode::CreateFrameNode("child", 4021, AceType::MakeRefPtr<Pattern>(), true);
    pageNode->AddChild(childNode);
    NG::InspectorTreeMap treesInfos;
    Inspector::RecordOnePageNodes(pageNode, treesInfos);
    EXPECT_FALSE(treesInfos.empty());
    auto it = treesInfos.find(4020);
    EXPECT_TRUE(it != treesInfos.end());
    it = treesInfos.find(4021);
    EXPECT_TRUE(it != treesInfos.end());
}

namespace {
class InsetInspectorFlagNode : public FrameNode {
    DECLARE_ACE_TYPE(InsetInspectorFlagNode, FrameNode);
public:
    InsetInspectorFlagNode() : FrameNode("insetInspectorHost", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>()) {}

    const std::list<RefPtr<UINode>>& GetChildren(bool notDetach = false) const override
    {
        queries.push_back(notDetach);
        return UINode::GetChildren(notDetach);
    }

    mutable std::vector<bool> queries;
};
}

/**
 * @tc.name: SafeAreaInsetInspectorLookup001
 * @tc.desc: Inspector reaches content by key, preserves notDetach, and drops replaced/reset roots.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, SafeAreaInsetInspectorLookup001, TestSize.Level0)
{
    auto host = AceType::MakeRefPtr<InsetInspectorFlagNode>();
    host->InitializePatternAndContext();
    auto content = FrameNode::CreateFrameNode("insetInspectorContent",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    content->UpdateInspectorId("inset-observed");
    SafeAreaInsetOptions options;
    options.edges[0] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[0] = content;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    for (bool notDetach : { false, true }) {
        host->queries.clear();
        EXPECT_EQ(Inspector::GetInspectorByKey(host, "inset-observed", notDetach), content);
        ASSERT_FALSE(host->queries.empty());
        for (bool flag : host->queries) {
            EXPECT_EQ(flag, notDetach);
        }
    }
    auto replacement = FrameNode::CreateFrameNode("replacementInspectorContent",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    replacement->UpdateInspectorId("inset-replacement");
    contents[0] = replacement;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    EXPECT_EQ(Inspector::GetInspectorByKey(host, "inset-observed"), nullptr);
    EXPECT_EQ(Inspector::GetInspectorByKey(host, "inset-replacement"), replacement);
    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(Inspector::GetInspectorByKey(host, "inset-replacement"), nullptr);
}

/**
 * @tc.name: SafeAreaInsetInspectorTree001
 * @tc.desc: Inspector tree maps include actual content once, keep the carrier hidden, and respect depth.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, SafeAreaInsetInspectorTree001, TestSize.Level0)
{
    auto makeNode = [](const char* tag) {
        return FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>());
    };
    auto host = makeNode("insetInspectorHost");
    auto business = makeNode("ordinaryInspectorChild");
    business->MountToParent(host);
    auto content = makeNode("insetInspectorContent");
    content->SetDebugLine("inset-content-debug-line");
    business->SetDebugLine("business-debug-line");
    SafeAreaInsetOptions options;
    options.edges[2] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[2] = content;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    InspectorTreeMap nodes;
    Inspector::GetInspectorChildrenInfo(host, nodes, 0, 0);
    EXPECT_TRUE(nodes.empty());
    Inspector::GetInspectorChildrenInfo(host, nodes, 0, 1);
    EXPECT_EQ(nodes.count(content->GetId()), 1);
    ASSERT_NE(nodes.find(content->GetId()), nodes.end());
    EXPECT_EQ(nodes.at(content->GetId())->GetParentId(), host->GetId());
    EXPECT_EQ(nodes.count(business->GetId()), 1);
    EXPECT_EQ(nodes.count(carrier->GetId()), 0);
    EXPECT_EQ(nodes.size(), 2);
    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    nodes.clear();
    Inspector::GetInspectorChildrenInfo(host, nodes, 0, 1);
    EXPECT_EQ(nodes.count(content->GetId()), 0);
    EXPECT_EQ(nodes.count(business->GetId()), 1);
}

/**
 * @tc.name: SafeAreaInsetInspectorJson001
 * @tc.desc: Normal and layout Inspector JSON retain inset content while hiding the internal carrier.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, SafeAreaInsetInspectorJson001, TestSize.Level0)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto stage = FrameNode::CreateFrameNode("stage", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<StagePattern>());
    auto page = FrameNode::CreateFrameNode("page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    page->MountToParent(stage);
    auto host = FrameNode::CreateFrameNode("insetJsonHost", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    host->SetDebugLine("inset-host-line");
    host->SetActive(true);
    host->MountToParent(page);
    auto content = FrameNode::CreateFrameNode("insetJsonContent", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    content->SetDebugLine("inset-content-line");
    content->SetActive(true);
    SafeAreaInsetOptions options;
    options.edges[1] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[1] = content;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    auto previousStage = context->stageManager_;
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(stage);
    const auto normal = Inspector::GetInspector(false);
    const auto layout = Inspector::GetInspector(true);
    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    const auto cleared = Inspector::GetInspector(false);
    context->stageManager_ = previousStage;
    for (const auto& text : { normal, layout }) {
        EXPECT_NE(text.find("insetJsonContent"), std::string::npos);
        EXPECT_EQ(text.find("SafeAreaInsetContainer"), std::string::npos);
    }
    EXPECT_EQ(cleared.find("insetJsonContent"), std::string::npos);
}

/**
 * @tc.name: SafeAreaInsetInspectorFourEdges001
 * @tc.desc: All four roots and a nested custom component are observable without changing business children.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, SafeAreaInsetInspectorFourEdges001, TestSize.Level0)
{
    auto makeNode = [](const char* tag) {
        auto node = FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>());
        node->SetDebugLine(tag);
        return node;
    };
    auto host = makeNode("inspectorFourHost");
    auto business = makeNode("inspectorFourBusiness");
    business->MountToParent(host);
    SafeAreaInsetOptions options;
    SafeAreaInsetContentArray contents;
    for (size_t edge = 0; edge < SAFE_AREA_INSET_EDGE_COUNT; ++edge) {
        options.edges[edge] = SafeAreaInsetEdgeOptions {};
        auto content = makeNode("inspectorFourContent");
        content->UpdateInspectorId("inspector-edge-" + std::to_string(edge));
        contents[edge] = content;
    }
    auto custom = CustomNode::CreateCustomNode(ElementRegister::GetInstance()->MakeUniqueId(), "inspector-custom");
    auto nested = makeNode("inspectorFourNested");
    nested->UpdateInspectorId("inspector-nested");
    nested->MountToParent(custom);
    custom->MountToParent(contents[3]);
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    InspectorTreeMap nodes;
    Inspector::GetInspectorChildrenInfo(host, nodes, 0, 1);
    EXPECT_EQ(nodes.size(), 5);
    EXPECT_EQ(nodes.count(carrier->GetId()), 0);
    EXPECT_EQ(nodes.count(nested->GetId()), 0);
    for (size_t edge = 0; edge < SAFE_AREA_INSET_EDGE_COUNT; ++edge) {
        EXPECT_EQ(Inspector::GetInspectorByKey(host, "inspector-edge-" + std::to_string(edge)), contents[edge]);
        ASSERT_EQ(nodes.count(contents[edge]->GetId()), 1);
        EXPECT_EQ(nodes.at(contents[edge]->GetId())->GetParentId(), host->GetId());
    }
    EXPECT_EQ(Inspector::GetInspectorByKey(host, "inspector-nested", true), nested);
    EXPECT_EQ(host->GetChildren().size(), 1);
    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(Inspector::GetInspectorByKey(host, "inspector-nested"), nullptr);
    nodes.clear();
    Inspector::GetInspectorChildrenInfo(host, nodes, 0, 1);
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.count(business->GetId()), 1);
}

namespace {
// Getter helpers follow the API memory contract: the callback allocates the
// payload buffer with malloc() and the engine releases it with free() after
// deep-copying, so every invocation returns a fresh malloc'd buffer.
void ExternalPayloadFromUserData(void* userData, char** inspectorTree)
{
    const auto* payload = static_cast<const std::string*>(userData);
    if (payload == nullptr || payload->empty()) {
        *inspectorTree = nullptr;
        return;
    }
    size_t length = payload->size() + 1; // including the terminating NUL
    char* buffer = static_cast<char*>(malloc(length));
    if (buffer == nullptr) {
        *inspectorTree = nullptr;
        return;
    }
    errno_t result = memcpy_s(buffer, length, payload->c_str(), length);
    if (result != 0) {
        free(buffer);
        *inspectorTree = nullptr;
        return;
    }
    *inspectorTree = buffer;
}

void ExternalPayloadNull(void* /*userData*/, char** inspectorTree)
{
    *inspectorTree = nullptr;
}

// Returns a fixed JSON payload each call (fresh malloc'd buffer); used to
// demonstrate multiple getter callbacks coexisting on one node.
void ExternalPayloadConstant(void* /*userData*/, char** inspectorTree)
{
    static const char payload[] = R"([{"$type":"MultiTwo","$ID":90022}])";
    size_t length = sizeof(payload); // including the terminating NUL
    char* buffer = static_cast<char*>(malloc(length));
    if (buffer == nullptr) {
        *inspectorTree = nullptr;
        return;
    }
    errno_t result = memcpy_s(buffer, length, payload, length);
    if (result != 0) {
        free(buffer);
        *inspectorTree = nullptr;
        return;
    }
    *inspectorTree = buffer;
}

std::unique_ptr<JsonValue> FindInspectorNodeById(JsonValue* node, int32_t nodeId)
{
    CHECK_NULL_RETURN(node, nullptr);
    if (node->GetInt("$ID", -1) == nodeId) {
        return JsonUtil::ParseJsonString(node->ToString());
    }
    auto children = node->GetValue("$children");
    CHECK_NULL_RETURN(children, nullptr);
    int32_t size = children->GetArraySize();
    for (int32_t index = 0; index < size; ++index) {
        auto child = children->GetArrayItem(index);
        auto result = FindInspectorNodeById(child.get(), nodeId);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

// Finds the node with the given id in an inspector tree (layout format has the
// page nodes under "content"; non-layout format has them at the root level).
std::unique_ptr<JsonValue> FindNodeInTree(const std::string& tree, int32_t nodeId)
{
    auto root = JsonUtil::ParseJsonString(tree);
    CHECK_NULL_RETURN(root, nullptr);
    auto content = root->GetValue("content");
    if (content) {
        auto result = FindInspectorNodeById(content.get(), nodeId);
        if (result) {
            return result;
        }
    }
    return FindInspectorNodeById(root.get(), nodeId);
}

bool NodeChildrenContainType(JsonValue* node, const std::string& type)
{
    CHECK_NULL_RETURN(node, false);
    auto children = node->GetValue("$children");
    CHECK_NULL_RETURN(children, false);
    int32_t size = children->GetArraySize();
    for (int32_t index = 0; index < size; ++index) {
        auto child = children->GetArrayItem(index);
        if (child && child->GetString("$type") == type) {
            return true;
        }
    }
    return false;
}

// Returns the index of the first direct child with the given "$type" in the
// node's "$children" array, or -1 when absent (used for order assertions).
int32_t FindChildIndexByType(JsonValue* node, const std::string& type)
{
    CHECK_NULL_RETURN(node, -1);
    auto children = node->GetValue("$children");
    CHECK_NULL_RETURN(children, -1);
    int32_t size = children->GetArraySize();
    for (int32_t index = 0; index < size; ++index) {
        auto child = children->GetArrayItem(index);
        if (child && child->GetString("$type") == type) {
            return index;
        }
    }
    return -1;
}

// External host format sample (CustomNode/XComponent provider output) used by
// the format-conversion tests. Delimited raw string: the payload contains ')"'
// inside "Offset(...)" attribute values, which would terminate a plain R"(...)"
// literal early. Non-const: Register takes the userData as void*, and a
// const std::string* cannot implicitly convert to void*.
std::string g_externalHostFormatPayload = R"json([
  {
    "elementId": 1,
    "parentId": -1,
    "componentType": "CustomFrameNode",
    "screenRect": {"x": 0.0, "y": 0.0, "width": 1316.0, "height": 2598.0},
    "attributes": {"visible": true, "enabled": true, "clickable": false,
    "focused": false, "scrollable": false, "selected": false,
    "isPlaced": "true", "width": "1316", "height": "2598",
    "positionInParent": "Offset(0.0, 0.0)"},
    "children": [
      {
        "elementId": 11,
        "parentId": 1,
        "componentType": "Text",
        "screenRect": {"x": 392.0, "y": 63.0, "width": 533.0, "height": 98.0},
        "attributes": {"visible": true, "enabled": true, "clickable": false,
        "focused": false, "scrollable": false, "selected": false,
        "isPlaced": "true", "width": "533", "height": "98",
        "positionInParent": "Offset(0.0, 0.0)", "text": "属性示例 - Skia",
        "fontSize": "22.0.sp", "actualFontSize": "22.0.sp"},
        "children": []
      }
    ]
  },
  {
    "elementId": 200,
    "parentId": -1,
    "componentType": "CustomFrameNode",
    "screenRect": {"x": 168.0, "y": 350.0, "width": 980.0, "height": 504.0},
    "attributes": {"visible": true, "enabled": true, "clickable": false,
    "focused": false, "scrollable": false, "selected": false,
    "isPlaced": "true", "width": "980", "height": "504",
    "positionInParent": "Offset(0.0, 0.0)"},
    "children": [
      {
        "elementId": 201,
        "parentId": 200,
        "componentType": "Button",
        "screenRect": {"x": 364.0, "y": 630.0, "width": 588.0, "height": 112.0},
        "attributes": {"visible": true, "enabled": true, "clickable": true,
        "focused": false, "scrollable": false, "selected": false,
        "isPlaced": "true", "width": "588", "height": "112",
        "positionInParent": "Offset(0.0, 742.0)",
        "text": "确定"},
        "children": []
      }
    ]
  }
])json";

// Verifies the first converted root (elementId 1 with Text child 11) of the
// external host-format sample payload.
void AssertExternalHostFormatRoot1(JsonValue* hostNode)
{
    auto root1 = FindInspectorNodeById(hostNode, 1);
    ASSERT_NE(root1, nullptr);
    EXPECT_EQ(root1->GetString("$type"), "CustomFrameNode");
    EXPECT_EQ(root1->GetString("$rect"), "[0.00, 0.00],[1316.00,2598.00]");
    EXPECT_FALSE(root1->Contains("parentId"));
    EXPECT_FALSE(root1->Contains("elementId"));
    auto attrs1 = root1->GetValue("$attrs");
    ASSERT_NE(attrs1, nullptr);
    EXPECT_TRUE(attrs1->GetBool("visible", false));
    EXPECT_EQ(attrs1->GetString("width"), "1316");
    auto textNode = FindInspectorNodeById(root1.get(), 11);
    ASSERT_NE(textNode, nullptr);
    EXPECT_EQ(textNode->GetString("$type"), "Text");
    EXPECT_EQ(textNode->GetString("$rect"), "[392.00, 63.00],[925.00,161.00]");
    auto textAttrs = textNode->GetValue("$attrs");
    ASSERT_NE(textAttrs, nullptr);
    EXPECT_EQ(textAttrs->GetString("text"), "属性示例 - Skia");
    EXPECT_EQ(textAttrs->GetString("fontSize"), "22.0.sp");
}

// Verifies the second converted root (elementId 200 with Button child 201).
void AssertExternalHostFormatRoot200(JsonValue* hostNode)
{
    auto root200 = FindInspectorNodeById(hostNode, 200);
    ASSERT_NE(root200, nullptr);
    EXPECT_EQ(root200->GetString("$type"), "CustomFrameNode");
    EXPECT_EQ(root200->GetString("$rect"), "[168.00, 350.00],[1148.00,854.00]");
    EXPECT_FALSE(root200->Contains("parentId"));
    auto buttonNode = FindInspectorNodeById(root200.get(), 201);
    ASSERT_NE(buttonNode, nullptr);
    EXPECT_EQ(buttonNode->GetString("$type"), "Button");
    EXPECT_EQ(buttonNode->GetString("$rect"), "[364.00, 630.00],[952.00,742.00]");
    auto buttonAttrs = buttonNode->GetValue("$attrs");
    ASSERT_NE(buttonAttrs, nullptr);
    EXPECT_EQ(buttonAttrs->GetString("text"), "确定");
    EXPECT_TRUE(buttonAttrs->GetBool("clickable", false));
}
} // namespace

/**
 * @tc.name: InspectorExternalSubtreeSpliced001
 * @tc.desc: AC-1.1/R-1: registered provider subtree is spliced verbatim under
 *           the host node's $children during collection.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeSpliced001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string payload = R"([{"$type":"FlutterView","$ID":90001}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    EXPECT_NE(tree, "");
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "FlutterView"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeNoRegistrationUnchanged002
 * @tc.desc: AC-1.2/R-3/INV-4: no registration or empty payload keeps the tree
 *           output identical to the baseline.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeNoRegistrationUnchanged002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string baseline = Inspector::GetInspector(true);

    // Provider returns nullptr => no data this time, output identical.
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadNull, nullptr),
        InspectorTreeProviderRegistry::RESULT_OK);
    EXPECT_EQ(Inspector::GetInspector(true), baseline);

    std::string payload = R"([{"$type":"FlutterView","$ID":90002}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);
    EXPECT_NE(Inspector::GetInspector(true), baseline);

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    EXPECT_EQ(Inspector::GetInspector(true), baseline);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeReplace003
 * @tc.desc: AC-1.4/R-2: re-registering on the same anchor replaces the
 *           previous provider as a whole.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeReplace003, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string payloadA = R"([{"$type":"FlutterA","$ID":90003}])";
    std::string payloadB = R"([{"$type":"FlutterB","$ID":90004}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payloadA),
        InspectorTreeProviderRegistry::RESULT_OK);
    auto treeA = Inspector::GetInspector(true);
    auto hostNodeA = FindNodeInTree(treeA, hostId);
    ASSERT_NE(hostNodeA, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNodeA.get(), "FlutterA"));
    EXPECT_FALSE(NodeChildrenContainType(hostNodeA.get(), "FlutterB"));

    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payloadB),
        InspectorTreeProviderRegistry::RESULT_OK);
    auto treeB = Inspector::GetInspector(true);
    auto hostNodeB = FindNodeInTree(treeB, hostId);
    ASSERT_NE(hostNodeB, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNodeB.get(), "FlutterB"));
    EXPECT_FALSE(NodeChildrenContainType(hostNodeB.get(), "FlutterA"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeHostInvalid004
 * @tc.desc: AC-1.3/INV-2: after the host node is destroyed, a re-created node
 *           reusing the same node id must not splice the stale registration.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeHostInvalid004, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto reusedId = ElementRegister::GetInstance()->MakeUniqueId();
    {
        RefPtr<FrameNode> host =
            FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, reusedId, AceType::MakeRefPtr<Pattern>());
        std::string payload = R"([{"$type":"FlutterView","$ID":90005}])";
        ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
            InspectorTreeProviderRegistry::RESULT_OK);
        // host destroyed at scope end while still registered.
    }
    // A new node reusing the same id (same instance) must not match the dead anchor.
    RefPtr<FrameNode> host2 = FrameNode::CreateFrameNode("host2", reusedId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host2);
    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, reusedId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_FALSE(NodeChildrenContainType(hostNode.get(), "FlutterView"));
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeDegradeAndRecover005
 * @tc.desc: AC-3.1/AC-3.2/R-11/INV-6: malformed payload degrades in isolation;
 *           the next valid payload recovers without re-registration.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeDegradeAndRecover005, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string baseline = Inspector::GetInspector(true);

    std::string payload = R"([{"$type":"FlutterView","$ID":90006}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);
    // Malformed JSON: skipped, main tree intact.
    payload = "{\"$type\":\"FlutterView\",\"$ID\":90006"; // broken JSON
    EXPECT_EQ(Inspector::GetInspector(true), baseline);
    // Oversized payload (valid JSON, > 4 MiB content): skipped by the size
    // limit, main tree intact.
    payload = "{\"$type\":\"Big\",\"$ID\":90006,\"$attrs\":{\"pad\":\"" + std::string(4194305, 'a') + "\"}}";
    EXPECT_EQ(Inspector::GetInspector(true), baseline);
    // Recovery: valid payload splices again without re-registration.
    payload = R"([{"$type":"FlutterView","$ID":90006}])";
    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "FlutterView"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeInstanceIsolation006
 * @tc.desc: AC-1.5/R-5: registrations are keyed by (instanceId, nodeId); a
 *           node with a different instance id never matches another
 *           instance's registration.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeInstanceIsolation006, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto reusedId = ElementRegister::GetInstance()->MakeUniqueId();
    {
        // Registered in the default instance (instanceId -1).
        RefPtr<FrameNode> host =
            FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, reusedId, AceType::MakeRefPtr<Pattern>());
        std::string payload = R"([{"$type":"FlutterView","$ID":90007}])";
        ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
            InspectorTreeProviderRegistry::RESULT_OK);
    }
    // Re-created node with the same id but a different instance id must not match.
    RefPtr<FrameNode> otherInstanceHost =
        FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, reusedId, AceType::MakeRefPtr<Pattern>());
    otherInstanceHost->instanceId_ = 42; // private member access via test macro
    page->AddChild(otherInstanceHost);
    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, reusedId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_FALSE(NodeChildrenContainType(hostNode.get(), "FlutterView"));

    // Registering under the node's own instance key works normally.
    std::string payload = R"([{"$type":"FlutterView","$ID":90008}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(
                  AceType::RawPtr(otherInstanceHost), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);
    tree = Inspector::GetInspector(true);
    hostNode = FindNodeInTree(tree, reusedId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "FlutterView"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(otherInstanceHost), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeCustomHost007
 * @tc.desc: AC-1.1/R-1: CustomNode hosts also participate in external-subtree
 *           splicing (layout inspector mode).
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeCustomHost007, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<CustomNode> host = CustomNode::CreateCustomNode(hostId, "inspector_custom_host");
    ASSERT_NE(host, nullptr);
    page->AddChild(host);
    std::string payload = R"([{"$type":"CmpMockRoot","$ID":90009}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    EXPECT_NE(tree, "");
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "CmpMockRoot"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeNonHostRejected008
 * @tc.desc: AC-1.7/R-1: a getter registered on a node that is neither a
 *           CustomNode nor an XComponent is rejected at registration time
 *           (RESULT_UNSUPPORTED_HOST; no entry is stored and the tree output
 *           stays identical to the baseline).
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeNonHostRejected008, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    // Plain column host: not CustomNode / XComponent.
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode("column", hostId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(host);
    std::string baseline = Inspector::GetInspector(true);

    std::string payload = R"([{"$type":"FlutterView","$ID":90010}])";
    // Rejected at registration: unsupported host type, no entry stored.
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_UNSUPPORTED_HOST);
    std::vector<InspectorTreeProviderRegistry::GetterEntry> registeredGetters;
    InspectorTreeProviderRegistry::GetInspectorGettersForNode(host, registeredGetters);
    EXPECT_TRUE(registeredGetters.empty());
    // Tree output unchanged.
    EXPECT_EQ(Inspector::GetInspector(true), baseline);

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    EXPECT_EQ(Inspector::GetInspector(true), baseline);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorWebSubtreeUnitTestStub009
 * @tc.desc: Web-tag node in the unit-test build: the Web subtree branch is
 *           compiled as a no-op (the WebPattern header chain is not part of
 *           the unit-test source set), so collection stays stable and the
 *           tree output is identical to the baseline. The public C API
 *           registration on the Web host keeps being rejected by the
 *           host-type whitelist. (The real Web branch is exercised by the
 *           product build via WEB_SUPPORTED.)
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorWebSubtreeUnitTestStub009, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::WEB_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(host);
    std::string baseline = Inspector::GetInspector(true);

    // Web-tag node present: collection stays stable, output unchanged.
    EXPECT_EQ(Inspector::GetInspector(true), baseline);

    // Public registration on the Web host keeps being rejected by the
    // host-type whitelist.
    std::string payload = R"([{"$type":"rootWebArea","$ID":90020}])";
    EXPECT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_UNSUPPORTED_HOST);
    EXPECT_EQ(Inspector::GetInspector(true), baseline);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeHostFormatConverted010
 * @tc.desc: AC-1.1: a payload in the external host format
 *           (elementId/componentType/screenRect/attributes/children) is
 *           converted to the inspector node schema before splicing:
 *           elementId -> "$ID", componentType -> "$type", screenRect ->
 *           "$rect" ("[left, top],[right, bottom]"), attributes -> "$attrs",
 *           children -> "$children" (recursively); "parentId" and unknown
 *           fields are dropped. Native-schema payloads keep being spliced
 *           verbatim (covered by 001).
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeHostFormatConverted010, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(
                  AceType::RawPtr(host), ExternalPayloadFromUserData, &g_externalHostFormatPayload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "CustomFrameNode"));
    AssertExternalHostFormatRoot1(hostNode.get());
    AssertExternalHostFormatRoot200(hostNode.get());

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeScreenToWindow011
 * @tc.desc: External "screenRect" values are screen-space while the
 *           inspector "$rect" is window-relative: the window origin
 *           (GetDisplayWindowRectInfo) is subtracted during conversion,
 *           so with the window placed at screen (100, 200) a screen rect
 *           {150, 263, 533, 98} is spliced as "[50.00, 63.00],[583.00,161.00]"
 *           (full-screen windows at screen origin keep the values as-is,
 *           covered by 010).
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeScreenToWindow011, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    // Window placed at screen (100, 200): a floating/free window.
    context->SetDisplayWindowRectInfo(Rect(100, 200, 1116, 2398));
    std::string payload = R"([{"elementId":300,"parentId":-1,"componentType":"CustomFrameNode",
        "screenRect":{"x":150.0,"y":263.0,"width":533.0,"height":98.0},
        "attributes":{"text":"screen"},"children":[]}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    auto node300 = FindInspectorNodeById(hostNode.get(), 300);
    ASSERT_NE(node300, nullptr);
    EXPECT_EQ(node300->GetString("$type"), "CustomFrameNode");
    // screen {150, 263} - window origin {100, 200} = window {50, 63}.
    EXPECT_EQ(node300->GetString("$rect"), "[50.00, 63.00],[583.00,161.00]");

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->SetDisplayWindowRectInfo(Rect(0, 0, 0, 0));
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeMultiGetter012
 * @tc.desc: API definition semantics: multiple getter callbacks can be
 *           registered on the same node and are invoked in registration
 *           order (both subtrees appear); Unregister matches by function
 *           pointer (no effect when absent, success), and re-registering
 *           the same callback replaces its userData.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeMultiGetter012, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string payloadOne = R"([{"$type":"MultiOne","$ID":90021}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payloadOne),
        InspectorTreeProviderRegistry::RESULT_OK);
    // A different getter function coexists on the same node.
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadConstant, nullptr),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "MultiOne"));
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "MultiTwo"));
    // Registration order equals splice order: MultiOne precedes MultiTwo.
    EXPECT_GT(FindChildIndexByType(hostNode.get(), "MultiTwo"), FindChildIndexByType(hostNode.get(), "MultiOne"));

    // Unregister by function pointer: only the matching getter is removed.
    ASSERT_EQ(InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), ExternalPayloadConstant),
        InspectorTreeProviderRegistry::RESULT_OK);
    tree = Inspector::GetInspector(true);
    hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "MultiOne"));
    EXPECT_FALSE(NodeChildrenContainType(hostNode.get(), "MultiTwo"));

    // A non-matching pointer has no effect and still returns success.
    ASSERT_EQ(InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), ExternalPayloadNull),
        InspectorTreeProviderRegistry::RESULT_NOT_FOUND);
    tree = Inspector::GetInspector(true);
    hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "MultiOne"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeNativeCustomHost013
 * @tc.desc: API definition: natively created ARKUI_NODE_CUSTOM hosts (FrameNode
 *           with the "Custom" tag, see the node_model.cpp type mapping) are
 *           supported registration targets, same as ARKUI_NODE_XCOMPONENT.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeNativeCustomHost013, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    // Native ARKUI_NODE_CUSTOM host: a FrameNode with the "Custom" tag.
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::CUSTOM_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    std::string payload = R"([{"$type":"NativeCustomRoot","$ID":90023}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_TRUE(NodeChildrenContainType(hostNode.get(), "NativeCustomRoot"));

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorExternalSubtreeScreenToWindowDensity014
 * @tc.desc: GetDisplayWindowRectInfo returns the window origin in physical
 *           px while payload screenRect values and the inspector "$rect" are
 *           in vp: with density 2.0 and the window placed at screen px
 *           (200, 400) (= vp (100, 200)), a screen rect {150, 263, 533, 98}
 *           in vp is spliced as "[50.00, 63.00],[583.00,161.00]" — the px
 *           origin is divided by the pipeline density before subtraction.
 *           Regression for the unit-mismatch defect found by the on-device
 *           floating-window case (without the division the engine subtracted
 *           200/400 from vp coordinates and produced a shifted rect).
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorExternalSubtreeScreenToWindowDensity014, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::XCOMPONENT_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);
    // Window at screen px (200, 400) with density 2.0 -> window origin vp (100, 200).
    MockPipelineContext::GetCurrent()->SetDensity(2.0);
    context->SetDisplayWindowRectInfo(Rect(200, 400, 2232, 3196));
    std::string payload = R"([{"elementId":400,"parentId":-1,"componentType":"CustomFrameNode",
        "screenRect":{"x":150.0,"y":263.0,"width":533.0,"height":98.0},
        "attributes":{"text":"screen"},"children":[]}])";
    ASSERT_EQ(InspectorTreeProviderRegistry::Register(AceType::RawPtr(host), ExternalPayloadFromUserData, &payload),
        InspectorTreeProviderRegistry::RESULT_OK);

    auto tree = Inspector::GetInspector(true);
    auto hostNode = FindNodeInTree(tree, hostId);
    ASSERT_NE(hostNode, nullptr);
    auto node400 = FindInspectorNodeById(hostNode.get(), 400);
    ASSERT_NE(node400, nullptr);
    EXPECT_EQ(node400->GetString("$type"), "CustomFrameNode");
    // screen vp {150, 263} - window vp origin {200/2, 400/2} = window vp {50, 63}.
    EXPECT_EQ(node400->GetString("$rect"), "[50.00, 63.00],[583.00,161.00]");

    InspectorTreeProviderRegistry::Unregister(AceType::RawPtr(host), nullptr);
    // Restore the mock defaults so other tests are unaffected.
    MockPipelineContext::GetCurrent()->SetDensity(1.0);
    context->SetDisplayWindowRectInfo(Rect(0, 0, 0, 0));
    context->stageManager_ = nullptr;
}

/**
 * @tc.name: InspectorFilterWebEnable015
 * @tc.desc: InspectorFilter web flag: a freshly constructed filter has the
 *           web subtree disabled (external filtered-tree entries rely on this
 *           default), SetWebEnable toggles the flag, and the flag stays
 *           independent of the free-nodes flag.
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorFilterWebEnable015, TestSize.Level1)
{
    InspectorFilter filter;
    EXPECT_FALSE(filter.IsWebEnable());

    filter.SetWebEnable(true);
    EXPECT_TRUE(filter.IsWebEnable());

    filter.SetWebEnable(false);
    EXPECT_FALSE(filter.IsWebEnable());

    filter.EnableFreeNodes();
    EXPECT_TRUE(filter.IsFreeNodesEnabled());
    EXPECT_FALSE(filter.IsWebEnable());

    filter.SetWebEnable(true);
    EXPECT_TRUE(filter.IsWebEnable());
    EXPECT_TRUE(filter.IsFreeNodesEnabled());
}

/**
 * @tc.name: InspectorWebSubtreeFlagGate016
 * @tc.desc: Web-flag gate regression on a tree containing a Web-tag node:
 *           GetInspector(bool) enables the web flag only for the layout
 *           inspector (isLayoutInspector == true) while the non-layout call
 *           and the shared 3-arg entry with a default filter (what the
 *           external getFilteredInspectorTree* entries build) keep it
 *           disabled; GetFreeNodesInspector enables it as well. In the
 *           unit-test build the gated web splice is a no-op stub, so both
 *           flag states must be observationally equal and the Web host node
 *           itself is never dropped by the gate. (The real web splice is
 *           exercised by the product build via WEB_SUPPORTED.)
 * @tc.type: FUNC
 */
HWTEST_F(InspectorTestNg, InspectorWebSubtreeFlagGate016, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> root = FrameNode::CreateFrameNode("one", rootId, AceType::MakeRefPtr<Pattern>(), true);
    context->stageManager_ = AceType::MakeRefPtr<StageManager>(root);
    auto pageId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, pageId, AceType::MakeRefPtr<Pattern>());
    root->AddChild(page);
    auto hostId = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> host = FrameNode::CreateFrameNode(V2::WEB_ETS_TAG, hostId, AceType::MakeRefPtr<Pattern>());
    page->AddChild(host);

    // Non-layout GetInspector(bool) keeps the web flag disabled and matches
    // the shared 3-arg entry with a default (web-disabled) filter.
    std::string nonLayoutTree = Inspector::GetInspector(false);
    InspectorFilter externalFilter;
    bool needThrow = false;
    std::string externalTree = Inspector::GetInspector(false, externalFilter, needThrow);
    EXPECT_FALSE(needThrow);
    EXPECT_EQ(nonLayoutTree, externalTree);

    // Same shared entry with the web flag explicitly enabled: identical
    // output (unit-test web splice is a no-op stub).
    InspectorFilter webEnabledFilter;
    webEnabledFilter.SetWebEnable(true);
    std::string webEnabledTree = Inspector::GetInspector(false, webEnabledFilter, needThrow);
    EXPECT_EQ(webEnabledTree, externalTree);

    // Layout entry GetInspector(true) enables the web flag: valid layout
    // tree containing the Web host node, which is never dropped by the gate.
    std::string layoutTree = Inspector::GetInspector(true);
    auto layoutJson = JsonUtil::ParseJsonString(layoutTree);
    ASSERT_NE(layoutJson, nullptr);
    EXPECT_EQ(layoutJson->GetString("type"), "root");
    auto hostNode = FindNodeInTree(layoutTree, hostId);
    ASSERT_NE(hostNode, nullptr);
    EXPECT_EQ(hostNode->GetString("$type"), std::string(V2::WEB_ETS_TAG));

    // The Web host node is present in the non-layout trees as well.
    ASSERT_NE(FindNodeInTree(nonLayoutTree, hostId), nullptr);
    ASSERT_NE(FindNodeInTree(externalTree, hostId), nullptr);

    // Free-nodes entry (web enabled): valid layout tree containing the host.
    std::string freeNodesTree = Inspector::GetFreeNodesInspector();
    auto freeNodesJson = JsonUtil::ParseJsonString(freeNodesTree);
    ASSERT_NE(freeNodesJson, nullptr);
    EXPECT_EQ(freeNodesJson->GetString("type"), "root");
    ASSERT_NE(FindNodeInTree(freeNodesTree, hostId), nullptr);
    context->stageManager_ = nullptr;
}
} // namespace OHOS::Ace::NG

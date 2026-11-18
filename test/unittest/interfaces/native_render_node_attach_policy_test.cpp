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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
// UT access overrides must retain the C++ keyword spelling.
#define private public // NOLINT
#define protected public // NOLINT
// Keep independent test contexts without relying on the process-wide context manager.
#include "render_service_client/core/ui/rs_ui_context.h"
#include "render_service_client/core/ui/rs_node.h"
#include "native_interface.h"
#include "native_node.h"
#include "native_render.h"
#include "native_type.h"
#include "interfaces/native/node/animate_impl.h"
#include "interfaces/native/node/config_manager.h"
#include "interfaces/native/node/node_extened.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/node/render_node.h"
#include "interfaces/native/node/styled_string.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "frameworks/base/error/error_code.h"
#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/base/mixed_mount_registry.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace {
class NativeRenderNodeAttachPolicyTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->pipelineContext_ = NG::MockPipelineContext::GetCurrent();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        PipelineBase::GetCurrentContext()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    void SetUp() override
    {
        nodeApi_ = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
            OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
        ASSERT_NE(nodeApi_, nullptr);
        ASSERT_NE(NodeModel::GetFullImpl(), nullptr);
    }

    void TearDown() override
    {
        for (auto node : renderNodes_) {
            OH_ArkUI_RenderNodeUtils_ClearChildren(node);
        }
        for (auto node : renderNodes_) {
            OH_ArkUI_RenderNodeUtils_DisposeNode(node);
        }
        for (auto node : frameNodes_) {
            nodeApi_->disposeNode(node);
        }
    }

protected:
    struct BranchingTree {
        ArkUI_RenderNodeHandle root = nullptr;
        ArkUI_RenderNodeHandle left = nullptr;
        ArkUI_RenderNodeHandle right = nullptr;
        ArkUI_RenderNodeHandle leftLeaf = nullptr;
        ArkUI_RenderNodeHandle rightLeaf = nullptr;
        ArkUI_RenderNodeHandle deepLeaf = nullptr;

        std::vector<ArkUI_RenderNodeHandle> Nodes() const
        {
            return { root, left, right, leftLeaf, rightLeaf, deepLeaf };
        }
    };

    void CreateBranchingTree(BranchingTree& tree, const std::shared_ptr<Rosen::RSUIContext>& context)
    {
        tree = { CreateRenderNode(context), CreateRenderNode(context), CreateRenderNode(context),
            CreateRenderNode(context), CreateRenderNode(context), CreateRenderNode(context) };
        for (auto node : tree.Nodes()) {
            ASSERT_NE(node, nullptr);
            ASSERT_NE(GetRSNode(node), nullptr);
        }
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(tree.root, tree.left), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(tree.root, tree.right), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(tree.left, tree.leftLeaf), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(tree.right, tree.rightLeaf), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(tree.leftLeaf, tree.deepLeaf), ARKUI_ERROR_CODE_NO_ERROR);
    }

    void ExpectChildren(ArkUI_RenderNodeHandle parent, const std::vector<ArkUI_RenderNodeHandle>& children)
    {
        auto* rsParent = GetRSNode(parent);
        ASSERT_NE(rsParent, nullptr);
        ASSERT_EQ(rsParent->GetChildren().size(), children.size());
        for (size_t index = 0; index < children.size(); ++index) {
            auto* rsChild = GetRSNode(children[index]);
            ASSERT_NE(rsChild, nullptr);
            EXPECT_EQ(rsParent->GetChildByIndex(index).get(), rsChild);
            EXPECT_EQ(rsChild->GetParent().get(), rsParent);
        }
    }

    void ExpectBranchingTree(const BranchingTree& tree, const std::shared_ptr<Rosen::RSUIContext>& context)
    {
        for (auto node : tree.Nodes()) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), context);
        }
        ExpectChildren(tree.root, { tree.left, tree.right });
        ExpectChildren(tree.left, { tree.leftLeaf });
        ExpectChildren(tree.right, { tree.rightLeaf });
        ExpectChildren(tree.leftLeaf, { tree.deepLeaf });
        ExpectChildren(tree.rightLeaf, {});
        ExpectChildren(tree.deepLeaf, {});
    }

    ArkUI_RenderNodeHandle CreateRenderNode(const std::shared_ptr<Rosen::RSUIContext>& context)
    {
        auto node = OH_ArkUI_RenderNodeUtils_CreateNode();
        if (!node) {
            return nullptr;
        }
        renderNodes_.push_back(node);
        auto rsNode = GetRSNode(node);
        if (!rsNode) {
            return nullptr;
        }
        rsNode->SetRSUIContext(context);
        return node;
    }

    ArkUI_NodeHandle CreateFrameNode()
    {
        auto node = nodeApi_->createNode(ARKUI_NODE_CUSTOM);
        if (node) {
            frameNodes_.push_back(node);
        }
        return node;
    }

    Rosen::RSNode* GetRSNode(ArkUI_RenderNodeHandle node)
    {
        const auto* modifier = NodeModel::GetFullImpl()->getNodeModifiers()->getNDKRenderNodeModifier();
        return reinterpret_cast<Rosen::RSNode*>(modifier->getRSNode(node->renderNodeHandle));
    }

    std::shared_ptr<Rosen::RSUIContext> CreateContext(uint64_t token)
    {
        sptr<IRemoteObject> remote;
        return std::make_shared<Rosen::RSUIContext>(token, remote);
    }

    static constexpr auto INHERIT = OH_ARKUI_RENDER_NODE_UTILS_ATTACH_POLICY_INHERIT_PARENT_INSTANCE;
    static constexpr auto KEEP = OH_ARKUI_RENDER_NODE_UTILS_ATTACH_POLICY_NOT_INHERIT_PARENT_INSTANCE;
    ArkUI_NativeNodeAPI_1* nodeApi_ = nullptr;
    std::vector<ArkUI_RenderNodeHandle> renderNodes_;
    std::vector<ArkUI_NodeHandle> frameNodes_;
};

/**
 * @tc.name: SubtreeRemigration
 * @tc.desc: Migrate all descendants A to B to C; removal and clearing retain the last context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, SubtreeRemigration, TestSize.Level1)
{
    auto contextA = CreateContext(1001);
    auto contextB = CreateContext(1002);
    auto contextC = CreateContext(1003);
    auto parentB = CreateRenderNode(contextB);
    auto parentC = CreateRenderNode(contextC);
    auto child = CreateRenderNode(contextA);
    auto grandchild = CreateRenderNode(contextA);
    auto leaf = CreateRenderNode(contextA);
    ASSERT_NE(parentB, nullptr);
    ASSERT_NE(parentC, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_NE(grandchild, nullptr);
    ASSERT_NE(leaf, nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, leaf), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parentB, child, INHERIT), ARKUI_ERROR_CODE_NO_ERROR);
    for (auto node : { child, grandchild, leaf }) {
        EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), contextB);
    }
    EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parentB));
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parentB, child), ARKUI_ERROR_CODE_NO_ERROR);
    for (auto node : { child, grandchild, leaf }) {
        EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), contextB);
    }
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parentC, child, INHERIT), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_ClearChildren(parentC), ARKUI_ERROR_CODE_NO_ERROR);
    for (auto node : { child, grandchild, leaf }) {
        EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), contextC);
    }
    EXPECT_EQ(GetRSNode(child)->GetParent(), nullptr);
    EXPECT_EQ(GetRSNode(grandchild)->GetParent().get(), GetRSNode(child));
    EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(grandchild));
}

/**
 * @tc.name: InsertOrderAndForeignSibling
 * @tc.desc: Insert after a direct sibling and append for a foreign sibling while migrating.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, InsertOrderAndForeignSibling, TestSize.Level1)
{
    auto contextA = CreateContext(1011);
    auto contextB = CreateContext(1012);
    auto parent = CreateRenderNode(contextB);
    auto first = CreateRenderNode(contextB);
    auto tail = CreateRenderNode(contextB);
    auto child = CreateRenderNode(contextA);
    auto leaf = CreateRenderNode(contextA);
    auto foreign = CreateRenderNode(contextB);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(tail, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_NE(leaf, nullptr);
    ASSERT_NE(foreign, nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, leaf), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, first), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, tail), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, first, INHERIT),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(1).get(), GetRSNode(child));
    EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(2).get(), GetRSNode(tail));
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextB);
    EXPECT_EQ(GetRSNode(leaf)->GetRSUIContext(), contextB);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, foreign, INHERIT),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(2).get(), GetRSNode(child));
    EXPECT_EQ(GetRSNode(foreign)->GetParent(), nullptr);
}

/**
 * @tc.name: KeepContextAndLegacyCompatibility
 * @tc.desc: Both NOT_INHERIT entries and legacy entries preserve the child's different context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, KeepContextAndLegacyCompatibility, TestSize.Level1)
{
    auto contextA = CreateContext(1021);
    auto contextB = CreateContext(1022);
    auto parent = CreateRenderNode(contextB);
    auto sibling = CreateRenderNode(contextB);
    auto child = CreateRenderNode(contextA);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(sibling, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, sibling, INHERIT), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, KEEP), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, sibling, KEEP),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfter(parent, child, sibling), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
}

/**
 * @tc.name: InvalidParametersAreAtomic
 * @tc.desc: Null outer/inner handles and invalid policies fail without changing the child.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, InvalidParametersAreAtomic, TestSize.Level1)
{
    auto contextA = CreateContext(1031);
    auto parent = CreateRenderNode(CreateContext(1032));
    auto child = CreateRenderNode(contextA);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    ArkUI_RenderNode invalid {};
    for (auto bad : { static_cast<ArkUI_RenderNodeHandle>(nullptr), &invalid }) {
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(bad, child, INHERIT), ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, bad, INHERIT), ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(bad, child, parent, INHERIT),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, bad, parent, INHERIT),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, bad, INHERIT),
            ERROR_CODE_PARAM_INVALID);
    }
    for (int32_t value : { -1, 2 }) {
        auto policy = static_cast<OH_ArkUI_RenderNodeAttachPolicy>(value);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, policy), ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, parent, policy),
            ERROR_CODE_PARAM_INVALID);
    }
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
    EXPECT_EQ(GetRSNode(child)->GetParent(), nullptr);
    EXPECT_TRUE(GetRSNode(parent)->GetChildren().empty());
}

/**
 * @tc.name: ExistingParentIsAtomic
 * @tc.desc: Reject attached children for both policies; legacy AddChild still allows reparenting.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, ExistingParentIsAtomic, TestSize.Level1)
{
    auto contextA = CreateContext(1041);
    auto parentA = CreateRenderNode(contextA);
    auto parentB = CreateRenderNode(CreateContext(1042));
    auto child = CreateRenderNode(contextA);
    ASSERT_NE(parentA, nullptr);
    ASSERT_NE(parentB, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parentA, child), ARKUI_ERROR_CODE_NO_ERROR);
    for (auto policy : { KEEP, INHERIT }) {
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parentB, child, policy), ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parentB, child, parentA, policy),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parentA));
        EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
        EXPECT_TRUE(GetRSNode(parentB)->GetChildren().empty());
    }
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parentB, child), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parentB));
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
}

/**
 * @tc.name: MixedParentIsAtomic
 * @tc.desc: Reject a child mounted below a mixed FrameNode before migrating it.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, MixedParentIsAtomic, TestSize.Level1)
{
    auto frame = CreateFrameNode();
    auto contextA = CreateContext(1051);
    auto parent = CreateRenderNode(CreateContext(1052));
    auto child = CreateRenderNode(contextA);
    ASSERT_NE(frame, nullptr);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(OH_ArkUI_NativeModule_SetChildMountPolicy(frame, OH_ARKUI_NODE_MOUNT_POLICY_MIXED),
        ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddRenderNode(frame, child), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, parent, INHERIT),
        ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextA);
    EXPECT_TRUE(GetRSNode(parent)->GetChildren().empty());
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_RemoveRenderNode(frame, child), ARKUI_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: FrameNodeSourcesAreValidated
 * @tc.desc: Reject FrameNode targets/native children and stale adopted children with exact error codes.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, FrameNodeSourcesAreValidated, TestSize.Level1)
{
    auto owner = CreateFrameNode();
    auto frame = CreateFrameNode();
    auto parent = CreateRenderNode(CreateContext(1061));
    auto child = CreateRenderNode(CreateContext(1062));
    ASSERT_NE(owner, nullptr);
    ASSERT_NE(frame, nullptr);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(OH_ArkUI_NativeModule_SetChildMountPolicy(owner, OH_ARKUI_NODE_MOUNT_POLICY_MIXED),
        ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(nodeApi_->addChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
    ArkUI_RenderNodeHandle nativeRender = nullptr;
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_GetRenderNodeAt(owner, 0, &nativeRender), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(nativeRender, nullptr);
    renderNodes_.push_back(nativeRender);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(nativeRender, child, INHERIT),
        ERROR_CODE_RENDER_IS_FROM_FRAME_NODE);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(nativeRender, child, parent, INHERIT),
        ERROR_CODE_RENDER_IS_FROM_FRAME_NODE);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, nativeRender, INHERIT),
        ERROR_CODE_RENDER_IS_FROM_FRAME_NODE);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, nativeRender, child, INHERIT),
        ERROR_CODE_RENDER_IS_FROM_FRAME_NODE);
    ASSERT_EQ(nodeApi_->removeChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_NativeModule_AdoptChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
    ArkUI_RenderNodeHandle adoptedRender = nullptr;
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_GetRenderNode(frame, &adoptedRender), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(adoptedRender, nullptr);
    renderNodes_.push_back(adoptedRender);
    ASSERT_EQ(OH_ArkUI_NativeModule_RemoveAdoptedChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, adoptedRender, INHERIT),
        ERROR_CODE_RENDER_HAS_INVALID_FRAME_NODE);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, adoptedRender, child, INHERIT),
        ERROR_CODE_RENDER_HAS_INVALID_FRAME_NODE);
    EXPECT_TRUE(GetRSNode(parent)->GetChildren().empty());
}

/**
 * @tc.name: ValidAdoptedChildMigrates
 * @tc.desc: A valid adopted child can migrate without changing its adoption owner.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, ValidAdoptedChildMigrates, TestSize.Level1)
{
    auto owner = CreateFrameNode();
    auto frame = CreateFrameNode();
    auto contextA = CreateContext(1081);
    auto contextB = CreateContext(1082);
    auto parent = CreateRenderNode(contextB);
    ASSERT_NE(owner, nullptr);
    ASSERT_NE(frame, nullptr);
    ASSERT_NE(parent, nullptr);
    ASSERT_EQ(OH_ArkUI_NativeModule_AdoptChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
    ArkUI_RenderNodeHandle child = nullptr;
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_GetRenderNode(frame, &child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(child, nullptr);
    renderNodes_.push_back(child);
    ASSERT_NE(GetRSNode(child), nullptr);
    GetRSNode(child)->SetRSUIContext(contextA);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextB);
    EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parent));
    auto* childFrame = reinterpret_cast<NG::FrameNode*>(frame->uiNodeHandle);
    EXPECT_TRUE(childFrame->IsAdopted());
    EXPECT_EQ(childFrame->GetAdoptParent().GetRawPtr(), reinterpret_cast<NG::FrameNode*>(owner->uiNodeHandle));
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), contextB);
    EXPECT_EQ(OH_ArkUI_NativeModule_RemoveAdoptedChild(owner, frame), ARKUI_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: CyclicMountIsAtomic
 * @tc.desc: Self-mount and ancestor-under-descendant requests fail before migration.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, CyclicMountIsAtomic, TestSize.Level1)
{
    auto contextA = CreateContext(1071);
    auto contextB = CreateContext(1072);
    auto root = CreateRenderNode(contextA);
    auto descendant = CreateRenderNode(contextB);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(descendant, nullptr);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(root, root, INHERIT), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(root, root, descendant, INHERIT),
        ERROR_CODE_PARAM_INVALID);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(root, descendant), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(descendant, root, INHERIT), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(descendant, root, descendant, INHERIT),
        ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(GetRSNode(root)->GetParent(), nullptr);
    EXPECT_EQ(GetRSNode(descendant)->GetParent().get(), GetRSNode(root));
    EXPECT_EQ(GetRSNode(root)->GetRSUIContext(), contextA);
    EXPECT_EQ(GetRSNode(descendant)->GetRSUIContext(), contextB);
}

/**
 * @tc.name: MissingTargetContextIsAtomic
 * @tc.desc: Reject migration to a null context before mounting; KEEP can still mount the child.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, MissingTargetContextIsAtomic, TestSize.Level1)
{
    auto context = CreateContext(1091);
    auto parent = CreateRenderNode(nullptr);
    auto child = CreateRenderNode(context);
    auto leaf = CreateRenderNode(context);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_NE(leaf, nullptr);
    ASSERT_EQ(GetRSNode(parent)->GetRSUIContext(), nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, leaf), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, INHERIT),
        ERROR_CODE_PARAM_INVALID);
    EXPECT_TRUE(GetRSNode(parent)->GetChildren().empty());
    EXPECT_EQ(GetRSNode(child)->GetParent(), nullptr);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), context);
    EXPECT_EQ(GetRSNode(leaf)->GetRSUIContext(), context);
    EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(child));

    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, KEEP), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parent));
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), context);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, child), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, KEEP),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parent));
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), context);
    EXPECT_EQ(GetRSNode(leaf)->GetRSUIContext(), context);
}

/**
 * @tc.name: BothContextsEmptyCanMount
 * @tc.desc: Equal null contexts do not require migration; both entries preserve mounting order.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, BothContextsEmptyCanMount, TestSize.Level1)
{
    auto parent = CreateRenderNode(nullptr);
    auto first = CreateRenderNode(nullptr);
    auto second = CreateRenderNode(nullptr);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_EQ(GetRSNode(parent)->GetRSUIContext(), nullptr);
    ASSERT_EQ(GetRSNode(first)->GetRSUIContext(), nullptr);
    ASSERT_EQ(GetRSNode(second)->GetRSUIContext(), nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, first, INHERIT), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, second, first, INHERIT),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(0).get(), GetRSNode(first));
    EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(1).get(), GetRSNode(second));
    EXPECT_EQ(GetRSNode(first)->GetRSUIContext(), nullptr);
    EXPECT_EQ(GetRSNode(second)->GetRSUIContext(), nullptr);
}

/**
 * @tc.name: EmptyContextSubtreeMigrates
 * @tc.desc: Both policy entries migrate every level of a subtree without an initial context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, EmptyContextSubtreeMigrates, TestSize.Level1)
{
    auto targetContext = CreateContext(1101);
    for (bool insert : { false, true }) {
        auto parent = CreateRenderNode(targetContext);
        auto sibling = CreateRenderNode(targetContext);
        auto child = CreateRenderNode(nullptr);
        auto grandchild = CreateRenderNode(nullptr);
        auto leaf = CreateRenderNode(nullptr);
        for (auto node : { parent, sibling, child, grandchild, leaf }) {
            ASSERT_NE(node, nullptr);
        }
        for (auto node : { child, grandchild, leaf }) {
            ASSERT_EQ(GetRSNode(node)->GetRSUIContext(), nullptr);
        }
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, sibling), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, leaf), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, sibling, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        for (auto node : { child, grandchild, leaf }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), targetContext);
        }
        EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(0).get(), GetRSNode(sibling));
        EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(1).get(), GetRSNode(child));
        EXPECT_EQ(GetRSNode(grandchild)->GetParent().get(), GetRSNode(child));
        EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(grandchild));
    }
}

/**
 * @tc.name: MatchingContextDoesNotSkipDescendants
 * @tc.desc: Matching root and intermediate contexts do not hide descendants in another context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, MatchingContextDoesNotSkipDescendants, TestSize.Level1)
{
    auto sourceContext = CreateContext(1111);
    auto targetContext = CreateContext(1112);
    for (bool insert : { false, true }) {
        auto parent = CreateRenderNode(targetContext);
        auto child = CreateRenderNode(targetContext);
        auto grandchild = CreateRenderNode(sourceContext);
        auto intermediate = CreateRenderNode(targetContext);
        auto leaf = CreateRenderNode(sourceContext);
        for (auto node : { parent, child, grandchild, intermediate, leaf }) {
            ASSERT_NE(node, nullptr);
        }
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, intermediate), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(intermediate, leaf), ARKUI_ERROR_CODE_NO_ERROR);
        // A valid sibling outside parent's child list must retain append semantics.
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        for (auto node : { child, grandchild, intermediate, leaf }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), targetContext);
        }
        EXPECT_EQ(GetRSNode(parent)->GetChildByIndex(0).get(), GetRSNode(child));
        EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(intermediate));
    }
}

/**
 * @tc.name: EmptyIntermediateContextDoesNotSkipDescendants
 * @tc.desc: A null context in the middle of a subtree must not stop migration or omit a sibling branch.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, EmptyIntermediateContextDoesNotSkipDescendants, TestSize.Level1)
{
    auto sourceContext = CreateContext(1121);
    auto targetContext = CreateContext(1122);
    for (bool insert : { false, true }) {
        auto parent = CreateRenderNode(targetContext);
        auto child = CreateRenderNode(sourceContext);
        auto grandchild = CreateRenderNode(nullptr);
        auto leaf = CreateRenderNode(sourceContext);
        auto branch = CreateRenderNode(nullptr);
        for (auto node : { parent, child, grandchild, leaf, branch }) {
            ASSERT_NE(node, nullptr);
        }
        ASSERT_EQ(GetRSNode(grandchild)->GetRSUIContext(), nullptr);
        ASSERT_EQ(GetRSNode(branch)->GetRSUIContext(), nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, branch), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, leaf), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        for (auto node : { child, grandchild, leaf, branch }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), targetContext);
        }
        EXPECT_EQ(GetRSNode(child)->GetChildByIndex(0).get(), GetRSNode(grandchild));
        EXPECT_EQ(GetRSNode(child)->GetChildByIndex(1).get(), GetRSNode(branch));
        EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(grandchild));
    }
}

/**
 * @tc.name: MissingTargetContextInDescendantIsAtomic
 * @tc.desc: Reject a non-null descendant context under a null root before changing any node.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, MissingTargetContextInDescendantIsAtomic, TestSize.Level1)
{
    auto sourceContext = CreateContext(1131);
    auto parent = CreateRenderNode(nullptr);
    auto child = CreateRenderNode(nullptr);
    auto grandchild = CreateRenderNode(nullptr);
    auto leaf = CreateRenderNode(sourceContext);
    for (auto node : { parent, child, grandchild, leaf }) {
        ASSERT_NE(node, nullptr);
    }
    ASSERT_EQ(GetRSNode(parent)->GetRSUIContext(), nullptr);
    ASSERT_EQ(GetRSNode(child)->GetRSUIContext(), nullptr);
    ASSERT_EQ(GetRSNode(grandchild)->GetRSUIContext(), nullptr);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, leaf), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, INHERIT), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, INHERIT),
        ERROR_CODE_PARAM_INVALID);
    EXPECT_TRUE(GetRSNode(parent)->GetChildren().empty());
    EXPECT_EQ(GetRSNode(child)->GetParent(), nullptr);
    EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), nullptr);
    EXPECT_EQ(GetRSNode(grandchild)->GetRSUIContext(), nullptr);
    EXPECT_EQ(GetRSNode(leaf)->GetRSUIContext(), sourceContext);
    EXPECT_EQ(GetRSNode(grandchild)->GetParent().get(), GetRSNode(child));
    EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(grandchild));
}

/**
 * @tc.name: KeepPreservesMixedSubtreeContexts
 * @tc.desc: NOT_INHERIT preserves a mixed-context subtree, including when the target context is null.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, KeepPreservesMixedSubtreeContexts, TestSize.Level1)
{
    auto sourceContext = CreateContext(1141);
    for (bool insert : { false, true }) {
        auto parent = CreateRenderNode(nullptr);
        auto child = CreateRenderNode(nullptr);
        auto grandchild = CreateRenderNode(sourceContext);
        auto leaf = CreateRenderNode(nullptr);
        for (auto node : { parent, child, grandchild, leaf }) {
            ASSERT_NE(node, nullptr);
        }
        ASSERT_EQ(GetRSNode(parent)->GetRSUIContext(), nullptr);
        ASSERT_EQ(GetRSNode(child)->GetRSUIContext(), nullptr);
        ASSERT_EQ(GetRSNode(leaf)->GetRSUIContext(), nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(child, grandchild), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(grandchild, leaf), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, child, leaf, KEEP)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, child, KEEP);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(GetRSNode(child)->GetRSUIContext(), nullptr);
        EXPECT_EQ(GetRSNode(grandchild)->GetRSUIContext(), sourceContext);
        EXPECT_EQ(GetRSNode(leaf)->GetRSUIContext(), nullptr);
        EXPECT_EQ(GetRSNode(child)->GetParent().get(), GetRSNode(parent));
        EXPECT_EQ(GetRSNode(grandchild)->GetParent().get(), GetRSNode(child));
        EXPECT_EQ(GetRSNode(leaf)->GetParent().get(), GetRSNode(grandchild));
    }
}
/**
 * @tc.name: BranchingTreeMigratesEveryDescendant
 * @tc.desc: Both entries migrate a four-level branching tree without changing sibling order.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, BranchingTreeMigratesEveryDescendant, TestSize.Level1)
{
    auto source = CreateContext(1201);
    auto target = CreateContext(1202);
    for (bool insert : { false, true }) {
        BranchingTree tree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
        GetRSNode(tree.right)->SetRSUIContext(target);
        GetRSNode(tree.rightLeaf)->SetRSUIContext(source);
        auto parent = CreateRenderNode(target);
        auto first = CreateRenderNode(target);
        auto last = CreateRenderNode(target);
        ASSERT_NE(parent, nullptr);
        ASSERT_NE(first, nullptr);
        ASSERT_NE(last, nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, first), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(parent, last), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.root, first, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.root, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectBranchingTree(tree, target);
        ExpectChildren(parent, insert ? std::vector<ArkUI_RenderNodeHandle> { first, tree.root, last }
                                      : std::vector<ArkUI_RenderNodeHandle> { first, last, tree.root });
        EXPECT_EQ(GetRSNode(first)->GetRSUIContext(), target);
        EXPECT_EQ(GetRSNode(last)->GetRSUIContext(), target);
    }
}

/**
 * @tc.name: IndependentTreesRemainIsolatedDuringRemigration
 * @tc.desc: Moving one complete tree again preserves another tree mounted under the same parent.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, IndependentTreesRemainIsolatedDuringRemigration, TestSize.Level1)
{
    auto source = CreateContext(1211);
    auto target = CreateContext(1212);
    auto next = CreateContext(1213);
    for (bool insert : { false, true }) {
        BranchingTree firstTree;
        BranchingTree secondTree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(firstTree, source));
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(secondTree, next));
        auto parent = CreateRenderNode(target);
        auto nextParent = CreateRenderNode(next);
        ASSERT_NE(parent, nullptr);
        ASSERT_NE(nextParent, nullptr);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
                                   parent, firstTree.root, nextParent, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, firstTree.root, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectBranchingTree(firstTree, target);
        ExpectBranchingTree(secondTree, next);
        EXPECT_EQ(GetRSNode(secondTree.root)->GetParent(), nullptr);
        result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
                              parent, secondTree.root, firstTree.root, INHERIT)
                        : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, secondTree.root, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectChildren(parent, { firstTree.root, secondTree.root });
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(parent, firstTree.root), ARKUI_ERROR_CODE_NO_ERROR);
        result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
                              nextParent, firstTree.root, secondTree.root, INHERIT)
                        : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(nextParent, firstTree.root, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectBranchingTree(firstTree, next);
        ExpectBranchingTree(secondTree, target);
        ExpectChildren(parent, { secondTree.root });
        ExpectChildren(nextParent, { firstTree.root });
    }
}

/**
 * @tc.name: DetachedBranchDoesNotMigrateRemainingTree
 * @tc.desc: Migrating an internal branch preserves the original root and its other branch.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, DetachedBranchDoesNotMigrateRemainingTree, TestSize.Level1)
{
    auto source = CreateContext(1221);
    auto target = CreateContext(1222);
    for (bool insert : { false, true }) {
        BranchingTree tree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
        auto parent = CreateRenderNode(target);
        ASSERT_NE(parent, nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveChild(tree.root, tree.left), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
                                   parent, tree.left, tree.right, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.left, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        for (auto node : { tree.left, tree.leftLeaf, tree.deepLeaf }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), target);
        }
        for (auto node : { tree.root, tree.right, tree.rightLeaf }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), source);
        }
        ExpectChildren(parent, { tree.left });
        ExpectChildren(tree.root, { tree.right });
        ExpectChildren(tree.left, { tree.leftLeaf });
        ExpectChildren(tree.leftLeaf, { tree.deepLeaf });
        ExpectChildren(tree.right, { tree.rightLeaf });
    }
}

/**
 * @tc.name: SharedBranchMountIsRejectedWithoutMutation
 * @tc.desc: Neither policy can attach an already-parented branch through a second path.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, SharedBranchMountIsRejectedWithoutMutation, TestSize.Level1)
{
    auto source = CreateContext(1231);
    auto target = CreateContext(1232);
    BranchingTree tree;
    ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
    auto parent = CreateRenderNode(target);
    ASSERT_NE(parent, nullptr);
    for (auto policy : { KEEP, INHERIT }) {
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.left, policy), ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.left, tree.right, policy),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(tree.right, tree.leftLeaf, policy),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
            tree.right, tree.leftLeaf, tree.rightLeaf, policy), ERROR_CODE_PARAM_INVALID);
        ExpectBranchingTree(tree, source);
        ExpectChildren(parent, {});
        EXPECT_EQ(GetRSNode(tree.root)->GetParent(), nullptr);
        EXPECT_EQ(GetRSNode(parent)->GetRSUIContext(), target);
    }
}

/**
 * @tc.name: DeepCycleInBranchingTreeIsRejected
 * @tc.desc: Mounting the root under a deep descendant leaves every branch unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, DeepCycleInBranchingTreeIsRejected, TestSize.Level1)
{
    auto source = CreateContext(1241);
    BranchingTree tree;
    ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
    for (auto policy : { KEEP, INHERIT }) {
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(tree.deepLeaf, tree.root, policy),
            ERROR_CODE_PARAM_INVALID);
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(
            tree.deepLeaf, tree.root, tree.right, policy), ERROR_CODE_PARAM_INVALID);
        ExpectBranchingTree(tree, source);
        EXPECT_EQ(GetRSNode(tree.root)->GetParent(), nullptr);
    }
}

/**
 * @tc.name: KeepPolicyPreservesBranchingTreeContext
 * @tc.desc: Both NOT_INHERIT entries preserve the entire branching tree's original context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, KeepPolicyPreservesBranchingTreeContext, TestSize.Level1)
{
    auto source = CreateContext(1251);
    auto target = CreateContext(1252);
    for (bool insert : { false, true }) {
        BranchingTree tree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
        auto parent = CreateRenderNode(target);
        ASSERT_NE(parent, nullptr);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.root, tree.right, KEEP)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.root, KEEP);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectBranchingTree(tree, source);
        ExpectChildren(parent, { tree.root });
        EXPECT_EQ(GetRSNode(parent)->GetRSUIContext(), target);
    }
}

/**
 * @tc.name: MixedHostMustReleaseWholeTreeBeforeMigration
 * @tc.desc: A mixed host protects its subtree until removal, then both entries migrate all descendants.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, MixedHostMustReleaseWholeTreeBeforeMigration, TestSize.Level1)
{
    auto source = CreateContext(1261);
    auto target = CreateContext(1262);
    for (bool insert : { false, true }) {
        BranchingTree tree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, source));
        auto frame = CreateFrameNode();
        auto parent = CreateRenderNode(target);
        ASSERT_NE(frame, nullptr);
        ASSERT_NE(parent, nullptr);
        ASSERT_EQ(OH_ArkUI_NativeModule_SetChildMountPolicy(frame, OH_ARKUI_NODE_MOUNT_POLICY_MIXED),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddRenderNode(frame, tree.root), ARKUI_ERROR_CODE_NO_ERROR);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.root, parent, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.root, INHERIT);
        EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);
        ExpectBranchingTree(tree, source);
        ExpectChildren(parent, {});
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_RemoveRenderNode(frame, tree.root), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(NG::MixedMountRegistry::GetParent(GetRSNode(tree.root)->GetId()), nullptr);
        result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.root, parent, INHERIT)
                        : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.root, INHERIT);
        ASSERT_EQ(result, ARKUI_ERROR_CODE_NO_ERROR);
        ExpectBranchingTree(tree, target);
        ExpectChildren(parent, { tree.root });
    }
}
/**
 * @tc.name: BranchingPreflightFailureDoesNotPartiallyMigrate
 * @tc.desc: A nonempty context at the deepest leaf rejects a null target before changing any branch.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAttachPolicyTest, BranchingPreflightFailureDoesNotPartiallyMigrate, TestSize.Level1)
{
    auto source = CreateContext(1271);
    for (bool insert : { false, true }) {
        BranchingTree tree;
        ASSERT_NO_FATAL_FAILURE(CreateBranchingTree(tree, nullptr));
        auto parent = CreateRenderNode(nullptr);
        ASSERT_NE(parent, nullptr);
        for (auto node : tree.Nodes()) {
            ASSERT_EQ(GetRSNode(node)->GetRSUIContext(), nullptr);
        }
        ASSERT_EQ(GetRSNode(parent)->GetRSUIContext(), nullptr);
        GetRSNode(tree.deepLeaf)->SetRSUIContext(source);
        auto result = insert ? OH_ArkUI_RenderNodeUtils_InsertChildAfterWithPolicy(parent, tree.root, parent, INHERIT)
                             : OH_ArkUI_RenderNodeUtils_AddChildWithPolicy(parent, tree.root, INHERIT);
        EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);
        for (auto node : { tree.root, tree.left, tree.right, tree.leftLeaf, tree.rightLeaf }) {
            EXPECT_EQ(GetRSNode(node)->GetRSUIContext(), nullptr);
        }
        EXPECT_EQ(GetRSNode(tree.deepLeaf)->GetRSUIContext(), source);
        EXPECT_EQ(GetRSNode(tree.root)->GetParent(), nullptr);
        ExpectChildren(tree.root, { tree.left, tree.right });
        ExpectChildren(tree.left, { tree.leftLeaf });
        ExpectChildren(tree.right, { tree.rightLeaf });
        ExpectChildren(tree.leftLeaf, { tree.deepLeaf });
        ExpectChildren(tree.deepLeaf, {});
        ExpectChildren(parent, {});
    }
}
} // namespace OHOS::Ace

/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#define private public
#define protected public

#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "accessibility_system_ability_client.h"
#include "core/components_ng/base/frame_node.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "core/components_ng/base/cmp_accessibility_registry.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
namespace {
// Explicit using-declarations: this suite must not depend on transitive NG name
// injection from other headers' include order (the legacy OHOS::Ace::FrameNode
// forward declaration otherwise shadows NG::FrameNode for unqualified lookups).
using NG::FrameNode;
using NG::Pattern;

// Minimal callback stand-in for migration tests: the suite only exercises the
// table-moving logic of MigrateChildTreeCallbacksToSubAM, not callback behavior.
// (CmpChildTreeCallback was removed — dead code after the architecture rework
// moved the exported subtree's own registration to the binding state machine.)
class FakeChildTreeCallback : public AccessibilityChildTreeCallback {
public:
    explicit FakeChildTreeCallback(int64_t accessibilityId)
        : AccessibilityChildTreeCallback(accessibilityId) {}
    ~FakeChildTreeCallback() override = default;

    bool OnRegister(uint32_t windowId, int32_t treeId) override
    {
        return true;
    }
    bool OnDeregister() override
    {
        return true;
    }
    bool OnSetChildTree(int32_t childWindowId, int32_t childTreeId) override
    {
        return true;
    }
    bool OnDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {
        return true;
    }
    void OnClearRegisterFlag() override {}
};

/*
 * Test suite for CMP mixed-rendering binding management and state machine.
 * Architecture (2026-09-15 revision, Option A):
 *   - subAM created at Bind entry (creation decoupled from registration);
 *   - registration waits for the CMP treeId convergence (WAIT_TREE_ID -> REGISTERING);
 *   - T_sub assigned asynchronously via CmpInteractionOperation::SetBelongTreeId
 *     (REGISTERING -> REGISTERED);
 *   - composite binding key (owner nodeId, bindId); one exportedRoot one active binding;
 *   - Unbind = whole-subtree detach; adoption state is irrelevant to the binding lifecycle.
 */
class JsAccessibilityManagerCmpTest : public testing::Test {
public:
    // NOTE: MockPipelineContext must stay fully qualified here — an unqualified reference
    // inside this namespace resolves through a different using-chain and trips latent
    // 32-bit -Wconstant-conversion diagnostics in unrelated animation headers.
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_ = NG::MockPipelineContext::GetCurrentContext();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    }
    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    void SetUp() {}
    void TearDown() {}
};

RefPtr<FrameNode> CreateAdoptedPair(int32_t ownerId, int32_t rootId)
{
    auto owner = FrameNode::CreateFrameNode("test_owner", ownerId, AceType::MakeRefPtr<Pattern>());
    auto exportedRoot = FrameNode::CreateFrameNode("test_exported_root", rootId,
        AceType::MakeRefPtr<Pattern>());
    // Physical tree attachment only (enables the parent-chain subAM walk-up path in tests);
    // the binding lifecycle is independent of how the CMP framework places the subtree.
    owner->AddChild(exportedRoot);
    exportedRoot->SetAdoptParent(owner);
    return owner;
}

/**
 * @tc.name: CmpBindingStateEnum001
 *  @tc.desc: Test CmpBindingState State enum values and empty binding map.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, CmpBindingStateEnum001, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->cmpBindingMap_.size(), 0u);
}

/**
 * @tc.name: BindExportTreeBasic002
 * @tc.desc: Test Bind creates the subAM at entry and a WAIT_TREE_ID binding (Option A).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, BindExportTreeBasic002, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);

    int64_t bindId = 300;
    // No third-provider treeId for this host -> WAIT branch.
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), bindId, exportedRoot.GetRawPtr()));

    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, bindId));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    // subAM is created by Bind itself (binding lifecycle).
    EXPECT_NE(it->second.subAM, nullptr);
    EXPECT_NE(it->second.subAM.GetRawPtr(), mainAM.GetRawPtr());
}

/**
 * @tc.name: BindExportTreeWithoutAdoption003
 * @tc.desc: Test Bind accepts an exportedRoot without adoption relation (adoption not required).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, BindExportTreeWithoutAdoption003, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = FrameNode::CreateFrameNode("test_owner", 1, AceType::MakeRefPtr<Pattern>());
    // Not attached: no adoption — still a valid Bind target.
    auto orphan = FrameNode::CreateFrameNode("orphan_root", 2, AceType::MakeRefPtr<Pattern>());
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 303, orphan.GetRawPtr()));
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, 303));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
}

/**
 * @tc.name: BindExportTreeIdempotent004
 * @tc.desc: Test re-bind on an active binding is idempotent (INV-4).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, BindExportTreeIdempotent004, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto key = std::make_pair(1, static_cast<int64_t>(301));

    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 301, exportedRoot.GetRawPtr()));
    auto subAMFirst = mainAM->cmpBindingMap_[key].subAM;
    // Manually advance state to test the idempotent guard for active bindings.
    mainAM->cmpBindingMap_[key].state = JsAccessibilityManager::CmpBindingState::State::REGISTERED;
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 301, exportedRoot.GetRawPtr()));
    EXPECT_EQ(mainAM->cmpBindingMap_[key].state,
        JsAccessibilityManager::CmpBindingState::State::REGISTERED);
    // Same subAM preserved (no re-creation on idempotent re-bind).
    EXPECT_EQ(mainAM->cmpBindingMap_[key].subAM, subAMFirst);
}

/**
 * @tc.name: MultiBindIsolation005
 *  @tc.desc: Test multi-binding isolation: two bindIds, unbind one, other unaffected.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, MultiBindIsolation005, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner1 = CreateAdoptedPair(1, 2);
    auto exportedRoot1 = AceType::DynamicCast<FrameNode>(owner1->GetChildAtIndex(0));
    auto owner2 = CreateAdoptedPair(3, 4);
    auto exportedRoot2 = AceType::DynamicCast<FrameNode>(owner2->GetChildAtIndex(0));

    int64_t bindId1 = 310;
    int64_t bindId2 = 311;
    EXPECT_TRUE(mainAM->BindExportTree(owner1.GetRawPtr(), bindId1, exportedRoot1.GetRawPtr()));
    EXPECT_TRUE(mainAM->BindExportTree(owner2.GetRawPtr(), bindId2, exportedRoot2.GetRawPtr()));
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 2u);

    mainAM->UnbindExportTree(owner1.GetRawPtr(), bindId1);
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 1u);
    EXPECT_EQ(mainAM->cmpBindingMap_.find(std::make_pair(1, bindId1)),
        mainAM->cmpBindingMap_.end());
    auto it2 = mainAM->cmpBindingMap_.find(std::make_pair(3, bindId2));
    ASSERT_NE(it2, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it2->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    // Composite key isolation: same bindId on another owner is a distinct binding.
    EXPECT_EQ(mainAM->cmpBindingMap_.count(std::make_pair(3, bindId1)), 0u);
}

/**
 * @tc.name: SameRootSecondBindRejected006
 * @tc.desc: Test one exportedRoot supports at most one active binding (registry single key).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, SameRootSecondBindRejected006, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner1 = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner1->GetChildAtIndex(0));
    auto owner2 = FrameNode::CreateFrameNode("test_owner2", 3, AceType::MakeRefPtr<Pattern>());
    owner2->AddChild(exportedRoot);
    exportedRoot->SetAdoptParent(owner2);

    EXPECT_TRUE(mainAM->BindExportTree(owner1.GetRawPtr(), 320, exportedRoot.GetRawPtr()));
    // Same exportedRoot bound again (another owner/bindId) — rejected.
    EXPECT_FALSE(mainAM->BindExportTree(owner2.GetRawPtr(), 321, exportedRoot.GetRawPtr()));
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 1u);
}

/**
 * @tc.name: UnbindUnknownIdempotent007
 *  @tc.desc: Test Unbind of unknown bindId is an idempotent no-op.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, UnbindUnknownIdempotent007, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = FrameNode::CreateFrameNode("test_owner", 1, AceType::MakeRefPtr<Pattern>());
    mainAM->UnbindExportTree(owner.GetRawPtr(), 404);
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 0u);
}

/**
 * @tc.name: OnCmpTreeRegisteredConvergence008
 *  @tc.desc: Test convergence moves WAIT_TREE_ID -> REGISTERING for matching owner.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, OnCmpTreeRegisteredConvergence008, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 330, exportedRoot.GetRawPtr()));
    // No third-provider operator for this host -> GetThirdProviderTreeIdForHost returns
    // UNDEFINED, binding stays WAIT_TREE_ID after Bind. Converge explicitly:
    mainAM->OnCmpTreeRegistered(owner.GetRawPtr(), 777);
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(330)));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::REGISTERING);
    EXPECT_EQ(it->second.cmpTreeId, 777);
}

/**
 * @tc.name: OnCmpTreeRegisteredIgnoresOthers009
 * @tc.desc: Test convergence only touches WAIT bindings of the matching owner (INV-4).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, OnCmpTreeRegisteredIgnoresOthers009, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner1 = CreateAdoptedPair(1, 2);
    auto root1 = AceType::DynamicCast<FrameNode>(owner1->GetChildAtIndex(0));
    auto owner2 = CreateAdoptedPair(3, 4);
    auto root2 = AceType::DynamicCast<FrameNode>(owner2->GetChildAtIndex(0));
    EXPECT_TRUE(mainAM->BindExportTree(owner1.GetRawPtr(), 331, root1.GetRawPtr()));
    EXPECT_TRUE(mainAM->BindExportTree(owner2.GetRawPtr(), 332, root2.GetRawPtr()));

    mainAM->OnCmpTreeRegistered(owner1.GetRawPtr(), 888);
    auto it1 = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(331)));
    auto it2 = mainAM->cmpBindingMap_.find(std::make_pair(3, static_cast<int64_t>(332)));
    ASSERT_NE(it1, mainAM->cmpBindingMap_.end());
    ASSERT_NE(it2, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it1->second.state, JsAccessibilityManager::CmpBindingState::State::REGISTERING);
    // Non-matching owner untouched.
    EXPECT_EQ(it2->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
}

/**
 * @tc.name: MarkCmpBindingRegistered010
 * @tc.desc: Test T_sub arrival transition REGISTERING -> REGISTERED (SetBelongTreeId path).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, MarkCmpBindingRegistered010, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 340, exportedRoot.GetRawPtr()));
    mainAM->OnCmpTreeRegistered(owner.GetRawPtr(), 999);

    mainAM->MarkCmpBindingRegistered(1, 340);
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(340)));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::REGISTERED);
}

/**
 * @tc.name: UnregisterSubAMKeepsBinding011
 *  @tc.desc: Adopt decoupling : UnregisterSubAM drops only the routing
 *           index — the binding survives until Unbind (binding teardown is Unbind-driven).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, UnregisterSubAMKeepsBinding011, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 350, exportedRoot.GetRawPtr()));
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 1u);

    // Registry-index removal alone must not touch the binding (adopt decoupling).
    CmpAccessibility::CmpAccessibilityRegistry::UnregisterSubAM(exportedRoot.GetRawPtr());
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 1u);
    // Binding teardown is Unbind-driven only.
    mainAM->UnbindExportTree(owner.GetRawPtr(), 350);
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 0u);
}

/**
 * @tc.name: MigrateChildTreeCallbacksToSubAM012
 * @tc.desc: Test migration moves main-AM callbacks of in-subtree nodes to the subAM table.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, MigrateChildTreeCallbacksToSubAM012, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto inSubtree = FrameNode::CreateFrameNode("web_like", 5, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(inSubtree);
    auto outside = FrameNode::CreateFrameNode("outside", 6, AceType::MakeRefPtr<Pattern>());

    auto callback = std::make_shared<FakeChildTreeCallback>(inSubtree->GetAccessibilityId());
    auto callbackOutside = std::make_shared<FakeChildTreeCallback>(outside->GetAccessibilityId());
    mainAM->RegisterAccessibilityChildTreeCallback(inSubtree->GetAccessibilityId(), callback);
    mainAM->RegisterAccessibilityChildTreeCallback(outside->GetAccessibilityId(), callbackOutside);

    mainAM->MigrateChildTreeCallbacksToSubAM(exportedRoot, subAM);

    // In-subtree callback moved to the subAM, out-of-subtree stays on the main AM.
    EXPECT_EQ(mainAM->childTreeCallbackMap_.count(inSubtree->GetAccessibilityId()), 0u);
    EXPECT_EQ(subAM->childTreeCallbackMap_.count(inSubtree->GetAccessibilityId()), 1u);
    EXPECT_EQ(mainAM->childTreeCallbackMap_.count(outside->GetAccessibilityId()), 1u);
    EXPECT_EQ(subAM->childTreeCallbackMap_.count(outside->GetAccessibilityId()), 0u);
}

/**
 * @tc.name: GetAccessibilityManagerForNode013
 * @tc.desc: Test node-aware AM routing: in-subtree -> subAM, outside -> main AM (base default).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, GetAccessibilityManagerForNode013, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto inSubtree = FrameNode::CreateFrameNode("inner", 5, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(inSubtree);
    auto outside = FrameNode::CreateFrameNode("outer", 6, AceType::MakeRefPtr<Pattern>());

    // Before Bind: routing falls back to the main AM for every node.
    auto before = mainAM->GetAccessibilityManagerForNode(inSubtree);
    EXPECT_EQ(before.GetRawPtr(), mainAM.GetRawPtr());

    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 360, exportedRoot.GetRawPtr()));
    // After Bind: in-subtree nodes route to the binding's subAM.
    auto routed = mainAM->GetAccessibilityManagerForNode(inSubtree);
    EXPECT_NE(routed.GetRawPtr(), mainAM.GetRawPtr());
    auto key = std::make_pair(1, static_cast<int64_t>(360));
    EXPECT_EQ(routed.GetRawPtr(), mainAM->cmpBindingMap_[key].subAM.GetRawPtr());
    // Outside nodes still route to the main AM.
    auto routedOutside = mainAM->GetAccessibilityManagerForNode(outside);
    EXPECT_EQ(routedOutside.GetRawPtr(), mainAM.GetRawPtr());
    // The owner (host) node itself is NOT inside the exported subtree — the walk-up goes to
    // its main-tree ancestors — so it must route to the main AM. This is load-bearing: the
    // CMP tree's third-provider registration and GetThirdProviderTreeIdForHost both operate
    // on the main AM's tables.
    auto routedOwner = mainAM->GetAccessibilityManagerForNode(owner);
    EXPECT_EQ(routedOwner.GetRawPtr(), mainAM.GetRawPtr());
    // Null node falls back to the main AM.
    auto routedNull = mainAM->GetAccessibilityManagerForNode(nullptr);
    EXPECT_EQ(routedNull.GetRawPtr(), mainAM.GetRawPtr());
}

/**
 * @tc.name: UnbindReleasesSubAMRegistry014
 * @tc.desc: Test Unbind drops the node-registry entry (subAM no longer reachable by walk-up).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, UnbindReleasesSubAMRegistry014, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto inSubtree = FrameNode::CreateFrameNode("inner", 5, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(inSubtree);

    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 370, exportedRoot.GetRawPtr()));
    EXPECT_NE(mainAM->GetAccessibilityManagerForNode(inSubtree).GetRawPtr(), mainAM.GetRawPtr());

    mainAM->UnbindExportTree(owner.GetRawPtr(), 370);
    EXPECT_EQ(mainAM->cmpBindingMap_.size(), 0u);
    // After Unbind the subtree is fully detached: routing falls back to the main AM.
    EXPECT_EQ(mainAM->GetAccessibilityManagerForNode(inSubtree).GetRawPtr(), mainAM.GetRawPtr());
}

/**
 * @tc.name: SearchFromRootWholeTree015
 * @tc.desc: SearchElementInfoByAccessibilityIdFromRoot returns the whole subtree rooted at
 *           elementId (-1 = whole exported tree) and the exported root carries the invalid
 *           parent marker instead of a host-tree parent id.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, SearchFromRootWholeTree015, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    // exportedRoot -> (child1 -> grandChild, child2)
    auto exportedRoot = FrameNode::CreateFrameNode("exported_root",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto child1 = FrameNode::CreateFrameNode("cmp_child1",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto child2 = FrameNode::CreateFrameNode("cmp_child2",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto grandChild = FrameNode::CreateFrameNode("cmp_grand",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(child1);
    exportedRoot->AddChild(child2);
    child1->AddChild(grandChild);
    for (const auto& node : { exportedRoot, child1, child2, grandChild }) {
        node->context_ = AceType::RawPtr(context);
        node->isActive_ = true;
    }

    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    subAM->SetPipelineContext(context);
    subAM->Register(true);

    // elementId -1: whole exported tree, root first, root parent = INVALID_PARENT_ID.
    std::list<AccessibilityElementInfo> infos;
    subAM->SearchElementInfoByAccessibilityIdFromRoot(-1,
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN, infos, context, exportedRoot);
    EXPECT_EQ(infos.size(), 4u);
    ASSERT_FALSE(infos.empty());
    EXPECT_EQ(infos.front().GetAccessibilityId(), exportedRoot->GetAccessibilityId());
    EXPECT_EQ(infos.front().GetParentNodeId(), JsAccessibilityManager::INVALID_PARENT_ID);

    // Mid-node query: subtree rooted at child1 only; parent fix applies only to the root.
    infos.clear();
    subAM->SearchElementInfoByAccessibilityIdFromRoot(child1->GetAccessibilityId(),
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN, infos, context, exportedRoot);
    EXPECT_EQ(infos.size(), 2u);
    ASSERT_FALSE(infos.empty());
    EXPECT_EQ(infos.front().GetAccessibilityId(), child1->GetAccessibilityId());
    EXPECT_EQ(infos.front().GetParentNodeId(), exportedRoot->GetAccessibilityId());

    // Non-recursive mode: only the start node itself.
    infos.clear();
    subAM->SearchElementInfoByAccessibilityIdFromRoot(exportedRoot->GetAccessibilityId(),
        0, infos, context, exportedRoot);
    EXPECT_EQ(infos.size(), 1u);
    ASSERT_FALSE(infos.empty());
    EXPECT_EQ(infos.front().GetParentNodeId(), JsAccessibilityManager::INVALID_PARENT_ID);
}

namespace {
// Minimal AAMS-side callback stand-in: records the search-by-id result list.
class MockAmsElementOperatorCallback : public Accessibility::AccessibilityElementOperatorCallback {
public:
    ~MockAmsElementOperatorCallback() override = default;

    void SetSearchElementInfoByAccessibilityIdResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos, const int32_t requestId) override
    {
        mockInfos_ = infos;
        mockRequestId = requestId;
    }
    void SetSearchDefaultFocusByWindowIdResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos, const int32_t requestId) override {}
    void SetSearchElementInfoByTextResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos, const int32_t requestId) override {}
    void SetFindFocusedElementInfoResult(
        const Accessibility::AccessibilityElementInfo& info, const int32_t requestId) override {}
    void SetFocusMoveSearchResult(
        const Accessibility::AccessibilityElementInfo& info, const int32_t requestId) override
    {
        mockLegacyFocusMoveCalled_ = true;
    }
    void SetExecuteActionResult(const bool succeeded, const int32_t requestId) override {}
    void SetCursorPositionResult(const int32_t cursorPosition, const int32_t requestId) override {}
    void SetSearchElementInfoBySpecificPropertyResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const std::list<Accessibility::AccessibilityElementInfo>& treeInfos,
        const int32_t requestId) override {}
    void SetFocusMoveSearchWithConditionResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const Accessibility::FocusMoveResult& result, const int32_t requestId) override
    {
        mockFocusMoveWithConditionCalled_ = true;
        mockFocusMoveInfos_ = infos;
        mockFocusMoveResult_ = result;
        mockRequestId = requestId;
    }
    void SetUpdateCustomAccessibilityPropertyResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}
    void SetAddAccessibilityVirtualNodeResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}
    void SetRemoveAccessibilityVirtualNodeResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}

    std::list<Accessibility::AccessibilityElementInfo> mockInfos_;
    int32_t mockRequestId = 0;
    bool mockLegacyFocusMoveCalled_ = false;
    bool mockFocusMoveWithConditionCalled_ = false;
    std::list<Accessibility::AccessibilityElementInfo> mockFocusMoveInfos_;
    Accessibility::FocusMoveResult mockFocusMoveResult_ {};
};
} // namespace

/**
 * @tc.name: CmpSearchByIdSplicesTreeId016
 * @tc.desc: CmpInteractionOperation::SearchElementInfoByAccessibilityId splices the exported
 *           tree's treeId into outbound element/parent/child ids so AAMS can route follow-up
 *           queries back to the operator, and still resolves a spliced inbound id.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, CmpSearchByIdSplicesTreeId016, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    // exportedRoot -> (child1 -> grandChild, child2)
    auto exportedRoot = FrameNode::CreateFrameNode("exported_root",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto child1 = FrameNode::CreateFrameNode("cmp_child1",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto child2 = FrameNode::CreateFrameNode("cmp_child2",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto grandChild = FrameNode::CreateFrameNode("cmp_grand",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(child1);
    exportedRoot->AddChild(child2);
    child1->AddChild(grandChild);
    for (const auto& node : { exportedRoot, child1, child2, grandChild }) {
        node->context_ = AceType::RawPtr(context);
        node->isActive_ = true;
    }

    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    subAM->SetPipelineContext(context);
    subAM->Register(true);
    // T_sub, normally assigned via CmpInteractionOperation::SetBelongTreeId.
    constexpr int32_t testTreeId = 5;
    subAM->treeId_ = testTreeId;
    // Window-match precondition for belongTreeId stamping (normally set in
    // RegisterCmpSubtreeToAams): take it from a probe info of the same pipeline.
    std::list<AccessibilityElementInfo> probe;
    subAM->SearchElementInfoByAccessibilityIdFromRoot(-1,
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN, probe, context, exportedRoot);
    ASSERT_FALSE(probe.empty());
    subAM->SetWindowId(static_cast<uint32_t>(probe.front().GetWindowId()));

    auto operation = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
        1, 100, exportedRoot, 1, WeakPtr<JsAccessibilityManager>());
    operation->SetHandler(Referenced::WeakClaim(subAM.GetRawPtr()));

    // 1. Root query (-1): whole tree, ids spliced with T_sub, root parent stays invalid.
    MockAmsElementOperatorCallback callback;
    operation->SearchElementInfoByAccessibilityId(-1, 2, callback,
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN);
    EXPECT_EQ(callback.mockInfos_.size(), 4u);
    ASSERT_FALSE(callback.mockInfos_.empty());
    int64_t splicedRootId = exportedRoot->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(testTreeId, splicedRootId);
    EXPECT_EQ(callback.mockInfos_.front().GetAccessibilityId(), splicedRootId);
    EXPECT_EQ(callback.mockInfos_.front().GetParentNodeId(), JsAccessibilityManager::INVALID_PARENT_ID);
    for (const auto& info : callback.mockInfos_) {
        EXPECT_EQ(info.GetBelongTreeId(), testTreeId);
    }

    // 2. Spliced inbound id (echoed by AAMS): resolves to child1 and returns its subtree.
    int64_t splicedChildId = child1->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(testTreeId, splicedChildId);
    MockAmsElementOperatorCallback childCallback;
    operation->SearchElementInfoByAccessibilityId(splicedChildId, 3, childCallback,
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN);
    EXPECT_EQ(childCallback.mockInfos_.size(), 2u);
    ASSERT_FALSE(childCallback.mockInfos_.empty());
    EXPECT_EQ(childCallback.mockInfos_.front().GetAccessibilityId(), splicedChildId);
    // In-subtree parent of child1 is the exported root — also spliced.
    EXPECT_EQ(childCallback.mockInfos_.front().GetParentNodeId(), splicedRootId);
}

/**
 * @tc.name: CmpFocusMoveSearchWithConditionEnvelope017
 * @tc.desc: CmpInteractionOperation::FocusMoveSearchWithCondition on subtree exhaustion
 *           (no focusable next node inside the exported tree) returns the cross-tree
 *           continuation envelope: SEARCH_FAIL_IN_CHILDTREE + the subtree-root ChangeToRoot
 *           anchor + nowLevelBelongTreeId=T_sub + needTerminate=false, mirroring the
 *           third-provider operator's child-tree exhaustion form (FIND_FAIL_IN_CHILDTREE ->
 *           SEARCH_FAIL_IN_CHILDTREE + ChangeToRoot, osal.cpp:70-73 — the exported subtree
 *           is itself a child tree; the AA outer iteration continues in the parent CMP tree).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, CmpFocusMoveSearchWithConditionEnvelope017, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto exportedRoot = FrameNode::CreateFrameNode("exported_root",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto child1 = FrameNode::CreateFrameNode("cmp_child1",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(child1);
    for (const auto& node : { exportedRoot, child1 }) {
        node->context_ = AceType::RawPtr(context);
        node->isActive_ = true;
    }

    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    subAM->SetPipelineContext(context);
    subAM->Register(true);
    constexpr int32_t testTreeId = 5; // T_sub
    subAM->treeId_ = testTreeId;
    // Window-match precondition for belongTreeId stamping (RegisterCmpSubtreeToAams).
    std::list<AccessibilityElementInfo> probe;
    subAM->SearchElementInfoByAccessibilityIdFromRoot(-1,
        OHOS::Accessibility::PREFETCH_RECURSIVE_CHILDREN, probe, context, exportedRoot);
    ASSERT_FALSE(probe.empty());
    subAM->SetWindowId(static_cast<uint32_t>(probe.front().GetWindowId()));

    auto operation = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
        1, 100, exportedRoot, 1, WeakPtr<JsAccessibilityManager>());
    operation->SetHandler(Referenced::WeakClaim(subAM.GetRawPtr()));

    // No geometry in the mock env → FilterAccessibilityElementByFocusable prunes all nodes
    // (zero rects) → no next focus in the subtree = traversal completed at the boundary.
    Accessibility::AccessibilityElementInfo info;
    Accessibility::AccessibilityFocusMoveParam param;
    param.direction = Accessibility::FocusMoveDirection::FORWARD;
    MockAmsElementOperatorCallback callback;
    operation->FocusMoveSearchWithCondition(info, param, 8, callback);

    EXPECT_TRUE(callback.mockFocusMoveWithConditionCalled_);
    EXPECT_FALSE(callback.mockLegacyFocusMoveCalled_);
    EXPECT_EQ(static_cast<int32_t>(callback.mockFocusMoveResult_.resultType),
        static_cast<int32_t>(Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE));
    // Envelope marks the completing level (T_sub) and keeps the outer iteration going.
    EXPECT_EQ(callback.mockFocusMoveResult_.nowLevelBelongTreeId, testTreeId);
    EXPECT_FALSE(callback.mockFocusMoveResult_.needTerminate);
    EXPECT_EQ(callback.mockRequestId, 8);
    ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
}

/**
 * @tc.name: CmpFocusMoveSearchWithConditionFallback018
 * @tc.desc: CmpInteractionOperation::FocusMoveSearchWithCondition with a dead binding
 *           (subAM/exportedRoot gone) returns the F-05 host anchor through the
 *           with-condition callback (SetFocusMoveSearchWithConditionResult), not the
 *           without-condition SetFocusMoveSearchResult: envelope SEARCH_FAIL +
 *           nowLevelBelongTreeId=T_cmp + needTerminate=false, host info carries
 *           belongTreeId=T_cmp and the bindId spliced with T_cmp high bits.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, CmpFocusMoveSearchWithConditionFallback018, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto exportedRoot = FrameNode::CreateFrameNode("exported_root",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    exportedRoot->context_ = AceType::RawPtr(context);
    exportedRoot->isActive_ = true;

    // No SetHandler: subAM upgrade fails inside the operator → early-exit fallback path.
    constexpr int32_t cmpTreeId = 1;
    constexpr int64_t bindId = 100;
    auto operation = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
        cmpTreeId, bindId, exportedRoot, 1, WeakPtr<JsAccessibilityManager>());

    Accessibility::AccessibilityElementInfo info;
    Accessibility::AccessibilityFocusMoveParam param;
    param.direction = Accessibility::FocusMoveDirection::FORWARD;
    MockAmsElementOperatorCallback callback;
    operation->FocusMoveSearchWithCondition(info, param, 9, callback);

    // Answered on the with-condition channel — the wrong-channel reply is the regression
    // this guards against (the AA side would otherwise drop the fallback info).
    EXPECT_TRUE(callback.mockFocusMoveWithConditionCalled_);
    EXPECT_FALSE(callback.mockLegacyFocusMoveCalled_);
    EXPECT_EQ(static_cast<int32_t>(callback.mockFocusMoveResult_.resultType),
        static_cast<int32_t>(Accessibility::FocusMoveResultType::SEARCH_FAIL));
    EXPECT_EQ(callback.mockFocusMoveResult_.nowLevelBelongTreeId, cmpTreeId);
    EXPECT_FALSE(callback.mockFocusMoveResult_.needTerminate);
    EXPECT_EQ(callback.mockRequestId, 9);
    ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
    const auto& hostInfo = callback.mockFocusMoveInfos_.front();
    EXPECT_TRUE(hostInfo.IsValidElement());
    EXPECT_EQ(hostInfo.GetBelongTreeId(), cmpTreeId);
    int64_t expectedHostId = bindId;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(cmpTreeId, expectedHostId);
    EXPECT_EQ(hostInfo.GetAccessibilityId(), expectedHostId);
}


// ============================================================================
// SearchInDissolvedEmbedContent : focus moves whose
// origin lands inside a same-layer-dissolved embed anchor's content. The subtree's
// flat list cannot resolve such origins (the dissolution cut removed the anchor and
// its content from enumeration); the helper searches the content's own list (embed
// map) instead.
// ============================================================================

/**
 * @tc.name: DissolvedEmbedDirectionGuard001
 * @tc.desc: Non-relative directions (UP) are not applicable — no search, no crash.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, DissolvedEmbedDirectionGuard001, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    auto context = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    AccessibilityElementInfo outInfo;
    auto ret = mainAM->SearchInDissolvedEmbedContent(exportedRoot->GetAccessibilityId(),
        static_cast<int32_t>(Accessibility::FocusMoveDirection::UP), false, 0, context,
        exportedRoot, outInfo);
    EXPECT_EQ(ret, JsAccessibilityManager::DissolvedEmbedSearch::NOT_APPLICABLE);
}

/**
 * @tc.name: DissolvedEmbedNoAnchor002
 * @tc.desc: An origin without any registered embed anchor above it (plain subtree) is
 *           not applicable — the walk-up terminates at the exported root.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, DissolvedEmbedNoAnchor002, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    // Plain child inside the subtree — never registered in the embed map.
    auto plainChild = FrameNode::CreateFrameNode("plain_child", 10, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(plainChild);
    plainChild->isActive_ = true;
    auto context = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    AccessibilityElementInfo outInfo;
    auto ret = mainAM->SearchInDissolvedEmbedContent(plainChild->GetAccessibilityId(),
        static_cast<int32_t>(Accessibility::FocusMoveDirection::FORWARD), false, 0, context,
        exportedRoot, outInfo);
    EXPECT_EQ(ret, JsAccessibilityManager::DissolvedEmbedSearch::NOT_APPLICABLE);
}

/**
 * @tc.name: DissolvedEmbedUnresolvableOrigin003
 * @tc.desc: An origin id that resolves to no node in the subtree is not applicable.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, DissolvedEmbedUnresolvableOrigin003, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    auto context = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    AccessibilityElementInfo outInfo;
    auto ret = mainAM->SearchInDissolvedEmbedContent(999999,
        static_cast<int32_t>(Accessibility::FocusMoveDirection::FORWARD), false, 0, context,
        exportedRoot, outInfo);
    EXPECT_EQ(ret, JsAccessibilityManager::DissolvedEmbedSearch::NOT_APPLICABLE);
}

/**
 * @tc.name: DissolvedEmbedInContentHit004
 * @tc.desc: A registered embed anchor with a sized child: FORWARD from the anchor finds
 *           the child in the content list (embed map source), answering FOUND.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, DissolvedEmbedInContentHit004, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto context = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootElement = context->GetRootElement();
    ASSERT_NE(rootElement, nullptr);

    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    rootElement->AddChild(owner);
    owner->isActive_ = true;
    exportedRoot->isActive_ = true;

    // Embed anchor: registered in the ElementRegister embed map (the same registration
    // NodeContainerPattern::HandleTextureExport performs in production).
    auto anchor = FrameNode::CreateFrameNode("embed_anchor", 10, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(anchor);
    anchor->isActive_ = true;
    anchor->context_ = AceType::RawPtr(context);
    uint64_t surfaceId = static_cast<uint64_t>(anchor->GetId());
    ElementRegister::GetInstance()->RegisterEmbedNode(surfaceId, anchor);

    // Content child with a non-zero paint rect (candidates with zero rects are skipped).
    auto contentChild = FrameNode::CreateFrameNode("content_button", 11, AceType::MakeRefPtr<Pattern>());
    anchor->AddChild(contentChild);
    contentChild->isActive_ = true;
    contentChild->context_ = AceType::RawPtr(context);
    NG::RectF rect(0.0f, 0.0f, 100.0f, 50.0f);
    auto renderContext = contentChild->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdatePaintRect(rect);

    mainAM->SetPipelineContext(context);
    mainAM->Register(true);
    mainAM->SetWindowId(static_cast<int32_t>(context->GetRealHostWindowId()));

    AccessibilityElementInfo outInfo;
    auto ret = mainAM->SearchInDissolvedEmbedContent(anchor->GetAccessibilityId(),
        static_cast<int32_t>(Accessibility::FocusMoveDirection::FORWARD), false, 0, context,
        exportedRoot, outInfo);
    EXPECT_EQ(ret, JsAccessibilityManager::DissolvedEmbedSearch::FOUND);
    EXPECT_EQ(outInfo.GetAccessibilityId(), contentChild->GetAccessibilityId());

    rootElement->RemoveChild(owner);
    ElementRegister::GetInstance()->UnregisterEmbedNode(surfaceId, anchor);
}

/**
 * @tc.name: DissolvedEmbedSplicedIdMatch005
 *  @tc.desc: The regression guard — with a non-zero treeId the content list
 *           carries treeId-spliced ids while the inbound origin is raw; the origin
 *           scan must compare in the spliced space and still resolve.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, DissolvedEmbedSplicedIdMatch005, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto context = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto rootElement = context->GetRootElement();
    ASSERT_NE(rootElement, nullptr);

    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    rootElement->AddChild(owner);
    owner->isActive_ = true;
    exportedRoot->isActive_ = true;

    auto anchor = FrameNode::CreateFrameNode("embed_anchor", 20, AceType::MakeRefPtr<Pattern>());
    exportedRoot->AddChild(anchor);
    anchor->isActive_ = true;
    anchor->context_ = AceType::RawPtr(context);
    uint64_t surfaceId = static_cast<uint64_t>(anchor->GetId());
    ElementRegister::GetInstance()->RegisterEmbedNode(surfaceId, anchor);

    auto contentChild = FrameNode::CreateFrameNode("content_button", 21, AceType::MakeRefPtr<Pattern>());
    anchor->AddChild(contentChild);
    contentChild->isActive_ = true;
    contentChild->context_ = AceType::RawPtr(context);
    NG::RectF rect(10.0f, 10.0f, 80.0f, 40.0f);
    contentChild->GetRenderContext()->UpdatePaintRect(rect);

    mainAM->SetPipelineContext(context);
    mainAM->Register(true);
    mainAM->SetWindowId(static_cast<int32_t>(context->GetRealHostWindowId()));
    // Non-zero tree id: the content list gets treeId-spliced by UpdateElementInfosTreeId
    // inside SearchElementInfoBySurfaceId (the production subAM has T_sub here).
    mainAM->treeId_ = 20;

    AccessibilityElementInfo outInfo;
    auto ret = mainAM->SearchInDissolvedEmbedContent(anchor->GetAccessibilityId(),
        static_cast<int32_t>(Accessibility::FocusMoveDirection::FORWARD), false, 0, context,
        exportedRoot, outInfo);
    EXPECT_EQ(ret, JsAccessibilityManager::DissolvedEmbedSearch::FOUND);
    // The FOUND info is the raw candidate from the content list — with treeId_ set the
    // list entries carry spliced ids; the raw-origin comparison still resolved ().
    int64_t expectedId = contentChild->GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(mainAM->treeId_, expectedId);
    EXPECT_EQ(outInfo.GetAccessibilityId(), expectedId);

    rootElement->RemoveChild(owner);
    ElementRegister::GetInstance()->UnregisterEmbedNode(surfaceId, anchor);
}

// ============================================================================
// GetWebPatternBySurfaceIdWithMainFallback (): subAM reads fall back to the
// main AM's webPatternMap_ (the only place WebDelegate writes to). Valid WebPattern
// instances cannot be constructed in this target (the web component's sources are not
// linked here), so these tests exercise the resolution path for no-crash and the
// miss-invalid contract; the routing itself was verified on device ().
// ============================================================================

/**
 * @tc.name: WebPatternMainFallbackSubAMNoCrash001
 * @tc.desc: A subAM's fallback path resolves without crash on empty maps (context
 *           resolution + main AM lookup) and reports the miss as an invalid weak ptr.
 */
HWTEST_F(JsAccessibilityManagerCmpTest, WebPatternMainFallbackSubAMNoCrash001, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_NE(exportedRoot, nullptr);
    ASSERT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 400, exportedRoot.GetRawPtr()));
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(400)));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    auto subAM = it->second.subAM;
    ASSERT_NE(subAM, nullptr);

    // Own map empty; the fallback resolves the main AM from the subAM's pipeline context
    // (set at Bind). Whether the mock pipeline exposes the main AM or not, the contract
    // is: no crash, miss reported as an invalid weak ptr.
    EXPECT_TRUE(subAM->GetWebPatternBySurfaceIdWithMainFallback("no_entry_surface").Invalid());
}

/**
 * @tc.name: WebPatternMainFallbackMainAMGate002
 * @tc.desc: The main AM (not a subAM) never takes the fallback branch — a miss stays
 *           a miss (IsCmpSubAM gate).
 */
HWTEST_F(JsAccessibilityManagerCmpTest, WebPatternMainFallbackMainAMGate002, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    ASSERT_NE(mainAM, nullptr);
    // Not a subAM: the IsCmpSubAM gate keeps the fallback branch inert.
    EXPECT_TRUE(mainAM->GetWebPatternBySurfaceIdWithMainFallback("no_entry_surface").Invalid());
}
} // namespace
} // namespace OHOS::Ace::Framework

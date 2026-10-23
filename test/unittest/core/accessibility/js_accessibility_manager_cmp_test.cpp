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
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "accessibility_system_ability_client.h"
#include "base/ressched/ressched_touch_optimizer.h"
#include "core/accessibility/accessibility_manager_ng.h"
#include "core/common/ai/ai_write_adapter.h"
#include "core/common/back_press_handler_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/manager/avoid_info/avoid_info_manager.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/components_ng/manager/force_split/force_split_manager.h"
#include "core/components_ng/manager/form_event/form_event_manager.h"
#include "core/components_ng/manager/form_gesture/form_gesture_manager.h"
#include "core/components_ng/manager/form_visible/form_visible_manager.h"
#include "core/components_ng/manager/frame_rate/frame_rate_manager.h"
#include "core/components_ng/manager/full_screen/full_screen_manager.h"
#include "core/components_ng/manager/material/material_processor.h"
#include "core/components_ng/manager/memory/memory_manager.h"
#include "core/components_ng/manager/navigation/navigation_manager.h"
#include "core/components_ng/manager/post_event/post_event_manager.h"
#include "core/components_ng/manager/privacy_sensitive/privacy_sensitive_manager.h"
#include "core/components_ng/manager/recoverable/recoverable_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/shared_overlay/shared_overlay_manager.h"
#include "core/components_ng/manager/toolbar/toolbar_manager.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/recycle_view/recycle_manager.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/ui_extension/dynamic_component/dynamic_component_manager.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#include "core/pipeline_ng/environment_manager.h"
#include "interfaces/inner_api/ace_kit/src/view/ui_context_impl.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "core/components_ng/base/cmp_accessibility_registry.h"
#include "test/unittest/core/accessibility/js_third_provider_interaction_operation_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
void FilterAccessibilityElementByFocusable(std::list<AccessibilityElementInfo>& elements,
    AccessibilityElementInfo& focus, int64_t elementId, bool preserveChildTreeHosts);
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
        ++registerCount;
        registeredTreeId = treeId;
        return true;
    }
    bool OnDeregister() override
    {
        ++deregisterCount;
        return !failDeregister;
    }
    bool OnSetChildTree(int32_t childWindowId, int32_t childTreeId) override
    {
        if (onSetChildTree) {
            onSetChildTree(childWindowId, childTreeId);
        }
        return true;
    }
    bool OnDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {
        return true;
    }
    void OnClearRegisterFlag() override
    {
        ++clearRegisterCount;
    }

    int32_t registerCount = 0;
    int32_t deregisterCount = 0;
    int32_t clearRegisterCount = 0;
    int32_t registeredTreeId = -1;
    bool failDeregister = false;
    std::function<void(int32_t, int32_t)> onSetChildTree;
};

class QueuedUITaskExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW,
        VsyncBarrierOption barrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (type == TaskType::UI) {
            uiTasks.emplace_back(std::move(task));
        } else {
            task();
        }
        return true;
    }

    mutable std::vector<Task> uiTasks;
};

class RecordingHoverPipeline : public NG::MockPipelineContext {
public:
    void OnAccessibilityHoverEvent(const TouchEvent& event, const RefPtr<FrameNode>& node) override
    {
        events.emplace_back(node->GetId(), event.type);
    }
    std::vector<std::pair<int32_t, TouchType>> events;
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

HWTEST_F(JsAccessibilityManagerCmpTest, OnCmpTreeRegistrationFailureReturnsToWait008, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1, 2);
    auto exportedRoot = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 330, exportedRoot.GetRawPtr()));
    // No third-provider operator for this host -> GetThirdProviderTreeIdForHost returns
    // UNDEFINED, binding stays WAIT_TREE_ID after Bind. Converge explicitly:
    owner->RemoveChild(exportedRoot);
    exportedRoot.Reset();
    mainAM->OnCmpTreeRegistered(owner.GetRawPtr(), 777);
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(330)));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
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

    owner1->RemoveChild(root1);
    root1.Reset();
    mainAM->OnCmpTreeRegistered(owner1.GetRawPtr(), 888);
    auto it1 = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(331)));
    auto it2 = mainAM->cmpBindingMap_.find(std::make_pair(3, static_cast<int64_t>(332)));
    ASSERT_NE(it1, mainAM->cmpBindingMap_.end());
    ASSERT_NE(it2, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it1->second.state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
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
    auto key = std::make_pair(1, static_cast<int64_t>(340));
    auto& binding = mainAM->cmpBindingMap_[key];
    auto registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
    binding.state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
    binding.registration = registration;
    auto callback = std::make_shared<FakeChildTreeCallback>(exportedRoot->GetAccessibilityId());
    mainAM->RegisterAccessibilityChildTreeCallback(exportedRoot->GetAccessibilityId(), callback);
    registration->SetTreeId(42);
    mainAM->MarkCmpBindingRegistered(key, binding.subAM, registration);
    EXPECT_EQ(binding.state, JsAccessibilityManager::CmpBindingState::State::REGISTERING);
    EXPECT_EQ(callback->deregisterCount, 0);
    EXPECT_TRUE(mainAM->CompleteCmpRegistration(key, binding.subAM, registration, true));
    registration->Complete(true);
    mainAM->MarkCmpBindingRegistered(key, binding.subAM, registration);
    auto it = mainAM->cmpBindingMap_.find(std::make_pair(1, static_cast<int64_t>(340)));
    ASSERT_NE(it, mainAM->cmpBindingMap_.end());
    EXPECT_EQ(it->second.state, JsAccessibilityManager::CmpBindingState::State::REGISTERED);
    EXPECT_TRUE(binding.subAM->IsRegister());
    EXPECT_EQ(binding.subAM->treeId_, 42);
    EXPECT_EQ(callback->deregisterCount, 1);
    EXPECT_EQ(callback->registerCount, 1);
    EXPECT_EQ(mainAM->childTreeCallbackMap_.count(exportedRoot->GetAccessibilityId()), 0u);
    EXPECT_EQ(binding.subAM->childTreeCallbackMap_.count(exportedRoot->GetAccessibilityId()), 1u);
    mainAM->MarkCmpBindingRegistered(key, binding.subAM, registration);
    EXPECT_EQ(callback->deregisterCount, 1);
    EXPECT_EQ(callback->registerCount, 1);
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
    callback->SetChildTreeId(71);
    callbackOutside->SetChildTreeId(72);
    mainAM->RegisterAccessibilityChildTreeCallback(inSubtree->GetAccessibilityId(), callback);
    mainAM->RegisterAccessibilityChildTreeCallback(outside->GetAccessibilityId(), callbackOutside);

    mainAM->MigrateChildTreeCallbacksToSubAM(exportedRoot, subAM);

    // In-subtree callback moved to the subAM, out-of-subtree stays on the main AM.
    EXPECT_EQ(mainAM->childTreeCallbackMap_.count(inSubtree->GetAccessibilityId()), 0u);
    EXPECT_EQ(subAM->childTreeCallbackMap_.count(inSubtree->GetAccessibilityId()), 1u);
    EXPECT_EQ(mainAM->childTreeCallbackMap_.count(outside->GetAccessibilityId()), 1u);
    EXPECT_EQ(subAM->childTreeCallbackMap_.count(outside->GetAccessibilityId()), 0u);
    EXPECT_EQ(callback->GetChildTreeId(), 0);
    EXPECT_EQ(callbackOutside->GetChildTreeId(), 72);
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
        mockRequestId = requestId;
    }
    void SetExecuteActionResult(const bool succeeded, const int32_t requestId) override
    {
        mockActionSucceeded_ = succeeded;
        ++mockActionCallbackCount_;
        mockRequestId = requestId;
    }
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
        ++mockFocusMoveCallbackCount_;
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
    int32_t mockFocusMoveCallbackCount_ = 0;
    bool mockActionSucceeded_ = false;
    int32_t mockActionCallbackCount_ = 0;
    std::list<Accessibility::AccessibilityElementInfo> mockFocusMoveInfos_;
    Accessibility::FocusMoveResult mockFocusMoveResult_ {};
};

class ExplicitFocusPipeline : public NG::MockPipelineContext {
public:
    RefPtr<AccessibilityManager> GetAccessibilityManager() const override
    {
        return mainAM;
    }
    RefPtr<JsAccessibilityManager> mainAM;
};

class CmpExplicitFocusTest : public JsAccessibilityManagerCmpTest {
public:
    void SetUp() override
    {
        context_ = AceType::MakeRefPtr<ExplicitFocusPipeline>();
        context_->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
        context_->mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
        context_->mainAM->SetPipelineContext(context_);
        root_ = CreateNode("exported_root", 0);
        first_ = CreateNode("first", 10);
        middle_ = CreateNode("middle", 30);
        last_ = CreateNode("last", 50);
        root_->AddChild(first_);
        root_->AddChild(middle_);
        root_->AddChild(last_);
        middle_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(71);
        last_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(72);
        SetNextFocus(first_, "last");

        subAM_ = AceType::MakeRefPtr<JsAccessibilityManager>();
        subAM_->SetPipelineContext(context_);
        subAM_->Register(true);
        subAM_->treeId_ = TREE_ID;
        std::list<AccessibilityElementInfo> probe;
        subAM_->SearchElementInfoByAccessibilityIdFromRoot(-1,
            Accessibility::PREFETCH_RECURSIVE_CHILDREN, probe, context_, root_);
        ASSERT_EQ(probe.size(), 4u);
        subAM_->SetWindowId(static_cast<uint32_t>(probe.front().GetWindowId()));
        operation_ = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
            1, 100, root_, 1, WeakPtr<JsAccessibilityManager>());
        operation_->SetHandler(Referenced::WeakClaim(subAM_.GetRawPtr()));
    }

    void TearDown() override
    {
        subAM_->Register(false);
    }

    RefPtr<FrameNode> CreateNode(const std::string& inspectorId, float top)
    {
        auto node = FrameNode::CreateFrameNode("cmp_focus_node",
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        node->context_ = AceType::RawPtr(context_);
        node->isActive_ = true;
        node->UpdateInspectorId(inspectorId);
        auto renderContext = AceType::MakeRefPtr<NG::MockRenderContext>();
        renderContext->SetPaintRectWithTransform(NG::RectF(0, top, 100, 100));
        node->renderContext_ = renderContext;
        return node;
    }

    void SetNextFocus(const RefPtr<FrameNode>& source, const std::string& targetKey)
    {
        auto property = source->GetAccessibilityProperty<NG::AccessibilityProperty>();
        property->accessibilityNextFocusParams_ = NG::AccessibilityNextFocusParams {};
        property->accessibilityNextFocusParams_->nextFocusInspectorKey = targetKey;
        context_->mainAM->JsAccessibilityManager::UpdateAccessibilityNextFocusIdMap(
            context_->GetInstanceId(), targetKey, source->GetAccessibilityId(), false);
    }

    MockAmsElementOperatorCallback Search(const RefPtr<FrameNode>& origin,
        Accessibility::FocusMoveDirection direction,
        Accessibility::DetailCondition condition = Accessibility::DetailCondition::BYPASS_SELF,
        Accessibility::FocusRuleType type = Accessibility::FocusRuleType::DEFAULT)
    {
        AccessibilityElementInfo info;
        auto elementId = origin->GetAccessibilityId();
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(TREE_ID, elementId);
        info.SetAccessibilityId(elementId);
        Accessibility::AccessibilityFocusMoveParam param;
        param.direction = direction;
        param.condition = condition;
        param.type = type;
        MockAmsElementOperatorCallback callback;
        operation_->FocusMoveSearchWithCondition(info, param, 123, callback);
        return callback;
    }

    void ExpectHandoff(const MockAmsElementOperatorCallback& callback, const RefPtr<FrameNode>& target,
        Accessibility::FocusMoveResultType resultType = Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY)
    {
        EXPECT_TRUE(callback.mockFocusMoveWithConditionCalled_);
        EXPECT_FALSE(callback.mockLegacyFocusMoveCalled_);
        EXPECT_EQ(callback.mockFocusMoveCallbackCount_, 1);
        EXPECT_EQ(callback.mockRequestId, 123);
        EXPECT_EQ(callback.mockFocusMoveResult_.resultType, resultType);
        EXPECT_EQ(callback.mockFocusMoveResult_.nowLevelBelongTreeId, -1);
        EXPECT_FALSE(callback.mockFocusMoveResult_.needTerminate);
        ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
        auto expectedId = target->GetAccessibilityId();
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(TREE_ID, expectedId);
        EXPECT_EQ(callback.mockFocusMoveInfos_.front().GetAccessibilityId(), expectedId);
        EXPECT_EQ(callback.mockFocusMoveInfos_.front().GetBelongTreeId(), TREE_ID);
    }

    static constexpr int32_t TREE_ID = 5;
    RefPtr<ExplicitFocusPipeline> context_;
    RefPtr<FrameNode> root_;
    RefPtr<FrameNode> first_;
    RefPtr<FrameNode> middle_;
    RefPtr<FrameNode> last_;
    RefPtr<JsAccessibilityManager> subAM_;
    std::shared_ptr<JsAccessibilityManager::CmpInteractionOperation> operation_;
};

class DetachFocusTaskExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW,
        VsyncBarrierOption barrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (type == TaskType::BACKGROUND) {
            backgroundTasks.emplace_back(std::move(task));
        } else if (type == TaskType::UI) {
            uiTasks.emplace_back(std::move(task));
        } else {
            task();
        }
        return true;
    }
    void FlushUI()
    {
        std::vector<Task> tasks;
        tasks.swap(uiTasks);
        for (auto& task : tasks) {
            task();
        }
    }
    mutable std::vector<Task> uiTasks;
    mutable std::vector<Task> backgroundTasks;
};

class CmpDetachFocusTest : public CmpExplicitFocusTest {
public:
    void SetUp() override
    {
        CmpExplicitFocusTest::SetUp();
        SetNextFocus(first_, "");
        context_->rootNode_ = CreateNode("main_root", 0);
        executor_ = AceType::MakeRefPtr<DetachFocusTaskExecutor>();
        context_->SetTaskExecutor(executor_);
        subAM_->MarkCmpSubAM();
        subAM_->cmpExportedRoot_ = root_;
        subAM_->SetWindowId(123);
        auto& binding = context_->mainAM->cmpBindingMap_[{100, 1001}];
        binding.state = JsAccessibilityManager::CmpBindingState::State::REGISTERED;
        binding.subAM = subAM_;
        binding.exportedRoot = root_;
        CmpAccessibility::CmpAccessibilityRegistry::RegisterSubAM(
            root_.GetRawPtr(), AceType::DynamicCast<AceType>(subAM_));
    }

    void TearDown() override
    {
        CmpAccessibility::CmpAccessibilityRegistry::UnregisterSubAM(root_.GetRawPtr());
        context_->mainAM->cmpBindingMap_.clear();
        executor_->uiTasks.clear();
        executor_->backgroundTasks.clear();
        CmpExplicitFocusTest::TearDown();
    }

    void Focus(const RefPtr<FrameNode>& node)
    {
        ASSERT_TRUE(subAM_->ExecuteActionFromRoot(node->GetAccessibilityId(), {},
            Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, context_, root_));
        ASSERT_EQ(subAM_->currentFocusNodeId_, node->GetAccessibilityId());
    }

    void FlushDetach()
    {
        executor_->FlushUI();
    }

    RefPtr<DetachFocusTaskExecutor> executor_;
};

HWTEST_F(CmpDetachFocusTest, DeleteFocusedNodeKeepsNextCandidates, TestSize.Level1)
{
    Focus(first_);
    ASSERT_EQ(first_->GetPaintNode(), nullptr);
    context_->mainAM->currentFocusNodeId_ = 99999;
    context_->mainAM->OnAccessbibilityDetachFromMainTree(first_);
    EXPECT_EQ(context_->mainAM->currentFocusNodeId_, 99999);
    EXPECT_EQ(subAM_->currentFocusNodeId_, -1);
    EXPECT_FALSE(first_->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetAccessibilityFocusState());
    auto cached = subAM_->detachFocusFallbackCache_.at(context_->GetInstanceId());
    EXPECT_EQ(cached.focusNodeId, first_->GetAccessibilityId());
    EXPECT_EQ(cached.treeId, TREE_ID);
    const std::vector<int64_t> expected {middle_->GetAccessibilityId()};
    EXPECT_EQ(cached.candidateIds, expected);
    root_->RemoveChild(first_);
    EXPECT_EQ(first_->GetParent(), nullptr);
    EXPECT_EQ(executor_->uiTasks.size(), 1u);
    EXPECT_TRUE(executor_->backgroundTasks.empty());
    FlushDetach();
    EXPECT_TRUE(subAM_->detachFocusFallbackCache_.empty());
    EXPECT_EQ(executor_->backgroundTasks.size(), 1u);
    FlushDetach();
    EXPECT_EQ(executor_->backgroundTasks.size(), 1u);
}

HWTEST_F(CmpDetachFocusTest, RemovingUnfocusedNodeDoesNotMoveFocus, TestSize.Level1)
{
    Focus(first_);
    context_->mainAM->OnAccessbibilityDetachFromMainTree(middle_);
    root_->RemoveChild(middle_);
    FlushDetach();
    EXPECT_EQ(subAM_->currentFocusNodeId_, first_->GetAccessibilityId());
    EXPECT_TRUE(subAM_->detachFocusFallbackCache_.empty());
    EXPECT_TRUE(executor_->backgroundTasks.empty());
}

HWTEST_F(CmpDetachFocusTest, ExplicitRelationsStayWithinExportedRoot, TestSize.Level1)
{
    SetNextFocus(first_, "last");
    auto candidates = subAM_->QueryDetachFocusFallbackCandidates(first_, context_, 3);
    ASSERT_FALSE(candidates.empty());
    EXPECT_EQ(candidates.front(), last_->GetAccessibilityId());
    auto outside = CreateNode("outside", 80);
    context_->rootNode_->AddChild(outside);
    SetNextFocus(first_, "outside");
    candidates = subAM_->QueryDetachFocusFallbackCandidates(first_, context_, 3);
    const std::vector<int64_t> expected {middle_->GetAccessibilityId()};
    EXPECT_EQ(candidates, expected);
    candidates = subAM_->QueryDetachFocusFallbackCandidates(last_, context_, 3);
    EXPECT_TRUE(candidates.empty());
}

HWTEST_F(CmpDetachFocusTest, EventSourceAndCandidatesUseSubtreeId, TestSize.Level1)
{
    subAM_->CacheDetachFocusFallbackCandidates(context_->GetInstanceId(), first_->GetAccessibilityId(),
        {middle_->GetAccessibilityId(), last_->GetAccessibilityId()});
    auto data = subAM_->GetAndClearDetachFocusFallbackCandidates(context_->GetInstanceId());
    auto event = subAM_->BuildDetachFocusFallbackEvent(data, 123);
    auto splice = [](int64_t id) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(TREE_ID, id);
        return id;
    };
    EXPECT_EQ(event.GetEventType(), Accessibility::EventType::TYPE_FOCUS_INVISIBLE);
    EXPECT_EQ(event.GetAccessibilityId(), splice(first_->GetAccessibilityId()));
    EXPECT_EQ(event.GetElementInfo().GetAccessibilityId(), splice(first_->GetAccessibilityId()));
    EXPECT_EQ(event.GetElementInfo().GetBelongTreeId(), TREE_ID);
    EXPECT_EQ(event.GetWindowId(), 123);
    EXPECT_EQ(event.GetExtraEvent().GetExtraEventInfoValueByKey("candidateIds"),
        std::to_string(splice(middle_->GetAccessibilityId())) + ", " +
        std::to_string(splice(last_->GetAccessibilityId())));
}

HWTEST_F(CmpDetachFocusTest, ExpiredRegistrationDoesNotSendFallback, TestSize.Level1)
{
    Focus(first_);
    context_->mainAM->OnAccessbibilityDetachFromMainTree(first_);
    subAM_->treeId_ = TREE_ID + 1;
    FlushDetach();
    EXPECT_TRUE(subAM_->detachFocusFallbackCache_.empty());
    EXPECT_TRUE(executor_->backgroundTasks.empty());
}

HWTEST_F(CmpDetachFocusTest, UnboundSubtreeDoesNotSendFallback, TestSize.Level1)
{
    Focus(first_);
    context_->mainAM->OnAccessbibilityDetachFromMainTree(first_);
    context_->mainAM->DetachCmpBinding({100, 1001});
    FlushDetach();
    EXPECT_TRUE(executor_->backgroundTasks.empty());
}

HWTEST_F(CmpDetachFocusTest, NewFocusCancelsPendingFallback, TestSize.Level1)
{
    Focus(first_);
    context_->mainAM->OnAccessbibilityDetachFromMainTree(first_);
    ASSERT_FALSE(subAM_->detachFocusFallbackCache_.empty());
    Focus(last_);
    FlushDetach();
    EXPECT_TRUE(subAM_->detachFocusFallbackCache_.empty());
    EXPECT_TRUE(executor_->backgroundTasks.empty());
    EXPECT_EQ(subAM_->currentFocusNodeId_, last_->GetAccessibilityId());
}

HWTEST_F(CmpDetachFocusTest, MainTreeCallbackPreservesMainTreeIds, TestSize.Level1)
{
    auto mainAM = context_->mainAM;
    mainAM->treeId_ = 2;
    mainAM->CacheDetachFocusFallbackCandidates(context_->GetInstanceId(), 101, {102});
    auto data = mainAM->GetAndClearDetachFocusFallbackCandidates(context_->GetInstanceId());
    auto event = mainAM->BuildDetachFocusFallbackEvent(data, 10);
    int64_t source = 101;
    int64_t target = 102;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(2, source);
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(2, target);
    EXPECT_EQ(event.GetAccessibilityId(), source);
    EXPECT_EQ(event.GetExtraEvent().GetExtraEventInfoValueByKey("candidateIds"), std::to_string(target));
    mainAM->CacheDetachFocusFallbackCandidates(context_->GetInstanceId(), 101, {102});
    mainAM->FireAccessibilityEventCallback(
        static_cast<uint32_t>(AccessibilityCallbackEventId::ON_SEND_DETACH_FOCUS_FALLBACK),
        context_->GetInstanceId());
    EXPECT_TRUE(mainAM->detachFocusFallbackCache_.empty());
    EXPECT_EQ(executor_->backgroundTasks.size(), 1u);
}

HWTEST_F(CmpDetachFocusTest, DeviceIdsPreserveExportedTreeOnDispatch, TestSize.Level1)
{
    subAM_->treeId_ = 85;
    subAM_->CacheDetachFocusFallbackCandidates(context_->GetInstanceId(), 54, {55});
    auto data = subAM_->GetAndClearDetachFocusFallbackCandidates(context_->GetInstanceId());
    auto event = subAM_->BuildDetachFocusFallbackEvent(data, 123);
    EXPECT_EQ(event.GetAccessibilityId(), 93458488361014);
    EXPECT_EQ(event.GetExtraEvent().GetExtraEventInfoValueByKey("candidateIds"), "93458488361015");
    EXPECT_EQ(event.GetEventType(), Accessibility::EventType::TYPE_FOCUS_INVISIBLE);
}

HWTEST_F(CmpDetachFocusTest, FrameCallbackAndUITaskSendOnce, TestSize.Level1)
{
    Focus(first_);
    context_->mainAM->OnAccessbibilityDetachFromMainTree(first_);
    root_->RemoveChild(first_);
    ASSERT_EQ(executor_->uiTasks.size(), 1u);
    context_->mainAM->FireAccessibilityEventCallback(
        static_cast<uint32_t>(AccessibilityCallbackEventId::ON_SEND_DETACH_FOCUS_FALLBACK),
        context_->GetInstanceId());
    EXPECT_EQ(executor_->backgroundTasks.size(), 1u);
    FlushDetach();
    EXPECT_EQ(executor_->backgroundTasks.size(), 1u);
    EXPECT_TRUE(subAM_->detachFocusFallbackCache_.empty());
}

class CmpScrollFocusTest : public CmpExplicitFocusTest {
public:
    void SetUp() override
    {
        CmpExplicitFocusTest::SetUp();
        SetNextFocus(first_, "");
        middle_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(0);
        root_->RemoveChild(first_);
        root_->RemoveChild(middle_);
        root_->RemoveChild(last_);
        before_ = CreateNode("before_scroll", 0);
        before_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(73);
        scroll_ = CreateScroll();
        auto content = CreateNode("scroll_content", 0);
        content->AddChild(first_);
        content->AddChild(middle_);
        scroll_->AddChild(content);
        root_->AddChild(before_);
        root_->AddChild(scroll_);
        root_->AddChild(last_);
    }

    RefPtr<FrameNode> CreateScroll()
    {
        auto pattern = AceType::MakeRefPtr<NG::ScrollPattern>();
        pattern->SetAxis(Axis::VERTICAL);
        pattern->scrollableDistance_ = 100.0f;
        pattern->currentOffset_ = -50.0f;
        auto node = FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        node->context_ = AceType::RawPtr(context_);
        node->isActive_ = true;
        auto renderContext = AceType::MakeRefPtr<NG::MockRenderContext>();
        renderContext->SetPaintRectWithTransform(NG::RectF(0, 0, 100, 100));
        node->renderContext_ = renderContext;
        return node;
    }

    void ExpectScroll(const MockAmsElementOperatorCallback& callback, const RefPtr<FrameNode>& scroll)
    {
        EXPECT_EQ(callback.mockFocusMoveCallbackCount_, 1);
        EXPECT_EQ(callback.mockRequestId, 123);
        EXPECT_EQ(callback.mockFocusMoveResult_.resultType, Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_SCROLL);
        EXPECT_EQ(callback.mockFocusMoveResult_.nowLevelBelongTreeId, TREE_ID);
        EXPECT_FALSE(callback.mockFocusMoveResult_.needTerminate);
        ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
        auto expectedId = scroll->GetAccessibilityId();
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(TREE_ID, expectedId);
        EXPECT_EQ(callback.mockFocusMoveInfos_.front().GetAccessibilityId(), expectedId);
        EXPECT_EQ(callback.mockFocusMoveInfos_.front().GetBelongTreeId(), TREE_ID);
        EXPECT_TRUE(callback.mockFocusMoveInfos_.front().IsValidElement());
        EXPECT_TRUE(callback.mockFocusMoveInfos_.front().IsScrollable());
    }

    RefPtr<FrameNode> before_;
    RefPtr<FrameNode> scroll_;
};

class CmpParentLifecycleTest : public JsAccessibilityManagerCmpTest {
public:
    void SetUp() override
    {
        context_ = AceType::MakeRefPtr<ExplicitFocusPipeline>();
        executor_ = AceType::MakeRefPtr<QueuedUITaskExecutor>();
        context_->SetTaskExecutor(executor_);
        manager_ = AceType::MakeRefPtr<JsAccessibilityManager>();
        manager_->SetPipelineContext(context_);
        context_->mainAM = manager_;
        owner_ = CreateAdoptedPair(2101, 2102);
        root_ = AceType::DynamicCast<FrameNode>(owner_->GetChildAtIndex(0));
        owner_->context_ = AceType::RawPtr(context_);
        root_->context_ = AceType::RawPtr(context_);
        callback_ = std::make_shared<FakeChildTreeCallback>(root_->GetAccessibilityId());
        manager_->RegisterAccessibilityChildTreeCallback(root_->GetAccessibilityId(), callback_);
        provider_ = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
        ASSERT_TRUE(manager_->BindExportTree(owner_.GetRawPtr(), BIND_ID, root_.GetRawPtr()));
        subAM_ = manager_->cmpBindingMap_.at(Key()).subAM;
    }

    void TearDown() override
    {
        manager_->UnbindExportTree(owner_.GetRawPtr(), BIND_ID);
        manager_->DeregisterAccessibilityChildTreeCallback(root_->GetAccessibilityId());
        DrainUI();
    }

    std::pair<int32_t, int64_t> Key() const
    {
        return { owner_->GetId(), BIND_ID };
    }

    void DrainUI()
    {
        auto tasks = std::move(executor_->uiTasks);
        executor_->uiTasks.clear();
        for (auto& task : tasks) {
            task();
        }
    }

    std::shared_ptr<JsThirdProviderInteractionOperation> CreateProvider(int32_t treeId)
    {
        auto operation = std::make_shared<JsThirdProviderInteractionOperation>(provider_, manager_, owner_);
        manager_->RegisterJsThirdProviderInteractionOperation(owner_->GetAccessibilityId(), operation);
        operation->SetBelongTreeId(treeId);
        return operation;
    }

    std::shared_ptr<JsAccessibilityManager::CmpInteractionOperation> AcceptSubtree(int32_t treeId)
    {
        auto& state = manager_->cmpBindingMap_.at(Key());
        EXPECT_GT(state.cmpTreeId, 0);
        state.state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
        auto registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
        state.registration = registration;
        EXPECT_TRUE(manager_->CompleteCmpRegistration(Key(), subAM_, registration, true));
        registration->Complete(true);
        auto parentId = BIND_ID;
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(state.cmpTreeId, parentId);
        manager_->WireSubAmAnchors(subAM_, root_, owner_.GetRawPtr(), state.cmpTreeId, parentId, 123);
        auto operation = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
            state.cmpTreeId, BIND_ID, root_, owner_->GetId(), manager_, registration);
        operation->SetHandler(subAM_);
        if (treeId > 0) {
            operation->SetBelongTreeId(treeId);
            DrainUI();
        }
        return operation;
    }

    static constexpr int64_t BIND_ID = 1001;
    RefPtr<ExplicitFocusPipeline> context_;
    RefPtr<QueuedUITaskExecutor> executor_;
    RefPtr<JsAccessibilityManager> manager_;
    RefPtr<JsAccessibilityManager> subAM_;
    RefPtr<FrameNode> owner_;
    RefPtr<FrameNode> root_;
    RefPtr<MockOhAccessibilityProvider> provider_;
    std::shared_ptr<FakeChildTreeCallback> callback_;
};
} // namespace

HWTEST_F(CmpScrollFocusTest, ForwardBoundaryRequestsScroll, TestSize.Level1)
{
    ExpectScroll(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), scroll_);
}

HWTEST_F(CmpScrollFocusTest, BackwardBoundaryRequestsScroll, TestSize.Level1)
{
    ExpectScroll(Search(first_, Accessibility::FocusMoveDirection::BACKWARD), scroll_);
}

HWTEST_F(CmpScrollFocusTest, InternalCandidateBeforeScroll, TestSize.Level1)
{
    middle_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(71);
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), middle_);
    first_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(70);
    ExpectHandoff(Search(middle_, Accessibility::FocusMoveDirection::BACKWARD), first_,
        Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST);
}

HWTEST_F(CmpScrollFocusTest, ScrollEndsAllowOutsideCandidate, TestSize.Level1)
{
    auto pattern = scroll_->GetPattern<NG::ScrollPattern>();
    pattern->currentOffset_ = -100.0f;
    ExpectHandoff(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), last_);
    pattern->currentOffset_ = 0.0f;
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::BACKWARD), before_,
        Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST);
}

HWTEST_F(CmpScrollFocusTest, NestedScrollUsesNearestAvailableAncestor, TestSize.Level1)
{
    auto outer = CreateScroll();
    root_->RemoveChild(scroll_);
    outer->AddChild(scroll_);
    root_->AddChild(outer, 1);
    ExpectScroll(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), scroll_);
    scroll_->GetPattern<NG::ScrollPattern>()->currentOffset_ = -100.0f;
    ExpectScroll(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), outer);
    ExpectScroll(Search(first_, Accessibility::FocusMoveDirection::BACKWARD), scroll_);
    scroll_->GetPattern<NG::ScrollPattern>()->currentOffset_ = 0.0f;
    ExpectScroll(Search(first_, Accessibility::FocusMoveDirection::BACKWARD), outer);
}

HWTEST_F(CmpScrollFocusTest, ScrollRootClipsAncestorSearch, TestSize.Level1)
{
    operation_ = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
        1, 100, scroll_, 1, WeakPtr<JsAccessibilityManager>());
    operation_->SetHandler(subAM_);
    auto outer = CreateScroll();
    outer->AddChild(root_);
    ExpectScroll(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), scroll_);
    scroll_->GetPattern<NG::ScrollPattern>()->currentOffset_ = -100.0f;
    auto callback = Search(middle_, Accessibility::FocusMoveDirection::FORWARD);
    EXPECT_EQ(callback.mockFocusMoveResult_.resultType, Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE);
    EXPECT_EQ(callback.mockFocusMoveCallbackCount_, 1);
}

HWTEST_F(CmpScrollFocusTest, ExplicitRelationPrecedesScroll, TestSize.Level1)
{
    SetNextFocus(middle_, "last");
    ExpectHandoff(Search(middle_, Accessibility::FocusMoveDirection::FORWARD), last_);
}

HWTEST_F(CmpScrollFocusTest, TitleAndLinkDoNotRequestScroll, TestSize.Level1)
{
    for (auto type : { Accessibility::FocusRuleType::FOCUS_BY_TITLE, Accessibility::FocusRuleType::FOCUS_BY_LINK }) {
        auto callback = Search(middle_, Accessibility::FocusMoveDirection::FORWARD,
            Accessibility::DetailCondition::BYPASS_SELF, type);
        EXPECT_EQ(callback.mockFocusMoveResult_.resultType, Accessibility::FocusMoveResultType::SEARCH_FAIL);
        ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
        EXPECT_FALSE(callback.mockFocusMoveInfos_.front().IsValidElement());
    }
}

HWTEST_F(CmpScrollFocusTest, ReturnedScrollRoutesAction, TestSize.Level1)
{
    auto property = scroll_->GetAccessibilityProperty<NG::AccessibilityProperty>();
    int forwardCount = 0;
    int backwardCount = 0;
    property->SetActionScrollForward([&forwardCount] { ++forwardCount; });
    property->SetActionScrollBackward([&backwardCount] { ++backwardCount; });
    for (auto direction : { Accessibility::FocusMoveDirection::FORWARD, Accessibility::FocusMoveDirection::BACKWARD }) {
        bool forward = direction == Accessibility::FocusMoveDirection::FORWARD;
        auto callback = Search(forward ? middle_ : first_, direction);
        ExpectScroll(callback, scroll_);
        ASSERT_EQ(callback.mockFocusMoveInfos_.size(), 1u);
        auto action = forward ? Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD
                              : Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD;
        operation_->ExecuteAction(callback.mockFocusMoveInfos_.front().GetAccessibilityId(),
            static_cast<int32_t>(action), {}, 124, callback);
        EXPECT_TRUE(callback.mockActionSucceeded_);
        EXPECT_EQ(callback.mockActionCallbackCount_, 1);
        EXPECT_EQ(callback.mockRequestId, 124);
    }
    EXPECT_EQ(forwardCount, 1);
    EXPECT_EQ(backwardCount, 1);
}

HWTEST_F(CmpParentLifecycleTest, ProviderDestructionThenReaderRegistration, TestSize.Level1)
{
    auto viewer = CreateProvider(601);
    DrainUI();
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).cmpTreeId, 601);
    auto oldOperation = AcceptSubtree(602);
    auto oldRegistration = manager_->cmpBindingMap_.at(Key()).registration;
    ASSERT_TRUE(subAM_->IsRegister());
    EXPECT_EQ(callback_->registerCount, 1);
    callback_->SetChildTreeId(603);
    callback_->failDeregister = true;
    viewer.reset();
    EXPECT_TRUE(subAM_->IsRegister());
    DrainUI();
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    EXPECT_FALSE(subAM_->IsRegister());
    EXPECT_EQ(subAM_->treeId_, 0);
    EXPECT_TRUE(oldRegistration->IsCanceled());
    EXPECT_EQ(callback_->clearRegisterCount, 1);
    EXPECT_EQ(callback_->GetChildTreeId(), 0);
    EXPECT_EQ(subAM_->childTreeCallbackMap_.at(root_->GetAccessibilityId()), callback_);
    EXPECT_EQ(JsAccessibilityManager::GetSubAccessibilityManager(root_.GetRawPtr()).Upgrade(), subAM_);

    auto reader = CreateProvider(637);
    DrainUI();
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).cmpTreeId, 637);
    auto newOperation = AcceptSubtree(638);
    EXPECT_TRUE(subAM_->IsRegister());
    EXPECT_EQ(subAM_->parentTreeId_, 637);
    EXPECT_EQ(subAM_->treeId_, 638);
    EXPECT_EQ(callback_->registerCount, 2);
    EXPECT_EQ(callback_->registeredTreeId, 638);
    EXPECT_TRUE(manager_->BindExportTree(owner_.GetRawPtr(), BIND_ID, root_.GetRawPtr()));
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).subAM, subAM_);
    EXPECT_EQ(subAM_->treeId_, 638);
}

HWTEST_F(CmpParentLifecycleTest, ReplacedParentIgnoresLateOldNotifications, TestSize.Level1)
{
    auto oldParent = CreateProvider(611);
    DrainUI();
    auto oldOperation = AcceptSubtree(612);
    auto oldRegistration = manager_->cmpBindingMap_.at(Key()).registration;
    auto newParent = CreateProvider(637);
    DrainUI();
    EXPECT_TRUE(oldRegistration->IsCanceled());
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).cmpTreeId, 637);
    auto newOperation = AcceptSubtree(638);
    auto newRegistration = manager_->cmpBindingMap_.at(Key()).registration;
    callback_->SetChildTreeId(639);
    oldParent->SetBelongTreeId(611);
    oldParent.reset();
    oldOperation->SetBelongTreeId(612);
    oldOperation->SetChildTreeIdAndWinId(root_->GetAccessibilityId(), 613, 123);
    DrainUI();
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).registration, newRegistration);
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).state, JsAccessibilityManager::CmpBindingState::State::REGISTERED);
    EXPECT_EQ(subAM_->treeId_, 638);
    EXPECT_EQ(callback_->GetChildTreeId(), 639);
    EXPECT_EQ(callback_->registerCount, 2);
    newParent->SetBelongTreeId(637);
    DrainUI();
    EXPECT_EQ(callback_->registerCount, 2);
}

HWTEST_F(CmpParentLifecycleTest, LostParentDuringSubtreeRegistration, TestSize.Level1)
{
    auto parent = CreateProvider(621);
    DrainUI();
    auto oldOperation = AcceptSubtree(-1);
    auto oldRegistration = manager_->cmpBindingMap_.at(Key()).registration;
    parent.reset();
    DrainUI();
    EXPECT_TRUE(oldRegistration->IsCanceled());
    EXPECT_FALSE(oldRegistration->cleaned);
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    EXPECT_EQ(callback_->registerCount, 0);
    oldOperation->SetBelongTreeId(622);
    DrainUI();
    EXPECT_TRUE(oldRegistration->cleaned);
    EXPECT_FALSE(subAM_->IsRegister());
    EXPECT_EQ(callback_->registerCount, 0);
}

HWTEST_F(CmpParentLifecycleTest, InvalidationOnlySuspendsMatchingOwner, TestSize.Level1)
{
    auto otherOwner = CreateAdoptedPair(2111, 2112);
    auto otherRoot = AceType::DynamicCast<FrameNode>(otherOwner->GetChildAtIndex(0));
    ASSERT_TRUE(manager_->BindExportTree(otherOwner.GetRawPtr(), BIND_ID, otherRoot.GetRawPtr()));
    auto otherKey = std::make_pair(otherOwner->GetId(), BIND_ID);
    manager_->cmpBindingMap_.at(otherKey).cmpTreeId = 631;
    manager_->cmpBindingMap_.at(otherKey).state = JsAccessibilityManager::CmpBindingState::State::REGISTERED;
    auto parent = CreateProvider(631);
    DrainUI();
    auto operation = AcceptSubtree(632);
    parent->SetChildTreeIdAndWinId(BIND_ID, 632, 123);
    parent->SetBelongTreeId(0);
    DrainUI();
    EXPECT_FALSE(parent->HasChildTree(BIND_ID));
    EXPECT_EQ(manager_->cmpBindingMap_.at(Key()).state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    EXPECT_EQ(manager_->cmpBindingMap_.at(otherKey).state, JsAccessibilityManager::CmpBindingState::State::REGISTERED);
    EXPECT_EQ(manager_->cmpBindingMap_.at(otherKey).cmpTreeId, 631);
    manager_->UnbindExportTree(otherOwner.GetRawPtr(), BIND_ID);
}

HWTEST_F(CmpExplicitFocusTest, ForwardRelationOverridesReadingOrder, TestSize.Level1)
{
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), last_);
}

HWTEST_F(CmpExplicitFocusTest, BackwardRelationUsesMainManager, TestSize.Level1)
{
    first_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(70);
    EXPECT_TRUE(subAM_->nextFocusRelationController_.GetTargetMap(context_->GetInstanceId()).empty());
    ASSERT_EQ(context_->mainAM->nextFocusRelationController_.GetTargetMap(context_->GetInstanceId())
        .at("last").sourceAccessibilityId, first_->GetAccessibilityId());
    ExpectHandoff(Search(last_, Accessibility::FocusMoveDirection::BACKWARD), first_,
        Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST);
}

HWTEST_F(CmpExplicitFocusTest, OriginChildTreeKeepsPriority, TestSize.Level1)
{
    first_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(70);
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), first_);
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD,
        Accessibility::DetailCondition::CHECK_SELF), first_);
}

HWTEST_F(CmpExplicitFocusTest, MissingAndSelfRelationsFallBack, TestSize.Level1)
{
    for (const auto& key : { "", "missing", "first" }) {
        SCOPED_TRACE(key);
        SetNextFocus(first_, key);
        ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), middle_);
    }
}

HWTEST_F(CmpExplicitFocusTest, RelationsCannotEscapeExportedRoot, TestSize.Level1)
{
    auto appRoot = CreateNode("app_root", 0);
    auto outside = CreateNode("outside", 70);
    appRoot->AddChild(root_);
    appRoot->AddChild(outside);
    outside->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(73);
    SetNextFocus(first_, "outside");
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), middle_);
    SetNextFocus(outside, "last");
    ExpectHandoff(Search(last_, Accessibility::FocusMoveDirection::BACKWARD), middle_,
        Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST);
}

HWTEST_F(CmpExplicitFocusTest, FilteredRelationTargetFallsBack, TestSize.Level1)
{
    last_->GetAccessibilityProperty<NG::AccessibilityProperty>()->SetChildTreeId(0);
    AceType::DynamicCast<NG::MockRenderContext>(last_->GetRenderContext())->SetPaintRectWithTransform(NG::RectF());
    ExpectHandoff(Search(first_, Accessibility::FocusMoveDirection::FORWARD), middle_);
}

HWTEST_F(CmpExplicitFocusTest, BypassDescendantsDoesNotHandoff, TestSize.Level1)
{
    for (const auto& node : { middle_, last_ }) {
        AceType::DynamicCast<NG::MockRenderContext>(node->GetRenderContext())->SetPaintRectWithTransform(NG::RectF());
    }
    auto callback = Search(first_, Accessibility::FocusMoveDirection::FORWARD,
        Accessibility::DetailCondition::BYPASS_SELF_DESCENDANTS);
    EXPECT_EQ(callback.mockFocusMoveCallbackCount_, 1);
    EXPECT_EQ(callback.mockFocusMoveResult_.resultType, Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE);
    EXPECT_EQ(callback.mockFocusMoveResult_.nowLevelBelongTreeId, TREE_ID);
}

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
HWTEST_F(JsAccessibilityManagerCmpTest, RegistrationFailurePreservesCallbacks, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1001, 1002);
    auto root = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto callback = std::make_shared<FakeChildTreeCallback>(root->GetAccessibilityId());
    mainAM->RegisterAccessibilityChildTreeCallback(root->GetAccessibilityId(), callback);
    ASSERT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 123, root.GetRawPtr()));
    auto key = std::make_pair(1001, static_cast<int64_t>(123));
    auto subAM = mainAM->cmpBindingMap_[key].subAM;
    auto registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(321);
    mainAM->cmpBindingMap_[key].state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
    mainAM->cmpBindingMap_[key].registration = registration;
    EXPECT_FALSE(mainAM->CompleteCmpRegistration(key, subAM, registration, false));
    registration->Complete(false);
    EXPECT_EQ(mainAM->cmpBindingMap_[key].state, JsAccessibilityManager::CmpBindingState::State::WAIT_TREE_ID);
    EXPECT_EQ(mainAM->cmpBindingMap_[key].windowId, 0u);
    EXPECT_EQ(mainAM->childTreeCallbackMap_[root->GetAccessibilityId()], callback);
    EXPECT_EQ(callback->deregisterCount, 0);
    EXPECT_TRUE(subAM->childTreeCallbackMap_.empty());
    EXPECT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 123, root.GetRawPtr()));
    EXPECT_EQ(mainAM->cmpBindingMap_[key].subAM, subAM);
    mainAM->UnbindExportTree(owner.GetRawPtr(), 123);
}

HWTEST_F(JsAccessibilityManagerCmpTest, UnbindDuringRegistrationCleansLateTreeId, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1011, 1012);
    auto root = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 124, root.GetRawPtr()));
    auto key = std::make_pair(1011, static_cast<int64_t>(124));
    auto subAM = mainAM->cmpBindingMap_[key].subAM;
    auto registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(654);
    mainAM->cmpBindingMap_[key].state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
    mainAM->cmpBindingMap_[key].registration = registration;
    mainAM->UnbindExportTree(owner.GetRawPtr(), 124);
    EXPECT_TRUE(registration->IsCanceled());
    EXPECT_FALSE(registration->cleaned);
    EXPECT_FALSE(mainAM->CompleteCmpRegistration(key, subAM, registration, true));
    registration->Complete(false);
    registration->SetTreeId(0);
    EXPECT_FALSE(registration->cleaned);
    registration->SetTreeId(43);
    EXPECT_TRUE(registration->cleaned);
    EXPECT_EQ(registration->windowId, 654u);
    mainAM->MarkCmpBindingRegistered(key, subAM, registration);
    EXPECT_FALSE(subAM->IsRegister());
    EXPECT_EQ(mainAM->cmpBindingMap_.count(key), 0u);
}

HWTEST_F(JsAccessibilityManagerCmpTest, OldRegistrationCannotActivateReboundKey, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(1021, 1022);
    auto root = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    ASSERT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 125, root.GetRawPtr()));
    auto key = std::make_pair(1021, static_cast<int64_t>(125));
    auto oldSubAM = mainAM->cmpBindingMap_[key].subAM;
    auto oldRegistration = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
    oldRegistration->SetTreeId(44);
    oldRegistration->Complete(true);
    mainAM->UnbindExportTree(owner.GetRawPtr(), 125);
    ASSERT_TRUE(mainAM->BindExportTree(owner.GetRawPtr(), 125, root.GetRawPtr()));
    auto newSubAM = mainAM->cmpBindingMap_[key].subAM;
    mainAM->cmpBindingMap_[key].state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
    mainAM->cmpBindingMap_[key].registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
    mainAM->MarkCmpBindingRegistered(key, oldSubAM, oldRegistration);
    EXPECT_EQ(mainAM->cmpBindingMap_[key].state, JsAccessibilityManager::CmpBindingState::State::REGISTERING);
    EXPECT_FALSE(newSubAM->IsRegister());
    EXPECT_TRUE(oldRegistration->IsCanceled());
    auto staleRetry = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
    staleRetry->SetTreeId(45);
    staleRetry->Complete(true);
    mainAM->MarkCmpBindingRegistered(key, newSubAM, staleRetry);
    EXPECT_TRUE(staleRetry->IsCanceled());
    EXPECT_FALSE(newSubAM->IsRegister());
    mainAM->UnbindExportTree(owner.GetRawPtr(), 125);
}

HWTEST_F(JsAccessibilityManagerCmpTest, ChildTreeFocusExemptionOnlyForExportedSearch, TestSize.Level1)
{
    AccessibilityElementInfo parent;
    parent.SetAccessibilityId(1);
    Accessibility::Rect zeroRect { 0, 0, 0, 0 };
    Accessibility::Rect visibleRect { 0, 0, 10, 10 };
    parent.SetRectInScreen(zeroRect);
    AccessibilityElementInfo web;
    web.SetAccessibilityId(2);
    web.SetParent(1);
    web.SetChildTreeIdAndWinId(99, 1);
    web.SetRectInScreen(visibleRect);
    AccessibilityElementInfo zeroWeb;
    zeroWeb.SetAccessibilityId(3);
    zeroWeb.SetChildTreeIdAndWinId(100, 1);
    zeroWeb.SetRectInScreen(zeroRect);
    std::list<AccessibilityElementInfo> normal { parent, web, zeroWeb };
    auto exported = normal;
    AccessibilityElementInfo focus;
    FilterAccessibilityElementByFocusable(normal, focus, 2, false);
    EXPECT_TRUE(normal.empty());
    FilterAccessibilityElementByFocusable(exported, focus, 2, true);
    ASSERT_EQ(exported.size(), 2u);
    EXPECT_EQ(focus.GetAccessibilityId(), 2);
}

HWTEST_F(JsAccessibilityManagerCmpTest, FocusMoveSearchWaitsForUI, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    auto originalExecutor = context->taskExecutor_;
    auto executor = AceType::MakeRefPtr<QueuedUITaskExecutor>();
    context->taskExecutor_ = executor;
    auto root = FrameNode::CreateFrameNode("exported_root", 1032, AceType::MakeRefPtr<Pattern>());
    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    subAM->SetPipelineContext(context);
    auto operation = std::make_shared<JsAccessibilityManager::CmpInteractionOperation>(
        1, 126, root, 1031, WeakPtr<JsAccessibilityManager>());
    operation->SetHandler(Referenced::WeakClaim(AceType::RawPtr(subAM)));
    MockAmsElementOperatorCallback callback;
    operation->FocusMoveSearch(-1, static_cast<int32_t>(Accessibility::FocusMoveDirection::FORWARD), 7, callback);
    EXPECT_FALSE(callback.mockLegacyFocusMoveCalled_);
    EXPECT_EQ(executor->uiTasks.size(), 1u);
    for (auto& task : executor->uiTasks) {
        task();
    }
    EXPECT_TRUE(callback.mockLegacyFocusMoveCalled_);
    EXPECT_EQ(callback.mockRequestId, 7);
    context->taskExecutor_ = originalExecutor;
}

HWTEST_F(JsAccessibilityManagerCmpTest, ChildTreeNotificationRunsOutsideMapLock, TestSize.Level1)
{
    auto mainAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto callback = std::make_shared<FakeChildTreeCallback>(88);
    bool called = false;
    callback->onSetChildTree = [&](int32_t windowId, int32_t treeId) {
        called = true;
        EXPECT_EQ(windowId, 456);
        EXPECT_EQ(treeId, 89);
        bool unlocked = mainAM->childTreeCallbackMapMutex_.try_lock();
        EXPECT_TRUE(unlocked);
        if (unlocked) {
            mainAM->childTreeCallbackMapMutex_.unlock();
        }
    };
    mainAM->RegisterAccessibilityChildTreeCallback(88, callback);
    mainAM->NotifySetChildTreeIdAndWinId(88, 89, 456);
    EXPECT_TRUE(called);
    EXPECT_EQ(callback->GetChildTreeId(), 89);
    mainAM->DeregisterAccessibilityChildTreeCallback(88);
}

HWTEST_F(JsAccessibilityManagerCmpTest, HoverTransferWaitsForUI, TestSize.Level1)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    auto originalExecutor = context->taskExecutor_;
    auto executor = AceType::MakeRefPtr<QueuedUITaskExecutor>();
    context->taskExecutor_ = executor;
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    AccessibilityHoverForThirdConfig config;
    config.context = context;
    config.mountBindIds = { 123 };
    manager->TransferHoverToExportedSubtreeIfNeeded(123, config);
    EXPECT_EQ(executor->uiTasks.size(), 1u);
    for (auto& task : executor->uiTasks) {
        task();
    }
    context->taskExecutor_ = originalExecutor;
}

HWTEST_F(JsAccessibilityManagerCmpTest, RebindRestoresDormantCallbacks, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(2011, 2012);
    auto root = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    auto callback = std::make_shared<FakeChildTreeCallback>(root->GetAccessibilityId());
    manager->RegisterAccessibilityChildTreeCallback(root->GetAccessibilityId(), callback);
    const auto key = std::make_pair(owner->GetId(), int64_t(1));
    auto complete = [&](int32_t treeId) {
        auto& state = manager->cmpBindingMap_.at(key);
        state.state = JsAccessibilityManager::CmpBindingState::State::REGISTERING;
        state.registration = std::make_shared<JsAccessibilityManager::CmpRegistration>(123);
        state.registration->SetTreeId(treeId);
        state.registration->Complete(true);
        manager->MarkCmpBindingRegistered(key, state.subAM, state.registration);
    };
    ASSERT_TRUE(manager->BindExportTree(owner.GetRawPtr(), 1, root.GetRawPtr()));
    complete(51);
    EXPECT_EQ(callback->registerCount, 1);
    WeakPtr<JsAccessibilityManager> first = manager->cmpBindingMap_.at(key).subAM;
    manager->UnbindExportTree(owner.GetRawPtr(), 1);
    EXPECT_EQ(first.Upgrade(), nullptr);
    EXPECT_TRUE(manager->childTreeCallbackMap_.empty());
    EXPECT_EQ(callback->GetChildTreeId(), 0);
    manager->NotifyChildTreeOnRegister(99);
    EXPECT_EQ(callback->registerCount, 1);
    ASSERT_TRUE(manager->BindExportTree(owner.GetRawPtr(), 1, root.GetRawPtr()));
    complete(52);
    EXPECT_EQ(callback->registerCount, 2);
    EXPECT_EQ(manager->cmpBindingMap_.at(key).subAM->childTreeCallbackMap_.at(root->GetAccessibilityId()), callback);
    EXPECT_TRUE(manager->detachedCmpChildTreeCallbacks_.empty());
    manager->UnbindExportTree(owner.GetRawPtr(), 1);
    manager->DeregisterAccessibilityChildTreeCallback(root->GetAccessibilityId());
    EXPECT_TRUE(manager->detachedCmpChildTreeCallbacks_.empty());
}

HWTEST_F(JsAccessibilityManagerCmpTest, SameBindIdIsIsolatedByOwner, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto ownerA = CreateAdoptedPair(2021, 2022);
    auto ownerB = CreateAdoptedPair(2023, 2024);
    auto rootA = AceType::DynamicCast<FrameNode>(ownerA->GetChildAtIndex(0));
    auto rootB = AceType::DynamicCast<FrameNode>(ownerB->GetChildAtIndex(0));
    ASSERT_TRUE(manager->BindExportTree(ownerA.GetRawPtr(), 1, rootA.GetRawPtr()));
    ASSERT_TRUE(manager->BindExportTree(ownerB.GetRawPtr(), 1, rootB.GetRawPtr()));
    for (auto& [key, state] : manager->cmpBindingMap_) {
        state.state = JsAccessibilityManager::CmpBindingState::State::REGISTERED;
    }
    ASSERT_TRUE(manager->GetCmpBindingForHover(ownerA->GetId(), 1));
    ASSERT_TRUE(manager->GetCmpBindingForHover(ownerB->GetId(), 1));
    EXPECT_EQ(manager->GetCmpBindingForHover(ownerA->GetId(), 1)->exportedRoot.Upgrade(), rootA);
    EXPECT_EQ(manager->GetCmpBindingForHover(ownerB->GetId(), 1)->exportedRoot.Upgrade(), rootB);
    EXPECT_EQ(manager->GetCmpBindingBindIdsForHover(ownerB->GetId()), std::vector<int64_t>({1}));
    EXPECT_TRUE(manager->GetCmpBindingBindIdsForHover(9999).empty());
    manager->UnbindExportTree(ownerA.GetRawPtr(), 1);
    EXPECT_FALSE(manager->GetCmpBindingForHover(ownerA->GetId(), 1));
    EXPECT_TRUE(manager->GetCmpBindingForHover(ownerB->GetId(), 1));
    manager->UnbindExportTree(ownerB.GetRawPtr(), 1);
}

HWTEST_F(JsAccessibilityManagerCmpTest, UnbindClearsOnlyOwnerProviderMount, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto ownerA = CreateAdoptedPair(2031, 2032);
    auto ownerB = CreateAdoptedPair(2033, 2034);
    auto root = AceType::DynamicCast<FrameNode>(ownerA->GetChildAtIndex(0));
    auto provider = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto opA = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, ownerA);
    auto opB = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, ownerB);
    manager->RegisterJsThirdProviderInteractionOperation(ownerA->GetAccessibilityId(), opA);
    manager->RegisterJsThirdProviderInteractionOperation(ownerB->GetAccessibilityId(), opB);
    ASSERT_TRUE(manager->BindExportTree(ownerA.GetRawPtr(), 1, root.GetRawPtr()));
    opA->SetChildTreeIdAndWinId(1, 61, 123);
    opA->SetChildTreeIdAndWinId(2, 62, 123);
    opB->SetChildTreeIdAndWinId(1, 63, 124);
    manager->UnbindExportTree(ownerA.GetRawPtr(), 1);
    EXPECT_FALSE(opA->HasChildTree(1));
    EXPECT_TRUE(opA->HasChildTree(2));
    EXPECT_TRUE(opB->HasChildTree(1));
    std::list<AccessibilityElementInfo> infos;
    ASSERT_TRUE(opA->FindAccessibilityNodeInfosByIdFromProvider(1, 0, 0, infos, true));
    ASSERT_FALSE(infos.empty());
    EXPECT_LE(infos.front().GetChildTreeId(), 0);
    opA->SetChildTreeIdAndWinId(2, 0, 0);
    EXPECT_FALSE(opA->HasChildTree(2));
}

HWTEST_F(JsAccessibilityManagerCmpTest, HoverLeavesOldRootBeforeEnteringNewRoot, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto pipeline = AceType::MakeRefPtr<RecordingHoverPipeline>();
    auto rootA = FrameNode::CreateFrameNode("root_a", 2041, AceType::MakeRefPtr<Pattern>());
    auto rootB = FrameNode::CreateFrameNode("root_b", 2042, AceType::MakeRefPtr<Pattern>());
    AccessibilityHoverForThirdConfig config;
    config.context = pipeline;
    config.sourceType = SourceType::TOUCH;
    config.eventType = NG::AccessibilityHoverEventType::MOVE;
    manager->UpdateHoverTransferOnUI(rootA, {10, 10}, config);
    manager->UpdateHoverTransferOnUI(rootA, {11, 10}, config);
    manager->UpdateHoverTransferOnUI(nullptr, {}, config);
    manager->UpdateHoverTransferOnUI(rootA, {10, 10}, config);
    manager->UpdateHoverTransferOnUI(rootB, {20, 20}, config);
    manager->UpdateHoverTransferOnUI(nullptr, {}, config);
    manager->UpdateHoverTransferOnUI(rootB, {20, 20}, config);
    config.eventType = NG::AccessibilityHoverEventType::EXIT;
    manager->UpdateHoverTransferOnUI(nullptr, {}, config);
    const std::vector<std::pair<int32_t, TouchType>> expected {
        {2041, TouchType::HOVER_ENTER}, {2041, TouchType::HOVER_MOVE}, {2041, TouchType::HOVER_EXIT},
        {2041, TouchType::HOVER_ENTER}, {2041, TouchType::HOVER_EXIT}, {2042, TouchType::HOVER_ENTER},
        {2042, TouchType::HOVER_EXIT}, {2042, TouchType::HOVER_ENTER}, {2042, TouchType::HOVER_EXIT}
    };
    EXPECT_EQ(pipeline->events, expected);
    EXPECT_EQ(manager->nativeHoverRoot_.Upgrade(), nullptr);
}

HWTEST_F(JsAccessibilityManagerCmpTest, MountOffsetUsesRawIdWithNonzeroTreeId, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto owner = CreateAdoptedPair(2051, 2052);
    auto root = AceType::DynamicCast<FrameNode>(owner->GetChildAtIndex(0));
    root->SetIsAdopted(true);
    auto provider = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    provider->providerMockResult_.elementInfosList_[1].SetRect({30, 40, 80, 90});
    auto op = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, owner);
    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    subAM->cmpExportedRoot_ = root;
    subAM->cmpProviderOperation_ = op;
    subAM->parentElementId_ = 1;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(77, subAM->parentElementId_);
    float x = 0;
    float y = 0;
    ASSERT_TRUE(subAM->GetExportedRootWindowOffset(x, y));
    EXPECT_EQ(provider->providerMockResult_.elementId_, 1);
    auto hostRect = owner->GetTransformRectRelativeToWindow();
    EXPECT_FLOAT_EQ(x, hostRect.Left() + 30);
    EXPECT_FLOAT_EQ(y, hostRect.Top() + 40);
}

HWTEST_F(JsAccessibilityManagerCmpTest, HoverSameMountIdRoutesCurrentOwner, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto pipeline = AceType::MakeRefPtr<RecordingHoverPipeline>();
    auto ownerA = CreateAdoptedPair(2061, 2062);
    auto ownerB = CreateAdoptedPair(2063, 2064);
    auto rootA = AceType::DynamicCast<FrameNode>(ownerA->GetChildAtIndex(0));
    auto rootB = AceType::DynamicCast<FrameNode>(ownerB->GetChildAtIndex(0));
    auto provider = AceType::MakeRefPtr<MockOhAccessibilityProvider>();
    auto opA = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, ownerA);
    auto opB = std::make_shared<JsThirdProviderInteractionOperation>(provider, manager, ownerB);
    manager->RegisterJsThirdProviderInteractionOperation(ownerA->GetAccessibilityId(), opA);
    manager->RegisterJsThirdProviderInteractionOperation(ownerB->GetAccessibilityId(), opB);
    ASSERT_TRUE(manager->BindExportTree(ownerA.GetRawPtr(), 1, rootA.GetRawPtr()));
    ASSERT_TRUE(manager->BindExportTree(ownerB.GetRawPtr(), 1, rootB.GetRawPtr()));
    for (auto& [key, state] : manager->cmpBindingMap_) {
        state.state = JsAccessibilityManager::CmpBindingState::State::REGISTERED;
    }
    AccessibilityHoverForThirdConfig config;
    config.context = pipeline;
    config.eventType = NG::AccessibilityHoverEventType::MOVE;
    config.hostElementId = ownerA->GetAccessibilityId();
    config.ownerNodeId = ownerA->GetId();
    manager->TransferHoverToExportedSubtreeOnUI(1, config);
    config.hostElementId = ownerB->GetAccessibilityId();
    config.ownerNodeId = ownerB->GetId();
    manager->TransferHoverToExportedSubtreeOnUI(1, config);
    manager->TransferHoverToExportedSubtreeOnUI(9, config);
    manager->TransferHoverToExportedSubtreeOnUI(1, config);
    manager->TransferHoverToExportedSubtreeOnUI(-1, config);
    const std::vector<std::pair<int32_t, TouchType>> expected {
        {2062, TouchType::HOVER_ENTER}, {2062, TouchType::HOVER_EXIT}, {2064, TouchType::HOVER_ENTER},
        {2064, TouchType::HOVER_EXIT}, {2064, TouchType::HOVER_ENTER}, {2064, TouchType::HOVER_EXIT}
    };
    EXPECT_EQ(pipeline->events, expected);
    manager->UnbindExportTree(ownerA.GetRawPtr(), 1);
    manager->UnbindExportTree(ownerB.GetRawPtr(), 1);
}


} // namespace OHOS::Ace::Framework

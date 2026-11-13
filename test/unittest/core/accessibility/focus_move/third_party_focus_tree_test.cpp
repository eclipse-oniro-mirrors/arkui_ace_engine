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

#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "accessibility_system_ability_client.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/log/dump_log.h"
#include "adapter/ohos/osal/js_accessibility_manager.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal_third.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_third_node_utils.h"
#include "adapter/ohos/osal/accessibility/third_accessibility_traversal.h"
#include "adapter/ohos/osal/js_third_provider_interaction_operation.h"
#include "test/unittest/core/accessibility/js_third_provider_interaction_operation_test.h"

using namespace OHOS::Accessibility;
using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
namespace {
constexpr int64_t TEST_ROOT_ID = 100;
constexpr int64_t TEST_FIRST_NODE_ID = 101;
constexpr int64_t TEST_SECOND_NODE_ID = 102;
constexpr int64_t TEST_THIRD_NODE_ID = 103;
constexpr int64_t TEST_EXTRA_NODE_ID_BASE = 1000;

// Provider stub for the third focus-tree assembly: a node table queried per id, with an
// optional recursive mode that answers -1 with the whole table (minus hidden holes).
class FocusTreeProvider : public MockOhAccessibilityProvider {
    DECLARE_ACE_TYPE(FocusTreeProvider, MockOhAccessibilityProvider);
public:
    std::map<int64_t, ArkUI_AccessibilityElementInfo> nodes;
    std::map<int64_t, size_t> queries;
    std::set<int64_t> emptyIds;
    // Answer -1 with the whole tree (recursive providers).
    bool recursive = false;
    // Ids omitted from the recursive answer (holes).
    std::set<int64_t> recursiveHidden;
    // Reject every prefetch mode on the root query (strict legacy providers).
    bool rejectRecursive = false;
    int32_t FindAccessibilityNodeInfosById(int64_t id, int32_t mode, int32_t requestId,
        std::vector<ArkUI_AccessibilityElementInfo>& infos) override
    {
        ++queries[id];
        if (emptyIds.count(id)) {
            return 0;
        }
        if (id == -1 && rejectRecursive && mode != 0) {
            return -1;
        }
        if (id == -1 && recursive) {
            for (const auto& [nodeId, node] : nodes) {
                if (!recursiveHidden.count(nodeId)) {
                    infos.push_back(node);
                }
            }
            return 0;
        }
        auto it = nodes.find(id == -1 ? TEST_ROOT_ID : id);
        if (it == nodes.end()) {
            return -1;
        }
        infos.push_back(it->second);
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
} // namespace

class ThirdFocusTreeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ThirdFocusTreeTest::SetUpTestCase()
{
    NG::MockPipelineContext::SetUp();
    MockContainer::SetUp();

    std::unique_ptr<std::ostream> ostream = std::make_unique<std::ostringstream>();
    DumpLog::GetInstance().SetDumpFile(std::move(ostream));
}

void ThirdFocusTreeTest::TearDownTestCase()
{
    NG::MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

// Form A: a complete recursive answer assembles the whole tree with a single provider callback.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeFlatHappyPathSingleQuery, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_THIRD_NODE_ID, TEST_FIRST_NODE_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_THIRD_NODE_ID));
    EXPECT_EQ(provider->queries.size(), 1);
    EXPECT_EQ(provider->queries.count(-1), 1);
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), TEST_ROOT_ID);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[0], nullptr);
    ASSERT_NE(children[1], nullptr);
    EXPECT_EQ(children[0]->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    EXPECT_EQ(children[1]->GetAccessibilityId(), TEST_SECOND_NODE_ID);
}

// a non-empty recursive answer without any root marker fails the focus search.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeNoMarkerFailsSearch, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, -1, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
    EXPECT_EQ(strategy.ChangeToRoot(), nullptr);
}

// an empty root answer is not a provider anomaly (for example an empty page);
// the focus search fails silently.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeEmptyAnswerFailsSilently, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->emptyIds.insert(-1);
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(-1));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
    EXPECT_EQ(strategy.ChangeToRoot(), nullptr);
}

// A provider that rejects the recursive prefetch mode keeps the legacy contract: the root
// query retries once without a prefetch mode, then the per-node discovery builds the tree.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeRecursiveModeRejectedFallsBack, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->rejectRecursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    // One rejected recursive attempt plus one legacy retry.
    EXPECT_EQ(provider->queries.at(-1), 2);
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_ROOT_ID);
}

// A single-record answer keeps the legacy root contract: the record returned for the -1
// query is the root, with no marker required (the tree is a root only).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeSingleRecordKeepsLegacyRoot, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, -1, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_ROOT_ID));
    EXPECT_EQ(provider->queries.size(), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAccessibilityId(), TEST_ROOT_ID);
}

// Form B: a child referenced by the answer but missing from it is fetched per node
// (1 + H provider callbacks).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeHoleFilledByQuery, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->recursiveHidden = {TEST_SECOND_NODE_ID};
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_SECOND_NODE_ID));
    EXPECT_EQ(provider->queries.at(-1), 1);
    EXPECT_EQ(provider->queries.at(TEST_SECOND_NODE_ID), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[1], nullptr);
    EXPECT_EQ(children[1]->GetAccessibilityId(), TEST_SECOND_NODE_ID);
}

// Form B: a massive dangling child list exhausts the chase budget; the tree is rejected
// after at most MAX_NODES hole chases, bounding the provider callback count.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeHoleChaseBudgetExceeded, TestSize.Level1)
{
    constexpr int64_t danglingIdBase = 200000;
    constexpr size_t danglingCount = 45000;
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    for (size_t i = 0; i < danglingCount; ++i) {
        provider->nodes[TEST_ROOT_ID].AddChildNodeId(danglingIdBase + static_cast<int64_t>(i));
    }
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
    // 1 root query + exactly MAX_NODES chases: the budget stops the loop before the
    // remaining dangling references reach the provider.
    size_t totalQueries = 0;
    for (const auto& entry : provider->queries) {
        totalQueries += entry.second;
    }
    EXPECT_EQ(totalQueries, 1 + ThirdAccessibilityTraversal::MAX_NODES);
}

// Form B: a dangling reference whose per-node query fails is pruned; the healthy part
// of the tree stays usable (failed chases never reject the tree).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeHoleQueryFailedPruned, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_EXTRA_NODE_ID_BASE});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    // TEST_EXTRA_NODE_ID_BASE is absent from the node table: the chase query fails.
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(provider->queries.at(TEST_EXTRA_NODE_ID_BASE), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAceChildren().size(), 1);
}

// Form B: a hole-fetched record is charged once; a hole payload around half the byte
// budget must still assemble (the removed double charging would have hit PAYLOAD_LIMIT).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeHolePayloadSingleCharge, TestSize.Level1)
{
    constexpr size_t holePayload = ThirdAccessibilityTraversal::MAX_BYTES / 2 + 512 * 1024;
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->recursiveHidden = {TEST_SECOND_NODE_ID};
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    provider->nodes[TEST_SECOND_NODE_ID].SetContents(std::string(holePayload, 'a'));
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_SECOND_NODE_ID));
    EXPECT_EQ(provider->queries.at(TEST_SECOND_NODE_ID), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAceChildren().size(), 2);
}

// Form C: per-node discovery charges each non-root record once; a tree whose total
// payload is around half the byte budget must still assemble.
HWTEST_F(ThirdFocusTreeTest, ThirdTreePerNodePayloadSingleCharge, TestSize.Level1)
{
    constexpr size_t childPayload = ThirdAccessibilityTraversal::MAX_BYTES / 4 + 256 * 1024;
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    // The recursive answer is the root record alone: the provider ignored the
    // recursive mode, discovery goes per node.
    provider->recursiveHidden = {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID};
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    provider->nodes[TEST_FIRST_NODE_ID].SetContents(std::string(childPayload, 'a'));
    provider->nodes[TEST_SECOND_NODE_ID].SetContents(std::string(childPayload, 'a'));
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(provider->queries.at(-1), 1);
    EXPECT_EQ(provider->queries.at(TEST_FIRST_NODE_ID), 1);
    EXPECT_EQ(provider->queries.at(TEST_SECOND_NODE_ID), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAceChildren().size(), 2);
}

// records that only declare a parent (nobody lists them as a child) are adopted
// in receive order and appended at the children tail.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeParentOnlyList, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[0], nullptr);
    ASSERT_NE(children[1], nullptr);
    EXPECT_EQ(children[0]->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    EXPECT_EQ(children[1]->GetAccessibilityId(), TEST_SECOND_NODE_ID);
}

// a node claimed by several parents without a usable declaration keeps the first claimer.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeMultiParentKeepsFirstClaimer, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_THIRD_NODE_ID, -1, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_THIRD_NODE_ID));
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[1], nullptr);
    EXPECT_TRUE(children[1]->GetAceChildren().empty());
}

// nodes that would close a cycle are dropped; the healthy part of the tree stays usable.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeCycleDropped, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    // Root claims only the healthy child; the other two claim each other.
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_THIRD_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_FIRST_NODE_ID, -1, {TEST_SECOND_NODE_ID});
    provider->Add(TEST_SECOND_NODE_ID, -1, {TEST_FIRST_NODE_ID});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_ROOT_ID));
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(TEST_SECOND_NODE_ID));
    // The healthy part of the tree stays usable.
    EXPECT_TRUE(strategy.UpdateOriginNodeInfo(TEST_THIRD_NODE_ID));
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 1);
    ASSERT_NE(children.front(), nullptr);
    EXPECT_EQ(children.front()->GetAccessibilityId(), TEST_THIRD_NODE_ID);
}

// a record with neither a parent declaration nor a claimer is dropped; the tree stays usable.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeOrphanDropped, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    // No declaration, nobody claims it.
    provider->Add(TEST_SECOND_NODE_ID, -1, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_FALSE(strategy.UpdateOriginNodeInfo(TEST_SECOND_NODE_ID));
    EXPECT_EQ(strategy.GetCurrentCheckNode(), nullptr);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 1);
    ASSERT_NE(children.front(), nullptr);
    EXPECT_EQ(children.front()->GetAccessibilityId(), TEST_FIRST_NODE_ID);
}

// a record with an invalid element id and an invalid child reference are ignored;
// the healthy records still assemble.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeInvalidRecordIgnored, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_EXTRA_NODE_ID_BASE, -1, {});
    // Invalid element id.
    provider->nodes[TEST_EXTRA_NODE_ID_BASE].SetElementId(-5);
    // Invalid child reference.
    provider->nodes[TEST_ROOT_ID].AddChildNodeId(-7);
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    // The root recursive query only.
    EXPECT_EQ(provider->queries.size(), 1);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 1);
    ASSERT_NE(children.front(), nullptr);
    EXPECT_EQ(children.front()->GetAccessibilityId(), TEST_FIRST_NODE_ID);
}

// duplicate element ids in one answer keep the first record (receive order).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeDuplicateIdFirstWins, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {});
    provider->nodes[TEST_FIRST_NODE_ID].SetContents("first");
    provider->Add(TEST_EXTRA_NODE_ID_BASE, TEST_ROOT_ID, {});
    provider->nodes[TEST_EXTRA_NODE_ID_BASE].SetElementId(TEST_FIRST_NODE_ID);
    provider->nodes[TEST_EXTRA_NODE_ID_BASE].SetContents("second");
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    auto node = std::static_pointer_cast<ThirdRulesCheckNode>(strategy.GetCurrentCheckNode());
    ASSERT_NE(node, nullptr);
    ASSERT_NE(node->GetNodeInfo(), nullptr);
    EXPECT_EQ(node->GetNodeInfo()->GetContents(), "first");
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetAceChildren().size(), 1);
}

// a parentId reference to a record missing from the answer is fetched per-node
// (only references greater than 0 are chased).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeParentHoleFilledByQuery, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->recursiveHidden = {TEST_SECOND_NODE_ID};
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {});
    provider->Add(TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID, {});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {TEST_FIRST_NODE_ID});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(provider->queries.at(-1), 1);
    // The parentId hole fill.
    EXPECT_EQ(provider->queries.at(TEST_SECOND_NODE_ID), 1);
    EXPECT_EQ(provider->queries.count(TEST_FIRST_NODE_ID), 0);
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_SECOND_NODE_ID);
    auto grandparent = parent->GetAceParent();
    ASSERT_NE(grandparent, nullptr);
    EXPECT_EQ(grandparent->GetAccessibilityId(), TEST_ROOT_ID);
}

// parentId=0 is the struct default; it is never chased as a hole.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeParentZeroNotChased, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, 0, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_FIRST_NODE_ID));
    EXPECT_EQ(provider->queries.count(0), 0);
    EXPECT_EQ(provider->queries.size(), 1);
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_ROOT_ID);
}

// Missing-declaration repairs and receive-order adoptions merge in one answer; claimed
// children keep childIds order and the adopted node is appended at the tail.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeMixedSources, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID, {TEST_FIRST_NODE_ID});
    // Missing declaration, claimed by root.
    provider->Add(TEST_FIRST_NODE_ID, -1, {});
    // Declared root, unclaimed.
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_SECOND_NODE_ID));
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[0], nullptr);
    ASSERT_NE(children[1], nullptr);
    // Claimed: childIds order.
    EXPECT_EQ(children[0]->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    // Adopted: receive order.
    EXPECT_EQ(children[1]->GetAccessibilityId(), TEST_SECOND_NODE_ID);
}

// a declaration that disagrees with the claimer side keeps the claimer
// (childNodeIds is the authoritative source).
HWTEST_F(ThirdFocusTreeTest, ThirdTreeConflictKeepsClaimer, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {});
    provider->Add(TEST_THIRD_NODE_ID, TEST_SECOND_NODE_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_THIRD_NODE_ID));
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    // Claimer wins over declaration.
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_FIRST_NODE_ID);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[0], nullptr);
    ASSERT_NE(children[1], nullptr);
    // The claimer keeps the node.
    EXPECT_EQ(children[0]->GetAceChildren().size(), 1);
    // The declared parent never adopted it.
    EXPECT_TRUE(children[1]->GetAceChildren().empty());
}

// several claimers with a self-declared parent keep the declaration.
HWTEST_F(ThirdFocusTreeTest, ThirdTreeMultiParentSelfDeclared, TestSize.Level1)
{
    auto provider = AceType::MakeRefPtr<FocusTreeProvider>();
    provider->recursive = true;
    provider->Add(TEST_ROOT_ID, AccessibilityElementInfo::ROOT_PARENT_ID,
        {TEST_FIRST_NODE_ID, TEST_SECOND_NODE_ID});
    provider->Add(TEST_FIRST_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_SECOND_NODE_ID, TEST_ROOT_ID, {TEST_THIRD_NODE_ID});
    provider->Add(TEST_THIRD_NODE_ID, TEST_SECOND_NODE_ID, {});
    auto manager = AceType::MakeRefPtr<JsAccessibilityManager>();
    FocusStrategyOsalThird strategy(manager, provider, NodeConfig{});
    ASSERT_TRUE(strategy.UpdateOriginNodeInfo(TEST_THIRD_NODE_ID));
    auto node = strategy.GetCurrentCheckNode();
    ASSERT_NE(node, nullptr);
    auto parent = node->GetAceParent();
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->GetAccessibilityId(), TEST_SECOND_NODE_ID);
    auto root = strategy.ChangeToRoot();
    ASSERT_NE(root, nullptr);
    auto children = root->GetAceChildren();
    ASSERT_EQ(children.size(), 2);
    ASSERT_NE(children[0], nullptr);
    ASSERT_NE(children[1], nullptr);
    // The other claimer's edge is pruned.
    EXPECT_TRUE(children[0]->GetAceChildren().empty());
    // The self-declared parent keeps the node.
    EXPECT_EQ(children[1]->GetAceChildren().size(), 1);
}

} // namespace OHOS::Ace::Framework

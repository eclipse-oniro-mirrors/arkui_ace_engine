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

#include <atomic>

#include "gtest/gtest.h"
#include "core/components_ng/base/native_node_tree_scope.h"
#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DISABLED = 0;
constexpr int32_t CRASH = 2;
constexpr int32_t TEST_NODE_DEPTH = 8;
// Models the public API boundary: only operands and explicitly prepared roots.
class PublicCall {
public:
    PublicCall(std::initializer_list<UINode*> nodes, const char* apiName)
        : scope_(nodes, apiName) {}
private:
    NativeNodeTreeScope scope_;
};

class NativeTreeTestNode : public UINode {
    DECLARE_ACE_TYPE(NativeTreeTestNode, UINode);
public:
    explicit NativeTreeTestNode(int32_t id) : UINode("NativeTreeTest", id) {}
    bool IsAtomicNode() const override { return false; }
};

RefPtr<NativeTreeTestNode> MakeNode()
{
    static std::atomic<int32_t> nextId { 2200000 };
    auto node = AceType::MakeRefPtr<NativeTreeTestNode>(nextId.fetch_add(1));
    node->RegisterReleaseFunc(false);
    node->SetIsFree(true);
    return node;
}

} // namespace

class NativeNodeTreeScopeTest : public testing::Test {
public:
    void SetUp() override { NativeNodeTreeScope::SetCheckMode(CRASH); }
    void TearDown() override { NativeNodeTreeScope::SetCheckMode(-1); }
};

TEST_F(NativeNodeTreeScopeTest, DisabledCallsDoNotRetainNodesOrTakeProtocolLocks)
{
    NativeNodeTreeScope::SetCheckMode(DISABLED);
    EXPECT_FALSE(NativeNodeTreeScope::NeedsSynchronization());
    auto node = MakeNode();
    WeakPtr<UINode> weak = node;
    {
        PublicCall call({ node.GetRawPtr() }, "disabled");
        node.Reset();
        EXPECT_TRUE(weak.Invalid());
        EXPECT_FALSE(NativeNodeTreeScope::NeedsSynchronization());
    }
}

TEST_F(NativeNodeTreeScopeTest, DisabledParentReleasePreservesOriginalChildDepth)
{
    NativeNodeTreeScope::SetCheckMode(DISABLED);
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->SetDepth(TEST_NODE_DEPTH);
    parent->AddChild(child);
    const auto depth = child->GetDepth();
    parent.Reset();
    EXPECT_EQ(child->GetDepth(), depth);
    EXPECT_FALSE(child->GetParent());
}

TEST_F(NativeNodeTreeScopeTest, AncestorsAreRetainedOnlyUntilOutermostReturn)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    WeakPtr<UINode> weak = parent;
    {
        PublicCall call({ child.GetRawPtr() }, "hold.ancestor");
        parent.Reset();
        EXPECT_FALSE(weak.Invalid());
    }
    EXPECT_TRUE(weak.Invalid());
}

TEST_F(NativeNodeTreeScopeTest, UnrelatedNodesAreNotRetainedForTheWholeCall)
{
    auto parent = MakeNode();
    auto unrelated = MakeNode();
    WeakPtr<UINode> weak = unrelated;
    PublicCall outer({ parent.GetRawPtr() }, "outer");
    unrelated.Reset();
    EXPECT_TRUE(weak.Invalid());
}

TEST_F(NativeNodeTreeScopeTest, OwnerIdsLiveOnAnnouncedNodesAndClearAtOutermostExit)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    auto untouched = MakeNode();
    parent->AddChild(child);
    EXPECT_EQ(parent->GetNativeTreeOperationThreadId(), 0u);
    EXPECT_EQ(child->GetNativeTreeOperationThreadId(), 0u);
    {
        PublicCall outer({ parent.GetRawPtr() }, "outer");
        const auto owner = parent->GetNativeTreeOperationThreadId();
        EXPECT_NE(owner, 0u);
        EXPECT_EQ(untouched->GetNativeTreeOperationThreadId(), 0u);
        { PublicCall inner({ child.GetRawPtr() }, "inner"); }
        EXPECT_EQ(child->GetNativeTreeOperationThreadId(), owner);
    }
    EXPECT_EQ(parent->GetNativeTreeOperationThreadId(), 0u);
    EXPECT_EQ(child->GetNativeTreeOperationThreadId(), 0u);
}

TEST_F(NativeNodeTreeScopeTest, OwnMarkedSubtreeDoesNotReplaceWalkingTheActualParentChain)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    auto descendant = MakeNode();
    child->AddChild(descendant);
    {
        PublicCall outer({ child.GetRawPtr() }, "outer");
        const auto owner = child->GetNativeTreeOperationThreadId();
        parent->AddChild(child);
        EXPECT_EQ(parent->GetNativeTreeOperationThreadId(), 0u);
        { PublicCall lookup({ descendant.GetRawPtr() }, "walk.actual.parents"); }
        // A cached child-root lookup would have stopped at child and missed parent.
        EXPECT_EQ(parent->GetNativeTreeOperationThreadId(), owner);
    }
    EXPECT_EQ(parent->GetNativeTreeOperationThreadId(), 0u);
}

} // namespace OHOS::Ace::NG

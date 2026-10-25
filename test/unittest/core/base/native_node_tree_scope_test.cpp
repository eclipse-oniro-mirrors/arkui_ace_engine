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
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "core/components_ng/base/native_node_tree_scope.h"
#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DISABLED = 0;
constexpr int32_t LOG = 1;
constexpr int32_t CRASH = 2;
constexpr int32_t EXIT_CODE_TIMEOUT = 2;
constexpr int32_t INDEPENDENT_TREE_ITERATIONS = 1000;
constexpr int32_t TEST_NODE_DEPTH = 8;
constexpr int32_t REPARENT_LOOKUP_ITERATIONS = 32;
constexpr auto TIMEOUT = std::chrono::seconds(3);
// Models the public API boundary: only operands and explicitly prepared roots.
class PublicCall {
public:
    PublicCall(std::initializer_list<UINode*> nodes, const char* apiName)
        : scope_(nodes, apiName) {}
    void IncludeNode(UINode* node) { scope_.IncludeNode(node); }
    void PrepareChildren(UINode* parent) { scope_.PrepareChildren(parent); }
private:
    NativeNodeTreeScope scope_;
};


class NativeTreeTestNode : public UINode {
    DECLARE_ACE_TYPE(NativeTreeTestNode, UINode);
public:
    explicit NativeTreeTestNode(int32_t id) : UINode("NativeTreeTest", id) {}
    bool IsAtomicNode() const override { return false; }
    void OnMixedMountChildAdded(const RefPtr<UINode>& child) override
    {
        if (onAdd) {
            onAdd();
        }
    }
    void CacheChildrenForTest() { cached_.splice(cached_.end(), ModifyChildren()); }
    void KeepDetachedReference(const RefPtr<UINode>& child) { ModifyChildren().push_back(child); }
    std::function<void()> onAdd;
private:
    std::list<RefPtr<UINode>> cached_;
};

RefPtr<NativeTreeTestNode> MakeNode()
{
    static std::atomic<int32_t> nextId { 2200000 };
    auto node = AceType::MakeRefPtr<NativeTreeTestNode>(nextId.fetch_add(1));
    node->RegisterReleaseFunc(false);
    node->SetIsFree(true);
    return node;
}

struct Gate {
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;
    bool done = false;
    bool SignalAndWait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        ready = true;
        cv.notify_all();
        return cv.wait_for(lock, TIMEOUT, [this] { return done; });
    }
    bool Wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, TIMEOUT, [this] { return ready; });
    }
    void Finish()
    {
        std::lock_guard<std::mutex> lock(mutex);
        done = true;
        cv.notify_all();
    }
};

// Each death-test child starts its own threads. A timeout fails instead of hanging the test runner.
void WithOwner(UINode* node, const std::function<void()>& operation)
{
    Gate gate;
    std::thread owner([&gate, &node] {
        PublicCall scope({ node }, "owner");
        gate.SignalAndWait();
    });
    if (!gate.Wait()) {
        gate.Finish();
        owner.join();
        std::_Exit(EXIT_CODE_TIMEOUT);
    }
    operation();
    gate.Finish();
    owner.join();
}
} // namespace

class NativeNodeTreeScopeTest : public testing::Test {
public:
    void SetUp() override { NativeNodeTreeScope::SetCheckMode(CRASH); }
    void TearDown() override { NativeNodeTreeScope::SetCheckMode(-1); }
};

TEST_F(NativeNodeTreeScopeTest, IndependentTreesExecuteConcurrently)
{
    auto first = MakeNode();
    auto second = MakeNode();
    WithOwner(first.GetRawPtr(), [&second] {
        for (int i = 0; i < INDEPENDENT_TREE_ITERATIONS; ++i) {
            PublicCall call({ second.GetRawPtr() }, "independent");
        }
    });
}

TEST_F(NativeNodeTreeScopeTest, SameThreadReentryAndDuplicateInputsAreLegal)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    PublicCall outer({ parent.GetRawPtr() }, "outer");
    PublicCall inner({ parent.GetRawPtr(), child.GetRawPtr(), parent.GetRawPtr() }, "callback");
}

TEST_F(NativeNodeTreeScopeTest, SplitChildCanTransferAfterOuterReturn)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    {
        PublicCall outer({ parent.GetRawPtr() }, "removeChild");
        parent->RemoveChild(child);
    }
    WithOwner(parent.GetRawPtr(), [&child] {
        PublicCall call({ child.GetRawPtr() }, "split.after");
    });
}

TEST_F(NativeNodeTreeScopeTest, InternalMutationDoesNotEstablishAnOperation)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    // A real internal callback runs, but no public Native scope has been entered.
    parent->onAdd = [&child] {
        std::thread worker([&child] { PublicCall call({ child.GetRawPtr() }, "public"); });
        worker.join();
    };
    parent->AddChild(child);
}

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

TEST_F(NativeNodeTreeScopeTest, NullInputsAndEarlyReturnsAreBalanced)
{
    { PublicCall empty({ nullptr, nullptr }, nullptr); }
    auto node = MakeNode();
    const auto invoke = [&node] {
        PublicCall call({ node.GetRawPtr() }, "early.return");
        return;
    };
    invoke();
    std::thread worker([&node] { PublicCall call({ node.GetRawPtr() }, "later"); });
    worker.join();
}

TEST_F(NativeNodeTreeScopeTest, DetachedListReferenceDoesNotJoinIndependentTrees)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    parent->RemoveChild(child);
    parent->KeepDetachedReference(child);
    WithOwner(parent.GetRawPtr(), [&child] {
        PublicCall call({ child.GetRawPtr() }, "independent.detached");
    });
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

TEST_F(NativeNodeTreeScopeTest, RemoveAllRootsCanTransferAfterOutermostReturn)
{
    auto parent = MakeNode();
    auto first = MakeNode();
    auto second = MakeNode();
    parent->AddChild(first);
    parent->AddChild(second);
    {
        PublicCall clear({ parent.GetRawPtr() }, "removeAllChildren");
        clear.PrepareChildren(parent.GetRawPtr());
        parent->RemoveChild(first);
        parent->RemoveChild(second);
    }
    WithOwner(first.GetRawPtr(), [&second] { PublicCall call({ second.GetRawPtr() }, "independent"); });
}

TEST_F(NativeNodeTreeScopeTest, LogLookupCanRunWhileAnAnnouncedRootChangesParent)
{
    auto first = MakeNode();
    auto second = MakeNode();
    auto child = MakeNode();
    auto descendant = MakeNode();
    first->AddChild(child);
    child->AddChild(descendant);
    NativeNodeTreeScope::SetCheckMode(LOG);
    Gate gate;
    std::atomic<bool> stop { false };
    std::thread mover([&first, &second, &child, &gate, &stop] {
            PublicCall move({ first.GetRawPtr(), second.GetRawPtr(), child.GetRawPtr() }, "move");
            {
                std::lock_guard<std::mutex> lock(gate.mutex);
                gate.ready = true;
                gate.cv.notify_all();
            }
            while (!stop.load()) {
                child->SetParent(AceType::WeakClaim(first.GetRawPtr()));
                child->SetParent(AceType::WeakClaim(second.GetRawPtr()));
            }
        });
    const bool ready = gate.Wait();
    EXPECT_TRUE(ready);
    if (ready) {
        for (int i = 0; i < REPARENT_LOOKUP_ITERATIONS; ++i) {
            PublicCall call({ descendant.GetRawPtr() }, "lookup.while.reparenting");
        }
    }
    stop.store(true);
    mover.join();
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

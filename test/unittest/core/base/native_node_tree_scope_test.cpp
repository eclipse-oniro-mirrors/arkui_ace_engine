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
constexpr auto TIMEOUT = std::chrono::seconds(3);
std::mutex g_publishedMutex;
std::vector<WeakPtr<UINode>> g_published;

void* CapturePublishedNodes()
{
    if (!NativeNodeTreeScope::NeedsSynchronization()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_publishedMutex);
    std::vector<RefPtr<UINode>> pins;
    std::vector<UINode*> nodes;
    for (const auto& weak : g_published) {
        if (auto node = weak.Upgrade()) {
            nodes.push_back(node.GetRawPtr());
            pins.push_back(std::move(node));
        }
    }
    return NativeNodeTreeScope::CaptureNodes(nodes.data(), nodes.size());
}

// Models the Native bridge's already-published handle snapshot, including cached nodes.
class PublicCall {
public:
    PublicCall(std::initializer_list<UINode*> nodes, const char* apiName)
        : scope_(nodes, apiName, CapturePublishedNodes()) {}
    void IncludeNode(UINode* node) { scope_.IncludeNode(node); }
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
    {
        std::lock_guard<std::mutex> lock(g_publishedMutex);
        g_published.emplace_back(node);
    }
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
    std::thread owner([&] {
        PublicCall scope({ node }, "owner");
        gate.SignalAndWait();
    });
    if (!gate.Wait()) {
        gate.Finish();
        owner.join();
        std::_Exit(2);
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

TEST_F(NativeNodeTreeScopeTest, SameTreeDifferentNodesConflict)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    EXPECT_DEATH(WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ child.GetRawPtr() }, "getAttribute");
    }), "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, SiblingReferenceTreeAlsoConflicts)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    auto sibling = MakeNode();
    EXPECT_DEATH(WithOwner(sibling.GetRawPtr(), [&] {
        PublicCall call({ parent.GetRawPtr(), child.GetRawPtr(), sibling.GetRawPtr() }, "insertChildAfter");
    }), "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, IndependentTreesExecuteConcurrently)
{
    auto first = MakeNode();
    auto second = MakeNode();
    WithOwner(first.GetRawPtr(), [&] {
        for (int i = 0; i < 1000; ++i) {
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

TEST_F(NativeNodeTreeScopeTest, NestedCallRetainsNewTreeUntilOuterReturn)
{
    auto first = MakeNode();
    auto second = MakeNode();
    EXPECT_DEATH({
        PublicCall outer({ first.GetRawPtr() }, "outer");
        { PublicCall inner({ second.GetRawPtr() }, "callback"); }
        std::thread contender([&] { PublicCall call({ second.GetRawPtr() }, "overlap"); });
        contender.join();
    }, "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, SplitChildRemainsClaimedBeforeOuterReturn)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    auto descendant = MakeNode();
    parent->AddChild(child);
    child->AddChild(descendant);
    EXPECT_DEATH({
        PublicCall outer({ parent.GetRawPtr() }, "outer");
        parent->RemoveChild(child);
        std::thread contender([&] { PublicCall call({ descendant.GetRawPtr() }, "split.before"); });
        contender.join();
    }, "MULTI_THREAD");
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
    WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ child.GetRawPtr() }, "split.after");
    });
}

TEST_F(NativeNodeTreeScopeTest, MergeClaimsTargetUntilOuterReturn)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    EXPECT_DEATH({
        PublicCall outer({ parent.GetRawPtr(), child.GetRawPtr() }, "addChild");
        parent->AddChild(child);
        std::thread contender([&] { PublicCall call({ parent.GetRawPtr() }, "merge"); });
        contender.join();
    }, "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, AdoptedNodesShareTheOwnersTree)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    child->SetAdoptParent(AceType::WeakClaim(parent.GetRawPtr()));
    EXPECT_DEATH(WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ child.GetRawPtr() }, "adopted");
    }), "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, InternalMutationDoesNotEstablishAnOperation)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    // A real internal callback runs, but no public Native scope has been entered.
    parent->onAdd = [&] {
        std::thread worker([&] { PublicCall call({ child.GetRawPtr() }, "public"); });
        worker.join();
    };
    parent->AddChild(child);
}

TEST_F(NativeNodeTreeScopeTest, PublicScopeCoversSynchronousCallback)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    EXPECT_DEATH({
        parent->onAdd = [&] {
            std::thread worker([&] { PublicCall call({ child.GetRawPtr() }, "callback.overlap"); });
            worker.join();
        };
        PublicCall call({ parent.GetRawPtr(), child.GetRawPtr() }, "addChild");
        parent->AddChild(child);
    }, "MULTI_THREAD");
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
    parent->SetDepth(8);
    parent->AddChild(child);
    const auto depth = child->GetDepth();
    parent.Reset();
    EXPECT_EQ(child->GetDepth(), depth);
    EXPECT_FALSE(child->GetParent());
}

TEST_F(NativeNodeTreeScopeTest, EnablingFindsNodesCreatedAndReparentedWhileDisabled)
{
    NativeNodeTreeScope::SetCheckMode(DISABLED);
    auto oldParent = MakeNode();
    auto parent = MakeNode();
    auto child = MakeNode();
    oldParent->AddChild(child);
    oldParent->RemoveChild(child);
    parent->AddChild(child);
    NativeNodeTreeScope::SetCheckMode(CRASH);
    EXPECT_DEATH(WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ child.GetRawPtr() }, "enabled.later");
    }), "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, DisableDrainsAnExistingScopeAndTracksItsSplits)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    {
        PublicCall outer({ parent.GetRawPtr() }, "outer");
        NativeNodeTreeScope::SetCheckMode(DISABLED);
        parent->RemoveChild(child);
        EXPECT_TRUE(NativeNodeTreeScope::NeedsSynchronization());
        WithOwner(child.GetRawPtr(), [&] {});
        NativeNodeTreeScope::SetCheckMode(CRASH);
        EXPECT_DEATH({
            std::thread worker([&] { PublicCall call({ child.GetRawPtr() }, "reenabled"); });
            worker.join();
        }, "MULTI_THREAD");
        NativeNodeTreeScope::SetCheckMode(DISABLED);
    }
    EXPECT_FALSE(NativeNodeTreeScope::NeedsSynchronization());
}

TEST_F(NativeNodeTreeScopeTest, LogContenderContinuesAndDoesNotClearOriginalOwner)
{
    auto node = MakeNode();
    NativeNodeTreeScope::SetCheckMode(LOG);
    WithOwner(node.GetRawPtr(), [&] {
        { PublicCall call({ node.GetRawPtr() }, "log.continue"); }
        NativeNodeTreeScope::SetCheckMode(CRASH);
        EXPECT_DEATH({ PublicCall call({ node.GetRawPtr() }, "still.owned"); }, "MULTI_THREAD");
    });
    PublicCall call({ node.GetRawPtr() }, "after.owner");
}

TEST_F(NativeNodeTreeScopeTest, LogPrimaryMayFinishBeforeContender)
{
    auto node = MakeNode();
    NativeNodeTreeScope::SetCheckMode(LOG);
    Gate gate;
    auto primary = std::make_unique<PublicCall>(std::initializer_list<UINode*> { node.GetRawPtr() }, "primary");
    std::thread contender([&] {
        PublicCall call({ node.GetRawPtr() }, "contender");
        gate.SignalAndWait();
    });
    EXPECT_TRUE(gate.Wait());
    primary.reset();
    NativeNodeTreeScope::SetCheckMode(CRASH);
    EXPECT_DEATH({ PublicCall call({ node.GetRawPtr() }, "contender.still.active"); }, "MULTI_THREAD");
    NativeNodeTreeScope::SetCheckMode(DISABLED);
    gate.Finish();
    contender.join();
    EXPECT_FALSE(NativeNodeTreeScope::NeedsSynchronization());
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
    const auto invoke = [&] {
        PublicCall call({ node.GetRawPtr() }, "early.return");
        return;
    };
    invoke();
    std::thread worker([&] { PublicCall call({ node.GetRawPtr() }, "later"); });
    worker.join();
}
TEST_F(NativeNodeTreeScopeTest, CachedMembersOutsideOrdinaryChildrenRemainCovered)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    auto descendant = MakeNode();
    parent->AddChild(child);
    child->AddChild(descendant);
    parent->CacheChildrenForTest();
    EXPECT_DEATH(WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ descendant.GetRawPtr() }, "cached.member");
    }), "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, DetachedListReferenceDoesNotJoinIndependentTrees)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    parent->AddChild(child);
    parent->RemoveChild(child);
    parent->KeepDetachedReference(child);
    WithOwner(parent.GetRawPtr(), [&] {
        PublicCall call({ child.GetRawPtr() }, "independent.detached");
    });
}

TEST_F(NativeNodeTreeScopeTest, ReturnedNewNodeIsIncludedBeforeCallbackCanPublishIt)
{
    auto parent = MakeNode();
    EXPECT_DEATH({
        PublicCall outer({ parent.GetRawPtr() }, "outer");
        auto created = MakeNode();
        outer.IncludeNode(created.GetRawPtr());
        std::thread worker([&] { PublicCall call({ created.GetRawPtr() }, "returned.node"); });
        worker.join();
    }, "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, ReturnedChildExtendsAnExistingSnapshot)
{
    auto parent = MakeNode();
    EXPECT_DEATH({
        PublicCall outer({ parent.GetRawPtr() }, "outer");
        auto created = MakeNode();
        parent->AddChild(created);
        outer.IncludeNode(created.GetRawPtr());
        std::thread worker([&] { PublicCall call({ created.GetRawPtr() }, "returned.child"); });
        worker.join();
    }, "MULTI_THREAD");
}

TEST_F(NativeNodeTreeScopeTest, UnrelatedPublishedNodesAreNotRetainedForTheWholeCall)
{
    auto parent = MakeNode();
    auto unrelated = MakeNode();
    WeakPtr<UINode> weak = unrelated;
    PublicCall outer({ parent.GetRawPtr() }, "outer");
    unrelated.Reset();
    EXPECT_TRUE(weak.Invalid());
}

TEST_F(NativeNodeTreeScopeTest, LogMergeKeepsBothInputSnapshotsAfterOneCallerReturns)
{
    auto parent = MakeNode();
    auto child = MakeNode();
    NativeNodeTreeScope::SetCheckMode(LOG);
    Gate gate;
    std::thread first([&] {
        PublicCall call({ child.GetRawPtr() }, "first");
        gate.SignalAndWait();
    });
    EXPECT_TRUE(gate.Wait());
    {
        PublicCall merge({ parent.GetRawPtr(), child.GetRawPtr() }, "addChild");
        parent->AddChild(child);
    }
    NativeNodeTreeScope::SetCheckMode(CRASH);
    EXPECT_DEATH({ PublicCall call({ parent.GetRawPtr() }, "merged.still.owned"); }, "MULTI_THREAD");
    gate.Finish();
    first.join();
}

} // namespace OHOS::Ace::NG

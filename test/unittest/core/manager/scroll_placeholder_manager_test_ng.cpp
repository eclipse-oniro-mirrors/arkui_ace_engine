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

#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "base/utils/time_util.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_cost_model.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_node_clone.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_restricted_scope.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_shell_node.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_observer.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline/base/element_register_multi_thread.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEST_INSTANCE_ID = 100;
constexpr int64_t COLD_START_NS = 5000000; // 5ms
constexpr int64_t FRAME_120HZ_NS = 8333333;
constexpr int64_t LARGE_DURATION_NS = 50000000; // 50ms
constexpr int64_t SMALL_DURATION_NS = 500000;   // 0.5ms

ScrollPlaceholderPredictParams MakeParams(
    ScrollPlaceholderComponentType type, const std::string& templateId, int32_t hostNodeId, int32_t index)
{
    ScrollPlaceholderPredictParams params;
    params.componentType = type;
    params.templateId = templateId;
    params.hostNodeId = hostNodeId;
    params.index = index;
    return params;
}
} // namespace

class TestPlaceholderNode : public UINode {
    DECLARE_ACE_TYPE(TestPlaceholderNode, UINode);

public:
    static RefPtr<TestPlaceholderNode> CreateTestNode(int32_t nodeId)
    {
        return MakeRefPtr<TestPlaceholderNode>(nodeId);
    }

    explicit TestPlaceholderNode(int32_t nodeId) : UINode("TestPlaceholderNode", nodeId) {}

    bool IsAtomicNode() const override
    {
        return true;
    }
};

// Captures background clone tasks instead of handing them to the BackgroundTaskExecutor, so
// the pool policy is tested deterministically: the synchronous seed is observed immediately
// and the replenish tasks run only when the test drains them. The per-node copy protocol
// stands in for the compiler restricted factory: a structural TestPlaceholderNode copy with
// a fresh id, counted for traversal assertions.
class TestableScrollPlaceholderManager : public ScrollPlaceholderManager {
public:
    explicit TestableScrollPlaceholderManager(int32_t instanceId) : ScrollPlaceholderManager(instanceId) {}

    void RunCapturedTasks()
    {
        auto tasks = std::move(capturedTasks_);
        capturedTasks_.clear();
        for (auto& task : tasks) {
            task();
        }
    }

    size_t CapturedTaskCount() const
    {
        return capturedTasks_.size();
    }

    uint32_t copiedNodeCount = 0;
    bool copyProtocolEnabled = true;
    bool useDefaultCopyProtocol = false;
    // False routes submissions to the real BackgroundTaskExecutor so the parallel clone flow
    // (worker thread + thread-safe node scope + free-node registration) runs for real.
    bool captureTasks = true;

protected:
    bool SubmitBackgroundCloneTask(std::function<void()> task) override
    {
        if (!captureTasks) {
            return ScrollPlaceholderManager::SubmitBackgroundCloneTask(std::move(task));
        }
        capturedTasks_.emplace_back(std::move(task));
        return true;
    }

    RefPtr<UINode> CreatePlaceholderNodeCopy(const RefPtr<UINode>& node) override
    {
        if (useDefaultCopyProtocol) {
            return ScrollPlaceholderManager::CreatePlaceholderNodeCopy(node);
        }
        if (!copyProtocolEnabled) {
            return nullptr;
        }
        copiedNodeCount++;
        return TestPlaceholderNode::CreateTestNode(node->GetId() + 1000);
    }

private:
    std::vector<std::function<void()>> capturedTasks_;
};

// Builder producing a two-level immutable source subtree for traversal assertions.
ScrollPlaceholderBuilder MakeTreeBuilder(int32_t baseId)
{
    return ScrollPlaceholderBuilder([baseId]() -> RefPtr<UINode> {
        auto root = TestPlaceholderNode::CreateTestNode(baseId);
        root->AddChild(TestPlaceholderNode::CreateTestNode(baseId + 1));
        return root;
    });
}

// Builder producing a frame-node placeholder subtree for the production copy protocol.
ScrollPlaceholderBuilder MakeFrameTreeBuilder()
{
    // Restricted copy protocol (step2): only whitelisted patterns clone, so the
    // production copy path needs whitelisted pattern nodes (plain containers).
    return ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        auto root = FrameNode::CreateFrameNode("placeholder_root",
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        auto child = FrameNode::CreateFrameNode("placeholder_child",
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        child->GetLayoutProperty()->UpdateAspectRatio(1.5f);
        root->AddChild(child);
        return root;
    });
}

// Waits until every submitted clone task has finished (each releases its pending slot at the
// end), so pool assertions after it are stable without pinning worker scheduling.
bool WaitForPendingClones(const RefPtr<TestableScrollPlaceholderManager>& manager, const std::string& templateId,
    uint32_t timeoutMs = 3000)
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        if (manager->GetPendingCloneCount(templateId) == 0 && manager->CapturedTaskCount() == 0) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return manager->GetPendingCloneCount(templateId) == 0 && manager->CapturedTaskCount() == 0;
}

class ScrollPlaceholderManagerTestNg : public testing::Test {
public:
    void SetUp() override
    {
        manager_ = AceType::MakeRefPtr<TestableScrollPlaceholderManager>(TEST_INSTANCE_ID);
    }
    void TearDown() override
    {
        manager_ = nullptr;
    }

    RefPtr<TestableScrollPlaceholderManager> manager_;
};

/**
 * @tc.name: CostModelColdStartEstimate001
 * @tc.desc: Without any sample the estimate falls back to the cold start constant.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, CostModelColdStartEstimate001, TestSize.Level1)
{
    ScrollPlaceholderCostModel model;
    EXPECT_EQ(model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tpl"), COLD_START_NS);
    EXPECT_EQ(model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::GRID, std::string()), COLD_START_NS);
    EXPECT_EQ(model.GetTemplateBucketSampleCount(ScrollPlaceholderComponentType::LIST, "tpl"), 0u);
    EXPECT_EQ(model.GetComponentSampleCount(ScrollPlaceholderComponentType::LIST), 0u);
}

/**
 * @tc.name: CostModelRecordAndEstimate001
 * @tc.desc: Recording real build durations makes the estimate follow recent samples.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, CostModelRecordAndEstimate001, TestSize.Level1)
{
    ScrollPlaceholderCostModel model;
    model.RecordRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tpl", LARGE_DURATION_NS);
    int64_t estimate = model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tpl");
    EXPECT_GE(estimate, LARGE_DURATION_NS); // mean + deviation never below the first sample here

    for (int i = 0; i < 20; i++) {
        model.RecordRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tpl", SMALL_DURATION_NS);
    }
    estimate = model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tpl");
    EXPECT_LT(estimate, SMALL_DURATION_NS * 2);
    EXPECT_EQ(model.GetTemplateBucketSampleCount(ScrollPlaceholderComponentType::LIST, "tpl"), 21u);
    EXPECT_EQ(model.GetComponentSampleCount(ScrollPlaceholderComponentType::LIST), 21u);
}

/**
 * @tc.name: CostModelComponentFallback001
 * @tc.desc: Unknown templates fall back to the component aggregate, other components stay cold.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, CostModelComponentFallback001, TestSize.Level1)
{
    ScrollPlaceholderCostModel model;
    model.RecordRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tplA", LARGE_DURATION_NS);
    int64_t sameComponent = model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::LIST, "tplB");
    EXPECT_GE(sameComponent, LARGE_DURATION_NS); // component aggregate carries the sample
    int64_t otherComponent = model.EstimateRealBuildDuration(ScrollPlaceholderComponentType::WATER_FLOW, "tplB");
    EXPECT_EQ(otherComponent, COLD_START_NS); // isolated bucket, cold start
}

/**
 * @tc.name: TemplateRegisterAndQuery001
 * @tc.desc: Register/query/lookup a template through the manager facade.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateRegisterAndQuery001, TestSize.Level1)
{
    bool builderCalled = false;
    uint64_t generation = manager_->RegisterTemplate(
        "tpl", [&builderCalled]() -> RefPtr<UINode> {
            builderCalled = true;
            return TestPlaceholderNode::CreateTestNode(1);
        });
    EXPECT_EQ(generation, 1u);
    EXPECT_TRUE(manager_->IsTemplateRegistered("tpl"));

    auto snapshot = manager_->LookupTemplateForCreate("tpl");
    ASSERT_TRUE(snapshot.has_value());
    EXPECT_EQ(snapshot->generation, 1u);
    ASSERT_TRUE(snapshot->builder);
    auto node = snapshot->builder();
    EXPECT_TRUE(builderCalled);
    ASSERT_TRUE(node);
}

/**
 * @tc.name: TemplateRegisterInvalidRejected001
 * @tc.desc: Empty id or empty builder is rejected with a diagnostics counter bump.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateRegisterInvalidRejected001, TestSize.Level1)
{
    EXPECT_EQ(manager_->RegisterTemplate(std::string(), []() -> RefPtr<UINode> { return nullptr; }), 0u);
    EXPECT_EQ(manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder()), 0u);
    EXPECT_FALSE(manager_->IsTemplateRegistered("tpl"));
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.registerRejected, 2u);
}

/**
 * @tc.name: TemplateUnregisterIdempotent001
 * @tc.desc: Unregister removes the entry once and is an idempotent no-op afterwards.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateUnregisterIdempotent001, TestSize.Level1)
{
    manager_->RegisterTemplate("tpl", []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); });
    EXPECT_TRUE(manager_->UnregisterTemplate("tpl"));
    EXPECT_FALSE(manager_->UnregisterTemplate("tpl"));
    EXPECT_FALSE(manager_->IsTemplateRegistered("tpl"));
    EXPECT_FALSE(manager_->LookupTemplateForCreate("tpl").has_value());
}

/**
 * @tc.name: TemplateGenerationMonotonic001
 * @tc.desc: Re-register and unregister keep the per-id generation monotonically increasing.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateGenerationMonotonic001, TestSize.Level1)
{
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    uint64_t first = manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    uint64_t second = manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    EXPECT_GT(second, first);
    EXPECT_TRUE(manager_->UnregisterTemplate("tpl"));
    EXPECT_FALSE(manager_->LookupTemplateForCreate("tpl").has_value());
    uint64_t third = manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    EXPECT_GT(third, second);
}

/**
 * @tc.name: TemplateInstanceCache001
 * @tc.desc: Built instances are retained per template with capacity and LIFO intake;
 *           acquires never consume the pool - every acquire is a fresh builder product.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateInstanceCache001, TestSize.Level1)
{
    manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    constexpr size_t spareSlots = SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1;
    std::vector<RefPtr<TestPlaceholderNode>> nodes;
    for (size_t i = 0; i < spareSlots + 1; ++i) { // one over the spare capacity
        nodes.emplace_back(TestPlaceholderNode::CreateTestNode(10 + static_cast<int32_t>(i)));
    }
    for (const auto& node : nodes) {
        manager_->CacheTemplateInstance("tpl", node);
    }
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), spareSlots); // overflow dropped
    ASSERT_TRUE(manager_->GetTemplateSourceInstance("tpl"));

    // The pool is not consumed on acquire: every call returns a fresh builder product
    // (distinct from the cached spares) and the cached count stays untouched.
    auto acquired = manager_->AcquireTemplateInstance("tpl");
    ASSERT_TRUE(acquired);
    EXPECT_NE(acquired, AceType::DynamicCast<UINode>(nodes[spareSlots - 1])); // fresh build, not the spare
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), spareSlots); // pool untouched by acquire
    auto synced = manager_->AcquireTemplateInstance("tpl");
    EXPECT_TRUE(synced);
    EXPECT_NE(synced, acquired); // each acquire builds its own product
    EXPECT_TRUE(manager_->GetTemplateSourceInstance("tpl")); // source never leaves the pool
}

/**
 * @tc.name: TemplateLruEvictionAndRebuild001
 * @tc.desc: Registering the 21st hot template evicts the least recently used one and releases
 *           its unmounted instances; the cold entry stays registered and rebuilds on demand.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateLruEvictionAndRebuild001, TestSize.Level1)
{
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    auto evictedInstance = TestPlaceholderNode::CreateTestNode(21);
    WeakPtr<UINode> evictedWeak = evictedInstance;
    for (int i = 0; i < 20; i++) {
        manager_->RegisterTemplate("tpl" + std::to_string(i), ScrollPlaceholderBuilder(builder));
    }
    manager_->CacheTemplateInstance("tpl0", evictedInstance);
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl0"), 1u); // single spare slot; source is resident
    EXPECT_EQ(manager_->GetHotTemplateCount(), 20u);

    // Promoting tpl19 (the MRU entry) keeps tpl0 at the LRU end; the 21st registration evicts it.
    manager_->LookupTemplateForCreate("tpl19");
    manager_->RegisterTemplate("tpl20", ScrollPlaceholderBuilder(builder));
    EXPECT_EQ(manager_->GetHotTemplateCount(), 20u);
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl0"), 0u);
    evictedInstance = nullptr;
    EXPECT_FALSE(evictedWeak.Upgrade()); // instance released by eviction

    // The evicted entry keeps its registration relation and rebuilds on the next create hit.
    EXPECT_TRUE(manager_->IsTemplateRegistered("tpl0"));
    auto snapshot = manager_->LookupTemplateForCreate("tpl0");
    ASSERT_TRUE(snapshot.has_value());
    EXPECT_EQ(manager_->GetHotTemplateCount(), 20u);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_GE(diagnostics.lruEvictedTemplates, 1u);
}

/**
 * @tc.name: TemplateLruQueryDoesNotPromote001
 * @tc.desc: isRegistered is read-only: it never keeps an entry hot.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, TemplateLruQueryDoesNotPromote001, TestSize.Level1)
{
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    auto retained = TestPlaceholderNode::CreateTestNode(31);
    for (int i = 0; i < 20; i++) {
        manager_->RegisterTemplate("tpl" + std::to_string(i), ScrollPlaceholderBuilder(builder));
    }
    // tpl0 is the LRU end; querying it must not promote it above tpl1.
    EXPECT_TRUE(manager_->IsTemplateRegistered("tpl0"));
    manager_->CacheTemplateInstance("tpl1", retained);
    manager_->RegisterTemplate("tpl20", ScrollPlaceholderBuilder(builder));
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl1"), 1u); // spare survived; tpl0 evicted
    retained = nullptr;
}

/**
 * @tc.name: PlaceholderPoolRegisterSeedAndReplenish001
 * @tc.desc: Registration builds the resident source synchronously on the UI thread; one
 *           background task then traverses and copies it into the single spare slot.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolRegisterSeedAndReplenish001, TestSize.Level1)
{
    EXPECT_EQ(manager_->RegisterTemplate("tpl", MakeTreeBuilder(1)), 1u);
    auto source = manager_->GetTemplateSourceInstance("tpl");
    ASSERT_TRUE(source);                       // synchronous source build at registration
    EXPECT_EQ(source->GetChildren().size(), 1u);
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u); // no spare yet
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_EQ(manager_->CapturedTaskCount(), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);

    manager_->RunCapturedTasks();
    EXPECT_EQ(manager_->copiedNodeCount, 2u * (SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1)); // root + child per submitted clone
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u);
    EXPECT_EQ(manager_->CapturedTaskCount(), 0u); // no extra tasks once at capacity
    EXPECT_EQ(manager_->GetTemplateSourceInstance("tpl"), source); // source untouched by clones
}

/**
 * @tc.name: PlaceholderPoolAcquireAlwaysBuilds001
 * @tc.desc: Acquire always executes the builder (full-fidelity product, never the
 *           resident source and never a pooled clone) and leaves the pool untouched:
 *           no spare is consumed and no replenish task is scheduled by the acquire.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolAcquireAlwaysBuilds001, TestSize.Level1)
{
    manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    manager_->RunCapturedTasks();
    auto source = manager_->GetTemplateSourceInstance("tpl");
    ASSERT_TRUE(source);
    ASSERT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);

    auto acquired = manager_->AcquireTemplateInstance("tpl");
    ASSERT_TRUE(acquired);
    EXPECT_NE(acquired, source);                   // a builder product, not the resident source
    EXPECT_NE(manager_->GetCachedInstanceCount("tpl"), 0u); // spares still cached
    EXPECT_EQ(acquired->GetChildren().size(), 1u); // builder kept the subtree shape
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u);   // acquire schedules no clone
    EXPECT_EQ(manager_->CapturedTaskCount(), 0u);
    EXPECT_EQ(manager_->GetTemplateSourceInstance("tpl"), source);
}

/**
 * @tc.name: PlaceholderPoolSyncCreateWhenEmpty001
 * @tc.desc: With no spare the acquire path synchronously builds from the template on the UI
 *           thread; the pending clone refills the spare afterwards.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolSyncCreateWhenEmpty001, TestSize.Level1)
{
    manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    EXPECT_EQ(manager_->CapturedTaskCount(), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1); // replenish pending, not run yet

    auto synced = manager_->AcquireTemplateInstance("tpl"); // acquire always builds
    ASSERT_TRUE(synced);
    EXPECT_EQ(synced->GetChildren().size(), 1u);  // built by the builder on the UI thread
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u); // served directly, not cached
    EXPECT_EQ(manager_->CapturedTaskCount(), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1); // spare slots already in flight
    manager_->RunCapturedTasks();
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u);
}

/**
 * @tc.name: PlaceholderPoolStaleCloneDroppedOnReRegister001
 * @tc.desc: A clone carrying the previous generation is dropped after re-register; the clone
 *           of the current generation still fills the spare slot from the rebuilt source.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolStaleCloneDroppedOnReRegister001, TestSize.Level1)
{
    uint64_t first = manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    auto firstSource = manager_->GetTemplateSourceInstance("tpl");
    uint64_t second = manager_->RegisterTemplate("tpl", MakeTreeBuilder(2));
    EXPECT_GT(second, first);
    auto secondSource = manager_->GetTemplateSourceInstance("tpl");
    ASSERT_TRUE(secondSource);
    EXPECT_NE(secondSource, firstSource);      // re-register rebuilt the source on the UI thread
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u);
    EXPECT_EQ(manager_->CapturedTaskCount(), 2u * (SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1)); // stale clones + current clones

    manager_->RunCapturedTasks();
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u); // late stale release is floor-guarded
}

/**
 * @tc.name: PlaceholderPoolDestroyDropsInFlight001
 * @tc.desc: Destroy drops an in-flight clone without caching and drains its pending slot.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolDestroyDropsInFlight001, TestSize.Level1)
{
    manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    ASSERT_EQ(manager_->CapturedTaskCount(), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    manager_->Destroy();
    manager_->RunCapturedTasks();
    EXPECT_FALSE(manager_->GetTemplateSourceInstance("tpl")); // registry cleared
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u);
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u);
}

/**
 * @tc.name: PlaceholderPoolCloneWithoutProtocolDegrades001
 * @tc.desc: Without a per-node copy protocol the background clone fails harmlessly and the
 *           pool keeps serving through the UI-thread paths.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolCloneWithoutProtocolDegrades001, TestSize.Level1)
{
    manager_->copyProtocolEnabled = false;
    manager_->RegisterTemplate("tpl", MakeTreeBuilder(1));
    ASSERT_TRUE(manager_->GetTemplateSourceInstance("tpl"));
    manager_->RunCapturedTasks();
    EXPECT_EQ(manager_->copiedNodeCount, 0u);
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u); // clone dropped
    EXPECT_EQ(manager_->GetPendingCloneCount("tpl"), 0u);   // slot released
    EXPECT_TRUE(manager_->AcquireTemplateInstance("tpl"));  // UI-thread fallback still serves
}

/**
 * @tc.name: PlaceholderPoolDefaultCopyCreatesSameTypeNodes001
 * @tc.desc: The production copy protocol creates a same-type node per visited source node
 *           and copies the layout property over; plain frame node subtrees clone fully.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolDefaultCopyCreatesSameTypeNodes001, TestSize.Level1)
{
    manager_->useDefaultCopyProtocol = true;
    auto builder = []() -> RefPtr<UINode> {
        // Whitelisted (restricted copy protocol) patterns: plain containers.
        auto root = FrameNode::CreateFrameNode("placeholder_root",
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        auto child = FrameNode::CreateFrameNode("placeholder_child",
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        child->GetLayoutProperty()->UpdateAspectRatio(1.5f);
        root->AddChild(child);
        return root;
    };
    manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    auto source = manager_->GetTemplateSourceInstance("tpl");
    ASSERT_TRUE(source);
    manager_->RunCapturedTasks();
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);

    auto acquired = manager_->AcquireTemplateInstance("tpl");
    ASSERT_TRUE(acquired);
    EXPECT_NE(acquired, source); // same-type copies, not the resident source
    auto copyRoot = AceType::DynamicCast<FrameNode>(acquired);
    ASSERT_TRUE(copyRoot);
    EXPECT_EQ(copyRoot->GetTag(), "placeholder_root");
    EXPECT_NE(copyRoot->GetId(), AceType::DynamicCast<FrameNode>(source)->GetId());
    const auto& copyChildren = copyRoot->GetChildren();
    ASSERT_EQ(copyChildren.size(), 1u);
    auto copyChild = AceType::DynamicCast<FrameNode>(copyChildren.front());
    ASSERT_TRUE(copyChild);
    EXPECT_EQ(copyChild->GetTag(), "placeholder_child");
    EXPECT_FLOAT_EQ(copyChild->GetLayoutProperty()->GetAspectRatio(), 1.5f); // property copied
}

/**
 * @tc.name: PlaceholderPoolParallelCloneFillsPool001
 * @tc.desc: With the real BackgroundTaskExecutor a worker thread clones the resident
 *           source and fills the pool off-thread; an acquire executes the builder on
 *           the UI thread instead, so the product registers in the global element
 *           register rather than the thread-safe registry the clones use.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolParallelCloneFillsPool001, TestSize.Level1)
{
    manager_->captureTasks = false;            // real executor, real worker thread
    manager_->useDefaultCopyProtocol = true;   // production copy: create same type + copy property
    manager_->RegisterTemplate("tpl", MakeFrameTreeBuilder());
    ASSERT_TRUE(WaitForPendingClones(manager_, "tpl"));
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);

    auto source = AceType::DynamicCast<FrameNode>(manager_->GetTemplateSourceInstance("tpl"));
    ASSERT_TRUE(source);
    auto acquired = manager_->AcquireTemplateInstance("tpl"); // builder runs on the UI thread
    ASSERT_TRUE(acquired);
    auto copyRoot = AceType::DynamicCast<FrameNode>(acquired);
    ASSERT_TRUE(copyRoot);
    EXPECT_EQ(copyRoot->GetTag(), source->GetTag());   // builder kept the template type
    EXPECT_NE(copyRoot->GetId(), source->GetId());     // fresh node, not the resident source
    ASSERT_EQ(copyRoot->GetChildren().size(), 1u);
    auto copyChild = AceType::DynamicCast<FrameNode>(copyRoot->GetChildren().front());
    ASSERT_TRUE(copyChild);
    EXPECT_FLOAT_EQ(copyChild->GetLayoutProperty()->GetAspectRatio(), 1.5f); // builder set the property
    // Builder product created on the UI thread: registered in the global element
    // register, not the thread-safe registry the worker clones use.
    EXPECT_EQ(ElementRegisterMultiThread::GetInstance()->GetThreadSafeNodeById(copyRoot->GetId()), nullptr);
    // The pool of worker clones stays untouched by the acquire.
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
}

/**
 * @tc.name: PlaceholderPoolParallelReplenishLoop001
 * @tc.desc: Repeated acquire on the UI thread interleaves with worker refills; every handed
 *           out instance is unique and the pool returns to capacity after each cycle.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolParallelReplenishLoop001, TestSize.Level1)
{
    manager_->captureTasks = false;
    manager_->useDefaultCopyProtocol = true;
    manager_->RegisterTemplate("tpl", MakeFrameTreeBuilder());
    ASSERT_TRUE(WaitForPendingClones(manager_, "tpl"));

    std::set<int32_t> seenIds;
    std::vector<RefPtr<UINode>> held;
    for (int i = 0; i < 6; i++) {
        auto instance = manager_->AcquireTemplateInstance("tpl");
        ASSERT_TRUE(instance); // built synchronously on the UI thread
        auto frame = AceType::DynamicCast<FrameNode>(instance);
        ASSERT_TRUE(frame);
        EXPECT_EQ(seenIds.count(frame->GetId()), 0u); // unique instance every time
        seenIds.insert(frame->GetId());
        held.push_back(instance);
        ASSERT_TRUE(WaitForPendingClones(manager_, "tpl"));
    }
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
}

/**
 * @tc.name: PlaceholderPoolParallelStaleDrop001
 * @tc.desc: A re-register racing an in-flight worker clone leaves exactly one spare of the
 *           current generation; stale clones are dropped by the generation guard.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolParallelStaleDrop001, TestSize.Level1)
{
    manager_->captureTasks = false;
    manager_->useDefaultCopyProtocol = true;
    uint64_t first = manager_->RegisterTemplate("tpl", MakeFrameTreeBuilder());
    uint64_t second = manager_->RegisterTemplate("tpl", MakeFrameTreeBuilder()); // may race the clone
    EXPECT_GT(second, first);
    ASSERT_TRUE(WaitForPendingClones(manager_, "tpl"));
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    EXPECT_TRUE(manager_->AcquireTemplateInstance("tpl"));
}

/**
 * @tc.name: PlaceholderPoolParallelDestroy001
 * @tc.desc: Destroy while a worker clone is in flight drops it safely: no crash, slots drain.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolParallelDestroy001, TestSize.Level1)
{
    manager_->captureTasks = false;
    manager_->useDefaultCopyProtocol = true;
    manager_->RegisterTemplate("tpl", MakeFrameTreeBuilder());
    manager_->Destroy(); // task may still be executing on the worker
    ASSERT_TRUE(WaitForPendingClones(manager_, "tpl"));
    EXPECT_EQ(manager_->GetCachedInstanceCount("tpl"), 0u);
    EXPECT_FALSE(manager_->GetTemplateSourceInstance("tpl"));
}

/**
 * @tc.name: PlaceholderPoolDefaultSubmitterSmoke001
 * @tc.desc: The production submitter path (BackgroundTaskExecutor) registers the source on
 *           the UI thread; the background clone never overfills the pool.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PlaceholderPoolDefaultSubmitterSmoke001, TestSize.Level1)
{
    auto real = AceType::MakeRefPtr<ScrollPlaceholderManager>(TEST_INSTANCE_ID + 2);
    EXPECT_GT(real->RegisterTemplate("tpl", MakeTreeBuilder(1)), 0u);
    EXPECT_TRUE(real->GetTemplateSourceInstance("tpl")); // synchronous source is guaranteed
    EXPECT_LE(real->GetCachedInstanceCount("tpl"), SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1);
    real->Destroy();
}

/**
 * @tc.name: PredictWithoutFrameAnchor001
 * @tc.desc: Without a frame anchor or explicit deadline the legacy synchronous path is kept.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PredictWithoutFrameAnchor001, TestSize.Level1)
{
    auto result = manager_->Predict(MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0), 0);
    EXPECT_EQ(result.decision, ScrollPlaceholderDecision::BUILD_REAL_NOW);
    EXPECT_EQ(result.remainingBudgetNs, 0);
}

/**
 * @tc.name: PredictWithVsyncAnchor001
 * @tc.desc: The vsync derived deadline uses the reported period with a 60Hz fallback.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PredictWithVsyncAnchor001, TestSize.Level1)
{
    manager_->NotifyVsync(1000, FRAME_120HZ_NS);
    EXPECT_EQ(manager_->GetVsyncDerivedFrameDeadlineNs(), 1000 + FRAME_120HZ_NS);
    manager_->NotifyVsync(2000, 0);
    EXPECT_EQ(manager_->GetVsyncDerivedFrameDeadlineNs(), 2000 + 16666667);
}

/**
 * @tc.name: PredictDecisionByBudget001
 * @tc.desc: A generous deadline builds synchronously, a tight deadline defers to placeholder.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PredictDecisionByBudget001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 1, 0);
    int64_t generous = GetSysTimestamp() + 10 * COLD_START_NS;
    auto allow = manager_->Predict(params, generous);
    EXPECT_EQ(allow.decision, ScrollPlaceholderDecision::BUILD_REAL_NOW);
    EXPECT_GT(allow.remainingBudgetNs, 0);

    int64_t tight = GetSysTimestamp() + 2 * 1000000; // 2ms, below the 5ms cold estimate
    auto defer = manager_->Predict(params, tight);
    EXPECT_EQ(defer.decision, ScrollPlaceholderDecision::USE_PLACEHOLDER);
}

/**
 * @tc.name: PredictAdaptsToSamples001
 * @tc.desc: The decision follows the sampled cost of the bucket.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PredictAdaptsToSamples001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0);
    for (int i = 0; i < 5; i++) {
        manager_->RecordRealBuildDuration(params, LARGE_DURATION_NS);
    }
    int64_t deadline = GetSysTimestamp() + 10 * 1000000; // 10ms
    EXPECT_EQ(manager_->Predict(params, deadline).decision, ScrollPlaceholderDecision::USE_PLACEHOLDER);

    for (int i = 0; i < 30; i++) {
        manager_->RecordRealBuildDuration(params, SMALL_DURATION_NS);
    }
    deadline = GetSysTimestamp() + 10 * 1000000;
    EXPECT_EQ(manager_->Predict(params, deadline).decision, ScrollPlaceholderDecision::BUILD_REAL_NOW);
}

/**
 * @tc.name: RealBuildEnqueueDedup001
 * @tc.desc: Enqueue deduplicates by item identity (host node id + index).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildEnqueueDedup001, TestSize.Level1)
{
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 7, 3), 0, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 7, 3), 0, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 7, 4), 0, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 2u);
    EXPECT_TRUE(manager_->HasPendingRealBuild());
}

/**
 * @tc.name: RealBuildFlushExecutesAndRecords001
 * @tc.desc: Flush executes tasks within the budget and samples their duration.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildFlushExecutesAndRecords001, TestSize.Level1)
{
    uint64_t generation =
        manager_->RegisterTemplate("tpl", []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); });
    int executed = 0;
    for (int i = 0; i < 3; i++) {
        manager_->EnqueueRealBuild(
            MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, i), generation,
            [&executed]() {
                executed++;
                return true;
            },
            ScrollPlaceholderRealBuildPriority::VISIBLE);
    }
    manager_->FlushRealBuild(GetSysTimestamp() + 100 * 1000000);
    EXPECT_EQ(executed, 3);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.realBuildExecuted, 3u);
    // Executed durations were sampled into the component bucket.
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0);
    EXPECT_LE(manager_->EstimateRealBuildDuration(params), 5 * COLD_START_NS);
}

/**
 * @tc.name: RealBuildFlushStopsOnBudget001
 * @tc.desc: Flush keeps tasks queued and requests another frame when the estimate exceeds the
 *           remaining budget.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildFlushStopsOnBudget001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0);
    for (int i = 0; i < 5; i++) {
        manager_->RecordRealBuildDuration(params, LARGE_DURATION_NS);
    }
    int executed = 0;
    for (int i = 0; i < 2; i++) {
        manager_->EnqueueRealBuild(
            MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, i), 0,
            [&executed]() {
                executed++;
                return true;
            },
            ScrollPlaceholderRealBuildPriority::VISIBLE);
    }
    manager_->FlushRealBuild(GetSysTimestamp() + 5 * 1000000); // ~4ms usable, estimate ~50ms
    EXPECT_EQ(executed, 0);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 2u);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.realBuildRequeued, 2u);
    EXPECT_GE(diagnostics.requestFrameCount, 1u);
}

/**
 * @tc.name: RealBuildFlushPriorityOrder001
 * @tc.desc: Visible tasks run before cache-only tasks regardless of enqueue order.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildFlushPriorityOrder001, TestSize.Level1)
{
    std::vector<int32_t> order;
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 1, 5), 0,
        [&order]() {
            order.push_back(5);
            return true;
        },
        ScrollPlaceholderRealBuildPriority::CACHE_ONLY);
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 1, 1), 0,
        [&order]() {
            order.push_back(1);
            return true;
        },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    manager_->FlushRealBuild(GetSysTimestamp() + 100 * 1000000);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 5);
}

/**
 * @tc.name: RealBuildStaleTemplateGeneration001
 * @tc.desc: Unregister purges pending tasks eagerly; re-register invalidates them at flush.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildStaleTemplateGeneration001, TestSize.Level1)
{
    int executed = 0;
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    uint64_t generation = manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0), generation,
        [&executed]() {
            executed++;
            return true;
        },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 1), generation,
        [&executed]() {
            executed++;
            return true;
        },
        ScrollPlaceholderRealBuildPriority::VISIBLE);

    // Re-register publishes a new generation: the queued tasks become stale at flush time.
    manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    manager_->FlushRealBuild(GetSysTimestamp() + 100 * 1000000);
    EXPECT_EQ(executed, 0);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.realBuildDropped, 2u);

    // Unregister drops matching pending tasks immediately.
    uint64_t latest = manager_->RegisterTemplate("tpl2", ScrollPlaceholderBuilder(builder));
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, "tpl2", 2, 0), latest,
        [&executed]() {
            executed++;
            return true;
        },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    EXPECT_TRUE(manager_->UnregisterTemplate("tpl2"));
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.cancelledByReason[static_cast<size_t>(ScrollPlaceholderCancelReason::UNREGISTER)], 1u);
}

/**
 * @tc.name: RealBuildCancelHost001
 * @tc.desc: Cancelling a host removes its pending tasks and reports the reason code.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RealBuildCancelHost001, TestSize.Level1)
{
    int executed = 0;
    for (int i = 0; i < 3; i++) {
        manager_->EnqueueRealBuild(
            MakeParams(ScrollPlaceholderComponentType::GRID, std::string(), i + 1, 0), 0,
            [&executed]() {
                executed++;
                return true;
            },
            ScrollPlaceholderRealBuildPriority::VISIBLE);
    }
    manager_->CancelHostRealBuilds(2, ScrollPlaceholderCancelReason::HOST_RESET);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 2u);
    manager_->FlushRealBuild(GetSysTimestamp() + 100 * 1000000);
    EXPECT_EQ(executed, 2);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.cancelledByReason[static_cast<size_t>(ScrollPlaceholderCancelReason::HOST_RESET)], 1u);
}

/**
 * @tc.name: ManagerDestroy001
 * @tc.desc: Destroy drains pending tasks, releases the registry and turns later calls into no-ops.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ManagerDestroy001, TestSize.Level1)
{
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0), 1, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    manager_->Destroy();
    EXPECT_TRUE(manager_->IsDestroyed());
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    EXPECT_FALSE(manager_->IsTemplateRegistered("tpl"));
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.cancelledByReason[static_cast<size_t>(ScrollPlaceholderCancelReason::PIPELINE_DESTROYED)],
        1u);

    manager_->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, "tpl", 1, 0), 1, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    manager_->FlushRealBuild(GetSysTimestamp() + 100 * 1000000);
    manager_->Destroy(); // idempotent
}

/**
 * @tc.name: ManagerIsolation001
 * @tc.desc: Registries and queues are isolated per manager (per container instance).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ManagerIsolation001, TestSize.Level1)
{
    auto other = AceType::MakeRefPtr<ScrollPlaceholderManager>(TEST_INSTANCE_ID + 1);
    auto builder = []() -> RefPtr<UINode> { return TestPlaceholderNode::CreateTestNode(1); };
    manager_->RegisterTemplate("tpl", ScrollPlaceholderBuilder(builder));
    EXPECT_FALSE(other->IsTemplateRegistered("tpl"));
    EXPECT_FALSE(other->LookupTemplateForCreate("tpl").has_value());
    EXPECT_EQ(other->GetHotTemplateCount(), 0u);
    EXPECT_EQ(manager_->GetHotTemplateCount(), 1u);

    other->EnqueueRealBuild(
        MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 1, 0), 0, []() { return true; },
        ScrollPlaceholderRealBuildPriority::VISIBLE);
    EXPECT_EQ(other->GetPendingRealBuildCount(), 1u);
    EXPECT_EQ(manager_->GetPendingRealBuildCount(), 0u);
    other->Destroy();
}

/**
 * @tc.name: PredictDiagnosticsCounters001
 * @tc.desc: Decision counters accumulate for both outcomes.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, PredictDiagnosticsCounters001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 1, 0);
    manager_->Predict(params, GetSysTimestamp() + 10 * COLD_START_NS);
    manager_->Predict(params, GetSysTimestamp() + 2 * 1000000);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.predictBuildRealNow, 1u);
    EXPECT_EQ(diagnostics.predictUsePlaceholder, 1u);
}

/**
 * @tc.name: ObservationFrameStatsAccumulateAndClose001
 * @tc.desc: Real build observation accumulates per frame and closes on the next vsync; the
 *           suggestion counts follow the anchored budget (build-now vs placeholder).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ObservationFrameStatsAccumulateAndClose001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 7, 0);

    // Before the first anchor: the build is observed but no budget suggestion is counted.
    manager_->NotifyRealBuildStart(params);
    manager_->NotifyRealBuildEnd(params, GetSysTimestamp() - 1000);
    auto stats = manager_->GetLastFrameObservation(ScrollPlaceholderComponentType::LIST);
    EXPECT_EQ(stats.observedBuilds, 0u); // frame not closed yet

    // Close the (anchorless) frame; anchor the next frame with a generous budget.
    manager_->NotifyVsync(GetSysTimestamp(), FRAME_120HZ_NS);
    stats = manager_->GetLastFrameObservation(ScrollPlaceholderComponentType::LIST);
    EXPECT_EQ(stats.observedBuilds, 1u);
    EXPECT_EQ(stats.suggestedReal, 0u);
    EXPECT_EQ(stats.suggestedPlaceholder, 0u);

    // Anchored frame with ~7ms usable budget above the 5ms cold estimate: suggest real.
    params.index = 1;
    auto result = manager_->NotifyRealBuildStart(params);
    EXPECT_TRUE(result.budgetAvailable);
    EXPECT_EQ(result.decision, ScrollPlaceholderDecision::BUILD_REAL_NOW);
    manager_->NotifyRealBuildEnd(params, GetSysTimestamp() - 50000);
    manager_->NotifyVsync(GetSysTimestamp(), FRAME_120HZ_NS);
    stats = manager_->GetLastFrameObservation(ScrollPlaceholderComponentType::LIST);
    EXPECT_EQ(stats.observedBuilds, 1u);
    EXPECT_EQ(stats.suggestedReal, 1u);
    EXPECT_EQ(stats.suggestedPlaceholder, 0u);
    EXPECT_GT(stats.lastRemainingBudgetNs, 0);
    auto diagnostics = manager_->GetDiagnostics();
    EXPECT_EQ(diagnostics.observedRealBuilds, 2u);

    // Anchored frame whose deadline is already exhausted: suggest placeholder.
    manager_->NotifyVsync(GetSysTimestamp() - FRAME_120HZ_NS, 1);
    params.index = 2;
    result = manager_->NotifyRealBuildStart(params);
    EXPECT_TRUE(result.budgetAvailable);
    EXPECT_EQ(result.decision, ScrollPlaceholderDecision::USE_PLACEHOLDER);
    manager_->NotifyRealBuildEnd(params, GetSysTimestamp() - 1000);
    manager_->NotifyVsync(GetSysTimestamp(), FRAME_120HZ_NS);
    stats = manager_->GetLastFrameObservation(ScrollPlaceholderComponentType::LIST);
    EXPECT_EQ(stats.observedBuilds, 1u);
    EXPECT_EQ(stats.suggestedReal, 0u);
    EXPECT_EQ(stats.suggestedPlaceholder, 1u);
}

/**
 * @tc.name: ObservationFeedsCostModel001
 * @tc.desc: Observed durations are sampled into the component bucket used for estimates.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ObservationFeedsCostModel001, TestSize.Level1)
{
    auto params = MakeParams(ScrollPlaceholderComponentType::GRID, std::string(), 3, 0);
    for (int i = 0; i < 4; i++) {
        params.index = i;
        manager_->NotifyRealBuildStart(params);
        manager_->NotifyRealBuildEnd(params, GetSysTimestamp() - LARGE_DURATION_NS);
    }
    // Duration sampling reaches the component bucket of the cost model.
    EXPECT_GE(manager_->EstimateRealBuildDuration(params), LARGE_DURATION_NS);
    EXPECT_EQ(manager_->GetDiagnostics().observedRealBuilds, 4u);
}

// Observer scope tests exercise the List/Grid/WaterFlow call point glue: the cache gate that
// keeps already built items out of the observation, and the empty-acquisition skip that keeps
// zero-cost samples away from the cost model. The end-to-end sampling semantics of the
// manager hooks are covered by the cases above.
class ScrollPlaceholderObserverTestNg : public testing::Test {
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

namespace {
RefPtr<FrameNode> CreateObserverHost(int32_t nodeId, bool withChild)
{
    auto host = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>());
    if (withChild) {
        host->AddChild(FrameNode::CreateFrameNode("observer_child", nodeId + 1, AceType::MakeRefPtr<Pattern>()));
    }
    return host;
}

uint64_t CurrentObservedRealBuilds()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, std::numeric_limits<uint64_t>::max());
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    CHECK_NULL_RETURN(manager, std::numeric_limits<uint64_t>::max());
    return manager->GetDiagnostics().observedRealBuilds;
}
} // namespace

/**
 * @tc.name: ObserverScopeSkipsBuiltItem001
 * @tc.desc: An index that resolves without running the real builder is not observed at all.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderObserverTestNg, ObserverScopeSkipsBuiltItem001, TestSize.Level1)
{
    auto host = CreateObserverHost(2001, true);
    auto wrapper = host->CreateLayoutWrapper();
    auto before = CurrentObservedRealBuilds();
    {
        ScrollPlaceholderItemBuildScope buildScope(
            ScrollPlaceholderComponentType::LIST, AceType::RawPtr(wrapper), 0, false);
        auto acquired = wrapper->GetOrCreateChildByIndex(0, true, false);
        buildScope.SetAcquiredWrapper(acquired);
        EXPECT_TRUE(acquired); // the built child resolves from the wrapper cache
    }
    EXPECT_EQ(CurrentObservedRealBuilds(), before); // cache gate: no observation
}

/**
 * @tc.name: ObserverScopeSkipsEmptyAcquisition001
 * @tc.desc: A null acquisition result (out of range / empty branch) never samples the model.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderObserverTestNg, ObserverScopeSkipsEmptyAcquisition001, TestSize.Level1)
{
    auto host = CreateObserverHost(2021, false);
    auto wrapper = host->CreateLayoutWrapper();
    auto before = CurrentObservedRealBuilds();
    {
        ScrollPlaceholderItemBuildScope buildScope(
            ScrollPlaceholderComponentType::LIST, AceType::RawPtr(wrapper), 0, false);
        auto acquired = wrapper->GetOrCreateChildByIndex(0, true, false);
        buildScope.SetAcquiredWrapper(acquired);
        EXPECT_FALSE(acquired); // nothing to build for this index
    }
    EXPECT_EQ(CurrentObservedRealBuilds(), before); // no zero-cost sample
}

/**
 * @tc.name: ObserverScopeObservesRealAcquisition001
 * @tc.desc: An unbuilt index with a non-null acquisition result is observed and sampled.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderObserverTestNg, ObserverScopeObservesRealAcquisition001, TestSize.Level1)
{
    auto host = CreateObserverHost(2041, false);
    auto wrapper = host->CreateLayoutWrapper();
    // Stand-in wrapper for the item the real builder would produce for the unbuilt index.
    RefPtr<LayoutWrapper> builtElsewhere = CreateObserverHost(2061, false)->CreateLayoutWrapper();
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_TRUE(context);
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto before = CurrentObservedRealBuilds();
    {
        ScrollPlaceholderItemBuildScope buildScope(
            ScrollPlaceholderComponentType::LIST, AceType::RawPtr(wrapper), 0, false);
        buildScope.SetAcquiredWrapper(builtElsewhere);
    }
    EXPECT_EQ(CurrentObservedRealBuilds(), before + 1); // sampled through NotifyRealBuildEnd
    // The tiny measured duration replaced the 5ms cold start estimate.
    auto probe = MakeParams(ScrollPlaceholderComponentType::LIST, std::string(), 0, 0);
    EXPECT_LT(manager->EstimateRealBuildDuration(probe), COLD_START_NS);
}

/**
 * @tc.name: ProbeTemplateCloneNoSource001
 * @tc.desc: A probe for a template without a resident source instance reports the
 *           dedicated diagnostic path and keeps the template unregistered state.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneNoSource001, TestSize.Level1)
{
    std::string subtreePath = "untouched";
    EXPECT_FALSE(manager_->ProbeTemplateClone("tpl-no-source", subtreePath));
    EXPECT_EQ(subtreePath, "<no source instance>");
    EXPECT_FALSE(manager_->IsTemplateRegistered("tpl-no-source"));
}

namespace {
// Shared probe scaffold: registers a template, runs the admission probe and reports
// (admitted, failing subtree path). unregisters afterwards unless keepRegistered
// (the admitted case asserts on the pool the probe product joined).
std::pair<bool, std::string> ProbeRegisteredTemplate(const RefPtr<TestableScrollPlaceholderManager>& manager,
    ScrollPlaceholderBuilder&& builder, bool keepRegistered = false)
{
    const std::string templateId = "tpl-probe-case";
    manager->RegisterTemplate(templateId, std::move(builder));
    std::string subtreePath;
    bool admitted = manager->ProbeTemplateClone(templateId, subtreePath);
    if (!keepRegistered) {
        manager->UnregisterTemplate(templateId);
    }
    return { admitted, subtreePath };
}
} // namespace

/**
 * @tc.name: ProbeTemplateCloneUnsupportedRoot001
 * @tc.desc: A template whose root pattern is not on the restricted copy whitelist
 *           (base Pattern) is rejected with the offending node tag as the path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneUnsupportedRoot001, TestSize.Level1)
{
    auto rejectedBefore = manager_->GetDiagnostics().registerRejected;
    auto [admitted, subtreePath] = ProbeRegisteredTemplate(manager_, ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode(
            "plain_root", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    }));
    EXPECT_FALSE(admitted);
    EXPECT_EQ(subtreePath, "plain_root");
    EXPECT_GE(manager_->GetDiagnostics().registerRejected, rejectedBefore + 1);
}

/**
 * @tc.name: ProbeTemplateCloneUnsupportedNested001
 * @tc.desc: A whitelisted root with an unsupported descendant is rejected with the
 *           full subtree route to the offending node in the diagnostic path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneUnsupportedNested001, TestSize.Level1)
{
    auto [admitted, subtreePath] = ProbeRegisteredTemplate(manager_, ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        auto root = FrameNode::CreateFrameNode("nested_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        root->AddChild(FrameNode::CreateFrameNode("bad_child", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>()));
        return root;
    }));
    EXPECT_FALSE(admitted);
    EXPECT_EQ(subtreePath, "nested_root > bad_child");
}

/**
 * @tc.name: ProbeTemplateCloneAdmitted001
 * @tc.desc: A whitelisted subtree passes the admission probe: validation is the clone
 *           itself, the probe product joins the instance pool and a background
 *           replenish is scheduled.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneAdmitted001, TestSize.Level1)
{
    auto [admitted, subtreePath] = ProbeRegisteredTemplate(manager_, MakeFrameTreeBuilder(), true);
    EXPECT_TRUE(admitted);
    EXPECT_TRUE(subtreePath.empty());
    EXPECT_GE(manager_->GetCachedInstanceCount("tpl-probe-case"), 1u);
    EXPECT_GE(manager_->CapturedTaskCount(), 1u);
    manager_->UnregisterTemplate("tpl-probe-case");
}

/**
 * @tc.name: ProbeTemplateCloneCloneFails001
 * @tc.desc: A probe whose production copy hook refuses every node fails at the
 *           source root and reports the root tag as the failing path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneCloneFails001, TestSize.Level1)
{
    manager_->copyProtocolEnabled = false;
    auto [admitted, subtreePath] = ProbeRegisteredTemplate(manager_, MakeFrameTreeBuilder());
    manager_->copyProtocolEnabled = true;
    EXPECT_FALSE(admitted);
    EXPECT_EQ(subtreePath, "placeholder_root");
}

/**
 * @tc.name: ProbeTemplateCloneDestroyedManager001
 * @tc.desc: A destroyed manager rejects every probe outright.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneDestroyedManager001, TestSize.Level1)
{
    manager_->Destroy();
    std::string subtreePath = "untouched";
    EXPECT_FALSE(manager_->ProbeTemplateClone("tpl-any", subtreePath));
    EXPECT_TRUE(subtreePath.empty());
}

/**
 * @tc.name: ProbeTemplateCloneNonFrameNode001
 * @tc.desc: A template whose source root is not a frame node at all is rejected
 *           with the node tag as the diagnostic path (the walk stops before any
 *           pattern check).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ProbeTemplateCloneNonFrameNode001, TestSize.Level1)
{
    auto [admitted, subtreePath] = ProbeRegisteredTemplate(manager_, ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        auto root = TestPlaceholderNode::CreateTestNode(930001);
        root->AddChild(TestPlaceholderNode::CreateTestNode(930002));
        return root;
    }));
    EXPECT_FALSE(admitted);
    EXPECT_EQ(subtreePath, "TestPlaceholderNode");
}

/**
 * @tc.name: RestrictedCopyWhitelistPatterns001
 * @tc.desc: Every whitelisted render-only pattern clones through the production
 *           restricted copy protocol (fresh same-type pattern + wholesale layout
 *           property + zIndex), and the copy restores the pattern type.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, RestrictedCopyWhitelistPatterns001, TestSize.Level1)
{
    // Route the clone through the production copy protocol (the Testable default
    // stub would produce plain test nodes instead of frame nodes).
    manager_->useDefaultCopyProtocol = true;
    struct WhitelistCase {
        std::string tag;
        RefPtr<Pattern> pattern;
    };
    std::vector<WhitelistCase> cases = {
        { "text_node", AceType::MakeRefPtr<TextPattern>() },
        { "image_node", AceType::MakeRefPtr<ImagePattern>() },
        { "linear_node", AceType::MakeRefPtr<LinearLayoutPattern>(false) },
        { "stack_node", AceType::MakeRefPtr<StackPattern>() },
        { "flex_node", AceType::MakeRefPtr<FlexLayoutPattern>(true) },
    };
    for (const auto& entry : cases) {
        std::string templateId = "tpl-whitelist-" + entry.tag;
        ScrollPlaceholderBuilder builder = [entry]() -> RefPtr<UINode> {
            auto node = FrameNode::CreateFrameNode(
                entry.tag, ElementRegister::GetInstance()->MakeUniqueId(), entry.pattern);
            node->GetRenderContext()->UpdateZIndex(7);
            return node;
        };
        manager_->RegisterTemplate(templateId, std::move(builder));
        std::string subtreePath;
        EXPECT_TRUE(manager_->ProbeTemplateClone(templateId, subtreePath)) << entry.tag;
        // The probe product (a spare clone) carries the copied layout-relevant state.
        auto instance = manager_->AcquireTemplateInstance(templateId);
        ASSERT_TRUE(instance) << entry.tag;
        auto frame = AceType::DynamicCast<FrameNode>(instance);
        ASSERT_TRUE(frame) << entry.tag;
        EXPECT_EQ(frame->GetTag(), entry.tag);
        EXPECT_EQ(frame->GetRenderContext()->GetZIndex().value_or(0), 7) << entry.tag;
        manager_->UnregisterTemplate(templateId);
    }
    manager_->useDefaultCopyProtocol = false;
}

/**
 * @tc.name: ShellNodeInvisibleToChildEnumeration001
 * @tc.desc: A placeholder shell keeps a zero frame count on purpose: host-level
 *           child-slot enumerations must never count it as an item.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ShellNodeInvisibleToChildEnumeration001, TestSize.Level1)
{
    auto shell = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(
        "shell", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_TRUE(shell);
    EXPECT_EQ(shell->FrameCount(), 0);
    EXPECT_EQ(shell->CurrentFrameCount(), 0);
}

// The restricted-scope counters are process-global statics shared by every test of this
// binary, so each case balances its own Begin/End pairs and the TearDown drains any
// depth a failed assertion may have left behind.
class ScrollPlaceholderRestrictedScopeTestNg : public testing::Test {
public:
    void TearDown() override
    {
        while (ScrollPlaceholderRestrictedScope::IsInRestrictedScope()) {
            ScrollPlaceholderRestrictedScope::End();
        }
    }
};

/**
 * @tc.name: RestrictedScopePairingAndUnderflow001
 * @tc.desc: Nested Begin/End pairs keep the scope depth, and an unpaired End
 *           underflows safely (the depth never goes negative).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopePairingAndUnderflow001, TestSize.Level1)
{
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    ScrollPlaceholderRestrictedScope::Begin();
    EXPECT_TRUE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    ScrollPlaceholderRestrictedScope::Begin(); // re-entrant nesting
    EXPECT_TRUE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    ScrollPlaceholderRestrictedScope::End();
    EXPECT_TRUE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    ScrollPlaceholderRestrictedScope::End();
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    // Unpaired End (underflow guard): the depth stays at zero instead of wrapping.
    ScrollPlaceholderRestrictedScope::End();
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
}

/**
 * @tc.name: RestrictedScopeRaiiGuard001
 * @tc.desc: The RAII guard opens the scope on construction and closes it on every
 *           exit path, keeping the pairing balanced.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeRaiiGuard001, TestSize.Level1)
{
    {
        ScopedScrollPlaceholderRestricted guard;
        EXPECT_TRUE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
    }
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsInRestrictedScope());
}

/**
 * @tc.name: RestrictedScopeProbeAdmitsPairedScope001
 * @tc.desc: A probe window covering one paired Begin/End admits the product.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeProbeAdmitsPairedScope001, TestSize.Level1)
{
    ScrollPlaceholderRestrictedScope::Probe probe;
    ScrollPlaceholderRestrictedScope::Begin();
    ScrollPlaceholderRestrictedScope::End();
    probe.Close();
    EXPECT_TRUE(probe.RestrictedScopeEntered());
    EXPECT_TRUE(probe.ScopeClosed());
    EXPECT_FALSE(probe.OutOfScopeTriggered());
    EXPECT_TRUE(probe.Admitted());
}

/**
 * @tc.name: RestrictedScopeProbeRejectsMissingScope001
 * @tc.desc: A probe window without any Begin (a plain uninstrumented builder)
 *           never admits: no restricted scope was entered at all.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeProbeRejectsMissingScope001, TestSize.Level1)
{
    ScrollPlaceholderRestrictedScope::Probe probe;
    probe.Close();
    EXPECT_FALSE(probe.RestrictedScopeEntered());
    EXPECT_FALSE(probe.ScopeClosed());
    EXPECT_FALSE(probe.Admitted());
}

/**
 * @tc.name: RestrictedScopeProbeRejectsOpenScope001
 * @tc.desc: A probe window whose Begin never paired its End (a half-executed
 *           builder) does not close the pairing and never admits.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeProbeRejectsOpenScope001, TestSize.Level1)
{
    ScrollPlaceholderRestrictedScope::Probe probe;
    ScrollPlaceholderRestrictedScope::Begin();
    probe.Close();
    EXPECT_TRUE(probe.RestrictedScopeEntered());
    EXPECT_FALSE(probe.ScopeClosed());
    EXPECT_FALSE(probe.Admitted());
    ScrollPlaceholderRestrictedScope::End(); // rebalance the global depth
}

/**
 * @tc.name: RestrictedScopeProbeRejectsViolation001
 * @tc.desc: A violation reported inside the probe window (out-of-scope behavior
 *           while the restricted scope is active) rejects the admission.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeProbeRejectsViolation001, TestSize.Level1)
{
    ScrollPlaceholderRestrictedScope::Probe probe;
    ScrollPlaceholderRestrictedScope::Begin();
    ScrollPlaceholderRestrictedScope::NotifyViolation();
    ScrollPlaceholderRestrictedScope::End();
    probe.Close();
    EXPECT_TRUE(probe.RestrictedScopeEntered());
    EXPECT_TRUE(probe.ScopeClosed());
    EXPECT_TRUE(probe.OutOfScopeTriggered());
    EXPECT_FALSE(probe.Admitted());
}

/**
 * @tc.name: RestrictedScopeProductMark001
 * @tc.desc: Product marks answer per node root (null roots are inert), and the
 *           bounded registry discards the whole set on overflow (marks are
 *           advisory defense-in-depth, never correctness).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderRestrictedScopeTestNg, RestrictedScopeProductMark001, TestSize.Level1)
{
    RefPtr<UINode> nullRoot;
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsMarkedProduct(nullRoot));
    ScrollPlaceholderRestrictedScope::MarkProduct(nullRoot);    // inert on null
    ScrollPlaceholderRestrictedScope::ClearProductMark(nullRoot);

    auto root = TestPlaceholderNode::CreateTestNode(910001);
    ScrollPlaceholderRestrictedScope::MarkProduct(root);
    EXPECT_TRUE(ScrollPlaceholderRestrictedScope::IsMarkedProduct(root));
    ScrollPlaceholderRestrictedScope::ClearProductMark(root);
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsMarkedProduct(root));

    // Overflow the bounded registry: the set is cleared instead of growing unbounded.
    constexpr int32_t overflowBaseId = 920000;
    constexpr int32_t overflowCount = 2048; // beyond MAX_MARKED_PRODUCTS
    for (int32_t i = 0; i < overflowCount; i++) {
        ScrollPlaceholderRestrictedScope::MarkProduct(TestPlaceholderNode::CreateTestNode(overflowBaseId + i));
    }
    EXPECT_FALSE(ScrollPlaceholderRestrictedScope::IsMarkedProduct(
        TestPlaceholderNode::CreateTestNode(overflowBaseId)));
}

/**
 * @tc.name: ScrollPlaceholderTextCloneMeasure001
 * @tc.desc: A Text source built offline — content written into TextLayoutProperty and
 *           nothing else ever runs, the exact shape the isolated builder produces and
 *           what the device demo registers — must clone with its content intact and
 *           measure to a non-zero size after mount, like the first shell layout would.
 *           The source is the control: if even it measures zero on host typography,
 *           the measurement half of the assertion is inconclusive and only the
 *           content-copy half is binding.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ScrollPlaceholderTextCloneMeasure001, TestSize.Level1)
{
    auto source = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_TRUE(source);
    auto sourceProp = source->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_TRUE(sourceProp);
    sourceProp->UpdateContent(u"I'm placeholder");

    // Full production chain with the real copy protocol: register (the builder returns
    // the offline-built source, like RegisterTemplate stores it), probe-clone, acquire.
    manager_->useDefaultCopyProtocol = true;
    ASSERT_EQ(manager_->RegisterTemplate("tpl-text", [source]() -> RefPtr<UINode> { return source; }), 1u);
    std::string subtreePath;
    ASSERT_TRUE(manager_->ProbeTemplateClone("tpl-text", subtreePath)) << "path: " << subtreePath;
    auto copy = manager_->AcquireTemplateInstance("tpl-text");
    ASSERT_TRUE(copy);
    auto copyFrame = AceType::DynamicCast<FrameNode>(copy);
    ASSERT_TRUE(copyFrame);
    auto copyProp = copyFrame->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_TRUE(copyProp);
    // The layout property clone must carry the text content across.
    EXPECT_EQ(copyProp->GetContent().value_or(u""), u"I'm placeholder");

    LayoutConstraintF constraint;
    constraint.maxSize = { 1000.0f, 1000.0f };
    constraint.percentReference = { 1000.0f, 1000.0f };
    auto measureFrame = [&constraint](const RefPtr<FrameNode>& node) -> SizeF {
        auto wrapper = node->CreateLayoutWrapper(true, true);
        wrapper->GetLayoutProperty()->UpdateLayoutConstraint(constraint);
        wrapper->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
        wrapper->Measure(constraint);
        return wrapper->GetGeometryNode()->GetFrameSize();
    };
    auto sourceSize = measureFrame(source);
    auto copySize = measureFrame(copyFrame);
    // Host UT has no real typography (text suites measure against mock paragraphs),
    // so sizes here do not discriminate: the source control measures 0x0 as well.
    // Sizes are recorded for the log; on device the post-layout "shell live" log is
    // the authoritative discriminator (contentSize zero vs non-zero vs not painted).
    GTEST_LOG_(INFO) << "TextCloneMeasure source=" << sourceSize.Width() << "x" << sourceSize.Height()
                     << " copy=" << copySize.Width() << "x" << copySize.Height();
    EXPECT_EQ(copySize, sourceSize);
}

/**
 * @tc.name: ScrollPlaceholderDeferralTable001
 * @tc.desc: The acquisition deferral table (consumed by
 *           FrameProxy::GetFrameNodeByIndex): records key by (host, index),
 *           replaces instead of stacking, answers only for its own key, clears
 *           by index and by shell, sweeps dead entries on query, and drives the
 *           process-wide HasAnyDeferrals gate used as the FrameProxy fast path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollPlaceholderManagerTestNg, ScrollPlaceholderDeferralTable001, TestSize.Level1)
{
    const int32_t hostId = 4242;
    auto shellA = FrameNode::CreateFrameNode("deferral_shell_a",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto shellB = FrameNode::CreateFrameNode("deferral_shell_b",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_TRUE(shellA && shellB);

    // HasAnyDeferrals is process-wide and shared with other fixtures, so only the
    // per-manager table semantics are asserted here.
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 0) == nullptr);

    // Record answers only for its own (host, index) and a re-record replaces.
    manager_->RecordDeferral(hostId, 7, shellA);
    EXPECT_EQ(manager_->QueryDeferredShell(hostId, 7), shellA);
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 8) == nullptr);
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId + 1, 7) == nullptr);
    manager_->RecordDeferral(hostId, 7, shellB);
    EXPECT_EQ(manager_->QueryDeferredShell(hostId, 7), shellB);

    // Clear-by-shell removes whatever index the shell sits on (retirement funnel).
    manager_->RecordDeferral(hostId, 9, shellA);
    manager_->RemoveDeferralsByShell(hostId, shellA);
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 9) == nullptr);

    // Clear-by-index drops a single entry; the gate follows the table.
    manager_->ClearDeferral(hostId, 7);
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 7) == nullptr);

    // A dead weak reference is treated as no deferral and swept on query.
    manager_->RecordDeferral(hostId, 11, shellB);
    shellB = nullptr;
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 11) == nullptr);

    // Destroy clears everything (pipeline teardown).
    auto shellC = FrameNode::CreateFrameNode("deferral_shell_c",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    manager_->RecordDeferral(hostId, 12, shellC);
    manager_->Destroy();
    EXPECT_TRUE(manager_->QueryDeferredShell(hostId, 12) == nullptr);
}
} // namespace OHOS::Ace::NG

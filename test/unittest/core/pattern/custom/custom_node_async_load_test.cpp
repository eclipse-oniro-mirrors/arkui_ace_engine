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

#include <limits>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/utils/time_util.h"
#include "core/components_ng/base/async_load_config.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/pipeline_ng/async_load_scheduler.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEST_NODE_ID = 1001;
constexpr int32_t TEST_TIMEOUT_MS = 1000;
constexpr int64_t FAR_FUTURE_DEADLINE = std::numeric_limits<int64_t>::max();
} // namespace

class CustomNodeAsyncLoadTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }

    void SetUp() override {}
    void TearDown() override {}

    // Creates a custom node whose renderFunction builds one frame child.
    RefPtr<CustomNode> CreateCustomNode()
    {
        auto customNode = CustomNode::CreateCustomNode(TEST_NODE_ID, "AsyncLoadTestView");
        RenderFunction renderFunction = [](int64_t deadline, bool& isTimeout) -> RefPtr<UINode> {
            return FrameNode::CreateFrameNode("async_load_content", TEST_NODE_ID + 1,
                AceType::MakeRefPtr<Pattern>());
        };
        customNode->SetRenderFunction(std::move(renderFunction));
        return customNode;
    }

    void AttachToMainTree(const RefPtr<CustomNode>& node)
    {
        auto context = PipelineContext::GetCurrentContext();
        ASSERT_NE(context, nullptr);
        node->AttachToMainTree(true, AceType::RawPtr(context));
    }
};

/**
 * @tc.name: NoConfigSyncBuild001
 * @tc.desc: a component that never called enableAsyncLoad keeps the fully
 *           synchronous build path (state stays NONE, content builds immediately).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, NoConfigSyncBuild001, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    // renderFunction consumed synchronously and its child mounted
    EXPECT_EQ(customNode->renderFunction_, nullptr);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: DeferOnBuild002
 * @tc.desc: the first Build of an enabled component defers the
 *           content (PENDING, renderFunction kept, no content child, no crash).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, DeferOnBuild002, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);
    // deferred: the render function is kept for the idle/forced paths
    EXPECT_NE(customNode->renderFunction_, nullptr);
    // no placeholder configured: nothing is mounted
    EXPECT_TRUE(customNode->GetChildren().empty());
}

/**
 * @tc.name: PrebuildSyncBuild003
 * @tc.desc: a prebuild-context call chain (node detached from the
 *           main tree, e.g. LazyForEach/Repeat caches, preRender) builds
 *           synchronously; async loading does not take effect.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, PrebuildSyncBuild003, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    // deliberately NOT attached: prebuild context
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    EXPECT_EQ(customNode->renderFunction_, nullptr);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: IdleCompletion004
 * @tc.desc: idle-path completion: the idle consumer builds within the budget and the
 *           machine settles DONE with the content mounted.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, IdleCompletion004, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    ASSERT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);

    customNode->RunAsyncLoadOnIdle(FAR_FUTURE_DEADLINE);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(customNode->renderFunction_, nullptr);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: ForceLoadCompletion005
 * @tc.desc: timeout backstop: ForceLoad builds the whole content
 *           synchronously and settles DONE; a second call is a no-op (single swap
 *           exactly once).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, ForceLoadCompletion005, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    ASSERT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);

    customNode->ForceLoad();
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);

    // idempotent: DONE is terminal
    customNode->ForceLoad();
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: DetachKeepsPending006
 * @tc.desc: detaching an unfinished component keeps the machine state
 *           (resume semantics); re-attaching re-registers without a crash. The
 *           content can then still be produced by the idle/forced paths.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, DetachKeepsPending006, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    ASSERT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);

    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    customNode->DetachFromMainTree(true, AceType::RawPtr(context));
    EXPECT_TRUE(customNode->IsAsyncLoadPending());

    // remount: re-registration path (register is pipeline-side), then completion
    AttachToMainTree(customNode);
    EXPECT_TRUE(customNode->IsAsyncLoadPending());
    customNode->RunAsyncLoadOnIdle(FAR_FUTURE_DEADLINE);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::DONE);
}

/**
 * @tc.name: DeferredLayoutGuards007
 * @tc.desc: Review F3: while deferred, the layout/predict entry points must not
 *           build the content synchronously; RenderCustomChild reports unfinished.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, DeferredLayoutGuards007, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    ASSERT_TRUE(customNode->IsAsyncLoadDeferred());

    // predict path: deferred means "not finished", not a synchronous build
    EXPECT_FALSE(customNode->RenderCustomChild(FAR_FUTURE_DEADLINE));
    EXPECT_TRUE(customNode->IsAsyncLoadDeferred());

    // frame-child path: guard skips the synchronous render
    auto child = customNode->GetFrameChildByIndex(0, true, false);
    EXPECT_TRUE(customNode->IsAsyncLoadDeferred());
}

/**
 * @tc.name: UpdatePathFirstBuildDefers009
 * @tc.desc: BUG fix (if/else branch flip): a node created by a rerender pass has its
 *           first content build driven by the layout-path entries (no Build() call).
 *           The gate engages there too: on-tree + enabled => PENDING, renderFunction
 *           kept, no synchronous content child.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, UpdatePathFirstBuildDefers009, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);

    // Index-driven entry stands in for the mount-frame first build of an if/else
    // branch child / runtime-added array item.
    customNode->GetFrameChildByIndex(0, true, false);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);
    EXPECT_NE(customNode->renderFunction_, nullptr);
    EXPECT_TRUE(customNode->GetChildren().empty());

    // The predict entry reports "unfinished" instead of building synchronously.
    EXPECT_FALSE(customNode->RenderCustomChild(FAR_FUTURE_DEADLINE));
}

/**
 * @tc.name: UpdatePathPrebuildStaysSync010
 * @tc.desc: prebuild semantics preserved on the layout-path entries: the same index path reached
 *           while the node is detached (prebuild/cache chain) keeps the synchronous
 *           semantics (state stays NONE, renderFunction consumed, child mounted).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, UpdatePathPrebuildStaysSync010, TestSize.Level1)
{
    auto customNode = CreateCustomNode(); // never attached: prebuild context
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));

    customNode->GetFrameChildByIndex(0, true, false);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    EXPECT_EQ(customNode->renderFunction_, nullptr);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: ExternalRenderFunnelDefers011
 * @tc.desc: External container callers (FrameNode::FrameProxy flat wrapper building,
 *           menu/navigation hosts) invoke Render() directly, bypassing Build() and the
 *           layout entries. The gate lives at the Render funnel itself: on-tree +
 *           enabled => deferred (returns false, no content), and a direct Render() on
 *           the deferred node keeps refusing to build synchronously.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, ExternalRenderFunnelDefers011, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);

    EXPECT_FALSE(customNode->Render()); // external sync caller engages the deferral
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);
    EXPECT_NE(customNode->renderFunction_, nullptr);
    EXPECT_TRUE(customNode->GetChildren().empty());

    EXPECT_FALSE(customNode->Render()); // deferred node: still refused
    EXPECT_TRUE(customNode->IsAsyncLoadDeferred());
}

/**
 * @tc.name: NestedAsyncSuppressed012
 * @tc.desc: A custom component created inside an in-progress asynchronous build of an
 *           ancestor ignores enableAsyncLoad: the config is dropped at delivery and the
 *           node keeps fully synchronous semantics (state NONE, content built inline).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, NestedAsyncSuppressed012, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;

    AsyncLoadScheduler::EnterBuildScope();
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AsyncLoadScheduler::ExitBuildScope();

    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE); // not deferred
    EXPECT_EQ(customNode->renderFunction_, nullptr);              // built synchronously
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: NestedAsyncSuppressionSticky013
 * @tc.desc: The suppression is sticky: re-delivering the configuration after the
 *           ancestor's build finished cannot re-enable deferral, so a re-attach
 *           push never flips a born-inside child back to asynchronous.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, NestedAsyncSuppressionSticky013, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    AsyncLoadScheduler::EnterBuildScope();
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AsyncLoadScheduler::ExitBuildScope();

    AsyncLoadConfig config2; // later re-delivery outside the scope (e.g. re-attach push)
    config2.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config2)));

    AttachToMainTree(customNode);
    EXPECT_FALSE(customNode->ShouldStartAsyncLoad(customNode->IsOnMainTree()));
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: RenderFlagUnconditionalRestore014
 * @tc.desc: Render keeps needMarkParent_ false while a build is unfinished (the
 *           partially built prefix must not propagate render-tree sync) and the
 *           normal completion restores it to true unconditionally — no entry-value
 *           backup exists, so the flag can never stay poisoned across slices.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, RenderFlagUnconditionalRestore014, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AttachToMainTree(customNode);

    // Deadline already in the past: the budget-exhaustion early return fires.
    EXPECT_FALSE(customNode->Render(1));
    EXPECT_FALSE(customNode->needMarkParent_); // stays suppressed between slices

    // The completing slice restores the flag unconditionally.
    EXPECT_TRUE(customNode->Render());
    EXPECT_TRUE(customNode->needMarkParent_);
}

/**
 * @tc.name: PrebuiltMountKeepsContent015
 * @tc.desc: A component pre-built off the main tree (lazy caches, reuse-pool
 *           preRender) mounts with its build already done: the gate must not
 *           engage on it — no deferral, no placeholder beside the built content,
 *           direct display per the prebuild contract.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, PrebuiltMountKeepsContent015, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));

    // Prebuild: built synchronously while detached, renderFunction consumed.
    customNode->Build(nullptr);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    EXPECT_EQ(customNode->renderFunction_, nullptr);
    ASSERT_EQ(customNode->GetChildren().size(), 1UL);

    // Mount and drive the on-screen entries: no engagement, content stays direct.
    AttachToMainTree(customNode);
    customNode->GetFrameChildByIndex(0, true, false);
    EXPECT_EQ(customNode->asyncLoadState_, AsyncLoadState::NONE);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL); // no placeholder added
    EXPECT_FALSE(customNode->IsAsyncLoadDeferred());
}

/**
 * @tc.name: TimerMonotonic008
 * @tc.desc: the resolved timeout carried from the config is exposed for the
 *           scheduler entry snapshot at (re)registration; the countdown start
 *           itself is stamped on the entry, not on the node (the mock pipeline
 *           replaces the scheduler, so entry-level timing is covered by the
 *           pipeline scheduler tests with the real scheduler compiled in).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, TimerMonotonic008, TestSize.Level1)
{
    auto customNode = CreateCustomNode();
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    customNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    AttachToMainTree(customNode);
    customNode->Build(nullptr);
    ASSERT_EQ(customNode->asyncLoadState_, AsyncLoadState::PENDING);
    // the value the scheduler snapshots at (re)registration
    EXPECT_EQ(customNode->GetAsyncLoadResolvedTimeoutMs(), TEST_TIMEOUT_MS);
}

/**
 * @tc.name: MeasureHostDeferAndComplete001
 * @tc.desc: measure host: TryStartAsyncLoad defers (PENDING), the
 *           idle consumer builds through the pattern's render function and the
 *           machine settles DONE with the content mounted.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, MeasureHostDeferAndComplete001, TestSize.Level1)
{
    auto measureNode = CustomMeasureLayoutNode::CreateCustomMeasureLayoutNode(TEST_NODE_ID + 10, "MeasureHostView");
    // Framework-style build for a measure host: content lands as a child of the
    // stack's current top (the dummy parent created by the build slice) through
    // Push/Pop, mirroring how the JS layer mounts children; the returned root
    // alone is not mounted for this host.
    RenderFunction renderFunction = [](int64_t deadline, bool& isTimeout) -> RefPtr<UINode> {
        auto content = FrameNode::CreateFrameNode("measure_host_content", TEST_NODE_ID + 11,
            AceType::MakeRefPtr<Pattern>());
        auto* stack = ViewStackProcessor::GetInstance();
        stack->Push(content);
        stack->Pop();
        return content;
    };
    measureNode->SetRenderFunction(std::move(renderFunction));
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    measureNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    measureNode->AttachToMainTree(true, AceType::RawPtr(context));

    EXPECT_TRUE(measureNode->ShouldStartAsyncLoad());
    EXPECT_TRUE(measureNode->TryStartAsyncLoad());
    EXPECT_EQ(measureNode->asyncLoadState_, AsyncLoadState::PENDING);
    // no placeholder configured: nothing is mounted while deferred
    EXPECT_TRUE(measureNode->GetChildren().empty());

    measureNode->RunAsyncLoadOnIdle(FAR_FUTURE_DEADLINE);
    EXPECT_EQ(measureNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(measureNode->GetChildren().size(), 1UL);

    // terminal: a later force call is a no-op (single swap)
    measureNode->ForceLoad();
    EXPECT_EQ(measureNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(measureNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: MeasureHostForceLoad002
 * @tc.desc: timeout backstop on the measure host: ForceLoad builds the whole
 *           content synchronously and settles DONE.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, MeasureHostForceLoad002, TestSize.Level1)
{
    auto measureNode = CustomMeasureLayoutNode::CreateCustomMeasureLayoutNode(TEST_NODE_ID + 20, "MeasureHostView");
    // Same framework-style Push/Pop build as MeasureHostDeferAndComplete001.
    RenderFunction renderFunction = [](int64_t deadline, bool& isTimeout) -> RefPtr<UINode> {
        auto content = FrameNode::CreateFrameNode("measure_host_content", TEST_NODE_ID + 21,
            AceType::MakeRefPtr<Pattern>());
        auto* stack = ViewStackProcessor::GetInstance();
        stack->Push(content);
        stack->Pop();
        return content;
    };
    measureNode->SetRenderFunction(std::move(renderFunction));
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    measureNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    measureNode->AttachToMainTree(true, AceType::RawPtr(context));
    ASSERT_TRUE(measureNode->TryStartAsyncLoad());

    measureNode->ForceLoad();
    EXPECT_EQ(measureNode->asyncLoadState_, AsyncLoadState::DONE);
    EXPECT_EQ(measureNode->GetChildren().size(), 1UL);
}

/**
 * @tc.name: MeasureHostPrebuildKeepsSync003
 * @tc.desc: a detached (prebuild-context) measure host does not start the
 *           deferral; ShouldStartAsyncLoad/TryStartAsyncLoad both refuse.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, MeasureHostPrebuildKeepsSync003, TestSize.Level1)
{
    auto measureNode = CustomMeasureLayoutNode::CreateCustomMeasureLayoutNode(TEST_NODE_ID + 30, "MeasureHostView");
    AsyncLoadConfig config;
    config.timeoutMs = TEST_TIMEOUT_MS;
    measureNode->SetAsyncLoadConfig(std::make_shared<AsyncLoadConfig>(std::move(config)));
    // deliberately NOT attached: prebuild context
    EXPECT_FALSE(measureNode->ShouldStartAsyncLoad());
    EXPECT_FALSE(measureNode->TryStartAsyncLoad());
    EXPECT_EQ(measureNode->asyncLoadState_, AsyncLoadState::NONE);
}

/**
 * @tc.name: PlaceholderSuggestedSize001
 * @tc.desc: the suggested placeholder size acts as the placeholder root's
 *           ideal size (vp and percent both usable, percent kept symbolic).
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, PlaceholderSuggestedSize001, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetOrCreateScrollPlaceholderManager();
    ASSERT_NE(manager, nullptr);
    manager->RegisterTemplate("async_load_size_template", []() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("placeholder_root", TEST_NODE_ID + 40,
            AceType::MakeRefPtr<Pattern>());
    });

    auto customNode = CreateCustomNode();
    AttachToMainTree(customNode);
    AsyncLoadConfig config;
    config.placeholderId = "async_load_size_template";
    config.sizeWidth = Dimension(50.0, DimensionUnit::VP);
    config.sizeHeight = Dimension(10.0, DimensionUnit::PERCENT);
    auto placeholder = CustomNodeBase::AcquireAsyncLoadPlaceholder(customNode, config);
    ASSERT_NE(placeholder, nullptr);
    EXPECT_EQ(customNode->GetChildren().size(), 1UL);
    auto frame = AceType::DynamicCast<FrameNode>(placeholder);
    ASSERT_NE(frame, nullptr);
    const auto& constraint = frame->GetLayoutProperty()->GetCalcLayoutConstraint();
    ASSERT_TRUE(constraint != nullptr);
    ASSERT_TRUE(constraint->selfIdealSize.has_value());
    const auto& ideal = constraint->selfIdealSize.value();
    ASSERT_TRUE(ideal.Width().has_value());
    EXPECT_EQ(ideal.Width()->GetDimension(), Dimension(50.0, DimensionUnit::VP));
    ASSERT_TRUE(ideal.Height().has_value());
    EXPECT_EQ(ideal.Height()->GetDimension(), Dimension(10.0, DimensionUnit::PERCENT));
    manager->UnregisterTemplate("async_load_size_template");
}

/**
 * @tc.name: PlaceholderSuggestedSize002
 * @tc.desc: negative contract: a per-axis unusable suggested dimension
 *           (non-positive / non-finite) is dropped as if unset; usable axes still apply.
 */
HWTEST_F(CustomNodeAsyncLoadTestNg, PlaceholderSuggestedSize002, TestSize.Level1)
{
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetOrCreateScrollPlaceholderManager();
    ASSERT_NE(manager, nullptr);
    manager->RegisterTemplate("async_load_unusable_template", []() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("placeholder_root", TEST_NODE_ID + 41,
            AceType::MakeRefPtr<Pattern>());
    });

    auto customNode = CreateCustomNode();
    AttachToMainTree(customNode);
    AsyncLoadConfig config;
    config.placeholderId = "async_load_unusable_template";
    config.sizeWidth = Dimension(-5.0, DimensionUnit::VP);
    config.sizeHeight = Dimension(30.0, DimensionUnit::VP);
    auto placeholder = CustomNodeBase::AcquireAsyncLoadPlaceholder(customNode, config);
    ASSERT_NE(placeholder, nullptr);
    auto frame = AceType::DynamicCast<FrameNode>(placeholder);
    ASSERT_NE(frame, nullptr);
    const auto& constraint = frame->GetLayoutProperty()->GetCalcLayoutConstraint();
    ASSERT_TRUE(constraint != nullptr);
    ASSERT_TRUE(constraint->selfIdealSize.has_value());
    const auto& ideal = constraint->selfIdealSize.value();
    // negative width dropped as unusable, keeps the template's own layout on that axis
    EXPECT_FALSE(ideal.Width().has_value());
    ASSERT_TRUE(ideal.Height().has_value());
    EXPECT_EQ(ideal.Height()->GetDimension(), Dimension(30.0, DimensionUnit::VP));
    manager->UnregisterTemplate("async_load_unusable_template");
}
} // namespace OHOS::Ace::NG

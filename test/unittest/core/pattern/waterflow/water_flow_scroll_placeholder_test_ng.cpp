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

#include "water_flow_test_ng.h"

#include <algorithm>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "base/utils/time_util.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_shell_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_types.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_item_pattern.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int64_t TEST_VSYNC_PERIOD_NS = 16666667; // ~60Hz
constexpr int32_t WF_ITEM_COUNT = 20;
constexpr float WF_ITEM_HEIGHT = ITEM_MAIN_SIZE;

ScrollPlaceholderOptions MakeWaterFlowPlaceholderOptions()
{
    ScrollPlaceholderOptions options;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.height = Dimension(WF_ITEM_HEIGHT, DimensionUnit::VP);
    options.size = sizeOption;
    return options;
}

ScrollPlaceholderOptions MakeSizedWaterFlowPlaceholderOptions(const std::string& id)
{
    ScrollPlaceholderOptions options;
    options.id = id;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.width = Dimension(WATER_FLOW_WIDTH, DimensionUnit::VP);
    sizeOption.height = Dimension(WF_ITEM_HEIGHT, DimensionUnit::VP);
    options.size = sizeOption;
    return options;
}
} // namespace

// Scroll placeholder on a lazy WaterFlow: the component call-site replacement
// (WaterFlowLayoutUtils::GetWaterFlowItem -> PrepareScrollPlaceholderForAcquisition
// -> FrameProxy substitution) is the single adapter shared by the three layout
// algorithms; the pattern-level surface (decision, tables, commit/retire/sweep,
// footer exclusion) is asserted here like the List/Grid sibling suites.
class WaterFlowScrollPlaceholderTestNg : public WaterFlowTestNg {
public:
    // Anchor a frame whose derived deadline is already reached: every prediction
    // defers (remaining budget 0) regardless of the cost-model samples recorded
    // by other tests in this binary, so the placeholder path is deterministic. A
    // live anchor instead puts the deadline a long period ahead, so the stage-1
    // budget gate (aggregate estimate) keeps BUILD_REAL_NOW.
    void AnchorFrame(bool expired)
    {
        auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
        ASSERT_TRUE(manager);
        int64_t timestamp = expired ? GetSysTimestamp() - TEST_VSYNC_PERIOD_NS : GetSysTimestamp();
        int64_t period = expired ? TEST_VSYNC_PERIOD_NS : TEST_VSYNC_PERIOD_NS * 1000;
        manager->NotifyVsync(timestamp, period);
    }

    // CreateWaterFlow + scroll window + anchor + provider in one step: the
    // scaffold every deferring test starts with (the model-level setter applies
    // while the WaterFlow is under construction).
    WaterFlowModelNG CreatePlaceholderWaterFlow(ScrollPlaceholderProvider provider, bool expiredAnchor)
    {
        WaterFlowModelNG model = CreateWaterFlow();
        AnchorFrame(expiredAnchor);
        // Placeholder deferral is a scroll-time feature: seed the scroll-activity
        // window, otherwise the pattern never defers (initial viewport rule).
        pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
        model.SetScrollPlaceholder(std::move(provider));
        return model;
    }

    void CreateItemsAndDone()
    {
        CreateItemsInLazyForEach(WF_ITEM_COUNT, [](int32_t index) { return WF_ITEM_HEIGHT; });
        CreateDone();
    }

    bool IsChildOfHost(const RefPtr<FrameNode>& node) const
    {
        for (const auto& child : frameNode_->GetChildren()) {
            if (AceType::RawPtr(child) == AceType::RawPtr(node)) {
                return true;
            }
        }
        return false;
    }

    int32_t TotalCount() const
    {
        return frameNode_->GetTotalChildCount();
    }

    // First slot with neither a placeholder entry nor a built real child.
    int32_t FindUnbuiltSlot() const
    {
        for (int32_t slot = 0; slot < TotalCount(); slot++) {
            if (!pattern_->HasScrollPlaceholderItem(slot) &&
                frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(slot)) == nullptr) {
                return slot;
            }
        }
        return -1;
    }

    // First slot already holding a real child (built really, never deferred).
    int32_t FindBuiltSlot() const
    {
        for (int32_t slot = 0; slot < TotalCount(); slot++) {
            if (frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(slot)) != nullptr) {
                return slot;
            }
        }
        return -1;
    }
};

/**
 * @tc.name: WaterFlowScrollPlaceholderDeferAndDedup001
 * @tc.desc: Under an expired frame anchor with the scroll window active, the
 *           layout defers unbuilt visible slots to placeholder shells; provider
 *           indexes stay inside the host child-slot space and a second
 *           acquisition of the same index returns the same shell, never a stack.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderDeferAndDedup001, TestSize.Level1)
{
    std::vector<int32_t> providerIndexes;
    auto model = CreatePlaceholderWaterFlow(
        [&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerIndexes.push_back(index);
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    EXPECT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    for (auto index : providerIndexes) {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, TotalCount());
    }
    // Every mounted shell is a direct child of the WaterFlow host, carries a zero
    // frame count (index-space integrity) and is tracked exactly once.
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        ASSERT_TRUE(info.shell);
        EXPECT_TRUE(IsChildOfHost(info.shell));
        EXPECT_EQ(info.shell->FrameCount(), 0);
        EXPECT_EQ(pattern_->mountedPlaceholderShells_.count(info.shell), 1u);
    }
    // Same-index dedup: a repeated decision returns the existing shell.
    auto first = pattern_->scrollPlaceholderItems_.begin();
    int32_t index = first->first;
    auto shell = first->second.shell;
    EXPECT_EQ(pattern_->CreateScrollPlaceholderShell(index), shell);
    EXPECT_EQ(pattern_->scrollPlaceholderItems_.size(), pattern_->mountedPlaceholderShells_.size());
}

/**
 * @tc.name: WaterFlowScrollPlaceholderCommitAndRetire001
 * @tc.desc: The framed real-build commit drives the real child into the slot
 *           through the existing build-style acquisition; the next layout-round
 *           reconcile retires the shell and clears both tables.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderCommitAndRetire001, TestSize.Level1)
{
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    pattern_->ReconcileScrollPlaceholderItems();
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) == nullptr);
    }

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_EQ(pattern_->CreateScrollPlaceholderShell(index), shell);
    int32_t builtSlot = FindBuiltSlot();
    if (builtSlot >= 0) {
        EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(builtSlot) == nullptr);
    }

    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) != nullptr);
    EXPECT_FALSE(pattern_->retiringPlaceholderShells_.empty());

    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    EXPECT_FALSE(IsChildOfHost(shell));
}

/**
 * @tc.name: WaterFlowScrollPlaceholderOrphanShellSweep001
 * @tc.desc: A shell mounted on the host that neither placeholder table tracks
 *           anymore is swept by the layout-round reconcile; the provider reset
 *           retires committed shells awaiting reconcile, never just forgets them.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderOrphanShellSweep001, TestSize.Level1)
{
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    pattern_->ReconcileScrollPlaceholderItems();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(IsChildOfHost(shell));

    // Simulate a bookkeeping miss: the entry disappears without retiring the shell.
    pattern_->scrollPlaceholderItems_.erase(index);
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(IsChildOfHost(shell));

    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(IsChildOfHost(shell));

    // A committed shell awaiting the reconcile is retired by the reset as well.
    auto retiringShell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(retiringShell);
    pattern_->scrollPlaceholderItems_.erase(index);
    pattern_->retiringPlaceholderShells_.emplace_back(retiringShell, std::string());
    pattern_->ResetScrollPlaceholder();
    EXPECT_FALSE(IsChildOfHost(retiringShell));
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
}

/**
 * @tc.name: WaterFlowScrollPlaceholderLightLoadSkipsProvider001
 * @tc.desc: Two-stage decision, stage 1: with a live frame budget the aggregate
 *           prediction already says BUILD_REAL_NOW, so the ArkTS provider is
 *           never consulted on light-load frames.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderLightLoadSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    auto model = CreatePlaceholderWaterFlow(
        [&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerCalls++;
            return MakeWaterFlowPlaceholderOptions();
        },
        false);
    CreateItemsAndDone();

    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_GE(FindBuiltSlot(), 0);
}

/**
 * @tc.name: WaterFlowScrollPlaceholderStageTwoTemplateFlip001
 * @tc.desc: Stage 2 precision: when the aggregate estimate exceeds the budget
 *           (pressure) but the template-specific estimate of the provider's
 *           template fits, the decision flips back to BUILD_REAL_NOW.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderStageTwoTemplateFlip001, TestSize.Level1)
{
    WaterFlowModelNG model = CreateWaterFlow();
    AnchorFrame(false);
    pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    ScrollPlaceholderPredictParams mineParams;
    mineParams.componentType = ScrollPlaceholderComponentType::WATER_FLOW;
    mineParams.templateId = "wf-tpl-flip-mine";
    ScrollPlaceholderPredictParams otherParams = mineParams;
    otherParams.templateId = "wf-tpl-flip-other";
    constexpr int64_t tinyNs = 100000; // 0.1ms
    for (int i = 0; i < 20; i++) {
        manager->RecordRealBuildDuration(mineParams, tinyNs);
    }
    manager->RecordRealBuildDuration(otherParams, 500000000000LL); // 500s

    int32_t providerCalls = 0;
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        ScrollPlaceholderOptions options;
        options.id = "wf-tpl-flip-mine";
        return options;
    });
    CreateItemsAndDone();

    EXPECT_GE(providerCalls, 1);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: WaterFlowScrollPlaceholderRegisteredTemplateFlow001
 * @tc.desc: A registered template id mounts shells carrying one template instance
 *           child and the generation snapshot; commit + reconcile retires the
 *           shell and returns the instance content to the pool.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderRegisteredTemplateFlow001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("wf-tpl-shell", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeSizedWaterFlowPlaceholderOptions("wf-tpl-shell");
        },
        true);
    CreateItemsAndDone();

    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    pattern_->ReconcileScrollPlaceholderItems();
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    const auto& info = pattern_->scrollPlaceholderItems_[index];
    EXPECT_EQ(info.templateId, "wf-tpl-shell");
    EXPECT_GT(info.templateGen, 0u);
    EXPECT_EQ(info.shell->GetChildren().size(), 1u);
    EXPECT_EQ(info.shell->FrameCount(), 0);
    auto instance = AceType::DynamicCast<FrameNode>(info.shell->GetChildren().front());
    ASSERT_TRUE(instance);
    EXPECT_TRUE(instance->IsActive());

    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_GE(manager->GetCachedInstanceCount("wf-tpl-shell"), 1u);
}

/**
 * @tc.name: WaterFlowScrollPlaceholderReplaceCancelsPending001
 * @tc.desc: Replacing the provider cancels the host's pending real-builds
 *           (RE_REGISTER); a null provider then retires every shell and clears
 *           the tables.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderReplaceCancelsPending001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    ASSERT_GE(manager->GetPendingRealBuildCount(), 1u);
    auto shell = pattern_->scrollPlaceholderItems_.begin()->second.shell;
    ASSERT_TRUE(shell);

    pattern_->SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeWaterFlowPlaceholderOptions();
    });
    EXPECT_EQ(manager->GetPendingRealBuildCount(), 0u);
    EXPECT_TRUE(pattern_->IsScrollPlaceholderEnabled());

    pattern_->SetScrollPlaceholder(nullptr);
    EXPECT_FALSE(pattern_->IsScrollPlaceholderEnabled());
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
    EXPECT_FALSE(IsChildOfHost(shell));
}

/**
 * @tc.name: WaterFlowScrollPlaceholderFooterExclusion001
 * @tc.desc: Section/footer exclusion (interface contract): the footer child
 *           indices [0, footerIndex_] never enter the placeholder selection —
 *           no provider call, no shell, no deferral - while item indices above
 *           the footer defer through the same funnel as without a footer.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderFooterExclusion001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetFooter(GetDefaultHeaderBuilder());
    AnchorFrame(true);
    pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
    std::set<int32_t> providerIndexes;
    model.SetScrollPlaceholder([&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerIndexes.insert(index);
        return MakeWaterFlowPlaceholderOptions();
    });
    CreateItemsAndDone();

    ASSERT_GE(pattern_->layoutInfo_->footerIndex_, 0);
    const int32_t footerIndex = pattern_->layoutInfo_->footerIndex_;
    // The footer child indices never deferred: no placeholder entry and no
    // provider consultation below or at the footer index.
    for (int32_t slot = 0; slot <= footerIndex; slot++) {
        EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(slot));
    }
    for (auto index : providerIndexes) {
        EXPECT_GT(index, footerIndex);
    }
    // The direct decision-side call refuses the footer range explicitly.
    EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(footerIndex) == nullptr);
    EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(0) == nullptr);
    // Items above the footer still defer through the shared funnel.
    EXPECT_FALSE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: WaterFlowScrollPlaceholderFrameProxySubstitution001
 * @tc.desc: The index->wrapper substitution executes in
 *           FrameProxy::GetFrameNodeByIndex. A recorded deferral resolves the
 *           declared host acquisition to the mounted shell, an undeclared
 *           acquisition never substitutes, the built-probe stays deferral-blind,
 *           and the commit clears the deferral so the index resolves to the real
 *           child again.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderFrameProxySubstitution001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), index), shell);

    frameNode_->SetSkipSyncGeometryNode(true);
    auto acquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), true, false, true);
    ASSERT_EQ(AceType::RawPtr(acquired), AceType::RawPtr(shell));
    EXPECT_TRUE(shell->IsActive());
    EXPECT_EQ(shell->SkipSyncGeometryNode(), frameNode_->SkipSyncGeometryNode());

    // The built-probe stays deferral-blind.
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) == nullptr);

    ASSERT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(manager->QueryDeferredShell(frameNode_->GetId(), index) == nullptr);
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) != nullptr);
    auto realAcquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), false, false);
    ASSERT_TRUE(realAcquired);
    EXPECT_NE(AceType::RawPtr(realAcquired), AceType::RawPtr(shell));

    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: WaterFlowScrollPlaceholderLayoutAlgorithmParity001
 * @tc.desc: The three layout algorithms share the single adapter: after an
 *           explicit runtime switch of the layout mode, the same pattern-level
 *           deferral and the same funnel substitution keep working - the
 *           scheduling logic never forks per algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowScrollPlaceholderTestNg, WaterFlowScrollPlaceholderLayoutAlgorithmParity001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderWaterFlow(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeWaterFlowPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // Whatever mode the fixture created the flow in, switching to TOP_DOWN keeps
    // the deferral surface: the pattern defers an unbuilt slot and FrameProxy
    // substitutes the shell for a declared acquisition.
    pattern_->SetLayoutMode(WaterFlowLayoutMode::TOP_DOWN);
    pattern_->ReconcileScrollPlaceholderItems();
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), index), shell);
    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(IsChildOfHost(shell));

    // Same surface under the sliding-window algorithm.
    pattern_->SetLayoutMode(WaterFlowLayoutMode::SLIDING_WINDOW);
    int32_t second = FindUnbuiltSlot();
    ASSERT_GE(second, 0);
    auto swShell = pattern_->CreateScrollPlaceholderShell(second);
    ASSERT_TRUE(swShell);
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), second), swShell);
    auto acquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(second), true, false, true);
    EXPECT_EQ(AceType::RawPtr(acquired), AceType::RawPtr(swShell));
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}
} // namespace OHOS::Ace::NG

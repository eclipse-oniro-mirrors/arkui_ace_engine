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

#include "grid_test_ng.h"

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
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int64_t TEST_VSYNC_PERIOD_NS = 16666667; // ~60Hz
constexpr int32_t GRID_ITEM_COUNT = 20;
constexpr float GRID_ITEM_HEIGHT = 80.0f;

ScrollPlaceholderOptions MakeGridPlaceholderOptions()
{
    ScrollPlaceholderOptions options;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.height = Dimension(GRID_ITEM_HEIGHT, DimensionUnit::VP);
    options.size = sizeOption;
    return options;
}

ScrollPlaceholderOptions MakeSizedGridPlaceholderOptions(const std::string& id)
{
    ScrollPlaceholderOptions options;
    options.id = id;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.width = Dimension(120.0f, DimensionUnit::VP);
    sizeOption.height = Dimension(GRID_ITEM_HEIGHT, DimensionUnit::VP);
    options.size = sizeOption;
    return options;
}
} // namespace

// Scroll placeholder on a lazy scrollable Grid: the component call-site
// replacement (GetGridItem -> PrepareScrollPlaceholderForAcquisition ->
// FrameProxy substitution) is driven end-to-end by the grid layout itself; the
// pattern-level surface (decision, tables, commit/retire/sweep) is asserted here
// like the List sibling suite.
class GridScrollPlaceholderTestNg : public GridTestNg {
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

    // CreateGrid + scroll window + anchor + provider in one step: the scaffold
    // every deferring test starts with (the model-level setter applies while the
    // Grid is under construction).
    GridModelNG CreatePlaceholderGrid(ScrollPlaceholderProvider provider, bool expiredAnchor)
    {
        GridModelNG model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr");
        AnchorFrame(expiredAnchor);
        // Placeholder deferral is a scroll-time feature: seed the scroll-activity
        // window, otherwise the pattern never defers (initial viewport rule).
        pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
        model.SetScrollPlaceholder(std::move(provider));
        return model;
    }

    void CreateItemsAndDone()
    {
        CreateItemsInLazyForEach(GRID_ITEM_COUNT, [](uint32_t index) { return GRID_ITEM_HEIGHT; });
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

    // First slot with neither a placeholder entry nor a built real child.
    int32_t FindUnbuiltSlot() const
    {
        for (int32_t slot = 0; slot < GRID_ITEM_COUNT; slot++) {
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
        for (int32_t slot = 0; slot < GRID_ITEM_COUNT; slot++) {
            if (frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(slot)) != nullptr) {
                return slot;
            }
        }
        return -1;
    }
};

/**
 * @tc.name: GridScrollPlaceholderDeferAndDedup001
 * @tc.desc: Under an expired frame anchor with the scroll window active, the grid
 *           layout defers unbuilt visible slots to placeholder shells; provider
 *           indexes stay inside the host child-slot space and a second acquisition
 *           of the same index returns the same shell, never a stack.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderDeferAndDedup001, TestSize.Level1)
{
    std::vector<int32_t> providerIndexes;
    auto model = CreatePlaceholderGrid(
        [&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerIndexes.push_back(index);
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    EXPECT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    for (auto index : providerIndexes) {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, GRID_ITEM_COUNT);
    }
    // Every mounted shell is a direct child of the Grid host, carries a zero
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
 * @tc.name: GridScrollPlaceholderCommitAndRetire001
 * @tc.desc: The framed real-build commit drives the real child into the slot
 *           through the existing build-style acquisition and retires the shell
 *           in the same idle flush (no retiring-list deferral: unmounted from
 *           the host right away so the shell cannot paint over the real item);
 *           the next layout-round reconcile keeps both tables consistent.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderCommitAndRetire001, TestSize.Level1)
{
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // The layout cache path may have built some slots really; those indexes never
    // get a placeholder anymore (create refuses built slots) and stale entries are
    // dropped by the reconcile: the real child always wins over the shell.
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) == nullptr);
    }

    // Stage a deferred, genuinely unbuilt slot and drive the full commit flow.
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_EQ(pattern_->CreateScrollPlaceholderShell(index), shell);
    // A slot whose real child is already built never gets a placeholder.
    int32_t builtSlot = FindBuiltSlot();
    if (builtSlot >= 0) {
        EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(builtSlot) == nullptr);
    }

    // Real build: the real child enters the tree at the same slot index, and the
    // shell is retired in the same commit (List fix-chain v2: the retiring-list
    // deferral is gone — unmounted from the host right away, so no placeholder
    // keeps painting over the freshly built real item).
    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) != nullptr);
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_FALSE(IsChildOfHost(shell));

    // The reconcile keeps both tables consistent for the remaining entries.
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
}

/**
 * @tc.name: GridScrollPlaceholderOrphanShellSweep001
 * @tc.desc: A shell mounted on the host that neither placeholder table tracks
 *           anymore (a bookkeeping miss; index-based cleanup can not see shells
 *           by design) is swept by the layout-round reconcile; the provider
 *           reset retires committed shells awaiting reconcile, never just
 *           forgets them.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderOrphanShellSweep001, TestSize.Level1)
{
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);

    // Find a genuinely unbuilt slot and mount a shell on it.
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(IsChildOfHost(shell));

    // Simulate a bookkeeping miss: the entry disappears without retiring the shell.
    pattern_->scrollPlaceholderItems_.erase(index);
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(IsChildOfHost(shell));

    // The next layout-round reconcile sweeps the orphan off the host.
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
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
 * @tc.name: GridScrollPlaceholderLightLoadSkipsProvider001
 * @tc.desc: Two-stage decision, stage 1: with a live frame budget the aggregate
 *           prediction already says BUILD_REAL_NOW, so the ArkTS provider is
 *           never consulted - no per-index callback on light-load frames - and
 *           every visible slot builds real.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderLightLoadSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    auto model = CreatePlaceholderGrid(
        [&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerCalls++;
            return MakeGridPlaceholderOptions();
        },
        false);
    CreateItemsAndDone();

    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    int32_t builtSlot = FindBuiltSlot();
    EXPECT_GE(builtSlot, 0);
}

/**
 * @tc.name: GridScrollPlaceholderNoAnchorSkipsProvider001
 * @tc.desc: Without any vsync anchor the prediction keeps the legacy synchronous
 *           build path, so the provider is never consulted either.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderNoAnchorSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    GridModelNG model = CreateGrid(); // no anchor: the manager never received a vsync
    model.SetColumnsTemplate("1fr 1fr");
    pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        return MakeGridPlaceholderOptions();
    });
    CreateItemsAndDone();

    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: GridScrollPlaceholderStageTwoTemplateFlip001
 * @tc.desc: Stage 2 precision: when the aggregate estimate exceeds the budget
 *           (pressure) but the template-specific estimate of the provider's
 *           template fits, the decision flips back to BUILD_REAL_NOW - the
 *           provider is consulted, but no placeholder is mounted.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderStageTwoTemplateFlip001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    AnchorFrame(false);
    pattern_->OnScrollCallback(0.0f, SCROLL_FROM_START);
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    ScrollPlaceholderPredictParams mineParams;
    mineParams.componentType = ScrollPlaceholderComponentType::GRID;
    mineParams.templateId = "grid-tpl-flip-mine";
    ScrollPlaceholderPredictParams otherParams = mineParams;
    otherParams.templateId = "grid-tpl-flip-other";
    // Tiny samples for the provider's template, then one huge sample under another
    // template id recorded last: the per-template bucket stays tiny while the
    // aggregate EWMA (mean + deviation) sits far above the live-frame budget.
    constexpr int64_t tinyNs = 100000; // 0.1ms
    for (int i = 0; i < 20; i++) {
        manager->RecordRealBuildDuration(mineParams, tinyNs);
    }
    manager->RecordRealBuildDuration(otherParams, 500000000000LL); // 500s

    int32_t providerCalls = 0;
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        ScrollPlaceholderOptions options;
        options.id = "grid-tpl-flip-mine";
        return options;
    });
    CreateItemsAndDone();

    EXPECT_GE(providerCalls, 1);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: GridScrollPlaceholderRegisteredTemplateFlow001
 * @tc.desc: A registered template id mounts shells carrying one template instance
 *           child and the generation snapshot; commit + reconcile retires the
 *           shell and returns the instance content to the pool.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderRegisteredTemplateFlow001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("grid-tpl-shell", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeSizedGridPlaceholderOptions("grid-tpl-shell");
        },
        true);
    CreateItemsAndDone();

    // The layout cache path may have built deferred slots really in the meantime
    // (real child wins): drop those entries, then defer a genuinely unbuilt slot
    // fresh (the expired anchor still holds) and drive it through the full flow.
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    const auto& info = pattern_->scrollPlaceholderItems_[index];
    EXPECT_EQ(info.templateId, "grid-tpl-shell");
    EXPECT_GT(info.templateGen, 0u);
    // Registered template: the shell carries exactly one template instance child and
    // stays invisible to host child-slot enumerations by design.
    EXPECT_EQ(info.shell->GetChildren().size(), 1u);
    EXPECT_EQ(info.shell->FrameCount(), 0);
    // The offline-built instance subtree must be activated on mount —
    // FrameNode::SetActive does not recurse, and inactive content is dropped
    // from layout participation and render sync.
    auto instance = AceType::DynamicCast<FrameNode>(info.shell->GetChildren().front());
    ASSERT_TRUE(instance);
    EXPECT_TRUE(instance->IsActive());

    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    // The template content went back to the per-template instance pool.
    EXPECT_GE(manager->GetCachedInstanceCount("grid-tpl-shell"), 1u);
}

/**
 * @tc.name: GridScrollPlaceholderShellMeasure001
 * @tc.desc: The provider size lands on the shell as its user-defined ideal size
 *           and the template content measures through the shell's own measure;
 *           the sized template source must resolve through the grid item chain.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderShellMeasure001, TestSize.Level1)
{
    constexpr float sourceWidth = 100.0f;
    constexpr float sourceHeight = 50.0f;
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("grid-tpl-sized", []() -> RefPtr<UINode> {
        auto root = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(false));
        root->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(sourceWidth), CalcLength(sourceHeight)));
        return root;
    });
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeSizedGridPlaceholderOptions("grid-tpl-sized");
        },
        true);
    CreateItemsAndDone();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    ASSERT_EQ(shell->GetChildren().size(), 1u);

    // Node-as-wrapper: the grid algorithm drives the shell node itself through
    // FrameNode::Measure; geometry must land on the NODES.
    LayoutConstraintF constraint;
    constraint.maxSize = { 480.0f, 5000.0f };
    constraint.percentReference = { 480.0f, 5000.0f };
    shell->Measure(constraint);
    auto shellSize = shell->GetGeometryNode()->GetFrameSize();
    auto content = AceType::DynamicCast<FrameNode>(shell->GetChildren().front());
    ASSERT_TRUE(content);
    auto contentSize = content->GetGeometryNode()->GetFrameSize();

    EXPECT_FLOAT_EQ(contentSize.Width(), sourceWidth);
    EXPECT_FLOAT_EQ(contentSize.Height(), sourceHeight);
    // With a provider size the shell takes it as its ideal size (GridItem semantics).
    EXPECT_FLOAT_EQ(shellSize.Width(), 120.0f);
    EXPECT_FLOAT_EQ(shellSize.Height(), GRID_ITEM_HEIGHT);
}

/**
 * @tc.name: GridScrollPlaceholderReplaceCancelsPending001
 * @tc.desc: Replacing the provider cancels the host's pending real-builds
 *           (RE_REGISTER) while mounted shells keep serving until the reset;
 *           a null provider then retires every shell and clears the tables.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderReplaceCancelsPending001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    ASSERT_GE(manager->GetPendingRealBuildCount(), 1u);
    auto shell = pattern_->scrollPlaceholderItems_.begin()->second.shell;
    ASSERT_TRUE(shell);
    EXPECT_TRUE(IsChildOfHost(shell));

    // Provider replacement: the new provider's decisions start from a clean queue.
    // (The pattern-level entry is used because the model-level setter only applies
    // while the Grid is the node under construction.)
    pattern_->SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeGridPlaceholderOptions();
    });
    EXPECT_EQ(manager->GetPendingRealBuildCount(), 0u);
    // Mounted shells keep serving until the reset; the capability stays enabled.
    EXPECT_TRUE(pattern_->IsScrollPlaceholderEnabled());
    EXPECT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(IsChildOfHost(shell));

    // A null provider fully resets the capability and retires every mounted shell.
    pattern_->SetScrollPlaceholder(nullptr);
    EXPECT_FALSE(pattern_->IsScrollPlaceholderEnabled());
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
    EXPECT_FALSE(IsChildOfHost(shell));
    EXPECT_EQ(manager->GetPendingRealBuildCount(), 0u);
}

/**
 * @tc.name: GridScrollPlaceholderCommitEdgeBranches001
 * @tc.desc: The commit drops a placeholder whose index fell out of the data range
 *           (data shrank) and retires one whose slot is already built through the
 *           normal path; the reconcile drops out-of-range table entries.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderCommitEdgeBranches001, TestSize.Level1)
{
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // (a) Out of range at commit time: the placeholder is dropped, never mounted.
    int32_t goneIndex = GRID_ITEM_COUNT + 3;
    auto goneShell = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::GRID_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    pattern_->scrollPlaceholderItems_[goneIndex] = { goneShell, std::string(), 0 };
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(goneIndex));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(goneIndex));

    // (b) Already built through the normal path: the real child wins, the
    // placeholder is retired without touching the built child.
    int32_t builtSlot = FindBuiltSlot();
    if (builtSlot >= 0) {
        auto builtShell = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::GRID_ITEM_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
        pattern_->scrollPlaceholderItems_[builtSlot] = { builtShell, std::string(), 0 };
        EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(builtSlot));
        EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(builtSlot)) != nullptr);
    }

    // (c) The reconcile drops every out-of-range table entry.
    auto staleShell = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::GRID_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    pattern_->scrollPlaceholderItems_[GRID_ITEM_COUNT + 1] = { staleShell, std::string(), 0 };
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(GRID_ITEM_COUNT + 1));
}

/**
 * @tc.name: GridScrollPlaceholderFrameProxySubstitution001
 * @tc.desc: The index->wrapper substitution executes in
 *           FrameProxy::GetFrameNodeByIndex. A recorded deferral resolves the
 *           normal host acquisition to the mounted shell (with the standard
 *           skip-flag/active actions), an undeclared or cache acquisition never
 *           substitutes, the built-probe stays deferral-blind, and the commit
 *           clears the deferral so the index resolves to the real child again.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderFrameProxySubstitution001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(IsChildOfHost(shell));

    // The deferral table answers for the shell's host and index only.
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), index), shell);
    EXPECT_TRUE(manager->QueryDeferredShell(frameNode_->GetId() + 1, index) == nullptr);
    EXPECT_TRUE(manager->QueryDeferredShell(frameNode_->GetId(), index + 1) == nullptr);

    // The plain host acquisition substitutes the shell only when THIS acquisition
    // explicitly declares it open to substitution; the declaration path also applies
    // the standard acquisition actions, exactly like a normally acquired child.
    frameNode_->SetSkipSyncGeometryNode(true);
    auto acquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), true, false, true);
    ASSERT_EQ(AceType::RawPtr(acquired), AceType::RawPtr(shell));
    EXPECT_TRUE(shell->IsActive());
    EXPECT_EQ(shell->SkipSyncGeometryNode(), frameNode_->SkipSyncGeometryNode());

    // The built-probe stays deferral-blind: the deferred index still reads as
    // unbuilt (the shell never occupies the index slot).
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) == nullptr);

    // Commit: the deferral is dropped first, the real child is built through the
    // normal acquisition and the index resolves to it afterwards.
    ASSERT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(manager->QueryDeferredShell(frameNode_->GetId(), index) == nullptr);
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) != nullptr);
    auto realAcquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), false, false);
    ASSERT_TRUE(realAcquired);
    EXPECT_NE(AceType::RawPtr(realAcquired), AceType::RawPtr(shell));

    // Declaration gates, not just the table: with a live deferral on a second slot,
    // an acquisition WITH the declaration substitutes, and one WITHOUT it never
    // substitutes (it builds the real item, which then also wins a later declared
    // acquisition through the registration-first rule).
    int32_t second = FindUnbuiltSlot();
    if (second >= 0) {
        auto secondShell = pattern_->CreateScrollPlaceholderShell(second);
        ASSERT_TRUE(secondShell);
        EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), second), secondShell);
        auto declared = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(second), false, false, true);
        EXPECT_EQ(AceType::RawPtr(declared), AceType::RawPtr(secondShell));
        auto undeclared = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(second), false, false);
        ASSERT_TRUE(undeclared);
        EXPECT_NE(AceType::RawPtr(undeclared), AceType::RawPtr(secondShell));
        auto declaredAgain = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(second), false, false, true);
        EXPECT_NE(AceType::RawPtr(declaredAgain), AceType::RawPtr(secondShell));
    }

    // Retire funnels clear any deferral still pointing at a retired shell, and the
    // test leaves no process-wide deferral behind (the static fast gate is shared).
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);
    EXPECT_FALSE(IsChildOfHost(shell));
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: GridScrollPlaceholderGetGridItemFunnel001
 * @tc.desc: The single Grid child build call point (GetGridItem) drives the whole
 *           chain for an enabled host: decision, shell mount, deferral record and
 *           the declared acquisition that substitutes the shell - and applies the
 *           standard acquisition actions on it like a normally acquired child.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderGetGridItemFunnel001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    // The layout funnel call for an unbuilt visible index: the enabled host defers
    // it and the acquisition returns the mounted shell's wrapper.
    frameNode_->SetSkipSyncGeometryNode(true);
    auto wrapper = GridLayoutBaseAlgorithm::GetGridItem(AceType::RawPtr(frameNode_), index);
    ASSERT_TRUE(wrapper);
    auto shell = pattern_->scrollPlaceholderItems_[index].shell;
    ASSERT_TRUE(shell);
    EXPECT_EQ(AceType::RawPtr(wrapper), AceType::RawPtr(shell));
    EXPECT_TRUE(shell->IsActive());
    EXPECT_EQ(shell->SkipSyncGeometryNode(), frameNode_->SkipSyncGeometryNode());
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), index), shell);

    // The cache path of the same funnel never substitutes.
    int32_t other = FindUnbuiltSlot();
    if (other >= 0) {
        auto cacheWrapper = GridLayoutBaseAlgorithm::GetGridItem(AceType::RawPtr(frameNode_), other, false, true);
        EXPECT_NE(AceType::RawPtr(cacheWrapper), AceType::RawPtr(shell));
    }

    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: GridScrollPlaceholderUnregisteredTemplateRefused001
 * @tc.desc: A provider id that is not registered never mounts a shell: the
 *           deferral is refused (a content-less shell would render a blank
 *           cell that is neither a placeholder nor a real item) and the slot
 *           keeps the synchronous real-build path, so no third cell state and
 *           no deferred task is left behind.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderUnregisteredTemplateRefused001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    int32_t providerCalls = 0;
    auto model = CreatePlaceholderGrid(
        [&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerCalls++;
            return MakeSizedGridPlaceholderOptions("grid-tpl-missing");
        },
        true);
    CreateItemsAndDone();

    // Stage-1 pressure consults the provider, but the unregistered id refuses
    // the deferral: no shell, no deferral, no queued real-build duplication.
    EXPECT_GE(providerCalls, 1);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
    EXPECT_EQ(manager->QueryDeferredShell(frameNode_->GetId(), 0), nullptr);
    // The slots keep the normal real-build path: the visible ones built really.
    int32_t builtSlot = FindBuiltSlot();
    EXPECT_GE(builtSlot, 0);
}

/**
 * @tc.name: GridScrollPlaceholderCoverageFreezeHide001
 * @tc.desc: The layout-round reconcile suppresses the render output of a shell
 *           whose index left the just-finished round's gridMatrix_ fill coverage
 *           (frozen geometry would keep painting a stale on-screen position)
 *           while the margin band keeps it mounted; a later round that covers
 *           the index again restores the output.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollPlaceholderTestNg, GridScrollPlaceholderCoverageFreezeHide001, TestSize.Level1)
{
    auto model = CreatePlaceholderGrid(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeGridPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    pattern_->ReconcileScrollPlaceholderItems(pattern_->info_);

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(IsChildOfHost(shell));
    EXPECT_FALSE(pattern_->scrollPlaceholderItems_[index].frozenHidden);

    // A layout round whose fill coverage no longer contains the index but whose
    // window still keeps it inside the margin band (no drop, hide only).
    GridLayoutInfo round;
    round.startIndex_ = 0;
    round.endIndex_ = GRID_ITEM_COUNT - 1;
    for (int32_t slot = 0; slot < GRID_ITEM_COUNT; slot++) {
        if (slot == index) {
            continue;
        }
        round.gridMatrix_[slot / 2][slot % 2] = slot;
    }
    pattern_->ReconcileScrollPlaceholderItems(round);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(IsChildOfHost(shell));
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_[index].frozenHidden);

    // Coverage returns: the round that re-acquires the index restores the output.
    round.gridMatrix_[index / 2][index % 2] = index;
    pattern_->ReconcileScrollPlaceholderItems(round);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_FALSE(pattern_->scrollPlaceholderItems_[index].frozenHidden);

    // An empty matrix (unknown coverage) fails open: no hide, no drop.
    round.gridMatrix_.clear();
    pattern_->ReconcileScrollPlaceholderItems(round);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_FALSE(pattern_->scrollPlaceholderItems_[index].frozenHidden);
    EXPECT_TRUE(IsChildOfHost(shell));

    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}
} // namespace OHOS::Ace::NG

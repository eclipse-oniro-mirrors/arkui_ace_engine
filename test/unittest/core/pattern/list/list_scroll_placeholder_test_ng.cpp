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

#include "list_test_ng.h"

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
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int64_t TEST_VSYNC_PERIOD_NS = 16666667; // ~60Hz

ScrollPlaceholderOptions MakeDefaultPlaceholderOptions()
{
    ScrollPlaceholderOptions options;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.height = ITEM_MAIN_SIZE;
    options.size = sizeOption;
    return options;
}

ScrollPlaceholderOptions MakeSizedPlaceholderOptions(const std::string& id)
{
    ScrollPlaceholderOptions options;
    options.id = id;
    ScrollPlaceholderSizeOption sizeOption;
    sizeOption.width = ITEM_MAIN_SIZE;
    sizeOption.height = ITEM_MAIN_SIZE;
    options.size = sizeOption;
    return options;
}
} // namespace

// LazyForEach slots mixing plain ListItems (even slots) and ListItemGroups (odd slots):
// the index space the scroll placeholder provider sees must stay the host child-slot
// space regardless of what a slot expands to.
class MixedGroupItemMockLazy : public Framework::MockLazyForEachBuilder {
protected:
    int32_t OnGetTotalCount() override
    {
        return TOTAL_ITEM_NUMBER;
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, NG::LazyForEachCacheChild>& expiringItems) override
    {
        if (index % EVEN_NUMBER_MOD == 0) {
            // Plain ListItem slot, like ListItemMockLazy.
            ListItemModelNG itemModel;
            itemModel.Create();
            ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
            ViewAbstract::SetHeight(CalcLength(ITEM_MAIN_SIZE));
            ViewAbstract::SetFocusable(true);
            auto node = ViewStackProcessor::GetInstance()->Finish();
            return { std::to_string(index), node };
        }
        // ListItemGroup slot, like ListItemGroupMockLazy.
        auto listNode = ViewStackProcessor::GetInstance()->GetMainElementNode();
        auto weakList = AceType::WeakClaim(AceType::RawPtr(listNode));
        ListItemGroupModelNG groupModel;
        V2::ListItemGroupOptions groupOptions;
        groupOptions.style = V2::ListItemGroupStyle::NONE;
        groupModel.Create(groupOptions);
        auto listItemGroup = ViewStackProcessor::GetInstance()->GetMainElementNode();
        listItemGroup->SetParent(weakList);
        for (int32_t itemIndex = 0; itemIndex < GROUP_ITEM_NUMBER; ++itemIndex) {
            ListItemModelNG itemModel;
            itemModel.Create();
            ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
            ViewAbstract::SetHeight(CalcLength(ITEM_MAIN_SIZE));
            ViewAbstract::SetFocusable(true);
            ViewStackProcessor::GetInstance()->Pop();
        }
        auto groupNode = ViewStackProcessor::GetInstance()->Finish();
        return { std::to_string(index), groupNode };
    }
};

class ListScrollPlaceholderTestNg : public ListTestNg {
public:
    // Anchor a frame whose derived deadline is already reached: every prediction defers
    // (remaining budget 0) regardless of the cost-model samples recorded by other tests
    // in this binary, so the placeholder path is deterministic. A live anchor instead
    // puts the deadline a long period ahead, so the stage-1 budget gate (aggregate
    // estimate) keeps BUILD_REAL_NOW.
    void AnchorFrame(bool expired)
    {
        auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
        ASSERT_TRUE(manager);
        int64_t timestamp = expired ? GetSysTimestamp() - TEST_VSYNC_PERIOD_NS : GetSysTimestamp();
        int64_t period = expired ? TEST_VSYNC_PERIOD_NS : TEST_VSYNC_PERIOD_NS * 1000;
        manager->NotifyVsync(timestamp, period);
    }

    // CreateList + anchor + provider in one step: the scaffold every deferring test
    // starts with (the model-level setter applies while the List is under construction).
    ListModelNG CreatePlaceholderList(ScrollPlaceholderProvider provider, bool expiredAnchor)
    {
        ListModelNG model = CreateList();
        AnchorFrame(expiredAnchor);
        model.SetScrollPlaceholder(std::move(provider));
        return model;
    }

    void CreateItemsAndDone()
    {
        CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
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
        for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
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
        for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
            if (frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(slot)) != nullptr) {
                return slot;
            }
        }
        return -1;
    }

    // A zero-frame-count shell for direct table injection (never resolved by the
    // host child-slot enumeration, like every production shell).
    RefPtr<FrameNode> MakeInjectedShell()
    {
        return ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::LIST_ITEM_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    }

    void InjectTableEntry(int32_t index, const RefPtr<FrameNode>& shell, const std::string& templateId)
    {
        pattern_->scrollPlaceholderItems_[index] = { shell, templateId, 0 };
    }
};

/**
 * @tc.name: ScrollPlaceholderStackFromEndIndex001
 * @tc.desc: With stackFromEnd the provider is consulted with the transformed host
 *           child-slot index; every index stays inside [0, totalItemCount) and each
 *           deferred slot mounts exactly one placeholder shell.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderStackFromEndIndex001, TestSize.Level1)
{
    std::vector<int32_t> providerIndexes;
    auto model = CreatePlaceholderList(
        [&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerIndexes.push_back(index);
            return MakeDefaultPlaceholderOptions();
        },
        true);
    model.SetStackFromEnd(true);
    CreateItemsAndDone();

    ASSERT_FALSE(providerIndexes.empty());
    // All provider indexes live in the host child-slot space, never out of range.
    for (auto index : providerIndexes) {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, TOTAL_ITEM_NUMBER);
    }
    // stackFromEnd maps layout index 0 onto the last data slot: the first visible
    // acquisition must be the tail item.
    EXPECT_EQ(providerIndexes.front(), TOTAL_ITEM_NUMBER - 1);
    // One placeholder shell per deferred index, no duplicates.
    std::set<int32_t> uniqueIndexes(providerIndexes.begin(), providerIndexes.end());
    EXPECT_EQ(uniqueIndexes.size(), providerIndexes.size());
    EXPECT_EQ(pattern_->scrollPlaceholderItems_.size(), uniqueIndexes.size());
    for (auto index : providerIndexes) {
        EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    }
}

/**
 * @tc.name: ScrollPlaceholderGroupMixedIndex001
 * @tc.desc: Slots mixing ListItems and ListItemGroups keep a consistent index space:
 *           the provider only sees in-range slot indexes, group slots returning nullopt
 *           keep the synchronous build path, item slots defer to placeholder shells.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderGroupMixedIndex001, TestSize.Level1)
{
    std::vector<int32_t> providerIndexes;
    CreatePlaceholderList(
        [&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerIndexes.push_back(index);
            if (index % EVEN_NUMBER_MOD != 0) {
                return std::nullopt; // group slots keep the synchronous build path
            }
            return MakeDefaultPlaceholderOptions();
        },
        true);
    RefPtr<LazyForEachActuator> mockLazy = AceType::MakeRefPtr<MixedGroupItemMockLazy>();
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    LazyForEachModelNG lazyForEachModelNG;
    lazyForEachModelNG.Create(mockLazy);
    CreateDone();

    int32_t totalChildCount = frameNode_->GetTotalChildCount();
    EXPECT_EQ(totalChildCount, TOTAL_ITEM_NUMBER);
    ASSERT_FALSE(providerIndexes.empty());
    for (auto index : providerIndexes) {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, totalChildCount);
    }
    // First viewport: slot 0 (item) is deferred to a shell, slot 1 (group) built real.
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(0));
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(1) != nullptr);
    // Placeholders never sit on group slots.
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_EQ(index % EVEN_NUMBER_MOD, 0);
        EXPECT_LT(index, totalChildCount);
    }
}

/**
 * @tc.name: ScrollPlaceholderCommitAndRetire001
 * @tc.desc: The framed real-build closure builds the real item through the existing
 *           acquisition and the layout-round reconcile retires the shell from the
 *           host; placeholders left over an already-built slot are retired by the
 *           reconcile (real child wins, shell never rebuilds over it).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderCommitAndRetire001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // The layout cache path may have built some slots really; those indexes never
    // get a placeholder anymore (create refuses built slots) and stale entries are
    // dropped by the reconcile: the real child always wins over the shell.
    pattern_->ReconcileScrollPlaceholderItems();
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) == nullptr);
    }

    // Stage a deferred, genuinely unbuilt slot and drive the full commit flow.
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    // A second acquisition of the same index returns the same shell, never a stack.
    EXPECT_EQ(pattern_->CreateScrollPlaceholderShell(index), shell);
    // A slot whose real child is already built never gets a placeholder.
    int32_t builtSlot = FindBuiltSlot();
    if (builtSlot >= 0) {
        EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(builtSlot) == nullptr);
    }

    // Real build: the real child enters the tree at the same slot index.
    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(index)) != nullptr);
    EXPECT_FALSE(pattern_->retiringPlaceholderShells_.empty());

    // Reconcile retires the shell: unmounted from the host, tables cleared.
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    EXPECT_FALSE(IsChildOfHost(shell));
}

/**
 * @tc.name: ScrollPlaceholderOrphanShellSweep001
 * @tc.desc: A shell mounted on the host that neither placeholder table tracks
 *           anymore (a bookkeeping miss; index-based cleanup can not see shells
 *           by design) is swept by the layout-round reconcile; the provider
 *           reset retires committed shells awaiting reconcile, never just
 *           forgets them.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderOrphanShellSweep001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();
    pattern_->ReconcileScrollPlaceholderItems();

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
 * @tc.name: ScrollPlaceholderLightLoadSkipsProvider001
 * @tc.desc: Two-stage decision, stage 1: with a live frame budget the aggregate
 *           prediction already says
 *           BUILD_REAL_NOW, so the ArkTS provider is never consulted - no per-index
 *           callback on light-load frames - and every visible slot builds real.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderLightLoadSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    CreatePlaceholderList(
        [&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            providerCalls++;
            return MakeDefaultPlaceholderOptions();
        },
        false);
    CreateItemsAndDone();

    // Budget gate first: a plentiful remaining budget keeps the synchronous build
    // path without the provider ever being called.
    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    int32_t builtSlot = FindBuiltSlot();
    EXPECT_GE(builtSlot, 0);
}

/**
 * @tc.name: ScrollPlaceholderNoAnchorSkipsProvider001
 * @tc.desc: Without any vsync anchor the prediction keeps the legacy synchronous
 *           build path, so the provider is never consulted either.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderNoAnchorSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    ListModelNG model = CreateList(); // no anchor: the manager never received a vsync
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsAndDone();

    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: ScrollPlaceholderStageTwoTemplateFlip001
 * @tc.desc: Stage 2 precision: when the aggregate estimate exceeds the budget
 *           (pressure) but the template-specific estimate of the provider's
 *           template fits, the decision flips back to BUILD_REAL_NOW - the
 *           provider is consulted, but no placeholder is mounted.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderStageTwoTemplateFlip001, TestSize.Level1)
{
    ListModelNG model = CreateList();
    AnchorFrame(false);
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    ScrollPlaceholderPredictParams mineParams;
    mineParams.componentType = ScrollPlaceholderComponentType::LIST;
    mineParams.templateId = "tpl-flip-mine";
    ScrollPlaceholderPredictParams otherParams = mineParams;
    otherParams.templateId = "tpl-flip-other";
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
        options.id = "tpl-flip-mine";
        return options;
    });
    CreateItemsAndDone();

    // Pressure was reached, so the provider ran; the template-specific estimate
    // fits the budget, so no slot deferred to a placeholder.
    EXPECT_GE(providerCalls, 1);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: ScrollPlaceholderRegisteredTemplateFlow001
 * @tc.desc: A registered template id mounts shells carrying one template instance
 *           child and the generation snapshot; commit + reconcile retires the
 *           shell and returns the instance content to the pool.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderRegisteredTemplateFlow001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-shell", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeSizedPlaceholderOptions("tpl-shell");
        },
        true);
    CreateItemsAndDone();

    // The layout cache path may have built deferred slots really in the meantime
    // (real child wins): drop those entries, then defer a genuinely unbuilt slot
    // fresh (the expired anchor still holds) and drive it through the full flow.
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    pattern_->ReconcileScrollPlaceholderItems();
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    const auto& info = pattern_->scrollPlaceholderItems_[index];
    EXPECT_EQ(info.templateId, "tpl-shell");
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
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    // The template content went back to the per-template instance pool.
    EXPECT_GE(manager->GetCachedInstanceCount("tpl-shell"), 1u);
}

/**
 * @tc.name: ScrollPlaceholderShellMeasure001
 * @tc.desc: A template source with an explicit ideal size must measure through the
 *           exact wrapper path GetListItem hands to the list layout. A 0x0 result
 *           here means the shell measurement chain itself is broken (list side);
 *           the source size means the chain is fine and any remaining zero-size
 *           report is content-specific measurement.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderShellMeasure001, TestSize.Level1)
{
    constexpr float sourceWidth = 100.0f;
    constexpr float sourceHeight = 50.0f;
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-sized", []() -> RefPtr<UINode> {
        auto root = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(false));
        root->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(sourceWidth), CalcLength(sourceHeight)));
        return root;
    });
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeSizedPlaceholderOptions("tpl-sized");
        },
        true);
    CreateItemsAndDone();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    ASSERT_EQ(shell->GetChildren().size(), 1u);

    // Node-as-wrapper: the list algorithm drives the shell node itself through
    // FrameNode::Measure (CreateLayoutTask path); geometry must land on the NODES.
    LayoutConstraintF constraint;
    constraint.maxSize = { 1080.0f, 5000.0f };
    constraint.percentReference = { 1080.0f, 5000.0f };
    shell->Measure(constraint);
    auto shellSize = shell->GetGeometryNode()->GetFrameSize();
    auto content = AceType::DynamicCast<FrameNode>(shell->GetChildren().front());
    ASSERT_TRUE(content);
    auto contentSize = content->GetGeometryNode()->GetFrameSize();
    GTEST_LOG_(INFO) << "ShellMeasure shellNode=" << shellSize.Width() << "x" << shellSize.Height()
                     << " contentNode=" << contentSize.Width() << "x" << contentSize.Height();

    EXPECT_FLOAT_EQ(contentSize.Width(), sourceWidth);
    EXPECT_FLOAT_EQ(contentSize.Height(), sourceHeight);
    // With a provider size the shell takes it as its ideal size (ListItem semantics).
    EXPECT_FLOAT_EQ(shellSize.Width(), ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(shellSize.Height(), ITEM_MAIN_SIZE);
}

/**
 * @tc.name: ScrollPlaceholderShellMeasureNoProviderSize001
 * @tc.desc: The device demo shape: the provider returns only the template id, so the
 *           shell has no ideal size and its extent must come from measuring the content
 *           through the ListItem constraint chain. A sized template source must still
 *           measure through (device shows 0x0 there with a text source). This case
 *           decides on host whether the no-size shell constraint chain itself is broken.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderShellMeasureNoProviderSize001, TestSize.Level1)
{
    constexpr float sourceWidth = 100.0f;
    constexpr float sourceHeight = 50.0f;
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-sized-nosize", []() -> RefPtr<UINode> {
        auto root = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(false));
        root->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(sourceWidth), CalcLength(sourceHeight)));
        return root;
    });
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            ScrollPlaceholderOptions options;
            options.id = "tpl-sized-nosize";
            return options;
        },
        true);
    CreateItemsAndDone();

    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    ASSERT_EQ(shell->GetChildren().size(), 1u);

    // Node-as-wrapper, no provider size: the shell must size to its measured content.
    LayoutConstraintF constraint;
    constraint.maxSize = { 1080.0f, 5000.0f };
    constraint.percentReference = { 1080.0f, 5000.0f };
    shell->Measure(constraint);
    auto shellSize = shell->GetGeometryNode()->GetFrameSize();
    auto content = AceType::DynamicCast<FrameNode>(shell->GetChildren().front());
    ASSERT_TRUE(content);
    auto contentSize = content->GetGeometryNode()->GetFrameSize();
    GTEST_LOG_(INFO) << "ShellMeasureNoSize shellNode=" << shellSize.Width() << "x" << shellSize.Height()
                     << " contentNode=" << contentSize.Width() << "x" << contentSize.Height();

    EXPECT_FLOAT_EQ(contentSize.Width(), sourceWidth);
    EXPECT_FLOAT_EQ(contentSize.Height(), sourceHeight);
    EXPECT_FLOAT_EQ(shellSize.Width(), sourceWidth);
    EXPECT_FLOAT_EQ(shellSize.Height(), sourceHeight);
}


/**
 * @tc.name: ScrollPlaceholderReplaceCancelsPending001
 * @tc.desc: Replacing the provider cancels the host's pending real-builds
 *           (RE_REGISTER) while mounted shells keep serving until the reset;
 *           a null provider then retires every shell and clears the tables.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderReplaceCancelsPending001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
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
    // while the List is the node under construction.)
    pattern_->SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeDefaultPlaceholderOptions();
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
 * @tc.name: ScrollPlaceholderCommitEdgeBranches001
 * @tc.desc: The commit drops a placeholder whose index fell out of the data range
 *           (data shrank) and retires one whose slot is already built through the
 *           normal path; the reconcile drops out-of-range table entries.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderCommitEdgeBranches001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // (a) Out of range at commit time: the placeholder is dropped, never mounted.
    int32_t goneIndex = TOTAL_ITEM_NUMBER + 3;
    InjectTableEntry(goneIndex, MakeInjectedShell(), std::string());
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(goneIndex));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(goneIndex));

    // (b) Already built through the normal path: the real child wins, the placeholder
    // is retired without touching the built child.
    int32_t builtSlot = FindBuiltSlot();
    if (builtSlot < 0) {
        builtSlot = FindUnbuiltSlot();
        ASSERT_GE(builtSlot, 0);
        ASSERT_TRUE(frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(builtSlot), true));
    }
    ASSERT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(builtSlot)) != nullptr);
    InjectTableEntry(builtSlot, MakeInjectedShell(), std::string());
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(builtSlot));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(builtSlot));
    EXPECT_TRUE(frameNode_->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(builtSlot)) != nullptr);

    // (c) The reconcile drops entries whose index fell out of the data range.
    int32_t staleIndex = TOTAL_ITEM_NUMBER + 4;
    InjectTableEntry(staleIndex, MakeInjectedShell(), std::string());
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(staleIndex));
}

/**
 * @tc.name: ScrollPlaceholderRetirePoolMiss001
 * @tc.desc: Retiring a shell whose template content misses the instance pool
 *           (the template was unregistered before the retire) drops the content
 *           with the anomaly log instead of pooling it.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderRetirePoolMiss001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-miss", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_miss_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            ScrollPlaceholderOptions options;
            options.id = "tpl-miss";
            return options;
        },
        true);
    CreateItemsAndDone();
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    auto shell = pattern_->scrollPlaceholderItems_.begin()->second.shell;
    ASSERT_TRUE(shell);
    EXPECT_EQ(shell->GetChildren().size(), 1u); // the template instance is inside

    // Unregister before the retire: the pool entry is gone, so the content cannot
    // be pooled back and is dropped (visible through the anomaly log path).
    manager->UnregisterTemplate("tpl-miss");
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
}

/**
 * @tc.name: ScrollPlaceholderAcquisitionParity001
 * @tc.desc: A shell returned by the acquisition intercept carries the same
 *           post-acquisition state as a child of the standard path: the host's
 *           skip-sync flag is inherited on every acquisition and an
 *           addToRenderTree acquisition leaves the shell active. Sentinel for
 *           drift if FrameNode::GetOrCreateChildByIndex ever grows new
 *           standard actions the intercept has to mirror.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderAcquisitionParity001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // Host flag inherited as-is: skip=true reaches the shell through the
    // acquisition intercept, exactly like GetOrCreateChildByIndex does for a
    // normally acquired child.
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    frameNode_->SetSkipSyncGeometryNode(true);
    ListLayoutAlgorithm algorithm;
    auto shell = algorithm.GetListItem(AceType::RawPtr(frameNode_), index, true, false);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(shell->IsActive());
    EXPECT_TRUE(shell->SkipSyncGeometryNode());
    EXPECT_EQ(shell->SkipSyncGeometryNode(), frameNode_->SkipSyncGeometryNode());

    // Re-acquisition of the same deferred index keeps the parity (the dedup
    // returns the mounted shell and the standard actions re-apply idempotently).
    auto shellAgain = algorithm.GetListItem(AceType::RawPtr(frameNode_), index, true, false);
    EXPECT_EQ(shellAgain, shell);
    EXPECT_EQ(shellAgain->SkipSyncGeometryNode(), frameNode_->SkipSyncGeometryNode());

    // Host flag cleared: a fresh acquisition on another unbuilt slot inherits
    // skip=false the same way.
    frameNode_->SetSkipSyncGeometryNode(false);
    int32_t second = FindUnbuiltSlot();
    if (second >= 0) {
        auto shellTwo = algorithm.GetListItem(AceType::RawPtr(frameNode_), second, true, false);
        ASSERT_TRUE(shellTwo);
        EXPECT_FALSE(shellTwo->SkipSyncGeometryNode());
    }
}

/**
 * @tc.name: ScrollPlaceholderProviderShapeEdgeBranches001
 * @tc.desc: Provider shape edge branches: a size hint carrying width reaches the
 *           shell ideal size, an unregistered template id mounts a childless shell
 *           whose retire walks the content-miss arm, and negative / unknown
 *           indexes never acquire or commit.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderProviderShapeEdgeBranches001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            // Non-empty but never registered: the shell mounts without template content.
            return MakeSizedPlaceholderOptions("tpl-unregistered");
        },
        true);
    CreateItemsAndDone();

    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    for (const auto& [slot, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_EQ(info.templateId, "tpl-unregistered");
        // No template instance could be acquired for an unregistered id.
        EXPECT_EQ(info.shell->GetChildren().size(), 0u);
    }

    // Guard branches: a negative index never acquires, never commits; a valid
    // index without a placeholder entry never commits either.
    EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(-1) == nullptr);
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(-5));
    int32_t unknownSlot = FindUnbuiltSlot();
    ASSERT_GE(unknownSlot, 0);
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(unknownSlot));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(unknownSlot));

    // Retire with a non-empty template id but no shell children: the content
    // misses the pool and the anomaly path runs instead of pooling.
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
}

/**
 * @tc.name: ScrollPlaceholderFrameProxySubstitution001
 * @tc.desc: The index->wrapper substitution executes in
 *           FrameProxy::GetFrameNodeByIndex. A recorded deferral resolves the
 *           normal host acquisition to the mounted shell (with the standard
 *           skip-flag/active actions), a cache acquisition never substitutes,
 *           the built-probe stays deferral-blind, and the commit clears the
 *           deferral so the index resolves to the real child again.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderFrameProxySubstitution001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
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
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(IsChildOfHost(shell));
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
}

/**
 * @tc.name: ScrollPlaceholderFrameProxyRealChildWins001
 * @tc.desc: A registered real child always wins over a deferral: once the real
 *           item sits in FrameProxy's partFrameNodeChildren_, a stale deferral
 *           entry for the same index must not substitute anymore (the hook
 *           checks the registered child first).
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderFrameProxyRealChildWins001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        false); // live anchor: no deferral happens during the layout
    CreateItemsAndDone();

    // Build one slot really through the acquisition itself (no deferral recorded:
    // live anchor), which registers the real child into FrameProxy's cache; then
    // record a stale deferral for the same index pointing at a foreign shell: the
    // hook must still answer with the real child.
    int32_t index = FindUnbuiltSlot();
    ASSERT_GE(index, 0);
    auto realChild = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), false, false);
    ASSERT_TRUE(realChild);
    auto staleShell = MakeInjectedShell();
    manager->RecordDeferral(frameNode_->GetId(), index, staleShell);
    EXPECT_TRUE(ScrollPlaceholderManager::HasAnyDeferrals());
    // Even an acquisition that explicitly declares substitution openness answers
    // with the registered real child.
    auto acquired = frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(index), false, false, true);
    EXPECT_EQ(AceType::RawPtr(acquired), AceType::RawPtr(realChild));
    EXPECT_NE(AceType::RawPtr(acquired), AceType::RawPtr(staleShell));

    // Cleanup via the by-shell funnel matches the recorded shell whatever its index.
    manager->RemoveDeferralsByShell(frameNode_->GetId(), staleShell);
    EXPECT_TRUE(manager->QueryDeferredShell(frameNode_->GetId(), index) == nullptr);
    // Leave no process-wide deferral behind for later fixtures.
    pattern_->ResetScrollPlaceholder();
}

/**
 * @tc.name: ScrollPlaceholderActiveRangeImmunity001
 * @tc.desc: The shell keeps a zero frame count, so its entry in FrameProxy's child
 *           table starts at the total item count and any real viewport window is
 *           negative in its coordinates; the inherited
 *           FrameNode::DoSetActiveChildRange would read that as out-of-range and
 *           deactivate the shell on every layout round, which then drops it from
 *           the render-tree children collection. The shell must stay active
 *           through an active-range update, while a built real item in the range
 *           keeps the normal activation semantics.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderActiveRangeImmunity001, TestSize.Level1)
{
    CreatePlaceholderList(
        [](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
            return MakeDefaultPlaceholderOptions();
        },
        true);
    CreateItemsAndDone();

    // Mount one shell through the production mount path (host child, zero frame
    // count, deferral recorded), then rebuild the FrameProxy child table so the
    // shell is present in it (AddChild reset the table; the acquisition below
    // rebuilds it lazily).
    int32_t shellIndex = FindUnbuiltSlot();
    ASSERT_GE(shellIndex, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(shellIndex);
    ASSERT_TRUE(shell);
    // Exact identity: the shell carries its dedicated tag, so the active-range
    // dispatch recognizes it by tag instead of inferring from its zero frame count.
    EXPECT_EQ(shell->GetTag(), V2::SCROLL_PLACEHOLDER_SHELL_ETS_TAG);
    ASSERT_TRUE(shell->IsActive());
    auto rangeChild = frameNode_->GetOrCreateChildByIndex(0, false, false);
    ASSERT_TRUE(rangeChild);

    // A viewport window over the first items: without the fix the shell's window is
    // (start - totalCount, end - totalCount), deeply negative, which reads as
    // out-of-range and deactivates the shell.
    frameNode_->SetActiveChildRange(0, 3, 0, 0, false);
    EXPECT_TRUE(shell->IsActive()) << "index-less shell deactivated by active-range dispatch";
    // The built real item inside the window keeps the normal activation semantics.
    EXPECT_TRUE(rangeChild->IsActive());

    // Cleanup: retire the shell and leave no deferral behind for later fixtures.
    pattern_->ResetScrollPlaceholder();
    EXPECT_FALSE(ScrollPlaceholderManager::HasAnyDeferrals());
}
} // namespace OHOS::Ace::NG

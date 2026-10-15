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
#include <vector>

#include "base/utils/time_util.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_types.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_shell_node.h"
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
    // in this binary, so the placeholder path is deterministic.
    void AnchorExpiredFrame()
    {
        auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
        ASSERT_TRUE(manager);
        manager->NotifyVsync(GetSysTimestamp() - TEST_VSYNC_PERIOD_NS, TEST_VSYNC_PERIOD_NS);
    }

    // Anchor a frame whose derived deadline lies far ahead (a long anchor period, so
    // the remaining budget also survives slow emulated test runs): with the cold start
    // / small-sample estimate the stage-1 budget gate keeps BUILD_REAL_NOW.
    void AnchorLiveFrame()
    {
        auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
        ASSERT_TRUE(manager);
        manager->NotifyVsync(GetSysTimestamp(), TEST_VSYNC_PERIOD_NS * 1000);
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
    ListModelNG model = CreateList();
    model.SetStackFromEnd(true);
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerIndexes.push_back(index);
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([&providerIndexes](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerIndexes.push_back(index);
        if (index % EVEN_NUMBER_MOD != 0) {
            return std::nullopt; // group slots keep the synchronous build path
        }
        return MakeDefaultPlaceholderOptions();
    });
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
    EXPECT_TRUE(frameNode_->GetChildByIndex(1) != nullptr);
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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

    // The layout cache path may have built some slots really; those indexes never
    // get a placeholder anymore (create refuses built slots) and stale entries are
    // dropped by the reconcile: the real child always wins over the shell.
    pattern_->ReconcileScrollPlaceholderItems();
    for (const auto& [index, info] : pattern_->scrollPlaceholderItems_) {
        EXPECT_TRUE(frameNode_->GetChildByIndex(static_cast<uint32_t>(index)) == nullptr);
    }

    // Stage a deferred, genuinely unbuilt slot and drive the full commit flow.
    int32_t index = -1;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (!pattern_->HasScrollPlaceholderItem(slot) &&
            frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) == nullptr) {
            index = slot;
            break;
        }
    }
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(index));
    // A second acquisition of the same index returns the same shell, never a stack.
    EXPECT_EQ(pattern_->CreateScrollPlaceholderShell(index), shell);
    // A slot whose real child is already built never gets a placeholder.
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) != nullptr) {
            EXPECT_TRUE(pattern_->CreateScrollPlaceholderShell(slot) == nullptr);
        }
    }

    // Real build: the real child enters the tree at the same slot index.
    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    EXPECT_TRUE(frameNode_->GetChildByIndex(static_cast<uint32_t>(index)) != nullptr);
    EXPECT_FALSE(pattern_->retiringPlaceholderShells_.empty());

    // Reconcile retires the shell: unmounted from the host, tables cleared.
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    auto* shellRaw = AceType::RawPtr(shell);
    for (const auto& child : frameNode_->GetChildren()) {
        EXPECT_NE(AceType::RawPtr(child), shellRaw);
    }
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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();
    pattern_->ReconcileScrollPlaceholderItems();

    // Find a genuinely unbuilt slot and mount a shell on it.
    int32_t index = -1;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (!pattern_->HasScrollPlaceholderItem(slot) &&
            frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) == nullptr) {
            index = slot;
            break;
        }
    }
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    auto isChildOfHost = [this](const RefPtr<FrameNode>& node) {
        for (const auto& child : frameNode_->GetChildren()) {
            if (AceType::RawPtr(child) == AceType::RawPtr(node)) {
                return true;
            }
        }
        return false;
    };
    EXPECT_TRUE(isChildOfHost(shell));

    // Simulate a bookkeeping miss: the entry disappears without retiring the shell.
    pattern_->scrollPlaceholderItems_.erase(index);
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(isChildOfHost(shell));

    // The next layout-round reconcile sweeps the orphan off the host.
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(isChildOfHost(shell));

    // A committed shell awaiting the reconcile is retired by the reset as well.
    auto retiringShell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(retiringShell);
    pattern_->scrollPlaceholderItems_.erase(index);
    pattern_->retiringPlaceholderShells_.emplace_back(retiringShell, std::string());
    pattern_->ResetScrollPlaceholder();
    EXPECT_FALSE(isChildOfHost(retiringShell));
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
}

/**
 * @tc.name: ScrollPlaceholderLightLoadSkipsProvider001
 * @tc.desc: Two-stage decision, stage 1 (the fix for the 2026-10-14 device finding):
 *           with a live frame budget the aggregate prediction already says
 *           BUILD_REAL_NOW, so the ArkTS provider is never consulted - no per-index
 *           callback on light-load frames - and every visible slot builds real.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderLightLoadSkipsProvider001, TestSize.Level1)
{
    int32_t providerCalls = 0;
    ListModelNG model = CreateList();
    AnchorLiveFrame();
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

    // Budget gate first: a plentiful remaining budget keeps the synchronous build
    // path without the provider ever being called.
    EXPECT_EQ(providerCalls, 0);
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    int32_t builtCount = 0;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (frameNode_->GetChildByIndex(static_cast<uint32_t>(slot))) {
            builtCount++;
        }
    }
    EXPECT_GE(builtCount, 1);
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
    ListModelNG model = CreateList();
    // No AnchorExpiredFrame/AnchorLiveFrame: the manager never received a vsync.
    model.SetScrollPlaceholder([&providerCalls](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        providerCalls++;
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

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
    AnchorLiveFrame();
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
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-shell", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        ScrollPlaceholderOptions options;
        options.id = "tpl-shell";
        ScrollPlaceholderSizeOption sizeOption;
        sizeOption.height = ITEM_MAIN_SIZE;
        options.size = sizeOption;
        return options;
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    // The layout cache path may have built deferred slots really in the meantime
    // (real child wins): drop those entries, then defer a genuinely unbuilt slot
    // fresh (the expired anchor still holds) and drive it through the full flow.
    pattern_->ReconcileScrollPlaceholderItems();
    int32_t index = -1;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (!pattern_->HasScrollPlaceholderItem(slot) &&
            frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) == nullptr) {
            index = slot;
            break;
        }
    }
    ASSERT_GE(index, 0);
    auto shell = pattern_->CreateScrollPlaceholderShell(index);
    ASSERT_TRUE(shell);
    const auto& info = pattern_->scrollPlaceholderItems_[index];
    EXPECT_EQ(info.templateId, "tpl-shell");
    EXPECT_GT(info.templateGen, 0u);
    ASSERT_TRUE(info.shell);
    // Registered template: the shell carries exactly one template instance child and
    // stays invisible to host child-slot enumerations by design.
    EXPECT_EQ(info.shell->GetChildren().size(), 1u);
    EXPECT_EQ(info.shell->FrameCount(), 0);

    EXPECT_TRUE(pattern_->CommitScrollPlaceholderRealBuild(index));
    pattern_->ReconcileScrollPlaceholderItems();
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(index));
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    // The template content went back to the per-template instance pool.
    EXPECT_GE(manager->GetCachedInstanceCount("tpl-shell"), 1u);
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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();
    ASSERT_FALSE(pattern_->scrollPlaceholderItems_.empty());
    ASSERT_GE(manager->GetPendingRealBuildCount(), 1u);
    auto shell = pattern_->scrollPlaceholderItems_.begin()->second.shell;
    ASSERT_TRUE(shell);
    auto isChildOfHost = [this](const RefPtr<FrameNode>& node) {
        for (const auto& child : frameNode_->GetChildren()) {
            if (AceType::RawPtr(child) == AceType::RawPtr(node)) {
                return true;
            }
        }
        return false;
    };
    EXPECT_TRUE(isChildOfHost(shell));

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
    EXPECT_TRUE(isChildOfHost(shell));

    // A null provider fully resets the capability and retires every mounted shell.
    pattern_->SetScrollPlaceholder(nullptr);
    EXPECT_FALSE(pattern_->IsScrollPlaceholderEnabled());
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->retiringPlaceholderShells_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
    EXPECT_FALSE(isChildOfHost(shell));
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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        return MakeDefaultPlaceholderOptions();
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

    // (a) Out of range at commit time: the placeholder is dropped, never mounted.
    int32_t goneIndex = TOTAL_ITEM_NUMBER + 3;
    auto orphan = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_TRUE(orphan);
    pattern_->scrollPlaceholderItems_[goneIndex] = { orphan, std::string(), 0 };
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(goneIndex));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(goneIndex));

    // (b) Already built through the normal path: the real child wins, the placeholder
    // is retired without touching the built child.
    int32_t builtSlot = -1;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) != nullptr) {
            builtSlot = slot;
            break;
        }
    }
    if (builtSlot < 0) {
        // Nothing built really yet (every visible slot deferred): build one unbuilt,
        // non-deferred slot through the existing acquisition first.
        for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
            if (!pattern_->HasScrollPlaceholderItem(slot) &&
                frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) == nullptr) {
                builtSlot = slot;
                break;
            }
        }
        ASSERT_GE(builtSlot, 0);
        ASSERT_TRUE(frameNode_->GetOrCreateChildByIndex(static_cast<uint32_t>(builtSlot), true));
    }
    ASSERT_TRUE(frameNode_->GetChildByIndex(static_cast<uint32_t>(builtSlot)) != nullptr);
    auto stale = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_TRUE(stale);
    pattern_->scrollPlaceholderItems_[builtSlot] = { stale, std::string(), 0 };
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(builtSlot));
    EXPECT_FALSE(pattern_->HasScrollPlaceholderItem(builtSlot));
    EXPECT_TRUE(frameNode_->GetChildByIndex(static_cast<uint32_t>(builtSlot)) != nullptr);

    // (c) The reconcile drops entries whose index fell out of the data range.
    int32_t staleIndex = TOTAL_ITEM_NUMBER + 4;
    pattern_->scrollPlaceholderItems_[staleIndex] = { stale, std::string(), 0 };
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
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    auto manager = MockPipelineContext::GetCurrent()->GetOrCreateScrollPlaceholderManager();
    ASSERT_TRUE(manager);
    manager->RegisterTemplate("tpl-miss", ScrollPlaceholderBuilder([]() -> RefPtr<UINode> {
        return FrameNode::CreateFrameNode("tpl_miss_root", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
    }));
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        ScrollPlaceholderOptions options;
        options.id = "tpl-miss";
        return options;
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();
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
 * @tc.name: ScrollPlaceholderProviderShapeEdgeBranches001
 * @tc.desc: Provider shape edge branches: a size hint carrying width reaches the
 *           shell ideal size, an unregistered template id mounts a childless shell
 *           whose retire walks the content-miss arm, and negative / unknown
 *           indexes never acquire or commit.
 * @tc.type: FUNC
 */
HWTEST_F(ListScrollPlaceholderTestNg, ScrollPlaceholderProviderShapeEdgeBranches001, TestSize.Level1)
{
    ListModelNG model = CreateList();
    AnchorExpiredFrame();
    model.SetScrollPlaceholder([](int32_t index) -> std::optional<ScrollPlaceholderOptions> {
        ScrollPlaceholderOptions options;
        // Non-empty but never registered: the shell mounts without template content.
        options.id = "tpl-unregistered";
        ScrollPlaceholderSizeOption sizeOption;
        sizeOption.width = ITEM_MAIN_SIZE;
        sizeOption.height = ITEM_MAIN_SIZE;
        options.size = sizeOption;
        return options;
    });
    CreateItemsInLazyForEach(TOTAL_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();

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
    int32_t unknownSlot = -1;
    for (int32_t slot = 0; slot < TOTAL_ITEM_NUMBER; slot++) {
        if (!pattern_->HasScrollPlaceholderItem(slot) &&
            frameNode_->GetChildByIndex(static_cast<uint32_t>(slot)) == nullptr) {
            unknownSlot = slot;
            break;
        }
    }
    ASSERT_GE(unknownSlot, 0);
    EXPECT_FALSE(pattern_->CommitScrollPlaceholderRealBuild(unknownSlot));
    EXPECT_TRUE(pattern_->HasScrollPlaceholderItem(unknownSlot) == false);

    // Retire with a non-empty template id but no shell children: the content
    // misses the pool and the anomaly path runs instead of pooling.
    pattern_->ResetScrollPlaceholder();
    EXPECT_TRUE(pattern_->scrollPlaceholderItems_.empty());
    EXPECT_TRUE(pattern_->mountedPlaceholderShells_.empty());
}
} // namespace OHOS::Ace::NG

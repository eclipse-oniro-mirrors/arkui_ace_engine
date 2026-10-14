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
} // namespace OHOS::Ace::NG

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_TAB_LIST_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_TAB_LIST_PATTERN_H

#include <map>
#include <set>
#include <string>

#include "base/memory/referenced.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model.h"

namespace OHOS::Ace::NG {

class TabContentPattern;
class SwiperPattern;
class TabsLayoutProperty;
class TabsNode;

class TabsSideBarTabListPattern : public Pattern {
    DECLARE_ACE_TYPE(TabsSideBarTabListPattern, Pattern);

public:
    TabsSideBarTabListPattern() = default;
    ~TabsSideBarTabListPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TabsSideBarTabListLayoutAlgorithm>();
    }

    void SetSwiperController(const RefPtr<SwiperController>& controller)
    {
        swiperController_ = controller;
    }

    void SetCurrentIndex(int32_t index);
    void InitCurrentIndex(int32_t index);
    int32_t GetCurrentIndex() const
    {
        return currentIndex_;
    }

    void SetTabsNode(const RefPtr<FrameNode>& tabsNode)
    {
        tabsNode_ = tabsNode;
    }
    void SetColumnNode(const RefPtr<FrameNode>& columnNode)
    {
        columnNode_ = columnNode;
    }
    const RefPtr<FrameNode>& GetTabItemContainerNode() const
    {
        return columnNode_;
    }
    void ApplySearchFilter(
        std::function<bool(int32_t, const std::string& text)> searchFilter, const std::u16string& searchText);
    void AddOrUpdateTabListItem(const RefPtr<FrameNode>& tabContentNode, int32_t position, bool update);
    void RefreshAllTabItemColors();
    void RebuildSections();
    // Set the customization from TabsPattern. Called before RefreshSidebarTabVisibilityAndLayout
    // so the three-way dispatcher sees the latest customization on first application.
    void SetTabViewCustomization(const std::optional<TabViewCustomization>& customization)
    {
        tabViewCustomization_ = customization;
    }

    // Unified sidebar visibility + section grouping refresh.
    // Three-way dispatch based on configuration level:
    //   - Has tabViewCustomization → ApplyTabViewCustomization (Level 3)
    //   - No customization, has sections → RebuildSections (Level 1/2 grouping + visibility)
    //   - No customization, no sections → direct visibility update only (Level 1)
    // Called from: layout phase (UpdateSideBarAndSideBarDividerVisibility),
    //              search filter (ApplySearchFilter), and rebuild paths.
    void RefreshSidebarTabVisibilityAndLayout();

    // Apply centralized tab view customization (visibilityMap + sectionMap + orderMap).
    // Computes the merged sort order and re-arranges sidebar tab items accordingly.
    void ApplyTabViewCustomization(const std::optional<TabViewCustomization>& customization);

    // --- ApplyTabViewCustomization helpers ---
    // Build swiper-index -> tab item node map from current column children.
    std::map<int32_t, RefPtr<UINode>> BuildTabItemNodeMap(
        const RefPtr<FrameNode>& swiperNode, int32_t tabCount) const;
    // Remove all section header nodes from the column.
    void RemoveSectionHeaders(const RefPtr<UINode>& columnNode) const;
    // Move or mount a tab item node to the target slot, then apply visibility.
    void MoveOrMountTabItem(const RefPtr<UINode>& columnNode, const RefPtr<UINode>& node,
        int32_t slot, int32_t tabIndex, bool visible);
    // Reorder tab items without sections (no section headers).
    void ReorderWithoutSections(const RefPtr<UINode>& columnNode,
        const std::vector<int32_t>& finalOrder,
        const std::map<int32_t, RefPtr<UINode>>& tabItemNodes,
        const std::map<int32_t, bool>& visibilityMap);
    // Reorder tab items with section headers inserted at group boundaries.
    void ReorderWithSections(const RefPtr<UINode>& columnNode,
        const std::vector<int32_t>& finalOrder,
        const std::map<int32_t, RefPtr<UINode>>& tabItemNodes,
        const std::map<int32_t, bool>& visibilityMap,
        const std::optional<TabViewCustomization>& customization);
    // Insert a section header node at the given slot and attach click handler.
    int32_t InsertSectionHeader(const RefPtr<UINode>& columnNode,
        const std::string& title, int32_t slot,
        SidebarSectionTitleStyle titleStyle = SidebarSectionTitleStyle::STANDARD);
    // Compute effective visibility for a tab, considering section collapse state.
    bool ComputeEffectiveVisibility(int32_t tabIndex, bool baseVisible,
        const std::string& currentSection, bool inSection) const;
    // Capture old Y offsets of tab items and section headers for animation.
    void CaptureOldOffsetsForAnimation(const RefPtr<UINode>& columnNode,
        std::map<int32_t, float>& oldTabOffsets,
        std::map<std::string, float>& oldHeaderOffsets) const;
    // Collect section titles that have at least one base-visible (non-collapsed) tab.
    std::set<std::string> ComputeSectionsWithVisibleTab(const std::vector<int32_t>& finalOrder,
        const std::map<int32_t, bool>& visibilityMap,
        const std::optional<TabViewCustomization>& customization) const;
    // Clear residual transform/opacity left by previous collapse/expand animations.
    void ClearResidualTransform(const RefPtr<UINode>& node);
    // Schedule expand/collapse animation after layout pass for the pending section.
    void ScheduleSectionAnimation(const std::vector<int32_t>& finalOrder,
        const std::optional<TabViewCustomization>& customization,
        std::map<int32_t, float>& oldTabOffsets,
        std::map<std::string, float>& oldHeaderOffsets);

    // Returns true if the search filter hides the tab at tabIndex.
    // If no search is active, returns false.
    bool IsHiddenBySearchFilter(int32_t tabIndex) const;

    // Check whether a tab should be hidden by defaultVisibility.
    // Returns true if the tab should be GONE per defaultVisibility rules.
    bool IsHiddenByDefaultVisibility(int32_t tabIndex) const;

    RefPtr<UINode> GetBuilderByContentId(int32_t tabContentId, const RefPtr<UINode>& builderNode)
    {
        auto iter = builderNode_.find(tabContentId);
        if (iter == builderNode_.end()) {
            builderNode_.try_emplace(tabContentId, builderNode);
            return nullptr;
        }
        auto result = iter->second;
        iter->second = builderNode;
        return result;
    }

    void RemoveBuilderByContentId(int32_t tabContentId)
    {
        builderNode_.erase(tabContentId);
    }

private:
    void OnModifyDone() override;
    void OnFontScaleConfigurationUpdate() override;
    void UpdateTabItemStyle(int32_t index, bool selected);
    RefPtr<FrameNode> GetOrCreateTabItemNode(int32_t id);
    void UpdateTabItemTextAndIconColor(int32_t selectedIndex);
    void UpdateTextColorAndIconColor(const RefPtr<FrameNode>& rowNode, Color textColor, Color iconColor,
        bool isSelected, const RefPtr<TabContentPattern>& tabContentPattern);
    void AddOrUpdateTabItemWithContent(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, int32_t tabContentId, int32_t myIndex);
    void AddOrUpdateTabItemWithBuilder(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, int32_t tabContentId, int32_t myIndex);
    void CreateOrUpdateTabItemTextAndIcon(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode,
        const RefPtr<TabContentPattern>& tabContentPattern, int32_t position,
        RefPtr<FrameNode>& iconNode, RefPtr<FrameNode>& textNode);
    void UpdateTabBarItemTextProperties(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& textNode, const RefPtr<SwiperPattern>& swiperPattern,
        const RefPtr<TabContentPattern>& tabContentPattern, int32_t myIndex, int32_t indicator);
    void UpdateTabBarItemIconProperties(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& iconNode,
        const RefPtr<TabContentPattern>& tabContentPattern, int32_t myIndex, int32_t indicator);
    void AddOrUpdateTabItemWithIconAndText(
        const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, const RefPtr<FrameNode>& swiperNode,
        const RefPtr<TabContentPattern>& tabContentPattern, int32_t position, int32_t myIndex);
    struct SectionInfo {
        std::string title;
        std::optional<int32_t> order;
        SidebarSectionTitleStyle titleStyle = SidebarSectionTitleStyle::STANDARD;
        int32_t firstVisibleTabIndex = -1;
        std::vector<int32_t> tabIndices;
    };
    void BuildSectionGroups(std::vector<SectionInfo>& sections, std::vector<int32_t>& unsectionedIndices);
    void SortSections(std::vector<SectionInfo>& sections);
    RefPtr<FrameNode> CreateOrUpdateSectionHeader(const std::string& title, int32_t sectionId,
        SidebarSectionTitleStyle titleStyle = SidebarSectionTitleStyle::STANDARD);
    void ApplySectionLayout(const std::vector<SectionInfo>& sections,
        const std::vector<int32_t>& unsectionedIndices);
    // Play expand/collapse animation after section layout rebuild.
    // collapsedTabIndices: tab items that just became GONE (collapse) — animate from old pos to header pos, fade out.
    // expandedTabIndices: tab items that just became VISIBLE (expand) — animate from header pos to new pos, fade in.
    // pushedTabOldOffsets: tab items that remained visible but moved — animate from old offset to new offset.
    // sectionHeaderOffsets: per-section-title Y offset of the header (relative to column) used as animation
    // origin/target.
    void PlaySectionExpandCollapseAnimation(const std::map<int32_t, float>& oldTabOffsets,
        const std::set<int32_t>& animatedTabIndices, const std::string& triggeredSectionTitle,
        bool isCollapse, const std::map<std::string, float>& oldHeaderOffsets);
    // Play arrow morph animation (vertex moves up↔down) on section header.
    // progress 0.0 = arrow up (^), 1.0 = arrow down (v).
    void PlayArrowAnimation(const RefPtr<FrameNode>& arrowNode, bool isCollapse);
    void UpdateArrowPoints(const RefPtr<FrameNode>& arrowNode, float progress);
    bool IsTabVisible(int32_t tabIndex) const;
    RefPtr<FrameNode> FindTabItemNodeBySwiperIndex(int32_t swiperIndex) const;
    // Find a section header node in columnNode by matching its text child's content to title.
    // Returns nullptr if not found. The header structure is: children[0]=text, children[1]=arrow.
    RefPtr<FrameNode> FindSectionHeaderByTitle(const std::string& title) const;
    // Get the title text from a section header node's text child. Returns empty string on failure.
    std::string GetSectionHeaderTitle(const RefPtr<FrameNode>& headerNode) const;
    // Toggle collapse state and play arrow + section expand/collapse animation for a section.
    void HandleSectionHeaderClick(const std::string& title);
    // Check if a node at nodeY is positioned below any of the animated tab items.
    bool IsNodeBelowAnimatedSection(float nodeY, const std::set<int32_t>& animatedTabIndices) const;
    // Find the swiper index corresponding to a tab item frame node. Returns -1 if not found.
    int32_t FindSwiperIndexForTabItem(const RefPtr<FrameNode>& tabItemNode) const;

    // Merged sort helpers
    // Step 1: Build sortedOrder — indices sorted by orderMap priority (configured first ascending,
    //         unconfigured after, ties by original swiper order).
    std::vector<int32_t> BuildSortedOrder(
        int32_t tabCount, const std::optional<TabViewCustomization>& customization) const;
    // Step 2: Precompute visibility for each tab (visibilityMap override > defaultVisibility > search filter).
    //   Does NOT affect sort/group — only used during node remounting.
    std::map<int32_t, bool> ComputeVisibilityMap(
        int32_t tabCount, const std::optional<TabViewCustomization>& customization) const;
    // Step 3+4+5: Group by section and produce final display order.
    //   Unsectioned first (in sortedOrder order), then sections (internal sortedOrder order),
    //   inter-section sorted by SidebarSection.order (has-order first ascending,
    //   no-order after; ties broken by first tab's sortedOrder position).
    std::vector<int32_t> GroupAndSortSections(
        const std::vector<int32_t>& sortedOrder, const std::optional<TabViewCustomization>& customization,
        bool& hasSection) const;
    // Get effective visibility for a tab: visibilityMap override > defaultVisibility > search filter.
    bool IsTabVisibleWithCustomization(
        int32_t tabIndex, const std::optional<TabViewCustomization>& customization) const;
    // Get effective section for a tab: sectionMap override > original sidebarSection.
    // Returns nullopt or empty title => unsectioned.
    std::optional<std::string> GetEffectiveSectionTitle(
        int32_t tabIndex, const std::optional<TabViewCustomization>& customization) const;
    std::optional<int32_t> GetEffectiveSectionOrder(
        int32_t tabIndex, const std::optional<TabViewCustomization>& customization) const;
    // Get effective section title style: sectionMap override > original sidebarSection.
    SidebarSectionTitleStyle GetEffectiveSectionTitleStyle(
        int32_t tabIndex, const std::optional<TabViewCustomization>& customization) const;
    // Level 1 helper: directly set each tab item visibility based on
    // defaultVisibility + search filter (no section grouping, no reorder).
    void UpdateDirectVisibility(const RefPtr<FrameNode>& swiperNode);

    WeakPtr<FrameNode> tabsNode_ = nullptr;
    RefPtr<FrameNode> columnNode_ = nullptr;
    RefPtr<SwiperController> swiperController_ = nullptr;
    int32_t currentIndex_ = -1;
    std::map<int32_t, RefPtr<UINode>> builderNode_;
    // Active search state (set by ApplySearchFilter, consumed by IsHiddenBySearchFilter)
    std::function<bool(int32_t, const std::string& text)> activeSearchFilter_;
    std::u16string activeSearchText_;
    // Section header collapse state: key = section title hash, value = collapsed
    std::map<std::string, bool> sectionCollapsedState_;
    // Tracks the section title whose collapse/expand was just toggled, to drive the animation.
    // Cleared after the animation is scheduled. Empty means no animation (e.g. initial build).
    std::string pendingAnimationSection_;
    // True if the pending animation is a collapse (hide tabs), false if expand (show tabs).
    bool pendingAnimationIsCollapse_ = false;
    // True while a section expand/collapse animation is running — ignore click events.
    bool sectionAnimationRunning_ = false;

    // Cached customization for incremental rebuilds (e.g. section collapse toggle).
    std::optional<TabViewCustomization> tabViewCustomization_;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_TAB_LIST_PATTERN_H

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
    void ApplyDefaultVisibility();
    void RefreshAllTabItemColors();
    void RebuildSections();

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

    WeakPtr<FrameNode> tabsNode_ = nullptr;
    RefPtr<FrameNode> columnNode_ = nullptr;
    RefPtr<SwiperController> swiperController_ = nullptr;
    int32_t currentIndex_ = -1;
    std::map<int32_t, RefPtr<UINode>> builderNode_;
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
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_TAB_LIST_PATTERN_H

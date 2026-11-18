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

#include "core/components_ng/pattern/tabs/tab_content_pattern.h"

#include "core/components_ng/manager/material/material_processor.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"

namespace OHOS::Ace::NG {

void TabContentPattern::OnAttachToMainTree()
{
    auto hostNode = AceType::DynamicCast<FrameNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto tabs = TabContentModelNG::FindTabsNode(hostNode);
    CHECK_NULL_VOID(tabs);
    auto swiper = tabs->GetTabs();
    CHECK_NULL_VOID(swiper);
    auto indexPair = swiper->GetChildFlatIndex(hostNode->GetId());
    if (!indexPair.first) {
        return;
    }
    GenerateGlobalComponentId(std::to_string(indexPair.second));
}

void TabContentPattern::OnModifyDone()
{
    Pattern::OnModifyDone();

    const auto& defaultVisibility = GetDefaultVisibility();
    const auto& sidebarSection = GetSidebarSection();
    bool visibilityChanged = !(defaultVisibility == lastAppliedVisibility_);
    bool sectionChanged = !(sidebarSection == lastAppliedSidebarSection_);
    if (!visibilityChanged && !sectionChanged) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabContentNode = AceType::DynamicCast<TabContentNode>(host);
    CHECK_NULL_VOID(tabContentNode);
    lastAppliedVisibility_ = defaultVisibility;
    lastAppliedSidebarSection_ = sidebarSection;

    auto tabsNode = AceType::DynamicCast<TabsNode>(TabContentModelNG::FindTabsNode(host));
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto activeDisplayMode = tabsPattern->GetActiveBarDisplayMode();
    bool shouldHide = defaultVisibility.ShouldHide(activeDisplayMode);

    if (activeDisplayMode == TabBarDisplayMode::BOTTOMTABBAR || activeDisplayMode == TabBarDisplayMode::TOP_TABBAR) {
        HandleBottomTopTabBar(tabContentNode, visibilityChanged, shouldHide);
        return;
    }

    // Sidebar mode.
    // If the sidebar tab item hasn't been created yet, nothing to update.
    if (!tabContentNode->HasSideBarTabBarItemId()) {
        return;
    }

    if (HandleSideBarCustomization(tabsPattern, tabContentNode, tabsNode, visibilityChanged, sectionChanged)) {
        return;
    }
    UpdateSideBarTabItem(tabsPattern, tabContentNode, tabsNode, shouldHide);
}

void TabContentPattern::UpdateNodeVisibility(const RefPtr<FrameNode>& node, bool shouldHide)
{
    CHECK_NULL_VOID(node);
    auto property = node->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    property->UpdateVisibility(shouldHide ? VisibleType::GONE : VisibleType::VISIBLE);
    node->MarkNeedSyncRenderTree();
    node->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabContentPattern::HandleBottomTopTabBar(
    const RefPtr<TabContentNode>& tabContentNode, bool visibilityChanged, bool shouldHide)
{
    // Bottom/top tab bar — only visibility matters, sections are irrelevant.
    if (!visibilityChanged) {
        return;
    }
    if (!tabContentNode->HasTabBarItemId()) {
        return;
    }
    auto columnNode = AceType::DynamicCast<FrameNode>(
        ElementRegister::GetInstance()->GetUINodeById(tabContentNode->GetTabBarItemId()));
    CHECK_NULL_VOID(columnNode);
    auto columnProperty = columnNode->GetLayoutProperty();
    CHECK_NULL_VOID(columnProperty);
    columnProperty->UpdateVisibility(shouldHide ? VisibleType::GONE : VisibleType::VISIBLE);
    columnNode->MarkNeedSyncRenderTree();
    columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

bool TabContentPattern::HandleSideBarCustomization(const RefPtr<TabsPattern>& tabsPattern,
    const RefPtr<TabContentNode>& tabContentNode, const RefPtr<TabsNode>& tabsNode,
    bool visibilityChanged, bool sectionChanged)
{
    CHECK_NULL_RETURN(tabsPattern, false);
    CHECK_NULL_RETURN(tabsNode, false);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, false);
    int32_t tabIndex = swiperNode->GetChildIndex(tabContentNode);
    const auto& customization = tabsPattern->GetAppliedTabViewCustomization();

    // If TabViewCustomization overrides this tab's changed property, the
    // property change is irrelevant — the merged result is driven by the
    // customization. Skip the direct update; the customization will be (or has
    // already been) applied via UpdateSideBarIfNeeded → ApplyTabViewCustomization
    // during TabsPattern::OnModifyDone.
    // (At this point at least one of visibilityChanged/sectionChanged is true,
    //  guaranteed by the early-return at the top of this function.)
    if (customization.has_value()) {
        bool visibilityOverridden = customization->visibilityMap.find(tabIndex) != customization->visibilityMap.end();
        bool sectionOverridden = customization->sectionMap.find(tabIndex) != customization->sectionMap.end();
        if (visibilityChanged && visibilityOverridden) {
            visibilityChanged = false;
        }
        if (sectionChanged && sectionOverridden) {
            sectionChanged = false;
        }
        if (!visibilityChanged && !sectionChanged) {
            return true; // both changed properties are overridden — nothing to do
        }
    }

    // Section change (not overridden) requires a full sidebar rebuild for
    // regrouping. The direct tabItemNode update below only handles visibility.
    if (sectionChanged) {
        tabsPattern->RefreshSideBarTabView();
        return true;
    }

    // Only visibility changed (and not overridden).
    // If this tab belongs to a section, a visibility change may cause the entire
    // section to become empty (all tabs hidden) — in which case the section header
    // must also be hidden. That requires a full sidebar rebuild.
    // Note: customization sectionMap takes precedence over original sidebarSection.
    // If sectionMap overrides this tab with an empty title, the tab is explicitly
    // unsectioned — even if the original sidebarSection has a non-empty title.
    bool hasSection = false;
    if (customization.has_value()) {
        auto it = customization->sectionMap.find(tabIndex);
        if (it != customization->sectionMap.end()) {
            // sectionMap overrides — empty title means explicitly unsectioned
            hasSection = !it->second.title.empty();
        }
    }
    if (!hasSection) {
        // Fall back to original sidebarSection
        const auto& sidebarSection = GetSidebarSection();
        if (sidebarSection.has_value() && !sidebarSection->title.empty()) {
            hasSection = true;
        }
    }
    if (hasSection) {
        tabsPattern->RefreshSideBarTabView();
        return true;
    }
    return false;
}

void TabContentPattern::UpdateSideBarTabItem(const RefPtr<TabsPattern>& tabsPattern,
    const RefPtr<TabContentNode>& tabContentNode, const RefPtr<TabsNode>& tabsNode, bool shouldHide)
{
    // Not in any section — update this tab's sidebar tabItemNode directly.
    auto tabItemNode = AceType::DynamicCast<FrameNode>(
        ElementRegister::GetInstance()->GetUINodeById(tabContentNode->GetSideBarTabBarItemId()));
    CHECK_NULL_VOID(tabItemNode);
    auto property = tabItemNode->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    do {
        if (shouldHide) {
            break;
        }
        // defaultVisibility says visible — but if a search filter is active, the tab
        // may still need to be hidden by the search criteria.
        auto sideBarNode = tabsPattern->GetSideBarNode();
        CHECK_NULL_BREAK(sideBarNode);
        auto sidebarPattern = sideBarNode->GetPattern<TabsSideBarPattern>();
        CHECK_NULL_BREAK(sidebarPattern);
        auto tabListNode = sidebarPattern->GetTabListNode();
        CHECK_NULL_BREAK(tabListNode);
        auto tabListPattern = tabListNode->GetPattern<TabsSideBarTabListPattern>();
        CHECK_NULL_BREAK(tabListPattern);
        auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
        CHECK_NULL_BREAK(swiperNode);
        int32_t tabIndex = swiperNode->GetChildIndex(tabContentNode);
        shouldHide = tabListPattern->IsHiddenBySearchFilter(tabIndex);
    } while (false);
    property->UpdateVisibility(shouldHide ? VisibleType::GONE : VisibleType::VISIBLE);
    tabItemNode->MarkNeedSyncRenderTree();
    tabItemNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabContentPattern::DumpAdvanceInfo(std::unique_ptr<JsonValue>& json)
{
    TabContentPattern::DumpAdvanceInfo(json, ParamConfig());
}

void TabContentPattern::DumpAdvanceInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config)
{
    if (!config.IsPublicTree() || config.visualProperties) {
        switch (selectedMode_) {
            case SelectedMode::INDICATOR: {
                json->Put("SelectedMode", "INDICATOR");
                break;
            }
            case SelectedMode::BOARD: {
                json->Put("SelectedMode", "BOARD");
                break;
            }
            default: {
                break;
            }
        }
    }
}

} // namespace OHOS::Ace::NG

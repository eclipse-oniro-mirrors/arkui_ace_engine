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

#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"

#include <algorithm>

#include "base/utils/utils.h"
#include "base/utils/utf_helper.h"
#include "base/log/log_wrapper.h"
#include "core/animation/spring_curve.h"
#include "core/animation/curves.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/shape/polygon_model_ng.h"
#include "core/components_ng/pattern/shape/polygon_pattern.h"
#include "core/components_ng/pattern/shape/shape_abstract_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/pipeline/pipeline_context.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/interfaces/native/node/node_button_modifier.h"

namespace OHOS::Ace::NG {
namespace {
const Dimension SIDEBAR_TAB_ITEM_MARGIN = 8.0_vp;
constexpr float SIDEBAR_TAB_MAX_FONT_SCALE = 2.0f;
constexpr float SIDEBAR_TAB_FONT_SCALE_THRESHOLD_LEVEL8 = 1.75f;
constexpr float SIDEBAR_TAB_FONT_SCALE_THRESHOLD_LEVEL10 = 2.0f;
const Dimension SIDEBAR_TAB_ITEM_FOCUS_WIDTH = 2.0_vp;

// Get vertical margin for text node based on font scale.
// Default: padding_level4, >=1.75: padding_level8, >=2.0: padding_level10
Dimension GetVerticalMarginForFontScale(float fontScale, const RefPtr<TabTheme>& tabTheme)
{
    if (GreatOrEqual(fontScale, SIDEBAR_TAB_FONT_SCALE_THRESHOLD_LEVEL10)) {
        return tabTheme->GetSideBarPaddingLevel10();
    } else if (GreatOrEqual(fontScale, SIDEBAR_TAB_FONT_SCALE_THRESHOLD_LEVEL8)) {
        return tabTheme->GetSideBarPaddingLevel8();
    }
    return tabTheme->GetSideBarPaddingLevel4();
}

std::u16string ToLowerU16Str(std::u16string str)
{
    for (auto& ch : str) {
        if (ch >= u'A' && ch <= u'Z') {
            ch += (u'a' - u'A');
        }
    }
    return str;
}
}

void TabsSideBarTabListPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
}

void TabsSideBarTabListPattern::OnFontScaleConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto tabTheme = host->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto columnNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(columnNode);
    auto fontScale = context->GetFontScaleFromEnv(host);
    auto verticalMargin = GetVerticalMarginForFontScale(fontScale, tabTheme);
    for (const auto& child : columnNode->GetChildren()) {
        auto tabItemNode = AceType::DynamicCast<FrameNode>(child);
        CHECK_NULL_CONTINUE(tabItemNode);
        // Only update icon+text type tab items (Button > Row > Image + Text).
        // Skip custom builder/content tab items — their layout is user-defined.
        const auto& itemChild = tabItemNode->GetChildren();
        if (itemChild.empty() || itemChild.front()->GetTag() != V2::ROW_ETS_TAG) {
            continue;
        }
        // Update text node top/bottom margin based on current font scale
        auto rowNode = AceType::DynamicCast<FrameNode>(itemChild.front());
        CHECK_NULL_CONTINUE(rowNode);
        constexpr size_t ICON_AND_TEXT_SIZE = 2;
        if (rowNode->GetChildren().size() < ICON_AND_TEXT_SIZE) {
            continue;
        }
        auto textNode = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().back());
        CHECK_NULL_CONTINUE(textNode);
        auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_CONTINUE(textLayoutProperty);
        MarginProperty textMargin;
        textMargin.left = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
        textMargin.right = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
        textMargin.top = CalcLength(verticalMargin);
        textMargin.bottom = CalcLength(verticalMargin);
        textLayoutProperty->UpdateMargin(textMargin);
        tabItemNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

void TabsSideBarTabListPattern::SetCurrentIndex(int32_t index)
{
    if (currentIndex_ == index) {
        return;
    }
    int32_t prevIndex = currentIndex_;
    currentIndex_ = index;

    // Cancel old selected item highlight
    UpdateTabItemStyle(prevIndex, false);
    // Set new selected item highlight
    UpdateTabItemStyle(index, true);

    // Notify Swiper to switch page.
    // In sidebar display mode the system default is to switch pages without animation.
    // Only when the developer has configured a custom content transition
    // (customContentTransition, reflected as TabsPattern::isCustomAnimation_,
    // kept in sync with SwiperLayoutProperty::IsCustomAnimation by TabsModelNG::
    // SetIsCustomAnimation) do we honor the animated path — updating the TabBar
    // cubic curve and calling SwipeTo so that the developer's custom animation is
    // invoked. Otherwise we jump directly via SwipeToWithoutAnimation to avoid the
    // default sliding animation.
    if (swiperController_) {
        bool useCustomAnimation = false;
        auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
        if (tabsNode) {
            auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
            if (tabsPattern) {
                useCustomAnimation = tabsPattern->GetIsCustomAnimation();
            }
        }
        if (useCustomAnimation) {
            const auto& updateCubicCurveCallback = swiperController_->GetUpdateCubicCurveCallback();
            if (updateCubicCurveCallback) {
                updateCubicCurveCallback();
            }
            swiperController_->SwipeTo(index);
        } else {
            swiperController_->SwipeToWithoutAnimation(index);
        }
    }

    UpdateTabItemTextAndIconColor(index);
}

void TabsSideBarTabListPattern::UpdateTabItemTextAndIconColor(int32_t selectedIndex)
{
    auto host = AceType::DynamicCast<FrameNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto tabTheme = host->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    int32_t tabContentNum = swiperNode->TotalChildCount();

    for (int32_t index = 0; index < tabContentNum; index++) {
        auto tabItemNode = FindTabItemNodeBySwiperIndex(index);
        CHECK_NULL_CONTINUE(tabItemNode);
        auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(index));
        CHECK_NULL_CONTINUE(tabContentNode);
        auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
        CHECK_NULL_CONTINUE(tabContentPattern);
        const auto& tabBarParam = tabContentPattern->GetTabBarParam();

        if (tabBarParam.HasContent() || tabBarParam.HasBuilder()) {
            continue;
        }

        bool isSelected = (index == selectedIndex);
        Color textColor = isSelected ? tabTheme->GetSideBarSelectedTextColor() :
            tabTheme->GetSideBarUnselectedTextColor();
        Color iconColor = isSelected ? tabTheme->GetSideBarSelectedIconColor() :
            tabTheme->GetSideBarUnselectedIconColor();
        auto rowNode = AceType::DynamicCast<FrameNode>(tabItemNode->GetChildren().front());
        CHECK_NULL_CONTINUE(rowNode);
        UpdateTextColorAndIconColor(rowNode, textColor, iconColor, isSelected, tabContentPattern);
    }
}

void TabsSideBarTabListPattern::UpdateTextColorAndIconColor(const RefPtr<FrameNode>& rowNode,
    Color textColor, Color iconColor, bool isSelected, const RefPtr<TabContentPattern>& tabContentPattern)
{
    CHECK_NULL_VOID(rowNode);
    CHECK_NULL_VOID(tabContentPattern);
    auto iconNode = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().front());
    CHECK_NULL_VOID(iconNode);
    auto textNode = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().back());
    CHECK_NULL_VOID(textNode);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto imagePaintProperty = iconNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(imagePaintProperty);

    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    auto contentProperty = tabContentPattern->GetLayoutProperty<TabContentLayoutProperty>();
    const auto& labelStyle = tabContentPattern->GetLabelStyle();
    const auto& iconStyle = tabContentPattern->GetIconStyle();
    if (isSelected) {
        if (contentProperty && contentProperty->GetLabelSelectedColorSetByUserValue(false) &&
            labelStyle.selectedColor.has_value()) {
            textColor = labelStyle.selectedColor.value();
        } else if (tabLayoutProperty && tabLayoutProperty->GetSidebarSelectedTextColor().has_value()) {
            textColor = tabLayoutProperty->GetSidebarSelectedTextColor().value();
        }
        if (contentProperty && contentProperty->GetIconSelectedColorSetByUserValue(false) &&
            iconStyle.selectedColor.has_value()) {
            iconColor = iconStyle.selectedColor.value();
        } else if (tabLayoutProperty && tabLayoutProperty->GetSidebarSelectedIconColor().has_value()) {
            iconColor = tabLayoutProperty->GetSidebarSelectedIconColor().value();
        }
    } else {
        if (contentProperty && contentProperty->GetLabelUnselectedColorSetByUserValue(false) &&
            labelStyle.unselectedColor.has_value()) {
            textColor = labelStyle.unselectedColor.value();
        } else if (tabLayoutProperty && tabLayoutProperty->GetSidebarUnselectedTextColor().has_value()) {
            textColor = tabLayoutProperty->GetSidebarUnselectedTextColor().value();
        }
        if (contentProperty && contentProperty->GetIconUnselectedColorSetByUserValue(false) &&
            iconStyle.unselectedColor.has_value()) {
            iconColor = iconStyle.unselectedColor.value();
        } else if (tabLayoutProperty && tabLayoutProperty->GetSidebarUnselectedIconColor().has_value()) {
            iconColor = tabLayoutProperty->GetSidebarUnselectedIconColor().value();
        }
    }

    textLayoutProperty->UpdateTextColor(textColor);
    imagePaintProperty->UpdateSvgFillColor(iconColor);
    textNode->MarkModifyDone();
    textNode->MarkDirtyNode();
    iconNode->MarkModifyDone();
    iconNode->MarkDirtyNode();
}

void TabsSideBarTabListPattern::RefreshAllTabItemColors()
{
    // Re-apply text/icon colors for all tab items based on current selected index
    UpdateTabItemTextAndIconColor(currentIndex_);
    // Re-apply selected board color for the currently selected item
    if (currentIndex_ >= 0) {
        UpdateTabItemStyle(currentIndex_, true);
    }
}

void TabsSideBarTabListPattern::InitCurrentIndex(int32_t index)
{
    // Initialize current index without triggering Swiper animation
    // Used when SideBar is first created or after mode switch
    if (currentIndex_ == index) {
        return;
    }
    int32_t prevIndex = currentIndex_;
    currentIndex_ = index;
    UpdateTabItemStyle(prevIndex, false);
    UpdateTabItemStyle(index, true);
}

RefPtr<FrameNode> TabsSideBarTabListPattern::FindTabItemNodeBySwiperIndex(int32_t swiperIndex) const
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, nullptr);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, nullptr);
    auto tabContentNode = AceType::DynamicCast<TabContentNode>(swiperNode->GetChildByIndex(swiperIndex));
    CHECK_NULL_RETURN(tabContentNode, nullptr);
    auto tabItemId = tabContentNode->GetSideBarTabBarItemId();
    auto columnNode = GetTabItemContainerNode();
    CHECK_NULL_RETURN(columnNode, nullptr);
    for (const auto& child : columnNode->GetChildren()) {
        if (!child || child->GetTag() == "TabsSideBarSectionHeader") {
            continue;
        }
        auto frameNode = AceType::DynamicCast<FrameNode>(child);
        if (frameNode && frameNode->GetId() == tabItemId) {
            return frameNode;
        }
    }
    return nullptr;
}

std::string TabsSideBarTabListPattern::GetSectionHeaderTitle(const RefPtr<FrameNode>& headerNode) const
{
    CHECK_NULL_RETURN(headerNode, "");
    if (headerNode->GetChildren().empty()) {
        return "";
    }
    auto textChild = AceType::DynamicCast<FrameNode>(headerNode->GetChildren().front());
    CHECK_NULL_RETURN(textChild, "");
    auto textProp = textChild->GetLayoutProperty<TextLayoutProperty>();
    if (!textProp || !textProp->HasContent()) {
        return "";
    }
    return UtfUtils::Str16ToStr8(textProp->GetContentValue(u""));
}

RefPtr<FrameNode> TabsSideBarTabListPattern::FindSectionHeaderByTitle(const std::string& title) const
{
    auto columnNode = GetTabItemContainerNode();
    CHECK_NULL_RETURN(columnNode, nullptr);
    for (const auto& child : columnNode->GetChildren()) {
        if (!child || child->GetTag() != "TabsSideBarSectionHeader") {
            continue;
        }
        auto headerFrame = AceType::DynamicCast<FrameNode>(child);
        if (!headerFrame) {
            continue;
        }
        if (GetSectionHeaderTitle(headerFrame) == title) {
            return headerFrame;
        }
    }
    return nullptr;
}

void TabsSideBarTabListPattern::HandleSectionHeaderClick(const std::string& title)
{
    // Ignore clicks while a section animation is still running.
    if (sectionAnimationRunning_) {
        return;
    }
    auto it = sectionCollapsedState_.find(title);
    bool currentlyCollapsed = (it != sectionCollapsedState_.end()) ? it->second : false;
    // Record which section is being toggled and the animation direction.
    pendingAnimationSection_ = title;
    pendingAnimationIsCollapse_ = !currentlyCollapsed;
    sectionAnimationRunning_ = true;
    if (it != sectionCollapsedState_.end()) {
        it->second = !currentlyCollapsed;
    } else {
        sectionCollapsedState_[title] = true;
    }
    // RebuildSections() destroys and recreates section headers (new node IDs),
    // so we must find the arrow node AFTER the rebuild to animate the correct node, need 2 children.
    RebuildSections();
    auto headerNode = FindSectionHeaderByTitle(title);
    CHECK_NULL_VOID(headerNode);
    if (headerNode->GetChildren().size() < 2) {
        return;
    }
    auto arrowNode = AceType::DynamicCast<FrameNode>(headerNode->GetChildren().back());
    if (arrowNode && arrowNode->GetTag() == POLYLINE_ETS_TAG) {
        PlayArrowAnimation(arrowNode, !currentlyCollapsed);
    }
}

bool TabsSideBarTabListPattern::IsNodeBelowAnimatedSection(
    float nodeY, const std::set<int32_t>& animatedTabIndices) const
{
    for (int32_t animIdx : animatedTabIndices) {
        auto animNode = FindTabItemNodeBySwiperIndex(animIdx);
        if (animNode && animNode->GetGeometryNode()) {
            float animY = animNode->GetGeometryNode()->GetMarginFrameOffset().GetY();
            if (nodeY > animY) {
                return true;
            }
        }
    }
    return false;
}

int32_t TabsSideBarTabListPattern::FindSwiperIndexForTabItem(const RefPtr<FrameNode>& tabItemNode) const
{
    CHECK_NULL_RETURN(tabItemNode, -1);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, -1);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, -1);
    for (int32_t i = 0; i < swiperNode->TotalChildCount(); ++i) {
        auto tabContentNode = AceType::DynamicCast<TabContentNode>(swiperNode->GetChildByIndex(i));
        if (tabContentNode && tabContentNode->GetSideBarTabBarItemId() == tabItemNode->GetId()) {
            return i;
        }
    }
    return -1;
}

void TabsSideBarTabListPattern::UpdateTabItemStyle(int32_t index, bool selected)
{
    auto host = AceType::DynamicCast<FrameNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto tabTheme = host->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto itemNode = FindTabItemNodeBySwiperIndex(index);
    CHECK_NULL_VOID(itemNode);
    auto renderContext = itemNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto color = selected ? tabTheme->GetSideBarListItemActivedColor() : Color::TRANSPARENT;
    if (selected) {
        auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
        if (tabsNode) {
            auto tabLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
            if (tabLayoutProperty && tabLayoutProperty->GetSidebarSelectedBoardColor().has_value()) {
                color = tabLayoutProperty->GetSidebarSelectedBoardColor().value();
            }
        }
    }
    renderContext->UpdateBackgroundColor(color);
    itemNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void TabsSideBarTabListPattern::ApplySearchFilter(
    std::function<bool(int32_t, const std::string& text)> searchFilter, const std::u16string& searchText)
{
    // Store active search state for cross-concern visibility coordination
    activeSearchFilter_ = searchFilter;
    activeSearchText_ = searchText;

    // When search is active, rebuild sections as a flat list (no section headers,
    // no collapse state) so filtered tabs — including those under collapsed sections —
    // are shown. When search is cleared, restore the normal section layout.
    // If no sections are configured at all, fall back to direct visibility update.
    bool hasAnySection = false;
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    if (tabsNode) {
        auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
        if (swiperNode) {
            for (int32_t i = 0; i < swiperNode->TotalChildCount(); ++i) {
                auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(i));
                if (!tabContentNode) {
                    continue;
                }
                auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
                if (!tabContentPattern) {
                    continue;
                }
                if (tabContentPattern->GetSidebarSection().has_value() &&
                    !tabContentPattern->GetSidebarSection().value().title.empty()) {
                    hasAnySection = true;
                    break;
                }
            }
        }
    }
    if (hasAnySection) {
        RebuildSections();
    } else {
        auto host = AceType::DynamicCast<FrameNode>(GetHost());
        CHECK_NULL_VOID(host);
        auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
        CHECK_NULL_VOID(swiperNode);
        auto tabContentNum = swiperNode->TotalChildCount();
        for (int32_t tabIndex = 0; tabIndex < tabContentNum; ++tabIndex) {
            auto itemNode = FindTabItemNodeBySwiperIndex(tabIndex);
            CHECK_NULL_CONTINUE(itemNode);
            auto property = itemNode->GetLayoutProperty();
            CHECK_NULL_CONTINUE(property);
            if (IsHiddenByDefaultVisibility(tabIndex)) {
                property->UpdateVisibility(VisibleType::GONE);
                continue;
            }
            bool isVisible = !IsHiddenBySearchFilter(tabIndex);
            property->UpdateVisibility(isVisible ? VisibleType::VISIBLE : VisibleType::GONE);
        }
        host->MarkNeedSyncRenderTree();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

void TabsSideBarTabListPattern::AddOrUpdateTabItemWithContent(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, int32_t tabContentId, int32_t myIndex)
{
    CHECK_NULL_VOID(tabItemNode);
    auto tabItemContainerNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(tabItemContainerNode);
    ScopedViewStackProcessor builderViewStackProcessor;
    auto builderNode = tabBarParam.GetContent().Upgrade();
    CHECK_NULL_VOID(builderNode);
    if (!tabItemNode->GetChildren().empty()) {
        tabItemNode->Clean();
    }
    if (builderNode) {
        builderNode->MountToParent(tabItemNode);
    }
    auto oldTabItemNode = GetBuilderByContentId(tabContentId, tabItemNode);
    if (!oldTabItemNode) {
        auto index =
            std::clamp(myIndex, 0, static_cast<int32_t>(tabItemContainerNode->GetChildren().size()));
        tabItemNode->MountToParent(tabItemContainerNode, index);
    } else if (oldTabItemNode != tabItemNode) {
        tabItemContainerNode->ReplaceChild(oldTabItemNode, tabItemNode);
    }
    auto tabBarItemPadding = Dimension(0);
    auto layoutProperty = tabItemNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdatePadding({ CalcLength(tabBarItemPadding), CalcLength(tabBarItemPadding),
        CalcLength(tabBarItemPadding), CalcLength(tabBarItemPadding), {}, {} });
    tabItemNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    tabItemContainerNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void TabsSideBarTabListPattern::AddOrUpdateTabItemWithBuilder(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, int32_t tabContentId, int32_t myIndex)
{
    CHECK_NULL_VOID(tabItemNode);
    auto tabItemContainerNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(tabItemContainerNode);
    ScopedViewStackProcessor builderViewStackProcessor;
    tabBarParam.ExecuteBuilder();
    auto builderNode = ViewStackProcessor::GetInstance()->Finish();
    if (!tabItemNode->GetChildren().empty()) {
        tabItemNode->Clean();
    }
    if (builderNode) {
        builderNode->MountToParent(tabItemNode);
    }
    auto oldTabItemNode = GetBuilderByContentId(tabContentId, tabItemNode);
    if (!oldTabItemNode) {
        auto index = std::clamp(myIndex, 0, static_cast<int32_t>(tabItemContainerNode->GetChildren().size()));
        tabItemNode->MountToParent(tabItemContainerNode, index);
    } else if (oldTabItemNode != tabItemNode) {
        tabItemContainerNode->ReplaceChild(oldTabItemNode, tabItemNode);
    }
    tabItemNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    tabItemContainerNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void TabsSideBarTabListPattern::CreateOrUpdateTabItemTextAndIcon(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode,
    const RefPtr<TabContentPattern>& tabContentPattern, int32_t position,
    RefPtr<FrameNode>& iconNode, RefPtr<FrameNode>& textNode)
{
    CHECK_NULL_VOID(tabItemNode);
    CHECK_NULL_VOID(tabContentPattern);
    auto tabItemContainerNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(tabItemContainerNode);
    auto tabTheme = tabItemNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto context = tabItemNode->GetContext();
    CHECK_NULL_VOID(context);
    RefPtr<FrameNode> rowNode = nullptr;
    if (tabItemNode->GetChildren().size() != 1 || !tabItemNode->GetChildren().front() ||
        tabItemNode->GetChildren().front()->GetTag() != V2::ROW_ETS_TAG) {
        tabItemNode->Clean();
        rowNode = FrameNode::GetOrCreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto linearLayoutProperty = rowNode->GetLayoutProperty<LinearLayoutProperty>();
        CHECK_NULL_VOID(linearLayoutProperty);
        linearLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
        linearLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
        linearLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
        linearLayoutProperty->SetIsVertical(false);
        // Row fills TabItem width (100%) so icon+text content area covers the full item width.
        // Height is WRAP content — do NOT use MATCH_PARENT for height because TabItem
        // itself has WRAP height with minHeight=56vp; MATCH_PARENT on Row would cause
        // circular expansion in an unconstrained Scroll container.
        linearLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), std::nullopt));
        rowNode->MountToParent(tabItemNode);
    } else {
        rowNode = AceType::DynamicCast<FrameNode>(tabItemNode->GetChildren().front());
    }
    CHECK_NULL_VOID(rowNode);
    auto tabBarStyle = tabContentPattern->GetTabBarStyle();
    bool isFrameNode = tabBarStyle == TabBarStyle::SUBTABBATSTYLE && tabContentPattern->HasSubTabBarStyleNode();
    if (rowNode->GetChildren().empty()) {
        if (tabBarParam.GetSymbol().has_value()) {
            iconNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        } else {
            iconNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
        }
        if (isFrameNode) {
            textNode = tabContentPattern->FireCustomStyleNode();
        } else {
            textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        }
        CHECK_NULL_VOID(textNode);
        CHECK_NULL_VOID(iconNode);
        iconNode->MountToParent(rowNode);
        textNode->MountToParent(rowNode);
        // Row mounts to ColumnNode (not directly to tabListNode)
        auto index = std::clamp(position, 0, static_cast<int32_t>(tabItemContainerNode->GetChildren().size()));
        tabItemNode->MountToParent(tabItemContainerNode, index);
    } else {
        if (isFrameNode) {
            auto builderNode = tabContentPattern->FireCustomStyleNode();
            rowNode->ReplaceChild(AceType::DynamicCast<FrameNode>(rowNode->GetChildren().back()), builderNode);
        }
        auto oldIcon = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().front());
        CHECK_NULL_VOID(oldIcon);
        if (tabBarParam.GetSymbol().has_value() && oldIcon->GetTag() != V2::SYMBOL_ETS_TAG) {
            auto icon = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
            rowNode->ReplaceChild(oldIcon, icon);
        } else if (!tabBarParam.GetIcon().empty() && oldIcon->GetTag() != V2::IMAGE_ETS_TAG) {
            auto icon = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
            rowNode->ReplaceChild(oldIcon, icon);
        }
        iconNode = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().front());
        textNode = AceType::DynamicCast<FrameNode>(rowNode->GetChildren().back());
    }
    // Set left/right margin on icon for edge spacing and half of the icon-text gap
    if (iconNode) {
        auto iconLayoutProperty = iconNode->GetLayoutProperty();
        if (iconLayoutProperty) {
            MarginProperty iconMargin;
            iconMargin.left = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            iconMargin.right = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            iconLayoutProperty->UpdateMargin(iconMargin);
        }
    }
    // Set left/right + top/bottom margin on text for edge spacing, icon-text gap, and vertical spacing
    if (textNode) {
        auto textLayoutProperty = textNode->GetLayoutProperty();
        if (textLayoutProperty) {
            // Text uses layoutWeight(1) to fill remaining Row width after icon+margin
            textLayoutProperty->UpdateLayoutWeight(1);
            // Text margin: left/right for edge spacing + icon-text gap,
            // top/bottom for vertical spacing (default padding_level4)
            MarginProperty textMargin;
            textMargin.left = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            textMargin.right = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            auto fontScale = context->GetFontScaleFromEnv(textNode);
            auto verticalMargin = GetVerticalMarginForFontScale(fontScale, tabTheme);
            textMargin.top = CalcLength(verticalMargin);
            textMargin.bottom = CalcLength(verticalMargin);
            textLayoutProperty->UpdateMargin(textMargin);
        }
    }
    rowNode->MarkModifyDone();
}

void TabsSideBarTabListPattern::UpdateTabBarItemTextProperties(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& textNode,
    const RefPtr<SwiperPattern>& swiperPattern, const RefPtr<TabContentPattern>& tabContentPattern,
    int32_t myIndex, int32_t indicator)
{
    CHECK_NULL_VOID(textNode);
    CHECK_NULL_VOID(tabContentPattern);
    CHECK_NULL_VOID(swiperPattern);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    TabContentModelNG::InitTabText(textLayoutProperty);
    auto tabBarStyle = tabContentPattern->GetTabBarStyle();
    bool isFrameNode = tabBarStyle == TabBarStyle::SUBTABBATSTYLE && tabContentPattern->HasSubTabBarStyleNode();
    auto labelStyle = tabContentPattern->GetLabelStyle();

    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateClipEdge(true);
    if (!isFrameNode) {
        textLayoutProperty->UpdateContent(tabBarParam.GetText());
        textLayoutProperty->UpdateFontSize(tabTheme->GetSideBarTextFontSize());
        textLayoutProperty->UpdateTextAlign(TextAlign::START); // Left-aligned (Row layout)
        textLayoutProperty->UpdateMaxFontScale(SIDEBAR_TAB_MAX_FONT_SCALE);
        textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
        // Default to single-line with ellipsis for long tab labels.
        // UpdateLabelStyle below can override these if the user set them.
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    }
    if (!isFrameNode) {
        TabContentModelNG::UpdateLabelStyle(labelStyle, textLayoutProperty);
    }
    auto tabLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    CHECK_NULL_VOID(tabLayoutProperty);
    bool isSelected = (myIndex == indicator);
    Color textColor;
    auto contentProperty = tabContentPattern->GetLayoutProperty<TabContentLayoutProperty>();
    if (isSelected) {
        if (contentProperty && contentProperty->GetLabelSelectedColorSetByUserValue(false) &&
            labelStyle.selectedColor.has_value()) {
            textColor = labelStyle.selectedColor.value();
        } else if (tabLayoutProperty->GetSidebarSelectedTextColor().has_value()) {
            textColor = tabLayoutProperty->GetSidebarSelectedTextColor().value();
        } else {
            textColor = tabTheme->GetSideBarSelectedTextColor();
        }
        textLayoutProperty->UpdateTextColor(textColor);
        return;
    }
    if (contentProperty && contentProperty->GetLabelUnselectedColorSetByUserValue(false) &&
        labelStyle.unselectedColor.has_value()) {
        textColor = labelStyle.unselectedColor.value();
    } else if (tabLayoutProperty->GetSidebarUnselectedTextColor().has_value()) {
        textColor = tabLayoutProperty->GetSidebarUnselectedTextColor().value();
    } else {
        textColor = tabTheme->GetSideBarUnselectedTextColor();
    }
    textLayoutProperty->UpdateTextColor(textColor);
}

void TabsSideBarTabListPattern::UpdateTabBarItemIconProperties(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& iconNode,
    const RefPtr<TabContentPattern>& tabContentPattern, int32_t myIndex, int32_t indicator)
{
    CHECK_NULL_VOID(iconNode);
    CHECK_NULL_VOID(tabContentPattern);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    if (tabBarParam.GetSymbol().has_value()) {
        auto symbolProperty = iconNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(symbolProperty);
        TabContentModelNG::UpdateDefaultSymbol(tabTheme, symbolProperty);
        auto modifierOnApply = tabBarParam.GetSymbol().value().onApply;
        if (modifierOnApply != nullptr) {
            modifierOnApply(AceType::WeakClaim(AceType::RawPtr(iconNode)), "normal");
            TabContentModelNG::UpdateDefaultSymbol(tabTheme, symbolProperty);
        }
        if (modifierOnApply != nullptr && tabBarParam.GetSymbol().value().selectedFlag && myIndex == indicator) {
            modifierOnApply(AceType::WeakClaim(AceType::RawPtr(iconNode)), "selected");
            TabContentModelNG::UpdateSymbolEffect(symbolProperty, false);
        }
        return;
    }
    auto imageProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageProperty);
    if (!tabBarParam.GetIcon().empty()) {
        imageProperty->UpdateUserDefinedIdealSize(CalcSize(
            CalcLength(tabTheme->GetBottomTabImageSize()), CalcLength(tabTheme->GetBottomTabImageSize())));
    } else {
        imageProperty->UpdateUserDefinedIdealSize(CalcSize());
    }
    ImageSourceInfo imageSourceInfo(tabBarParam.GetIcon());
    imageProperty->UpdateImageSourceInfo(imageSourceInfo);

    auto tabLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    CHECK_NULL_VOID(tabLayoutProperty);
    auto imagePaintProperty = iconNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(imagePaintProperty);
    auto contentProperty = tabContentPattern->GetLayoutProperty<TabContentLayoutProperty>();
    const auto& iconStyle = tabContentPattern->GetIconStyle();
    bool isSelected = (myIndex == indicator);
    Color iconColor;
    if (isSelected) {
        if (contentProperty && contentProperty->GetIconSelectedColorSetByUserValue(false) &&
            iconStyle.selectedColor.has_value()) {
            iconColor = iconStyle.selectedColor.value();
        } else if (tabLayoutProperty->GetSidebarSelectedIconColor().has_value()) {
            iconColor = tabLayoutProperty->GetSidebarSelectedIconColor().value();
        } else {
            iconColor = tabTheme->GetSideBarSelectedIconColor();
        }
        imagePaintProperty->UpdateSvgFillColor(iconColor);
        return;
    }
    if (contentProperty && contentProperty->GetIconUnselectedColorSetByUserValue(false) &&
        iconStyle.unselectedColor.has_value()) {
        iconColor = iconStyle.unselectedColor.value();
    } else if (tabLayoutProperty->GetSidebarUnselectedIconColor().has_value()) {
        iconColor = tabLayoutProperty->GetSidebarUnselectedIconColor().value();
    } else {
        iconColor = tabTheme->GetSideBarUnselectedIconColor();
    }
    imagePaintProperty->UpdateSvgFillColor(iconColor);
}

void TabsSideBarTabListPattern::AddOrUpdateTabItemWithIconAndText(
    const TabBarParam& tabBarParam, const RefPtr<FrameNode>& tabItemNode, const RefPtr<FrameNode>& swiperNode,
    const RefPtr<TabContentPattern>& tabContentPattern, int32_t position, int32_t myIndex)
{
    CHECK_NULL_VOID(tabItemNode);
    CHECK_NULL_VOID(swiperNode);
    CHECK_NULL_VOID(tabContentPattern);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);

    auto tabLayoutProperty = AceType::DynamicCast<TabsLayoutProperty>(tabsNode->GetLayoutProperty());
    CHECK_NULL_VOID(tabLayoutProperty);
    RefPtr<FrameNode> iconNode;
    RefPtr<FrameNode> textNode;
    CreateOrUpdateTabItemTextAndIcon(tabBarParam, tabItemNode, tabContentPattern, position, iconNode, textNode);
    CHECK_NULL_VOID(textNode);
    CHECK_NULL_VOID(iconNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    int32_t indicator = 0;
    if (tabLayoutProperty->GetIndexSetByUser().has_value()) {
        indicator = tabLayoutProperty->GetIndexSetByUser().value();
    } else if (swiperLayoutProperty->GetIndex().has_value()) {
        indicator = swiperLayoutProperty->GetIndex().value();
    }
    int32_t totalCount = swiperPattern->TotalCount();
    if (indicator > totalCount - 1 || indicator < 0) {
        indicator = 0;
    }
    // Update text properties
    UpdateTabBarItemTextProperties(tabBarParam, textNode, swiperPattern, tabContentPattern, myIndex, indicator);
    // Update property of image/symbol.
    UpdateTabBarItemIconProperties(tabBarParam, iconNode, tabContentPattern, myIndex, indicator);

    tabItemNode->MarkModifyDone();
    textNode->MarkModifyDone();
    textNode->MarkDirtyNode();
    iconNode->MarkModifyDone();
    iconNode->MarkDirtyNode();
}

RefPtr<FrameNode> TabsSideBarTabListPattern::GetOrCreateTabItemNode(int32_t id)
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, nullptr);
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    CHECK_NULL_RETURN(tabTheme, nullptr);
    auto* buttonModifier = NodeModifier::GetButtonCustomModifier();
    CHECK_NULL_RETURN(buttonModifier, nullptr);
    auto tabItemNode = FrameNode::GetOrCreateFrameNode("TabsSideBarTabItem", id,
        []() -> RefPtr<Pattern> {
            RefPtr<Pattern> buttonPattern = nullptr;
            auto* buttonModifier = NodeModifier::GetButtonCustomModifier();
            CHECK_NULL_RETURN(buttonModifier, buttonPattern);
            return buttonModifier->createButtonPattern();
        });
    CHECK_NULL_RETURN(tabItemNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(tabItemNode));
    buttonModifier->updateBackgroundColorFlagByUserToLayoutProp(nodeHandle, true);
    buttonModifier->setBlendColor(
        nodeHandle, tabTheme->GetSideBarListItemPressedColor(), tabTheme->GetSideBarListItemHoverColor());
    buttonModifier->setFocusBorderColor(nodeHandle, tabTheme->GetSideBarListItemFocusColor());
    buttonModifier->setFocusBorderWidth(nodeHandle, SIDEBAR_TAB_ITEM_FOCUS_WIDTH);
    buttonModifier->updateTypeToLayoutProp(nodeHandle, ButtonType::NORMAL);
    auto radius = tabTheme->GetSideBarListItemCornerRadius();
    buttonModifier->setButtonBorderRadius(nodeHandle, radius, radius, radius, radius);
    auto property = tabItemNode->GetLayoutProperty();
    CHECK_NULL_RETURN(property, nullptr);
    // Width: 100% of parent; Height: match-content with minimum height
    property->UpdateUserDefinedIdealSize(CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), std::nullopt));
    auto itemHeight = tabTheme->GetSideBarListItemHeight();
    property->UpdateCalcMinSize(CalcSize(std::nullopt, CalcLength(itemHeight)));
    property->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, false);
    return tabItemNode;
}

bool TabsSideBarTabListPattern::IsHiddenByDefaultVisibility(int32_t tabIndex) const
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, false);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, false);
    if (tabIndex >= swiperNode->TotalChildCount()) {
        return false;
    }
    auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(tabIndex));
    CHECK_NULL_RETURN(tabContentNode, false);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(tabContentPattern, false);
    return tabContentPattern->GetDefaultVisibility().ShouldHide(TabBarDisplayMode::SIDEBAR);
}

bool TabsSideBarTabListPattern::IsHiddenBySearchFilter(int32_t tabIndex) const
{
    if (activeSearchText_.empty()) {
        return false;
    }
    auto searchText8 = UtfUtils::Str16ToStr8(activeSearchText_);
    if (activeSearchFilter_) {
        return !activeSearchFilter_(tabIndex, searchText8);
    }
    // Default: fuzzy match by tab text (Unicode-aware case-insensitive)
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, false);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, false);
    if (tabIndex >= swiperNode->TotalChildCount()) {
        return false;
    }
    auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(tabIndex));
    CHECK_NULL_RETURN(tabContentNode, false);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(tabContentPattern, false);
    auto searchTextLower = ToLowerU16Str(activeSearchText_);
    auto tabTextLower = ToLowerU16Str(UtfUtils::Str8ToStr16(tabContentPattern->GetTabBarParam().GetText()));
    return tabTextLower.find(searchTextLower) == std::u16string::npos;
}

void TabsSideBarTabListPattern::ApplyDefaultVisibility()
{
    auto host = AceType::DynamicCast<FrameNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabsProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    CHECK_NULL_VOID(tabsProperty);
    // The sidebar does not display in Bottom/Top style, so there is no need to update the visibility of the internal tab.
    auto barLayoutStyle = tabsProperty->GetBarLayoutStyleValue(TabBarLayoutStyle::BOTTOM);
    if (barLayoutStyle == TabBarLayoutStyle::BOTTOM || barLayoutStyle == TabBarLayoutStyle::TOP) {
        return;
    }

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto tabContentNum = swiperNode->TotalChildCount();
    for (int32_t tabIndex = 0; tabIndex < tabContentNum; ++tabIndex) {
        auto itemNode = FindTabItemNodeBySwiperIndex(tabIndex);
        CHECK_NULL_CONTINUE(itemNode);
        auto property = itemNode->GetLayoutProperty();
        CHECK_NULL_CONTINUE(property);
        // Check defaultVisibility
        if (IsHiddenByDefaultVisibility(tabIndex)) {
            property->UpdateVisibility(VisibleType::GONE);
            continue;
        }
        // defaultVisibility says visible — check search filter
        bool isVisible = !IsHiddenBySearchFilter(tabIndex);
        property->UpdateVisibility(isVisible ? VisibleType::VISIBLE : VisibleType::GONE);
    }
    host->MarkNeedSyncRenderTree();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarTabListPattern::AddOrUpdateTabListItem(
    const RefPtr<FrameNode>& tabContent, int32_t position, bool update)
{
    auto tabContentNode = AceType::DynamicCast<TabContentNode>(tabContent);
    CHECK_NULL_VOID(tabContentNode);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabsProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    CHECK_NULL_VOID(tabsProperty);
    auto tabContentId = tabContentNode->GetId();
    // Get tabBarParam
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(tabContentPattern);
    const auto& tabBarParam = tabContentPattern->GetTabBarParam();

    // Get ColumnNode (mounting target for Row tab items)
    auto tabItemContainerNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(tabItemContainerNode);
    // Use independent sideBarTabBarItemId (avoid ID collision with bottom TabBar)
    int32_t sideBarTabBarItemId = tabContentNode->GetSideBarTabBarItemId();
    auto tabItemNode = GetOrCreateTabItemNode(sideBarTabBarItemId);
    CHECK_NULL_VOID(tabItemNode);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto myIndex = swiperNode->GetChildFlatIndex(tabContentId).second;
    if (tabBarParam.HasContent()) {
        // Create tab bar with content.
        AddOrUpdateTabItemWithContent(tabBarParam, tabItemNode, tabContentId, myIndex);
    } else if (tabBarParam.HasBuilder()) {
        // Create tab bar with builder.
        AddOrUpdateTabItemWithBuilder(tabBarParam, tabItemNode, tabContentId, myIndex);
    } else {
        // Create icon and text child nodes
        AddOrUpdateTabItemWithIconAndText(tabBarParam, tabItemNode, swiperNode, tabContentPattern, position, myIndex);
    }

    // Bind click event: only on first creation to avoid duplicate handlers on update.
    // Capture tabContentId instead of index to handle Tab insertion/deletion/reordering dynamically.
    if (!update) {
        auto clickEvent = [weakTabsNode = WeakPtr(tabsNode), weakPattern = WeakClaim(this),
                          capturedTabContentId = tabContentId](const GestureEvent& info) {
            auto tabsNode = weakTabsNode.Upgrade();
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(tabsNode);
            CHECK_NULL_VOID(pattern);
            auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
            CHECK_NULL_VOID(swiperNode);
            auto currentIndex = swiperNode->GetChildFlatIndex(capturedTabContentId).second;
            pattern->SetCurrentIndex(currentIndex);
        };
        auto gestureHub = tabItemNode->GetOrCreateGestureEventHub();
        if (gestureHub) {
            gestureHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(std::move(clickEvent)));
        }
    }

    // Set initial selected/unselected background color inline (same as bottom TabBar)
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    int32_t indicator = 0;
    if (tabsProperty->GetIndexSetByUser().has_value()) {
        indicator = tabsProperty->GetIndexSetByUser().value();
    } else if (swiperLayoutProperty->GetIndex().has_value()) {
        indicator = swiperLayoutProperty->GetIndex().value();
    }
    int32_t totalCount = swiperPattern->TotalCount();
    if (indicator > totalCount - 1 || indicator < 0) {
        indicator = 0;
    }
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    auto itemRenderContext = tabItemNode->GetRenderContext();
    if (tabTheme && itemRenderContext) {
        auto color = (myIndex == indicator)
            ? tabTheme->GetSideBarListItemActivedColor()
            : Color::TRANSPARENT;
        if (myIndex == indicator) {
            auto tabLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
            if (tabLayoutProperty && tabLayoutProperty->GetSidebarSelectedBoardColor().has_value()) {
                color = tabLayoutProperty->GetSidebarSelectedBoardColor().value();
            }
        }
        itemRenderContext->UpdateBackgroundColor(color);
    }
    if (currentIndex_ == -1) {
        currentIndex_ = indicator;
    }

    tabItemContainerNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // Apply defaultVisibility for this single tab item
    auto tabItemProperty = tabItemNode->GetLayoutProperty();
    CHECK_NULL_VOID(tabItemProperty);
    bool shouldHide = tabContentPattern->GetDefaultVisibility().ShouldHide(TabBarDisplayMode::SIDEBAR);
    tabItemProperty->UpdateVisibility(shouldHide ? VisibleType::GONE : VisibleType::VISIBLE);

    // Rebuild sections if any tab has sidebar section configured
    RebuildSections();
}

bool TabsSideBarTabListPattern::IsTabVisible(int32_t tabIndex) const
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, false);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, false);
    if (tabIndex >= swiperNode->TotalChildCount() || tabIndex < 0) {
        return false;
    }
    auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(tabIndex));
    CHECK_NULL_RETURN(tabContentNode, false);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(tabContentPattern, false);
    // Check defaultVisibility
    if (tabContentPattern->GetDefaultVisibility().ShouldHide(TabBarDisplayMode::SIDEBAR)) {
        return false;
    }
    // Check search filter
    if (!activeSearchText_.empty()) {
        auto searchTextLower = ToLowerU16Str(activeSearchText_);
        auto searchText8 = UtfUtils::Str16ToStr8(activeSearchText_);
        if (activeSearchFilter_) {
            return activeSearchFilter_(tabIndex, searchText8);
        }
        auto tabTextLower = ToLowerU16Str(UtfUtils::Str8ToStr16(tabContentPattern->GetTabBarParam().GetText()));
        return tabTextLower.find(searchTextLower) != std::u16string::npos;
    }
    return true;
}

void TabsSideBarTabListPattern::BuildSectionGroups(
    std::vector<SectionInfo>& sections, std::vector<int32_t>& unsectionedIndices)
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto tabContentNum = swiperNode->TotalChildCount();
    // Map: section title -> section index in sections vector
    std::map<std::string, size_t> sectionTitleMap;
    for (int32_t i = 0; i < tabContentNum; ++i) {
        auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(i));
        CHECK_NULL_CONTINUE(tabContentNode);
        auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
        CHECK_NULL_CONTINUE(tabContentPattern);
        const auto& sidebarSection = tabContentPattern->GetSidebarSection();
        if (!sidebarSection.has_value() || sidebarSection.value().title.empty()) {
            unsectionedIndices.push_back(i);
            continue;
        }
        const auto& sec = sidebarSection.value();
        auto it = sectionTitleMap.find(sec.title);
        if (it == sectionTitleMap.end()) {
            SectionInfo info;
            info.title = sec.title;
            info.order = sec.order;
            info.titleStyle = sec.titleStyle;
            info.tabIndices.push_back(i);
            if (IsTabVisible(i)) {
                info.firstVisibleTabIndex = i;
            }
            sections.push_back(std::move(info));
            sectionTitleMap[sec.title] = sections.size() - 1;
        } else {
            sections[it->second].tabIndices.push_back(i);
            if (sections[it->second].firstVisibleTabIndex < 0 && IsTabVisible(i)) {
                sections[it->second].firstVisibleTabIndex = i;
            }
        }
    }
}

void TabsSideBarTabListPattern::SortSections(std::vector<SectionInfo>& sections)
{
    // Sections with order set are sorted first by order ascending.
    // Sections without order follow, sorted by firstVisibleTabIndex ascending.
    // Sections with same order are sorted by firstVisibleTabIndex.
    std::stable_sort(sections.begin(), sections.end(),
        [](const SectionInfo& a, const SectionInfo& b) {
            if (a.order.has_value() && b.order.has_value()) {
                if (a.order.value() != b.order.value()) {
                    return a.order.value() < b.order.value();
                }
                // Same order: sort by first visible tab index
                return a.firstVisibleTabIndex < b.firstVisibleTabIndex;
            }
            if (a.order.has_value() && !b.order.has_value()) {
                return true; // a (has order) comes first
            }
            if (!a.order.has_value() && b.order.has_value()) {
                return false; // b (has order) comes first
            }
            // Neither has order: sort by first visible tab index
            return a.firstVisibleTabIndex < b.firstVisibleTabIndex;
        });
}

RefPtr<FrameNode> TabsSideBarTabListPattern::CreateOrUpdateSectionHeader(
    const std::string& title, int32_t sectionId, SidebarSectionTitleStyle titleStyle)
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_RETURN(tabsNode, nullptr);
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    CHECK_NULL_RETURN(tabTheme, nullptr);
    // Use a fixed tag for section header nodes
    auto headerNode = FrameNode::GetOrCreateFrameNode("TabsSideBarSectionHeader", sectionId,
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    CHECK_NULL_RETURN(headerNode, nullptr);
    auto layoutProperty = headerNode->GetLayoutProperty<LinearLayoutProperty>();
    if (layoutProperty) {
        layoutProperty->UpdateFlexDirection(FlexDirection::ROW);
        layoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
        layoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_END);
        // Section header height is fixed at 56vp
        constexpr float SECTION_HEADER_HEIGHT_VP = 56.0f;
        layoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(1.0, DimensionUnit::PERCENT),
                     CalcLength(SECTION_HEADER_HEIGHT_VP, DimensionUnit::VP)));
    }
    // Create or update the text child (first child)
    RefPtr<FrameNode> textNode;
    bool needCreateArrow = headerNode->GetChildren().empty();
    if (needCreateArrow) {
        textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        CHECK_NULL_RETURN(textNode, nullptr);
        textNode->MountToParent(headerNode);
    } else {
        textNode = AceType::DynamicCast<FrameNode>(headerNode->GetChildren().front());
    }
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    if (textLayoutProperty) {
        textLayoutProperty->UpdateContent(title);
        // Font size and weight depend on titleStyle:
        // STANDARD (0): 14fp, MEDIUM; EMPHASIZED (1): 18fp, BOLD
        constexpr Dimension STANDARD_FONT_SIZE = 14.0_fp;
        constexpr Dimension EMPHASIZED_FONT_SIZE = 18.0_fp;
        if (titleStyle == SidebarSectionTitleStyle::EMPHASIZED) {
            textLayoutProperty->UpdateFontSize(EMPHASIZED_FONT_SIZE);
            textLayoutProperty->UpdateFontWeight(FontWeight::BOLD);
        } else {
            textLayoutProperty->UpdateFontSize(STANDARD_FONT_SIZE);
            textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
        }
        textLayoutProperty->UpdateTextColor(tabTheme->GetSideBarUnselectedTextColor());
        textLayoutProperty->UpdateLayoutWeight(1); // text fills remaining width, pushes arrow to the right
        // Text overflow: wrap to 2 line then ellipsis
        textLayoutProperty->UpdateMaxLines(2);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        // Margins: left 8vp (from section left), bottom 8vp (from section bottom)
        MarginProperty margin;
        margin.left = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
        margin.bottom = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
        textLayoutProperty->UpdateMargin(margin);
    }
    // Create or update the arrow child (second child) — a Polyline drawing two line segments (^ shape)
    if (needCreateArrow) {
        // Arrow component: 24x24vp (matching the design icon size).
        // Arrow shape (from design): base points at (5, 8) and (18, 8), vertex at (12, 15).
        // Two line segments: left-base → vertex → right-base.
        // During animation, the base points stay fixed and only the vertex Y moves.
        constexpr float ARROW_COMPONENT_SIZE_VP = 24.0f;
        auto arrowNode = FrameNode::GetOrCreateFrameNode(POLYLINE_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<PolygonPattern>(false); });
        CHECK_NULL_RETURN(arrowNode, nullptr);
        auto arrowLayoutProperty = arrowNode->GetLayoutProperty();
        if (arrowLayoutProperty) {
            arrowLayoutProperty->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(ARROW_COMPONENT_SIZE_VP, DimensionUnit::VP),
                         CalcLength(ARROW_COMPONENT_SIZE_VP, DimensionUnit::VP)));
            MarginProperty arrowMargin;
            arrowMargin.right = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            arrowMargin.left = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            arrowMargin.bottom = CalcLength(SIDEBAR_TAB_ITEM_MARGIN);
            arrowLayoutProperty->UpdateMargin(arrowMargin);
        }
        // Only stroke (no fill) — two line segments forming ^ shape
        ShapeAbstractModelNG::SetStroke(AceType::RawPtr(arrowNode), tabTheme->GetSideBarUnselectedTextColor());
        ShapeAbstractModelNG::SetStrokeWidth(AceType::RawPtr(arrowNode), Dimension(1.5_vp));
        ShapeAbstractModelNG::SetFill(AceType::RawPtr(arrowNode), Color::TRANSPARENT);
        // Initial arrow state: set to the pre-toggle position so the animation starts
        // from the correct origin. Without this, the arrow renders at the post-toggle
        // (animation end) state for one frame before the animation begins.
        auto collapsedIt = sectionCollapsedState_.find(title);
        bool collapsed = (collapsedIt != sectionCollapsedState_.end()) ? collapsedIt->second : false;
        bool hasPendingAnim = !pendingAnimationSection_.empty() && pendingAnimationSection_ == title;
        if (hasPendingAnim) {
            // Use the pre-toggle state as the animation start point.
            UpdateArrowPoints(arrowNode, collapsed ? 0.0f : 1.0f);
        } else {
            UpdateArrowPoints(arrowNode, collapsed ? 1.0f : 0.0f);
        }
        arrowNode->MountToParent(headerNode);
    }
    headerNode->MarkModifyDone();
    headerNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return headerNode;
}

void TabsSideBarTabListPattern::ApplySectionLayout(
    const std::vector<SectionInfo>& sections, const std::vector<int32_t>& unsectionedIndices)
{
    auto columnNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(columnNode);

    // --- Capture old positions (Y offset relative to column) for animation ---
    // Only capture if a section toggle triggered this rebuild.
    bool hasPendingAnimation = !pendingAnimationSection_.empty();
    std::map<int32_t, float> oldTabOffsets;
    std::map<std::string, float> oldHeaderOffsets;
    if (hasPendingAnimation) {
        for (const auto& child : columnNode->GetChildren()) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            if (!frameNode) {
                continue;
            }
            auto geometry = frameNode->GetGeometryNode();
            if (!geometry) {
                continue;
            }
            if (child->GetTag() == "TabsSideBarSectionHeader") {
                auto title = GetSectionHeaderTitle(frameNode);
                if (!title.empty()) {
                    oldHeaderOffsets[title] = geometry->GetMarginFrameOffset().GetY();
                }
                continue;
            }
            int32_t idx = FindSwiperIndexForTabItem(frameNode);
            if (idx >= 0) {
                oldTabOffsets[idx] = geometry->GetMarginFrameOffset().GetY();
            }
        }
    }

    // Remove all existing section header nodes
    auto children = columnNode->GetChildren();
    for (const auto& child : children) {
        if (child && child->GetTag() == "TabsSideBarSectionHeader") {
            columnNode->RemoveChild(child);
        }
    }

    // Map: swiper index -> tab item node
    std::map<int32_t, RefPtr<UINode>> tabItemNodes;
    for (const auto& child : columnNode->GetChildren()) {
        if (!child || child->GetTag() == "TabsSideBarSectionHeader") {
            continue;
        }
        auto frameNode = AceType::DynamicCast<FrameNode>(child);
        if (!frameNode) {
            continue;
        }
        int32_t idx = FindSwiperIndexForTabItem(frameNode);
        if (idx >= 0) {
            tabItemNodes[idx] = child;
        }
    }
    // Remove all children from column
    columnNode->Clean();
    // Re-add: unsectioned tabs first (no title area at top), then sectioned tabs
    int32_t insertIndex = 0;
    // Add unsectioned tabs first
    for (int32_t tabIndex : unsectionedIndices) {
        auto it = tabItemNodes.find(tabIndex);
        if (it == tabItemNodes.end()) {
            continue;
        }
        it->second->MountToParent(columnNode, insertIndex);
        insertIndex++;
        // Update visibility based on search filter / defaultVisibility.
        // Without this, tabs that were GONE (e.g. from a previous collapse) would stay hidden.
        auto frameNode = AceType::DynamicCast<FrameNode>(it->second);
        if (frameNode) {
            auto prop = frameNode->GetLayoutProperty();
            if (prop) {
                prop->UpdateVisibility(IsTabVisible(tabIndex) ? VisibleType::VISIBLE : VisibleType::GONE);
            }
            // Clear any residual transform/opacity from previous collapse/expand animations.
            // Without this, tabs that were translated/faded during a collapse animation
            // would retain those visual offsets in the new flat layout, causing blank gaps.
            auto renderContext = frameNode->GetRenderContext();
            if (renderContext) {
                renderContext->UpdateTransformTranslate(TranslateOptions(0.0f, 0.0f, 0.0f));
                renderContext->UpdateOpacity(1.0f);
            }
        }
    }
    // Then add sectioned tabs with section headers
    for (const auto& sec : sections) {
        // Check if section has any visible tabs
        bool hasVisible = false;
        for (int32_t tabIndex : sec.tabIndices) {
            if (IsTabVisible(tabIndex)) {
                hasVisible = true;
                break;
            }
        }
        if (!hasVisible) {
            continue; // Skip sections with no visible tabs
        }
        // Create section header
        int32_t sectionId = ElementRegister::GetInstance()->MakeUniqueId();
        auto headerNode = CreateOrUpdateSectionHeader(sec.title, sectionId, sec.titleStyle);
        if (headerNode) {
            headerNode->MountToParent(columnNode, insertIndex);
            insertIndex++;
            // Bind click event for collapse/expand
            auto gestureHub = headerNode->GetOrCreateGestureEventHub();
            if (gestureHub) {
                auto clickEvent = [weakPattern = WeakClaim(this), title = sec.title](
                                      const GestureEvent& info) {
                    auto pattern = weakPattern.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    pattern->HandleSectionHeaderClick(title);
                };
                gestureHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(std::move(clickEvent)));
            }
        }
        // Add tab items for this section
        auto collapsedIt = sectionCollapsedState_.find(sec.title);
        bool collapsed = (collapsedIt != sectionCollapsedState_.end()) ? collapsedIt->second : false;
        // When a collapse animation is pending for this section, keep the tabs VISIBLE
        // during layout so no GONE→VISIBLE re-layout is triggered later. The animation
        // will move them visually; the finish callback will set them to GONE.
        bool keepVisibleForCollapseAnim = hasPendingAnimation && pendingAnimationIsCollapse_ &&
            pendingAnimationSection_ == sec.title;
        for (int32_t tabIndex : sec.tabIndices) {
            auto it = tabItemNodes.find(tabIndex);
            if (it == tabItemNodes.end()) {
                continue;
            }
            it->second->MountToParent(columnNode, insertIndex);
            insertIndex++;
            // If section is collapsed, hide the tab item; otherwise restore visibility.
            auto frameNode = AceType::DynamicCast<FrameNode>(it->second);
            if (frameNode) {
                auto prop = frameNode->GetLayoutProperty();
                if (prop) {
                    if (collapsed && !keepVisibleForCollapseAnim) {
                        prop->UpdateVisibility(VisibleType::GONE);
                    } else {
                        prop->UpdateVisibility(
                            IsTabVisible(tabIndex) ? VisibleType::VISIBLE : VisibleType::GONE);
                    }
                }
                // Clear residual transform/opacity from previous animations when not
                // in a pending animation context (e.g. search filter rebuild).
                if (!hasPendingAnimation) {
                    auto renderContext = frameNode->GetRenderContext();
                    if (renderContext) {
                        renderContext->UpdateTransformTranslate(TranslateOptions(0.0f, 0.0f, 0.0f));
                        renderContext->UpdateOpacity(1.0f);
                    }
                }
            }
        }
    }
    columnNode->MarkNeedSyncRenderTree();
    columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    // --- Schedule expand/collapse animation after layout pass ---
    if (hasPendingAnimation) {
        std::string triggeredSection = pendingAnimationSection_;
        bool isCollapse = pendingAnimationIsCollapse_;
        pendingAnimationSection_.clear();
        // Collect the tab indices belonging to the triggered section for animation.
        std::set<int32_t> animatedTabIndices;
        for (const auto& sec : sections) {
            if (sec.title == triggeredSection) {
                for (int32_t idx : sec.tabIndices) {
                    animatedTabIndices.insert(idx);
                }
                break;
            }
        }
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->AddAfterLayoutTask(
                [weak = WeakClaim(this), oldTabOffsets = std::move(oldTabOffsets),
                 animatedTabIndices = std::move(animatedTabIndices),
                 triggeredSection = std::move(triggeredSection), isCollapse,
                 oldHeaderOffsets = std::move(oldHeaderOffsets)]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    pattern->PlaySectionExpandCollapseAnimation(
                        oldTabOffsets, animatedTabIndices, triggeredSection, isCollapse, oldHeaderOffsets);
                });
        }
    }
}

void TabsSideBarTabListPattern::UpdateArrowPoints(const RefPtr<FrameNode>& arrowNode, float progress)
{
    CHECK_NULL_VOID(arrowNode);
    // Arrow shape in a 24x24vp component (from design icons):
    //   chevron_up  (^):  left(5,15)  vertex(12,8)  right(18,15)
    //   chevron_down (v): left(5,8)   vertex(12,15) right(18,8)
    // All three points move linearly between the two states.
    // progress 0.0 = ^ (up), 1.0 = v (down)
    constexpr float ARROW_LEFT_X = 5.0f;
    constexpr float ARROW_RIGHT_X = 18.0f;
    constexpr float ARROW_CENTER_X = 12.0f;
    constexpr float ARROW_BASE_Y_UP = 15.0f;   // base Y when ^
    constexpr float ARROW_BASE_Y_DOWN = 8.0f;   // base Y when v
    constexpr float ARROW_VERTEX_Y_UP = 8.0f;    // vertex Y when ^
    constexpr float ARROW_VERTEX_Y_DOWN = 15.0f; // vertex Y when v
    float baseY = ARROW_BASE_Y_UP + (ARROW_BASE_Y_DOWN - ARROW_BASE_Y_UP) * progress;
    float vertexY = ARROW_VERTEX_Y_UP + (ARROW_VERTEX_Y_DOWN - ARROW_VERTEX_Y_UP) * progress;
    ShapePoints points = {
        { Dimension(ARROW_LEFT_X, DimensionUnit::VP), Dimension(baseY, DimensionUnit::VP) },
        { Dimension(ARROW_CENTER_X, DimensionUnit::VP), Dimension(vertexY, DimensionUnit::VP) },
        { Dimension(ARROW_RIGHT_X, DimensionUnit::VP), Dimension(baseY, DimensionUnit::VP) },
    };
    PolygonModelNG::SetPoints(AceType::RawPtr(arrowNode), points);
    arrowNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TabsSideBarTabListPattern::PlayArrowAnimation(const RefPtr<FrameNode>& arrowNode, bool isCollapse)
{
    CHECK_NULL_VOID(arrowNode);
    // Use AnimatablePropertyFloat for frame-by-frame callback animation.
    // This binds the animation to the arrow node's lifecycle (auto-stops on node destruction)
    // and integrates with the framework's animation scheduling.
    // progress: 0.0 = ^ (up), 1.0 = v (down)
    // Collapse: ^ → v (0.0 → 1.0)
    // Expand: v → ^ (1.0 → 0.0)
    const std::string arrowPropertyName = "sidebar_arrow_progress";
    float begin = isCollapse ? 0.0f : 1.0f;
    float end = isCollapse ? 1.0f : 0.0f;
    auto weakPattern = AceType::WeakClaim(this);
    // Create the property only once (CreateAnimatablePropertyFloat is a no-op if it
    // already exists — it won't update the value or callback on subsequent calls).
    arrowNode->CreateAnimatablePropertyFloat(
        arrowPropertyName, begin,
        [weakArrow = AceType::WeakClaim(AceType::RawPtr(arrowNode)), weakPattern](float value) {
            auto arrow = weakArrow.Upgrade();
            CHECK_NULL_VOID(arrow);
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->UpdateArrowPoints(arrow, value);
        });
    // Reset to begin value before starting the animation. Without this, the property
    // retains the end value from the previous animation and the new animation has
    // zero distance to animate, causing an instant one-frame change.
    arrowNode->UpdateAnimatablePropertyFloat(arrowPropertyName, begin);
    AnimationOption option;
    option.SetDuration(250); // 250ms linear
    option.SetCurve(Curves::LINEAR);
    option.SetFillMode(FillMode::FORWARDS);
    AnimationUtils::StartAnimation(
        option,
        [weakArrow = AceType::WeakClaim(AceType::RawPtr(arrowNode)), end, arrowPropertyName]() {
            auto arrow = weakArrow.Upgrade();
            CHECK_NULL_VOID(arrow);
            arrow->UpdateAnimatablePropertyFloat(arrowPropertyName, end);
        },
        nullptr, nullptr, arrowNode->GetContextRefPtr());
}

void TabsSideBarTabListPattern::PlaySectionExpandCollapseAnimation(
    const std::map<int32_t, float>& oldTabOffsets, const std::set<int32_t>& animatedTabIndices,
    const std::string& triggeredSection, bool isCollapse, const std::map<std::string, float>& oldHeaderOffsets)
{
    auto columnNode = GetTabItemContainerNode();
    CHECK_NULL_VOID(columnNode);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);

    // Find the section header node's vertical center (relative to column) — this is the
    // animation origin (for expand) or target (for collapse) of the animated tab items.
    float headerY = 0.0f;
    bool headerFound = false;
    auto headerNode = FindSectionHeaderByTitle(triggeredSection);
    if (headerNode) {
        auto geometry = headerNode->GetGeometryNode();
        if (geometry) {
            float headerTop = geometry->GetMarginFrameOffset().GetY();
            float headerHeight = geometry->GetMarginFrameSize().Height();
            headerY = headerTop + headerHeight / 2.0f;
            headerFound = true;
        }
    }

    // Build the animation option with spring curve: stiffness 328, damping 34.
    constexpr float SPRING_VELOCITY = 0.0f;
    constexpr float SPRING_MASS = 1.0f;
    constexpr float SPRING_STIFFNESS = 328.0f;
    constexpr float SPRING_DAMPING = 34.0f;
    auto springCurve = AceType::MakeRefPtr<SpringCurve>(
        SPRING_VELOCITY, SPRING_MASS, SPRING_STIFFNESS, SPRING_DAMPING);
    AnimationOption option;
    option.SetCurve(springCurve);
    // Spring curves ignore duration, but set a sane default 400 for the framework.
    option.SetDuration(400);
    option.SetFillMode(FillMode::FORWARDS);

    // Capture new offsets and compute per-tab animation parameters.
    struct TabAnimInfo {
        RefPtr<FrameNode> node;
        float deltaY = 0.0f;        // initial translate offset
        float targetDeltaY = 0.0f;  // final translate offset (always 0 for pushed items)
        bool fade = false;          // opacity animation needed
        float startOpacity = 1.0f;
        float endOpacity = 1.0f;
        bool needHideAfterAnim = false; // set to GONE after animation (collapse case)
    };
    std::vector<TabAnimInfo> animInfos;

    // For collapse: collapsing tabs are kept VISIBLE during layout (not GONE) to avoid
    // GONE→VISIBLE re-layout triggering a visual jump. This means pushed items are still
    // at their old positions (oldY == newY). We need to calculate the push distance
    // (total height of collapsing tabs) to animate pushed items upward.
    float collapsePushDistance = 0.0f;
    if (isCollapse) {
        for (int32_t tabIndex : animatedTabIndices) {
            auto tabItemNode = FindTabItemNodeBySwiperIndex(tabIndex);
            if (!tabItemNode) {
                continue;
            }
            auto geometry = tabItemNode->GetGeometryNode();
            if (!geometry) {
                continue;
            }
            collapsePushDistance += geometry->GetMarginFrameSize().Height();
        }
    }

    // Compute a uniform shift distance for the animated tab group so that tabs
    // maintain their relative vertical spacing during the animation (instead of
    // all converging to the header position).
    // Collapse: first animated tab moves to headerY; all others shift by the same delta.
    // Expand:   first animated tab starts at headerY; all others offset by the same delta.
    float groupShift = 0.0f; // uniform translate applied to all animated tabs
    if (headerFound) {
        // Find the first (topmost) animated tab's Y position.
        float firstTabY = std::numeric_limits<float>::max();
        for (int32_t tabIndex : animatedTabIndices) {
            auto tabItemNode = FindTabItemNodeBySwiperIndex(tabIndex);
            if (tabItemNode && tabItemNode->GetGeometryNode()) {
                float y = tabItemNode->GetGeometryNode()->GetMarginFrameOffset().GetY();
                if (y < firstTabY) {
                    firstTabY = y;
                }
            }
        }
        if (firstTabY != std::numeric_limits<float>::max()) {
            groupShift = headerY - firstTabY; // distance from first tab to header
        }
    }

    for (int32_t tabIndex : animatedTabIndices) {
        auto tabItemNode = FindTabItemNodeBySwiperIndex(tabIndex);
        if (!tabItemNode) {
            continue;
        }
        auto geometry = tabItemNode->GetGeometryNode();
        if (!geometry) {
            continue;
        }
        TabAnimInfo info;
        info.node = tabItemNode;

        if (isCollapse) {
            // Collapse: tabs are VISIBLE (kept for animation). oldY == newY.
            // Animate from current position (translate=0) → shifted by groupShift, fade out.
            // All tabs move by the same distance so relative spacing is preserved.
            info.deltaY = 0.0f;
            info.targetDeltaY = groupShift;
            info.fade = true;
            info.startOpacity = 1.0f;
            info.endOpacity = 0.0f;
            info.needHideAfterAnim = true;
        } else {
            // Expand: tab item is now VISIBLE. Animate from shifted position → final position.
            // All tabs start at the same uniform offset so relative spacing is preserved.
            info.deltaY = groupShift;
            info.targetDeltaY = 0.0f;
            info.fade = true;
            info.startOpacity = 0.0f;
            info.endOpacity = 1.0f;
        }
        animInfos.push_back(std::move(info));
    }

    // Also animate pushed (still-visible, non-animated) tab items from old → new position.
    for (const auto& child : columnNode->GetChildren()) {
        if (!child || child->GetTag() == "TabsSideBarSectionHeader") {
            continue;
        }
        auto frameNode = AceType::DynamicCast<FrameNode>(child);
        if (!frameNode) {
            continue;
        }
        int32_t swiperIndex = FindSwiperIndexForTabItem(frameNode);
        if (swiperIndex < 0 || animatedTabIndices.count(swiperIndex) > 0) {
            continue;
        }
        auto oldIt = oldTabOffsets.find(swiperIndex);
        if (oldIt == oldTabOffsets.end()) {
            continue;
        }
        auto geometry = frameNode->GetGeometryNode();
        if (!geometry) {
            continue;
        }
        float newY = geometry->GetMarginFrameOffset().GetY();
        float oldY = oldIt->second;
        float delta = oldY - newY;
        if (isCollapse) {
            // Collapse: tabs are VISIBLE, so layout didn't change (oldY == newY, delta ≈ 0).
            // Pushed items need to move up by collapsePushDistance. Only animate items
            // that are below the collapsing section (Y > first collapsing tab's Y).
            if (NearZero(collapsePushDistance) ||
                !IsNodeBelowAnimatedSection(newY, animatedTabIndices)) {
                continue;
            }
            TabAnimInfo info;
            info.node = frameNode;
            info.deltaY = 0.0f; // start at current position
            info.targetDeltaY = -collapsePushDistance; // move up
            info.fade = false;
            animInfos.push_back(std::move(info));
        } else {
            if (NearZero(delta)) {
                continue;
            }
            TabAnimInfo info;
            info.node = frameNode;
            info.deltaY = delta;
            info.targetDeltaY = 0.0f;
            info.fade = false;
            animInfos.push_back(std::move(info));
        }
    }

    // Animate section headers that were pushed (moved) by the expand/collapse.
    // The triggered section's own header stays in place — only headers below it move.
    for (const auto& child : columnNode->GetChildren()) {
        if (!child || child->GetTag() != "TabsSideBarSectionHeader") {
            continue;
        }
        auto headerFrame = AceType::DynamicCast<FrameNode>(child);
        if (!headerFrame) {
            continue;
        }
        auto title = GetSectionHeaderTitle(headerFrame);
        if (title.empty()) {
            continue;
        }
        auto oldIt = oldHeaderOffsets.find(title);
        if (oldIt == oldHeaderOffsets.end()) {
            continue;
        }
        auto geometry = headerFrame->GetGeometryNode();
        if (!geometry) {
            continue;
        }
        float newY = geometry->GetMarginFrameOffset().GetY();
        float oldY = oldIt->second;
        float delta = oldY - newY;
        if (isCollapse) {
            // Collapse: layout didn't change (tabs are VISIBLE), so delta ≈ 0.
            // Headers below the collapsing section need to move up by collapsePushDistance.
            // Skip the triggered section's own header (it stays in place).
            if (title == triggeredSection) {
                continue;
            }
            if (NearZero(collapsePushDistance) ||
                !IsNodeBelowAnimatedSection(newY, animatedTabIndices)) {
                continue;
            }
            TabAnimInfo info;
            info.node = headerFrame;
            info.deltaY = 0.0f;
            info.targetDeltaY = -collapsePushDistance;
            info.fade = false;
            animInfos.push_back(std::move(info));
        } else {
            if (NearZero(delta)) {
                continue;
            }
            TabAnimInfo info;
            info.node = headerFrame;
            info.deltaY = delta;
            info.targetDeltaY = 0.0f;
            info.fade = false;
            animInfos.push_back(std::move(info));
        }
    }

    // Animate the footer container (sibling of columnNode inside contentContainer).
    // The footer sits below all tabs, so it is always "pushed" when tabs collapse/expand.
    // Collapse: footer moves up by collapsePushDistance (tabs kept VISIBLE, so layout
    //           hasn't changed yet; footer is at its old position).
    // Expand:   footer moves from old position (delta) to new position (0).
    if (!NearZero(collapsePushDistance) || !isCollapse) {
        auto contentContainer = AceType::DynamicCast<FrameNode>(columnNode->GetParent());
        if (contentContainer) {
            for (const auto& sibling : contentContainer->GetChildren()) {
                if (!sibling || sibling == columnNode) {
                    continue;
                }
                auto footerFrame = AceType::DynamicCast<FrameNode>(sibling);
                if (!footerFrame) {
                    continue;
                }
                TabAnimInfo info;
                info.node = footerFrame;
                if (isCollapse) {
                    info.deltaY = 0.0f;
                    info.targetDeltaY = -collapsePushDistance;
                } else {
                    // Expand: tabs are now VISIBLE, layout has updated, footer is at its
                    // new (lower) position. The old position was higher by expandDistance.
                    // So start with a negative translate (up) to visually place footer at
                    // its old position, then animate to 0 (new position).
                    float expandDistance = 0.0f;
                    for (int32_t tabIndex : animatedTabIndices) {
                        auto tabItemNode = FindTabItemNodeBySwiperIndex(tabIndex);
                        if (tabItemNode && tabItemNode->GetGeometryNode()) {
                            expandDistance += tabItemNode->GetGeometryNode()->GetMarginFrameSize().Height();
                        }
                    }
                    if (NearZero(expandDistance)) {
                        continue;
                    }
                    info.deltaY = -expandDistance; // start at old position (up)
                    info.targetDeltaY = 0.0f;      // animate to new position
                }
                info.fade = false;
                animInfos.push_back(std::move(info));
                break; // Only one footer sibling
            }
        }
    }

    if (animInfos.empty()) {
        return;
    }

    // Set initial transform/opacity (before animation start).
    for (const auto& info : animInfos) {
        // Disable hit test on fading tabs during animation to prevent misclicks
        // (e.g. user tapping a collapsing tab while it slides toward the header).
        if (info.fade) {
            info.node->SetHitTestMode(HitTestMode::HTMNONE);
        }
        auto renderContext = info.node->GetRenderContext();
        if (!renderContext) {
            continue;
        }
        renderContext->UpdateTransformTranslate(TranslateOptions(0.0f, info.deltaY, 0.0f));
        if (info.fade) {
            renderContext->UpdateOpacity(info.startOpacity);
        }
    }

    // Animate to final state (translate to targetDeltaY, target opacity).
    auto pipeline = columnNode->GetContextRefPtr();
    // Copy animInfos for the finish callback (the animate lambda moves the original).
    auto animInfosForFinish = animInfos;
    AnimationUtils::Animate(option, [animInfos = std::move(animInfos)]() {
        for (const auto& info : animInfos) {
            auto renderContext = info.node->GetRenderContext();
            if (!renderContext) {
                continue;
            }
            renderContext->UpdateTransformTranslate(TranslateOptions(0.0f, info.targetDeltaY, 0.0f));
            if (info.fade) {
                renderContext->UpdateOpacity(info.endOpacity);
            }
        }
    },
        // Finish callback: set collapsed tabs to GONE (triggers re-layout) and schedule
        // transform reset after the re-layout completes to avoid visual jumps.
        [animInfosForFinish = std::move(animInfosForFinish),
            weakPattern = AceType::WeakClaim(this)]() {
            for (const auto& info : animInfosForFinish) {
                if (info.needHideAfterAnim) {
                    auto prop = info.node->GetLayoutProperty();
                    if (prop) {
                        prop->UpdateVisibility(VisibleType::GONE);
                    }
                    info.node->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
                }
            }
            // Reset transforms after the re-layout so pushed items don't jump.
            auto pipeline = PipelineContext::GetCurrentContext();
            if (pipeline) {
                pipeline->AddAfterLayoutTask(
                    [animInfosForFinish, weakPattern]() {
                        for (const auto& info : animInfosForFinish) {
                            // Restore hit test mode after animation completes.
                            info.node->SetHitTestMode(HitTestMode::HTMDEFAULT);
                            auto renderContext = info.node->GetRenderContext();
                            if (renderContext) {
                                renderContext->UpdateTransformTranslate(TranslateOptions(0.0f, 0.0f, 0.0f));
                            }
                        }
                        auto pattern = weakPattern.Upgrade();
                        if (pattern) {
                            pattern->sectionAnimationRunning_ = false;
                            pattern->pendingAnimationSection_.clear();
                        }
                    });
            }
        },
        nullptr, pipeline);
}

void TabsSideBarTabListPattern::RebuildSections()
{
    auto host = AceType::DynamicCast<FrameNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabsNode_.Upgrade());
    CHECK_NULL_VOID(tabsNode);
    auto tabsProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    CHECK_NULL_VOID(tabsProperty);
    // Only apply sections in sidebar mode
    auto barLayoutStyle = tabsProperty->GetBarLayoutStyleValue(TabBarLayoutStyle::BOTTOM);
    if (barLayoutStyle != TabBarLayoutStyle::SIDEBAR && barLayoutStyle != TabBarLayoutStyle::SIDEBAR_ADAPTABLE) {
        return;
    }
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    if (swiperNode->TotalChildCount() == 0) {
        return;
    }
    // Check if any tab has a sidebar section configured
    bool hasAnySection = false;
    for (int32_t i = 0; i < swiperNode->TotalChildCount(); ++i) {
        auto tabContentNode = AceType::DynamicCast<FrameNode>(swiperNode->GetChildByIndex(i));
        CHECK_NULL_CONTINUE(tabContentNode);
        auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
        CHECK_NULL_CONTINUE(tabContentPattern);
        if (tabContentPattern->GetSidebarSection().has_value() &&
            !tabContentPattern->GetSidebarSection().value().title.empty()) {
            hasAnySection = true;
            break;
        }
    }
    if (!hasAnySection) {
        return;
    }
    std::vector<SectionInfo> sections;
    std::vector<int32_t> unsectionedIndices;
    // When search is active, treat all tabs as unsectioned (flat list, no headers,
    // no collapse state) so the filtered view behaves like no sections were set.
    if (!activeSearchText_.empty()) {
        auto swiperNodeForSearch = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
        CHECK_NULL_VOID(swiperNodeForSearch);
        for (int32_t i = 0; i < swiperNodeForSearch->TotalChildCount(); ++i) {
            unsectionedIndices.push_back(i);
        }
    } else {
        BuildSectionGroups(sections, unsectionedIndices);
        SortSections(sections);
    }
    ApplySectionLayout(sections, unsectionedIndices);
}
} // namespace OHOS::Ace::NG

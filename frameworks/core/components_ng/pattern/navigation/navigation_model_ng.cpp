/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/navigation/navigation_model_ng.h"

#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/shadow.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/divider/divider_layout_property.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/navigation/bar_item_event_hub.h"
#include "core/components_ng/pattern/navigation/bar_item_node.h"
#include "core/components_ng/pattern/navigation/bar_item_pattern.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_content_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_event_hub.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_node.h"
#include "core/components_ng/pattern/navigation/tool_bar_pattern.h"
#include "core/components_ng/pattern/navigator/navigator_event_hub.h"
#include "core/components_ng/pattern/navigator/navigator_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navrouter_group_node.h"
#include "core/components_ng/pattern/option/option_view.h"
#include "core/components_ng/pattern/select/select_model.h"
#include "core/components_ng/pattern/select/select_model_ng.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"
#include "frameworks/bridge/common/utils/engine_helper.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEXT_MAX_LINES_TWO = 2;
RefPtr<FrameNode> CreateBarItemTextNode(const std::string& text)
{
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, nodeId, AceType::MakeRefPtr<TextPattern>());
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent(text);
    textLayoutProperty->UpdateFontSize(TEXT_FONT_SIZE);
    textLayoutProperty->UpdateTextColor(TEXT_COLOR);
    textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    return textNode;
}

RefPtr<FrameNode> CreateBarItemIconNode(const std::string& src)
{
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ImageSourceInfo info(src);
    auto iconNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, nodeId, AceType::MakeRefPtr<ImagePattern>());
    auto imageLayoutProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, nullptr);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navigationGroupNode, nullptr);
    auto hub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(hub, nullptr);
    if (!hub->IsEnabled()) {
        info.SetFillColor(theme->GetMenuIconColor().BlendOpacity(theme->GetAlphaDisabled()));
    } else {
        info.SetFillColor(theme->GetMenuIconColor());
    }
    imageLayoutProperty->UpdateImageSourceInfo(info);

    auto iconSize = theme->GetMenuIconSize();
    imageLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(iconSize), CalcLength(iconSize)));
    iconNode->MarkModifyDone();
    return iconNode;
}

void UpdateBarItemNodeWithItem(const RefPtr<BarItemNode>& barItemNode, const BarItem& barItem)
{
    if (barItem.text.has_value() && !barItem.text.value().empty()) {
        auto textNode = CreateBarItemTextNode(barItem.text.value());
        barItemNode->SetTextNode(textNode);
        barItemNode->AddChild(textNode);
    }
    if (barItem.icon.has_value() && !barItem.icon.value().empty()) {
        auto iconNode = CreateBarItemIconNode(barItem.icon.value());
        barItemNode->SetIconNode(iconNode);
        barItemNode->AddChild(iconNode);
    }
    if (barItem.action) {
        auto eventHub = barItemNode->GetEventHub<BarItemEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->SetItemAction(barItem.action);
    }
    auto barItemPattern = barItemNode->GetPattern<BarItemPattern>();
    barItemNode->MarkModifyDone();
}

void UpdateOldBarItems(const RefPtr<UINode>& oldBarContainer, const std::vector<BarItem>& newBarItems)
{
    auto oldBarItems = oldBarContainer->GetChildren();
    auto prevChildrenSize = static_cast<int32_t>(oldBarItems.size());
    auto newChildrenSize = static_cast<int32_t>(newBarItems.size());
    auto oldIter = oldBarItems.begin();
    auto newIter = newBarItems.begin();
    // if old container has m items and incoming array has n items
    // we update first min(m, n) items in the old container
    for (int32_t i = 0; i < std::min(prevChildrenSize, newChildrenSize); i++) {
        do {
            auto oldBarItem = AceType::DynamicCast<BarItemNode>(*oldIter);
            BarItem newBarItem = *newIter;
            if (!oldBarItem) {
                break;
            }
            if (newBarItem.text.has_value()) {
                oldBarItem->UpdateText(newBarItem.text.value());
                if (oldBarItem->GetTextNode()) {
                    auto textNode = AceType::DynamicCast<FrameNode>(oldBarItem->GetTextNode());
                    CHECK_NULL_VOID(textNode);
                    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
                    CHECK_NULL_VOID(textLayoutProperty);
                    textLayoutProperty->UpdateContent(newBarItem.text.value());
                    textNode->MarkModifyDone();
                } else {
                    auto textNode = CreateBarItemTextNode(newBarItem.text.value());
                    oldBarItem->SetTextNode(textNode);
                    oldBarItem->AddChild(textNode);
                    oldBarItem->MarkModifyDone();
                }
            } else {
                oldBarItem->ResetText();
                if (oldBarItem->GetTextNode()) {
                    auto textNode = AceType::DynamicCast<FrameNode>(oldBarItem->GetTextNode());
                    CHECK_NULL_VOID(textNode);
                    oldBarItem->RemoveChild(textNode);
                }
            }
            if (newBarItem.icon.has_value()) {
                oldBarItem->UpdateIconSrc(newBarItem.icon.value());
                if (oldBarItem->GetIconNode()) {
                    auto iconNode = AceType::DynamicCast<FrameNode>(oldBarItem->GetIconNode());
                    CHECK_NULL_VOID(iconNode);
                    auto imageLayoutProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
                    CHECK_NULL_VOID(imageLayoutProperty);
                    imageLayoutProperty->UpdateImageSourceInfo(ImageSourceInfo(newBarItem.icon.value()));
                    iconNode->MarkModifyDone();
                } else {
                    auto iconNode = CreateBarItemIconNode(newBarItem.icon.value());
                    oldBarItem->SetIconNode(iconNode);
                    oldBarItem->AddChild(iconNode);
                    oldBarItem->MarkModifyDone();
                }
            } else {
                oldBarItem->ResetIconSrc();
                if (oldBarItem->GetIconNode()) {
                    auto iconNode = AceType::DynamicCast<FrameNode>(oldBarItem->GetIconNode());
                    CHECK_NULL_VOID(iconNode);
                    oldBarItem->RemoveChild(iconNode);
                }
            }
        } while (false);
        oldIter++;
        newIter++;
    }
    // if m > n, we remove (m - n) children from the back of old container
    if (prevChildrenSize > newChildrenSize) {
        for (int32_t i = 0; i < prevChildrenSize - newChildrenSize; i++) {
            oldBarContainer->RemoveChild(oldBarItems.back());
            oldBarItems.pop_back();
        }
    } else if (prevChildrenSize < newChildrenSize) {
        // if m < n, we add (n - m) children created by info in new item list
        for (int32_t i = 0; i < newChildrenSize - prevChildrenSize; i++) {
            auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
            auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, nodeId);
            barItemNode->InitializePatternAndContext();
            UpdateBarItemNodeWithItem(barItemNode, *newIter);
            oldBarContainer->AddChild(barItemNode);
            newIter++;
        }
    }
    auto container = AceType::DynamicCast<TitleBarNode>(oldBarContainer);
    CHECK_NULL_VOID(container);
    container->MarkModifyDone();
}

void CreateToolBarDividerNode(const RefPtr<NavBarNode>& navBarNode)
{
    int32_t dividerNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto dividerNode = FrameNode::GetOrCreateFrameNode(
        V2::DIVIDER_ETS_TAG, dividerNodeId, []() { return AceType::MakeRefPtr<DividerPattern>(); });
    navBarNode->AddChild(dividerNode);
    auto dividerLayoutProperty = dividerNode->GetLayoutProperty<DividerLayoutProperty>();
    CHECK_NULL_VOID(dividerLayoutProperty);
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    dividerLayoutProperty->UpdateStrokeWidth(theme->GetToolBarDividerWidth());
    dividerLayoutProperty->UpdateVertical(false);
    auto dividerRenderProperty = dividerNode->GetPaintProperty<DividerRenderProperty>();
    CHECK_NULL_VOID(dividerRenderProperty);
    dividerRenderProperty->UpdateDividerColor(theme->GetToolBarDividerColor());
    navBarNode->SetToolBarDividerNode(dividerNode);
}

RefPtr<FrameNode> CreateToolbarItemsContainerNode(const RefPtr<FrameNode>& toolBarNode)
{
    int32_t containerNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto containerNode = FrameNode::GetOrCreateFrameNode(
        V2::TOOL_BAR_ETS_TAG, containerNodeId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    CHECK_NULL_RETURN(containerNode, nullptr);
    auto containerRowProperty = containerNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(containerRowProperty, nullptr);
    containerRowProperty->UpdateMainAxisAlign(FlexAlign::SPACE_EVENLY);
    toolBarNode->AddChild(containerNode);
    return containerNode;
}

RefPtr<FrameNode> CreateToolbarItemTextNode(const std::string& text)
{
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, nodeId, AceType::MakeRefPtr<TextPattern>());
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, nullptr);
    textLayoutProperty->UpdateContent(text);
    textLayoutProperty->UpdateFontSize(theme->GetToolBarItemFontSize());
    textLayoutProperty->UpdateTextColor(theme->GetToolBarItemFontColor());
    textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
    textLayoutProperty->UpdateAdaptMinFontSize(theme->GetToolBarItemMinFontSize());
    textLayoutProperty->UpdateAdaptMaxFontSize(theme->GetToolBarItemFontSize());
    textLayoutProperty->UpdateMaxLines(theme->GetToolbarItemTextMaxLines());
    textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);

    textLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), std::nullopt));
    return textNode;
}

RefPtr<FrameNode> CreateToolbarItemIconNode(const BarItem& barItem)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, nullptr);
    if (barItem.iconSymbol.has_value() && barItem.iconSymbol.value() != nullptr) {
        auto iconNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        CHECK_NULL_RETURN(iconNode, nullptr);
        auto symbolProperty = iconNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_RETURN(symbolProperty, nullptr);
        symbolProperty->UpdateFontSize(theme->GetToolbarIconSize());
        symbolProperty->UpdateSymbolColorList({ theme->GetToolbarIconColor() });
        barItem.iconSymbol.value()(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(iconNode)));

        iconNode->MarkModifyDone();
        return iconNode;
    }
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ImageSourceInfo info(barItem.icon.value());
    auto iconNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, nodeId, AceType::MakeRefPtr<ImagePattern>());
    auto imageLayoutProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);

    info.SetFillColor(theme->GetToolbarIconColor());
    imageLayoutProperty->UpdateImageSourceInfo(info);

    auto iconSize = theme->GetToolbarIconSize();
    imageLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(iconSize), CalcLength(iconSize)));

    iconNode->MarkModifyDone();
    return iconNode;
}

bool CheckNavigationGroupEnableStatus()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navigationGroupNode, false);
    auto eventHub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    return eventHub->IsEnabled();
}

void RegisterToolbarHotZoneEvent(const RefPtr<FrameNode>& buttonNode, const RefPtr<BarItemNode>& barItemNode)
{
    auto gestureEventHub = buttonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto clickCallback = [weakNode = WeakPtr<BarItemNode>(barItemNode)](GestureEvent& info) {
        if (info.GetSourceDevice() == SourceType::KEYBOARD) {
            return;
        }
        auto barItemNode = weakNode.Upgrade();
        auto eventHub = barItemNode->GetEventHub<BarItemEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto pattern = barItemNode->GetPattern<BarItemPattern>();
        CHECK_NULL_VOID(pattern);
        eventHub->FireItemAction();
        pattern->UpdateBarItemActiveStatusResource();
    };
    gestureEventHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(clickCallback));
}

void UpdateBarItemPattern(const RefPtr<BarItemNode>& barItemNode, const BarItem& barItem)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    auto barItemPattern = barItemNode->GetPattern<BarItemPattern>();
    if (barItem.status == NG::NavToolbarItemStatus::ACTIVE &&
        ((barItem.activeIcon.has_value() && !barItem.activeIcon.value().empty()) ||
            (barItem.activeIconSymbol.has_value() && barItem.activeIconSymbol.value() != nullptr)) &&
        ((barItem.icon.has_value() && !barItem.icon.value().empty()) ||
            (barItem.iconSymbol.has_value() && barItem.iconSymbol.value() != nullptr))) {
        if (barItem.iconSymbol.has_value()) {
            barItemPattern->SetInitialIconSymbol(barItem.iconSymbol.value());
        } else if (barItem.icon.has_value()) {
            ImageSourceInfo initialIconInfo(barItem.icon.value());
            initialIconInfo.SetFillColor(theme->GetToolbarIconColor());
            barItemPattern->SetInitialIconImageSourceInfo(initialIconInfo);
        }

        if (barItem.activeIconSymbol.has_value()) {
            barItemPattern->SetActiveIconSymbol(barItem.activeIconSymbol.value());
        } else if (barItem.activeIcon.has_value()) {
            ImageSourceInfo activeIconInfo(barItem.activeIcon.value());
            activeIconInfo.SetFillColor(theme->GetToolbarActiveIconColor());
            barItemPattern->SetActiveIconImageSourceInfo(activeIconInfo);
        }
        barItemPattern->SetToolbarItemStatus(barItem.status);
        barItemPattern->SetCurrentIconStatus(NG::ToolbarIconStatus::INITIAL);
    }
}

void UpdateToolbarItemNodeWithConfiguration(
    const RefPtr<BarItemNode>& barItemNode, const BarItem& barItem, const RefPtr<FrameNode>& buttonNode)
{
    barItemNode->SetBarItemUsedInToolbarConfiguration(true);
    if (barItem.text.has_value() && !barItem.text.value().empty()) {
        auto textNode = CreateToolbarItemTextNode(barItem.text.value());
        barItemNode->SetTextNode(textNode);
        barItemNode->AddChild(textNode);
    }
    if ((barItem.icon.has_value() && !barItem.icon.value().empty())
        || (barItem.iconSymbol.has_value() && barItem.iconSymbol.value() != nullptr)) {
        auto iconNode = CreateToolbarItemIconNode(barItem);
        barItemNode->SetIconNode(iconNode);
        barItemNode->AddChild(iconNode);
    }
    if (barItem.action) {
        auto eventHub = barItemNode->GetEventHub<BarItemEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->SetItemAction(barItem.action);
        RegisterToolbarHotZoneEvent(buttonNode, barItemNode);
    }

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    bool navigationEnableStatus = CheckNavigationGroupEnableStatus();
    if (barItem.status == NG::NavToolbarItemStatus::DISABLED || !navigationEnableStatus) {
        auto renderContext = barItemNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->UpdateOpacity(theme->GetToolbarItemDisabledAlpha());

        auto itemEventHub = barItemNode->GetEventHub<BarItemEventHub>();
        CHECK_NULL_VOID(itemEventHub);
        itemEventHub->SetEnabled(false);

        auto buttonEventHub = buttonNode->GetEventHub<ButtonEventHub>();
        CHECK_NULL_VOID(buttonEventHub);
        buttonEventHub->SetEnabled(false);
    }

    UpdateBarItemPattern(barItemNode, barItem);
    barItemNode->MarkModifyDone();
}

void AddSafeIntervalBetweenToolbarItem(
    MarginProperty& margin, uint32_t count, size_t toolbarItemSize, bool needMoreButton)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    if (count == ONE_TOOLBAR_ITEM && toolbarItemSize != ONE_TOOLBAR_ITEM) {
        margin.right = CalcLength(theme->GetToolbarItemMargin());
    } else if (!needMoreButton && (count == toolbarItemSize) && (toolbarItemSize != ONE_TOOLBAR_ITEM)) {
        margin.left = CalcLength(theme->GetToolbarItemMargin());
    } else if (toolbarItemSize == ONE_TOOLBAR_ITEM) {
        margin.left = CalcLength(theme->GetToolbarItemSpecialMargin());
        margin.right = CalcLength(theme->GetToolbarItemSpecialMargin());
    } else {
        margin.left = CalcLength(theme->GetToolbarItemMargin());
        margin.right = CalcLength(theme->GetToolbarItemMargin());
    }
}

RefPtr<FrameNode> CreateToolbarItemInContainer(
    const NG::BarItem& toolBarItem, size_t toolbarItemSize, uint32_t count, bool needMoreButton)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, nullptr);
    auto buttonPattern = AceType::MakeRefPtr<NG::ButtonPattern>();
    CHECK_NULL_RETURN(buttonPattern, nullptr);
    buttonPattern->setComponentButtonType(ComponentButtonType::NAVIGATION);
    buttonPattern->SetFocusBorderColor(theme->GetToolBarItemFocusColor());
    buttonPattern->SetFocusBorderWidth(theme->GetToolBarItemFocusBorderWidth());
    auto toolBarItemNode = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), buttonPattern);
    CHECK_NULL_RETURN(toolBarItemNode, nullptr);
    auto toolBarItemLayoutProperty = toolBarItemNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(toolBarItemLayoutProperty, nullptr);
    toolBarItemLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(std::nullopt, CalcLength(theme->GetToolbarItemHeigth())));
    toolBarItemLayoutProperty->UpdateType(ButtonType::NORMAL);
    toolBarItemLayoutProperty->UpdateBorderRadius(theme->GetToolBarItemBorderRadius());
    auto renderContext = toolBarItemNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    MarginProperty margin;
    AddSafeIntervalBetweenToolbarItem(margin, count, toolbarItemSize, needMoreButton);
    toolBarItemLayoutProperty->UpdateMargin(margin);

    PaddingProperty padding;
    padding.left = CalcLength(theme->GetToolbarItemLeftOrRightPadding());
    padding.right = CalcLength(theme->GetToolbarItemLeftOrRightPadding());
    padding.top = CalcLength(theme->GetToolbarItemTopPadding());
    padding.bottom = CalcLength(theme->GetToolbarItemBottomPadding());
    toolBarItemLayoutProperty->UpdatePadding(padding);

    int32_t barItemNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, barItemNodeId);
    barItemNode->InitializePatternAndContext();
    UpdateToolbarItemNodeWithConfiguration(barItemNode, toolBarItem, toolBarItemNode);
    auto barItemLayoutProperty = barItemNode->GetLayoutProperty();
    CHECK_NULL_RETURN(barItemLayoutProperty, nullptr);
    barItemLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

    barItemNode->MountToParent(toolBarItemNode);
    toolBarItemNode->MarkModifyDone();

    return toolBarItemNode;
}

void BuildToolbarMoreItemNode(const RefPtr<BarItemNode>& barItemNode)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto hub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto renderContext = barItemNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

    if (!hub->IsEnabled()) {
        renderContext->UpdateOpacity(theme->GetToolbarItemDisabledAlpha());
    }

    auto textNode = CreateToolbarItemTextNode(Localization::GetInstance()->GetEntryLetters("common.more"));
    CHECK_NULL_VOID(textNode);
    barItemNode->SetTextNode(textNode);
    barItemNode->SetBarItemUsedInToolbarConfiguration(true);
    barItemNode->AddChild(textNode);

    auto symbolNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_VOID(symbolNode);
    auto symbolProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(symbolProperty);
    symbolProperty->UpdateSymbolSourceInfo(SymbolSourceInfo(theme->GetMoreSymbolId()));
    symbolProperty->UpdateFontSize(theme->GetToolbarIconSize());
    symbolProperty->UpdateSymbolColorList({ theme->GetToolbarIconColor() });
    symbolNode->MarkModifyDone();
    barItemNode->SetIconNode(symbolNode);
    barItemNode->AddChild(symbolNode);
    barItemNode->MarkModifyDone();
}

RefPtr<FrameNode> CreateToolbarMoreMenuNode(const RefPtr<BarItemNode>& barItemNode)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, nullptr);
    auto buttonPattern = AceType::MakeRefPtr<NG::ButtonPattern>();
    CHECK_NULL_RETURN(buttonPattern, nullptr);
    buttonPattern->setComponentButtonType(ComponentButtonType::NAVIGATION);
    buttonPattern->SetFocusBorderColor(theme->GetToolBarItemFocusColor());
    buttonPattern->SetFocusBorderWidth(theme->GetToolBarItemFocusBorderWidth());
    auto toolBarItemNode = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), buttonPattern);
    CHECK_NULL_RETURN(toolBarItemNode, nullptr);
    auto menuItemLayoutProperty = toolBarItemNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(menuItemLayoutProperty, nullptr);
    menuItemLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(std::nullopt, CalcLength(theme->GetToolbarItemHeigth())));
    menuItemLayoutProperty->UpdateType(ButtonType::NORMAL);
    menuItemLayoutProperty->UpdateBorderRadius(theme->GetToolBarItemBorderRadius());

    auto renderContext = toolBarItemNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    MarginProperty menuButtonMargin;
    menuButtonMargin.left = CalcLength(theme->GetToolbarItemMargin());
    menuItemLayoutProperty->UpdateMargin(menuButtonMargin);

    PaddingProperty padding;
    padding.left = CalcLength(theme->GetToolbarItemLeftOrRightPadding());
    padding.right = CalcLength(theme->GetToolbarItemLeftOrRightPadding());
    padding.top = CalcLength(theme->GetToolbarItemTopPadding());
    padding.bottom = CalcLength(theme->GetToolbarItemBottomPadding());
    menuItemLayoutProperty->UpdatePadding(padding);

    barItemNode->MountToParent(toolBarItemNode);
    barItemNode->MarkModifyDone();
    toolBarItemNode->MarkModifyDone();

    return toolBarItemNode;
}

void BuildToolbarMoreMenuNodeAction(
    const RefPtr<BarItemNode>& barItemNode, const RefPtr<FrameNode>& barMenuNode, const RefPtr<FrameNode>& buttonNode)
{
    auto eventHub = barItemNode->GetEventHub<BarItemEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto context = PipelineContext::GetCurrentContext();
    auto clickCallback = [weakContext = WeakPtr<PipelineContext>(context), id = barItemNode->GetId(),
                             weakMenu = WeakPtr<FrameNode>(barMenuNode),
                             weakBarItemNode = WeakPtr<BarItemNode>(barItemNode)]() {
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);

        auto overlayManager = context->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);

        auto menu = weakMenu.Upgrade();
        CHECK_NULL_VOID(menu);

        auto barItemNode = weakBarItemNode.Upgrade();
        CHECK_NULL_VOID(barItemNode);

        auto imageNode = barItemNode->GetChildAtIndex(0);
        CHECK_NULL_VOID(imageNode);

        auto imageFrameNode = AceType::DynamicCast<FrameNode>(imageNode);
        CHECK_NULL_VOID(imageFrameNode);
        auto imgOffset = imageFrameNode->GetOffsetRelativeToWindow();
        auto imageSize = imageFrameNode->GetGeometryNode()->GetFrameSize();

        auto menuNode = AceType::DynamicCast<FrameNode>(menu->GetChildAtIndex(0));
        CHECK_NULL_VOID(menuNode);
        auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
        CHECK_NULL_VOID(menuLayoutProperty);
        menuLayoutProperty->UpdateTargetSize(imageSize);
        auto menuPattern = menuNode->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);
        menuPattern->SetIsSelectMenu(true);

        bool isRightToLeft = AceApplicationInfo::GetInstance().IsRightToLeft();
        if (isRightToLeft) {
            imgOffset.SetX(imgOffset.GetX() + imageSize.Width());
        } else {
            imgOffset.SetX(imgOffset.GetX());
        }
        imgOffset.SetY(imgOffset.GetY() - imageSize.Height());
        overlayManager->ShowMenu(id, imgOffset, menu);
    };
    eventHub->SetItemAction(clickCallback);
    RegisterToolbarHotZoneEvent(buttonNode, barItemNode);
}
} // namespace

void NavigationModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    // navigation node
    int32_t nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVIGATION_VIEW_ETS_TAG, nodeId);
    auto navigationGroupNode = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    if (!CreateNavBarNodeIfNeeded(navigationGroupNode) ||  // navBar node
        !CreateContentNodeIfNeeded(navigationGroupNode) || // content node
        !CreateDividerNodeIfNeeded(navigationGroupNode)) { // divider node
        return;
    }

    stack->Push(navigationGroupNode);
    auto navigationLayoutProperty = navigationGroupNode->GetLayoutProperty<NavigationLayoutProperty>();
    if (!navigationLayoutProperty->HasNavigationMode()) {
        navigationLayoutProperty->UpdateNavigationMode(NavigationMode::AUTO);
    }
    navigationLayoutProperty->UpdateNavBarWidth(DEFAULT_NAV_BAR_WIDTH);
}

bool NavigationModelNG::CreateNavBarNodeIfNeeded(const RefPtr<NavigationGroupNode>& navigationGroupNode)
{
    if (!navigationGroupNode->GetNavBarNode()) {
        int32_t navBarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVBAR_ETS_TAG, navBarNodeId);
        auto navBarNode = NavBarNode::GetOrCreateNavBarNode(
            V2::NAVBAR_ETS_TAG, navBarNodeId, []() { return AceType::MakeRefPtr<NavBarPattern>(); });
        navBarNode->SetJSViewActive(true);
        auto navBarRenderContext = navBarNode->GetRenderContext();
        CHECK_NULL_RETURN(navBarRenderContext, false);
        navBarRenderContext->UpdateClipEdge(true);
        navigationGroupNode->AddChild(navBarNode);
        navigationGroupNode->SetNavBarNode(navBarNode);

        if (!CreateNavBarNodeChildsIfNeeded(navBarNode)) {
            return false;
        }

        auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
        CHECK_NULL_RETURN(navBarLayoutProperty, false);
        navBarLayoutProperty->UpdateTitleMode(NavigationTitleMode::FREE);
    }

    return true;
}

bool NavigationModelNG::CreateNavBarNodeChildsIfNeeded(const RefPtr<NavBarNode>& navBarNode)
{
    // titleBar node
    if (!navBarNode->GetTitleBarNode()) {
        int32_t titleBarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::TITLE_BAR_ETS_TAG, titleBarNodeId);
        auto titleBarNode = TitleBarNode::GetOrCreateTitleBarNode(
            V2::TITLE_BAR_ETS_TAG, titleBarNodeId, []() { return AceType::MakeRefPtr<TitleBarPattern>(); });
        navBarNode->AddChild(titleBarNode);
        navBarNode->SetTitleBarNode(titleBarNode);
    }

    // navBar content node
    if (!navBarNode->GetNavBarContentNode()) {
        int32_t navBarContentNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVBAR_CONTENT_ETS_TAG, navBarContentNodeId);
        auto navBarContentNode = FrameNode::GetOrCreateFrameNode(V2::NAVBAR_CONTENT_ETS_TAG, navBarContentNodeId,
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto navBarContentRenderContext = navBarContentNode->GetRenderContext();
        CHECK_NULL_RETURN(navBarContentRenderContext, false);
        navBarContentRenderContext->UpdateClipEdge(true);
        navBarNode->AddChild(navBarContentNode);
        navBarNode->SetNavBarContentNode(navBarContentNode);

        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
                .edges = SAFE_AREA_EDGE_ALL };
            navBarContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
        }
    }

    // toolBar node
    if (!navBarNode->GetToolBarNode()) {
        int32_t toolBarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::TOOL_BAR_ETS_TAG, toolBarNodeId);
        auto toolBarNode = NavToolbarNode::GetOrCreateToolbarNode(
            V2::TOOL_BAR_ETS_TAG, toolBarNodeId, []() { return AceType::MakeRefPtr<NavToolbarPattern>(); });
        toolBarNode->MarkModifyDone();
        auto property = toolBarNode->GetLayoutProperty();
        CHECK_NULL_RETURN(property, false);
        property->UpdateVisibility(VisibleType::GONE);
        navBarNode->AddChild(toolBarNode);
        navBarNode->SetToolBarNode(toolBarNode);
        navBarNode->SetPreToolBarNode(toolBarNode);
        navBarNode->UpdatePrevToolBarIsCustom(false);
    }

    return true;
}

bool NavigationModelNG::CreateContentNodeIfNeeded(const RefPtr<NavigationGroupNode>& navigationGroupNode)
{
    if (!navigationGroupNode->GetContentNode()) {
        int32_t contentNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVIGATION_CONTENT_ETS_TAG, contentNodeId);
        auto contentNode = FrameNode::GetOrCreateFrameNode(V2::NAVIGATION_CONTENT_ETS_TAG, contentNodeId,
            []() { return AceType::MakeRefPtr<NavigationContentPattern>(); });
        contentNode->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
        contentNode->GetEventHub<EventHub>()->GetOrCreateGestureEventHub()->SetHitTestMode(
            HitTestMode::HTMTRANSPARENT_SELF);
        navigationGroupNode->AddChild(contentNode);
        navigationGroupNode->SetContentNode(contentNode);
    }

    return true;
}

bool NavigationModelNG::CreateDividerNodeIfNeeded(const RefPtr<NavigationGroupNode>& navigationGroupNode)
{
    if (!navigationGroupNode->GetDividerNode()) {
        int32_t dividerNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::DIVIDER_ETS_TAG, dividerNodeId);
        auto dividerNode = FrameNode::GetOrCreateFrameNode(
            V2::DIVIDER_ETS_TAG, dividerNodeId, []() { return AceType::MakeRefPtr<DividerPattern>(); });
        navigationGroupNode->AddChild(dividerNode);
        navigationGroupNode->SetDividerNode(dividerNode);

        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
                .edges = SAFE_AREA_EDGE_ALL };
            dividerNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
        }

        auto dividerLayoutProperty = dividerNode->GetLayoutProperty<DividerLayoutProperty>();
        CHECK_NULL_RETURN(dividerLayoutProperty, false);
        dividerLayoutProperty->UpdateStrokeWidth(DIVIDER_WIDTH);
        dividerLayoutProperty->UpdateVertical(true);
        auto theme = NavigationGetTheme();
        CHECK_NULL_RETURN(theme, false);
        auto dividerRenderProperty = dividerNode->GetPaintProperty<DividerRenderProperty>();
        CHECK_NULL_RETURN(dividerRenderProperty, false);
        dividerRenderProperty->UpdateDividerColor(Color::TRANSPARENT);
        dividerNode->GetRenderContext()->UpdateBackgroundColor(theme->GetNavigationDividerColor());
    }

    return true;
}

bool NavigationModelNG::ParseCommonTitle(
    bool hasSubTitle, bool hasMainTitle, const std::string& subtitle, const std::string& title, bool ignoreMainTitle)
{
    if (!hasSubTitle && !hasMainTitle) {
        return false;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navigationGroupNode, false);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_RETURN(navBarNode, false);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleBarNode, false);
    auto titleBarPattern = titleBarNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_RETURN(titleBarPattern, false);
    titleBarPattern->SetIsTitleChanged(true);
    if (navBarNode->GetPrevTitleIsCustomValue(false)) {
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        titleBarNode->SetTitle(nullptr);
        auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
        CHECK_NULL_RETURN(titleBarLayoutProperty, false);
        if (titleBarLayoutProperty->HasTitleHeight()) {
            titleBarLayoutProperty->ResetTitleHeight();
            navBarNode->GetLayoutProperty<NavBarLayoutProperty>()->ResetTitleMode();
        }
    }
    navBarNode->UpdatePrevTitleIsCustom(false);

    // create or update main title
    do {
        if (ignoreMainTitle) {
            break;
        }
        auto mainTitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
        if (!hasMainTitle) {
            // remove main title if any.
            titleBarNode->RemoveChild(mainTitle);
            titleBarNode->SetTitle(nullptr);
            break;
        }

        if (mainTitle) {
            // update main title
            auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
            textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : TEXT_MAX_LINES_TWO);
            textLayoutProperty->UpdateContent(title);
            break;
        }
        // create and init main title
        mainTitle = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
        auto theme = NavigationGetTheme();
        Color mainTitleColor = theme->GetTitleColor();
        FontWeight mainTitleWeight = FontWeight::MEDIUM;
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            mainTitleColor = theme->GetMainTitleFontColor();
            mainTitleWeight = FontWeight::BOLD;
        }
        textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : TEXT_MAX_LINES_TWO);
        textLayoutProperty->UpdateContent(title);
        textLayoutProperty->UpdateTextColor(mainTitleColor);
        textLayoutProperty->UpdateFontWeight(mainTitleWeight);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        titleBarNode->SetTitle(mainTitle);
        titleBarNode->AddChild(mainTitle);
    } while (false);

    // create or update subtitle
    auto subTitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    if (!hasSubTitle) {
        // remove subtitle if any.
        titleBarNode->RemoveChild(subTitle);
        titleBarNode->SetSubtitle(nullptr);
        return true;
    }
    if (subTitle) {
        // update subtitle
        auto textLayoutProperty = subTitle->GetLayoutProperty<TextLayoutProperty>();
        textLayoutProperty->UpdateContent(subtitle);
        auto renderContext = subTitle->GetRenderContext();
        renderContext->UpdateOpacity(1.0);
    } else {
        // create and init subtitle
        subTitle = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        auto textLayoutProperty = subTitle->GetLayoutProperty<TextLayoutProperty>();
        auto theme = NavigationGetTheme();
        Color subTitleColor = theme->GetSubTitleColor();
        auto subTitleSize = theme->GetSubTitleFontSize();
        FontWeight subTitleWeight = FontWeight::REGULAR; // ohos_id_text_font_family_regular
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            subTitleSize = theme->GetSubTitleFontSizeS();
            subTitleColor = theme->GetSubTitleFontColor();
        }
        textLayoutProperty->UpdateContent(subtitle);
        textLayoutProperty->UpdateFontSize(subTitleSize);
        textLayoutProperty->UpdateTextColor(subTitleColor);
        textLayoutProperty->UpdateFontWeight(subTitleWeight);
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        titleBarNode->SetSubtitle(subTitle);
        titleBarNode->AddChild(subTitle);
    }
    return true;
}

void NavigationModelNG::SetTitle(const std::string& title, bool hasSubTitle) {}

void NavigationModelNG::SetCustomTitle(const RefPtr<AceType>& customNode)
{
    auto customTitle = AceType::DynamicCast<NG::UINode>(customNode);
    CHECK_NULL_VOID(customTitle);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);

    if (!navBarNode->GetPrevTitleIsCustomValue(false)) {
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        titleBarNode->RemoveChild(titleBarNode->GetSubtitle());
        titleBarNode->SetTitle(nullptr);
        titleBarNode->SetSubtitle(nullptr);
    }
    navBarNode->UpdatePrevTitleIsCustom(true);

    auto currentTitle = titleBarNode->GetTitle();
    if (currentTitle && customTitle->GetId() == currentTitle->GetId()) {
        // do nothing
        return;
    }
    // update custom title
    titleBarNode->RemoveChild(currentTitle);
    titleBarNode->SetTitle(customTitle);
    titleBarNode->AddChild(customTitle);
}

void NavigationModelNG::SetTitleHeight(const Dimension& height, bool isValid)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (!isValid) {
        titleBarLayoutProperty->ResetTitleHeight();
        return;
    }
    titleBarLayoutProperty->UpdateTitleHeight(height);
    SetHideBackButton(true);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateTitleMode(static_cast<NG::NavigationTitleMode>(NavigationTitleMode::MINI));
}

void NavigationModelNG::SetTitleMode(NG::NavigationTitleMode mode)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    const auto& titleHeightProperty = titleBarLayoutProperty->GetTitleHeight();
    if (titleHeightProperty.has_value()) {
        mode = NavigationTitleMode::MINI;
    }
    navBarLayoutProperty->UpdateTitleMode(static_cast<NG::NavigationTitleMode>(mode));
    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
    if (mode != NavigationTitleMode::MINI) {
        // remove back button if any.
        titleBarNode->RemoveChild(backButtonNode);
        titleBarNode->SetBackButton(nullptr);
        return;
    }

    if (backButtonNode != nullptr) {
        return;
    }
    // create back button
    backButtonNode = FrameNode::CreateFrameNode(
        V2::BACK_BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());
    auto gestureEventHub = backButtonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto context = PipelineContext::GetCurrentContext();
    auto clickCallback = [weakContext = WeakPtr<PipelineContext>(context)](GestureEvent& /* info */) {
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);
        bool result = context->OnBackPressed();
        if (!result) {
            auto delegate = EngineHelper::GetCurrentDelegate();
            CHECK_NULL_VOID(delegate);
            delegate->Back("");
        }
    };
    gestureEventHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(clickCallback));
    auto buttonPattern = backButtonNode->GetPattern<ButtonPattern>();
    CHECK_NULL_VOID(buttonPattern);
    auto theme = NavigationGetTheme();
    buttonPattern->SetSkipColorConfigurationUpdate();
    buttonPattern->setComponentButtonType(ComponentButtonType::NAVIGATION);
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        buttonPattern->SetBlendColor(theme->GetBackgroundPressedColor(), theme->GetBackgroundHoverColor());
        buttonPattern->SetFocusBorderColor(theme->GetBackgroundFocusOutlineColor());
        buttonPattern->SetFocusBorderWidth(theme->GetBackgroundFocusOutlineWeight());
    }
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(backButtonLayoutProperty);
    auto renderContext = backButtonNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto backButtonWidth = BACK_BUTTON_SIZE;
    auto backButtonHeight = BACK_BUTTON_SIZE;
    auto backButtonRadiusSize = BUTTON_RADIUS_SIZE;
    auto backButtonPadding = BUTTON_PADDING;
    auto backButtonColor = Color::TRANSPARENT;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        backButtonWidth = theme->GetIconBackgroundWidth();
        backButtonHeight = theme->GetIconBackgroundHeight();
        backButtonRadiusSize = theme->GetCornerRadius();
        backButtonPadding = MENU_BUTTON_PADDING;
        backButtonColor = theme->GetCompBackgroundColor();
    }
    backButtonLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(backButtonWidth), CalcLength(backButtonHeight)));
    backButtonLayoutProperty->UpdateBorderRadius(BorderRadiusProperty(backButtonRadiusSize));
    renderContext->UpdateBackgroundColor(backButtonColor);
    PaddingProperty padding;
    padding.SetEdges(CalcLength(backButtonPadding));
    backButtonLayoutProperty->UpdatePadding(padding);
    backButtonLayoutProperty->UpdateType(ButtonType::NORMAL);
    backButtonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

    auto eventHub = backButtonNode->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(eventHub);

    auto symbolNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_VOID(symbolNode);
    auto symbolProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(symbolProperty);
    symbolProperty->UpdateSymbolSourceInfo(SymbolSourceInfo(theme->GetBackSymbolId()));
    symbolProperty->UpdateFontSize(BACK_BUTTON_ICON_SIZE);
    auto navigationEventHub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(navigationEventHub);
    if (!navigationEventHub->IsEnabled()) {
        symbolProperty->UpdateSymbolColorList(
            { theme->GetBackButtonIconColor().BlendOpacity(theme->GetAlphaDisabled()) });
    } else {
        symbolProperty->UpdateSymbolColorList({ theme->GetBackButtonIconColor() });
    }
    symbolNode->MountToParent(backButtonNode);
    symbolNode->MarkModifyDone();
    backButtonNode->MarkModifyDone();
    titleBarNode->SetBackButton(backButtonNode);
    titleBarNode->AddChild(backButtonNode, 0);
}

void NavigationModelNG::SetSubtitle(const std::string& subtitle)
{
    ParseCommonTitle(true, false, subtitle, "", true);
}

void NavigationModelNG::SetHideTitleBar(bool hideTitleBar)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideTitleBar(hideTitleBar);
}

void NavigationModelNG::SetHideNavBar(bool hideNavBar)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    auto navigationLayoutProperty = navigationGroupNode->GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(navigationLayoutProperty);

    auto lastHideNavBarValue = navigationLayoutProperty->GetHideNavBar();
    if (lastHideNavBarValue.has_value()) {
        pattern->SetNavBarVisibilityChange(hideNavBar != lastHideNavBarValue.value());
    } else {
        pattern->SetNavBarVisibilityChange(true);
    }
    auto navBarNode = AceType::DynamicCast<FrameNode>(navigationGroupNode->GetNavBarNode());
    auto layoutProperty = navBarNode->GetLayoutProperty();
    layoutProperty->UpdateVisibility(hideNavBar ? VisibleType::INVISIBLE : VisibleType::VISIBLE, true);
    navBarNode->SetJSViewActive(!hideNavBar);
    if (pattern->GetNavBarVisibilityChange()) {
        navBarNode->MarkDirtyNode();
    }
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, HideNavBar, hideNavBar);
}

void NavigationModelNG::SetBackButtonIcon(const std::function<void(WeakPtr<NG::FrameNode>)>& symbolApply,
    const std::string& src, bool noPixMap, RefPtr<PixelMap>& pixMap,
    const std::vector<std::string>& nameList)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);

    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    ImageSourceInfo imageSourceInfo(src, nameList[0], nameList[1]);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, NoPixMap, noPixMap);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, ImageSource, imageSourceInfo);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, PixelMap, pixMap);
    titleBarLayoutProperty->UpdateImageSource(imageSourceInfo);
    titleBarLayoutProperty->UpdateNoPixMap(noPixMap);
    titleBarLayoutProperty->UpdatePixelMap(pixMap);
    titleBarLayoutProperty->SetBackIconSymbol(symbolApply);
    titleBarNode->MarkModifyDone();
}

void NavigationModelNG::SetHideBackButton(bool hideBackButton)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideBackButton(hideBackButton);
}

void NavigationModelNG::SetHideToolBar(bool hideToolBar)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideToolBar(hideToolBar);
}

void NavigationModelNG::SetCustomToolBar(const RefPtr<AceType>& customNode)
{
    auto customToolBar = AceType::DynamicCast<NG::UINode>(customNode);
    CHECK_NULL_VOID(customToolBar);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    if (navBarNode->GetPrevToolBarIsCustom().value_or(false)) {
        if (customToolBar->GetId() == navBarNode->GetToolBarNode()->GetId()) {
            navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::NONE);
            navBarNode->UpdatePrevToolBarIsCustom(true);
            return;
        }
    }
    navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::REPLACE);
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    toolBarNode->Clean();
    customToolBar->MountToParent(toolBarNode);
    navBarNode->UpdatePrevToolBarIsCustom(true);
    toolBarNode->SetHasValidContent(true);
    auto property = toolBarNode->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    property->UpdateVisibility(VisibleType::VISIBLE);
}

bool NavigationModelNG::NeedSetItems()
{
    return true;
}

void NavigationModelNG::SetToolBarItems(std::vector<NG::BarItem>&& toolBarItems)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    if (navBarNode->GetPrevToolBarIsCustom().value_or(false)) {
        navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::REPLACE);
    } else {
        if (navBarNode->GetPreToolBarNode() &&
            static_cast<int32_t>(navBarNode->GetPreToolBarNode()->GetChildren().size()) != 0) {
            UpdateOldBarItems(navBarNode->GetPreToolBarNode(), toolBarItems);
            navBarNode->SetToolBarNode(navBarNode->GetPreToolBarNode());
            navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::NONE);
            return;
        }
        navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::REPLACE);
    }
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetPreToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    auto rowProperty = toolBarNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_VOID(rowProperty);
    rowProperty->UpdateMainAxisAlign(FlexAlign::SPACE_EVENLY);
    for (const auto& toolBarItem : toolBarItems) {
        int32_t barItemNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, barItemNodeId);
        barItemNode->InitializePatternAndContext();
        UpdateBarItemNodeWithItem(barItemNode, toolBarItem);
        toolBarNode->AddChild(barItemNode);
    }
    bool hasValidContent = !toolBarNode->GetChildren().empty();
    toolBarNode->SetHasValidContent(hasValidContent);
    rowProperty->UpdateVisibility(hasValidContent ? VisibleType::VISIBLE : VisibleType::GONE);
    navBarNode->SetToolBarNode(toolBarNode);
    navBarNode->SetPreToolBarNode(toolBarNode);
    navBarNode->UpdatePrevToolBarIsCustom(false);
}

void NavigationModelNG::SetToolbarConfiguration(std::vector<NG::BarItem>&& toolBarItems)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    if (navBarNode->GetPrevToolBarIsCustom().value_or(false)) {
        navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::REPLACE);
    } else {
        auto toolbarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetPreToolBarNode());
        auto containerNode = toolbarNode->GetToolbarContainerNode();
        if (toolbarNode && containerNode) {
            navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::REPLACE);
            auto preToolbarNode = navBarNode->GetPreToolBarNode();
            preToolbarNode->RemoveChild(containerNode);
            navBarNode->RemoveChild(navBarNode->GetToolBarDividerNode());
        } else {
            navBarNode->UpdateToolBarNodeOperation(ChildNodeOperation::ADD);
        }
    }
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetPreToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    toolBarNode->SetIsUseNewToolbar(true);
    auto rowProperty = toolBarNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_VOID(rowProperty);
    rowProperty->UpdateMainAxisAlign(FlexAlign::CENTER);

    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN) ||
        !SystemProperties::GetNavigationBlurEnabled()) {
        CreateToolBarDividerNode(navBarNode);
    }

    auto containerNode = CreateToolbarItemsContainerNode(toolBarNode);
    CHECK_NULL_VOID(containerNode);
    toolBarNode->SetToolbarContainerNode(containerNode);
    bool needMoreButton = toolBarItems.size() > MAXIMUM_TOOLBAR_ITEMS_IN_BAR ? true : false;
    uint32_t count = 0;
    std::vector<OptionParam> params;
    for (const auto& toolBarItem : toolBarItems) {
        ++count;
        if (needMoreButton && (count > MAXIMUM_TOOLBAR_ITEMS_IN_BAR - 1)) {
            params.push_back({ toolBarItem.text.value_or(""), toolBarItem.icon.value_or(""), toolBarItem.action,
                toolBarItem.iconSymbol.value_or(nullptr) });
        } else {
            auto toolBarItemNode =
                CreateToolbarItemInContainer(toolBarItem, toolBarItems.size(), count, needMoreButton);
            CHECK_NULL_VOID(toolBarItemNode);
            containerNode->AddChild(toolBarItemNode);
        }
    }

    bool hasValidContent = !containerNode->GetChildren().empty();
    toolBarNode->SetHasValidContent(hasValidContent);
    rowProperty->UpdateVisibility(hasValidContent ? VisibleType::VISIBLE : VisibleType::GONE);

    if (needMoreButton) {
        int32_t barItemNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, barItemNodeId);
        barItemNode->InitializePatternAndContext();
        auto barItemLayoutProperty = barItemNode->GetLayoutProperty();
        CHECK_NULL_VOID(barItemLayoutProperty);
        barItemLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
        BuildToolbarMoreItemNode(barItemNode);
        MenuParam menuParam;
        menuParam.isShowInSubWindow = false;
        auto barMenuNode = MenuView::Create(
            std::move(params), barItemNodeId, V2::BAR_ITEM_ETS_TAG, MenuType::NAVIGATION_MENU, menuParam);
        auto toolBarItemNode = CreateToolbarMoreMenuNode(barItemNode);
        CHECK_NULL_VOID(toolBarItemNode);
        BuildToolbarMoreMenuNodeAction(barItemNode, barMenuNode, toolBarItemNode);
        containerNode->AddChild(toolBarItemNode);
        navBarNode->SetToolbarMenuNode(barMenuNode);
    }
    navBarNode->SetToolBarNode(toolBarNode);
    navBarNode->SetPreToolBarNode(toolBarNode);
    navBarNode->UpdatePrevToolBarIsCustom(false);
    navBarNode->SetNarBarUseToolbarConfiguration(true);

    auto navBarPattern = navBarNode->GetPattern<NavBarPattern>();
    CHECK_NULL_VOID(navBarPattern);
    navBarPattern->SetToolBarMenuItems(toolBarItems);
}

void NavigationModelNG::SetMenuItems(std::vector<NG::BarItem>&& menuItems)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    // if previous menu is custom, just remove it and create new menu, otherwise update old menu
    if (navBarNode->GetPrevMenuIsCustom().value_or(false)) {
        navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
    } else {
        if (navBarNode->GetMenu()) {
            navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
        } else {
            navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::ADD);
        }
    }
    auto navBarPattern = navBarNode->GetPattern<NavBarPattern>();
    CHECK_NULL_VOID(navBarPattern);
    navBarPattern->SetTitleBarMenuItems(menuItems);
    navBarPattern->SetMenuNodeId(ElementRegister::GetInstance()->MakeUniqueId());
    navBarPattern->SetLandscapeMenuNodeId(ElementRegister::GetInstance()->MakeUniqueId());
    navBarNode->UpdatePrevMenuIsCustom(false);
}

void NavigationModelNG::SetCustomMenu(const RefPtr<AceType>& customNode)
{
    auto customMenu = AceType::DynamicCast<NG::UINode>(customNode);
    CHECK_NULL_VOID(customMenu);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    // if previous menu exists, remove it if their ids are not the same
    // if previous node is not custom, their ids must not be the same
    if (navBarNode->GetMenu()) {
        if (customMenu->GetId() == navBarNode->GetMenu()->GetId()) {
            navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::NONE);
            return;
        }
        navBarNode->SetMenu(customMenu);
        navBarNode->UpdatePrevMenuIsCustom(true);
        navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
        return;
    }
    navBarNode->SetMenu(customMenu);
    navBarNode->UpdatePrevMenuIsCustom(true);
    navBarNode->UpdateMenuNodeOperation(ChildNodeOperation::ADD);
}

void NavigationModelNG::SetOnTitleModeChange(std::function<void(NG::NavigationTitleMode)>&& onTitleModeChange,
    std::function<void(const BaseEventInfo* baseInfo)>&& eventInfo)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto eventHub = navigationGroupNode->GetEventHub<NavigationEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnTitleModeChange(std::move(eventInfo));
}

void NavigationModelNG::SetUsrNavigationMode(NavigationMode mode)
{
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, UsrNavigationMode, mode);
}

void NavigationModelNG::SetNavBarPosition(NG::NavBarPosition mode)
{
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, NavBarPosition, static_cast<NG::NavBarPosition>(mode));
}

void NavigationModelNG::SetNavBarWidth(const Dimension& value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, NavBarWidth, value);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetUserSetNavBarWidthFlag(true);
    if (navigationPattern->GetInitNavBarWidth() != value) {
        navigationPattern->SetInitNavBarWidth(value);
    }
}

void NavigationModelNG::SetMinNavBarWidth(const Dimension& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, MinNavBarWidth, value);
}

void NavigationModelNG::SetMaxNavBarWidth(const Dimension& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, MaxNavBarWidth, value);
}

void NavigationModelNG::SetMinContentWidth(const Dimension& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, MinContentWidth, value);
}

void NavigationModelNG::SetOnNavBarStateChange(std::function<void(bool)>&& onNavBarStateChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationEventHub = AceType::DynamicCast<NavigationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navigationEventHub);
    navigationEventHub->SetOnNavBarStateChange(std::move(onNavBarStateChange));
}

void NavigationModelNG::SetOnNavigationModeChange(std::function<void(NavigationMode)>&& modeChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationEventHub = AceType::DynamicCast<NavigationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navigationEventHub);
    navigationEventHub->SetOnNavigationModeChange(std::move(modeChange));
}

void NavigationModelNG::SetNavigationMode(NavigationMode mode)
{
    ACE_UPDATE_LAYOUT_PROPERTY(NavigationLayoutProperty, NavigationMode, mode);
}

void NavigationModelNG::SetNavigationStack(const RefPtr<NG::NavigationStack>& navigationStack)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    const auto& stack = pattern->GetNavigationStack();
    if (stack) {
        stack->UpdateStackInfo(navigationStack);
    } else {
        pattern->SetNavigationStack(navigationStack);
    }
}

void NavigationModelNG::SetNavigationStackWithCreatorAndUpdater(
    std::function<RefPtr<NG::NavigationStack>()> creator, std::function<void(RefPtr<NG::NavigationStack>)> updater)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    RefPtr<NavigationStack> stack = pattern->GetNavigationStack();
    if (!stack) {
        stack = creator();
        pattern->SetNavigationStack(stack);
    }
    updater(stack);
}

void NavigationModelNG::SetNavigationStack()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    auto navigationStack = pattern->GetNavigationStack();
    if (!navigationStack) {
        auto navigationStack = AceType::MakeRefPtr<NavigationStack>();
        pattern->SetNavigationStack(navigationStack);
    }
}

void NavigationModelNG::SetNavigationStackProvided(bool provided)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetNavigationStackProvided(provided);
}

void NavigationModelNG::SetNavDestination(std::function<void(std::string)>&& builder)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetNavDestination(std::move(builder));
}

RefPtr<NG::NavigationStack> NavigationModelNG::GetNavigationStack()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navigationGroupNode, nullptr);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    return pattern->GetNavigationStack();
}

void NavigationModelNG::SetMenuCount(int32_t menuCount)
{
    return;
}

void NavigationModelNG::SetHideToolBar(FrameNode* frameNode, bool hideToolBar)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideToolBar(hideToolBar);
}

void NavigationModelNG::SetMinContentWidth(FrameNode* frameNode, const Dimension& value)
{
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, MinContentWidth, value, frameNode);
}

void NavigationModelNG::SetMinNavBarWidth(FrameNode* frameNode, const Dimension& value)
{
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, MinNavBarWidth, value, frameNode);
}

void NavigationModelNG::SetMaxNavBarWidth(FrameNode* frameNode, const Dimension& value)
{
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetIfNeedInit(true);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, MaxNavBarWidth, value, frameNode);
}

void NavigationModelNG::SetNavBarWidth(FrameNode* frameNode, const Dimension& value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, NavBarWidth, value, frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(navigationPattern);
    navigationPattern->SetUserSetNavBarWidthFlag(true);
    if (navigationPattern->GetInitNavBarWidth() != value) {
        navigationPattern->SetInitNavBarWidth(value);
    }
}

void NavigationModelNG::SetNavBarPosition(FrameNode* frameNode, NG::NavBarPosition mode)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        NavigationLayoutProperty, NavBarPosition, static_cast<NG::NavBarPosition>(mode), frameNode);
}

void NavigationModelNG::SetUsrNavigationMode(FrameNode* frameNode, NavigationMode mode)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, UsrNavigationMode, mode, frameNode);
}

void NavigationModelNG::SetBackButtonIcon(
    FrameNode* frameNode, const std::string& src, bool noPixMap, RefPtr<PixelMap>& pixMap)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);

    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    ImageSourceInfo imageSourceInfo(src);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, NoPixMap, noPixMap, frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, ImageSource, imageSourceInfo, frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, PixelMap, pixMap, frameNode);
    titleBarLayoutProperty->UpdateImageSource(imageSourceInfo);
    titleBarLayoutProperty->UpdateNoPixMap(noPixMap);
    titleBarLayoutProperty->UpdatePixelMap(pixMap);
    titleBarNode->MarkModifyDone();
}

void NavigationModelNG::SetHideNavBar(FrameNode* frameNode, bool hideNavBar)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    auto navigationLayoutProperty = navigationGroupNode->GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_VOID(navigationLayoutProperty);

    auto lastHideNavBarValue = navigationLayoutProperty->GetHideNavBar();
    if (lastHideNavBarValue.has_value()) {
        pattern->SetNavBarVisibilityChange(hideNavBar != lastHideNavBarValue.value());
    } else {
        pattern->SetNavBarVisibilityChange(true);
    }
    auto navBarNode = AceType::DynamicCast<FrameNode>(navigationGroupNode->GetNavBarNode());
    auto layoutProperty = navBarNode->GetLayoutProperty();
    layoutProperty->UpdateVisibility(hideNavBar ? VisibleType::INVISIBLE : VisibleType::VISIBLE, true);
    navBarNode->SetJSViewActive(!hideNavBar);
    if (pattern->GetNavBarVisibilityChange()) {
        navBarNode->MarkDirtyNode();
    }
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavigationLayoutProperty, HideNavBar, hideNavBar, frameNode);
}

void NavigationModelNG::SetHideTitleBar(FrameNode* frameNode, bool hideTitleBar)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideTitleBar(hideTitleBar);
}

void NavigationModelNG::SetSubtitle(FrameNode* frameNode, const std::string& subtitle)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    if (navBarNode->GetPrevTitleIsCustomValue(false)) {
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        titleBarNode->SetTitle(nullptr);
        auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
        CHECK_NULL_VOID(titleBarLayoutProperty);
        if (titleBarLayoutProperty->HasTitleHeight()) {
            titleBarLayoutProperty->ResetTitleHeight();
            navBarNode->GetLayoutProperty<NavBarLayoutProperty>()->ResetTitleMode();
        }
    }
    navBarNode->UpdatePrevTitleIsCustom(false);
    // create or update subtitle
    auto subTitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    if (subTitle) {
        // update subtitle
        auto textLayoutProperty = subTitle->GetLayoutProperty<TextLayoutProperty>();
        textLayoutProperty->UpdateContent(subtitle);
        auto renderContext = subTitle->GetRenderContext();
        renderContext->UpdateOpacity(1.0);
    } else {
        // create and init subtitle
        subTitle = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        auto textLayoutProperty = subTitle->GetLayoutProperty<TextLayoutProperty>();
        auto theme = NavigationGetTheme();
        textLayoutProperty->UpdateContent(subtitle);
        textLayoutProperty->UpdateFontSize(theme->GetSubTitleFontSize());
        textLayoutProperty->UpdateTextColor(theme->GetSubTitleColor());
        textLayoutProperty->UpdateFontWeight(FontWeight::REGULAR); // ohos_id_text_font_family_regular
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        titleBarNode->SetSubtitle(subTitle);
        titleBarNode->AddChild(subTitle);
    }
}

void NavigationModelNG::SetHideBackButton(FrameNode* frameNode, bool hideBackButton)
{
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateHideBackButton(hideBackButton);
}

void NavigationModelNG::SetTitleMode(FrameNode* frameNode, NG::NavigationTitleMode mode)
{
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    const auto& titleHeightProperty = titleBarLayoutProperty->GetTitleHeight();
    if (titleHeightProperty.has_value()) {
        mode = NavigationTitleMode::MINI;
    }
    navBarLayoutProperty->UpdateTitleMode(static_cast<NG::NavigationTitleMode>(mode));
    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
    if (mode != NavigationTitleMode::MINI) {
        // remove back button if any.
        titleBarNode->RemoveChild(backButtonNode);
        titleBarNode->SetBackButton(nullptr);
        return;
    }

    if (backButtonNode != nullptr) {
        return;
    }
    // create back button
    backButtonNode = FrameNode::CreateFrameNode(
        V2::BACK_BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());
    auto gestureEventHub = backButtonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto context = PipelineContext::GetCurrentContext();
    auto clickCallback = [weakContext = WeakPtr<PipelineContext>(context)](GestureEvent& /* info */) {
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);
        bool result = context->OnBackPressed();
        if (!result) {
            auto delegate = EngineHelper::GetCurrentDelegate();
            CHECK_NULL_VOID(delegate);
            delegate->Back("");
        }
    };
    gestureEventHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(clickCallback));
    auto buttonPattern = backButtonNode->GetPattern<ButtonPattern>();
    CHECK_NULL_VOID(buttonPattern);
    buttonPattern->SetSkipColorConfigurationUpdate();
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(backButtonLayoutProperty);
    backButtonLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(BACK_BUTTON_SIZE), CalcLength(BACK_BUTTON_SIZE)));
    backButtonLayoutProperty->UpdateType(ButtonType::NORMAL);
    backButtonLayoutProperty->UpdateBorderRadius(BorderRadiusProperty(BUTTON_RADIUS_SIZE));
    backButtonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto renderContext = backButtonNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    auto eventHub = backButtonNode->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(eventHub);

    PaddingProperty padding;
    padding.left = CalcLength(BUTTON_PADDING);
    padding.right = CalcLength(BUTTON_PADDING);
    padding.top = CalcLength(BUTTON_PADDING);
    padding.bottom = CalcLength(BUTTON_PADDING);
    backButtonLayoutProperty->UpdatePadding(padding);

    auto backButtonImageNode = FrameNode::CreateFrameNode(V2::BACK_BUTTON_IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(backButtonImageNode);
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    ImageSourceInfo imageSourceInfo;
    imageSourceInfo.SetResourceId(theme->GetBackResourceId());
    auto backButtonImageLayoutProperty = backButtonImageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(backButtonImageLayoutProperty);

    auto navigationEventHub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(navigationEventHub);
    if (!navigationEventHub->IsEnabled()) {
        imageSourceInfo.SetFillColor(theme->GetBackButtonIconColor().BlendOpacity(theme->GetAlphaDisabled()));
    } else {
        imageSourceInfo.SetFillColor(theme->GetBackButtonIconColor());
    }
    backButtonImageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    backButtonImageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

    backButtonImageNode->MountToParent(backButtonNode);
    backButtonImageNode->MarkModifyDone();
    backButtonNode->MarkModifyDone();
    titleBarNode->SetBackButton(backButtonNode);
    titleBarNode->AddChild(backButtonNode, 0);
}

void NavigationModelNG::SetIsCustomAnimation(bool isCustom)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetIsCustomAnimation(isCustom);
}

void NavigationModelNG::SetCustomTransition(NavigationTransitionEvent&& customTransition)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<NavigationPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetNavigationTransition(std::move(customTransition));
}

void NavigationModelNG::SetTitlebarOptions(NavigationTitlebarOptions&& opt)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarPattern = titleBarNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titleBarPattern);
    titleBarPattern->SetTitlebarOptions(std::move(opt));
}

void NavigationModelNG::SetToolbarOptions(NavigationToolbarOptions&& opt)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    auto toolBarPattern = toolBarNode->GetPattern<NavToolbarPattern>();
    CHECK_NULL_VOID(toolBarPattern);
    toolBarPattern->SetToolbarOptions(std::move(opt));
}

void NavigationModelNG::SetIgnoreLayoutSafeArea(const SafeAreaExpandOpts& opts)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    CHECK_NULL_VOID(navBarNode);
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    navBarLayoutProperty->UpdateIgnoreLayoutSafeArea(opts);
}

void NavigationModelNG::SetNavigationPathInfo(const std::string& moduleName, const std::string& pagePath)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    navigationGroupNode->SetNavigationPathInfo(moduleName, pagePath);
}
} // namespace OHOS::Ace::NG

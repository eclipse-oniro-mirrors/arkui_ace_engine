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

#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"

#include "base/log/ace_scoring_log.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr int32_t TEXT_MAX_LINES_TWO = 2;
} // namespace
bool NavDestinationModelNG::ParseCommonTitle(
    bool hasSubTitle, bool hasMainTitle, const std::string& subtitle, const std::string& title)
{
    if (!hasSubTitle && !hasMainTitle) {
        return false;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navDestinationNode, false);

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleBarNode, false);
    if (navDestinationNode->GetPrevTitleIsCustomValue(false)) {
        // remove custom title if any.
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        titleBarNode->SetTitle(nullptr);
        auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
        CHECK_NULL_RETURN(titleBarLayoutProperty, false);
        // reset custom title height if any.
        if (titleBarLayoutProperty->HasTitleHeight()) {
            titleBarLayoutProperty->ResetTitleHeight();
        }
    }
    navDestinationNode->UpdatePrevTitleIsCustom(false);

    auto theme = NavigationGetTheme();
    // create or update main title
    auto mainTitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    if (hasMainTitle) {
        if (mainTitle) {
            // update main title
            auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
            textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : TEXT_MAX_LINES_TWO);
            if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
                textLayoutProperty->UpdateHeightAdaptivePolicy(hasSubTitle ? TextHeightAdaptivePolicy::MAX_LINES_FIRST :
                    TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
            }
            textLayoutProperty->UpdateContent(title);
        } else {
            // create and init main title
            mainTitle = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
            textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : TEXT_MAX_LINES_TWO);
            textLayoutProperty->UpdateContent(title);
            textLayoutProperty->UpdateTextColor(theme->GetTitleColor());
            //max title font size should be 20.0 vp, because of backbutton
            textLayoutProperty->UpdateAdaptMaxFontSize(theme->GetTitleFontSizeMin());
            //min title font size should be 14.0 vp
            textLayoutProperty->UpdateAdaptMinFontSize(MIN_ADAPT_TITLE_FONT_SIZE);
            textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
            textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
            if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
                textLayoutProperty->UpdateAdaptMaxFontSize(theme->GetMainTitleFontSizeS());
                textLayoutProperty->UpdateTextColor(theme->GetMainTitleFontColor());
                textLayoutProperty->UpdateFontWeight(FontWeight::BOLD);
                textLayoutProperty->UpdateHeightAdaptivePolicy(hasSubTitle ? TextHeightAdaptivePolicy::MAX_LINES_FIRST :
                    TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
            }
            titleBarNode->SetTitle(mainTitle);
            titleBarNode->AddChild(mainTitle);
        }
    } else {
        // remove main title if any.
        titleBarNode->RemoveChild(mainTitle);
        titleBarNode->SetTitle(nullptr);
    }

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
    } else {
        // create and init subtitle
        subTitle = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        auto textLayoutProperty = subTitle->GetLayoutProperty<TextLayoutProperty>();
        textLayoutProperty->UpdateContent(subtitle);
        //max title font size shoule be 14.0 vp
        textLayoutProperty->UpdateAdaptMaxFontSize(theme->GetSubTitleFontSize());
        //min title font size should be 10.0 vp
        textLayoutProperty->UpdateAdaptMinFontSize(MIN_ADAPT_SUBTITLE_FONT_SIZE);
        textLayoutProperty->UpdateTextColor(theme->GetSubTitleColor());
        textLayoutProperty->UpdateFontWeight(FontWeight::REGULAR);
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            textLayoutProperty->UpdateAdaptMaxFontSize(theme->GetSubTitleFontSizeS());
            textLayoutProperty->UpdateTextColor(theme->GetSubTitleFontColor());
            textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MAX_LINES_FIRST);
        }
        titleBarNode->SetSubtitle(subTitle);
        titleBarNode->AddChild(subTitle);
    }
    return true;
}

void NavDestinationModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    // navDestination node
    int32_t nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVDESTINATION_VIEW_ETS_TAG, nodeId);
    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    if (!navDestinationNode->GetTitleBarNode()) {
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            CreateImageButton(navDestinationNode);
        } else {
            CreateBackButton(navDestinationNode);
        }
    }
    // content node
    if (!navDestinationNode->GetContentNode()) {
        int32_t contentNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVDESTINATION_CONTENT_ETS_TAG, contentNodeId);
        auto contentNode = FrameNode::GetOrCreateFrameNode(V2::NAVDESTINATION_CONTENT_ETS_TAG, contentNodeId,
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        navDestinationNode->AddChild(contentNode);
        navDestinationNode->SetContentNode(contentNode);
    }

    stack->Push(navDestinationNode);
}

void NavDestinationModelNG::CreateImageButton(const RefPtr<NavDestinationGroupNode>& navDestinationNode)
{
    int32_t titleBarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::TITLE_BAR_ETS_TAG, titleBarNodeId);
    auto titleBarNode = TitleBarNode::GetOrCreateTitleBarNode(
        V2::TITLE_BAR_ETS_TAG, titleBarNodeId, []() { return AceType::MakeRefPtr<TitleBarPattern>(); });
    navDestinationNode->AddChild(titleBarNode);
    navDestinationNode->SetTitleBarNode(titleBarNode);

    int32_t backButtonNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto backButtonNode =
        FrameNode::CreateFrameNode(V2::BACK_BUTTON_ETS_TAG, backButtonNodeId, AceType::MakeRefPtr<ImagePattern>());
    titleBarNode->AddChild(backButtonNode);
    titleBarNode->SetBackButton(backButtonNode);

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    ImageSourceInfo imageSourceInfo;
    imageSourceInfo.SetResourceId(theme->GetBackResourceId());
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(backButtonLayoutProperty);
    backButtonLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    backButtonNode->MarkModifyDone();

    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    titleBarLayoutProperty->UpdateTitleBarParentType(TitleBarParentType::NAV_DESTINATION);
}

void NavDestinationModelNG::CreateBackButton(const RefPtr<NavDestinationGroupNode>& navDestinationNode)
{
    int32_t titleBarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::TITLE_BAR_ETS_TAG, titleBarNodeId);
    auto titleBarNode = TitleBarNode::GetOrCreateTitleBarNode(
        V2::TITLE_BAR_ETS_TAG, titleBarNodeId, []() { return AceType::MakeRefPtr<TitleBarPattern>(); });
    navDestinationNode->AddChild(titleBarNode);
    navDestinationNode->SetTitleBarNode(titleBarNode);

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    int32_t backButtonNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto backButtonNode =
        FrameNode::CreateFrameNode(V2::BACK_BUTTON_ETS_TAG, backButtonNodeId, AceType::MakeRefPtr<ButtonPattern>());
    auto buttonPattern = backButtonNode->GetPattern<ButtonPattern>();
    CHECK_NULL_VOID(buttonPattern);
    buttonPattern->SetSkipColorConfigurationUpdate();
    buttonPattern->setComponentButtonType(ComponentButtonType::NAVIGATION);
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        buttonPattern->SetBlendColor(theme->GetBackgroundPressedColor(), theme->GetBackgroundHoverColor());
        buttonPattern->SetFocusBorderColor(theme->GetBackgroundFocusOutlineColor());
        buttonPattern->SetFocusBorderWidth(theme->GetBackgroundFocusOutlineWeight());
    }
    titleBarNode->AddChild(backButtonNode);
    titleBarNode->SetBackButton(backButtonNode);
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(backButtonLayoutProperty);
    auto renderContext = backButtonNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    backButtonLayoutProperty->UpdateType(ButtonType::NORMAL);
    backButtonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        backButtonLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(theme->GetIconBackgroundWidth()), CalcLength(theme->GetIconBackgroundHeight())));
        backButtonLayoutProperty->UpdateBorderRadius(BorderRadiusProperty(theme->GetCornerRadius()));
        renderContext->UpdateBackgroundColor(theme->GetCompBackgroundColor());
        PaddingProperty padding;
        padding.SetEdges(CalcLength(MENU_BUTTON_PADDING));
        backButtonLayoutProperty->UpdatePadding(padding);
    } else {
        backButtonLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(BACK_BUTTON_SIZE), CalcLength(BACK_BUTTON_SIZE)));
        backButtonLayoutProperty->UpdateBorderRadius(BorderRadiusProperty(BUTTON_RADIUS_SIZE));
        renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
        PaddingProperty padding;
        padding.SetEdges(CalcLength(BUTTON_PADDING));
        backButtonLayoutProperty->UpdatePadding(padding);
    }

    auto symbolNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_VOID(symbolNode);
    auto symbolProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(symbolProperty);
    symbolProperty->UpdateSymbolSourceInfo(SymbolSourceInfo(theme->GetBackSymbolId()));
    symbolProperty->UpdateFontSize(BACK_BUTTON_ICON_SIZE);
    symbolNode->MountToParent(backButtonNode);
    auto iconColor = theme->GetIconColor();
    symbolProperty->UpdateSymbolColorList({ iconColor });
    symbolNode->MarkModifyDone();
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    titleBarLayoutProperty->UpdateTitleBarParentType(TitleBarParentType::NAV_DESTINATION);
    backButtonNode->MarkModifyDone();
}

void NavDestinationModelNG::Create(std::function<void()>&& deepRenderFunc, RefPtr<NG::NavDestinationContext> context)
{
    auto* stack = ViewStackProcessor::GetInstance();
    // navDestination node
    auto nodeId = stack->ClaimNodeId();
    auto deepRender = [nodeId, deepRenderFunc = std::move(deepRenderFunc)]() -> RefPtr<UINode> {
        CHECK_NULL_RETURN(deepRenderFunc, nullptr);
        auto parent = AceType::DynamicCast<UINode>(FrameNode::GetFrameNode(V2::NAVDESTINATION_VIEW_ETS_TAG, nodeId));
        auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(parent);
        if (navDestinationNode && navDestinationNode->GetContentNode()) {
            parent = navDestinationNode->GetContentNode();
        }
        ScopedViewStackProcessor scopedViewStackProcessor;
        ViewStackProcessor::GetInstance()->Push(parent);
        deepRenderFunc();
        ViewStackProcessor::GetInstance()->PopContainer();
        ViewStackProcessor::GetInstance()->Finish();
        return parent;
    };
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVDESTINATION_VIEW_ETS_TAG, nodeId);
    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(V2::NAVDESTINATION_VIEW_ETS_TAG, nodeId,
        [shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(std::move(deepRender)), context]() {
            auto pattern = AceType::MakeRefPtr<NavDestinationPattern>(shallowBuilder);
            pattern->SetNavDestinationContext(context);
            return pattern;
        });
    if (!navDestinationNode->GetTitleBarNode()) {
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            CreateImageButton(navDestinationNode);
        } else {
            CreateBackButton(navDestinationNode);
        }
    }
    // content node
    if (!navDestinationNode->GetContentNode()) {
        int32_t contentNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::NAVDESTINATION_CONTENT_ETS_TAG, contentNodeId);
        auto contentNode = FrameNode::GetOrCreateFrameNode(V2::NAVDESTINATION_CONTENT_ETS_TAG, contentNodeId,
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        navDestinationNode->AddChild(contentNode);
        navDestinationNode->SetContentNode(contentNode);

        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
                .edges = SAFE_AREA_EDGE_ALL };
            contentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
        }
    }

    stack->Push(navDestinationNode);
}

void NavDestinationModelNG::SetHideTitleBar(bool hideTitleBar)
{
    ACE_UPDATE_LAYOUT_PROPERTY(NavDestinationLayoutProperty, HideTitleBar, hideTitleBar);
}

void NavDestinationModelNG::SetTitle(const std::string& title, bool hasSubTitle)
{
    ParseCommonTitle(hasSubTitle, true, "", title);
}

void NavDestinationModelNG::SetTitlebarOptions(NavigationTitlebarOptions&& opt)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarPattern = titleBarNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titleBarPattern);
    titleBarPattern->SetTitlebarOptions(std::move(opt));
}

void NavDestinationModelNG::SetBackButtonIcon(const std::function<void(WeakPtr<NG::FrameNode>)>& symbolApply,
    const std::string& src, bool noPixMap, RefPtr<PixelMap>& pixMap, const std::vector<std::string>& nameList)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    ImageSourceInfo imageSourceInfo(src, nameList[0], nameList[1]);
    titleBarLayoutProperty->UpdateImageSource(imageSourceInfo);
    titleBarLayoutProperty->UpdateNoPixMap(noPixMap);
    titleBarLayoutProperty->UpdatePixelMap(pixMap);
    titleBarLayoutProperty->SetBackIconSymbol(symbolApply);
    titleBarNode->MarkModifyDone();
}

void NavDestinationModelNG::SetSubtitle(const std::string& subtitle)
{
    ParseCommonTitle(true, false, subtitle, "");
}

void NavDestinationModelNG::SetCustomTitle(const RefPtr<AceType>& customNode)
{
    auto customTitle = AceType::DynamicCast<NG::UINode>(customNode);
    CHECK_NULL_VOID(customTitle);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    if (!navDestinationNode->GetPrevTitleIsCustomValue(false)) {
        // remove main and subtitle if any.
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        titleBarNode->RemoveChild(titleBarNode->GetSubtitle());
        titleBarNode->SetTitle(nullptr);
        titleBarNode->SetSubtitle(nullptr);
    }
    navDestinationNode->UpdatePrevTitleIsCustom(true);
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

void NavDestinationModelNG::SetTitleHeight(const Dimension& titleHeight, bool isValid)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationGroupNode);

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationGroupNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (isValid) {
        titleBarLayoutProperty->UpdateTitleHeight(titleHeight);
    } else {
        titleBarLayoutProperty->ResetTitleHeight();
    }
}

void NavDestinationModelNG::SetOnShown(std::function<void()>&& onShow)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnShown(onShow);
}

void NavDestinationModelNG::SetOnHidden(std::function<void()>&& onHidden)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnHidden(onHidden);
}

void NavDestinationModelNG::SetOnBackPressed(std::function<bool()>&& onBackPressed)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnBackPressed(onBackPressed);
}

void NavDestinationModelNG::SetOnReady(std::function<void(RefPtr<NavDestinationContext>)>&& onReady)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnReady(onReady);
}

RefPtr<AceType> NavDestinationModelNG::CreateEmpty()
{
    ScopedViewStackProcessor processor;
    Create();
    auto uiNode = ViewStackProcessor::GetInstance()->Finish();
    uiNode->SetRemoveSilently(true);
    auto navigationNode = AceType::DynamicCast<NavDestinationGroupNode>(uiNode);
    CHECK_NULL_RETURN(navigationNode, uiNode);
    auto pattern = navigationNode->GetPattern<NavDestinationPattern>();
    auto context = AceType::MakeRefPtr<NavDestinationContext>();
    CHECK_NULL_RETURN(context, uiNode);
    context->SetIsEmpty(true);
    pattern->SetNavDestinationContext(context);
    return uiNode;
}

void NavDestinationModelNG::SetHideTitleBar(FrameNode* frameNode, bool hideTitleBar)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(NavDestinationLayoutProperty, HideTitleBar, hideTitleBar, frameNode);
}

void NavDestinationModelNG::SetNavDestinationMode(NavDestinationMode mode)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestination);
    navDestination->SetNavDestinationMode(mode);
}

void NavDestinationModelNG::SetMenuItems(std::vector<NG::BarItem>&& menuItems)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationGroupNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationGroupNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    // if previous menu is custom, just remove it and create new menu, otherwise update old menu
    if (titleBarNode->GetPrevMenuIsCustom().value_or(false)) {
        titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
    } else {
        if (titleBarNode->GetMenu()) {
            titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
        } else {
            titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::ADD);
        }
    }
    auto titleBarPattern = titleBarNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titleBarPattern);
    titleBarPattern->SetTitleBarMenuItems(menuItems);
    titleBarPattern->SetMenuNodeId(ElementRegister::GetInstance()->MakeUniqueId());
    titleBarNode->UpdatePrevMenuIsCustom(false);
}

void NavDestinationModelNG::SetCustomMenu(const RefPtr<AceType>& customNode)
{
    auto customMenu = AceType::DynamicCast<NG::UINode>(customNode);
    CHECK_NULL_VOID(customMenu);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationGroupNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationGroupNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    // if previous menu exists, remove it if their ids are not the same
    // if previous node is not custom, their ids must not be the same
    if (titleBarNode->GetMenu()) {
        if (customMenu->GetId() == titleBarNode->GetMenu()->GetId()) {
            titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::NONE);
            return;
        }
        titleBarNode->SetMenu(customMenu);
        titleBarNode->UpdatePrevMenuIsCustom(true);
        titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::REPLACE);
        return;
    }
    titleBarNode->SetMenu(customMenu);
    titleBarNode->UpdatePrevMenuIsCustom(true);
    titleBarNode->UpdateMenuNodeOperation(ChildNodeOperation::ADD);
}

void NavDestinationModelNG::SetBackgroundColor(const Color& color, bool isVaild)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    CHECK_NULL_VOID(navDestinationPattern);
    if (!isVaild) {
        navDestinationPattern->SetIsUserDefinedBgColor(false);
        return;
    }
    ViewAbstract::SetBackgroundColor(color);
    navDestinationPattern->SetIsUserDefinedBgColor(true);
}

void NavDestinationModelNG::SetBackgroundColor(FrameNode* frameNode, const Color& color, bool isVaild)
{
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    CHECK_NULL_VOID(navDestinationPattern);
    if (!isVaild) {
        navDestinationPattern->SetIsUserDefinedBgColor(false);
        return;
    }
    ViewAbstract::SetBackgroundColor(frameNode, color);
    navDestinationPattern->SetIsUserDefinedBgColor(true);
}

void NavDestinationModelNG::SetOnWillAppear(std::function<void()>&& willAppear)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnWillAppear(willAppear);
}

void NavDestinationModelNG::SetOnWillHide(std::function<void()>&& willHide)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnWillHide(willHide);
}

void NavDestinationModelNG::SetOnWillShow(std::function<void()>&& willShow)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnWillShow(willShow);
}

void NavDestinationModelNG::SetOnWillDisAppear(std::function<void()>&& willDisAppear)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationEventHub = AceType::DynamicCast<NavDestinationEventHub>(frameNode->GetEventHub<EventHub>());
    CHECK_NULL_VOID(navDestinationEventHub);
    navDestinationEventHub->SetOnWillDisAppear(willDisAppear);
}

void NavDestinationModelNG::SetIgnoreLayoutSafeArea(const SafeAreaExpandOpts& opts)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestination);

    auto navdestinationLayoutProperty = navDestination->GetLayoutProperty<NavDestinationLayoutProperty>();
    navdestinationLayoutProperty->UpdateIgnoreLayoutSafeArea(opts);
}

void NavDestinationModelNG::SetNavDestinationPathInfo(const std::string& moduleName, const std::string& pagePath)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestination);
    navDestination->SetNavDestinationPathInfo(moduleName, pagePath);
}
} // namespace OHOS::Ace::NG

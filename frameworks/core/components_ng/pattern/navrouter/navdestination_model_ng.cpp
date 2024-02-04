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
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
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

    // create or update main title
    auto mainTitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    if (hasMainTitle) {
        if (mainTitle) {
            // update main title
            auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
            textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : 2);
            textLayoutProperty->UpdateContent(title);
        } else {
            // create and init main title
            mainTitle = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            auto textLayoutProperty = mainTitle->GetLayoutProperty<TextLayoutProperty>();
            auto theme = NavigationGetTheme();
            textLayoutProperty->UpdateMaxLines(hasSubTitle ? 1 : 2);
            textLayoutProperty->UpdateContent(title);
            textLayoutProperty->UpdateTextColor(theme->GetTitleColor());
            textLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeMin());
            textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
            textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
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
        textLayoutProperty->UpdateFontSize(SUBTITLE_FONT_SIZE);
        textLayoutProperty->UpdateTextColor(SUBTITLE_COLOR);
        textLayoutProperty->UpdateFontWeight(FontWeight::REGULAR);
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
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

    int32_t backButtonNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto backButtonNode =
        FrameNode::CreateFrameNode(V2::BACK_BUTTON_ETS_TAG, backButtonNodeId, AceType::MakeRefPtr<ButtonPattern>());
    auto buttonPattern = backButtonNode->GetPattern<ButtonPattern>();
    CHECK_NULL_VOID(buttonPattern);
    buttonPattern->SetSkipColorConfigurationUpdate();
    titleBarNode->AddChild(backButtonNode);
    titleBarNode->SetBackButton(backButtonNode);
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

    PaddingProperty padding;
    padding.SetEdges(CalcLength(BUTTON_PADDING));
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

    backButtonImageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    backButtonImageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    backButtonNode->AddChild(backButtonImageNode);
    backButtonImageNode->MarkModifyDone();
    backButtonNode->MarkModifyDone();

    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    titleBarLayoutProperty->UpdateTitleBarParentType(TitleBarParentType::NAV_DESTINATION);
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
            SafeAreaExpandOpts opts = {.type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_BOTTOM};
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

void NavDestinationModelNG::SetBackButtonIcon(const std::string& src, bool noPixMap, RefPtr<PixelMap>& pixMap,
    const std::string& bundleName, const std::string& moduleName)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navDestinationNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    CHECK_NULL_VOID(navDestinationNode);

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navDestinationNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    ImageSourceInfo imageSourceInfo(src, bundleName, moduleName);
    titleBarLayoutProperty->UpdateImageSource(imageSourceInfo);
    titleBarLayoutProperty->UpdateNoPixMap(noPixMap);
    titleBarLayoutProperty->UpdatePixelMap(pixMap);
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
} // namespace OHOS::Ace::NG

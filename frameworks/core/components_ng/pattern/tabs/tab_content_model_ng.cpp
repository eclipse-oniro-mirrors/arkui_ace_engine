/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include <optional>
#include <string>
#include <string_view>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/common/resource/resource_parse_utils.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_item_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_event_hub.h"
#include "core/components_ng/pattern/tabs/tab_content_node.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/symbol/constants.h"
#include "core/components_ng/pattern/symbol/symbol_effect_options.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/badge/badge_model_ng.h"
#include "core/components_ng/pattern/badge/badge_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint16_t PIXEL_ROUND = static_cast<uint16_t>(PixelRoundPolicy::FORCE_FLOOR_START) |
                                static_cast<uint16_t>(PixelRoundPolicy::FORCE_FLOOR_TOP) |
                                static_cast<uint16_t>(PixelRoundPolicy::FORCE_CEIL_END) |
                                static_cast<uint16_t>(PixelRoundPolicy::FORCE_CEIL_BOTTOM);
constexpr uint32_t DEFAULT_RENDERING_STRATEGY = 2;
const auto MASK_COUNT = 2;
const auto IMAGE_INDICATOR_COUNT = 1;
constexpr std::string_view KEY_PADDING = "tabContent.tabBarPadding";
constexpr std::string_view KEY_PADDING_LEFT = "tabContent.tabBarPadding.left";
constexpr std::string_view KEY_PADDING_RIGHT = "tabContent.tabBarPadding.right";
constexpr std::string_view KEY_PADDING_TOP = "tabContent.tabBarPadding.top";
constexpr std::string_view KEY_PADDING_BOTTOM = "tabContent.tabBarPadding.bottom";
const char TAB_CONTENT_ITEM_ETS_TAG[] = "TabContent";
} // namespace
// Resolve RTL direction from the Tabs/TabBar ancestor of the badge node.
// The badge node is mounted as: badgeNode → columnNode → tabBarNode → tabsNode.
// Tabs resolves its RTL via tabLayoutProperty->GetNonAutoLayoutDirection(); TabBar may
// also carry an explicit direction. This helper walks up from the badge node to find the
// TabBar/Tabs ancestors and computes isRTL the same way as AddBottomStyleTabBarItem.
bool TabContentModelNG::ResolveBadgeRTL(const RefPtr<FrameNode>& badgeNode)
{
    CHECK_NULL_RETURN(badgeNode, false);
    auto columnNode = AceType::DynamicCast<FrameNode>(badgeNode->GetParent());
    CHECK_NULL_RETURN(columnNode, false);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(columnNode->GetParent());
    CHECK_NULL_RETURN(tabBarNode, false);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_RETURN(tabBarLayoutProperty, false);
    auto tabsNode = AceType::DynamicCast<TabsNode>(tabBarNode->GetParent());
    CHECK_NULL_RETURN(tabsNode, false);
    auto tabsLayoutProperty = AceType::DynamicCast<TabsLayoutProperty>(tabsNode->GetLayoutProperty());
    CHECK_NULL_RETURN(tabsLayoutProperty, false);
    auto tabsDirection = tabsLayoutProperty->GetNonAutoLayoutDirection();
    auto tabBarDirection = tabBarLayoutProperty->GetLayoutDirection();
    return tabBarDirection == TextDirection::RTL ||
           (tabBarDirection == TextDirection::AUTO && tabsDirection == TextDirection::RTL);
}
// Build BadgeParameters from TabBarBadgeParam
// - value undefined or unexpected type → 6vp dot, RIGHT_TOP enum mode
// - value is number → ellipse with count display
// - value is string → ellipse with text display
BadgeParameters TabContentModelNG::BuildBadgeParameters(const TabBarBadgeParam& badgeParam, bool& isDefaultFontSize,
    bool& isDefaultBadgeSize, bool& isEllipseMode)
{
    BadgeParameters badgeParams;
    isDefaultFontSize = true;
    isDefaultBadgeSize = true;
    isEllipseMode = false;
    badgeParams.badgePosition = static_cast<int>(BadgePosition::RIGHT_TOP);
    badgeParams.isPositionXy = false;

    if (!badgeParam.badgeValue.has_value() && !badgeParam.badgeCount.has_value()) {
        // Dot mode: no text content. Set badgeCircleSize as diameter and a near-zero font size
        // so that std::max(textSize.Height(), badgeCircleDiameter) in BadgeLayoutAlgorithm::Measure
        // does not inflate the dot to the font height.
        // 6.0vp circle.
        badgeParams.badgeCircleSize = Dimension(6.0, DimensionUnit::VP);
        badgeParams.badgeFontSize = Dimension(0.0, DimensionUnit::VP);
        badgeParams.badgeValue = "";  // Empty string triggers badgeVisible=true in BadgePattern::OnModifyDone
        // Mark as user-specified to prevent OnColorConfigurationUpdate from overwriting
        // with theme defaults (10vp font, 6vp circle) on theme/color config changes.
        badgeParams.badgeFontSizeByUser = true;
        badgeParams.badgeCircleSizeByUser = true;
        isDefaultBadgeSize = false;
        isDefaultFontSize = false;
    } else if (badgeParam.badgeCount.has_value()) {
        badgeParams.badgeCount = badgeParam.badgeCount.value();
        if (badgeParam.badgeMaxCount.has_value()) {
            badgeParams.badgeMaxCount = badgeParam.badgeMaxCount.value();
        }
        isEllipseMode = true;
    } else if (badgeParam.badgeValue.has_value()) {
        badgeParams.badgeValue = badgeParam.badgeValue.value();
        badgeParams.resourceBadgeValueObject = badgeParam.resourceBadgeValueObject;
        isEllipseMode = true;
    }
    return badgeParams;
}
// Calculate the RTL position X for an ellipse badge.
// In RTL the badge circle sits on the left side of the content:
// circle right edge = content left edge + 8vp
// → positionX = 8vp - circleWidth (circleWidth from text child geometry)
float TabContentModelNG::CalculateEllipseRtlPositionX(const RefPtr<FrameNode>& badgeNode, float eightVpPx)
{
    float circleWidth = 0.0f;
    const auto& badgeChildren = badgeNode->GetChildren();
    if (!badgeChildren.empty()) {
        auto textChild = AceType::DynamicCast<FrameNode>(badgeChildren.back());
        if (textChild) {
            auto textGeometry = textChild->GetGeometryNode();
            CHECK_NULL_RETURN(textGeometry, eightVpPx);
            circleWidth = textGeometry->GetFrameRect().Width();
        }
    }
    return eightVpPx - circleWidth;
}
// Schedule a post-render task to adjust ellipse badge position.
// The badge node wraps the contentChild (icon+text column); badge frame width = content width.
// positionX positions the badge circle (text child) relative to the badge frame's left edge.
// LTR: circle LEFT edge = content RIGHT edge - 8vp  →  positionX = badgeWidth - 8vp
// RTL: circle RIGHT edge = content LEFT edge + 8vp  →  positionX = 8vp - circleWidth
//      (circleWidth = text child frame width, NOT badgeWidth)
void TabContentModelNG::ScheduleEllipsePositionAdjustment(const RefPtr<FrameNode>& badgeNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddAfterRenderTask([weakBadge = WeakPtr<FrameNode>(badgeNode)]() {
        auto badgeNode = weakBadge.Upgrade();
        CHECK_NULL_VOID(badgeNode);
        auto badgeGeometry = badgeNode->GetGeometryNode();
        CHECK_NULL_VOID(badgeGeometry);
        float badgeWidth = badgeGeometry->GetFrameRect().Width();
        if (badgeWidth <= 0.0f) {
            return;
        }
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        float vpToPx = pipeline->GetDipScale();
        float eightVpPx = 8.0f * vpToPx;
        bool isRTL = ResolveBadgeRTL(badgeNode);
        float positionXPx;
        if (isRTL) {
            positionXPx = CalculateEllipseRtlPositionX(badgeNode, eightVpPx);
        } else {
            positionXPx = badgeWidth - eightVpPx;
        }
        auto badgeLayoutProperty = badgeNode->GetLayoutProperty<BadgeLayoutProperty>();
        CHECK_NULL_VOID(badgeLayoutProperty);
        badgeLayoutProperty->UpdateIsPositionXy(true);
        badgeLayoutProperty->UpdateBadgePositionX(Dimension(positionXPx, DimensionUnit::PX));
        // Offset upward by -4.0vp in the y direction.
        badgeLayoutProperty->UpdateBadgePositionY(Dimension(-4.0, DimensionUnit::VP));
        badgeNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    });
}

void TabContentModelNG::UpdateBottomStyleBadge(const RefPtr<FrameNode>& columnNode,
    const RefPtr<TabContentPattern>& tabContentPattern, bool hasBadge,
    const RefPtr<LinearLayoutProperty>& linearLayoutProperty)
{
    CHECK_NULL_VOID(columnNode);
    CHECK_NULL_VOID(tabContentPattern);
    auto existingBadgeNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    bool hasExistingBadge = existingBadgeNode && existingBadgeNode->GetTag() == BADGE_ETS_TAG;
    if (hasBadge && !hasExistingBadge) {
        AddBadgeToColumn(columnNode, tabContentPattern, linearLayoutProperty);
    } else if (!hasBadge && hasExistingBadge) {
        RemoveBadgeFromColumn(columnNode, existingBadgeNode);
    } else if (hasBadge && hasExistingBadge) {
        UpdateExistingBadge(columnNode, existingBadgeNode, tabContentPattern);
    }
}

void TabContentModelNG::AddBadgeToColumn(const RefPtr<FrameNode>& columnNode,
    const RefPtr<TabContentPattern>& tabContentPattern,
    const RefPtr<LinearLayoutProperty>& linearLayoutProperty)
{
    auto iconNode = TabBarPattern::GetIconNodeFromColumn(columnNode);
    auto textNode = TabBarPattern::GetTextNodeFromColumn(columnNode);
    if (iconNode && textNode) {
        columnNode->Clean();
        WrapWithBadge(
            columnNode, iconNode, textNode, tabContentPattern->GetTabBarBadge().value(), linearLayoutProperty);
    }
}

void TabContentModelNG::RemoveBadgeFromColumn(const RefPtr<FrameNode>& columnNode,
    const RefPtr<FrameNode>& existingBadgeNode)
{
    RefPtr<FrameNode> iconNode;
    RefPtr<FrameNode> textNode;
    auto& badgeChildren = existingBadgeNode->GetChildren();
    if (!badgeChildren.empty()) {
        auto contentChild = AceType::DynamicCast<FrameNode>(badgeChildren.front());
        // text and image node size count 2.
        if (contentChild && contentChild->GetChildren().size() >= 2) {
            iconNode = AceType::DynamicCast<FrameNode>(contentChild->GetChildren().front());
            textNode = AceType::DynamicCast<FrameNode>(contentChild->GetChildren().back());
        }
    }
    columnNode->Clean();
    if (iconNode) {
        iconNode->MountToParent(columnNode);
    }
    if (textNode) {
        textNode->MountToParent(columnNode);
    }
}

void TabContentModelNG::UpdateExistingBadge(const RefPtr<FrameNode>& columnNode,
    const RefPtr<FrameNode>& existingBadgeNode, const RefPtr<TabContentPattern>& tabContentPattern)
{
    auto badgeParam = tabContentPattern->GetTabBarBadge().value();
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto badgeParams = BuildBadgeParameters(badgeParam, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);

    BadgeModelNG::SetBadgeParam(
        AceType::RawPtr(existingBadgeNode), badgeParams, isDefaultFontSize, isDefaultBadgeSize);

    auto updateBadgeLayoutProp = existingBadgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    if (updateBadgeLayoutProp) {
        updateBadgeLayoutProp->UpdateBadgeFontSizeByuser(badgeParams.badgeFontSizeByUser);
        updateBadgeLayoutProp->UpdateBadgeCircleSizeByuser(badgeParams.badgeCircleSizeByUser);
        if (!badgeParams.badgeCount.has_value()) {
            updateBadgeLayoutProp->ResetBadgeCount();
        }
    }

    if (SystemProperties::ConfigChangePerform()) {
        BadgeModelNG badgeModel;
        badgeModel.CreateWithResourceObj(existingBadgeNode, badgeParams);
    }

    existingBadgeNode->MarkModifyDone();

    auto colRC = columnNode->GetRenderContext();
    if (colRC) {
        colRC->UpdateClipEdge(false);
    }
    auto badgeRC = existingBadgeNode->GetRenderContext();
    if (badgeRC) {
        badgeRC->UpdateClipEdge(false);
    }

    // Keep RTL direction in sync when the badge is updated (e.g. direction change at runtime).
    // Only the badge node direction is set — contentChild direction is intentionally left
    // to system/app-level resolution for consistency with non-badge tab items.
    auto isRTL = ResolveBadgeRTL(existingBadgeNode);
    auto badgeLayoutProp = existingBadgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    if (badgeLayoutProp) {
        badgeLayoutProp->UpdateLayoutDirection(isRTL ? TextDirection::RTL : TextDirection::LTR);
    }

    if (isEllipseMode) {
        ScheduleEllipsePositionAdjustment(existingBadgeNode);
    }
}

RefPtr<FrameNode> TabContentModelNG::CreateBadgeContentChild(const RefPtr<FrameNode>& iconNode,
    const RefPtr<FrameNode>& textNode, const RefPtr<LinearLayoutProperty>& columnLayoutProperty)
{
    auto contentChildId = ElementRegister::GetInstance()->MakeUniqueId();
    auto contentChild = FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, contentChildId,
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_RETURN(contentChild, nullptr);

    auto contentChildLayout = contentChild->GetLayoutProperty<LinearLayoutProperty>();
    if (columnLayoutProperty && contentChildLayout) {
        if (columnLayoutProperty->HasFlexDirection()) {
            contentChildLayout->UpdateFlexDirection(
                columnLayoutProperty->GetFlexDirectionValue(FlexDirection::COLUMN));
        }
        if (columnLayoutProperty->HasMainAxisAlign()) {
            contentChildLayout->UpdateMainAxisAlign(
                columnLayoutProperty->GetMainAxisAlignValue(FlexAlign::FLEX_START));
        }
        if (columnLayoutProperty->HasCrossAxisAlign()) {
            contentChildLayout->UpdateCrossAxisAlign(
                columnLayoutProperty->GetCrossAxisAlignValue(FlexAlign::CENTER));
        }
        if (columnLayoutProperty->HasSpace()) {
            contentChildLayout->UpdateSpace(columnLayoutProperty->GetSpaceValue(Dimension(0.0f)));
        }
    }

    iconNode->MountToParent(contentChild);
    textNode->MountToParent(contentChild);
    return contentChild;
}

RefPtr<FrameNode> TabContentModelNG::ConfigureBadgeNode(const TabBarBadgeParam& badgeParam, bool& isEllipseMode)
{
    auto badgeNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto badgeNode = BadgeModelNG::CreateFrameNode(badgeNodeId);
    CHECK_NULL_RETURN(badgeNode, nullptr);

    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    isEllipseMode = false;
    auto badgeParams = BuildBadgeParameters(badgeParam, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);

    BadgeModelNG::SetBadgeParam(AceType::RawPtr(badgeNode), badgeParams, isDefaultFontSize, isDefaultBadgeSize);

    auto badgeLayoutProp = badgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    if (badgeLayoutProp) {
        badgeLayoutProp->UpdateBadgeFontSizeByuser(badgeParams.badgeFontSizeByUser);
        badgeLayoutProp->UpdateBadgeCircleSizeByuser(badgeParams.badgeCircleSizeByUser);
    }

    if (SystemProperties::ConfigChangePerform()) {
        BadgeModelNG badgeModel;
        badgeModel.CreateWithResourceObj(badgeNode, badgeParams);
    }
    return badgeNode;
}

void TabContentModelNG::WrapWithBadge(const RefPtr<FrameNode>& columnNode, const RefPtr<FrameNode>& iconNode,
    const RefPtr<FrameNode>& textNode, const TabBarBadgeParam& badgeParam,
    const RefPtr<LinearLayoutProperty>& columnLayoutProperty)
{
    auto contentChild = CreateBadgeContentChild(iconNode, textNode, columnLayoutProperty);
    CHECK_NULL_VOID(contentChild);

    bool isEllipseMode = false;
    auto badgeNode = ConfigureBadgeNode(badgeParam, isEllipseMode);
    CHECK_NULL_VOID(badgeNode);

    contentChild->MountToParent(badgeNode);
    badgeNode->MountToParent(columnNode);
    badgeNode->MarkModifyDone();

    auto columnRenderContext = columnNode->GetRenderContext();
    if (columnRenderContext) {
        columnRenderContext->UpdateClipEdge(false);
    }
    auto badgeRenderContext = badgeNode->GetRenderContext();
    if (badgeRenderContext) {
        badgeRenderContext->UpdateClipEdge(false);
    }

    // Propagate RTL direction to badge node so that the badge layout algorithm's
    // GetNonAutoLayoutDirection() returns the correct value. Dot badge relies on this
    // to trigger the RTL branches in GetTextDataOffset / AdjustTextOffsetForBadge /
    // content child horizontal offset. contentChild direction is NOT set here — it
    // mirrors the same system/app-level RTL resolution as the non-badge tab item column,
    // keeping behavior consistent with or without badge.
    auto isRTL = ResolveBadgeRTL(badgeNode);
    auto badgeLayoutProp = badgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    if (badgeLayoutProp) {
        badgeLayoutProp->UpdateLayoutDirection(isRTL ? TextDirection::RTL : TextDirection::LTR);
    }

    if (isEllipseMode) {
        ScheduleEllipsePositionAdjustment(badgeNode);
    }
}

void TabContentModelNG::Create(std::function<void()>&& deepRenderFunc)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto deepRender = [nodeId, deepRenderFunc = std::move(deepRenderFunc)]() -> RefPtr<UINode> {
        CHECK_NULL_RETURN(deepRenderFunc, nullptr);
        ScopedViewStackProcessor scopedViewStackProcessor;
        deepRenderFunc();
        auto deepChild = ViewStackProcessor::GetInstance()->Finish();
        auto parent = FrameNode::GetFrameNode(TAB_CONTENT_ITEM_ETS_TAG, nodeId);
        if (deepChild && parent) {
            deepChild->MountToParent(parent);
        }
        return deepChild;
    };
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", TAB_CONTENT_ITEM_ETS_TAG, nodeId);
    ACE_UINODE_TRACE(nodeId);
    auto frameNode = TabContentNode::GetOrCreateTabContentNode(TAB_CONTENT_ITEM_ETS_TAG, nodeId,
        [shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(std::move(deepRender))]() {
            return AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
        });
    stack->Push(frameNode);
    auto tabTheme = frameNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    SetTabBar(tabTheme->GetDefaultTabBarName(), "", std::nullopt, nullptr, true); // Set default tab bar.
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, Text, tabTheme->GetDefaultTabBarName());
    if (SystemProperties::ConfigChangePerform()) {
        auto tabsLayoutProperty = frameNode->GetLayoutProperty<TabContentLayoutProperty>();
        CHECK_NULL_VOID(tabsLayoutProperty);
        tabsLayoutProperty->ResetIndicatorColorSetByUser();
        tabsLayoutProperty->ResetLabelUnselectedColorSetByUser();
        tabsLayoutProperty->ResetLabelSelectedColorSetByUser();
        tabsLayoutProperty->ResetIconUnselectedColorSetByUser();
        tabsLayoutProperty->ResetIconSelectedColorSetByUser();
    }
}

void TabContentModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", TAB_CONTENT_ITEM_ETS_TAG, nodeId);
    ACE_UINODE_TRACE(nodeId);
    auto frameNode = TabContentNode::GetOrCreateTabContentNode(
        TAB_CONTENT_ITEM_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TabContentPattern>(nullptr); });
    stack->Push(frameNode);
    auto tabTheme = frameNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    SetTabBar(tabTheme->GetDefaultTabBarName(), "", std::nullopt, nullptr, true); // Set default tab bar.
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, Text, tabTheme->GetDefaultTabBarName());
}

RefPtr<FrameNode> TabContentModelNG::CreateFrameNode(int32_t nodeId)
{
    ACE_UINODE_TRACE(nodeId);
    auto frameNode = TabContentNode::GetOrCreateTabContentNode(
        TAB_CONTENT_ITEM_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TabContentPattern>(nullptr); });
    auto tabTheme = frameNode->GetTheme<TabTheme>(true);
    CHECK_NULL_RETURN(tabTheme, nullptr);
    auto layout = frameNode->GetLayoutProperty<TabContentLayoutProperty>();
    CHECK_NULL_RETURN(layout, nullptr);
    auto text = tabTheme->GetDefaultTabBarName();
    layout->UpdateText(text);
    layout->UpdateIcon("");
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    pattern->SetTabBar(text, "", std::nullopt, nullptr);
    pattern->SetTabBarWithContent(nullptr);
    return frameNode;
}

void TabContentModelNG::Pop()
{
    auto tabContent = AceType::Claim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    AddTabBarItem(tabContent, DEFAULT_NODE_SLOT, true);
    NG::ViewStackProcessor::GetInstance()->PopContainer();
}

RefPtr<TabsNode> TabContentModelNG::FindTabsNode(const RefPtr<UINode>& tabContent)
{
    CHECK_NULL_RETURN(tabContent, nullptr);
    RefPtr<UINode> parent = tabContent->GetParent();

    while (parent) {
        if (AceType::InstanceOf<TabsNode>(parent)) {
            return AceType::DynamicCast<TabsNode>(parent);
        }
        parent = parent->GetParent();
    }
    return nullptr;
}

void TabContentModelNG::AddTabBarItem(const RefPtr<UINode>& tabContent, int32_t position, bool update)
{
    AddBottomStyleTabBarItem(tabContent, position, update);
    AddSideBarAdaptableStyleTabBarItem(tabContent, position, update);
}

void TabContentModelNG::AddBottomStyleTabBarItem(const RefPtr<UINode>& tabContent, int32_t position, bool update)
{
    CHECK_NULL_VOID(tabContent);
    ACE_UINODE_TRACE(tabContent);
    auto tabContentId = tabContent->GetId();

    auto tabContentNode = AceType::DynamicCast<TabContentNode>(tabContent);
    CHECK_NULL_VOID(tabContentNode);

    if (update && !tabContentNode->HasTabBarItemId()) {
        return;
    }

    auto tabsNode = FindTabsNode(tabContent);
    CHECK_NULL_VOID(tabsNode);

    // Lazy create TabBar internal child nodes when first TabContent is added
    if (!tabsNode->HasTabBarChildNodes()) {
        TabsModelNG::InitTabBarChildNodes(tabsNode);
        // TabBar transitions from "empty shell" to "has content",
        // trigger OnModifyDone to create GestureEventHub and initialize events
        auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabBar());
        if (tabBarNode) {
            tabBarNode->MarkModifyDone();
        }
    }

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabBar());
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarRenderContext = tabBarNode->GetRenderContext();
    CHECK_NULL_VOID(tabBarRenderContext);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(tabContentPattern);
    const auto& tabBarParam = tabContentPattern->GetTabBarParam();
    // Create column node to contain image and text or builder.
    auto columnNode = FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, tabContentNode->GetTabBarItemId(),
        []() { return AceType::MakeRefPtr<TabBarItemPattern>(); });
    auto tabTheme = tabsNode->GetTheme<TabTheme>(true);
    CHECK_NULL_VOID(tabTheme);
    auto linearLayoutProperty = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_VOID(linearLayoutProperty);
    linearLayoutProperty->UpdateMainAxisAlign(FlexAlign::CENTER);
    linearLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    linearLayoutProperty->UpdateSpace(tabTheme->GetBottomTabBarSpace());
    linearLayoutProperty->UpdatePixelRound(PIXEL_ROUND);
    auto columnRenderContext = columnNode->GetRenderContext();
    CHECK_NULL_VOID(columnRenderContext);
    if (tabTheme->GetIsChangeFocusTextStyle()) {
        columnRenderContext->UpdateClipEdge(false);
    } else {
        columnRenderContext->UpdateClipEdge(tabBarRenderContext->GetClipEdgeValue(true));
    }
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    tabBarPattern->SetTabBarStyle(tabBarParam.GetTabBarStyle());
    tabBarPattern->AddTabBarItemClickAndTouchEvent(columnNode);
    tabBarPattern->AddTabBarItemCallBack(columnNode);
    auto selectedMode = tabContentPattern->GetSelectedMode();
    auto indicatorStyle = tabContentPattern->GetIndicatorStyle();
    auto drawableIndicatorConfig = tabContentPattern->GetDrawableIndicatorConfig();
    auto isDrawableIndicator = tabContentPattern->IsDrawableIndicator();
    auto boardStyle = tabContentPattern->GetBoardStyle();
    auto bottomTabBarStyle = tabContentPattern->GetBottomTabBarStyle();
    auto padding = tabContentPattern->GetPadding();
    auto tabLayoutProperty = AceType::DynamicCast<TabsLayoutProperty>(tabsNode->GetLayoutProperty());
    CHECK_NULL_VOID(tabLayoutProperty);
    auto tabBarLayoutProperty = AceType::DynamicCast<TabBarLayoutProperty>(tabBarNode->GetLayoutProperty());
    CHECK_NULL_VOID(tabBarLayoutProperty);
    auto tabsDirection = tabLayoutProperty->GetNonAutoLayoutDirection();
    auto tabBarDirection = tabBarLayoutProperty->GetLayoutDirection();
    auto isRTL = tabBarDirection == TextDirection::RTL ||
             (tabBarDirection == TextDirection::AUTO && tabsDirection == TextDirection::RTL);
    if (isRTL && tabContentPattern->GetUseLocalizedPadding()) {
        PaddingProperty paddingRtl;
        paddingRtl.left = padding.right;
        paddingRtl.right = padding.left;
        paddingRtl.top = padding.top;
        paddingRtl.bottom = padding.bottom;
        padding = paddingRtl;
    }

    if (tabBarParam.GetTabBarStyle() == TabBarStyle::BOTTOMTABBATSTYLE) {
        if (bottomTabBarStyle.layoutMode == LayoutMode::HORIZONTAL) {
            linearLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
            linearLayoutProperty->UpdateSpace(tabTheme->GetHorizontalBottomTabBarSpace());
            linearLayoutProperty->UpdateCrossAxisAlign(bottomTabBarStyle.verticalAlign);
            linearLayoutProperty->SetIsVertical(false);
        } else {
            linearLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
            linearLayoutProperty->UpdateSpace(tabTheme->GetBottomTabBarSpace());
            linearLayoutProperty->UpdateMainAxisAlign(bottomTabBarStyle.verticalAlign);
            linearLayoutProperty->SetIsVertical(true);
        }
    }
    bool isTopFloating = tabLayoutProperty->GetBarLayoutStyleValue(TabBarLayoutStyle::BOTTOM) ==
        TabBarLayoutStyle::TOP;
    if (isTopFloating) {
        linearLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
        linearLayoutProperty->UpdateSpace(0.0_vp);
        linearLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
        linearLayoutProperty->SetIsVertical(false);
    }

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto myIndex = swiperNode->GetChildFlatIndex(tabContentId).second;

    auto newTabBar = tabBarPattern->IsNewTabBar(columnNode->GetId());
    tabBarPattern->SetTabBarStyle(tabBarParam.GetTabBarStyle(), myIndex, newTabBar);
    tabBarPattern->AddTabBarItemId(columnNode->GetId(), myIndex, newTabBar);
    tabBarPattern->SetBottomTabBarStyle(bottomTabBarStyle, myIndex, newTabBar);
    auto labelStyle = tabContentPattern->GetLabelStyle();
    tabBarPattern->SetLabelStyle(columnNode->GetId(), labelStyle);
    auto iconStyle = tabContentPattern->GetIconStyle();
    tabBarPattern->SetIconStyle(iconStyle, myIndex, newTabBar);
    auto symbol = tabContentPattern->GetSymbol();
    tabBarPattern->SetSymbol(symbol, myIndex, newTabBar);
    auto tabBarStyle = tabContentPattern->GetTabBarStyle();
    if (tabBarStyle == TabBarStyle::SUBTABBATSTYLE) {
        auto renderContext = columnNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        BorderRadiusProperty borderRadiusProperty;
        borderRadiusProperty.SetRadius(boardStyle.borderRadius);
        renderContext->UpdateBorderRadius(borderRadiusProperty);
    }
    if (isTopFloating) {
        auto renderContext = columnNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        BorderRadiusProperty borderRadiusProperty;
        borderRadiusProperty.SetRadius(Dimension(23, DimensionUnit::VP));
        renderContext->UpdateBorderRadius(borderRadiusProperty);
    }
    if (tabBarStyle != TabBarStyle::SUBTABBATSTYLE) {
        indicatorStyle.marginTop = 0.0_vp;
    }
    tabBarPattern->SetSelectedMode(selectedMode, myIndex, newTabBar);
    tabBarPattern->SetIndicatorStyle(indicatorStyle, myIndex, newTabBar);
    tabBarPattern->SetDrawableIndicatorConfig(drawableIndicatorConfig, myIndex, newTabBar);
    tabBarPattern->SetDrawableIndicatorFlag(isDrawableIndicator, myIndex, newTabBar);

    if (tabBarParam.GetTabBarStyle() == TabBarStyle::NOSTYLE && !tabBarParam.HasBuilder() &&
        !tabBarParam.HasContent() && tabBarParam.GetIcon().empty() && tabBarParam.GetText().empty()) {
        if (!columnNode->GetChildren().empty()) {
            columnNode->Clean();
        }
    }

    auto columnFocusHub = columnNode->GetFocusHub();
    if (columnFocusHub) {
        columnFocusHub->SetFocusDependence(FocusDependence::SELF);
    }

    // Apply defaultVisibility for this single bottom tab bar item
    const auto& defaultVisibility = tabContentPattern->GetDefaultVisibility();
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    auto columnProperty = columnNode->GetLayoutProperty();
    if (tabsPattern && columnProperty) {
        auto activeDisplayMode = tabsPattern->GetActiveBarDisplayMode();
        bool shouldHide = defaultVisibility.ShouldHide(activeDisplayMode);
        columnProperty->UpdateVisibility(shouldHide ? VisibleType::GONE : VisibleType::VISIBLE);
    }

    // Create tab bar with content.
    if (tabBarParam.HasContent()) {
        ScopedViewStackProcessor builderViewStackProcessor;
        auto builderNode = tabBarParam.GetContent().Upgrade();
        CHECK_NULL_VOID(builderNode);
        if (!columnNode->GetChildren().empty()) {
            columnNode->Clean();
        }
        if (builderNode) {
            builderNode->MountToParent(columnNode);
        }
        tabBarPattern->SetIsExecuteBuilder(false);
        auto oldColumnNode = tabsNode->GetBuilderByContentId(tabContentId, columnNode);
        if (oldColumnNode != columnNode) {
            if (!oldColumnNode) {
                auto index =
                    std::clamp(myIndex, 0, static_cast<int32_t>(tabBarNode->GetChildren().size()) -
                    MASK_COUNT - IMAGE_INDICATOR_COUNT);
                columnNode->MountToParent(tabBarNode, index);
            } else if (oldColumnNode != columnNode) {
                tabBarNode->ReplaceChild(oldColumnNode, columnNode);
            }
        }
        auto tabBarItemPadding = Dimension(0);
        auto layoutProperty = columnNode->GetLayoutProperty();
        layoutProperty->UpdatePadding({ CalcLength(tabBarItemPadding), CalcLength(tabBarItemPadding),
            CalcLength(tabBarItemPadding), CalcLength(tabBarItemPadding), {}, {} });
        columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
        tabBarPattern->AddTabBarItemType(columnNode->GetId(), TabBarParamType::COMPONENT_CONTENT);
        tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
        return;
    }

    // Create tab bar with builder.
    if (tabBarParam.HasBuilder()) {
        ScopedViewStackProcessor builderViewStackProcessor;
        tabBarParam.ExecuteBuilder();
        auto builderNode = ViewStackProcessor::GetInstance()->Finish();
        if (static_cast<int32_t>(columnNode->GetChildren().size()) != 0) {
            columnNode->Clean();
        }
        if (builderNode) {
            builderNode->MountToParent(columnNode);
        }
        auto oldColumnNode = tabsNode->GetBuilderByContentId(tabContentId, columnNode);
        if (!oldColumnNode) {
            auto index = std::clamp(myIndex, 0, static_cast<int32_t>(tabBarNode->GetChildren().size()) -
                MASK_COUNT - IMAGE_INDICATOR_COUNT);
            columnNode->MountToParent(tabBarNode, index);
        } else if (oldColumnNode != columnNode) {
            tabBarNode->ReplaceChild(oldColumnNode, columnNode);
        }
        columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
        tabBarPattern->AddTabBarItemType(columnNode->GetId(), TabBarParamType::CUSTOM_BUILDER);
        tabBarPattern->SetIsExecuteBuilder(true);
        tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
        return;
    }

    // Create text node and image node.
    RefPtr<FrameNode> textNode;
    RefPtr<FrameNode> iconNode;
    auto layoutProperty = columnNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    if (isTopFloating) {
        PaddingProperty topPadding;
        topPadding.SetEdges(CalcLength(18, DimensionUnit::VP), CalcLength(18, DimensionUnit::VP),
            CalcLength(0, DimensionUnit::VP), CalcLength(0, DimensionUnit::VP));
        layoutProperty->UpdatePadding(topPadding);
    } else if (tabBarStyle == TabBarStyle::SUBTABBATSTYLE || tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE) {
        layoutProperty->UpdatePadding(padding);
        auto accessibilityProperty = columnNode->GetAccessibilityProperty<AccessibilityProperty>();
        accessibilityProperty->SetAccessibilityGroup(true);
        auto id = tabContentPattern->GetId();
        columnNode->UpdateInspectorId(id);
    } else {
        auto tabBarItemPadding = tabTheme->GetSubTabItemPadding();
        auto subTabItemHorizontalPadding_ = tabTheme->GetSubTabItemHorizontalPadding();
        layoutProperty->UpdatePadding({ CalcLength(subTabItemHorizontalPadding_),
            CalcLength(subTabItemHorizontalPadding_), CalcLength(tabBarItemPadding),
            CalcLength(tabBarItemPadding), {}, {} });
    }

    bool isFrameNode = tabBarStyle == TabBarStyle::SUBTABBATSTYLE && tabContentPattern->HasSubTabBarStyleNode();
    if (isFrameNode) {
        tabBarPattern->AddTabBarItemType(columnNode->GetId(), TabBarParamType::SUB_COMPONENT_CONTENT);
    } else {
        tabBarPattern->AddTabBarItemType(columnNode->GetId(), TabBarParamType::NORMAL);
    }
    bool hasBadge = tabContentPattern->GetTabBarBadge().has_value() && tabBarStyle != TabBarStyle::SUBTABBATSTYLE;
    if (static_cast<int32_t>(columnNode->GetChildren().size()) == 0) {
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
            textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<TextPattern>(); });
        }
        CHECK_NULL_VOID(textNode);
        CHECK_NULL_VOID(iconNode);
        auto index = std::clamp(position, 0, static_cast<int32_t>(tabBarNode->GetChildren().size()) -
            MASK_COUNT - IMAGE_INDICATOR_COUNT);
        columnNode->MountToParent(tabBarNode, index);
        if (hasBadge) {
            WrapWithBadge(
                columnNode, iconNode, textNode, tabContentPattern->GetTabBarBadge().value(), linearLayoutProperty);
        } else {
            iconNode->MountToParent(columnNode);
            textNode->MountToParent(columnNode);
        }
    } else {
        UpdateBottomStyleBadge(columnNode, tabContentPattern, hasBadge, linearLayoutProperty);

        if (isFrameNode) {
            auto builderNode = tabContentPattern->FireCustomStyleNode();
            auto currentTextNode = TabBarPattern::GetTextNodeFromColumn(columnNode);
            columnNode->ReplaceChild(currentTextNode, builderNode);
        }
        auto oldIcon = TabBarPattern::GetIconNodeFromColumn(columnNode);
        CHECK_NULL_VOID(oldIcon);
        if (tabBarParam.GetSymbol().has_value() && oldIcon->GetTag() != V2::SYMBOL_ETS_TAG) {
            auto icon = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
            columnNode->ReplaceChild(oldIcon, icon);
        } else if (!tabBarParam.GetIcon().empty() && oldIcon->GetTag() != V2::IMAGE_ETS_TAG) {
            auto icon = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
                ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
            columnNode->ReplaceChild(oldIcon, icon);
        }
        iconNode = TabBarPattern::GetIconNodeFromColumn(columnNode);
        textNode = TabBarPattern::GetTextNodeFromColumn(columnNode);
    }
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
    tabBarPattern->UpdateSubTabBoard(indicator);
    // Update property of text.
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    InitTabText(textLayoutProperty);
    auto axis = tabBarLayoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    if ((!swiperPattern->IsUseCustomAnimation() || !swiperPattern->GetCustomAnimationToIndex().has_value()) &&
        !isFrameNode) {
        if (myIndex == indicator) {
            if (labelStyle.selectedColor.has_value()) {
                textLayoutProperty->UpdateTextColor(labelStyle.selectedColor.value());
            } else {
                selectedMode == SelectedMode::BOARD && axis == Axis::HORIZONTAL ?
                    textLayoutProperty->UpdateTextColor(tabTheme->GetSubTabBoardTextOnColor()) :
                    textLayoutProperty->UpdateTextColor(tabTheme->GetSubTabTextOnColor());
            }
        } else {
            if (labelStyle.unselectedColor.has_value()) {
                textLayoutProperty->UpdateTextColor(labelStyle.unselectedColor.value());
            } else {
                textLayoutProperty->UpdateTextColor(tabTheme->GetSubTabTextOffColor());
            }
        }
    }

    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateClipEdge(true);
    if (!isFrameNode) {
        textLayoutProperty->UpdateContent(tabBarParam.GetText());
        textLayoutProperty->UpdateFontSize(tabTheme->GetSubTabTextDefaultFontSize());
        textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
        textLayoutProperty->UpdateMaxLines(1);
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    }
    if (tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE) {
        textLayoutProperty->UpdateFlexShrink(1.0f);
    }

    if (!tabBarParam.GetIcon().empty() || tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE) {
        textLayoutProperty->UpdateFontSize(tabTheme->GetBottomTabTextSize());
    }
    if (isTopFloating) {
        textLayoutProperty->UpdateFontSize(tabTheme->GetTopTabTextSize());
        textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
    } else if (tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE) {
        textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
    } else if (tabBarStyle == TabBarStyle::SUBTABBATSTYLE && !isFrameNode) {
        textLayoutProperty->UpdateFontWeight(myIndex == indicator ? FontWeight::MEDIUM : FontWeight::NORMAL);
    }
    if (tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE && bottomTabBarStyle.layoutMode == LayoutMode::HORIZONTAL) {
        textLayoutProperty->UpdateTextAlign(TextAlign::LEFT);
        if (!labelStyle.fontSize.has_value() &&
            Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
            textLayoutProperty->UpdateFontSize(tabTheme->GetBottomTabHorizontalTextSize());
        }
    }
    if (!isFrameNode) {
        UpdateLabelStyle(labelStyle, textLayoutProperty);
    }

    // Update property of image/symbol.
    if (tabBarParam.GetSymbol().has_value()) {
        auto symbolProperty = iconNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(symbolProperty);
        UpdateDefaultSymbol(tabTheme, symbolProperty);
        auto modifierOnApply = tabBarParam.GetSymbol().value().onApply;
        if (myIndex == indicator) {
            tabBarPattern->SetImageColorOnIndex(indicator);
            symbolProperty->UpdateSymbolColorList({tabTheme->GetBottomTabSymbolOn()});
            if (modifierOnApply != nullptr) {
                modifierOnApply(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(iconNode)), "normal");
                UpdateDefaultSymbol(tabTheme, symbolProperty);
                symbolProperty->UpdateSymbolColorList({tabTheme->GetBottomTabSymbolOn()});
            }
            if (modifierOnApply != nullptr && tabBarParam.GetSymbol().value().selectedFlag) {
                modifierOnApply(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(iconNode)),
                    "selected");
                UpdateSymbolEffect(symbolProperty, false);
            }
        } else {
            symbolProperty->UpdateSymbolColorList({tabTheme->GetBottomTabSymbolOff()});
            if (modifierOnApply != nullptr) {
                modifierOnApply(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(iconNode)), "normal");
                UpdateSymbolEffect(symbolProperty, false);
            }
        }
    } else {
        auto imageProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageProperty);
        if (!tabBarParam.GetIcon().empty() || tabBarStyle == TabBarStyle::BOTTOMTABBATSTYLE) {
            imageProperty->UpdateUserDefinedIdealSize(CalcSize(
                NG::CalcLength(tabTheme->GetBottomTabImageSize()), NG::CalcLength(tabTheme->GetBottomTabImageSize())));
        } else {
            imageProperty->UpdateUserDefinedIdealSize(CalcSize());
        }
        ImageSourceInfo imageSourceInfo(tabBarParam.GetIcon());
        auto imagePaintProperty = iconNode->GetPaintProperty<ImageRenderProperty>();
        CHECK_NULL_VOID(imagePaintProperty);
        if (imageSourceInfo.IsSvg() && myIndex == indicator) {
            tabBarPattern->SetImageColorOnIndex(indicator);
            if (iconStyle.selectedColor.has_value()) {
                imagePaintProperty->UpdateSvgFillColor(iconStyle.selectedColor.value());
            } else {
                imagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOn());
            }
        }
        if (imageSourceInfo.IsSvg() && myIndex != indicator) {
            if (iconStyle.unselectedColor.has_value()) {
                imagePaintProperty->UpdateSvgFillColor(iconStyle.unselectedColor.value());
            } else {
                imagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOff());
            }
        }
        imageProperty->UpdateImageSourceInfo(imageSourceInfo);
    }
    columnNode->MarkModifyDone();
    textNode->MarkModifyDone();
    textNode->MarkDirtyNode();
    iconNode->MarkModifyDone();
    tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void TabContentModelNG::AddSideBarAdaptableStyleTabBarItem(
    const RefPtr<UINode>& tabContent, int32_t position, bool update)
{
    CHECK_NULL_VOID(tabContent);
    auto tabContentNode = AceType::DynamicCast<TabContentNode>(tabContent);
    CHECK_NULL_VOID(tabContentNode);
    auto tabsNode = FindTabsNode(tabContent);
    CHECK_NULL_VOID(tabsNode);
    // Always record TabContentNode in TabsPattern for deferred SideBar registration
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    if (tabsPattern) {
        tabsPattern->AddTabContentNode(tabContentNode);
    }
    AddSideBarTabListItem(tabsNode, tabContentNode, position, update);
}

void TabContentModelNG::AddSideBarTabListItem(const RefPtr<TabsNode>& tabsNode,
    const RefPtr<TabContentNode>& tabContentNode, int32_t position, bool update)
{
    CHECK_NULL_VOID(tabsNode);
    CHECK_NULL_VOID(tabContentNode);
    if (update && !tabContentNode->HasSideBarTabBarItemId()) {
        return;
    }
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto sideBarNode = tabsPattern->GetSideBarNode();
    CHECK_NULL_VOID(sideBarNode);
    auto sideBarPattern = sideBarNode->GetPattern<TabsSideBarPattern>();
    CHECK_NULL_VOID(sideBarPattern);
    auto tabListNode = sideBarPattern->GetTabListNode();
    CHECK_NULL_VOID(tabListNode);
    auto tabListPattern = tabListNode->GetPattern<TabsSideBarTabListPattern>();
    CHECK_NULL_VOID(tabListPattern);
    tabListPattern->AddOrUpdateTabListItem(tabContentNode, position, update);
}

void TabContentModelNG::RemoveTabBarItem(const RefPtr<TabContentNode>& tabContentNode)
{
    RemoveBottomStyleTabBarItem(tabContentNode);
    RemoveSideBarAdaptableStyleTabBarItem(tabContentNode);
}

void TabContentModelNG::RemoveBottomStyleTabBarItem(const RefPtr<TabContentNode>& tabContentNode)
{
    CHECK_NULL_VOID(tabContentNode);
    if (!tabContentNode->HasTabBarItemId()) {
        return;
    }

    auto tabBarItemId = tabContentNode->GetTabBarItemId();
    auto tabBarItemNode = ElementRegister::GetInstance()->GetUINodeById(tabBarItemId);
    CHECK_NULL_VOID(tabBarItemNode);
    auto tabBarNode = tabBarItemNode->GetParent();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarFrameNode = AceType::DynamicCast<FrameNode>(tabBarNode);
    CHECK_NULL_VOID(tabBarFrameNode);
    auto tabBarPattern = tabBarFrameNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    tabBarPattern->RemoveTabBarItemInfo(tabBarItemId);
    tabBarNode->RemoveChild(tabBarItemNode);
    tabContentNode->ResetTabBarItemId();

    auto tabsNode = FindTabsNode(tabContentNode);
    CHECK_NULL_VOID(tabsNode);
    tabsNode->RemoveBuilderByContentId(tabContentNode->GetId());
    tabBarFrameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void TabContentModelNG::RemoveSideBarAdaptableStyleTabBarItem(const RefPtr<TabContentNode>& tabContentNode)
{
    CHECK_NULL_VOID(tabContentNode);
    auto tabsNode = FindTabsNode(tabContentNode);
    CHECK_NULL_VOID(tabsNode);
    // Remove from TabsPattern's record
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    if (tabsPattern) {
        tabsPattern->RemoveTabContentNode(tabContentNode);
    }
    RemoveSideBarTabBarItem(tabsNode, tabContentNode);
}

void TabContentModelNG::RemoveSideBarTabBarItem(const RefPtr<TabsNode>& tabsNode,
    const RefPtr<TabContentNode>& tabContentNode)
{
    CHECK_NULL_VOID(tabsNode);
    CHECK_NULL_VOID(tabContentNode);

    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto sideBarNode = tabsPattern->GetSideBarNode();
    CHECK_NULL_VOID(sideBarNode);
    auto sideBarPattern = sideBarNode->GetPattern<TabsSideBarPattern>();
    CHECK_NULL_VOID(sideBarPattern);
    auto tabListNode = sideBarPattern->GetTabListNode();
    CHECK_NULL_VOID(tabListNode);
    auto tabListPattern = tabListNode->GetPattern<TabsSideBarTabListPattern>();
    CHECK_NULL_VOID(tabListPattern);
    auto tabItemContainerNode = tabListPattern->GetTabItemContainerNode();
    CHECK_NULL_VOID(tabItemContainerNode);

    auto sideBarTabBarItemId = tabContentNode->GetSideBarTabBarItemId();
    auto tabBarItemNode = ElementRegister::GetInstance()->GetUINodeById(sideBarTabBarItemId);
    if (tabBarItemNode) {
        tabItemContainerNode->RemoveChild(tabBarItemNode);
    }
    tabContentNode->ResetSideBarTabBarItemId();
    tabListPattern->RemoveBuilderByContentId(tabContentNode->GetId());
    tabItemContainerNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

template<typename T>
bool ParseType(const RefPtr<ResourceObject>& resObj, const std::string& name, T& result)
{
    if constexpr (std::is_same_v<T, Color>) {
        if (!ResourceParseUtils::ParseResColor(resObj, result)) {
            auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
            CHECK_NULL_RETURN(frameNode, false);
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_RETURN(tabTheme, false);
            result = tabTheme->GetActiveIndicatorColor();
            return false;
        }
        return true;
    } else if constexpr (std::is_same_v<T, std::optional<Color>>) {
        if (name == "selectedColor" || name == "unselectedColor") {
            Color color;
            if (ResourceParseUtils::ParseResColor(resObj, color, true)) {
                result = color;
            }
            return true;
        }
    } else if constexpr (std::is_same_v<T, std::optional<Dimension>>) {
        if (name == "fontSize") {
            CalcDimension fontSize;
            if (ResourceParseUtils::ParseResDimensionFp(resObj, fontSize) && NonNegative(fontSize.Value()) &&
                fontSize.Unit() != DimensionUnit::PERCENT) {
                result = fontSize;
            }
            return true;
        }
    } else if constexpr (std::is_same_v<T, CalcDimension>) {
        if (name == "height" || name == "borderRadius" || name == "width" || name == "marginTop") {
            return ResourceParseUtils::ParseResDimensionVp(resObj, result);
        } else if (name == "minFontSize" || name == "maxFontSize") {
            return ResourceParseUtils::ParseResDimensionFp(resObj, result);
        } else {
            return ResourceParseUtils::ParseResDimensionNG(resObj, result, DimensionUnit::PX);
        }
    } else if constexpr (std::is_same_v<T, Dimension>) {
        return ResourceParseUtils::ParseResDimensionFpNG(resObj, result);
    } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        return ResourceParseUtils::ParseResFontFamilies(resObj, result);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return ResourceParseUtils::ParseResString(resObj, result);
    }
    return false;
}

#define REGISTER_RESOURCE_UPDATE_ATTR_FUNC(caseType, attrType, name, resObj, resultType)                          \
    case caseType:                                                                                                \
        do {                                                                                                      \
            auto pattern = frameNode->GetPattern<TabContentPattern>();                                            \
            CHECK_NULL_VOID(pattern);                                                                             \
            const std::string key = "tabContent." #attrType #name;                                                \
            pattern->RemoveResObj(key);                                                                           \
            CHECK_NULL_VOID(resObj);                                                                              \
            auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& theResObj) { \
                auto frameNode = weak.Upgrade();                                                                  \
                CHECK_NULL_VOID(frameNode);                                                                       \
                auto pattern = frameNode->GetPattern<TabContentPattern>();                                        \
                CHECK_NULL_VOID(pattern);                                                                         \
                resultType result;                                                                                \
                ParseType(theResObj, #name, result);                                                              \
                auto attrs = pattern->Get##attrType();                                                            \
                attrs.name = result;                                                                              \
                pattern->Set##attrType(attrs);                                                                    \
            };                                                                                                    \
            pattern->AddResObj(key, resObj, std::move(updateFunc));                                               \
        } while (false);                                                                                          \
        break

#define REGISTER_RESOURCE_UPDATE_ATTR_FONT_SIZE_FUNC(caseType, attrType, name, resObj, resultType)                \
    case caseType:                                                                                                \
        do {                                                                                                      \
            auto pattern = frameNode->GetPattern<TabContentPattern>();                                            \
            CHECK_NULL_VOID(pattern);                                                                             \
            const std::string key = "tabContent." #attrType #name;                                                \
            pattern->RemoveResObj(key);                                                                           \
            CHECK_NULL_VOID(resObj);                                                                              \
            auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& theResObj) { \
                auto frameNode = weak.Upgrade();                                                                  \
                CHECK_NULL_VOID(frameNode);                                                                       \
                auto pattern = frameNode->GetPattern<TabContentPattern>();                                        \
                CHECK_NULL_VOID(pattern);                                                                         \
                resultType result;                                                                                \
                if (ParseType(theResObj, #name, result)) {                                                        \
                    auto attrs = pattern->Get##attrType();                                                        \
                    attrs.name = result;                                                                          \
                    pattern->Set##attrType(attrs);                                                                \
                }                                                                                                 \
            };                                                                                                    \
            pattern->AddResObj(key, resObj, std::move(updateFunc));                                               \
        } while (false);                                                                                          \
        break

void TabContentModelNG::CreateWithResourceObj(TabContentJsType jsType, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_VOID(SystemProperties::ConfigChangePerform());
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    switch (jsType) {
        REGISTER_RESOURCE_UPDATE_ATTR_FONT_SIZE_FUNC(
            TabContentJsType::FONT_SIZE, LabelStyle, fontSize, resObj, std::optional<Dimension>);
        REGISTER_RESOURCE_UPDATE_ATTR_FUNC(
            TabContentJsType::FONT_FAMILY, LabelStyle, fontFamily, resObj, std::vector<std::string>);
        REGISTER_RESOURCE_UPDATE_ATTR_FONT_SIZE_FUNC(
            TabContentJsType::MIN_FONT_SIZE, LabelStyle, minFontSize, resObj, CalcDimension);
        REGISTER_RESOURCE_UPDATE_ATTR_FONT_SIZE_FUNC(
            TabContentJsType::MAX_FONT_SIZE, LabelStyle, maxFontSize, resObj, CalcDimension);
        REGISTER_RESOURCE_UPDATE_ATTR_FUNC(
            TabContentJsType::LABEL_SELECT_COLOR, LabelStyle, selectedColor, resObj, std::optional<Color>);
        REGISTER_RESOURCE_UPDATE_ATTR_FUNC(
            TabContentJsType::LABEL_UNSELECT_COLOR, LabelStyle, unselectedColor, resObj, std::optional<Color>);
        REGISTER_RESOURCE_UPDATE_ATTR_FUNC(
            TabContentJsType::ICON_SELECT_COLOR, IconStyle, selectedColor, resObj, std::optional<Color>);
        REGISTER_RESOURCE_UPDATE_ATTR_FUNC(
            TabContentJsType::ICON_UNSELECT_COLOR, IconStyle, unselectedColor, resObj, std::optional<Color>);
        default:
            CreateMoreWithResourceObj(jsType, frameNode, resObj);
            break;
    }
}

void TabContentModelNG::CreateMoreWithResourceObj(TabContentJsType jsType, FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_VOID(frameNode);
    switch (jsType) {
        case TabContentJsType::INDICATOR_COLOR:
            CreateIndicatorColorWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::INDICATOR_HEIGHT:
            CreateIndicatorHeightWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::INDICATOR_WIDTH:
            CreateIndicatorWidthWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::INDICATOR_RADIUS:
            CreateIndicatorBorderRadiusWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::INDICATOR_MARGIN_TOP:
            CreateIndicatorMarginTopWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::BORDER_RADIUS:
            CreateBoardStyleBorderRadiusWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::PADDING:
            CreatePaddingWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::TEXT_CONTENT:
            CreateTextContentWithResourceObj(frameNode, resObj);
            break;
        case TabContentJsType::TAB_BAR_OPTIONS_ICON: {
            CreateIconWithResourceObjWithKey(frameNode, "tabContent.tabBarOptions", resObj);
            break;
        }
        case TabContentJsType::BOTTOM_TAB_BAR_STYLE_ICON: {
            CreateIconWithResourceObjWithKey(frameNode, "tabContent.bottomTabBarStyle", resObj);
            break;
        }
        default:
            break;
    }
}

bool TabContentModelNG::CreateIconWithResourceObjWithKey(FrameNode* frameNode, const std::string key,
    const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);
    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        std::string result;
        ResourceParseUtils::ParseResMedia(resObj, result);
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, Icon, result, frameNode);
        pattern->UpdateTabBarParamIcon(result);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

void TabContentModelNG::CreatePaddingHorWithResourceObj(const RefPtr<ResourceObject>& resObjLeft,
    const RefPtr<ResourceObject>& resObjRight, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_VOID(SystemProperties::ConfigChangePerform());
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    CreatePaddingLeftWithResourceObj(frameNode, resObjLeft, isSubTabStyle, useLocalizedPadding);
    CreatePaddingRightWithResourceObj(frameNode, resObjRight, isSubTabStyle, useLocalizedPadding);
}

void TabContentModelNG::CreatePaddingVerWithResourceObj(const RefPtr<ResourceObject>& resObjTop,
    const RefPtr<ResourceObject>& resObjBottom, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_VOID(SystemProperties::ConfigChangePerform());
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    CreatePaddingTopWithResourceObj(frameNode, resObjTop, isSubTabStyle, useLocalizedPadding);
    CreatePaddingBottomWithResourceObj(frameNode, resObjBottom, isSubTabStyle, useLocalizedPadding);
}

bool TabContentModelNG::CreatePaddingWithResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.tabBarPadding";
    pattern->RemoveResObj(key);
    pattern->RemoveResObj(std::string(KEY_PADDING_LEFT));
    pattern->RemoveResObj(std::string(KEY_PADDING_RIGHT));
    pattern->RemoveResObj(std::string(KEY_PADDING_TOP));
    pattern->RemoveResObj(std::string(KEY_PADDING_BOTTOM));
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        if (ResourceParseUtils::ParseResDimensionVp(resObj, result) && NonNegative(result.Value()) &&
            result.Unit() != DimensionUnit::PERCENT) {
            NG::PaddingProperty padding;
            padding.left = NG::CalcLength(result);
            padding.right = NG::CalcLength(result);
            padding.top = NG::CalcLength(result);
            padding.bottom = NG::CalcLength(result);
            pattern->SetPadding(padding);
        }
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateTextContentWithResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.tabBarParamText";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        std::string result;
        ResourceParseUtils::ParseResString(resObj, result);
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, Text, result, frameNode);
        pattern->UpdateTabBarParamText(result);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreatePaddingLeftWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObjLeft, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->RemoveResObj(std::string(KEY_PADDING));
    pattern->RemoveResObj(std::string(KEY_PADDING_LEFT));
    CHECK_NULL_RETURN(resObjLeft, true);

    auto&& updateFunc = [weakNode = AceType::WeakClaim(frameNode), isSubTabStyle](
        const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension left;
        auto padding = pattern->GetPadding();
        if (ResourceParseUtils::ParseResDimensionVp(resObj, left) && NonNegative(left.Value()) &&
            left.Unit() != DimensionUnit::PERCENT) {
            padding.left = NG::CalcLength(left);
        } else {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            padding.left = (isSubTabStyle) ? NG::CalcLength(tabTheme->GetSubTabHorizontalPadding()) :
                NG::CalcLength(tabTheme->GetBottomTabHorizontalPadding());
        }
        pattern->SetPadding(padding);
    };
    pattern->AddResObj(std::string(KEY_PADDING_LEFT), resObjLeft, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreatePaddingRightWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObjRight, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->RemoveResObj(std::string(KEY_PADDING));
    pattern->RemoveResObj(std::string(KEY_PADDING_RIGHT));
    CHECK_NULL_RETURN(resObjRight, true);

    auto&& updateFunc = [weakNode = AceType::WeakClaim(frameNode), isSubTabStyle](
        const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension right;
        auto padding = pattern->GetPadding();
        if (ResourceParseUtils::ParseResDimensionVp(resObj, right) && NonNegative(right.Value()) &&
            right.Unit() != DimensionUnit::PERCENT) {
            padding.right = NG::CalcLength(right);
        } else {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            padding.right = (isSubTabStyle) ? NG::CalcLength(tabTheme->GetSubTabHorizontalPadding()) :
                NG::CalcLength(tabTheme->GetBottomTabHorizontalPadding());
        }
        pattern->SetPadding(padding);
    };
    pattern->AddResObj(std::string(KEY_PADDING_RIGHT), resObjRight, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreatePaddingTopWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObjTop, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->RemoveResObj(std::string(KEY_PADDING));
    pattern->RemoveResObj(std::string(KEY_PADDING_TOP));
    CHECK_NULL_RETURN(resObjTop, true);

    auto&& updateFunc = [weakNode = AceType::WeakClaim(frameNode), isSubTabStyle](
        const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension top;
        auto padding = pattern->GetPadding();
        if (ResourceParseUtils::ParseResDimensionVp(resObj, top) && NonNegative(top.Value()) &&
            top.Unit() != DimensionUnit::PERCENT) {
            padding.top = NG::CalcLength(top);
        } else {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            padding.top = (isSubTabStyle) ? NG::CalcLength(tabTheme->GetSubTabTopPadding()) : NG::CalcLength(0.0_vp);
        }
        pattern->SetPadding(padding);
    };
    pattern->AddResObj(std::string(KEY_PADDING_TOP), resObjTop, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreatePaddingBottomWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObjBottom, bool isSubTabStyle, bool useLocalizedPadding)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->RemoveResObj(std::string(KEY_PADDING));
    pattern->RemoveResObj(std::string(KEY_PADDING_BOTTOM));
    CHECK_NULL_RETURN(resObjBottom, true);

    auto&& updateFunc = [weakNode = AceType::WeakClaim(frameNode), isSubTabStyle](
        const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension bottom;
        auto padding = pattern->GetPadding();
        if (ResourceParseUtils::ParseResDimensionVp(resObj, bottom) && NonNegative(bottom.Value()) &&
            bottom.Unit() != DimensionUnit::PERCENT) {
            padding.bottom = NG::CalcLength(bottom);
        } else {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            padding.bottom = (isSubTabStyle) ? NG::CalcLength(tabTheme->GetSubTabBottomPadding()) :
                NG::CalcLength(0.0_vp);
        }
        pattern->SetPadding(padding);
    };
    pattern->AddResObj(std::string(KEY_PADDING_BOTTOM), resObjBottom, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateBoardStyleBorderRadiusWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.BoardStyle.borderRadius";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        auto attrs = pattern->GetBoardStyle();
        if (!ParseType(resObj, "borderRadius", result) || result.Value() < 0.0f ||
            result.Unit() == DimensionUnit::PERCENT) {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            attrs.borderRadius = tabTheme->GetFocusIndicatorRadius();
        } else {
            attrs.borderRadius = result;
        }
        pattern->SetBoardStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateIndicatorColorWithResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.IndicatorStyle.color";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        Color result;
        auto attrs = pattern->GetIndicatorStyle();
        if (!ParseType(resObj, "color", result)) {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            attrs.color = tabTheme->GetActiveIndicatorColor();
        } else {
            attrs.color = result;
        }
        pattern->SetIndicatorStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateIndicatorHeightWithResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.IndicatorStyle.height";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        auto attrs = pattern->GetIndicatorStyle();
        if (!ParseType(resObj, "height", result) || result.Value() < 0.0f ||
            result.Unit() == DimensionUnit::PERCENT) {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            attrs.height = tabTheme->GetActiveIndicatorWidth();
        } else {
            attrs.height = result;
        }
        pattern->SetIndicatorStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateIndicatorWidthWithResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.IndicatorStyle.width";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        auto attrs = pattern->GetIndicatorStyle();
        if (!ParseType(resObj, "width", result) || result.Value() < 0.0f || result.Unit() == DimensionUnit::PERCENT) {
            attrs.width = 0.0_vp;
        } else {
            attrs.width = result;
        }
        pattern->SetIndicatorStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateIndicatorBorderRadiusWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.IndicatorStyle.borderRadius";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        auto attrs = pattern->GetIndicatorStyle();
        if (!ParseType(resObj, "borderRadius", result) || result.Value() < 0.0f ||
            result.Unit() == DimensionUnit::PERCENT) {
            attrs.borderRadius = 0.0_vp;
        } else {
            attrs.borderRadius = result;
        }
        pattern->SetIndicatorStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

bool TabContentModelNG::CreateIndicatorMarginTopWithResourceObj(FrameNode* frameNode,
    const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pattern = frameNode->GetPattern<TabContentPattern>();
    CHECK_NULL_RETURN(pattern, false);
    const std::string key = "tabContent.IndicatorStyle.marginTop";
    pattern->RemoveResObj(key);
    CHECK_NULL_RETURN(resObj, true);

    auto&& updateFunc = [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<TabContentPattern>();
        CHECK_NULL_VOID(pattern);
        CalcDimension result;
        auto attrs = pattern->GetIndicatorStyle();
        if (!ParseType(resObj, "marginTop", result) || result.Value() < 0.0f ||
            result.Unit() == DimensionUnit::PERCENT) {
            auto tabTheme = frameNode->GetTheme<TabTheme>(true);
            CHECK_NULL_VOID(tabTheme);
            attrs.marginTop = tabTheme->GetSubTabIndicatorGap();
        } else {
            attrs.marginTop = result;
        }
        pattern->SetIndicatorStyle(attrs);
    };
    pattern->AddResObj(key, resObj, std::move(updateFunc));
    return true;
}

void TabContentModelNG::SetTabBar(const std::optional<std::string>& text, const std::optional<std::string>& icon,
    const std::optional<TabBarSymbol>& tabBarSymbol, TabBarBuilderFunc&& builder, bool /*useContentOnly*/)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, Icon, icon.value_or(""));
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, Text, text.value_or(""));
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBar(text.value_or(""), icon.value_or(""), tabBarSymbol, std::move(builder));
    frameNodePattern->SetTabBarWithContent(nullptr);
}

void TabContentModelNG::SetTabBarWithContent(const RefPtr<NG::UINode>& content)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBarWithContent(content);
}

void TabContentModelNG::SetTabBarStyle(TabBarStyle tabBarStyle)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBarStyle(tabBarStyle);
}

void TabContentModelNG::SetIndicator(const IndicatorStyle& indicator)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetIndicatorStyle(indicator);
}

void TabContentModelNG::SetDrawableIndicatorConfig(const ImageInfoConfig& config)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetDrawableIndicatorConfig(config);
}

void TabContentModelNG::SetDrawableIndicatorFlag(bool isDrawableIndicator)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetDrawableIndicatorFlag(isDrawableIndicator);
}

void TabContentModelNG::SetIndicatorColorByUser(bool isByUser)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, IndicatorColorSetByUser, isByUser);
}

void TabContentModelNG::SetCustomTabBar(FrameNode* node, FrameNode* tabBar)
{
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(tabBar);
    auto frameNodePattern = node->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBarStyle(TabBarStyle::NOSTYLE);
    frameNodePattern->SetCustomTabBar(tabBar);
}

void TabContentModelNG::SetBoard(const BoardStyle& board)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetBoardStyle(board);
}

void TabContentModelNG::SetSelectedMode(SelectedMode selectedMode)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetSelectedMode(selectedMode);
}

void TabContentModelNG::SetLabelStyle(const LabelStyle& labelStyle)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetLabelStyle(labelStyle);
}

void TabContentModelNG::SetLabelUnselectedColorByUser(bool isByUser)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, LabelUnselectedColorSetByUser, isByUser);
}

void TabContentModelNG::SetLabelSelectedColorByUser(bool isByUser)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, LabelSelectedColorSetByUser, isByUser);
}

void TabContentModelNG::SetIconStyle(const IconStyle& iconStyle)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetIconStyle(iconStyle);
}

void TabContentModelNG::SetIconUnselectedColorByUser(bool isByUser)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, IconUnselectedColorSetByUser, isByUser);
}

void TabContentModelNG::SetIconSelectedColorByUser(bool isByUser)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TabContentLayoutProperty, IconSelectedColorSetByUser, isByUser);
}

void TabContentModelNG::SetTabBarBadge(FrameNode* node, const std::optional<TabBarBadgeParam>& badgeParam)
{
    CHECK_NULL_VOID(node);
    auto pattern = node->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetTabBarBadge(badgeParam);
}

void TabContentModelNG::SetPadding(const PaddingProperty& padding)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    if (padding.left.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.left.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.right.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.right.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.top.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.top.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.bottom.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.bottom.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    frameNodePattern->SetPadding(padding);
}

void TabContentModelNG::SetUseLocalizedPadding(bool useLocalizedPadding)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetUseLocalizedPadding(useLocalizedPadding);
}

void TabContentModelNG::SetLayoutMode(LayoutMode layoutMode)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetLayoutMode(layoutMode);
}

void TabContentModelNG::SetVerticalAlign(FlexAlign verticalAlign)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetVerticalAlign(verticalAlign);
}

void TabContentModelNG::SetSymmetricExtensible(bool isExtensible)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetSymmetricExtensible(isExtensible);
}

void TabContentModelNG::SetId(const std::string& id)
{
    auto frameNodePattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetId(id);
}

void TabContentModelNG::UpdateLabelStyle(const LabelStyle& labelStyle, RefPtr<TextLayoutProperty> textLayoutProperty)
{
    CHECK_NULL_VOID(textLayoutProperty);

    if (labelStyle.fontSize.has_value()) {
        textLayoutProperty->UpdateFontSize(labelStyle.fontSize.value());
    }
    if (labelStyle.fontWeight.has_value()) {
        textLayoutProperty->UpdateFontWeight(labelStyle.fontWeight.value());
    }
    if (labelStyle.fontStyle.has_value()) {
        textLayoutProperty->UpdateItalicFontStyle(labelStyle.fontStyle.value());
    }
    if (labelStyle.fontFamily.has_value()) {
        textLayoutProperty->UpdateFontFamily(labelStyle.fontFamily.value());
    }
    if (labelStyle.textOverflow.has_value()) {
        textLayoutProperty->UpdateTextOverflow(labelStyle.textOverflow.value());
        if (labelStyle.textOverflow.value() == TextOverflow::MARQUEE) {
            textLayoutProperty->UpdateTextMarqueeStartPolicy(MarqueeStartPolicy::DEFAULT);
        }
    }
    if (labelStyle.maxLines.has_value()) {
        textLayoutProperty->UpdateMaxLines(labelStyle.maxLines.value());
    }
    if (labelStyle.minFontSize.has_value()) {
        textLayoutProperty->UpdateAdaptMinFontSize(labelStyle.minFontSize.value());
    }
    if (labelStyle.maxFontSize.has_value()) {
        textLayoutProperty->UpdateAdaptMaxFontSize(labelStyle.maxFontSize.value());
    }
    if (labelStyle.heightAdaptivePolicy.has_value()) {
        textLayoutProperty->UpdateHeightAdaptivePolicy(labelStyle.heightAdaptivePolicy.value());
    }
}

void TabContentModelNG::UpdateDefaultSymbol(RefPtr<TabTheme>& tabTheme, RefPtr<TextLayoutProperty> symbolProperty)
{
    symbolProperty->UpdateFontSize(tabTheme->GetBottomTabImageSize());
    symbolProperty->UpdateSymbolRenderingStrategy(DEFAULT_RENDERING_STRATEGY);
    UpdateSymbolEffect(symbolProperty, false);
}
void TabContentModelNG::UpdateSymbolEffect(RefPtr<TextLayoutProperty> symbolProperty, bool isActive)
{
    auto symbolEffectOptions = SymbolEffectOptions(SymbolEffectType::BOUNCE);
    symbolEffectOptions.SetIsTxtActive(isActive);
    symbolEffectOptions.SetIsTxtActiveSource(0);
    symbolProperty->UpdateSymbolEffectOptions(symbolEffectOptions);
}

void TabContentModelNG::SetOnWillShow(std::function<void()>&& onWillShow)
{
    auto tabContentNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(tabContentNode);
    auto tabContentEventHub = tabContentNode->GetEventHub<TabContentEventHub>();
    CHECK_NULL_VOID(tabContentEventHub);
    tabContentEventHub->SetOnWillShow(onWillShow);
}

void TabContentModelNG::SetOnWillHide(std::function<void()>&& onWillHide)
{
    auto tabContentNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(tabContentNode);
    auto tabContentEventHub = tabContentNode->GetEventHub<TabContentEventHub>();
    CHECK_NULL_VOID(tabContentEventHub);
    tabContentEventHub->SetOnWillHide(onWillHide);
}

void TabContentModelNG::SetCustomStyleNode(const RefPtr<FrameNode>& customStyleNode)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetCustomStyleNode(customStyleNode);
}

void TabContentModelNG::CreateTabContent()
{
    TabContentModelNG().Create();
}

void TabContentModelNG::CreateTabContent(std::function<void()>&& deepRenderFunc)
{
    TabContentModelNG().Create(std::move(deepRenderFunc));
}

void TabContentModelNG::PopTabContent()
{
    TabContentModelNG().Pop();
}

RefPtr<TabContentPattern> TabContentModelNG::GetTabContentPattern(FrameNode* node)
{
    CHECK_NULL_RETURN(node, nullptr);
    return node->GetPattern<TabContentPattern>();
}

void TabContentModelNG::SetCustomStyleNode(FrameNode* node, const RefPtr<FrameNode>& customStyleNode)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetCustomStyleNode(customStyleNode);
}

void TabContentModelNG::SetTabBar(FrameNode* node, const std::optional<std::string>& text,
    const std::optional<std::string>& icon, const std::optional<TabBarSymbol>& tabBarSymbol,
    TabBarBuilderFunc&& builder, bool /*useContentOnly*/)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, Icon, icon.value_or(""), node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, Text, text.value_or(""), node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetTabBar(text.value_or(""), icon.value_or(""), tabBarSymbol, std::move(builder));
    pattern->SetTabBarWithContent(nullptr);
}

void TabContentModelNG::SetTabBarWithContent(FrameNode* node, const RefPtr<UINode>& content)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetTabBarWithContent(content);
}

void TabContentModelNG::SetTabBarStyle(FrameNode* node, TabBarStyle tabBarStyle)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetTabBarStyle(tabBarStyle);
}

void TabContentModelNG::SetIndicator(FrameNode* node, const IndicatorStyle& indicator)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetIndicatorStyle(indicator);
}

void TabContentModelNG::SetDrawableIndicatorConfig(FrameNode* node, const ImageInfoConfig& config)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetDrawableIndicatorConfig(config);
}

void TabContentModelNG::SetDrawableIndicatorFlag(FrameNode* node, bool isDrawableIndicator)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetDrawableIndicatorFlag(isDrawableIndicator);
}

void TabContentModelNG::SetIndicatorColorByUser(FrameNode* node, bool isByUser)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, IndicatorColorSetByUser, isByUser, node);
}

void TabContentModelNG::SetBoard(FrameNode* node, const BoardStyle& board)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetBoardStyle(board);
}

void TabContentModelNG::SetSelectedMode(FrameNode* node, SelectedMode selectedMode)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedMode(selectedMode);
}

void TabContentModelNG::SetLabelStyle(FrameNode* node, const LabelStyle& labelStyle)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetLabelStyle(labelStyle);
}

void TabContentModelNG::SetLabelUnselectedColorByUser(FrameNode* node, bool isByUser)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, LabelUnselectedColorSetByUser, isByUser, node);
}

void TabContentModelNG::SetLabelSelectedColorByUser(FrameNode* node, bool isByUser)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, LabelSelectedColorSetByUser, isByUser, node);
}

void TabContentModelNG::SetIconStyle(FrameNode* node, const IconStyle& iconStyle)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetIconStyle(iconStyle);
}

void TabContentModelNG::SetIconUnselectedColorByUser(FrameNode* node, bool isByUser)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, IconUnselectedColorSetByUser, isByUser, node);
}

void TabContentModelNG::SetIconSelectedColorByUser(FrameNode* node, bool isByUser)
{
    CHECK_NULL_VOID(node);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(TabContentLayoutProperty, IconSelectedColorSetByUser, isByUser, node);
}

void TabContentModelNG::SetPadding(FrameNode* node, const PaddingProperty& padding)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    if (padding.left.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.left.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.right.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.right.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.top.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.top.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    if (padding.bottom.has_value()) {
        ACE_CHECK_LPX_ATTRIBUTE(padding.bottom.value().GetDimension(), LpxAttribute::ALWAYS);
    }
    pattern->SetPadding(padding);
}

void TabContentModelNG::SetUseLocalizedPadding(FrameNode* node, bool useLocalizedPadding)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetUseLocalizedPadding(useLocalizedPadding);
}

void TabContentModelNG::SetLayoutMode(FrameNode* node, LayoutMode layoutMode)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetLayoutMode(layoutMode);
}

void TabContentModelNG::SetVerticalAlign(FrameNode* node, FlexAlign verticalAlign)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetVerticalAlign(verticalAlign);
}

void TabContentModelNG::SetSymmetricExtensible(FrameNode* node, bool isExtensible)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetSymmetricExtensible(isExtensible);
}

void TabContentModelNG::SetId(FrameNode* node, const std::string& id)
{
    CHECK_NULL_VOID(node);
    auto pattern = GetTabContentPattern(node);
    CHECK_NULL_VOID(pattern);
    pattern->SetId(id);
}

void TabContentModelNG::SetDefaultVisibility(const NG::TabContentDefaultVisibility& defaultVisibility)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<TabContentPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetDefaultVisibility(defaultVisibility);
}

void TabContentModelNG::SetDefaultVisibility(FrameNode* node, const NG::TabContentDefaultVisibility& defaultVisibility)
{
    CHECK_NULL_VOID(node);
    auto pattern = node->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetDefaultVisibility(defaultVisibility);
}

void TabContentModelNG::SetTabBarBuilder(FrameNode* node, TabBarBuilderFunc&& builder)
{
    CHECK_NULL_VOID(node);
    auto frameNodePattern = node->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBar("", "", std::nullopt, std::move(builder));
    frameNodePattern->SetTabBarWithContent(nullptr);
}

void TabContentModelNG::SetTabBarLabel(FrameNode* node, const std::string& label)
{
    CHECK_NULL_VOID(node);
    auto frameNodePattern = node->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(frameNodePattern);
    frameNodePattern->SetTabBar(label, "", std::nullopt, nullptr);
    frameNodePattern->SetTabBarWithContent(nullptr);
}

void TabContentModelNG::SetOnWillShow(FrameNode* tabContentNode, std::function<void()>&& onWillShow)
{
    CHECK_NULL_VOID(tabContentNode);
    auto tabContentEventHub = tabContentNode->GetEventHub<TabContentEventHub>();
    CHECK_NULL_VOID(tabContentEventHub);
    tabContentEventHub->SetOnWillShow(onWillShow);
}
void TabContentModelNG::SetOnWillHide(FrameNode* tabContentNode, std::function<void()>&& onWillHide)
{
    CHECK_NULL_VOID(tabContentNode);
    auto tabContentEventHub = tabContentNode->GetEventHub<TabContentEventHub>();
    CHECK_NULL_VOID(tabContentEventHub);
    tabContentEventHub->SetOnWillHide(onWillHide);
}

void TabContentModelNG::InitTabText(const RefPtr<TextLayoutProperty>& textLayoutProperty)
{
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateEnablePunctuationOverflowOptimize(true);
    if (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWENTY_SIX)) {
        return;
    }
    auto& textStyle = textLayoutProperty->GetTextLineStyle();
    CHECK_NULL_VOID(textStyle);
    textStyle->UpdateOrphanCharOptimization(true);
    auto hostNode = textLayoutProperty->GetHost();
    CHECK_NULL_VOID(hostNode);
    auto context = hostNode->GetContext();
    CHECK_NULL_VOID(context);
    auto fontManager = context->GetFontManager();
    CHECK_NULL_VOID(fontManager);
    if (fontManager->GetFallbackLineSpacingStyleOptimizeFlag()) {
        textLayoutProperty->UpdateIncludeFontPadding(true);
        textLayoutProperty->UpdateFallbackLineSpacing(true);
    }
}
} // namespace OHOS::Ace::NG

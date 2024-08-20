/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PREVIEW

#include "core/components_ng/pattern/select_overlay/service_collaboration_menu_ace_helper.h"

#include "base/geometry/calc_dimension.h"
#include "base/image/pixel_map.h"
#include "base/json/json_util.h"
#include "base/log/ace_scoring_log.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/system_properties.h"
#include "core/common/container.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/placement.h"
#include "core/components/divider/divider_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/pattern/divider/divider_model_ng.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/image/image_model_ng.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_model_ng.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/sub_menu_layout_algorithm.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/option/option_view.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "frameworks/core/pipeline/base/element_register.h"
#include "frameworks/base/thread/cancelable_callback.h"
#include "core/components/common/properties/popup_param.h"
#include "image_source.h"

namespace OHOS::Ace::NG {

void ServiceCollaborationMenuAceHelper::CreateText(
    const std::string& value, const RefPtr<FrameNode>& parent, const Color& color, bool needMargin)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "text is %{public}s", value.c_str());
    auto textPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(textPipeline);
    auto textTheme = textPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(textTheme);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(textNode);
    auto textProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textProperty);
    textProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textProperty->UpdateFontSize(textTheme->GetMenuFontSize());
    textProperty->UpdateFontWeight(FontWeight::REGULAR);
    textProperty->UpdateTextColor(color);
    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateForegroundColor(color);
    textProperty->UpdateContent(value);
    if (needMargin) {
        MarginProperty margin;
        margin.right = CalcLength(static_cast<float>(TEXT_RIGHT_MARGIN));
        margin.left = CalcLength(static_cast<float>(TEXT_LEFT_MARGIN));
        textProperty->UpdateMargin(margin);
    }
    textNode->MountToParent(parent);
    textNode->MarkModifyDone();
}
void ServiceCollaborationMenuAceHelper::CreateHeaderText(const std::string& value, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, text is %{public}s", value.c_str());
    auto textPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(textPipeline);
    auto textTheme = textPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(textTheme);
    auto richTheme = textPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richTheme);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(textNode);
    auto textProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textProperty);
    textProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textProperty->UpdateFontSize(textTheme->GetMenuFontSize());
    textProperty->UpdateFontWeight(FontWeight::REGULAR);
    textProperty->UpdateTextColor(richTheme->GetMenuTitleColor());
    textProperty->UpdateCalcMinSize(
        CalcSize(CalcLength(static_cast<float>(HEADER_MIN_WIDTH)), CalcLength(static_cast<float>(HEADER_MIN_HEIGHT))));
    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateForegroundColor(richTheme->GetMenuTitleColor());
    textProperty->UpdateContent(value);
    MarginProperty margin;
    margin.right = CalcLength(static_cast<float>(HEADER_MARGIN_LEFT));
    margin.left = CalcLength(static_cast<float>(HEADER_MARGIN_RIGHT));
    margin.top = CalcLength(static_cast<float>(HEADER_MARGIN_TOP));
    margin.bottom = CalcLength(static_cast<float>(HEADER_MARGIN_BOTTOM));
    textProperty->UpdateMargin(margin);
    textNode->MountToParent(parent);
    textNode->MarkModifyDone();
}
void ServiceCollaborationMenuAceHelper::CreateEndIcon(const std::string& icon, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, icon is %{public}s", icon.c_str());
    auto iconPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(iconPipeline);
    auto iconTheme = iconPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto richTheme = iconPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richTheme);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(iconNode);
    auto iconProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(iconProperty);
    ImageSourceInfo info(icon);
    info.SetFillColor(richTheme->GetMenuIconColor());
    iconProperty->UpdateImageSourceInfo(info);
    iconProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(iconTheme->GetIconSideLength()), CalcLength(iconTheme->GetIconSideLength())));
    iconProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    MarginProperty margin;
    margin.right = CalcLength(static_cast<float>(ENDICON_MARGIN_RIGHT));
    iconProperty->UpdateMargin(margin);
    iconNode->MountToParent(parent);
    iconNode->MarkModifyDone();
}
void ServiceCollaborationMenuAceHelper::CreateStartIcon(const std::string& icon, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, icon is %{public}s", icon.c_str());
    auto iconPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(iconPipeline);
    auto iconTheme = iconPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto richTheme = iconPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richTheme);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(iconNode);
    auto iconProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(iconProperty);
    ImageSourceInfo info(icon);
    info.SetFillColor(richTheme->GetMenuIconColor());
    iconProperty->UpdateImageSourceInfo(info);
    iconProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(iconTheme->GetIconSideLength()), CalcLength(iconTheme->GetIconSideLength())));
    iconProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    MarginProperty margin;
    margin.right = CalcLength(iconTheme->GetIconContentPadding());
    iconProperty->UpdateMargin(margin);
    iconNode->MountToParent(parent, 0);
    iconNode->MarkModifyDone();
}
RefPtr<FrameNode> ServiceCollaborationMenuAceHelper::CreateMainMenuItem(
    const std::string& value, InternalResource::ResourceId resId, const Color& color)
{
    auto iconPipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(iconPipeline, nullptr);
    auto iconTheme = iconPipeline->GetTheme<IconTheme>();
    CHECK_NULL_RETURN(iconTheme, nullptr);
    auto richTheme = iconPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(richTheme, nullptr);
    auto iconPath = iconTheme ? iconTheme->GetIconPath(resId) : "";
    return CreateMainMenuItem(
        value, iconPath, color == Color::BLACK ? richTheme->GetMenuTextColor() : color);
}
RefPtr<FrameNode> ServiceCollaborationMenuAceHelper::CreateMainMenuItem(
    const std::string& value, const std::string& icon, const Color& color)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter");
    auto menuPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(menuPipeline, nullptr);
    auto menuTheme = menuPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(menuTheme, nullptr);
    auto menuItemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    menuItemPattern->onClickEventSet_ = true;
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), menuItemPattern);
    CHECK_NULL_RETURN(menuItemNode, nullptr);
    auto menuItemProperty = menuItemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    CHECK_NULL_RETURN(menuItemProperty, nullptr);
    menuItemProperty->UpdatePadding({ .right = CalcLength(2.0f), .top = CalcLength(0.0f) });
    auto renderContext = menuItemNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    BorderRadiusProperty border;
    border.SetRadius(menuTheme->GetInnerBorderRadius());
    renderContext->UpdateBorderRadius(border);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(row, nullptr);
    auto rowProperty = row->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(rowProperty, nullptr);
    BorderWidthProperty borderWidth;
    borderWidth.topDimen = Dimension(static_cast<float>(BORDER_WIDTH), DimensionUnit::PX);
    BorderWidthProperty width;
    width.UpdateWithCheck(borderWidth);
    rowProperty->UpdateBorderWidth(width);
    auto rowContext = row->GetRenderContext();
    CHECK_NULL_RETURN(rowContext, nullptr);
    rowContext->UpdateBorderWidth(width);
    BorderColorProperty borderColorProperty;
    borderColorProperty.SetColor(Color::GRAY);
    rowContext->UpdateBorderColor(borderColorProperty);
    rowProperty->UpdateCalcMinSize(
        CalcSize(CalcLength(static_cast<float>(MENUITEM_WIDTH)), CalcLength(static_cast<float>(MENUITEM_HEIGHT))));
    rowProperty->UpdatePadding({.right = CalcLength(0.0f)});
    MarginProperty margin;
    margin.bottom = CalcLength(static_cast<float>(ROW_PADDING));
    rowProperty->UpdateMargin(margin);
    CreateText(value, row, color, true);
    CreateEndIcon(icon, row);
    row->MountToParent(menuItemNode);
    row->MarkModifyDone();
    return menuItemNode;
}
std::string ServiceCollaborationMenuAceHelper::GetIconPath(const std::string& abilityType)
{
    auto iconPipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(iconPipeline, "");
    auto iconTheme = iconPipeline->GetTheme<IconTheme>();
    CHECK_NULL_RETURN(iconTheme, "");
    auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_TAKEPHOTO_SVG) : "";
    TAG_LOGI(AceLogTag::ACE_MENU, "iconPath is %{public}s", iconPath.c_str());
    if (abilityType == "CAMERA") {
        return "resource:///ohos_ic_public_camera.svg";
    }
    if (abilityType == "SCAN") {
        return "resource:///ohos_ic_public_scan.svg";
    }
    if (abilityType == "IMAGE_PICKER") {
        return "resource:///ohos_ic_public_albums.svg";
    }
    return "";
}
RefPtr<FrameNode> ServiceCollaborationMenuAceHelper::CreateDeviceMenuItem(
    const std::string& value, const std::string& icon)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter icon is %{public}s, value is %{public}s", icon.c_str(), value.c_str());
    auto menuPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(menuPipeline, nullptr);
    auto menuTheme = menuPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(menuTheme, nullptr);
    auto richTheme = menuPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(richTheme, nullptr);
    auto menuItemNode = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuItemPattern>());
    CHECK_NULL_RETURN(menuItemNode, nullptr);
    auto renderContext = menuItemNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    BorderRadiusProperty border;
    border.SetRadius(menuTheme->GetInnerBorderRadius());
    renderContext->UpdateBorderRadius(border);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(row, nullptr);
    auto rowProperty = row->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(rowProperty, nullptr);
    MarginProperty margin;
    margin.top = CalcLength(static_cast<float>(MENUITEM_MARGIN));
    margin.bottom = CalcLength(static_cast<float>(MENUITEM_MARGIN));
    auto size = CalcSize();
    size.SetHeight(CalcLength(static_cast<float>(MENUITEM_HEIGHT)));
    rowProperty->UpdateMargin(margin);
    rowProperty->UpdateUserDefinedIdealSize(size);
    CreateStartIcon(icon, row);
    CreateText(value, row, richTheme->GetMenuTextColor(), false);
    row->MountToParent(menuItemNode);
    row->MarkModifyDone();
    return menuItemNode;
}
void ServiceCollaborationMenuAceHelper::RemoveSubmenu(const RefPtr<FrameNode>& menuWrapper)
{
    CHECK_NULL_VOID(!mainMenuIsHover_);
    CHECK_NULL_VOID(menuWrapper);
    auto childrenList = menuWrapper->GetChildren();
    for (const auto& children : childrenList) {
        if (children == nullptr) {
            continue;
        }
        auto frameNode = AceType::DynamicCast<FrameNode>(children);
        auto pattern = frameNode->GetPattern<MenuPattern>();
        if (!pattern) {
            continue;
        }
        if (pattern && pattern->GetTargetTag() == SUN_MENU_TAG) {
            menuWrapper->RemoveChild(children);
            subMenuIsShow_ = false;
        }
    }
}
    void ServiceCollaborationMenuAceHelper::SubMenuDown(const RefPtr<FrameNode>& menuWrapper)
{
    CHECK_NULL_VOID(menuWrapper);
    auto childrenList = menuWrapper->GetChildren();
    for (const auto& children : childrenList) {
        if (!children) {
            continue;
        }
        auto frameNode = AceType::DynamicCast<FrameNode>(children);
        if (!frameNode) {
            continue;
        }
        auto pattern = frameNode->GetPattern<MenuPattern>();
        if (!pattern) {
            continue;
        }
        if (pattern && pattern->GetTargetTag() == SUN_MENU_TAG) {
            menuWrapper->RemoveChild(children);
            subMenuIsShow_ = false;
        }
    }
}
RefPtr<FrameNode> ServiceCollaborationMenuAceHelper::CreateMenuItemGroupNode(
    uint32_t index, const std::string& deviceName)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter");
    auto menuItemGroupNode = FrameNode::GetOrCreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<MenuItemGroupPattern>();});
    CHECK_NULL_RETURN(menuItemGroupNode, nullptr);
    TAG_LOGI(AceLogTag::ACE_MENU, "DEVICE NAME IS %{public}s", deviceName.c_str());
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(row, nullptr);
    auto rowProperty = row->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(rowProperty, nullptr);
    MarginProperty margin;
    margin.right = CalcLength(static_cast<float>(GROUP_MARGIN));
    margin.left = CalcLength(static_cast<float>(GROUP_MARGIN));
    rowProperty->UpdateCalcMinSize(
        CalcSize(CalcLength(static_cast<float>(GROUP_MIN_WIDTH)), std::nullopt));
    if (index > 0) {
        margin.top = CalcLength(static_cast<float>(BORDER_MARGIN_TOP));
        BorderWidthProperty borderWidth;
        borderWidth.topDimen = Dimension(static_cast<float>(BORDER_WIDTH), DimensionUnit::VP);
        BorderWidthProperty borderWidth1;
        borderWidth1.UpdateWithCheck(borderWidth);
        rowProperty->UpdateBorderWidth(borderWidth1);
        auto context = row->GetRenderContext();
        CHECK_NULL_RETURN(context, nullptr);
        context->UpdateBorderWidth(borderWidth1);
        BorderColorProperty borderColorProperty;
        borderColorProperty.SetColor(Color(BORDER_COLOR));
        context->UpdateBorderColor(borderColorProperty);
    }
    rowProperty->UpdateMargin(margin);
    CreateHeaderText(deviceName, row);
    auto focusHub = row->GetFocusHub();
    focusHub->SetEnabled(false);
    row->MountToParent(menuItemGroupNode);
    return menuItemGroupNode;
}


RefPtr<FrameNode> ServiceCollaborationMenuAceHelper::CreateSubDeviceMenuOnCol(
    const RefPtr<FrameNode>& column, const RefPtr<FrameNode>& menuWrapper)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "CreateSubDeviceMenuOnCol");
    MenuParam param;
    param.type = MenuType::SUB_MENU;
    param.isShowInSubWindow = false;
    auto subMenu = MenuView::Create(column, SUB_MENU_ID, SUN_MENU_TAG, param);
    auto inputHub = subMenu->GetOrCreateInputEventHub();
    CHECK_NULL_RETURN(inputHub, nullptr);
    auto mouseTask = [this, weak = AceType::WeakClaim(AceType::RawPtr(menuWrapper))](bool isHover) {
        TAG_LOGI(AceLogTag::ACE_MENU, "mouseTask, isHover=%{public}d", isHover);
        auto menuWrapper = weak.Upgrade();
        if (isHover) {
            subMenuIsHover_ = true;
            return;
        }
        subMenuIsHover_ = false;
        ContainerScope scope(Container::CurrentIdSafely());
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto cancelableCallback = [this, weak2 = AceType::WeakClaim(AceType::RawPtr(menuWrapper)),
                                   instanceId = Container::CurrentIdSafely()] {
            auto menuWrapper = weak2.Upgrade();
            ContainerScope scope(instanceId);
            RemoveSubmenu(menuWrapper);
        };
        auto taskExecutor = context->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostDelayedTask(
            cancelableCallback, TaskExecutor::TaskType::UI, 100, "ArkUIRichEditorRemoveMenuNode");
    };
    auto mouseEvent = AceType::MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnHoverEvent(mouseEvent);
    return subMenu;
}

void ServiceCollaborationMenuAceHelper::SubMeunMountToMainMenu(
    const RefPtr<FrameNode>& menuNode, const RefPtr<FrameNode>& menuWrapper,
    std::function<RefPtr<FrameNode>(void)> subDeviceMenuCreator)
{
    auto inputHub = menuNode->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    auto mouseTask = [this, &menuNode, wrapper = AceType::WeakClaim(AceType::RawPtr(menuWrapper)),
        subDeviceMenuCreator, node = AceType::WeakClaim(AceType::RawPtr(menuNode))](bool isHover) {
        auto menuItemNode = node.Upgrade();
        auto menuWrapper = wrapper.Upgrade();
        if (isHover) {
            mainMenuIsHover_ = true;
            if (!subMenuIsShow_) {
                TAG_LOGI(AceLogTag::ACE_MENU, "create SubMenu enter.1");
                auto subMenu = subDeviceMenuCreator();
                TAG_LOGI(AceLogTag::ACE_MENU, "create SubMenu enter.2");
                auto submenuPattern = subMenu->GetPattern<MenuPattern>();
                submenuPattern->SetParentMenuItem(menuItemNode);
                subMenu->MountToParent(menuWrapper);
                auto menuProps = subMenu->GetLayoutProperty<MenuLayoutProperty>();
                OffsetF position;
                auto frameSize = menuItemNode->GetGeometryNode()->GetMarginFrameSize();
                position = menuItemNode->GetPaintRectOffset() + OffsetF(frameSize.Width(), 0.0);
                menuProps->UpdateMenuOffset(position);
                subMenu->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
                subMenuIsShow_ = true;
            }
            return;
        }
        TAG_LOGI(AceLogTag::ACE_MENU, "remove SubMenu enter.");
        mainMenuIsHover_ = false;
        // timeout 100ms to RemoveChild
        ContainerScope scope(Container::CurrentIdSafely());
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto cancelableCallback = [this, weak = AceType::WeakClaim(AceType::RawPtr(menuWrapper)),
                instanceId = Container::CurrentIdSafely()] {
            auto menuWrapper = weak.Upgrade();
            ContainerScope scope(instanceId);
            if (!subMenuIsHover_) {
                SubMenuDown(menuWrapper);
            }
        };
        auto taskExecutor = context->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostDelayedTask(
            cancelableCallback, TaskExecutor::TaskType::UI, 100, "ArkUIRichEditorRemoveMenuNode");
    };
    auto mouseEvent = AceType::MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnHoverEvent(mouseEvent);
}

// Callback

void ServiceCollaborationAceCallback::CreateText(const std::string& value, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, text is %{public}s", value.c_str());
    auto textPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(textPipeline);
    auto textTheme = textPipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(textTheme);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(textNode);
    auto textProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textProperty);
    textProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textProperty->UpdateFontSize(Dimension(static_cast<double>(TEXT_FONT_SIZE), DimensionUnit::VP));
    textProperty->UpdateFontWeight(FontWeight::REGULAR);
    textProperty->UpdateTextColor(textTheme->GetMenuFontColor());
    textProperty->UpdateCalcMaxSize(
        CalcSize(CalcLength(static_cast<float>(TEXT_MAX_WIDTH)), CalcLength(static_cast<float>(TEXT_MAX_HIGHT))));
    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateForegroundColor(textTheme->GetMenuFontColor());
    textProperty->UpdateContent(value);
    MarginProperty margin;
    margin.top = CalcLength(static_cast<float>(TEXT_MARGIN_TOP));
    margin.bottom = CalcLength(static_cast<float>(TEXT_MARGIN_BOTTOM));
    textProperty->UpdateMargin(margin);
    textNode->MountToParent(parent);
    textNode->MarkModifyDone();
}

void ServiceCollaborationAceCallback::CreateEndIcon(const std::string& icon, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, icon is %{public}s", icon.c_str());
    auto iconPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(iconPipeline);
    auto iconTheme = iconPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(iconNode);
    auto iconProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(iconProperty);
    ImageSourceInfo info(icon);
    info.SetFillColor(iconTheme->GetPopIconColor());
    iconProperty->UpdateImageSourceInfo(info);
    iconProperty->UpdateUserDefinedIdealSize(
        CalcSize(
            CalcLength(static_cast<float>(ENDICON_SIZE), DimensionUnit::VP),
            CalcLength(static_cast<float>(ENDICON_SIZE), DimensionUnit::VP)
        )
    );
    iconProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    MarginProperty margin;
    margin.right = CalcLength(static_cast<float>(ENDICON_MARGIN));
    margin.left = CalcLength(static_cast<float>(ENDICON_MARGIN));
    margin.top = CalcLength(static_cast<float>(ENDICON_MARGIN_TOP));
    margin.bottom = CalcLength(static_cast<float>(ENDICON_MARGIN));
    iconProperty->UpdateMargin(margin);
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>([this](GestureEvent& callback) {
        ContainerScope scope(Container::CurrentIdSafely());
        RemovePopupNode();
    });
    auto gestureHub = iconNode->GetOrCreateGestureEventHub();
    gestureHub->AddClickEvent(clickEvent);
    iconNode->MountToParent(parent);
    iconNode->MarkModifyDone();
}

void ServiceCollaborationAceCallback::CreateStartIcon(
    const std::string& icon, const RefPtr<FrameNode>& parent)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "enter, icon is %{public}s", icon.c_str());
    auto iconPipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(iconPipeline);
    auto iconTheme = iconPipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(iconNode);
    auto iconProperty = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(iconProperty);
    ImageSourceInfo info(icon);
    info.SetFillColor(iconTheme->GetPopIconColor());
    iconProperty->UpdateImageSourceInfo(info);
    iconProperty->UpdateUserDefinedIdealSize(
        CalcSize(
            CalcLength(static_cast<float>(STARTICON_SIZE), DimensionUnit::VP),
            CalcLength(static_cast<float>(STARTICON_SIZE), DimensionUnit::VP)
        )
    );
    iconProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    MarginProperty margin;
    margin.right = CalcLength(static_cast<float>(STARTICON_MARGIN));
    margin.left = CalcLength(static_cast<float>(STARTICON_MARGIN));
    margin.top = CalcLength(static_cast<float>(STARTICON_MARGIN));
    margin.bottom = CalcLength(static_cast<float>(STARTICON_MARGIN));
    iconProperty->UpdateMargin(margin);
    iconNode->MountToParent(parent, 0);
    iconNode->MarkModifyDone();
}

RefPtr<FrameNode> ServiceCollaborationAceCallback::CreateCustomPopUpNode(
    const std::string& value, const std::string& icon)
{
    if (!row_) {
        auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        CHECK_NULL_RETURN(row, nullptr);
        auto rowProperty = row->GetLayoutProperty<LinearLayoutProperty>();
        CHECK_NULL_RETURN(rowProperty, nullptr);
        if (ability_ == "CAMERA") {
            CreateStartIcon("resource:///ohos_ic_public_camera.svg", row);
        }
        if (ability_ == "SCAN") {
            CreateStartIcon("resource:///ohos_ic_public_scan.svg", row);
        }
        if (ability_ == "IMAGE_PICKER") {
            CreateStartIcon("resource:///ohos_ic_public_albums.svg", row);
        }
        CreateText(value, row);
        CreateEndIcon("resource:///ohos_ic_public_cancel.svg", row);
        row_ = row;
        row_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
        return row_;
    }
    auto childrenList = row_->GetChildren();
    for (const auto& children : childrenList) {
        auto frameNode = AceType::DynamicCast<FrameNode>(children);
        if (frameNode && frameNode->GetTag() == V2::TEXT_ETS_TAG) {
            auto textProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
            textProperty->UpdateContent(value);
        }
    }
    row_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    return row_;
}

void ServiceCollaborationAceCallback::RemovePopupNode()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto overlay = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(overlay);
    CHECK_NULL_VOID(info_);
    CHECK_NULL_VOID(info_->pattern.Upgrade());
    CHECK_NULL_VOID(info_->pattern.Upgrade()->GetHost());
    auto pattern = AceType::DynamicCast<RichEditorPattern>(info_->pattern.Upgrade());
    CHECK_NULL_VOID(pattern);
    pattern->RegisterCaretChangeListener(nullptr);
    auto targetId = info_->pattern.Upgrade()->GetHost()->GetId();
    auto popupInfo = overlay->GetPopupInfo(targetId);
    popupInfo.markNeedUpdate = true;
    overlay->HidePopup(targetId, popupInfo);
    CHECK_NULL_VOID(menuHelper_);
    menuHelper_->photoCount_ = 0;
}

RefPtr<PopupParam> ServiceCollaborationAceCallback::GetPopupParam(bool isShow, StateChangeFunc& onStateChange)
{
    auto popupParam = AceType::MakeRefPtr<PopupParam>();
    ViewStackModel::GetInstance()->NewScope();
    popupParam->SetIsShow(isShow);
    Offset offset = { position_.GetX(), position_.GetY() };
    popupParam->SetTargetOffset(offset);
    popupParam->SetPlacement(Placement::TOP);
    popupParam->SetEnableArrow(true);
    popupParam->SetUseCustomComponent(true);
    popupParam->SetBackgroundColor(Color::WHITE);
    popupParam->SetTargetSpace(Dimension(static_cast<float>(TARGET_SPACE), DimensionUnit::VP));
    popupParam->SetOnStateChange(std::move(onStateChange));
    Shadow shadow;
    auto colorMode = SystemProperties::GetColorMode();
    auto container = Container::Current();
    auto pipelineContext = container->GetPipelineContext();
    auto shadowTheme = pipelineContext->GetTheme<ShadowTheme>();
    shadow =  shadowTheme->GetShadow(ShadowStyle::OuterDefaultSM, colorMode);
    popupParam->SetShadow(shadow);
    return popupParam;
}

int32_t ServiceCollaborationAceCallback::OnEvent(uint32_t code, uint32_t eventId, std::string &category,
    StateChangeFunc& onStateChange_)
{
    TAG_LOGI(AceLogTag::ACE_MENU, "code is %{public}d, msg is %{public}s.", code, category.c_str());
    ContainerScope scope(Container::CurrentIdSafely());
    CHECK_NULL_RETURN(info_, -1);
    if (code == PROCES_PREPARE) {
        position_ = AceType::DynamicCast<RichEditorPattern>(info_->pattern.Upgrade())->GetCaretRect().GetOffset();
        auto popupParam = GetPopupParam(true, onStateChange_);
        auto pattern = AceType::DynamicCast<RichEditorPattern>(info_->pattern.Upgrade());
        std::function<void(int32_t)> func = [this](int32_t num) {
            if (!isTransmit_) {
                RemovePopupNode();
                info_ = nullptr;
            }
        };
        pattern->RegisterCaretChangeListener(std::move(func));
        auto row = CreateCustomPopUpNode(category, "");
        CHECK_NULL_RETURN(row, -1);
        ViewAbstract::BindPopup(popupParam, info_->pattern.Upgrade()->GetHost(), row);
        return 0;
    }
    if (code == PHOTO_SENDING) {
        auto popupParam = GetPopupParam(true, onStateChange_);
        auto row = CreateCustomPopUpNode(category, "");
        ViewAbstract::BindPopup(popupParam, info_->pattern.Upgrade()->GetHost(), row);
        return 0;
    }
    if (code == MULTI_PHOTO_SENDING_BACK) {
        auto popupParam = GetPopupParam(true, onStateChange_);
        auto row = CreateCustomPopUpNode(category, "");
        ViewAbstract::BindPopup(popupParam, info_->pattern.Upgrade()->GetHost(), row);
        return 0;
    }
    if (code == REMOTE_CANCEL) {
        RemovePopupNode();
        info_ = nullptr;
        return 0;
    }
    RemovePopupNode();
    auto toastPipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(toastPipeline, -1);
    auto overlay = toastPipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlay, -1);
    overlay->ShowToast({ category, 2000, "", true }, nullptr);
    info_ = nullptr;
    return 0;
}

RefPtr<PixelMap> ServiceCollaborationAceCallback::CreatePixelMap(void *buffer, uint32_t code, uint32_t dataLength)
{
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource((uint8_t *)buffer, dataLength, opts, errorCode);
    if (errorCode != 0) {
        TAG_LOGE(AceLogTag::ACE_MENU, "CreateImageSource from buffer failed.");
        return nullptr;
    }
    Media::DecodeOptions decodeOpts;
    std::shared_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        TAG_LOGE(AceLogTag::ACE_MENU, "CreatePixelMap failed.");
        return nullptr;
    }
    CHECK_NULL_RETURN(pixelMap, nullptr);
    auto imagePix = PixelMap::CreatePixelMap(reinterpret_cast<void*>(&pixelMap));
    return imagePix;
}

int32_t ServiceCollaborationAceCallback::OnDataCallback(uint32_t code, uint32_t collaborationId,
    uint32_t dataLength, std::unique_ptr<char[]>& data)
{
    CHECK_NULL_RETURN(menuHelper_, -1);
    auto& photoCount = menuHelper_->photoCount_;
    ContainerScope scope(Container::CurrentIdSafely());
    CHECK_NULL_RETURN(info_, -1);
    isTransmit_ = true;
    TAG_LOGI(AceLogTag::ACE_MENU, "code is %{public}d, dataLength is %{public}d.", code, dataLength);
    auto imagePix = CreatePixelMap((void *)data.get(), code, dataLength);
    CHECK_NULL_RETURN(imagePix, -1);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, -1);
    CancelableCallback<void()> caretTwinklingTask;
    caretTwinklingTask.Reset([this, code, weak = imagePix, instanceId = Container::CurrentIdSafely(), &photoCount] {
        CHECK_NULL_VOID(weak);
        CHECK_NULL_VOID(info_);
        auto richEditorPattern = AceType::DynamicCast<RichEditorPattern>(info_->pattern.Upgrade());
        CHECK_NULL_VOID(richEditorPattern);
        ContainerScope scope(instanceId);
        ImageSpanOptions options;
        options.imagePixelMap = weak;
        options.offset = richEditorPattern->GetCaretPosition() + photoCount;
        auto width = weak->GetWidth();
        auto height = weak->GetHeight();
        photoCount++;
        ImageSpanAttribute attr = {
            .size = ImageSpanSize{ .width = CalcDimension(width), .height = CalcDimension(height) } };
        options.imageAttribute = attr;
        if (!info_->pattern.Upgrade()) {
            TAG_LOGE(AceLogTag::ACE_MENU, "info_->pattern.Upgrade() is nullptr.");
            return;
        }
        if (!richEditorPattern->GetTextSelector().SelectNothing()) {
            richEditorPattern->DeleteBackward(1);
        }
        richEditorPattern->AddImageSpan(options, false, 0, false);
        if (code == SEND_PHOTO_SUCCESS) {
            richEditorPattern->SetCaretPosition(richEditorPattern->GetCaretPosition() + photoCount);
            RemovePopupNode();
            isTransmit_ = false;
            info_ = nullptr;
        }
    });
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, -1);
    taskExecutor->PostTask(caretTwinklingTask, TaskExecutor::TaskType::UI, "ArkUIRichEditorAddImageSpan");
    return 0;
}
} // namespace OHOS::Ace::NG
#endif
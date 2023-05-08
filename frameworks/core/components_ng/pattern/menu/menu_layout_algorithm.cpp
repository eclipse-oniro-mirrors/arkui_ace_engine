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

#include "core/components_ng/pattern/menu/menu_layout_algorithm.h"

#include <optional>
#include <vector>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/placement.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
constexpr uint32_t MIN_GRID_COUNTS = 2;
constexpr uint32_t GRID_COUNTS_4 = 4;
constexpr uint32_t GRID_COUNTS_6 = 6;
constexpr uint32_t GRID_COUNTS_8 = 8;
constexpr uint32_t GRID_COUNTS_12 = 12;

uint32_t GetMaxGridCounts(const RefPtr<GridColumnInfo>& columnInfo)
{
    CHECK_NULL_RETURN(columnInfo, GRID_COUNTS_8);
    auto currentColumns = columnInfo->GetParent()->GetColumns();
    auto maxGridCounts = GRID_COUNTS_8;
    switch (currentColumns) {
        case GRID_COUNTS_4:
            maxGridCounts = GRID_COUNTS_4;
            break;
        case GRID_COUNTS_8:
            maxGridCounts = GRID_COUNTS_6;
            break;
        case GRID_COUNTS_12:
            maxGridCounts = GRID_COUNTS_8;
            break;
        default:
            break;
    }
    return maxGridCounts;
}
} // namespace

MenuLayoutAlgorithm::MenuLayoutAlgorithm(int32_t id, const std::string& tag) : targetNodeId_(id), targetTag_(tag)
{
    placementFuncMap_[Placement::TOP] = &MenuLayoutAlgorithm::GetPositionWithPlacementTop;
    placementFuncMap_[Placement::TOP_LEFT] = &MenuLayoutAlgorithm::GetPositionWithPlacementTopLeft;
    placementFuncMap_[Placement::TOP_RIGHT] = &MenuLayoutAlgorithm::GetPositionWithPlacementTopRight;
    placementFuncMap_[Placement::BOTTOM] = &MenuLayoutAlgorithm::GetPositionWithPlacementBottom;
    placementFuncMap_[Placement::BOTTOM_LEFT] = &MenuLayoutAlgorithm::GetPositionWithPlacementBottomLeft;
    placementFuncMap_[Placement::BOTTOM_RIGHT] = &MenuLayoutAlgorithm::GetPositionWithPlacementBottomRight;
    placementFuncMap_[Placement::LEFT] = &MenuLayoutAlgorithm::GetPositionWithPlacementLeft;
    placementFuncMap_[Placement::LEFT_TOP] = &MenuLayoutAlgorithm::GetPositionWithPlacementLeftTop;
    placementFuncMap_[Placement::LEFT_BOTTOM] = &MenuLayoutAlgorithm::GetPositionWithPlacementLeftBottom;
    placementFuncMap_[Placement::RIGHT] = &MenuLayoutAlgorithm::GetPositionWithPlacementRight;
    placementFuncMap_[Placement::RIGHT_TOP] = &MenuLayoutAlgorithm::GetPositionWithPlacementRightTop;
    placementFuncMap_[Placement::RIGHT_BOTTOM] = &MenuLayoutAlgorithm::GetPositionWithPlacementRightBottom;
}

MenuLayoutAlgorithm::~MenuLayoutAlgorithm()
{
    placementFuncMap_.clear();
}

void MenuLayoutAlgorithm::Initialize(FrameNode* frameNode)
{
    // currently using click point as menu position
    auto props = AceType::DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(props);
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    auto targetSize = props->GetTargetSizeValue(SizeF());
    position_ = props->GetMenuOffset().value_or(OffsetF());
    positionOffset_ = props->GetPositionOffset().value_or(OffsetF());
    LOGD("menu position_ = %{public}s, targetSize = %{public}s", position_.ToString().c_str(),
        targetSize.ToString().c_str());

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    if (!menuPattern->IsSelectOverlayExtensionMenu()) {
        margin_ = static_cast<float>(theme->GetOutPadding().ConvertToPx());
        optionPadding_ = margin_;
    } else {
        optionPadding_ = static_cast<float>(theme->GetOutPadding().ConvertToPx());
    }

    auto constraint = props->GetLayoutConstraint();
    auto wrapperIdealSize =
        CreateIdealSize(constraint.value(), Axis::FREE, props->GetMeasureType(MeasureType::MATCH_PARENT), true);
    wrapperSize_ = wrapperIdealSize;
    if (menuPattern->IsSelectOverlayExtensionMenu()) {
        topSpace_ = 0.0f;
        bottomSpace_ = constraint->maxSize.Height() - position_.GetY();
        leftSpace_ = Infinity<float>();
    } else {
        topSpace_ = position_.GetY() - targetSize.Height() - margin_ * 2.0f;
        bottomSpace_ = wrapperSize_.Height() - position_.GetY() - margin_ * 2.0f;
        leftSpace_ = position_.GetX();
        rightSpace_ = wrapperSize_.Width() - leftSpace_;
    }

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto stageManager = context->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto page = stageManager->GetLastPage();
    CHECK_NULL_VOID(page);
    pageOffset_ = page->GetOffsetRelativeToWindow();
    topSpace_ -= pageOffset_.GetY();
    leftSpace_ -= pageOffset_.GetX();
    placement_ = props->GetMenuPlacement().value_or(Placement::BOTTOM);
}

// Called to perform layout render node and child.
void MenuLayoutAlgorithm::Measure(FrameNode* frameNode)
{
    // initialize screen size and menu position
    Initialize(frameNode);

    auto menuLayoutProperty = AceType::DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(menuLayoutProperty);
    const auto& constraint = menuLayoutProperty->GetLayoutConstraint();
    if (!constraint) {
        LOGE("fail to measure menu due to layoutConstraint is nullptr");
        return;
    }
    auto idealSize = CreateIdealSize(
        constraint.value(), Axis::VERTICAL, menuLayoutProperty->GetMeasureType(MeasureType::MATCH_CONTENT), true);
    const auto& padding = menuLayoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, idealSize);

    // calculate menu main size
    auto childConstraint = CreateChildConstraint(frameNode);
    float idealHeight = 0.0f;
    float idealWidth = 0.0f;
    for (const auto& child : frameNode->GetAllFrameNodeChildren()) {
        child->Measure(childConstraint);
        auto childSize = child->GetGeometryNode()->GetMarginFrameSize();
        LOGD("child finish measure, child %{public}s size = %{public}s", child->GetTag().c_str(),
            child->GetGeometryNode()->GetMarginFrameSize().ToString().c_str());
        idealHeight += childSize.Height();
        idealWidth = std::max(idealWidth, childSize.Width());
    }
    idealSize.SetHeight(idealHeight);
    idealSize.SetWidth(idealWidth);
    AddPaddingToSize(padding, idealSize);

    auto geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    LOGD("finish measure, menu size = %{public}f x %{public}f", idealSize.Width(), idealSize.Height());
    geometryNode->SetFrameSize(idealSize);
}

void MenuLayoutAlgorithm::Layout(FrameNode* frameNode)
{
    auto menuProp = DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(menuProp);
    if (!targetTag_.empty()) {
        InitTargetSizeAndPosition(menuProp);
    }

    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    if (menuPattern->IsSubMenu()) {
        LayoutSubMenu(frameNode);
        return;
    }

    auto geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto size = geometryNode->GetMarginFrameSize();
    if (menuPattern->IsSelectMenu()) {
        ComputeMenuPositionByAlignType(menuProp, size);
    }

    auto menuPosition = MenuLayoutAvoidAlgorithm(menuProp, menuPattern, size);
    if (menuPattern->IsSelectMenu()) {
        auto offset = ComputeMenuPositionByOffset(menuProp, geometryNode);
        menuPosition += offset;
        position_ = menuPosition;
        menuPosition = MenuLayoutAvoidAlgorithm(menuProp, menuPattern, size);
    }

    LOGD("Menu layout, offset = %{public}s", menuPosition.ToString().c_str());
    geometryNode->SetFrameOffset(menuPosition);

    // translate each option by the height of previous options
    OffsetF translate;
    for (const auto& child : frameNode->GetAllFrameNodeChildren()) {
        LOGD("layout child at offset: %{public}s, tag %{public}s", translate.ToString().c_str(),
            child->GetTag().c_str());
        child->GetGeometryNode()->SetMarginFrameOffset(translate);
        child->Layout();
        translate += OffsetF(0, child->GetGeometryNode()->GetFrameSize().Height());
    }

    if (menuPattern->IsContextMenu()) {
        std::vector<Rect> rects;
        auto frameRect = frameNode->GetGeometryNode()->GetFrameRect();
        auto rect = Rect(frameRect.GetX(), frameRect.GetY(), frameRect.Width(), frameRect.Height());
        rects.emplace_back(rect);
        SubwindowManager::GetInstance()->SetHotAreas(rects);
    }
}

void MenuLayoutAlgorithm::ComputeMenuPositionByAlignType(
    const RefPtr<MenuLayoutProperty>& menuProp, const SizeF& menuSize)
{
    auto alignType = menuProp->GetAlignType().value_or(MenuAlignType::START);
    auto targetSize = menuProp->GetTargetSizeValue(SizeF());
    switch (alignType) {
        case MenuAlignType::CENTER: {
            position_.AddX(targetSize.Width() / 2.0f - menuSize.Width() / 2.0f);
            break;
        }
        case MenuAlignType::END: {
            position_.AddX(targetSize.Width() - menuSize.Width());
            break;
        }
        default:
            break;
    }
}

OffsetF MenuLayoutAlgorithm::ComputeMenuPositionByOffset(
    const RefPtr<MenuLayoutProperty>& menuProp, const RefPtr<GeometryNode>& geometryNode)
{
    CHECK_NULL_RETURN(menuProp, OffsetF(0, 0));
    CHECK_NULL_RETURN(geometryNode, OffsetF(0, 0));

    const auto& layoutConstraint = menuProp->GetLayoutConstraint();
    CHECK_NULL_RETURN(layoutConstraint, OffsetF(0, 0));
    auto menuAlignOffset = menuProp->GetOffset().value_or(
        DimensionOffset(Dimension(0, DimensionUnit::VP), Dimension(0, DimensionUnit::VP)));

    auto menuSize = geometryNode->GetFrameSize();
    auto menuTrimOffsetX =
        ConvertToPx(CalcLength(menuAlignOffset.GetX()), layoutConstraint->scaleProperty, menuSize.Width());
    auto menuTrimOffsetY =
        ConvertToPx(CalcLength(menuAlignOffset.GetY()), layoutConstraint->scaleProperty, menuSize.Height());
    OffsetF menuTrimOffset = OffsetF(menuTrimOffsetX.value_or(0.0), menuTrimOffsetY.value_or(0.0));
    return menuTrimOffset;
}

OffsetF MenuLayoutAlgorithm::MenuLayoutAvoidAlgorithm(
    const RefPtr<MenuLayoutProperty>& menuProp, const RefPtr<MenuPattern>& menuPattern, const SizeF& size)
{
    CHECK_NULL_RETURN(menuProp, OffsetF(0, 0));
    CHECK_NULL_RETURN(menuPattern, OffsetF(0, 0));
    float x = 0.0f;
    float y = 0.0f;
    float windowsOffsetX = 0.0f;
    float windowsOffsetY = 0.0f;
    if (menuProp->GetMenuPlacement().has_value() && (targetSize_.Width() > 0.0 || targetSize_.Height() > 0.0)) {
        auto childOffset = GetChildPosition(size, menuProp);
        x = childOffset.GetX() + positionOffset_.GetX();
        y = childOffset.GetY() + positionOffset_.GetY();
        auto pipelineContext = PipelineContext::GetCurrentContext();
        if (pipelineContext) {
            auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
            windowsOffsetX = windowGlobalRect.GetOffset().GetX();
            windowsOffsetY = windowGlobalRect.GetOffset().GetY();
            x += windowsOffsetX;
            y += windowsOffsetY;
        }
    } else {
        x = HorizontalLayout(size, position_.GetX(), menuPattern->IsSelectMenu()) + positionOffset_.GetX();
        y = VerticalLayout(size, position_.GetY()) + positionOffset_.GetY();
        if (!menuPattern->IsContextMenu()) {
            x -= pageOffset_.GetX();
            y -= pageOffset_.GetY();
        }
    }
    x = std::clamp(x, windowsOffsetX, wrapperSize_.Width() - size.Width() - margin_ * 2.0f + windowsOffsetX);
    y = std::clamp(y, windowsOffsetY, wrapperSize_.Height() - size.Height() - margin_ * 2.0f + windowsOffsetY);

    return OffsetF(x, y);
}

void MenuLayoutAlgorithm::UpdateConstraintWidth(FrameNode* frameNode, LayoutConstraintF& constraint)
{
    // set min width
    RefPtr<GridColumnInfo> columnInfo;
    columnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::MENU);
    columnInfo->GetParent()->BuildColumnWidth();
    float minWidth = static_cast<float>(columnInfo->GetWidth(MIN_GRID_COUNTS));
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    if (menuPattern->IsSelectOverlayExtensionMenu() && minWidth > constraint.maxSize.Width()) {
        minWidth = constraint.maxSize.Width();
    }
    constraint.minSize.SetWidth(minWidth);

    // set max width
    auto menuLayoutProperty = AceType::DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(menuLayoutProperty);
    const auto& padding = menuLayoutProperty->CreatePaddingAndBorder();
    auto maxHorizontalSpace = std::max(leftSpace_, rightSpace_) - 2.0f * padding.Width();
    auto maxGridWidth = static_cast<float>(columnInfo->GetWidth(GetMaxGridCounts(columnInfo)));
    auto maxWidth = std::min(maxHorizontalSpace, maxGridWidth);
    maxWidth = std::min(constraint.maxSize.Width(), maxWidth);
    constraint.maxSize.SetWidth(maxWidth);
    constraint.percentReference.SetWidth(maxWidth);
}

void MenuLayoutAlgorithm::UpdateConstraintHeight(FrameNode* frameNode, LayoutConstraintF& constraint)
{
    auto maxSpaceHeight = std::max(topSpace_, bottomSpace_);
    constraint.maxSize.SetHeight(maxSpaceHeight);
    constraint.percentReference.SetHeight(maxSpaceHeight);
}

LayoutConstraintF MenuLayoutAlgorithm::CreateChildConstraint(FrameNode* frameNode)
{
    auto menuLayoutProperty = AceType::DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_RETURN(menuLayoutProperty, LayoutConstraintF());

    auto childConstraint = menuLayoutProperty->CreateChildConstraint();
    UpdateConstraintWidth(frameNode, childConstraint);
    UpdateConstraintHeight(frameNode, childConstraint);
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(menuLayoutProperty, childConstraint);
    if (!menuPattern->IsMultiMenu()) {
        UpdateConstraintBaseOnOptions(frameNode, childConstraint);
    } else {
        UpdateConstraintBaseOnMenuItems(frameNode, childConstraint);
    }
    return childConstraint;
}

void MenuLayoutAlgorithm::UpdateConstraintBaseOnOptions(FrameNode* frameNode, LayoutConstraintF& constraint)
{
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto options = menuPattern->GetOptions();
    if (options.empty()) {
        LOGD("options is empty, no need to update constraint.");
        return;
    }
    auto maxChildrenWidth = constraint.minSize.Width();
    auto optionConstraint = constraint;
    optionConstraint.maxSize.MinusWidth(optionPadding_ * 2.0f);
    auto optionsNode = GetOptionsLayoutWrappper(frameNode);
    for (const auto& option : optionsNode) {
        option->Measure(optionConstraint);
        auto childSize = option->GetGeometryNode()->GetMarginFrameSize();
        maxChildrenWidth = std::max(maxChildrenWidth, childSize.Width());
    }
    if (menuPattern->IsSelectOverlayExtensionMenu()) {
        maxChildrenWidth = std::min(maxChildrenWidth, optionConstraint.maxSize.Width());
        UpdateOptionConstraint(optionsNode, maxChildrenWidth);
        constraint.minSize.SetWidth(maxChildrenWidth);
        return;
    }
    UpdateOptionConstraint(optionsNode, maxChildrenWidth);
    constraint.minSize.SetWidth(maxChildrenWidth + optionPadding_ * 2.0f);
}

std::list<RefPtr<FrameNode>> MenuLayoutAlgorithm::GetOptionsLayoutWrappper(FrameNode* frameNode)
{
    std::list<RefPtr<FrameNode>> options;
    auto scroll = frameNode->GetFrameNodeByIndex(0);
    CHECK_NULL_RETURN(scroll, options);
    auto column = frameNode->GetFrameNodeByIndex(0);
    CHECK_NULL_RETURN(column, options);
    options = frameNode->GetAllFrameNodeChildren();
    return options;
}

void MenuLayoutAlgorithm::UpdateOptionConstraint(std::list<RefPtr<FrameNode>>& options, float width)
{
    for (const auto& option : options) {
        auto optionLayoutProps = option->GetLayoutProperty();
        CHECK_NULL_VOID(optionLayoutProps);
        optionLayoutProps->UpdateCalcMinSize(CalcSize(CalcLength(width), std::nullopt));
    }
}

void MenuLayoutAlgorithm::UpdateConstraintBaseOnMenuItems(FrameNode* frameNode, LayoutConstraintF& constraint)
{
    // multiMenu children are menuItem or menuItemGroup, constrain width is same as the menu
    auto maxChildrenWidth = GetChildrenMaxWidth(frameNode, constraint);
    constraint.minSize.SetWidth(maxChildrenWidth);
}

void MenuLayoutAlgorithm::LayoutSubMenu(FrameNode* frameNode)
{
    auto size = frameNode->GetGeometryNode()->GetFrameSize();
    auto menuLayoutProperty = AceType::DynamicCast<MenuLayoutProperty>(frameNode->GetLayoutProperty());
    CHECK_NULL_VOID(menuLayoutProperty);
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto parentMenuItem = menuPattern->GetParentMenuItem();
    CHECK_NULL_VOID(parentMenuItem);
    auto menuItemSize = parentMenuItem->GetGeometryNode()->GetFrameSize();

    float x = HorizontalLayoutSubMenu(size, position_.GetX(), menuItemSize) - pageOffset_.GetX();
    float y = VerticalLayoutSubMenu(size, position_.GetY(), menuItemSize) - pageOffset_.GetY();

    const auto& geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    geometryNode->SetMarginFrameOffset(NG::OffsetF(x, y));

    if (parentMenuItem) {
        auto parentPattern = parentMenuItem->GetPattern<MenuItemPattern>();
        auto topLeftPoint = OffsetF(x, y);
        auto bottomRightPoint = OffsetF(x + size.Width(), y + size.Height());
        parentPattern->AddHoverRegions(topLeftPoint, bottomRightPoint);
    }

    auto child = frameNode->GetFrameNodeByIndex(0);
    child->Layout();
}

// return vertical offset
float MenuLayoutAlgorithm::VerticalLayout(const SizeF& size, float position)
{
    float wrapperHeight = wrapperSize_.Height();
    // can put menu below click point
    if (bottomSpace_ >= size.Height()) {
        return position + margin_;
    }

    // put menu above click point
    if (topSpace_ >= size.Height()) {
        return topSpace_ - size.Height() + margin_ + pageOffset_.GetY();
    }

    // line up bottom of menu with bottom of the screen
    if (size.Height() < wrapperHeight) {
        return wrapperHeight - size.Height();
    }
    // can't fit in screen, line up with top of the screen
    return 0.0f;
}

// returns horizontal offset
float MenuLayoutAlgorithm::HorizontalLayout(const SizeF& size, float position, bool isSelectMenu)
{
    float wrapperWidth = wrapperSize_.Width();
    // can fit menu on the right side of position
    if (rightSpace_ >= size.Width()) {
        return position + margin_;
    }

    // fit menu on the left side
    if (!isSelectMenu && leftSpace_ >= size.Width()) {
        return position - size.Width();
    }

    // line up right side of menu with right boundary of the screen
    if (size.Width() < wrapperWidth) {
        return wrapperWidth - size.Width();
    }

    // can't fit in screen, line up with left side of the screen
    return 0.0f;
}

// return vertical offset
float MenuLayoutAlgorithm::VerticalLayoutSubMenu(const SizeF& size, float position, const SizeF& menuItemSize)
{
    float wrapperHeight = wrapperSize_.Height();
    float bottomSpace = wrapperHeight - position;
    // line up top of subMenu with top of the menuItem
    if (bottomSpace >= size.Height()) {
        return position;
    }
    // line up bottom of menu with bottom of the screen
    if (size.Height() < wrapperHeight) {
        return wrapperHeight - size.Height();
    }
    // can't fit in screen, line up with top of the screen
    return 0.0f;
}

// returns submenu horizontal offset
float MenuLayoutAlgorithm::HorizontalLayoutSubMenu(const SizeF& size, float position, const SizeF& menuItemSize)
{
    float wrapperWidth = wrapperSize_.Width();
    float rightSpace = wrapperWidth - position;
    float leftSpace = position - pageOffset_.GetX() - menuItemSize.Width();
    // can fit subMenu on the right side of menuItem
    if (rightSpace >= size.Width()) {
        return position;
    }
    // fit subMenu on the left side of menuItem
    if (leftSpace >= size.Width()) {
        return position - size.Width() - menuItemSize.Width();
    }
    // line up right side of menu with right boundary of the screen
    if (size.Width() < wrapperWidth) {
        return wrapperWidth - size.Width();
    }
    // can't fit in screen, line up with left side of the screen
    return 0.0f;
}

float MenuLayoutAlgorithm::GetChildrenMaxWidth(FrameNode* frameNode, const LayoutConstraintF& layoutConstraint)
{
    float maxWidth = layoutConstraint.minSize.Width();
    for (const auto& child : frameNode->GetAllFrameNodeChildren()) {
        child->Measure(layoutConstraint);
        auto childSize = child->GetGeometryNode()->GetFrameSize();
        maxWidth = std::max(maxWidth, childSize.Width());
    }
    return maxWidth;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacement(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;

    auto func = placementFuncMap_.find(placement_);
    if (func != placementFuncMap_.end()) {
        auto placementFunc = func->second;
        if (placementFunc != nullptr) {
            childPosition = (this->*placementFunc)(childSize, topPosition, bottomPosition);
        } else {
            LOGE("Invalid Placement of menu layout.");
        }
    } else {
            LOGE("Invalid Placement of menu layout.");
    }

    return childPosition;
}

void MenuLayoutAlgorithm::InitTargetSizeAndPosition(const RefPtr<MenuLayoutProperty>& layoutProp)
{
    auto targetNode = FrameNode::GetFrameNode(targetTag_, targetNodeId_);
    CHECK_NULL_VOID(targetNode);
    auto geometryNode = targetNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    targetSize_ = geometryNode->GetFrameSize();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto isContainerModal = pipelineContext->GetWindowModal() == WindowModal::CONTAINER_MODAL &&
                            pipelineContext->GetWindowManager()->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    targetOffset_ = targetNode->GetPaintRectOffset();
    if (isContainerModal) {
        auto newOffsetX = targetOffset_.GetX() - static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx()) -
                          static_cast<float>(CONTENT_PADDING.ConvertToPx());
        auto newOffsetY = targetOffset_.GetY() - static_cast<float>(CONTAINER_TITLE_HEIGHT.ConvertToPx());
        targetOffset_.SetX(newOffsetX);
        targetOffset_.SetY(newOffsetY);
    }
}

MenuLayoutAlgorithm::ErrorPositionType MenuLayoutAlgorithm::GetErrorPositionType(
    const OffsetF& childOffset, const SizeF& childSize)
{
    RectF validRegion = RectF(targetOffset_, targetSize_);
    RectF childRegion(childOffset, childSize);
    if (validRegion.IsInnerIntersectWith(childRegion)) {
        return ErrorPositionType::TOP_LEFT_ERROR;
    }
    return ErrorPositionType::NORMAL;
}

OffsetF MenuLayoutAlgorithm::FitToScreen(const OffsetF& fitPosition, const SizeF& childSize)
{
    float x = fitPosition.GetX() + positionOffset_.GetX();
    float y = fitPosition.GetY() + positionOffset_.GetY();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
        float windowsOffsetX = windowGlobalRect.GetOffset().GetX();
        float windowsOffsetY = windowGlobalRect.GetOffset().GetY();
        x += windowsOffsetX;
        y += windowsOffsetY;
        x = std::clamp(x, windowsOffsetX, wrapperSize_.Width() - childSize.Width() - margin_ * 2.0f + windowsOffsetX);
        y = std::clamp(y, windowsOffsetY, wrapperSize_.Height() - childSize.Height() - margin_ * 2.0f + windowsOffsetY);

        return OffsetF(x, y);
    }
    x = std::clamp(x, 0.0f, wrapperSize_.Width() - childSize.Width() - margin_ * 2.0f);
    y = std::clamp(y, 0.0f, wrapperSize_.Height() - childSize.Height() - margin_ * 2.0f);

    return OffsetF(x, y);
}

OffsetF MenuLayoutAlgorithm::GetChildPosition(const SizeF& childSize, const RefPtr<MenuLayoutProperty>& layoutProp)
{
    OffsetF bottomPosition = OffsetF(targetOffset_.GetX() + (targetSize_.Width() - childSize.Width()) / 2.0,
        targetOffset_.GetY() + targetSize_.Height());
    OffsetF topPosition = OffsetF(targetOffset_.GetX() + (targetSize_.Width() - childSize.Width()) / 2.0,
        targetOffset_.GetY() - childSize.Height());

    OffsetF originOffset = GetPositionWithPlacement(childSize, topPosition, bottomPosition);
    OffsetF childPosition = originOffset;
    // Fit popup to screen range.
    auto fitOffset = FitToScreen(childPosition, childSize);
    ErrorPositionType errorType = GetErrorPositionType(fitOffset, childSize);
    if (errorType == ErrorPositionType::NORMAL) {
        return childPosition;
    }
    auto placement = placement_;
    // If childPosition is error, adjust menu to bottom.
    if (placement_ != Placement::BOTTOM) {
        placement_ = Placement::BOTTOM;
        childPosition = GetPositionWithPlacement(childSize, topPosition, bottomPosition);
        fitOffset = FitToScreen(childPosition, childSize);
        if (GetErrorPositionType(fitOffset, childSize) == ErrorPositionType::NORMAL) {
            placement_ = placement;
            return childPosition;
        }
    }
    // If childPosition is error, adjust menu to top.
    if (placement_ != Placement::TOP) {
        placement_ = Placement::TOP;
        childPosition = GetPositionWithPlacement(childSize, topPosition, bottomPosition);
        fitOffset = FitToScreen(childPosition, childSize);
        if (GetErrorPositionType(fitOffset, childSize) == ErrorPositionType::NORMAL) {
            placement_ = placement;
            return childPosition;
        }
    }
    placement_ = placement;
    return originOffset;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementTop(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    return topPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementTopLeft(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginRight = 0.0f;
    float marginBottom = 0.0f;
    double targetSpace = 0.0;
    childPosition = OffsetF(targetOffset_.GetX() - marginRight,
        targetOffset_.GetY() - childSize.Height() - marginBottom - targetSpace);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementTopRight(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginBottom = 0.0f;
    float marginLeft = 0.0f;
    double targetSpace = 0.0;
    childPosition = OffsetF(targetOffset_.GetX() + targetSize_.Width() - childSize.Width() + marginLeft,
        targetOffset_.GetY() - childSize.Height() - targetSpace - marginBottom);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementBottom(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    return bottomPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementBottomLeft(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginRight = 0.0f;
    float marginTop = 0.0f;
    double targetSpace = 0.0;
    childPosition = OffsetF(targetOffset_.GetX() - marginRight,
        targetOffset_.GetY() + targetSize_.Height() + targetSpace + marginTop);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementBottomRight(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginTop = 0.0f;
    float marginLeft = 0.0f;
    double targetSpace = 0.0;
    childPosition = OffsetF(targetOffset_.GetX() + targetSize_.Width() - childSize.Width() + marginLeft,
        targetOffset_.GetY() + targetSize_.Height() + targetSpace + marginTop);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementLeft(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginRight = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() - targetSpace - childSize.Width() - marginRight,
            targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementLeftTop(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginRight = 0.0f;
    float marginBottom = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() - targetSpace - childSize.Width() - marginRight,
            targetOffset_.GetY() - marginBottom);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementLeftBottom(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginRight = 0.0f;
    float marginTop = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() - targetSpace - childSize.Width() - marginRight,
            targetOffset_.GetY() + targetSize_.Height() - childSize.Height() - marginTop);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementRight(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginLeft = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + marginLeft,
            targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementRightTop(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginBottom = 0.0f;
    float marginLeft = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + marginLeft,
            targetOffset_.GetY() - marginBottom);
    return childPosition;
}

OffsetF MenuLayoutAlgorithm::GetPositionWithPlacementRightBottom(
    const SizeF& childSize, const OffsetF& topPosition, const OffsetF& bottomPosition)
{
    OffsetF childPosition;
    float marginTop = 0.0f;
    float marginLeft = 0.0f;
    double targetSpace = 0.0;
    childPosition =
        OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + marginLeft,
            targetOffset_.GetY() + targetSize_.Height() - childSize.Height() - marginTop);
    return childPosition;
}
} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/select_overlay/select_overlay_layout_algorithm.h"

#include <cmath>
#include <optional>

#include "base/geometry/ng/offset_t.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension MORE_MENU_INTERVAL = 8.0_vp;
}

void SelectOverlayLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto layoutConstraint = layoutProperty->CreateChildConstraint();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto safeAreaManager = pipeline->GetSafeAreaManager();
    CHECK_NULL_VOID(safeAreaManager);
    auto keyboardHeight = safeAreaManager->GetKeyboardInset().Length();
    auto maxSize =
        SizeF(layoutConstraint.maxSize.Width(), layoutConstraint.maxSize.Height() - keyboardHeight -
                                                    (info_->isUsingMouse ? info_->rightClickOffset.GetY() : 0.0f));
    layoutConstraint.maxSize = maxSize;
    bool isMouseCustomMenu = false;
    if (info_->menuInfo.menuBuilder) {
        auto customMenuLayoutWrapper = layoutWrapper->GetChildByIndex(0);
        CHECK_NULL_VOID(customMenuLayoutWrapper);
        auto customMenuLayoutConstraint = layoutConstraint;
        CalculateCustomMenuLayoutConstraint(layoutWrapper, customMenuLayoutConstraint);
        customMenuLayoutWrapper->Measure(customMenuLayoutConstraint);
        isMouseCustomMenu = true;
    }
    auto childIndex = -1;
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        childIndex++;
        if (childIndex == 0 && isMouseCustomMenu) {
            continue;
        }
        child->Measure(layoutConstraint);
    }
    PerformMeasureSelf(layoutWrapper);
}

void SelectOverlayLayoutAlgorithm::CalculateCustomMenuLayoutConstraint(
    LayoutWrapper* layoutWrapper, LayoutConstraintF& layoutConstraint)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    // Calculate the spacing with text and handle, menu is fixed up the handle and text.
    double menuSpacingBetweenText = theme->GetMenuSpacingWithText().ConvertToPx();
    double menuSpacingBetweenHandle = theme->GetHandleDiameter().ConvertToPx();

    // paint rect is in global position, need to convert to local position
    auto offset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    const auto firstHandleRect = info_->firstHandle.GetPaintRect() - offset;
    const auto secondHandleRect = info_->secondHandle.GetPaintRect() - offset;

    auto top = info_->isNewAvoid ? info_->selectArea.Top() : firstHandleRect.Top();
    auto bottom = info_->isNewAvoid ? info_->selectArea.Bottom() : secondHandleRect.Bottom();
    auto topSpace = top - menuSpacingBetweenText - menuSpacingBetweenHandle;
    auto bottomSpace = layoutConstraint.maxSize.Height() -
                       (bottom + menuSpacingBetweenText + menuSpacingBetweenHandle);
    if (info_->isUsingMouse) {
        layoutConstraint.selfIdealSize = OptionalSizeF(std::nullopt, layoutConstraint.maxSize.Height());
    } else {
        layoutConstraint.selfIdealSize = OptionalSizeF(std::nullopt, std::max(topSpace, bottomSpace));
    }
}

OffsetF SelectOverlayLayoutAlgorithm::CalculateCustomMenuByMouseOffset(LayoutWrapper* layoutWrapper)
{
    auto menuOffset = info_->rightClickOffset;
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, menuOffset);
    auto layoutConstraint = layoutProperty->GetLayoutConstraint();
    CHECK_NULL_RETURN(layoutConstraint, menuOffset);
    auto menu = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(menu, menuOffset);
    auto maxWidth = layoutConstraint->selfIdealSize.Width().value_or(0.0f);
    auto menuSize = menu->GetGeometryNode()->GetFrameSize();
    if (GreatNotEqual(menuOffset.GetX() + menuSize.Width(), maxWidth)) {
        if (GreatOrEqual(menuOffset.GetX(), menuSize.Width())) {
            menuOffset.SetX(menuOffset.GetX() - menuSize.Width());
        } else if (LessOrEqual(menuSize.Width(), maxWidth)) {
            menuOffset.SetX(maxWidth - menuSize.Width());
        } else if (GreatNotEqual(menuSize.Width(), maxWidth)) {
            menuOffset.SetX(menuOffset.GetX() - menuSize.Width() / 2.0f);
        }
    }
    return menuOffset;
}

void SelectOverlayLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto menu = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(menu);
    auto isNewAvoid = info_->isNewAvoid && !info_->isSingleHandle;
    auto shouldInActiveByHandle =
        !info_->firstHandle.isShow && !info_->secondHandle.isShow && !info_->isSelectRegionVisible;
    if ((!CheckInShowArea(*info_) || (!isNewAvoid && shouldInActiveByHandle)) && !info_->isUsingMouse) {
        menu->SetActive(false);
        return;
    } else {
        menu->SetActive(true);
    }
    OffsetF menuOffset;
    if (info_->isUsingMouse) {
        menuOffset = CalculateCustomMenuByMouseOffset(layoutWrapper);
    } else {
        menuOffset = ComputeSelectMenuPosition(layoutWrapper);
    }
    menu->GetGeometryNode()->SetMarginFrameOffset(menuOffset);

    // custom menu need to layout all menu and submenu
    if (info_->menuInfo.menuBuilder) {
        for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
            child->Layout();
        }
        return;
    }
    menu->Layout();
    auto button = layoutWrapper->GetOrCreateChildByIndex(1);
    CHECK_NULL_VOID(button);
    if (!info_->menuInfo.menuIsShow || info_->menuInfo.menuDisable) {
        hasExtensionMenu_ = false;
        return;
    }
    hasExtensionMenu_ = true;
    auto buttonSize = button->GetGeometryNode()->GetMarginFrameSize();
    auto menuSize = menu->GetGeometryNode()->GetMarginFrameSize();
    OffsetF buttonOffset;
    if (GreatNotEqual(menuSize.Width(), menuSize.Height())) {
        buttonOffset = OffsetF(menuOffset.GetX() + menuSize.Width() - buttonSize.Width(), menuOffset.GetY());
    } else {
        buttonOffset = menuOffset;
    }
    button->GetGeometryNode()->SetMarginFrameOffset(buttonOffset);
    button->Layout();
    auto extensionMenu = layoutWrapper->GetOrCreateChildByIndex(2);
    CHECK_NULL_VOID(extensionMenu);
    extensionMenu->Layout();
    CheckHideBackOrMoreButton(extensionMenu, button);
}

void SelectOverlayLayoutAlgorithm::CheckHideBackOrMoreButton(
    const RefPtr<LayoutWrapper>& extensionMenu, const RefPtr<LayoutWrapper>& button)
{
    auto extensionMenuRect = extensionMenu->GetGeometryNode()->GetFrameRect();
    auto buttonRect = button->GetGeometryNode()->GetFrameRect();
    auto constraintRect = extensionMenuRect.Constrain(buttonRect);
    if (GreatNotEqual(constraintRect.Width(), 0.0f) && GreatNotEqual(constraintRect.Height(), 0.0f)) {
        hideMoreOrBack_ = true;
    }
}

bool SelectOverlayLayoutAlgorithm::CheckInShowArea(const SelectOverlayInfo& info)
{
    if (info.useFullScreen) {
        return true;
    }
    if (info.isSingleHandle) {
        return info.firstHandle.GetPaintRect().IsWrappedBy(info.showArea);
    }
    return info.firstHandle.GetPaintRect().IsWrappedBy(info.showArea) &&
           info.secondHandle.GetPaintRect().IsWrappedBy(info.showArea);
}

OffsetF SelectOverlayLayoutAlgorithm::ComputeSelectMenuPosition(LayoutWrapper* layoutWrapper)
{
    auto menuItem = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(menuItem, OffsetF());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF());
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(theme, OffsetF());
    OffsetF menuPosition;
    bool isExtension = false;

    // Calculate the spacing with text and handle, menu is fixed up the handle and text.
    double menuSpacingBetweenText = theme->GetMenuSpacingWithText().ConvertToPx();
    double menuSpacingBetweenHandle = theme->GetHandleDiameter().ConvertToPx();

    auto width = menuItem->GetGeometryNode()->GetMarginFrameSize().Width();
    auto height = menuItem->GetGeometryNode()->GetMarginFrameSize().Height();

    // When the extended menu is displayed, the default menu becomes circular, but the position of the circle is aligned
    // with the end of the original menu.
    if (GreatNotEqual(width, height)) {
        menuWidth_ = width;
        menuHeight_ = height;
    } else {
        isExtension = true;
    }
    auto menuWidth = menuWidth_.value_or(width);
    auto menuHeight = menuHeight_.value_or(height);

    // paint rect is in global position, need to convert to local position
    auto offset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    const auto firstHandleRect = info_->firstHandle.GetPaintRect() - offset;
    const auto secondHandleRect = info_->secondHandle.GetPaintRect() - offset;

    auto singleHandle = firstHandleRect;
    if (!info_->firstHandle.isShow) {
        singleHandle = secondHandleRect;
    }
    if (IsTextAreaSelectAll()) {
        singleHandle = RectF(info_->menuInfo.menuOffset.value().GetX(), info_->menuInfo.menuOffset.value().GetY(),
            singleHandle.Width(), singleHandle.Height());
    }

    if (info_->isSingleHandle) {
        auto menuSpacing = static_cast<float>(menuSpacingBetweenText);
        menuPosition = OffsetF((singleHandle.Left() + singleHandle.Right() - menuWidth) / 2.0f,
            static_cast<float>(singleHandle.Top() - menuSpacing - menuHeight));
    } else {
        auto menuSpacing = static_cast<float>(menuSpacingBetweenText + menuSpacingBetweenHandle);
        menuPosition = OffsetF((firstHandleRect.Left() + secondHandleRect.Left() - menuWidth) / 2.0f,
            static_cast<float>(firstHandleRect.Top() - menuSpacing - menuHeight));

        if (!info_->firstHandle.isShow && info_->secondHandle.isShow && !info_->handleReverse) {
            menuPosition.SetY(secondHandleRect.Bottom() + menuSpacing);
        }
        if (info_->firstHandle.isShow && !info_->secondHandle.isShow && info_->handleReverse) {
            menuPosition.SetY(firstHandleRect.Bottom() + menuSpacing);
        }
        if (info_->firstHandle.isShow && info_->secondHandle.isShow &&
            !NearEqual(firstHandleRect.Top(), secondHandleRect.Top())) {
            auto top = std::min(firstHandleRect.Top(), secondHandleRect.Top());
            menuPosition.SetY(static_cast<float>(top - menuSpacing - menuHeight));
        }
        if (!info_->firstHandle.isShow && info_->secondHandle.isShow) {
            menuPosition.SetX(secondHandleRect.Left() - menuWidth / 2.0f);
        }
        if (info_->firstHandle.isShow && !info_->secondHandle.isShow) {
            menuPosition.SetX(firstHandleRect.Left() - menuWidth / 2.0f);
        }
    }

    auto overlayWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width();
    RectF viewPort = layoutWrapper->GetGeometryNode()->GetFrameRect() - offset;
    info_->GetCallerNodeAncestorViewPort(viewPort);
    // Adjust position of overlay.
    auto adjustPositionXWithViewPort = [&](OffsetF& menuPosition) {
        auto defaultMenuPositionX = theme->GetDefaultMenuPositionX();
        if (LessOrEqual(menuPosition.GetX(), defaultMenuPositionX)) {
            menuPosition.SetX(defaultMenuPositionX);
        } else if (GreatOrEqual(
            menuPosition.GetX() + menuWidth, viewPort.GetX() + viewPort.Width() - defaultMenuPositionX)) {
            menuPosition.SetX(overlayWidth - menuWidth - defaultMenuPositionX);
        }
    };
    adjustPositionXWithViewPort(menuPosition);
    if (LessNotEqual(menuPosition.GetY(), menuHeight)) {
        if (IsTextAreaSelectAll()) {
            menuPosition.SetY(singleHandle.Top());
        } else {
            menuPosition.SetY(
                static_cast<float>(singleHandle.Bottom() + menuSpacingBetweenText + menuSpacingBetweenHandle));
        }
    }
    auto spaceBetweenViewPort = menuSpacingBetweenText + menuSpacingBetweenHandle;
    if (LessNotEqual(menuPosition.GetY(), viewPort.GetY() - spaceBetweenViewPort - menuHeight) ||
        LessNotEqual(menuPosition.GetY(), menuSpacingBetweenText)) {
        auto menuOffsetY = viewPort.GetY() - spaceBetweenViewPort - menuHeight;
        if (menuOffsetY > menuSpacingBetweenText) {
            menuPosition.SetY(menuOffsetY);
        } else {
            menuPosition.SetY(menuSpacingBetweenText);
        }
    } else if (GreatOrEqual(menuPosition.GetY(), viewPort.GetY() + viewPort.Height() + spaceBetweenViewPort)) {
        menuPosition.SetY(viewPort.GetY() + viewPort.Height() + spaceBetweenViewPort);
    }

    auto safeAreaManager = pipeline->GetSafeAreaManager();
    if (safeAreaManager) {
        // ignore status bar
        auto top = safeAreaManager->GetSystemSafeArea().top_.Length();
        if (menuPosition.GetY() < top) {
            menuPosition.SetY(top);
        }
    }
    if (info_->firstHandle.isShow && info_->secondHandle.isShow &&
        !NearEqual(firstHandleRect.Top(), secondHandleRect.Top())) {
        auto menuRect = RectF(menuPosition, SizeF(menuWidth, menuHeight));
        auto downHandleRect =
            LessNotEqual(firstHandleRect.Top(), secondHandleRect.Top()) ? secondHandleRect : firstHandleRect;
        auto circleDiameter = menuSpacingBetweenHandle;
        auto circleOffset =
            OffsetF(downHandleRect.GetX() - (circleDiameter - downHandleRect.Width()) / 2.0f, downHandleRect.Bottom());
        auto downHandleCircleRect = RectF(circleOffset, SizeF(circleDiameter, circleDiameter));
        if (menuRect.IsIntersectWith(downHandleRect) || menuRect.IsInnerIntersectWith(downHandleCircleRect)) {
            auto menuSpacing = static_cast<float>(menuSpacingBetweenText + circleDiameter);
            menuPosition.SetY(downHandleRect.Bottom() + menuSpacing);
        }
    }
    auto menuRect = RectF(menuPosition, SizeF(menuWidth, menuHeight));
    menuPosition = info_->isNewAvoid && !info_->isSingleHandle ? NewMenuAvoidStrategy(menuWidth, menuHeight) :
        AdjustSelectMenuOffset(layoutWrapper, menuRect, menuSpacingBetweenText, menuSpacingBetweenHandle);
    AdjustMenuInRootRect(menuPosition, menuRect.GetSize(), layoutWrapper->GetGeometryNode()->GetFrameSize());
    defaultMenuEndOffset_ = menuPosition + OffsetF(menuWidth, 0.0f);
    if (isExtension) {
        return defaultMenuEndOffset_ - OffsetF(width, 0);
    }
    return menuPosition;
}

void SelectOverlayLayoutAlgorithm::AdjustMenuInRootRect(
    OffsetF& menuOffset, const SizeF& menuSize, const SizeF& rootSize)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    // adjust x
    auto defaultPositionX = theme->GetDefaultMenuPositionX();
    auto menuX = LessOrEqual(menuOffset.GetX(), defaultPositionX) ? defaultPositionX : menuOffset.GetX();
    menuX = GreatOrEqual(menuX + menuSize.Width(), rootSize.Width() - defaultPositionX)
                ? rootSize.Width() - defaultPositionX - menuSize.Width()
                : menuX;
    menuOffset.SetX(menuX);
    // adjust y
    auto menuY = LessNotEqual(menuOffset.GetY(), 0.0f) ? 0.0f : menuOffset.GetY();
    menuY = GreatNotEqual(menuY + menuSize.Height(), rootSize.Height()) ? rootSize.Height() - menuSize.Height() : menuY;
    menuOffset.SetY(menuY);
}

OffsetF SelectOverlayLayoutAlgorithm::AdjustSelectMenuOffset(
    LayoutWrapper* layoutWrapper, const RectF& menuRect, double spaceBetweenText, double spaceBetweenHandle)
{
    auto menuOffset = menuRect.GetOffset();
    if (!info_->firstHandle.isShow && !info_->secondHandle.isShow) {
        return menuOffset;
    }
    auto offset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    auto upHandle = info_->handleReverse ? info_->secondHandle : info_->firstHandle;
    auto downHandle = info_->handleReverse ? info_->firstHandle : info_->secondHandle;
    AdjustMenuTooFarAway(menuOffset, menuRect);
    // menu cover up handle
    auto upPaint = upHandle.GetPaintRect() - offset;
    auto downPaint = downHandle.GetPaintRect() - offset;
    if (!info_->isSingleHandle && upHandle.isShow && !downHandle.isShow) {
        auto circleOffset = OffsetF(
            upPaint.GetX() - (spaceBetweenHandle - upPaint.Width()) / 2.0f, upPaint.GetY() - spaceBetweenHandle);
        auto upCircleRect = RectF(circleOffset, SizeF(spaceBetweenHandle, spaceBetweenHandle));
        if (menuRect.IsIntersectWith(upPaint) || menuRect.IsIntersectWith(upCircleRect)) {
            menuOffset.SetY(upPaint.Bottom() + spaceBetweenText + spaceBetweenHandle);
        }
        return menuOffset;
    }
    // avoid soft keyboard and root bottom
    if ((!upHandle.isShow && downHandle.isShow) || info_->menuInfo.menuBuilder) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, menuOffset);
        auto safeAreaManager = pipeline->GetSafeAreaManager();
        CHECK_NULL_RETURN(safeAreaManager, menuOffset);
        auto keyboardInsert = safeAreaManager->GetKeyboardInset();
        auto shouldAvoidKeyboard =
            GreatNotEqual(keyboardInsert.Length(), 0.0f) && GreatNotEqual(menuRect.Bottom(), keyboardInsert.start);
        auto rootRect = layoutWrapper->GetGeometryNode()->GetFrameRect();
        auto shouldAvoidBottom = GreatNotEqual(menuRect.Bottom(), rootRect.Height());
        auto menuSpace = NearEqual(upPaint.Top(), downPaint.Top()) ? spaceBetweenHandle : spaceBetweenText;
        auto offsetY = downPaint.GetY() - menuSpace - menuRect.Height();
        if ((shouldAvoidKeyboard || shouldAvoidBottom) && offsetY > 0) {
            menuOffset.SetY(offsetY);
        }
    }
    return menuOffset;
}

void SelectOverlayLayoutAlgorithm::AdjustMenuTooFarAway(OffsetF& menuOffset, const RectF& menuRect)
{
    // the menu is too far away.
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto hostFrameNode = info_->callerFrameNode.Upgrade();
    CHECK_NULL_VOID(hostFrameNode);
    auto hostFrameRect = hostFrameNode->GetGeometryNode()->GetFrameRect();
    auto hostGlobalOffset = hostFrameNode->GetPaintRectOffset() - pipeline->GetRootRect().GetOffset();
    auto centerX = menuRect.Width() / 2.0f;
    if (info_->callerNodeInfo) {
        hostFrameRect = info_->callerNodeInfo->paintFrameRect;
        hostGlobalOffset = info_->callerNodeInfo->paintOffset;
    }
    if (GreatNotEqual(menuRect.GetX() + centerX, hostGlobalOffset.GetX() + hostFrameRect.Width())) {
        menuOffset.SetX(hostGlobalOffset.GetX() + hostFrameRect.Width() - centerX);
        return;
    }
    if (LessNotEqual(menuRect.GetX() + centerX, hostGlobalOffset.GetX())) {
        menuOffset.SetX(hostGlobalOffset.GetX() - centerX);
    }
}

OffsetF SelectOverlayLayoutAlgorithm::ComputeExtensionMenuPosition(LayoutWrapper* layoutWrapper, const OffsetF& offset)
{
    auto extensionItem = layoutWrapper->GetOrCreateChildByIndex(2);
    CHECK_NULL_RETURN(extensionItem, OffsetF());
    auto extensionLayoutConstraint = extensionItem->GetLayoutProperty()->GetLayoutConstraint();
    auto extensionLayoutConstraintMaxSize = extensionLayoutConstraint->maxSize;
    auto extensionWidth = extensionItem->GetGeometryNode()->GetMarginFrameSize().Width();
    auto extensionHeight = extensionItem->GetGeometryNode()->GetMarginFrameSize().Height();
    auto menuItem = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(menuItem, OffsetF());
    auto menuHeight = menuItem->GetGeometryNode()->GetMarginFrameSize().Height();
    auto extensionOffset =
        defaultMenuEndOffset_ - OffsetF(extensionWidth, -menuHeight - MORE_MENU_INTERVAL.ConvertToPx());
    auto extensionBottom = extensionOffset.GetY() + extensionHeight;
    auto isCoveredBySoftKeyBoard = [extensionBottom]() -> bool {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, false);
        auto safeAreaManager = pipeline->GetSafeAreaManager();
        CHECK_NULL_RETURN(safeAreaManager, false);
        auto keyboardInsert = safeAreaManager->GetKeyboardInset();
        return GreatNotEqual(keyboardInsert.Length(), 0.0f) && GreatNotEqual(extensionBottom, keyboardInsert.start);
    };
    if (GreatNotEqual(extensionBottom, extensionLayoutConstraintMaxSize.Height()) || isCoveredBySoftKeyBoard()) {
        extensionOffset =
            defaultMenuEndOffset_ - OffsetF(extensionWidth, extensionHeight + MORE_MENU_INTERVAL.ConvertToPx());
    }
    return extensionOffset;
}

bool SelectOverlayLayoutAlgorithm::IsTextAreaSelectAll()
{
    return info_->menuInfo.menuOffset.has_value() && (!info_->firstHandle.isShow || !info_->secondHandle.isShow);
}

OffsetF SelectOverlayLayoutAlgorithm::NewMenuAvoidStrategy(float menuWidth, float menuHeight)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF());
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(theme, OffsetF());

    // Calculate the spacing with text and handle, menu is fixed up the handle and
    // text.
    double menuSpacingBetweenText = theme->GetMenuSpacingWithText().ConvertToPx();
    double menuSpacingBetweenHandle = theme->GetHandleDiameter().ConvertToPx();
    double safeSpacing = theme->GetMenuSafeSpacing().ConvertToPx();
    auto selectArea = info_->selectArea;
    // 安全区域
    auto safeAreaManager = pipeline->GetSafeAreaManager();
    CHECK_NULL_RETURN(safeAreaManager, OffsetF());
    auto topArea = safeAreaManager->GetSystemSafeArea().top_.Length();
    auto keyboardInsert = safeAreaManager->GetKeyboardInset();
    float positionX = (selectArea.Left() + selectArea.Right() - menuWidth) / 2.0f;
    auto menuSpacing = static_cast<float>(menuSpacingBetweenText + menuSpacingBetweenHandle);
    auto upHandle = info_->handleReverse ? info_->secondHandle : info_->firstHandle;
    auto downHandle = info_->handleReverse ? info_->firstHandle : info_->secondHandle;
    auto viewPort = pipeline->GetRootRect();
    auto hasKeyboard = GreatNotEqual(keyboardInsert.Length(), 0.0f);
    auto bottomLimitOffsetY = hasKeyboard ? std::max(keyboardInsert.start - safeSpacing - menuHeight, (double)topArea)
                                          : safeAreaManager->GetSafeArea().bottom_.start - menuHeight;
    // 顶部避让
    auto offsetY = selectArea.Top() - menuSpacing - menuHeight;
    if (!upHandle.isShow || LessOrEqual(offsetY, topArea)) {
        selectArea = selectArea.IntersectRectT(viewPort);
        auto offsetUponSelectArea = selectArea.Top() - menuSpacingBetweenText - menuHeight;
        auto offsetBetweenSelectArea = std::clamp(
            (double)(selectArea.Top() + selectArea.Bottom() - menuHeight) / 2.0f, (double)topArea, bottomLimitOffsetY);
        if (downHandle.isShow) {
            bool isOffsetYInBottom = false;
            // The upper handle is not visible and not in a single row, or offsetY <= topArea
            if ((!upHandle.isShow && !info_->isSingleLine) || (LessOrEqual(offsetY, topArea))) {
                offsetY = selectArea.Bottom() + menuSpacing;
                isOffsetYInBottom = true;
            }
            if (isOffsetYInBottom && (offsetY > bottomLimitOffsetY)) {
                // 底部避让失败 -> 选区上方 > 选区中间
                offsetY = !upHandle.isShow && LessNotEqual(topArea, offsetUponSelectArea) ? offsetUponSelectArea
                                                                                          : offsetBetweenSelectArea;
            }
        } else {
            // 上下手柄均不可见 -> 选区上方 > 选区中间
            offsetY = !upHandle.isShow && LessNotEqual(topArea, offsetUponSelectArea) ? offsetUponSelectArea
                                                                                      : offsetBetweenSelectArea;
        }
    }
    if (hasKeyboard && GreatNotEqual(offsetY, bottomLimitOffsetY)) {
        offsetY = bottomLimitOffsetY;
    }
    return OffsetF(positionX, offsetY);
}
} // namespace OHOS::Ace::NG
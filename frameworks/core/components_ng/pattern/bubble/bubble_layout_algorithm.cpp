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

#include "core/components_ng/pattern/bubble/bubble_layout_algorithm.h"

#include <algorithm>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/device_config.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/placement.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components/popup/popup_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/bubble/bubble_layout_property.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

namespace {

constexpr Dimension ARROW_WIDTH = 32.0_vp;
constexpr Dimension ARROW_HEIGHT = 8.0_vp;
constexpr Dimension HORIZON_SPACING_WITH_SCREEN = 6.0_vp;
constexpr Dimension BEZIER_WIDTH_HALF = 16.0_vp;

} // namespace

void BubbleLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto bubbleProp = DynamicCast<BubbleLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(bubbleProp);
    InitProps(bubbleProp);
    auto bubbleLayoutProperty = AceType::DynamicCast<BubbleLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(bubbleLayoutProperty);

    const auto& layoutConstraint = bubbleLayoutProperty->GetLayoutConstraint();
    if (!layoutConstraint) {
        LOGE("fail to measure bubble due to layoutConstraint is nullptr");
        return;
    }
    bool useCustom = bubbleLayoutProperty->GetUseCustom().value_or(false);
    // bubble size fit screen.
    layoutWrapper->GetGeometryNode()->SetFrameSize(layoutConstraint->maxSize);
    layoutWrapper->GetGeometryNode()->SetContentSize(layoutConstraint->maxSize);

    // update child layout constraint
    LayoutConstraintF childLayoutConstraint = bubbleLayoutProperty->CreateChildConstraint();
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    if (children.empty()) {
        return;
    }
    auto child = children.front();
    // childSize_ and childOffset_ is used in Layout.
    child->Measure(childLayoutConstraint);
    bool showInSubWindow = bubbleLayoutProperty->GetShowInSubWindowValue(false);
    if (useCustom && !showInSubWindow) {
        auto context = layoutWrapper->GetHostNode()->GetContext();
        float rootH = context->GetRootHeight();
        float rootW = context->GetRootWidth();
        auto childHeight = child->GetGeometryNode()->GetMarginFrameSize().Height();
        auto childWidth = child->GetGeometryNode()->GetMarginFrameSize().Width();
        auto scaledBubbleSpacing = scaledBubbleSpacing_ * 2;
        auto targetNode = FrameNode::GetFrameNode(targetTag_, targetNodeId_);
        CHECK_NULL_VOID(targetNode);
        auto geometryNode = targetNode->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto targetSize = geometryNode->GetFrameSize();
        auto targetOffset = targetNode->GetPaintRectOffset();
        auto constrainHeight = layoutWrapper->GetGeometryNode()->GetFrameSize().Height();
        auto constrainWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width();
        auto placement = bubbleLayoutProperty->GetPlacement().value_or(Placement::BOTTOM);
        std::unordered_set<Placement> setHorizontal = { Placement::LEFT, Placement::LEFT_BOTTOM, Placement::LEFT_TOP,
            Placement::RIGHT, Placement::RIGHT_BOTTOM, Placement::RIGHT_TOP };
        std::unordered_set<Placement> setVertical = { Placement::TOP, Placement::TOP_LEFT, Placement::TOP_RIGHT,
            Placement::BOTTOM, Placement::BOTTOM_LEFT, Placement::BOTTOM_RIGHT };
        if (setHorizontal.find(placement) != setHorizontal.end()) {
            if (childWidth + targetOffset.GetX() + targetSize.Width() + scaledBubbleSpacing <= rootW &&
                targetOffset.GetX() - childWidth - scaledBubbleSpacing >= 0) {
                return;
            }
            constrainWidth = rootW - scaledBubbleSpacing;
        }
        if (setVertical.find(placement) != setVertical.end()) {
            if (childHeight + targetOffset.GetY() + targetSize.Height() + scaledBubbleSpacing <= rootH &&
                targetOffset.GetY() - childHeight - scaledBubbleSpacing >= 0) {
                return;
            }
            constrainHeight = std::max(rootH - targetOffset.GetY() - targetSize.Height() - scaledBubbleSpacing,
                targetOffset.GetY() - scaledBubbleSpacing);
        }
        SizeF size = SizeF(constrainWidth, constrainHeight);
        childLayoutConstraint.UpdateMaxSizeWithCheck(size);
        child->Measure(childLayoutConstraint);
    }
}

void BubbleLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto bubbleProp = DynamicCast<BubbleLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(bubbleProp);
    InitTargetSizeAndPosition(bubbleProp);
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    if (children.empty()) {
        return;
    }
    selfSize_ = layoutWrapper->GetGeometryNode()->GetFrameSize(); // bubble's size
    auto child = children.front();
    childSize_ = child->GetGeometryNode()->GetMarginFrameSize(); // bubble's child's size
    childOffset_ = GetChildPosition(childSize_, bubbleProp);     // bubble's child's offset
    bool useCustom = bubbleProp->GetUseCustom().value_or(false);
    UpdateChildPosition(bubbleProp);
    UpdateTouchRegion();
    child->GetGeometryNode()->SetMarginFrameOffset(childOffset_);
    child->Layout();
    auto childLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(childLayoutWrapper);
    const auto& columnChild = childLayoutWrapper->GetAllChildrenWithBuild();
    if (columnChild.size() > 1 && !useCustom) {
        auto buttonRow = columnChild.back();
        buttonRowSize_ = buttonRow->GetGeometryNode()->GetMarginFrameSize();
        buttonRowOffset_ = buttonRow->GetGeometryNode()->GetMarginFrameOffset() + childOffset_;
    }
    if (bubbleProp->GetShowInSubWindowValue(false)) {
        std::vector<Rect> rects;
        if (!bubbleProp->GetBlockEventValue(true)) {
            auto rect = Rect(childOffset_.GetX(), childOffset_.GetY(), childSize_.Width(), childSize_.Height());
            rects.emplace_back(rect);
        } else {
            auto rect = Rect(0.0f, 0.0f, selfSize_.Width(), selfSize_.Height());
            rects.emplace_back(rect);
        }
        SubwindowManager::GetInstance()->SetHotAreas(rects);
    }
}

void BubbleLayoutAlgorithm::InitProps(const RefPtr<BubbleLayoutProperty>& layoutProp)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto popupTheme = pipeline->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    padding_ = popupTheme->GetPadding();
    borderRadius_ = popupTheme->GetRadius().GetX();
    border_.SetBorderRadius(popupTheme->GetRadius());
    targetSpace_ = layoutProp->GetTargetSpace().value_or(popupTheme->GetTargetSpace());
    placement_ = layoutProp->GetPlacement().value_or(Placement::BOTTOM);
    scaledBubbleSpacing_ = static_cast<float>(popupTheme->GetBubbleSpacing().ConvertToPx());
    arrowHeight_ = static_cast<float>(popupTheme->GetArrowHeight().ConvertToPx());
}

OffsetF BubbleLayoutAlgorithm::GetChildPosition(const SizeF& childSize, const RefPtr<BubbleLayoutProperty>& layoutProp)
{
    float targetSpace = targetSpace_.ConvertToPx();
    OffsetF bottomPosition = OffsetF(targetOffset_.GetX() + (targetSize_.Width() - childSize.Width()) / 2.0,
        targetOffset_.GetY() + targetSize_.Height() + targetSpace + arrowHeight_);
    OffsetF topPosition = OffsetF(targetOffset_.GetX() + (targetSize_.Width() - childSize.Width()) / 2.0,
        targetOffset_.GetY() - childSize.Height() - targetSpace - arrowHeight_);
    OffsetF topArrowPosition;
    OffsetF bottomArrowPosition;
    InitArrowTopAndBottomPosition(topArrowPosition, bottomArrowPosition, topPosition, bottomPosition, childSize);

    OffsetF originOffset =
        GetPositionWithPlacement(childSize, topPosition, bottomPosition, topArrowPosition, bottomArrowPosition);
    OffsetF childPosition = originOffset;
    arrowPlacement_ = placement_;

    // Fit popup to screen range.
    ErrorPositionType errorType = GetErrorPositionType(childPosition, childSize);
    if (errorType == ErrorPositionType::NORMAL) {
        return childPosition;
    }
    // If childPosition is error, adjust bubble to bottom.
    if (placement_ != Placement::TOP || errorType == ErrorPositionType::TOP_LEFT_ERROR) {
        childPosition = FitToScreen(bottomPosition, childSize);
        arrowPosition_ = bottomArrowPosition;
        arrowPlacement_ = Placement::BOTTOM;
        if (GetErrorPositionType(childPosition, childSize) == ErrorPositionType::NORMAL) {
            return childPosition;
        }
    }
    // If childPosition is error, adjust bubble to top.
    childPosition = FitToScreen(topPosition, childSize);
    arrowPosition_ = topArrowPosition;
    arrowPlacement_ = Placement::TOP;
    if (GetErrorPositionType(childPosition, childSize) == ErrorPositionType::NORMAL) {
        return childPosition;
    }
    // If childPosition is error, adjust bubble to origin position.
    arrowPlacement_ = placement_;
    arrowPosition_ = arrowPlacement_ == Placement::TOP ? topArrowPosition : bottomArrowPosition;
    return originOffset;
}

void BubbleLayoutAlgorithm::InitArrowTopAndBottomPosition(OffsetF& topArrowPosition, OffsetF& bottomArrowPosition,
    OffsetF& topPosition, OffsetF& bottomPosition, const SizeF& childSize)
{
    topArrowPosition =
        topPosition + OffsetF(std::max(padding_.Left().ConvertToPx(), border_.TopLeftRadius().GetX().ConvertToPx()) +
                                  BEZIER_WIDTH_HALF.ConvertToPx(),
                          childSize.Height() + arrowHeight_);
    bottomArrowPosition = bottomPosition + OffsetF(std::max(padding_.Left().ConvertToPx(),
                                                       border_.BottomLeftRadius().GetX().ConvertToPx()) +
                                                       BEZIER_WIDTH_HALF.ConvertToPx(), -arrowHeight_);
}

OffsetF BubbleLayoutAlgorithm::GetPositionWithPlacement(const SizeF& childSize, const OffsetF& topPosition,
    const OffsetF& bottomPosition, const OffsetF& topArrowPosition, const OffsetF& bottomArrowPosition)
{
    OffsetF childPosition;
    float bubbleSpacing = scaledBubbleSpacing_;
    float marginRight = 0.0f;
    float marginBottom = 0.0f;
    float marginTop = 0.0f;
    float marginLeft = 0.0f;
    float arrowHalfWidth = ARROW_WIDTH.ConvertToPx() / 2.0;
    float radius = borderRadius_.ConvertToPx();
    float targetSpace = targetSpace_.ConvertToPx();
    switch (placement_) {
        case Placement::TOP:
            childPosition = topPosition;
            arrowPosition_ = topArrowPosition;
            break;
        case Placement::TOP_LEFT:
            childPosition = OffsetF(targetOffset_.GetX() - marginRight,
                targetOffset_.GetY() - childSize.Height() - bubbleSpacing - marginBottom - targetSpace);
            arrowPosition_ = childPosition + OffsetF(radius + arrowHalfWidth, childSize.Height() + bubbleSpacing);
            break;
        case Placement::TOP_RIGHT:
            childPosition = OffsetF(targetOffset_.GetX() + targetSize_.Width() - childSize.Width() + marginLeft,
                targetOffset_.GetY() - childSize.Height() - targetSpace - bubbleSpacing - marginBottom);
            arrowPosition_ = childPosition + OffsetF(radius + arrowHalfWidth, childSize.Height() + bubbleSpacing);
            break;
        case Placement::BOTTOM:
            childPosition = bottomPosition;
            arrowPosition_ = bottomArrowPosition;
            break;
        case Placement::BOTTOM_LEFT:
            childPosition = OffsetF(targetOffset_.GetX() - marginRight,
                targetOffset_.GetY() + targetSize_.Height() + targetSpace + bubbleSpacing + marginTop);
            arrowPosition_ = childPosition + OffsetF(radius + arrowHalfWidth, -bubbleSpacing);
            break;
        case Placement::BOTTOM_RIGHT:
            childPosition = OffsetF(targetOffset_.GetX() + targetSize_.Width() - childSize.Width() + marginLeft,
                targetOffset_.GetY() + targetSize_.Height() + targetSpace + bubbleSpacing + marginTop);
            arrowPosition_ = childPosition + OffsetF(radius + arrowHalfWidth, -bubbleSpacing);
            break;
        case Placement::LEFT:
            childPosition =
                OffsetF(targetOffset_.GetX() - targetSpace - bubbleSpacing - childSize.Width() - marginRight,
                    targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0);
            arrowPosition_ = childPosition + OffsetF(childSize_.Width() + bubbleSpacing, radius + arrowHalfWidth);
            break;
        case Placement::LEFT_TOP:
            childPosition =
                OffsetF(targetOffset_.GetX() - targetSpace - bubbleSpacing - childSize.Width() - marginRight,
                    targetOffset_.GetY() - marginBottom);
            arrowPosition_ = childPosition + OffsetF(childSize_.Width() + bubbleSpacing, radius + arrowHalfWidth);
            break;
        case Placement::LEFT_BOTTOM:
            childPosition =
                OffsetF(targetOffset_.GetX() - targetSpace - bubbleSpacing - childSize.Width() - marginRight,
                    targetOffset_.GetY() + targetSize_.Height() - childSize.Height() - marginTop);
            arrowPosition_ = childPosition + OffsetF(childSize_.Width() + bubbleSpacing, radius + arrowHalfWidth);
            break;
        case Placement::RIGHT:
            childPosition =
                OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + bubbleSpacing + marginLeft,
                    targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0);
            arrowPosition_ = childPosition + OffsetF(-bubbleSpacing, radius + arrowHalfWidth);
            break;
        case Placement::RIGHT_TOP:
            childPosition =
                OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + bubbleSpacing + marginLeft,
                    targetOffset_.GetY() - marginBottom);
            arrowPosition_ = childPosition + OffsetF(-bubbleSpacing, radius + arrowHalfWidth);
            break;
        case Placement::RIGHT_BOTTOM:
            childPosition =
                OffsetF(targetOffset_.GetX() + targetSize_.Width() + targetSpace + bubbleSpacing + marginLeft,
                    targetOffset_.GetY() + targetSize_.Height() - childSize.Height() - marginTop);
            arrowPosition_ = childPosition + OffsetF(-bubbleSpacing, radius + arrowHalfWidth);
            break;
        default:
            break;
    }
    return childPosition;
}

BubbleLayoutAlgorithm::ErrorPositionType BubbleLayoutAlgorithm::GetErrorPositionType(
    const OffsetF& childOffset, const SizeF& childSize)
{
    auto horizonSpacing = static_cast<float>(HORIZON_SPACING_WITH_SCREEN.ConvertToPx());
    RectF validRegion =
        RectF(OffsetF(horizonSpacing, 0.0), OffsetF(selfSize_.Width() - horizonSpacing, selfSize_.Height()));
    PointF childPoint(childOffset.GetX(), childOffset.GetY());
    if (!validRegion.IsInRegion(childPoint)) {
        return ErrorPositionType::TOP_LEFT_ERROR;
    }
    if (!validRegion.IsInRegion(
            PointF(childOffset.GetX() + childSize.Width(), childOffset.GetY() + childSize.Height()))) {
        return ErrorPositionType::BOTTOM_RIGHT_ERROR;
    }
    return ErrorPositionType::NORMAL;
}

OffsetF BubbleLayoutAlgorithm::FitToScreen(const OffsetF& fitPosition, const SizeF& childSize)
{
    auto validation = GetErrorPositionType(fitPosition, childSize);
    if (validation == ErrorPositionType::NORMAL) {
        return fitPosition;
    }
    OffsetF childPosition = fitPosition;
    auto horizonSpacing = static_cast<float>(HORIZON_SPACING_WITH_SCREEN.ConvertToPx());
    if (validation == ErrorPositionType::TOP_LEFT_ERROR) {
        childPosition.SetX(horizonSpacing);
    } else {
        childPosition.SetX(selfSize_.Width() - childSize.Width() - horizonSpacing);
    }
    return childPosition;
}

void BubbleLayoutAlgorithm::UpdateChildPosition(const RefPtr<BubbleLayoutProperty>& layoutProp)
{
    auto enableArrow = layoutProp->GetEnableArrow().value_or(true);
    double arrowWidth = ARROW_WIDTH.ConvertToPx();
    double twoRadiusPx = borderRadius_.ConvertToPx() * 2.0;
    switch (arrowPlacement_) {
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            showArrow_ = GreatOrEqual(childSize_.Width() - twoRadiusPx, arrowWidth);
            if (!showArrow_ || !enableArrow) {
                childOffset_ += OffsetF(0.0, ARROW_HEIGHT.ConvertToPx());
            }
            break;
        case Placement::BOTTOM:
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            showArrow_ = GreatOrEqual(childSize_.Width() - twoRadiusPx, arrowWidth);
            if (!showArrow_ || !enableArrow) {
                childOffset_ += OffsetF(0.0, -ARROW_HEIGHT.ConvertToPx());
            }
            break;
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            showArrow_ = GreatOrEqual(childSize_.Height() - twoRadiusPx, arrowWidth);
            if (!showArrow_ || !enableArrow) {
                childOffset_ += OffsetF(ARROW_HEIGHT.ConvertToPx(), 0.0);
            }
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            showArrow_ = GreatOrEqual(childSize_.Height() - twoRadiusPx, arrowWidth);
            if (!showArrow_ || !enableArrow) {
                childOffset_ += OffsetF(-ARROW_HEIGHT.ConvertToPx(), 0.0);
            }
            break;
        default:
            break;
    }
}

void BubbleLayoutAlgorithm::UpdateTouchRegion()
{
    OffsetF topLeft;
    OffsetF bottomRight;
    switch (arrowPlacement_) {
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            topLeft = childOffset_;
            bottomRight = OffsetF(childSize_.Width(), targetSpace_.ConvertToPx() + childSize_.Height());
            if (showArrow_) {
                bottomRight += OffsetF(0.0, ARROW_HEIGHT.ConvertToPx());
            }
            break;
        case Placement::BOTTOM:
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            topLeft = childOffset_ + OffsetF(0.0, -targetSpace_.ConvertToPx());
            bottomRight = OffsetF(childSize_.Width(), targetSpace_.ConvertToPx() + childSize_.Height());
            if (showArrow_) {
                topLeft += OffsetF(0.0, -ARROW_HEIGHT.ConvertToPx());
                bottomRight += OffsetF(0.0, ARROW_HEIGHT.ConvertToPx());
            }
            break;
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            topLeft = childOffset_;
            bottomRight = OffsetF(targetSpace_.ConvertToPx() + childSize_.Width(), childSize_.Height());
            if (showArrow_) {
                bottomRight += OffsetF(ARROW_HEIGHT.ConvertToPx(), 0.0);
            }
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            topLeft = childOffset_ + OffsetF(-targetSpace_.ConvertToPx(), 0.0);
            bottomRight = OffsetF(targetSpace_.ConvertToPx() + childSize_.Width(), childSize_.Height());
            if (showArrow_) {
                topLeft += OffsetF(-ARROW_HEIGHT.ConvertToPx(), 0.0);
                bottomRight += OffsetF(ARROW_HEIGHT.ConvertToPx(), 0.0);
            }
            break;
        default:
            break;
    }
    touchRegion_ = RectF(topLeft, topLeft + bottomRight);
}

void BubbleLayoutAlgorithm::InitTargetSizeAndPosition(const RefPtr<BubbleLayoutProperty>& layoutProp)
{
    auto targetNode = FrameNode::GetFrameNode(targetTag_, targetNodeId_);
    CHECK_NULL_VOID(targetNode);
    auto geometryNode = targetNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    targetSize_ = geometryNode->GetFrameSize();
    auto showInSubWindow = layoutProp->GetShowInSubWindow().value_or(false);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto isContainerModal = pipelineContext->GetWindowModal() == WindowModal::CONTAINER_MODAL &&
                            pipelineContext->GetWindowManager()->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    targetOffset_ = targetNode->GetPaintRectOffset();
    // Show in SubWindow
    if (showInSubWindow) {
        auto overlayManager = pipelineContext->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        auto displayWindowOffset = layoutProp->GetDisplayWindowOffset().value_or(OffsetF());
        targetOffset_ += displayWindowOffset;
        auto currentSubwindow = SubwindowManager::GetInstance()->GetCurrentWindow();
        if (currentSubwindow) {
            auto subwindowRect = currentSubwindow->GetRect();
            targetOffset_ -= subwindowRect.GetOffset();
        }
        auto popupInfo = overlayManager->GetPopupInfo(targetNodeId_);
    } else if (isContainerModal) {
        // popup not show in subwindow need minus container modal.
        auto newOffsetX = targetOffset_.GetX() - static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx()) -
                          static_cast<float>(CONTENT_PADDING.ConvertToPx());
        auto newOffsetY = targetOffset_.GetY() - static_cast<float>(CONTAINER_TITLE_HEIGHT.ConvertToPx());
        targetOffset_.SetX(newOffsetX);
        targetOffset_.SetY(newOffsetY);
    }
}

} // namespace OHOS::Ace::NG

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

#include "core/components_ng/pattern/text/text_select_overlay.h"

#include <algorithm>
#include <optional>

#include "base/utils/utils.h"
#include "core/common/vibrator/vibrator_utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/manager/select_content_overlay/select_content_overlay_manager.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/pattern/text/text_layout_adapter.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float BOX_EPSILON = 0.5f;
}

bool TextSelectOverlay::PreProcessOverlay(const OverlayRequest& request)
{
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_RETURN(pipeline, false);
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, false);
    SetUsingMouse(textPattern->IsUsingMouse());
    SetEnableHandleLevel(true);
    textPattern->CalculateHandleOffsetAndShowOverlay();
    selectTextUseTopHandle = true;
    return true;
}

std::optional<SelectHandleInfo> TextSelectOverlay::GetFirstHandleInfo()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, std::nullopt);
    SelectHandleInfo handleInfo;
    handleInfo.paintRect = textPattern->GetTextSelector().firstHandle;
    handleInfo.isShow = CheckAndAdjustHandle(handleInfo.paintRect);

    auto localPaintRect = textPattern->GetTextSelector().firstHandle;
    localPaintRect.SetOffset(localPaintRect.GetOffset() - GetPaintOffsetWithoutTransform());
    handleInfo.localPaintRect = localPaintRect;
    SetTransformPaintInfo(handleInfo, localPaintRect);
    return handleInfo;
}

std::optional<SelectHandleInfo> TextSelectOverlay::GetSecondHandleInfo()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, std::nullopt);
    SelectHandleInfo handleInfo;
    handleInfo.paintRect = textPattern->GetTextSelector().secondHandle;
    handleInfo.isShow = CheckAndAdjustHandle(handleInfo.paintRect);

    auto localPaintRect = textPattern->GetTextSelector().secondHandle;
    localPaintRect.SetOffset(localPaintRect.GetOffset() - GetPaintOffsetWithoutTransform());
    handleInfo.localPaintRect = localPaintRect;
    SetTransformPaintInfo(handleInfo, localPaintRect);
    return handleInfo;
}

RectF TextSelectOverlay::GetFirstHandleLocalPaintRect()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, RectF());
    auto localPaintRect = textPattern->GetTextSelector().firstHandle;
    localPaintRect.SetOffset(localPaintRect.GetOffset() - GetPaintOffsetWithoutTransform());
    return localPaintRect;
}

RectF TextSelectOverlay::GetSecondHandleLocalPaintRect()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, RectF());
    auto localPaintRect = textPattern->GetTextSelector().secondHandle;
    localPaintRect.SetOffset(localPaintRect.GetOffset() - GetPaintOffsetWithoutTransform());
    return localPaintRect;
}

bool TextSelectOverlay::CheckAndAdjustHandle(RectF& paintRect)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, false);
    auto host = textPattern->GetHost();
    CHECK_NULL_RETURN(host, false);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    auto clip = false;
    if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        clip = true;
    }
    if (!renderContext->GetClipEdge().value_or(clip)) {
        if (handleLevelMode_ == HandleLevelMode::EMBED) {
            return true;
        }
        auto contentRect = textPattern->GetTextContentRect();
        auto localPaintRect = paintRect;
        localPaintRect.SetOffset(localPaintRect.GetOffset() - GetPaintOffsetWithoutTransform());
        localPaintRect.SetOffset(OffsetF(localPaintRect.GetX() + localPaintRect.Width() / 2.0f, localPaintRect.GetY()));
        auto visibleContentRect = contentRect.CombineRectT(localPaintRect);
        visibleContentRect.SetOffset(visibleContentRect.GetOffset() + textPattern->GetTextPaintOffset());
        visibleContentRect = GetVisibleRect(host, visibleContentRect);
        return CheckAndAdjustHandleWithContent(visibleContentRect, paintRect);
    }
    auto contentRect = textPattern->GetTextContentRect();
    RectF visibleContentRect(contentRect.GetOffset() + textPattern->GetTextPaintOffset(), contentRect.GetSize());
    if (handleLevelMode_ == HandleLevelMode::OVERLAY) {
        visibleContentRect = GetVisibleRect(host, visibleContentRect);
    }
    return CheckAndAdjustHandleWithContent(visibleContentRect, paintRect);
}

bool TextSelectOverlay::CheckAndAdjustHandleWithContent(const RectF& visibleContentRect, RectF& paintRect)
{
    auto paintLeft = paintRect.GetX() + paintRect.Width() / 2.0f;
    PointF bottomPoint = { paintLeft, paintRect.Bottom() - BOX_EPSILON };
    PointF topPoint = { paintLeft, paintRect.Top() + BOX_EPSILON };
    bool bottomInRegion = visibleContentRect.IsInRegion(bottomPoint);
    bool topInRegion = visibleContentRect.IsInRegion(topPoint);
    if (!bottomInRegion && topInRegion) {
        paintRect.SetHeight(visibleContentRect.Bottom() - paintRect.Top());
    } else if (bottomInRegion && !topInRegion) {
        paintRect.SetHeight(paintRect.Bottom() - visibleContentRect.Top());
        paintRect.SetTop(visibleContentRect.Top());
    }
    return bottomInRegion || topInRegion;
}

bool TextSelectOverlay::CheckHandleVisible(const RectF& paintRect)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, false);
    auto host = textPattern->GetHost();
    CHECK_NULL_RETURN(host, false);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    auto clip = false;
    if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        clip = true;
    }
    if (!renderContext->GetClipEdge().value_or(clip)) {
        return true;
    }
    auto contentRect = textPattern->GetTextContentRect();
    RectF visibleContentRect(contentRect.GetOffset() + textPattern->GetTextPaintOffset(), contentRect.GetSize());
    visibleContentRect = GetVisibleRect(host, visibleContentRect);
    PointF bottomPoint = { paintRect.Left(), paintRect.Bottom() - BOX_EPSILON };
    PointF topPoint = { paintRect.Left(), paintRect.Top() + BOX_EPSILON };
    return visibleContentRect.IsInRegion(bottomPoint) && visibleContentRect.IsInRegion(topPoint);
}

void TextSelectOverlay::OnResetTextSelection()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    textPattern->ResetSelection();
}

void TextSelectOverlay::OnHandleMove(const RectF& handleRect, bool isFirst)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    auto host = textPattern->GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto contentRect = textPattern->GetTextContentRect();
    auto contentOffset = contentRect.GetOffset();
    auto localHandleOffset = handleRect.GetOffset();
    if (IsOverlayMode()) {
        contentOffset = contentOffset + GetPaintOffsetWithoutTransform();
        localHandleOffset -= GetPaintOffsetWithoutTransform();
    }
    localHandleOffset.SetY(localHandleOffset.GetY() + handleRect.Height() / 2.0f);
    textPattern->GetMagnifierController()->SetLocalOffset(localHandleOffset);
    auto handleOffset = handleRect.GetOffset();
    if (!selectTextUseTopHandle) {
        bool isUseHandleTop = (isFirst != IsHandleReverse());
        handleOffset.SetY(handleOffset.GetY() + (isUseHandleTop ? 0 : handleRect.Height()));
    }

    auto clip = false;
    if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        clip = true;
    }
    if (renderContext->GetClipEdge().value_or(clip)) {
        handleOffset.SetX(
            std::clamp(handleOffset.GetX(), contentOffset.GetX(), contentOffset.GetX() + contentRect.Width()));
        handleOffset.SetY(
            std::clamp(handleOffset.GetY(), contentOffset.GetY(), contentOffset.GetY() + contentRect.Height()));
    }
    auto textPaintOffset = contentOffset - OffsetF(0.0f, std::min(textPattern->GetBaselineOffset(), 0.0f));
    handleOffset -= textPaintOffset;
    // the handle position is calculated based on the middle of the handle height.
    UpdateSelectorOnHandleMove(handleOffset, isFirst);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    auto overlayManager = GetManager<SelectContentOverlayManager>();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->MarkInfoChange(DIRTY_SELECT_TEXT);
}

void TextSelectOverlay::UpdateSelectorOnHandleMove(const OffsetF& handleOffset, bool isFirstHandle)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    auto currentHandleIndex = textPattern->GetHandleIndex(Offset(handleOffset.GetX(), handleOffset.GetY()));
    if (isFirstHandle) {
        if (textPattern->GetTextSelector().baseOffset != currentHandleIndex) {
            VibratorUtils::StartVibraFeedback("slide");
        }
        textPattern->HandleSelectionChange(currentHandleIndex, textPattern->GetTextSelector().destinationOffset);
    } else {
        if (textPattern->GetTextSelector().destinationOffset != currentHandleIndex) {
            VibratorUtils::StartVibraFeedback("slide");
        }
        textPattern->HandleSelectionChange(textPattern->GetTextSelector().baseOffset, currentHandleIndex);
    }
}

void TextSelectOverlay::OnHandleMoveDone(const RectF& rect, bool isFirst)
{
    BaseTextSelectOverlay::OnHandleMoveDone(rect, isFirst);
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    if (textPattern->GetMagnifierController()) {
        textPattern->GetMagnifierController()->RemoveMagnifierFrameNode();
    }
    textPattern->SetTextResponseType(TextResponseType::LONG_PRESS);
    auto textSelector = textPattern->GetTextSelector();
    textPattern->UpdateSelectionSpanType(textSelector.GetTextStart(), textSelector.GetTextEnd());
    textPattern->CalculateHandleOffsetAndShowOverlay();
    auto overlayManager = GetManager<SelectContentOverlayManager>();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->MarkInfoChange((isFirst ? DIRTY_FIRST_HANDLE : DIRTY_SECOND_HANDLE) | DIRTY_SELECT_AREA |
                                   DIRTY_SELECT_TEXT | DIRTY_COPY_ALL_ITEM);
    if (textPattern->CheckSelectedTypeChange()) {
        CloseOverlay(false, CloseReason::CLOSE_REASON_NORMAL);
        ProcessOverlay({ .animation = true });
    }
    overlayManager->ShowOptionMenu();
    overlayManager->SetHandleCircleIsShow(isFirst, true);
    auto host = textPattern->GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

std::string TextSelectOverlay::GetSelectedText()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, "");
    auto start = textPattern->GetTextSelector().GetTextStart();
    auto end = textPattern->GetTextSelector().GetTextEnd();
    return textPattern->GetSelectedText(start, end);
}

RectF TextSelectOverlay::GetSelectArea()
{
    auto pattern = GetPattern<TextPattern>();
    RectF res;
    CHECK_NULL_RETURN(pattern, res);
    auto selectRects = pattern->GetTextBoxes();
    auto textPaintOffset = GetPaintOffsetWithoutTransform();
    if (selectRects.empty()) {
        res.SetOffset(res.GetOffset() + textPaintOffset);
        GetSelectAreaFromHandle(res);
        return res;
    }
    auto contentRect = pattern->GetTextContentRect();
    auto textRect = pattern->GetTextRect();
    res = MergeSelectedBoxes(selectRects, contentRect, textRect, textPaintOffset);
    RectF visibleContentRect(contentRect.GetOffset() + textPaintOffset, contentRect.GetSize());
    visibleContentRect = GetVisibleRect(pattern->GetHost(), visibleContentRect);
    auto intersectRect = res.IntersectRectT(visibleContentRect);
    if (hasTransform_) {
        intersectRect.SetOffset(intersectRect.GetOffset() - textPaintOffset);
        GetGlobalRectWithTransform(intersectRect);
    }
    return intersectRect;
}

void TextSelectOverlay::GetSelectAreaFromHandle(RectF& rect)
{
    auto firstHandle = GetFirstHandleInfo();
    if (firstHandle) {
        auto firstRect = firstHandle->paintRect;
        if (hasTransform_) {
            firstRect.SetOffset(firstRect.GetOffset() - GetPaintOffsetWithoutTransform());
            GetGlobalRectWithTransform(firstRect);
        }
        rect = firstRect;
        return;
    }
    auto secondHandle = GetSecondHandleInfo();
    if (secondHandle) {
        auto secondRect = secondHandle->paintRect;
        if (hasTransform_) {
            secondRect.SetOffset(secondRect.GetOffset() - GetPaintOffsetWithoutTransform());
            GetGlobalRectWithTransform(secondRect);
        }
        rect = secondRect;
    }
}

void TextSelectOverlay::OnUpdateMenuInfo(SelectMenuInfo& menuInfo, SelectOverlayDirtyFlag dirtyFlag)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    menuInfo.showCopyAll = !textPattern->IsSelectAll();
    menuInfo.showCopy = !textPattern->GetTextSelector().SelectNothing();
    if (dirtyFlag == DIRTY_COPY_ALL_ITEM) {
        return;
    }
    menuInfo.menuIsShow = IsShowMenu();
    menuInfo.showCut = false;
    menuInfo.showPaste = false;
}

void TextSelectOverlay::OnUpdateSelectOverlayInfo(SelectOverlayInfo& overlayInfo, int32_t requestCode)
{
    BaseTextSelectOverlay::OnUpdateSelectOverlayInfo(overlayInfo, requestCode);
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    textPattern->CopySelectionMenuParams(overlayInfo);
    OnUpdateOnCreateMenuCallback(overlayInfo);
}

void TextSelectOverlay::OnMenuItemAction(OptionMenuActionId id, OptionMenuType type)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    switch (id) {
        case OptionMenuActionId::COPY:
            textPattern->HandleOnCopy();
            break;
        case OptionMenuActionId::SELECT_ALL:
            textPattern->HandleOnSelectAll();
            break;
        default:
            TAG_LOGI(AceLogTag::ACE_TEXT, "Unsupported menu option id %{public}d", id);
            break;
    }
}

void TextSelectOverlay::OnCloseOverlay(OptionMenuType menuType, CloseReason reason, RefPtr<OverlayInfo> info)
{
    BaseTextSelectOverlay::OnCloseOverlay(menuType, reason, info);
    if (reason == CloseReason::CLOSE_REASON_HOLD_BY_OTHER || reason == CloseReason::CLOSE_REASON_TOOL_BAR ||
        reason == CloseReason::CLOSE_REASON_BACK_PRESSED) {
        auto textPattern = GetPattern<TextPattern>();
        CHECK_NULL_VOID(textPattern);
        textPattern->ResetSelection();
    }
}

void TextSelectOverlay::OnHandleGlobalTouchEvent(SourceType sourceType, TouchType touchType)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    if (IsTouchUp(sourceType, touchType) || IsMouseClickDown(sourceType, touchType)) {
        CloseOverlay(false, CloseReason::CLOSE_REASON_CLICK_OUTSIDE);
        textPattern->ResetSelection();
    }
}

void TextSelectOverlay::OnAncestorNodeChanged(FrameNodeChangeInfoFlag flag)
{
    if (IsAncestorNodeGeometryChange(flag)) {
        auto textPattern = GetPattern<TextPattern>();
        CHECK_NULL_VOID(textPattern);
        textPattern->UpdateParentGlobalOffset();
        textPattern->CalculateHandleOffsetAndShowOverlay();
        UpdateAllHandlesOffset();
    }
    BaseTextSelectOverlay::OnAncestorNodeChanged(flag);
}

void TextSelectOverlay::OnHandleLevelModeChanged(HandleLevelMode mode)
{
    if (handleLevelMode_ != mode && mode == HandleLevelMode::OVERLAY) {
        auto textPattern = GetPattern<TextPattern>();
        CHECK_NULL_VOID(textPattern);
        textPattern->UpdateParentGlobalOffset();
        textPattern->CalculateHandleOffsetAndShowOverlay();
        UpdateAllHandlesOffset();
    }
    BaseTextSelectOverlay::OnHandleLevelModeChanged(mode);
}

void TextSelectOverlay::OnHandleMoveStart(bool isFirst)
{
    auto manager = GetManager<SelectContentOverlayManager>();
    CHECK_NULL_VOID(manager);
    manager->SetHandleCircleIsShow(isFirst, false);
}

void TextSelectOverlay::OnOverlayClick(const GestureEvent& event, bool isFirst)
{
    if (!IsSingleHandle()) {
        ToggleMenu();
    }
}
} // namespace OHOS::Ace::NG

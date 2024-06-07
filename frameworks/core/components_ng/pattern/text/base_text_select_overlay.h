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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_SELECT_OVERLAY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_SELECT_OVERLAY_H
#include <optional>

#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/manager/select_content_overlay/select_overlay_callback.h"
#include "core/components_ng/manager/select_content_overlay/select_overlay_holder.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/event/ace_events.h"
#include "core/event/touch_event.h"

namespace OHOS::Ace::NG {
struct OverlayRequest {
    bool menuIsShow = true;
    bool hideHandle = false;
    bool animation = false;
    bool hideHandleLine = false;
    int32_t requestCode = 0;
};

class BaseTextSelectOverlay : public SelectOverlayHolder, public SelectOverlayCallback {
    DECLARE_ACE_TYPE(BaseTextSelectOverlay, SelectOverlayHolder, SelectOverlayCallback);

public:
    explicit BaseTextSelectOverlay(const WeakPtr<TextBase>& textBase) : hostTextBase_(textBase) {}
    ~BaseTextSelectOverlay() = default;

    static RectF GetVisibleRect(const RefPtr<FrameNode>& node, const RectF& visibleRect);

    template<class T>
    RefPtr<T> GetPattern()
    {
        return DynamicCast<T>(hostTextBase_.Upgrade());
    }

    void ProcessOverlay(const OverlayRequest& request = OverlayRequest());
    void ProcessOverlayOnAreaChanged(const OverlayRequest& request = OverlayRequest());
    virtual bool PreProcessOverlay(const OverlayRequest& request)
    {
        return true;
    }
    virtual void AfterCloseOverlay() {}

    // override SelectOverlayHolder
    RefPtr<FrameNode> GetOwner() override;
    void OnHandleGlobalTouchEvent(SourceType sourceType, TouchType touchType) override;
    bool CheckTouchInHostNode(const PointF& touchPoint) override;
    void OnUpdateSelectOverlayInfo(SelectOverlayInfo& overlayInfo, int32_t requestCode) override;
    bool CheckRestartHiddenHandleTask(int32_t requestCode) override;
    RefPtr<SelectOverlayCallback> GetCallback() override
    {
        return AceType::Claim(this);
    }
    // override SelectOverlayHolder end

    // common method
    bool SelectOverlayIsOn();
    bool SelectOverlayIsCreating();
    void CloseOverlay(bool animation, CloseReason reason);
    void ToggleMenu();
    void ShowMenu();
    void HideMenu(bool noAnimation = false);
    void DisableMenu();
    void EnableMenu();
    void UpdateAllHandlesOffset();
    void UpdateFirstHandleOffset();
    void UpdateSecondHandleOffset();
    bool IsShowMouseMenu();
    bool IsCurrentMenuVisibile();
    bool IsHandleReverse();
    void RemoveSelectionHoldCallback();
    void SetSelectionHoldCallback();

    bool IsSingleHandle()
    {
        return isSingleHandle_;
    }

    bool IsUsingMouse()
    {
        return isUsingMouse_;
    }

    void SetIsSingleHandle(bool isSingleHandle)
    {
        isSingleHandle_ = isSingleHandle;
    }

    void SetUsingMouse(bool isUsingMouse)
    {
        isUsingMouse_ = isUsingMouse;
    }

    void SetShowPaste(bool isShowPaste)
    {
        isShowPaste_ = isShowPaste;
    }

    bool IsShowPaste()
    {
        return isShowPaste_;
    }

    void SetMenuIsShow(bool isShowMenu)
    {
        isShowMenu_ = isShowMenu;
    }

    bool IsShowMenu()
    {
        return isShowMenu_;
    }

    void SetIsShowHandleLine(bool isShowHandleLine)
    {
        isShowHandleLine_ = isShowHandleLine;
    }

    void SetMouseMenuOffset(const OffsetF& offset)
    {
        mouseMenuOffset_ = offset;
    }

    // common virtual methods.
    virtual RectF GetVisibleContentRect();
    virtual bool CheckHandleVisible(const RectF& paintRect) = 0;
    virtual void OnResetTextSelection() {}

    virtual bool IsAcceptResetSelectionEvent(SourceType sourceType, TouchType touchType);

    bool HasRenderTransform()
    {
        UpdateTransformFlag();
        return hasTransform_;
    }
    // transformed global point to original local point.
    void RevertLocalPointWithTransform(OffsetF& point);
    // original local point to transformed global point.
    void GetGlobalPointsWithTransform(std::vector<OffsetF>& points);
    // original local rect to transformd global rect.
    void GetGlobalRectWithTransform(RectF& rect);
    std::vector<OffsetF> GetGlobalRectVertexWithTransform(const RectF& rect, float extendValue = 0.0f);
    // original local point to transformed local point.
    void GetLocalPointsWithTransform(std::vector<OffsetF>& localPoints);
    void GetLocalPointWithTransform(OffsetF& localPoint);
    // original local rect to transformed local rect.
    void GetLocalRectWithTransform(RectF& localRect);

    OffsetF GetPaintOffsetWithoutTransform();
    RectF GetPaintRectWithTransform();
    OffsetF GetPaintRectOffsetWithTransform();
    RectF GetVisibleContentRectWithTransform(float epsilon);
    bool CheckHandleIsVisibleWithTransform(const OffsetF& startPoint, const OffsetF& endPoint, float epsilon);
    bool IsPointInRect(const OffsetF& point, const OffsetF& leftBottom, const OffsetF& rightBottom,
        const OffsetF& rightTop, const OffsetF& leftTop);
    void OnTouchTestHit(SourceType hitTestType);

    void SetScrollableParentCallback();
    void ResetScrollableParentCallback();
    virtual void OnParentScrollStart();
    virtual void OnParentScrollEnd() {}
    virtual void OnParentScrolling();

    void SetkeyBoardChangeCallback();
    void RemoveKeyboardChangeCallback();
    virtual void OnKeyboardChanged(bool isKeyboardShow);

protected:
    RectF MergeSelectedBoxes(
        const std::vector<RectF>& boxes, const RectF& contentRect, const RectF& textRect, const OffsetF& paintOffset);
    bool IsMouseClickDown(SourceType sourceType, TouchType touchType)
    {
        return sourceType == SourceType::MOUSE && touchType == TouchType::DOWN;
    }

    bool IsTouchUp(SourceType sourceType, TouchType touchType)
    {
        return (sourceType == SourceType::TOUCH || sourceType == SourceType::TOUCH_PAD) && touchType == TouchType::UP;
    }

    RectF ConvertPaintInfoToRect(const SelectHandlePaintInfo& paintInfo);
    void SetTransformPaintInfo(SelectHandleInfo& handleInfo, const RectF& localHandleRect);
    std::optional<RectF> GetAncestorNodeViewPort();
    std::optional<OverlayRequest> latestReqeust_;
    bool hasTransform_ = false;

private:
    void UpdateTransformFlag();
    void FindScrollableParentAndSetCallback(const RefPtr<FrameNode>& host);
    void RegisterParentScrollCallback(const RefPtr<FrameNode>& host, int32_t parentId);
    bool isSingleHandle_ = false;
    bool isShowPaste_ = false;
    bool isShowMenu_ = true;
    bool isShowHandleLine_ = true;
    bool isUsingMouse_ = false;
    OffsetF mouseMenuOffset_;
    WeakPtr<TextBase> hostTextBase_;
    bool hasScrollableParent_ = true;
    std::vector<int32_t> scrollableParentIds_;
    bool hasTouchTestHit_ = false;
    bool resetSelectionHitTest_ = false;
    bool accepResetSelectionHitTest_ = false;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_SELECT_OVERLAY_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_MANAGER_H

#include <cstdint>
#include <string>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "base/window/drag_window.h"
#include "core/common/interaction/interaction_data.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/drag_drop/drag_drop_proxy.h"
#include "core/gestures/velocity_tracker.h"

namespace OHOS::Ace {
class UnifiedData;
}
namespace OHOS::Ace::NG {
enum class DragDropMgrState : int32_t {
    IDLE,
    ABOUT_TO_PREVIEW,
    DRAGGING
};

class ACE_EXPORT DragDropManager : public virtual AceType {
    DECLARE_ACE_TYPE(DragDropManager, AceType);

public:
    DragDropManager() = default;
    ~DragDropManager() override = default;

    RefPtr<DragDropProxy> CreateAndShowDragWindow(const RefPtr<PixelMap>& pixelMap, const GestureEvent& info);
    RefPtr<DragDropProxy> CreateAndShowDragWindow(const RefPtr<UINode>& customNode, const GestureEvent& info);
    RefPtr<DragDropProxy> CreateTextDragDropProxy();

    void AddDragFrameNode(int32_t id, const WeakPtr<FrameNode>& dragFrameNode)
    {
        dragFrameNodes_.try_emplace(id, dragFrameNode);
    }

    void RemoveDragFrameNode(int32_t id)
    {
        dragFrameNodes_.erase(id);
        gridDragFrameNodes_.erase(id);
        listDragFrameNodes_.erase(id);
        textFieldDragFrameNodes_.erase(id);
    }

    void AddGridDragFrameNode(int32_t id, const WeakPtr<FrameNode>& dragFrameNode)
    {
        gridDragFrameNodes_.try_emplace(id, dragFrameNode);
    }

    void AddListDragFrameNode(int32_t id, const WeakPtr<FrameNode>& dragFrameNode)
    {
        listDragFrameNodes_.try_emplace(id, dragFrameNode);
    }

    void AddTextFieldDragFrameNode(int32_t id, const WeakPtr<FrameNode>& dragFrameNode)
    {
        textFieldDragFrameNodes_.try_emplace(id, dragFrameNode);
    }

    void SetEventStrictReportingEnabled(bool dragEventStrictReportingEnabled)
    {
        eventStrictReportingEnabled_ = dragEventStrictReportingEnabled;
    }

    bool IsEventStrictReportingEnabled()
    {
        return eventStrictReportingEnabled_;
    }

    void UpdateDragWindowPosition(int32_t globalX, int32_t globalY);
    void OnDragStart(const Point& point);
    void OnDragStart(const Point& point, const RefPtr<FrameNode>& frameNode);
    void OnDragMove(const PointerEvent& pointerEvent, const std::string& extraInfo);
    void OnDragEnd(const PointerEvent& pointerEvent, const std::string& extraInfo);
    void DoDropAction(const RefPtr<FrameNode>& dragFrameNode, const Point& point,
        const RefPtr<UnifiedData>& unifiedData, const std::string& udKey);
    void RequestDragSummaryInfoAndPrivilege();
    RefPtr<UnifiedData> RequestUDMFDataWithUDKey(const std::string& udKey);
    void TryGetDataBackGround(
        const RefPtr<FrameNode>& dragFrameNode, const Point& point, const std::string& udKey, int32_t count = 0);
    void OnDragDrop(RefPtr<OHOS::Ace::DragEvent>& event, const RefPtr<FrameNode>& dragFrameNode, const Point& point);
    void ResetDragDropStatus(const Point& point, const DragDropRet& dragDropRet, int32_t windowId);
    bool CheckRemoteData(
        const RefPtr<FrameNode>& dragFrameNode, const PointerEvent& pointerEvent, const std::string& udKey);
    void OnDragMoveOut(const PointerEvent& pointerEvent);
    void OnTextDragEnd(float globalX, float globalY, const std::string& extraInfo);
    void onDragCancel();
    void OnItemDragStart(float globalX, float globalY, const RefPtr<FrameNode>& frameNode);
    void OnItemDragMove(float globalX, float globalY, int32_t draggedIndex, DragType dragType);
    void OnItemDragEnd(float globalX, float globalY, int32_t draggedIndex, DragType dragType);
    void onItemDragCancel();
    void AddDataToClipboard(const std::string& extraInfo);
    void GetExtraInfoFromClipboard(std::string& extraInfo);
    void RestoreClipboardData();
    void DestroyDragWindow();
    void CancelItemDrag();
    std::string GetExtraInfo();
    void SetExtraInfo(const std::string& extraInfo);
    void ClearExtraInfo();
    float GetWindowScale() const;
    void UpdateDragStyle(const DragCursorStyleCore& dragCursorStyleCore = DragCursorStyleCore::DEFAULT);
    void UpdateDragAllowDrop(const RefPtr<FrameNode>& dragFrameNode, const DragBehavior dragBehavior);
    void RequireSummary();
    void ClearSummary();
    void SetSummaryMap(const std::map<std::string, int64_t>& summaryMap)
    {
        summaryMap_ = summaryMap;
    }
    void ResetRecordSize(uint32_t recordSize = 0);
    uint32_t GetRecordSize() const;
    Rect GetDragWindowRect(const Point& point);
    RefPtr<DragDropProxy> CreateFrameworkDragDropProxy();
    void UpdatePixelMapPosition(int32_t globalX, int32_t globalY);
    void HideDragPreviewOverlay();
    bool IsMsdpDragging() const;
    void UpdateDragEvent(RefPtr<OHOS::Ace::DragEvent>& event, const Point& point);
    void UpdateNotifyDragEvent(
        RefPtr<NotifyDragEvent>& notifyEvent, const Point& point, const DragEventType dragEventType);
    bool CheckDragDropProxy(int64_t id) const;
    void FireOnEditableTextComponent(const RefPtr<FrameNode>& frameNode, DragEventType type);
    void FireOnDragLeave(const RefPtr<FrameNode>& preTargetFrameNode_, const Point& point,
        const std::string& extraInfo);

    bool IsWindowConsumed() const
    {
        return isWindowConsumed_;
    }

    void SetIsWindowConsumed(bool consumed)
    {
        isWindowConsumed_ = consumed;
    }

    bool IsDragged() const
    {
        return isDragged_;
    }

    void SetIsDragged(bool isDragged)
    {
        if (isDragged && isDragged_ != isDragged && notifyInDraggedCallback_) {
            notifyInDraggedCallback_();
        }
        isDragged_ = isDragged;
    }

    void SetIsDragCancel(bool isDragCancel)
    {
        isDragCancel_ = isDragCancel;
    }

    void SetIsMouseDrag(bool isMouseDragged)
    {
        isMouseDragged_ = isMouseDragged;
    }

    void SetIsDragWindowShow(bool isDragWindowShow)
    {
        isDragWindowShow_ = isDragWindowShow;
    }

    bool IsDragWindowShow() const
    {
        return isDragWindowShow_;
    }

    void SetPreviewRect(const Rect& rect)
    {
        previewRect_ = rect;
    }

    Rect GetPreviewRect() const
    {
        return previewRect_;
    }

    void SetDragCursorStyleCore(DragCursorStyleCore dragCursorStyleCore)
    {
        dragCursorStyleCore_ = dragCursorStyleCore;
    }

    RefPtr<FrameNode> FindTargetInChildNodes(const RefPtr<UINode> parentNode,
        std::vector<RefPtr<FrameNode>> hitFrameNodes, bool findDrop);

    std::unordered_set<int32_t> FindHitFrameNodes(const Point& point);

    void UpdateDragListener(const Point& point);

    void NotifyDragRegisterFrameNode(std::unordered_map<int32_t, WeakPtr<FrameNode>> nodes, DragEventType dragEventType,
        RefPtr<NotifyDragEvent>& notifyEvent);

    void RegisterDragStatusListener(int32_t nodeId, const WeakPtr<FrameNode>& node)
    {
        auto ret = nodesForDragNotify_.try_emplace(nodeId, node);
        if (!ret.second) {
            nodesForDragNotify_[nodeId] = node;
        }
    }

    void UnRegisterDragStatusListener(int32_t nodeId)
    {
        nodesForDragNotify_.erase(nodeId);
    }

    void SetNotifyInDraggedCallback(const std::function<void(void)>& callback)
    {
        notifyInDraggedCallback_ = callback;
    }

    bool IsDragging() const
    {
        return dragDropState_ == DragDropMgrState::DRAGGING;
    }

    bool IsItemDragging() const
    {
        return dragDropState_ == DragDropMgrState::DRAGGING && draggedGridFrameNode_ != nullptr;
    }

    bool IsAboutToPreview() const
    {
        return dragDropState_ == DragDropMgrState::ABOUT_TO_PREVIEW;
    }

    void ResetDragging(DragDropMgrState dragDropMgrState = DragDropMgrState::IDLE)
    {
        dragDropState_ = dragDropMgrState;
    }

    void SetDraggingPressedState(bool pointerPressed)
    {
        draggingPressedState_ = pointerPressed;
    }

    bool IsDraggingPressed(int32_t currentPointerId) const
    {
        if (currentPointerId_ == currentPointerId) {
            return draggingPressedState_;
        }
        return false;
    }

    bool IsSameDraggingPointer(int32_t currentPointerId) const
    {
        return currentPointerId_ == currentPointerId;
    }

    void SetDraggingPointer(int32_t currentPointerId)
    {
        currentPointerId_ = currentPointerId;
    }

    static inline bool IsEditableTextComponent(const std::string& frameTag)
    {
        if (frameTag != V2::TEXTINPUT_ETS_TAG && frameTag != V2::TEXTAREA_ETS_TAG &&
            frameTag != V2::RICH_EDITOR_ETS_TAG && frameTag != V2::SEARCH_Field_ETS_TAG) {
            return false;
        }
        return true;
    }

    typedef struct DragPreviewInfo {
        double width { 0.0 };
        double height { 0.0 };
        double maxWidth { 0.0 };
        double scale { -1.0 };
        RefPtr<FrameNode> imageNode { nullptr };
        RefPtr<FrameNode> textNode { nullptr };
    } DragPreviewInfo;
    bool IsNeedScaleDragPreview();
    void DoDragMoveAnimate(const PointerEvent& pointerEvent);
    void DoDragStartAnimation(const RefPtr<OverlayManager>& overlayManager, const GestureEvent& event);
    void SetDragResult(const DragNotifyMsgCore& notifyMessage, const RefPtr<OHOS::Ace::DragEvent>& dragEvent);
    void SetDragBehavior(const DragNotifyMsgCore& notifyMessage, const RefPtr<OHOS::Ace::DragEvent>& dragEvent);
    void ResetDragPreviewInfo()
    {
        info_ = DragPreviewInfo();
    }

    void SetPrepareDragFrameNode(const WeakPtr<FrameNode>& prepareDragFrameNode)
    {
        prepareDragFrameNode_ = prepareDragFrameNode;
    }

    const WeakPtr<FrameNode> GetPrepareDragFrameNode() const
    {
        return prepareDragFrameNode_;
    }

    void SetPreDragStatus(PreDragStatus preDragStatus)
    {
        preDragStatus_ = preDragStatus;
    }

    PreDragStatus GetPreDragStatus() const
    {
        return preDragStatus_;
    }

    void ResetPullMoveReceivedForCurrentDrag(bool isPullMoveReceivedForCurrentDrag = false)
    {
        isPullMoveReceivedForCurrentDrag_ = isPullMoveReceivedForCurrentDrag;
    }

    bool IsPullMoveReceivedForCurrentDrag() const
    {
        return isPullMoveReceivedForCurrentDrag_;
    }

    static void UpdateGatherNodeAttr(const RefPtr<OverlayManager>& overlayManager,
        OffsetF gatherNodeCenter, float scale);
    static void UpdateGatherNodePosition(const RefPtr<OverlayManager>& overlayManager,
        const RefPtr<FrameNode>& imageNode);
    static void UpdateTextNodePosition(const RefPtr<FrameNode>& textNode, const Offset& localPoint);
    double CalcGatherNodeMaxDistanceWithPoint(const RefPtr<OverlayManager>& overlayManager, int32_t x, int32_t y);

    void SetPixelMapOffset(OffsetF pixelMapOffset) {
        pixelMapOffset_ = pixelMapOffset;
    }
    bool IsNeedDisplayInSubwindow();
    void ClearGatherPixelMap()
    {
        gatherPixelMaps_.clear();
    }

    void GetGatherPixelMap(const RefPtr<PixelMap>& pixelMap);
    void PushGatherPixelMap(DragDataCore& dragData, float scale);
    bool HasGatherNode()
    {
        return hasGatherNode_;
    }

    void SetHasGatherNode(bool hasGatherNode)
    {
        hasGatherNode_ = hasGatherNode;
    }

    void SetIsShowBadgeAnimation(bool isShowBadgeAnimation)
    {
        isShowBadgeAnimation_ = isShowBadgeAnimation;
    }

    bool IsShowBadgeAnimation()
    {
        return isShowBadgeAnimation_;
    }

    void SetBadgeNumber(int32_t badgeNumber)
    {
        badgeNumber_ = badgeNumber;
    }

    int32_t GetBadgeNumber() const
    {
        return badgeNumber_;
    }
    
    void SetIsTouchGatherAnimationPlaying(bool isTouchGatherAnimationPlaying)
    {
        isTouchGatherAnimationPlaying_ = isTouchGatherAnimationPlaying;
    }

    bool IsDragWithContextMenu() const
    {
        return isDragWithContextMenu_;
    }

    void SetIsDragWithContextMenu(bool isDragWithContextMenu)
    {
        isDragWithContextMenu_ = isDragWithContextMenu;
    }

private:
    double CalcDragPreviewDistanceWithPoint(
        const OHOS::Ace::Dimension& preserverHeight, int32_t x, int32_t y, const DragPreviewInfo& info);
    Offset CalcDragMoveOffset(
        const OHOS::Ace::Dimension& preserverHeight, int32_t x, int32_t y, const DragPreviewInfo& info);
    bool UpdateDragMovePositionFinished(
        bool needDoDragMoveAnimate, bool isMenuShow, const Offset& newOffset, int32_t containerId);
    bool GetDragPreviewInfo(
        const OHOS::Ace::RefPtr<OHOS::Ace::NG::OverlayManager>& overlayManager, DragPreviewInfo& dragPreviewInfo);
    bool IsNeedDoDragMoveAnimate(const PointerEvent& pointerEvent);
    RefPtr<FrameNode> FindDragFrameNodeByPosition(float globalX, float globalY, DragType dragType, bool findDrop);
    void FireOnDragEvent(
        const RefPtr<FrameNode>& frameNode, const Point& point, DragEventType type, const std::string& extraInfo);
    void FireOnItemDragEvent(const RefPtr<FrameNode>& frameNode, DragType dragType,
        const ItemDragInfo& itemDragInfo, DragEventType type, int32_t draggedIndex, int32_t insertIndex = 0);
    bool FireOnItemDropEvent(const RefPtr<FrameNode>& frameNode, DragType dragType,
        const ItemDragInfo& itemDragInfo, int32_t draggedIndex, int32_t insertIndex, bool isSuccess);
    int32_t GetItemIndex(const RefPtr<FrameNode>& frameNode, DragType dragType, float globalX, float globalY);
    void CreateDragWindow(const GestureEvent& info, uint32_t width, uint32_t height);
    RefPtr<FrameNode> CreateDragRootNode(const RefPtr<UINode>& customNode);
    void ClearVelocityInfo();
    void UpdateVelocityTrackerPoint(const Point& point, bool isEnd = false);
    void PrintDragFrameNode(const Point& point, const RefPtr<FrameNode>& dragFrameNode);
    void PrintGridDragFrameNode(const float globalX, const float globalY, const RefPtr<FrameNode>& dragFrameNode);
    void FireOnDragEventWithDragType(const RefPtr<EventHub>& eventHub, DragEventType type,
        RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams);
    void NotifyDragFrameNode(
        const Point& point, const DragEventType& dragEventType, const DragRet& dragRet = DragRet::DRAG_DEFAULT);
    void TransDragWindowToDragFwk(int32_t windowContainerId);
    void ResetDragDrop(int32_t windowId, const Point& point);
    bool isDistanceLimited(const Point& point);
    bool isTimeLimited(const PointerEvent& pointerEvent, const Point& point);
    bool ReachMoveLimit(const PointerEvent& pointerEvent, const Point& point);
    bool IsDropAllowed(const RefPtr<FrameNode>& dragFrameNode);
    bool IsUIExtensionShowPlaceholder(const RefPtr<NG::UINode>& node);

    std::map<int32_t, WeakPtr<FrameNode>> dragFrameNodes_;
    std::map<int32_t, WeakPtr<FrameNode>> gridDragFrameNodes_;
    std::map<int32_t, WeakPtr<FrameNode>> listDragFrameNodes_;
    std::map<int32_t, WeakPtr<FrameNode>> textFieldDragFrameNodes_;
    RefPtr<DragWindow> dragWindow_;
    RefPtr<FrameNode> draggedFrameNode_;
    RefPtr<FrameNode> preTargetFrameNode_;
    RefPtr<FrameNode> draggedGridFrameNode_;
    RefPtr<FrameNode> preGridTargetFrameNode_;
    RefPtr<FrameNode> dragWindowRootNode_;
    RefPtr<Clipboard> clipboard_;
    Point preMovePoint_ = Point(0, 0);
    uint64_t preTimeStamp_ = 0L;
    int64_t nanoPreDragMoveAnimationTime_ = 0L;
    WeakPtr<FrameNode> prepareDragFrameNode_;
    std::function<void(const std::string&)> addDataCallback_ = nullptr;
    std::function<void(const std::string&)> getDataCallback_ = nullptr;
    std::function<void(const std::string&)> deleteDataCallback_ = nullptr;
    std::string extraInfo_;
    std::unique_ptr<JsonValue> newData_ = nullptr;
    bool isDragCancel_ = false;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> nodesForDragNotify_;
    std::unordered_set<int32_t> parentHitNodes_;
    DragCursorStyleCore dragCursorStyleCore_ = DragCursorStyleCore::DEFAULT;
    std::map<std::string, int64_t> summaryMap_;
    uint32_t recordSize_ = 0;
    int64_t currentId_ = -1;
    int32_t currentPointerId_ = -1;
    bool draggingPressedState_ = false;

    std::function<void(void)> notifyInDraggedCallback_ = nullptr;
    bool isDragged_ = false;
    bool isMouseDragged_ = false;
    bool isWindowConsumed_ = false;
    bool isDragWindowShow_ = false;
    bool hasNotifiedTransformation_ = false;
    bool isPullMoveReceivedForCurrentDrag_ = false;
    VelocityTracker velocityTracker_;
    DragDropMgrState dragDropState_ = DragDropMgrState::IDLE;
    PreDragStatus preDragStatus_ = PreDragStatus::ACTION_DETECTING_STATUS;
    Rect previewRect_ { -1, -1, -1, -1 };
    DragPreviewInfo info_;
    bool isDragFwkShow_ { false };
    OffsetF pixelMapOffset_ {0.0f, 0.0f};
    OffsetF prePointerOffset_ {0.0f, 0.0f};
    std::vector<RefPtr<PixelMap>> gatherPixelMaps_;
    bool hasGatherNode_ = false;
    bool isTouchGatherAnimationPlaying_ = false;
    bool isShowBadgeAnimation_ = true;
    bool eventStrictReportingEnabled_ = false;
    int32_t badgeNumber_ = -1;
    bool isDragWithContextMenu_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(DragDropManager);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_MANAGER_H

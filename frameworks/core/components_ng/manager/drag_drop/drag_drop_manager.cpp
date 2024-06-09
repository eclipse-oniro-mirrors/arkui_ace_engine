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

#include "core/components_ng/manager/drag_drop/drag_drop_manager.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/point.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/utils.h"
#include "core/common/interaction/interaction_data.h"
#include "core/common/interaction/interaction_interface.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/pattern/grid/grid_event_hub.h"
#include "core/components_ng/pattern/list/list_event_hub.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "base/utils/time_util.h"

#include "base/geometry/rect.h"
#include "core/common/udmf/udmf_client.h"

namespace OHOS::Ace::NG {
namespace {
int64_t g_proxyId = 0;
constexpr float PIXELMAP_POSITION_WIDTH = 0.5f;
constexpr float PIXELMAP_POSITION_HEIGHT = 0.2f;
constexpr Dimension PIXELMAP_BORDER_RADIUS = 8.0_vp;
constexpr Dimension PRESERVE_HEIGHT = 8.0_vp;
constexpr float FIRST_PIXELMAP_OPACITY = 0.6f;
constexpr float SECOND_PIXELMAP_OPACITY = 0.3f;
constexpr float FIRST_PIXELMAP_ANGLE = 8.0f;
constexpr float SECOND_PIXELMAP_ANGLE = -8.0f;
constexpr int32_t FIRST_GATHER_PIXEL_MAP = 1;
constexpr int32_t SECOND_GATHER_PIXEL_MAP = 2;
constexpr float TOUCH_DRAG_PPIXELMAP_SCALE = 1.05f;
constexpr int32_t MAX_RETRY_TIMES = 3;
constexpr int32_t MAX_RETRY_DURATION = 800;
constexpr float MOVE_DISTANCE_LIMIT = 20.0f;
constexpr uint64_t MOVE_TIME_LIMIT = 6L;
constexpr int64_t DRAG_MOVE_TIME_THRESHOLD = 16 * 1000 * 1000;
constexpr float MAX_DISTANCE_TO_PRE_POINTER = 3.0f;
} // namespace

RefPtr<DragDropProxy> DragDropManager::CreateAndShowDragWindow(
    const RefPtr<PixelMap>& pixelMap, const GestureEvent& info)
{
    CHECK_NULL_RETURN(pixelMap, nullptr);
    SetIsDragged(true);
    isDragCancel_ = false;
#if !defined(PREVIEW)
    auto windowScale = GetWindowScale();
    pixelMap->Scale(windowScale, windowScale, AceAntiAliasingOption::HIGH);
    CreateDragWindow(info, pixelMap->GetWidth(), pixelMap->GetHeight());
    CHECK_NULL_RETURN(dragWindow_, nullptr);
    dragWindow_->DrawPixelMap(pixelMap);
#endif
    currentId_ = ++g_proxyId;
    return MakeRefPtr<DragDropProxy>(currentId_);
}

RefPtr<DragDropProxy> DragDropManager::CreateAndShowDragWindow(
    const RefPtr<UINode>& customNode, const GestureEvent& info)
{
    dragWindowRootNode_ = CreateDragRootNode(customNode);
    CHECK_NULL_RETURN(dragWindowRootNode_, nullptr);
    SetIsDragged(true);
    isDragCancel_ = false;
#if !defined(PREVIEW)
    if (dragWindow_) {
        return nullptr;
    }

    auto geometryNode = dragWindowRootNode_->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, nullptr);

    auto frameRect = geometryNode->GetFrameSize();
    CreateDragWindow(info, static_cast<uint32_t>(frameRect.Width()), static_cast<uint32_t>(frameRect.Height()));
    if (!dragWindow_) {
        return nullptr;
    }
    dragWindow_->DrawFrameNode(dragWindowRootNode_);
#endif
    currentId_ = ++g_proxyId;
    return MakeRefPtr<DragDropProxy>(currentId_);
}

RefPtr<DragDropProxy> DragDropManager::CreateTextDragDropProxy()
{
    SetIsDragged(true);
    isDragCancel_ = false;
    currentId_ = ++g_proxyId;
    return MakeRefPtr<DragDropProxy>(currentId_);
}

void DragDropManager::CreateDragWindow(const GestureEvent& info, uint32_t width, uint32_t height)
{
#if !defined(PREVIEW)
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto rect = pipeline->GetDisplayWindowRectInfo();
    auto windowScale = GetWindowScale();
    int32_t windowX = static_cast<int32_t>(info.GetGlobalPoint().GetX() * windowScale);
    int32_t windowY = static_cast<int32_t>(info.GetGlobalPoint().GetY() * windowScale);
    dragWindow_ = DragWindow::CreateDragWindow("APP_DRAG_WINDOW",
        windowX + rect.Left(), windowY + rect.Top(), width, height);
    if (dragWindow_) {
        dragWindow_->SetOffset(rect.Left(), rect.Top());
    }
#endif
}

RefPtr<FrameNode> DragDropManager::CreateDragRootNode(const RefPtr<UINode>& customNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);

    auto rootNode = FrameNode::CreateFrameNodeWithTree(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<RootPattern>());
    rootNode->SetActive(true);
    rootNode->SetHostRootId(pipeline->GetInstanceId());
    rootNode->SetHostPageId(-1);
    rootNode->AddChild(customNode);
    rootNode->AttachToMainTree(false, AceType::RawPtr(pipeline));
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    pipeline->FlushUITasks();
    return rootNode;
}

void DragDropManager::UpdateDragWindowPosition(int32_t globalX, int32_t globalY)
{
#if !defined(PREVIEW)
    CHECK_NULL_VOID(dragWindow_);
    dragWindow_->MoveTo(globalX, globalY);
#endif
}

void DragDropManager::UpdatePixelMapPosition(int32_t globalX, int32_t globalY)
{
    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    auto rootNode = pipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    if (manager->GetHasPixelMap()) {
        auto columnNode = AceType::DynamicCast<NG::FrameNode>(rootNode->GetLastChild());
        CHECK_NULL_VOID(columnNode);
        auto imageNode = AceType::DynamicCast<NG::FrameNode>(columnNode->GetLastChild());
        CHECK_NULL_VOID(imageNode);
        auto geometryNode = imageNode->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto width = geometryNode->GetFrameSize().Width();
        auto height = geometryNode->GetFrameSize().Height();
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        CHECK_NULL_VOID(draggedFrameNode_);
        auto hub = draggedFrameNode_->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(hub);
        if (!hub->GetTextDraggable()) {
            hub = columnNode->GetOrCreateGestureEventHub();
            CHECK_NULL_VOID(hub);
        }
        RefPtr<PixelMap> pixelMap = hub->GetPixelMap();
        CHECK_NULL_VOID(pixelMap);
        float scale = NearZero(width) ? 1.0f : pixelMap->GetWidth() / width;
        imageContext->UpdatePosition(NG::OffsetT<Dimension>(
            Dimension(globalX - width * PIXELMAP_POSITION_WIDTH * scale - width / 2.0f + width * scale / 2.0f),
            Dimension(globalY - height * PIXELMAP_POSITION_HEIGHT * scale - height / 2.0f + height * scale / 2.0f)));
        imageContext->OnModifyDone();
    }
}

void DragDropManager::HideDragPreviewOverlay()
{
    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemovePixelMap();
    manager->RemoveGatherNode();
    SubwindowManager::GetInstance()->HidePreviewNG();
}

RefPtr<FrameNode> DragDropManager::FindTargetInChildNodes(
    const RefPtr<UINode> parentNode, std::vector<RefPtr<FrameNode>> hitFrameNodes, bool findDrop)
{
    CHECK_NULL_RETURN(parentNode, nullptr);
    auto parentFrameNode = AceType::DynamicCast<FrameNode>(parentNode);
    if (parentFrameNode && (!parentFrameNode->IsActive() || !parentFrameNode->IsVisible())) {
        return nullptr;
    }
    auto children = parentFrameNode->GetFrameChildren();

    for (auto iter = children.rbegin(); iter != children.rend(); iter++) {
        auto child = iter->Upgrade();
        if (child == nullptr) {
            continue;
        }
        auto childNode = AceType::DynamicCast<UINode>(child);
        auto childFindResult = FindTargetInChildNodes(childNode, hitFrameNodes, findDrop);
        if (childFindResult) {
            return childFindResult;
        }
    }

    CHECK_NULL_RETURN(parentFrameNode, nullptr);
    for (auto iter : hitFrameNodes) {
        if (parentFrameNode == iter) {
            auto eventHub = parentFrameNode->GetEventHub<EventHub>();
            if (!eventHub) {
                continue;
            }
            if ((eventHub->HasOnDrop()) || (eventHub->HasOnItemDrop()) || (eventHub->HasCustomerOnDrop())) {
                return parentFrameNode;
            }
            if ((V2::UI_EXTENSION_COMPONENT_ETS_TAG == parentFrameNode->GetTag() ||
                V2::EMBEDDED_COMPONENT_ETS_TAG == parentFrameNode->GetTag()) &&
                (!IsUIExtensionShowPlaceholder(parentFrameNode))) {
                return parentFrameNode;
            }
        }
    }
    return nullptr;
}

RefPtr<FrameNode> DragDropManager::FindDragFrameNodeByPosition(
    float globalX, float globalY, DragType dragType, bool findDrop)
{
    std::map<int32_t, WeakPtr<FrameNode>> frameNodes;
    switch (dragType) {
        case DragType::COMMON:
            frameNodes = dragFrameNodes_;
            break;
        case DragType::GRID:
            frameNodes = gridDragFrameNodes_;
            break;
        case DragType::LIST:
            frameNodes = listDragFrameNodes_;
            break;
        case DragType::TEXT:
            frameNodes = textFieldDragFrameNodes_;
            break;
        default:
            break;
    }

    if (frameNodes.empty()) {
        return nullptr;
    }

    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto nanoTimestamp = pipeline->GetVsyncTime();
    PointF point(globalX, globalY);
    std::vector<RefPtr<FrameNode>> hitFrameNodes;
    for (auto iterOfFrameNode = frameNodes.begin(); iterOfFrameNode != frameNodes.end(); iterOfFrameNode++) {
        auto frameNode = iterOfFrameNode->second.Upgrade();
        if (!frameNode || !frameNode->IsVisible()) {
            continue;
        }
        auto geometryNode = frameNode->GetGeometryNode();
        if (!geometryNode) {
            continue;
        }
        auto globalFrameRect = geometryNode->GetFrameRect();
        globalFrameRect.SetOffset(frameNode->CalculateCachedTransformRelativeOffset(nanoTimestamp));
        if (globalFrameRect.IsInRegion(point)) {
            hitFrameNodes.push_back(frameNode);
        }
    }

    if (hitFrameNodes.empty()) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Cannot find targetNodes.");
        return nullptr;
    }
    auto manager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(manager, nullptr);
    auto rootNode = pipeline->GetRootElement();

    auto result = FindTargetInChildNodes(rootNode, hitFrameNodes, findDrop);
    if (result) {
        return result;
    }
    return nullptr;
}

bool DragDropManager::CheckDragDropProxy(int64_t id) const
{
    return currentId_ == id;
}

void DragDropManager::UpdateDragAllowDrop(const RefPtr<FrameNode>& dragFrameNode, const DragBehavior dragBehavior)
{
    if (!IsDropAllowed(dragFrameNode)) {
        UpdateDragStyle(DragCursorStyleCore::FORBIDDEN);
        return;
    }
    
    // drop allowed
    const auto& dragFrameNodeAllowDrop = dragFrameNode->GetAllowDrop();
    // special handling for no drag data present situation, always show as move
    if (dragFrameNodeAllowDrop.empty() || summaryMap_.empty()) {
        UpdateDragStyle(DragCursorStyleCore::MOVE);
        return;
    }

    //other case, check drag behavior
    switch (dragBehavior) {
        case DragBehavior::UNKNOWN: {
            // the application does not config the drag behavior, use copy as default
            UpdateDragStyle(DragCursorStyleCore::COPY);
            break;
        }
        case DragBehavior::MOVE: {
            UpdateDragStyle(DragCursorStyleCore::MOVE);
            break;
        }
        case DragBehavior::COPY: {
            UpdateDragStyle(DragCursorStyleCore::COPY);
            break;
        }
        default: {
            UpdateDragStyle(DragCursorStyleCore::COPY);
            break;
        }
    }
}

void DragDropManager::UpdateDragStyle(const DragCursorStyleCore& dragStyle)
{
    if (dragStyle != dragCursorStyleCore_) {
        dragCursorStyleCore_ = dragStyle;
        TAG_LOGI(AceLogTag::ACE_DRAG, "Update DragStyle to %{public}d.", dragCursorStyleCore_);
        InteractionInterface::GetInstance()->UpdateDragStyle(dragCursorStyleCore_);
    }
}

bool CheckParentVisible(const RefPtr<FrameNode>& frameNode)
{
    bool isVisible = frameNode->IsVisible();
    if (!isVisible) {
        return false;
    }
    auto parent = frameNode->GetParent();
    while (parent && parent->GetDepth() != 1) {
        auto parentFrameNode = AceType::DynamicCast<FrameNode>(parent);
        if (parentFrameNode && !parentFrameNode->IsVisible()) {
            isVisible = false;
            break;
        }
        parent = parent->GetParent();
    }
    return isVisible;
}

std::unordered_set<int32_t> DragDropManager::FindHitFrameNodes(const Point& point)
{
    std::unordered_set<int32_t> frameNodeList;
    for (auto iter = nodesForDragNotify_.begin(); iter != nodesForDragNotify_.end(); iter++) {
        auto frameNode = iter->second.Upgrade();
        if (!frameNode || !frameNode->IsActive() || !frameNode->IsVisible()) {
            continue;
        }
        auto geometryNode = frameNode->GetGeometryNode();
        if (!geometryNode) {
            continue;
        }
        auto globalFrameRect = geometryNode->GetFrameRect();
        globalFrameRect.SetOffset(frameNode->GetTransformRelativeOffset());
        if (globalFrameRect.IsInRegion(PointF(static_cast<float>(point.GetX()), static_cast<float>(point.GetY())))) {
            frameNodeList.emplace(frameNode->GetId());
        }
    }
    return frameNodeList;
}

void DragDropManager::UpdateDragListener(const Point& point)
{
    auto hitNodes = FindHitFrameNodes(point);
    std::unordered_map<int32_t, WeakPtr<FrameNode>> dragEnterNodes;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> dragMoveNodes;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> dragLeaveNodes;
    for (auto iter = nodesForDragNotify_.begin(); iter != nodesForDragNotify_.end(); iter++) {
        auto localHitResult = hitNodes.find(iter->first) != hitNodes.end();
        auto preHitResult = parentHitNodes_.find(iter->first) != parentHitNodes_.end();
        if (localHitResult && preHitResult) {
            dragMoveNodes[iter->first] = iter->second;
        }
        if (localHitResult && !preHitResult) {
            dragEnterNodes[iter->first] = iter->second;
        }
        if (!localHitResult && preHitResult) {
            dragLeaveNodes[iter->first] = iter->second;
        }
    }
    RefPtr<NotifyDragEvent> notifyEvent = AceType::MakeRefPtr<NotifyDragEvent>();
    UpdateNotifyDragEvent(notifyEvent, point, DragEventType::MOVE);

    NotifyDragRegisterFrameNode(dragMoveNodes, DragEventType::MOVE, notifyEvent);
    NotifyDragRegisterFrameNode(dragEnterNodes, DragEventType::ENTER, notifyEvent);
    NotifyDragRegisterFrameNode(dragLeaveNodes, DragEventType::LEAVE, notifyEvent);
    parentHitNodes_ = std::move(hitNodes);
}

void DragDropManager::NotifyDragRegisterFrameNode(std::unordered_map<int32_t, WeakPtr<FrameNode>> nodes,
    DragEventType dragEventType, RefPtr<NotifyDragEvent>& notifyEvent)
{
    for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
        auto frameNode = iter->second.Upgrade();
        if (!frameNode) {
            continue;
        }
        auto eventHub = frameNode->GetEventHub<EventHub>();
        if (!CheckParentVisible(frameNode) || (eventHub && !eventHub->IsEnabled())) {
            continue;
        }
        auto pattern = frameNode->GetPattern<Pattern>();
        if (!pattern) {
            continue;
        }
        pattern->HandleOnDragStatusCallback(dragEventType, notifyEvent);
    }
}

void DragDropManager::NotifyDragFrameNode(
    const Point& point, const DragEventType& dragEventType, const DragRet& dragRet)
{
    RefPtr<NotifyDragEvent> notifyEvent = AceType::MakeRefPtr<NotifyDragEvent>();
    UpdateNotifyDragEvent(notifyEvent, point, dragEventType);
    notifyEvent->SetResult(dragRet);
    NotifyDragRegisterFrameNode(nodesForDragNotify_, dragEventType, notifyEvent);
}

void DragDropManager::OnDragStart(const Point& point, const RefPtr<FrameNode>& frameNode)
{
    dragDropState_ = DragDropMgrState::DRAGGING;
    NotifyDragFrameNode(point, DragEventType::START);
    CHECK_NULL_VOID(frameNode);
    preTargetFrameNode_ = frameNode;
    draggedFrameNode_ = preTargetFrameNode_;
    preMovePoint_ = point;
    parentHitNodes_.emplace(frameNode->GetId());
}

void DragDropManager::OnDragStart(const Point& point)
{
    dragDropState_ = DragDropMgrState::DRAGGING;
    NotifyDragFrameNode(point, DragEventType::START);
}

void DragDropManager::PrintDragFrameNode(const Point& point, const RefPtr<FrameNode>& dragFrameNode)
{
    CHECK_NULL_VOID(dragFrameNode);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    if (preTargetFrameNode_) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Current windowId is %{public}d, drag position is (%{public}f, %{public}f),"
            "PreTargetFrameNode is %{public}s, depth is %{public}d, id is %{public}s,"
            "New find targetNode is %{public}s, depth is %{public}d, id is %{public}s.",
            container->GetWindowId(), static_cast<float>(point.GetX()), static_cast<float>(point.GetY()),
            preTargetFrameNode_->GetTag().c_str(), preTargetFrameNode_->GetDepth(),
            preTargetFrameNode_->GetInspectorId()->c_str(),
            dragFrameNode->GetTag().c_str(), dragFrameNode->GetDepth(), dragFrameNode->GetInspectorId()->c_str());
    } else {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Current windowId is %{public}d, drag position is (%{public}f, %{public}f), "
            "PreTargetFrameNode is nullptr, New find targetNode is %{public}s, depth is %{public}d, id is %{public}s.",
            container->GetWindowId(), static_cast<float>(point.GetX()), static_cast<float>(point.GetY()),
            dragFrameNode->GetTag().c_str(), dragFrameNode->GetDepth(), dragFrameNode->GetInspectorId()->c_str());
    }
}

void DragDropManager::PrintGridDragFrameNode(
    const float globalX, const float globalY, const RefPtr<FrameNode>& dragFrameNode)
{
    CHECK_NULL_VOID(dragFrameNode);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    if (preGridTargetFrameNode_) {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "Current windowId is %{public}d, drag position is (%{public}f, %{public}f),"
            "PreTargetFrameNode is %{public}s, depth is %{public}d, id is %{public}s,"
            "New find targetNode is %{public}s, depth is %{public}d, id is %{public}s.",
            container->GetWindowId(), globalX, globalY, preGridTargetFrameNode_->GetTag().c_str(),
            preGridTargetFrameNode_->GetDepth(), preGridTargetFrameNode_->GetInspectorId()->c_str(),
            dragFrameNode->GetTag().c_str(), dragFrameNode->GetDepth(), dragFrameNode->GetInspectorId()->c_str());
    } else {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "Current windowId is %{public}d, drag position is (%{public}f, %{public}f), "
            "PreTargetFrameNode is nullptr, New find targetNode is %{public}s, depth is %{public}d, id is %{public}s.",
            container->GetWindowId(), globalX, globalY, dragFrameNode->GetTag().c_str(), dragFrameNode->GetDepth(),
            dragFrameNode->GetInspectorId()->c_str());
    }
}

void DragDropManager::TransDragWindowToDragFwk(int32_t windowContainerId)
{
    if (isDragFwkShow_) {
        return;
    }
    InteractionInterface::GetInstance()->SetDragWindowVisible(true);
    isDragFwkShow_ = true;
    auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(windowContainerId);
    CHECK_NULL_VOID(subwindow);
    auto overlayManager = subwindow->GetOverlayManager();
    overlayManager->RemovePixelMap();
    overlayManager->RemoveGatherNode();
    SubwindowManager::GetInstance()->HidePreviewNG();
    info_.scale = -1.0;
}

void DragDropManager::OnDragMoveOut(const PointerEvent& pointerEvent)
{
    Point point = pointerEvent.GetPoint();
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        if (IsDragged() && IsWindowConsumed()) {
            SetIsWindowConsumed(false);
            return;
        }
    }
    SetIsWindowConsumed(false);
    UpdateVelocityTrackerPoint(point, false);
    UpdateDragListener(Point(-1, -1));
    if (preTargetFrameNode_) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Leave the current window, windowId is %{public}d,"
            "drag Position is (%{public}f, %{public}f),"
            "PreTargetFrameNode is %{public}s, depth is %{public}d, id is %{public}s",
            container->GetWindowId(), static_cast<float>(point.GetX()), static_cast<float>(point.GetY()),
            preTargetFrameNode_->GetTag().c_str(), preTargetFrameNode_->GetDepth(),
            preTargetFrameNode_->GetInspectorId()->c_str());
        FireOnDragEvent(preTargetFrameNode_, point, DragEventType::LEAVE, extraInfo_);
        preTargetFrameNode_ = nullptr;
    }
    if (IsNeedDisplayInSubwindow()) {
        TransDragWindowToDragFwk(Container::CurrentId());
    }
}

bool DragDropManager::isDistanceLimited(const Point& point)
{
    auto distance = sqrt(pow(point.GetX() - preMovePoint_.GetX(), 2) + pow(point.GetY() - preMovePoint_.GetY(), 2));
    TAG_LOGD(AceLogTag::ACE_DRAG,
        "onDragMove, preMovePoint X: %{public}f, Y: %{public}f, point X: %{public}f, Y: %{public}f, distance: "
        "%{public}f",
        preMovePoint_.GetX(), preMovePoint_.GetY(), point.GetX(), point.GetY(), distance);
    if (distance < MOVE_DISTANCE_LIMIT) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "onDragMove, distance is less than limit, point X: %{public}f, Y: %{public}f",
            point.GetX(), point.GetY());
        return true;
    }
    return false;
}

bool DragDropManager::isTimeLimited(const PointerEvent& pointerEvent, const Point& point)
{
    uint64_t currentTimeStamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(pointerEvent.time.time_since_epoch()).count());
    if (currentTimeStamp > preTimeStamp_ && currentTimeStamp - preTimeStamp_ < MOVE_TIME_LIMIT) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "onDragMove, time is less than limit, point X: %{public}f, Y: %{public}f",
            point.GetX(), point.GetY());
        return true;
    }
    return false;
}

bool DragDropManager::ReachMoveLimit(const PointerEvent& pointerEvent, const Point& point)
{
    if (pointerEvent.sourceTool == SourceTool::MOUSE) {
        if (isTimeLimited(pointerEvent, point) && isDistanceLimited(point)) {
            return true;
        }
    }
    return false;
}

void DragDropManager::OnDragMove(const PointerEvent& pointerEvent, const std::string& extraInfo)
{
    Point point = pointerEvent.GetPoint();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    if (container && container->IsScenceBoardWindow()) {
        if (IsDragged() && IsWindowConsumed()) {
            SetIsWindowConsumed(false);
            return;
        }
    }
    if (ReachMoveLimit(pointerEvent, point)) {
        return;
    }
    preMovePoint_ = point;
    preTimeStamp_ = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(pointerEvent.time.time_since_epoch()).count());
    SetIsWindowConsumed(false);
    SubwindowManager::GetInstance()->UpdateHideMenuOffsetNG(OffsetF(static_cast<float>(point.GetX()),
        static_cast<float>(point.GetY())));
    UpdateVelocityTrackerPoint(point, false);
    UpdateDragListener(point);
    auto dragFrameNode = FindDragFrameNodeByPosition(
        static_cast<float>(point.GetX()), static_cast<float>(point.GetY()), DragType::COMMON, false);
    if (!dragFrameNode) {
        if (preTargetFrameNode_) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Not find drag target node, current windowId is %{public}d,"
                "drag Position is (%{public}f, %{public}f),"
                "PreTargetFrameNode is %{public}s, depth is %{public}d, id is %{public}s",
                container->GetWindowId(), static_cast<float>(point.GetX()), static_cast<float>(point.GetY()),
                preTargetFrameNode_->GetTag().c_str(), preTargetFrameNode_->GetDepth(),
                preTargetFrameNode_->GetInspectorId()->c_str());
            FireOnDragEvent(preTargetFrameNode_, point, DragEventType::LEAVE, extraInfo);
            preTargetFrameNode_ = nullptr;
        }

        if (!isMouseDragged_ || isDragWindowShow_) {
            UpdateDragStyle(DragCursorStyleCore::MOVE);
        }
        return;
    }

    if ((V2::UI_EXTENSION_COMPONENT_ETS_TAG == dragFrameNode->GetTag() ||
        V2::EMBEDDED_COMPONENT_ETS_TAG == dragFrameNode->GetTag()) &&
        (!IsUIExtensionShowPlaceholder(dragFrameNode))) {
        auto pattern = dragFrameNode->GetPattern<Pattern>();
        pattern->HandleDragEvent(pointerEvent);
        return;
    }

    if (dragFrameNode == preTargetFrameNode_) {
        FireOnDragEvent(dragFrameNode, point, DragEventType::MOVE, extraInfo);
        return;
    }

    FireOnDragLeave(preTargetFrameNode_, point, extraInfo);
    PrintDragFrameNode(point, dragFrameNode);
    FireOnDragEvent(dragFrameNode, point, DragEventType::ENTER, extraInfo);
    preTargetFrameNode_ = dragFrameNode;
}

void DragDropManager::ResetDragDropStatus(const Point& point, const DragDropRet& dragDropRet, int32_t windowId)
{
    if (dragDropRet.result != DragRet::DRAG_FAIL || !isMouseDragged_) {
        InteractionInterface::GetInstance()->SetDragWindowVisible(!dragDropRet.hasCustomAnimation);
    }
    InteractionInterface::GetInstance()->StopDrag(dragDropRet);
    NotifyDragFrameNode(point, DragEventType::DROP, dragDropRet.result);
    summaryMap_.clear();
    parentHitNodes_.clear();
    dragCursorStyleCore_ = DragCursorStyleCore::DEFAULT;
}

void DragDropManager::OnDragEnd(const PointerEvent& pointerEvent, const std::string& extraInfo)
{
    Point point = pointerEvent.GetPoint();
    dragDropState_ = DragDropMgrState::IDLE;
    preTargetFrameNode_ = nullptr;
    draggedFrameNode_ = nullptr;
    preMovePoint_ = Point(0, 0);
    hasNotifiedTransformation_ = false;
    badgeNumber_ = -1;
    isDragWithContextMenu_ = false;
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        if (IsDragged() && IsWindowConsumed()) {
            TAG_LOGD(AceLogTag::ACE_DRAG, "DragDropManager is dragged or window consumed. WindowId is %{public}d",
                container->GetWindowId());
            return;
        }
    }
    HideDragPreviewOverlay();
    if (isDragCancel_) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "DragDropManager is dragCancel, finish drag. WindowId is %{public}d.",
            container->GetWindowId());
        DragDropRet dragDropRet { DragRet::DRAG_CANCEL, false, container->GetWindowId(), DragBehavior::UNKNOWN };
        ResetDragDropStatus(point, dragDropRet, container->GetWindowId());
        ClearVelocityInfo();
        return;
    }
    UpdateVelocityTrackerPoint(point, true);
    auto dragFrameNode = FindDragFrameNodeByPosition(
        static_cast<float>(point.GetX()), static_cast<float>(point.GetY()), DragType::COMMON, true);
    if (!dragFrameNode) {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "DragDropManager onDragEnd, not find drop target, stop drag. WindowId is %{public}d.",
            container->GetWindowId());
        ResetDragDrop(container->GetWindowId(), point);
        return;
    }
    if (!IsDropAllowed(dragFrameNode)) {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "DragDropManager onDragEnd, target data is not allowed to fall into. WindowId is %{public}d.",
            container->GetWindowId());
        ResetDragDrop(container->GetWindowId(), point);
        return;
    }
    TAG_LOGI(AceLogTag::ACE_DRAG, "Current windowId is %{public}d, drag position is (%{public}f, %{public}f)."
        "TargetNode is %{public}s, id is %{public}s",
        container->GetWindowId(), static_cast<float>(point.GetX()), static_cast<float>(point.GetY()),
        dragFrameNode->GetTag().c_str(), dragFrameNode->GetInspectorId()->c_str());
    if ((V2::UI_EXTENSION_COMPONENT_ETS_TAG == dragFrameNode->GetTag() ||
        V2::EMBEDDED_COMPONENT_ETS_TAG == dragFrameNode->GetTag()) &&
        (!IsUIExtensionShowPlaceholder(dragFrameNode))) {
        auto pattern = dragFrameNode->GetPattern<Pattern>();
        pattern->HandleDragEvent(pointerEvent);
        return;
    }

    RequestDragSummaryInfoAndPrivilege();
    std::string udKey;
    InteractionInterface::GetInstance()->GetUdKey(udKey);
    if (!CheckRemoteData(dragFrameNode, pointerEvent, udKey)) {
        auto unifiedData = RequestUDMFDataWithUDKey(udKey);
        DoDropAction(dragFrameNode, point, unifiedData, udKey);
    }
}

bool DragDropManager::IsDropAllowed(const RefPtr<FrameNode>& dragFrameNode)
{
    // application passed in null to indicate refusing all drag data forcedly
    bool isDisallowDropForcedly = dragFrameNode->GetDisallowDropForcedly();
    if (isDisallowDropForcedly) {
        return false;
    }
    const auto& dragFrameNodeAllowDrop = dragFrameNode->GetAllowDrop();
    // if application does not set allow drop or set with empty, treat as all data types is allowed
    if (dragFrameNodeAllowDrop.empty() || summaryMap_.empty()) {
        return true;
    }
    for (const auto& it : summaryMap_) {
        // if one matched found, allow drop
        if (dragFrameNodeAllowDrop.find(it.first) != dragFrameNodeAllowDrop.end()) {
            return true;
        }
    }
    return false;
}

void DragDropManager::RequestDragSummaryInfoAndPrivilege()
{
    RequireSummary();
    int ret = InteractionInterface::GetInstance()->AddPrivilege();
    if (ret != 0 && SystemProperties::GetDebugEnabled()) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Interaction AddPrivilege in DragEnd with code:%{public}d", ret);
    }
    ShadowOffsetData shadowOffsetData { -1, -1, -1, -1 };
    ret = InteractionInterface::GetInstance()->GetShadowOffset(shadowOffsetData);
    if (ret == 0) {
        previewRect_ =
            Rect(shadowOffsetData.offsetX, shadowOffsetData.offsetY, shadowOffsetData.width, shadowOffsetData.height);
    } else {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Interaction GetShadowOffset in DragEnd with code:%{public}d", ret);
    }
}

void DragDropManager::DoDropAction(const RefPtr<FrameNode>& dragFrameNode, const Point& point,
    const RefPtr<UnifiedData>& unifiedData, const std::string& udKey)
{
    RefPtr<OHOS::Ace::DragEvent> event = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    if (!udKey.empty()) {
        event->SetUdKey(udKey);
    }
    if (unifiedData == nullptr) {
        event->SetIsGetDataSuccess(false);
    } else {
        event->SetData(unifiedData);
        event->SetIsGetDataSuccess(true);
    }
    OnDragDrop(event, dragFrameNode, point);
}

RefPtr<UnifiedData> DragDropManager::RequestUDMFDataWithUDKey(const std::string& udKey)
{
    if (udKey.empty()) {
        return nullptr;
    }
    RefPtr<UnifiedData> udData = UdmfClient::GetInstance()->CreateUnifiedData();
    auto ret = UdmfClient::GetInstance()->GetData(udData, udKey);
    if (ret != 0) {
        return nullptr;
    }
    return udData;
}

void DragDropManager::TryGetDataBackGround(
    const RefPtr<FrameNode>& dragFrameNode, const Point& point, const std::string& udKey, int32_t count)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto taskScheduler = pipeline->GetTaskExecutor();
    CHECK_NULL_VOID(taskScheduler);
    taskScheduler->PostTask(
        [id = Container::CurrentId(), pipeline, dragFrameNode, point, udKey, count, weakManager = WeakClaim(this)]() {
            ContainerScope scope(id);
            auto dragDropManager = weakManager.Upgrade();
            CHECK_NULL_VOID(dragDropManager);
            auto taskScheduler = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID(taskScheduler);
            auto result = dragDropManager->RequestUDMFDataWithUDKey(udKey);
            if (result != nullptr || count >= MAX_RETRY_TIMES) {
                taskScheduler->PostTask(
                    [dragFrameNode, point, weakManager, result, udKey]() {
                        auto dragDropManager = weakManager.Upgrade();
                        CHECK_NULL_VOID(dragDropManager);
                        dragDropManager->DoDropAction(dragFrameNode, point, result, udKey);
                    },
                    TaskExecutor::TaskType::UI, "ArkUIDragDropAction");
            } else {
                // first temp get udmfData failed, prepare to retryGetData.
                taskScheduler->PostDelayedTask(
                    [dragFrameNode, point, weakManager, count, udKey]() {
                        auto dragDropManager = weakManager.Upgrade();
                        CHECK_NULL_VOID(dragDropManager);
                        dragDropManager->TryGetDataBackGround(dragFrameNode, point, udKey, count + 1);
                    },
                    TaskExecutor::TaskType::UI, MAX_RETRY_DURATION, "ArkUIDragDropGetDataBackground");
            }
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIDragDropGetDataBackground");
}

bool DragDropManager::CheckRemoteData(
    const RefPtr<FrameNode>& dragFrameNode, const PointerEvent& pointerEvent, const std::string& udKey)
{
    Point point = pointerEvent.GetPoint();
    if (udKey.empty()) {
        return false;
    }
    std::string remoteUdKey = udKey;
    auto isRemoteData = UdmfClient::GetInstance()->GetRemoteStatus(remoteUdKey);
    if (isRemoteData) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Stop drag with motion drag action, target device id : %{public}d.",
            pointerEvent.deviceId);
        TryGetDataBackGround(dragFrameNode, point, udKey);
    }
    return isRemoteData;
}

void DragDropManager::OnDragDrop(
    RefPtr<OHOS::Ace::DragEvent>& event, const RefPtr<FrameNode>& dragFrameNode, const Point& point)
{
    auto eventHub = dragFrameNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    UpdateDragEvent(event, point);
    auto extraParams = eventHub->GetDragExtraParams(extraInfo_, point, DragEventType::DROP);
    eventHub->FireCustomerOnDragFunc(DragFuncType::DRAG_DROP, event, extraParams);
    eventHub->HandleInternalOnDrop(event, extraParams);
    ClearVelocityInfo();
    SetIsDragged(false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragResult = event->GetResult();
    auto useCustomAnimation = event->IsUseCustomAnimation();
    auto dragBehavior = event->GetDragBehavior();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto windowId = container->GetWindowId();
    pipeline->AddAfterRenderTask([dragResult, useCustomAnimation, windowId, dragBehavior]() {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "Stop drag, start do drop animation. UseCustomAnimation is %{public}d,"
            "WindowId is %{public}d.",
            useCustomAnimation, windowId);
        InteractionInterface::GetInstance()->SetDragWindowVisible(!useCustomAnimation);
        DragDropRet dragDropRet { dragResult, useCustomAnimation, windowId, dragBehavior };
        InteractionInterface::GetInstance()->StopDrag(dragDropRet);
    });
    NotifyDragFrameNode(point, DragEventType::DROP, event->GetResult());
    dragFrameNode->MarkDirtyNode();
    summaryMap_.clear();
    parentHitNodes_.clear();
    dragCursorStyleCore_ = DragCursorStyleCore::DEFAULT;
}

void DragDropManager::RequireSummary()
{
    std::map<std::string, int64_t> summary;
    int32_t ret = InteractionInterface::GetInstance()->GetDragSummary(summary);
    if (ret != 0) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "RequireSummary: Interaction GetSummary failed: %{public}d", ret);
    }
    std::string extraInfo;
    ret = InteractionInterface::GetInstance()->GetDragExtraInfo(extraInfo);
    if (ret != 0) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "GetExtraInfo: Interaction GetExtraInfo failed: %{public}d", ret);
    }
    previewRect_ = Rect(-1, -1, -1, -1);
    extraInfo_ = extraInfo;
    summaryMap_ = summary;
}

void DragDropManager::ResetRecordSize(uint32_t recordSize)
{
    recordSize_ = recordSize;
}

uint32_t DragDropManager::GetRecordSize() const
{
    return recordSize_;
}

Rect DragDropManager::GetDragWindowRect(const Point& point)
{
    if (!previewRect_.IsValid()) {
        ShadowOffsetData shadowOffsetData { -1, -1, -1, -1 };
        int ret = InteractionInterface::GetInstance()->GetShadowOffset(shadowOffsetData);
        if (ret == 0) {
            previewRect_ = Rect(
                shadowOffsetData.offsetX,
                shadowOffsetData.offsetY,
                shadowOffsetData.width,
                shadowOffsetData.height);
        }
    }
    return previewRect_ + Offset(point.GetX(), point.GetY());
}

void DragDropManager::ClearSummary()
{
    previewRect_ = Rect(-1, -1, -1, -1);
    summaryMap_.clear();
    parentHitNodes_.clear();
    ResetRecordSize();
}

void DragDropManager::OnTextDragEnd(float globalX, float globalY, const std::string& extraInfo)
{
    dragDropState_ = DragDropMgrState::IDLE;
    auto dragFrameNode = FindDragFrameNodeByPosition(globalX, globalY, DragType::TEXT, true);
    if (dragFrameNode) {
        auto textFieldPattern = dragFrameNode->GetPattern<TextFieldPattern>();
        if (textFieldPattern) {
            textFieldPattern->InsertValue(extraInfo);
        }
    }
    SetIsDragged(false);
    currentId_ = -1;
}

void DragDropManager::onDragCancel()
{
    preTargetFrameNode_ = nullptr;
    draggedFrameNode_ = nullptr;
}

void DragDropManager::FireOnDragEventWithDragType(const RefPtr<EventHub>& eventHub, DragEventType type,
    RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams)
{
    switch (type) {
        case DragEventType::ENTER: {
            eventHub->FireCustomerOnDragFunc(DragFuncType::DRAG_ENTER, event, extraParams);
            eventHub->FireOnDragEnter(event, extraParams);
            break;
        }
        case DragEventType::MOVE: {
            eventHub->FireCustomerOnDragFunc(DragFuncType::DRAG_MOVE, event, extraParams);
            eventHub->FireOnDragMove(event, extraParams);
            break;
        }
        case DragEventType::LEAVE: {
            eventHub->FireCustomerOnDragFunc(DragFuncType::DRAG_LEAVE, event, extraParams);
            eventHub->FireOnDragLeave(event, extraParams);
            break;
        }
        case DragEventType::DROP: {
            eventHub->FireCustomerOnDragFunc(DragFuncType::DRAG_DROP, event, extraParams);
            eventHub->HandleInternalOnDrop(event, extraParams);
            break;
        }
        default:
            break;
    }
}

void DragDropManager::FireOnDragEvent(
    const RefPtr<FrameNode>& frameNode, const Point& point, DragEventType type, const std::string& extraInfo)
{
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (!eventHub->HasOnDrop() && !eventHub->HasOnItemDrop() && !eventHub->HasCustomerOnDrop()) {
        return;
    }
    auto extraParams = eventHub->GetDragExtraParams(extraInfo_.empty() ? extraInfo : extraInfo_, point, type);
    RefPtr<OHOS::Ace::DragEvent> event = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    event->SetX((double)point.GetX());
    event->SetY((double)point.GetY());
    event->SetScreenX((double)point.GetScreenX());
    event->SetScreenY((double)point.GetScreenY());
    event->SetVelocity(velocityTracker_.GetVelocity());
    event->SetSummary(summaryMap_);
    event->SetPreviewRect(GetDragWindowRect(point));

    FireOnEditableTextComponent(frameNode, type);
    FireOnDragEventWithDragType(eventHub, type, event, extraParams);

    if (isMouseDragged_ && !isDragWindowShow_) {
        return;
    }
    if (event->GetResult() == DragRet::ENABLE_DROP) {
        if (event->GetDragBehavior() == DragBehavior::MOVE) {
            UpdateDragStyle(DragCursorStyleCore::MOVE);
        } else {
            UpdateDragStyle(DragCursorStyleCore::COPY);
        }
    } else if (event->GetResult() == DragRet::DISABLE_DROP) {
        UpdateDragStyle(DragCursorStyleCore::FORBIDDEN);
    } else {
        UpdateDragAllowDrop(frameNode, event->GetDragBehavior());
    }
}

void DragDropManager::OnItemDragStart(float globalX, float globalY, const RefPtr<FrameNode>& frameNode)
{
    dragDropState_ = DragDropMgrState::DRAGGING;
    preGridTargetFrameNode_ = frameNode;
    draggedGridFrameNode_ = frameNode;
}

void DragDropManager::OnItemDragMove(float globalX, float globalY, int32_t draggedIndex, DragType dragType)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);

    auto windowScale = GetWindowScale();
    auto windowX = globalX * windowScale;
    auto windowY = globalY * windowScale;
    UpdateDragWindowPosition(static_cast<int32_t>(windowX), static_cast<int32_t>(windowY));

    OHOS::Ace::ItemDragInfo itemDragInfo;
    itemDragInfo.SetX(pipeline->ConvertPxToVp(Dimension(windowX, DimensionUnit::PX)));
    itemDragInfo.SetY(pipeline->ConvertPxToVp(Dimension(windowY, DimensionUnit::PX)));

    // use -1 for grid item not in eventGrid
    auto getDraggedIndex = [draggedGrid = draggedGridFrameNode_, draggedIndex, dragType](
                               const RefPtr<FrameNode>& eventGrid) {
        return (dragType == DragType::GRID) ? (eventGrid == draggedGrid ? draggedIndex : -1) : draggedIndex;
    };

    auto dragFrameNode = FindDragFrameNodeByPosition(globalX, globalY, dragType, false);
    if (!dragFrameNode) {
        if (preGridTargetFrameNode_) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Not find drag target node, current windowId is %{public}d,"
                "drag Position is (%{public}f, %{public}f),"
                "PreGridTargetFrameNode is %{public}s, depth is %{public}d, id is %{public}s",
                container->GetWindowId(), globalX, globalY, preGridTargetFrameNode_->GetTag().c_str(),
                preGridTargetFrameNode_->GetDepth(), preGridTargetFrameNode_->GetInspectorId()->c_str());
            FireOnItemDragEvent(preGridTargetFrameNode_, dragType, itemDragInfo, DragEventType::LEAVE,
                getDraggedIndex(preGridTargetFrameNode_));
            preGridTargetFrameNode_ = nullptr;
        }
        return;
    }

    if (dragFrameNode == preGridTargetFrameNode_) {
        int32_t insertIndex = GetItemIndex(dragFrameNode, dragType, globalX, globalY);
        FireOnItemDragEvent(
            dragFrameNode, dragType, itemDragInfo, DragEventType::MOVE, getDraggedIndex(dragFrameNode), insertIndex);
        return;
    }

    if (preGridTargetFrameNode_) {
        FireOnItemDragEvent(preGridTargetFrameNode_, dragType, itemDragInfo, DragEventType::LEAVE,
            getDraggedIndex(preGridTargetFrameNode_));
    }

    PrintGridDragFrameNode(globalX, globalY, dragFrameNode);
    FireOnItemDragEvent(dragFrameNode, dragType, itemDragInfo, DragEventType::ENTER, getDraggedIndex(dragFrameNode));
    preGridTargetFrameNode_ = dragFrameNode;
}

float DragDropManager::GetWindowScale() const
{
    float scale = 1.0f;
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, scale);
    scale = container->GetWindowScale();
    return scale;
}

void DragDropManager::OnItemDragEnd(float globalX, float globalY, int32_t draggedIndex, DragType dragType)
{
    dragDropState_ = DragDropMgrState::IDLE;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto windowScale = GetWindowScale();
    auto windowX = globalX * windowScale;
    auto windowY = globalY * windowScale;

    OHOS::Ace::ItemDragInfo itemDragInfo;
    itemDragInfo.SetX(pipeline->ConvertPxToVp(Dimension(windowX, DimensionUnit::PX)));
    itemDragInfo.SetY(pipeline->ConvertPxToVp(Dimension(windowY, DimensionUnit::PX)));

    auto dragFrameNode = FindDragFrameNodeByPosition(globalX, globalY, dragType, true);
    if (!dragFrameNode) {
        // drag on one grid and drop on other area
        if (draggedGridFrameNode_) {
            if (dragType == DragType::GRID) {
                auto eventHub = draggedGridFrameNode_->GetEventHub<GridEventHub>();
                CHECK_NULL_VOID(eventHub);
                eventHub->FireOnItemDrop(itemDragInfo, draggedIndex, -1, false);
            } else {
                auto eventHub = draggedGridFrameNode_->GetEventHub<ListEventHub>();
                CHECK_NULL_VOID(eventHub);
                eventHub->FireOnItemDrop(itemDragInfo, draggedIndex, -1, false);
            }
        }
    } else {
        int32_t insertIndex = GetItemIndex(dragFrameNode, dragType, globalX, globalY);
        // drag and drop on the same grid
        if (dragFrameNode == draggedGridFrameNode_) {
            FireOnItemDropEvent(dragFrameNode, dragType, itemDragInfo, draggedIndex, insertIndex, true);
        } else {
            // drag and drop on different grid
            bool isSuccess = FireOnItemDropEvent(dragFrameNode, dragType, itemDragInfo, -1, insertIndex, true);
            if (draggedGridFrameNode_) {
                FireOnItemDropEvent(draggedGridFrameNode_, dragType, itemDragInfo, draggedIndex, -1, isSuccess);
            }
        }
    }

    preGridTargetFrameNode_ = nullptr;
    draggedGridFrameNode_ = nullptr;
}

void DragDropManager::onItemDragCancel()
{
    dragDropState_ = DragDropMgrState::IDLE;
    preGridTargetFrameNode_ = nullptr;
    draggedGridFrameNode_ = nullptr;
}

void DragDropManager::FireOnItemDragEvent(const RefPtr<FrameNode>& frameNode, DragType dragType,
    const OHOS::Ace::ItemDragInfo& itemDragInfo, DragEventType type, int32_t draggedIndex, int32_t insertIndex)
{
    if (dragType == DragType::GRID) {
        auto eventHub = frameNode->GetEventHub<GridEventHub>();
        CHECK_NULL_VOID(eventHub);
        switch (type) {
            case DragEventType::ENTER:
                eventHub->FireOnItemDragEnter(itemDragInfo);
                break;
            case DragEventType::MOVE:
                eventHub->FireOnItemDragMove(itemDragInfo, draggedIndex, insertIndex);
                break;
            case DragEventType::LEAVE:
                eventHub->FireOnItemDragLeave(itemDragInfo, draggedIndex);
                break;
            default:
                break;
        }
    } else if (dragType == DragType::LIST) {
        auto eventHub = frameNode->GetEventHub<ListEventHub>();
        CHECK_NULL_VOID(eventHub);
        switch (type) {
            case DragEventType::ENTER:
                eventHub->FireOnItemDragEnter(itemDragInfo);
                break;
            case DragEventType::MOVE:
                eventHub->FireOnItemDragMove(itemDragInfo, draggedIndex, insertIndex);
                break;
            case DragEventType::LEAVE:
                eventHub->FireOnItemDragLeave(itemDragInfo, draggedIndex);
                break;
            default:
                break;
        }
    }
}

bool DragDropManager::FireOnItemDropEvent(const RefPtr<FrameNode>& frameNode, DragType dragType,
    const OHOS::Ace::ItemDragInfo& itemDragInfo, int32_t draggedIndex, int32_t insertIndex, bool isSuccess)
{
    if (dragType == DragType::GRID) {
        auto eventHub = frameNode->GetEventHub<GridEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        return eventHub->FireOnItemDrop(itemDragInfo, draggedIndex, insertIndex, isSuccess);
    } else if (dragType == DragType::LIST) {
        auto eventHub = frameNode->GetEventHub<ListEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        return eventHub->FireOnItemDrop(itemDragInfo, draggedIndex, insertIndex, isSuccess);
    }
    return false;
}

int32_t DragDropManager::GetItemIndex(
    const RefPtr<FrameNode>& frameNode, DragType dragType, float globalX, float globalY)
{
    CHECK_NULL_RETURN(frameNode, -1);
    if (dragType == DragType::GRID) {
        auto eventHub = frameNode->GetEventHub<GridEventHub>();
        CHECK_NULL_RETURN(eventHub, -1);
        if (frameNode != draggedGridFrameNode_) {
            return eventHub->GetInsertPosition(globalX, globalY);
        }
        auto itemFrameNode = frameNode->FindChildByPosition(globalX, globalY);
        if (!itemFrameNode) {
            if (eventHub->CheckPostionInGrid(globalX, globalY)) {
                return eventHub->GetFrameNodeChildSize();
            }
        } else {
            return eventHub->GetGridItemIndex(itemFrameNode);
        }
    } else if (dragType == DragType::LIST) {
        auto eventHub = frameNode->GetEventHub<ListEventHub>();
        CHECK_NULL_RETURN(eventHub, -1);
        return eventHub->GetListItemIndexByPosition(globalX, globalY);
    }
    return -1;
}

void DragDropManager::AddDataToClipboard(const std::string& extraInfo)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (!extraInfo.empty()) {
        if (!newData_) {
            newData_ = JsonUtil::Create(true);
            newData_->Put("customDragInfo", extraInfo.c_str());
        } else {
            newData_->Replace("customDragInfo", extraInfo.c_str());
        }
    } else {
        return;
    }
    if (!clipboard_) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    }
    if (!addDataCallback_) {
        auto callback = [weakManager = WeakClaim(this)](const std::string& data) {
            auto dragDropManager = weakManager.Upgrade();
            auto addData = dragDropManager->newData_->ToString();
            CHECK_NULL_VOID(dragDropManager);
            auto clipboardAllData = JsonUtil::Create(true);
            clipboardAllData->Put("preData", data.c_str());
            clipboardAllData->Put("newData", addData.c_str());
            dragDropManager->clipboard_->SetData(clipboardAllData->ToString(), CopyOptions::Local, true);
        };
        addDataCallback_ = callback;
    }
    if (clipboard_) {
        clipboard_->GetData(addDataCallback_, true);
    }
}

void DragDropManager::GetExtraInfoFromClipboard(std::string& extraInfo)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    if (!clipboard_) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    }

    if (!getDataCallback_) {
        auto callback = [weak = WeakClaim(this)](const std::string& data) {
            auto manager = weak.Upgrade();
            CHECK_NULL_VOID(manager);
            auto json = JsonUtil::ParseJsonString(data);
            auto newData = JsonUtil::ParseJsonString(json->GetString("newData"));
            manager->extraInfo_ = newData->GetString("customDragInfo");
        };
        getDataCallback_ = callback;
    }

    if (getDataCallback_ && clipboard_) {
        clipboard_->GetData(getDataCallback_, true);
    }

    extraInfo = extraInfo_;
}

void DragDropManager::RestoreClipboardData()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    if (!clipboard_) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    }

    if (!deleteDataCallback_) {
        auto callback = [weakManager = WeakClaim(this)](const std::string& data) {
            auto dragDropManager = weakManager.Upgrade();
            CHECK_NULL_VOID(dragDropManager);
            auto json = JsonUtil::ParseJsonString(data);
            if (json->Contains("preData")) {
                dragDropManager->clipboard_->SetData(json->GetString("preData"));
            }
        };
        deleteDataCallback_ = callback;
    }
    if (clipboard_) {
        clipboard_->GetData(deleteDataCallback_, true);
    }
}

void DragDropManager::DestroyDragWindow()
{
    if (dragWindow_ != nullptr) {
        dragWindow_->Destroy();
        dragWindow_ = nullptr;
    }
    ResetRecordSize();
    if (dragWindowRootNode_) {
        dragWindowRootNode_ = nullptr;
    }
    SetIsDragged(false);
    SetIsDragWindowShow(false);
    previewRect_ = Rect(-1, -1, -1, -1);
    isMouseDragged_ = false;
    currentId_ = -1;
}

void DragDropManager::CancelItemDrag()
{
    if (draggedGridFrameNode_) {
        auto listEventHub = draggedGridFrameNode_->GetEventHub<ListEventHub>();
        if (listEventHub) {
            listEventHub->HandleOnItemDragCancel();
            return;
        }
        auto gridEventHub = draggedGridFrameNode_->GetEventHub<GridEventHub>();
        if (gridEventHub) {
            gridEventHub->HandleOnItemDragCancel();
            return;
        }
    }
}

RefPtr<DragDropProxy> DragDropManager::CreateFrameworkDragDropProxy()
{
    SetIsDragged(true);
    isDragCancel_ = false;
    currentId_ = ++g_proxyId;
    return MakeRefPtr<DragDropProxy>(currentId_);
}

void DragDropManager::UpdateNotifyDragEvent(
    RefPtr<NotifyDragEvent>& notifyEvent, const Point& point, const DragEventType dragEventType)
{
    notifyEvent->SetX((double)point.GetX());
    notifyEvent->SetY((double)point.GetY());
    notifyEvent->SetScreenX((double)point.GetScreenX());
    notifyEvent->SetScreenY((double)point.GetScreenY());
    if (dragEventType != DragEventType::START) {
        if (dragEventType != DragEventType::DROP) {
            notifyEvent->SetVelocity(velocityTracker_.GetVelocity());
        }
        notifyEvent->SetSummary(summaryMap_);
        notifyEvent->SetPreviewRect(GetDragWindowRect(point));
    }
}

void DragDropManager::UpdateDragEvent(RefPtr<OHOS::Ace::DragEvent>& event, const Point& point)
{
    event->SetX(point.GetX());
    event->SetY(point.GetY());
    event->SetScreenX(point.GetScreenX());
    event->SetScreenY(point.GetScreenY());
    event->SetVelocity(velocityTracker_.GetVelocity());
    event->SetSummary(summaryMap_);
    event->SetPreviewRect(GetDragWindowRect(point));
}

std::string DragDropManager::GetExtraInfo()
{
    return extraInfo_;
}

void DragDropManager::SetExtraInfo(const std::string& extraInfo)
{
    extraInfo_ = extraInfo;
}

void DragDropManager::ClearExtraInfo()
{
    extraInfo_.clear();
}

bool DragDropManager::IsMsdpDragging() const
{
    DragState dragState;
    InteractionInterface::GetInstance()->GetDragState(dragState);
    return dragState == DragState::START;
}

void DragDropManager::ClearVelocityInfo()
{
    velocityTracker_.Reset();
}

void DragDropManager::UpdateVelocityTrackerPoint(const Point& point, bool isEnd)
{
    std::chrono::microseconds microseconds(GetMicroTickCount());
    TimeStamp curTime(microseconds);
    velocityTracker_.UpdateTrackerPoint(point.GetX(), point.GetY(), curTime, isEnd);
}

bool DragDropManager::GetDragPreviewInfo(const RefPtr<OverlayManager>& overlayManager,
    DragPreviewInfo& dragPreviewInfo)
{
    if (!overlayManager->GetHasPixelMap()) {
        return false;
    }
    auto imageNode = overlayManager->GetPixelMapContentNode();
    CHECK_NULL_RETURN(imageNode, false);
    auto badgeNode = overlayManager->GetPixelMapBadgeNode();
    if (badgeNode) {
        dragPreviewInfo.textNode = badgeNode;
    }
    double maxWidth = GridSystemManager::GetInstance().GetMaxWidthWithColumnType(GridColumnType::DRAG_PANEL);
    auto width = imageNode->GetGeometryNode()->GetFrameRect().Width();
    dragPreviewInfo.scale = static_cast<float>(imageNode->GetPreviewScaleVal());
    if (!isMouseDragged_ && dragPreviewInfo.scale == 1.0f) {
        dragPreviewInfo.scale = TOUCH_DRAG_PPIXELMAP_SCALE;
    }
    dragPreviewInfo.height = imageNode->GetGeometryNode()->GetFrameRect().Height();
    dragPreviewInfo.width = static_cast<double>(width);
    dragPreviewInfo.maxWidth = maxWidth;
    dragPreviewInfo.imageNode = imageNode;
    return true;
}

bool DragDropManager::IsNeedDoDragMoveAnimate(const PointerEvent& pointerEvent)
{
    if (!IsNeedDisplayInSubwindow() || isDragFwkShow_) {
        return false;
    }
    auto x = pointerEvent.GetPoint().GetX();
    auto y = pointerEvent.GetPoint().GetY();
    auto distanceToPrePointer = sqrt(pow(prePointerOffset_.GetX() - x, 2) + pow(prePointerOffset_.GetY() - y, 2));
    int64_t nanoCurrentTimeStamp = pointerEvent.downTime.time_since_epoch().count();
    if ((nanoCurrentTimeStamp < nanoPreDragMoveAnimationTime_ + DRAG_MOVE_TIME_THRESHOLD) &&
        (distanceToPrePointer < MAX_DISTANCE_TO_PRE_POINTER)) {
        return false;
    }
    nanoPreDragMoveAnimationTime_ = nanoCurrentTimeStamp;
    prePointerOffset_ = { x, y };
    return true;
}

bool DragDropManager::IsNeedScaleDragPreview()
{
    return info_.scale > 0 && info_.scale < 1.0f;
}

double DragDropManager::CalcDragPreviewDistanceWithPoint(
    const OHOS::Ace::Dimension& preserverHeight, int32_t x, int32_t y, const DragPreviewInfo& info)
{
    CHECK_NULL_RETURN(info.imageNode, 0.0);
    auto nodeOffset = info.imageNode->GetTransformRelativeOffset();
    auto renderContext = info.imageNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, 0.0);
    nodeOffset -= pixelMapOffset_;
    auto pipeline = PipelineContext::GetCurrentContext();
    auto windowScale = GetWindowScale();
    if (pipeline && NearEqual(windowScale, 1.0f)) {
        auto windowOffset = pipeline->GetWindow()->GetCurrentWindowRect().GetOffset();
        x += windowOffset.GetX();
        y += windowOffset.GetY();
    }
    return sqrt(pow(nodeOffset.GetX() - x, 2) + pow(nodeOffset.GetY() - y, 2));
}

Offset DragDropManager::CalcDragMoveOffset(
    const OHOS::Ace::Dimension& preserverHeight, int32_t x, int32_t y, const DragPreviewInfo& info)
{
    CHECK_NULL_RETURN(info.imageNode, Offset(0.0f, 0.0f));
    auto originPoint = info.imageNode->GetOffsetRelativeToWindow();
    originPoint.SetX(originPoint.GetX() - pixelMapOffset_.GetX() + (1 - info.scale) * info.width / 2.0f);
    originPoint.SetY(originPoint.GetY() - pixelMapOffset_.GetY() + (1 - info.scale) * info.height / 2.0f);
    Offset newOffset { x - originPoint.GetX(), y - originPoint.GetY() };
    auto pipeline = PipelineContext::GetCurrentContext();
    auto windowScale = GetWindowScale();
    if (pipeline && NearEqual(windowScale, 1.0f)) {
        auto windowOffset = pipeline->GetWindow()->GetCurrentWindowRect().GetOffset();
        newOffset.SetX(newOffset.GetX() + windowOffset.GetX());
        newOffset.SetY(newOffset.GetY() + windowOffset.GetY());
    }
    return newOffset;
}

bool DragDropManager::UpdateDragMovePositionFinished(
    bool needDoDragMoveAnimate, bool isMenuShow, const Offset& newOffset, int32_t containerId)
{
    if (!isDragWithContextMenu_) {
        return false;
    }

    CHECK_NULL_RETURN(info_.imageNode, false);
    auto renderContext = info_.imageNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (!needDoDragMoveAnimate) {
        renderContext->UpdateTransformTranslate({ newOffset.GetX(), newOffset.GetY(), 0.0f });
        if (!isMenuShow) {
            TransDragWindowToDragFwk(containerId);
        }
        return true;
    }
    return false;
}
void DragDropManager::DoDragMoveAnimate(const PointerEvent& pointerEvent)
{
    bool needDoDragMoveAnimate = IsNeedDoDragMoveAnimate(pointerEvent);
    if (!needDoDragMoveAnimate && !isDragWithContextMenu_) {
        return;
    }
    isPullMoveReceivedForCurrentDrag_ = true;
    auto containerId = Container::CurrentId();
    auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(containerId);
    CHECK_NULL_VOID(info_.imageNode);
    CHECK_NULL_VOID(subwindow);
    auto overlayManager = subwindow->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto x = pointerEvent.GetPoint().GetX();
    auto y = pointerEvent.GetPoint().GetY();
    Offset newOffset = CalcDragMoveOffset(PRESERVE_HEIGHT, x, y, info_);
    bool isMenuShow = overlayManager->IsMenuShow();
    if (UpdateDragMovePositionFinished(needDoDragMoveAnimate, isMenuShow, newOffset, containerId) ||
        !needDoDragMoveAnimate) {
        return;
    }
    auto distance = CalcDragPreviewDistanceWithPoint(PRESERVE_HEIGHT, x, y, info_);
    auto gatherNodeCenter = info_.imageNode->GetPaintRectCenter();
    auto maxDistance = CalcGatherNodeMaxDistanceWithPoint(overlayManager,
        gatherNodeCenter.GetX(), gatherNodeCenter.GetY());
    distance = std::max(distance, maxDistance);
    AnimationOption option;
    const RefPtr<Curve> curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(0.347f, 0.99f, 0.0f);
    constexpr int32_t animateDuration = 30;
    option.SetCurve(curve);
    option.SetDuration(animateDuration);
    bool dragWithContextMenu = isDragWithContextMenu_;
    option.SetOnFinishEvent([distance, weakManager = WeakClaim(this), containerId, dragWithContextMenu, isMenuShow]() {
        constexpr decltype(distance) MAX_DIS = 5.0;
        if (distance < MAX_DIS && (!dragWithContextMenu || !isMenuShow)) {
            auto dragDropManager = weakManager.Upgrade();
            if (dragDropManager) {
                dragDropManager->TransDragWindowToDragFwk(containerId);
            }
        }
    });
    auto renderContext = info_.imageNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    AnimationUtils::Animate(
        option,
        [renderContext, localPoint = newOffset, info = info_, overlayManager]() {
            renderContext->UpdateTransformTranslate({ localPoint.GetX(), localPoint.GetY(), 0.0f });
            UpdateGatherNodePosition(overlayManager, info.imageNode);
            UpdateTextNodePosition(info.textNode, localPoint);
        },
        option.GetOnFinishEvent());
}

void DragDropManager::DoDragStartAnimation(const RefPtr<OverlayManager>& overlayManager, const GestureEvent& event)
{
    CHECK_NULL_VOID(overlayManager);
    if (!(GetDragPreviewInfo(overlayManager, info_)) || !IsNeedDisplayInSubwindow()) {
        if (isDragWithContextMenu_) {
            isDragFwkShow_ = false;
        }
        return;
    }
    CHECK_NULL_VOID(info_.imageNode);
    auto containerId = Container::CurrentId();
    isDragFwkShow_ = false;
    ResetPullMoveReceivedForCurrentDrag();
    auto distance = CalcDragPreviewDistanceWithPoint(PRESERVE_HEIGHT, event.GetGlobalLocation().GetX(),
        event.GetGlobalLocation().GetY(), info_);
    auto gatherNodeCenter = info_.imageNode->GetPaintRectCenter();
    auto maxDistance = CalcGatherNodeMaxDistanceWithPoint(overlayManager, gatherNodeCenter.GetX(),
        gatherNodeCenter.GetY());
    constexpr decltype(distance) MAX_DIS = 5.0;
    if ((distance < MAX_DIS || !IsNeedScaleDragPreview()) &&
        (maxDistance < MAX_DIS || (!isMouseDragged_ && !isTouchGatherAnimationPlaying_))) {
        TransDragWindowToDragFwk(containerId);
        return;
    }
    Offset newOffset = CalcDragMoveOffset(PRESERVE_HEIGHT, static_cast<int32_t>(event.GetGlobalLocation().GetX()),
        static_cast<int32_t>(event.GetGlobalLocation().GetY()), info_);
    nanoPreDragMoveAnimationTime_ = GetSysTimestamp();
    prePointerOffset_ = { newOffset.GetX(), newOffset.GetY() };
    AnimationOption option;
    const RefPtr<Curve> curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(0.347f, 0.99f, 0.0f);
    constexpr int32_t animateDuration = 300;
    option.SetCurve(curve);
    option.SetDuration(animateDuration);
    option.SetOnFinishEvent([weakManager = WeakClaim(this), containerId]() {
        auto dragDropManager = weakManager.Upgrade();
        if (dragDropManager && !dragDropManager->IsPullMoveReceivedForCurrentDrag()) {
            dragDropManager->TransDragWindowToDragFwk(containerId);
        }
    });
    auto renderContext = info_.imageNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    AnimationUtils::Animate(
        option,
        [renderContext, info = info_, newOffset, overlayManager, gatherNodeCenter]() {
            renderContext->UpdateTransformScale({ info.scale, info.scale });
            renderContext->UpdateTransformTranslate({ newOffset.GetX(), newOffset.GetY(), 0.0f });
            UpdateGatherNodeAttr(overlayManager, gatherNodeCenter, info.scale);
            UpdateTextNodePosition(info.textNode, newOffset);
        },
        option.GetOnFinishEvent());
}

void DragDropManager::FireOnEditableTextComponent(const RefPtr<FrameNode>& frameNode,
    DragEventType type)
{
    CHECK_NULL_VOID(frameNode);
    auto frameTag = frameNode->GetTag();
    if (!IsEditableTextComponent(frameTag)) {
        return;
    }

    if (type != DragEventType::ENTER && type != DragEventType::LEAVE) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "It is an invalid drag type %{public}d", type);
        return;
    }

    if (type == DragEventType::LEAVE) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "The current control has been dragged away.");
        hasNotifiedTransformation_ = false;
        return;
    }

    if (hasNotifiedTransformation_) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Coordinates have been transformed.");
        return;
    }

    auto ret = InteractionInterface::GetInstance()->EnterTextEditorArea(true);
    if (ret != 0) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Fail to notify entering text editor erea.");
        return;
    }

    hasNotifiedTransformation_ = true;
}

void DragDropManager::SetDragResult(
    const DragNotifyMsgCore& notifyMessage, const RefPtr<OHOS::Ace::DragEvent>& dragEvent)
{
    DragRet result = DragRet::DRAG_FAIL;
    switch (notifyMessage.result) {
        case DragRet::DRAG_SUCCESS:
            result = DragRet::DRAG_SUCCESS;
            break;
        case DragRet::DRAG_FAIL:
            result = DragRet::DRAG_FAIL;
            break;
        case DragRet::DRAG_CANCEL:
            result = DragRet::DRAG_CANCEL;
            break;
        default:
            break;
    }
    CHECK_NULL_VOID(dragEvent);
    dragEvent->SetResult(result);
}

void DragDropManager::SetDragBehavior(
    const DragNotifyMsgCore& notifyMessage, const RefPtr<OHOS::Ace::DragEvent>& dragEvent)
{
    DragBehavior dragBehavior = DragBehavior::UNKNOWN;
    switch (notifyMessage.dragBehavior) {
        case DragBehavior::COPY:
            dragBehavior = DragBehavior::COPY;
            break;
        case DragBehavior::MOVE:
            dragBehavior = DragBehavior::MOVE;
            break;
        default:
            break;
    }
    CHECK_NULL_VOID(dragEvent);
    dragEvent->SetDragBehavior(dragBehavior);
}

void DragDropManager::UpdateGatherNodeAttr(const RefPtr<OverlayManager>& overlayManager,
    OffsetF gatherNodeCenter, float scale)
{
    CHECK_NULL_VOID(overlayManager);
    auto gatherNodeChildrenInfo = overlayManager->GetGatherNodeChildrenInfo();
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(PIXELMAP_BORDER_RADIUS);
    borderRadius.multiValued = false;
    int i = 0;
    int cnt = static_cast<int>(gatherNodeChildrenInfo.size());
    for (const auto& child : gatherNodeChildrenInfo) {
        auto imageNode = child.imageNode.Upgrade();
        CHECK_NULL_VOID(imageNode);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        imageContext->UpdatePosition(OffsetT<Dimension>(
            Dimension(gatherNodeCenter.GetX() - child.halfWidth),
            Dimension(gatherNodeCenter.GetY() - child.halfHeight)));
        if (scale > 0) {
            imageContext->UpdateTransformScale({ scale, scale });
        }
        imageContext->UpdateBorderRadius(borderRadius);
        auto angle = 0.0f;
        if (i == cnt - FIRST_GATHER_PIXEL_MAP) {
            angle = FIRST_PIXELMAP_ANGLE;
            imageContext->UpdateOpacity(FIRST_PIXELMAP_OPACITY);
        } else if (i == cnt - SECOND_GATHER_PIXEL_MAP) {
            angle = SECOND_PIXELMAP_ANGLE;
            imageContext->UpdateOpacity(SECOND_PIXELMAP_OPACITY);
        } else {
            imageContext->UpdateOpacity(0.0);
        }

        i++;
        Vector5F rotate = Vector5F(0.0f, 0.0f, 1.0f, angle, 0.0f);
        imageContext->UpdateTransformRotate(rotate);
    }
}

void DragDropManager::UpdateGatherNodePosition(const RefPtr<OverlayManager>& overlayManager,
    const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(imageNode);
    auto gatherNodeCenter = imageNode->GetPaintRectCenter();
    CHECK_NULL_VOID(overlayManager);
    Dimension x = Dimension(0.0f);
    Dimension y = Dimension(0.0f);
    OffsetT<Dimension> offset(x, y);
    auto gatherNodeChildrenInfo = overlayManager->GetGatherNodeChildrenInfo();
    for (const auto& child : gatherNodeChildrenInfo) {
        auto imageNode = child.imageNode.Upgrade();
        CHECK_NULL_VOID(imageNode);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        x.SetValue(gatherNodeCenter.GetX() - child.halfWidth);
        y.SetValue(gatherNodeCenter.GetY() - child.halfHeight);
        offset.SetX(x);
        offset.SetY(y);
        imageContext->UpdatePosition(offset);
    }
}

void DragDropManager::UpdateTextNodePosition(const RefPtr<FrameNode>& textNode, const Offset& localPoint)
{
    CHECK_NULL_VOID(textNode);
    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateTransformTranslate({ localPoint.GetX(), localPoint.GetY(), 0.0f });
}

double DragDropManager::CalcGatherNodeMaxDistanceWithPoint(const RefPtr<OverlayManager>& overlayManager,
    int32_t x, int32_t y)
{
    CHECK_NULL_RETURN(overlayManager, 0.0f);
    auto gatherNodeChildrenInfo = overlayManager->GetGatherNodeChildrenInfo();
    double maxDistance = 0.0f;
    for (const auto& child : gatherNodeChildrenInfo) {
        auto imageNode = child.imageNode.Upgrade();
        CHECK_NULL_RETURN(imageNode, 0.0f);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_RETURN(imageContext, 0.0f);
        auto renderPosition = imageContext->GetPropertyOfPosition();
        double dis = sqrt(pow(renderPosition.GetX() + child.halfWidth - x, 2) +
            pow(renderPosition.GetY() + child.halfHeight - y, 2));
        maxDistance = std::max(maxDistance, dis);
    }
    return maxDistance;
}

bool DragDropManager::IsNeedDisplayInSubwindow()
{
    if (IsNeedScaleDragPreview()) {
        return true;
    }
    auto containerId = Container::CurrentId();
    auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(containerId);
    CHECK_NULL_RETURN(subwindow, false);
    auto overlayManager = subwindow->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    auto gatherNode = overlayManager->GetGatherNode();
    CHECK_NULL_RETURN(gatherNode, false);
    return true;
}

void DragDropManager::GetGatherPixelMap(const RefPtr<PixelMap>& pixelMap)
{
    gatherPixelMaps_.push_back(pixelMap);
}

void DragDropManager::PushGatherPixelMap(DragDataCore& dragData, float scale)
{
    for (auto gatherPixelMap : gatherPixelMaps_) {
        RefPtr<PixelMap> pixelMapDuplicated = gatherPixelMap;
#if defined(PIXEL_MAP_SUPPORTED)
        pixelMapDuplicated = PixelMap::CopyPixelMap(gatherPixelMap);
        if (!pixelMapDuplicated) {
            TAG_LOGW(AceLogTag::ACE_DRAG, "Copy PixelMap is failure!");
            pixelMapDuplicated = gatherPixelMap;
        }
#endif
        pixelMapDuplicated->Scale(scale, scale, AceAntiAliasingOption::HIGH);
        dragData.shadowInfos.push_back({pixelMapDuplicated, 0.0f, 0.0f});
    }
    gatherPixelMaps_.clear();
    return;
}

void DragDropManager::ResetDragDrop(int32_t windowId, const Point& point)
{
    DragDropRet dragDropRet { DragRet::DRAG_FAIL, isMouseDragged_, windowId, DragBehavior::UNKNOWN };
    ResetDragDropStatus(point, dragDropRet, windowId);
    dragCursorStyleCore_ = DragCursorStyleCore::DEFAULT;
}

void DragDropManager::FireOnDragLeave(
    const RefPtr<FrameNode>& preTargetFrameNode, const Point& point, const std::string& extraInfo)
{
    if (preTargetFrameNode) {
        auto preRect = preTargetFrameNode->GetTransformRectRelativeToWindow();
        // If the point is out of the pre node, it means we are totally inside a new node, notify leave anyway
        if (!preRect.IsInnerRegion(PointF(static_cast<float>(point.GetX()), static_cast<float>(point.GetY())))) {
            FireOnDragEvent(preTargetFrameNode, point, DragEventType::LEAVE, extraInfo);
            return;
        }

        // If reach here, it means we are entering one new node's area, but without leaving the area of the pre
        // one, this usually happens when moving from parent into its child.
        // Check the configuration to decide the notify to parent node.
        if (eventStrictReportingEnabled_) {
            FireOnDragEvent(preTargetFrameNode_, point, DragEventType::LEAVE, extraInfo);
        }
    }
}

bool DragDropManager::IsUIExtensionShowPlaceholder(const RefPtr<NG::UINode>& node)
{
#ifdef WINDOW_SCENE_SUPPORTED
    CHECK_NULL_RETURN(node, true);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, true);
    auto manager = pipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(manager, true);
    return manager->IsShowPlaceholder(node->GetId());
#endif
    return true;
}
} // namespace OHOS::Ace::NG

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

#include "core/components_ng/event/drag_event.h"

#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/interaction/interaction_data.h"
#include "core/common/interaction/interaction_interface.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/gesture_info.h"
#include "core/components_ng/gestures/recognizers/long_press_recognizer.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/gestures/recognizers/sequenced_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

#include "base/subwindow/subwindow_manager.h"
#include "core/animation/animation_pub.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components/theme/blur_style_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/render/adapter/component_snapshot.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/manager/drag_drop/utils/drag_animation_helper.h"

#ifdef WEB_SUPPORTED
#include "core/components_ng/pattern/web/web_pattern.h"
#endif // WEB_SUPPORTED

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PAN_FINGER = 1;
constexpr double PAN_DISTANCE = 5.0;
constexpr int32_t LONG_PRESS_DURATION = 500;
constexpr int32_t PREVIEW_LONG_PRESS_RECONGNIZER = 800;
constexpr Dimension FILTER_VALUE(0.0f);
constexpr float PIXELMAP_DRAG_SCALE_MULTIPLE = 1.05f;
constexpr int32_t PIXELMAP_ANIMATION_TIME = 800;
constexpr float SCALE_NUMBER = 0.95f;
constexpr int32_t FILTER_TIMES = 250;
constexpr int32_t PRE_DRAG_TIMER_DEADLINE = 50;
constexpr int32_t PIXELMAP_ANIMATION_DURATION = 300;
constexpr float SPRING_RESPONSE = 0.416f;
constexpr float SPRING_DAMPING_FRACTION = 0.73f;
constexpr Dimension PIXELMAP_BORDER_RADIUS = 16.0_vp;
constexpr Dimension PREVIEW_BORDER_RADIUS = 12.0_vp;
constexpr float BLUR_SIGMA_SCALE = 0.57735f;
constexpr float SCALE_HALF = 0.5f;
constexpr float DEFAULT_ANIMATION_SCALE = 0.95f;
constexpr Dimension BADGE_RELATIVE_OFFSET = 8.0_vp;
constexpr float DEFAULT_OPACITY = 0.95f;
constexpr float MIN_OPACITY { 0.0f };
constexpr float MAX_OPACITY { 1.0f };
#if defined(PIXEL_MAP_SUPPORTED)
constexpr int32_t CREATE_PIXELMAP_TIME = 80;
#endif
} // namespace

DragEventActuator::DragEventActuator(
    const WeakPtr<GestureEventHub>& gestureEventHub, PanDirection direction, int32_t fingers, float distance)
    : gestureEventHub_(gestureEventHub), direction_(direction), fingers_(fingers), distance_(distance)
{
    if (fingers_ < PAN_FINGER) {
        fingers_ = PAN_FINGER;
    }

    if (LessOrEqual(distance_, PAN_DISTANCE)) {
        distance_ = PAN_DISTANCE;
    }

    panRecognizer_ = MakeRefPtr<PanRecognizer>(fingers_, direction_, distance_);
    panRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    longPressRecognizer_ = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, fingers_, false, true);
    longPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_ =
        AceType::MakeRefPtr<LongPressRecognizer>(PREVIEW_LONG_PRESS_RECONGNIZER, fingers_, false, true);
    previewLongPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_->SetThumbnailDeadline(PRE_DRAG_TIMER_DEADLINE);
    isNotInPreviewState_ = false;
}

void DragEventActuator::StartDragTaskForWeb(const GestureEvent& info)
{
    auto gestureInfo = const_cast<GestureEvent&>(info);
    if (actionStart_) {
        TAG_LOGI(AceLogTag::ACE_WEB, "DragDrop start drag task for web success");
        actionStart_(gestureInfo);
    } else {
        TAG_LOGE(AceLogTag::ACE_WEB, "DragDrop start drag task for web failed,"
            "because actionStart function is null");
    }
}

void DragEventActuator::StartLongPressActionForWeb()
{
    if (!isReceivedLongPress_) {
        TAG_LOGW(AceLogTag::ACE_DRAG, "not received long press action, don't start long press action for web");
        return;
    }
    if (longPressUpdate_) {
        longPressUpdate_(longPressInfo_);
    }
    isReceivedLongPress_ = false;
}

void DragEventActuator::CancelDragForWeb()
{
    if (actionCancel_) {
        actionCancel_();
    }
}

/**
 * Do some nessessary check before returning the gesture recognizer collection result
 * to parent during the hittest process. For example, if there is one drag operation
 * already in our system, it is not allowed to start new interation for drag operation.
 */
bool DragEventActuator::IsGlobalStatusSuitableForDragging()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_RETURN(dragDropManager, false);
    if (dragDropManager->IsDragging() || dragDropManager->IsMsdpDragging()) {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "No need to collect drag gestures result, dragging is %{public}d,"
            "MSDP dragging is %{public}d",
            dragDropManager->IsDragging(), dragDropManager->IsMsdpDragging());
        return false;
    }

    return true;
}

/**
 * check the current node's status to decide if it can initiate one drag operation
 */
bool DragEventActuator::IsCurrentNodeStatusSuitableForDragging(
    const RefPtr<FrameNode>& frameNode, const TouchRestrict& touchRestrict)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, false);
    if (gestureHub->IsDragForbidden() || (!frameNode->IsDraggable() && frameNode->IsCustomerSet()) ||
        IsBelongToMultiItemNode(frameNode) || touchRestrict.inputEventType == InputEventType::AXIS) {
        TAG_LOGI(AceLogTag::ACE_DRAG,
            "No need to collect drag gestures result, drag forbidden set is %{public}d,"
            "frameNode draggable is %{public}d, custom set is %{public}d",
            gestureHub->IsDragForbidden(), frameNode->IsDraggable(), frameNode->IsCustomerSet());
        return false;
    }

    return true;
}

void DragEventActuator::OnCollectTouchTarget(const OffsetF& coordinateOffset, const TouchRestrict& touchRestrict,
    const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result)
{
    CHECK_NULL_VOID(userCallback_);
    isDragUserReject_ = false;
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->SetPrepareDragFrameNode(nullptr);
    if (!IsGlobalStatusSuitableForDragging() || !IsCurrentNodeStatusSuitableForDragging(frameNode, touchRestrict)) {
        return;
    }
    dragDropManager->SetPreDragStatus(PreDragStatus::ACTION_DETECTING_STATUS);
    auto actionStart = [weak = WeakClaim(this), this](GestureEvent& info) {
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        if (dragDropManager->IsDragging() || dragDropManager->IsMsdpDragging()) {
            TAG_LOGI(AceLogTag::ACE_DRAG,
                "It's already dragging now, dragging is %{public}d, MSDP dragging is %{public}d",
                dragDropManager->IsDragging(), dragDropManager->IsMsdpDragging());
            return;
        }
        dragDropManager->SetHasGatherNode(false);
        dragDropManager->ResetDragging(DragDropMgrState::ABOUT_TO_PREVIEW);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto renderContext = frameNode->GetRenderContext();
        if (info.GetSourceDevice() != SourceType::MOUSE) {
            if (gestureHub->GetTextDraggable()) {
                auto pattern = frameNode->GetPattern<TextBase>();
                CHECK_NULL_VOID(pattern);
                if (!pattern->IsSelected()) {
                    dragDropManager->ResetDragging();
                    gestureHub->SetIsTextDraggable(false);
                    return;
                }
                if (gestureHub->GetIsTextDraggable()) {
                    SetTextPixelMap(gestureHub);
                } else {
                    gestureHub->SetPixelMap(nullptr);
                }
            } else if (!isNotInPreviewState_) {
                if (gestureHub->GetTextDraggable()) {
                    HideTextAnimation(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                } else {
                    HideEventColumn();
                    HidePixelMap(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                    HideFilter();
                    SubwindowManager::GetInstance()->HideMenuNG(false, true);
                }
            }
        }

        if (info.GetSourceDevice() == SourceType::MOUSE) {
            frameNode->MarkModifyDone();
            dragDropManager->SetIsShowBadgeAnimation(true);
            auto pattern = frameNode->GetPattern<TextBase>();
            if (gestureHub->GetTextDraggable() && pattern) {
                if (!pattern->IsSelected() || pattern->GetMouseStatus() == MouseStatus::MOVE) {
                    dragDropManager->ResetDragging();
                    gestureHub->SetIsTextDraggable(false);
                    return;
                }
                if (pattern->BetweenSelectedPosition(info.GetGlobalLocation())) {
                    if (textDragCallback_) {
                        textDragCallback_(info.GetGlobalLocation());
                    }
                } else if (!gestureHub->GetIsTextDraggable()) {
                    gestureHub->SetPixelMap(nullptr);
                }
            } else {
                // For the drag initiacating from mouse, there is no chance to execute the modifier in the floating
                // pharse, so need to update here to make sure the preview option is available to pass through to drag
                // framwork later.
                actuator->UpdatePreviewOptionFromModifier(frameNode);
            }
        }

       // Trigger drag start event set by user.
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionStart = actuator->userCallback_->GetActionStartEventFunc();
        if (userActionStart) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger drag start event set by user.");
            userActionStart(info);
        }
        // Trigger custom drag start event
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionStart = actuator->customCallback_->GetActionStartEventFunc();
        if (customActionStart) {
            customActionStart(info);
        }
    };
    actionStart_ = actionStart;
    panRecognizer_->SetOnActionStart(actionStart);

    auto actionUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent panRecognizer onActionUpdate.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionUpdate = actuator->userCallback_->GetActionUpdateEventFunc();
        if (userActionUpdate) {
            userActionUpdate(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionUpdate = actuator->customCallback_->GetActionUpdateEventFunc();
        if (customActionUpdate) {
            customActionUpdate(info);
        }
    };
    panRecognizer_->SetOnActionUpdate(actionUpdate);

    auto actionEnd = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto dragDropManager = pipelineContext->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->SetHasGatherNode(false);
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (gestureHub->GetTextDraggable()) {
            actuator->textPixelMap_ = nullptr;
            actuator->HideTextAnimation();
        }
        auto userActionEnd = actuator->userCallback_->GetActionEndEventFunc();
        if (userActionEnd) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger drag end event set by user.");
            userActionEnd(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionEnd = actuator->customCallback_->GetActionEndEventFunc();
        if (customActionEnd) {
            customActionEnd(info);
        }
        actuator->SetIsNotInPreviewState(false);
    };
    panRecognizer_->SetOnActionEnd(actionEnd);
    auto actionCancel = [weak = WeakClaim(this), this]() {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Drag event has been canceled.");
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto dragDropManager = pipelineContext->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->SetHasGatherNode(false);
        dragDropManager->SetBadgeNumber(-1);
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        if (actuator->GetIsNotInPreviewState() && !gestureHub->GetTextDraggable()) {
            DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_CANCELED_BEFORE_DRAG, frameNode);
        }
        if (!GetIsBindOverlayValue(actuator)) {
            if (gestureHub->GetTextDraggable()) {
                if (gestureHub->GetIsTextDraggable()) {
                    textPixelMap_ = nullptr;
                    HideTextAnimation();
                }
            } else {
                auto renderContext = frameNode->GetRenderContext();
                BorderRadiusProperty borderRadius;
                if (renderContext->GetBorderRadius().has_value()) {
                    borderRadius.UpdateWithCheck(renderContext->GetBorderRadius().value());
                }
                borderRadius.multiValued = false;
                AnimationOption option;
                option.SetDuration(PIXELMAP_ANIMATION_DURATION);
                option.SetCurve(Curves::FRICTION);
                AnimationUtils::Animate(
                    option,
                    [renderContext_ = renderContext, borderRadius_ = borderRadius]() {
                        renderContext_->UpdateBorderRadius(borderRadius_);
                    },
                    option.GetOnFinishEvent());
                HidePixelMap();
                HideFilter();
            }
        } else {
            if (actuator->panRecognizer_->getDeviceType() == SourceType::MOUSE) {
                if (!gestureHub->GetTextDraggable()) {
                    HideEventColumn();
                    HidePixelMap();
                    HideFilter();
                }
            }
        }
        actuator->SetIsNotInPreviewState(false);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionCancel = actuator->userCallback_->GetActionCancelEventFunc();
        if (userActionCancel) {
            userActionCancel();
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionCancel = actuator->customCallback_->GetActionCancelEventFunc();
        if (customActionCancel) {
            customActionCancel();
        }
    };
    panRecognizer_->SetIsForDrag(true);
    panRecognizer_->SetMouseDistance(DRAG_PAN_DISTANCE_MOUSE.ConvertToPx());
    actionCancel_ = actionCancel;
    panRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    panRecognizer_->SetOnActionCancel(actionCancel);
    if (touchRestrict.sourceType == SourceType::MOUSE) {
        std::vector<RefPtr<NGGestureRecognizer>> recognizers { panRecognizer_ };
        SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
        SequencedRecognizer_->RemainChildOnResetStatus();
        SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
        SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
        result.emplace_back(SequencedRecognizer_);
        return;
    }
    auto longPressUpdateValue = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Trigger long press for 500ms.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->SetIsNotInPreviewState(true);
        if (actuator->userCallback_) {
            auto customLongPress = actuator->userCallback_->GetLongPressEventFunc();
            if (customLongPress) {
                customLongPress(info);
            }
        }
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto dragDropManager = pipelineContext->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->SetIsShowBadgeAnimation(true);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        if (!gestureHub->GetTextDraggable()) {
            // For the drag initiacating from long press gesture, the preview option set by the modifier
            // should also be applied in floating pharse, so we need to update the preview option here.
            actuator->UpdatePreviewOptionFromModifier(frameNode);
            DragEventActuator::ExecutePreDragAction(PreDragStatus::READY_TO_TRIGGER_DRAG_ACTION, frameNode);
        }
    };
    longPressRecognizer_->SetOnActionUpdate(longPressUpdateValue);
    auto longPressUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger long press for 800ms.");
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);

        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        if (overlayManager->GetHasPixelMap()) {
            return;
        }
        
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        if (dragDropManager->IsAboutToPreview() || dragDropManager->IsDragging()) {
            return;
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->isOnBeforeLiftingAnimation = false;
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto focusHub = frameNode->GetFocusHub();
        CHECK_NULL_VOID(focusHub);
        bool hasContextMenu = focusHub->FindContextMenuOnKeyEvent(OnKeyEventType::CONTEXT_MENU);
        if (gestureHub->GetTextDraggable()) {
            actuator->SetIsNotInPreviewState(false);
            if (gestureHub->GetIsTextDraggable()) {
                actuator->SetTextAnimation(gestureHub, info.GetGlobalLocation());
            }
            return;
        }

        bool isAllowedDrag = actuator->IsAllowedDrag();
        if (!isAllowedDrag) {
            actuator->longPressInfo_ = info;
            actuator->isReceivedLongPress_ = true;
            return;
        }
        auto dragPreviewOptions = frameNode->GetDragPreviewOption();
        auto badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
        if (badgeNumber.has_value()) {
            dragDropManager->SetBadgeNumber(badgeNumber.value());
        }
        dragDropManager->SetPrepareDragFrameNode(frameNode);
        bool isBindMenuPreview = gestureHub->GetPreviewMode() != MenuPreviewMode::NONE;
        if (!isBindMenuPreview) {
            actuator->SetFilter(actuator);
        }
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        actuator->SetIsNotInPreviewState(false);
        actuator->SetGatherNodeAboveFilter(actuator);
        actuator->SetPixelMap(actuator);
        auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(SPRING_RESPONSE, SPRING_DAMPING_FRACTION, 0);
        auto column = manager->GetPixelMapNode();
        CHECK_NULL_VOID(column);

        auto imageNode = AceType::DynamicCast<FrameNode>(column->GetFirstChild());
        CHECK_NULL_VOID(imageNode);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        if (!isBindMenuPreview) {
            if (actuator->IsNeedGather()) {
                DragAnimationHelper::PlayGatherAnimation(imageNode, manager);
                actuator->ShowPreviewBadgeAnimation(actuator, manager);
            }
            AnimationOption option;
            option.SetDuration(PIXELMAP_ANIMATION_TIME);
            option.SetCurve(motion);
            DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LIFT_STARTED);
            dragDropManager->SetDraggingPointer(info.GetPointerId());
            dragDropManager->SetDraggingPressedState(true);
            option.SetOnFinishEvent([id = Container::CurrentId(), pointerId = info.GetPointerId(), hasContextMenu]() {
                ContainerScope scope(id);
                auto pipeline = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(pipeline);
                auto dragDropManager = pipeline->GetDragDropManager();
                CHECK_NULL_VOID(dragDropManager);
                if (dragDropManager->IsDraggingPressed(pointerId) || hasContextMenu) {
                    DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LIFT_FINISHED);
                }
            });
            AnimationUtils::Animate(
                option,
                [imageContext]() {
                    imageContext->UpdateTransformScale({ PIXELMAP_DRAG_SCALE_MULTIPLE, PIXELMAP_DRAG_SCALE_MULTIPLE });
                },
                option.GetOnFinishEvent());
        } else {
            imageContext->UpdateOpacity(0.0);
        }
        actuator->SetEventColumn(actuator);
    };
    auto longPressCancel = [weak = WeakClaim(this)] {
        // remove drag overlay info by Cancel event.
        TAG_LOGD(AceLogTag::ACE_DRAG, "Long press event has been canceled.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->HideEventColumn();
        actuator->HidePixelMap(true, 0, 0, false);
        actuator->HideFilter();
        actuator->SetIsNotInPreviewState(false);
    };
    longPressUpdate_ = longPressUpdate;
    auto preDragCallback = [weak = WeakClaim(this)](Offset point) {
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        if (!gestureHub->GetTextDraggable()) {
            DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS, frameNode);
        }
    };
    previewLongPressRecognizer_->SetOnAction(longPressUpdate);
    previewLongPressRecognizer_->SetOnActionCancel(longPressCancel);
    previewLongPressRecognizer_->SetThumbnailCallback(std::move(preDragCallback));
    previewLongPressRecognizer_->SetGestureHub(gestureEventHub_);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    if (!longPressRecognizer_->HasThumbnailCallback() && isAllowedDrag) {
        auto callback = [weakPtr = gestureEventHub_, weak = WeakClaim(this)](Offset point) {
            auto gestureHub = weakPtr.Upgrade();
            CHECK_NULL_VOID(gestureHub);
            auto frameNode = gestureHub->GetFrameNode();
            CHECK_NULL_VOID(frameNode);
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto actuator = weak.Upgrade();
            CHECK_NULL_VOID(actuator);
            auto dragPreviewInfo = frameNode->GetDragPreview();
            if (dragPreviewInfo.inspectorId != "") {
                auto previewPixelMap = GetPreviewPixelMap(dragPreviewInfo.inspectorId, frameNode);
                gestureHub->SetPixelMap(previewPixelMap);
                gestureHub->SetDragPreviewPixelMap(previewPixelMap);
            } else if (dragPreviewInfo.pixelMap != nullptr) {
                gestureHub->SetPixelMap(dragPreviewInfo.pixelMap);
                gestureHub->SetDragPreviewPixelMap(dragPreviewInfo.pixelMap);
            } else if (dragPreviewInfo.customNode != nullptr) {
#if defined(PIXEL_MAP_SUPPORTED)
                bool hasImageNode = false;
                std::list<RefPtr<FrameNode>> imageNodes;
                gestureHub->PrintBuilderNode(dragPreviewInfo.customNode, hasImageNode, imageNodes);
                auto callback = [id = Container::CurrentId(), pipeline, gestureHub]
                    (std::shared_ptr<Media::PixelMap> pixelMap, int32_t arg, std::function<void()>) {
                    ContainerScope scope(id);
                    if (pixelMap != nullptr) {
                        auto customPixelMap = PixelMap::CreatePixelMap(reinterpret_cast<void*>(&pixelMap));
                        auto taskScheduler = pipeline->GetTaskExecutor();
                        CHECK_NULL_VOID(taskScheduler);
                        taskScheduler->PostTask([gestureHub, customPixelMap]() {
                            CHECK_NULL_VOID(gestureHub);
                            gestureHub->SetPixelMap(customPixelMap);
                            gestureHub->SetDragPreviewPixelMap(customPixelMap);
                        }, TaskExecutor::TaskType::UI, "ArkUIDragSetPixelMap");
                    }
                };

                OHOS::Ace::NG::ComponentSnapshot::Create(
                    dragPreviewInfo.customNode, std::move(callback), false, CREATE_PIXELMAP_TIME);
                gestureHub->CheckImageDecode(imageNodes);
                imageNodes.clear();
#endif
            } else {
                actuator->GetThumbnailPixelMapAsync(gestureHub);
            }
            auto longPressRecognizer = actuator->longPressRecognizer_;
            if (longPressRecognizer && longPressRecognizer->GetGestureDisposal() != GestureDisposal::REJECT) {
                CreateGatherNode(actuator);
                actuator->isOnBeforeLiftingAnimation = true;
                DragAnimationHelper::PlayGatherAnimationBeforeLifting(actuator);
                DragAnimationHelper::PlayNodeAnimationBeforeLifting(frameNode);
            } else {
                actuator->isOnBeforeLiftingAnimation = false;
            }
        };

        longPressRecognizer_->SetThumbnailCallback(std::move(callback));
    }
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        if (info.GetTouches().front().GetTouchType() == TouchType::UP) {
            actuator->HandleTouchUpEvent();
        } else if (info.GetTouches().front().GetTouchType() == TouchType::CANCEL) {
            actuator->HandleTouchCancelEvent();
        } else if (info.GetTouches().front().GetTouchType() == TouchType::MOVE) {
            actuator->HandleTouchMoveEvent();
        }
    };
    auto touchListener = AceType::MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchListener);
    std::vector<RefPtr<NGGestureRecognizer>> recognizers { longPressRecognizer_, panRecognizer_ };
    SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
    SequencedRecognizer_->RemainChildOnResetStatus();
    previewLongPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    longPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
    result.emplace_back(SequencedRecognizer_);
    result.emplace_back(previewLongPressRecognizer_);
}

void DragEventActuator::SetFilter(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start setFilter.");
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto parent = frameNode->GetParent();
    CHECK_NULL_VOID(parent);
    while (parent && parent->GetDepth() != 1) {
        parent = parent->GetParent();
    }
    if (!parent) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "DragFrameNode is %{public}s, depth %{public}d, can not find filter root",
            frameNode->GetTag().c_str(), frameNode->GetDepth());
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (!manager->GetHasFilter() && !manager->GetIsOnAnimation()) {
        if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
#ifdef WEB_SUPPORTED
            auto webPattern = frameNode->GetPattern<WebPattern>();
            CHECK_NULL_VOID(webPattern);
            bool isWebmageDrag = webPattern->IsImageDrag();
            CHECK_NULL_VOID(isWebmageDrag && SystemProperties::GetDeviceType() == DeviceType::PHONE);
#endif
        } else {
            bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
            CHECK_NULL_VOID(isBindOverlayValue && SystemProperties::GetDeviceType() == DeviceType::PHONE);
        }
        // insert columnNode to rootNode
        auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        columnNode->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
        // set filter
        TAG_LOGI(AceLogTag::ACE_DRAG, "User Device use default Filter");
        auto container = Container::Current();
        if (container && container->IsScenceBoardWindow()) {
            auto windowScene = manager->FindWindowScene(frameNode);
            manager->MountFilterToWindowScene(columnNode, windowScene);
        } else {
            columnNode->MountToParent(parent);
            columnNode->OnMountToParentDone();
            manager->SetHasFilter(true);
            manager->SetFilterColumnNode(columnNode);
            parent->MarkDirtyNode(NG::PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
        AnimationOption option;
        BlurStyleOption styleOption;
        styleOption.blurStyle = static_cast<BlurStyle>(BlurStyle::BACKGROUND_THIN);
        styleOption.colorMode = static_cast<ThemeColorMode>(static_cast<int32_t>(ThemeColorMode::SYSTEM));
        option.SetDuration(FILTER_TIMES);
        option.SetCurve(Curves::SHARP);
        option.SetOnFinishEvent([pipelineWeak = WeakClaim(RawPtr(pipelineContext))] {
            auto pipelineContext = pipelineWeak.Upgrade();
            CHECK_NULL_VOID(pipelineContext);
            auto manager = pipelineContext->GetOverlayManager();
            CHECK_NULL_VOID(manager);
            manager->SetFilterActive(false);
        });
        columnNode->GetRenderContext()->UpdateBackBlurRadius(FILTER_VALUE);
        AnimationUtils::Animate(
            option, [columnNode, styleOption]() { columnNode->GetRenderContext()->UpdateBackBlurStyle(styleOption); },
            option.GetOnFinishEvent());
    }
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set filter success.");
}

OffsetF DragEventActuator::GetFloatImageOffset(const RefPtr<FrameNode>& frameNode, const RefPtr<PixelMap>& pixelMap)
{
    CHECK_NULL_RETURN(frameNode, OffsetF());
    auto centerPosition = frameNode->GetPaintRectCenter(true);
    float width = 0.0f;
    float height = 0.0f;
    if (pixelMap) {
        width = pixelMap->GetWidth();
        height = pixelMap->GetHeight();
    }
    auto offsetX = centerPosition.GetX() - width / 2.0f;
    auto offsetY = centerPosition.GetY() - height / 2.0f;
#ifdef WEB_SUPPORTED
    if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
        auto webPattern = frameNode->GetPattern<WebPattern>();
        if (webPattern) {
            auto offsetToWindow = frameNode->GetPaintRectOffset();
            offsetX = offsetToWindow.GetX() + webPattern->GetDragOffset().GetX();
            offsetY = offsetToWindow.GetY() + webPattern->GetDragOffset().GetY();
        }
    }
#endif
    // Check web tag.
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, OffsetF());
    if (pipelineContext->HasFloatTitle()) {
        offsetX -= static_cast<float>((CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx());
        offsetY -= static_cast<float>((pipelineContext->GetCustomTitleHeight() + CONTAINER_BORDER_WIDTH).ConvertToPx());
    }
    return OffsetF(offsetX, offsetY);
}

void DragEventActuator::UpdatePreviewPositionAndScale(const RefPtr<FrameNode>& imageNode, const OffsetF& frameOffset)
{
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(frameOffset.GetX()), Dimension(frameOffset.GetY())));
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);
}

void DragEventActuator::UpdatePreviewAttr(const RefPtr<FrameNode>& frameNode, const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(frameNode);
    auto frameTag = frameNode->GetTag();
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (gestureHub->IsTextCategoryComponent(frameTag) && gestureHub->GetTextDraggable()) {
        return;
    }
    CHECK_NULL_VOID(imageNode);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    if (dragPreviewOption.options.shadow.has_value()) {
        imageContext->UpdateBackShadow(dragPreviewOption.options.shadow.value());
    }
    if (dragPreviewOption.options.borderRadius.has_value()) {
        imageContext->UpdateBorderRadius(dragPreviewOption.options.borderRadius.value());
        imageContext->UpdateClipEdge(true);
    }
    auto optionsFromModifier = frameNode->GetDragPreviewOption().options;
    if (optionsFromModifier.blurbgEffect.backGroundEffect.radius.IsValid()) {
        ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundEffect, optionsFromModifier.blurbgEffect.backGroundEffect, frameNode);
    }
}

void DragEventActuator::CreatePreviewNode(const RefPtr<FrameNode>& frameNode, OHOS::Ace::RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(frameNode);
    auto pixelMap = frameNode->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    auto frameOffset = frameNode->GetOffsetInScreen();
    imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() {return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(imageNode);
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());

    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(pixelMap->GetWidth()), NG::CalcLength(pixelMap->GetHeight()));
    props->UpdateUserDefinedIdealSize(targetSize);

    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    CHECK_NULL_VOID(imagePattern);
    imagePattern->SetSyncLoad(true);

    UpdatePreviewPositionAndScale(imageNode, frameOffset);
    UpdatePreviewAttr(frameNode, imageNode);
    imageNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    imageNode->SetLayoutDirtyMarked(true);
    imageNode->SetActive(true);
    imageNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
}

void DragEventActuator::SetPreviewDefaultAnimateProperty(const RefPtr<FrameNode>& imageNode)
{
    if (imageNode->IsPreviewNeedScale()) {
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        imageContext->UpdateTransformScale({ 1.0f, 1.0f });
        imageContext->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });
    }
}

void DragEventActuator::MountPixelMap(const RefPtr<OverlayManager>& manager, const RefPtr<GestureEventHub>& gestureHub,
    const RefPtr<FrameNode>& imageNode, const RefPtr<FrameNode>& textNode)
{
    CHECK_NULL_VOID(manager);
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(gestureHub);
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(imageNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    hub->SetPixelMap(gestureHub->GetPixelMap());
    if (textNode) {
        columnNode->AddChild(textNode);
    }
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountPixelMapToWindowScene(columnNode, windowScene);
    } else {
        manager->MountPixelMapToRootNode(columnNode);
    }
    SetPreviewDefaultAnimateProperty(imageNode);
    columnNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
    columnNode->MarkModifyDone();
    columnNode->SetActive(true);
    columnNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
}

/* Retrieves a preview PixelMap for a given drag event action.
 * This function attempts to obtain a screenshot of a frameNode associated with an inspector ID.
 * If the frameNode with the given ID does not exist or hasn't been rendered,
 * it falls back to taking a screenshot of the provided frame node.
 *
 * @param inspectorId A string representing the unique identifier for the frameNode's ID.
 * @param selfFrameNode A RefPtr to the frame node associated with the drag event.
 * @return A RefPtr to a PixelMap containing the preview image, or nullptr if not found.
 */
RefPtr<PixelMap> DragEventActuator::GetPreviewPixelMap(
    const std::string& inspectorId, const RefPtr<FrameNode>& selfFrameNode)
{
    // Attempt to retrieve the PixelMap using the inspector ID.
    auto previewPixelMap = GetPreviewPixelMapByInspectorId(inspectorId);

    // If a preview PixelMap was found, return it.
    if (previewPixelMap != nullptr) {
        return previewPixelMap;
    }

    // If not found by inspector ID, attempt to get a screenshot of the frame node.
    return GetScreenShotPixelMap(selfFrameNode);
}

/* Retrieves a preview PixelMap based on an inspector ID.
 * This function attempts to find a frame node associated with the given inspector ID and then takes a screenshot of it.
 *
 * @param inspectorId The unique identifier for a frameNode.
 * @return A RefPtr to a PixelMap containing the preview image, or nullptr if not found or the ID is empty.
 */
RefPtr<PixelMap> DragEventActuator::GetPreviewPixelMapByInspectorId(const std::string& inspectorId)
{
    // Check for an empty inspector ID and return nullptr if it is empty.
    if (inspectorId == "") {
        return nullptr;
    }

    // Retrieve the frame node using the inspector's ID.
    auto dragPreviewFrameNode = Inspector::GetFrameNodeByKey(inspectorId);
    CHECK_NULL_RETURN(dragPreviewFrameNode, nullptr);

    auto layoutProperty = dragPreviewFrameNode->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, nullptr);

    auto visibility = layoutProperty->GetVisibilityValue(VisibleType::VISIBLE);
    if (visibility == VisibleType::INVISIBLE || visibility == VisibleType::GONE) {
        return nullptr;
    }

    // Take a screenshot of the frame node and return it as a PixelMap.
    return GetScreenShotPixelMap(dragPreviewFrameNode);
}


/* Captures a screenshot of the specified frame node and encapsulates it in a PixelMap.
 *
 * @param frameNode A RefPtr reference to the frame node from which to capture the screenshot.
 * @return A RefPtr to a PixelMap containing the screenshot.
 */
RefPtr<PixelMap> DragEventActuator::GetScreenShotPixelMap(const RefPtr<FrameNode>& frameNode)
{
    // Ensure the frame node is not nulls before proceeding.
    CHECK_NULL_RETURN(frameNode, nullptr);

    // Obtain the rendering context from the frame node.
    auto context = frameNode->GetRenderContext();

    // If the rendering context is not available, return nullptr.
    CHECK_NULL_RETURN(context, nullptr);

    // Capture and return the thumbnail PixelMap from the rendering context.
    return context->GetThumbnailPixelMap(true);
}

void DragEventActuator::SetPixelMap(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set pixelMap");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasPixelMap()) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Dragging animation is currently executing, unable to float other pixelMap.");
        return;
    }
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    RefPtr<PixelMap> pixelMap = gestureHub->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    auto width = pixelMap->GetWidth();
    auto height = pixelMap->GetHeight();
    auto offset = GetFloatImageOffset(frameNode, pixelMap);
    // create imageNode
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(imageNode);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    CHECK_NULL_VOID(imagePattern);
    imagePattern->SetSyncLoad(true);
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(offset.GetX()), Dimension(offset.GetY())));
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(imageNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    hub->SetPixelMap(gestureHub->GetPixelMap());
    // mount to rootNode
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountPixelMapToWindowScene(columnNode, windowScene);
    } else {
        manager->MountPixelMapToRootNode(columnNode);
    }
    imageNode->MarkModifyDone();
    imageNode->SetLayoutDirtyMarked(true);
    imageNode->SetActive(true);
    imageNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    bool hasContextMenu = focusHub->FindContextMenuOnKeyEvent(OnKeyEventType::CONTEXT_MENU);
    ShowPixelMapAnimation(imageNode, frameNode, hasContextMenu);
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set pixelMap success.");
    SetPreviewDefaultAnimateProperty(imageNode);
}

// called when the preview floating or drag begin, this method will try to execute the modifier
// on one temporary frame node, and then get the value user setted from the modifier
void DragEventActuator::UpdatePreviewOptionFromModifier(const RefPtr<FrameNode>& frameNode)
{
    UpdatePreviewOptionDefaultAttr(frameNode);
    auto modifierOnApply = frameNode->GetDragPreviewOption().onApply;
    if (modifierOnApply == nullptr) {
        TAG_LOGE(AceLogTag::ACE_DRAG, "OnApply is null");
        return;
    }

    // create one temporary frame node for receiving the value from the modifier
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(imageNode);

    // execute the modifier
    modifierOnApply(WeakClaim(RawPtr(imageNode)));

    // get values from the temporary frame node
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto opacity = imageContext->GetOpacity();

    OptionsAfterApplied options;
    if (opacity.has_value() && (opacity.value()) <= MAX_OPACITY && (opacity.value()) > MIN_OPACITY) {
        options.opacity = opacity.value();
    } else {
        options.opacity = DEFAULT_OPACITY;
    }

    auto shadow = imageContext->GetBackShadow();
    if (shadow.has_value()) {
        options.shadow = shadow;
    }

    auto borderRadius = imageContext->GetBorderRadius();
    if (borderRadius.has_value()) {
        options.borderRadius = borderRadius;
    }

    // get the old preview option
    DragPreviewOption dragPreviewOption = frameNode->GetDragPreviewOption();
    auto bgEffect = imageContext->GetBackgroundEffect();
    if (bgEffect.has_value()) {
        options.blurbgEffect.backGroundEffect = bgEffect.value();
    } else {
        auto blurstyletmp = imageContext->GetBackBlurStyle();
        if (blurstyletmp.has_value()) {
            bgEffect = BrulStyleToEffection(blurstyletmp);
            if (bgEffect.has_value()) {
                options.blurbgEffect.backGroundEffect = bgEffect.value();
            }
        }
    }
    dragPreviewOption.options = options; // replace the options with the new one after applied
    frameNode->SetDragPreviewOptions(dragPreviewOption);
}

void DragEventActuator::UpdatePreviewOptionDefaultAttr(const RefPtr<FrameNode>& frameNode)
{
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    dragPreviewOption.options.opacity = DEFAULT_OPACITY;
    if (dragPreviewOption.isDefaultShadowEnabled) {
        dragPreviewOption.options.shadow = GetDefaultShadow();
    } else {
        dragPreviewOption.options.shadow = std::nullopt;
    }
    if (dragPreviewOption.isDefaultRadiusEnabled) {
        dragPreviewOption.options.borderRadius = GetDefaultBorderRadius();
    } else {
        dragPreviewOption.options.borderRadius = std::nullopt;
    }
    frameNode->SetDragPreviewOptions(dragPreviewOption); // replace the options with the new one
}

float DragEventActuator::RadiusToSigma(float radius)
{
    return GreatNotEqual(radius, 0.0f) ? BLUR_SIGMA_SCALE * radius + SCALE_HALF : 0.0f;
}

std::optional<EffectOption> DragEventActuator::BrulStyleToEffection(const std::optional<BlurStyleOption>& blurStyleOp)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, std::nullopt);
    auto blurStyleTheme = pipeline->GetTheme<BlurStyleTheme>();
    if (!blurStyleTheme) {
        LOGW("cannot find theme of blurStyle, create blurStyle failed");
        return std::nullopt;
    }
    ThemeColorMode colorMode = blurStyleOp->colorMode;
    if (blurStyleOp->colorMode == ThemeColorMode::SYSTEM) {
        colorMode = SystemProperties::GetColorMode() == ColorMode::DARK ? ThemeColorMode::DARK : ThemeColorMode::LIGHT;
    }
    auto blurParam = blurStyleTheme->GetBlurParameter(blurStyleOp->blurStyle, colorMode);
    CHECK_NULL_RETURN(blurParam, std::nullopt);
    auto ratio = blurStyleOp->scale;
    auto maskColor = blurParam->maskColor.BlendOpacity(ratio);
    auto radiusPx = blurParam->radius * pipeline->GetDipScale();
    auto radiusBlur = RadiusToSigma(radiusPx) * ratio;
    auto saturation = (blurParam->saturation - 1) * ratio + 1.0;
    auto brightness = (blurParam->brightness - 1) * ratio + 1.0;
    Dimension dimen(radiusBlur);
    EffectOption bgEffection = {dimen, saturation, brightness, maskColor,
        blurStyleOp->adaptiveColor, blurStyleOp->blurOption};
    return std::optional<EffectOption>(bgEffection);
}

void DragEventActuator::ApplyNewestOptionExecutedFromModifierToNode(
    const RefPtr<FrameNode>& optionHolderNode, const RefPtr<FrameNode>& targetNode)
{
    auto optionsFromModifier = targetNode->GetDragPreviewOption().options;
    ACE_UPDATE_NODE_RENDER_CONTEXT(Opacity, optionsFromModifier.opacity, targetNode);
    if (optionsFromModifier.blurbgEffect.backGroundEffect.radius.IsValid()) {
        ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundEffect, optionsFromModifier.blurbgEffect.backGroundEffect, targetNode);
    }
    if (optionsFromModifier.shadow.has_value()) {
        // if shadow is unfilled, set shadow after animation
        if (optionsFromModifier.shadow->GetIsFilled()) {
            ACE_UPDATE_NODE_RENDER_CONTEXT(BackShadow, optionsFromModifier.shadow.value(), targetNode);
        }
    }

    const auto& target = targetNode->GetRenderContext();
    if (optionsFromModifier.borderRadius.has_value()) {
        target->UpdateBorderRadius(optionsFromModifier.borderRadius.value());
        target->UpdateClipEdge(true);
    }
}

void DragEventActuator::SetEventColumn(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set eventColumn.");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasEvent()) {
        return;
    }
    auto rootNode = pipelineContext->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto geometryNode = rootNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto width = geometryNode->GetFrameSize().Width();
    auto height = geometryNode->GetFrameSize().Height();
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto props = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    BindClickEvent(columnNode);
    columnNode->MarkModifyDone();
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountEventToWindowScene(columnNode, windowScene);
    } else {
        manager->MountEventToRootNode(columnNode);
    }
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set eventColumn success.");
}

void DragEventActuator::HideFilter()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveFilterAnimation();
}

void DragEventActuator::HidePixelMap(bool startDrag, double x, double y, bool showAnimation)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (!startDrag) {
        manager->RemovePreviewBadgeNode();
        manager->RemoveGatherNodeWithAnimation();
    }
    if (showAnimation) {
        manager->RemovePixelMapAnimation(startDrag, x, y);
    } else {
        manager->RemovePixelMap();
    }
}

void DragEventActuator::HideEventColumn()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveEventColumn();
}

void DragEventActuator::BindClickEvent(const RefPtr<FrameNode>& columnNode)
{
    auto callback = [this, weak = WeakClaim(this)](GestureEvent& /* info */) {
        HideEventColumn();
        HidePixelMap();
        HideFilter();
    };
    auto columnGestureHub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(columnGestureHub);
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(callback));
    columnGestureHub->AddClickEvent(clickListener);
    if (!GetIsBindOverlayValue(Claim(this))) {
        columnGestureHub->SetHitTestMode(HitTestMode::HTMBLOCK);
    }
}

void DragEventActuator::ShowPixelMapAnimation(const RefPtr<FrameNode>& imageNode,
    const RefPtr<FrameNode>& frameNode, bool hasContextMenu)
{
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    SetImageNodeInitAttr(frameNode, imageNode);
    // pixel map animation
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    option.SetOnFinishEvent([imageNode, frameNode]() {
        DragEventActuator::SetImageNodeFinishAttr(frameNode, imageNode);
    });
    
    AnimationUtils::Animate(
        option,
        [imageNode, hasContextMenu, frameNode]() mutable {
            auto imageContext = imageNode->GetRenderContext();
            CHECK_NULL_VOID(imageContext);
            ApplyNewestOptionExecutedFromModifierToNode(frameNode, imageNode);
            if (hasContextMenu) {
                BorderRadiusProperty borderRadius;
                borderRadius.SetRadius(PIXELMAP_BORDER_RADIUS);
                imageContext->UpdateBorderRadius(borderRadius);
            }
            DragEventActuator::ResetNode(frameNode);
        },
        option.GetOnFinishEvent());
}

void DragEventActuator::ExecutePreDragAction(const PreDragStatus preDragStatus, const RefPtr<FrameNode>& frameNode)
{
    auto mainPipeline = PipelineContext::GetMainPipelineContext();
    CHECK_NULL_VOID(mainPipeline);
    auto dragDropManager = mainPipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    if (dragDropManager->IsDragging() || dragDropManager->IsMsdpDragging()) {
        return;
    }
    RefPtr<EventHub> eventHub;
    if (frameNode) {
        eventHub = frameNode->GetEventHub<EventHub>();
    } else {
        auto preDragFrameNode = dragDropManager->GetPrepareDragFrameNode().Upgrade();
        CHECK_NULL_VOID(preDragFrameNode);
        eventHub = preDragFrameNode->GetEventHub<EventHub>();
    }
    CHECK_NULL_VOID(eventHub);
    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (!gestureHub->IsAllowedDrag(eventHub) || gestureHub->GetTextDraggable()) {
        return;
    }
    auto onPreDragStatus = PreDragStatus::ACTION_CANCELED_BEFORE_DRAG;
    if (dragDropManager->GetPreDragStatus() <= preDragStatus &&
        preDragStatus != PreDragStatus::ACTION_CANCELED_BEFORE_DRAG) {
        auto nextPreDragStatus = static_cast<PreDragStatus>(static_cast<int32_t>(preDragStatus) + 1);
        dragDropManager->SetPreDragStatus(nextPreDragStatus);
        onPreDragStatus = preDragStatus;
    }
    auto onPreDragFunc = eventHub->GetOnPreDrag();
    CHECK_NULL_VOID(onPreDragFunc);
    if (preDragStatus == PreDragStatus::PREVIEW_LIFT_FINISHED ||
        preDragStatus == PreDragStatus::PREVIEW_LANDING_FINISHED) {
        auto taskScheduler = mainPipeline->GetTaskExecutor();
        CHECK_NULL_VOID(taskScheduler);
        taskScheduler->PostTask(
            [onPreDragStatus, callback = onPreDragFunc]() {
                CHECK_NULL_VOID(callback);
                callback(onPreDragStatus);
            },
            TaskExecutor::TaskType::UI, "ArkUIDragExecutePreDrag");
    } else {
        onPreDragFunc(onPreDragStatus);
    }
}

void DragEventActuator::SetThumbnailCallback(std::function<void(Offset)>&& callback)
{
    textDragCallback_ = callback;
    longPressRecognizer_->SetThumbnailCallback(std::move(callback));
}

void DragEventActuator::SetTextPixelMap(const RefPtr<GestureEventHub>& gestureHub)
{
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);

    auto dragNode = pattern->MoveDragNode();
    pattern->CloseSelectOverlay();
    CHECK_NULL_VOID(dragNode);
    auto pixelMap = dragNode->GetRenderContext()->GetThumbnailPixelMap();
    if (textPixelMap_) {
        gestureHub->SetPixelMap(textPixelMap_);
    } else if (pixelMap) {
        gestureHub->SetPixelMap(pixelMap);
    } else {
        gestureHub->SetPixelMap(nullptr);
    }
}

void DragEventActuator::SetTextAnimation(const RefPtr<GestureEventHub>& gestureHub, const Offset& globalLocation)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set textAnimation.");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->SetHasFilter(false);
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    auto textBase = frameNode->GetPattern<TextBase>();
    CHECK_NULL_VOID(pattern);
    CHECK_NULL_VOID(textBase);
    if (!textBase->BetweenSelectedPosition(globalLocation)) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Position is between selected position, stop set text animation.");
        return;
    }
    auto isHandlesShow = pattern->IsHandlesShow();
    auto dragNode = pattern->MoveDragNode();
    CHECK_NULL_VOID(dragNode);
    dragNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(dragNode);
    // mount to rootNode
    manager->MountPixelMapToRootNode(columnNode);
    auto textDragPattern = dragNode->GetPattern<TextDragPattern>();
    CHECK_NULL_VOID(textDragPattern);
    auto modifier = textDragPattern->GetOverlayModifier();
    CHECK_NULL_VOID(modifier);
    modifier->UpdateHandlesShowFlag(isHandlesShow);
    auto renderContext = dragNode->GetRenderContext();
    if (renderContext) {
        textPixelMap_ = renderContext->GetThumbnailPixelMap();
    }
    modifier->StartFloatingAnimate();
    pattern->CloseHandleAndSelect();
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set text animation success.");
}

void DragEventActuator::HideTextAnimation(bool startDrag, double globalX, double globalY)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start hide text animation.");
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    bool isAllowedDrag = IsAllowedDrag();
    if (!gestureHub->GetTextDraggable() || !isAllowedDrag) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Text is not draggable, stop set hide text animation.");
        return;
    }
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);
    auto node = pattern->MoveDragNode();
    CHECK_NULL_VOID(node);
    auto textDragPattern = node->GetPattern<TextDragPattern>();
    CHECK_NULL_VOID(textDragPattern);
    auto modifier = textDragPattern->GetOverlayModifier();
    CHECK_NULL_VOID(modifier);
    auto removeColumnNode = [id = Container::CurrentId(), startDrag, weakPattern = WeakPtr<TextDragBase>(pattern),
            weakEvent = gestureEventHub_, weakModifier = WeakPtr<TextDragOverlayModifier>(modifier)] {
        ContainerScope scope(id);
        if (!startDrag) {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto modifier = weakModifier.Upgrade();
            CHECK_NULL_VOID(modifier);
            pattern->ShowHandles(modifier->IsHandlesShow());
        }
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        manager->RemovePixelMap();
        TAG_LOGD(AceLogTag::ACE_DRAG, "In removeColumnNode callback, set DragWindowVisible true.");
        auto gestureHub = weakEvent.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (!gestureHub->IsPixelMapNeedScale()) {
            InteractionInterface::GetInstance()->SetDragWindowVisible(true);
        }
        gestureHub->SetPixelMap(nullptr);
    };
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    option.SetOnFinishEvent(removeColumnNode);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetOverlayManager();
    auto dragNode = manager->GetPixelMapNode();
    CHECK_NULL_VOID(dragNode);
    auto dragFrame = dragNode->GetGeometryNode()->GetFrameRect();
    auto frameWidth = dragFrame.Width();
    auto frameHeight = dragFrame.Height();
    auto pixelMap = gestureHub->GetPixelMap();
    float scale = 1.0f;
    if (pixelMap) {
        scale = gestureHub->GetPixelMapScale(pixelMap->GetHeight(), pixelMap->GetWidth());
    }
    auto context = dragNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateTransformScale(VectorF(1.0f, 1.0f));
    AnimationUtils::Animate(
        option,
        [context, startDrag, globalX, globalY, frameWidth, frameHeight, scale]() {
            if (startDrag) {
                context->UpdatePosition(OffsetT<Dimension>(Dimension(globalX + frameWidth * PIXELMAP_WIDTH_RATE),
                    Dimension(globalY + frameHeight * PIXELMAP_HEIGHT_RATE)));
                context->UpdateTransformScale(VectorF(scale, scale));
                context->OnModifyDone();
            }
        },
        option.GetOnFinishEvent());
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set hide text animation success.");
}

// update ImageNode default attr before floating
void DragEventActuator::SetImageNodeInitAttr(const RefPtr<FrameNode>& frameNode, const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(frameNode);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();

    // update default scale
    bool defaultAnimationBeforeLifting = dragPreviewOption.defaultAnimationBeforeLifting;
    if (defaultAnimationBeforeLifting) {
        auto layoutProperty = frameNode->GetLayoutProperty();
        if (layoutProperty) {
            layoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
        }
        imageContext->UpdateTransformScale({ DEFAULT_ANIMATION_SCALE, DEFAULT_ANIMATION_SCALE });
    } else {
        imageContext->UpdateTransformScale({ 1, 1 });
    }
    
    // update shadow
    auto shadow = Shadow::CreateShadow(ShadowStyle::None);
    if (dragPreviewOption.options.shadow.has_value()) {
        shadow = dragPreviewOption.options.shadow.value();
        shadow.SetColor(Color(0x00000000));
    }
    imageContext->UpdateBackShadow(shadow);
    
    // update radius
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(0.0_vp);
    imageContext->UpdateBorderRadius(borderRadius);
    
    // update opacity
    imageContext->UpdateOpacity(1.0f);
}

void DragEventActuator::SetImageNodeFinishAttr(const RefPtr<FrameNode>& frameNode,
    const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(frameNode);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    if (dragPreviewOption.options.shadow.has_value() && !dragPreviewOption.options.shadow->GetIsFilled()) {
        imageContext->UpdateBackShadow(dragPreviewOption.options.shadow.value());
    }
}

bool DragEventActuator::GetIsBindOverlayValue(const RefPtr<DragEventActuator>& actuator)
{
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, true);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, true);
    bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
    return isBindOverlayValue;
}

bool DragEventActuator::IsAllowedDrag()
{
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, false);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    return isAllowedDrag;
}

void DragEventActuator::CopyDragEvent(const RefPtr<DragEventActuator>& dragEventActuator)
{
    userCallback_ = dragEventActuator->userCallback_;
    customCallback_ = dragEventActuator->customCallback_;
    panRecognizer_ = MakeRefPtr<PanRecognizer>(fingers_, direction_, distance_);
    panRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    longPressRecognizer_ = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, fingers_, false, false);
    longPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_ =
        AceType::MakeRefPtr<LongPressRecognizer>(PREVIEW_LONG_PRESS_RECONGNIZER, fingers_, false, false);
    previewLongPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    isNotInPreviewState_ = false;
    actionStart_ = dragEventActuator->actionStart_;
    longPressUpdate_ = dragEventActuator->longPressUpdate_;
    actionCancel_ = dragEventActuator->actionCancel_;
    textDragCallback_ = dragEventActuator->textDragCallback_;
    longPressInfo_ = dragEventActuator->longPressInfo_;
}

void DragEventActuator::FlushSyncGeometryNodeTasks()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->FlushSyncGeometryNodeTasks();
}

void DragEventActuator::SetGatherNodeAboveFilter(const RefPtr<DragEventActuator>& actuator)
{
    CHECK_NULL_VOID(actuator);
    if (!actuator->IsNeedGather()) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto gatherNode = manager->GetGatherNode();
    auto gatherNodeChildrenInfo = manager->GetGatherNodeChildrenInfo();
    manager->RemoveGatherNode();
    MountGatherNode(manager, frameNode, gatherNode, gatherNodeChildrenInfo);
}

RefPtr<FrameNode> DragEventActuator::GetOrCreateGatherNode(const RefPtr<NG::OverlayManager>& overlayManager,
    const RefPtr<DragEventActuator>& actuator, std::vector<GatherNodeChildInfo>& gatherNodeChildrenInfo)
{
    CHECK_NULL_RETURN(actuator, nullptr);
    if (!actuator->IsNeedGather()) {
        return nullptr;
    }
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto previewOptions = frameNode->GetDragPreviewOption();
    if (!previewOptions.isMultiSelectionEnabled) {
        return nullptr;
    }
    CHECK_NULL_RETURN(overlayManager, nullptr);
    auto gatherNode = overlayManager->GetGatherNode();
    if (!gatherNode) {
        auto gatherNode = CreateGatherNode(actuator);
        if (gatherNode) {
            MarkDirtyGatherNode(gatherNode);
        }
        gatherNodeChildrenInfo = actuator->GetGatherNodeChildrenInfo();
        actuator->ClearGatherNodeChildrenInfo();
        actuator->SetGatherNode(nullptr);
        return gatherNode;
    } else {
        gatherNodeChildrenInfo = overlayManager->GetGatherNodeChildrenInfo();
        overlayManager->RemoveGatherNode();
    }
    return gatherNode;
}

RefPtr<FrameNode> DragEventActuator::CreateGatherNode(const RefPtr<DragEventActuator>& actuator)
{
    CHECK_NULL_RETURN(actuator, nullptr);
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, nullptr);

    if (!actuator->IsNeedGather()) {
        return nullptr;
    }
    auto fatherNode = actuator->itemFatherNode_.Upgrade();
    CHECK_NULL_RETURN(fatherNode, nullptr);
    auto scrollPattern = fatherNode->GetPattern<ScrollablePattern>();
    CHECK_NULL_RETURN(scrollPattern, nullptr);
    auto children = scrollPattern->GetVisibleSelectedItems();
    if (children.size() <= 1) {
        return nullptr;
    }
    std::reverse(children.begin(), children.end());
    auto stackNode = FrameNode::GetOrCreateFrameNode(V2::STACK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<StackPattern>(); });
    actuator->ClearGatherNodeChildrenInfo();

    for (const auto& itemFrameNode : children) {
        if (itemFrameNode == frameNode) {
            continue;
        }
        GatherNodeChildInfo gatherNodeChildInfo;
        auto imageNode = CreateImageNode(itemFrameNode, gatherNodeChildInfo);
        CHECK_NULL_RETURN(imageNode, nullptr);
        stackNode->AddChild(imageNode);
        actuator->PushBackGatherNodeChild(gatherNodeChildInfo);
    }
    actuator->SetGatherNode(stackNode);
    return stackNode;
}

RefPtr<FrameNode> DragEventActuator::CreateImageNode(const RefPtr<FrameNode>& frameNode,
    GatherNodeChildInfo& gatherNodeChildInfo)
{
    auto context = frameNode->GetRenderContext();
    CHECK_NULL_RETURN(context, nullptr);
    GetFrameNodePreviewPixelMap(frameNode);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    auto pixelMap = gestureHub->GetDragPreviewPixelMap();
    int32_t width = 0;
    int32_t height = 0;
    if (pixelMap) {
        width = pixelMap->GetWidth();
        height = pixelMap->GetHeight();
    }
    auto offset = GetFloatImageOffset(frameNode, pixelMap);
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
    
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    renderProps->UpdateNeedBorderRadius(false);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_RETURN(imageContext, nullptr);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(offset.GetX()), Dimension(offset.GetY())));
    imageContext->UpdateOpacity(1.0);
    imageContext->UpdateTransformScale({ 1.0f, 1.0f });
    Vector5F rotate = Vector5F(0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    imageContext->UpdateTransformRotate(rotate);
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(0.0_vp);
    imageContext->UpdateBorderRadius(borderRadius);
    imageContext->UpdateClipEdge(true);
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);

    gatherNodeChildInfo = {imageNode, offset, width, height, width / 2.0f, height / 2.0f};
    return imageNode;
}

void DragEventActuator::MarkDirtyGatherNode(const RefPtr<FrameNode>& gatherNode)
{
    CHECK_NULL_VOID(gatherNode);
    gatherNode->MarkModifyDone();
    gatherNode->SetLayoutDirtyMarked(true);
    gatherNode->CreateLayoutTask();

    auto children = gatherNode->GetChildren();
    for (const auto& child : children) {
        CHECK_NULL_VOID(child);
        auto imageNode = AceType::DynamicCast<FrameNode>(child);
        CHECK_NULL_VOID(imageNode);
        imageNode->MarkModifyDone();
        imageNode->SetLayoutDirtyMarked(true);
        imageNode->CreateLayoutTask();
    }
}

void DragEventActuator::ResetNode(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    bool defaultAnimationBeforeLifting = frameNode->GetDragPreviewOption().defaultAnimationBeforeLifting;
    if (!defaultAnimationBeforeLifting) {
        return;
    }
    auto frameContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(frameContext);
    frameContext->UpdateTransformScale({ 1.0, 1.0 });
    auto layoutProperty = frameNode->GetLayoutProperty();
    if (layoutProperty) {
        layoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    }
}

void DragEventActuator::MountGatherNode(const RefPtr<OverlayManager>& overlayManager,
    const RefPtr<FrameNode>& frameNode, const RefPtr<FrameNode>& gatherNode,
    std::vector<GatherNodeChildInfo>& gatherNodeChildrenInfo)
{
    CHECK_NULL_VOID(overlayManager);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(gatherNode);

    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto windowScene = overlayManager->FindWindowScene(frameNode);
        overlayManager->MountGatherNodeToWindowScene(gatherNode, gatherNodeChildrenInfo, windowScene);
    } else {
        overlayManager->MountGatherNodeToRootNode(gatherNode, gatherNodeChildrenInfo);
    }
    MarkDirtyGatherNode(gatherNode);
}

void DragEventActuator::GetFrameNodePreviewPixelMap(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto dragPreviewInfo = frameNode->GetDragPreview();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (dragPreviewInfo.inspectorId != "") {
        auto previewPixelMap = GetPreviewPixelMap(dragPreviewInfo.inspectorId, frameNode);
        gestureHub->SetDragPreviewPixelMap(previewPixelMap);
    } else if (dragPreviewInfo.pixelMap != nullptr) {
        gestureHub->SetDragPreviewPixelMap(dragPreviewInfo.pixelMap);
    } else {
        auto context = frameNode->GetRenderContext();
        CHECK_NULL_VOID(context);
        auto pixelMap = context->GetThumbnailPixelMap(true);
        gestureHub->SetDragPreviewPixelMap(pixelMap);
    }
}

bool DragEventActuator::IsBelongToMultiItemNode(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_RETURN(dragDropManager, false);
    if (IsSelectedItemNode(frameNode)) {
        if (dragDropManager->HasGatherNode()) {
            return true;
        }
        dragDropManager->SetHasGatherNode(true);
        isSelectedItemNode_ = true;
        FindItemFatherNode(frameNode);
        return false;
    }
    isSelectedItemNode_ = false;
    auto uiNode = frameNode->GetParent();
    CHECK_NULL_RETURN(uiNode, false);
    while (!IsSelectedItemNode(uiNode)) {
        uiNode = uiNode->GetParent();
        CHECK_NULL_RETURN(uiNode, false);
    }
    return true;
}

bool DragEventActuator::IsSelectedItemNode(const RefPtr<UINode>& uiNode)
{
    auto frameNode = AceType::DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_RETURN(frameNode, false);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, false);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    auto dragPreview = frameNode->GetDragPreviewOption();
    if (!dragPreview.isMultiSelectionEnabled) {
        return false;
    }
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    if (!isAllowedDrag) {
        return false;
    }
    if (frameNode->GetTag() == V2::GRID_ITEM_ETS_TAG) {
        auto itemPattern = frameNode->GetPattern<GridItemPattern>();
        CHECK_NULL_RETURN(itemPattern, false);
        if (itemPattern->IsSelected()) {
            return true;
        }
    }
    if (frameNode->GetTag() == V2::LIST_ITEM_ETS_TAG) {
        auto itemPattern = frameNode->GetPattern<ListItemPattern>();
        CHECK_NULL_RETURN(itemPattern, false);
        if (itemPattern->IsSelected()) {
            return true;
        }
    }
    return false;
}

void DragEventActuator::FindItemFatherNode(const RefPtr<FrameNode>& frameNode)
{
    itemFatherNode_ = nullptr;
    CHECK_NULL_VOID(frameNode);
    if (frameNode->GetTag() != V2::GRID_ITEM_ETS_TAG && frameNode->GetTag() != V2::LIST_ITEM_ETS_TAG) {
        return;
    }
    auto parentType = frameNode->GetTag() == V2::GRID_ITEM_ETS_TAG ? V2::GRID_ETS_TAG : V2::LIST_ETS_TAG;
    auto uiNode = frameNode->GetParent();
    CHECK_NULL_VOID(uiNode);
    while (uiNode->GetTag() != parentType) {
        uiNode = uiNode->GetParent();
        CHECK_NULL_VOID(uiNode);
    }
    auto parentNode = AceType::DynamicCast<FrameNode>(uiNode);
    itemFatherNode_ = parentNode;
}

bool DragEventActuator::IsNeedGather()
{
    auto fatherNode = itemFatherNode_.Upgrade();
    CHECK_NULL_RETURN(fatherNode, false);
    auto scrollPattern = fatherNode->GetPattern<ScrollablePattern>();
    CHECK_NULL_RETURN(scrollPattern, false);
    auto children = scrollPattern->GetVisibleSelectedItems();
    if (!isSelectedItemNode_ || children.size() <= 1) {
        return false;
    }
    return true;
}

void DragEventActuator::HandleTouchUpEvent()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto dragDropManager = pipelineContext->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->SetHasGatherNode(false);
    DragAnimationHelper::PlayNodeResetAnimation(Claim(this));
    auto preDragStatus = dragDropManager->GetPreDragStatus();
    if (preDragStatus >= PreDragStatus::ACTION_DETECTING_STATUS &&
        preDragStatus < PreDragStatus::PREVIEW_LIFT_STARTED) {
        if (IsNeedGather()) {
            SetGatherNode(nullptr);
            ClearGatherNodeChildrenInfo();
            auto manager = pipelineContext->GetOverlayManager();
            CHECK_NULL_VOID(manager);
            manager->RemoveGatherNodeWithAnimation();
        }
    }
}

void DragEventActuator::HandleTouchCancelEvent()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto dragDropManager = pipelineContext->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->SetHasGatherNode(false);
}

void DragEventActuator::HandleTouchMoveEvent()
{
    if (longPressRecognizer_ && isOnBeforeLiftingAnimation &&
        longPressRecognizer_->GetGestureDisposal() == GestureDisposal::REJECT) {
        SetGatherNode(nullptr);
        ClearGatherNodeChildrenInfo();
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto manager = pipelineContext->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        manager->RemoveGatherNode();
        isOnBeforeLiftingAnimation = false;
    }
}

void DragEventActuator::SetGatherNode(const RefPtr<FrameNode>& gatherNode)
{
    gatherNode_ = gatherNode;
}

RefPtr<FrameNode> DragEventActuator::GetGatherNode()
{
    return gatherNode_;
}

std::vector<GatherNodeChildInfo> DragEventActuator::GetGatherNodeChildrenInfo()
{
    return gatherNodeChildrenInfo_;
}

void DragEventActuator::ClearGatherNodeChildrenInfo()
{
    gatherNodeChildrenInfo_.clear();
}

void DragEventActuator::PushBackGatherNodeChild(GatherNodeChildInfo& gatherNodeChild)
{
    gatherNodeChildrenInfo_.push_back(gatherNodeChild);
}

RefPtr<FrameNode> DragEventActuator::GetItemFatherNode()
{
    return itemFatherNode_.Upgrade();
}

RefPtr<FrameNode> DragEventActuator::GetFrameNode()
{
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    auto frameNode = gestureHub->GetFrameNode();
    return frameNode;
}

void DragEventActuator::PrepareShadowParametersForDragData(const RefPtr<FrameNode>& frameNode,
    std::unique_ptr<JsonValue>& arkExtraInfoJson, float scale)
{
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(arkExtraInfoJson);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    auto shadow = dragPreviewOption.options.shadow;
    if (!shadow.has_value() || !shadow->IsValid()) {
        arkExtraInfoJson->Put("shadow_enable", false);
        return;
    }
    auto frameTag = frameNode->GetTag();
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (gestureHub->IsTextCategoryComponent(frameTag) && gestureHub->GetTextDraggable() &&
        gestureHub->GetIsTextDraggable()) {
        if (frameTag != V2::RICH_EDITOR_ETS_TAG) {
            arkExtraInfoJson->Put("shadow_enable", false);
            return;
        }
        auto stringPath = dragPreviewOption.options.shadowPath;
        RSPath path;
        if (path.BuildFromSVGString(stringPath)) {
            RSMatrix matrix;
            matrix.Set(RSMatrix::SCALE_X, scale);
            matrix.Set(RSMatrix::SCALE_Y, scale);
            path.Transform(matrix);
            stringPath = path.ConvertToSVGString();
        }
        arkExtraInfoJson->Put("drag_type", "text");
        arkExtraInfoJson->Put("drag_shadow_path", stringPath.c_str());
    } else {
        arkExtraInfoJson->Put("drag_type", "non-text");
    }
    arkExtraInfoJson->Put("shadow_enable", true);
    ParseShadowInfo(shadow.value(), arkExtraInfoJson);
}

void DragEventActuator::ParseShadowInfo(Shadow& shadow, std::unique_ptr<JsonValue>& arkExtraInfoJson)
{
    arkExtraInfoJson->Put("shadow_is_filled", shadow.GetIsFilled());
    arkExtraInfoJson->Put("drag_shadow_OffsetX", shadow.GetOffset().GetX());
    arkExtraInfoJson->Put("drag_shadow_OffsetY", shadow.GetOffset().GetY());
    arkExtraInfoJson->Put("shadow_mask", shadow.GetShadowType() == ShadowType::BLUR);
    int32_t argb = static_cast<int32_t>(shadow.GetColor().GetValue());
    arkExtraInfoJson->Put("drag_shadow_argb", argb);
    int64_t strategy = static_cast<int64_t>(shadow.GetShadowColorStrategy());
    arkExtraInfoJson->Put("shadow_color_strategy", strategy);
    arkExtraInfoJson->Put("shadow_corner", shadow.GetBlurRadius());
    arkExtraInfoJson->Put("shadow_elevation", shadow.GetElevation());
    arkExtraInfoJson->Put("shadow_is_hardwareacceleration", shadow.GetHardwareAcceleration());
}

std::optional<Shadow> DragEventActuator::GetDefaultShadow()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, std::nullopt);
    auto shadowTheme = pipelineContext->GetTheme<ShadowTheme>();
    CHECK_NULL_RETURN(shadowTheme, std::nullopt);
    auto colorMode = SystemProperties::GetColorMode();
    auto shadow = shadowTheme->GetShadow(ShadowStyle::OuterFloatingSM, colorMode);
    shadow.SetIsFilled(true);
    return shadow;
}

void DragEventActuator::PrepareRadiusParametersForDragData(const RefPtr<FrameNode>& frameNode,
    std::unique_ptr<JsonValue>& arkExtraInfoJson)
{
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(arkExtraInfoJson);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    auto borderRadius = dragPreviewOption.options.borderRadius;
    if (borderRadius.has_value()) {
        if (borderRadius.value().radiusTopLeft.has_value()) {
            arkExtraInfoJson->Put("drag_corner_radius1", borderRadius.value().radiusTopLeft.value().Value());
        }
        if (borderRadius.value().radiusTopRight.has_value()) {
            arkExtraInfoJson->Put("drag_corner_radius2", borderRadius.value().radiusTopRight.value().Value());
        }
        if (borderRadius.value().radiusBottomRight.has_value()) {
            arkExtraInfoJson->Put("drag_corner_radius3", borderRadius.value().radiusBottomRight.value().Value());
        }
        if (borderRadius.value().radiusBottomLeft.has_value()) {
            arkExtraInfoJson->Put("drag_corner_radius4", borderRadius.value().radiusBottomLeft.value().Value());
        }
    }
}

std::optional<BorderRadiusProperty> DragEventActuator::GetDefaultBorderRadius()
{
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(PREVIEW_BORDER_RADIUS);
    return borderRadius;
}

void DragEventActuator::ShowPreviewBadgeAnimation(
    const RefPtr<DragEventActuator>& actuator, const RefPtr<OverlayManager>& manager)
{
    auto gestureEventHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureEventHub);
    auto frameNode = gestureEventHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto dragPreviewOptions = frameNode->GetDragPreviewOption();
    auto badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    auto childSize = badgeNumber.has_value() ? badgeNumber.value()
                                             : static_cast<int32_t>(manager->GetGatherNodeChildrenInfo().size()) + 1;
    auto textNode = CreateBadgeTextNode(frameNode, childSize, PIXELMAP_DRAG_SCALE_MULTIPLE, true);
    CHECK_NULL_VOID(textNode);
    auto column = manager->GetPixelMapNode();
    CHECK_NULL_VOID(column);
    column->AddChild(textNode);

    DragAnimationHelper::ShowBadgeAnimation(textNode);
}

RefPtr<FrameNode> DragEventActuator::CreateBadgeTextNode(
    const RefPtr<FrameNode>& frameNode, int32_t childSize, float previewScale, bool isUsePixelMapOffset)
{
    if (childSize <= 1) {
        return nullptr;
    }
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_RETURN(textNode, nullptr);
    auto badgeLength = std::to_string(childSize).size();
    DragAnimationHelper::UpdateBadgeLayoutAndRenderContext(textNode, badgeLength, childSize);

    auto textRenderContext = textNode->GetRenderContext();
    CHECK_NULL_RETURN(textRenderContext, nullptr);
    auto pixelMap = frameNode->GetPixelMap();
    CHECK_NULL_RETURN(pixelMap, nullptr);
    auto width = pixelMap->GetWidth();
    auto height = pixelMap->GetHeight();
    auto offset = isUsePixelMapOffset ? GetFloatImageOffset(frameNode, pixelMap) : frameNode->GetOffsetInScreen();
    double textOffsetX = offset.GetX() + width * (previewScale + 1) / 2 -
        BADGE_RELATIVE_OFFSET.ConvertToPx() - (BADGE_RELATIVE_OFFSET.ConvertToPx() * badgeLength);
    double textOffsetY = offset.GetY() - height * (previewScale - 1) / 2 - BADGE_RELATIVE_OFFSET.ConvertToPx();
    textRenderContext->UpdatePosition(OffsetT<Dimension>(Dimension(textOffsetX), Dimension(textOffsetY)));
    textNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
    textNode->MarkModifyDone();
    textNode->SetLayoutDirtyMarked(true);
    textNode->SetActive(true);
    textNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
    return textNode;
}

void DragEventActuator::GetThumbnailPixelMapAsync(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(gestureHub);
    auto callback = [id = Container::CurrentId(), gestureHub](const RefPtr<PixelMap>& pixelMap) {
        ContainerScope scope(id);
        if (pixelMap != nullptr) {
            auto taskScheduler = Container::CurrentTaskExecutor();
            CHECK_NULL_VOID(taskScheduler);
            taskScheduler->PostTask(
                [gestureHub, pixelMap]() {
                    CHECK_NULL_VOID(gestureHub);
                    gestureHub->SetPixelMap(pixelMap);
                    TAG_LOGI(AceLogTag::ACE_DRAG, "Set thumbnail pixelMap async success.");
                },
                TaskExecutor::TaskType::UI, "ArkUIDragSetPixelMap");
        }
    };
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (!context->CreateThumbnailPixelMapAsyncTask(true, std::move(callback))) {
        TAG_LOGW(AceLogTag::ACE_DRAG, "Create thumbnail pixelMap async task failed!");
    }
}
} // namespace OHOS::Ace::NG

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

#include "core/components_ng/pattern/overlay/overlay_manager.h"

#include <cstdint>
#include <utility>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/animation/animation_pub.h"
#include "core/animation/spring_curve.h"
#include "core/common/container.h"
#include "core/components/common/properties/color.h"
#include "core/components/select/select_theme.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/bubble/bubble_event_hub.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/overlay/modal_presentation_pattern.h"
#include "core/components_ng/pattern/overlay/sheet_drag_bar_pattern.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_property.h"
#include "core/components_ng/pattern/overlay/sheet_view.h"
#include "core/components_ng/pattern/picker/datepicker_dialog_view.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/text_picker/textpicker_dialog_view.h"
#include "core/components_ng/pattern/time_picker/timepicker_dialog_view.h"
#include "core/components_ng/pattern/toast/toast_view.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline/pipeline_context.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "base/msdp/device_status/interfaces/innerkits/interaction/include/interaction_manager.h"
#endif // ENABLE_DRAG_FRAMEWORK

namespace OHOS::Ace::NG {
namespace {
// should be moved to theme.
constexpr int32_t TOAST_ANIMATION_DURATION = 100;
constexpr int32_t MENU_ANIMATION_DURATION = 150;
constexpr float TOAST_ANIMATION_POSITION = 15.0f;

#ifdef ENABLE_DRAG_FRAMEWORK
constexpr float PIXELMAP_ANIMATION_WIDTH_RATE = 0.5f;
constexpr float PIXELMAP_ANIMATION_HEIGHT_RATE = 0.2f;
constexpr float PIXELMAP_DRAG_SCALE = 1.0f;
constexpr int32_t PIXELMAP_ANIMATION_DURATION = 300;
#endif // ENABLE_DRAG_FRAMEWORK

constexpr int32_t FULL_MODAL_ALPHA_ANIMATION_DURATION = 200;

// dialog animation params
const RefPtr<Curve> SHOW_SCALE_ANIMATION_CURVE = AceType::MakeRefPtr<CubicCurve>(0.38f, 1.33f, 0.6f, 1.0f);

void OnDialogCloseEvent(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    auto dialogPattern = node->GetPattern<DialogPattern>();
    CHECK_NULL_VOID(dialogPattern);
    auto option = dialogPattern->GetCloseAnimation().value_or(AnimationOption());
    auto onFinish = option.GetOnFinishEvent();

    auto dialogLayoutProp = dialogPattern->GetLayoutProperty<DialogLayoutProperty>();
    bool isShowInSubWindow = false;
    if (dialogLayoutProp) {
        isShowInSubWindow = dialogLayoutProp->GetShowInSubWindowValue(false);
    }
    if (onFinish != nullptr) {
        onFinish();
    }

    auto root = node->GetParent();
    CHECK_NULL_VOID(root);
    root->RemoveChild(node);
    root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    auto lastChild = AceType::DynamicCast<FrameNode>(root->GetLastChild());
    if (lastChild) {
        auto pattern = lastChild->GetPattern();
        if (!AceType::InstanceOf<StagePattern>(pattern)) {
            LOGI("root has other overlay children.");
            return;
        }
    }

    auto container = Container::Current();
    CHECK_NULL_VOID_NOLOG(container);
    if (container->IsDialogContainer() || (container->IsSubContainer() && isShowInSubWindow)) {
        SubwindowManager::GetInstance()->HideSubWindowNG();
    }
}
} // namespace

void OverlayManager::OpenDialogAnimation(const RefPtr<FrameNode>& node)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<DialogTheme>();
    CHECK_NULL_VOID(theme);

    auto root = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(root && node);
    node->MountToParent(root);
    root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FocusOverlayNode(node);

    AnimationOption option;
    // default opacity animation params
    option.SetCurve(Curves::SHARP);
    option.SetDuration(theme->GetOpacityAnimationDurIn());
    option.SetFillMode(FillMode::FORWARDS);

    auto dialogPattern = node->GetPattern<DialogPattern>();
    option = dialogPattern->GetOpenAnimation().value_or(option);
    auto onFinish = option.GetOnFinishEvent();

    option.SetOnFinishEvent(
        [weak = WeakClaim(this), nodeWK = WeakClaim(RawPtr(node)), id = Container::CurrentId(), onFinish] {
            ContainerScope scope(id);
            auto context = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(context);
            auto taskExecutor = context->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [id, onFinish]() {
                    ContainerScope scope(id);
                    CHECK_NULL_VOID(onFinish);
                    onFinish();
                },
                TaskExecutor::TaskType::UI);
        });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, theme->GetOpacityStart(), theme->GetOpacityEnd());

    // scale animation on dialog content
    auto contentNode = DynamicCast<FrameNode>(node->GetFirstChild());
    CHECK_NULL_VOID(contentNode);
    ctx = contentNode->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    option.SetOnFinishEvent(nullptr);
    option.SetCurve(SHOW_SCALE_ANIMATION_CURVE);
    option.SetDuration(theme->GetAnimationDurationIn());
    ctx->ScaleAnimation(option, theme->GetScaleStart(), theme->GetScaleEnd());
    node->OnAccessibilityEvent(AccessibilityEventType::CHANGE, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
}

void OverlayManager::CloseDialogAnimation(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<DialogTheme>();
    CHECK_NULL_VOID(theme);
    // blur dialog node, set focus to last page
    BlurOverlayNode();

    // default opacity animation params
    AnimationOption option;
    option.SetFillMode(FillMode::FORWARDS);
    option.SetCurve(Curves::SHARP);

    option.SetDuration(theme->GetAnimationDurationOut());
    // get customized animation params
    auto dialogPattern = node->GetPattern<DialogPattern>();
    option = dialogPattern->GetCloseAnimation().value_or(option);

    option.SetOnFinishEvent([nodeWk = WeakClaim(RawPtr(node)), id = Container::CurrentId()] {
        ContainerScope scope(id);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID_NOLOG(context);
        auto taskExecutor = context->GetTaskExecutor();
        CHECK_NULL_VOID_NOLOG(taskExecutor);
        // animation finish event should be posted to UI thread.
        taskExecutor->PostTask(
            [nodeWk, id]() {
                ContainerScope scope(id);
                LOGI("Execute dialog OnDialogCloseEvent");
                auto node = nodeWk.Upgrade();
                CHECK_NULL_VOID(node);
                OnDialogCloseEvent(node);
            },
            TaskExecutor::TaskType::UI);
    });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, theme->GetOpacityEnd(), theme->GetOpacityStart());

    // scale animation
    auto contentNode = DynamicCast<FrameNode>(node->GetFirstChild());
    CHECK_NULL_VOID(contentNode);
    ctx = contentNode->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    option.SetOnFinishEvent(nullptr);
    option.SetCurve(Curves::FRICTION);
    ctx->ScaleAnimation(option, theme->GetScaleEnd(), theme->GetScaleStart());
    // start animation immediately
    pipeline->RequestFrame();
}

void OverlayManager::ShowMenuAnimation(const RefPtr<FrameNode>& menu, bool isInSubWindow)
{
    AnimationOption option;
    option.SetCurve(Curves::FAST_OUT_SLOW_IN);
    option.SetDuration(MENU_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent(
        [weak = WeakClaim(this), menuWK = WeakClaim(RawPtr(menu)), id = Container::CurrentId(), isInSubWindow] {
            ContainerScope scope(id);
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            taskExecutor->PostTask(
                [weak, menuWK, id, isInSubWindow]() {
                    auto menu = menuWK.Upgrade();
                    auto overlayManager = weak.Upgrade();
                    CHECK_NULL_VOID_NOLOG(menu && overlayManager);
                    ContainerScope scope(id);
                    overlayManager->FocusOverlayNode(menu, isInSubWindow);
                    overlayManager->CallOnShowMenuCallback();
                },
                TaskExecutor::TaskType::UI);
        });

    auto context = menu->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateOpacity(0.0);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto menuAnimationOffset = static_cast<float>(theme->GetMenuAnimationOffset().ConvertToPx());
    context->OnTransformTranslateUpdate({ 0.0f, -menuAnimationOffset, 0.0f });

    AnimationUtils::Animate(
        option,
        [context]() {
            if (context) {
                context->UpdateOpacity(1.0);
                context->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
            }
        },
        option.GetOnFinishEvent());
}

void OverlayManager::PopMenuAnimation(const RefPtr<FrameNode>& menu)
{
    AnimationOption option;
    option.SetCurve(Curves::FAST_OUT_SLOW_IN);
    option.SetDuration(MENU_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent([rootWeak = rootNodeWeak_, menuWK = WeakClaim(RawPtr(menu)), id = Container::CurrentId(),
                                weak = WeakClaim(this)] {
        ContainerScope scope(id);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID_NOLOG(pipeline);
        auto taskExecutor = pipeline->GetTaskExecutor();
        CHECK_NULL_VOID_NOLOG(taskExecutor);
        taskExecutor->PostTask(
            [rootWeak, menuWK, id, weak]() {
                auto menu = menuWK.Upgrade();
                auto root = rootWeak.Upgrade();
                auto overlayManager = weak.Upgrade();
                CHECK_NULL_VOID_NOLOG(menu && root && overlayManager);
                ContainerScope scope(id);
                overlayManager->CallOnHideMenuCallback();
                auto menuWrapperPattern = menu->GetPattern<MenuWrapperPattern>();
                // clear contextMenu then return
                if (menuWrapperPattern && menuWrapperPattern->IsContextMenu()) {
                    SubwindowManager::GetInstance()->ClearMenuNG(id);
                    return;
                }
                root->RemoveChild(menu);
                root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            },
            TaskExecutor::TaskType::UI);
    });

    auto context = menu->GetRenderContext();
    CHECK_NULL_VOID(context);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto menuAnimationOffset = static_cast<float>(theme->GetMenuAnimationOffset().ConvertToPx());
    AnimationUtils::Animate(
        option,
        [context, menuAnimationOffset]() {
            context->UpdateOpacity(0.0);
            context->OnTransformTranslateUpdate({ 0.0f, -menuAnimationOffset, 0.0f });
        },
        option.GetOnFinishEvent());

    // start animation immediately
    pipeline->RequestFrame();
}

void OverlayManager::ShowToast(
    const std::string& message, int32_t duration, const std::string& bottom, bool isRightToLeft)
{
    LOGI("OverlayManager::ShowToast");
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    // only one toast
    for (auto [id, toastNodeWeak] : toastMap_) {
        rootNode->RemoveChild(toastNodeWeak.Upgrade());
    }
    toastMap_.clear();

    auto toastNode = ToastView::CreateToastNode(message, bottom, isRightToLeft);
    CHECK_NULL_VOID(toastNode);
    auto toastId = toastNode->GetId();
    // mount to parent
    toastNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    toastMap_[toastId] = toastNode;
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f);
    option.SetCurve(curve);
    option.SetDuration(TOAST_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    auto&& callback = [weak = WeakClaim(this), toastId, duration, id = Container::CurrentId()]() {
        auto overlayManager = weak.Upgrade();
        CHECK_NULL_VOID(overlayManager);
        ContainerScope scope(id);
        overlayManager->PopToast(toastId);
    };
    continuousTask_.Reset(callback);
    option.SetOnFinishEvent([continuousTask = continuousTask_, duration, id = Container::CurrentId()] {
        ContainerScope scope(id);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID_NOLOG(context);
        context->GetTaskExecutor()->PostDelayedTask(continuousTask, TaskExecutor::TaskType::UI, duration);
    });
    auto ctx = toastNode->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->UpdateOpacity(0.0);
    ctx->OnTransformTranslateUpdate({ 0.0f, TOAST_ANIMATION_POSITION, 0.0f });
    AnimationUtils::Animate(
        option,
        [ctx]() {
            if (ctx) {
                ctx->UpdateOpacity(1.0);
                ctx->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
            }
        },
        option.GetOnFinishEvent());
    toastNode->OnAccessibilityEvent(
        AccessibilityEventType::CHANGE, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
}

void OverlayManager::PopToast(int32_t toastId)
{
    LOGI("OverlayManager::PopToast");
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f);
    option.SetCurve(curve);
    option.SetDuration(TOAST_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    // OnFinishEvent should be executed in UI thread.
    option.SetOnFinishEvent([weak = WeakClaim(this), toastId, id = Container::CurrentId()] {
        ContainerScope scope(id);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID_NOLOG(context);
        context->GetTaskExecutor()->PostTask(
            [weak, toastId, id]() {
                ContainerScope scope(id);
                auto overlayManager = weak.Upgrade();
                CHECK_NULL_VOID_NOLOG(overlayManager);
                auto toastIter = overlayManager->toastMap_.find(toastId);
                if (toastIter == overlayManager->toastMap_.end()) {
                    LOGI("No toast under pop");
                    return;
                }
                auto toastUnderPop = toastIter->second.Upgrade();
                CHECK_NULL_VOID_NOLOG(toastUnderPop);
                LOGI("begin to pop toast, id is %{public}d", toastUnderPop->GetId());
                auto context = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID_NOLOG(context);
                auto rootNode = context->GetRootElement();
                CHECK_NULL_VOID_NOLOG(rootNode);
                rootNode->RemoveChild(toastUnderPop);
                overlayManager->toastMap_.erase(toastId);
                rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);

                auto container = Container::Current();
                CHECK_NULL_VOID_NOLOG(container);
                if (container->IsDialogContainer() ||
                    (container->IsSubContainer() && rootNode->GetChildren().empty())) {
                    // hide window when toast show in subwindow.
                    SubwindowManager::GetInstance()->HideSubWindowNG();
                }
            },
            TaskExecutor::TaskType::UI);
    });
    auto toastIter = toastMap_.find(toastId);
    if (toastIter == toastMap_.end()) {
        LOGI("No toast under pop");
        return;
    }
    auto toastUnderPop = toastIter->second.Upgrade();
    CHECK_NULL_VOID_NOLOG(toastUnderPop);
    auto ctx = toastUnderPop->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->UpdateOpacity(1.0);
    ctx->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
    AnimationUtils::Animate(
        option,
        [ctx]() {
            if (ctx) {
                ctx->UpdateOpacity(0.0);
                ctx->OnTransformTranslateUpdate({ 0.0f, TOAST_ANIMATION_POSITION, 0.0f });
            }
        },
        option.GetOnFinishEvent());
    // start animation immediately
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
    AccessibilityEvent event;
    event.type = AccessibilityEventType::CHANGE;
    event.windowContentChangeTypes = WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE;
    pipeline->SendEventToAccessibility(event);
}

void OverlayManager::AdaptToSafeArea(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID_NOLOG(node);
    auto pipeline = PipelineBase::GetCurrentContext();
    const static int32_t PLATFORM_VERSION_TEN = 10;
    auto layoutProperty = node->GetLayoutProperty();
    if (pipeline->GetMinPlatformVersion() >= PLATFORM_VERSION_TEN && pipeline->GetIsAppWindow() &&
        pipeline->GetIsLayoutFullScreen() && !pipeline->GetIgnoreViewSafeArea() && layoutProperty) {
        NG::MarginProperty margins;
        SafeAreaEdgeInserts safeArea = pipeline->GetCurrentViewSafeArea();
        margins.top = NG::CalcLength(safeArea.topRect_.Height());
        margins.bottom = NG::CalcLength(safeArea.bottomRect_.Height());
        margins.left = NG::CalcLength(safeArea.leftRect_.Width());
        margins.right = NG::CalcLength(safeArea.rightRect_.Width());
        layoutProperty->UpdateMargin(margins);
    }
}

void OverlayManager::UpdatePopupNode(int32_t targetId, const PopupInfo& popupInfo)
{
    popupMap_[targetId] = popupInfo;
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    CHECK_NULL_VOID_NOLOG(popupInfo.markNeedUpdate);
    CHECK_NULL_VOID_NOLOG(popupInfo.popupNode);

    popupMap_[targetId].markNeedUpdate = false;
    auto rootChildren = rootNode->GetChildren();
    auto iter = std::find(rootChildren.begin(), rootChildren.end(), popupInfo.popupNode);
    if (iter != rootChildren.end()) {
        // Pop popup
        CHECK_NULL_VOID_NOLOG(popupInfo.isCurrentOnShow);
        LOGI("OverlayManager: Popup begin pop");
        popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
        rootNode->RemoveChild(popupMap_[targetId].popupNode);
        AccessibilityEvent event;
        event.type = AccessibilityEventType::CHANGE;
        event.windowContentChangeTypes = WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE;
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->SendEventToAccessibility(event);
#ifdef ENABLE_DRAG_FRAMEWORK
        RemoveEventColumn();
        RemovePixelMapAnimation(false, 0, 0);
        RemoveFilter();
#endif // ENABLE_DRAG_FRAMEWORK
    } else {
        // Push popup
        CHECK_NULL_VOID_NOLOG(!popupInfo.isCurrentOnShow);
        LOGI("OverlayManager: Popup begin push");
        popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(true);
        auto hub = popupInfo.popupNode->GetEventHub<BubbleEventHub>();
        if (!popupInfo.isBlockEvent && hub) {
            auto ges = hub->GetOrCreateGestureEventHub();
            if (ges) {
                ges->SetHitTestMode(HitTestMode::HTMTRANSPARENT);
            }
        }
        popupMap_[targetId].popupNode->MountToParent(rootNode);
    }
    popupMap_[targetId].isCurrentOnShow = !popupInfo.isCurrentOnShow;
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void OverlayManager::ShowIndexerPopup(int32_t targetId, RefPtr<FrameNode>& customNode)
{
    CHECK_NULL_VOID(customNode);
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    if (!customPopupMap_[targetId] || customPopupMap_[targetId] != customNode) {
        customPopupMap_[targetId] = customNode;
        customNode->MountToParent(rootNode);
        customNode->MarkModifyDone();
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
}

void OverlayManager::RemoveIndexerPopup()
{
    if (customPopupMap_.empty()) {
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    for (const auto& popup : customPopupMap_) {
        auto popupNode = popup.second;
        rootNode->RemoveChild(popupNode);
    }
    customPopupMap_.clear();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
}

void OverlayManager::HidePopup(int32_t targetId, const PopupInfo& popupInfo)
{
    LOGI("OverlayManager:: Hide Popup");
    popupMap_[targetId] = popupInfo;
    CHECK_NULL_VOID_NOLOG(popupInfo.markNeedUpdate);
    popupMap_[targetId].markNeedUpdate = false;
    CHECK_NULL_VOID_NOLOG(popupInfo.popupNode);
    popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
    CHECK_NULL_VOID_NOLOG(popupInfo.isCurrentOnShow);
    popupMap_[targetId].isCurrentOnShow = !popupInfo.isCurrentOnShow;

    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    auto rootChildren = rootNode->GetChildren();
    auto iter = std::find(rootChildren.begin(), rootChildren.end(), popupInfo.popupNode);
    if (iter != rootChildren.end()) {
        rootNode->RemoveChild(popupMap_[targetId].popupNode);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
}

void OverlayManager::HideAllPopups()
{
    LOGD("OverlayManager::HideAllPopups");
    if (popupMap_.empty()) {
        LOGD("OverlayManager: popupMap is empty");
        return;
    }
    for (const auto& popup : popupMap_) {
        auto popupInfo = popup.second;
        if (popupInfo.isCurrentOnShow && popupInfo.target.Upgrade()) {
            popupInfo.markNeedUpdate = true;
            popupInfo.popupId = -1;
            auto targetNodeId = popupInfo.target.Upgrade()->GetId();
            auto popupNode = popupInfo.popupNode;
            CHECK_NULL_VOID(popupNode);
            auto layoutProp = popupNode->GetLayoutProperty<BubbleLayoutProperty>();
            CHECK_NULL_VOID(layoutProp);
            auto showInSubWindow = layoutProp->GetShowInSubWindow().value_or(false);
            if (showInSubWindow) {
                SubwindowManager::GetInstance()->HidePopupNG(targetNodeId);
            } else {
                UpdatePopupNode(targetNodeId, popupInfo);
            }
        }
    }
}

void OverlayManager::ErasePopup(int32_t targetId)
{
    if (popupMap_.find(targetId) != popupMap_.end()) {
        LOGI("Erase popup id %{public}d when destroyed.", targetId);
        auto rootNode = rootNodeWeak_.Upgrade();
        CHECK_NULL_VOID(rootNode);
        rootNode->RemoveChild(popupMap_[targetId].popupNode);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        popupMap_.erase(targetId);
    }
}

bool OverlayManager::ShowMenuHelper(RefPtr<FrameNode>& menu, int32_t targetId, const NG::OffsetF& offset)
{
    if (!menu) {
        // get existing menuNode
        auto it = menuMap_.find(targetId);
        if (it != menuMap_.end()) {
            menu = it->second;
        } else {
            LOGW("menuNode doesn't exists %{public}d", targetId);
        }
    } else {
        // creating new menu
        menuMap_[targetId] = menu;
        LOGI("menuNode %{public}d added to map", targetId);
    }
    CHECK_NULL_RETURN(menu, false);

    RefPtr<FrameNode> menuFrameNode = menu;
    if (menu->GetTag() != V2::MENU_ETS_TAG) {
        auto menuChild = menu->GetChildAtIndex(0);
        CHECK_NULL_RETURN(menuChild, false);
        menuFrameNode = DynamicCast<FrameNode>(menuChild);
    }

    auto props = menuFrameNode->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(props, false);
    props->UpdateMenuOffset(offset);
    menuFrameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    return true;
}

void OverlayManager::ShowMenu(int32_t targetId, const NG::OffsetF& offset, RefPtr<FrameNode> menu)
{
    if (!ShowMenuHelper(menu, targetId, offset)) {
        LOGW("show menu failed");
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    auto rootChildren = rootNode->GetChildren();
    auto iter = std::find(rootChildren.begin(), rootChildren.end(), menu);
    // menuNode already showing
    if (iter != rootChildren.end()) {
        LOGW("menuNode already appended");
    } else {
        menu->MountToParent(rootNode);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        menu->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        ShowMenuAnimation(menu);
        menu->MarkModifyDone();
        LOGI("menuNode mounted");
    }
    menu->OnAccessibilityEvent(AccessibilityEventType::CHANGE, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
}

// subwindow only contains one menu instance.
void OverlayManager::ShowMenuInSubWindow(int32_t targetId, const NG::OffsetF& offset, RefPtr<FrameNode> menu)
{
    auto menuOffset = offset;
    auto currentSubwindow = SubwindowManager::GetInstance()->GetCurrentWindow();
    if (currentSubwindow) {
        auto subwindowRect = currentSubwindow->GetRect();
        menuOffset -= subwindowRect.GetOffset();
    }
    if (!ShowMenuHelper(menu, targetId, menuOffset)) {
        LOGW("show menu failed");
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    rootNode->Clean();
    menu->MountToParent(rootNode);
    ShowMenuAnimation(menu, true);
    menu->MarkModifyDone();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    LOGI("menuNode mounted in subwindow");
}

void OverlayManager::HideMenuInSubWindow(int32_t targetId)
{
    LOGI("OverlayManager::HideMenuInSubWindow");
    if (menuMap_.find(targetId) == menuMap_.end()) {
        LOGW("OverlayManager: menuNode %{public}d not found in map", targetId);
        return;
    }
    auto node = menuMap_[targetId];
    CHECK_NULL_VOID(node);
    PopMenuAnimation(node);
    BlurOverlayNode(true);
}

void OverlayManager::HideMenuInSubWindow()
{
    LOGI("OverlayManager::HideMenuInSubWindow from close");
    if (menuMap_.empty()) {
        LOGW("OverlayManager: menuMap is empty");
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    for (const auto& child : rootNode->GetChildren()) {
        auto node = DynamicCast<FrameNode>(child);
        PopMenuAnimation(node);
    }
}

void OverlayManager::HideMenu(int32_t targetId)
{
    LOGI("OverlayManager::HideMenuNode menu targetId is %{public}d", targetId);
    if (menuMap_.find(targetId) == menuMap_.end()) {
        LOGW("OverlayManager: menuNode %{public}d not found in map", targetId);
        return;
    }
    PopMenuAnimation(menuMap_[targetId]);
    BlurOverlayNode();
    menuMap_[targetId]->OnAccessibilityEvent(
        AccessibilityEventType::CHANGE, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
}

void OverlayManager::HideAllMenus()
{
    LOGD("OverlayManager::HideAllMenus");
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    for (const auto& child : rootNode->GetChildren()) {
        auto node = DynamicCast<FrameNode>(child);
        if (node && node->GetTag() == V2::MENU_WRAPPER_ETS_TAG) {
            PopMenuAnimation(node);
            BlurOverlayNode();
        }
    }
}

void OverlayManager::DeleteMenu(int32_t targetId)
{
    LOGI("OverlayManager::DeleteMenuNode");
    auto it = menuMap_.find(targetId);
    if (it == menuMap_.end()) {
        LOGW("OverlayManager: menuNode %{public}d doesn't exist", targetId);
        return;
    }
    menuMap_.erase(it);
}

void OverlayManager::CleanMenuInSubWindow()
{
    LOGI("OverlayManager::CleanMenuInSubWindow");
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    rootNode->Clean();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

RefPtr<FrameNode> OverlayManager::ShowDialog(
    const DialogProperties& dialogProps, const RefPtr<UINode>& customNode, bool isRightToLeft)
{
    LOGI("OverlayManager::ShowDialog");
    auto dialog = DialogView::CreateDialogNode(dialogProps, customNode);
    AdaptToSafeArea(dialog);
    OpenDialogAnimation(dialog);
    return dialog;
}

void OverlayManager::ShowCustomDialog(const RefPtr<FrameNode>& customNode)
{
    LOGI("OverlayManager::ShowCustomDialog");
    AdaptToSafeArea(customNode);
    OpenDialogAnimation(customNode);
}

void OverlayManager::ShowDateDialog(const DialogProperties& dialogProps, const DatePickerSettingData& settingData,
    std::map<std::string, NG::DialogEvent> dialogEvent, std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    LOGI("OverlayManager::ShowDateDialogPicker");
    auto dialogNode = DatePickerDialogView::Show(
        dialogProps, std::move(settingData), std::move(dialogEvent), std::move(dialogCancelEvent));
    AdaptToSafeArea(dialogNode);
    OpenDialogAnimation(dialogNode);
}

void OverlayManager::ShowTimeDialog(const DialogProperties& dialogProps, const TimePickerSettingData& settingData,
    std::map<std::string, PickerTime> timePickerProperty, std::map<std::string, NG::DialogEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    LOGI("OverlayManager::ShowTimeDialogPicker");
    auto dialogNode = TimePickerDialogView::Show(
        dialogProps, settingData, std::move(timePickerProperty), std::move(dialogEvent), std::move(dialogCancelEvent));
    AdaptToSafeArea(dialogNode);
    OpenDialogAnimation(dialogNode);
}

void OverlayManager::ShowTextDialog(const DialogProperties& dialogProps, const TextPickerSettingData& settingData,
    std::map<std::string, NG::DialogTextEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    LOGI("OverlayManager::ShowTextDialogPicker");
    auto dialogNode =
        TextPickerDialogView::Show(dialogProps, settingData, std::move(dialogEvent), std::move(dialogCancelEvent));
    AdaptToSafeArea(dialogNode);
    OpenDialogAnimation(dialogNode);
}

void OverlayManager::CloseDialog(const RefPtr<FrameNode>& dialogNode)
{
    LOGI("OverlayManager::CloseDialog");
    if (dialogNode->IsRemoving()) {
        // already in close animation
        return;
    }
    dialogNode->MarkRemoving();
    CloseDialogAnimation(dialogNode);
    dialogNode->OnAccessibilityEvent(
        AccessibilityEventType::CHANGE, WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
}

bool OverlayManager::RemoveOverlay()
{
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_RETURN(rootNode, true);
    RemoveIndexerPopup();
    auto childrenSize = rootNode->GetChildren().size();
    if (rootNode->GetChildren().size() > 1) {
        // stage node is at index 0, remove overlay at last
        auto overlay = DynamicCast<FrameNode>(rootNode->GetLastChild());
        CHECK_NULL_RETURN(overlay, false);
        // close dialog with animation
        auto pattern = overlay->GetPattern();
        if (AceType::DynamicCast<DialogPattern>(pattern)) {
            if (FireBackPressEvent()) {
                return true;
            }
            auto hub = overlay->GetEventHub<DialogEventHub>();
            if (hub) {
                hub->FireCancelEvent();
            }
            CloseDialog(overlay);
            SetBackPressEvent(nullptr);
            return true;
        } else if (AceType::DynamicCast<BubblePattern>(pattern)) {
            auto popupNode = AceType::DynamicCast<NG::FrameNode>(rootNode->GetChildAtIndex(childrenSize - 1));
            popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
            for (const auto& popup : popupMap_) {
                auto targetId = popup.first;
                auto popupInfo = popup.second;
                if (popupNode == popupInfo.popupNode) {
                    popupMap_.erase(targetId);
                    rootNode->RemoveChild(popupNode);
                    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
                    return true;
                }
            }
            return false;
        }
        if (!modalStack_.empty()) {
            return RemoveModalInOverlay();
        }
        rootNode->RemoveChild(overlay);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        LOGI("overlay removed successfully");
        return true;
    }
    LOGI("No overlay in this page.");
    return false;
}

bool OverlayManager::RemoveModalInOverlay()
{
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_RETURN(rootNode, true);
    auto topModalNode = modalStack_.top().Upgrade();
    CHECK_NULL_RETURN(topModalNode, false);
    if (topModalNode->GetTag() == "ModalPage") {
        topModalNode->GetPattern<ModalPresentationPattern>()->FireCallback("false");
        auto builder = AceType::DynamicCast<FrameNode>(topModalNode->GetFirstChild());
        CHECK_NULL_RETURN(topModalNode, false);
        auto modalTransition = topModalNode->GetPattern<ModalPresentationPattern>()->GetType();
        if (builder->GetRenderContext()->HasTransition()) {
            topModalNode->Clean();
            topModalNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
        if (modalTransition == ModalTransition::DEFAULT) {
            PlayDefaultModalTransition(topModalNode, false);
        } else if (modalTransition == ModalTransition::ALPHA) {
            PlayAlphaModalTransition(topModalNode, false);
        } else {
            rootNode->RemoveChild(topModalNode);
            rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
    } else if (topModalNode->GetTag() == "SheetPage") {
        topModalNode->GetPattern<SheetPresentationPattern>()->FireCallback("false");
        auto builder = AceType::DynamicCast<FrameNode>(topModalNode->GetLastChild());
        CHECK_NULL_RETURN(topModalNode, false);
        if (builder->GetRenderContext()->HasTransition()) {
            topModalNode->Clean();
            topModalNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
        PlaySheetTransition(topModalNode, false);
    }
    modalStack_.pop();
    SaveLastModalNode();
    return true;
}

bool OverlayManager::RemoveOverlayInSubwindow()
{
    LOGI("OverlayManager::RemoveOverlayInSubwindow");
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_RETURN(rootNode, false);
    if (rootNode->GetChildren().empty()) {
        LOGI("No overlay in this subwindow.");
        return false;
    }

    // remove the overlay node just mounted in subwindow
    auto overlay = DynamicCast<FrameNode>(rootNode->GetLastChild());
    CHECK_NULL_RETURN(overlay, false);
    // close dialog with animation
    auto pattern = overlay->GetPattern();
    if (AceType::InstanceOf<DialogPattern>(pattern)) {
        auto hub = overlay->GetEventHub<DialogEventHub>();
        if (hub) {
            hub->FireCancelEvent();
        }
        CloseDialog(overlay);
        return true;
    }
    if (AceType::InstanceOf<BubblePattern>(pattern)) {
        overlay->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
        for (const auto& popup : popupMap_) {
            auto targetId = popup.first;
            auto popupInfo = popup.second;
            if (overlay == popupInfo.popupNode) {
                popupMap_.erase(targetId);
                rootNode->RemoveChild(overlay);
                rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
                if (rootNode->GetChildren().empty()) {
                    SubwindowManager::GetInstance()->HideSubWindowNG();
                }
                return true;
            }
        }
        return false;
    }
    if (AceType::InstanceOf<MenuWrapperPattern>(pattern)) {
        HideMenuInSubWindow();
        return true;
    }
    rootNode->RemoveChild(overlay);
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    if (rootNode->GetChildren().empty()) {
        SubwindowManager::GetInstance()->HideSubWindowNG();
    }
    LOGI("overlay removed successfully");
    return true;
}

void OverlayManager::FocusOverlayNode(const RefPtr<FrameNode>& overlayNode, bool isInSubWindow)
{
    LOGI("OverlayManager::FocusOverlayNode when overlay node show");
    CHECK_NULL_VOID(overlayNode);
    auto focusHub = overlayNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocus();

    if (isInSubWindow) {
        // no need to set page lost focus in sub window.
        return;
    }

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pageFocusHub = pageNode->GetFocusHub();
    CHECK_NULL_VOID(pageFocusHub);
    pageFocusHub->SetParentFocusable(false);
    pageFocusHub->LostFocus();
}

void OverlayManager::BlurOverlayNode(bool isInSubWindow)
{
    LOGI("OverlayManager::BlurOverlayNode");
    if (isInSubWindow) {
        // no need to set page request focus in sub window.
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pageFocusHub = pageNode->GetFocusHub();
    CHECK_NULL_VOID(pageFocusHub);
    pageFocusHub->SetParentFocusable(true);
    pageFocusHub->RequestFocus();
}

void OverlayManager::SaveLastModalNode()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto stageManager = pipeline->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    if (modalStack_.empty()) {
        lastModalNode_ = WeakClaim(RawPtr(pageNode));
    } else {
        auto topModalNode = modalStack_.top().Upgrade();
        modalStack_.pop();
        if (modalStack_.empty()) {
            lastModalNode_ = WeakClaim(RawPtr(pageNode));
        } else {
            lastModalNode_ = modalStack_.top();
        }
        modalStack_.push(topModalNode);
    }
}

void OverlayManager::BindContentCover(bool isShow, std::function<void(const std::string&)>&& callback,
    std::function<RefPtr<UINode>()>&& buildNodeFunc, int32_t type, int32_t targetId)
{
    LOGI("BindContentCover isShow: %{public}d, type: %{public}d, targetId: %{public}d", isShow, type, targetId);
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    auto modalTransition = static_cast<ModalTransition>(type);
    if (isShow) {
        if (!modalStack_.empty()) {
            auto topModalNode = modalStack_.top().Upgrade();
            CHECK_NULL_VOID(topModalNode);
            if (topModalNode->GetTag() == "ModalPage") {
                if (topModalNode->GetPattern<ModalPresentationPattern>()->GetTargetId() == targetId) {
                    topModalNode->GetPattern<ModalPresentationPattern>()->SetType(static_cast<ModalTransition>(type));
                    return;
                }
            }
        }
        // builder content
        auto builder = AceType::DynamicCast<FrameNode>(buildNodeFunc());
        CHECK_NULL_VOID(builder);
        builder->GetRenderContext()->SetIsModalRootNode(true);

        // create modal page
        auto modalNode = FrameNode::CreateFrameNode("ModalPage", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<ModalPresentationPattern>(
                targetId, static_cast<ModalTransition>(type), std::move(callback)));
        modalStack_.push(WeakClaim(RawPtr(modalNode)));
        SaveLastModalNode();
        modalNode->MountToParent(rootNode);
        modalNode->AddChild(builder);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        if (modalTransition == ModalTransition::DEFAULT) {
            PlayDefaultModalTransition(modalNode, true);
        } else if (modalTransition == ModalTransition::ALPHA) {
            PlayAlphaModalTransition(modalNode, true);
        }
        return;
    }
    if (!modalStack_.empty()) {
        auto topModalNode = modalStack_.top().Upgrade();
        CHECK_NULL_VOID(topModalNode);
        if (topModalNode->GetTag() != "ModalPage") {
            return;
        }
        if (topModalNode->GetPattern<ModalPresentationPattern>()->GetTargetId() != targetId) {
            LOGW("current modal is not existed.");
            return;
        }
        auto builder = AceType::DynamicCast<FrameNode>(topModalNode->GetFirstChild());
        CHECK_NULL_VOID(builder);
        if (builder->GetRenderContext()->HasTransition()) {
            topModalNode->Clean(false, true);
            topModalNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        auto modalPresentationPattern = topModalNode->GetPattern<ModalPresentationPattern>();
        CHECK_NULL_VOID(modalPresentationPattern);
        modalTransition = modalPresentationPattern->GetType();
        if (modalTransition == ModalTransition::DEFAULT) {
            PlayDefaultModalTransition(topModalNode, false);
        } else if (modalTransition == ModalTransition::ALPHA) {
            PlayAlphaModalTransition(topModalNode, false);
        } else if (!builder->GetRenderContext()->HasTransition()) {
            rootNode->RemoveChild(topModalNode);
            rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        modalStack_.pop();
        SaveLastModalNode();
    }
}

void OverlayManager::PlayDefaultModalTransition(const RefPtr<FrameNode>& modalNode, bool isTransitionIn)
{
    // current modal animation
    AnimationOption option;
    const RefPtr<InterpolatingSpring> curve = AceType::MakeRefPtr<InterpolatingSpring>(0.0f, 1.0f, 100.0f, 20.0f);
    option.SetCurve(curve);
    option.SetFillMode(FillMode::FORWARDS);
    auto context = modalNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pageNode = pipeline->GetStageManager()->GetLastPage();
    auto pageHeight = pageNode->GetGeometryNode()->GetFrameSize().Height();
    if (isTransitionIn) {
        context->OnTransformTranslateUpdate({ 0.0f, pageHeight, 0.0f });
        AnimationUtils::Animate(option, [context]() {
            if (context) {
                context->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
            }
        });
    } else {
        auto lastModalNode = lastModalNode_.Upgrade();
        CHECK_NULL_VOID(lastModalNode);
        auto lastModalContext = lastModalNode->GetRenderContext();
        CHECK_NULL_VOID(lastModalContext);
        lastModalContext->UpdateOpacity(1.0);
        option.SetOnFinishEvent(
            [rootWeak = rootNodeWeak_, modalWK = WeakClaim(RawPtr(modalNode)), id = Container::CurrentId()] {
                ContainerScope scope(id);
                auto context = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID_NOLOG(context);
                auto taskExecutor = context->GetTaskExecutor();
                CHECK_NULL_VOID_NOLOG(taskExecutor);
                // animation finish event should be posted to UI thread.
                taskExecutor->PostTask([rootWeak, modalWK, id]() {
                    auto modal = modalWK.Upgrade();
                    auto root = rootWeak.Upgrade();
                    CHECK_NULL_VOID_NOLOG(modal && root);
                    ContainerScope scope(id);
                    root->RemoveChild(modal);
                    root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
                    }, TaskExecutor::TaskType::UI);
            });
        context->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
        AnimationUtils::Animate(
            option,
            [context, pageHeight]() {
                if (context) {
                    context->OnTransformTranslateUpdate({ 0.0f, pageHeight, 0.0f });
                }
            },
            option.GetOnFinishEvent());
    }
}

void OverlayManager::PlayAlphaModalTransition(const RefPtr<FrameNode>& modalNode, bool isTransitionIn)
{
    AnimationOption option;
    option.SetCurve(Curves::FRICTION);
    option.SetDuration(FULL_MODAL_ALPHA_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    auto lastModalNode = lastModalNode_.Upgrade();
    CHECK_NULL_VOID(lastModalNode);
    auto lastModalContext = lastModalNode->GetRenderContext();
    CHECK_NULL_VOID(lastModalContext);
    auto context = modalNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (isTransitionIn) {
        // last page animation
        lastModalContext->OpacityAnimation(option, 1, 0);
        lastModalContext->UpdateOpacity(0);

        // current modal page animation
        context->OpacityAnimation(option, 0, 1);
    } else {
        // last page animation
        lastModalContext->OpacityAnimation(option, 0, 1);

        // current modal page animation
        option.SetOnFinishEvent(
            [rootWeak = rootNodeWeak_, modalWK = WeakClaim(RawPtr(modalNode)), id = Container::CurrentId()] {
                ContainerScope scope(id);
                auto context = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID_NOLOG(context);
                auto taskExecutor = context->GetTaskExecutor();
                CHECK_NULL_VOID_NOLOG(taskExecutor);
                // animation finish event should be posted to UI thread.
                taskExecutor->PostTask([rootWeak, modalWK, id]() {
                    auto modal = modalWK.Upgrade();
                    auto root = rootWeak.Upgrade();
                    CHECK_NULL_VOID_NOLOG(modal && root);
                    ContainerScope scope(id);
                    root->RemoveChild(modal);
                    root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
                    }, TaskExecutor::TaskType::UI);
            });
        context->OpacityAnimation(option, 1, 0);
    }
}

void OverlayManager::BindSheet(bool isShow, std::function<void(const std::string&)>&& callback,
    std::function<RefPtr<UINode>()>&& buildNodeFunc, NG::SheetStyle& sheetStyle, int32_t targetId)
{
    LOGI("BindSheet isShow: %{public}d, targetId: %{public}d", isShow, targetId);
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    if (isShow) {
        if (!modalStack_.empty()) {
            auto topModalNode = modalStack_.top().Upgrade();
            CHECK_NULL_VOID(topModalNode);
            if (topModalNode->GetTag() == "SheetPage") {
                if (topModalNode->GetPattern<SheetPresentationPattern>()->GetTargetId() == targetId) {
                    ComputeSheetOffset(sheetStyle);
                    auto layoutProperty = topModalNode->GetLayoutProperty<SheetPresentationProperty>();
                    layoutProperty->UpdateSheetStyle(sheetStyle);
                    topModalNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
                    PlaySheetTransition(topModalNode, true, false);
                    return;
                }
            }
        }
        // builder content
        auto builder = AceType::DynamicCast<FrameNode>(buildNodeFunc());
        CHECK_NULL_VOID(builder);
        builder->GetRenderContext()->SetIsModalRootNode(true);
        // create modal page
        auto sheetNode = SheetView::CreateSheetPage(targetId, builder, std::move(callback), sheetStyle);
        ComputeSheetOffset(sheetStyle);
        modalStack_.push(WeakClaim(RawPtr(sheetNode)));
        SaveLastModalNode();
        sheetNode->MountToParent(rootNode);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);

        // start transition animation
        PlaySheetTransition(sheetNode, true);
        return;
    }

    if (!modalStack_.empty()) {
        auto topSheetNode = modalStack_.top().Upgrade();
        CHECK_NULL_VOID(topSheetNode);
        if (topSheetNode->GetTag() != "SheetPage") {
            return;
        }
        if (topSheetNode->GetPattern<SheetPresentationPattern>()->GetTargetId() != targetId) {
            return;
        }
        auto builder = AceType::DynamicCast<FrameNode>(topSheetNode->GetLastChild());
        CHECK_NULL_VOID(builder);
        if (builder->GetRenderContext()->HasTransition()) {
            topSheetNode->Clean();
            topSheetNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        PlaySheetTransition(topSheetNode, false);
        modalStack_.pop();
        SaveLastModalNode();
    }
}

void OverlayManager::PlaySheetTransition(RefPtr<FrameNode> sheetNode, bool isTransitionIn, bool isFirstTransition)
{
    // current sheet animation
    AnimationOption option;
    const RefPtr<InterpolatingSpring> curve = AceType::MakeRefPtr<InterpolatingSpring>(0.0f, 1.0f, 100.0f, 20.0f);
    option.SetCurve(curve);
    option.SetFillMode(FillMode::FORWARDS);
    auto context = sheetNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pageNode = pipeline->GetStageManager()->GetLastPage();
    auto pageHeight = pageNode->GetGeometryNode()->GetFrameSize().Height();
    if (isTransitionIn) {
        auto offset = pageHeight - sheetHeight_;
        if (isFirstTransition) {
            context->OnTransformTranslateUpdate({ 0.0f, pageHeight, 0.0f });
        }
        AnimationUtils::Animate(
            option,
            [context, offset]() {
                if (context) {
                    context->OnTransformTranslateUpdate({ 0.0f, offset, 0.0f });
                }
            });
    } else {
        option.SetOnFinishEvent([rootWeak = rootNodeWeak_, sheetWK = WeakClaim(RawPtr(sheetNode)),
            id = Container::CurrentId()] {
            ContainerScope scope(id);
            auto context = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(context);
            auto taskExecutor = context->GetTaskExecutor();
            CHECK_NULL_VOID_NOLOG(taskExecutor);
            // animation finish event should be posted to UI thread.
            taskExecutor->PostTask([rootWeak, sheetWK, id]() {
                auto sheet = sheetWK.Upgrade();
                auto root = rootWeak.Upgrade();
                CHECK_NULL_VOID_NOLOG(sheet && root);
                ContainerScope scope(id);
                root->RemoveChild(sheet);
                root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
                }, TaskExecutor::TaskType::UI);
            });
        AnimationUtils::Animate(
            option,
            [context, pageHeight]() {
                if (context) {
                    context->OnTransformTranslateUpdate({ 0.0f, pageHeight, 0.0f });
                }
            }, option.GetOnFinishEvent());
    }
}

void OverlayManager::ComputeSheetOffset(NG::SheetStyle& sheetStyle)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pageNode = pipeline->GetStageManager()->GetLastPage();
    auto pageHeight = pageNode->GetGeometryNode()->GetFrameSize().Height();
    auto largeHeight = pageHeight - SHEET_BLANK_MINI_HEIGHT.ConvertToPx();
    if (sheetStyle.sheetMode.has_value()) {
        if (sheetStyle.sheetMode == SheetMode::MEDIUM) {
            sheetHeight_ = pageHeight / 2; // 2 : half
        } else if (sheetStyle.sheetMode == SheetMode::LARGE) {
            sheetHeight_ = largeHeight;
        }
    } else {
        double height = 0.0;
        if (sheetStyle.height->Unit() == DimensionUnit::PERCENT) {
            height = sheetStyle.height->ConvertToPxWithSize(pageHeight);
        } else {
            height = sheetStyle.height->ConvertToPx();
        }
        if (height > largeHeight) {
            sheetHeight_ = largeHeight;
        } else if (height < 0) {
            sheetHeight_ = largeHeight;
        } else {
            sheetHeight_ = height;
        }
    }
}

void OverlayManager::DestroySheet(const RefPtr<FrameNode>& sheetNode, int32_t targetId)
{
    auto topSheetNode = modalStack_.top().Upgrade();
    CHECK_NULL_VOID(topSheetNode);
    if (topSheetNode->GetTag() != "SheetPage") {
        return;
    }
    if (topSheetNode->GetPattern<SheetPresentationPattern>()->GetTargetId() != targetId) {
        return;
    }
    topSheetNode->GetPattern<SheetPresentationPattern>()->FireCallback("false");
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    auto root = DynamicCast<FrameNode>(rootNode);
    root->RemoveChild(sheetNode);
    root->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    modalStack_.pop();
    SaveLastModalNode();
}

#ifdef ENABLE_DRAG_FRAMEWORK
void OverlayManager::MountPixelmapToRootNode(const RefPtr<FrameNode>& columnNode)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    columnNode->MountToParent(rootNode);
    columnNode->OnMountToParentDone();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    pixelmapColumnNodeWeak_ = columnNode;
    hasPixelMap_ = true;
}

void OverlayManager::MountEventToRootNode(const RefPtr<FrameNode>& columnNode)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    columnNode->MountToParent(rootNode, 1);
    columnNode->OnMountToParentDone();
    eventColumnNodeWeak_ = columnNode;
    hasEvent_ = true;
}

void OverlayManager::RemovePixelMap()
{
    if (!hasPixelMap_) {
        return;
    }
    auto columnNode = pixelmapColumnNodeWeak_.Upgrade();
    CHECK_NULL_VOID(columnNode);
    auto rootNode = columnNode->GetParent();
    CHECK_NULL_VOID(rootNode);
    rootNode->RemoveChild(columnNode);
    rootNode->RebuildRenderContextTree();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    hasPixelMap_ = false;
    isOnAnimation_ = false;
}

void OverlayManager::RemovePixelMapAnimation(bool startDrag, double x, double y)
{
    if (isOnAnimation_ || !hasPixelMap_) {
        return;
    }
    auto columnNode = pixelmapColumnNodeWeak_.Upgrade();
    CHECK_NULL_VOID(columnNode);
    auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetFirstChild());
    CHECK_NULL_VOID(imageNode);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    RefPtr<PixelMap> pixelMap = hub->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    float scale = PIXELMAP_DRAG_SCALE;
    UpdatePixelMapScale(scale);
    int32_t width = pixelMap->GetWidth();
    int32_t height = pixelMap->GetHeight();

    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    option.SetOnFinishEvent([this, id = Container::CurrentId()] {
        ContainerScope scope(id);
        Msdp::DeviceStatus::InteractionManager::GetInstance()->SetDragWindowVisible(true);
        RemovePixelMap();
    });
    auto shadow = imageContext->GetBackShadow();
    if (!shadow.has_value()) {
        shadow = Shadow::CreateShadow(ShadowStyle::None);
    }
    imageContext->UpdateBackShadow(shadow.value());

    AnimationUtils::Animate(
        option,
        [imageContext, shadow, startDrag, x, y, width, height, scale]() mutable {
            auto color = shadow->GetColor();
            auto newColor = Color::FromARGB(1, color.GetRed(), color.GetGreen(), color.GetBlue());
            if (startDrag) {
                imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(x - width * PIXELMAP_ANIMATION_WIDTH_RATE),
                    Dimension(y - height * PIXELMAP_ANIMATION_HEIGHT_RATE)));
                imageContext->UpdateTransformScale({ scale, scale });
                imageContext->OnModifyDone();
            } else {
                shadow->SetColor(newColor);
                imageContext->UpdateBackShadow(shadow.value());
                imageContext->UpdateTransformScale({ 1.0f, 1.0f });
            }
        },
        option.GetOnFinishEvent());
    isOnAnimation_ = true;
}

void OverlayManager::UpdatePixelMapScale(float& scale)
{
    auto columnNode = pixelmapColumnNodeWeak_.Upgrade();
    CHECK_NULL_VOID(columnNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    RefPtr<PixelMap> pixelMap = hub->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    if (pixelMap->GetWidth() > Msdp::DeviceStatus::MAX_PIXEL_MAP_WIDTH ||
        pixelMap->GetHeight() > Msdp::DeviceStatus::MAX_PIXEL_MAP_HEIGHT) {
        float scaleWidth = static_cast<float>(Msdp::DeviceStatus::MAX_PIXEL_MAP_WIDTH) / pixelMap->GetWidth();
        float scaleHeight = static_cast<float>(Msdp::DeviceStatus::MAX_PIXEL_MAP_HEIGHT) / pixelMap->GetHeight();
        scale = std::min(scaleWidth, scaleHeight);
    }
}

void OverlayManager::RemoveFilter()
{
    if (!hasFilter_) {
        return;
    }
    auto columnNode = filterColumnNodeWeak_.Upgrade();
    CHECK_NULL_VOID(columnNode);
    auto rootNode = columnNode->GetParent();
    CHECK_NULL_VOID(rootNode);
    auto children = columnNode->GetChildren();
    rootNode->RemoveChild(columnNode);
    int32_t slot = 0;
    for (auto& child : children) {
        columnNode->RemoveChild(child);
        child->MountToParent(rootNode, slot);
        slot++;
    }
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    hasFilter_ = false;
}

void OverlayManager::RemoveEventColumn()
{
    if (!hasEvent_) {
        return;
    }
    auto columnNode = eventColumnNodeWeak_.Upgrade();
    CHECK_NULL_VOID(columnNode);
    auto rootNode = columnNode->GetParent();
    CHECK_NULL_VOID(rootNode);
    rootNode->RemoveChild(columnNode);
    hasEvent_ = false;
}
#endif // ENABLE_DRAG_FRAMEWORK
} // namespace OHOS::Ace::NG

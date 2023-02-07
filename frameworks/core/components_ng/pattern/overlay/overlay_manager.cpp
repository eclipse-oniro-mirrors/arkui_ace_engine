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

#include "core/components_ng/pattern/overlay/overlay_manager.h"

#include <utility>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/animation/animation_pub.h"
#include "core/components/common/properties/color.h"
#include "core/components/select/select_theme.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/bubble/bubble_event_hub.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_dialog_view.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_picker/textpicker_dialog_view.h"
#include "core/components_ng/pattern/time_picker/timepicker_dialog_view.h"
#include "core/components_ng/pattern/toast/toast_view.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
const int ANIMATION_DUR = 200;
constexpr int32_t MENU_ANIMATION_DURATION = 150;
} // namespace

void OverlayManager::Show(const RefPtr<FrameNode>& node)
{
    auto root = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(root && node);
    node->MountToParent(root);
    root->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);

    AnimationOption option;
    option.SetCurve(Curves::LINEAR);
    option.SetDuration(ANIMATION_DUR);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent([weak = WeakClaim(this), nodeWK = WeakClaim(RawPtr(node)), id = Container::CurrentId()] {
        auto node = nodeWK.Upgrade();
        auto overlayManager = weak.Upgrade();
        CHECK_NULL_VOID(node && overlayManager);
        ContainerScope scope(id);
        overlayManager->FocusDialog(node);
    });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, 0.0, 1.0);
}

void OverlayManager::OpenDialogAnimation(const RefPtr<FrameNode>& node)
{
    auto root = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(root && node);
    node->MountToParent(root);
    root->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);

    AnimationOption option;
    option.SetFillMode(FillMode::FORWARDS);

    auto dialogPattern = node->GetPattern<DialogPattern>();
    option = dialogPattern->GetOpenAnimation().value_or(option);
    auto onFinish = option.GetOnFinishEvent();

    option.SetOnFinishEvent(
        [weak = WeakClaim(this), nodeWK = WeakClaim(RawPtr(node)), id = Container::CurrentId(), onFinish] {
            auto node = nodeWK.Upgrade();
            auto overlayManager = weak.Upgrade();
            CHECK_NULL_VOID(node && overlayManager);
            ContainerScope scope(id);
            overlayManager->FocusDialog(node);

            if (onFinish != nullptr) {
                onFinish();
            }
        });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, 0.0, 1.0);
}

void OverlayManager::CloseDialogAnimation(const RefPtr<FrameNode>& node)
{
    auto root = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(root && node);

    AnimationOption option;
    option.SetFillMode(FillMode::FORWARDS);

    auto dialogPattern = node->GetPattern<DialogPattern>();
    option = dialogPattern->GetCloseAnimation().value_or(option);
    auto onFinish = option.GetOnFinishEvent();

    option.SetOnFinishEvent([root, node, id = Container::CurrentId(), onFinish] {
        ContainerScope scope(id);
        if (onFinish != nullptr) {
            onFinish();
        }

        root->RemoveChild(node);
        root->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, 1.0, 0.0);
    // start animation immediately
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
}

void OverlayManager::Pop(const RefPtr<FrameNode>& node)
{
    AnimationOption option;
    option.SetCurve(Curves::LINEAR);
    option.SetDuration(ANIMATION_DUR);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent([rootWk = rootNodeWeak_, nodeWk = WeakClaim(RawPtr(node)), id = Container::CurrentId()] {
        auto root = rootWk.Upgrade();
        auto node = nodeWk.Upgrade();
        CHECK_NULL_VOID(root && node);
        // ContainerScope is lost when running in lambda
        ContainerScope scope(id);
        root->RemoveChild(node);
        root->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    });
    auto ctx = node->GetRenderContext();
    CHECK_NULL_VOID(ctx);
    ctx->OpacityAnimation(option, 1.0, 0.0);
    // start animation immediately
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
}

void OverlayManager::ShowMenuAnimation(const RefPtr<FrameNode>& menu)
{
    AnimationOption option;
    option.SetCurve(Curves::FAST_OUT_SLOW_IN);
    option.SetDuration(MENU_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent([weak = WeakClaim(this), menuWK = WeakClaim(RawPtr(menu)), id = Container::CurrentId()] {
        auto menu = menuWK.Upgrade();
        auto overlayManager = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(menu && overlayManager);
        ContainerScope scope(id);
        overlayManager->FocusDialog(menu);
    });

    bool isSelectMenu = false;
    auto menuWrapperPattern = menu->GetPattern<MenuWrapperPattern>();
    if (menuWrapperPattern && menuWrapperPattern->IsSelectMenu()) {
        isSelectMenu = true;
    }

    auto context = menu->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateOpacity(0.0);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto menuAnimationOffset = static_cast<float>(theme->GetMenuAnimationOffset().ConvertToPx());
    if (isSelectMenu) {
        context->OnTransformTranslateUpdate({ 0.0f, -menuAnimationOffset, 0.0f });
    } else {
        context->OnTransformTranslateUpdate({ 0.0f, menuAnimationOffset, 0.0f });
    }

    AnimationUtils::Animate(
        option,
        [context]() {
            context->UpdateOpacity(1.0);
            context->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
        },
        option.GetOnFinishEvent());
}

void OverlayManager::PopMenuAnimation(const RefPtr<FrameNode>& menu)
{
    AnimationOption option;
    option.SetCurve(Curves::FAST_OUT_SLOW_IN);
    option.SetDuration(MENU_ANIMATION_DURATION);
    option.SetFillMode(FillMode::FORWARDS);
    option.SetOnFinishEvent([rootWeak = rootNodeWeak_, menuWK = WeakClaim(RawPtr(menu)), id = Container::CurrentId()] {
        auto menu = menuWK.Upgrade();
        auto root = rootWeak.Upgrade();
        CHECK_NULL_VOID_NOLOG(menu && root);
        ContainerScope scope(id);
        auto menuWrapperPattern = menu->GetPattern<MenuWrapperPattern>();
        // clear contextMenu then return
        if (menuWrapperPattern && menuWrapperPattern->IsContextMenu()) {
            SubwindowManager::GetInstance()->ClearMenuNG();
            return;
        }
        root->RemoveChild(menu);
        root->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);

        auto menuPattern = menu->GetPattern<MenuPattern>();
        if (!menuPattern || !menuPattern->IsSubMenu()) {
            return;
        }
        menuPattern->RemoveParentHoverStyle();
    });

    bool isSelectMenu = false;
    auto menuWrapperPattern = menu->GetPattern<MenuWrapperPattern>();
    if (menuWrapperPattern && menuWrapperPattern->IsSelectMenu()) {
        isSelectMenu = true;
    }

    auto context = menu->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateOpacity(1.0);
    context->OnTransformTranslateUpdate({ 0.0f, 0.0f, 0.0f });
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto menuAnimationOffset = static_cast<float>(theme->GetMenuAnimationOffset().ConvertToPx());
    AnimationUtils::Animate(
        option,
        [context, isSelectMenu, menuAnimationOffset]() {
            context->UpdateOpacity(0.0);
            if (isSelectMenu) {
                context->OnTransformTranslateUpdate({ 0.0f, -menuAnimationOffset, 0.0f });
            } else {
                context->OnTransformTranslateUpdate({ 0.0f, menuAnimationOffset, 0.0f });
            }
        },
        option.GetOnFinishEvent());

    // start animation immediately
    pipeline->RequestFrame();
}

void OverlayManager::ShowToast(
    const std::string& message, int32_t duration, const std::string& bottom, bool isRightToLeft)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    // only one toast
    if (!toast_.Invalid()) {
        rootNode->RemoveChild(toast_.Upgrade());
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }

    auto toastNode = ToastView::CreateToastNode(message, bottom, isRightToLeft);
    CHECK_NULL_VOID(toastNode);

    // mount to parent
    toastNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    toast_ = toastNode;

    context->GetTaskExecutor()->PostDelayedTask(
        [weak = WeakClaim(this)] {
            auto overlayManager = weak.Upgrade();
            CHECK_NULL_VOID(overlayManager);
            overlayManager->PopToast();
        },
        TaskExecutor::TaskType::UI, duration);
}

void OverlayManager::PopToast()
{
    auto toastUnderPop = toast_.Upgrade();
    CHECK_NULL_VOID(toastUnderPop);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    LOGI("begin to pop toast, id is %{public}d", toastUnderPop->GetId());
    rootNode->RemoveChild(toastUnderPop);
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
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
        LOGI("OverlayManager: popup begin pop");
        popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
        rootNode->RemoveChild(popupMap_[targetId].popupNode);
    } else {
        // Push popup
        CHECK_NULL_VOID_NOLOG(!popupInfo.isCurrentOnShow);
        LOGI("OverlayManager: popup begin push");
        popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(true);
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
    auto it = customPopupMap_.find(targetId);
    if (it != customPopupMap_.end()) {
        rootNode->RemoveChild(customPopupMap_[targetId]);
        customPopupMap_.erase(it);
    }
    customPopupMap_[targetId] = customNode;
    auto popupNode = customPopupMap_[targetId];
    customNode->MountToParent(rootNode);
    customNode->MarkModifyDone();
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void OverlayManager::EraseIndexerPopup(int32_t targetId)
{
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    auto it = customPopupMap_.find(targetId);
    if (it != customPopupMap_.end()) {
        rootNode->RemoveChild(customPopupMap_[targetId]);
        customPopupMap_.erase(it);
    }
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

RefPtr<FrameNode> OverlayManager::GetIndexerPopup(int32_t targetId)
{
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto it = customPopupMap_.find(targetId);
    if (it != customPopupMap_.end()) {
        return customPopupMap_[targetId];
    }
    return nullptr;
}

void OverlayManager::HidePopup(int32_t targetId, const PopupInfo& popupInfo)
{
    popupMap_[targetId] = popupInfo;
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    CHECK_NULL_VOID_NOLOG(popupInfo.markNeedUpdate);
    CHECK_NULL_VOID_NOLOG(popupInfo.popupNode);
    popupMap_[targetId].markNeedUpdate = false;
    auto rootChildren = rootNode->GetChildren();
    auto iter = std::find(rootChildren.begin(), rootChildren.end(), popupInfo.popupNode);
    if (iter == rootChildren.end()) {
        LOGW("OverlayManager: popupNode is not found in rootChildren");
        return;
    }
    CHECK_NULL_VOID_NOLOG(popupInfo.isCurrentOnShow);
    LOGI("begin pop");
    popupInfo.popupNode->GetEventHub<BubbleEventHub>()->FireChangeEvent(false);
    rootNode->RemoveChild(popupMap_[targetId].popupNode);
    popupMap_[targetId].isCurrentOnShow = !popupInfo.isCurrentOnShow;
    rootNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void OverlayManager::HideAllPopups()
{
    LOGI("OverlayManager::HideAllPopups");
    if (popupMap_.empty()) {
        LOGW("OverlayManager: popupMap is empty");
        return;
    }
    for (const auto& popup : popupMap_) {
        auto popupInfo = popup.second;
        if (popupInfo.isCurrentOnShow && popupInfo.target.Upgrade()) {
            popupInfo.markNeedUpdate = true;
            popupInfo.popupId = -1;
            UpdatePopupNode(popupInfo.target.Upgrade()->GetId(), popupInfo);
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
    menuFrameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
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
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);

        ShowMenuAnimation(menu);
        menu->MarkModifyDone();
        LOGI("menuNode mounted");
    }
}

// subwindow only contains one menu instance.
void OverlayManager::ShowMenuInSubWindow(int32_t targetId, const NG::OffsetF& offset, RefPtr<FrameNode> menu)
{
    if (!ShowMenuHelper(menu, targetId, offset)) {
        LOGW("show menu failed");
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_VOID(rootNode);
    rootNode->Clean();
    menu->MountToParent(rootNode);
    ShowMenuAnimation(menu);
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
    BlurDialog();
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
    BlurDialog();
}

void OverlayManager::HideAllMenus()
{
    LOGI("OverlayManager::HideAllMenus");
    if (menuMap_.empty()) {
        LOGW("OverlayManager: menuMap is empty");
        return;
    }
    auto rootNode = rootNodeWeak_.Upgrade();
    for (const auto& child : rootNode->GetChildren()) {
        auto node = DynamicCast<FrameNode>(child);
        if (node->GetTag() == V2::MENU_WRAPPER_ETS_TAG) {
            PopMenuAnimation(node);
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
    OpenDialogAnimation(dialog);
    return dialog;
}

void OverlayManager::ShowDateDialog(const DialogProperties& dialogProps,
    std::map<std::string, PickerDate> datePickerProperty, bool isLunar,
    std::map<std::string, NG::DialogEvent> dialogEvent, std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto dialogNode = DatePickerDialogView::Show(
        dialogProps, std::move(datePickerProperty), isLunar, std::move(dialogEvent), std::move(dialogCancelEvent));
    CHECK_NULL_VOID(dialogNode);
    Show(dialogNode);
}

void OverlayManager::ShowTimeDialog(const DialogProperties& dialogProps,
    std::map<std::string, PickerTime> timePickerProperty, bool isUseMilitaryTime,
    std::map<std::string, NG::DialogEvent> dialogEvent, std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto dialogNode = TimePickerDialogView::Show(dialogProps, std::move(timePickerProperty), isUseMilitaryTime,
        std::move(dialogEvent), std::move(dialogCancelEvent));
    Show(dialogNode);
}

void OverlayManager::ShowTextDialog(const DialogProperties& dialogProps, uint32_t selected, const Dimension& height,
    const std::vector<std::string>& getRangeVector, std::map<std::string, NG::DialogTextEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto dialogNode = TextPickerDialogView::Show(
        dialogProps, selected, height, getRangeVector, std::move(dialogEvent), std::move(dialogCancelEvent));
    Show(dialogNode);
}

void OverlayManager::CloseDialog(const RefPtr<FrameNode>& dialogNode)
{
    LOGI("OverlayManager::CloseDialog");
    CloseDialogAnimation(dialogNode);
    BlurDialog();
}

bool OverlayManager::RemoveOverlay()
{
    auto rootNode = rootNodeWeak_.Upgrade();
    CHECK_NULL_RETURN(rootNode, true);
    auto childrenSize = rootNode->GetChildren().size();
    if (rootNode->GetChildren().size() > 1) {
        // stage node is at index 0, remove overlay at index 1
        auto overlay = DynamicCast<FrameNode>(rootNode->GetChildAtIndex(1));
        CHECK_NULL_RETURN(overlay, false);
        // close dialog with animation
        auto pattern = overlay->GetPattern();
        if (AceType::DynamicCast<DialogPattern>(pattern)) {
            CloseDialog(overlay);
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
        rootNode->RemoveChildAtIndex(1);
        rootNode->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        LOGI("overlay removed successfully");
        return true;
    }
    return false;
}

void OverlayManager::FocusDialog(const RefPtr<FrameNode>& dialogNode)
{
    LOGI("OverlayManager::FocusDialog when dialog show");
    CHECK_NULL_VOID(dialogNode);
    auto focusHub = dialogNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pageFocusHub = pageNode->GetFocusHub();
    CHECK_NULL_VOID(pageFocusHub);
    pageFocusHub->LostFocus();
}

void OverlayManager::BlurDialog()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto pageNode = stageManager->GetLastPage();
    CHECK_NULL_VOID(pageNode);
    auto pageFocusHub = pageNode->GetFocusHub();
    CHECK_NULL_VOID(pageFocusHub);
    pageFocusHub->RequestFocus();
}

} // namespace OHOS::Ace::NG

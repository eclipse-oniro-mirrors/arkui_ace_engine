/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "base/subwindow/subwindow_manager.h"

#include <memory>
#include <mutex>

#include "unistd.h"

#include "base/geometry/rect.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "core/common/ace_page.h"
#include "core/common/container.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace {

std::mutex SubwindowManager::instanceMutex_;
std::shared_ptr<SubwindowManager> SubwindowManager::instance_;

std::shared_ptr<SubwindowManager> SubwindowManager::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (!instance_) {
        instance_ = std::make_shared<SubwindowManager>();
    }
    return instance_;
}

void SubwindowManager::AddContainerId(uint32_t windowId, int32_t containerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto result = containerMap_.try_emplace(windowId, containerId);
    if (!result.second) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "Already have container of this windowId, windowId: %{public}u", windowId);
    }
}

void SubwindowManager::RemoveContainerId(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    containerMap_.erase(windowId);
}

int32_t SubwindowManager::GetContainerId(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto result = containerMap_.find(windowId);
    if (result != containerMap_.end()) {
        return result->second;
    } else {
        return -1;
    }
}

void SubwindowManager::AddParentContainerId(int32_t containerId, int32_t parentContainerId)
{
    TAG_LOGI(AceLogTag::ACE_SUB_WINDOW, "Container id is %{public}d, parent id is %{public}d.", containerId,
        parentContainerId);
    std::lock_guard<std::mutex> lock(parentMutex_);
    parentContainerMap_.try_emplace(containerId, parentContainerId);
}

void SubwindowManager::RemoveParentContainerId(int32_t containerId)
{
    std::lock_guard<std::mutex> lock(parentMutex_);
    parentContainerMap_.erase(containerId);
}

int32_t SubwindowManager::GetParentContainerId(int32_t containerId)
{
    std::lock_guard<std::mutex> lock(parentMutex_);
    auto result = parentContainerMap_.find(containerId);
    if (result != parentContainerMap_.end()) {
        return result->second;
    } else {
        return -1;
    }
}

int32_t SubwindowManager::GetSubContainerId(int32_t parentContainerId)
{
    std::lock_guard<std::mutex> lock(parentMutex_);
    for (auto it = parentContainerMap_.begin(); it != parentContainerMap_.end(); it++) {
        if (it->second == parentContainerId) {
            return it->first;
        }
    }
    return -1;
}

void SubwindowManager::AddSubwindow(int32_t instanceId, RefPtr<Subwindow> subwindow)
{
    if (!subwindow) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "add subwindow failed.");
        return;
    }
    TAG_LOGI(AceLogTag::ACE_SUB_WINDOW, "Add subwindow into map, instanceId is %{public}d, subwindow id is %{public}d.",
        instanceId, subwindow->GetSubwindowId());
    std::lock_guard<std::mutex> lock(subwindowMutex_);
    auto result = subwindowMap_.try_emplace(instanceId, subwindow);
    if (!result.second) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "Add failed of this instance %{public}d", instanceId);
        return;
    }
}
void SubwindowManager::DeleteHotAreas(int32_t instanceId, int32_t nodeId)
{
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    } else {
        subwindow = GetCurrentWindow();
    }
    if (subwindow) {
        subwindow->DeleteHotAreas(nodeId);
    }
}
void SubwindowManager::RemoveSubwindow(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(subwindowMutex_);
    subwindowMap_.erase(instanceId);
}

const RefPtr<Subwindow> SubwindowManager::GetSubwindow(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(subwindowMutex_);
    auto result = subwindowMap_.find(instanceId);
    if (result != subwindowMap_.end()) {
        return result->second;
    } else {
        return nullptr;
    }
}

int32_t SubwindowManager::GetDialogSubwindowInstanceId(int32_t SubwindowId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "get dialog subwindow instanceid enter");
    std::lock_guard<std::mutex> lock(subwindowMutex_);
    for (auto it = subwindowMap_.begin(); it != subwindowMap_.end(); it++) {
        if (it->second->GetSubwindowId() == SubwindowId) {
            return it->first;
        }
    }
    return 0;
}

void SubwindowManager::SetCurrentSubwindowName(const std::string& currentSubwindowName)
{
    std::lock_guard<std::mutex> lock(currentSubwindowMutex_);
    currentSubwindowName_ = currentSubwindowName;
}

std::string SubwindowManager::GetCurrentSubWindowName()
{
    std::lock_guard<std::mutex> lock(currentSubwindowMutex_);
    return currentSubwindowName_;
}

void SubwindowManager::SetCurrentSubwindow(const RefPtr<Subwindow>& subwindow)
{
    std::lock_guard<std::mutex> lock(currentSubwindowMutex_);
    currentSubwindow_ = subwindow;
}

const RefPtr<Subwindow>& SubwindowManager::GetCurrentWindow()
{
    std::lock_guard<std::mutex> lock(currentSubwindowMutex_);
    return currentSubwindow_;
}

Rect SubwindowManager::GetParentWindowRect()
{
    std::lock_guard<std::mutex> lock(currentSubwindowMutex_);
    Rect rect;
    CHECK_NULL_RETURN(currentSubwindow_, rect);
    return currentSubwindow_->GetParentWindowRect();
}

RefPtr<Subwindow> SubwindowManager::ShowPreviewNG()
{
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    if (!subwindow->ShowPreviewNG()) {
        return nullptr;
    }
    return subwindow;
}

void SubwindowManager::ShowMenuNG(const RefPtr<NG::FrameNode>& menuNode, const NG::MenuParam& menuParam,
    const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show menu ng enter");
    CHECK_NULL_VOID(targetNode);
    auto pipelineContext = targetNode->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto containerId = pipelineContext->GetInstanceId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    subwindow->ShowMenuNG(menuNode, menuParam, targetNode, offset);
}

void SubwindowManager::ShowMenuNG(std::function<void()>&& buildFunc, std::function<void()>&& previewBuildFunc,
    const NG::MenuParam& menuParam, const RefPtr<NG::FrameNode>& targetNode, const NG::OffsetF& offset)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show menu ng enter");
    CHECK_NULL_VOID(targetNode);
    auto pipelineContext = targetNode->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto containerId = pipelineContext->GetInstanceId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    subwindow->ShowMenuNG(std::move(buildFunc), std::move(previewBuildFunc), menuParam, targetNode, offset);
}

void SubwindowManager::HidePreviewNG()
{
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->HidePreviewNG();
    }
}

void SubwindowManager::HideMenuNG(const RefPtr<NG::FrameNode>& menu, int32_t targetId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "hide menu ng enter");
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->HideMenuNG(menu, targetId);
    }
}

void SubwindowManager::HideMenuNG(bool showPreviewAnimation, bool startDrag)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "hide menu ng enter");
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->HideMenuNG(showPreviewAnimation, startDrag);
    }
}

void SubwindowManager::UpdateHideMenuOffsetNG(const NG::OffsetF& offset)
{
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->UpdateHideMenuOffsetNG(offset);
    }
}

void SubwindowManager::ContextMenuSwitchDragPreviewAnimation(const RefPtr<NG::FrameNode>& dragPreviewNode,
    const NG::OffsetF& offset)
{
    CHECK_NULL_VOID(dragPreviewNode);
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->ContextMenuSwitchDragPreviewAnimationtNG(dragPreviewNode, offset);
    }
}

void SubwindowManager::UpdatePreviewPosition(const NG::OffsetF& offset, const Rect& rect)
{
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->UpdatePreviewPosition(offset, rect);
    }
}

void SubwindowManager::ClearMenuNG(int32_t instanceId, int32_t targetId, bool inWindow, bool showAnimation)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "clear menu ng enter");
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
#ifdef OHOS_STANDARD_SYSTEM
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
#else
        subwindow =
            GetSubwindow(GetParentContainerId(instanceId) != -1 ? GetParentContainerId(instanceId) : instanceId);
#endif
    } else {
        subwindow = GetCurrentWindow();
    }
    if (subwindow) {
        subwindow->ClearMenuNG(targetId, inWindow, showAnimation);
    }
}

void SubwindowManager::ClearPopupInSubwindow(int32_t instanceId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "clear popup in subwindow enter");
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    } else {
        subwindow = GetCurrentWindow();
    }
    if (subwindow) {
        subwindow->ClearPopupNG();
    }
}

void SubwindowManager::ShowPopupNG(const RefPtr<NG::FrameNode>& targetNode, const NG::PopupInfo& popupInfo,
    const std::function<void(int32_t)>&& onWillDismiss, bool interactiveDismiss)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show popup ng enter");
    CHECK_NULL_VOID(targetNode);
    auto pipelineContext = targetNode->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto containerId = pipelineContext->GetInstanceId();

    auto manager = SubwindowManager::GetInstance();
    CHECK_NULL_VOID(manager);
    auto subwindow = manager->GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->InitContainer();
        manager->AddSubwindow(containerId, subwindow);
    }
    subwindow->ShowPopupNG(targetNode->GetId(), popupInfo, std::move(onWillDismiss), interactiveDismiss);
}

void SubwindowManager::HidePopupNG(int32_t targetId, int32_t instanceId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "hide popup ng enter");
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    } else {
        subwindow = GetCurrentWindow();
    }

    if (subwindow) {
        subwindow->HidePopupNG(targetId);
    }
}

void SubwindowManager::ShowPopup(const RefPtr<Component>& newComponent, bool disableTouchEvent)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show popup enter");
    auto containerId = Container::CurrentId();
    auto taskExecutor = Container::CurrentTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [containerId, newComponentWeak = WeakPtr<Component>(newComponent), disableTouchEvent] {
            auto manager = SubwindowManager::GetInstance();
            CHECK_NULL_VOID(manager);
            auto subwindow = manager->GetSubwindow(containerId);
            if (!subwindow) {
                subwindow = Subwindow::CreateSubwindow(containerId);
                subwindow->InitContainer();
                manager->AddSubwindow(containerId, subwindow);
            }
            auto newComponent = newComponentWeak.Upgrade();
            CHECK_NULL_VOID(newComponent);
            subwindow->ShowPopup(newComponent, disableTouchEvent);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUISubwindowShowPopup");
}

bool SubwindowManager::CancelPopup(const std::string& id)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "cancel popup enter");
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        return subwindow->CancelPopup(id);
    }
    return false;
}

void SubwindowManager::ShowMenu(const RefPtr<Component>& newComponent)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show menu enter");
    auto containerId = Container::CurrentId();
    auto taskExecutor = Container::CurrentTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [containerId, weakMenu = AceType::WeakClaim(AceType::RawPtr(newComponent))] {
            auto manager = SubwindowManager::GetInstance();
            CHECK_NULL_VOID(manager);
            auto menu = weakMenu.Upgrade();
            CHECK_NULL_VOID(menu);
            auto subwindow = manager->GetSubwindow(containerId);
            if (!subwindow) {
                subwindow = Subwindow::CreateSubwindow(containerId);
                subwindow->InitContainer();
                manager->AddSubwindow(containerId, subwindow);
            }
            subwindow->ShowMenu(menu);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUISubwindowShowMenu");
}

void SubwindowManager::CloseMenu()
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "close menu enter");
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->CloseMenu();
    }
}

void SubwindowManager::ClearMenu()
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "clear menu enter");
    auto subwindow = GetCurrentWindow();
    if (subwindow) {
        subwindow->ClearMenu();
    }
}

void SubwindowManager::SetHotAreas(const std::vector<Rect>& rects, int32_t nodeId, int32_t instanceId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "set hot areas enter");
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    } else {
        subwindow = GetCurrentWindow();
    }

    if (subwindow) {
        subwindow->SetHotAreas(rects, nodeId);
    }
}

RefPtr<NG::FrameNode> SubwindowManager::ShowDialogNG(
    const DialogProperties& dialogProps, std::function<void()>&& buildFunc)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show dialog ng enter");
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        CHECK_NULL_RETURN(subwindow, nullptr);
        CHECK_NULL_RETURN(subwindow->CheckHostWindowStatus(), nullptr);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    return subwindow->ShowDialogNG(dialogProps, std::move(buildFunc));
}
RefPtr<NG::FrameNode> SubwindowManager::ShowDialogNGWithNode(const DialogProperties& dialogProps,
    const RefPtr<NG::UINode>& customNode)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show dialog ng enter");
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        CHECK_NULL_RETURN(subwindow, nullptr);
        CHECK_NULL_RETURN(subwindow->CheckHostWindowStatus(), nullptr);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    return subwindow->ShowDialogNGWithNode(dialogProps, customNode);
}
void SubwindowManager::CloseDialogNG(const RefPtr<NG::FrameNode>& dialogNode)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "close dialog ng enter");
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "get subwindow failed.");
        return;
    }
    return subwindow->CloseDialogNG(dialogNode);
}

void SubwindowManager::OpenCustomDialogNG(const DialogProperties& dialogProps, std::function<void(int32_t)>&& callback)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show customDialog ng enter");
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        CHECK_NULL_VOID(subwindow);
        CHECK_NULL_VOID(subwindow->CheckHostWindowStatus());
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    return subwindow->OpenCustomDialogNG(dialogProps, std::move(callback));
}

void SubwindowManager::CloseCustomDialogNG(int32_t dialogId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "close customDialog ng enter");
    auto iter = subwindowMap_.begin();
    while (iter != subwindowMap_.end()) {
        auto overlay = iter->second->GetOverlayManager();
        if (overlay->GetDialogMap().find(dialogId) != overlay->GetDialogMap().end()) {
            return overlay->CloseCustomDialog(dialogId);
        }
        iter++;
    }
}

void SubwindowManager::CloseCustomDialogNG(const WeakPtr<NG::UINode>& node, std::function<void(int32_t)>&& callback)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "close customDialog ng enter");
    auto iter = subwindowMap_.begin();
    while (iter != subwindowMap_.end()) {
        auto overlay = iter->second->GetOverlayManager();
        overlay->CloseCustomDialog(node, std::move(callback));
        iter++;
    }
}

void SubwindowManager::UpdateCustomDialogNG(
    const WeakPtr<NG::UINode>& node, const PromptDialogAttr &dialogAttr, std::function<void(int32_t)>&& callback)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "update customDialog ng enter");
    DialogProperties dialogProperties = {
        .isSysBlurStyle = false,
        .autoCancel = dialogAttr.autoCancel,
        .maskColor = dialogAttr.maskColor
    };
    if (dialogAttr.alignment.has_value()) {
        dialogProperties.alignment = dialogAttr.alignment.value();
    }
    if (dialogAttr.offset.has_value()) {
        dialogProperties.offset = dialogAttr.offset.value();
    }
    auto iter = subwindowMap_.begin();
    while (iter != subwindowMap_.end()) {
        auto overlay = iter->second->GetOverlayManager();
        overlay->UpdateCustomDialog(node, dialogProperties, std::move(callback));
        iter++;
    }
}

void SubwindowManager::HideDialogSubWindow(int32_t instanceId)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "hide dialog subwindow enter");
    auto subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    CHECK_NULL_VOID(subwindow);
    auto overlay = subwindow->GetOverlayManager();
    if (overlay->GetDialogMap().size() == 0) {
        subwindow->HideSubWindowNG();
    }
}

void SubwindowManager::AddDialogSubwindow(int32_t instanceId, const RefPtr<Subwindow>& subwindow)
{
    if (!subwindow) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "Add dialog subwindow failed, the subwindow is null.");
        return;
    }
    std::lock_guard<std::mutex> lock(dialogSubwindowMutex_);
    auto result = dialogSubwindowMap_.try_emplace(instanceId, subwindow);
    if (!result.second) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "Add dialog failed of this instance %{public}d", instanceId);
        return;
    }
}

const RefPtr<Subwindow> SubwindowManager::GetDialogSubwindow(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(dialogSubwindowMutex_);
    auto result = dialogSubwindowMap_.find(instanceId);
    if (result != dialogSubwindowMap_.end()) {
        return result->second;
    } else {
        return nullptr;
    }
}

void SubwindowManager::SetCurrentDialogSubwindow(const RefPtr<Subwindow>& subwindow)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "set current dialog subwindow enter");
    std::lock_guard<std::mutex> lock(currentDialogSubwindowMutex_);
    currentDialogSubwindow_ = subwindow;
}

const RefPtr<Subwindow>& SubwindowManager::GetCurrentDialogWindow()
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "get current dialog window enter");
    std::lock_guard<std::mutex> lock(currentDialogSubwindowMutex_);
    return currentDialogSubwindow_;
}

RefPtr<Subwindow> SubwindowManager::GetOrCreateSubWindow(bool isDialog)
{
    auto containerId = Container::CurrentId();
    auto subwindow = GetDialogSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        CHECK_NULL_RETURN(subwindow, nullptr);
        if (isDialog) {
            CHECK_NULL_RETURN(subwindow->CheckHostWindowStatus(), nullptr);
        }
        AddDialogSubwindow(containerId, subwindow);
    }
    return subwindow;
}

RefPtr<Subwindow> SubwindowManager::GetOrCreateSystemSubWindow()
{
    auto containerId = Container::CurrentId();
    auto subwindow = GetSystemToastWindow();
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        CHECK_NULL_RETURN(subwindow, nullptr);
        SetSystemToastWindow(subwindow);
    }
    return subwindow;
}

void SubwindowManager::ShowToast(const std::string& message, int32_t duration, const std::string& bottom,
    const NG::ToastShowMode& showMode, int32_t alignment, std::optional<DimensionOffset> offset)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show toast enter");
    auto containerId = Container::CurrentId();
    // for pa service
    if (containerId >= MIN_PA_SERVICE_ID || containerId < 0) {
        auto subwindow =
            showMode == NG::ToastShowMode::SYSTEM_TOP_MOST ? GetOrCreateSystemSubWindow() : GetOrCreateSubWindow();
        CHECK_NULL_VOID(subwindow);
        TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "before show toast");
        subwindow->ShowToast(message, duration, bottom, showMode, alignment, offset);
    } else {
        // for ability
        auto taskExecutor = Container::CurrentTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [containerId, message, duration, bottom, showMode, alignment, offset] {
                auto manager = SubwindowManager::GetInstance();
                CHECK_NULL_VOID(manager);
                auto subwindow = manager->GetOrCreateToastWindow(containerId, showMode);
                CHECK_NULL_VOID(subwindow);
                TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "before show toast : %{public}d", containerId);
                subwindow->ShowToast(message, duration, bottom, showMode, alignment, offset);
            },
            TaskExecutor::TaskType::PLATFORM, "ArkUISubwindowShowToast");
    }
}

RefPtr<Subwindow> SubwindowManager::GetOrCreateToastWindow(int32_t containerId, const NG::ToastShowMode& showMode)
{
    auto isSystemTopMost = (showMode == NG::ToastShowMode::SYSTEM_TOP_MOST);
    auto subwindow = isSystemTopMost ? GetSystemToastWindow() : GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->SetIsSystemTopMost(isSystemTopMost);
        subwindow->SetAboveApps(showMode == NG::ToastShowMode::TOP_MOST);
        subwindow->InitContainer();
        if (isSystemTopMost) {
            SetSystemToastWindow(subwindow);
        } else {
            AddSubwindow(containerId, subwindow);
        }
    }

    return subwindow;
}

void SubwindowManager::ClearToastInSubwindow()
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "clear toast in subwindow enter");
    auto containerId = Container::CurrentId();
    // Get active container when current instanceid is less than 0
    if (containerId < 0) {
        auto container = Container::GetActive();
        if (container) {
            containerId = container->GetInstanceId();
        }
    }
    RefPtr<Subwindow> subwindow;
    // The main window does not need to clear Toast
    if (containerId != -1 && containerId < MIN_SUBCONTAINER_ID) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(containerId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(containerId) : containerId);
    }
    if (subwindow) {
        subwindow->ClearToast();
    }
}

void SubwindowManager::ShowDialog(const std::string& title, const std::string& message,
    const std::vector<ButtonInfo>& buttons, bool autoCancel, std::function<void(int32_t, int32_t)>&& napiCallback,
    const std::set<std::string>& dialogCallbacks)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show dialog enter");
    auto containerId = Container::CurrentId();
    // Get active container when current instanceid is less than 0
    if (containerId < 0) {
        auto container = Container::GetActive();
        if (container) {
            containerId = container->GetInstanceId();
        }
    }
    // for pa service
    if (containerId >= MIN_PA_SERVICE_ID || containerId < 0) {
        auto subwindow = GetOrCreateSubWindow(true);
        CHECK_NULL_VOID(subwindow);
        subwindow->ShowDialog(title, message, buttons, autoCancel, std::move(napiCallback), dialogCallbacks);
        // for ability
    } else {
        auto subwindow = GetSubwindow(containerId);
        if (!subwindow) {
            subwindow = Subwindow::CreateSubwindow(containerId);
            CHECK_NULL_VOID(subwindow);
            CHECK_NULL_VOID(subwindow->CheckHostWindowStatus());
            subwindow->InitContainer();
            AddSubwindow(containerId, subwindow);
        }
        subwindow->ShowDialog(title, message, buttons, autoCancel, std::move(napiCallback), dialogCallbacks);
    }
}

void SubwindowManager::ShowDialog(const PromptDialogAttr& dialogAttr, const std::vector<ButtonInfo>& buttons,
    std::function<void(int32_t, int32_t)>&& napiCallback, const std::set<std::string>& dialogCallbacks)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show dialog enter");
    auto containerId = Container::CurrentId();
    // Get active container when current instanceid is less than 0
    if (containerId < 0) {
        auto container = Container::GetActive();
        if (container) {
            containerId = container->GetInstanceId();
        }
    }
    // for pa service
    if (containerId >= MIN_PA_SERVICE_ID || containerId < 0) {
        auto subWindow = GetOrCreateSubWindow(true);
        CHECK_NULL_VOID(subWindow);
        subWindow->ShowDialog(dialogAttr, buttons, std::move(napiCallback), dialogCallbacks);
        // for ability
    } else {
        auto subWindow = GetSubwindow(containerId);
        if (!subWindow) {
            subWindow = Subwindow::CreateSubwindow(containerId);
            CHECK_NULL_VOID(subWindow);
            CHECK_NULL_VOID(subWindow->CheckHostWindowStatus());
            subWindow->InitContainer();
            AddSubwindow(containerId, subWindow);
        }
        subWindow->ShowDialog(dialogAttr, buttons, std::move(napiCallback), dialogCallbacks);
    }
}

void SubwindowManager::ShowActionMenu(
    const std::string& title, const std::vector<ButtonInfo>& button, std::function<void(int32_t, int32_t)>&& callback)
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "show action menu enter");
    auto containerId = Container::CurrentId();
    // Get active container when current instanceid is less than 0
    if (containerId < 0) {
        auto container = Container::GetActive();
        if (container) {
            containerId = container->GetInstanceId();
        }
    }
    // for pa service
    if (containerId >= MIN_PA_SERVICE_ID || containerId < 0) {
        auto subwindow = GetOrCreateSubWindow(true);
        CHECK_NULL_VOID(subwindow);
        subwindow->ShowActionMenu(title, button, std::move(callback));
        // for ability
    } else {
        auto subwindow = GetSubwindow(containerId);
        if (!subwindow) {
            subwindow = Subwindow::CreateSubwindow(containerId);
            CHECK_NULL_VOID(subwindow);
            CHECK_NULL_VOID(subwindow->CheckHostWindowStatus());
            subwindow->InitContainer();
            AddSubwindow(containerId, subwindow);
        }
        subwindow->ShowActionMenu(title, button, std::move(callback));
    }
}

void SubwindowManager::CloseDialog(int32_t instanceId)
{
    auto subwindow = GetDialogSubwindow(instanceId);
    if (!subwindow) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW, "get dialog subwindow failed.");
        return;
    }
    for (auto& containerMap : parentContainerMap_) {
        if (containerMap.second == instanceId) {
            subwindow->CloseDialog(containerMap.first);
        }
    }
}

void SubwindowManager::OpenCustomDialog(const PromptDialogAttr &dialogAttr, std::function<void(int32_t)> &&callback)
{
    PromptDialogAttr tmpPromptAttr = dialogAttr;
    tmpPromptAttr.showInSubWindow = false;
    auto containerId = Container::CurrentId();
    // for pa service
    TAG_LOGI(AceLogTag::ACE_SUB_WINDOW, "container %{public}d open the custom dialog", containerId);
    if (containerId >= MIN_PA_SERVICE_ID || containerId < 0) {
        auto subWindow = GetOrCreateSubWindow(true);
        CHECK_NULL_VOID(subWindow);
        subWindow->OpenCustomDialog(tmpPromptAttr, std::move(callback));
        // for ability
    } else {
        auto subWindow = GetSubwindow(containerId);
        if (!subWindow) {
            subWindow = Subwindow::CreateSubwindow(containerId);
            CHECK_NULL_VOID(subWindow);
            CHECK_NULL_VOID(subWindow->CheckHostWindowStatus());
            subWindow->InitContainer();
            AddSubwindow(containerId, subWindow);
        }
        subWindow->OpenCustomDialog(tmpPromptAttr, std::move(callback));
    }
    return;
}

void SubwindowManager::CloseCustomDialog(const int32_t dialogId)
{
    auto containerId = Container::CurrentId();
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "CloseCustomDialog dialogId = %{public}d, containerId = %{public}d.",
        dialogId, containerId);
    auto subwindow = GetDialogSubwindow(containerId);
    if (!subwindow) {
        return;
    }
    subwindow->CloseCustomDialog(dialogId);
    return;
}

void SubwindowManager::CloseCustomDialog(const WeakPtr<NG::UINode>& node, std::function<void(int32_t)> &&callback)
{
    auto containerId = Container::CurrentId();
    auto subwindow = GetDialogSubwindow(containerId);
    if (!subwindow) {
        return;
    }
    subwindow->CloseCustomDialog(node, std::move(callback));
    return;
}

void SubwindowManager::HideSubWindowNG()
{
    RefPtr<Subwindow> subwindow;
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    if (container->IsDialogContainer()) {
        subwindow = GetCurrentDialogWindow();
    } else {
        subwindow = GetCurrentWindow();
    }
    if (subwindow) {
        subwindow->HideSubWindowNG();
    }
}

void SubwindowManager::RequestFocusSubwindow(int32_t instanceId)
{
    RefPtr<Subwindow> subwindow;
    if (instanceId != -1) {
        // get the subwindow which overlay node in, not current
        subwindow = GetSubwindow(instanceId >= MIN_SUBCONTAINER_ID ? GetParentContainerId(instanceId) : instanceId);
    } else {
        subwindow = GetCurrentWindow();
    }
    if (subwindow) {
        subwindow->RequestFocus();
    }
}

bool SubwindowManager::GetShown()
{
    auto containerId = Container::CurrentId();
    auto subwindow = GetSubwindow(containerId);
    if (!subwindow) {
        subwindow = Subwindow::CreateSubwindow(containerId);
        subwindow->InitContainer();
        AddSubwindow(containerId, subwindow);
    }
    return subwindow->GetShown();
}

void SubwindowManager::ResizeWindowForFoldStatus(int32_t parentContainerId)
{
    auto containerId = Container::CurrentId();
    auto subwindow = parentContainerId < 0 ? GetDialogSubwindow(parentContainerId) : GetSubwindow(containerId);
    if (!subwindow) {
        TAG_LOGW(AceLogTag::ACE_SUB_WINDOW,
            "Get Subwindow error, containerId = %{public}d, parentContainerId = %{public}d", containerId,
            parentContainerId);
        return;
    }
    subwindow->ResizeWindowForFoldStatus(parentContainerId);
}

void SubwindowManager::MarkDirtyDialogSafeArea()
{
    auto containerId = Container::CurrentId();
    auto manager = SubwindowManager::GetInstance();
    CHECK_NULL_VOID(manager);
    auto subwindow = manager->GetSubwindow(containerId);
    CHECK_NULL_VOID(subwindow);
    subwindow->MarkDirtyDialogSafeArea();
}

void SubwindowManager::HideSystemTopMostWindow()
{
    CHECK_NULL_VOID(systemToastWindow_);
    systemToastWindow_->HideSubWindowNG();
}

void SubwindowManager::ClearToastInSystemSubwindow()
{
    TAG_LOGD(AceLogTag::ACE_SUB_WINDOW, "clear toast in system subwindow enter");
    auto subwindow = GetSystemToastWindow();
    if (subwindow) {
        subwindow->ClearToast();
    }
}
void SubwindowManager::OnWindowSizeChanged(int32_t instanceId, Rect windowRect, WindowSizeChangeReason reason)
{
    auto container = Container::GetContainer(instanceId);
    CHECK_NULL_VOID(container);
    if (!container->IsUIExtensionWindow() || uiExtensionWindowRect_ == windowRect) {
        return;
    }
    auto subContainer = Container::GetContainer(GetSubContainerId(instanceId));
    CHECK_NULL_VOID(subContainer);
    auto pipeline = AceType::DynamicCast<NG::PipelineContext>(subContainer->GetPipelineContext());
    CHECK_NULL_VOID(pipeline);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->OnUIExtensionWindowSizeChange();
    uiExtensionWindowRect_ = windowRect;
}
} // namespace OHOS::Ace

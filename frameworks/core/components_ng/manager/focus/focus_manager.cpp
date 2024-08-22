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

#include "core/components_ng/manager/focus/focus_manager.h"

#include "base/log/dump_log.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

FocusManager::FocusManager(const RefPtr<PipelineContext>& pipeline): pipeline_(pipeline)
{
    if (pipeline && pipeline->GetRootElement()) {
        currentFocus_ = pipeline->GetRootElement()->GetFocusHub();
    }
}

void FocusManager::FocusViewShow(const RefPtr<FocusView>& focusView, bool isTriggerByStep)
{
    CHECK_NULL_VOID(focusView);
    if (!focusView->HasParentFocusHub()) {
        TAG_LOGD(AceLogTag::ACE_FOCUS, "FocusView: %{public}s/%{public}d has no parent. Do not need show.",
            focusView->GetFrameName().c_str(), focusView->GetFrameId());
        return;
    }
    focusView->SetIsViewHasShow(true);
    auto lastFocusView = lastFocusView_.Upgrade();
    if (lastFocusView) {
        if (lastFocusView == focusView || lastFocusView->IsChildFocusViewOf(focusView)) {
            return;
        }
        if (!focusView->IsChildFocusViewOf(lastFocusView)) {
            lastFocusView->LostViewFocus();
        }
    }

    auto focusViewWeak = AceType::WeakClaim(AceType::RawPtr(focusView));
    if (std::find(focusViewStack_.begin(), focusViewStack_.end(), focusViewWeak) != focusViewStack_.end()) {
        focusViewStack_.remove(focusViewWeak);
    }
    focusViewStack_.emplace_back(focusViewWeak);
    lastFocusView_ = focusViewWeak;

    // do not set LastWeakFocus to Previous node/scope in focusView when FocusViewShow trigger by FocusStep
    if (!isTriggerByStep) {
        lastFocusView = lastFocusView_.Upgrade();
        if (!lastFocusView) {
            return;
        }
        auto lastFocusViewHub = lastFocusView->GetFocusHub();
        if (lastFocusViewHub) {
            lastFocusViewHub->SetLastWeakFocusToPreviousInFocusView();
        }
    }
}

void FocusManager::FocusViewHide(const RefPtr<FocusView>& focusView)
{
    CHECK_NULL_VOID(focusView);
    focusView->LostViewFocus();
    auto lastFocusView = lastFocusView_.Upgrade();
    if (lastFocusView && (lastFocusView == focusView || lastFocusView->IsChildFocusViewOf(focusView))) {
        lastFocusView_ = nullptr;
    }
}

void FocusManager::FocusViewClose(const RefPtr<FocusView>& focusView)
{
    CHECK_NULL_VOID(focusView);
    focusView->LostViewFocus();
    focusView->SetIsViewHasShow(false);
    for (auto iter = focusViewStack_.begin(); iter != focusViewStack_.end();) {
        auto view = (*iter).Upgrade();
        if (view && (view == focusView || view->IsChildFocusViewOf(focusView))) {
            auto focusHub = view->GetFocusHub();
            if (focusHub) {
                focusHub->RemoveFocusScopeIdAndPriority();
            }
            iter = focusViewStack_.erase(iter);
        } else {
            ++iter;
        }
    }
    if (focusViewStack_.empty()) {
        lastFocusView_ = nullptr;
        return;
    }
    if (focusViewStack_.back() != lastFocusView_) {
        lastFocusView_ = focusViewStack_.back();
    }
}

void FocusManager::FlushFocusView()
{
    auto lastFocusView = lastFocusView_.Upgrade();
    auto lastFocusViewHub = lastFocusView ? lastFocusView->GetFocusHub() : nullptr;
    if (lastFocusViewHub && lastFocusViewHub->IsCurrentFocus()) {
        return;
    }
    RefPtr<FocusView> currFocusView = nullptr;
    for (const auto& weakView : focusViewStack_) {
        auto view = weakView.Upgrade();
        auto viewHub = view ? view->GetFocusHub() : nullptr;
        if (!viewHub || !viewHub->IsCurrentFocus()) {
            continue;
        }
        if (currFocusView && currFocusView->IsChildFocusViewOf(view)) {
            continue;
        }
        currFocusView = view;
    }
    lastFocusView_ = currFocusView ? AceType::WeakClaim(AceType::RawPtr(currFocusView)) : nullptr;
}

void FocusManager::GetFocusViewMap(FocusViewMap& focusViewMap)
{
    for (const auto& focusViewWeak : focusViewStack_) {
        auto focusView = focusViewWeak.Upgrade();
        if (!focusView) {
            continue;
        }
        auto focusViewId = focusView->GetFrameId();
        auto entryFocusView = focusView->GetEntryFocusView();
        if (entryFocusView && entryFocusView != focusView) {
            auto entryFocusViewId = entryFocusView->GetFrameId();
            auto entryFocusViewWeak = AceType::WeakClaim(AceType::RawPtr(entryFocusView));
            auto iter = focusViewMap.find(entryFocusViewId);
            if (iter == focusViewMap.end()) {
                focusViewMap[entryFocusViewId] = { entryFocusViewWeak, { focusViewWeak } };
            } else {
                iter->second.second.emplace_back(focusViewWeak);
            }
        } else {
            focusViewMap[focusViewId] = { focusViewWeak, {} };
        }
    }
}

void FocusManager::PaintFocusState()
{
    auto pipeline = pipeline_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    auto rootNode = pipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto rootFocusHub = rootNode->GetFocusHub();
    CHECK_NULL_VOID(rootFocusHub);
    if (!pipeline->GetIsFocusActive()) {
        return;
    }
    rootFocusHub->ClearAllFocusState();
    rootFocusHub->PaintAllFocusState();
}


void FocusManager::DumpFocusManager()
{
    if (!DumpLog::GetInstance().GetDumpFile()) {
        return;
    }
    DumpLog::GetInstance().Print("Focus view:");
    std::unordered_map<int32_t, std::pair<WeakPtr<FocusView>, std::list<WeakPtr<FocusView>>>> focusViewMap;
    GetFocusViewMap(focusViewMap);
    for (const auto& focusViewInfo : focusViewMap) {
        auto focusView = focusViewInfo.second.first.Upgrade();
        if (!focusView) {
            continue;
        }
        auto childFocusViewWeakList = focusViewInfo.second.second;
        bool isFocusedView = false;
        auto lastFocusView = lastFocusView_.Upgrade();
        auto lastEntryFocusView = lastFocusView ? lastFocusView->GetEntryFocusView() : nullptr;
        if (focusView == lastEntryFocusView) {
            isFocusedView = true;
        }
        std::string information = focusView->GetFrameName();
        information += isFocusedView ? "(*)" : "";
        information += " id:" + std::to_string(focusView->GetFrameId());
        DumpLog::GetInstance().Print(0, information, static_cast<int32_t>(childFocusViewWeakList.size()));
        for (const auto& childWeak : childFocusViewWeakList) {
            auto child = childWeak.Upgrade();
            if (!child) {
                continue;
            }
            std::string childInformation = child->GetFrameName();
            childInformation += child == lastFocusView ? "(*)" : "";
            childInformation += " id:" + std::to_string(child->GetFrameId());
            DumpLog::GetInstance().Print(1, childInformation, 0);
        }
    }
}

bool FocusManager::AddFocusScope(const std::string& focusScopeId, const RefPtr<FocusHub>& scopeFocusHub)
{
    auto iter = focusHubScopeMap_.find(focusScopeId);
    if (iter != focusHubScopeMap_.end()) {
        auto focusScope = iter->second.first.Upgrade();
        if (!focusScope) {
            iter->second.first = scopeFocusHub;
            return true;
        }
        return false;
    } else {
        focusHubScopeMap_[focusScopeId] = { scopeFocusHub, {} };
    }
    return true;
}

void FocusManager::RemoveFocusScope(const std::string& focusScopeId)
{
    auto iter = focusHubScopeMap_.find(focusScopeId);
    if (iter != focusHubScopeMap_.end()) {
        if (iter->second.second.empty()) {
            focusHubScopeMap_.erase(iter);
        } else {
            iter->second.first = nullptr;
        }
    }
}

void FocusManager::AddScopePriorityNode(const std::string& focusScopeId, const RefPtr<FocusHub>& priorFocusHub,
    bool pushFront)
{
    auto iter = focusHubScopeMap_.find(focusScopeId);
    if (iter != focusHubScopeMap_.end()) {
        if (pushFront) {
            iter->second.second.emplace_front(priorFocusHub);
        } else {
            iter->second.second.emplace_back(priorFocusHub);
        }
    } else {
        focusHubScopeMap_[focusScopeId] = { nullptr, { priorFocusHub } };
    }
}

void FocusManager::RemoveScopePriorityNode(const std::string& focusScopeId, const RefPtr<FocusHub>& priorFocusHub)
{
    auto iter = focusHubScopeMap_.find(focusScopeId);
    if (iter != focusHubScopeMap_.end()) {
        if (iter->second.second.empty()) {
            return;
        }
        iter->second.second.remove(priorFocusHub);
        auto focusScope = iter->second.first.Upgrade();
        if (!focusScope && iter->second.second.empty()) {
            focusHubScopeMap_.erase(iter);
        }
    }
}

std::optional<std::list<WeakPtr<FocusHub>>*> FocusManager::GetFocusScopePriorityList(const std::string& focusScopeId)
{
    auto iter = focusHubScopeMap_.find(focusScopeId);
    if (iter != focusHubScopeMap_.end()) {
        if (!iter->second.second.empty()) {
            return &(iter->second.second);
        }
    }
    return std::nullopt;
}

void FocusManager::UpdateCurrentFocus(const RefPtr<FocusHub>& current, SwitchingUpdateReason reason)
{
    if (isSwitchingFocus_.value_or(false)) {
        switchingFocus_ = current;
        updateReason = reason;
    }
}

RefPtr<FocusHub> FocusManager::GetCurrentFocus()
{
    return currentFocus_.Upgrade();
}

int32_t FocusManager::AddFocusListener(FocusChangeCallback&& callback)
{
    // max callbacks count: INT32_MAX - 1
    if (listeners_.size() == static_cast<size_t>(std::numeric_limits<int32_t>::max() - 1)) {
        return -1;
    }
    int32_t handler = nextListenerHdl_;
    listeners_.emplace(handler, std::move(callback));

    do {
        nextListenerHdl_ = (nextListenerHdl_ == std::numeric_limits<int32_t>::max()) ? 0 : ++nextListenerHdl_;
    } while (listeners_.count(nextListenerHdl_) != 0);
    return handler;
}

void FocusManager::RemoveFocusListener(int32_t handler)
{
    listeners_.erase(handler);
}

RefPtr<FocusManager> FocusManager::GetFocusManager(RefPtr<FrameNode>& node)
{
    auto context = node->GetContextRefPtr();
    CHECK_NULL_RETURN(context, nullptr);
    auto focusManager = context->GetFocusManager();
    return focusManager;
}

void FocusManager::FocusSwitchingStart(const RefPtr<FocusHub>& focusHub,
    SwitchingStartReason reason)
{
    isSwitchingFocus_ = true;
    switchingFocus_ = focusHub;
    startReason = reason;
}

void FocusManager::FocusSwitchingEnd(SwitchingEndReason reason)
{
    // While switching window, focus may move by steps.(WindowFocus/FlushFocus)
    // Merge all steps together as a single movement.
    if (!isSwitchingFocus_.value_or(false)) {
        return;
    }
    if (!isSwitchingWindow_) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "FocusSwitching end, startReason: %{public}d, endReason: %{public}d, "
            "updateReason: %{public}d",
            startReason.value_or(SwitchingStartReason::DEFAULT),
            reason, updateReason.value_or(SwitchingUpdateReason::DEFAULT));
        for (auto& [_, cb] : listeners_) {
            cb(currentFocus_, switchingFocus_);
        }
        currentFocus_ = switchingFocus_;
        isSwitchingFocus_.reset();
        startReason.reset();
        updateReason.reset();
    } else {
        isSwitchingFocus_ = false;
        endReason = reason;
    }
}

void FocusManager::WindowFocusMoveStart()
{
    isSwitchingWindow_ = true;
}

void FocusManager::WindowFocusMoveEnd()
{
    isSwitchingWindow_ = false;
    if (!isSwitchingFocus_.value_or(true)) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "WindowFocusMove end, startReason: %{public}d, endReason: %{public}d, "
            "updateReason: %{public}d",
            startReason.value_or(SwitchingStartReason::DEFAULT),
            endReason.value_or(SwitchingEndReason::DEFAULT),
            updateReason.value_or(SwitchingUpdateReason::DEFAULT));
        for (auto& [_, cb] : listeners_) {
            cb(currentFocus_, switchingFocus_);
        }
        currentFocus_ = switchingFocus_;
        isSwitchingFocus_.reset();
        startReason.reset();
        updateReason.reset();
        endReason.reset();
    }
}

void FocusManager::FocusGuard::CreateFocusGuard(const RefPtr<FocusHub>& focusHub,
    const RefPtr<FocusManager>& focusManager, SwitchingStartReason reason)
{
    CHECK_NULL_VOID(focusHub);
    CHECK_NULL_VOID(focusManager);
    if (focusManager->isSwitchingFocus_.value_or(false)) {
        return;
    }
    focusManager->FocusSwitchingStart(focusHub, reason);
    focusMng_ = focusManager;
}

FocusManager::FocusGuard::FocusGuard(const RefPtr<FocusHub>& focusHub,
    SwitchingStartReason reason)
{
    RefPtr<FocusHub> hub = focusHub;
    if (!focusHub ||!focusHub->GetFocusManager()) {
        auto curFocusView = FocusView::GetCurrentFocusView();
        CHECK_NULL_VOID(curFocusView);
        auto curFocusViewHub = curFocusView->GetFocusHub();
        CHECK_NULL_VOID(curFocusViewHub);
        hub = curFocusViewHub;
    }
    auto mng = hub->GetFocusManager();
    CreateFocusGuard(hub, mng, reason);
}

FocusManager::FocusGuard::~FocusGuard()
{
    if (focusMng_) {
        focusMng_->FocusSwitchingEnd();
    }
}
} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "form_renderer_delegate_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace Ace {
int32_t FormRendererDelegateImpl::OnSurfaceCreate(
    const std::shared_ptr<Rosen::RSSurfaceNode>& surfaceNode,
    const OHOS::AppExecFwk::FormJsInfo& formJsInfo,
    const AAFwk::Want& want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (!surfaceNode) {
        HILOG_ERROR("surface is invalid");
        return ERR_NULL_OBJECT;
    }
    int64_t formId = formJsInfo.formId;
    if (formId < 0) {
        HILOG_ERROR("%{public}s error, the passed form id can't be negative.", __func__);
        return ERR_INVALID_DATA;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        HILOG_ERROR("%{public}s error, not find formId:%{public}s.", __func__, std::to_string(formId).c_str());
        return ERR_INVALID_DATA;
    }
    for (const auto& callback : iter->second) {
        HILOG_INFO("%{public}s, formId: %{public}" PRId64 ", jspath: %{public}s, data: %{public}s", __func__, formId,
            formJsInfo.jsFormCodePath.c_str(), formJsInfo.formData.c_str());
        callback->ProcessAddFormSurface(formJsInfo, surfaceNode, want);
    }
    return ERR_OK;
}

int32_t FormRendererDelegateImpl::OnActionEvent(const std::string& action)
{
    HILOG_INFO("OnActionEvent %{public}s", action.c_str());
    if (!actionEventHandler_) {
        HILOG_ERROR("actionEventHandler_ is null,  %{public}s", action.c_str());
        return ERR_INVALID_DATA;
    }

    actionEventHandler_(action);
    return ERR_OK;
}

int32_t FormRendererDelegateImpl::OnError(const std::string& param)
{
    HILOG_INFO("OnError %{public}s", param.c_str());
    if (!errorEventHandler_) {
        HILOG_ERROR("errorEventHandler_ is null,  %{public}s", param.c_str());
        return ERR_INVALID_DATA;
    }

    errorEventHandler_(param);
    return ERR_OK;
}

void FormRendererDelegateImpl::RegisterSurfaceCreateCallback(
    std::shared_ptr<FormSurfaceCallbackInterface> formCallback, int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0 || formCallback == nullptr) {
        HILOG_ERROR("%{public}s error, invalid formId or formCallback.", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        std::set<std::shared_ptr<FormSurfaceCallbackInterface>> callbacks;
        callbacks.emplace(formCallback);
        formCallbackMap_.emplace(formId, callbacks);
    } else {
        iter->second.emplace(formCallback);
    }
}

void FormRendererDelegateImpl::SetActionEventHandler(
    std::function<void(const std::string&)>&& listener)
{
    actionEventHandler_ = std::move(listener);
}

void FormRendererDelegateImpl::SetErrorEventHandler(
    std::function<void(const std::string&)>&& listener)
{
    errorEventHandler_ = std::move(listener);
}
} // namespace Ace
} // namespace OHOS

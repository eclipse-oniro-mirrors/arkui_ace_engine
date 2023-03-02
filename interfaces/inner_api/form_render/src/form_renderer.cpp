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

#include "form_renderer.h"

#include "form_constants.h"
#include "form_renderer_hilog.h"
#include "refbase.h"

namespace OHOS {
namespace Ace {
namespace {
constexpr char FORM_RENDERER_DISPATCHER[] = "ohos.extra.param.key.process_on_form_renderer_dispatcher";
constexpr char ALLOW_UPDATE[] = "allowUpdate";
}
FormRenderer::FormRenderer(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> context,
    const std::shared_ptr<OHOS::AbilityRuntime::Runtime> runtime)
    : context_(context), runtime_(runtime)
{
    HILOG_INFO("FormRenderer %{public}p created.", this);
    if (!context_ || !runtime_) {
        return;
    }
    auto& nativeEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime_.get())).GetNativeEngine();
    uiContent_ = UIContent::Create(context_.get(), &nativeEngine, true);
    formRendererDispatcherImpl_ = std::make_unique<FormRendererDispatcherImpl>(uiContent_);
}

void FormRenderer::InitUiContent()
{
    auto actionEventHandler = [weak = weak_from_this()](const std::string& action) {
        auto formRenderer = weak.lock();
        if (formRenderer) {
            formRenderer->OnActionEvent(action);
        }
    };
    uiContent_->SetActionEventHandler(actionEventHandler);

    auto errorEventHandler = [weak = weak_from_this()](const std::string& code, const std::string& msg) {
        auto formRenderer = weak.lock();
        if (formRenderer) {
            formRenderer->OnError(code, msg);
        }
    };
    uiContent_->SetErrorEventHandler(errorEventHandler);
}

void FormRenderer::AddForm(const OHOS::AAFwk::Want& want, const OHOS::AppExecFwk::FormJsInfo& formJsInfo)
{
    if (uiContent_ == nullptr) {
        HILOG_ERROR("uiContent is null!");
        return;
    }

    SetAllowUpdate(want.GetBoolParam(ALLOW_UPDATE, true));
    auto width = want.GetDoubleParam(OHOS::AppExecFwk::Constants::PARAM_FORM_WIDTH_KEY, 100.0f);
    auto height = want.GetDoubleParam(OHOS::AppExecFwk::Constants::PARAM_FORM_HEIGHT_KEY, 100.0f);
    uiContent_->SetFormWidth(width);
    uiContent_->SetFormHeight(height);
    uiContent_->SetFormModuleName(formJsInfo.moduleName);
    uiContent_->SetFormBundleName(formJsInfo.bundleName);
    uiContent_->Initialize(nullptr, formJsInfo.formSrc, nullptr);
    InitUiContent();

    auto rsSurfaceNode = uiContent_->GetCardRootNode();
    if (rsSurfaceNode == nullptr) {
        return;
    }
    rsSurfaceNode->SetBounds(0.0f, 0.0f, width, height);

    sptr<IRemoteObject> proxy = want.GetRemoteObject("ohos.extra.param.key.process_on_add_surface");
    SetRenderDelegate(proxy);

    OHOS::AAFwk::Want newWant;
    newWant.SetParam(FORM_RENDERER_DISPATCHER, formRendererDispatcherImpl_->AsObject());
    if (formRendererDelegate_ == nullptr) {
        return;
    }
    formRendererDelegate_->OnSurfaceCreate(rsSurfaceNode, formJsInfo, newWant);
    uiContent_->Foreground();
}

bool FormRenderer::IsAllowUpdate()
{
    if (formRendererDispatcherImpl_ == nullptr) {
        HILOG_ERROR("formRendererDispatcherImpl is null");
        return true;
    }

    return formRendererDispatcherImpl_->IsAllowUpdate();
}

void FormRenderer::SetAllowUpdate(bool allowUpdate)
{
    if (formRendererDispatcherImpl_ == nullptr) {
        HILOG_ERROR("formRendererDispatcherImpl is null");
        return;
    }

    formRendererDispatcherImpl_->SetAllowUpdate(allowUpdate);
}

void FormRenderer::UpdateForm(const OHOS::AppExecFwk::FormJsInfo& formJsInfo)
{
    if (!IsAllowUpdate()) {
        HILOG_ERROR("Not allow update");
        return;
    }

    uiContent_->ProcessFormUpdate(formJsInfo.formData);
}

void FormRenderer::Destroy()
{
    HILOG_INFO("FormRenderer %{public}p Destroy start.", this);
}

void FormRenderer::OnActionEvent(const std::string& action)
{
    if (!formRendererDelegate_) {
        HILOG_ERROR("formRendererDelegate is null!");
        return;
    }

    formRendererDelegate_->OnActionEvent(action);
}

void FormRenderer::OnError(const std::string& code, const std::string& msg)
{
    if (!formRendererDelegate_) {
        HILOG_ERROR("formRendererDelegate is null!");
        return;
    }

    formRendererDelegate_->OnError(code, msg);
}

void FormRenderer::SetRenderDelegate(const sptr<IRemoteObject> &remoteObj)
{
    HILOG_INFO("Get renderRemoteObj, add death recipient.");
    auto renderRemoteObj = iface_cast<IFormRendererDelegate>(remoteObj);
    if (renderRemoteObj == nullptr) {
        HILOG_ERROR("renderRemoteObj is nullptr.");
        return;
    }
    if (formRendererDelegate_ == nullptr) {
        formRendererDelegate_ = renderRemoteObj;
    }

    if (renderDelegateDeathRecipient_) {
        return;
    }
    renderDelegateDeathRecipient_ = new FormRenderDelegateRecipient([weak = weak_from_this()]() {
        auto formRender = weak.lock();
        if (!formRender) {
            HILOG_ERROR("formRender is nullptr");
        }
        formRender->ResetRenderDelegate();
    });
    auto renderDelegate = formRendererDelegate_->AsObject();
    if (renderDelegate == nullptr) {
        HILOG_ERROR("renderDelegate is nullptr, can not get obj from renderRemoteObj.");
        return;
    }
    renderDelegate->AddDeathRecipient(renderDelegateDeathRecipient_);
}

void FormRenderer::ResetRenderDelegate()
{
    formRendererDelegate_ = nullptr;
}

void FormRenderDelegateRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_ERROR("Recv FormRenderDelegate death notice");
    if (remote == nullptr) {
        HILOG_ERROR("weak remote is null");
        return;
    }
    if (handler_) {
        handler_();
    }
}
}  // namespace Ace
}  // namespace OHOS

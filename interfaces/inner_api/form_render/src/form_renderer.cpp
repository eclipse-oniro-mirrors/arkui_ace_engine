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
#include "hilog_wrapper.h"
#include "refbase.h"

namespace OHOS {
namespace Ace {
namespace {
constexpr char FORM_RENDERER_DISPATCHER[] = "ohos.extra.param.key.process_on_form_renderer_dispatcher";
}
FormRenderer::FormRenderer(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> context,
    const std::shared_ptr<OHOS::AbilityRuntime::Runtime> runtime)
    : context_(context), runtime_(runtime)
{
    if (!context_ || !runtime_) {
        return;
    }
    auto& nativeEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime_.get())).GetNativeEngine();
    uiContent_ = UIContent::Create(context_.get(), &nativeEngine, true);
    formRendererDispatcherImpl_ = std::make_unique<FormRendererDispatcherImpl>(uiContent_);
}

void FormRenderer::InitUiContent()
{
    auto actionEventHandler = [&](const std::string& action) {
        OnActionEvent(action);
    };
    uiContent_->SetActionEventHandler(actionEventHandler);
}

void FormRenderer::AddForm(const OHOS::AAFwk::Want& want, const OHOS::AppExecFwk::FormJsInfo& formJsInfo)
{
    if (uiContent_ == nullptr) {
        HILOG_ERROR("uiContent is null!");
        return;
    }

    auto width = want.GetDoubleParam(OHOS::AppExecFwk::Constants::PARAM_FORM_WIDTH_KEY, 100.0f);
    auto height = want.GetDoubleParam(OHOS::AppExecFwk::Constants::PARAM_FORM_HEIGHT_KEY, 100.0f);
    uiContent_->SetFormWidth(width);
    uiContent_->SetFormHeight(height);
    uiContent_->Initialize(nullptr, formJsInfo.formSrc, nullptr);
    InitUiContent();

    auto rsSurfaceNode = uiContent_->GetCardRootNode();
    if (rsSurfaceNode == nullptr) {
        return;
    }
    rsSurfaceNode->SetBounds(0.0f, 0.0f, width, height);

    sptr<IRemoteObject> proxy = want.GetRemoteObject("ohos.extra.param.key.process_on_add_surface");
    sptr<IFormRendererDelegate> formRendererDelegate = iface_cast<IFormRendererDelegate>(proxy);
    if (formRendererDelegate == nullptr) {
        return;
    }
    formRendererDelegate_ = formRendererDelegate;

    OHOS::AAFwk::Want newWant;
    newWant.SetParam(FORM_RENDERER_DISPATCHER, formRendererDispatcherImpl_->AsObject());
    formRendererDelegate_->OnSurfaceCreate(rsSurfaceNode, formJsInfo, newWant);
    uiContent_->Foreground();
}

void FormRenderer::UpdateForm(const OHOS::AppExecFwk::FormJsInfo& formJsInfo)
{
    uiContent_->ProcessFormUpdate(formJsInfo.formData);
}

void FormRenderer::Destroy() {}

void FormRenderer::OnActionEvent(const std::string& action)
{
    if (!formRendererDelegate_) {
        HILOG_ERROR("formRendererDelegate is null!");
        return;
    }

    formRendererDelegate_->OnActionEvent(action);
}

void FormRenderer::OnError(const std::string& param)
{
    if (!formRendererDelegate_) {
        HILOG_ERROR("formRendererDelegate is null!");
        return;
    }

    formRendererDelegate_->OnError(param);
}
}  // namespace Ace
}  // namespace OHOS

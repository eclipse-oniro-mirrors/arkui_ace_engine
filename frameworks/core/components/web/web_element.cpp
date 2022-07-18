/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/components/web/web_element.h"

#include "base/log/log.h"
#include "core/components/web/render_web.h"
#include "core/components/web/web_component.h"

namespace OHOS::Ace {

void WebElement::SetNewComponent(const RefPtr<Component>& newComponent)
{
    if (newComponent == nullptr) {
        Element::SetNewComponent(newComponent);
        return;
    }
    auto webComponent = AceType::DynamicCast<WebComponent>(newComponent);
    if (webComponent) {
        if (!webSrc_.empty() && webSrc_ != webComponent->GetSrc()) {
            WebClient::GetInstance().UpdateWebviewUrl(webComponent->GetSrc());
        }
        webSrc_ = webComponent->GetSrc();
        Element::SetNewComponent(webComponent);
    }
}

void WebElement::Update()
{
    RenderElement::Update();
    auto webComponent = AceType::DynamicCast<WebComponent>(component_);
    if (webComponent) {
        webComponent->SetFocusElement(AceType::WeakClaim(this));
    }
}

void WebElement::OnFocus()
{
    LOGI("web element onfocus");
    auto renderWeb = AceType::DynamicCast<RenderWeb>(renderNode_);
    if (!renderWeb) {
        return;
    }
    if (!renderWeb->GetDelegate()) {
        LOGE("Delegate is nullptr.");
        return;
    }
    renderWeb->GetDelegate()->OnFocus();
    FocusNode::OnFocus();
}

void WebElement::OnBlur()
{
    LOGI("web element onblur");
    auto renderWeb = AceType::DynamicCast<RenderWeb>(renderNode_);
    if (!renderWeb) {
        return;
    }
    if (!renderWeb->GetDelegate()) {
        LOGE("Delegate is nullptr.");
        return;
    }
    renderWeb->GetDelegate()->OnBlur();
    FocusNode::OnBlur();
}

bool WebElement::OnKeyEvent(const KeyEvent& keyEvent)
{
    auto renderWeb = AceType::DynamicCast<RenderWeb>(renderNode_);
    if (!renderWeb) {
        return false;
    }
    if (!renderWeb->GetDelegate()) {
        LOGE("Delegate is nullptr.");
        return false;
    }
    return renderWeb->GetDelegate()->OnKeyEvent(static_cast<int32_t>(keyEvent.code),
        static_cast<int32_t>(keyEvent.action));
}
} // namespace OHOS::Ace

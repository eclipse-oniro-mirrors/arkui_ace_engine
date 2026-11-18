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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_MODAL_UI_EXTENSION_PROXY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_MODAL_UI_EXTENSION_PROXY_H

#include "base/memory/ace_type.h"
#include "interfaces/inner_api/ace/modal_ui_extension_proxy.h"

namespace OHOS::Ace::NG {
class UIExtensionProxy;

class ACE_FORCE_EXPORT ModalUIExtensionProxyImpl : public ModalUIExtensionProxy {

public:
    explicit ModalUIExtensionProxyImpl(const RefPtr<UIExtensionProxy>& innerProxy);
    ~ModalUIExtensionProxyImpl() override;
    void SendData(const AAFwk::WantParams& params) override;
    int32_t SendDataSync(const AAFwk::WantParams& params, AAFwk::WantParams& reply);
    const RefPtr<UIExtensionProxy>& GetInnerProxy() const { return innerProxy_; }

private:
    RefPtr<UIExtensionProxy> innerProxy_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_MODAL_UI_EXTENSION_PROXY_H

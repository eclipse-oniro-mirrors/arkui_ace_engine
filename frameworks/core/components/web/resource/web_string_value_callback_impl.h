/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEB_STRING_VALUE_CALLBACK_IMPL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEB_STRING_VALUE_CALLBACK_IMPL_H

#include <functional>
#include <string>

#include "nweb_value_callback.h"
#include "foundation/arkui/ace_engine/frameworks/base/memory/referenced.h"

namespace OHOS::Ace {

class WebDelegate;

class WebStringValueCallbackImpl : public OHOS::NWeb::NWebStringValueCallback {
public:
    WebStringValueCallbackImpl() = default;
    explicit WebStringValueCallbackImpl(const WeakPtr<WebDelegate>& delegate) : webDelegate_(delegate) {}
    virtual ~WebStringValueCallbackImpl() = default;

    virtual void OnReceiveValue(const std::string& value) override;

    void SetCallBack(std::function<void(const std::string&)>&& callback)
    {
        callback_ = std::move(callback);
    }

private:
    WeakPtr<WebDelegate> webDelegate_;
    std::function<void(const std::string&)> callback_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEB_STRING_VALUE_CALLBACK_IMPL_H
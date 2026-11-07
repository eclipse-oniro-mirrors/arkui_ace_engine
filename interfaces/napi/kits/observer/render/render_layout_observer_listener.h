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

#ifndef FOUNDATION_ACE_INTERFACES_OBSERVER_RENDER_LAYOUT_OBSERVER_LISTENER_H
#define FOUNDATION_ACE_INTERFACES_OBSERVER_RENDER_LAYOUT_OBSERVER_LISTENER_H

#include "interfaces/napi/kits/observer/common/napi_observer_callback.h"
#include "core/common/ui_observer/render_layout_store.h"

namespace OHOS::Ace::Napi {

class RenderLayoutObserverListener final : public RenderLayoutListener {
public:
    RenderLayoutObserverListener(napi_env env, napi_value callback) : callback_(env, callback) {}
    ~RenderLayoutObserverListener() override = default;

    void OnDrawOrLayout() override;

    bool NapiEqual(napi_value callback) const
    {
        return callback_.Equals(callback);
    }

private:
    NapiObserverCallback callback_;
};

} // namespace OHOS::Ace::Napi

#endif // FOUNDATION_ACE_INTERFACES_OBSERVER_RENDER_LAYOUT_OBSERVER_LISTENER_H

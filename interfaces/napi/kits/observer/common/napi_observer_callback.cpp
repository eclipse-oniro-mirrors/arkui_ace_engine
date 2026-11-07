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

#include "napi_observer_callback.h"

namespace OHOS::Ace::Napi {

NapiObserverCallback::NapiObserverCallback(napi_env env, napi_value callback) : env_(env)
{
    if (env_ && callback) {
        napi_create_reference(env_, callback, 1, &reference_);
    }
}

NapiObserverCallback::~NapiObserverCallback()
{
    Reset();
}

bool NapiObserverCallback::IsValid() const
{
    return env_ != nullptr && reference_ != nullptr;
}

void NapiObserverCallback::Reset()
{
    if (reference_) {
        napi_delete_reference(env_, reference_);
        reference_ = nullptr;
    }
    env_ = nullptr;
}

napi_value NapiObserverCallback::GetValue() const
{
    napi_value callback = nullptr;
    if (IsValid()) {
        napi_get_reference_value(env_, reference_, &callback);
    }
    return callback;
}

bool NapiObserverCallback::Equals(napi_value callback) const
{
    if (!IsValid() || !callback) {
        return false;
    }
    bool isEquals = false;
    napi_strict_equals(env_, callback, GetValue(), &isEquals);
    return isEquals;
}

} // namespace OHOS::Ace::Napi

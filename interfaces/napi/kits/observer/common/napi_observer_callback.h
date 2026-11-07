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

#ifndef FOUNDATION_ACE_INTERFACES_OBSERVER_NAPI_OBSERVER_CALLBACK_H
#define FOUNDATION_ACE_INTERFACES_OBSERVER_NAPI_OBSERVER_CALLBACK_H

#include <array>

#include "base/utils/napi_scope_raii.h"
#include "napi/native_api.h"

namespace OHOS::Ace::Napi {

// One owned callback reference. All operations, including Reset and destruction,
// run on the env's owner thread before that env is destroyed.
class NapiObserverCallback final {
public:
    NapiObserverCallback(napi_env env, napi_value callback);
    ~NapiObserverCallback();

    NapiObserverCallback(const NapiObserverCallback&) = delete;
    NapiObserverCallback& operator=(const NapiObserverCallback&) = delete;
    NapiObserverCallback(NapiObserverCallback&&) = delete;
    NapiObserverCallback& operator=(NapiObserverCallback&&) = delete;

    bool IsValid() const;
    bool Equals(napi_value callback) const;
    void Reset();

    // Borrowed value for unmigrated callers that already own a handle scope.
    // New business listeners should build their arguments through Invoke instead.
    napi_value GetValue() const;

    template<size_t ArgumentCount, typename ArgumentBuilder>
    napi_status Invoke(ArgumentBuilder&& buildArguments) const
    {
        if (!IsValid()) {
            return napi_invalid_arg;
        }
        ScopeRAII scope(env_);
        if (!scope) {
            return napi_generic_failure;
        }
        napi_value callback = nullptr;
        auto status = napi_get_reference_value(env_, reference_, &callback);
        if (status != napi_ok) {
            return status;
        }
        std::array<napi_value, ArgumentCount> arguments {};
        status = buildArguments(env_, arguments);
        if (status != napi_ok) {
            return status;
        }
        // Preserve the existing receiver and pending-exception behavior.
        return napi_call_function(env_, nullptr, callback, arguments.size(), arguments.data(), nullptr);
    }

private:
    napi_env env_ = nullptr;
    napi_ref reference_ = nullptr;
};

} // namespace OHOS::Ace::Napi

#endif // FOUNDATION_ACE_INTERFACES_OBSERVER_NAPI_OBSERVER_CALLBACK_H

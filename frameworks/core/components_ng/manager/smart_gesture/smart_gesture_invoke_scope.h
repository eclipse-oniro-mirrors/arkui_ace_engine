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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SMART_GESTURE_SMART_GESTURE_INVOKE_SCOPE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SMART_GESTURE_SMART_GESTURE_INVOKE_SCOPE_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace OHOS::Ace::NG {

template <typename MonitorState>
class SmartGestureMonitorInvokeScope;

template <typename MonitorState>
class SmartGestureMonitorRegistryContext {
public:
    using State = std::shared_ptr<MonitorState>;
    using StateList = std::vector<State>;
    using Env = decltype(std::declval<const MonitorState&>().GetEnv());

    // The caller must hold mutex while removing states and deciding whether to defer their release.
    // Grouping by env prevents an unrelated env's invocation from taking over these releases.
    // This does not authorize cross-thread VM access: callers must follow the runtime's contracts.
    // States left in the argument must be detached after unlocking mutex.
    void DeferDetachLocked(State& state)
    {
        if (!state) {
            return;
        }
        auto iter = envStates_.find(state->GetEnv());
        if (iter != envStates_.end() && iter->second->invoking != 0) {
            iter->second->pendingDetaches.emplace_back(std::move(state));
        }
    }

    void DeferDetachLocked(StateList& statesToDetach)
    {
        bool anyDeferred = false;
        for (const auto& state : statesToDetach) {
            auto iter = state ? envStates_.find(state->GetEnv()) : envStates_.end();
            if (iter != envStates_.end() && iter->second->invoking != 0) {
                anyDeferred = true;
                break;
            }
        }
        if (!anyDeferred) {
            return;
        }
        StateList remaining;
        remaining.reserve(statesToDetach.size());
        for (auto& state : statesToDetach) {
            if (!state) {
                continue;
            }
            DeferDetachLocked(state);
            if (state) {
                remaining.emplace_back(std::move(state));
            }
        }
        statesToDetach.swap(remaining);
    }

    // The caller must hold mutex and detach the returned states while env is still valid, following
    // the runtime's cleanup contract. This does not keep an executing VM stack or a local release
    // batch alive across arbitrary teardown.
    StateList CloseEnvLocked(Env env)
    {
        StateList closing;
        auto envIter = envStates_.find(env);
        if (envIter != envStates_.end()) {
            auto& pending = envIter->second->pendingDetaches;
            closing.swap(pending);
            envStates_.erase(envIter);
        }
        for (auto iter = states.begin(); iter != states.end();) {
            auto& stateStack = iter->second;
            auto removeIter = std::remove_if(stateStack.begin(), stateStack.end(),
                [&closing, env](const State& state) {
                    if (!state || state->GetEnv() != env) {
                        return false;
                    }
                    closing.emplace_back(state);
                    return true;
                });
            stateStack.erase(removeIter, stateStack.end());
            if (!stateStack.empty()) {
                ++iter;
                continue;
            }
            iter = states.erase(iter);
        }
        return closing;
    }

    // Protect every access to states and the invocation bookkeeping with this mutex.
    std::mutex mutex;
    std::unordered_map<int32_t, StateList> states;

private:
    friend class SmartGestureMonitorInvokeScope<MonitorState>;

    struct EnvState {
        explicit EnvState(Env env) : env(env) {}
        const Env env;
        int32_t invoking = 0;
        StateList pendingDetaches;
    };

    // Scopes keep this entry directly. After cleanup erases env, an old scope must not decrement
    // a newly created entry at the same address. This ownership protects bookkeeping, not the VM.
    std::shared_ptr<EnvState> GetEnvStateLocked(Env env)
    {
        auto& entry = envStates_[env];
        if (!entry) {
            entry = std::make_shared<EnvState>(env);
        }
        return entry;
    }

    std::unordered_map<Env, std::shared_ptr<EnvState>> envStates_;
};

template <typename MonitorState>
class SmartGestureMonitorInvokeScope final {
    using RegistryContext = SmartGestureMonitorRegistryContext<MonitorState>;

public:
    // The context must outlive the scope; callbacks run only after construction releases its lock.
    SmartGestureMonitorInvokeScope(RegistryContext& context, int32_t instanceId) : context_(context)
    {
        std::lock_guard<std::mutex> lock(context_.mutex);
        auto iter = context_.states.find(instanceId);
        if (iter != context_.states.end()) {
            states_ = iter->second;
        }
        for (const auto& state : states_) {
            if (state) {
                auto entry = context_.GetEnvStateLocked(state->GetEnv());
                if (std::find(envs_.begin(), envs_.end(), entry) == envs_.end()) {
                    envs_.emplace_back(entry);
                }
            }
        }
        for (const auto& entry : envs_) {
            ++entry->invoking;
        }
    }

    SmartGestureMonitorInvokeScope(const SmartGestureMonitorInvokeScope&) = delete;
    SmartGestureMonitorInvokeScope& operator=(const SmartGestureMonitorInvokeScope&) = delete;

    ~SmartGestureMonitorInvokeScope()
    {
        typename RegistryContext::StateList pending;
        {
            std::lock_guard<std::mutex> lock(context_.mutex);
            for (const auto& entry : envs_) {
                if (--entry->invoking != 0) {
                    continue;
                }
                auto& envPending = entry->pendingDetaches;
                pending.insert(pending.end(),
                    std::make_move_iterator(envPending.begin()), std::make_move_iterator(envPending.end()));
                entry->pendingDetaches.clear();
                auto iter = context_.envStates_.find(entry->env);
                // Cleanup may already have replaced this address with another bookkeeping entry.
                if (iter != context_.envStates_.end() && iter->second == entry) {
                    context_.envStates_.erase(iter);
                }
            }
        }
        for (const auto& state : pending) {
            if (state) {
                state->Detach();
            }
        }
    }

    const typename RegistryContext::StateList& GetStates() const
    {
        return states_;
    }

private:
    RegistryContext& context_;
    typename RegistryContext::StateList states_;
    std::vector<std::shared_ptr<typename RegistryContext::EnvState>> envs_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SMART_GESTURE_SMART_GESTURE_INVOKE_SCOPE_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_LISTENER_STORE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_LISTENER_STORE_H

#include <algorithm>
#include <memory>
#include <vector>

namespace OHOS::Ace {

// All operations are confined to the owning instance's JS thread.
template<typename Listener>
class UIObserverListenerStore final {
public:
    using ListenerPtr = std::shared_ptr<Listener>;

    void Register(const ListenerPtr& listener)
    {
        if (listener && std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
            listeners_.emplace_back(listener);
        }
    }

    template<typename Predicate>
    void Unregister(const Predicate& predicate)
    {
        listeners_.erase(std::remove_if(listeners_.begin(), listeners_.end(), predicate), listeners_.end());
    }

    void ClearListeners()
    {
        listeners_.clear();
    }

    std::vector<ListenerPtr> Snapshot() const
    {
        return listeners_;
    }

private:
    std::vector<ListenerPtr> listeners_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_LISTENER_STORE_H

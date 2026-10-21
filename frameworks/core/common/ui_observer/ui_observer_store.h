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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_STORE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_STORE_H

#include <memory>

#include "base/utils/macros.h"

namespace OHOS::Ace {
class RenderLayoutStore;

// One root per Container; business stores are composed, never derived from this root.
class UIObserverStore final {
public:
    UIObserverStore();
    ~UIObserverStore();

    UIObserverStore(const UIObserverStore&) = delete;
    UIObserverStore& operator=(const UIObserverStore&) = delete;

    ACE_FORCE_EXPORT RenderLayoutStore& GetRenderStore();
    void ClearListeners();
    void Close();

    bool IsClosed() const
    {
        return closed_;
    }

private:
    std::unique_ptr<RenderLayoutStore> renderStore_;
    bool closed_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_STORE_H

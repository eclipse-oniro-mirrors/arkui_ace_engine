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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_RENDER_LAYOUT_STORE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_RENDER_LAYOUT_STORE_H

#include <cstdint>

#include "core/common/ui_observer/ui_observer_listener_store.h"

namespace OHOS::Ace {

enum class UIObserverRenderType : uint8_t {
    DRAW,
    LAYOUT,
};

// Domain contract only: JS values and callback identity belong to the NAPI bridge.
class RenderLayoutListener {
public:
    virtual ~RenderLayoutListener() = default;
    virtual void OnDrawOrLayout() = 0;
};

class RenderLayoutStore final {
public:
    using ListenerStore = UIObserverListenerStore<RenderLayoutListener>;

    ListenerStore& GetListeners(UIObserverRenderType type)
    {
        return type == UIObserverRenderType::DRAW ? drawListeners_ : layoutListeners_;
    }

    void ClearListeners()
    {
        drawListeners_.ClearListeners();
        layoutListeners_.ClearListeners();
    }

private:
    ListenerStore drawListeners_;
    ListenerStore layoutListeners_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_UI_OBSERVER_RENDER_LAYOUT_STORE_H

/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_NAVIGATION_TRANSITION_PROXY_PEER_H
#define FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_NAVIGATION_TRANSITION_PROXY_PEER_H

#include <optional>
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/navigation/navigation_transition_proxy.h"

using NavigationTransitionProxyRefPtr = OHOS::Ace::RefPtr<OHOS::Ace::NG::NavigationTransitionProxy>;

struct NavigationTransitionProxyPeer {
    NavigationTransitionProxyRefPtr handler;
    void SetHandler(const NavigationTransitionProxyRefPtr& handlerIn)
    {
        handler = handlerIn;
    }
};

#endif // FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_NAVIGATION_TRANSITION_PROXY_PEER_H

/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier
{
    namespace NavDestinationContextModifier {
        Ark_NativePointer CtorImpl()
        {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl()
        {
            return 0;
        }
        void GetConfigInRouteMapImpl(NavDestinationContextPeer* peer)
        {
        }
        void SetPathInfoImpl(NavDestinationContextPeer* peer, const Ark_NavPathInfo* pathInfo)
        {
        }
        void SetPathStackImpl(NavDestinationContextPeer* peer, const Ark_Materialized* pathStack)
        {
        }
        void GetNavDestinationIdImpl(NavDestinationContextPeer* peer)
        {
        }
        void SetNavDestinationIdImpl(NavDestinationContextPeer* peer, const Ark_String* navDestinationId)
        {
        }
    } // NavDestinationContextModifier
    const GENERATED_ArkUINavDestinationContextAccessor* GetNavDestinationContextAccessor()
    {
        static const GENERATED_ArkUINavDestinationContextAccessor NavDestinationContextAccessorImpl {
            NavDestinationContextModifier::CtorImpl,
            NavDestinationContextModifier::GetFinalizerImpl,
            NavDestinationContextModifier::GetConfigInRouteMapImpl,
            NavDestinationContextModifier::SetPathInfoImpl,
            NavDestinationContextModifier::SetPathStackImpl,
            NavDestinationContextModifier::GetNavDestinationIdImpl,
            NavDestinationContextModifier::SetNavDestinationIdImpl,
        };
        return &NavDestinationContextAccessorImpl;
    }

}

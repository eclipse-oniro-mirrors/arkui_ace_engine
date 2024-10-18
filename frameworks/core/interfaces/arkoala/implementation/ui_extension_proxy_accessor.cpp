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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace UIExtensionProxyAccessor {
Ark_NativePointer CtorImpl()
{
    return 0;
}
Ark_NativePointer GetFinalizerImpl()
{
    return 0;
}
void SendImpl(UIExtensionProxyPeer* peer,
              const Ark_Literal_* data)
{
}
Ark_NativePointer SendSyncImpl(UIExtensionProxyPeer* peer,
                               const Ark_Literal_* data)
{
    return 0;
}
void On0Impl(UIExtensionProxyPeer* peer,
             const Ark_String* type,
             Ark_Function callback)
{
}
void On1Impl(UIExtensionProxyPeer* peer,
             const Ark_String* type,
             Ark_Function callback)
{
}
void Off0Impl(UIExtensionProxyPeer* peer,
              const Ark_String* type,
              const Opt_Function* callback)
{
}
void Off1Impl(UIExtensionProxyPeer* peer,
              const Ark_String* type,
              const Opt_Function* callback)
{
}
} // UIExtensionProxyAccessor
const GENERATED_ArkUIUIExtensionProxyAccessor* GetUIExtensionProxyAccessor()
{
    static const GENERATED_ArkUIUIExtensionProxyAccessor UIExtensionProxyAccessorImpl {
        UIExtensionProxyAccessor::CtorImpl,
        UIExtensionProxyAccessor::GetFinalizerImpl,
        UIExtensionProxyAccessor::SendImpl,
        UIExtensionProxyAccessor::SendSyncImpl,
        UIExtensionProxyAccessor::On0Impl,
        UIExtensionProxyAccessor::On1Impl,
        UIExtensionProxyAccessor::Off0Impl,
        UIExtensionProxyAccessor::Off1Impl,
    };
    return &UIExtensionProxyAccessorImpl;
}

}

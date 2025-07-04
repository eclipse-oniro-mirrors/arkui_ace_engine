/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace UIExtensionProxyAccessor {
void DestroyPeerImpl(Ark_UIExtensionProxy peer)
{
    auto peerImpl = reinterpret_cast<UIExtensionProxyPeerImpl *>(peer);
    if (peerImpl) {
        delete peerImpl;
    }
}
Ark_UIExtensionProxy ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void SendImpl(Ark_UIExtensionProxy peer,
              const Map_String_Object* data)
{
}
Map_String_Object SendSyncImpl(Ark_UIExtensionProxy peer,
                               const Map_String_Object* data)
{
    return {};
}
void OnAsyncReceiverRegisterImpl(Ark_UIExtensionProxy peer,
                                 const Callback_UIExtensionProxy_Void* callback_)
{
}
void OnSyncReceiverRegisterImpl(Ark_UIExtensionProxy peer,
                                const Callback_UIExtensionProxy_Void* callback_)
{
}
void OffAsyncReceiverRegisterImpl(Ark_UIExtensionProxy peer,
                                  const Opt_Callback_UIExtensionProxy_Void* callback_)
{
}
void OffSyncReceiverRegisterImpl(Ark_UIExtensionProxy peer,
                                 const Opt_Callback_UIExtensionProxy_Void* callback_)
{
}
} // UIExtensionProxyAccessor
const GENERATED_ArkUIUIExtensionProxyAccessor* GetUIExtensionProxyAccessor()
{
    static const GENERATED_ArkUIUIExtensionProxyAccessor UIExtensionProxyAccessorImpl {
        UIExtensionProxyAccessor::DestroyPeerImpl,
        UIExtensionProxyAccessor::ConstructImpl,
        UIExtensionProxyAccessor::GetFinalizerImpl,
        UIExtensionProxyAccessor::SendImpl,
        UIExtensionProxyAccessor::SendSyncImpl,
        UIExtensionProxyAccessor::OnAsyncReceiverRegisterImpl,
        UIExtensionProxyAccessor::OnSyncReceiverRegisterImpl,
        UIExtensionProxyAccessor::OffAsyncReceiverRegisterImpl,
        UIExtensionProxyAccessor::OffSyncReceiverRegisterImpl,
    };
    return &UIExtensionProxyAccessorImpl;
}

struct UIExtensionProxyPeer {
    virtual ~UIExtensionProxyPeer() = default;
};
}

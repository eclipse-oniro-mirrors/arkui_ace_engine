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
#include "core/interfaces/native/utility/ace_engine_types.h"
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

struct WebCookiePeer {};

namespace OHOS::Ace::NG::GeneratedModifier {
namespace WebCookieAccessor {
void DestroyPeerImpl(WebCookiePeer* peer)
{
    auto peerImpl = reinterpret_cast<WebCookiePeerImpl *>(peer);
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_NativePointer CtorImpl()
{
    auto peerImpl = Referenced::MakeRefPtr<WebCookiePeerImpl>();
    peerImpl->IncRefCount();
    return reinterpret_cast<WebCookiePeer *>(Referenced::RawPtr(peerImpl));
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void SetCookieImpl(WebCookiePeer* peer)
{
    // deprecated
    LOGE("WebCookieAccessor::SetCookieImpl deprecated method");
}
void SaveCookieImpl(WebCookiePeer* peer)
{
    // deprecated
    LOGE("WebCookieAccessor::SaveCookieImpl deprecated method");
}
} // WebCookieAccessor
const GENERATED_ArkUIWebCookieAccessor* GetWebCookieAccessor()
{
    static const GENERATED_ArkUIWebCookieAccessor WebCookieAccessorImpl {
        WebCookieAccessor::DestroyPeerImpl,
        WebCookieAccessor::CtorImpl,
        WebCookieAccessor::GetFinalizerImpl,
        WebCookieAccessor::SetCookieImpl,
        WebCookieAccessor::SaveCookieImpl,
    };
    return &WebCookieAccessorImpl;
}

}

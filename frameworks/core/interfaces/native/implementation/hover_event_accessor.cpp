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
#include "core/interfaces/native/implementation/hover_event_peer.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace HoverEventAccessor {
void DestroyPeerImpl(HoverEventPeer* peer)
{
    delete peer;
}
Ark_NativePointer CtorImpl()
{
    return new HoverEventPeer();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void SetStopPropagationImpl(HoverEventPeer* peer,
                            const Callback_Void* stopPropagation)
{
    LOGE("HoverEventAccessor::SetStopPropagationImpl wen can only GET stopPropagation callback");
}
} // HoverEventAccessor
const GENERATED_ArkUIHoverEventAccessor* GetHoverEventAccessor()
{
    static const GENERATED_ArkUIHoverEventAccessor HoverEventAccessorImpl {
        HoverEventAccessor::DestroyPeerImpl,
        HoverEventAccessor::CtorImpl,
        HoverEventAccessor::GetFinalizerImpl,
        HoverEventAccessor::SetStopPropagationImpl,
    };
    return &HoverEventAccessorImpl;
}

}

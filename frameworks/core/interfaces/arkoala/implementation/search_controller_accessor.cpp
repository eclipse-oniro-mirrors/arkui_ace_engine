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
#include "search_controller_modifier_peer_impl.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace SearchControllerAccessor {

static void DestroyPeer(SearchControllerPeerImpl *peerImpl)
{
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_NativePointer CtorImpl()
{
    auto peerImpl = Referenced::MakeRefPtr<SearchControllerPeerImpl>();
    peerImpl->IncRefCount();
    return Referenced::RawPtr(peerImpl);
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeer);
}
void CaretPositionImpl(SearchControllerPeer* peer,
                       const Ark_Number* value)
{
    auto peerImpl = reinterpret_cast<SearchControllerPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerCaretPosition(value);
}
void StopEditingImpl(SearchControllerPeer* peer)
{
    auto peerImpl = reinterpret_cast<SearchControllerPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerStopEditing();
}
void SetTextSelectionImpl(SearchControllerPeer* peer,
                          const Ark_Number* selectionStart,
                          const Ark_Number* selectionEnd,
                          const Opt_SelectionOptions* options)
{
    auto peerImpl = reinterpret_cast<SearchControllerPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerSetTextSelection(selectionStart, selectionEnd, options);
}
} // SearchControllerAccessor
const GENERATED_ArkUISearchControllerAccessor* GetSearchControllerAccessor()
{
    static const GENERATED_ArkUISearchControllerAccessor SearchControllerAccessorImpl {
        SearchControllerAccessor::CtorImpl,
        SearchControllerAccessor::GetFinalizerImpl,
        SearchControllerAccessor::CaretPositionImpl,
        SearchControllerAccessor::StopEditingImpl,
        SearchControllerAccessor::SetTextSelectionImpl,
    };
    return &SearchControllerAccessorImpl;
}

}

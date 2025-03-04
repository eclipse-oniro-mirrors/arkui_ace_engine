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
namespace ListScrollerAccessor {
void DestroyPeerImpl(Ark_ListScroller peer)
{
}
Ark_ListScroller CtorImpl()
{
    return nullptr;
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_RectResult GetItemRectInGroupImpl(Ark_VMContext vmContext,
                                      Ark_ListScroller peer,
                                      const Ark_Number* index,
                                      const Ark_Number* indexInGroup)
{
    return {};
}
void ScrollToItemInGroupImpl(Ark_VMContext vmContext,
                             Ark_ListScroller peer,
                             const Ark_Number* index,
                             const Ark_Number* indexInGroup,
                             const Opt_Boolean* smooth,
                             const Opt_ScrollAlign* align)
{
}
void CloseAllSwipeActionsImpl(Ark_VMContext vmContext,
                              Ark_ListScroller peer,
                              const Opt_CloseSwipeActionOptions* options)
{
}
Ark_VisibleListContentInfo GetVisibleListContentInfoImpl(Ark_VMContext vmContext,
                                                         Ark_ListScroller peer,
                                                         const Ark_Number* x,
                                                         const Ark_Number* y)
{
    return {};
}
} // ListScrollerAccessor
const GENERATED_ArkUIListScrollerAccessor* GetListScrollerAccessor()
{
    static const GENERATED_ArkUIListScrollerAccessor ListScrollerAccessorImpl {
        ListScrollerAccessor::DestroyPeerImpl,
        ListScrollerAccessor::CtorImpl,
        ListScrollerAccessor::GetFinalizerImpl,
        ListScrollerAccessor::GetItemRectInGroupImpl,
        ListScrollerAccessor::ScrollToItemInGroupImpl,
        ListScrollerAccessor::CloseAllSwipeActionsImpl,
        ListScrollerAccessor::GetVisibleListContentInfoImpl,
    };
    return &ListScrollerAccessorImpl;
}

}

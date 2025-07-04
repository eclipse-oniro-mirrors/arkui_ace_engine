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
namespace ScrollerAccessor {
void DestroyPeerImpl(Ark_Scroller peer)
{
}
Ark_Scroller ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void ScrollToImpl(Ark_Scroller peer,
                  const Ark_ScrollOptions* options)
{
}
void ScrollEdgeImpl(Ark_Scroller peer,
                    Ark_Edge value,
                    const Opt_ScrollEdgeOptions* options)
{
}
void FlingImpl(Ark_Scroller peer,
               const Ark_Number* velocity)
{
}
void ScrollPageImpl(Ark_Scroller peer,
                    const Ark_ScrollPageOptions* value)
{
}
Ark_OffsetResult CurrentOffsetImpl(Ark_Scroller peer)
{
    return {};
}
void ScrollToIndexImpl(Ark_Scroller peer,
                       const Ark_Number* value,
                       const Opt_Boolean* smooth,
                       const Opt_ScrollAlign* align,
                       const Opt_ScrollToIndexOptions* options)
{
}
void ScrollByImpl(Ark_Scroller peer,
                  const Ark_Length* dx,
                  const Ark_Length* dy)
{
}
Ark_Boolean IsAtEndImpl(Ark_Scroller peer)
{
    return {};
}
Ark_RectResult GetItemRectImpl(Ark_Scroller peer,
                               const Ark_Number* index)
{
    return {};
}
Ark_Number GetItemIndexImpl(Ark_Scroller peer,
                            const Ark_Number* x,
                            const Ark_Number* y)
{
    return {};
}
} // ScrollerAccessor
const GENERATED_ArkUIScrollerAccessor* GetScrollerAccessor()
{
    static const GENERATED_ArkUIScrollerAccessor ScrollerAccessorImpl {
        ScrollerAccessor::DestroyPeerImpl,
        ScrollerAccessor::ConstructImpl,
        ScrollerAccessor::GetFinalizerImpl,
        ScrollerAccessor::ScrollToImpl,
        ScrollerAccessor::ScrollEdgeImpl,
        ScrollerAccessor::FlingImpl,
        ScrollerAccessor::ScrollPageImpl,
        ScrollerAccessor::CurrentOffsetImpl,
        ScrollerAccessor::ScrollToIndexImpl,
        ScrollerAccessor::ScrollByImpl,
        ScrollerAccessor::IsAtEndImpl,
        ScrollerAccessor::GetItemRectImpl,
        ScrollerAccessor::GetItemIndexImpl,
    };
    return &ScrollerAccessorImpl;
}

}

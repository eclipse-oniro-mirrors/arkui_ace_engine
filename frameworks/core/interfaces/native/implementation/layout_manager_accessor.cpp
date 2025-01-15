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
namespace LayoutManagerAccessor {
void DestroyPeerImpl(LayoutManagerPeer* peer)
{
}
Ark_NativePointer CtorImpl()
{
    return new LayoutManagerPeer();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_Int32 GetLineCountImpl(LayoutManagerPeer* peer)
{
    return 0;
}
Ark_NativePointer GetGlyphPositionAtCoordinateImpl(LayoutManagerPeer* peer,
                                                   const Ark_Number* x,
                                                   const Ark_Number* y)
{
    return nullptr;
}
Ark_NativePointer GetLineMetricsImpl(LayoutManagerPeer* peer,
                                     const Ark_Number* lineNumber)
{
    return nullptr;
}
void GetRectsForRangeImpl(LayoutManagerPeer* peer,
                          const Ark_TextRange* range,
                          Ark_RectWidthStyle widthStyle,
                          Ark_RectHeightStyle heightStyle)
{
}
} // LayoutManagerAccessor
const GENERATED_ArkUILayoutManagerAccessor* GetLayoutManagerAccessor()
{
    static const GENERATED_ArkUILayoutManagerAccessor LayoutManagerAccessorImpl {
        LayoutManagerAccessor::DestroyPeerImpl,
        LayoutManagerAccessor::CtorImpl,
        LayoutManagerAccessor::GetFinalizerImpl,
        LayoutManagerAccessor::GetLineCountImpl,
        LayoutManagerAccessor::GetGlyphPositionAtCoordinateImpl,
        LayoutManagerAccessor::GetLineMetricsImpl,
        LayoutManagerAccessor::GetRectsForRangeImpl,
    };
    return &LayoutManagerAccessorImpl;
}

}

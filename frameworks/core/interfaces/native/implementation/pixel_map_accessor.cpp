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
namespace PixelMapAccessor {
void DestroyPeerImpl(PixelMapPeer* peer)
{
}
Ark_NativePointer CtorImpl()
{
    return new PixelMapPeer();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void ReadPixelsToBufferSyncImpl(PixelMapPeer* peer,
                                const Ark_Buffer* dst)
{
}
void WriteBufferToPixelsImpl(PixelMapPeer* peer,
                             const Ark_Buffer* src)
{
}
Ark_Int32 GetIsEditableImpl(PixelMapPeer* peer)
{
    return 0;
}
void SetIsEditableImpl(PixelMapPeer* peer,
                       Ark_Int32 isEditable)
{
}
Ark_Int32 GetIsStrideAlignmentImpl(PixelMapPeer* peer)
{
    return 0;
}
void SetIsStrideAlignmentImpl(PixelMapPeer* peer,
                              Ark_Int32 isStrideAlignment)
{
}
} // PixelMapAccessor
const GENERATED_ArkUIPixelMapAccessor* GetPixelMapAccessor()
{
    static const GENERATED_ArkUIPixelMapAccessor PixelMapAccessorImpl {
        PixelMapAccessor::DestroyPeerImpl,
        PixelMapAccessor::CtorImpl,
        PixelMapAccessor::GetFinalizerImpl,
        PixelMapAccessor::ReadPixelsToBufferSyncImpl,
        PixelMapAccessor::WriteBufferToPixelsImpl,
        PixelMapAccessor::GetIsEditableImpl,
        PixelMapAccessor::SetIsEditableImpl,
        PixelMapAccessor::GetIsStrideAlignmentImpl,
        PixelMapAccessor::SetIsStrideAlignmentImpl,
    };
    return &PixelMapAccessorImpl;
}

}

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
namespace MutableStyledStringAccessor {
void DestroyPeerImpl(Ark_MutableStyledString peer)
{
}
Ark_MutableStyledString CtorImpl()
{
    return nullptr;
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void ReplaceStringImpl(Ark_VMContext vmContext,
                       Ark_MutableStyledString peer,
                       const Ark_Number* start,
                       const Ark_Number* length,
                       const Ark_String* other)
{
}
void InsertStringImpl(Ark_VMContext vmContext,
                      Ark_MutableStyledString peer,
                      const Ark_Number* start,
                      const Ark_String* other)
{
}
void RemoveStringImpl(Ark_VMContext vmContext,
                      Ark_MutableStyledString peer,
                      const Ark_Number* start,
                      const Ark_Number* length)
{
}
void ReplaceStyleImpl(Ark_VMContext vmContext,
                      Ark_MutableStyledString peer,
                      const Ark_SpanStyle* spanStyle)
{
}
void SetStyleImpl(Ark_VMContext vmContext,
                  Ark_MutableStyledString peer,
                  const Ark_SpanStyle* spanStyle)
{
}
void RemoveStyleImpl(Ark_VMContext vmContext,
                     Ark_MutableStyledString peer,
                     const Ark_Number* start,
                     const Ark_Number* length,
                     Ark_StyledStringKey styledKey)
{
}
void RemoveStylesImpl(Ark_VMContext vmContext,
                      Ark_MutableStyledString peer,
                      const Ark_Number* start,
                      const Ark_Number* length)
{
}
void ClearStylesImpl(Ark_MutableStyledString peer)
{
}
void ReplaceStyledStringImpl(Ark_VMContext vmContext,
                             Ark_MutableStyledString peer,
                             const Ark_Number* start,
                             const Ark_Number* length,
                             Ark_StyledString other)
{
}
void InsertStyledStringImpl(Ark_VMContext vmContext,
                            Ark_MutableStyledString peer,
                            const Ark_Number* start,
                            Ark_StyledString other)
{
}
void AppendStyledStringImpl(Ark_MutableStyledString peer,
                            Ark_StyledString other)
{
}
} // MutableStyledStringAccessor
const GENERATED_ArkUIMutableStyledStringAccessor* GetMutableStyledStringAccessor()
{
    static const GENERATED_ArkUIMutableStyledStringAccessor MutableStyledStringAccessorImpl {
        MutableStyledStringAccessor::DestroyPeerImpl,
        MutableStyledStringAccessor::CtorImpl,
        MutableStyledStringAccessor::GetFinalizerImpl,
        MutableStyledStringAccessor::ReplaceStringImpl,
        MutableStyledStringAccessor::InsertStringImpl,
        MutableStyledStringAccessor::RemoveStringImpl,
        MutableStyledStringAccessor::ReplaceStyleImpl,
        MutableStyledStringAccessor::SetStyleImpl,
        MutableStyledStringAccessor::RemoveStyleImpl,
        MutableStyledStringAccessor::RemoveStylesImpl,
        MutableStyledStringAccessor::ClearStylesImpl,
        MutableStyledStringAccessor::ReplaceStyledStringImpl,
        MutableStyledStringAccessor::InsertStyledStringImpl,
        MutableStyledStringAccessor::AppendStyledStringImpl,
    };
    return &MutableStyledStringAccessorImpl;
}

}

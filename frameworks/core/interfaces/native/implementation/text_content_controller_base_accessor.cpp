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
namespace TextContentControllerBaseAccessor {
void DestroyPeerImpl(Ark_TextContentControllerBase peer)
{
}
Ark_TextContentControllerBase ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_CaretOffset GetCaretOffsetImpl(Ark_TextContentControllerBase peer)
{
    return {};
}
Ark_RectResult GetTextContentRectImpl(Ark_TextContentControllerBase peer)
{
    return {};
}
Ark_Number GetTextContentLineCountImpl(Ark_TextContentControllerBase peer)
{
    return {};
}
Ark_Number AddTextImpl(Ark_TextContentControllerBase peer,
                       const Ark_String* text,
                       const Opt_TextContentControllerOptions* textOperationOptions)
{
    return {};
}
void DeleteTextImpl(Ark_TextContentControllerBase peer,
                    const Opt_TextRange* range)
{
}
Ark_TextRange GetSelectionImpl(Ark_TextContentControllerBase peer)
{
    return {};
}
void ClearPreviewTextImpl(Ark_TextContentControllerBase peer)
{
}
Ark_String GetTextImpl(Ark_TextContentControllerBase peer,
                       const Opt_TextRange* range)
{
    return {};
}
} // TextContentControllerBaseAccessor
const GENERATED_ArkUITextContentControllerBaseAccessor* GetTextContentControllerBaseAccessor()
{
    static const GENERATED_ArkUITextContentControllerBaseAccessor TextContentControllerBaseAccessorImpl {
        TextContentControllerBaseAccessor::DestroyPeerImpl,
        TextContentControllerBaseAccessor::ConstructImpl,
        TextContentControllerBaseAccessor::GetFinalizerImpl,
        TextContentControllerBaseAccessor::GetCaretOffsetImpl,
        TextContentControllerBaseAccessor::GetTextContentRectImpl,
        TextContentControllerBaseAccessor::GetTextContentLineCountImpl,
        TextContentControllerBaseAccessor::AddTextImpl,
        TextContentControllerBaseAccessor::DeleteTextImpl,
        TextContentControllerBaseAccessor::GetSelectionImpl,
        TextContentControllerBaseAccessor::ClearPreviewTextImpl,
        TextContentControllerBaseAccessor::GetTextImpl,
    };
    return &TextContentControllerBaseAccessorImpl;
}

}

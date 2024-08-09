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

#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RichEditorStyledStringControllerModifier {
Ark_NativePointer CtorImpl()
{
    return 0;
}
Ark_NativePointer GetFinalizerImpl()
{
    return 0;
}
void SetStyledStringImpl(RichEditorStyledStringControllerPeer* peer, const Ark_Materialized* styledString)
{
}
Ark_NativePointer GetStyledStringImpl(RichEditorStyledStringControllerPeer* peer)
{
    return 0;
}
Ark_NativePointer GetSelectionImpl(RichEditorStyledStringControllerPeer* peer)
{
    return 0;
}
void OnContentChangedImpl(RichEditorStyledStringControllerPeer* peer, const Ark_StyledStringChangedListener* listener)
{
}
} // RichEditorStyledStringControllerModifier
const GENERATED_ArkUIRichEditorStyledStringControllerAccessor* GetRichEditorStyledStringControllerAccessor()
{
    static const GENERATED_ArkUIRichEditorStyledStringControllerAccessor RichEditorStyledStringControllerAccessorImpl {
        RichEditorStyledStringControllerModifier::CtorImpl,
        RichEditorStyledStringControllerModifier::GetFinalizerImpl,
        RichEditorStyledStringControllerModifier::SetStyledStringImpl,
        RichEditorStyledStringControllerModifier::GetStyledStringImpl,
        RichEditorStyledStringControllerModifier::GetSelectionImpl,
        RichEditorStyledStringControllerModifier::OnContentChangedImpl,
    };
    return &RichEditorStyledStringControllerAccessorImpl;
}

}

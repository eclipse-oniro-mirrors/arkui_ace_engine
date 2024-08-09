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
namespace PasteButtonInterfaceModifier {
void _setPasteButtonOptions0Impl(Ark_NativePointer node)
{
}
void _setPasteButtonOptions1Impl(Ark_NativePointer node, const Ark_PasteButtonOptions* options)
{
}
} // PasteButtonInterfaceModifier
namespace PasteButtonAttributeModifier {
void OnClickImpl(Ark_NativePointer node, Ark_Function event)
{
}
} // PasteButtonAttributeModifier
const GENERATED_ArkUIPasteButtonModifier* GetPasteButtonModifier()
{
    static const GENERATED_ArkUIPasteButtonModifier ArkUIPasteButtonModifierImpl {
        PasteButtonInterfaceModifier::_setPasteButtonOptions0Impl,
        PasteButtonInterfaceModifier::_setPasteButtonOptions1Impl,
        PasteButtonAttributeModifier::OnClickImpl,
    };
    return &ArkUIPasteButtonModifierImpl;
}

}

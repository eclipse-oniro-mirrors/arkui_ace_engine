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
namespace HyperlinkModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // HyperlinkModifier
namespace HyperlinkInterfaceModifier {
void SetHyperlinkOptionsImpl(Ark_NativePointer node,
                             const Ark_Union_String_Resource* address,
                             const Opt_Union_String_Resource* content)
{
}
} // HyperlinkInterfaceModifier
namespace HyperlinkAttributeModifier {
void ColorImpl(Ark_NativePointer node,
               const Opt_Union_Color_Number_String_Resource* value)
{
}
} // HyperlinkAttributeModifier
const GENERATED_ArkUIHyperlinkModifier* GetHyperlinkModifier()
{
    static const GENERATED_ArkUIHyperlinkModifier ArkUIHyperlinkModifierImpl {
        HyperlinkModifier::ConstructImpl,
        HyperlinkInterfaceModifier::SetHyperlinkOptionsImpl,
        HyperlinkAttributeModifier::ColorImpl,
    };
    return &ArkUIHyperlinkModifierImpl;
}

}

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
namespace NavRouterInterfaceModifier {
void SetNavRouterOptions0Impl(Ark_NativePointer node)
{
}
void SetNavRouterOptions1Impl(Ark_NativePointer node,
                              const Ark_RouteInfo* value)
{
}
} // NavRouterInterfaceModifier
namespace NavRouterAttributeModifier {
void OnStateChangeImpl(Ark_NativePointer node,
                       Ark_Function callback)
{
}
void ModeImpl(Ark_NativePointer node,
              Ark_Int32 mode)
{
}
} // NavRouterAttributeModifier
const GENERATED_ArkUINavRouterModifier* GetNavRouterModifier()
{
    static const GENERATED_ArkUINavRouterModifier ArkUINavRouterModifierImpl {
        NavRouterInterfaceModifier::SetNavRouterOptions0Impl,
        NavRouterInterfaceModifier::SetNavRouterOptions1Impl,
        NavRouterAttributeModifier::OnStateChangeImpl,
        NavRouterAttributeModifier::ModeImpl,
    };
    return &ArkUINavRouterModifierImpl;
}

}

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
namespace RefreshInterfaceModifier {
void SetRefreshOptionsImpl(Ark_NativePointer node,
                           const Ark_RefreshOptions* value)
{
}
} // RefreshInterfaceModifier
namespace RefreshAttributeModifier {
void OnStateChangeImpl(Ark_NativePointer node,
                       Ark_Function callback)
{
}
void OnRefreshingImpl(Ark_NativePointer node,
                      Ark_Function callback)
{
}
void RefreshOffsetImpl(Ark_NativePointer node,
                       const Ark_Number* value)
{
}
void PullToRefreshImpl(Ark_NativePointer node,
                       Ark_Boolean value)
{
}
void OnOffsetChangeImpl(Ark_NativePointer node,
                        Ark_Function callback)
{
}
void PullDownRatioImpl(Ark_NativePointer node,
                       const Opt_Number* ratio)
{
}
} // RefreshAttributeModifier
const GENERATED_ArkUIRefreshModifier* GetRefreshModifier()
{
    static const GENERATED_ArkUIRefreshModifier ArkUIRefreshModifierImpl {
        RefreshInterfaceModifier::SetRefreshOptionsImpl,
        RefreshAttributeModifier::OnStateChangeImpl,
        RefreshAttributeModifier::OnRefreshingImpl,
        RefreshAttributeModifier::RefreshOffsetImpl,
        RefreshAttributeModifier::PullToRefreshImpl,
        RefreshAttributeModifier::OnOffsetChangeImpl,
        RefreshAttributeModifier::PullDownRatioImpl,
    };
    return &ArkUIRefreshModifierImpl;
}

}
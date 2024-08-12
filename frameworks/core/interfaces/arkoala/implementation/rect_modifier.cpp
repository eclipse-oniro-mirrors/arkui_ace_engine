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
namespace RectInterfaceModifier {
void _setRectOptionsImpl(Ark_NativePointer node,
                         const Opt_Type_RectInterface__setRectOptions_Arg0* value)
{
}
} // RectInterfaceModifier
namespace RectAttributeModifier {
void RadiusWidthImpl(Ark_NativePointer node,
                     const Type_RectAttribute_radiusWidth_Arg0* value)
{
}
void RadiusHeightImpl(Ark_NativePointer node,
                      const Type_RectAttribute_radiusHeight_Arg0* value)
{
}
void RadiusImpl(Ark_NativePointer node,
                const Type_RectAttribute_radius_Arg0* value)
{
}
} // RectAttributeModifier
const GENERATED_ArkUIRectModifier* GetRectModifier()
{
    static const GENERATED_ArkUIRectModifier ArkUIRectModifierImpl {
        RectInterfaceModifier::_setRectOptionsImpl,
        RectAttributeModifier::RadiusWidthImpl,
        RectAttributeModifier::RadiusHeightImpl,
        RectAttributeModifier::RadiusImpl,
    };
    return &ArkUIRectModifierImpl;
}

}

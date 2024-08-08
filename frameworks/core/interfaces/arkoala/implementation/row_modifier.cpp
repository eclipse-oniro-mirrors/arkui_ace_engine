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
    namespace RowInterfaceModifier {
        void _setRowOptionsImpl(Ark_NativePointer node, const Opt_Type_RowInterface__setRowOptions_Arg0* value)
        {
        }
    } // RowInterfaceModifier
    namespace RowAttributeModifier {
        void AlignItemsImpl(Ark_NativePointer node, Ark_Int32 value)
        {
        }
        void JustifyContentImpl(Ark_NativePointer node, Ark_Int32 value)
        {
        }
        void PointLightImpl(Ark_NativePointer node, const Ark_PointLightStyle* value)
        {
        }
    } // RowAttributeModifier
    const GENERATED_ArkUIRowModifier* GetRowModifier()
    {
        static const GENERATED_ArkUIRowModifier ArkUIRowModifierImpl {
            RowInterfaceModifier::_setRowOptionsImpl,
            RowAttributeModifier::AlignItemsImpl,
            RowAttributeModifier::JustifyContentImpl,
            RowAttributeModifier::PointLightImpl,
        };
        return &ArkUIRowModifierImpl;
    }

}

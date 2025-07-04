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
namespace TextMenuItemIdAccessor {
void DestroyPeerImpl(Ark_TextMenuItemId peer)
{
}
Ark_TextMenuItemId ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_TextMenuItemId OfImpl(const Ark_ResourceStr* id)
{
    return {};
}
Ark_Boolean EqualsImpl(Ark_TextMenuItemId peer,
                       Ark_TextMenuItemId id)
{
    return {};
}
Ark_TextMenuItemId GetCUTImpl()
{
    return {};
}
Ark_TextMenuItemId GetCOPYImpl()
{
    return {};
}
Ark_TextMenuItemId GetPASTEImpl()
{
    return {};
}
Ark_TextMenuItemId GetSELECT_ALLImpl()
{
    return {};
}
Ark_TextMenuItemId GetCOLLABORATION_SERVICEImpl()
{
    return {};
}
Ark_TextMenuItemId GetCAMERA_INPUTImpl()
{
    return {};
}
Ark_TextMenuItemId GetAI_WRITERImpl()
{
    return {};
}
Ark_TextMenuItemId GetTRANSLATEImpl()
{
    return {};
}
Ark_TextMenuItemId GetSEARCHImpl()
{
    return {};
}
Ark_TextMenuItemId GetSHAREImpl()
{
    return {};
}
} // TextMenuItemIdAccessor
const GENERATED_ArkUITextMenuItemIdAccessor* GetTextMenuItemIdAccessor()
{
    static const GENERATED_ArkUITextMenuItemIdAccessor TextMenuItemIdAccessorImpl {
        TextMenuItemIdAccessor::DestroyPeerImpl,
        TextMenuItemIdAccessor::ConstructImpl,
        TextMenuItemIdAccessor::GetFinalizerImpl,
        TextMenuItemIdAccessor::OfImpl,
        TextMenuItemIdAccessor::EqualsImpl,
        TextMenuItemIdAccessor::GetCUTImpl,
        TextMenuItemIdAccessor::GetCOPYImpl,
        TextMenuItemIdAccessor::GetPASTEImpl,
        TextMenuItemIdAccessor::GetSELECT_ALLImpl,
        TextMenuItemIdAccessor::GetCOLLABORATION_SERVICEImpl,
        TextMenuItemIdAccessor::GetCAMERA_INPUTImpl,
        TextMenuItemIdAccessor::GetAI_WRITERImpl,
        TextMenuItemIdAccessor::GetTRANSLATEImpl,
        TextMenuItemIdAccessor::GetSEARCHImpl,
        TextMenuItemIdAccessor::GetSHAREImpl,
    };
    return &TextMenuItemIdAccessorImpl;
}

}

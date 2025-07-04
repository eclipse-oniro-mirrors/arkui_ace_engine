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
namespace EventTargetInfoAccessor {
void DestroyPeerImpl(Ark_EventTargetInfo peer)
{
}
Ark_EventTargetInfo ConstructImpl()
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_String GetIdImpl(Ark_EventTargetInfo peer)
{
    return {};
}
Ark_Boolean IsScrollableComponentImpl(Ark_EventTargetInfo peer)
{
    auto defaultResult = Converter::ArkValue<Ark_Boolean>(true);
    CHECK_NULL_RETURN(peer, defaultResult);
    return Converter::ArkValue<Ark_Boolean>(peer->isScrollableComponent_);
}
} // EventTargetInfoAccessor
const GENERATED_ArkUIEventTargetInfoAccessor* GetEventTargetInfoAccessor()
{
    static const GENERATED_ArkUIEventTargetInfoAccessor EventTargetInfoAccessorImpl {
        EventTargetInfoAccessor::DestroyPeerImpl,
        EventTargetInfoAccessor::ConstructImpl,
        EventTargetInfoAccessor::GetFinalizerImpl,
        EventTargetInfoAccessor::GetIdImpl,
        EventTargetInfoAccessor::IsScrollableComponentImpl,
    };
    return &EventTargetInfoAccessorImpl;
}

}

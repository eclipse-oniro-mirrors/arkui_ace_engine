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
namespace TransitionEffectAccessor {
void DestroyPeerImpl(TransitionEffectPeer* peer)
{
}
Ark_NativePointer CtorImpl(const Ark_String* type,
                           const Ark_TransitionEffects* effect)
{
    return new TransitionEffectPeer(type, effect);
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_NativePointer TranslateImpl(const Ark_TranslateOptions* options)
{
    return nullptr;
}
Ark_NativePointer RotateImpl(const Ark_RotateOptions* options)
{
    return nullptr;
}
Ark_NativePointer ScaleImpl(const Ark_ScaleOptions* options)
{
    return nullptr;
}
Ark_NativePointer OpacityImpl(const Ark_Number* alpha)
{
    return nullptr;
}
Ark_NativePointer MoveImpl(Ark_TransitionEdge edge)
{
    return nullptr;
}
Ark_NativePointer AsymmetricImpl(const Ark_TransitionEffect* appear,
                                 const Ark_TransitionEffect* disappear)
{
    return nullptr;
}
Ark_NativePointer AnimationImpl(TransitionEffectPeer* peer,
                                const Ark_AnimateParam* value)
{
    return nullptr;
}
Ark_NativePointer CombineImpl(TransitionEffectPeer* peer,
                              const Ark_TransitionEffect* transitionEffect)
{
    return nullptr;
}
} // TransitionEffectAccessor
const GENERATED_ArkUITransitionEffectAccessor* GetTransitionEffectAccessor()
{
    static const GENERATED_ArkUITransitionEffectAccessor TransitionEffectAccessorImpl {
        TransitionEffectAccessor::DestroyPeerImpl,
        TransitionEffectAccessor::CtorImpl,
        TransitionEffectAccessor::GetFinalizerImpl,
        TransitionEffectAccessor::TranslateImpl,
        TransitionEffectAccessor::RotateImpl,
        TransitionEffectAccessor::ScaleImpl,
        TransitionEffectAccessor::OpacityImpl,
        TransitionEffectAccessor::MoveImpl,
        TransitionEffectAccessor::AsymmetricImpl,
        TransitionEffectAccessor::AnimationImpl,
        TransitionEffectAccessor::CombineImpl,
    };
    return &TransitionEffectAccessorImpl;
}

}

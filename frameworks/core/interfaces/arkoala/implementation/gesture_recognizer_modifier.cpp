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
    namespace GestureRecognizerModifier {
        Ark_NativePointer CtorImpl() {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl() {
            return 0;
        }
        void GetTagImpl(GestureRecognizerPeer* peer) {
        }
        Ark_NativePointer GetTypeImpl(GestureRecognizerPeer* peer) {
            return 0;
        }
        Ark_Boolean IsBuiltInImpl(GestureRecognizerPeer* peer) {
            return 0;
        }
        void SetEnabledImpl(GestureRecognizerPeer* peer, Ark_Boolean isEnabled) {
        }
        Ark_Boolean IsEnabledImpl(GestureRecognizerPeer* peer) {
            return 0;
        }
        Ark_NativePointer GetStateImpl(GestureRecognizerPeer* peer) {
            return 0;
        }
        Ark_NativePointer GetEventTargetInfoImpl(GestureRecognizerPeer* peer) {
            return 0;
        }
    } // GestureRecognizerModifier
    const GENERATED_ArkUIGestureRecognizerAccessor* GetGestureRecognizerAccessor() {
        static const GENERATED_ArkUIGestureRecognizerAccessor GestureRecognizerAccessorImpl {
            GestureRecognizerModifier::CtorImpl,
            GestureRecognizerModifier::GetFinalizerImpl,
            GestureRecognizerModifier::GetTagImpl,
            GestureRecognizerModifier::GetTypeImpl,
            GestureRecognizerModifier::IsBuiltInImpl,
            GestureRecognizerModifier::SetEnabledImpl,
            GestureRecognizerModifier::IsEnabledImpl,
            GestureRecognizerModifier::GetStateImpl,
            GestureRecognizerModifier::GetEventTargetInfoImpl,
        };
        return &GestureRecognizerAccessorImpl;
    }

}

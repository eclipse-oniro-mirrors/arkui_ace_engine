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
    namespace PermissionRequestModifier {
        Ark_NativePointer CtorImpl() {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl() {
            return 0;
        }
        void DenyImpl(PermissionRequestPeer* peer) {
        }
        void GetOriginImpl(PermissionRequestPeer* peer) {
        }
        Ark_NativePointer GetAccessibleResourceImpl(PermissionRequestPeer* peer) {
            return 0;
        }
        void GrantImpl(PermissionRequestPeer* peer, const Array_String* resources) {
        }
    } // PermissionRequestModifier
    const GENERATED_ArkUIPermissionRequestAccessor* GetPermissionRequestAccessor() {
        static const GENERATED_ArkUIPermissionRequestAccessor PermissionRequestAccessorImpl {
            PermissionRequestModifier::CtorImpl,
            PermissionRequestModifier::GetFinalizerImpl,
            PermissionRequestModifier::DenyImpl,
            PermissionRequestModifier::GetOriginImpl,
            PermissionRequestModifier::GetAccessibleResourceImpl,
            PermissionRequestModifier::GrantImpl,
        };
        return &PermissionRequestAccessorImpl;
    }

}

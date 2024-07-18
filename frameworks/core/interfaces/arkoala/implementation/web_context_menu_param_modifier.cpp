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
    namespace WebContextMenuParamModifier {
        Ark_NativePointer CtorImpl() {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl() {
            return 0;
        }
        Ark_Int32 XImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        Ark_Int32 YImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        void GetLinkUrlImpl(WebContextMenuParamPeer* peer) {
        }
        void GetUnfilteredLinkUrlImpl(WebContextMenuParamPeer* peer) {
        }
        void GetSourceUrlImpl(WebContextMenuParamPeer* peer) {
        }
        Ark_Boolean ExistsImageContentsImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        Ark_NativePointer GetMediaTypeImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        void GetSelectionTextImpl(WebContextMenuParamPeer* peer) {
        }
        Ark_NativePointer GetSourceTypeImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        Ark_NativePointer GetInputFieldTypeImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        Ark_Boolean IsEditableImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
        Ark_Int32 GetEditStateFlagsImpl(WebContextMenuParamPeer* peer) {
            return 0;
        }
    } // WebContextMenuParamModifier
    const GENERATED_ArkUIWebContextMenuParamAccessor* GetWebContextMenuParamAccessor() {
        static const GENERATED_ArkUIWebContextMenuParamAccessor WebContextMenuParamAccessorImpl {
            WebContextMenuParamModifier::CtorImpl,
            WebContextMenuParamModifier::GetFinalizerImpl,
            WebContextMenuParamModifier::XImpl,
            WebContextMenuParamModifier::YImpl,
            WebContextMenuParamModifier::GetLinkUrlImpl,
            WebContextMenuParamModifier::GetUnfilteredLinkUrlImpl,
            WebContextMenuParamModifier::GetSourceUrlImpl,
            WebContextMenuParamModifier::ExistsImageContentsImpl,
            WebContextMenuParamModifier::GetMediaTypeImpl,
            WebContextMenuParamModifier::GetSelectionTextImpl,
            WebContextMenuParamModifier::GetSourceTypeImpl,
            WebContextMenuParamModifier::GetInputFieldTypeImpl,
            WebContextMenuParamModifier::IsEditableImpl,
            WebContextMenuParamModifier::GetEditStateFlagsImpl,
        };
        return &WebContextMenuParamAccessorImpl;
    }

}

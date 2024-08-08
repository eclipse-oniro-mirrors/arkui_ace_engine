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
    namespace ChildrenMainSizeModifier {
        Ark_NativePointer CtorImpl(const Ark_Number* childDefaultSize)
        {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl()
        {
            return 0;
        }
        void SpliceImpl(ChildrenMainSizePeer* peer, const Ark_Number* start, const Opt_Number* deleteCount, const Opt_Array_Number* childrenSize)
        {
        }
        void UpdateImpl(ChildrenMainSizePeer* peer, const Ark_Number* index, const Ark_Number* childSize)
        {
        }
    } // ChildrenMainSizeModifier
    const GENERATED_ArkUIChildrenMainSizeAccessor* GetChildrenMainSizeAccessor()
    {
        static const GENERATED_ArkUIChildrenMainSizeAccessor ChildrenMainSizeAccessorImpl {
            ChildrenMainSizeModifier::CtorImpl,
            ChildrenMainSizeModifier::GetFinalizerImpl,
            ChildrenMainSizeModifier::SpliceImpl,
            ChildrenMainSizeModifier::UpdateImpl,
        };
        return &ChildrenMainSizeAccessorImpl;
    }

}

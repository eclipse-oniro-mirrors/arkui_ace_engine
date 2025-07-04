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
namespace Matrix2DAccessor {
void DestroyPeerImpl(Ark_Matrix2D peer)
{
    auto peerImpl = reinterpret_cast<Matrix2DPeerImpl *>(peer);
    if (peerImpl) {
        delete peerImpl;
    }
}
Ark_Matrix2D Construct0Impl()
{
    return {};
}
Ark_Matrix2D Construct1Impl(Ark_LengthMetricsUnit unit)
{
    return {};
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_Matrix2D IdentityImpl(Ark_Matrix2D peer)
{
    return {};
}
Ark_Matrix2D InvertImpl(Ark_Matrix2D peer)
{
    return {};
}
Ark_Matrix2D RotateImpl(Ark_Matrix2D peer,
                        const Ark_Number* degree,
                        const Opt_Number* rx,
                        const Opt_Number* ry)
{
    return {};
}
Ark_Matrix2D TranslateImpl(Ark_Matrix2D peer,
                           const Opt_Number* tx,
                           const Opt_Number* ty)
{
    return {};
}
Ark_Matrix2D ScaleImpl(Ark_Matrix2D peer,
                       const Opt_Number* sx,
                       const Opt_Number* sy)
{
    return {};
}
Opt_Number GetScaleXImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetScaleXImpl(Ark_Matrix2D peer,
                   const Opt_Number* scaleX)
{
}
Opt_Number GetRotateYImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetRotateYImpl(Ark_Matrix2D peer,
                    const Opt_Number* rotateY)
{
}
Opt_Number GetRotateXImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetRotateXImpl(Ark_Matrix2D peer,
                    const Opt_Number* rotateX)
{
}
Opt_Number GetScaleYImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetScaleYImpl(Ark_Matrix2D peer,
                   const Opt_Number* scaleY)
{
}
Opt_Number GetTranslateXImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetTranslateXImpl(Ark_Matrix2D peer,
                       const Opt_Number* translateX)
{
}
Opt_Number GetTranslateYImpl(Ark_Matrix2D peer)
{
    return {};
}
void SetTranslateYImpl(Ark_Matrix2D peer,
                       const Opt_Number* translateY)
{
}
} // Matrix2DAccessor
const GENERATED_ArkUIMatrix2DAccessor* GetMatrix2DAccessor()
{
    static const GENERATED_ArkUIMatrix2DAccessor Matrix2DAccessorImpl {
        Matrix2DAccessor::DestroyPeerImpl,
        Matrix2DAccessor::Construct0Impl,
        Matrix2DAccessor::Construct1Impl,
        Matrix2DAccessor::GetFinalizerImpl,
        Matrix2DAccessor::IdentityImpl,
        Matrix2DAccessor::InvertImpl,
        Matrix2DAccessor::RotateImpl,
        Matrix2DAccessor::TranslateImpl,
        Matrix2DAccessor::ScaleImpl,
        Matrix2DAccessor::GetScaleXImpl,
        Matrix2DAccessor::SetScaleXImpl,
        Matrix2DAccessor::GetRotateYImpl,
        Matrix2DAccessor::SetRotateYImpl,
        Matrix2DAccessor::GetRotateXImpl,
        Matrix2DAccessor::SetRotateXImpl,
        Matrix2DAccessor::GetScaleYImpl,
        Matrix2DAccessor::SetScaleYImpl,
        Matrix2DAccessor::GetTranslateXImpl,
        Matrix2DAccessor::SetTranslateXImpl,
        Matrix2DAccessor::GetTranslateYImpl,
        Matrix2DAccessor::SetTranslateYImpl,
    };
    return &Matrix2DAccessorImpl;
}

struct Matrix2DPeer {
    virtual ~Matrix2DPeer() = default;
};
}

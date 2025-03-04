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

#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

#include "matrix2d_peer.h"
#include "path2d_accessor_peer_impl.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace Path2DAccessor {
void DestroyPeerImpl(Ark_Path2D peer)
{
    auto peerImpl = reinterpret_cast<Path2DPeerImpl*>(peer);
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_Path2D CtorImpl()
{
    auto peerImpl = Referenced::MakeRefPtr<Path2DPeerImpl>();
    peerImpl->IncRefCount();
    return reinterpret_cast<Path2DPeer*>(Referenced::RawPtr(peerImpl));
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void AddPathImpl(Ark_Path2D peer,
                 Ark_Path2D path,
                 const Opt_Matrix2D* transform)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(path);
    auto peerImpl = reinterpret_cast<Path2DPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(peerImpl->path);
    auto peerImpl2 = reinterpret_cast<Path2DPeerImpl*>(path);
    CHECK_NULL_VOID(peerImpl2);
    CHECK_NULL_VOID(peerImpl2->path);
    peerImpl->path->AddPath(peerImpl2->path);
    CHECK_NULL_VOID(transform);
    auto opt = Converter::OptConvert<Ark_Matrix2D>(*transform);
    CHECK_NULL_VOID(opt);
    auto matrix = opt.value();
    CHECK_NULL_VOID(matrix);
    auto tr = matrix->transform;
    peerImpl->path->SetTransform(tr.scaleX, tr.skewX, tr.skewY, tr.scaleY, tr.translateX, tr.translateY);
}
} // Path2DAccessor
const GENERATED_ArkUIPath2DAccessor* GetPath2DAccessor()
{
    static const GENERATED_ArkUIPath2DAccessor Path2DAccessorImpl {
        Path2DAccessor::DestroyPeerImpl,
        Path2DAccessor::CtorImpl,
        Path2DAccessor::GetFinalizerImpl,
        Path2DAccessor::AddPathImpl,
    };
    return &Path2DAccessorImpl;
}
}

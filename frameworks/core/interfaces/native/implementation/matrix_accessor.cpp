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
namespace Drawing_MatrixAccessor {
void DestroyPeerImpl(Ark_drawing_Matrix peer)
{
}
Ark_drawing_Matrix CtorImpl()
{
    return nullptr;
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void SetRotationImpl(Ark_drawing_Matrix peer,
                     const Ark_Number* degree,
                     const Ark_Number* px,
                     const Ark_Number* py)
{
}
void SetScaleImpl(Ark_drawing_Matrix peer,
                  const Ark_Number* sx,
                  const Ark_Number* sy,
                  const Ark_Number* px,
                  const Ark_Number* py)
{
}
void SetTranslationImpl(Ark_drawing_Matrix peer,
                        const Ark_Number* dx,
                        const Ark_Number* dy)
{
}
void SetMatrixImpl(Ark_drawing_Matrix peer,
                   const Array_Number* values)
{
}
void PreConcatImpl(Ark_drawing_Matrix peer,
                   Ark_drawing_Matrix matrix)
{
}
Ark_Boolean IsEqualImpl(Ark_drawing_Matrix peer,
                        Ark_drawing_Matrix matrix)
{
    return {};
}
Ark_Boolean InvertImpl(Ark_drawing_Matrix peer,
                       Ark_drawing_Matrix matrix)
{
    return {};
}
Ark_Boolean IsIdentityImpl(Ark_drawing_Matrix peer)
{
    return {};
}
Ark_Number GetValueImpl(Ark_drawing_Matrix peer,
                        const Ark_Number* index)
{
    return {};
}
void PostRotateImpl(Ark_drawing_Matrix peer,
                    const Ark_Number* degree,
                    const Ark_Number* px,
                    const Ark_Number* py)
{
}
void PostScaleImpl(Ark_drawing_Matrix peer,
                   const Ark_Number* sx,
                   const Ark_Number* sy,
                   const Ark_Number* px,
                   const Ark_Number* py)
{
}
void PostTranslateImpl(Ark_drawing_Matrix peer,
                       const Ark_Number* dx,
                       const Ark_Number* dy)
{
}
void PreRotateImpl(Ark_drawing_Matrix peer,
                   const Ark_Number* degree,
                   const Ark_Number* px,
                   const Ark_Number* py)
{
}
void PreScaleImpl(Ark_drawing_Matrix peer,
                  const Ark_Number* sx,
                  const Ark_Number* sy,
                  const Ark_Number* px,
                  const Ark_Number* py)
{
}
void PreTranslateImpl(Ark_drawing_Matrix peer,
                      const Ark_Number* dx,
                      const Ark_Number* dy)
{
}
void ResetImpl(Ark_drawing_Matrix peer)
{
}
Array_common2D_Point MapPointsImpl(Ark_drawing_Matrix peer,
                                   const Array_common2D_Point* src)
{
    return {};
}
Array_Number GetAllImpl(Ark_drawing_Matrix peer)
{
    return {};
}
Ark_Boolean MapRectImpl(Ark_drawing_Matrix peer,
                        const Ark_common2D_Rect* dst,
                        const Ark_common2D_Rect* src)
{
    return {};
}
Ark_Boolean SetRectToRectImpl(Ark_drawing_Matrix peer,
                              const Ark_common2D_Rect* src,
                              const Ark_common2D_Rect* dst,
                              Ark_drawing_ScaleToFit scaleToFit)
{
    return {};
}
Ark_Boolean SetPolyToPolyImpl(Ark_drawing_Matrix peer,
                              const Array_common2D_Point* src,
                              const Array_common2D_Point* dst,
                              const Ark_Number* count)
{
    return {};
}
} // Drawing_MatrixAccessor
const GENERATED_ArkUIDrawing_MatrixAccessor* GetDrawing_MatrixAccessor()
{
    static const GENERATED_ArkUIDrawing_MatrixAccessor Drawing_MatrixAccessorImpl {
        Drawing_MatrixAccessor::DestroyPeerImpl,
        Drawing_MatrixAccessor::CtorImpl,
        Drawing_MatrixAccessor::GetFinalizerImpl,
        Drawing_MatrixAccessor::SetRotationImpl,
        Drawing_MatrixAccessor::SetScaleImpl,
        Drawing_MatrixAccessor::SetTranslationImpl,
        Drawing_MatrixAccessor::SetMatrixImpl,
        Drawing_MatrixAccessor::PreConcatImpl,
        Drawing_MatrixAccessor::IsEqualImpl,
        Drawing_MatrixAccessor::InvertImpl,
        Drawing_MatrixAccessor::IsIdentityImpl,
        Drawing_MatrixAccessor::GetValueImpl,
        Drawing_MatrixAccessor::PostRotateImpl,
        Drawing_MatrixAccessor::PostScaleImpl,
        Drawing_MatrixAccessor::PostTranslateImpl,
        Drawing_MatrixAccessor::PreRotateImpl,
        Drawing_MatrixAccessor::PreScaleImpl,
        Drawing_MatrixAccessor::PreTranslateImpl,
        Drawing_MatrixAccessor::ResetImpl,
        Drawing_MatrixAccessor::MapPointsImpl,
        Drawing_MatrixAccessor::GetAllImpl,
        Drawing_MatrixAccessor::MapRectImpl,
        Drawing_MatrixAccessor::SetRectToRectImpl,
        Drawing_MatrixAccessor::SetPolyToPolyImpl,
    };
    return &Drawing_MatrixAccessorImpl;
}

}

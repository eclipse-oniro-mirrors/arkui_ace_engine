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
namespace CanvasRendererModifier {
Ark_NativePointer CtorImpl()
{
    return 0;
}
Ark_NativePointer GetFinalizerImpl()
{
    return 0;
}
void DrawImage0Impl(CanvasRendererPeer* peer,
                    const Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* dx,
                    const Ark_Number* dy)
{
}
void DrawImage1Impl(CanvasRendererPeer* peer,
                    const Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* dx,
                    const Ark_Number* dy,
                    const Ark_Number* dw,
                    const Ark_Number* dh)
{
}
void DrawImage2Impl(CanvasRendererPeer* peer,
                    const Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* sx,
                    const Ark_Number* sy,
                    const Ark_Number* sw,
                    const Ark_Number* sh,
                    const Ark_Number* dx,
                    const Ark_Number* dy,
                    const Ark_Number* dw,
                    const Ark_Number* dh)
{
}
void BeginPathImpl(CanvasRendererPeer* peer)
{
}
void Clip0Impl(CanvasRendererPeer* peer,
               const Opt_CanvasFillRule* fillRule)
{
}
void Clip1Impl(CanvasRendererPeer* peer,
               const Ark_Materialized* path,
               const Opt_CanvasFillRule* fillRule)
{
}
void Fill0Impl(CanvasRendererPeer* peer,
               const Opt_CanvasFillRule* fillRule)
{
}
void Fill1Impl(CanvasRendererPeer* peer,
               const Ark_Materialized* path,
               const Opt_CanvasFillRule* fillRule)
{
}
void Stroke0Impl(CanvasRendererPeer* peer)
{
}
void Stroke1Impl(CanvasRendererPeer* peer,
                 const Ark_Materialized* path)
{
}
Ark_NativePointer CreateLinearGradientImpl(CanvasRendererPeer* peer,
                                           const Ark_Number* x0,
                                           const Ark_Number* y0,
                                           const Ark_Number* x1,
                                           const Ark_Number* y1)
{
    return 0;
}
Ark_NativePointer CreatePatternImpl(CanvasRendererPeer* peer,
                                    const Ark_Materialized* image,
                                    const Union_String_Ark_Undefined* repetition)
{
    return 0;
}
Ark_NativePointer CreateRadialGradientImpl(CanvasRendererPeer* peer,
                                           const Ark_Number* x0,
                                           const Ark_Number* y0,
                                           const Ark_Number* r0,
                                           const Ark_Number* x1,
                                           const Ark_Number* y1,
                                           const Ark_Number* r1)
{
    return 0;
}
Ark_NativePointer CreateConicGradientImpl(CanvasRendererPeer* peer,
                                          const Ark_Number* startAngle,
                                          const Ark_Number* x,
                                          const Ark_Number* y)
{
    return 0;
}
Ark_NativePointer CreateImageData0Impl(CanvasRendererPeer* peer,
                                       const Ark_Number* sw,
                                       const Ark_Number* sh)
{
    return 0;
}
Ark_NativePointer CreateImageData1Impl(CanvasRendererPeer* peer,
                                       const Ark_ImageData* imagedata)
{
    return 0;
}
Ark_NativePointer GetImageDataImpl(CanvasRendererPeer* peer,
                                   const Ark_Number* sx,
                                   const Ark_Number* sy,
                                   const Ark_Number* sw,
                                   const Ark_Number* sh)
{
    return 0;
}
Ark_NativePointer GetPixelMapImpl(CanvasRendererPeer* peer,
                                  const Ark_Number* sx,
                                  const Ark_Number* sy,
                                  const Ark_Number* sw,
                                  const Ark_Number* sh)
{
    return 0;
}
void PutImageData0Impl(CanvasRendererPeer* peer,
                       const Ark_ImageData* imagedata,
                       const Union_Number_String* dx,
                       const Union_Number_String* dy)
{
}
void PutImageData1Impl(CanvasRendererPeer* peer,
                       const Ark_ImageData* imagedata,
                       const Union_Number_String* dx,
                       const Union_Number_String* dy,
                       const Union_Number_String* dirtyX,
                       const Union_Number_String* dirtyY,
                       const Union_Number_String* dirtyWidth,
                       const Union_Number_String* dirtyHeight)
{
}
void GetLineDashImpl(CanvasRendererPeer* peer)
{
}
void SetLineDashImpl(CanvasRendererPeer* peer,
                     const Array_Number* segments)
{
}
void ClearRectImpl(CanvasRendererPeer* peer,
                   const Ark_Number* x,
                   const Ark_Number* y,
                   const Ark_Number* w,
                   const Ark_Number* h)
{
}
void FillRectImpl(CanvasRendererPeer* peer,
                  const Ark_Number* x,
                  const Ark_Number* y,
                  const Ark_Number* w,
                  const Ark_Number* h)
{
}
void StrokeRectImpl(CanvasRendererPeer* peer,
                    const Ark_Number* x,
                    const Ark_Number* y,
                    const Ark_Number* w,
                    const Ark_Number* h)
{
}
void RestoreImpl(CanvasRendererPeer* peer)
{
}
void SaveImpl(CanvasRendererPeer* peer)
{
}
void FillTextImpl(CanvasRendererPeer* peer,
                  const Ark_String* text,
                  const Ark_Number* x,
                  const Ark_Number* y,
                  const Opt_Number* maxWidth)
{
}
Ark_NativePointer MeasureTextImpl(CanvasRendererPeer* peer,
                                  const Ark_String* text)
{
    return 0;
}
void StrokeTextImpl(CanvasRendererPeer* peer,
                    const Ark_String* text,
                    const Ark_Number* x,
                    const Ark_Number* y,
                    const Opt_Number* maxWidth)
{
}
Ark_NativePointer GetTransformImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void ResetTransformImpl(CanvasRendererPeer* peer)
{
}
void RotateImpl(CanvasRendererPeer* peer,
                const Ark_Number* angle)
{
}
void ScaleImpl(CanvasRendererPeer* peer,
               const Ark_Number* x,
               const Ark_Number* y)
{
}
void SetTransform0Impl(CanvasRendererPeer* peer,
                       const Ark_Number* a,
                       const Ark_Number* b,
                       const Ark_Number* c,
                       const Ark_Number* d,
                       const Ark_Number* e,
                       const Ark_Number* f)
{
}
void SetTransform1Impl(CanvasRendererPeer* peer,
                       const Opt_Matrix2D* transform)
{
}
void TransformImpl(CanvasRendererPeer* peer,
                   const Ark_Number* a,
                   const Ark_Number* b,
                   const Ark_Number* c,
                   const Ark_Number* d,
                   const Ark_Number* e,
                   const Ark_Number* f)
{
}
void TranslateImpl(CanvasRendererPeer* peer,
                   const Ark_Number* x,
                   const Ark_Number* y)
{
}
void SetPixelMapImpl(CanvasRendererPeer* peer,
                     const Opt_PixelMap* value)
{
}
void TransferFromImageBitmapImpl(CanvasRendererPeer* peer,
                                 const Ark_Materialized* bitmap)
{
}
void SaveLayerImpl(CanvasRendererPeer* peer)
{
}
void RestoreLayerImpl(CanvasRendererPeer* peer)
{
}
void ResetImpl(CanvasRendererPeer* peer)
{
}
Ark_Int32 GetGlobalAlphaImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetGlobalAlphaImpl(CanvasRendererPeer* peer,
                        const Ark_Number* globalAlpha)
{
}
void GetGlobalCompositeOperationImpl(CanvasRendererPeer* peer)
{
}
void SetGlobalCompositeOperationImpl(CanvasRendererPeer* peer,
                                     const Ark_String* globalCompositeOperation)
{
}
void SetFillStyleImpl(CanvasRendererPeer* peer,
                      const Union_String_Number_CanvasGradient_CanvasPattern* fillStyle)
{
}
void SetStrokeStyleImpl(CanvasRendererPeer* peer,
                        const Union_String_Number_CanvasGradient_CanvasPattern* strokeStyle)
{
}
void GetFilterImpl(CanvasRendererPeer* peer)
{
}
void SetFilterImpl(CanvasRendererPeer* peer,
                   const Ark_String* filter)
{
}
Ark_Boolean GetImageSmoothingEnabledImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetImageSmoothingEnabledImpl(CanvasRendererPeer* peer,
                                  Ark_Boolean imageSmoothingEnabled)
{
}
void SetImageSmoothingQualityImpl(CanvasRendererPeer* peer,
                                  const ImageSmoothingQuality* imageSmoothingQuality)
{
}
void SetLineCapImpl(CanvasRendererPeer* peer,
                    const CanvasLineCap* lineCap)
{
}
Ark_Int32 GetLineDashOffsetImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetLineDashOffsetImpl(CanvasRendererPeer* peer,
                           const Ark_Number* lineDashOffset)
{
}
void SetLineJoinImpl(CanvasRendererPeer* peer,
                     const CanvasLineJoin* lineJoin)
{
}
Ark_Int32 GetLineWidthImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetLineWidthImpl(CanvasRendererPeer* peer,
                      const Ark_Number* lineWidth)
{
}
Ark_Int32 GetMiterLimitImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetMiterLimitImpl(CanvasRendererPeer* peer,
                       const Ark_Number* miterLimit)
{
}
Ark_Int32 GetShadowBlurImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetShadowBlurImpl(CanvasRendererPeer* peer,
                       const Ark_Number* shadowBlur)
{
}
void GetShadowColorImpl(CanvasRendererPeer* peer)
{
}
void SetShadowColorImpl(CanvasRendererPeer* peer,
                        const Ark_String* shadowColor)
{
}
Ark_Int32 GetShadowOffsetXImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetShadowOffsetXImpl(CanvasRendererPeer* peer,
                          const Ark_Number* shadowOffsetX)
{
}
Ark_Int32 GetShadowOffsetYImpl(CanvasRendererPeer* peer)
{
    return 0;
}
void SetShadowOffsetYImpl(CanvasRendererPeer* peer,
                          const Ark_Number* shadowOffsetY)
{
}
void SetDirectionImpl(CanvasRendererPeer* peer,
                      const CanvasDirection* direction)
{
}
void GetFontImpl(CanvasRendererPeer* peer)
{
}
void SetFontImpl(CanvasRendererPeer* peer,
                 const Ark_String* font)
{
}
void SetTextAlignImpl(CanvasRendererPeer* peer,
                      const CanvasTextAlign* textAlign)
{
}
void SetTextBaselineImpl(CanvasRendererPeer* peer,
                         const CanvasTextBaseline* textBaseline)
{
}
} // CanvasRendererModifier
const GENERATED_ArkUICanvasRendererAccessor* GetCanvasRendererAccessor()
{
    static const GENERATED_ArkUICanvasRendererAccessor CanvasRendererAccessorImpl {
        CanvasRendererModifier::CtorImpl,
        CanvasRendererModifier::GetFinalizerImpl,
        CanvasRendererModifier::DrawImage0Impl,
        CanvasRendererModifier::DrawImage1Impl,
        CanvasRendererModifier::DrawImage2Impl,
        CanvasRendererModifier::BeginPathImpl,
        CanvasRendererModifier::Clip0Impl,
        CanvasRendererModifier::Clip1Impl,
        CanvasRendererModifier::Fill0Impl,
        CanvasRendererModifier::Fill1Impl,
        CanvasRendererModifier::Stroke0Impl,
        CanvasRendererModifier::Stroke1Impl,
        CanvasRendererModifier::CreateLinearGradientImpl,
        CanvasRendererModifier::CreatePatternImpl,
        CanvasRendererModifier::CreateRadialGradientImpl,
        CanvasRendererModifier::CreateConicGradientImpl,
        CanvasRendererModifier::CreateImageData0Impl,
        CanvasRendererModifier::CreateImageData1Impl,
        CanvasRendererModifier::GetImageDataImpl,
        CanvasRendererModifier::GetPixelMapImpl,
        CanvasRendererModifier::PutImageData0Impl,
        CanvasRendererModifier::PutImageData1Impl,
        CanvasRendererModifier::GetLineDashImpl,
        CanvasRendererModifier::SetLineDashImpl,
        CanvasRendererModifier::ClearRectImpl,
        CanvasRendererModifier::FillRectImpl,
        CanvasRendererModifier::StrokeRectImpl,
        CanvasRendererModifier::RestoreImpl,
        CanvasRendererModifier::SaveImpl,
        CanvasRendererModifier::FillTextImpl,
        CanvasRendererModifier::MeasureTextImpl,
        CanvasRendererModifier::StrokeTextImpl,
        CanvasRendererModifier::GetTransformImpl,
        CanvasRendererModifier::ResetTransformImpl,
        CanvasRendererModifier::RotateImpl,
        CanvasRendererModifier::ScaleImpl,
        CanvasRendererModifier::SetTransform0Impl,
        CanvasRendererModifier::SetTransform1Impl,
        CanvasRendererModifier::TransformImpl,
        CanvasRendererModifier::TranslateImpl,
        CanvasRendererModifier::SetPixelMapImpl,
        CanvasRendererModifier::TransferFromImageBitmapImpl,
        CanvasRendererModifier::SaveLayerImpl,
        CanvasRendererModifier::RestoreLayerImpl,
        CanvasRendererModifier::ResetImpl,
        CanvasRendererModifier::GetGlobalAlphaImpl,
        CanvasRendererModifier::SetGlobalAlphaImpl,
        CanvasRendererModifier::GetGlobalCompositeOperationImpl,
        CanvasRendererModifier::SetGlobalCompositeOperationImpl,
        CanvasRendererModifier::SetFillStyleImpl,
        CanvasRendererModifier::SetStrokeStyleImpl,
        CanvasRendererModifier::GetFilterImpl,
        CanvasRendererModifier::SetFilterImpl,
        CanvasRendererModifier::GetImageSmoothingEnabledImpl,
        CanvasRendererModifier::SetImageSmoothingEnabledImpl,
        CanvasRendererModifier::SetImageSmoothingQualityImpl,
        CanvasRendererModifier::SetLineCapImpl,
        CanvasRendererModifier::GetLineDashOffsetImpl,
        CanvasRendererModifier::SetLineDashOffsetImpl,
        CanvasRendererModifier::SetLineJoinImpl,
        CanvasRendererModifier::GetLineWidthImpl,
        CanvasRendererModifier::SetLineWidthImpl,
        CanvasRendererModifier::GetMiterLimitImpl,
        CanvasRendererModifier::SetMiterLimitImpl,
        CanvasRendererModifier::GetShadowBlurImpl,
        CanvasRendererModifier::SetShadowBlurImpl,
        CanvasRendererModifier::GetShadowColorImpl,
        CanvasRendererModifier::SetShadowColorImpl,
        CanvasRendererModifier::GetShadowOffsetXImpl,
        CanvasRendererModifier::SetShadowOffsetXImpl,
        CanvasRendererModifier::GetShadowOffsetYImpl,
        CanvasRendererModifier::SetShadowOffsetYImpl,
        CanvasRendererModifier::SetDirectionImpl,
        CanvasRendererModifier::GetFontImpl,
        CanvasRendererModifier::SetFontImpl,
        CanvasRendererModifier::SetTextAlignImpl,
        CanvasRendererModifier::SetTextBaselineImpl,
    };
    return &CanvasRendererAccessorImpl;
}

}
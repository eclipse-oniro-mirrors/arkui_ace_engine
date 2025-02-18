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
#include "core/interfaces/native/utility/converter_union.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "canvas_renderer_peer_impl.h"
#include "canvas_path_accessor_peer_impl.h"
#include "canvas_pattern_peer.h"
#include "canvas_gradient_peer.h"
#include "matrix2d_peer.h"
#include "image_bitmap_peer_impl.h"
#include "image_data_peer.h"
#include "base/utils/utils.h"
#include "pixel_map_peer.h"

namespace OHOS::Ace::NG {
namespace {
const auto ALPHA_LIMIT_MIN = 0.0;
const auto ALPHA_LIMIT_MAX = 1.0;
const auto SIZE_LIMIT_MIN = 0.0;
const auto SEGMENT_LIMIT_MIN = 0.0;
const auto SCALE_LIMIT_MIN = 0.0;
constexpr uint32_t COLOR_WHITE = 0xffffffff;
const auto EMPTY_STRING = "";
const auto FILL_RULE_EVEN_ODD = "evenodd";
const auto DIR_AUTO = "auto";
const auto DIR_INHERIT = "inherit";
const auto DIR_LTR = "ltr";
const auto DIR_RTL = "rtl";
const auto DOM_CENTER = "center";
const auto DOM_END = "end";
const auto DOM_JUSTIFY = "justify";
const auto DOM_LEFT = "left";
const auto DOM_RIGHT = "right";
const auto DOM_START = "start";
const auto DOM_ALPHABETIC = "alphabetic";
const auto DOM_BOTTOM = "bottom";
const auto DOM_HANGING = "hanging";
const auto DOM_IDEOGRAPHIC = "ideographic";
const auto DOM_MIDDLE = "middle";
const auto DOM_TOP = "top";
struct Ark_Custom_Rect {
    Ark_Number x;
    Ark_Number y;
    Ark_Number w;
    Ark_Number h;
};
const std::unordered_map<std::string, CompositeOperation> COMPOSITE_TABLE = {
    { "SOURCE_OVER", CompositeOperation::SOURCE_OVER },
    { "SOURCE_ATOP", CompositeOperation::SOURCE_ATOP },
    { "SOURCE_IN", CompositeOperation::SOURCE_IN },
    { "SOURCE_OUT", CompositeOperation::SOURCE_OUT },
    { "DESTINATION_OVER", CompositeOperation::DESTINATION_OVER },
    { "DESTINATION_ATOP", CompositeOperation::DESTINATION_ATOP },
    { "DESTINATION_IN", CompositeOperation::DESTINATION_IN },
    { "DESTINATION_OUT", CompositeOperation::DESTINATION_OUT },
    { "LIGHTER", CompositeOperation::LIGHTER },
    { "COPY", CompositeOperation::COPY },
    { "XOR", CompositeOperation::XOR }
};
std::optional<double> ConvertDimension(
    GeneratedModifier::CanvasRendererPeerImpl* peerImpl, const Ark_Union_Number_String& src)
{
    std::optional<double> dst = std::nullopt;
    Converter::VisitUnion(
        src,
        [&dst, peerImpl](const Ark_String& value) {
            auto opt = Converter::OptConvert<Dimension>(value);
            if (opt) {
                dst = peerImpl->GetDimension(*opt, true);
            }
        },
        [&dst, peerImpl](const Ark_Number& value) {
            auto opt = Converter::OptConvert<Dimension>(value);
            if (opt) {
                dst = peerImpl->GetDimension(*opt);
            }
        },
        []() {});
    return dst;
}
} // namespace
namespace Converter {
template<>
TextBaseline Convert(const Ark_String& src)
{
    auto baseLine = Converter::Convert<std::string>(src);
    static const LinearMapNode<TextBaseline> textBaseAlignTable[] = {
        { DOM_ALPHABETIC, TextBaseline::ALPHABETIC },
        { DOM_BOTTOM, TextBaseline::BOTTOM },
        { DOM_HANGING, TextBaseline::HANGING },
        { DOM_IDEOGRAPHIC, TextBaseline::IDEOGRAPHIC },
        { DOM_MIDDLE, TextBaseline::MIDDLE },
        { DOM_TOP, TextBaseline::TOP },
    };
    auto index = BinarySearchFindIndex(textBaseAlignTable, ArraySize(textBaseAlignTable), baseLine.c_str());
    return index < 0 ? TextBaseline::ALPHABETIC : textBaseAlignTable[index].value;
}
template<>
TextAlign Convert(const Ark_String& src)
{
    auto align = Converter::Convert<std::string>(src);
    static const LinearMapNode<TextAlign> textAlignTable[] = {
        { DOM_CENTER, TextAlign::CENTER },
        { DOM_END, TextAlign::END },
        { DOM_JUSTIFY, TextAlign::JUSTIFY },
        { DOM_LEFT, TextAlign::LEFT },
        { DOM_RIGHT, TextAlign::RIGHT },
        { DOM_START, TextAlign::START },
    };
    auto index = BinarySearchFindIndex(textAlignTable, ArraySize(textAlignTable), align.c_str());
    return index < 0 ? TextAlign::CENTER : textAlignTable[index].value;
}
template<>
TextDirection Convert(const Ark_String& src)
{
    auto val = Converter::Convert<std::string>(src);
    static const LinearMapNode<TextDirection> textDirectionTable[] = {
        { DIR_AUTO, TextDirection::AUTO },
        { DIR_INHERIT, TextDirection::INHERIT },
        { DIR_LTR, TextDirection::LTR },
        { DIR_RTL, TextDirection::RTL },
    };
    auto index = BinarySearchFindIndex(textDirectionTable, ArraySize(textDirectionTable), val.c_str());
    return index < 0 ? TextDirection::LTR : textDirectionTable[index].value;
}
template<>
void AssignCast(std::optional<CompositeOperation>& dst, const Ark_String& src)
{
    auto str = Converter::Convert<std::string>(src);
    str = StringUtils::TrimStr(str);
    StringUtils::TransformStrCase(str, StringUtils::TEXT_CASE_UPPERCASE);
    auto item = COMPOSITE_TABLE.find(str);
    if (item != COMPOSITE_TABLE.end()) {
        dst = item->second;
    } else {
        LOGE("Unexpected enum value in Ark_String: %{public}s", str.c_str());
    }
}

template<>
Rect Convert(const Ark_Custom_Rect& src)
{
    return Rect(
        static_cast<double>(Converter::Convert<float>(src.x)),
        static_cast<double>(Converter::Convert<float>(src.y)),
        static_cast<double>(Converter::Convert<float>(src.w)),
        static_cast<double>(Converter::Convert<float>(src.h))
    );
}

} // namespace OHOS::Ace::NG::Converter
} // namespace OHOS::Ace::NG
namespace OHOS::Ace::NG::GeneratedModifier {
const GENERATED_ArkUICanvasGradientAccessor* GetCanvasGradientAccessor();
const GENERATED_ArkUICanvasPatternAccessor* GetCanvasPatternAccessor();
const GENERATED_ArkUIMatrix2DAccessor* GetMatrix2DAccessor();

namespace CanvasRendererAccessor {
void DestroyPeerImpl(CanvasRendererPeer* peer)
{
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    if (peerImpl) {
        peerImpl->DecRefCount();
    }
}
Ark_CanvasRenderer CtorImpl()
{
    auto peerImpl = Referenced::MakeRefPtr<CanvasRendererPeerImpl>();
    peerImpl->IncRefCount();
    return reinterpret_cast<CanvasRendererPeer*>(Referenced::RawPtr(peerImpl));
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void DrawImage0Impl(CanvasRendererPeer* peer,
                    const Ark_Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* dx,
                    const Ark_Number* dy)
{
    LOGE("ARKOALA CanvasRendererAccessor::DrawImage0Impl where Ark_Union_ImageBitmap_PixelMap "
        " includes Ark_PixelMap which is partially implemented.");
}
void DrawImage1Impl(CanvasRendererPeer* peer,
                    const Ark_Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* dx,
                    const Ark_Number* dy,
                    const Ark_Number* dw,
                    const Ark_Number* dh)
{
    LOGE("ARKOALA CanvasRendererAccessor::DrawImage1Impl where Ark_Union_ImageBitmap_PixelMap "
        " includes Ark_PixelMap which is partially implemented.");
}
void DrawImage2Impl(CanvasRendererPeer* peer,
                    const Ark_Union_ImageBitmap_PixelMap* image,
                    const Ark_Number* sx,
                    const Ark_Number* sy,
                    const Ark_Number* sw,
                    const Ark_Number* sh,
                    const Ark_Number* dx,
                    const Ark_Number* dy,
                    const Ark_Number* dw,
                    const Ark_Number* dh)
{
    LOGE("ARKOALA CanvasRendererAccessor::DrawImage2Impl where Ark_Union_ImageBitmap_PixelMap "
        " includes Ark_PixelMap which is partially implemented.");
}
void BeginPathImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerBeginPathImpl();
}
void Clip0Impl(CanvasRendererPeer* peer,
               const Opt_String* fillRule)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(fillRule);
    auto opt = Converter::OptConvert<std::string>(*fillRule);
    auto rule = opt && *opt == FILL_RULE_EVEN_ODD ? CanvasFillRule::EVENODD : CanvasFillRule::NONZERO;
    peerImpl->Clip(rule);
}
void Clip1Impl(CanvasRendererPeer* peer,
               Ark_Path2D path,
               const Opt_String* fillRule)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(path);
    CHECK_NULL_VOID(fillRule);
    auto opt = Converter::OptConvert<std::string>(*fillRule);
    auto rule = opt && *opt == FILL_RULE_EVEN_ODD ? CanvasFillRule::EVENODD : CanvasFillRule::NONZERO;
    auto pathImpl = reinterpret_cast<CanvasPathPeerImpl*>(path);
    CHECK_NULL_VOID(pathImpl);
    CHECK_NULL_VOID(pathImpl->path);
    peerImpl->Clip(rule, pathImpl->path);
}
void Fill0Impl(CanvasRendererPeer* peer,
               const Opt_String* fillRule)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(fillRule);
    auto opt = Converter::OptConvert<std::string>(*fillRule);
    auto rule = opt && *opt == FILL_RULE_EVEN_ODD ? CanvasFillRule::EVENODD : CanvasFillRule::NONZERO;
    peerImpl->Fill(rule);
}
void Fill1Impl(CanvasRendererPeer* peer,
               Ark_Path2D path,
               const Opt_String* fillRule)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(path);
    CHECK_NULL_VOID(fillRule);
    auto opt = Converter::OptConvert<std::string>(*fillRule);
    auto rule = opt && *opt == FILL_RULE_EVEN_ODD ? CanvasFillRule::EVENODD : CanvasFillRule::NONZERO;
    auto pathImpl = reinterpret_cast<CanvasPathPeerImpl*>(path);
    CHECK_NULL_VOID(pathImpl);
    CHECK_NULL_VOID(pathImpl->path);
    peerImpl->Fill(rule, pathImpl->path);
}
void Stroke0Impl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerStroke0Impl();
}
void Stroke1Impl(CanvasRendererPeer* peer,
                 Ark_Path2D path)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(path);
    CHECK_NULL_VOID(path);
    auto pathImpl = reinterpret_cast<CanvasPathPeerImpl*>(path);
    CHECK_NULL_VOID(pathImpl);
    CHECK_NULL_VOID(pathImpl->path);
    peerImpl->TriggerStroke1Impl(pathImpl->path);
}
Ark_CanvasGradient CreateLinearGradientImpl(CanvasRendererPeer* peer,
                                            const Ark_Number* x0,
                                            const Ark_Number* y0,
                                            const Ark_Number* x1,
                                            const Ark_Number* y1)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(x0, nullptr);
    CHECK_NULL_RETURN(y0, nullptr);
    CHECK_NULL_RETURN(x1, nullptr);
    CHECK_NULL_RETURN(y1, nullptr);
    double cx0 = static_cast<double>(Converter::Convert<float>(*x0));
    double cy0 = static_cast<double>(Converter::Convert<float>(*y0));
    double cx1 = static_cast<double>(Converter::Convert<float>(*x1));
    double cy1 = static_cast<double>(Converter::Convert<float>(*y1));
    auto gradient = peerImpl->CreateLinearGradient(cx0, cy0, cx1, cy1);
    CHECK_NULL_RETURN(gradient, nullptr);
    auto canvasGradientPeer = reinterpret_cast<CanvasGradientPeer*>(GetCanvasGradientAccessor()->ctor());
    CHECK_NULL_RETURN(canvasGradientPeer, nullptr);
    canvasGradientPeer->SetGradient(gradient);
    return canvasGradientPeer;
}
Ark_CanvasPattern CreatePatternImpl(CanvasRendererPeer* peer,
                                    Ark_ImageBitmap image,
                                    const Opt_String* repetition)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(image, nullptr);
    CHECK_NULL_RETURN(repetition, nullptr);
    auto bitmap = image;
    CHECK_NULL_RETURN(bitmap, nullptr);
    auto opt = Converter::OptConvert<std::string>(*repetition);
    std::string repeat = opt ? *opt : EMPTY_STRING;
    auto pattern = std::make_shared<OHOS::Ace::Pattern>();
    pattern->SetImgSrc(bitmap->GetSrc());
    pattern->SetImageWidth(bitmap->GetWidth());
    pattern->SetImageHeight(bitmap->GetHeight());
    pattern->SetRepetition(repeat);
#if !defined(PREVIEW)
    auto pixelMap = bitmap->GetPixelMap();
    pattern->SetPixelMap(pixelMap);
#endif
    peerImpl->patterns[peerImpl->patternCount];
    auto peerPattern = reinterpret_cast<CanvasPatternPeer*>(GetCanvasPatternAccessor()->ctor());
    CHECK_NULL_RETURN(peerPattern, nullptr);
    peerPattern->SetCanvasRenderer(AceType::WeakClaim(peerImpl));
    peerPattern->SetId(peerImpl->patternCount);
    peerPattern->SetUnit(peerImpl->GetUnit());
    peerImpl->patternCount++;
    return peerPattern;
}
Ark_CanvasGradient CreateRadialGradientImpl(CanvasRendererPeer* peer,
                                            const Ark_Number* x0,
                                            const Ark_Number* y0,
                                            const Ark_Number* r0,
                                            const Ark_Number* x1,
                                            const Ark_Number* y1,
                                            const Ark_Number* r1)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(x0, nullptr);
    CHECK_NULL_RETURN(y0, nullptr);
    CHECK_NULL_RETURN(r0, nullptr);
    CHECK_NULL_RETURN(x1, nullptr);
    CHECK_NULL_RETURN(y1, nullptr);
    CHECK_NULL_RETURN(r1, nullptr);
    double cx0 = static_cast<double>(Converter::Convert<float>(*x0));
    double cy0 = static_cast<double>(Converter::Convert<float>(*y0));
    double cr0 = static_cast<double>(Converter::Convert<float>(*r0));
    double cx1 = static_cast<double>(Converter::Convert<float>(*x1));
    double cy1 = static_cast<double>(Converter::Convert<float>(*y1));
    double cr1 = static_cast<double>(Converter::Convert<float>(*r1));
    std::vector<double> params;
    params.insert(params.end(), { cx0, cy0, cr0, cx1, cy1, cr1 });
    auto gradient = peerImpl->CreateRadialGradient(params);
    CHECK_NULL_RETURN(gradient, nullptr);
    auto canvasGradientPeer = reinterpret_cast<CanvasGradientPeer*>(GetCanvasGradientAccessor()->ctor());
    CHECK_NULL_RETURN(canvasGradientPeer, nullptr);
    canvasGradientPeer->SetGradient(gradient);
    return canvasGradientPeer;
}
Ark_CanvasGradient CreateConicGradientImpl(CanvasRendererPeer* peer,
                                           const Ark_Number* startAngle,
                                           const Ark_Number* x,
                                           const Ark_Number* y)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(startAngle, nullptr);
    CHECK_NULL_RETURN(x, nullptr);
    CHECK_NULL_RETURN(y, nullptr);
    double ca = static_cast<double>(Converter::Convert<float>(*startAngle));
    double cx = static_cast<double>(Converter::Convert<float>(*x));
    double cy = static_cast<double>(Converter::Convert<float>(*y));
    auto gradient = peerImpl->CreateConicGradient(ca, cx, cy);
    CHECK_NULL_RETURN(gradient, nullptr);
    auto canvasGradientPeer = reinterpret_cast<CanvasGradientPeer*>(GetCanvasGradientAccessor()->ctor());
    CHECK_NULL_RETURN(canvasGradientPeer, nullptr);
    canvasGradientPeer->SetGradient(gradient);
    return canvasGradientPeer;
}
Ark_ImageData CreateImageData0Impl(CanvasRendererPeer* peer,
                                   const Ark_Number* sw,
                                   const Ark_Number* sh)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(sw, nullptr);
    CHECK_NULL_RETURN(sh, nullptr);
    auto width = static_cast<double>(Converter::Convert<float>(*sw));
    auto height = static_cast<double>(Converter::Convert<float>(*sh));
    ImageSize imageSize = peerImpl->GetImageSize(0, 0, width, height);
    auto finalWidth = static_cast<uint32_t>(std::abs(imageSize.width));
    auto finalHeight = static_cast<uint32_t>(std::abs(imageSize.height));
    peerImpl->ClearImageData();
    if (finalHeight > 0 && finalWidth > (UINT32_MAX / finalHeight)) {
        LOGE("ARKOALA CanvasRendererPeerImpl::GetImageDataImpl Integer Overflow!!! "
             "The product of finalHeight and finalWidth is too big.");
    } else {
        peerImpl->imageData.dirtyWidth = finalWidth;
        peerImpl->imageData.dirtyHeight = finalHeight;
        for (uint32_t idx = 0; idx < finalWidth * finalHeight; ++idx) {
            peerImpl->imageData.data.push_back(COLOR_WHITE);
        }
    }
    return ImageDataPeer::Create(peerImpl->imageData);
}
Ark_ImageData CreateImageData1Impl(CanvasRendererPeer* peer,
                                   Ark_ImageData imagedata)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(imagedata, nullptr);
    peerImpl->imageData = imagedata->value;
    return ImageDataPeer::Create(peerImpl->imageData);
}
Ark_ImageData GetImageDataImpl(CanvasRendererPeer* peer,
                               const Ark_Number* sx,
                               const Ark_Number* sy,
                               const Ark_Number* sw,
                               const Ark_Number* sh)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(sx, nullptr);
    CHECK_NULL_RETURN(sy, nullptr);
    CHECK_NULL_RETURN(sw, nullptr);
    CHECK_NULL_RETURN(sh, nullptr);
    auto x = static_cast<double>(Converter::Convert<float>(*sx));
    auto y = static_cast<double>(Converter::Convert<float>(*sy));
    auto width = static_cast<double>(Converter::Convert<float>(*sw));
    auto height = static_cast<double>(Converter::Convert<float>(*sh));
    ImageSize imageSize = peerImpl->GetImageSize(x, y, width, height);
    auto finalWidth = static_cast<uint32_t>(std::abs(imageSize.width));
    auto finalHeight = static_cast<uint32_t>(std::abs(imageSize.height));
    peerImpl->ClearImageData();
    if (finalHeight > 0 && finalWidth > (UINT32_MAX / finalHeight)) {
        LOGE("ARKOALA CanvasRendererPeerImpl::GetImageDataImpl Integer Overflow!!! "
             "The product of finalHeight and finalWidth is too big.");
        return ImageDataPeer::Create(peerImpl->imageData);
    }
    std::unique_ptr<Ace::ImageData> canvasData = peerImpl->GetImageData(imageSize);
    CHECK_NULL_RETURN(canvasData, nullptr);
    if (canvasData) {
        peerImpl->imageData = *canvasData;
    }
    return ImageDataPeer::Create(peerImpl->imageData);
}
Ark_PixelMap GetPixelMapImpl(CanvasRendererPeer* peer,
                             const Ark_Number* sx,
                             const Ark_Number* sy,
                             const Ark_Number* sw,
                             const Ark_Number* sh)
{
#ifdef PIXEL_MAP_SUPPORTED
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(sx, nullptr);
    CHECK_NULL_RETURN(sy, nullptr);
    CHECK_NULL_RETURN(sw, nullptr);
    CHECK_NULL_RETURN(sh, nullptr);
    auto x = static_cast<double>(Converter::Convert<float>(*sx));
    auto y = static_cast<double>(Converter::Convert<float>(*sy));
    auto width = static_cast<double>(Converter::Convert<float>(*sw));
    auto height = static_cast<double>(Converter::Convert<float>(*sh));
    ImageSize imageSize = peerImpl->GetImageSize(x, y, width, height);
    peerImpl->ClearImageData();
    peerImpl->GetPixelMap(imageSize);
    return {};
#else
    LOGE("ARKOALA CanvasRendererAccessor::GetPixelMapImpl PixelMap is not supported on current platform.");
    return {};
#endif
}
void PutImageData0Impl(CanvasRendererPeer* peer,
                       Ark_ImageData imagedata,
                       const Ark_Union_Number_String* dx,
                       const Ark_Union_Number_String* dy)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(imagedata);
    CHECK_NULL_VOID(dx);
    CHECK_NULL_VOID(dy);
    ImageSizeExt ext;
    auto src = imagedata->value;
    ext.x = ConvertDimension(peerImpl, *dx);
    ext.y = ConvertDimension(peerImpl, *dy);
    peerImpl->PutImageData(src, ext);
}
void PutImageData1Impl(CanvasRendererPeer* peer,
                       Ark_ImageData imagedata,
                       const Ark_Union_Number_String* dx,
                       const Ark_Union_Number_String* dy,
                       const Ark_Union_Number_String* dirtyX,
                       const Ark_Union_Number_String* dirtyY,
                       const Ark_Union_Number_String* dirtyWidth,
                       const Ark_Union_Number_String* dirtyHeight)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(imagedata);
    CHECK_NULL_VOID(dx);
    CHECK_NULL_VOID(dy);
    CHECK_NULL_VOID(dirtyX);
    CHECK_NULL_VOID(dirtyY);
    CHECK_NULL_VOID(dirtyWidth);
    CHECK_NULL_VOID(dirtyHeight);
    ImageSizeExt ext;
    auto src = imagedata->value;
    ext.x = ConvertDimension(peerImpl, *dx);
    ext.y = ConvertDimension(peerImpl, *dy);
    ext.dirtyX = ConvertDimension(peerImpl, *dirtyX);
    ext.dirtyY = ConvertDimension(peerImpl, *dirtyY);
    ext.dirtyWidth = ConvertDimension(peerImpl, *dirtyWidth);
    ext.dirtyHeight = ConvertDimension(peerImpl, *dirtyHeight);
    peerImpl->PutImageData(src, ext);
}
void GetLineDashImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerGetLineDashImpl();
    LOGE("ARKOALA CanvasRendererAccessor::GetLineDashImpl method should return object with specific type");
}
void SetLineDashImpl(CanvasRendererPeer* peer,
                     const Array_Number* segments)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(segments);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto src = Converter::Convert<std::vector<float>>(*segments);
    if (src.size() == SIZE_LIMIT_MIN) {
        return;
    }
    for (const auto& segment : src) {
        if (segment < SEGMENT_LIMIT_MIN) {
            return;
        }
    }
    std::vector<double> dst;
    std::transform(src.begin(), src.end(), std::back_inserter(dst), [](auto val) { return static_cast<double>(val); });
    peerImpl->TriggerSetLineDashImpl(dst);
}
void ClearRectImpl(CanvasRendererPeer* peer,
                   const Ark_Number* x,
                   const Ark_Number* y,
                   const Ark_Number* w,
                   const Ark_Number* h)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    CHECK_NULL_VOID(w);
    CHECK_NULL_VOID(h);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto arkRect = Ark_Custom_Rect {
        .x = *x,
        .y = *y,
        .w = *w,
        .h = *h
    };
    auto rect = Converter::Convert<Rect>(arkRect);
    if (!rect.IsValid()) {
        return;
    }
    peerImpl->TriggerClearRectImpl(rect);
}
void FillRectImpl(CanvasRendererPeer* peer,
                  const Ark_Number* x,
                  const Ark_Number* y,
                  const Ark_Number* w,
                  const Ark_Number* h)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    CHECK_NULL_VOID(w);
    CHECK_NULL_VOID(h);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto arkRect = Ark_Custom_Rect {
        .x = *x,
        .y = *y,
        .w = *w,
        .h = *h
    };
    auto rect = Converter::Convert<Rect>(arkRect);
    if (!rect.IsValid()) {
        return;
    }
    peerImpl->TriggerFillRectImpl(rect);
}
void StrokeRectImpl(CanvasRendererPeer* peer,
                    const Ark_Number* x,
                    const Ark_Number* y,
                    const Ark_Number* w,
                    const Ark_Number* h)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    CHECK_NULL_VOID(w);
    CHECK_NULL_VOID(h);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto arkRect = Ark_Custom_Rect {
        .x = *x,
        .y = *y,
        .w = *w,
        .h = *h
    };
    auto rect = Converter::Convert<Rect>(arkRect);
    if (!rect.IsValid()) {
        return;
    }
    peerImpl->TriggerStrokeRectImpl(rect);
}
void RestoreImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerRestoreImpl();
}
void SaveImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerSaveImpl();
}
void FillTextImpl(CanvasRendererPeer* peer,
                  const Ark_String* text,
                  const Ark_Number* x,
                  const Ark_Number* y,
                  const Opt_Number* maxWidth)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(text);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    CHECK_NULL_VOID(maxWidth);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto valueText = Converter::Convert<std::string>(*text);
    auto valueX = static_cast<double>(Converter::Convert<float>(*x));
    auto valueY = static_cast<double>(Converter::Convert<float>(*y));
    auto opt = Converter::OptConvert<float>(*maxWidth);

    auto optMaxWidth = std::make_optional<double>();
    if (opt.has_value()) {
        *optMaxWidth = static_cast<double>(*opt);
    }
    peerImpl->TriggerFillTextImpl(valueText, valueX, valueY, optMaxWidth);
}
Ark_NativePointer MeasureTextImpl(CanvasRendererPeer* peer,
                                  const Ark_String* text)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    CHECK_NULL_RETURN(text, nullptr);
    auto content = Converter::Convert<std::string>(*text);
    auto opt = peerImpl->GetTextMetrics(content);
    CHECK_NULL_RETURN(opt, nullptr);
    LOGE("ARKOALA CanvasRendererAccessor::MeasureTextImpl return type Ark_NativePointer "
         "should be replaced with an accessor type for TextMetrics.");
    return nullptr;
}
void StrokeTextImpl(CanvasRendererPeer* peer,
                    const Ark_String* text,
                    const Ark_Number* x,
                    const Ark_Number* y,
                    const Opt_Number* maxWidth)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(text);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    CHECK_NULL_VOID(maxWidth);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto valueText = Converter::Convert<std::string>(*text);
    auto valueX = static_cast<double>(Converter::Convert<float>(*x));
    auto valueY = static_cast<double>(Converter::Convert<float>(*y));
    auto opt = Converter::OptConvert<float>(*maxWidth);

    auto optMaxWidth = std::make_optional<double>();
    if (opt.has_value()) {
        *optMaxWidth = static_cast<double>(*opt);
    }
    peerImpl->TriggerStrokeTextImpl(valueText, valueX, valueY, optMaxWidth);
}
Ark_Matrix2D GetTransformImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_RETURN(peer, nullptr);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, nullptr);
    auto matrixPeer = reinterpret_cast<Matrix2DPeer*>(GetMatrix2DAccessor()->ctor());
    CHECK_NULL_RETURN(matrixPeer, nullptr);
    if (Container::IsCurrentUseNewPipeline()) {
        auto opt = peerImpl->GetTransform();
        if (opt) {
            matrixPeer->transform = *opt;
        }
    }
    return matrixPeer;
}
void ResetTransformImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerResetTransformImpl();
}
void RotateImpl(CanvasRendererPeer* peer,
                const Ark_Number* angle)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(angle);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto rotateAngle = static_cast<double>(Converter::Convert<float>(*angle));
    peerImpl->TriggerRotateImpl(rotateAngle);
}
void ScaleImpl(CanvasRendererPeer* peer,
               const Ark_Number* x,
               const Ark_Number* y)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto scaleX = static_cast<double>(Converter::Convert<float>(*x));
    auto scaleY = static_cast<double>(Converter::Convert<float>(*y));
    if (scaleX < SCALE_LIMIT_MIN || scaleY < SCALE_LIMIT_MIN) {
        return;
    }
    peerImpl->TriggerScaleImpl(scaleX, scaleY);
}
void SetTransform0Impl(CanvasRendererPeer* peer,
                       const Ark_Number* a,
                       const Ark_Number* b,
                       const Ark_Number* c,
                       const Ark_Number* d,
                       const Ark_Number* e,
                       const Ark_Number* f)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(a);
    CHECK_NULL_VOID(b);
    CHECK_NULL_VOID(c);
    CHECK_NULL_VOID(d);
    CHECK_NULL_VOID(e);
    CHECK_NULL_VOID(f);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto param = TransformParam {
        .scaleX = static_cast<double>(Converter::Convert<float>(*a)),
        .skewX = static_cast<double>(Converter::Convert<float>(*b)),
        .skewY = static_cast<double>(Converter::Convert<float>(*c)),
        .scaleY = static_cast<double>(Converter::Convert<float>(*d)),
        .translateX = static_cast<double>(Converter::Convert<float>(*e)),
        .translateY = static_cast<double>(Converter::Convert<float>(*f)) };
    if (param.scaleX < SCALE_LIMIT_MIN || param.scaleY < SCALE_LIMIT_MIN) {
        return;
    }
    peerImpl->TriggerSetTransformImpl(param);
}
void SetTransform1Impl(CanvasRendererPeer* peer,
                       const Opt_Matrix2D* transform)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(transform);
    auto opt = Converter::OptConvert<Ark_Matrix2D>(*transform);
    CHECK_NULL_VOID(opt);
    auto matrixPeer = opt.value();
    CHECK_NULL_VOID(matrixPeer);
    auto param = matrixPeer->transform;
    if (LessNotEqual(param.scaleX, SCALE_LIMIT_MIN) || LessNotEqual(param.scaleY, SCALE_LIMIT_MIN)) {
        return;
    }
    peerImpl->TriggerSetTransformImpl(param);
}
void TransformImpl(CanvasRendererPeer* peer,
                   const Ark_Number* a,
                   const Ark_Number* b,
                   const Ark_Number* c,
                   const Ark_Number* d,
                   const Ark_Number* e,
                   const Ark_Number* f)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(a);
    CHECK_NULL_VOID(b);
    CHECK_NULL_VOID(c);
    CHECK_NULL_VOID(d);
    CHECK_NULL_VOID(e);
    CHECK_NULL_VOID(f);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto param = TransformParam {
        .scaleX = static_cast<double>(Converter::Convert<float>(*a)),
        .skewX = static_cast<double>(Converter::Convert<float>(*b)),
        .skewY = static_cast<double>(Converter::Convert<float>(*c)),
        .scaleY = static_cast<double>(Converter::Convert<float>(*d)),
        .translateX = static_cast<double>(Converter::Convert<float>(*e)),
        .translateY = static_cast<double>(Converter::Convert<float>(*f)) };
    if (LessNotEqual(param.scaleX, SCALE_LIMIT_MIN) || LessNotEqual(param.scaleY, SCALE_LIMIT_MIN)) {
        return;
    }
    peerImpl->TriggerTransformImpl(param);
}
void TranslateImpl(CanvasRendererPeer* peer,
                   const Ark_Number* x,
                   const Ark_Number* y)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(x);
    CHECK_NULL_VOID(y);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto transX = static_cast<double>(Converter::Convert<float>(*x));
    auto transY = static_cast<double>(Converter::Convert<float>(*y));
    peerImpl->TriggerTranslateImpl(transX, transY);
}
void SetPixelMapImpl(CanvasRendererPeer* peer,
                     const Opt_PixelMap* value)
{
#ifdef PIXEL_MAP_SUPPORTED
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(value);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto opt = Converter::OptConvert<Ark_PixelMap>(*value);
    CHECK_NULL_VOID(opt);
    auto pixelMapPeer = opt.value();
    CHECK_NULL_VOID(pixelMapPeer);
    peerImpl->SetPixelMap(pixelMapPeer->pixelMap);
#else
    LOGE("ARKOALA CanvasRendererAccessor::SetPixelMapImpl function 'setPixelMap'"
         " is not supported on the current platform.");
#endif
}
void TransferFromImageBitmapImpl(CanvasRendererPeer* peer,
                                 Ark_ImageBitmap bitmap)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(bitmap);
    auto bitmapPeer = bitmap;
    CHECK_NULL_VOID(bitmapPeer);
#ifdef PIXEL_MAP_SUPPORTED
    auto pixelMap = bitmapPeer->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    peerImpl->TriggerTransferFromImageBitmapImpl(pixelMap);
#else
    auto width = bitmapPeer->GetWidth();
    auto height = bitmapPeer->GetHeight();
    if (NonPositive(width) || (NonPositive(height))) {
        return;
    }
    peerImpl->TriggerTransferFromImageBitmapImpl(width, height);
#endif
}
void SaveLayerImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerSaveLayerImpl();
}
void RestoreLayerImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerRestoreLayerImpl();
}
void ResetImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    peerImpl->TriggerResetImpl();
}
Ark_Int32 GetGlobalAlphaImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetGlobalAlphaImpl there is no implementation in controller "
        "for getter method of GlobalAlpha.");
    return {};
}
void SetGlobalAlphaImpl(CanvasRendererPeer* peer,
                        const Ark_Number* globalAlpha)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(globalAlpha);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto alpha = static_cast<double>(Converter::Convert<float>(*globalAlpha));
    if (alpha < ALPHA_LIMIT_MIN || alpha > ALPHA_LIMIT_MAX) {
        return;
    }
    peerImpl->TriggerSetGlobalAlphaImpl(alpha);
}
void GetGlobalCompositeOperationImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetGlobalCompositeOperationImpl method should return "
        "object with specific type");
}
void SetGlobalCompositeOperationImpl(CanvasRendererPeer* peer,
                                     const Ark_String* globalCompositeOperation)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(globalCompositeOperation);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto opt = Converter::OptConvert<CompositeOperation>(*globalCompositeOperation);
    CHECK_NULL_VOID(opt);
    peerImpl->TriggerSetGlobalCompositeOperationImpl(*opt);
}
void SetFillStyleImpl(CanvasRendererPeer* peer,
                      const Ark_Union_String_Number_CanvasGradient_CanvasPattern* fillStyle)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(fillStyle);
    LOGE("ARKOALA CanvasRendererAccessor::SetStrokeStyleImpl input Union includes same type members");
}
void SetStrokeStyleImpl(CanvasRendererPeer* peer,
                        const Ark_Union_String_Number_CanvasGradient_CanvasPattern* strokeStyle)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    CHECK_NULL_VOID(strokeStyle);
    LOGE("ARKOALA CanvasRendererAccessor::SetStrokeStyleImpl input Union includes same type members");
}
void GetFilterImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetFilterImpl method should return object with specific type");
}
void SetFilterImpl(CanvasRendererPeer* peer,
                   const Ark_String* filter)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(filter);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto filterValue = Converter::Convert<std::string>(*filter);
    if (filterValue.length() == 0) {
        return;
    }
    peerImpl->TriggerSetFilterImpl(filterValue);
}
Ark_Boolean GetImageSmoothingEnabledImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetImageSmoothingEnabledImpl there is no implementation in controller "
        "for getter method of ImageSmoothingEnabled.");
    return {};
}
void SetImageSmoothingEnabledImpl(CanvasRendererPeer* peer,
                                  Ark_Boolean imageSmoothingEnabled)
{
    CHECK_NULL_VOID(peer);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto enabled = Converter::Convert<bool>(imageSmoothingEnabled);
    peerImpl->TriggerSetImageSmoothingEnabledImpl(enabled);
}
Ark_NativePointer GetImageSmoothingQualityImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetImageSmoothingQualityImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for ImageSmoothingQuality type.");
    return {};
}
void SetImageSmoothingQualityImpl(CanvasRendererPeer* peer,
                                  const Ark_String* imageSmoothingQuality)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(imageSmoothingQuality);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto quality = Converter::Convert<std::string>(*imageSmoothingQuality);
    peerImpl->SetImageSmoothingQuality(quality);
}
Ark_NativePointer GetLineCapImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetLineCapImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for CanvasLineCap type.");
    LOGE("ARKOALA CanvasRendererAccessor::GetLineCapImpl there is no implementation in controller "
        "for getter method of LineCap.");
    return {};
}
void SetLineCapImpl(CanvasRendererPeer* peer,
                    const Ark_String* lineCap)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(lineCap);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto capStr = Converter::Convert<std::string>(*lineCap);
    peerImpl->SetLineCap(capStr);
}
Ark_Int32 GetLineDashOffsetImpl(CanvasRendererPeer* peer)
{
    CHECK_NULL_RETURN(peer, 0);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_RETURN(peerImpl, 0);

    double offset = peerImpl->TriggerGetLineDashOffsetImpl();
    LOGE("ARKOALA CanvasRendererAccessor::GetLineDashOffsetImpl return type Ark_Int32 "
         "should be replaced with a valid Ark_Number for LineDashParam offset double type.");
    return Converter::ArkValue<Ark_Int32>(static_cast<int32_t>(offset));
}
void SetLineDashOffsetImpl(CanvasRendererPeer* peer,
                           const Ark_Number* lineDashOffset)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(lineDashOffset);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto offset = static_cast<double>(Converter::Convert<float>(*lineDashOffset));
    peerImpl->TriggerSetLineDashOffsetImpl(offset);
}
Ark_NativePointer GetLineJoinImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetLineJoinImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for CanvasLineJoin type.");
    LOGE("ARKOALA CanvasRendererAccessor::GetLineJoinImpl there is no implementation in controller "
        "for getter method of LineJoin.");
    return {};
}
void SetLineJoinImpl(CanvasRendererPeer* peer,
                     const Ark_String* lineJoin)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(lineJoin);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto joinStr = Converter::Convert<std::string>(*lineJoin);
    peerImpl->SetLineJoin(joinStr);
}
Ark_Int32 GetLineWidthImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetLineWidthImpl there is no implementation in controller "
        "for getter method of LinewWidth.");
    return {};
}
void SetLineWidthImpl(CanvasRendererPeer* peer,
                      const Ark_Number* lineWidth)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(lineWidth);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto width = static_cast<double>(Converter::Convert<float>(*lineWidth));
    peerImpl->TriggerSetLineWidthImpl(width);
}
Ark_Int32 GetMiterLimitImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetMiterLimitImpl there is no implementation in controller "
        "for getter method of MiterLimit.");
    return {};
}
void SetMiterLimitImpl(CanvasRendererPeer* peer,
                       const Ark_Number* miterLimit)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(miterLimit);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto limit = static_cast<double>(Converter::Convert<float>(*miterLimit));
    peerImpl->TriggerSetMiterLimitImpl(limit);
}
Ark_Int32 GetShadowBlurImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetShadowBlurImpl there is no implementation in controller "
        "for getter method of ShadowBlur.");
    return {};
}
void SetShadowBlurImpl(CanvasRendererPeer* peer,
                       const Ark_Number* shadowBlur)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(shadowBlur);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto blur = static_cast<double>(Converter::Convert<float>(*shadowBlur));
    peerImpl->TriggerSetShadowBlurImpl(blur);
}
void GetShadowColorImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetShadowColorImpl method should return object with specific type");
}
void SetShadowColorImpl(CanvasRendererPeer* peer,
                        const Ark_String* shadowColor)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(shadowColor);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto opt = Converter::OptConvert<Color>(*shadowColor);
    CHECK_NULL_VOID(opt);
    peerImpl->TriggerSetShadowColorImpl(*opt);
}
Ark_Int32 GetShadowOffsetXImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetShadowOffsetXImpl there is no implementation in controller "
        "for getter method of ShadowOffsetXImpl.");
    return {};
}
void SetShadowOffsetXImpl(CanvasRendererPeer* peer,
                          const Ark_Number* shadowOffsetX)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(shadowOffsetX);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto offsetX = static_cast<double>(Converter::Convert<float>(*shadowOffsetX));
    peerImpl->TriggerSetShadowOffsetXImpl(offsetX);
}
Ark_Int32 GetShadowOffsetYImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetShadowOffsetYImpl there is no implementation in controller "
        "for getter method of ShadowOffsetYImpl.");
    return {};
}
void SetShadowOffsetYImpl(CanvasRendererPeer* peer,
                          const Ark_Number* shadowOffsetY)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(shadowOffsetY);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);

    auto offsetY = static_cast<double>(Converter::Convert<float>(*shadowOffsetY));
    peerImpl->TriggerSetShadowOffsetYImpl(offsetY);
}
Ark_NativePointer GetDirectionImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetDirectionImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for CanvasDirection type.");
    LOGE("ARKOALA CanvasRendererAccessor::GetDirectionImpl there is no implementation in controller "
        "for getter method of Direction.");
    return {};
}
void SetDirectionImpl(CanvasRendererPeer* peer,
                      const Ark_String* direction)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(direction);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto dir = Converter::Convert<TextDirection>(*direction);
    peerImpl->SetTextDirection(dir);
}
void GetFontImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetFontImpl method should return object with specific type");
}
void SetFontImpl(CanvasRendererPeer* peer,
                 const Ark_String* font)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(font);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto fontStr = Converter::Convert<std::string>(*font);

    peerImpl->SetFont(fontStr);
}
Ark_NativePointer GetTextAlignImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetTextAlignImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for CanvasTextAlign type.");
    LOGE("ARKOALA CanvasRendererAccessor::GetTextAlignImpl there is no implementation in controller "
        "for getter method of TextAlign.");
    return {};
}
void SetTextAlignImpl(CanvasRendererPeer* peer,
                      const Ark_String* textAlign)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(textAlign);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto align = Converter::Convert<TextAlign>(*textAlign);
    peerImpl->SetTextAlign(align);
}
Ark_NativePointer GetTextBaselineImpl(CanvasRendererPeer* peer)
{
    LOGE("ARKOALA CanvasRendererAccessor::GetTextBaselineImpl return type Ark_NativePointer "
        "should be replaced with a valid ark enum for CanvasTextBaseline type.");
    LOGE("ARKOALA CanvasRendererAccessor::GetTextBaselineImpl there is no implementation in controller "
        "for getter method of TextBaseline.");
    return {};
}
void SetTextBaselineImpl(CanvasRendererPeer* peer,
                         const Ark_String* textBaseline)
{
    CHECK_NULL_VOID(peer);
    CHECK_NULL_VOID(textBaseline);
    auto peerImpl = reinterpret_cast<CanvasRendererPeerImpl*>(peer);
    CHECK_NULL_VOID(peerImpl);
    auto baseLine = Converter::Convert<TextBaseline>(*textBaseline);
    peerImpl->SetTextBaseline(baseLine);
}
} // CanvasRendererAccessor
const GENERATED_ArkUICanvasRendererAccessor* GetCanvasRendererAccessor()
{
    static const GENERATED_ArkUICanvasRendererAccessor CanvasRendererAccessorImpl {
        CanvasRendererAccessor::DestroyPeerImpl,
        CanvasRendererAccessor::CtorImpl,
        CanvasRendererAccessor::GetFinalizerImpl,
        CanvasRendererAccessor::DrawImage0Impl,
        CanvasRendererAccessor::DrawImage1Impl,
        CanvasRendererAccessor::DrawImage2Impl,
        CanvasRendererAccessor::BeginPathImpl,
        CanvasRendererAccessor::Clip0Impl,
        CanvasRendererAccessor::Clip1Impl,
        CanvasRendererAccessor::Fill0Impl,
        CanvasRendererAccessor::Fill1Impl,
        CanvasRendererAccessor::Stroke0Impl,
        CanvasRendererAccessor::Stroke1Impl,
        CanvasRendererAccessor::CreateLinearGradientImpl,
        CanvasRendererAccessor::CreatePatternImpl,
        CanvasRendererAccessor::CreateRadialGradientImpl,
        CanvasRendererAccessor::CreateConicGradientImpl,
        CanvasRendererAccessor::CreateImageData0Impl,
        CanvasRendererAccessor::CreateImageData1Impl,
        CanvasRendererAccessor::GetImageDataImpl,
        CanvasRendererAccessor::GetPixelMapImpl,
        CanvasRendererAccessor::PutImageData0Impl,
        CanvasRendererAccessor::PutImageData1Impl,
        CanvasRendererAccessor::GetLineDashImpl,
        CanvasRendererAccessor::SetLineDashImpl,
        CanvasRendererAccessor::ClearRectImpl,
        CanvasRendererAccessor::FillRectImpl,
        CanvasRendererAccessor::StrokeRectImpl,
        CanvasRendererAccessor::RestoreImpl,
        CanvasRendererAccessor::SaveImpl,
        CanvasRendererAccessor::FillTextImpl,
        CanvasRendererAccessor::MeasureTextImpl,
        CanvasRendererAccessor::StrokeTextImpl,
        CanvasRendererAccessor::GetTransformImpl,
        CanvasRendererAccessor::ResetTransformImpl,
        CanvasRendererAccessor::RotateImpl,
        CanvasRendererAccessor::ScaleImpl,
        CanvasRendererAccessor::SetTransform0Impl,
        CanvasRendererAccessor::SetTransform1Impl,
        CanvasRendererAccessor::TransformImpl,
        CanvasRendererAccessor::TranslateImpl,
        CanvasRendererAccessor::SetPixelMapImpl,
        CanvasRendererAccessor::TransferFromImageBitmapImpl,
        CanvasRendererAccessor::SaveLayerImpl,
        CanvasRendererAccessor::RestoreLayerImpl,
        CanvasRendererAccessor::ResetImpl,
        CanvasRendererAccessor::GetGlobalAlphaImpl,
        CanvasRendererAccessor::SetGlobalAlphaImpl,
        CanvasRendererAccessor::GetGlobalCompositeOperationImpl,
        CanvasRendererAccessor::SetGlobalCompositeOperationImpl,
        CanvasRendererAccessor::SetFillStyleImpl,
        CanvasRendererAccessor::SetStrokeStyleImpl,
        CanvasRendererAccessor::GetFilterImpl,
        CanvasRendererAccessor::SetFilterImpl,
        CanvasRendererAccessor::GetImageSmoothingEnabledImpl,
        CanvasRendererAccessor::SetImageSmoothingEnabledImpl,
        CanvasRendererAccessor::GetImageSmoothingQualityImpl,
        CanvasRendererAccessor::SetImageSmoothingQualityImpl,
        CanvasRendererAccessor::GetLineCapImpl,
        CanvasRendererAccessor::SetLineCapImpl,
        CanvasRendererAccessor::GetLineDashOffsetImpl,
        CanvasRendererAccessor::SetLineDashOffsetImpl,
        CanvasRendererAccessor::GetLineJoinImpl,
        CanvasRendererAccessor::SetLineJoinImpl,
        CanvasRendererAccessor::GetLineWidthImpl,
        CanvasRendererAccessor::SetLineWidthImpl,
        CanvasRendererAccessor::GetMiterLimitImpl,
        CanvasRendererAccessor::SetMiterLimitImpl,
        CanvasRendererAccessor::GetShadowBlurImpl,
        CanvasRendererAccessor::SetShadowBlurImpl,
        CanvasRendererAccessor::GetShadowColorImpl,
        CanvasRendererAccessor::SetShadowColorImpl,
        CanvasRendererAccessor::GetShadowOffsetXImpl,
        CanvasRendererAccessor::SetShadowOffsetXImpl,
        CanvasRendererAccessor::GetShadowOffsetYImpl,
        CanvasRendererAccessor::SetShadowOffsetYImpl,
        CanvasRendererAccessor::GetDirectionImpl,
        CanvasRendererAccessor::SetDirectionImpl,
        CanvasRendererAccessor::GetFontImpl,
        CanvasRendererAccessor::SetFontImpl,
        CanvasRendererAccessor::GetTextAlignImpl,
        CanvasRendererAccessor::SetTextAlignImpl,
        CanvasRendererAccessor::GetTextBaselineImpl,
        CanvasRendererAccessor::SetTextBaselineImpl,
    };
    return &CanvasRendererAccessorImpl;
}

struct CanvasRendererPeer {
    virtual ~CanvasRendererPeer() = default;
};
}

/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <shared_mutex>
#include "core/components_ng/image_provider/image_decoder.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"

namespace OHOS::Ace::NG {

std::shared_mutex ImageDecoder::pixelMapMtx_;
std::unordered_map<std::string, WeakPtr<PixelMap>> ImageDecoder::weakPixelMapCache_;
RefPtr<CanvasImage> ImageDecoder::MakeDrawingImage(
    const RefPtr<ImageObject>& obj, const ImageDecoderConfig& imageDecoderConfig)
{
    return MakeRefPtr<DrawingImage>(nullptr);
}

RefPtr<CanvasImage> ImageDecoder::MakePixmapImage(
    const RefPtr<ImageObject>& obj, const ImageDecoderConfig& imageDecoderConfig, ImageErrorInfo& errorInfo)
{
    return MakeRefPtr<PixelMapImage>(nullptr);
}

WeakPtr<PixelMap> ImageDecoder::GetFromPixelMapCache(const ImageSourceInfo& imageSourceInfo, const SizeF& size)
{
    return nullptr;
}

} // namespace OHOS::Ace::NG
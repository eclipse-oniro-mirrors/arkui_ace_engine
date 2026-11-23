/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "base/image/pixel_map.h"
#include "core/image/image_cache.h"
#include "core/components_ng/render/adapter/drawing_lattice_impl.h"

namespace OHOS::Ace {
Rosen::Drawing::Lattice* DrawingLatticeImpl::GetLattice()
{
    if (type_ == 0 && lattice_) {
        return lattice_.get();
    }
    if (type_ == 1 && latticeNative_) {
        return latticeNative_;
    }
    return nullptr;
}

void ImageCache::CacheImage(const std::string& key, const std::shared_ptr<CachedImage>& image) {}

RefPtr<PixelMap> PixelMap::ConvertSkImageToPixmap(
    const uint32_t* colors, uint32_t colorLength, int32_t width, int32_t height)
{
    return nullptr;
}
} // namespace OHOS::Ace

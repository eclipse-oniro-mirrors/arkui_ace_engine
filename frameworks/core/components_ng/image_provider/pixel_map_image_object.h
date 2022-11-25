/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_IMAGE_PROVIDER_PIXEL_MAP_IMAGE_OBJECT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_IMAGE_PROVIDER_PIXEL_MAP_IMAGE_OBJECT_H

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/image_provider/image_object.h"

namespace OHOS::Ace::NG {

class PixelMapImageObject : public ImageObject {
    DECLARE_ACE_TYPE(PixelMapImageObject, ImageObject);

public:
    PixelMapImageObject(const RefPtr<PixelMap>& pixmap, const ImageSourceInfo& imageSourceInfo, const SizeF& imageSize)
        : ImageObject(imageSourceInfo, imageSize, 1, nullptr), pixmap_(pixmap)
    {}
    ~PixelMapImageObject() override = default;

    void MakeCanvasImage(
        const LoadCallbacks& loadCallbacks, const SizeF& resizeTarget, bool forceResize, bool syncLoad) override;

    static RefPtr<PixelMapImageObject> Create(
        const ImageSourceInfo& sourceInfo, const RefPtr<ImageEncodedInfo>& encodedInfo, const RefPtr<ImageData>& data);

private:
    RefPtr<PixelMap> pixmap_;

    ACE_DISALLOW_COPY_AND_MOVE(PixelMapImageObject);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_IMAGE_PROVIDER_PIXEL_MAP_IMAGE_OBJECT_H

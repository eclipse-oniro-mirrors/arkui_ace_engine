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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_IMAGE_PAINT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_IMAGE_PAINT_H

#include "core/components/common/properties/alignment.h"
#include "core/components_ng/render/canvas.h"
#include "core/components_ng/render/canvas_image.h"

namespace OHOS::Ace::NG {

struct ImagePaintConfig {
    ImagePaintConfig(const RectF& srcRect, const RectF& dstRect) : srcRect_(srcRect), dstRect_(dstRect) {}
    ~ImagePaintConfig() = default;

    RectF srcRect_;
    RectF dstRect_;
};

class ImagePainter {
public:
    ImagePainter(const RefPtr<CanvasImage>& canvasImage) : canvasImage_(canvasImage) {}
    ~ImagePainter() = default;

    void DrawImage(const RefPtr<Canvas>& canvas, const OffsetF& offset, const ImagePaintConfig& ImagePaintConfig) const;

    static void ApplyImageFit(
        ImageFit imageFit, const SizeF& rawPicSize, const SizeF& dstSize, RectF& srcRect, RectF& dstRect);

private:
    RefPtr<CanvasImage> canvasImage_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_IMAGE_PAINT_H
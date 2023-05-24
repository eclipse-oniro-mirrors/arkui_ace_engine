/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_IMAGE_PAINTER_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_IMAGE_PAINTER_UTILS_H

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#ifdef NEW_SKIA
#include "include/core/SkSamplingOptions.h"
#endif

#include "core/components_ng/render/canvas_image.h"

namespace OHOS::Ace::NG {
class ImagePainterUtils {
public:
    static constexpr uint8_t RADIUS_POINTS_SIZE = 4;
    static std::unique_ptr<SkVector[]> ToSkRadius(const BorderRadiusArray& radiusXY);
    static void ClipRRect(RSCanvas& canvas, const RSRect& dstRect, const BorderRadiusArray& radiusXY);
#ifndef NEW_SKIA
    static void AddFilter(SkPaint& paint, const ImagePaintConfig& config);
#else
    static void AddFilter(SkPaint& paint, SkSamplingOptions& options, const ImagePaintConfig& config);
 #endif
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_IMAGE_PAINTER_UTILS_H

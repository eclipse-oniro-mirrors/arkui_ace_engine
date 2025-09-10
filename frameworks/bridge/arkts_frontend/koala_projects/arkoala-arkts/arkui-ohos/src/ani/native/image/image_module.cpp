/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "image_module.h"

#include "color_filter_ani/ani_color_filter.h"
#include "lattice_ani/ani_lattice.h"
#include "load.h"
#include "log/log.h"
#include "pixel_map_taihe_ani.h"

namespace OHOS::Ace::Ani {
void ImageResizableOptions(ani_env* env, [[maybe_unused]] ani_object obj, ani_long node, ani_object latticeAni)
{
    auto* arkNode = reinterpret_cast<ArkUINodeHandle>(node);
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !arkNode) {
        return;
    }
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(latticeAni, "nativeObj", &nativeObj);
    auto* lattice = reinterpret_cast<void*>(nativeObj);
    if (lattice == nullptr) {
        HILOGE("image reziable options is null");
        return;
    }
    auto* aniLattice = reinterpret_cast<OHOS::Rosen::Drawing::AniLattice*>(lattice);
    if (aniLattice == nullptr) {
        HILOGE("image reziable aniLattice is null");
        return;
    }
    auto resizableLattic = aniLattice->GetLattice();
    if (resizableLattic == nullptr) {
        HILOGE("image reziable resizableLattic is null");
        return;
    }
    modifier->getImageAniModifier()->setResizableLattice(arkNode, &resizableLattic);
}

void ImageConstructPixelMap(ani_env* env, [[maybe_unused]] ani_object obj, ani_long node, ani_object pixelMapAni)
{
    auto* arkNode = reinterpret_cast<ArkUINodeHandle>(node);
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !arkNode) {
        return;
    }
    auto pixelMap = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapAni);
    modifier->getImageAniModifier()->setPixelMap(arkNode, &pixelMap);
}

void ImageConstructDrawableDescriptor(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long node, ani_object drawableAni, ani_int drawableType)
{
    auto* arkNode = reinterpret_cast<ArkUINodeHandle>(node);
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !arkNode) {
        return;
    }
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* drawable = reinterpret_cast<void*>(nativeObj);
    if (drawable == nullptr) {
        HILOGE("image construct with drawable descriptor failed, nativeObj is nullptr");
        return;
    }
    modifier->getImageAniModifier()->setDrawableDescriptor(arkNode, drawable, static_cast<int>(drawableType));
}

void ImageDrawingColorFilter(ani_env* env, [[maybe_unused]] ani_object obj, ani_long node, ani_object colorFilterAni)
{
    auto* arkNode = reinterpret_cast<ArkUINodeHandle>(node);
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !arkNode) {
        return;
    }
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(colorFilterAni, "nativeObj", &nativeObj);
    auto* colorFilter = reinterpret_cast<void*>(nativeObj);
    if (colorFilter == nullptr) {
        HILOGE("image colorFilter options is null");
        return;
    }
    auto* aniColorFilter = reinterpret_cast<OHOS::Rosen::Drawing::AniColorFilter*>(colorFilter);
    if (aniColorFilter == nullptr) {
        HILOGE("image aniColorFilter options is null");
        return;
    }
    auto drawingColorFilter = aniColorFilter->GetColorFilter();
    if (drawingColorFilter == nullptr) {
        HILOGE("image drawingColorFilter options is null");
        return;
    }
    modifier->getImageAniModifier()->setDrawingColorFilter(arkNode, &drawingColorFilter);
}

ani_long Image_ColorFilter_TransferStatic(ani_env* env, [[maybe_unused]] ani_object obj, ani_long colorFilter)
{
    const auto* modifier = GetNodeAniModifier();
    if (modifier == nullptr || colorFilter == 0) {
        return 0;
    }
    auto colorFilterPeer = modifier->getImageAniModifier()->createColorFilterPeer(colorFilter);
    if (colorFilterPeer == 0) {
        return 0;
    }
    auto pointer = reinterpret_cast<ani_long>(colorFilterPeer);
    return pointer;
}

ani_long Image_ColorFilter_TransferDynamic(ani_env* env, [[maybe_unused]] ani_object obj, ani_long colorFilterPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (modifier == nullptr || colorFilterPeer == 0) {
        return 0;
    }
    auto pointer = modifier->getImageAniModifier()->getColorFilter(colorFilterPeer);
    return pointer;
}
} // namespace OHOS::Ace::Ani

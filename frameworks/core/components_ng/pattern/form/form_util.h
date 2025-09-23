/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FORM_FORM_UTIL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FORM_FORM_UTIL_H

#include <memory>
#include "pixel_map.h"

namespace OHOS::Ace::NG {
/**
 * @class FormUtil
 * form util.
 */
class FormUtil {
public:
    /**
     * @brief Determine whether the snapshot is transparent.
     * @param pixelMap the snapshot of SurfaceNode
     * @return true if the snapshot is transparent, false otherwise.
     */
    static bool IsTransparent(std::shared_ptr<Media::PixelMap>& pixelMap);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FORM_FORM_UTIL_H

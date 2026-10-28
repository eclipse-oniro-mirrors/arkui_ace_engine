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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TOKEN_THEME_BRAND_COLOR_INFO_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TOKEN_THEME_BRAND_COLOR_INFO_H

#include <functional>
#include <optional>

#include "ui/properties/color.h"

namespace OHOS::Ace::NG {

inline constexpr char DEFAULT_BRAND_COLOR_STR[] = "";

enum class BrandColorStrategy : int32_t {
    BRAND_COLOR_FOLLOW_SYSTEM = 0,
    BRAND_COLOR_FOLLOW_UEC = 1,
};

struct BrandColorInfo {
    std::optional<Color> brandColor;
    std::optional<BrandColorStrategy> colorStrategy;
};

using BrandColorChangeCallback = std::function<void(
    int32_t instanceId, const std::optional<Color>& brandColor,
    const std::optional<BrandColorStrategy>& colorStrategy)>;

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TOKEN_THEME_BRAND_COLOR_INFO_H

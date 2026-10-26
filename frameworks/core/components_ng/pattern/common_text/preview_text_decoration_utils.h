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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMON_TEXT_PREVIEW_TEXT_DECORATION_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMON_TEXT_PREVIEW_TEXT_DECORATION_UTILS_H

#include "base/geometry/dimension.h"
#include "core/common/ime/text_input_type.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {
constexpr Dimension DEFAULT_FONTSIZE = 16.0_vp;
constexpr float WAVY_WIDTH_DIVISOR = 18.0f;

struct PreviewDecorationParams {
    float left = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float underlineWidth = 0.0f;
    PreviewTextStyle underlineStyle = PreviewTextStyle::NORMAL;
    bool isLast = false;
    RSColor color;
};

void DrawPreviewTextUnderline(RSCanvas& canvas, const PreviewDecorationParams& params);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMON_TEXT_PREVIEW_TEXT_DECORATION_UTILS_H

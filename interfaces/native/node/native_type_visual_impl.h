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

#ifndef FOUNDATION_ACE_INTERFACES_NATIVE_NODE_NATIVE_TYPE_VISUAL_IMPL_H
#define FOUNDATION_ACE_INTERFACES_NATIVE_NODE_NATIVE_TYPE_VISUAL_IMPL_H

#include <vector>

#include "native_type.h"

struct OH_ArkUI_LinearGradientBlurOptions {
    float blurRadius = 0.0f;
    std::vector<float> fractionStops = { 0.0f, 0.0f, 0.0f, 1.0f };
    OH_ArkUI_LinearGradientBlurDirection direction = OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_BOTTOM;
};

#endif // FOUNDATION_ACE_INTERFACES_NATIVE_NODE_NATIVE_TYPE_VISUAL_IMPL_H

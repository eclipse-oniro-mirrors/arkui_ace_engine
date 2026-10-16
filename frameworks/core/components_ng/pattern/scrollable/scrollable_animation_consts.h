/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCROLL_ANIMATION_CONSTS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCROLL_ANIMATION_CONSTS_H

#include <cstdint>

namespace OHOS::Ace::NG {
inline constexpr float FLING_SCALE_K = 1.5f; // x(t) = [K / (4.2 * γ)] * v₀ * [1 - exp(-γt)]
inline constexpr float FRICTION_SCALE = -4.2f;
inline constexpr float HALF_PAGE_SCROLL_DURATION = 50.0f;
inline constexpr uint32_t CUSTOM_SPRING_ANIMATION_DURATION = 1000;
// Snap spring shape shared by the legacy list/scroll snap animation (scrollable.cpp) and the
// item snap stages (scrollable_item_snap.cpp); keep a single definition so they cannot drift.
inline constexpr float SNAP_SPRING_RESPONSE = 0.416f;
inline constexpr float SNAP_SPRING_DAMP = 0.99f;
inline constexpr float SNAP_SPRING_MINIMUM_AMPLITUDE_PX = 1.0f;
#ifdef SUPPORT_DIGITAL_CROWN
inline constexpr float SNAP_SPRING_AMPLITUDE_RATIO = 0.00025f;
#else
inline constexpr float SNAP_SPRING_AMPLITUDE_RATIO = 0.001f;
#endif
}

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCROLL_ANIMATION_CONSTS_H

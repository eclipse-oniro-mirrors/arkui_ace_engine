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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_ACTIVITY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_ACTIVITY_H

#include <cstdint>

namespace OHOS::Ace {
enum class InspectorTreeActivity : uint32_t {
    SCROLL = 0,
    SWIPER_SCROLL = 1,
    SWIPER_TRANSITION = 2,
    PAGE_TRANSITION = 3,
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_ACTIVITY_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_STATIC_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_STATIC_UTILS_H

#include "core/components/common/layout/constants.h"

namespace OHOS::Ace {

class AccessibilityStaticUtils {
public:
    static std::string GetRoleByType(AccessibilityRoleType roleType);
    static std::string GetPageModeType(int32_t pageModeType);
    static int32_t GetFocusDrawLevel(int32_t inputValue);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_STATIC_UTILS_H
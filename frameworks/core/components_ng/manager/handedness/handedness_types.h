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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_TYPES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_TYPES_H

#include <string>

namespace OHOS::Ace::NG {

/// 弹窗跟手布局模式（与前端 HandednessFollowMode 对应）
enum class HandednessFollowMode {
    NONE = 0,               // 默认，不改变当前布局行为
    FOLLOW_ANCHOR = 1,      // 跟随最近操作组件位置
    FOLLOW_GRIP_HAND = 2,   // 跟随握持手位置
};

inline std::string HandednessFollowModeToString(HandednessFollowMode mode)
{
    static const std::string modes[] = { "HandednessFollowMode.NONE",
        "HandednessFollowMode.FOLLOW_ANCHOR", "HandednessFollowMode.FOLLOW_GRIP_HAND" };
    auto index = static_cast<int32_t>(mode);
    if (index >= static_cast<int32_t>(HandednessFollowMode::NONE) &&
        index <= static_cast<int32_t>(HandednessFollowMode::FOLLOW_GRIP_HAND)) {
        return modes[index];
    }
    return modes[static_cast<int32_t>(HandednessFollowMode::NONE)];
}

/// 弹窗水平对齐侧别（FOLLOW_GRIP_HAND 模式返回值）
enum class PopupSide {
    LEFT = 0,
    CENTER = 1,
    RIGHT = 2,
};

inline std::string PopupSideToString(PopupSide side)
{
    static const std::string sides[] = { "PopupSide.LEFT", "PopupSide.CENTER", "PopupSide.RIGHT" };
    auto index = static_cast<int32_t>(side);
    if (index >= static_cast<int32_t>(PopupSide::LEFT) && index <= static_cast<int32_t>(PopupSide::RIGHT)) {
        return sides[index];
    }
    return sides[static_cast<int32_t>(PopupSide::CENTER)];
}

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_TYPES_H

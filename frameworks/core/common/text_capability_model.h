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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_CAPABILITY_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_CAPABILITY_MODEL_H

#include <cstdint>

namespace OHOS::Ace {

enum class TextCapability : uint32_t {
    NONE          = 0,
    COPY          = 1 << 0,
    CUT           = 1 << 1,
    CONTENT_DRAG  = 1 << 2,
    AI_ANALYSIS   = 1 << 3,
    AI_MENU       = 1 << 4,
    ASK_CELIA     = 1 << 5,
};

constexpr TextCapability operator|(TextCapability a, TextCapability b)
{
    return static_cast<TextCapability>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr TextCapability operator&(TextCapability a, TextCapability b)
{
    return static_cast<TextCapability>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr TextCapability& operator|=(TextCapability& a, TextCapability b)
{
    return a = a | b;
}

constexpr TextCapability& operator&=(TextCapability& a, TextCapability b)
{
    return a = a & b;
}

constexpr TextCapability GetPasswordDisabledCapabilities()
{
    return TextCapability::COPY | TextCapability::CUT |
           TextCapability::CONTENT_DRAG |
           TextCapability::AI_ANALYSIS | TextCapability::AI_MENU |
           TextCapability::ASK_CELIA;
}

constexpr bool IsCapabilityAllowed(TextCapability disabledMask, TextCapability cap)
{
    return (disabledMask & cap) == TextCapability::NONE;
}

constexpr bool IsCapabilityDisabled(TextCapability disabledMask, TextCapability cap)
{
    return !IsCapabilityAllowed(disabledMask, cap);
}

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_CAPABILITY_MODEL_H

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

#ifndef FOUNDATION_ACE_NG_COMMAND_VALIDATION_H
#define FOUNDATION_ACE_NG_COMMAND_VALIDATION_H

#include <cmath>
#include <limits>
#include "base/json/json_util.h"
#include "base/utils/macros.h"

namespace OHOS::Ace::NG {
inline bool IsCommandNumber(const std::unique_ptr<JsonValue>& object, const char* key, bool integer = false)
{
    auto value = object->GetValue(key);
    if (!value || !value->IsNumber() || !std::isfinite(value->GetDouble())) {
        return false;
    }
    auto number = value->GetDouble();
    return !integer || (number == std::trunc(number) && number >= std::numeric_limits<int32_t>::min() &&
        number <= std::numeric_limits<int32_t>::max());
}

// The callback entry validates types before invoking legacy parsers that use default values.
// Legacy SendCommand continues to use those parsers directly.
ACE_FORCE_EXPORT bool ValidateCommandParameters(const std::unique_ptr<JsonValue>& json, const std::string& command);
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_NG_COMMAND_VALIDATION_H

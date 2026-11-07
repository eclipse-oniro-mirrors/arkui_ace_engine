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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_UI_SESSION_ACCESSIBILITY_COMMAND_H
#define FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_UI_SESSION_ACCESSIBILITY_COMMAND_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "accessibility_def.h"
#include "base/json/json_util.h"

namespace OHOS::Ace::UiSessionAccessibilityCommand {
enum class ParseResult {
    SUCCESS = 0,
    INVALID_COMMAND,
    UNSUPPORTED_ACTION,
};

ParseResult ParseAccessibilityActionHeader(const JsonValue& json, Accessibility::ActionType& action);

bool ParseAccessibilityArguments(const JsonValue& json, Accessibility::ActionType action,
    std::map<std::string, std::string>& result);

bool ParseElementId(const JsonValue& json, std::optional<int64_t>& elementId);
} // namespace OHOS::Ace::UiSessionAccessibilityCommand

#endif // FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_UI_SESSION_ACCESSIBILITY_COMMAND_H

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

#include "adapter/ohos/entrance/ui_session/ui_session_accessibility_command.h"

#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace OHOS::Ace::UiSessionAccessibilityCommand {
namespace {
constexpr std::pair<const char*, Accessibility::ActionType> SUPPORTED_ACTIONS[] = {
    { "ACTION_CLICK", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK },
    { "ACTION_LONG_CLICK", Accessibility::ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
    { "ACTION_SCROLL_FORWARD", Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
    { "ACTION_SCROLL_BACKWARD", Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
    { "ACTION_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_FOCUS },
    { "ACTION_CLEAR_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
    { "ACTION_ACCESSIBILITY_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
    { "ACTION_CLEAR_ACCESSIBILITY_FOCUS",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
    { "ACTION_NEXT_AT_MOVEMENT_GRANULARITY", Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
    { "ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY", Accessibility::ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
    { "ACTION_SET_TEXT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
    { "ACTION_COPY", Accessibility::ActionType::ACCESSIBILITY_ACTION_COPY },
    { "ACTION_PASTE", Accessibility::ActionType::ACCESSIBILITY_ACTION_PASTE },
    { "ACTION_CUT", Accessibility::ActionType::ACCESSIBILITY_ACTION_CUT },
    { "ACTION_SELECT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SELECT },
    { "ACTION_CLEAR_SELECTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
    { "ACTION_SET_SELECTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
    { "ACTION_SET_CURSOR_POSITION", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
    { "ACTION_EXEC_SUB_COMPONENT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK },
    { "CUSTOM_ACTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM },
    { "BACK", Accessibility::ActionType::ACCESSIBILITY_ACTION_BACK },
};

constexpr const char* UNSUPPORTED_ACTIONS[] = {
    "ACTION_NONE",
    "ACTION_NEXT_HTML_ITEM",
    "ACTION_PREVIOUS_HTML_ITEM",
};

constexpr std::pair<Accessibility::ActionType, const char*> RESTRICTED_ARGUMENT_KEYS[] = {
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, "scrolltype" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, "scrolltype" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT, "textMoveUnit" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT, "textMoveUnit" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT, "setText" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION, "selectTextBegin" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION, "selectTextEnd" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION, "selectTextInForWard" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION, "offset" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK, "spanId" },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM, "customAction" },
};

bool IsEnvelopeField(const std::string& key)
{
    return key == "protocol" || key == "version" || key == "action" ||
        key == "arguments" || key == "elementId";
}

bool HasOnlyEnvelopeFields(const JsonValue& json)
{
    for (auto field = json.GetChild(); field && field->IsValid(); field = field->GetNext()) {
        if (!IsEnvelopeField(field->GetKey())) {
            return false;
        }
    }
    return true;
}

bool ValidateAccessibilityEnvelope(const JsonValue& json)
{
    auto protocol = json.GetValue("protocol");
    auto version = json.GetValue("version");
    auto actionName = json.GetValue("action");
    return protocol && protocol->IsString() && protocol->GetString() == "arkui.accessibility" &&
        version && version->IsNumber() && version->GetDouble() == 1.0 &&
        actionName && actionName->IsString() && !json.Contains("cmd");
}

bool FindSupportedAction(const std::string& name, Accessibility::ActionType& action)
{
    for (const auto& entry : SUPPORTED_ACTIONS) {
        if (name == entry.first) {
            action = entry.second;
            return true;
        }
    }
    return false;
}

bool IsUnsupportedAction(const std::string& name)
{
    for (const auto* unsupported : UNSUPPORTED_ACTIONS) {
        if (name == unsupported) {
            return true;
        }
    }
    return false;
}

struct IntRange {
    int32_t lower;
    int32_t upper;
};

bool ParseJsonInt32(const std::unique_ptr<JsonValue>& value, const IntRange& range, int32_t& parsed)
{
    if (!value->IsNumber()) {
        return false;
    }
    const double number = value->GetDouble();
    if (!std::isfinite(number) || number != std::trunc(number) || number < range.lower ||
        number > range.upper) {
        return false;
    }
    parsed = static_cast<int32_t>(number);
    return true;
}

bool ReadAccessibilityInteger(const JsonValue& arguments, const std::string& key,
    const IntRange& range, bool required, std::map<std::string, std::string>& result)
{
    auto value = arguments.GetValue(key);
    if (!value || !value->IsValid()) {
        return !required;
    }
    int32_t parsed = 0;
    if (!ParseJsonInt32(value, range, parsed)) {
        return false;
    }
    result[key] = std::to_string(parsed);
    return true;
}

bool ReadRequiredStringArgument(const JsonValue& arguments, const char* key,
    bool allowEmpty, std::map<std::string, std::string>& result)
{
    auto value = arguments.GetValue(key);
    if (!value || !value->IsString() || (!allowEmpty && value->GetString().empty())) {
        return false;
    }
    result[key] = value->GetString();
    return true;
}

bool ReadSelectionDirection(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    auto direction = arguments.GetValue("selectTextInForWard");
    if (!direction || !direction->IsValid()) {
        return true;
    }
    if (!direction->IsString() || (direction->GetString() != "forward" &&
        direction->GetString() != "backward")) {
        return false;
    }
    result["selectTextInForWard"] = direction->GetString();
    return true;
}

bool ValidateSelectionArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    constexpr int32_t maxValue = std::numeric_limits<int32_t>::max();
    if (!ReadAccessibilityInteger(arguments, "selectTextBegin", { 0, maxValue }, true, result) ||
        !ReadAccessibilityInteger(arguments, "selectTextEnd", { 0, maxValue }, true, result)) {
        return false;
    }
    return ReadSelectionDirection(arguments, result);
}

bool ValidateTextMoveArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    constexpr IntRange textMoveUnitRange = { Accessibility::TextMoveUnit::STEP_CHARACTER,
        Accessibility::TextMoveUnit::STEP_PARAGRAPH };
    if (!ReadAccessibilityInteger(arguments, "textMoveUnit", textMoveUnitRange, false, result)) {
        return false;
    }
    auto unit = result.find("textMoveUnit");
    return unit == result.end() || unit->second == "1" || unit->second == "2" ||
        unit->second == "4" || unit->second == "8" || unit->second == "16";
}

bool ValidateScrollArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    return ReadAccessibilityInteger(arguments, "scrolltype", { -1, 1 }, false, result);
}

bool ValidateCursorArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    constexpr int32_t maxValue = std::numeric_limits<int32_t>::max();
    return ReadAccessibilityInteger(arguments, "offset", { 0, maxValue }, true, result);
}

bool ValidateSpanIdArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    constexpr int32_t maxValue = std::numeric_limits<int32_t>::max();
    return ReadAccessibilityInteger(arguments, "spanId", { 0, maxValue }, true, result);
}

bool ValidateSetTextArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    return ReadRequiredStringArgument(arguments, "setText", true, result);
}

bool ValidateCustomActionArguments(const JsonValue& arguments, std::map<std::string, std::string>& result)
{
    return ReadRequiredStringArgument(arguments, "customAction", false, result);
}

using ArgumentValidator = bool (*)(const JsonValue&, std::map<std::string, std::string>&);

constexpr std::pair<Accessibility::ActionType, ArgumentValidator> ARGUMENT_VALIDATORS[] = {
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, ValidateScrollArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, ValidateScrollArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT, ValidateTextMoveArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT, ValidateTextMoveArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT, ValidateSetTextArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM, ValidateCustomActionArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION, ValidateSelectionArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION, ValidateCursorArguments },
    { Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK, ValidateSpanIdArguments },
};

ArgumentValidator FindArgumentValidator(Accessibility::ActionType action)
{
    for (const auto& entry : ARGUMENT_VALIDATORS) {
        if (entry.first == action) {
            return entry.second;
        }
    }
    return nullptr;
}

bool ValidateArgumentValues(const JsonValue& arguments, Accessibility::ActionType action,
    std::map<std::string, std::string>& result)
{
    auto validator = FindArgumentValidator(action);
    return validator == nullptr || validator(arguments, result);
}

bool IsAllowedArgumentKey(Accessibility::ActionType action, const std::string& key)
{
    for (const auto& rule : RESTRICTED_ARGUMENT_KEYS) {
        if (rule.first == action && key == rule.second) {
            return true;
        }
    }
    return false;
}

bool CheckAllowedArgumentKeys(const JsonValue& arguments, Accessibility::ActionType action)
{
    for (auto item = arguments.GetChild(); item && item->IsValid(); item = item->GetNext()) {
        if (!IsAllowedArgumentKey(action, item->GetKey())) {
            return false;
        }
    }
    return true;
}

bool ParseInt64String(const std::string& text, int64_t& parsed)
{
    int64_t value = 0;
    auto parse = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parse.ec != std::errc {} || parse.ptr != text.data() + text.size()) {
        return false;
    }
    parsed = value;
    return true;
}

bool ReadElementIdValue(const JsonValue& json, int64_t& parsed)
{
    auto value = json.GetValue("elementId");
    return value && value->IsString() && ParseInt64String(value->GetString(), parsed);
}
} // namespace

ParseResult ParseAccessibilityActionHeader(const JsonValue& json, Accessibility::ActionType& action)
{
    action = Accessibility::ActionType::ACCESSIBILITY_ACTION_INVALID;
    if (!ValidateAccessibilityEnvelope(json) || !HasOnlyEnvelopeFields(json)) {
        return ParseResult::INVALID_COMMAND;
    }
    const auto actionName = json.GetValue("action")->GetString();
    if (FindSupportedAction(actionName, action)) {
        return ParseResult::SUCCESS;
    }
    return IsUnsupportedAction(actionName) ? ParseResult::UNSUPPORTED_ACTION : ParseResult::INVALID_COMMAND;
}

bool ParseAccessibilityArguments(const JsonValue& json, Accessibility::ActionType action,
    std::map<std::string, std::string>& result)
{
    result.clear();
    auto arguments = json.GetValue("arguments");
    if (!arguments || !arguments->IsObject()) {
        return false;
    }
    if (!CheckAllowedArgumentKeys(*arguments, action)) {
        return false;
    }
    return ValidateArgumentValues(*arguments, action, result);
}

bool ParseElementId(const JsonValue& json, std::optional<int64_t>& elementId)
{
    elementId.reset();
    if (!json.Contains("elementId")) {
        return true;
    }
    int64_t parsed = 0;
    if (!ReadElementIdValue(json, parsed)) {
        return false;
    }
    elementId = parsed;
    return true;
}
} // namespace OHOS::Ace::UiSessionAccessibilityCommand

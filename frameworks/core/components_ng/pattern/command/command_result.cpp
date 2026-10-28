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

#include "core/components_ng/pattern/command/command_validation.h"

#include <cerrno>
#include <cstdlib>
#include <limits>

namespace OHOS::Ace::NG {
namespace {
constexpr double MIN_VIDEO_PLAYBACK_SPEED = 0.125;
constexpr double MAX_VIDEO_PLAYBACK_SPEED = 8.0;

bool IsCommandIndexString(const std::unique_ptr<JsonValue>& params)
{
    auto index = params->GetValue("index");
    if (!index || !index->IsString()) {
        return false;
    }
    auto text = index->GetString();
    if (text.empty() || text.find_first_not_of("0123456789", text.front() == '-' ? 1 : 0) != std::string::npos) {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    auto number = std::strtol(text.c_str(), &end, 10);
    return errno != ERANGE && end == text.c_str() + text.size() && number >= 0 &&
        number <= std::numeric_limits<int32_t>::max();
}

bool ValidatePickerCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    auto params = json->GetValue("params");
    if (!params || !params->IsObject()) {
        return false;
    }
    if (command != "setTimePickerTime" && command != "setTimePickerDialogTime") {
        if (!IsCommandNumber(params, "year", true) || !IsCommandNumber(params, "month", true) ||
            !IsCommandNumber(params, "day", true)) {
            return false;
        }
        if (command != "setDatePickerDialogTime") {
            return true;
        }
    }
    return IsCommandNumber(params, "hour", true) && IsCommandNumber(params, "minute", true) &&
        IsCommandNumber(params, "second", true);
}

bool ValidateDialogCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    auto params = json->GetValue("params");
    if (!params || !params->IsObject()) {
        return false;
    }
    if (command == "actionSheetClick") {
        return IsCommandNumber(params, "index", true) &&
            (params->GetString("clickType") == "sheet" || params->GetString("clickType") == "button");
    }
    return command == "actionMenuClick" && !params->Contains("buttonIndex")
        ? true : IsCommandNumber(params, "buttonIndex", true);
}

bool ValidateSelectionCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    if (command == "selectText") {
        return IsCommandNumber(json, "selectionStart", true) && IsCommandNumber(json, "selectionEnd", true);
    }
    if (command == "setCaretPosition") {
        return IsCommandNumber(json, "position", true);
    }
    auto params = json->GetValue("params");
    if (!params || !params->IsObject()) {
        return false;
    }
    auto value = params->GetValue("value");
    return (!params->Contains("value") || (value && value->IsString())) &&
        (!params->Contains("index") || IsCommandNumber(params, "index", true)) &&
        (params->Contains("value") || params->Contains("index"));
}

bool ValidateToggleCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    if (command == "selectCheckBox" || command == "selectCheckBoxGroup") {
        auto value = json->GetValue("selectStatus");
        return value && value->IsBool();
    }
    auto params = json->GetValue("params");
    if (command == "checked") {
        if (!params || params->IsNull()) {
            return true;
        }
        auto value = params->GetValue("value");
        return params->IsObject() && (!params->Contains("value") || (value && value->IsBool()));
    }
    auto value = params ? params->GetValue("isOn") : nullptr;
    return params && params->IsObject() && value && value->IsBool();
}

bool ValidateVideoCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    if (command == "setVideoPlaybackSpeed") {
        return IsCommandNumber(json, "value") && json->GetDouble("value") >= MIN_VIDEO_PLAYBACK_SPEED &&
            json->GetDouble("value") <= MAX_VIDEO_PLAYBACK_SPEED;
    }
    auto value = json->GetValue("value");
    return value && value->IsString() && (value->GetString() == "play" || value->GetString() == "paused");
}

bool ValidateValueCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    auto params = json->GetValue("params");
    return params && params->IsObject() && IsCommandNumber(params, "value", command == "setAlphabetIndexer");
}

bool ValidateTextCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    auto params = json->GetValue("params");
    if (!params || !params->IsObject()) {
        return false;
    }
    if (command == "deleteText") {
        return (!params->Contains("start") || IsCommandNumber(params, "start", true)) &&
            (!params->Contains("end") || IsCommandNumber(params, "end", true));
    }
    auto value = params->GetValue("value");
    return value && value->IsString() && (!params->Contains("offset") || IsCommandNumber(params, "offset", true));
}

bool ValidateIndexCommand(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    auto params = json->GetValue("params");
    if (!params || !params->IsObject()) {
        return false;
    }
    if (command == "changeIndex") {
        return IsCommandIndexString(params);
    }
    auto type = params->GetString("type");
    return type == "forward" || type == "backward" || (type == "index" && IsCommandIndexString(params));
}

struct CommandValidator {
    const char* command;
    bool (*validate)(const std::unique_ptr<JsonValue>&, const std::string&);
};
constexpr CommandValidator COMMAND_VALIDATORS[] = {
    { "setDatePickerTime", ValidatePickerCommand },
    { "setDatePickerDialogTime", ValidatePickerCommand },
    { "setCalendarPickerDialogTime", ValidatePickerCommand },
    { "setTimePickerTime", ValidatePickerCommand },
    { "setTimePickerDialogTime", ValidatePickerCommand },
    { "alertDialogButtonClick", ValidateDialogCommand },
    { "actionMenuClick", ValidateDialogCommand },
    { "actionSheetClick", ValidateDialogCommand },
    { "onSelect", ValidateSelectionCommand },
    { "selectText", ValidateSelectionCommand },
    { "setCaretPosition", ValidateSelectionCommand },
    { "selectCheckBox", ValidateToggleCommand },
    { "selectCheckBoxGroup", ValidateToggleCommand },
    { "checked", ValidateToggleCommand },
    { "onToggleChange", ValidateToggleCommand },
    { "setVideoPlayerStatus", ValidateVideoCommand },
    { "setVideoPlaybackSpeed", ValidateVideoCommand },
    { "onSliderChange", ValidateValueCommand },
    { "onRatingChange", ValidateValueCommand },
    { "setAlphabetIndexer", ValidateValueCommand },
    { "setText", ValidateTextCommand },
    { "addText", ValidateTextCommand },
    { "setSearchText", ValidateTextCommand },
    { "deleteText", ValidateTextCommand },
    { "changeIndex", ValidateIndexCommand },
    { "change", ValidateIndexCommand },
};
} // namespace

bool ValidateCommandParameters(const std::unique_ptr<JsonValue>& json, const std::string& command)
{
    for (const auto& validator : COMMAND_VALIDATORS) {
        if (command == validator.command) {
            return validator.validate(json, command);
        }
    }
    return true;
}

} // namespace OHOS::Ace::NG
